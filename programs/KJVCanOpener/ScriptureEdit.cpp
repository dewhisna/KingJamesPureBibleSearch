#include "ScriptureEdit.h"

#include "dbstruct.h"

#include <assert.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QEvent>
#include <QHelpEvent>

// ============================================================================

CScriptureEdit::CScriptureEdit(QWidget *parent)
	:	QTextEdit(parent),
		m_bDoingPopup(false),
		m_navigator(*this),
		m_pEditMenu(NULL),
		m_pActionCopy(NULL),
		m_pActionSelectAll(NULL),
		m_pActionCopyReferenceDetails(NULL),
		m_pActionCopyPassageStatistics(NULL),
		m_pActionCopyEntirePassageDetails(NULL)
{
	setMouseTracking(true);
	installEventFilter(this);

	viewport()->setCursor(QCursor(Qt::ArrowCursor));

	m_HighlightTimer.stop();

	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
	connect(&m_navigator, SIGNAL(changedDocumentText()), &m_Highlighter, SLOT(clearPhraseTags()));
	connect(&m_HighlightTimer, SIGNAL(timeout()), this, SLOT(clearHighlighting()));

	m_pEditMenu = new QMenu("&Edit", this);
	m_pEditMenu->setStatusTip("Scripture Editor Operations");
	m_pActionCopy = m_pEditMenu->addAction("&Copy", this, SLOT(copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	m_pActionCopy->setStatusTip("Copy selected passage browser text to the clipboard");
	m_pActionCopy->setEnabled(false);
	connect(this, SIGNAL(copyAvailable(bool)), m_pActionCopy, SLOT(setEnabled(bool)));
	m_pEditMenu->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction("Select &All", this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip("Select all current passage browser text");
	m_pEditMenu->addSeparator();
	m_pActionCopyReferenceDetails = m_pEditMenu->addAction("Copy &Reference Details (Word/Phrase)", this, SLOT(on_copyReferenceDetails()), QKeySequence(Qt::CTRL + Qt::Key_R));
	m_pActionCopyReferenceDetails->setStatusTip("Copy the Word/Phrase Reference Details in the passage browser to the clipboard");
	m_pActionCopyPassageStatistics = m_pEditMenu->addAction("Copy Passage &Statistics (Book/Chapter/Verse)", this, SLOT(on_copyPassageStatistics()), QKeySequence(Qt::CTRL + Qt::Key_S));
	m_pActionCopyPassageStatistics->setStatusTip("Copy the Book/Chapter/Verse Passage Statistics in the passage browser to the clipboard");
	m_pActionCopyEntirePassageDetails = m_pEditMenu->addAction("Copy Entire Passage &Details", this, SLOT(on_copyEntirePassageDetails()), QKeySequence(Qt::CTRL + Qt::Key_D));
	m_pActionCopyEntirePassageDetails->setStatusTip("Copy both the Word/Phrase Reference Detail and Book/Chapter/Verse Statistics in the passage browser to the clipboard");
}

CScriptureEdit::~CScriptureEdit()
{

}

void CScriptureEdit::clearHighlighting()
{
	m_navigator.doHighlighting(m_Highlighter, true);
	m_Highlighter.clearPhraseTags();
	m_HighlightTimer.stop();
}

bool CScriptureEdit::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == this) {
		switch (ev->type()) {
			case QEvent::Wheel:
			case QEvent::ActivationChange:
			case QEvent::KeyPress:
			case QEvent::KeyRelease:
			case QEvent::FocusOut:
			case QEvent::FocusIn:
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonDblClick:
			case QEvent::Leave:
				return false;
			default:
				break;
		}
	}

	return QTextEdit::eventFilter(obj, ev);
}

bool CScriptureEdit::event(QEvent *ev)
{
	if (ev->type() == QEvent::FocusIn) emit activatedScriptureEdit();

	switch (ev->type()) {
		case QEvent::ToolTip:
			{
				QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(ev);
				if (m_navigator.handleToolTipEvent(pHelpEvent, m_Highlighter)) {
					m_HighlightTimer.stop();
				} else {
					pHelpEvent->ignore();
				}
				return true;
			}
			break;

		// User input and window activation makes tooltips sleep
		case QEvent::Wheel:
		case QEvent::ActivationChange:
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		case QEvent::FocusOut:
		case QEvent::FocusIn:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
			// Unfortunately, there doesn't seem to be any event we can hook to to determine
			//		when the ToolTip disappears.  Looking at the Qt code, it looks to be on
			//		a 2 second timeout.  So, we'll do a similar timeout here for the highlight:
			if ((!m_bDoingPopup) && (!m_Highlighter.getHighlightTags().isEmpty()) && (!m_HighlightTimer.isActive()))
				m_HighlightTimer.start(2000);
			break;
		case QEvent::Leave:
			if ((!m_bDoingPopup) && (!m_Highlighter.getHighlightTags().isEmpty())) {
				m_HighlightTimer.start(20);
			}
			break;
		default:
			break;
	}

	return QTextEdit::event(ev);
}

void CScriptureEdit::mouseDoubleClickEvent(QMouseEvent *ev)
{
	m_bDoingPopup = true;
	CRelIndex ndxLast = m_navigator.ResolveCursorReference(cursorForPosition(ev->pos()));
	m_tagLast = TPhraseTag(ndxLast, (ndxLast.isSet() ? 1 : 0));
	m_navigator.highlightTag(m_Highlighter, m_tagLast);
	if (ndxLast.isSet()) emit gotoIndex(m_tagLast);
	m_bDoingPopup = false;
}

void CScriptureEdit::contextMenuEvent(QContextMenuEvent *ev)
{
	m_bDoingPopup = true;
	CRelIndex ndxLast = m_navigator.ResolveCursorReference(cursorForPosition(ev->pos()));
	m_tagLast = TPhraseTag(ndxLast, (ndxLast.isSet() ? 1 : 0));
	m_navigator.highlightTag(m_Highlighter, m_tagLast);
	QMenu *menu = createStandardContextMenu(ev->pos());
	menu->addSeparator();
	menu->addAction(m_pActionCopyReferenceDetails);
	menu->addAction(m_pActionCopyPassageStatistics);
	menu->addAction(m_pActionCopyEntirePassageDetails);
	menu->exec(ev->globalPos());
	delete menu;
	m_bDoingPopup = false;
}

void CScriptureEdit::on_cursorPositionChanged()
{
	CPhraseCursor cursor = textCursor();
	CRelIndex ndxLast = m_navigator.ResolveCursorReference(cursor);
	m_tagLast = TPhraseTag(ndxLast, (ndxLast.isSet() ? 1 : 0));
}

void CScriptureEdit::on_copyReferenceDetails()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, CPhraseEditNavigator::TTE_REFERENCE_ONLY, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, CPhraseEditNavigator::TTE_REFERENCE_ONLY, false));
	clipboard->setMimeData(mime);
}

void CScriptureEdit::on_copyPassageStatistics()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, CPhraseEditNavigator::TTE_STATISTICS_ONLY, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, CPhraseEditNavigator::TTE_STATISTICS_ONLY, false));
	clipboard->setMimeData(mime);
}

void CScriptureEdit::on_copyEntirePassageDetails()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, CPhraseEditNavigator::TTE_COMPLETE, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, CPhraseEditNavigator::TTE_COMPLETE, false));
	clipboard->setMimeData(mime);
}


// ============================================================================


