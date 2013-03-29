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

#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "KJVPassageNavigatorDlg.h"
#include "BuildDB.h"
#include "KJVAboutDlg.h"
#include "PhraseEdit.h"
#include "PhraseListModel.h"
#include "Highlighter.h"
#include "version.h"
#include "PersistentSettings.h"

#include <assert.h>

#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QStringList>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QItemSelection>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>
#include <QToolTip>
#include <ToolTipEdit.h>
#include <QFontDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>

// ============================================================================

#define KJS_FILE_VERSION 1				// Current KJS File Version
#define KJVAPP_REGISTRY_VERSION 1		// Version of Registry Settings

#define NUM_QUICK_ACTIONS 10

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

#ifndef Q_WS_MAC
	const char *g_constrHelpDocFilename = "../../KJVCanOpener/doc/KingJamesPureBibleSearch.pdf";
#else
	const char *g_constrHelpDocFilename = "../SharedSupport/doc/KingJamesPureBibleSearch.pdf";
#endif

	// Key constants:
	// --------------
	// MainApp:
	const QString constrMainAppRestoreStateGroup("RestoreState/MainApp");
	const QString constrSplitterRestoreStateGroup("RestoreState/Splitter");
	const QString constrGeometryKey("Geometry");
	const QString constrWindowStateKey("WindowState");

	// Search Phrases:
	const QString constrLastSearchGroup("LastSearch");

	// Search Results View:
	const QString constrSearchResultsViewGroup("SearchResultsView");
	const QString constrVerseDisplayModeKey("VerseDisplayMode");
	const QString constrVerseTreeModeKey("VerseTreeMode");
	const QString constrViewMissingNodesKey("TreeShowsMissingNodes");
	const QString constrCurrentIndexKey("CurrentIndex");
	const QString constrHasFocusKey("HasFocus");
	const QString constrFontKey("Font");

	// Browser View:
	const QString constrBrowserViewGroup("Browser");
	const QString constrLastReferenceKey("LastReference");
	const QString constrLastSelectionSizeKey("SelectionSize");
	//const QString constrHasFocusKey("HasFocus");
	//const QString constrFontKey("Font");
}

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

