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

#include "PhraseListModel.h"
#include "MimeHelper.h"
#include "PersistentSettings.h"

#ifdef SIGNAL_SPY_DEBUG
#include "myApplication.h"
#endif

#include <QTextCharFormat>
#include <QGridLayout>

#include <algorithm>
#include <string>

#include <assert.h>

#define PHRASE_COMPLETER_BUTTON_SIZE_X 24
#define PHRASE_COMPLETER_BUTTON_SIZE_Y 24

// ============================================================================

// Placeholder Constructor:
CPhraseLineEdit::CPhraseLineEdit(QWidget *pParent)
	:	CSingleLineTextEdit(PHRASE_COMPLETER_BUTTON_SIZE_Y, pParent),
		m_pCompleter(NULL),
		m_pCommonPhrasesCompleter(NULL),
		m_nLastCursorWord(-1),
		m_bDoingPopup(false),
		m_icoDroplist(":/res/droplist.png"),
		m_pButtonDroplist(NULL),
		m_pEditMenu(NULL),
		m_pActionSelectAll(NULL),
		m_pStatusAction(NULL)
{

}

CPhraseLineEdit::CPhraseLineEdit(CBibleDatabasePtr pBibleDatabase, QWidget *pParent)
	:	CSingleLineTextEdit(PHRASE_COMPLETER_BUTTON_SIZE_Y, pParent),
		CParsedPhrase(pBibleDatabase),
		m_pBibleDatabase(pBibleDatabase),
		m_pCompleter(NULL),
		m_pCommonPhrasesCompleter(NULL),
		m_nLastCursorWord(-1),
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
	pAction->setStatusTip(tr("Undo last operation to the Search Phrase Editor"));
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

	m_dlyUpdateCompleter.setMinimumDelay(10);				// Arbitrary time, but I think it must be less than our textChanged delay or we may have issues
	connect(&m_dlyUpdateCompleter, SIGNAL(triggered()), this, SLOT(delayed_UpdatedCompleter()));

	m_pCompleter = new SearchCompleter_t(*this, this);
//	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
	// TODO : ??? Add AccentSensitivity to completer ???

	m_pCompleter->setCompletionFilterMode(CPersistentSettings::instance()->searchPhraseCompleterFilterMode());
	m_pCompleter->selectFirstMatchString();					// Speed up the first popup a little by calculating our first match (of "") early

	m_pButtonDroplist = new QPushButton(m_icoDroplist, QString(), this);
	m_pButtonDroplist->setFlat(true);
	m_pButtonDroplist->setToolTip(tr("Show Phrase List"));
	m_pButtonDroplist->setStatusTip(tr("Show List of Common Phrases and User Phrases from Database"));
	m_pButtonDroplist->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_pButtonDroplist->setMaximumSize(PHRASE_COMPLETER_BUTTON_SIZE_X, PHRASE_COMPLETER_BUTTON_SIZE_Y);
	m_pButtonDroplist->setGeometry(sizeHint().width()-m_pButtonDroplist->sizeHint().width(),0,
								m_pButtonDroplist->sizeHint().width(),m_pButtonDroplist->sizeHint().height());

	m_pCommonPhrasesCompleter = new QCompleter(this);
	m_pCommonPhrasesCompleter->setWidget(this);
	m_pCommonPhrasesCompleter->setCompletionMode(QCompleter::PopupCompletion);
	m_pCommonPhrasesCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_pCommonPhrasesCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	CPhraseListModel *pCommonPhrasesModel = new CPhraseListModel(m_pCommonPhrasesCompleter);
	pCommonPhrasesModel->sort(0, Qt::AscendingOrder);
	m_pCommonPhrasesCompleter->setModel(pCommonPhrasesModel);		// Note: Parenting the model (above) to the Completer, the completer will delete the old model when we call setModel()

//	connect(m_pCompleter, SIGNAL(activated(const QString &)), this, SLOT(insertCompletion(const QString &)));
	connect(m_pCompleter, SIGNAL(activated(const QModelIndex &)), this, SLOT(insertCompletion(const QModelIndex &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)), this, SLOT(en_changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)));
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
//	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);

	if (!updateInProgress()) {
		UpdateCompleter();
		emit phraseChanged();
		emit changeCaseSensitive(bCaseSensitive);
	}
}

