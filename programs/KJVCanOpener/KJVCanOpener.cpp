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
#include "version.h"
#include "PersistentSettings.h"

#include <assert.h>

#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>
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
	m_bPhraseEditorActive(false),
	m_bSearchResultsActive(false),
	m_bBrowserActive(false),
	m_pSearchSpecWidget(NULL),
	m_pSplitter(NULL),
	m_pSearchResultWidget(NULL),
	m_pBrowserWidget(NULL),
	ui(new Ui::CKJVCanOpener)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui->setupUi(this);

	// -------------------- Setup the Three Panes:

	m_pSearchSpecWidget = new CKJVSearchSpec(m_pBibleDatabase, ui->centralWidget);
	m_pSearchSpecWidget->setObjectName(QString::fromUtf8("SearchSpecWidget"));

	ui->horizontalLayout->addWidget(m_pSearchSpecWidget);

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

	// --------------------

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
	QMenu *pFileMenu = ui->menuBar->addMenu(tr("&File"));

	pAction = pFileMenu->addAction(QIcon(":/res/file-new-icon2.png"), tr("&New Search"), this, SLOT(on_NewSearch()), QKeySequence(Qt::CTRL + Qt::Key_N));
	pAction->setStatusTip(tr("Clear All Search Phrases and Begin New Search"));
	pAction->setToolTip("Clear All Search Phrases and Begin New Search");
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/open-file-icon3.png"), tr("L&oad Search File..."), this, SLOT(on_OpenSearch()), QKeySequence(Qt::CTRL + Qt::Key_O));
	pAction->setStatusTip(tr("Load Search Phrases from a previously saved King James Search File"));
	pAction->setToolTip(tr("Load Search Phrases from a previously saved King James Search File"));
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/save-file-icon3.png"), tr("&Save Search File..."), this, SLOT(on_SaveSearch()), QKeySequence(Qt::CTRL + Qt::Key_S));
	pAction->setStatusTip(tr("Save current Search Phrases to a King James Search File"));
	pAction->setToolTip(tr("Save current Search Phrases to a King James Search File"));
	ui->mainToolBar->addAction(pAction);

	pFileMenu->addSeparator();

	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit"), this, SLOT(close()), QKeySequence(Qt::CTRL + Qt::Key_Q));
	pAction->setStatusTip(tr("Exit the King James Pure Bible Search Application"));
	pAction->setToolTip(tr("Exit Application"));
	pAction->setMenuRole(QAction::QuitRole);

	ui->mainToolBar->addSeparator();

	// --- Edit Menu
	connect(m_pBrowserWidget, SIGNAL(activatedScriptureText()), this, SLOT(on_activatedBrowser()));
	connect(m_pSearchResultWidget, SIGNAL(activatedSearchResults()), this, SLOT(on_activatedSearchResults()));
	connect(m_pSearchSpecWidget, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));

	// --- View Menu
	m_pViewMenu = ui->menuBar->addMenu(tr("&View"));

	QMenu *pViewToolbarsMenu = m_pViewMenu->addMenu(tr("&Toolbars"));
	pViewToolbarsMenu->addAction(ui->mainToolBar->toggleViewAction());
	ui->mainToolBar->toggleViewAction()->setStatusTip(tr("Show/Hide Main Tool Bar"));

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->addSeparator();

	m_pActionShowAsList = m_pViewMenu->addAction(tr("View as &List"), this, SLOT(on_viewAsList()));
	m_pActionShowAsList->setStatusTip(tr("Show Search Results as a List"));
	m_pActionShowAsList->setCheckable(true);
	m_pActionShowAsList->setChecked(nTreeMode == CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionShowAsList);

	m_pActionShowAsTreeBooks = m_pViewMenu->addAction(tr("View as Tree by &Book"), this, SLOT(on_viewAsTreeBooks()));
	m_pActionShowAsTreeBooks->setStatusTip(tr("Show Search Results in a Tree by Book"));
	m_pActionShowAsTreeBooks->setCheckable(true);
	m_pActionShowAsTreeBooks->setChecked(nTreeMode == CVerseListModel::VTME_TREE_BOOKS);
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionShowAsTreeBooks);

	m_pActionShowAsTreeChapters = m_pViewMenu->addAction(tr("View as Tree by Book/&Chapter"), this, SLOT(on_viewAsTreeChapters()));
	m_pActionShowAsTreeChapters->setStatusTip(tr("Show Search Results in a Tree by Book and Chapter"));
	m_pActionShowAsTreeChapters->setCheckable(true);
	m_pActionShowAsTreeChapters->setChecked(nTreeMode == CVerseListModel::VTME_TREE_CHAPTERS);
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionShowAsTreeChapters);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->addSeparator();

	m_pActionShowMissingLeafs = m_pViewMenu->addAction(tr("View &Missing Books/Chapters"), this, SLOT(on_viewShowMissingsLeafs()));
	m_pActionShowMissingLeafs->setStatusTip(tr("Show Missing Books and/or Chapters in the Tree (ones that had no matching Search Results)"));
	m_pActionShowMissingLeafs->setCheckable(true);
	m_pActionShowMissingLeafs->setChecked(bShowMissingLeafs);
	m_pActionShowMissingLeafs->setEnabled(nTreeMode != CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionShowMissingLeafs);

	m_pActionExpandAll = m_pViewMenu->addAction(tr("E&xpand All"), m_pSearchResultWidget, SIGNAL(expandAll()));
	m_pActionExpandAll->setStatusTip(tr("Expand all tree nodes in Search Results (Warning: May be slow if there are a lot of search results!)"));
	m_pActionExpandAll->setEnabled(false);
	connect(m_pSearchResultWidget, SIGNAL(canExpandAll(bool)), m_pActionExpandAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionExpandAll);

	m_pActionCollapseAll = m_pViewMenu->addAction(tr("Collap&se All"), m_pSearchResultWidget, SIGNAL(collapseAll()));
	m_pActionCollapseAll->setStatusTip(tr("Collapse all tree nodes in Search Results"));
	m_pActionCollapseAll->setEnabled(false);
	connect(m_pSearchResultWidget, SIGNAL(canCollapseAll(bool)), m_pActionCollapseAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionCollapseAll);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->addSeparator();

	m_pActionShowVerseHeading = m_pViewMenu->addAction(tr("View &References Only"), this, SLOT(on_viewVerseHeading()));
	m_pActionShowVerseHeading->setStatusTip(tr("Show Search Results Verse References Only"));
	m_pActionShowVerseHeading->setCheckable(true);
	m_pActionShowVerseHeading->setChecked(nDisplayMode == CVerseListModel::VDME_HEADING);
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionShowVerseHeading);

	m_pActionShowVerseRichText = m_pViewMenu->addAction(tr("View Verse &Preview"), this, SLOT(on_viewVerseRichText()));
	m_pActionShowVerseRichText->setStatusTip(tr("Show Search Results as Rich Text Verse Preview"));
	m_pActionShowVerseRichText->setCheckable(true);
	m_pActionShowVerseRichText->setChecked(nDisplayMode == CVerseListModel::VDME_RICHTEXT);
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionShowVerseRichText);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->addSeparator();

	m_pActionViewDetails = m_pViewMenu->addAction(tr("View &Details..."), this, SLOT(on_viewDetails()), QKeySequence(Qt::CTRL + Qt::Key_D));
	m_pActionViewDetails->setStatusTip(tr("View Passage Details"));
	m_pActionViewDetails->setEnabled(false);
	connect(this, SIGNAL(canShowDetails(bool)), m_pActionViewDetails, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionViewDetails);

	// --- Navigate Menu
	QMenu *pNavMenu = ui->menuBar->addMenu(tr("&Navigate"));

	pAction = pNavMenu->addAction(tr("Beginning of Bible"), m_pBrowserWidget, SLOT(on_Bible_Beginning()), QKeySequence(Qt::ALT + Qt::Key_Home));
	pAction->setStatusTip(tr("Goto the very Beginning of the Bible"));
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	pAction = pNavMenu->addAction(tr("Ending of Bible"), m_pBrowserWidget, SLOT(on_Bible_Ending()), QKeySequence(Qt::ALT + Qt::Key_End));
	pAction->setStatusTip(tr("Goto the very End of the Bible"));
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionBookBackward = pNavMenu->addAction(tr("Book Backward"), m_pBrowserWidget, SLOT(on_Book_Backward()), QKeySequence(Qt::CTRL + Qt::Key_PageUp));
	m_pActionBookBackward->setStatusTip(tr("Move Backward one Book"));
	connect(m_pActionBookBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionBookForward = pNavMenu->addAction(tr("Book Forward"), m_pBrowserWidget, SLOT(on_Book_Forward()), QKeySequence(Qt::CTRL + Qt::Key_PageDown));
	m_pActionBookForward->setStatusTip(tr("Move Forward one Book"));
	connect(m_pActionBookForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionChapterBackward = pNavMenu->addAction(tr("Chapter Backward"), m_pBrowserWidget, SLOT(on_ChapterBackward()), QKeySequence(Qt::ALT + Qt::Key_PageUp));
	m_pActionChapterBackward->setStatusTip(tr("Move Backward one Chapter"));
	connect(m_pActionChapterBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionChapterForward = pNavMenu->addAction(tr("Chapter Forward"), m_pBrowserWidget, SLOT(on_ChapterForward()), QKeySequence(Qt::ALT + Qt::Key_PageDown));
	m_pActionChapterForward->setStatusTip(tr("Move Forward one Chapter"));
	connect(m_pActionChapterForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	connect(m_pBrowserWidget, SIGNAL(on_gotoIndex(const TPhraseTag &)), this, SLOT(on_gotoIndex(const TPhraseTag &)));

	pNavMenu->addSeparator();

	m_pActionNavBackward = new QAction(QIcon(":/res/Nav3_Arrow_Left.png"), tr("History &Backward"), this);
	m_pActionNavBackward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	m_pActionNavBackward->setStatusTip(tr("Go Backward in Navigation History"));
	ui->mainToolBar->addAction(m_pActionNavBackward);
	connect(m_pBrowserWidget, SIGNAL(backwardAvailable(bool)), m_pActionNavBackward, SLOT(setEnabled(bool)));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget, SIGNAL(backward()));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionNavBackward->setEnabled(m_pBrowserWidget->isBackwardAvailable());
	pNavMenu->addAction(m_pActionNavBackward);

	m_pActionNavForward = new QAction(QIcon(":/res/Nav3_Arrow_Right.png"), tr("History &Forward"), this);
	m_pActionNavForward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));
	m_pActionNavForward->setStatusTip(tr("Go Forward in Navigation History"));
	ui->mainToolBar->addAction(m_pActionNavForward);
	connect(m_pBrowserWidget, SIGNAL(forwardAvailable(bool)), m_pActionNavForward, SLOT(setEnabled(bool)));
	connect(m_pActionNavForward, SIGNAL(triggered()), m_pBrowserWidget, SIGNAL(forward()));
	connect(m_pActionNavForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionNavForward->setEnabled(m_pBrowserWidget->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavForward);

	m_pActionNavHome = pNavMenu->addAction(QIcon(":/res/go_home.png"), tr("History &Home"), m_pBrowserWidget, SIGNAL(home()), QKeySequence(Qt::ALT + Qt::Key_Up));
	m_pActionNavHome->setStatusTip(tr("Jump to History Home Passage"));
	m_pActionNavHome->setEnabled(m_pBrowserWidget->isBackwardAvailable() ||
									m_pBrowserWidget->isForwardAvailable());

	m_pActionNavClear = new QAction(QIcon(":/res/edit_clear.png"), tr("&Clear Navigation History"), this);
	m_pActionNavClear->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));
	m_pActionNavClear->setStatusTip(tr("Clear All Passage Navigation History"));
	ui->mainToolBar->addAction(m_pActionNavClear);
	connect(m_pActionNavClear, SIGNAL(triggered()), this, SLOT(on_clearBrowserHistory()));
	m_pActionNavClear->setEnabled(m_pBrowserWidget->isBackwardAvailable() ||
									m_pBrowserWidget->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavClear);

	connect(m_pBrowserWidget, SIGNAL(historyChanged()), this, SLOT(on_browserHistoryChanged()));

