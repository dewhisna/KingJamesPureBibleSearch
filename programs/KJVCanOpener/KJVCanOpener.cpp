#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include "dbstruct.h"
#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "KJVPassageNavigatorDlg.h"
#include "BuildDB.h"
#include "KJVAboutDlg.h"

#include <assert.h>

#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QListView>
#include <QStringList>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>

// ============================================================================

QSize CSearchPhraseScrollArea::minimumSizeHint() const
{
	return QScrollArea::minimumSizeHint();
}

QSize CSearchPhraseScrollArea::sizeHint() const
{
	return QScrollArea::sizeHint();
}

// ============================================================================

CSearchResultsListView::CSearchResultsListView(QWidget *parent)
	:	QListView(parent)
{

}

CSearchResultsListView::~CSearchResultsListView()
{

}

void CSearchResultsListView::focusInEvent(QFocusEvent *event)
{
	emit activatedSearchResults();
	QListView::focusInEvent(event);
}

void CSearchResultsListView::contextMenuEvent(QContextMenuEvent *event)
{
	QListView::contextMenuEvent(event);
}


// ============================================================================

CKJVCanOpener::CKJVCanOpener(const QString &strUserDatabase, QWidget *parent) :
	QMainWindow(parent),
	m_strUserDatabase(strUserDatabase),
	m_bDoingUpdate(false),
	m_pActionPassageBrowserEditMenu(NULL),
	m_pViewMenu(NULL),
	m_pActionShowVerseHeading(NULL),
	m_pActionShowVerseRichText(NULL),
	m_pActionBookBackward(NULL),
	m_pActionBookForward(NULL),
	m_pActionChapterBackward(NULL),
	m_pActionChapterForward(NULL),
	m_pActionNavBackward(NULL),
	m_pActionNavForward(NULL),
	m_pActionNavHome(NULL),
	m_pActionNavClear(NULL),
	m_pActionJump(NULL),
	m_pActionAbout(NULL),
	ui(new Ui::CKJVCanOpener)
{
	ui->setupUi(this);

	// TODO : Set preference for start mode!:
	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode = CVerseListModel::VDME_RICHTEXT;

	QAction *pAction;

	QMenu *pFileMenu = ui->menuBar->addMenu("&File");
	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), "E&xit", this, SLOT(close()), QKeySequence(Qt::CTRL + Qt::Key_Q));
	pAction->setStatusTip("Exit the King James Can Opener Application");
	pFileMenu->addAction(pAction);

