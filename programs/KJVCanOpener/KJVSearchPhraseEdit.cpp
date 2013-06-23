/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#include "KJVSearchPhraseEdit.h"
#include "ui_KJVSearchPhraseEdit.h"

#include "PhraseListModel.h"
#include "MimeHelper.h"

#ifdef SIGNAL_SPY_DEBUG
#include "main.h"
#endif

#include <QStringListModel>
#include <QTextCharFormat>
#include <QFontMetrics>

#if QT_VERSION < 0x050000
#include <QInputContext>
#endif

#include <QTextDocumentFragment>

#include <QGridLayout>

#include <algorithm>
#include <string>

#include <assert.h>

#define PHRASE_COMPLETER_BUTTON_SIZE_X 24
#define PHRASE_COMPLETER_BUTTON_SIZE_Y 24

// ============================================================================

#if QT_VERSION < 0x050000

bool CComposingCompleter::eventFilter(QObject *obj, QEvent *ev)
{
	// The act of popping our completer, will cause the inputContext to
	//		shift focus from the editor to the popup and after dismissing the
	//		popup, it doesn't go back to the editor.  So, since we are eating
	//		FocusOut events in the popup, push the inputContext focus back to
	//		the editor when we "focus out".  It's our focusProxy anyway:
	if ((ev->type() == QEvent::FocusOut) && (obj == widget())) {
		if ((popup()) && (popup()->isVisible())) {
			QInputContext *pInputContext = popup()->inputContext();
			if (pInputContext) pInputContext->setFocusWidget(popup());
		}
	}

	return QCompleter::eventFilter(obj, ev);
}

#endif

// ============================================================================

// Placeholder Constructor:
CPhraseLineEdit::CPhraseLineEdit(QWidget *pParent)
	:	QTextEdit(pParent),
		m_pCompleter(NULL),
		m_pCommonPhrasesCompleter(NULL),
		m_nLastCursorWord(-1),
		m_bUpdateInProgress(false),
		m_bDoingPopup(false),
		m_icoDroplist(":/res/droplist.png"),
		m_pButtonDroplist(NULL),
		m_pEditMenu(NULL),
		m_pActionSelectAll(NULL),
		m_pStatusAction(NULL)
{

}