//	m_pActionJump = new QAction(QIcon(":/res/go_jump2.png"), tr("Passage Navigator"), this);
	m_pActionJump = new QAction(QIcon(":/res/green_arrow.png"), tr("Passage &Navigator"), this);
	m_pActionJump->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	m_pActionJump->setStatusTip(tr("Display the Passage Navigator Widget"));
	ui->mainToolBar->addAction(m_pActionJump);
	connect(m_pActionJump, SIGNAL(triggered()), this, SLOT(on_PassageNavigatorTriggered()));

	pNavMenu->addSeparator();
	pNavMenu->addAction(m_pActionJump);

	// --- Settings Menu
	QMenu *pSettingsMenu = ui->menuBar->addMenu(tr("Se&ttings"));

	pAction = pSettingsMenu->addAction(tr("Scripture &Browser Font..."), this, SLOT(on_setFontScriptureBrowser()));
	pAction->setStatusTip(tr("Adjust the Scripture Browser Font"));
	pAction->setToolTip(tr("Adjust the Scripture Browser Font"));

	pAction = pSettingsMenu->addAction(tr("Search &Results Font..."), this, SLOT(on_setFontSearchResults()));
	pAction->setStatusTip(tr("Adjust the Search Results Font"));
	pAction->setToolTip(tr("Adjust the Search Results Font"));

	// --- Help Menu
	QMenu *pHelpMenu = ui->menuBar->addMenu(tr("&Help"));
	pAction = pHelpMenu->addAction(QIcon(":/res/help_book.png"), tr("&Help"), this, SLOT(on_HelpManual()), QKeySequence(Qt::SHIFT + Qt::Key_F1));
	pAction->setStatusTip(tr("Display the Users Manual"));

	m_pActionAbout = new QAction(QIcon(":/res/help_icon1.png"), tr("About..."), this);
	m_pActionAbout->setShortcut(QKeySequence(Qt::Key_F1));
	m_pActionAbout->setStatusTip(tr("About the King James Pure Bible Search"));
	m_pActionAbout->setToolTip(tr("About the King James Pure Bible Search..."));
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

	connect(m_pSearchSpecWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SLOT(on_closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(m_pSearchSpecWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(on_phraseChanged(CKJVSearchPhraseEdit *)));
	connect(m_pSearchSpecWidget, SIGNAL(copySearchPhraseSummary()), this, SLOT(on_copySearchPhraseSummary()));

	// -------------------- Search Spec:

	connect(m_pSearchSpecWidget, SIGNAL(changedSearchSpec(const CSearchCriteria &, const TParsedPhrasesList &)), this, SLOT(on_changedSearchSpec(const CSearchCriteria &, const TParsedPhrasesList &)));

	// -------------------- Search Results List View:

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedSearchResults(const QFont &)), m_pSearchResultWidget, SIGNAL(setFontSearchResults(const QFont &)));

	connect(m_pSearchResultWidget, SIGNAL(activated(const QModelIndex &)), this, SLOT(on_SearchResultActivated(const QModelIndex &)));
	connect(m_pSearchResultWidget, SIGNAL(gotoIndex(const TPhraseTag &)), m_pBrowserWidget, SLOT(gotoIndex(const TPhraseTag &)));
	connect(m_pSearchResultWidget, SIGNAL(setDetailsEnable()), this, SLOT(setDetailsEnable()));

	// -------------------- Scripture Browser:

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedScriptureBrowser(const QFont &)), m_pBrowserWidget, SLOT(setFontScriptureBrowser(const QFont &)));

	// -------------------- Persistent Settings:
	restorePersistentSettings();
}