//	QMenu *pEditMenu = ui->menuBar->addMenu("&Edit");
//	QMenu *pBrowserEditMenu = ui->widgetKJVBrowser->browser()->getEditMenu();
//	ui->menuBar->addMenu(pBrowserEditMenu);
//	connect(ui->widgetKJVBrowser->browser(), SIGNAL(addEditMenu(bool)), this, SLOT(on_addPassageBrowserEditMenu(bool)));
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(activatedBrowser()), this, SLOT(on_activatedBrowser()));
	connect(ui->listViewSearchResults, SIGNAL(activatedSearchResults()), this, SLOT(on_activatedSearchResults()));

	m_pViewMenu = ui->menuBar->addMenu("&View");

	QMenu *pViewToolbarsMenu = m_pViewMenu->addMenu("&Toolbars");
	pViewToolbarsMenu->addAction(ui->mainToolBar->toggleViewAction());
	ui->mainToolBar->toggleViewAction()->setStatusTip("Show/Hide Main Tool Bar");

	m_pViewMenu->addSeparator();

	m_pActionShowVerseHeading = m_pViewMenu->addAction(QIcon(), "&References Only", this, SLOT(on_viewVerseHeading()));
	m_pActionShowVerseHeading->setStatusTip("Show Search Results Verse References Only");
	m_pActionShowVerseHeading->setCheckable(true);
	m_pActionShowVerseHeading->setChecked(nDisplayMode == CVerseListModel::VDME_HEADING);

	m_pActionShowVerseRichText = m_pViewMenu->addAction(QIcon(), "Verse &Preview", this, SLOT(on_viewVerseRichText()));
	m_pActionShowVerseRichText->setStatusTip("Show Search Results as Rich Text Verse Preview");
	m_pActionShowVerseRichText->setCheckable(true);
	m_pActionShowVerseRichText->setChecked(nDisplayMode == CVerseListModel::VDME_RICHTEXT);

	QMenu *pNavMenu = ui->menuBar->addMenu("&Navigate");

	pAction = pNavMenu->addAction("Beginning of Bible", ui->widgetKJVBrowser, SLOT(on_Bible_Beginning()), QKeySequence(Qt::ALT + Qt::Key_Home));
	pAction->setStatusTip("Goto the very Beginning of the Bible");
	connect(pAction, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	pAction = pNavMenu->addAction("Ending of Bible", ui->widgetKJVBrowser, SLOT(on_Bible_Ending()), QKeySequence(Qt::ALT + Qt::Key_End));
	pAction->setStatusTip("Goto the very End of the Bible");
	connect(pAction, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionBookBackward = pNavMenu->addAction("Book Backward", ui->widgetKJVBrowser, SLOT(on_Book_Backward()), QKeySequence(Qt::CTRL + Qt::Key_PageUp));
	m_pActionBookBackward->setStatusTip("Move Backward one Book");
	connect(m_pActionBookBackward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionBookForward = pNavMenu->addAction("Book Forward", ui->widgetKJVBrowser, SLOT(on_Book_Forward()), QKeySequence(Qt::CTRL + Qt::Key_PageDown));
	m_pActionBookForward->setStatusTip("Move Forward one Book");
	connect(m_pActionBookForward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionChapterBackward = pNavMenu->addAction("Chapter Backward", ui->widgetKJVBrowser, SLOT(on_ChapterBackward()), QKeySequence(Qt::ALT + Qt::Key_PageUp));
	m_pActionChapterBackward->setStatusTip("Move Backward one Chapter");
	connect(m_pActionChapterBackward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionChapterForward = pNavMenu->addAction("Chapter Forward", ui->widgetKJVBrowser, SLOT(on_ChapterForward()), QKeySequence(Qt::ALT + Qt::Key_PageDown));
	m_pActionChapterForward->setStatusTip("Move Forward one Chapter");
	connect(m_pActionChapterForward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	connect(ui->widgetKJVBrowser, SIGNAL(IndexChanged(const CRelIndex &)), this, SLOT(on_indexChanged(const CRelIndex &)));

	pNavMenu->addSeparator();

	m_pActionNavBackward = new QAction(QIcon(":/res/Nav3_Arrow_Left.png"), "History &Backward", this);
	m_pActionNavBackward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	m_pActionNavBackward->setStatusTip("Go Backward in Navigation History");
	ui->mainToolBar->addAction(m_pActionNavBackward);
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(backwardAvailable(bool)), m_pActionNavBackward, SLOT(setEnabled(bool)));
	connect(m_pActionNavBackward, SIGNAL(triggered()), ui->widgetKJVBrowser->browser(), SLOT(backward()));
	connect(m_pActionNavBackward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionNavBackward->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable());
	pNavMenu->addAction(m_pActionNavBackward);

	m_pActionNavForward = new QAction(QIcon(":/res/Nav3_Arrow_Right.png"), "History &Forward", this);
	m_pActionNavForward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));
	m_pActionNavForward->setStatusTip("Go Forward in Navigation History");
	ui->mainToolBar->addAction(m_pActionNavForward);
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(forwardAvailable(bool)), m_pActionNavForward, SLOT(setEnabled(bool)));
	connect(m_pActionNavForward, SIGNAL(triggered()), ui->widgetKJVBrowser->browser(), SLOT(forward()));
	connect(m_pActionNavForward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionNavForward->setEnabled(ui->widgetKJVBrowser->browser()->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavForward);

	m_pActionNavHome = pNavMenu->addAction(QIcon(":/res/go_home.png"), "History &Home", ui->widgetKJVBrowser->browser(), SLOT(home()), QKeySequence(Qt::ALT + Qt::Key_Up));
	m_pActionNavHome->setStatusTip("Jump to History Home Passage");
	m_pActionNavHome->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
									ui->widgetKJVBrowser->browser()->isForwardAvailable());

	m_pActionNavClear = new QAction(QIcon(":/res/edit_clear.png"), "&Clear Navigation History", this);
	m_pActionNavClear->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));
	m_pActionNavClear->setStatusTip("Clear All Passage Navigation History");
	ui->mainToolBar->addAction(m_pActionNavClear);
	connect(m_pActionNavClear, SIGNAL(triggered()), this, SLOT(on_clearBrowserHistory()));
	m_pActionNavClear->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
									ui->widgetKJVBrowser->browser()->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavClear);

	connect(ui->widgetKJVBrowser->browser(), SIGNAL(historyChanged()), this, SLOT(on_browserHistoryChanged()));