CPhraseLineEdit::CPhraseLineEdit(CBibleDatabasePtr pBibleDatabase, QWidget *pParent)
	:	QTextEdit(pParent),
		CParsedPhrase(pBibleDatabase),
		m_pBibleDatabase(pBibleDatabase),
		m_pCompleter(NULL),
		m_pCommonPhrasesCompleter(NULL),
		m_nLastCursorWord(-1),
		m_bUpdateInProgress(false),
		m_bDoingPopup(false),
		m_icoDroplist(":/res/droplist.png"),
		m_pButtonDroplist(NULL),
		m_pEditMenu(NULL),
		m_pActionSelectAll(NULL),
		m_pStatusAction(NULL)
{
	assert(pBibleDatabase.data() != NULL);

	setAcceptRichText(false);
	setUndoRedoEnabled(false);		// TODO : If we ever address what to do with undo/redo, then re-enable this

	setTabChangesFocus(true);
	setWordWrapMode(QTextOption::NoWrap);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(sizeHint().height());
	setLineWrapMode(QTextEdit::NoWrap);

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_PHRASE_SPY
	CMyApplication::createSpy(this);
#endif
#endif

	QAction *pAction;
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pEditMenu->setStatusTip(tr("Search Phrase Editor Operations"));
/*
	TODO : If we ever address what to do with undo/redo, then put this code back in:

	pAction = m_pEditMenu->addAction(tr("&Undo"), this, SLOT(undo()), QKeySequence(Qt::CTRL + Qt::Key_Z));
	pAction->setStatusTip(tr("Undo last operation to the Serach Phrase Editor"));
	pAction->setEnabled(false);
	connect(this, SIGNAL(undoAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction(tr("&Redo"), this, SLOT(redo()), QKeySequence(Qt::CTRL + Qt::Key_Y));
	pAction->setStatusTip(tr("Redo last operation on the Search Phrase Editor"));
	pAction->setEnabled(false);
	connect(this, SIGNAL(redoAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
*/
	pAction = m_pEditMenu->addAction(tr("Cu&t"), this, SLOT(cut()), QKeySequence(Qt::CTRL + Qt::Key_X));
	pAction->setStatusTip(tr("Cut selected text from the Search Phrase Editor to the clipboard"));
	pAction->setEnabled(false);
	connect(this, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction(tr("&Copy"), this, SLOT(copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	pAction->setStatusTip(tr("Copy selected text from the Search Phrase Editor to the clipboard"));
	pAction->setEnabled(false);
	connect(this, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction(tr("&Paste"), this, SLOT(paste()), QKeySequence(Qt::CTRL + Qt::Key_V));
	pAction->setStatusTip(tr("Paste text on clipboard into the Search Phrase Editor"));
	pAction->setEnabled(true);
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction(tr("&Delete"), this, SLOT(clear()), QKeySequence(Qt::Key_Delete));
	pAction->setStatusTip(tr("Delete selected text from the Search Phrase Editor"));
	pAction->setEnabled(false);
	connect(this, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction(tr("Select &All"), this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip(tr("Select All Text in the Search Phrase Editor"));
	m_pActionSelectAll->setEnabled(false);
	connect(m_pActionSelectAll, SIGNAL(triggered()), this, SLOT(setFocus()));

	connect(this, SIGNAL(textChanged()), this, SLOT(en_textChanged()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(en_cursorPositionChanged()));

	CSearchStringListModel *pModel = new CSearchStringListModel(*this, this);
#if QT_VERSION < 0x050000
	m_pCompleter = new CComposingCompleter(pModel, this);
#else
	m_pCompleter = new CSearchCompleter(pModel, this);
#endif
	m_pCompleter->setWidget(this);
	m_pCompleter->setCompletionMode(QCompleter::PopupCompletion /* UnfilteredPopupCompletion */ );
	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
	m_pCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);

	m_pButtonDroplist = new QPushButton(m_icoDroplist, QString(), this);
	m_pButtonDroplist->setFlat(true);
	m_pButtonDroplist->setToolTip(tr("Show Phrase List"));
	m_pButtonDroplist->setStatusTip(tr("Show List of Common Phrases and User Phrases from Database"));
	m_pButtonDroplist->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_pButtonDroplist->setMaximumSize(PHRASE_COMPLETER_BUTTON_SIZE_X, PHRASE_COMPLETER_BUTTON_SIZE_Y);
	m_pButtonDroplist->setGeometry(sizeHint().width()-m_pButtonDroplist->sizeHint().width(),0,
								m_pButtonDroplist->sizeHint().width(),m_pButtonDroplist->sizeHint().height());

	CPhraseList phrases = m_pBibleDatabase->phraseList();
	phrases.append(g_lstUserPhrases);
	phrases.removeDuplicates();
	CPhraseListModel *pCommonPhrasesModel = new CPhraseListModel(phrases, this);
	pCommonPhrasesModel->sort(0, Qt::AscendingOrder);
	m_pCommonPhrasesCompleter = new QCompleter(pCommonPhrasesModel, this);
	m_pCommonPhrasesCompleter->setWidget(this);
	m_pCommonPhrasesCompleter->setCompletionMode(QCompleter::PopupCompletion);
	m_pCommonPhrasesCompleter->setCaseSensitivity(Qt::CaseSensitive);
	m_pCommonPhrasesCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);

//	connect(m_pCompleter, SIGNAL(activated(const QString &)), this, SLOT(insertCompletion(const QString &)));
	connect(m_pCompleter, SIGNAL(activated(const QModelIndex &)), this, SLOT(insertCompletion(const QModelIndex &)));
	connect(m_pButtonDroplist, SIGNAL(clicked()), this, SLOT(en_dropCommonPhrasesClicked()));
	connect(m_pCommonPhrasesCompleter, SIGNAL(activated(const QString &)), this, SLOT(insertCommonPhraseCompletion(const QString&)));

	m_pStatusAction = new QAction(this);
}

CPhraseLineEdit::~CPhraseLineEdit()
{

}

void CPhraseLineEdit::setCaseSensitive(bool bCaseSensitive)
{
	CParsedPhrase::setCaseSensitive(bCaseSensitive);
	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);

	if (!m_bUpdateInProgress) {
		UpdateCompleter();
		emit phraseChanged();
		emit changeCaseSensitive(bCaseSensitive);
	}
}

QSize CPhraseLineEdit::sizeHint()
{
	QFontMetrics fm(font());
	int h = qMax(fm.height(), 14) + 4;
	int w = fm.width(QLatin1Char('x')) * 17 + 4;
	QStyleOptionFrameV2 opt;
	opt.initFrom(this);
	QSize szHint = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).
											 expandedTo(QApplication::globalStrut()), this);
	return (QSize(szHint.width(), qMax(szHint.height(), PHRASE_COMPLETER_BUTTON_SIZE_Y)));
}

void CPhraseLineEdit::en_phraseListChanged()
{
	assert(m_pBibleDatabase.data() != NULL);

	assert(m_pCommonPhrasesCompleter != NULL);
	if (m_pCommonPhrasesCompleter == NULL) return;

	CPhraseListModel *pModel = (CPhraseListModel *)(m_pCommonPhrasesCompleter->model());
	assert(pModel != NULL);
	if (pModel == NULL) return;

	CPhraseList phrases = m_pBibleDatabase->phraseList();
	phrases.append(g_lstUserPhrases);
	phrases.removeDuplicates();
	pModel->setPhraseList(phrases);
	pModel->sort(0, Qt::AscendingOrder);
}

void CPhraseLineEdit::insertCompletion(const QString& completion)
{
	CParsedPhrase::insertCompletion(textCursor(), completion);
}

void CPhraseLineEdit::insertCompletion(const QModelIndex &index)
{
//	CSearchStringListModel *pModel = (CSearchStringListModel *)(aCompleter.model());

	CParsedPhrase::insertCompletion(textCursor(), index.data(Qt::DisplayRole).toString());
}

void CPhraseLineEdit::insertCommonPhraseCompletion(const QString &completion)
{
	CPhraseCursor cursor(textCursor());
	cursor.clearSelection();
	cursor.select(QTextCursor::LineUnderCursor);
	bool bUpdateSave = m_bUpdateInProgress;
	m_bUpdateInProgress = true;									// Hold-over to update everything at once
	if (completion.startsWith(QChar(0xA7))) {
		cursor.insertText(completion.mid(1));					// Replace with complete word minus special flag
		setCaseSensitive(true);
	} else {
		cursor.insertText(completion);							// Replace with completed word
		setCaseSensitive(false);
	}
	m_bUpdateInProgress = bUpdateSave;
	if (!m_bUpdateInProgress) {
		UpdateCompleter();
		emit phraseChanged();
		emit changeCaseSensitive(isCaseSensitive());
	}
}

QString CPhraseLineEdit::textUnderCursor() const
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::WordUnderCursor);
	return cursor.selectedText();
}