CKJVCanOpener::~CKJVCanOpener()
{
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
	settings.beginGroup(constrSearchResultsViewGroup);
	settings.setValue(constrVerseDisplayModeKey, m_pSearchResultWidget->displayMode());
	settings.setValue(constrVerseTreeModeKey, m_pSearchResultWidget->treeMode());
	settings.setValue(constrViewMissingNodesKey, m_pSearchResultWidget->showMissingLeafs());
	settings.setValue(constrCurrentIndexKey, m_pSearchResultWidget->currentIndex().asAnchor());
	settings.setValue(constrHasFocusKey, m_pSearchResultWidget->hasFocusSearchResult());
	settings.setValue(constrFontKey, CPersistentSettings::instance()->fontSearchResults().toString());
	settings.endGroup();

	// Last Search:
	m_pSearchSpecWidget->writeKJVSearchFile(settings, constrLastSearchGroup);

	// Current Browser Reference:
	settings.beginGroup(constrBrowserViewGroup);
	TPhraseTag tag = m_pBrowserWidget->selection();
	settings.setValue(constrLastReferenceKey, tag.first.asAnchor());
	settings.setValue(constrLastSelectionSizeKey, tag.second);
	settings.setValue(constrHasFocusKey, m_pBrowserWidget->hasFocusBrowser());
	settings.setValue(constrFontKey, CPersistentSettings::instance()->fontScriptureBrowser().toString());
	settings.endGroup();

	// Browser Object (used for FindDialog, etc):
	m_pBrowserWidget->savePersistentSettings(constrBrowserViewGroup);
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
	settings.beginGroup(constrSearchResultsViewGroup);
	setDisplayMode(static_cast<CVerseListModel::VERSE_DISPLAY_MODE_ENUM>(settings.value(constrVerseDisplayModeKey, m_pSearchResultWidget->displayMode()).toUInt()));
	setTreeMode(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(settings.value(constrVerseTreeModeKey, m_pSearchResultWidget->treeMode()).toUInt()));
	setShowMissingLeafs(settings.value(constrViewMissingNodesKey, m_pSearchResultWidget->showMissingLeafs()).toBool());
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
	m_pSearchSpecWidget->readKJVSearchFile(settings, constrLastSearchGroup);

	// Current Browser Reference:
	//		Note: The actual browser reference has already been loaded in
	//			initialize().  However, this will lookup the reference and
	//			see if the user was displaying a search result and if so,
	//			will set the current index for the search result to that
	//			as a fallback for when there is no Last Current Index:
	bool bLastSet = false;
	if (ndxLastCurrentIndex.isSet()) bLastSet = m_pSearchResultWidget->setCurrentIndex(ndxLastCurrentIndex, false);
	settings.beginGroup(constrBrowserViewGroup);
	bool bFocusBrowser = settings.value(constrHasFocusKey, false).toBool();
	if (!bLastSet) {
		CRelIndex ndxLastBrowsed = CRelIndex(settings.value(constrLastReferenceKey, CRelIndex().asAnchor()).toString());
		if (ndxLastBrowsed.isSet()) m_pSearchResultWidget->setCurrentIndex(ndxLastBrowsed, false);
	}
	strFont = settings.value(constrFontKey).toString();
	if (!strFont.isEmpty()) {
		QFont aFont;
		aFont.fromString(strFont);
		CPersistentSettings::instance()->setFontScriptureBrowser(aFont);
	}
	settings.endGroup();

	// Browser Object (used for FindDialog, etc):
	m_pBrowserWidget->restorePersistentSettings(constrBrowserViewGroup);

	// If the Search Result was focused last time, focus it again, else if
	//	the browser was focus last time, focus it again.  Otherwise, leave
	//	the phrase editor focus:
	if ((bFocusSearchResults) && (m_pSearchResultWidget->haveResults())) {
		QTimer::singleShot(1, m_pSearchResultWidget, SLOT(setFocusSearchResult()));
	} else if (bFocusBrowser) {
		QTimer::singleShot(1, m_pBrowserWidget, SLOT(setFocusBrowser()));
	}
}