//	m_pActionJump = new QAction(QIcon(":/res/go_jump2.png"), "Passage Navigator", this);
	m_pActionJump = new QAction(QIcon(":/res/green_arrow.png"), "Passage Navigator", this);
	m_pActionJump->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	m_pActionJump->setStatusTip("Display the Passage Navigator Widget");
	ui->mainToolBar->addAction(m_pActionJump);
	connect(m_pActionJump, SIGNAL(triggered()), this, SLOT(on_PassageNavigatorTriggered()));

	pNavMenu->addSeparator();
	pNavMenu->addAction(m_pActionJump);

	QMenu *pHelpMenu = ui->menuBar->addMenu("&Help");
	pAction = pHelpMenu->addAction(QIcon(":/res/help_book.png"), "&Help", this, SLOT(on_HelpManual()), QKeySequence(Qt::SHIFT + Qt::Key_F1));
	pAction->setStatusTip("Display the Users Manual");

	m_pActionAbout = new QAction(QIcon(":/res/help_icon1.png"), "About...", this);
	m_pActionAbout->setShortcut(QKeySequence(Qt::Key_F1));
	m_pActionAbout->setStatusTip("About the King James Can Opener");
	m_pActionAbout->setToolTip("About the King James Can Opener...");
	connect(m_pActionAbout, SIGNAL(triggered()), this, SLOT(on_HelpAbout()));
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addAction(m_pActionAbout);
	pHelpMenu->addAction(m_pActionAbout);


	ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	CKJVSearchPhraseEdit *pPhraseEdit = new CKJVSearchPhraseEdit();
	connect(pPhraseEdit, SIGNAL(activatedPhraseEdit()), this, SLOT(on_activatedPhraseEditor()));

	QVBoxLayout *pLayoutPhrases = new QVBoxLayout(ui->scrollAreaWidgetContents);
	pLayoutPhrases->setSpacing(0);
	pLayoutPhrases->setContentsMargins(0, 0, 0, 0);
	pLayoutPhrases->addWidget(pPhraseEdit);

	ui->scrollAreaWidgetContents->setMinimumSize(/* pLayoutPhrases->sizeHint() */ pPhraseEdit->sizeHint() );

	ui->scrollAreaSearchPhrases->setMinimumSize(pLayoutPhrases->sizeHint().width() +
							ui->scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui->scrollAreaSearchPhrases->frameWidth() * 2,
							pLayoutPhrases->sizeHint().height() /* pPhraseEdit->sizeHint() */);

/*
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());

ui->scrollAreaWidgetContents->setMinimumSize(pPhraseEdit->sizeHint().width(), pPhraseEdit->sizeHint().height()*7);
*/

ui->scrollAreaWidgetContents->setMinimumSize(pPhraseEdit->sizeHint().width(), pPhraseEdit->sizeHint().height()*1);


//ui->widgetPhraseEdit->pStatusBar = ui->statusBar;
pPhraseEdit->pStatusBar = ui->statusBar;


	CVerseListModel *model = new CVerseListModel(ui->listViewSearchResults);
	model->setDisplayMode(nDisplayMode);
	ui->listViewSearchResults->setModel(model);

	CVerseListDelegate *delegate = new CVerseListDelegate(*model, ui->listViewSearchResults);
	ui->listViewSearchResults->setItemDelegate(delegate);

//connect(ui->widgetPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
	connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));

	connect(ui->listViewSearchResults, SIGNAL(activated(const QModelIndex &)), this, SLOT(on_SearchResultActivated(const QModelIndex &)));
}

CKJVCanOpener::~CKJVCanOpener()
{
	delete ui;
}

void CKJVCanOpener::Initialize(CRelIndex nInitialIndex)
{
	ui->widgetKJVBrowser->gotoIndex(nInitialIndex);
}

void CKJVCanOpener::closeEvent(QCloseEvent *event)
{
	if ((g_bUserPhrasesDirty) && haveUserDatabase()) {
		int nResult = QMessageBox::warning(this, windowTitle(), "Do you wish to save the search phrase list changes you've made to the user database?",
											QString("Yes"), QString("No"), QString("Cancel"), 0, 2);
		if (nResult == 2) {
			event->ignore();
			return;
		}
		if (nResult == 0) {
			CBuildDatabase bdb(this);
			if (!bdb.BuildUserDatabase(m_strUserDatabase)) {
				QMessageBox::warning(this, windowTitle(), "Failed to save KJV User Database!\nCheck installation and settings!");
				event->ignore();
				return;
			}
		}
	}

	return QMainWindow::closeEvent(event);
}