void CPhraseLineEdit::setAccentSensitive(bool bAccentSensitive)
{
	CParsedPhrase::setAccentSensitive(bAccentSensitive);
	// TODO : ??? Add AccentSensitivity to completer ???

	if (!updateInProgress()) {
		UpdateCompleter();
		emit phraseChanged();
		emit changeAccentSensitive(bAccentSensitive);
	}
}

void CPhraseLineEdit::setExclude(bool bExclude)
{
	CParsedPhrase::setExclude(bExclude);

	if (!updateInProgress()) {
		// No need to trigger UpdateCompleter() here as exclude has no bearing on completion
		emit phraseChanged();				// But, we need to recalculate our search results
		emit changeExclude(bExclude);
	}
}

void CPhraseLineEdit::insertCompletion(const QString& completion)
{
	CParsedPhrase::insertCompletion(textCursor(), completion);
}

void CPhraseLineEdit::insertCompletion(const QModelIndex &index)
{
	CParsedPhrase::insertCompletion(textCursor(), index.data(Qt::DisplayRole).toString());
}

void CPhraseLineEdit::insertCommonPhraseCompletion(const QString &completion)
{
	CPhraseCursor cursor(textCursor());
	cursor.clearSelection();
	cursor.select(QTextCursor::LineUnderCursor);
	bool bOldCaseSensitive = isCaseSensitive();
	bool bOldAccentSensitive = isAccentSensitive();
	bool bOldExclude = isExcluded();
	{
		CDoUpdate doUpdate(this);				// Hold-over to update everything at once
		CPhraseEntry phrase(completion);
		cursor.insertText(phrase.text());
		setCaseSensitive(phrase.caseSensitive());
		setAccentSensitive(phrase.accentSensitive());
		setExclude(phrase.isExcluded());
		// Release update here
	}
	if (!updateInProgress()) {
		UpdateCompleter();
		emit phraseChanged();
		if (bOldCaseSensitive != isCaseSensitive()) emit changeCaseSensitive(isCaseSensitive());
		if (bOldAccentSensitive != isAccentSensitive()) emit changeAccentSensitive(isAccentSensitive());
		if (bOldExclude != isExcluded()) emit changeExclude(isExcluded());
	}
}

void CPhraseLineEdit::en_textChanged()
{
	if (!updateInProgress()) {
		CSingleLineTextEdit::en_textChanged();

		emit phraseChanged();
	}

	m_pActionSelectAll->setEnabled(!document()->isEmpty());
}

void CPhraseLineEdit::processPendingUpdateCompleter()
{
	// For some reason, this checking of a triggered updateCompleter()
	//		isn't sufficient.  We still get periodically get
	//		phrases that have no results.  So for now, I've
	//		changed it to just always do an update.  It's
	//		not totally the fact we have a delayed updateCompleter()
	//		either, because disabling our delayed trigger entirely
	//		still has the issue.  Strange...
//	if (m_dlyUpdateCompleter.isTriggered()) delayed_UpdatedCompleter();
//	delayed_UpdatedCompleter();

	m_dlyUpdateCompleter.untrigger();
	ParsePhrase(textCursor());
	if (m_pCompleter->popup()->isVisible()) m_pCompleter->popup()->setCurrentIndex(QModelIndex());
}

void CPhraseLineEdit::UpdateCompleter()
{
	if (CPersistentSettings::instance()->searchActivationDelay() == -1) {
		// Immediate activation if the search activation delay is disabled,
		//		such as for restoring of the last search:
		delayed_UpdatedCompleter();
	} else {
		m_dlyUpdateCompleter.trigger();
	}
}