void CPhraseLineEdit::en_textChanged()
{
	if (!m_bUpdateInProgress) {
		UpdateCompleter();

		emit phraseChanged();
	}

	m_pActionSelectAll->setEnabled(!document()->isEmpty());
}

void CPhraseLineEdit::en_cursorPositionChanged()
{
	if (!m_bUpdateInProgress) UpdateCompleter();
}

void CPhraseLineEdit::UpdateCompleter()
{
	CParsedPhrase::UpdateCompleter(textCursor(), *m_pCompleter);

	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;

	QTextCursor saveCursor = textCursor();
	saveCursor.clearSelection();

	CPhraseCursor cursor(textCursor());
	QTextCharFormat fmt = cursor.charFormat();
	fmt.setFontStrikeOut(false);
	fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.setCharFormat(fmt);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	int nWord = 0;
	do {
		cursor.selectWordUnderCursor();
		if (/* (GetCursorWordPos() != nWord) && */
			(static_cast<int>(GetMatchLevel()) <= nWord) &&
			(static_cast<int>(GetCursorMatchLevel()) <= nWord) &&
			((nWord != GetCursorWordPos()) ||
			 ((!GetCursorWord().isEmpty()) && (nWord == GetCursorWordPos()))
			 )
			) {
			fmt.setFontStrikeOut(true);
			fmt.setUnderlineColor(QColor(255,0,0));
			fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
			cursor.setCharFormat(fmt);
		}

		nWord++;
	} while (cursor.moveCursorWordRight(QTextCursor::MoveAnchor));

	m_bUpdateInProgress = false;
}

