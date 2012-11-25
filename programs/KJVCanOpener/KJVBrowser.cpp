#include "KJVBrowser.h"
#include "ui_KJVBrowser.h"

#include "dbstruct.h"

#include "KJVPassageNavigatorDlg.h"

#include <assert.h>

#include <QHelpEvent>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>

// ============================================================================

CScriptureBrowser::CScriptureBrowser(QWidget *parent)
	:	QTextBrowser(parent),
		m_navigator(*this)
{
	setMouseTracking(true);
	installEventFilter(this);

	m_HighlightTimer.stop();

	connect(&m_navigator, SIGNAL(changedDocumentText()), &m_Highlighter, SLOT(clearPhraseTags()));
	connect(&m_HighlightTimer, SIGNAL(timeout()), this, SLOT(clearHighlighting()));
}

CScriptureBrowser::~CScriptureBrowser()
{

}

void CScriptureBrowser::clearHighlighting()
{
	m_navigator.doHighlighting(m_Highlighter, true);
	m_Highlighter.clearPhraseTags();
	m_HighlightTimer.stop();
}

bool CScriptureBrowser::eventFilter(QObject *obj, QEvent *ev)
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

	return QTextBrowser::eventFilter(obj, ev);
}

bool CScriptureBrowser::event(QEvent *e)
{
	switch (e->type()) {
		case QEvent::ToolTip:
			{
				QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(e);
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
			if ((!m_Highlighter.getHighlightTags().isEmpty()) && (!m_HighlightTimer.isActive()))
				m_HighlightTimer.start(2000);
			break;
		case QEvent::Leave:
			if (!m_Highlighter.getHighlightTags().isEmpty()) {
				m_HighlightTimer.start(20);
			}
			break;
		default:
			break;
	}

	return QTextBrowser::event(e);
}

void CScriptureBrowser::mouseDoubleClickEvent(QMouseEvent * e)
{
	CRelIndex ndxReference = m_navigator.ResolveCursorReference(cursorForPosition(e->pos()));
	if (ndxReference.isSet()) {
		CKJVPassageNavigatorDlg dlg(parentWidget());
		dlg.navigator().startRelativeMode(ndxReference, false);
		if (dlg.exec() == QDialog::Accepted) {
			emit gotoIndex(dlg.passage(), 1);
		}
	}
}

// ============================================================================

CKJVBrowser::CKJVBrowser(QWidget *parent) :
	QWidget(parent),
	m_ndxCurrent(0),
	m_bDoingUpdate(false),
	ui(new Ui::CKJVBrowser)
{
	ui->setupUi(this);

	ui->textBrowserMainText->setStyleSheet("font: 12pt \"Times New Roman\";");

	Initialize();

// UI Connections:
	connect(ui->textBrowserMainText, SIGNAL(gotoIndex(const CRelIndex &, unsigned int)), this, SLOT(gotoIndex(const CRelIndex &, unsigned int)));
	connect(ui->textBrowserMainText, SIGNAL(sourceChanged(const QUrl &)), this, SLOT(on_sourceChanged(const QUrl &)));

	connect(ui->comboBk, SIGNAL(currentIndexChanged(int)), this, SLOT(BkComboIndexChanged(int)));
	connect(ui->comboBkChp, SIGNAL(currentIndexChanged(int)), this, SLOT(BkChpComboIndexChanged(int)));
	connect(ui->comboTstBk, SIGNAL(currentIndexChanged(int)), this, SLOT(TstBkComboIndexChanged(int)));
	connect(ui->comboTstChp, SIGNAL(currentIndexChanged(int)), this, SLOT(TstChpComboIndexChanged(int)));
	connect(ui->comboBibleBk, SIGNAL(currentIndexChanged(int)), this, SLOT(BibleBkComboIndexChanged(int)));
	connect(ui->comboBibleChp, SIGNAL(currentIndexChanged(int)), this, SLOT(BibleChpComboIndexChanged(int)));
}

CKJVBrowser::~CKJVBrowser()
{
	delete ui;
}

CScriptureBrowser *CKJVBrowser::browser()
{
	return ui->textBrowserMainText;
}

// ----------------------------------------------------------------------------

void CKJVBrowser::Initialize(const CRelIndex &nInitialIndex)
{
	begin_update();

	unsigned int nBibleChp = 0;
	ui->comboBk->clear();
	ui->comboBibleBk->clear();
	for (unsigned int ndxBk=0; ndxBk<g_lstTOC.size(); ++ndxBk) {
		ui->comboBk->addItem(g_lstTOC[ndxBk].m_strBkName, ndxBk+1);
		ui->comboBibleBk->addItem(QString("%1").arg(ndxBk+1), ndxBk+1);
		nBibleChp += g_lstTOC[ndxBk].m_nNumChp;
	}
	ui->comboBibleChp->clear();
	for (unsigned int ndxBibleChp=0; ndxBibleChp<nBibleChp; ++ ndxBibleChp) {
		ui->comboBibleChp->addItem(QString("%1").arg(ndxBibleChp+1), ndxBibleChp+1);
	}

	end_update();

	if (nInitialIndex.isSet()) gotoIndex(nInitialIndex);
}

void CKJVBrowser::gotoIndex(const CRelIndex &ndx, unsigned int nWrdCount)
{
	begin_update();
	ui->textBrowserMainText->setSource(QString("#%1").arg(ndx.asAnchor()));
	end_update();

	gotoIndex2(ndx, nWrdCount);
}

void CKJVBrowser::gotoIndex2(const CRelIndex &ndx, unsigned int nWrdCount)
{
	setBook(ndx);
	setChapter(ndx);
	setVerse(ndx);
	setWord(ndx, nWrdCount);

	doHighlighting();

	emit IndexChanged(ndx);
}

void CKJVBrowser::on_sourceChanged(const QUrl &src)
{
	if (m_bDoingUpdate) return;

	QString strURL = src.toString();		// Internal URLs are in the form of "#nnnnnnnn" as anchors
	int nPos = strURL.indexOf('#');
	if (nPos > -1) {
		CRelIndex ndxRel(strURL.mid(nPos+1));
		if (ndxRel.isSet()) gotoIndex2(ndxRel);
	}
}

void CKJVBrowser::focusBrowser()
{
	ui->textBrowserMainText->setFocus();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setHighlightTags(const TPhraseTagList &lstPhraseTags)
{
	doHighlighting(true);				// Remove existing highlighting
	m_Highlighter.setHighlightTags(lstPhraseTags);
	doHighlighting();					// Highlight using new tags
}

void CKJVBrowser::doHighlighting(bool bClear)
{
	ui->textBrowserMainText->navigator().doHighlighting(m_Highlighter, bClear, m_ndxCurrent);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::on_Bible_Beginning()
{
	gotoIndex(CRelIndex(1,1,1,1));
}

void CKJVBrowser::on_Bible_Ending()
{
	CRelIndex ndx;
	ndx.setBook(g_lstTOC.size());
	ndx.setChapter(g_lstTOC.at(g_lstTOC.size()-1).m_nNumChp);
	ndx.setVerse(g_mapLayout[CRelIndex(ndx.book(), ndx.chapter(), 0, 0)].m_nNumVrs);
	ndx.setWord((g_lstBooks[ndx.book()-1])[CRelIndex(0, ndx.chapter(), ndx.verse(), 0)].m_nNumWrd);
	gotoIndex(ndx);
}

void CKJVBrowser::on_Book_Backward()
{
	if (m_ndxCurrent.book() < 2) return;

	gotoIndex(CRelIndex(m_ndxCurrent.book()-1, 1, 1, 1));
}

void CKJVBrowser::on_Book_Forward()
{
	if (m_ndxCurrent.book() >= g_lstTOC.size()) return;

	gotoIndex(CRelIndex(m_ndxCurrent.book()+1, 1, 1, 1));
}

void CKJVBrowser::on_ChapterBackward()
{
	CRelIndex ndx = CRefCountCalc::calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), true);
	if (ndx.isSet()) gotoIndex(ndx);
}

void CKJVBrowser::on_ChapterForward()
{
	CRelIndex ndx = CRefCountCalc::calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), false);
	if (ndx.isSet()) gotoIndex(ndx);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setBook(const CRelIndex &ndx)
{
	if (ndx.book() == 0) return;
	if (ndx.book() == m_ndxCurrent.book()) return;

	begin_update();

	m_ndxCurrent.setIndex(ndx.book(), 0, 0, 0);

	if (m_ndxCurrent.book() > g_lstTOC.size()) {
		assert(false);
		end_update();
		return;
	}

	const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];

	ui->comboBk->setCurrentIndex(ui->comboBk->findData(m_ndxCurrent.book()));
	ui->comboBibleBk->setCurrentIndex(ui->comboBibleBk->findData(m_ndxCurrent.book()));

	unsigned int nTst = toc.m_nTstNdx;
	ui->lblTestament->setText(g_lstTestaments[nTst-1].m_strTstName + ":");
	ui->comboTstBk->clear();
	for (unsigned int ndxTstBk=0; ndxTstBk<g_lstTestaments[nTst-1].m_nNumBk; ++ndxTstBk) {
		ui->comboTstBk->addItem(QString("%1").arg(ndxTstBk+1), ndxTstBk+1);
	}
	ui->comboTstBk->setCurrentIndex(ui->comboTstBk->findData(toc.m_nTstBkNdx));

	ui->comboBkChp->clear();
	for (unsigned int ndxBkChp=0; ndxBkChp<toc.m_nNumChp; ++ndxBkChp) {
		ui->comboBkChp->addItem(QString("%1").arg(ndxBkChp+1), ndxBkChp+1);
	}
	ui->comboTstChp->clear();
	for (unsigned int ndxTstChp=0; ndxTstChp<g_lstTestaments[nTst-1].m_nNumChp; ++ndxTstChp) {
		ui->comboTstChp->addItem(QString("%1").arg(ndxTstChp+1), ndxTstChp+1);
	}

	end_update();
}