void CKJVCanOpener::on_addPassageBrowserEditMenu(bool bAdd)
{
	if (bAdd) {
		if (m_pActionPassageBrowserEditMenu == NULL) {
			m_pActionPassageBrowserEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), ui->widgetKJVBrowser->browser()->getEditMenu());
			connect(m_pActionPassageBrowserEditMenu, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
		}
	} else {
		if (m_pActionPassageBrowserEditMenu) {
			ui->menuBar->removeAction(m_pActionPassageBrowserEditMenu);
			m_pActionPassageBrowserEditMenu = NULL;
		}
	}
}

void CKJVCanOpener::on_activatedBrowser()
{
	on_addPassageBrowserEditMenu(true);
}

void CKJVCanOpener::on_activatedSearchResults()
{
	on_addPassageBrowserEditMenu(false);
}

void CKJVCanOpener::on_activatedPhraseEditor()
{
	on_addPassageBrowserEditMenu(false);
}

void CKJVCanOpener::on_viewVerseHeading()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nMode = CVerseListModel::VDME_HEADING;

	if (m_pActionShowVerseHeading->isChecked()) {
		m_pActionShowVerseRichText->setChecked(false);
		nMode = CVerseListModel::VDME_HEADING;
	} else {
		m_pActionShowVerseRichText->setChecked(true);
		nMode = CVerseListModel::VDME_RICHTEXT;
	}

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		pModel->setDisplayMode(nMode);
	}

	m_bDoingUpdate = false;
}

void CKJVCanOpener::on_viewVerseRichText()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nMode = CVerseListModel::VDME_HEADING;

	if (m_pActionShowVerseRichText->isChecked()) {
		m_pActionShowVerseHeading->setChecked(false);
		nMode = CVerseListModel::VDME_RICHTEXT;
	} else {
		m_pActionShowVerseHeading->setChecked(true);
		nMode = CVerseListModel::VDME_HEADING;
	}

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		pModel->setDisplayMode(nMode);
	}

	m_bDoingUpdate = false;
}

void CKJVCanOpener::on_indexChanged(const CRelIndex &index)
{
	assert(m_pActionBookBackward != NULL);
	assert(m_pActionBookForward != NULL);
	assert(m_pActionChapterBackward != NULL);
	assert(m_pActionChapterForward != NULL);
	if ((m_pActionBookBackward == NULL) ||
		(m_pActionBookForward == NULL) ||
		(m_pActionChapterBackward == NULL) ||
		(m_pActionChapterForward == NULL)) return;

	m_pActionBookBackward->setEnabled(index.book() >= 2);
	m_pActionBookForward->setEnabled(index.book() < g_lstTOC.size());
	m_pActionChapterBackward->setEnabled((index.book() >= 2) ||
										((index.book() == 1) && (index.chapter() >= 2)));
	m_pActionChapterForward->setEnabled((index.book() < g_lstTOC.size()) ||
										((index.book() == g_lstTOC.size()) && (index.chapter() < g_lstTOC.at(index.book()-1).m_nNumChp)));
}

void CKJVCanOpener::on_browserHistoryChanged()
{
	if (m_pActionNavBackward) {
		m_pActionNavBackward->setToolTip(ui->widgetKJVBrowser->browser()->historyTitle(-1));
		if (ui->widgetKJVBrowser->browser()->isBackwardAvailable()) {
			m_pActionNavBackward->setStatusTip("Go to: " + ui->widgetKJVBrowser->browser()->historyTitle(-1));
		} else {
			m_pActionNavBackward->setStatusTip("Go Backward in Navigation History");
		}
	}
	if (m_pActionNavForward) {
		m_pActionNavForward->setToolTip(ui->widgetKJVBrowser->browser()->historyTitle(+1));
		if (ui->widgetKJVBrowser->browser()->isForwardAvailable()) {
			m_pActionNavForward->setStatusTip("Go to: " + ui->widgetKJVBrowser->browser()->historyTitle(+1));
		} else {
			m_pActionNavForward->setStatusTip("Go Forward in Navigation History");
		}
	}
	if (m_pActionNavClear) m_pActionNavClear->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
														ui->widgetKJVBrowser->browser()->isForwardAvailable());
	if (m_pActionNavHome) m_pActionNavHome->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
														ui->widgetKJVBrowser->browser()->isForwardAvailable());
}

void CKJVCanOpener::on_clearBrowserHistory()
{
	ui->widgetKJVBrowser->browser()->clearHistory();
}