void CPhraseLineEdit::ParsePhrase(const QTextCursor &curInsert)
{
	// TODO : Remove this function after done debugging!

	CParsedPhrase::ParsePhrase(curInsert);

/*
	if (m_pStatusAction) {
		QString strTemp;
//		for (int n=0; n<m_lstWords.size(); ++n) {
//			if (n==m_nCursorWord) strTemp += "(";
//			strTemp += m_lstWords[n];
//			if (n==m_nCursorWord) strTemp += ")";
//			strTemp += " ";
//		}
//		strTemp += QString("  Cursor: %1  CursorLevel: %2  Level: %3  Words: %4").arg(m_nCursorWord).arg(m_nCursorLevel).arg(m_nLevel).arg(m_lstWords.size());
		strTemp = QString("MatchLevel: %1  PhraseSize: %2").arg(GetMatchLevel()).arg(phraseSize());
		setStatusTip(strTemp);
		m_pStatusAction->setStatusTip(strTemp);
		m_pStatusAction->showStatusText();
	}
*/

}

bool CPhraseLineEdit::canInsertFromMimeData(const QMimeData *source) const
{
	if (source->hasFormat(g_constrPhraseTagMimeType)) {
		return true;
	} else {
		return QTextEdit::canInsertFromMimeData(source);
	}
}

void CPhraseLineEdit::insertFromMimeData(const QMimeData * source)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (!(textInteractionFlags() & Qt::TextEditable) || !source) return;

// For reference if we ever re-enable rich text:  (don't forget to change acceptRichText setting in constructor)
//	if (source->hasFormat(QLatin1String("application/x-qrichtext")) && acceptRichText()) {
//		// x-qrichtext is always UTF-8 (taken from Qt3 since we don't use it anymore).
//		QString richtext = QString::fromUtf8(source->data(QLatin1String("application/x-qrichtext")));
//		richtext.prepend(QLatin1String("<meta name=\"qrichtext\" content=\"1\" />"));
//		fragment = QTextDocumentFragment::fromHtml(richtext, document());
//		bHasData = true;
//	} else if (source->hasHtml() && acceptRichText()) {
//		fragment = QTextDocumentFragment::fromHtml(source->html(), document());
//		bHasData = true;
//	} else {


	if (source->hasFormat(g_constrPhraseTagMimeType)) {
		QString strPhrase;
		TPhraseTag tag = CMimeHelper::getPhraseTagFromMimeData(source);
		uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(tag.relIndex());
		if ((ndxNormal != 0) && (tag.count() > 0)) {
			for (unsigned int ndx = 0; ((ndx < tag.count()) && ((ndxNormal + ndx) <= m_pBibleDatabase->bibleEntry().m_nNumWrd)); ++ndx) {
				if (ndx) strPhrase += " ";
				strPhrase += m_pBibleDatabase->wordAtIndex(ndxNormal + ndx);
			}
			clear();
			setText(strPhrase);
		}
	} else if (source->hasText()) {
		bool bHasData = false;
		QTextDocumentFragment fragment;
		QString text = source->text();
		// Change all newlines to spaces, since we are simulating a single-line editor:
		if (!text.isNull()) {
			text.replace("\r","");
			text.replace("\n"," ");
			if (!text.isEmpty()) {
				fragment = QTextDocumentFragment::fromPlainText(text);
				bHasData = true;
			}
		}
		if (bHasData) textCursor().insertFragment(fragment);
	}

	ensureCursorVisible();
}