void CKJVCanOpener::closeEvent(QCloseEvent *event)
{
	if ((g_bUserPhrasesDirty) && haveUserDatabase()) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("Do you wish to save the search phrase list changes you've made to the user database?"),
																tr("Yes"), tr("No"), tr("Cancel"), 0, 2);
		if (nResult == 2) {
			event->ignore();
			return;
		}
		if (nResult == 0) {
			CBuildDatabase bdb(this);
			if (!bdb.BuildUserDatabase(m_strUserDatabase)) {
				QMessageBox::warning(this, windowTitle(), tr("Failed to save KJV User Database!\nCheck installation and settings!"));
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
			m_pActionShowVerseRichText->setChecked(true);
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
	m_pSearchSpecWidget->reset();
}

void CKJVCanOpener::on_OpenSearch()
{
	QString strFilePathName = QFileDialog::getOpenFileName(this, tr("Open KJV Search File"), QString(), tr("KJV Search Files (*.kjs)"), NULL, QFileDialog::ReadOnly);
	if (!strFilePathName.isEmpty())
		if (!openKJVSearchFile(strFilePathName))
			QMessageBox::warning(this, tr("KJV Search File Open Failed"), tr("Failed to open and read the specified KJV Search File!"));
}

void CKJVCanOpener::on_SaveSearch()
{
	QString strFilePathName = QFileDialog::getSaveFileName(this, tr("Save KJV Search File"), QString(), tr("KJV Search Files (*.kjs)"), NULL, 0);
	if (!strFilePathName.isEmpty())
		if (!saveKJVSearchFile(strFilePathName))
			QMessageBox::warning(this, tr("KJV Search File Save Failed"), tr("Failed to save the specified KJV Search File!"));
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
		QMessageBox::warning(this, tr("Opening King James Search File"), tr("Warning: The file you are opening was saved on "
									"an older version of King James Pure Bible Search.  Some manual editing may be necessary "
									"to configure any new search options added since that older version.\n\n"
									"To avoid this message when opening this file in the future, then resave your "
									"search phrases over top of this file, replacing this old version."));
	} else if (nFileVersion > KJS_FILE_VERSION) {
		QMessageBox::warning(this, tr("Opening King James Search File"), tr("Warning: The file you are opening was created on "
									"a newer version of King James Pure Bible Search.  It may contain settings for options not "
									"available on this version of King James Pure Bible Search.  If so, those options will be "
									"ignored."));
	}

	m_pSearchSpecWidget->readKJVSearchFile(kjsFile);

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

	m_pSearchSpecWidget->writeKJVSearchFile(kjsFile);

	kjsFile.sync();

	return (kjsFile.status() == QSettings::NoError);
}