CKJVCanOpener::CKJVCanOpener(CBibleDatabasePtr pBibleDatabase, const QString &strUserDatabase, QWidget *parent) :
	QMainWindow(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_strUserDatabase(strUserDatabase),
	m_bDoingUpdate(false),
	m_pActionPassageBrowserEditMenu(NULL),
	m_pActionSearchResultsEditMenu(NULL),
	m_pActionSearchPhraseEditMenu(NULL),
	m_pViewMenu(NULL),
	m_pActionShowVerseHeading(NULL),
	m_pActionShowVerseRichText(NULL),
	m_pActionShowAsList(NULL),
	m_pActionShowAsTreeBooks(NULL),
	m_pActionShowAsTreeChapters(NULL),
	m_pActionShowMissingLeafs(NULL),
	m_pActionExpandAll(NULL),
	m_pActionCollapseAll(NULL),
	m_pActionViewDetails(NULL),
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
	m_bBrowserActive(false),
	m_bSearchResultsActive(false),
	m_bPhraseEditorActive(false),
	m_pLayoutPhrases(NULL),
	m_nLastSearchOccurrences(0),
	m_nLastSearchVerses(0),
	m_nLastSearchChapters(0),
	m_nLastSearchBooks(0),
	m_bLastCalcSuccess(true),
	ui(new Ui::CKJVCanOpener),
	m_pSplitter(NULL),
	m_pSearchResultWidget(NULL),
	m_pBrowserWidget(NULL)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui->setupUi(this);

	m_pSplitter = new QSplitter(ui->centralWidget);
	m_pSplitter->setObjectName(QString::fromUtf8("splitter"));
	m_pSplitter->setOrientation(Qt::Horizontal);
	m_pSplitter->setChildrenCollapsible(false);

	m_pSearchResultWidget = new CKJVSearchResult(m_pBibleDatabase, m_pSplitter);
	m_pSearchResultWidget->setObjectName(QString::fromUtf8("SearchResultsWidget"));
	m_pSplitter->addWidget(m_pSearchResultWidget);

	m_pBrowserWidget = new CKJVBrowser(m_pBibleDatabase, m_pSplitter);
	m_pBrowserWidget->setObjectName(QString::fromUtf8("BrowserWidget"));
	QSizePolicy aSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	aSizePolicy.setHorizontalStretch(20);
	aSizePolicy.setVerticalStretch(0);
	aSizePolicy.setHeightForWidth(m_pBrowserWidget->sizePolicy().hasHeightForWidth());
	m_pBrowserWidget->setSizePolicy(aSizePolicy);
	m_pSplitter->addWidget(m_pBrowserWidget);

	ui->horizontalLayout->addWidget(m_pSplitter);



// The following is supposed to be another workaround for QTBUG-13768
//	m_pSplitter->setStyleSheet("QSplitterHandle:hover {}  QSplitter::handle:hover { background-color: palette(highlight); }");
	m_pSplitter->handle(1)->setAttribute(Qt::WA_Hover);		// Work-Around QTBUG-13768
	setStyleSheet("QSplitter::handle:hover { background-color: palette(highlight); }");


	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode = m_pSearchResultWidget->displayMode();
	CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode = m_pSearchResultWidget->treeMode();
	bool bShowMissingLeafs = m_pSearchResultWidget->showMissingLeafs();

	// --------------------

	QAction *pAction;

	// --- File Menu
	QMenu *pFileMenu = ui->menuBar->addMenu("&File");

	pAction = pFileMenu->addAction(QIcon(":/res/file-new-icon2.png"), "&New Search", this, SLOT(on_NewSearch()), QKeySequence(Qt::CTRL + Qt::Key_N));
	pAction->setStatusTip("Clear All Search Phrases and Begin New Search");
	pAction->setToolTip("Clear All Search Phrases and Begin New Search");
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/open-file-icon3.png"), "L&oad Search File...", this, SLOT(on_OpenSearch()), QKeySequence(Qt::CTRL + Qt::Key_O));
	pAction->setStatusTip("Load Search Phrases from a previously saved King James Search File");
	pAction->setToolTip("Load Search Phrases from a previously saved King James Search File");
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/save-file-icon3.png"), "&Save Search File...", this, SLOT(on_SaveSearch()), QKeySequence(Qt::CTRL + Qt::Key_S));
	pAction->setStatusTip("Save current Search Phrases to a King James Search File");
	pAction->setToolTip("Save current Search Phrases to a King James Search File");
	ui->mainToolBar->addAction(pAction);

	pFileMenu->addSeparator();

	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), "E&xit", this, SLOT(close()), QKeySequence(Qt::CTRL + Qt::Key_Q));
	pAction->setStatusTip("Exit the King James Pure Bible Search Application");
	pAction->setToolTip("Exit Application");
	pAction->setMenuRole(QAction::QuitRole);

	ui->mainToolBar->addSeparator();

	// --- Edit Menu
	connect(m_pBrowserWidget->browser(), SIGNAL(activatedScriptureText()), this, SLOT(on_activatedBrowser()));
	connect(m_pSearchResultWidget->treeView(), SIGNAL(activatedSearchResults()), this, SLOT(on_activatedSearchResults()));

	// --- View Menu
	m_pViewMenu = ui->menuBar->addMenu("&View");

	QMenu *pViewToolbarsMenu = m_pViewMenu->addMenu("&Toolbars");
	pViewToolbarsMenu->addAction(ui->mainToolBar->toggleViewAction());
	ui->mainToolBar->toggleViewAction()->setStatusTip("Show/Hide Main Tool Bar");

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addSeparator();

	m_pActionShowAsList = m_pViewMenu->addAction("View as &List", this, SLOT(on_viewAsList()));
	m_pActionShowAsList->setStatusTip("Show Search Results as a List");
	m_pActionShowAsList->setCheckable(true);
	m_pActionShowAsList->setChecked(nTreeMode == CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionShowAsList);

	m_pActionShowAsTreeBooks = m_pViewMenu->addAction("View as Tree by &Book", this, SLOT(on_viewAsTreeBooks()));
	m_pActionShowAsTreeBooks->setStatusTip("Show Search Results in a Tree by Book");
	m_pActionShowAsTreeBooks->setCheckable(true);
	m_pActionShowAsTreeBooks->setChecked(nTreeMode == CVerseListModel::VTME_TREE_BOOKS);
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionShowAsTreeBooks);

	m_pActionShowAsTreeChapters = m_pViewMenu->addAction("View as Tree by Book/&Chapter", this, SLOT(on_viewAsTreeChapters()));
	m_pActionShowAsTreeChapters->setStatusTip("Show Search Results in a Tree by Book and Chapter");
	m_pActionShowAsTreeChapters->setCheckable(true);
	m_pActionShowAsTreeChapters->setChecked(nTreeMode == CVerseListModel::VTME_TREE_CHAPTERS);
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionShowAsTreeChapters);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addSeparator();

	m_pActionShowMissingLeafs = m_pViewMenu->addAction("View &Missing Books/Chapters", this, SLOT(on_viewShowMissingsLeafs()));
	m_pActionShowMissingLeafs->setStatusTip("Show Missing Books and/or Chapters in the Tree (ones that had no matching Search Results)");
	m_pActionShowMissingLeafs->setCheckable(true);
	m_pActionShowMissingLeafs->setChecked(bShowMissingLeafs);
	m_pActionShowMissingLeafs->setEnabled(nTreeMode != CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionShowMissingLeafs);

	m_pActionExpandAll = m_pViewMenu->addAction("E&xpand All", m_pSearchResultWidget->treeView(), SLOT(expandAll()));
	m_pActionExpandAll->setStatusTip("Expand all tree nodes in Search Results (Warning: May be slow if there are a lot of search results!)");
	m_pActionExpandAll->setEnabled(false);
	connect(m_pSearchResultWidget->treeView(), SIGNAL(canExpandAll(bool)), m_pActionExpandAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionExpandAll);

	m_pActionCollapseAll = m_pViewMenu->addAction("Collap&se All", m_pSearchResultWidget->treeView(), SLOT(collapseAll()));
	m_pActionCollapseAll->setStatusTip("Collapse all tree nodes in Search Results");
	m_pActionCollapseAll->setEnabled(false);
	connect(m_pSearchResultWidget->treeView(), SIGNAL(canCollapseAll(bool)), m_pActionCollapseAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionCollapseAll);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addSeparator();

	m_pActionShowVerseHeading = m_pViewMenu->addAction("View &References Only", this, SLOT(on_viewVerseHeading()));
	m_pActionShowVerseHeading->setStatusTip("Show Search Results Verse References Only");
	m_pActionShowVerseHeading->setCheckable(true);
	m_pActionShowVerseHeading->setChecked(nDisplayMode == CVerseListModel::VDME_HEADING);
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionShowVerseHeading);

	m_pActionShowVerseRichText = m_pViewMenu->addAction("View Verse &Preview", this, SLOT(on_viewVerseRichText()));
	m_pActionShowVerseRichText->setStatusTip("Show Search Results as Rich Text Verse Preview");
	m_pActionShowVerseRichText->setCheckable(true);
	m_pActionShowVerseRichText->setChecked(nDisplayMode == CVerseListModel::VDME_RICHTEXT);
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionShowVerseRichText);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addSeparator();

	m_pActionViewDetails = m_pViewMenu->addAction("View &Details...", this, SLOT(on_viewDetails()), QKeySequence(Qt::CTRL + Qt::Key_D));
	m_pActionViewDetails->setStatusTip("View Passage Details");
	m_pActionViewDetails->setEnabled(false);
	connect(this, SIGNAL(canShowDetails(bool)), m_pActionViewDetails, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->treeView()->getLocalEditMenu()->addAction(m_pActionViewDetails);

	// --- Navigate Menu
	QMenu *pNavMenu = ui->menuBar->addMenu("&Navigate");

	pAction = pNavMenu->addAction("Beginning of Bible", m_pBrowserWidget, SLOT(on_Bible_Beginning()), QKeySequence(Qt::ALT + Qt::Key_Home));
	pAction->setStatusTip("Goto the very Beginning of the Bible");
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	pAction = pNavMenu->addAction("Ending of Bible", m_pBrowserWidget, SLOT(on_Bible_Ending()), QKeySequence(Qt::ALT + Qt::Key_End));
	pAction->setStatusTip("Goto the very End of the Bible");
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	m_pActionBookBackward = pNavMenu->addAction("Book Backward", m_pBrowserWidget, SLOT(on_Book_Backward()), QKeySequence(Qt::CTRL + Qt::Key_PageUp));
	m_pActionBookBackward->setStatusTip("Move Backward one Book");
	connect(m_pActionBookBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	m_pActionBookForward = pNavMenu->addAction("Book Forward", m_pBrowserWidget, SLOT(on_Book_Forward()), QKeySequence(Qt::CTRL + Qt::Key_PageDown));
	m_pActionBookForward->setStatusTip("Move Forward one Book");
	connect(m_pActionBookForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	m_pActionChapterBackward = pNavMenu->addAction("Chapter Backward", m_pBrowserWidget, SLOT(on_ChapterBackward()), QKeySequence(Qt::ALT + Qt::Key_PageUp));
	m_pActionChapterBackward->setStatusTip("Move Backward one Chapter");
	connect(m_pActionChapterBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	m_pActionChapterForward = pNavMenu->addAction("Chapter Forward", m_pBrowserWidget, SLOT(on_ChapterForward()), QKeySequence(Qt::ALT + Qt::Key_PageDown));
	m_pActionChapterForward->setStatusTip("Move Forward one Chapter");
	connect(m_pActionChapterForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	connect(m_pBrowserWidget, SIGNAL(IndexChanged(const TPhraseTag &)), this, SLOT(on_indexChanged(const TPhraseTag &)));

	pNavMenu->addSeparator();

	m_pActionNavBackward = new QAction(QIcon(":/res/Nav3_Arrow_Left.png"), "History &Backward", this);
	m_pActionNavBackward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	m_pActionNavBackward->setStatusTip("Go Backward in Navigation History");
	ui->mainToolBar->addAction(m_pActionNavBackward);
	connect(m_pBrowserWidget->browser(), SIGNAL(backwardAvailable(bool)), m_pActionNavBackward, SLOT(setEnabled(bool)));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget->browser(), SLOT(backward()));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	m_pActionNavBackward->setEnabled(m_pBrowserWidget->browser()->isBackwardAvailable());
	pNavMenu->addAction(m_pActionNavBackward);

	m_pActionNavForward = new QAction(QIcon(":/res/Nav3_Arrow_Right.png"), "History &Forward", this);
	m_pActionNavForward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));
	m_pActionNavForward->setStatusTip("Go Forward in Navigation History");
	ui->mainToolBar->addAction(m_pActionNavForward);
	connect(m_pBrowserWidget->browser(), SIGNAL(forwardAvailable(bool)), m_pActionNavForward, SLOT(setEnabled(bool)));
	connect(m_pActionNavForward, SIGNAL(triggered()), m_pBrowserWidget->browser(), SLOT(forward()));
	connect(m_pActionNavForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
	m_pActionNavForward->setEnabled(m_pBrowserWidget->browser()->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavForward);

	m_pActionNavHome = pNavMenu->addAction(QIcon(":/res/go_home.png"), "History &Home", m_pBrowserWidget->browser(), SLOT(home()), QKeySequence(Qt::ALT + Qt::Key_Up));
	m_pActionNavHome->setStatusTip("Jump to History Home Passage");
	m_pActionNavHome->setEnabled(m_pBrowserWidget->browser()->isBackwardAvailable() ||
									m_pBrowserWidget->browser()->isForwardAvailable());

	m_pActionNavClear = new QAction(QIcon(":/res/edit_clear.png"), "&Clear Navigation History", this);
	m_pActionNavClear->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));
	m_pActionNavClear->setStatusTip("Clear All Passage Navigation History");
	ui->mainToolBar->addAction(m_pActionNavClear);
	connect(m_pActionNavClear, SIGNAL(triggered()), this, SLOT(on_clearBrowserHistory()));
	m_pActionNavClear->setEnabled(m_pBrowserWidget->browser()->isBackwardAvailable() ||
									m_pBrowserWidget->browser()->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavClear);

	connect(m_pBrowserWidget->browser(), SIGNAL(historyChanged()), this, SLOT(on_browserHistoryChanged()));

//	m_pActionJump = new QAction(QIcon(":/res/go_jump2.png"), "Passage Navigator", this);
	m_pActionJump = new QAction(QIcon(":/res/green_arrow.png"), "Passage &Navigator", this);
	m_pActionJump->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	m_pActionJump->setStatusTip("Display the Passage Navigator Widget");
	ui->mainToolBar->addAction(m_pActionJump);
	connect(m_pActionJump, SIGNAL(triggered()), this, SLOT(on_PassageNavigatorTriggered()));

	pNavMenu->addSeparator();
	pNavMenu->addAction(m_pActionJump);

	// --- Settings Menu
	QMenu *pSettingsMenu = ui->menuBar->addMenu("Se&ttings");

	pAction = pSettingsMenu->addAction("Scripture &Browser Font...", this, SLOT(on_setBrowserFont()));
	pAction->setStatusTip("Adjust the Scripture Browser Font");
	pAction->setToolTip("Adjust the Scripture Browser Font");

	pAction = pSettingsMenu->addAction("Search &Results Font...", this, SLOT(on_setSearchResultsFont()));
	pAction->setStatusTip("Adjust the Search Results Font");
	pAction->setToolTip("Adjust the Search Results Font");

	// --- Help Menu
	QMenu *pHelpMenu = ui->menuBar->addMenu("&Help");
	pAction = pHelpMenu->addAction(QIcon(":/res/help_book.png"), "&Help", this, SLOT(on_HelpManual()), QKeySequence(Qt::SHIFT + Qt::Key_F1));
	pAction->setStatusTip("Display the Users Manual");

	m_pActionAbout = new QAction(QIcon(":/res/help_icon1.png"), "About...", this);
	m_pActionAbout->setShortcut(QKeySequence(Qt::Key_F1));
	m_pActionAbout->setStatusTip("About the King James Pure Bible Search");
	m_pActionAbout->setToolTip("About the King James Pure Bible Search...");
	m_pActionAbout->setMenuRole(QAction::AboutRole);
	connect(m_pActionAbout, SIGNAL(triggered()), this, SLOT(on_HelpAbout()));
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addAction(m_pActionAbout);
	pHelpMenu->addAction(m_pActionAbout);

	// -------------------- Quick Activate:

	for (int ndx=0; ndx<NUM_QUICK_ACTIONS; ++ndx) {
		m_lstpQuickActivate.append(new QAction(this));
		m_lstpQuickActivate.at(ndx)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0 + ndx));
		connect(m_lstpQuickActivate.at(ndx), SIGNAL(triggered()), this, SLOT(on_QuickActivate()));
	}
	addActions(m_lstpQuickActivate);

	// -------------------- Search Phrase Widgets:

	m_pLayoutPhrases = new QVBoxLayout(ui->scrollAreaWidgetContents);
	m_pLayoutPhrases->setSpacing(0);
	m_pLayoutPhrases->setContentsMargins(0, 0, 0, 0);

	ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	CKJVSearchPhraseEdit *pFirstSearchPhraseEditor = addSearchPhrase();
	QTimer::singleShot(0, pFirstSearchPhraseEditor, SLOT(focusEditor()));

	ui->scrollAreaSearchPhrases->setMinimumSize(m_pLayoutPhrases->sizeHint().width() +
							ui->scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui->scrollAreaSearchPhrases->frameWidth() * 2,
							m_pLayoutPhrases->sizeHint().height() /* pFirstSearchPhraseEditor->sizeHint() */);