void CPhraseLineEdit::wheelEvent(QWheelEvent *event)
{
	event->ignore();
}

void CPhraseLineEdit::focusInEvent(QFocusEvent *event)
{
	emit activatedPhraseEditor(this);
	QTextEdit::focusInEvent(event);

#if QT_VERSION < 0x050000
	// The following is needed to fix the QCompleter bug
	//	where the inputContext doesn't shift correctly
	//	from the QCompleter->popup back to the editor:
	//	(Only applies to Qt 4.8.x and seems to be fixed in Qt5)
	QInputContext *pInputContext = inputContext();
	if (pInputContext) pInputContext->setFocusWidget(this);
#endif
}

void CPhraseLineEdit::keyPressEvent(QKeyEvent* event)
{
	bool bForceCompleter = false;

//	if (m_pCompleter->popup()->isVisible())
//	{
		switch (event->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
				event->ignore();
				return;

			case Qt::Key_Down:
				bForceCompleter = true;
				break;
		}
//	}

	QTextEdit::keyPressEvent(event);

	setupCompleter(event->text(), bForceCompleter);
}

void CPhraseLineEdit::inputMethodEvent(QInputMethodEvent *event)
{
	// Call parent:
	QTextEdit::inputMethodEvent(event);
	setupCompleter(QString(), true);
}

void CPhraseLineEdit::setupCompleter(const QString &strText, bool bForce)
{
	ParsePhrase(textCursor());

	QString strPrefix = GetCursorWord();
	int nPreRegExp = strPrefix.indexOf(QRegExp("[\\[\\]\\*\\?]"));
	if (nPreRegExp != -1) strPrefix = strPrefix.left(nPreRegExp);
	strPrefix = CSearchStringListModel::decompose(strPrefix);

	if (strPrefix != m_pCompleter->completionPrefix()) {
		m_pCompleter->setCompletionPrefix(strPrefix);
		UpdateCompleter();
		if (m_nLastCursorWord != GetCursorWordPos()) {
			m_pCompleter->popup()->close();
			m_nLastCursorWord = GetCursorWordPos();
		}
		m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index(0, 0));
	}

	if (bForce || (!strText.isEmpty() && ((GetCursorWord().length() > 0) || (textCursor().atEnd()))))
		m_pCompleter->complete();
}

void CPhraseLineEdit::resizeEvent(QResizeEvent * /* event */)
{
	m_pButtonDroplist->move(width()-m_pButtonDroplist->width(),0);
}

void CPhraseLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
	m_bDoingPopup = true;
	m_pEditMenu->exec(event->globalPos());
	m_bDoingPopup = false;
}

void CPhraseLineEdit::en_dropCommonPhrasesClicked()
{
	m_pCommonPhrasesCompleter->complete();
}

// ============================================================================