void CPhraseLineEdit::delayed_UpdatedCompleter()
{
	m_dlyUpdateCompleter.untrigger();
	CParsedPhrase::UpdateCompleter(textCursor(), *m_pCompleter);

	if (updateInProgress()) return;
	CDoUpdate doUpdate(this);

	CPhraseCursor cursor(textCursor());
	cursor.beginEditBlock();

	QTextCharFormat fmt = cursor.charFormat();
	fmt.setFontStrikeOut(false);
	fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.setCharFormat(fmt);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	for (int nSubPhrase = 0; nSubPhrase < subPhraseCount(); ++nSubPhrase) {
		const CSubPhrase *pSubPhrase = subPhrase(nSubPhrase);

		int nPhraseSize = pSubPhrase->phraseSize();
		for (int nWord = 0; nWord < nPhraseSize; ++nWord) {
			cursor.selectWordUnderCursor();
			if (/* (pSubPhrase->GetCursorWordPos() != nWord) && */
				(pSubPhrase->GetMatchLevel() <= nWord) &&
				(pSubPhrase->GetCursorMatchLevel() <= nWord) &&
				((nWord != pSubPhrase->GetCursorWordPos()) ||
				 ((!pSubPhrase->GetCursorWord().isEmpty()) && (nWord == pSubPhrase->GetCursorWordPos()))
				)
				) {
				fmt.setFontStrikeOut(true);
				fmt.setUnderlineColor(QColor(255,0,0));
				fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
				cursor.setCharFormat(fmt);
			}

			cursor.moveCursorWordRight(QTextCursor::MoveAnchor);
		}
	}

	cursor.endEditBlock();
}

bool CPhraseLineEdit::canInsertFromMimeData(const QMimeData *source) const
{
	if (source->hasFormat(g_constrPhraseTagMimeType)) {
		return true;
	} else {
		return CSingleLineTextEdit::canInsertFromMimeData(source);
	}
}

void CPhraseLineEdit::insertFromMimeData(const QMimeData * source)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (!(textInteractionFlags() & Qt::TextEditable) || !source) return;

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

		ensureCursorVisible();
	} else {
		CSingleLineTextEdit::insertFromMimeData(source);
	}
}

void CPhraseLineEdit::focusInEvent(QFocusEvent *event)
{
	emit activatedPhraseEditor(this);
	CSingleLineTextEdit::focusInEvent(event);
}

void CPhraseLineEdit::setupCompleter(const QString &strText, bool bForce)
{
	ParsePhrase(textCursor());

	bool bCompleterOpen = m_pCompleter->popup()->isVisible();
	if ((bForce) || (!strText.isEmpty()) || (bCompleterOpen)) {
		delayed_UpdatedCompleter();			// Do an immediate update of the completer so we have values below (it speeds up the initial completer calculations!)
		if (m_nLastCursorWord != GetCursorWordPos()) {
			m_pCompleter->popup()->close();
			m_nLastCursorWord = GetCursorWordPos();
			if (bCompleterOpen) bForce = true;				// Reshow completer if it was open already and we're changing words
		}
		m_pCompleter->selectFirstMatchString();
	}

#ifdef SEARCH_COMPLETER_DEBUG_OUTPUT
	qDebug("CursorWord: \"%s\",  AtEnd: %s", GetCursorWord().toUtf8().data(), atEndOfSubPhrase() ? "yes" : "no");
#endif
	if (bForce || (!strText.isEmpty() && ((GetCursorWord().length() > 0) || (atEndOfSubPhrase())))) {
		m_pCompleter->complete();
		if (m_pCompleter->popup()->isVisible()) m_pCompleter->popup()->setCurrentIndex(QModelIndex());
	}
}

void CPhraseLineEdit::resizeEvent(QResizeEvent * /* event */)
{
	m_pButtonDroplist->move(width()-m_pButtonDroplist->width(),0);
}

void CPhraseLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
	m_bDoingPopup = true;
#ifndef USE_ASYNC_DIALOGS
	m_pEditMenu->exec(event->globalPos());
#else
	m_pEditMenu->popup(event->globalPos());
#endif
	m_bDoingPopup = false;
}

