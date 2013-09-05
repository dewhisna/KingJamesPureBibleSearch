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

#include "main.h"
#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "KJVPassageNavigatorDlg.h"
#include "BuildDB.h"
#include "KJVAboutDlg.h"
#include "version.h"
#include "PersistentSettings.h"
#include "KJVConfiguration.h"
#include "UserNotesDatabase.h"
#include "Highlighter.h"
#include "KJVNoteEditDlg.h"
#include "KJVCrossRefEditDlg.h"
#include "SearchCompleter.h"

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
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>

// ============================================================================

#define KJS_FILE_VERSION 1				// Current KJS File Version (King James Search file)
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
	// MainApp Control:
	const QString constrMainAppControlGroup("MainApp/Controls");
	const QString constrInvertTextBrightnessKey("InvertTextBrightness");
	const QString constrTextBrightnessKey("TextBrightness");
	const QString constrAdjustDialogElementBrightnessKey("AdjustDialogElementBrightness");

	// Colors:
	const QString constrColorsGroup("Colors");
	const QString constrColorsHighlightersSubgroup("Highlighters");
	const QString constrWordsOfJesusColorKey("WordsOfJesusColor");
	const QString constrSearchResultsColorKey("SearchResultsColor");
	const QString constrCursorTrackerColorKey("CursorTrackerColor");
	const QString constrHighlighterNameKey("HighlighterName");

	// RestoreState:
	const QString constrMainAppRestoreStateGroup("RestoreState/MainApp");
	const QString constrSplitterRestoreStateGroup("RestoreState/Splitter");
	const QString constrGeometryKey("Geometry");
	const QString constrWindowStateKey("WindowState");

	// UserNotesDatabase:
	const QString constrUserNotesDatabaseGroup("UserNotesDatabase");
	const QString constrFilePathNameKey("FilePathName");
	const QString constrKeepBackupKey("KeepBackup");
	const QString constrBackupFilenamePostfixKey("BackupFilenamePostfix");

	// Search Phrases:
	const QString constrLastSearchGroup("LastSearch");
	const QString constrSearchPhrasesGroup("SearchPhrases");
	const QString constrSearchActivationDelayKey("SearchActivationDelay");
	const QString constrSearchPhraseCompleterFilterModeKey("SearchPhraseCompleterFilterMode");

	// Search Results View:
	const QString constrSearchResultsViewGroup("SearchResultsView");
	const QString constrResultsViewModeKey("ResultsViewMode");
	const QString constrVerseDisplayModeKey("VerseDisplayMode");
	const QString constrVerseTreeModeKey("VerseTreeMode");
	const QString constrViewMissingNodesKey("TreeShowsMissingNodes");
	const QString constrCurrentIndexKey("CurrentIndex");
	const QString constrCurrentHighlighterKey("CurrentHighlighter");
	const QString constrHasFocusKey("HasFocus");
	const QString constrFontKey("Font");

	// Browser View:
	const QString constrBrowserViewGroup("Browser");
	const QString constrLastReferenceKey("LastReference");
	const QString constrLastSelectionSizeKey("SelectionSize");
	//const QString constrHasFocusKey("HasFocus");
	//const QString constrFontKey("Font");
	const QString constrNavigationActivationDelayKey("NavigationActivationDelay");
	const QString constrPassageReferenceActivationDelayKey("PassageReferenceActivationDelay");

	// Copy Options:
	const QString constrCopyOptionsGroup("CopyOptions");
	const QString constrReferenceDelimiterModeKey("ReferenceDelimiterMode");
	const QString constrReferencesAbbreviatedBookNamesKey("ReferencesAbbreviatedBookNames");
	const QString constrReferencesInBoldKey("ReferencesInBold");
	const QString constrVerseNumberDelimiterModeKey("VerseNumberDelimiterMode");
	const QString constrVerseNumbersAbbreviatedBookNamesKey("VerseNumbersAbbreviatedBookNames");
	const QString constrVerseNumbersInBoldKey("VerseNumbersInBold");
	const QString constrAddQuotesAroundVerseKey("AddQuotesAroundVerse");
	const QString constrTransChangeAddWordModeKey("TransChangeAddWordMode");

	// UserNoteEditor Dialog:
	const QString constrUserNoteEditorGroup("UserNoteEditor");

	// CrossRefsEditor Dialog:
	const QString constrCrossRefsEditorGroup("CrossRefsEditor");
}

// ============================================================================

CKJVCanOpener::CKJVCanOpener(CBibleDatabasePtr pBibleDatabase, bool bLoadPersistentSettings, QWidget *parent) :
	QMainWindow(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_bDoingUpdate(false),
	// ----
	m_pActionPassageBrowserEditMenu(NULL),
	m_pActionSearchResultsEditMenu(NULL),
	m_pActionSearchPhraseEditMenu(NULL),
	// ----
	m_pViewMenu(NULL),
	m_pActionGroupViewMode(NULL),
	m_pActionGroupDisplayMode(NULL),
	m_pActionGroupTreeMode(NULL),
	m_pActionShowMissingLeafs(NULL),
	m_pActionExpandAll(NULL),
	m_pActionCollapseAll(NULL),
	m_pActionViewDetails(NULL),
	// ----
	m_pActionBookBackward(NULL),
	m_pActionBookForward(NULL),
	m_pActionChapterBackward(NULL),
	m_pActionChapterForward(NULL),
	m_pActionNavBackward(NULL),
	m_pActionNavForward(NULL),
	m_pActionNavHome(NULL),
	m_pActionNavClear(NULL),
	m_pActionJump(NULL),
	// ----
	m_pActionAbout(NULL),
	// ----
	m_bPhraseEditorActive(false),
	m_bSearchResultsActive(false),
	m_bBrowserActive(false),
	m_pSearchSpecWidget(NULL),
	m_pSplitter(NULL),
	m_pSearchResultWidget(NULL),
	m_pBrowserWidget(NULL),
	m_pUserNoteEditorDlg(NULL),
	m_pCrossRefsEditorDlg(NULL),
	ui(new Ui::CKJVCanOpener)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui->setupUi(this);

	QAction *pAction;

	// --------------------

	extern CMyApplication *g_pMyApplication;
	m_strAppStartupStyleSheet = g_pMyApplication->styleSheet();

	// Setup Default Font and TextBrightness:
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(setAdjustDialogElementBrightness(bool)));

	// -------------------- User Notes/Highlighter/References Toolbar:

	// Note: Must set this up before creating CKJVBrowser, or else our toolbar
	//			will be null when its constructor is building menus:
	new CHighlighterButtons(ui->usernotesToolBar);

	ui->usernotesToolBar->addSeparator();

	pAction = ui->usernotesToolBar->addAction(QIcon(":/res/App-edit-icon-128.png"), tr("Add/Edit/Remove Note..."));
	pAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
	pAction->setStatusTip(tr("Add/Edit/Remove Note to current verse or passage"));
	pAction->setToolTip(tr("Add/Edit/Remove Note to current verse or passage"));
	CKJVNoteEditDlg::setActionUserNoteEditor(pAction);

	ui->usernotesToolBar->addSeparator();

	pAction = ui->usernotesToolBar->addAction(QIcon(":/res/insert-cross-reference.png"), tr("Add/Edit/Remove Cross Reference..."));
	pAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
	pAction->setStatusTip(tr("Add/Edit/Remove Cross Reference to link this verse or passage with another"));
	pAction->setToolTip(tr("Add/Edit/Remove Cross Reference to link this verse or passage with another"));
	CKJVCrossRefEditDlg::setActionCrossRefsEditor(pAction);

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

	m_pBrowserWidget = new CKJVBrowser(m_pSearchResultWidget->vlmodel(), m_pBibleDatabase, m_pSplitter);
	m_pBrowserWidget->setObjectName(QString::fromUtf8("BrowserWidget"));
	QSizePolicy aSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	aSizePolicy.setHorizontalStretch(20);
	aSizePolicy.setVerticalStretch(0);
	aSizePolicy.setHeightForWidth(m_pBrowserWidget->sizePolicy().hasHeightForWidth());
	m_pBrowserWidget->setSizePolicy(aSizePolicy);
	m_pSplitter->addWidget(m_pBrowserWidget);

	ui->horizontalLayout->addWidget(m_pSplitter);

	// --------------------