CKJVSearchPhraseEdit::CKJVSearchPhraseEdit(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase, QWidget *parent) :
	QWidget(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_bHaveUserDatabase(bHaveUserDatabase),
	m_bLastPhraseChangeHadResults(false),
	m_bUpdateInProgress(false),
	ui(new Ui::CKJVSearchPhraseEdit)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui->setupUi(this);

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_PHRASE_SPY
	CMyApplication::createSpy(this);
#endif
#endif

	// --------------------------------------------------------------

	//	Swapout the editPhrase from the layout with one that we
	//		can set the database on:

	int ndx = ui->gridLayout->indexOf(ui->editPhrase);
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;
	ui->gridLayout->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	CPhraseLineEdit *pEditPhrase = new CPhraseLineEdit(pBibleDatabase, this);
	pEditPhrase->setObjectName(QString::fromUtf8("editPhrase"));
	delete ui->editPhrase;
	ui->editPhrase = pEditPhrase;
	ui->gridLayout->addWidget(pEditPhrase, nRow, nCol, nRowSpan, nColSpan);
	setTabOrder(ui->editPhrase, ui->chkCaseSensitive);
	setTabOrder(ui->chkCaseSensitive, ui->editPhrase->getDropListButton());
	setTabOrder(ui->editPhrase->getDropListButton(), ui->buttonRemove);

	// --------------------------------------------------------------

	ui->chkCaseSensitive->setChecked(ui->editPhrase->isCaseSensitive());
	ui->buttonAddPhrase->setEnabled(false);
	ui->buttonAddPhrase->setToolTip(tr("Add Phrase to User Database"));
	ui->buttonAddPhrase->setStatusTip(tr("Add this Phrase to the User Database"));
	ui->buttonDelPhrase->setEnabled(false);
	ui->buttonDelPhrase->setToolTip(tr("Delete Phrase from User Database"));
	ui->buttonDelPhrase->setStatusTip(tr("Delete this Phrase from the User Database"));
	ui->buttonClear->setEnabled(false);
	ui->buttonClear->setToolTip(tr("Clear Phrase Text"));
	ui->buttonClear->setStatusTip(tr("Clear this Phrase Text"));

	ui->editPhrase->setToolTip(tr("Enter Word or Phrase to Search"));
	ui->editPhrase->setStatusTip(tr("Enter Word or Phrase to Search"));

	ui->buttonRemove->setToolTip(tr("Remove Phrase from Search Criteria"));
	ui->buttonRemove->setStatusTip(tr("Remove this Phrase from the current Search Criteria"));

	connect(ui->editPhrase, SIGNAL(phraseChanged()), &m_dlyTextChanged, SLOT(trigger()));
	connect(&m_dlyTextChanged, SIGNAL(triggered()), this, SLOT(en_phraseChanged()));

	connect(ui->chkCaseSensitive, SIGNAL(clicked(bool)), this, SLOT(en_CaseSensitiveChanged(bool)));
	connect(ui->editPhrase, SIGNAL(changeCaseSensitive(bool)), this, SLOT(en_CaseSensitiveChanged(bool)));
	connect(ui->buttonAddPhrase, SIGNAL(clicked()), this, SLOT(en_phraseAdd()));
	connect(ui->buttonDelPhrase, SIGNAL(clicked()), this, SLOT(en_phraseDel()));
	connect(ui->buttonClear, SIGNAL(clicked()), this, SLOT(en_phraseClear()));
	connect(this, SIGNAL(phraseListChanged()), ui->editPhrase, SLOT(en_phraseListChanged()));
	connect(ui->editPhrase, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)), this, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)));
	connect(ui->buttonRemove, SIGNAL(clicked()), this, SLOT(closeSearchPhrase()));
}

CKJVSearchPhraseEdit::~CKJVSearchPhraseEdit()
{
	delete ui;
}

void CKJVSearchPhraseEdit::closeSearchPhrase()
{
	emit closingSearchPhrase(this);
	delete this;
}

void CKJVSearchPhraseEdit::showSeperatorLine(bool bShow)
{
	ui->lineSeparator->setVisible(bShow);
	adjustSize();
}

void CKJVSearchPhraseEdit::enableCloseButton(bool bEnable)
{
	ui->buttonRemove->setEnabled(bEnable);
}

void CKJVSearchPhraseEdit::focusEditor() const
{
	ui->editPhrase->setFocus();
}

const CParsedPhrase *CKJVSearchPhraseEdit::parsedPhrase() const
{
	return ui->editPhrase;
}

CPhraseLineEdit *CKJVSearchPhraseEdit::phraseEditor() const
{
	return ui->editPhrase;
}