void CPhraseLineEdit::en_dropCommonPhrasesClicked()
{
	assert(m_pBibleDatabase.data() != NULL);
	if (m_pBibleDatabase == NULL) return;
	assert(m_pCommonPhrasesCompleter != NULL);
	if (m_pCommonPhrasesCompleter == NULL) return;
	CPhraseListModel *pModel = (CPhraseListModel *)(m_pCommonPhrasesCompleter->model());
	assert(pModel != NULL);
	if (pModel == NULL) return;

	CPhraseList phrases = m_pBibleDatabase->phraseList();
	phrases.append(userPhrases());
	phrases.removeDuplicates();
	pModel->setPhraseList(phrases);
	pModel->sort(0, Qt::AscendingOrder);
	m_pCommonPhrasesCompleter->complete();
}

void CPhraseLineEdit::en_changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode)
{
	m_pCompleter->setCompletionFilterMode(nMode);
}

// ============================================================================

CKJVSearchPhraseEdit::CKJVSearchPhraseEdit(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase, QWidget *parent) :
	QWidget(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_bHaveUserDatabase(bHaveUserDatabase),
	m_bLastPhraseChangeHadResults(false),
	m_bUpdateInProgress(false)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui.setupUi(this);

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_PHRASE_SPY
	CMyApplication::createSpy(this);
#endif
#endif

	// --------------------------------------------------------------

	//	Swapout the editPhrase from the layout with one that we
	//		can set the database on:

	int ndx = ui.gridLayout->indexOf(ui.editPhrase);
	assert(ndx != -1);
	if (ndx == -1) return;
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;
	ui.gridLayout->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	QString strEditPhraseToolTip = ui.editPhrase->toolTip();
	QString strEditPhraseStatusTip = ui.editPhrase->statusTip();
	CPhraseLineEdit *pEditPhrase = new CPhraseLineEdit(pBibleDatabase, this);
	pEditPhrase->setObjectName(QString::fromUtf8("editPhrase"));
	delete ui.editPhrase;
	ui.editPhrase = pEditPhrase;
	ui.editPhrase->setToolTip(strEditPhraseToolTip);
	ui.editPhrase->setStatusTip(strEditPhraseStatusTip);
	ui.gridLayout->addWidget(pEditPhrase, nRow, nCol, nRowSpan, nColSpan);
	setTabOrder(ui.editPhrase, ui.chkCaseSensitive);
	setTabOrder(ui.chkCaseSensitive, ui.chkAccentSensitive);
	setTabOrder(ui.chkAccentSensitive, ui.chkExclude);
	setTabOrder(ui.chkExclude, ui.chkDisable);
	setTabOrder(ui.chkDisable, ui.editPhrase->getDropListButton());
	setTabOrder(ui.editPhrase->getDropListButton(), ui.buttonRemove);

	// --------------------------------------------------------------

	ui.chkCaseSensitive->setChecked(ui.editPhrase->isCaseSensitive());
	ui.chkAccentSensitive->setChecked(ui.editPhrase->isAccentSensitive());
	ui.chkExclude->setChecked(ui.editPhrase->isExcluded());
	ui.chkDisable->setChecked(parsedPhrase()->isDisabled());
	ui.buttonAddPhrase->setEnabled(false);
	ui.buttonDelPhrase->setEnabled(false);
	ui.buttonClear->setEnabled(true);

	setSearchActivationDelay(CPersistentSettings::instance()->searchActivationDelay());
	connect(ui.editPhrase, SIGNAL(phraseChanged()), &m_dlyTextChanged, SLOT(trigger()));
	connect(&m_dlyTextChanged, SIGNAL(triggered()), this, SLOT(en_phraseChanged()));
	connect(CPersistentSettings::instance(), SIGNAL(changedSearchPhraseActivationDelay(int)), this, SLOT(setSearchActivationDelay(int)));

	connect(CPersistentSettings::instance(), SIGNAL(changedUserPhrases()), this, SLOT(setPhraseButtonEnables()));

	connect(ui.chkCaseSensitive, SIGNAL(clicked(bool)), this, SLOT(en_CaseSensitiveChanged(bool)));
	connect(ui.editPhrase, SIGNAL(changeCaseSensitive(bool)), this, SLOT(en_CaseSensitiveChanged(bool)));
	connect(ui.chkAccentSensitive, SIGNAL(clicked(bool)), this, SLOT(en_AccentSensitiveChanged(bool)));
	connect(ui.editPhrase, SIGNAL(changeAccentSensitive(bool)), this, SLOT(en_AccentSensitiveChanged(bool)));
	connect(ui.chkExclude, SIGNAL(clicked(bool)), this, SLOT(en_ExcludeChanged(bool)));
	connect(ui.editPhrase, SIGNAL(changeExclude(bool)), this, SLOT(en_ExcludeChanged(bool)));
	connect(ui.chkDisable, SIGNAL(clicked(bool)), this, SLOT(setDisabled(bool)));
	connect(ui.buttonAddPhrase, SIGNAL(clicked()), this, SLOT(en_phraseAdd()));
	connect(ui.buttonDelPhrase, SIGNAL(clicked()), this, SLOT(en_phraseDel()));
	connect(ui.buttonClear, SIGNAL(clicked()), this, SLOT(en_phraseClear()));
	connect(ui.editPhrase, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)), this, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)));
	connect(ui.buttonRemove, SIGNAL(clicked()), this, SLOT(closeSearchPhrase()));
}