#ifdef Q_WS_WIN
	setWindowIcon(QIcon(":/res/bible.ico"));
#else
	setWindowIcon(QIcon(":/res/bible_48.png"));
#endif

	// --------------------

// The following is supposed to be another workaround for QTBUG-13768
//	m_pSplitter->setStyleSheet("QSplitterHandle:hover {}  QSplitter::handle:hover { background-color: palette(highlight); }");
	m_pSplitter->handle(1)->setAttribute(Qt::WA_Hover);		// Work-Around QTBUG-13768
	setStyleSheet("QSplitter::handle:hover { background-color: palette(highlight); }");


	CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode = m_pSearchResultWidget->viewMode();
	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode = m_pSearchResultWidget->displayMode();
	CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode = m_pSearchResultWidget->treeMode();
	bool bShowMissingLeafs = m_pSearchResultWidget->showMissingLeafs();

	// --------------------

	// --- File Menu
	QMenu *pFileMenu = ui->menuBar->addMenu(tr("&File"));

	pAction = pFileMenu->addAction(QIcon(":/res/file-new-icon2.png"), tr("&New Search"), this, SLOT(en_NewSearch()), QKeySequence(Qt::CTRL + Qt::Key_N));
	pAction->setStatusTip(tr("Clear All Search Phrases and Begin New Search"));
	pAction->setToolTip("Clear All Search Phrases and Begin New Search");
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/open-file-icon3.png"), tr("L&oad Search File..."), this, SLOT(en_OpenSearch()), QKeySequence(Qt::CTRL + Qt::Key_O));
	pAction->setStatusTip(tr("Load Search Phrases from a previously saved King James Search File"));
	pAction->setToolTip(tr("Load Search Phrases from a previously saved King James Search File"));
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/save-file-icon3.png"), tr("&Save Search File..."), this, SLOT(en_SaveSearch()), QKeySequence(Qt::CTRL + Qt::Key_S));
	pAction->setStatusTip(tr("Save current Search Phrases to a King James Search File"));
	pAction->setToolTip(tr("Save current Search Phrases to a King James Search File"));
	ui->mainToolBar->addAction(pAction);

	pFileMenu->addSeparator();

	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit"), this, SLOT(close()), QKeySequence(Qt::CTRL + Qt::Key_Q));
	pAction->setStatusTip(tr("Exit the King James Pure Bible Search Application"));
	pAction->setToolTip(tr("Exit Application"));
	pAction->setMenuRole(QAction::QuitRole);

	// --- Edit Menu
	connect(m_pBrowserWidget, SIGNAL(activatedScriptureText()), this, SLOT(en_activatedBrowser()));
	connect(m_pSearchResultWidget, SIGNAL(activatedSearchResults()), this, SLOT(en_activatedSearchResults()));
	connect(m_pSearchSpecWidget, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)), this, SLOT(en_activatedPhraseEditor(const CPhraseLineEdit *)));

	// --- View Menu
	m_pViewMenu = ui->menuBar->addMenu(tr("&View"));

	QMenu *pViewToolbarsMenu = m_pViewMenu->addMenu(tr("&Toolbars"));
	pViewToolbarsMenu->addAction(ui->mainToolBar->toggleViewAction());
	ui->mainToolBar->toggleViewAction()->setStatusTip(tr("Show/Hide Main Tool Bar"));
	pViewToolbarsMenu->addAction(ui->browserNavigationToolBar->toggleViewAction());
	ui->browserNavigationToolBar->toggleViewAction()->setStatusTip(tr("Show/Hide the Scripture Browser Navigation Tool Bar"));
	pViewToolbarsMenu->addAction(ui->usernotesToolBar->toggleViewAction());
	ui->usernotesToolBar->toggleViewAction()->setStatusTip(tr("Show/Hide Highlighter/Notes/References Tool Bar"));

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());

	m_pActionGroupViewMode = new QActionGroup(this);
	m_pActionGroupViewMode->setExclusive(true);

	pAction = m_pActionGroupViewMode->addAction(tr("View S&earch Results"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_SEARCH_RESULTS);
	pAction->setStatusTip(tr("View Search Results from Search Phrases"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_SEARCH_RESULTS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupViewMode->addAction(tr("View &Highlighters"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_HIGHLIGHTERS);
	pAction->setStatusTip(tr("View Highlighted Passages"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_HIGHLIGHTERS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupViewMode->addAction(tr("View &Notes"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_USERNOTES);
	pAction->setStatusTip(tr("View All Notes"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_USERNOTES);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupViewMode->addAction(tr("View Cross Re&ferences"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_CROSSREFS);
	pAction->setStatusTip(tr("View Cross References"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_CROSSREFS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());

	connect(m_pActionGroupViewMode, SIGNAL(triggered(QAction*)), this, SLOT(en_viewModeChange(QAction*)));

	m_pActionGroupTreeMode = new QActionGroup(this);
	m_pActionGroupTreeMode->setExclusive(true);

	pAction = m_pActionGroupTreeMode->addAction(tr("View as &List"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VTME_LIST);
	pAction->setStatusTip(tr("Show Search Results as a List"));
	pAction->setCheckable(true);
	pAction->setChecked(nTreeMode == CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupTreeMode->addAction(tr("View as Tree by &Book"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VTME_TREE_BOOKS);
	pAction->setStatusTip(tr("Show Search Results in a Tree by Book"));
	pAction->setCheckable(true);
	pAction->setChecked(nTreeMode == CVerseListModel::VTME_TREE_BOOKS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupTreeMode->addAction(tr("View as Tree by Book/&Chapter"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VTME_TREE_CHAPTERS);
	pAction->setStatusTip(tr("Show Search Results in a Tree by Book and Chapter"));
	pAction->setCheckable(true);
	pAction->setChecked(nTreeMode == CVerseListModel::VTME_TREE_CHAPTERS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	connect(m_pActionGroupTreeMode, SIGNAL(triggered(QAction*)), this, SLOT(en_treeModeChange(QAction*)));

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());

	m_pActionShowMissingLeafs = m_pViewMenu->addAction(tr("View &Missing Books/Chapters"), this, SLOT(en_viewShowMissingsLeafs()));
	m_pActionShowMissingLeafs->setStatusTip(tr("Show Missing Books and/or Chapters in the Tree (ones that had no matching Search Results)"));
	m_pActionShowMissingLeafs->setCheckable(true);
	m_pActionShowMissingLeafs->setChecked(bShowMissingLeafs);
	m_pActionShowMissingLeafs->setEnabled(nTreeMode != CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), m_pActionShowMissingLeafs);

	m_pActionExpandAll = m_pViewMenu->addAction(tr("E&xpand All"), m_pSearchResultWidget, SIGNAL(expandAll()));
	m_pActionExpandAll->setStatusTip(tr("Expand all tree nodes in Search Results (Warning: May be slow if there are a lot of search results!)"));
	m_pActionExpandAll->setEnabled(false);
	connect(m_pSearchResultWidget, SIGNAL(canExpandAll(bool)), m_pActionExpandAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), m_pActionExpandAll);

	m_pActionCollapseAll = m_pViewMenu->addAction(tr("Collap&se All"), m_pSearchResultWidget, SIGNAL(collapseAll()));
	m_pActionCollapseAll->setStatusTip(tr("Collapse all tree nodes in Search Results"));
	m_pActionCollapseAll->setEnabled(false);
	connect(m_pSearchResultWidget, SIGNAL(canCollapseAll(bool)), m_pActionCollapseAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), m_pActionCollapseAll);

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());

	m_pActionGroupDisplayMode = new QActionGroup(this);
	m_pActionGroupDisplayMode->setExclusive(true);

	pAction = m_pActionGroupDisplayMode->addAction(tr("View &References Only"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VDME_HEADING);
	pAction->setStatusTip(tr("Show Search Results Verse References Only"));
	pAction->setCheckable(true);
	pAction->setChecked(nDisplayMode == CVerseListModel::VDME_HEADING);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupDisplayMode->addAction(tr("View Verse &Preview"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VDME_RICHTEXT);
	pAction->setStatusTip(tr("Show Search Results as Rich Text Verse Preview"));
	pAction->setCheckable(true);
	pAction->setChecked(nDisplayMode == CVerseListModel::VDME_RICHTEXT);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	connect(m_pActionGroupDisplayMode, SIGNAL(triggered(QAction*)), this, SLOT(en_displayModeChange(QAction*)));

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->addSeparator();			// Put details at the end

	m_pActionViewDetails = m_pViewMenu->addAction(QIcon(":/res/Windows-View-Detail-icon-48.png"), tr("View &Details..."), this, SLOT(en_viewDetails()), QKeySequence(Qt::CTRL + Qt::Key_D));
	m_pActionViewDetails->setStatusTip(tr("View Passage Details"));
	m_pActionViewDetails->setEnabled(false);
	connect(this, SIGNAL(canShowDetails(bool)), m_pActionViewDetails, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionViewDetails);

	// --- Navigate Menu
	QMenu *pNavMenu = ui->menuBar->addMenu(tr("&Navigate"));

	pAction = pNavMenu->addAction(tr("Beginning of Bible"), m_pBrowserWidget, SLOT(en_Bible_Beginning()), QKeySequence(Qt::ALT + Qt::Key_Home));
	pAction->setStatusTip(tr("Goto the very Beginning of the Bible"));
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	pAction = pNavMenu->addAction(tr("Ending of Bible"), m_pBrowserWidget, SLOT(en_Bible_Ending()), QKeySequence(Qt::ALT + Qt::Key_End));
	pAction->setStatusTip(tr("Goto the very End of the Bible"));
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionBookBackward = pNavMenu->addAction(tr("Book Backward"), m_pBrowserWidget, SLOT(en_Book_Backward()), QKeySequence(Qt::CTRL + Qt::Key_PageUp));
	m_pActionBookBackward->setStatusTip(tr("Move Backward one Book"));
	connect(m_pActionBookBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionBookForward = pNavMenu->addAction(tr("Book Forward"), m_pBrowserWidget, SLOT(en_Book_Forward()), QKeySequence(Qt::CTRL + Qt::Key_PageDown));
	m_pActionBookForward->setStatusTip(tr("Move Forward one Book"));
	connect(m_pActionBookForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionChapterBackward = pNavMenu->addAction(tr("Chapter Backward"), m_pBrowserWidget, SLOT(en_ChapterBackward()), QKeySequence(Qt::ALT + Qt::Key_PageUp));
	m_pActionChapterBackward->setStatusTip(tr("Move Backward one Chapter"));
	connect(m_pActionChapterBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionChapterForward = pNavMenu->addAction(tr("Chapter Forward"), m_pBrowserWidget, SLOT(en_ChapterForward()), QKeySequence(Qt::ALT + Qt::Key_PageDown));
	m_pActionChapterForward->setStatusTip(tr("Move Forward one Chapter"));
	connect(m_pActionChapterForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	connect(m_pBrowserWidget, SIGNAL(en_gotoIndex(const TPhraseTag &)), this, SLOT(en_gotoIndex(const TPhraseTag &)));

	pNavMenu->addSeparator();

	m_pActionNavBackward = new QAction(QIcon(":/res/Nav3_Arrow_Left.png"), tr("History &Backward"), this);
	m_pActionNavBackward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	m_pActionNavBackward->setStatusTip(tr("Go Backward in Navigation History"));
	ui->browserNavigationToolBar->addAction(m_pActionNavBackward);
	connect(m_pBrowserWidget, SIGNAL(backwardAvailable(bool)), m_pActionNavBackward, SLOT(setEnabled(bool)));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget, SIGNAL(backward()));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionNavBackward->setEnabled(m_pBrowserWidget->isBackwardAvailable());
	pNavMenu->addAction(m_pActionNavBackward);

	m_pActionNavForward = new QAction(QIcon(":/res/Nav3_Arrow_Right.png"), tr("History &Forward"), this);
	m_pActionNavForward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));
	m_pActionNavForward->setStatusTip(tr("Go Forward in Navigation History"));
	ui->browserNavigationToolBar->addAction(m_pActionNavForward);
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
	ui->browserNavigationToolBar->addAction(m_pActionNavClear);
	connect(m_pActionNavClear, SIGNAL(triggered()), this, SLOT(en_clearBrowserHistory()));
	m_pActionNavClear->setEnabled(m_pBrowserWidget->isBackwardAvailable() ||
									m_pBrowserWidget->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavClear);

	connect(m_pBrowserWidget, SIGNAL(historyChanged()), this, SLOT(en_browserHistoryChanged()));

//	m_pActionJump = new QAction(QIcon(":/res/go_jump2.png"), tr("Passage Navigator"), this);
	m_pActionJump = new QAction(QIcon(":/res/green_arrow.png"), tr("Passage &Navigator"), this);
	m_pActionJump->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	m_pActionJump->setStatusTip(tr("Display the Passage Navigator Widget"));
	ui->browserNavigationToolBar->addAction(m_pActionJump);
	connect(m_pActionJump, SIGNAL(triggered()), this, SLOT(en_PassageNavigatorTriggered()));

	pNavMenu->addSeparator();
	pNavMenu->addAction(m_pActionJump);

	ui->browserNavigationToolBar->addSeparator();
	ui->browserNavigationToolBar->addAction(m_pActionViewDetails);

	// --- Settings Menu
	QMenu *pSettingsMenu = ui->menuBar->addMenu(tr("Se&ttings"));

	pAction = pSettingsMenu->addAction(QIcon(":/res/Settings-icon2-128.png"), tr("Configure..."), this, SLOT(en_Configure()));
	pAction->setStatusTip(tr("Configure the King James Pure Bible Search Application"));
	pAction->setToolTip(tr("Configure King James Pure Bible Search"));
	pAction->setMenuRole(QAction::PreferencesRole);

	// --- Help Menu
	QMenu *pHelpMenu = ui->menuBar->addMenu(tr("&Help"));
	pAction = pHelpMenu->addAction(QIcon(":/res/help_book.png"), tr("&Help"), this, SLOT(en_HelpManual()), QKeySequence(Qt::SHIFT + Qt::Key_F1));
	pAction->setStatusTip(tr("Display the Users Manual"));

	m_pActionAbout = new QAction(QIcon(":/res/help_icon1.png"), tr("About..."), this);
	m_pActionAbout->setShortcut(QKeySequence(Qt::Key_F1));
	m_pActionAbout->setStatusTip(tr("About the King James Pure Bible Search"));
	m_pActionAbout->setToolTip(tr("About the King James Pure Bible Search..."));
	m_pActionAbout->setMenuRole(QAction::AboutRole);
	connect(m_pActionAbout, SIGNAL(triggered()), this, SLOT(en_HelpAbout()));
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addAction(m_pActionAbout);
	pHelpMenu->addAction(m_pActionAbout);

	// -------------------- Quick Activate:

	for (int ndx=0; ndx<NUM_QUICK_ACTIONS; ++ndx) {
		m_lstpQuickActivate.append(new QAction(this));
		m_lstpQuickActivate.at(ndx)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0 + ndx));
		connect(m_lstpQuickActivate.at(ndx), SIGNAL(triggered()), this, SLOT(en_QuickActivate()));
	}
	addActions(m_lstpQuickActivate);

	// -------------------- Search Phrase Widgets:

	connect(m_pSearchSpecWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SLOT(en_closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(m_pSearchSpecWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(en_phraseChanged(CKJVSearchPhraseEdit *)));
	connect(m_pSearchSpecWidget, SIGNAL(copySearchPhraseSummary()), this, SLOT(en_copySearchPhraseSummary()));

	// -------------------- Search Spec:

	connect(m_pSearchSpecWidget, SIGNAL(changedSearchSpec(const CSearchCriteria &, const TParsedPhrasesList &)), this, SLOT(en_changedSearchSpec(const CSearchCriteria &, const TParsedPhrasesList &)));

	// -------------------- Search Results List View:

	connect(m_pSearchResultWidget, SIGNAL(activated(const QModelIndex &)), this, SLOT(en_SearchResultActivated(const QModelIndex &)));
	connect(m_pSearchResultWidget, SIGNAL(gotoIndex(const TPhraseTag &)), m_pBrowserWidget, SLOT(gotoIndex(const TPhraseTag &)));
	connect(m_pSearchResultWidget, SIGNAL(setDetailsEnable()), this, SLOT(setDetailsEnable()));

	// -------------------- Scripture Browser:


	// -------------------- UserNoteEditor Dialog:
	m_pUserNoteEditorDlg = new CKJVNoteEditDlg(m_pBibleDatabase, g_pUserNotesDatabase, this);
	m_pUserNoteEditorDlg->setModal(true);
	connect(CKJVNoteEditDlg::actionUserNoteEditor(), SIGNAL(triggered()), this, SLOT(en_userNoteEditorTriggered()));


	// -------------------- CrossRefsEditor Dialog:
	m_pCrossRefsEditorDlg = new CKJVCrossRefEditDlg(m_pBibleDatabase, g_pUserNotesDatabase, this);
	m_pCrossRefsEditorDlg->setModal(true);
	connect(CKJVCrossRefEditDlg::actionCrossRefsEditor(), SIGNAL(triggered()), this, SLOT(en_crossRefsEditorTriggered()));


	// -------------------- Persistent Settings:
	if (bLoadPersistentSettings) restorePersistentSettings();
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
	tag.relIndex() = CRelIndex(settings.value(constrLastReferenceKey, CRelIndex(1,1,0,0).asAnchor()).toString());	// Default for unset key
	if (!tag.relIndex().isSet()) tag.relIndex() = CRelIndex(1,1,0,0);		// Default for zero key
	tag.count() = settings.value(constrLastSelectionSizeKey, 0).toUInt();
	settings.endGroup();

	m_pBrowserWidget->gotoIndex(tag);
}

void CKJVCanOpener::savePersistentSettings()
{
	QSettings &settings(CPersistentSettings::instance()->settings());

	// Main App and Toolbars RestoreState:
	settings.beginGroup(constrMainAppRestoreStateGroup);
	settings.setValue(constrGeometryKey, saveGeometry());
	settings.setValue(constrWindowStateKey, saveState(KJVAPP_REGISTRY_VERSION));
	settings.endGroup();

	// Main App General Settings:
	settings.beginGroup(constrMainAppControlGroup);
	settings.setValue(constrInvertTextBrightnessKey, CPersistentSettings::instance()->invertTextBrightness());
	settings.setValue(constrTextBrightnessKey, CPersistentSettings::instance()->textBrightness());
	settings.setValue(constrAdjustDialogElementBrightnessKey, CPersistentSettings::instance()->adjustDialogElementBrightness());
	settings.endGroup();

	// Colors:
	settings.beginGroup(constrColorsGroup);
	settings.setValue(constrWordsOfJesusColorKey, CPersistentSettings::instance()->colorWordsOfJesus().name());
	settings.setValue(constrSearchResultsColorKey, CPersistentSettings::instance()->colorSearchResults().name());
	settings.setValue(constrCursorTrackerColorKey, CPersistentSettings::instance()->colorCursorFollow().name());
	settings.endGroup();

	// Splitter:
	settings.beginGroup(constrSplitterRestoreStateGroup);
	settings.setValue(constrWindowStateKey, m_pSplitter->saveState());
	settings.endGroup();

	// User Notes Database:
	assert(g_pUserNotesDatabase != NULL);
	settings.beginGroup(constrUserNotesDatabaseGroup);
	settings.setValue(constrFilePathNameKey, g_pUserNotesDatabase->filePathName());
	settings.setValue(constrKeepBackupKey, g_pUserNotesDatabase->keepBackup());
	settings.setValue(constrBackupFilenamePostfixKey, g_pUserNotesDatabase->backupFilenamePostfix());
	settings.endGroup();

	m_pUserNoteEditorDlg->writeSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrUserNoteEditorGroup));
	m_pCrossRefsEditorDlg->writeSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrCrossRefsEditorGroup));

	// Highlighter Tool Bar:
	settings.beginWriteArray(groupCombine(constrColorsGroup, constrColorsHighlightersSubgroup));
	settings.remove("");
	for (int ndxColor = 0; ndxColor < CHighlighterButtons::instance()->count(); ++ndxColor) {
		settings.setArrayIndex(ndxColor);
		settings.setValue(constrHighlighterNameKey, CHighlighterButtons::instance()->highlighter(ndxColor));
	}
	settings.endArray();

	// Search Results mode:
	settings.beginGroup(constrSearchResultsViewGroup);
	settings.setValue(constrResultsViewModeKey, m_pSearchResultWidget->viewMode());
	settings.setValue(constrVerseDisplayModeKey, m_pSearchResultWidget->displayMode());
	settings.setValue(constrVerseTreeModeKey, m_pSearchResultWidget->treeMode());
	settings.setValue(constrViewMissingNodesKey, m_pSearchResultWidget->showMissingLeafs());
	settings.setValue(constrCurrentIndexKey, m_pSearchResultWidget->currentVerseIndex().relIndex().asAnchor());
	settings.setValue(constrCurrentHighlighterKey, ((m_pSearchResultWidget->currentVerseIndex().resultsType() != VLMRTE_HIGHLIGHTERS) ||
													(m_pSearchResultWidget->currentVerseIndex().specialIndex() == -1)) ? QString() :
													m_pSearchResultWidget->vlmodel()->results(VLMRTE_HIGHLIGHTERS, m_pSearchResultWidget->currentVerseIndex().specialIndex()).resultsName());
	settings.setValue(constrHasFocusKey, m_pSearchResultWidget->hasFocusSearchResult());
	settings.setValue(constrFontKey, CPersistentSettings::instance()->fontSearchResults().toString());
	settings.endGroup();

	// Search Phrases Settings:
	settings.beginGroup(constrSearchPhrasesGroup);
	settings.setValue(constrSearchActivationDelayKey, CPersistentSettings::instance()->searchActivationDelay());
	settings.setValue(constrSearchPhraseCompleterFilterModeKey, CPersistentSettings::instance()->searchPhraseCompleterFilterMode());
	settings.endGroup();

	// Last Search:
	m_pSearchSpecWidget->writeKJVSearchFile(settings, constrLastSearchGroup);

	// Current Browser Reference and Browser Settings:
	settings.beginGroup(constrBrowserViewGroup);
	TPhraseTag tag = m_pBrowserWidget->selection();
	settings.setValue(constrLastReferenceKey, tag.relIndex().asAnchor());
	settings.setValue(constrLastSelectionSizeKey, tag.count());
	settings.setValue(constrHasFocusKey, m_pBrowserWidget->hasFocusBrowser());
	settings.setValue(constrFontKey, CPersistentSettings::instance()->fontScriptureBrowser().toString());
	settings.setValue(constrNavigationActivationDelayKey, CPersistentSettings::instance()->navigationActivationDelay());
	settings.setValue(constrPassageReferenceActivationDelayKey, CPersistentSettings::instance()->passageReferenceActivationDelay());
	settings.endGroup();

	// Browser Object (used for Subwindows: FindDialog, etc):
	m_pBrowserWidget->savePersistentSettings(constrBrowserViewGroup);

	// Copy Options:
	settings.beginGroup(constrCopyOptionsGroup);
	settings.setValue(constrReferenceDelimiterModeKey, CPersistentSettings::instance()->referenceDelimiterMode());
	settings.setValue(constrReferencesAbbreviatedBookNamesKey, CPersistentSettings::instance()->referencesUseAbbreviatedBookNames());
	settings.setValue(constrReferencesInBoldKey, CPersistentSettings::instance()->referencesInBold());
	settings.setValue(constrVerseNumberDelimiterModeKey, CPersistentSettings::instance()->verseNumberDelimiterMode());
	settings.setValue(constrVerseNumbersAbbreviatedBookNamesKey, CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames());
	settings.setValue(constrVerseNumbersInBoldKey, CPersistentSettings::instance()->verseNumbersInBold());
	settings.setValue(constrAddQuotesAroundVerseKey, CPersistentSettings::instance()->addQuotesAroundVerse());
	settings.setValue(constrTransChangeAddWordModeKey, CPersistentSettings::instance()->transChangeAddWordMode());
	settings.endGroup();
}

void CKJVCanOpener::restorePersistentSettings()
{
	QSettings &settings(CPersistentSettings::instance()->settings());
	QString strFont;

	// Main App and Toolbars RestoreState:
	settings.beginGroup(constrMainAppRestoreStateGroup);
	restoreGeometry(settings.value(constrGeometryKey).toByteArray());
	restoreState(settings.value(constrWindowStateKey).toByteArray(), KJVAPP_REGISTRY_VERSION);
	settings.endGroup();

	// Main App General Settings:
	settings.beginGroup(constrMainAppControlGroup);
	CPersistentSettings::instance()->setInvertTextBrightness(settings.value(constrInvertTextBrightnessKey, CPersistentSettings::instance()->invertTextBrightness()).toBool());
	CPersistentSettings::instance()->setTextBrightness(settings.value(constrTextBrightnessKey, CPersistentSettings::instance()->textBrightness()).toInt());
	CPersistentSettings::instance()->setAdjustDialogElementBrightness(settings.value(constrAdjustDialogElementBrightnessKey, CPersistentSettings::instance()->adjustDialogElementBrightness()).toBool());
	settings.endGroup();

	// Colors:
	settings.beginGroup(constrColorsGroup);
	QColor clrTemp;
	clrTemp.setNamedColor(settings.value(constrWordsOfJesusColorKey, CPersistentSettings::instance()->colorWordsOfJesus().name()).toString());
	CPersistentSettings::instance()->setColorWordsOfJesus(clrTemp);
	clrTemp.setNamedColor(settings.value(constrSearchResultsColorKey, CPersistentSettings::instance()->colorSearchResults().name()).toString());
	CPersistentSettings::instance()->setColorSearchResults(clrTemp);
	clrTemp.setNamedColor(settings.value(constrCursorTrackerColorKey, CPersistentSettings::instance()->colorCursorFollow().name()).toString());
	CPersistentSettings::instance()->setColorCursorFollow(clrTemp);
	settings.endGroup();

	// Splitter:
	settings.beginGroup(constrSplitterRestoreStateGroup);
	m_pSplitter->restoreState(settings.value(constrWindowStateKey).toByteArray());
	settings.endGroup();

	// User Notes Database:
	assert(g_pUserNotesDatabase != NULL);
	settings.beginGroup(constrUserNotesDatabaseGroup);
	g_pUserNotesDatabase->setFilePathName(settings.value(constrFilePathNameKey, QString()).toString());
	g_pUserNotesDatabase->setKeepBackup(settings.value(constrKeepBackupKey, g_pUserNotesDatabase->keepBackup()).toBool());
	g_pUserNotesDatabase->setBackupFilenamePostfix(settings.value(constrBackupFilenamePostfixKey, g_pUserNotesDatabase->backupFilenamePostfix()).toString());
	settings.endGroup();

	m_pUserNoteEditorDlg->readSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrUserNoteEditorGroup));
	m_pCrossRefsEditorDlg->readSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrCrossRefsEditorGroup));

	if (!g_pUserNotesDatabase->filePathName().isEmpty()) {
		if (!g_pUserNotesDatabase->load()) {
			QMessageBox::warning(this, tr("King James Notes File Error"),  g_pUserNotesDatabase->lastLoadSaveError() + tr("\n\nCheck File existence and Program Settings!"));
			// Leave the isDirty flag set, but clear the filename to force the user to re-navigate to
			//		it, or else we may accidentally overwrite the file if it happens to be "fixed" by
			//		the time we exit.  But save a reference to it so we can get the user navigated back there:
			g_pUserNotesDatabase->setErrorFilePathName(g_pUserNotesDatabase->filePathName());
			g_pUserNotesDatabase->setFilePathName(QString());
		} else {
			if (g_pUserNotesDatabase->version() < KJN_FILE_VERSION) {
				QMessageBox::warning(this, tr("Loading King James Notes File"), tr("Warning: The King James Notes File being loaded was last saved on "
											"an older version of King James Pure Bible Search.  It will automatically be updated to this version of "
											"King James Pure Bible Search.  However, if you wish to keep a copy of your Notes File in the old format, you must "
											"manually save a copy of your file now BEFORE you exit King James Pure Bible Search.\n\nFilename: \"%1\"").arg(g_pUserNotesDatabase->filePathName()));
			} else if (g_pUserNotesDatabase->version() > KJS_FILE_VERSION) {
				QMessageBox::warning(this, tr("Loading King James Notes File"), tr("Warning: The King James Notes File being loaded was created on "
											"a newer version of King James Pure Bible Search.  It may contain data or settings for things not "
											"supported on this version of King James Pure Bible Search.  If so, those new things will be LOST the "
											"next time your Notes Files is saved.  If you wish to keep a copy of your original Notes File and not "
											"risk losing any data from it, you must manually save a copy of your file now BEFORE you exit King James "
																					"Pure Bible Search.\n\nFilename: \"%1\"").arg(g_pUserNotesDatabase->filePathName()));
			}

		}
	}

	// Highlighter Tool Bar (must be after loading the User Notes Database):
	int nColors = settings.beginReadArray(groupCombine(constrColorsGroup, constrColorsHighlightersSubgroup));
	if (nColors != 0) {
		for (int ndxColor = 0; ((ndxColor < nColors) && (ndxColor < CHighlighterButtons::instance()->count())); ++ndxColor) {
			settings.setArrayIndex(ndxColor);
			QString strHighlighterName = settings.value(constrHighlighterNameKey, QString()).toString();
			CHighlighterButtons::instance()->setHighlighterList(ndxColor, strHighlighterName);
		}
	} else {
		// For a new (empty) User Notes Database, set the ToolBar to the initial file default highlighters:
		if (g_pUserNotesDatabase->filePathName().isEmpty()) {
			const TUserDefinedColorMap mapHighlighters = g_pUserNotesDatabase->highlighterDefinitionsMap();
			int ndxColor = 0;
			for (TUserDefinedColorMap::const_iterator itrHighlighters = mapHighlighters.constBegin();
							((itrHighlighters != mapHighlighters.constEnd()) && (ndxColor < CHighlighterButtons::instance()->count()));
							++itrHighlighters) {
				CHighlighterButtons::instance()->setHighlighterList(ndxColor, itrHighlighters.key());
				ndxColor++;
			}
		}
	}
	settings.endArray();

	// Set Search Results View Keyword Filter:
	m_pSearchResultWidget->keywordListChanged(true);

	// Search Phrases Settings:
	settings.beginGroup(constrSearchPhrasesGroup);
	CPersistentSettings::instance()->setSearchActivationDelay(settings.value(constrSearchActivationDelayKey, CPersistentSettings::instance()->searchActivationDelay()).toInt());
	CPersistentSettings::instance()->setSearchPhraseCompleterFilterMode(static_cast<CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM>(settings.value(constrSearchPhraseCompleterFilterModeKey, CPersistentSettings::instance()->searchPhraseCompleterFilterMode()).toUInt()));
	settings.endGroup();

	// Read Last Search before setting Search Results mode or else the last settings
	//	won't get restored -- they will be overriden by the loading of the Last Search...
	//	But, we first need to disable our SearchActivationDelay so that the updates
	//	will happen immediately -- otherwise, Search Results mode settings won't
	//	restore properly:
	int nSaveSearchActivationDelay = CPersistentSettings::instance()->searchActivationDelay();
	CPersistentSettings::instance()->setSearchActivationDelay(-1);

	// Last Search:
	m_pSearchSpecWidget->readKJVSearchFile(settings, constrLastSearchGroup);

	// Restore our activation delay:
	CPersistentSettings::instance()->setSearchActivationDelay(nSaveSearchActivationDelay);

	// Search Results mode:
	settings.beginGroup(constrSearchResultsViewGroup);
	setViewMode(static_cast<CVerseListModel::VERSE_VIEW_MODE_ENUM>(settings.value(constrResultsViewModeKey, m_pSearchResultWidget->viewMode()).toUInt()));
	setDisplayMode(static_cast<CVerseListModel::VERSE_DISPLAY_MODE_ENUM>(settings.value(constrVerseDisplayModeKey, m_pSearchResultWidget->displayMode()).toUInt()));
	setTreeMode(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(settings.value(constrVerseTreeModeKey, m_pSearchResultWidget->treeMode()).toUInt()));
	setShowMissingLeafs(settings.value(constrViewMissingNodesKey, m_pSearchResultWidget->showMissingLeafs()).toBool());
	CRelIndex ndxLastCurrentIndex(settings.value(constrCurrentIndexKey, CRelIndex().asAnchor()).toString());
	QString strHighlighterName = settings.value(constrCurrentHighlighterKey, QString()).toString();
	if (m_pSearchResultWidget->viewMode() != CVerseListModel::VVME_HIGHLIGHTERS) strHighlighterName.clear();		// Make sure we load the correct verseIndex below for Search Results and UserNotes, etc
	bool bFocusSearchResults = settings.value(constrHasFocusKey, false).toBool();
	strFont = settings.value(constrFontKey).toString();
	if (!strFont.isEmpty()) {
		QFont aFont;
		aFont.fromString(strFont);
		// Just use face-name and point size from the stored font.  This is to work around the
		//		past bugs on Mac that caused us to get stuck if the user picked strike-through
		//		or something:
		QFont aFont2;
		aFont2.setFamily(aFont.family());
		aFont2.setPointSizeF(aFont.pointSizeF());
		CPersistentSettings::instance()->setFontSearchResults(aFont2);
	}
	settings.endGroup();

	// Current Browser Reference:
	//		Note: The actual browser reference has already been loaded in
	//			initialize().  However, this will lookup the reference and
	//			see if the user was displaying a search result and if so,
	//			will set the current index for the search result to that
	//			as a fallback for when there is no Last Current Index:
	bool bLastSet = false;
	if (ndxLastCurrentIndex.isSet()) bLastSet = m_pSearchResultWidget->setCurrentIndex(m_pSearchResultWidget->vlmodel()->resolveVerseIndex(ndxLastCurrentIndex, strHighlighterName), false);	// Note: Uses ViewMode set above! (this must come after Search Results mode restoration)
	settings.beginGroup(constrBrowserViewGroup);
	bool bFocusBrowser = settings.value(constrHasFocusKey, false).toBool();
	if (!bLastSet) {
		CRelIndex ndxLastBrowsed = CRelIndex(settings.value(constrLastReferenceKey, CRelIndex().asAnchor()).toString());
		if (ndxLastBrowsed.isSet()) m_pSearchResultWidget->setCurrentIndex(m_pSearchResultWidget->vlmodel()->resolveVerseIndex(ndxLastBrowsed, strHighlighterName), false);
	}
	strFont = settings.value(constrFontKey).toString();
	if (!strFont.isEmpty()) {
		QFont aFont;
		aFont.fromString(strFont);
		// Just use face-name and point size from the stored font.  This is to work around the
		//		past bugs on Mac that caused us to get stuck if the user picked strike-through
		//		or something:
		QFont aFont2;
		aFont2.setFamily(aFont.family());
		aFont2.setPointSizeF(aFont.pointSizeF());
		CPersistentSettings::instance()->setFontScriptureBrowser(aFont2);
	}
	CPersistentSettings::instance()->setNavigationActivationDelay(settings.value(constrNavigationActivationDelayKey, CPersistentSettings::instance()->navigationActivationDelay()).toInt());
	CPersistentSettings::instance()->setPassageReferenceActivationDelay(settings.value(constrPassageReferenceActivationDelayKey, CPersistentSettings::instance()->passageReferenceActivationDelay()).toInt());
	settings.endGroup();

	// Browser Object (used for Subwindows: FindDialog, etc):
	m_pBrowserWidget->restorePersistentSettings(constrBrowserViewGroup);

	// Copy Options:
	settings.beginGroup(constrCopyOptionsGroup);
	CPersistentSettings::instance()->setReferenceDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(settings.value(constrReferenceDelimiterModeKey, CPersistentSettings::instance()->referenceDelimiterMode()).toUInt()));
	CPersistentSettings::instance()->setReferencesUseAbbreviatedBookNames(settings.value(constrReferencesAbbreviatedBookNamesKey, CPersistentSettings::instance()->referencesUseAbbreviatedBookNames()).toBool());
	CPersistentSettings::instance()->setReferencesInBold(settings.value(constrReferencesInBoldKey, CPersistentSettings::instance()->referencesInBold()).toBool());
	CPersistentSettings::instance()->setVerseNumberDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(settings.value(constrVerseNumberDelimiterModeKey, CPersistentSettings::instance()->verseNumberDelimiterMode()).toUInt()));
	CPersistentSettings::instance()->setVerseNumbersUseAbbreviatedBookNames(settings.value(constrVerseNumbersAbbreviatedBookNamesKey, CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames()).toBool());
	CPersistentSettings::instance()->setVerseNumbersInBold(settings.value(constrVerseNumbersInBoldKey, CPersistentSettings::instance()->verseNumbersInBold()).toBool());
	CPersistentSettings::instance()->setAddQuotesAroundVerse(settings.value(constrAddQuotesAroundVerseKey, CPersistentSettings::instance()->addQuotesAroundVerse()).toBool());
	CPersistentSettings::instance()->setTransChangeAddWordMode(static_cast<CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM>(settings.value(constrTransChangeAddWordModeKey, CPersistentSettings::instance()->transChangeAddWordMode()).toUInt()));
	settings.endGroup();

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
	int nResult;
	bool bPromptFilename = false;

	extern CMyApplication *g_pMyApplication;
	assert(g_pMyApplication != NULL);

	if (g_pMyApplication->isLastCanOpener()) {
		assert(g_pUserNotesDatabase != NULL);
		if (g_pUserNotesDatabase->isDirty()) {
			// If we don't have a file name, yet made some change to the KJN, prompt them for a path:
			if (g_pUserNotesDatabase->filePathName().isEmpty()) {
				if (g_pUserNotesDatabase->errorFilePathName().isEmpty()) {
					// If we don't have a filename at all, prompt for new setup:
					nResult = QMessageBox::warning(this, windowTitle(), tr("You have edited Notes, Highlighters, and/or References, but don't yet have a King James Notes File setup.\n\n"
																			 "Do you wish to setup a Notes File and save your changes??\nWarning: If you select 'No', then your changes will be lost."),
															(QMessageBox::Yes  | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
				} else {
					// If we originally had a filename, but failed in opening it, just prompt the user about saving it since it's
					//		possible they don't want to attempt to overwrite the one that failed since we couldn't load it:
					nResult = QMessageBox::warning(this, windowTitle(), tr("The previous attempt to load your King James Notes File failed.\n"
																		   "Do you wish to save the changes you've made?\n"
																		   "Warning, if you save this file overtop of your original file, you will "
																		   "lose all ability to recover the remaining data in your original file.  It's "
																		   "recommended that you save it to a new file.\n\n"
																		   "Click 'Yes' to enter a filename and save your new changes, or\n"
																		   "Click 'No' to lose your changes and exit, or\n"
																		   "Click 'Cancel' to return to King James Pure Bible Search..."),
															(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
				}
				// If the user cancelled, return:
				if ((nResult != QMessageBox::Yes) && (nResult != QMessageBox::No)) {
					event->ignore();
					return;
				}
				// If they want to save, but don't have path yet, so we need to prompt them for a path:
				if (nResult == QMessageBox::Yes) {
					bPromptFilename = true;
				}
			}
			// If the user has a file path already (or is wanting to create a new one), try to save it:
			if ((!g_pUserNotesDatabase->filePathName().isEmpty()) || (bPromptFilename)) {
				bool bDone = false;
				do {
					if (bPromptFilename) {
						QString strFilePathName = QFileDialog::getSaveFileName(this, tr("Save King James Notes File"), g_pUserNotesDatabase->errorFilePathName(), tr("King James Notes Files (*.kjn)"), NULL, 0);
						if (!strFilePathName.isEmpty()) {
							g_pUserNotesDatabase->setFilePathName(strFilePathName);
						} else {
							// If the user aborted treating after the path after all:
							event->ignore();
							return;
						}
					}

					if (!g_pUserNotesDatabase->save()) {
						nResult = QMessageBox::warning(this, tr("King James Notes File Error"),  g_pUserNotesDatabase->lastLoadSaveError() +
															tr("\n\nUnable to save the King James Notes File!\n\n"
															   "Click 'Yes' to try again, or\n"
															   "Click 'No' to lose your changes and exit, or\n"
															   "Click 'Cancel' to return to King James Pure Bible Search..."),
													   (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
						// If the user cancelled, return back to the app:
						if ((nResult != QMessageBox::Yes) && (nResult != QMessageBox::No)) {
							event->ignore();
							return;
						}
						// If they want to lose their changes, break out of loop:
						if (nResult == QMessageBox::No) {
							bDone = true;
						}
						// Set our error file path in case we are prompting the user in the loop:
						g_pUserNotesDatabase->setErrorFilePathName(g_pUserNotesDatabase->filePathName());
					} else {
						// If the save was successful, break out of loop:
						bDone = true;
					}
				} while (!bDone);
			}
			// Either the user aborted creating the User Notes File or the User Notes File Saved OK....
		}	//	(or we didn't have an updated file to save)...

		if ((g_bUserPhrasesDirty) && (!g_strUserDatabase.isEmpty())) {
			nResult = QMessageBox::warning(this, windowTitle(), tr("Do you wish to save the search phrase list changes you've made to the user database?"),
																	(QMessageBox::Yes  | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
			if ((nResult != QMessageBox::Yes) && (nResult != QMessageBox::No)) {
				event->ignore();
				return;
			}
			if (nResult == QMessageBox::Yes) {
				CBuildDatabase bdb(this);
				if (!bdb.BuildUserDatabase(g_strUserDatabase)) {
					QMessageBox::warning(this, windowTitle(), tr("Failed to save KJV User Database!\nCheck installation and settings!"));
					event->ignore();
					return;
				}
			}
		}

		savePersistentSettings();
	}

//	QMainWindow::closeEvent(event);
	deleteLater();
}

// ------------------------------------------------------------------

void CKJVCanOpener::setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode)
{
	assert(m_pActionGroupViewMode != NULL);

	QList<QAction *> lstActions = m_pActionGroupViewMode->actions();

	for (int i = 0; i < lstActions.size(); ++i) {
		if (static_cast<CVerseListModel::VERSE_VIEW_MODE_ENUM>(lstActions.at(i)->data().toUInt()) == nViewMode) {
			lstActions.at(i)->setChecked(true);
			en_viewModeChange(lstActions.at(i));
			break;
		}
	}
}

void CKJVCanOpener::setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	assert(m_pActionGroupDisplayMode != NULL);

	QList<QAction *> lstActions = m_pActionGroupDisplayMode->actions();

	for (int i = 0; i < lstActions.size(); ++i) {
		if (static_cast<CVerseListModel::VERSE_DISPLAY_MODE_ENUM>(lstActions.at(i)->data().toUInt()) == nDisplayMode) {
			lstActions.at(i)->setChecked(true);
			en_displayModeChange(lstActions.at(i));
			break;
		}
	}
}

void CKJVCanOpener::setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode)
{
	assert(m_pActionGroupTreeMode != NULL);

	QList<QAction *> lstActions = m_pActionGroupTreeMode->actions();

	for (int i = 0; i < lstActions.size(); ++i) {
		if (static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(lstActions.at(i)->data().toUInt()) == nTreeMode) {
			lstActions.at(i)->setChecked(true);
			en_treeModeChange(lstActions.at(i));
			break;
		}
	}
}

void CKJVCanOpener::setShowMissingLeafs(bool bShowMissing)
{
	assert(m_pActionShowMissingLeafs != NULL);
	m_pActionShowMissingLeafs->setChecked(bShowMissing);
	en_viewShowMissingsLeafs();
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_NewSearch()
{
	m_pSearchSpecWidget->reset();
}

void CKJVCanOpener::en_OpenSearch()
{
	QString strFilePathName = QFileDialog::getOpenFileName(this, tr("Open KJV Search File"), QString(), tr("KJV Search Files (*.kjs)"), NULL, QFileDialog::ReadOnly);
	if (!strFilePathName.isEmpty())
		if (!openKJVSearchFile(strFilePathName))
			QMessageBox::warning(this, tr("KJV Search File Open Failed"), tr("Failed to open and read the specified KJV Search File!"));
}

void CKJVCanOpener::en_SaveSearch()
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

void CKJVCanOpener::en_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase)
{
	assert(pSearchPhrase != NULL);

	// If this search phrase's editor was currently active, remove it or else
	//		we'll crash later accessing data for a deleted object:
	if ((m_bPhraseEditorActive) && ((m_pActionSearchPhraseEditMenu != NULL) &&
									(m_pActionSearchPhraseEditMenu->menu() == pSearchPhrase->phraseEditor()->getEditMenu()))) {
		en_addSearchPhraseEditMenu(false);
	}
}

void CKJVCanOpener::en_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase)
{
	Q_UNUSED(pSearchPhrase);

	// The former functionality of this is now handled by en_changedSearchSpec()
}

void CKJVCanOpener::en_copySearchPhraseSummary()
{
	QString strSummary;

	strSummary += m_pSearchSpecWidget->searchPhraseSummaryText();
	strSummary += m_pSearchResultWidget->searchResultsSummaryText();

	QMimeData *mime = new QMimeData();

	mime->setText(strSummary);
	mime->setHtml("<qt><pre>\n" + strSummary + "</pre></qt>\n");
	QApplication::clipboard()->setMimeData(mime);
}

void CKJVCanOpener::en_changedSearchSpec(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases)
{
	m_pSearchResultWidget->setParsedPhrases(aSearchCriteria, phrases);		// Setting the phrases will build all of the results and set the verse list on the model
	m_pSearchSpecWidget->enableCopySearchPhraseSummary(m_pSearchResultWidget->haveResults());
	// Auto-switch to Search Results mode:
	if (m_pSearchResultWidget->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS)
		setViewMode(CVerseListModel::VVME_SEARCH_RESULTS);
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_addPassageBrowserEditMenu(bool bAdd)
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

void CKJVCanOpener::en_addSearchResultsEditMenu(bool bAdd)
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

void CKJVCanOpener::en_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor)
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

void CKJVCanOpener::en_activatedBrowser()
{
	m_pSearchSpecWidget->en_activatedPhraseEditor(NULL);		// Notify that we have no search phrase editor active
	en_addPassageBrowserEditMenu(true);
	en_addSearchResultsEditMenu(false);
	en_addSearchPhraseEditMenu(false);
	setDetailsEnable();
}

void CKJVCanOpener::en_activatedSearchResults()
{
	m_pSearchSpecWidget->en_activatedPhraseEditor(NULL);		// Notify that we have no search phrase editor active
	en_addPassageBrowserEditMenu(false);
	en_addSearchResultsEditMenu(true);
	en_addSearchPhraseEditMenu(false);
	setDetailsEnable();
}

void CKJVCanOpener::en_activatedPhraseEditor(const CPhraseLineEdit *pEditor)
{
	en_addPassageBrowserEditMenu(false);
	en_addSearchResultsEditMenu(false);
	en_addSearchPhraseEditMenu(true, pEditor);
	setDetailsEnable();

	CKJVNoteEditDlg::actionUserNoteEditor()->setEnabled(false);
	CKJVCrossRefEditDlg::actionCrossRefsEditor()->setEnabled(false);
	const QList<QAction *> lstHighlightActions = CHighlighterButtons::instance()->actions();
	for (int ndxHighlight = 0; ndxHighlight < lstHighlightActions.size(); ++ndxHighlight) {
		lstHighlightActions.at(ndxHighlight)->setEnabled(false);
	}
}

bool CKJVCanOpener::isBrowserFocusedOrActive() const
{
	assert(m_pBrowserWidget != NULL);

	return (m_pBrowserWidget->hasFocusBrowser() || isBrowserActive());
}

bool CKJVCanOpener::isSearchResultsFocusedOrActive() const
{
	assert(m_pSearchResultWidget != NULL);

	return (m_pSearchResultWidget->hasFocusSearchResult() || isSearchResultsActive());
}

bool CKJVCanOpener::isPhraseEditorFocusedOrActive() const
{
	return (isPhraseEditorActive());			// TODO : Add PhraseEditor hasFocus() when it's actually needed
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_viewModeChange(QAction *pAction)
{
	assert(pAction != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	TVerseIndex ndxCurrent(m_pSearchResultWidget->currentVerseIndex());

	m_pSearchResultWidget->setViewMode(static_cast<CVerseListModel::VERSE_VIEW_MODE_ENUM>(pAction->data().toUInt()));

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::en_displayModeChange(QAction *pAction)
{
	assert(pAction != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	TVerseIndex ndxCurrent(m_pSearchResultWidget->currentVerseIndex());

	m_pSearchResultWidget->setDisplayMode(static_cast<CVerseListModel::VERSE_DISPLAY_MODE_ENUM>(pAction->data().toUInt()));

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::en_treeModeChange(QAction *pAction)
{
	assert(pAction != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	TVerseIndex ndxCurrent(m_pSearchResultWidget->currentVerseIndex());

	m_pSearchResultWidget->setTreeMode(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(pAction->data().toUInt()));
	m_pActionShowMissingLeafs->setEnabled(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(pAction->data().toUInt()) != CVerseListModel::VTME_LIST);

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::en_viewShowMissingsLeafs()
{
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	TVerseIndex ndxCurrent(m_pSearchResultWidget->currentVerseIndex());

	if (m_pSearchResultWidget->treeMode() == CVerseListModel::VTME_LIST) {
		if (m_pSearchResultWidget->showMissingLeafs()) m_pSearchResultWidget->setShowMissingLeafs(false);
	} else {
		m_pSearchResultWidget->setShowMissingLeafs(m_pActionShowMissingLeafs->isChecked());
	}

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_gotoIndex(const TPhraseTag &tag)
{
	assert(m_pActionBookBackward != NULL);
	assert(m_pActionBookForward != NULL);
	assert(m_pActionChapterBackward != NULL);
	assert(m_pActionChapterForward != NULL);
	if ((m_pActionBookBackward == NULL) ||
		(m_pActionBookForward == NULL) ||
		(m_pActionChapterBackward == NULL) ||
		(m_pActionChapterForward == NULL)) return;

	m_pActionBookBackward->setEnabled(tag.relIndex().book() >= 2);
	m_pActionBookForward->setEnabled(tag.relIndex().book() < m_pBibleDatabase->bibleEntry().m_nNumBk);
	m_pActionChapterBackward->setEnabled((tag.relIndex().book() >= 2) ||
										((tag.relIndex().book() == 1) && (tag.relIndex().chapter() >= 2)));
	const CBookEntry *pBookEntry = m_pBibleDatabase->bookEntry(tag.relIndex().book());
	m_pActionChapterForward->setEnabled((tag.relIndex().book() < m_pBibleDatabase->bibleEntry().m_nNumBk) ||
										((tag.relIndex().book() == m_pBibleDatabase->bibleEntry().m_nNumBk) && (tag.relIndex().chapter() < (pBookEntry ? pBookEntry->m_nNumChp : 0))));
}

void CKJVCanOpener::en_browserHistoryChanged()
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

void CKJVCanOpener::en_clearBrowserHistory()
{
	m_pBrowserWidget->clearHistory();
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_SearchResultActivated(const QModelIndex &index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (!index.isValid()) return;

	CRelIndex ndxRel(m_pSearchResultWidget->vlmodel()->navigationIndexForModelIndex(index));

	if (!ndxRel.isSet()) return;				// If user double-clicks on a highligher, there will be no RelIndex

	m_pBrowserWidget->gotoIndex(TPhraseTag(ndxRel));
	m_pBrowserWidget->setFocusBrowser();
}

void CKJVCanOpener::en_PassageNavigatorTriggered()
{
	assert(m_pBibleDatabase.data() != NULL);

	if (isBrowserFocusedOrActive()) {
		m_pBrowserWidget->showPassageNavigator();
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->editableNodeSelected())) {
		m_pSearchResultWidget->showPassageNavigator();
	} else {
		CKJVPassageNavigatorDlg dlg(m_pBibleDatabase, this);

		if (dlg.exec() == QDialog::Accepted) {
			m_pBrowserWidget->gotoIndex(dlg.passage());
			m_pBrowserWidget->setFocusBrowser();
		}
	}
}

void CKJVCanOpener::en_userNoteEditorTriggered()
{
	if ((!isBrowserFocusedOrActive()) && (!isSearchResultsFocusedOrActive())) return;

	assert(m_pUserNoteEditorDlg != NULL);
	assert(g_pUserNotesDatabase != NULL);
	if ((m_pUserNoteEditorDlg == NULL) || (g_pUserNotesDatabase == NULL)) return;

	CRelIndex indexNote;

	if (isBrowserFocusedOrActive()) {
		indexNote = m_pBrowserWidget->selection().relIndex();
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->editableNodeSelected()))  {
		indexNote = m_pSearchResultWidget->vlmodel()->navigationIndexForModelIndex(m_pSearchResultWidget->currentIndex());
	}

	if (!indexNote.isSet()) return;
	m_pUserNoteEditorDlg->setLocationIndex(indexNote);
	if (m_pUserNoteEditorDlg->exec() == QDialog::Accepted) {
		if (isBrowserFocusedOrActive())
			m_pBrowserWidget->gotoIndex(m_pBrowserWidget->selection());		// Re-render text (note: The Note may be deleted as well as changed)
	}
}

void CKJVCanOpener::en_crossRefsEditorTriggered()
{
	if ((!isBrowserFocusedOrActive()) && (!isSearchResultsFocusedOrActive())) return;

	assert(m_pCrossRefsEditorDlg != NULL);
	assert(g_pUserNotesDatabase != NULL);
	if ((m_pCrossRefsEditorDlg == NULL) || (g_pUserNotesDatabase == NULL)) return;

	TPassageTag tagCrossRef;

	if (isBrowserFocusedOrActive()) {
		tagCrossRef.setFromPhraseTag(m_pBibleDatabase, m_pBrowserWidget->selection());
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->editableNodeSelected())) {
		// Unlike editing notes and passage navigation, editing cross-references should bring up the "Source" Cross-Reference:
		tagCrossRef = TPassageTag(m_pSearchResultWidget->currentVerseIndex().relIndex());
	}

	if (!tagCrossRef.isSet()) return;
	m_pCrossRefsEditorDlg->setSourcePassage(tagCrossRef);
	if (m_pCrossRefsEditorDlg->exec() == QDialog::Accepted) {
		if (isBrowserFocusedOrActive())
			m_pBrowserWidget->gotoIndex(m_pBrowserWidget->selection());		// Re-render text (note: The Cross-Ref may be deleted as well as changed)
	}
}

void CKJVCanOpener::en_viewDetails()
{
	if ((isBrowserFocusedOrActive()) && (m_pBrowserWidget->haveDetails())) {
		m_pBrowserWidget->showDetails();
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->haveDetails())) {
		m_pSearchResultWidget->showDetails();
	}
}

void CKJVCanOpener::setDetailsEnable()
{
	bool bDetailsEnable = false;

	if ((isBrowserFocusedOrActive()) && (m_pBrowserWidget->haveDetails())) {
		bDetailsEnable = true;
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->haveDetails())) {
		bDetailsEnable = true;
	}

	emit canShowDetails(bDetailsEnable);
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_HelpManual()
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

void CKJVCanOpener::en_HelpAbout()
{
	CKJVAboutDlg dlg(this);
	dlg.exec();
}

void CKJVCanOpener::en_QuickActivate()
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

void CKJVCanOpener::en_Configure()
{
	CHighlighterButtons::instance()->enterConfigurationMode();

	CKJVConfigurationDialog dlgConfigure(m_pBibleDatabase, this);
	dlgConfigure.exec();

	CHighlighterButtons::instance()->leaveConfigurationMode();
}

void CKJVCanOpener::setTextBrightness(bool bInvert, int nBrightness)
{
	extern CMyApplication *g_pMyApplication;

	// Note: This code needs to cooperate with the setStyleSheet in the constructor
	//			that works around QTBUG-13768...

	if (CPersistentSettings::instance()->adjustDialogElementBrightness()) {
		// Note: This will automatically cause a repaint:
		g_pMyApplication->setStyleSheet(QString("CPhraseLineEdit { background-color:%1; color:%2; }\n"
												"QLineEdit { background-color:%1; color:%2; }\n"
												"QComboBox { background-color:%1; color:%2; }\n"
												"QComboBox QAbstractItemView { background-color:%1; color:%2; }\n"
												"QFontComboBox { background-color:%1; color:%2; }\n"
												"QListView { background-color:%1; color:%2; }\n"						// Completers and QwwConfigWidget
												"QSpinBox { background-color:%1; color:%2; }\n"
												"QDoubleSpinBox { background-color:%1; color:%2; }\n"
										 ).arg(CPersistentSettings::textBackgroundColor(bInvert, nBrightness).name())
										  .arg(CPersistentSettings::textForegroundColor(bInvert, nBrightness).name()));
	} else {
		g_pMyApplication->setStyleSheet(m_strAppStartupStyleSheet);
	}

	return;
}

void CKJVCanOpener::setAdjustDialogElementBrightness(bool bAdjust)
{
	Q_UNUSED(bAdjust);
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
}