//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);


	ui->widgetSearchCriteria->enableCopySearchPhraseSummary(false);

	connect(ui->widgetSearchCriteria, SIGNAL(addSearchPhraseClicked()), this, SLOT(addSearchPhrase()));
	connect(ui->widgetSearchCriteria, SIGNAL(changedSearchScopeMode(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM)), this, SLOT(on_changedSearchCriteria()));
	connect(ui->widgetSearchCriteria, SIGNAL(copySearchPhraseSummary()), this, SLOT(on_copySearchPhraseSummary()));

	// -------------------- Search Results List View:

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedSearchResults(const QFont &)), m_pSearchResultWidget->treeView(), SLOT(setFont(const QFont &)));
// TODO : CLEAN
//	connect(CPersistentSettings::instance(), SIGNAL(fontChangedSearchResults(const QFont &)), model, SLOT(setFont(const QFont &)));


	connect(m_pSearchResultWidget->treeView(), SIGNAL(activated(const QModelIndex &)), this, SLOT(on_SearchResultActivated(const QModelIndex &)));
	connect(m_pSearchResultWidget->treeView(), SIGNAL(gotoIndex(const TPhraseTag &)), m_pBrowserWidget, SLOT(gotoIndex(const TPhraseTag &)));
	connect(m_pSearchResultWidget->treeView(), SIGNAL(setDetailsEnable()), this, SLOT(setDetailsEnable()));

	// -------------------- Scripture Browser:

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedBrowser(const QFont &)), m_pBrowserWidget->browser(), SLOT(setFont(const QFont &)));

	// -------------------- Persistent Settings:
	restorePersistentSettings();
}

CKJVCanOpener::~CKJVCanOpener()
{
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		delete m_lstSearchPhraseEditors[ndx];
	}
	m_lstSearchPhraseEditors.clear();

	delete ui;
}

void CKJVCanOpener::initialize()
{
	assert(m_pBibleDatabase.data() != NULL);

	QSettings &settings(CPersistentSettings::instance()->settings());

	TPhraseTag tag;
	settings.beginGroup(constrBrowserViewGroup);
	// Read last location : Default initial location is Genesis 1
	tag.first = CRelIndex(settings.value(constrLastReferenceKey, CRelIndex(1,1,0,0).asAnchor()).toString());	// Default for unset key
	if (!tag.first.isSet()) tag.first = CRelIndex(1,1,0,0);		// Default for zero key
	tag.second = settings.value(constrLastSelectionSizeKey, 0).toUInt();
	settings.endGroup();

	m_pBrowserWidget->gotoIndex(tag);
}

void CKJVCanOpener::savePersistentSettings()
{
	QSettings &settings(CPersistentSettings::instance()->settings());

	// Main App and Toolbars:
	settings.beginGroup(constrMainAppRestoreStateGroup);
	settings.setValue(constrGeometryKey, saveGeometry());
	settings.setValue(constrWindowStateKey, saveState(KJVAPP_REGISTRY_VERSION));
	settings.endGroup();

	// Splitter:
	settings.beginGroup(constrSplitterRestoreStateGroup);
	settings.setValue(constrWindowStateKey, m_pSplitter->saveState());
	settings.endGroup();

	// Search Results mode:
	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);
	settings.beginGroup(constrSearchResultsViewGroup);
	settings.setValue(constrVerseDisplayModeKey, pModel->displayMode());
	settings.setValue(constrVerseTreeModeKey, pModel->treeMode());
	settings.setValue(constrViewMissingNodesKey, pModel->showMissingLeafs());
	settings.setValue(constrCurrentIndexKey, CRelIndex(m_pSearchResultWidget->treeView()->currentIndex().internalId()).asAnchor());
	settings.setValue(constrHasFocusKey, m_pSearchResultWidget->treeView()->hasFocus());
	settings.setValue(constrFontKey, CPersistentSettings::instance()->fontSearchResults().toString());
	settings.endGroup();

	// Last Search:
	writeKJVSearchFile(settings, constrLastSearchGroup);

	// Current Browser Reference:
	settings.beginGroup(constrBrowserViewGroup);
	TPhraseTag tag = m_pBrowserWidget->browser()->selection();
	settings.setValue(constrLastReferenceKey, tag.first.asAnchor());
	settings.setValue(constrLastSelectionSizeKey, tag.second);
	settings.setValue(constrHasFocusKey, m_pBrowserWidget->hasFocusBrowser());
	settings.setValue(constrFontKey, CPersistentSettings::instance()->fontBrowser().toString());
	settings.endGroup();

	// Browser Object (used for FindDialog, etc):
	m_pBrowserWidget->browser()->savePersistentSettings(constrBrowserViewGroup);
}