CKJVSearchPhraseEdit::~CKJVSearchPhraseEdit()
{

}

void CKJVSearchPhraseEdit::closeSearchPhrase()
{
	emit closingSearchPhrase(this);
	delete this;
}

void CKJVSearchPhraseEdit::clearSearchPhrase()
{
	en_phraseClear();
}

void CKJVSearchPhraseEdit::showSeperatorLine(bool bShow)
{
	ui.lineSeparator->setVisible(bShow);
	adjustSize();
}

void CKJVSearchPhraseEdit::enableCloseButton(bool bEnable)
{
	ui.buttonRemove->setEnabled(bEnable);
}

void CKJVSearchPhraseEdit::focusEditor() const
{
	ui.editPhrase->setFocus();
}

const CParsedPhrase *CKJVSearchPhraseEdit::parsedPhrase() const
{
	return ui.editPhrase;
}

CPhraseLineEdit *CKJVSearchPhraseEdit::phraseEditor() const
{
	return ui.editPhrase;
}

void CKJVSearchPhraseEdit::en_phraseChanged()
{
	assert(m_pBibleDatabase.data() != NULL);

	const CParsedPhrase *pPhrase = parsedPhrase();
	assert(pPhrase != NULL);

	// Make sure any pending updates are complete:
	phraseEditor()->processPendingUpdateCompleter();

	m_phraseEntry.setFromPhrase(pPhrase);
	setPhraseButtonEnables();

	// If last time, this phrase didn't have anything meaningful, if it still doesn't
	//		then there's no need to send a notification as the overall search results
	//		still won't be affected:
	if (!m_bLastPhraseChangeHadResults) {
		if ((!pPhrase->isCompleteMatch()) || (pPhrase->GetNumberOfMatches() == 0) || (pPhrase->isDisabled())) {
			pPhrase->setIsDuplicate(false);
			pPhrase->ClearWithinPhraseTagSearchResults();
			pPhrase->ClearScopedPhraseTagSearchResults();
			phraseStatisticsChanged();
		} else {
			emit phraseChanged(this);
			m_bLastPhraseChangeHadResults = true;
		}
	} else {
		emit phraseChanged(this);
		m_bLastPhraseChangeHadResults = ((pPhrase->isCompleteMatch()) && (pPhrase->GetNumberOfMatches() != 0) && (!pPhrase->isDisabled()));
	}

	// Note: No need to call phraseStatisticsChanged() here as the parent
	//		CKJVCanOpener will be calling it for everyone (as a result of
	//		the above emit statements, since not only does this editor
	//		need to be updated, but all editors
}

void CKJVSearchPhraseEdit::phraseStatisticsChanged() const
{
	QString strTemp = tr("Number of Occurrences: ");
	if (parsedPhrase()->isDuplicate()) {
		strTemp += tr("(Duplicate)");
	} else {
		strTemp += QString("%1/%2/%3").arg(!parsedPhrase()->isDisabled() ? (parsedPhrase()->isExcluded() ? -parsedPhrase()->GetContributingNumberOfMatches() : parsedPhrase()->GetContributingNumberOfMatches()) : 0).arg(parsedPhrase()->GetNumberOfMatchesWithin()).arg(parsedPhrase()->GetNumberOfMatches());
	}
	ui.lblOccurrenceCount->setText(strTemp);
}