void CKJVBrowser::setChapter(const CRelIndex &ndx)
{
	begin_update();

	m_ndxCurrent.setIndex(m_ndxCurrent.book(), ndx.chapter(), 0, 0);

	ui->comboBkChp->setCurrentIndex(ui->comboBkChp->findData(ndx.chapter()));

	if ((m_ndxCurrent.book() == 0) || (m_ndxCurrent.chapter() == 0)) {
		ui->textBrowserMainText->clear();
		end_update();
		return;
	}

	if (m_ndxCurrent.book() > g_lstTOC.size()) {
		assert(false);
		end_update();
		return;
	}

	const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
	unsigned int nTstChp = 0;
	unsigned int nBibleChp = 0;
	for (unsigned int ndxBk=0; ndxBk<(m_ndxCurrent.book()-1); ++ndxBk) {
		if (g_lstTOC[ndxBk].m_nTstNdx == toc.m_nTstNdx)
			nTstChp += g_lstTOC[ndxBk].m_nNumChp;
		nBibleChp += g_lstTOC[ndxBk].m_nNumChp;
	}
	nTstChp += m_ndxCurrent.chapter();
	nBibleChp += m_ndxCurrent.chapter();

	ui->comboTstChp->setCurrentIndex(ui->comboTstChp->findData(nTstChp));
	ui->comboBibleChp->setCurrentIndex(ui->comboBibleChp->findData(nBibleChp));

	end_update();

	ui->textBrowserMainText->navigator().setDocumentToChapter(ndx);
}

void CKJVBrowser::setVerse(const CRelIndex &ndx)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndx.verse(), 0);

}

void CKJVBrowser::setWord(const CRelIndex &ndx, unsigned int nWrdCount)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), ndx.word());
	ui->textBrowserMainText->navigator().selectWords(ndx, nWrdCount);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::BkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui->comboBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::BkChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	ndxTarget.setBook(m_ndxCurrent.book());
	if (index != -1) {
		ndxTarget.setChapter(ui->comboBkChp->itemData(index).toUInt());
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::TstBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get TOC for current book so we know what testament we're currently in:
		const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
		ndxTarget = CRefCountCalc::calcRelIndex(0, 0, 0, ui->comboTstBk->itemData(index).toUInt(), toc.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::TstChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get TOC for current book so we know what testament we're currently in:
		const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
		ndxTarget = CRefCountCalc::calcRelIndex(0, 0, ui->comboTstChp->itemData(index).toUInt(), 0, toc.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::BibleBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui->comboBibleBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::BibleChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget = CRefCountCalc::calcRelIndex(0, 0, ui->comboBibleChp->itemData(index).toUInt(), 0, 0);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(ndxTarget);
}

// ----------------------------------------------------------------------------