void CKJVCanOpener::restorePersistentSettings()
{
	QSettings &settings(CPersistentSettings::instance()->settings());
	QString strFont;

	// Main App and Toolbars:
	settings.beginGroup(constrMainAppRestoreStateGroup);
	restoreGeometry(settings.value(constrGeometryKey).toByteArray());
	restoreState(settings.value(constrWindowStateKey).toByteArray(), KJVAPP_REGISTRY_VERSION);
	settings.endGroup();

	// Splitter:
	settings.beginGroup(constrSplitterRestoreStateGroup);
	m_pSplitter->restoreState(settings.value(constrWindowStateKey).toByteArray());
	settings.endGroup();

	// Search Results mode:
	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);
	settings.beginGroup(constrSearchResultsViewGroup);
	setDisplayMode(static_cast<CVerseListModel::VERSE_DISPLAY_MODE_ENUM>(settings.value(constrVerseDisplayModeKey, pModel->displayMode()).toUInt()));
	setTreeMode(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(settings.value(constrVerseTreeModeKey, pModel->treeMode()).toUInt()));
	setShowMissingLeafs(settings.value(constrViewMissingNodesKey, pModel->showMissingLeafs()).toBool());
	CRelIndex ndxLastCurrentIndex(settings.value(constrCurrentIndexKey, CRelIndex().asAnchor()).toString());
	bool bFocusSearchResults = settings.value(constrHasFocusKey, false).toBool();
	strFont = settings.value(constrFontKey).toString();
	if (!strFont.isEmpty()) {
		QFont aFont;
		aFont.fromString(strFont);
		CPersistentSettings::instance()->setFontSearchResults(aFont);
	}
	settings.endGroup();

	// Last Search:
	readKJVSearchFile(settings, constrLastSearchGroup);

	// Current Browser Reference:
	//		Note: The actual browser reference has already been loaded in
	//			initialize().  However, this will lookup the reference and
	//			see if the user was displaying a search result and if so,
	//			will set the current index for the search result to that
	//			as a fallback for when there is no Last Current Index:
	bool bLastSet = false;
	if (ndxLastCurrentIndex.isSet()) bLastSet = setCurrentIndex(ndxLastCurrentIndex, false);
	settings.beginGroup(constrBrowserViewGroup);
	bool bFocusBrowser = settings.value(constrHasFocusKey, false).toBool();
	if (!bLastSet) {
		CRelIndex ndxLastBrowsed = CRelIndex(settings.value(constrLastReferenceKey, CRelIndex().asAnchor()).toString());
		if (ndxLastBrowsed.isSet()) setCurrentIndex(ndxLastBrowsed, false);
	}
	strFont = settings.value(constrFontKey).toString();
	if (!strFont.isEmpty()) {
		QFont aFont;
		aFont.fromString(strFont);
		CPersistentSettings::instance()->setFontBrowser(aFont);
	}
	settings.endGroup();

	// Browser Object (used for FindDialog, etc):
	m_pBrowserWidget->browser()->restorePersistentSettings(constrBrowserViewGroup);

	// If the Search Result was focused last time, focus it again, else if
	//	the browser was focus last time, focus it again.  Otherwise, leave
	//	the phrase editor focus:
	if ((bFocusSearchResults) && (pModel->hasChildren())) {
		QTimer::singleShot(1, m_pSearchResultWidget->treeView(), SLOT(setFocus()));
	} else if (bFocusBrowser) {
		QTimer::singleShot(1, m_pBrowserWidget, SLOT(focusBrowser()));
	}
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

	savePersistentSettings();

	return QMainWindow::closeEvent(event);
}

// ------------------------------------------------------------------

void CKJVCanOpener::setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	switch (nDisplayMode) {
		case CVerseListModel::VDME_HEADING:
			m_pActionShowVerseHeading->setChecked(true);
			on_viewVerseHeading();
			break;
		case CVerseListModel::VDME_RICHTEXT:
			m_pActionShowVerseHeading->setChecked(true);
			on_viewVerseRichText();
			break;
		default:
			assert(false);		// Did you add some modes and forget to add them here?
			break;
	}
}

void CKJVCanOpener::setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode)
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);

	switch (nTreeMode) {
		case CVerseListModel::VTME_LIST:
			m_pActionShowAsList->setChecked(true);
			on_viewAsList();
			break;
		case CVerseListModel::VTME_TREE_BOOKS:
			m_pActionShowAsTreeBooks->setChecked(true);
			on_viewAsTreeBooks();
			break;
		case CVerseListModel::VTME_TREE_CHAPTERS:
			m_pActionShowAsTreeChapters->setChecked(true);
			on_viewAsTreeChapters();
			break;
		default:
			assert(false);		// Did you add some modes and forget to add them here?
			break;
	}
}

void CKJVCanOpener::setShowMissingLeafs(bool bShowMissing)
{
	assert(m_pActionShowMissingLeafs != NULL);
	m_pActionShowMissingLeafs->setChecked(bShowMissing);
	on_viewShowMissingsLeafs();
}

// ------------------------------------------------------------------

void CKJVCanOpener::on_NewSearch()
{
	closeAllSearchPhrases();
	addSearchPhrase();
	ui->widgetSearchCriteria->setSearchScopeMode(CKJVSearchCriteria::SSME_WHOLE_BIBLE);
}

void CKJVCanOpener::on_OpenSearch()
{
	QString strFilePathName = QFileDialog::getOpenFileName(this, "Open KJV Search File", QString(), "KJV Search Files (*.kjs)", NULL, QFileDialog::ReadOnly);
	if (!strFilePathName.isEmpty())
		if (!openKJVSearchFile(strFilePathName))
			QMessageBox::warning(this, "KJV Search File Open Failed", "Failed to open and read the specified KJV Search File!");
}

void CKJVCanOpener::on_SaveSearch()
{
	QString strFilePathName = QFileDialog::getSaveFileName(this, "Save KJV Search File", QString(), "KJV Search Files (*.kjs)", NULL, 0);
	if (!strFilePathName.isEmpty())
		if (!saveKJVSearchFile(strFilePathName))
			QMessageBox::warning(this, "KJV Search File Save Failed", "Failed to save the specified KJV Search File!");
}

void CKJVCanOpener::closeAllSearchPhrases()
{
	for (int ndx = m_lstSearchPhraseEditors.size()-1; ndx>=0; --ndx) {
		m_lstSearchPhraseEditors.at(ndx)->closeSearchPhrase();
	}
}

bool CKJVCanOpener::openKJVSearchFile(const QString &strFilePathName)
{
	QSettings kjsFile(strFilePathName, QSettings::IniFormat);
	if (kjsFile.status() != QSettings::NoError) return false;

	unsigned int nFileVersion = 0;

	kjsFile.beginGroup("KJVPureBibleSearch");
	nFileVersion = kjsFile.value("KJSFileVersion").toUInt();
	kjsFile.endGroup();

	if (nFileVersion < KJS_FILE_VERSION) {
		QMessageBox::warning(this, "Opening King James Search File", "Warning: The file you are opening was saved on "
									"an older version of King James Pure Bible Search.  Some manual editing may be necessary "
									"to configure any new search options added since that older version.\n\n"
									"To avoid this message when opening this file in the future, then resave your "
									"search phrases over top of this file, replacing this old version.");
	} else if (nFileVersion > KJS_FILE_VERSION) {
		QMessageBox::warning(this, "Opening King James Search File", "Warning: The file you are opening was created on "
									"a newer version of King James Pure Bible Search.  It may contain settings for options not "
									"available on this version of King James Pure Bible Search.  If so, those options will be "
									"ignored.");
	}

	readKJVSearchFile(kjsFile);

	return (kjsFile.status() == QSettings::NoError);
}