void CKJVSearchPhraseEdit::en_CaseSensitiveChanged(bool bCaseSensitive)
{
	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;
	ui.chkCaseSensitive->setChecked(bCaseSensitive);		// Set the checkbox in case the phrase editor is setting us
	ui.editPhrase->setCaseSensitive(bCaseSensitive);		// Set the phrase editor in case the checkbox is setting us
	m_bUpdateInProgress = false;
}

void CKJVSearchPhraseEdit::en_AccentSensitiveChanged(bool bAccentSensitive)
{
	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;
	ui.chkAccentSensitive->setChecked(bAccentSensitive);	// Set the checkbox in case the phrase editor is setting us
	ui.editPhrase->setAccentSensitive(bAccentSensitive);	// Set the phrase editor in case the checkbox is setting us
	m_bUpdateInProgress = false;
}

void CKJVSearchPhraseEdit::en_ExcludeChanged(bool bExclude)
{
	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;
	ui.chkExclude->setChecked(bExclude);					// Set the checkbox in case the phrase editor is setting us
	ui.editPhrase->setExclude(bExclude);					// Set the phrase editor in case the checkbox is setting us
	m_bUpdateInProgress = false;
}

void CKJVSearchPhraseEdit::setDisabled(bool bDisabled)
{
	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;
	ui.chkDisable->setChecked(bDisabled);					// Set the checkbox in case the phrase editor is setting us
	parsedPhrase()->setIsDisabled(bDisabled);				// Set the phrase editor in case the checkbox is setting us
	ui.editPhrase->setEnabled(!bDisabled);					// Disable the editor things so user realized this phrase is disabled
	ui.chkCaseSensitive->setEnabled(!bDisabled);
	ui.chkAccentSensitive->setEnabled(!bDisabled);
	ui.chkExclude->setEnabled(!bDisabled);
	ui.editPhrase->getDropListButton()->setEnabled(!bDisabled);
	setPhraseButtonEnables();
	m_bUpdateInProgress = false;
	en_phraseChanged();										// Unlike Case-Sensitive and Accent-Sensitive, CPhraseLineEdit doesn't have a signals handler for this to trigger a phraseChaged.  So we must do it here.
}

void CKJVSearchPhraseEdit::en_phraseAdd()
{
	CPersistentSettings::instance()->addUserPhrase(m_phraseEntry);
//	setPhraseButtonEnables();
}

void CKJVSearchPhraseEdit::en_phraseDel()
{
	CPersistentSettings::instance()->removeUserPhrase(m_phraseEntry);
//	setPhraseButtonEnables();
}

void CKJVSearchPhraseEdit::en_phraseClear()
{
	ui.editPhrase->clear();
	m_phraseEntry.clear();
	// No need to call setPhraseButtonEnables because the textChanged event caused by the call above will do it for us
	en_CaseSensitiveChanged(false);
	en_AccentSensitiveChanged(false);
	en_ExcludeChanged(false);
	setDisabled(false);
}

void CKJVSearchPhraseEdit::setPhraseButtonEnables()
{
	bool bCommonFound = m_pBibleDatabase->phraseList().contains(m_phraseEntry);
	bool bUserFound = CPersistentSettings::instance()->userPhrases().contains(m_phraseEntry);
	bool bHaveText = (!m_phraseEntry.text().isEmpty());
	ui.buttonAddPhrase->setEnabled(!parsedPhrase()->isDisabled() && m_bHaveUserDatabase && bHaveText && !bUserFound && !bCommonFound);
	ui.buttonDelPhrase->setEnabled(!parsedPhrase()->isDisabled() && m_bHaveUserDatabase && bHaveText && bUserFound);
//	ui.buttonClear->setEnabled(!parsedPhrase()->isDisabled() && !ui.editPhrase->toPlainText().isEmpty());
}