// ------------------------------------------------------------------

void CKJVCanOpener::on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase)
{
	assert(pSearchPhrase != NULL);

	// If this search phrase's editor was currently active, remove it or else
	//		we'll crash later accessing data for a deleted object:
	if ((m_bPhraseEditorActive) && ((m_pActionSearchPhraseEditMenu != NULL) &&
									(m_pActionSearchPhraseEditMenu->menu() == pSearchPhrase->phraseEditor()->getEditMenu()))) {
		on_addSearchPhraseEditMenu(false);
	}
}

void CKJVCanOpener::on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase)
{
	Q_UNUSED(pSearchPhrase);

	// The former functionality of this is now handled by on_changedSearchSpec()
}

void CKJVCanOpener::on_copySearchPhraseSummary()
{
	QString strSummary;

	strSummary += m_pSearchSpecWidget->searchPhraseSummaryText();
	strSummary += m_pSearchResultWidget->searchResultsSummaryText();

	QMimeData *mime = new QMimeData();

	mime->setText(strSummary);
	mime->setHtml("<qt><pre>\n" + strSummary + "</pre></qt>\n");
	QApplication::clipboard()->setMimeData(mime);
}

void CKJVCanOpener::on_changedSearchSpec(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases)
{
	const TPhraseTagList &lstResults = m_pSearchResultWidget->setParsedPhrases(aSearchCriteria, phrases);		// Setting the phrases will build all of the results and set the verse list on the model
	m_pSearchSpecWidget->enableCopySearchPhraseSummary(m_pSearchResultWidget->haveResults());
	m_pBrowserWidget->setHighlightTags(lstResults);
}