bool CKJVCanOpener::saveKJVSearchFile(const QString &strFilePathName) const
{
	QSettings kjsFile(strFilePathName, QSettings::IniFormat);
	if (kjsFile.status() != QSettings::NoError) return false;

	kjsFile.clear();

	kjsFile.beginGroup("KJVPureBibleSearch");
	kjsFile.setValue("AppVersion", VER_QT);
	kjsFile.setValue("KJSFileVersion", KJS_FILE_VERSION);
	kjsFile.endGroup();

	writeKJVSearchFile(kjsFile);

	kjsFile.sync();

	return (kjsFile.status() == QSettings::NoError);
}

void CKJVCanOpener::readKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup)
{
	CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScope = CKJVSearchCriteria::SSME_WHOLE_BIBLE;

	closeAllSearchPhrases();

	kjsFile.beginGroup(groupCombine(strSubgroup, "SearchCriteria"));
	nSearchScope = static_cast<CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(kjsFile.value("SearchScope", CKJVSearchCriteria::SSME_WHOLE_BIBLE).toInt());
	if ((nSearchScope < CKJVSearchCriteria::SSME_WHOLE_BIBLE) ||
		(nSearchScope > CKJVSearchCriteria::SSME_VERSE))
		nSearchScope = CKJVSearchCriteria::SSME_WHOLE_BIBLE;
	kjsFile.endGroup();

	ui->widgetSearchCriteria->setSearchScopeMode(nSearchScope);

	CKJVSearchPhraseEdit *pFirstSearchPhraseEditor = NULL;
	int nPhrases = kjsFile.beginReadArray(groupCombine(strSubgroup, "SearchPhrases"));
	if (nPhrases != 0) {
		for (int ndx = 0; ndx < nPhrases; ++ndx) {
			CKJVSearchPhraseEdit *pPhraseEditor = addSearchPhrase();
			assert(pPhraseEditor != NULL);
			if (ndx == 0) pFirstSearchPhraseEditor = pPhraseEditor;
			kjsFile.setArrayIndex(ndx);
			pPhraseEditor->phraseEditor()->setCaseSensitive(kjsFile.value("CaseSensitive", false).toBool());
			pPhraseEditor->phraseEditor()->setText(kjsFile.value("Phrase").toString());
		}
	} else {
		// If the search had no phrases (like default loading from registry), start
		//		with a single empty search phrase:
		pFirstSearchPhraseEditor = addSearchPhrase();
	}
	kjsFile.endArray();

	// Set focus to our first editor.  Note that calling of focusEditor
	//	doesn't work when running from the constructor during a restore
	//	operation.  So we'll set it to trigger later:
	assert(pFirstSearchPhraseEditor != NULL);
	QTimer::singleShot(0, pFirstSearchPhraseEditor, SLOT(focusEditor()));
}

void CKJVCanOpener::writeKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup) const
{
	kjsFile.beginGroup(groupCombine(strSubgroup, "SearchCriteria"));
	kjsFile.setValue("SearchScope", ui->widgetSearchCriteria->searchScopeMode());
	kjsFile.endGroup();

	int ndxCurrent = 0;
	kjsFile.beginWriteArray(groupCombine(strSubgroup, "SearchPhrases"));
	kjsFile.remove("");
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		if (m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase().isEmpty()) continue;
		kjsFile.setArrayIndex(ndxCurrent);
		kjsFile.setValue("Phrase", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase());
		kjsFile.setValue("CaseSensitive", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isCaseSensitive());
		ndxCurrent++;
	}
	kjsFile.endArray();
}

// ------------------------------------------------------------------

CKJVSearchPhraseEdit *CKJVCanOpener::addSearchPhrase()
{
	assert(m_pBibleDatabase.data() != NULL);

	CKJVSearchPhraseEdit *pPhraseWidget = new CKJVSearchPhraseEdit(m_pBibleDatabase, haveUserDatabase(), this);
	connect(pPhraseWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SLOT(on_closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(on_phraseChanged(CKJVSearchPhraseEdit *)));
	m_lstSearchPhraseEditors.append(pPhraseWidget);
	pPhraseWidget->showSeperatorLine(m_lstSearchPhraseEditors.size() > 1);
	pPhraseWidget->resize(pPhraseWidget->minimumSizeHint());
	m_pLayoutPhrases->addWidget(pPhraseWidget);
	// Calculate height, since it varies depending on whether or not the widget is showing a separator:
	int nHeight = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
	}
	ui->scrollAreaWidgetContents->setMinimumSize(pPhraseWidget->sizeHint().width(), nHeight);
	ensureSearchPhraseVisible(pPhraseWidget);
	pPhraseWidget->phraseStatisticsChanged();
	pPhraseWidget->focusEditor();

//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);

	return pPhraseWidget;
}

void CKJVCanOpener::ensureSearchPhraseVisible(int nIndex)
{
	if ((nIndex >= 0) && (nIndex < m_lstSearchPhraseEditors.size())) {
		ensureSearchPhraseVisible(m_lstSearchPhraseEditors.at(nIndex));
	}
}

void CKJVCanOpener::ensureSearchPhraseVisible(const CKJVSearchPhraseEdit *pSearchPhrase)
{
	// Calculate height, since it varies depending on whether or not the widget is showing a separator:
	int nHeight = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
		if (m_lstSearchPhraseEditors.at(ndx) == pSearchPhrase) break;
	}
	ui->scrollAreaSearchPhrases->ensureVisible((pSearchPhrase->sizeHint().width()/2),
												nHeight - (pSearchPhrase->sizeHint().height()/2));
}

void CKJVCanOpener::on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase)
{
	assert(pSearchPhrase != NULL);

	// If this search phrase's editor was currently active, remove it or else
	//		we'll crash later accessing data for a deleted object:
	if ((m_bPhraseEditorActive) && ((m_pActionSearchPhraseEditMenu != NULL) &&
									(m_pActionSearchPhraseEditMenu->menu() == pSearchPhrase->phraseEditor()->getEditMenu()))) {
		on_addSearchPhraseEditMenu(false);
	}

	bool bPhraseChanged = ((!pSearchPhrase->parsedPhrase()->IsDuplicate()) &&
							(pSearchPhrase->parsedPhrase()->GetNumberOfMatches() != 0) &&
							(pSearchPhrase->parsedPhrase()->isCompleteMatch()));

	int ndx = m_lstSearchPhraseEditors.indexOf(pSearchPhrase);
	assert(ndx != -1);
	if (ndx != -1) {
		m_lstSearchPhraseEditors.removeAt(ndx);
	}
	if ((ndx == 0) && (m_lstSearchPhraseEditors.size() != 0))
		m_lstSearchPhraseEditors.at(0)->showSeperatorLine(false);

	int nHeight = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
	}
	ui->scrollAreaWidgetContents->setMinimumSize(ui->scrollAreaWidgetContents->minimumSize().width(), nHeight);
	if (bPhraseChanged) on_phraseChanged(NULL);
}

void CKJVCanOpener::on_changedSearchCriteria()
{
	on_phraseChanged(NULL);
}

typedef struct {
	unsigned int m_nNumMatches;
	unsigned int m_nNumContributingMatches;
} TPhraseOccurrenceInfo;
Q_DECLARE_METATYPE(TPhraseOccurrenceInfo)