void CKJVCanOpener::on_phraseChanged(const CParsedPhrase &phrase)
{
	CVerseList lstReferences;
	TPhraseTagList lstTags;

	if (phrase.GetNumberOfMatches() <= 5000) {		// This check keeps the really heavy hitters like 'and' and 'the' from making us come to a complete stand-still
		TIndexList lstResults = phrase.GetNormalizedSearchResults();

		for (unsigned int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
			int nCount = 1;
			uint32_t ndxDenormal = DenormalizeIndex(lstResults.at(ndxResults));
			CRelIndex ndxRelative(ndxDenormal);
			CRelIndex ndxRelativeZW = CRelIndex(ndxRelative.book(), ndxRelative.chapter(), ndxRelative.verse(), 0);

			if ((lstResults[ndxResults] == 0) || (ndxDenormal == 0)) {
//				lstReferences.push_back(QString("Invalid Index: @ %1: Norm: %2  Denorm: %3").arg(ndxResults).arg(lstResults[ndxResults]).arg(ndxDenormal));

				lstReferences.push_back(CVerseListItem(
						0,
						QString("Invalid Index: @ %1: Norm: %2  Denorm: %3").arg(ndxResults).arg(lstResults.at(ndxResults)).arg(ndxDenormal),
						QString()));		// TODO : TOOLTIP
				continue;
			} else {
				lstReferences.push_back(CVerseListItem(
						ndxRelativeZW,
						QString(),
						QString()));		// TODO : TOOLTIP
			}

			QString strHeading = ndxRelative.PassageReferenceText();
			CVerseListItem &verseItem(lstReferences.last());
			unsigned int nPhraseSize = phrase.phraseSize();
			verseItem.phraseTags().push_back(TPhraseTag(ndxRelative, nPhraseSize));

			if (ndxResults<(lstResults.size()-1)) {
				bool bNextIsSameReference=false;
				do {
					CRelIndex ndxNextRelative(DenormalizeIndex(lstResults.at(ndxResults+1)));
					if ((ndxRelative.book() == ndxNextRelative.book()) &&
						(ndxRelative.chapter() == ndxNextRelative.chapter()) &&
						(ndxRelative.verse() == ndxNextRelative.verse())) {
						strHeading += QString("[%1]").arg(ndxNextRelative.word());
						verseItem.phraseTags().push_back(TPhraseTag(ndxNextRelative, nPhraseSize));
						bNextIsSameReference=true;
						nCount++;
						ndxResults++;
					} else {
						bNextIsSameReference=false;
					}
				} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
			}
			if (nCount > 1) strHeading = QString("(%1) ").arg(nCount) + strHeading;
			verseItem.setHeading(strHeading);

			lstTags.append(verseItem.phraseTags());
		}
//		lstReferences.removeDuplicates();
	}

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		if (lstReferences.size() <= 2000) {
			pModel->setVerseList(lstReferences);
		} else {
			pModel->setVerseList(CVerseList());
		}
	}

	if ((lstReferences.size() != 0) ||
		((lstReferences.size() == 0) && (phrase.GetNumberOfMatches() == 0))) {
		ui->lblSearchResultsCount->setText(QString("Found %1 occurrences in %2 verses").arg(phrase.GetNumberOfMatches()).arg(lstReferences.size()));
	} else {
		ui->lblSearchResultsCount->setText(QString("Found %1 occurrences (too many verses!)").arg(phrase.GetNumberOfMatches()));
	}

	ui->widgetKJVBrowser->setHighlightTags(lstTags);
}

void CKJVCanOpener::on_SearchResultActivated(const QModelIndex &index)
{
	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	CVerseListItem verse = pModel->data(index, CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>();

//	unsigned int nWordCount = 1;
//	if (verse.phraseTags().size() == 1) {
//		nWordCount = verse.phraseTags().at(0).second;
//		ui->widgetKJVBrowser->gotoIndex(verse.phraseTags().at(0).first, nWordCount);
//	} else {
//		if (verse.phraseTags().size() > 1) {
//			ui->widgetKJVBrowser->gotoIndex(verse.phraseTags().at(0).first, 0);
//		} else {
//			ui->widgetKJVBrowser->gotoIndex(verse.getIndex());
//		}
//	}

	if (verse.phraseTags().size() != 0) {
		ui->widgetKJVBrowser->gotoIndex(verse.phraseTags().at(0).first, 0);
	} else {
		ui->widgetKJVBrowser->gotoIndex(verse.getIndex());
	}

	ui->widgetKJVBrowser->focusBrowser();
}

void CKJVCanOpener::on_PassageNavigatorTriggered()
{
	CKJVPassageNavigatorDlg dlg(this);

	if (dlg.exec() == QDialog::Accepted) {
		ui->widgetKJVBrowser->gotoIndex(dlg.passage());
		ui->widgetKJVBrowser->focusBrowser();
	}
}

void CKJVCanOpener::on_HelpManual()
{

}

void CKJVCanOpener::on_HelpAbout()
{
	CKJVAboutDlg dlg(this);

	dlg.exec();
}