void CKJVSearchPhraseEdit::en_phraseChanged()
{
	assert(m_pBibleDatabase.data() != NULL);

	const CParsedPhrase *pPhrase = parsedPhrase();
	assert(pPhrase != NULL);

	m_phraseEntry.m_strPhrase=pPhrase->phrase();		// Use reconstituted phrase for save/restore
	m_phraseEntry.m_bCaseSensitive=pPhrase->isCaseSensitive();
	m_phraseEntry.m_nNumWrd=pPhrase->phraseSize();

	bool bCommonFound = m_pBibleDatabase->phraseList().contains(m_phraseEntry);
	bool bUserFound = g_lstUserPhrases.contains(m_phraseEntry);
	bool bHaveText = (!m_phraseEntry.m_strPhrase.isEmpty());
	ui->buttonAddPhrase->setEnabled(m_bHaveUserDatabase && bHaveText && !bUserFound && !bCommonFound);
	ui->buttonDelPhrase->setEnabled(m_bHaveUserDatabase && bHaveText && bUserFound);
	ui->buttonClear->setEnabled(!ui->editPhrase->toPlainText().isEmpty());

	// If last time, this phrase didn't have anything meaningful, if it still doesn't
	//		then there's no need to send a notification as the overall search results
	//		still won't be affected:
	if (!m_bLastPhraseChangeHadResults) {
		if ((!pPhrase->isCompleteMatch()) || (pPhrase->GetNumberOfMatches() == 0)) {
			pPhrase->SetIsDuplicate(false);
			pPhrase->ClearScopedPhraseTagSearchResults();
			phraseStatisticsChanged();
		} else {
			emit phraseChanged(this);
			m_bLastPhraseChangeHadResults = true;
		}
	} else {
		emit phraseChanged(this);
		m_bLastPhraseChangeHadResults = ((pPhrase->isCompleteMatch()) && (pPhrase->GetNumberOfMatches() != 0));
	}

	// Note: No need to call phraseStatisticsChanged() here as the parent
	//		CKJVCanOpener will be calling it for everyone (as a result of
	//		the above emit statements, since not only does this editor
	//		need to be updated, but all editors
}

void CKJVSearchPhraseEdit::phraseStatisticsChanged() const
{
	QString strTemp = tr("Number of Occurrences: ");
	if (parsedPhrase()->IsDuplicate()) {
		strTemp += tr("(Duplicate)");
	} else {
		strTemp += QString("%1/%2").arg(parsedPhrase()->GetContributingNumberOfMatches()).arg(parsedPhrase()->GetNumberOfMatches());
	}
	ui->lblOccurrenceCount->setText(strTemp);
}

void CKJVSearchPhraseEdit::en_CaseSensitiveChanged(bool bCaseSensitive)
{
	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;
	ui->chkCaseSensitive->setChecked(bCaseSensitive);		// Set the checkbox in case the phrase editor is setting us
	ui->editPhrase->setCaseSensitive(bCaseSensitive);		// Set the phrase editor in case the checkbox is setting us
	m_bUpdateInProgress = false;
}

void CKJVSearchPhraseEdit::en_phraseAdd()
{
	g_lstUserPhrases.push_back(m_phraseEntry);
	g_bUserPhrasesDirty = true;
	ui->buttonAddPhrase->setEnabled(false);
	ui->buttonDelPhrase->setEnabled(m_bHaveUserDatabase && !parsedPhrase()->phrase().isEmpty());
	emit phraseListChanged();
}

void CKJVSearchPhraseEdit::en_phraseDel()
{
	int ndx = g_lstUserPhrases.indexOf(m_phraseEntry);
	assert(ndx != -1);		// Shouldn't be in this handler if it didn't exist!!  What happened?
	if (ndx >= 0) {
		g_lstUserPhrases.removeAt(ndx);
		g_bUserPhrasesDirty = true;
	}
	ui->buttonAddPhrase->setEnabled(m_bHaveUserDatabase && !parsedPhrase()->phrase().isEmpty());
	ui->buttonDelPhrase->setEnabled(false);
	emit phraseListChanged();
}

void CKJVSearchPhraseEdit::en_phraseClear()
{
	ui->editPhrase->clear();
}