// ------------------------------------------------------------------

void CKJVCanOpener::on_addPassageBrowserEditMenu(bool bAdd)
{
	m_bBrowserActive = bAdd;

	if (bAdd) {
		if (m_pActionPassageBrowserEditMenu == NULL) {
			m_pActionPassageBrowserEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), m_pBrowserWidget->getEditMenu());
			connect(m_pActionPassageBrowserEditMenu, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
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
			m_pActionSearchResultsEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), m_pSearchResultWidget->getEditMenu());
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
	m_pSearchSpecWidget->on_activatedPhraseEditor(NULL);		// Notify that we have no search phrase editor active
	on_addPassageBrowserEditMenu(true);
	on_addSearchResultsEditMenu(false);
	on_addSearchPhraseEditMenu(false);
	setDetailsEnable();
}

void CKJVCanOpener::on_activatedSearchResults()
{
	m_pSearchSpecWidget->on_activatedPhraseEditor(NULL);		// Notify that we have no search phrase editor active
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

// ------------------------------------------------------------------

void CKJVCanOpener::on_viewVerseHeading()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->currentIndex());

	if (m_pActionShowVerseHeading->isChecked()) {
		m_pActionShowVerseRichText->setChecked(false);
		m_pSearchResultWidget->setDisplayMode(CVerseListModel::VDME_HEADING);
	} else {
		if (m_pSearchResultWidget->displayMode() == CVerseListModel::VDME_HEADING) {
			m_pActionShowVerseHeading->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewVerseRichText()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->currentIndex());

	if (m_pActionShowVerseRichText->isChecked()) {
		m_pActionShowVerseHeading->setChecked(false);
		m_pSearchResultWidget->setDisplayMode(CVerseListModel::VDME_RICHTEXT);
	} else {
		if (m_pSearchResultWidget->displayMode() == CVerseListModel::VDME_RICHTEXT) {
			m_pActionShowVerseRichText->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsList()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->currentIndex());

	if (m_pActionShowAsList->isChecked()) {
		m_pActionShowAsTreeBooks->setChecked(false);
		m_pActionShowAsTreeChapters->setChecked(false);
		m_pSearchResultWidget->setTreeMode(CVerseListModel::VTME_LIST);
		m_pActionShowMissingLeafs->setEnabled(false);
	} else {
		if (m_pSearchResultWidget->treeMode() == CVerseListModel::VTME_LIST) {
			m_pActionShowAsList->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsTreeBooks()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->currentIndex());

	if (m_pActionShowAsTreeBooks->isChecked()) {
		m_pActionShowAsList->setChecked(false);
		m_pActionShowAsTreeChapters->setChecked(false);
		m_pSearchResultWidget->setTreeMode(CVerseListModel::VTME_TREE_BOOKS);
		m_pActionShowMissingLeafs->setEnabled(true);
	} else {
		if (m_pSearchResultWidget->treeMode() == CVerseListModel::VTME_TREE_BOOKS) {
			m_pActionShowAsTreeBooks->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsTreeChapters()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->currentIndex());

	if (m_pActionShowAsTreeChapters->isChecked()) {
		m_pActionShowAsList->setChecked(false);
		m_pActionShowAsTreeBooks->setChecked(false);
		m_pSearchResultWidget->setTreeMode(CVerseListModel::VTME_TREE_CHAPTERS);
		m_pActionShowMissingLeafs->setEnabled(true);
	} else {
		if (m_pSearchResultWidget->treeMode() == CVerseListModel::VTME_TREE_CHAPTERS) {
			m_pActionShowAsTreeChapters->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewShowMissingsLeafs()
{
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(m_pSearchResultWidget->currentIndex());

	if (m_pSearchResultWidget->treeMode() == CVerseListModel::VTME_LIST) {
		if (m_pSearchResultWidget->showMissingLeafs()) m_pSearchResultWidget->setShowMissingLeafs(false);
	} else {
		m_pSearchResultWidget->setShowMissingLeafs(m_pActionShowMissingLeafs->isChecked());
	}

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

// ------------------------------------------------------------------

void CKJVCanOpener::on_gotoIndex(const TPhraseTag &tag)
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
	m_pActionBookForward->setEnabled(tag.first.book() < m_pBibleDatabase->bibleEntry().m_nNumBk);
	m_pActionChapterBackward->setEnabled((tag.first.book() >= 2) ||
										((tag.first.book() == 1) && (tag.first.chapter() >= 2)));
	const CTOCEntry *pTOCEntry = m_pBibleDatabase->tocEntry(tag.first.book());
	m_pActionChapterForward->setEnabled((tag.first.book() < m_pBibleDatabase->bibleEntry().m_nNumBk) ||
										((tag.first.book() == m_pBibleDatabase->bibleEntry().m_nNumBk) && (tag.first.chapter() < (pTOCEntry ? pTOCEntry->m_nNumChp : 0))));
}

void CKJVCanOpener::on_browserHistoryChanged()
{
	if (m_pActionNavBackward) {
		m_pActionNavBackward->setToolTip(m_pBrowserWidget->historyTitle(-1));
		if (m_pBrowserWidget->isBackwardAvailable()) {
			m_pActionNavBackward->setStatusTip(tr("Go to: ") + m_pBrowserWidget->historyTitle(-1));
		} else {
			m_pActionNavBackward->setStatusTip(tr("Go Backward in Navigation History"));
		}
	}
	if (m_pActionNavForward) {
		m_pActionNavForward->setToolTip(m_pBrowserWidget->historyTitle(+1));
		if (m_pBrowserWidget->isForwardAvailable()) {
			m_pActionNavForward->setStatusTip(tr("Go to: ") + m_pBrowserWidget->historyTitle(+1));
		} else {
			m_pActionNavForward->setStatusTip(tr("Go Forward in Navigation History"));
		}
	}
	if (m_pActionNavClear) m_pActionNavClear->setEnabled(m_pBrowserWidget->isBackwardAvailable() ||
														m_pBrowserWidget->isForwardAvailable());
	if (m_pActionNavHome) m_pActionNavHome->setEnabled(m_pBrowserWidget->isBackwardAvailable() ||
														m_pBrowserWidget->isForwardAvailable());
}

void CKJVCanOpener::on_clearBrowserHistory()
{
	m_pBrowserWidget->clearHistory();
}

// ------------------------------------------------------------------

void CKJVCanOpener::on_SearchResultActivated(const QModelIndex &index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (!index.isValid()) return;

	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());
	if (!ndxRel.isSet()) return;

	m_pBrowserWidget->gotoIndex(TPhraseTag(ndxRel));
	m_pBrowserWidget->setFocusBrowser();
}

void CKJVCanOpener::on_PassageNavigatorTriggered()
{
	assert(m_pBibleDatabase.data() != NULL);

	if ((m_pBrowserWidget->hasFocusBrowser()) ||
		(m_bBrowserActive)) {
		m_pBrowserWidget->showPassageNavigator();
	} else if (((m_pSearchResultWidget->hasFocusSearchResult()) || (m_bSearchResultsActive)) &&
				(m_pSearchResultWidget->canShowPassageNavigator())) {
		m_pSearchResultWidget->showPassageNavigator();
	} else {
		CKJVPassageNavigatorDlg dlg(m_pBibleDatabase, this);

		if (dlg.exec() == QDialog::Accepted) {
			m_pBrowserWidget->gotoIndex(dlg.passage());
			m_pBrowserWidget->setFocusBrowser();
		}
	}
}

void CKJVCanOpener::on_viewDetails()
{
	if (((m_pBrowserWidget->hasFocusBrowser()) || (m_bBrowserActive)) &&
		 (m_pBrowserWidget->haveDetails())) {
		m_pBrowserWidget->showDetails();
	} else if (((m_pSearchResultWidget->hasFocusSearchResult()) || (m_bSearchResultsActive)) &&
				(m_pSearchResultWidget->haveDetails())) {
		m_pSearchResultWidget->showDetails();
	}
}

void CKJVCanOpener::setDetailsEnable()
{
	bool bDetailsEnable = false;

	if (((m_pBrowserWidget->hasFocusBrowser()) || (m_bBrowserActive)) &&
		 (m_pBrowserWidget->haveDetails())) {
		bDetailsEnable = true;
	} else if (((m_pSearchResultWidget->hasFocusSearchResult()) || (m_bSearchResultsActive)) &&
				(m_pSearchResultWidget->haveDetails())) {
		bDetailsEnable = true;
	}

	emit canShowDetails(bDetailsEnable);
}

// ------------------------------------------------------------------

void CKJVCanOpener::on_HelpManual()
{
	QFileInfo fiHelpDoc(QApplication::applicationDirPath(), g_constrHelpDocFilename);
	if ((!fiHelpDoc.exists()) || (!QDesktopServices::openUrl(QUrl::fromLocalFile(fiHelpDoc.absoluteFilePath())))) {
		QMessageBox::warning(this, windowTitle(), tr("Unable to open the King James Pure Bible Search Users Manual.\n"
													 "Verify that you have a PDF Viewer, such as Adobe Acrobat, installed.\n"
													 "And check installation of King James Pure Bible Search User Manual at:\n\n"
													 "%1").arg(QDir::toNativeSeparators(fiHelpDoc.absoluteFilePath())));
	}

//	QMessageBox::information(this, windowTitle(), tr("An online help manual is coming soon for the King James Pure Bible Search Application.\n\nKeep your eyes open for future updates."));
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
					m_pSearchSpecWidget->setFocusSearchPhrase(ndx-1);
					break;
				case 9:
					m_pSearchResultWidget->setFocusSearchResult();
					break;
				case 0:
					m_pBrowserWidget->setFocusBrowser();
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

void CKJVCanOpener::on_setFontScriptureBrowser()
{
	bool bUpdate = false;
	QFont fnt = QFontDialog::getFont(&bUpdate, CPersistentSettings::instance()->fontScriptureBrowser(), this, tr("Select Scripture Browser Font"));
	if (bUpdate) CPersistentSettings::instance()->setFontScriptureBrowser(fnt);
}

void CKJVCanOpener::on_setFontSearchResults()
{
	bool bUpdate = false;
	QFont fnt = QFontDialog::getFont(&bUpdate, CPersistentSettings::instance()->fontSearchResults(), this, tr("Select Search Results Font"));
	if (bUpdate) CPersistentSettings::instance()->setFontSearchResults(fnt);
}