void CKJVCanOpener::on_copySearchPhraseSummary()
{
	int nNumPhrases = 0;
	bool bCaseSensitive = false;

	CPhraseList phrases;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		if ((pPhrase->GetNumberOfMatches()) &&
			(!pPhrase->IsDuplicate())) {
			nNumPhrases++;
			CPhraseEntry entry;
			entry.m_bCaseSensitive = pPhrase->isCaseSensitive();
			entry.m_strPhrase = pPhrase->phrase();
			entry.m_nNumWrd = pPhrase->phraseSize();
			TPhraseOccurrenceInfo poiUsage;
			poiUsage.m_nNumMatches = pPhrase->GetNumberOfMatches();
			poiUsage.m_nNumContributingMatches = pPhrase->GetContributingNumberOfMatches();
			entry.m_varExtraInfo = QVariant::fromValue(poiUsage);
			phrases.append(entry);
			if (entry.m_bCaseSensitive) bCaseSensitive = true;
		}
	}

	CPhraseListModel mdlPhrases(phrases);
	mdlPhrases.sort(0);

	QString strScope;
	CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nScope = ui->widgetSearchCriteria->searchScopeMode();
	switch (nScope) {
		case (CKJVSearchCriteria::SSME_WHOLE_BIBLE):
			strScope = "in the Entire Bible";
			break;
		case (CKJVSearchCriteria::SSME_TESTAMENT):
			strScope = "in the same Testament";
			break;
		case (CKJVSearchCriteria::SSME_BOOK):
			strScope = "in the same Book";
			break;
		case (CKJVSearchCriteria::SSME_CHAPTER):
			strScope = "in the same Chapter";
			break;
		case (CKJVSearchCriteria::SSME_VERSE):
			strScope = "in the same Verse";
			break;
		default:
			break;
	}

	QString strSummary;
	if (nNumPhrases != 1) {
		strSummary += QString("Search of %1 Phrases %2:\n")
								.arg(nNumPhrases)
								.arg(strScope);
	} else {
		strSummary += QString("Search of: ");
	}
	if (nNumPhrases > 1) strSummary += "\n";
	for (int ndx=0; ndx<mdlPhrases.rowCount(); ++ndx) {
		const CPhraseEntry &aPhrase = mdlPhrases.index(ndx).data(CPhraseListModel::PHRASE_ENTRY_ROLE).value<CPhraseEntry>();
		if (nNumPhrases > 1) {
			if (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE) {
				strSummary += QString("    \"%1\" (Found %2 Time%3 in the Entire Bible, %4 in Scope)\n")
									.arg(mdlPhrases.index(ndx).data().toString())
									.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches)
									.arg((aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches != 1) ? "s" : "")
									.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumContributingMatches);
			} else {
				strSummary += QString("    \"%1\" (Found %2 Time%3 in the Entire Bible)\n")
									.arg(mdlPhrases.index(ndx).data().toString())
									.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches)
									.arg((aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches != 1) ? "s" : "");
				assert(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches == aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumContributingMatches);
			}
		} else {
			strSummary += QString("\"%1\"\n").arg(mdlPhrases.index(ndx).data().toString());
		}
	}
	if (bCaseSensitive) {
		if (nNumPhrases > 1) strSummary += "\n";
		strSummary += QString("    (%1 = Case Sensitive)\n").arg(QChar(0xA7));
	}
	if (nNumPhrases) strSummary += "\n";
	if (m_bLastCalcSuccess) {
		strSummary += QString("Found %1 %2Occurrence%3\n").arg(m_nLastSearchOccurrences).arg((nNumPhrases > 1) ? "Combined " : "").arg((m_nLastSearchOccurrences != 1) ? "s" : "");
		strSummary += QString("    in %1 Verse%2\n").arg(m_nLastSearchVerses).arg((m_nLastSearchVerses != 1) ? "s" : "");
		strSummary += QString("    in %1 Chapter%2\n").arg(m_nLastSearchChapters).arg((m_nLastSearchChapters != 1) ? "s" : "");
		strSummary += QString("    in %1 Book%2\n").arg(m_nLastSearchBooks).arg((m_nLastSearchBooks != 1) ? "s" : "");
		strSummary += "\n";
		strSummary += QString("Not found%1 at all in %2 Verse%3 of the Bible\n").arg(((nNumPhrases > 1) && (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(g_pMainBibleDatabase->bibleEntry().m_nNumVrs - m_nLastSearchVerses).arg(((g_pMainBibleDatabase->bibleEntry().m_nNumVrs - m_nLastSearchVerses) != 1) ? "s" : "");
		strSummary += QString("Not found%1 at all in %2 Chapter%3 of the Bible\n").arg(((nNumPhrases > 1) && (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(g_pMainBibleDatabase->bibleEntry().m_nNumChp - m_nLastSearchChapters).arg(((g_pMainBibleDatabase->bibleEntry().m_nNumChp - m_nLastSearchChapters) != 1) ? "s" : "");
		strSummary += QString("Not found%1 at all in %2 Book%3 of the Bible\n").arg(((nNumPhrases > 1) && (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(g_pMainBibleDatabase->bibleEntry().m_nNumBk - m_nLastSearchBooks).arg(((g_pMainBibleDatabase->bibleEntry().m_nNumBk - m_nLastSearchBooks) != 1) ? "s" : "");
	} else {
		strSummary += QString("Search was incomplete -- too many possible matches\n");
	}

	QMimeData *mime = new QMimeData();

	mime->setText(strSummary);
	mime->setHtml("<qt><pre>\n" + strSummary + "</pre></qt>\n");
	QApplication::clipboard()->setMimeData(mime);
}

void CKJVCanOpener::on_addPassageBrowserEditMenu(bool bAdd)
{
	m_bBrowserActive = bAdd;

	if (bAdd) {
		if (m_pActionPassageBrowserEditMenu == NULL) {
			m_pActionPassageBrowserEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), m_pBrowserWidget->browser()->getEditMenu());
			connect(m_pActionPassageBrowserEditMenu, SIGNAL(triggered()), m_pBrowserWidget, SLOT(focusBrowser()));
		}
	} else {
		if (m_pActionPassageBrowserEditMenu) {
			// The following 'if' is needed for insert race conditions to
			//		keep us from crashing:
			if (ui->menuBar->actions().contains(m_pActionPassageBrowserEditMenu))
				ui->menuBar->removeAction(m_pActionPassageBrowserEditMenu);
			m_pActionPassageBrowserEditMenu = NULL;
		}
	}
}

void CKJVCanOpener::on_addSearchResultsEditMenu(bool bAdd)
{
	m_bSearchResultsActive = bAdd;

	if (bAdd) {
		if (m_pActionSearchResultsEditMenu == NULL) {
			m_pActionSearchResultsEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), m_pSearchResultWidget->treeView()->getEditMenu());
		}
	} else {
		if (m_pActionSearchResultsEditMenu) {
			// The following 'if' is needed for insert race conditions to
			//		keep us from crashing:
			if (ui->menuBar->actions().contains(m_pActionSearchResultsEditMenu))
				ui->menuBar->removeAction(m_pActionSearchResultsEditMenu);
			m_pActionSearchResultsEditMenu = NULL;
		}
	}
}

void CKJVCanOpener::on_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor)
{
	m_bPhraseEditorActive = bAdd;

	if (m_pActionSearchPhraseEditMenu) {
		// The following 'if' is needed for insert race conditions to
		//		keep us from crashing:
		if (ui->menuBar->actions().contains(m_pActionSearchPhraseEditMenu))
			ui->menuBar->removeAction(m_pActionSearchPhraseEditMenu);
		m_pActionSearchPhraseEditMenu = NULL;
	}
	if ((bAdd) && (pEditor != NULL)) {
		m_pActionSearchPhraseEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), pEditor->getEditMenu());
	}
}

void CKJVCanOpener::on_activatedBrowser()
{
	on_addPassageBrowserEditMenu(true);
	on_addSearchResultsEditMenu(false);
	on_addSearchPhraseEditMenu(false);
	setDetailsEnable();
}

void CKJVCanOpener::on_activatedSearchResults()
{
	on_addPassageBrowserEditMenu(false);
	on_addSearchResultsEditMenu(true);
	on_addSearchPhraseEditMenu(false);
	setDetailsEnable();
}

void CKJVCanOpener::on_activatedPhraseEditor(const CPhraseLineEdit *pEditor)
{
	on_addPassageBrowserEditMenu(false);
	on_addSearchResultsEditMenu(false);
	on_addSearchPhraseEditMenu(true, pEditor);
	setDetailsEnable();
}

void CKJVCanOpener::on_viewVerseHeading()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->treeView()->currentIndex().internalId());

	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	if (m_pActionShowVerseHeading->isChecked()) {
		m_pActionShowVerseRichText->setChecked(false);
		pModel->setDisplayMode(CVerseListModel::VDME_HEADING);
	} else {
		if (pModel->displayMode() == CVerseListModel::VDME_HEADING) {
			m_pActionShowVerseHeading->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewVerseRichText()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->treeView()->currentIndex().internalId());

	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	if (m_pActionShowVerseRichText->isChecked()) {
		m_pActionShowVerseHeading->setChecked(false);
		pModel->setDisplayMode(CVerseListModel::VDME_RICHTEXT);
	} else {
		if (pModel->displayMode() == CVerseListModel::VDME_RICHTEXT) {
			m_pActionShowVerseRichText->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsList()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->treeView()->currentIndex().internalId());

	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	if (m_pActionShowAsList->isChecked()) {
		m_pActionShowAsTreeBooks->setChecked(false);
		m_pActionShowAsTreeChapters->setChecked(false);
		pModel->setTreeMode(CVerseListModel::VTME_LIST);
		m_pSearchResultWidget->treeView()->setRootIsDecorated(false);
		m_pActionShowMissingLeafs->setEnabled(false);
	} else {
		if (pModel->treeMode() == CVerseListModel::VTME_LIST) {
			m_pActionShowAsList->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsTreeBooks()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->treeView()->currentIndex().internalId());

	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	if (m_pActionShowAsTreeBooks->isChecked()) {
		m_pActionShowAsList->setChecked(false);
		m_pActionShowAsTreeChapters->setChecked(false);
		pModel->setTreeMode(CVerseListModel::VTME_TREE_BOOKS);
		m_pSearchResultWidget->treeView()->setRootIsDecorated(true);
		m_pActionShowMissingLeafs->setEnabled(true);
	} else {
		if (pModel->treeMode() == CVerseListModel::VTME_TREE_BOOKS) {
			m_pActionShowAsTreeBooks->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsTreeChapters()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->treeView()->currentIndex().internalId());

	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	if (m_pActionShowAsTreeChapters->isChecked()) {
		m_pActionShowAsList->setChecked(false);
		m_pActionShowAsTreeBooks->setChecked(false);
		pModel->setTreeMode(CVerseListModel::VTME_TREE_CHAPTERS);
		m_pSearchResultWidget->treeView()->setRootIsDecorated(true);
		m_pActionShowMissingLeafs->setEnabled(true);
	} else {
		if (pModel->treeMode() == CVerseListModel::VTME_TREE_CHAPTERS) {
			m_pActionShowAsTreeChapters->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewShowMissingsLeafs()
{
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->treeView()->currentIndex().internalId());

	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	if (pModel->treeMode() == CVerseListModel::VTME_LIST) {
		if (pModel->showMissingLeafs()) pModel->setShowMissingLeafs(false);
	} else {
		pModel->setShowMissingLeafs(m_pActionShowMissingLeafs->isChecked());
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

bool CKJVCanOpener::setCurrentIndex(const CRelIndex &ndxCurrent, bool bFocusTreeView)
{
	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	QModelIndex mndxCurrent = pModel->locateIndex(ndxCurrent);
	m_pSearchResultWidget->treeView()->setCurrentIndex(mndxCurrent);
	m_pSearchResultWidget->treeView()->scrollTo(mndxCurrent, QAbstractItemView::EnsureVisible);
	if (bFocusTreeView) m_pSearchResultWidget->treeView()->setFocus();
	return mndxCurrent.isValid();
}

void CKJVCanOpener::on_indexChanged(const TPhraseTag &tag)
{
	assert(m_pActionBookBackward != NULL);
	assert(m_pActionBookForward != NULL);
	assert(m_pActionChapterBackward != NULL);
	assert(m_pActionChapterForward != NULL);
	if ((m_pActionBookBackward == NULL) ||
		(m_pActionBookForward == NULL) ||
		(m_pActionChapterBackward == NULL) ||
		(m_pActionChapterForward == NULL)) return;

	m_pActionBookBackward->setEnabled(tag.first.book() >= 2);
	m_pActionBookForward->setEnabled(tag.first.book() < g_pMainBibleDatabase->bibleEntry().m_nNumBk);
	m_pActionChapterBackward->setEnabled((tag.first.book() >= 2) ||
										((tag.first.book() == 1) && (tag.first.chapter() >= 2)));
	const CTOCEntry *pTOCEntry = g_pMainBibleDatabase->tocEntry(tag.first.book());
	m_pActionChapterForward->setEnabled((tag.first.book() < g_pMainBibleDatabase->bibleEntry().m_nNumBk) ||
										((tag.first.book() == g_pMainBibleDatabase->bibleEntry().m_nNumBk) && (tag.first.chapter() < (pTOCEntry ? pTOCEntry->m_nNumChp : 0))));
}

void CKJVCanOpener::on_browserHistoryChanged()
{
	if (m_pActionNavBackward) {
		m_pActionNavBackward->setToolTip(m_pBrowserWidget->browser()->historyTitle(-1));
		if (m_pBrowserWidget->browser()->isBackwardAvailable()) {
			m_pActionNavBackward->setStatusTip("Go to: " + m_pBrowserWidget->browser()->historyTitle(-1));
		} else {
			m_pActionNavBackward->setStatusTip("Go Backward in Navigation History");
		}
	}
	if (m_pActionNavForward) {
		m_pActionNavForward->setToolTip(m_pBrowserWidget->browser()->historyTitle(+1));
		if (m_pBrowserWidget->browser()->isForwardAvailable()) {
			m_pActionNavForward->setStatusTip("Go to: " + m_pBrowserWidget->browser()->historyTitle(+1));
		} else {
			m_pActionNavForward->setStatusTip("Go Forward in Navigation History");
		}
	}
	if (m_pActionNavClear) m_pActionNavClear->setEnabled(m_pBrowserWidget->browser()->isBackwardAvailable() ||
														m_pBrowserWidget->browser()->isForwardAvailable());
	if (m_pActionNavHome) m_pActionNavHome->setEnabled(m_pBrowserWidget->browser()->isBackwardAvailable() ||
														m_pBrowserWidget->browser()->isForwardAvailable());
}

void CKJVCanOpener::on_clearBrowserHistory()
{
	m_pBrowserWidget->browser()->clearHistory();
}

void CKJVCanOpener::on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase)
{
	Q_UNUSED(pSearchPhrase);

	TParsedPhrasesList lstPhrases;
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		pPhrase->SetContributingNumberOfMatches(0);
		pPhrase->SetIsDuplicate(false);
		pPhrase->ClearScopedPhraseTagSearchResults();
		if ((!pPhrase->isCompleteMatch()) || (pPhrase->GetNumberOfMatches() == 0)) {
			continue;		// Don't include phrases that had no matches of themselves
		}
		// Check for phrases with the same text and ignore them:
		bool bDuplicate = false;
		for (int ndx2 = 0; ndx2 < ndx; ++ndx2) {
			if ((*pPhrase) == (*m_lstSearchPhraseEditors.at(ndx2)->parsedPhrase())) {
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate) {
			pPhrase->SetIsDuplicate(true);
			continue;
		}

		// Do sorted insertion so the results with the greatest matches comes last.  That
		//		way it will have more things we'll discard rather than things we'll uselessly
		//		look at:
		int ndxInsert = 0;
		for ( ; ndxInsert < lstPhrases.size(); ++ndxInsert) {
			if (pPhrase->GetNumberOfMatches() < lstPhrases.at(ndxInsert)->GetNumberOfMatches()) break;
		}
		lstPhrases.insert(ndxInsert, pPhrase);
	}

	// ----------------------------

	int nVerses = 0;		// Results counts in Verses
	int nChapters = 0;		// Results counts in Chapters
	int nBooks = 0;			// Results counts in Books
	int nResults = 0;		// Total number of Results in Scope
	TPhraseTagList lstResults;

	CVerseListModel *pModel = m_pSearchResultWidget->treeView()->vlmodel();
	assert(pModel != NULL);

	lstResults = pModel->setParsedPhrases(ui->widgetSearchCriteria->searchScopeMode(), lstPhrases);		// Setting the phrases will build all of the results and set the verse list on the model

	nVerses = pModel->GetVerseIndexAndCount().second;
	nChapters = pModel->GetChapterIndexAndCount().second;
	nBooks = pModel->GetBookIndexAndCount().second;
	nResults = pModel->GetResultsCount();

	// ----------------------------

	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		m_lstSearchPhraseEditors.at(ndx)->phraseStatisticsChanged();
	}

	// ----------------------------

	QString strResults;

	strResults += QString("Found %1 Occurrence%2\n").arg(nResults).arg((nResults != 1) ? "s" : "");
	strResults += QString("    in %1 Verse%2 in %3 Chapter%4 in %5 Book%6")
							.arg(nVerses).arg((nVerses != 1) ? "s" : "")
							.arg(nChapters).arg((nChapters != 1) ? "s" : "")
							.arg(nBooks).arg((nBooks != 1) ? "s" : "");
	if (nResults > 0) {
		strResults += "\n";
		strResults += QString("    Not found at all in %1 Verse%2 of the Bible\n")
								.arg(g_pMainBibleDatabase->bibleEntry().m_nNumVrs - nVerses).arg(((g_pMainBibleDatabase->bibleEntry().m_nNumVrs - nVerses) != 1) ? "s" : "");
		strResults += QString("    Not found at all in %1 Chapter%2 of the Bible\n")
								.arg(g_pMainBibleDatabase->bibleEntry().m_nNumChp - nChapters).arg(((g_pMainBibleDatabase->bibleEntry().m_nNumChp - nChapters) != 1) ? "s" : "");
		strResults += QString("    Not found at all in %1 Book%2 of the Bible")
								.arg(g_pMainBibleDatabase->bibleEntry().m_nNumBk - nBooks).arg(((g_pMainBibleDatabase->bibleEntry().m_nNumBk - nBooks) != 1) ? "s" : "");
	}

	m_pSearchResultWidget->setSearchResultsCountText(strResults);

	m_bLastCalcSuccess = true;
	m_nLastSearchOccurrences = nResults;
	m_nLastSearchVerses = nVerses;
	m_nLastSearchChapters = nChapters;
	m_nLastSearchBooks = nBooks;
	ui->widgetSearchCriteria->enableCopySearchPhraseSummary(nResults > 0);

	m_pBrowserWidget->setHighlightTags(lstResults);

	emit changedSearchResults();
}

void CKJVCanOpener::on_SearchResultActivated(const QModelIndex &index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (!index.isValid()) return;

	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());
	if (!ndxRel.isSet()) return;

	m_pBrowserWidget->gotoIndex(TPhraseTag(ndxRel));
	m_pBrowserWidget->focusBrowser();
}

void CKJVCanOpener::on_PassageNavigatorTriggered()
{
	assert(m_pBibleDatabase.data() != NULL);

	if ((m_pBrowserWidget->browser()->hasFocus()) ||
		(m_bBrowserActive)) {
		m_pBrowserWidget->browser()->on_passageNavigator();
	} else if (((m_pSearchResultWidget->treeView()->hasFocus()) || (m_bSearchResultsActive)) &&
				(m_pSearchResultWidget->treeView()->selectionModel()->selectedRows().count()== 1)) {
		m_pSearchResultWidget->treeView()->on_passageNavigator();
	} else {
		CKJVPassageNavigatorDlg dlg(m_pBibleDatabase, this);

		if (dlg.exec() == QDialog::Accepted) {
			m_pBrowserWidget->gotoIndex(dlg.passage());
			m_pBrowserWidget->focusBrowser();
		}
	}
}

void CKJVCanOpener::on_viewDetails()
{
	if (((m_pBrowserWidget->browser()->hasFocus()) || (m_bBrowserActive)) &&
		 (m_pBrowserWidget->browser()->haveDetails())) {
		m_pBrowserWidget->browser()->showDetails();
	} else if (((m_pSearchResultWidget->treeView()->hasFocus()) || (m_bSearchResultsActive)) &&
				(m_pSearchResultWidget->treeView()->haveDetails())) {
		m_pSearchResultWidget->treeView()->showDetails();
	}
}

void CKJVCanOpener::setDetailsEnable()
{
	bool bDetailsEnable = false;

	if (((m_pBrowserWidget->browser()->hasFocus()) || (m_bBrowserActive)) &&
		 (m_pBrowserWidget->browser()->haveDetails())) {
		bDetailsEnable = true;
	} else if (((m_pSearchResultWidget->treeView()->hasFocus()) || (m_bSearchResultsActive)) &&
				(m_pSearchResultWidget->treeView()->haveDetails())) {
		bDetailsEnable = true;
	}

	emit canShowDetails(bDetailsEnable);
}

void CKJVCanOpener::on_HelpManual()
{
	QFileInfo fiHelpDoc(QApplication::applicationDirPath(), g_constrHelpDocFilename);
	if ((!fiHelpDoc.exists()) || (!QDesktopServices::openUrl(QUrl::fromLocalFile(fiHelpDoc.absoluteFilePath())))) {
		QMessageBox::warning(this, windowTitle(), QString("Unable to open the King James Pure Bible Search Users Manual.\n"
															"Verify that you have a PDF Viewer, such as Adobe Acrobat, installed.\n"
															"And check installation of King James Pure Bible Search User Manual at:\n\n"
															"%1").arg(QDir::toNativeSeparators(fiHelpDoc.absoluteFilePath())));
	}

//	QMessageBox::information(this, windowTitle(), "An online help manual is coming soon for the King James Pure Bible Search Application.\n\nKeep your eyes open for future updates.");
}

void CKJVCanOpener::on_HelpAbout()
{
	CKJVAboutDlg dlg(this);
	dlg.exec();
}

void CKJVCanOpener::on_QuickActivate()
{
	bool bServiced = false;

	for (int ndx=0; ndx<m_lstpQuickActivate.size(); ++ndx) {
		if (m_lstpQuickActivate.at(ndx) == static_cast<QAction*>(sender())) {
			switch (ndx) {
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					if ((ndx-1) < m_lstSearchPhraseEditors.size()) {
						m_lstSearchPhraseEditors.at(ndx-1)->focusEditor();
						ensureSearchPhraseVisible(m_lstSearchPhraseEditors.at(ndx-1));
					}
					break;
				case 9:
					m_pSearchResultWidget->treeView()->setFocus();
					break;
				case 0:
					m_pBrowserWidget->focusBrowser();
					break;
				default:
					assert(false);
					break;
			}
			bServiced = true;
			break;
		}
	}

	assert(bServiced);
}

void CKJVCanOpener::on_setBrowserFont()
{
	bool bUpdate = false;
	QFont fnt = QFontDialog::getFont(&bUpdate, CPersistentSettings::instance()->fontBrowser(), this, "Select Scripture Browser Font");
	if (bUpdate) CPersistentSettings::instance()->setFontBrowser(fnt);
}

void CKJVCanOpener::on_setSearchResultsFont()
{
	bool bUpdate = false;
	QFont fnt = QFontDialog::getFont(&bUpdate, CPersistentSettings::instance()->fontSearchResults(), this, "Select Search Results Font");
	if (bUpdate) CPersistentSettings::instance()->setFontSearchResults(fnt);
}

