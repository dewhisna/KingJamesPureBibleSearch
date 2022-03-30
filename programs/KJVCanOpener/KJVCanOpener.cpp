/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "ReportError.h"
#include "myApplication.h"
#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "PassageNavigatorDlg.h"
#include "version.h"
#include "PersistentSettings.h"
#include "UserNotesDatabase.h"
#include "HighlighterButtons.h"
#include "AboutDlg.h"
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
#include "Configuration.h"
#include "DictionaryWidget.h"			// Note: This one is needed if we are doing configuration in general, not just USING_DICTIONARIES
#if !defined(VNCSERVER) && !defined(EMSCRIPTEN)
#include "NoteEditDlg.h"
#include "CrossRefEditDlg.h"
#include "SaveLoadFileDialog.h"
#endif
#endif
#include "PhraseNavigator.h"
#include "PhraseListModel.h"

#if __cplusplus > 199711L
#include <random>
#include <chrono>
#define USE_STD_RANDOM						// Use std::default_random_engine instead of stdlib rand() functions
#else
#include <stdlib.h>
#include <time.h>
#endif

#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QTime>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>

// ============================================================================

#define KJS_FILE_VERSION 3				// Current KJS File Version (King James Search file)
#define KJVAPP_REGISTRY_VERSION 1		// Version of Registry Settings (update when changing main window layout and/or state properties)
#define PS_SPLITTER_VERSION 1			// Persistent Settings -- Splitter Version (update when changing splitter layout and/or state properties)

#define NUM_QUICK_ACTIONS 10

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

#ifdef Q_OS_ANDROID
//	const char *g_constrHelpDocFilename = "doc/KingJamesPureBibleSearch.pdf";
	const char *g_constrHelpDocFilename = "http://www.PureBibleSearch.com/manual/";
#elif defined(Q_OS_IOS)
	const char *g_constrHelpDocFilename = "doc/KingJamesPureBibleSearch.pdf";
#elif defined(Q_OS_OSX) || defined(Q_OS_MACX)
	const char *g_constrHelpDocFilename = "../SharedSupport/doc/KingJamesPureBibleSearch.pdf";
#elif defined(EMSCRIPTEN)
	const char *g_constrHelpDocFilename = "http://cloud.dewtronics.com/KingJamesPureBibleSearch/KingJamesPureBibleSearch.pdf";
#elif defined(VNCSERVER)
//	const char *g_constrHelpDocFilename = "";
#else
	const char *g_constrHelpDocFilename = "doc/KingJamesPureBibleSearch.pdf";
#endif

#ifndef VNCSERVER
	const char *g_constrPureBibleSearchURL = "http://www.PureBibleSearch.com/";
#endif

	// Key constants:
	// --------------
	// MainApp Control:
	const QString constrMainAppControlGroup("MainApp/Controls");
	const QString constrInvertTextBrightnessKey("InvertTextBrightness");
	const QString constrTextBrightnessKey("TextBrightness");
	const QString constrAdjustDialogElementBrightnessKey("AdjustDialogElementBrightness");

	// Main Bible Database Settings:
	const QString constrMainAppBibleDatabaseGroup("MainApp/BibleDatabase");
	const QString constrDatabaseUUIDKey("UUID");

	// Main Dictionary Database Settings:
	const QString constrMainAppDictDatabaseGroup("MainApp/DictionaryDatabase");
	//const QString constrDatabaseUUIDKey("UUID");

	// Colors:
	const QString constrColorsGroup("Colors");
	const QString constrColorsHighlightersSubgroup("Highlighters");
	const QString constrWordsOfJesusColorKey("WordsOfJesusColor");
	const QString constrSearchResultsColorKey("SearchResultsColor");
	const QString constrCursorTrackerColorKey("CursorTrackerColor");
	const QString constrHighlighterNameKey("HighlighterName");

	// RestoreState:
	const QString constrMainAppRestoreStateGroup("RestoreState/MainApp");
	const QString constrSplitterRestoreStateGroup("RestoreState/SplitterEx");
	const QString constrSplitterDictionaryRestoreStateGroup("RestoreState/SplitterDictionaryEx");
	const QString constrGeometryKey("Geometry");
	const QString constrWindowStateKey("WindowState");
	const QString constrStateVersionKey("StateVersion");

	// UserNotesDatabase:
	const QString constrUserNotesDatabaseGroup("UserNotesDatabase");
	const QString constrFilePathNameKey("FilePathName");
	const QString constrKeepBackupKey("KeepBackup");
	const QString constrBackupFilenamePostfixKey("BackupFilenamePostfix");
	const QString constrAutoSaveTimeKey("AutoSaveTime");
	const QString constrDefaultNoteBackgroundColorKey("DefaultNoteBackgroundColor");

	// Search Phrases:
	const QString constrLastSearchGroup("LastSearch");
	const QString constrSearchPhrasesGroup("SearchPhrases");
	const QString constrUserSearchPhrasesGroup("UserSearchPhrases");
	const QString constrSearchActivationDelayKey("SearchActivationDelay");
	const QString constrAutoCompleterActivationDelayKey("AutoCompleterActivationDelay");
	const QString constrSearchPhraseCompleterFilterModeKey("SearchPhraseCompleterFilterMode");
	const QString constrInitialNumberOfSearchPhrasesKey("InitialNumberOfSearchPhrases");
	const QString constrHideMatchingPhrasesListsKey("HideMatchingPhrasesLists");

	// Search Results View:
	const QString constrSearchResultsViewGroup("SearchResultsView");
	const QString constrResultsViewModeKey("ResultsViewMode");
	const QString constrVerseDisplayModeKey("VerseDisplayMode");
	const QString constrVerseTreeModeKey("VerseTreeMode");
	const QString constrViewMissingNodesKey("TreeShowsMissingNodes");
	const QString constrCurrentIndexKey("CurrentIndex");
	const QString constrCurrentHighlighterKey("CurrentHighlighter");
	const QString constrHasFocusKey("HasFocus");
	const QString constrFontKey("Font");				// Deprecated entry name -- here only for old value deletion (replaced by FontName and FontSize)
	const QString constrFontNameKey("FontName");
	const QString constrFontSizeKey("FontSize");
	const QString constrAutoExpandSearchResultsTreeViewKey("AutoExpandSearchResultsTreeView");
	const QString constrHideNotFoundInStatisticsKey("HideNotFoundInStatistics");
	const QString constrShowHighlightersInSearchResultsKey("ShowHighlightingInSearchResults");
	const QString constrShowOCntInSearchResultsRefs("ShowOCntInSearchResultsRefs");
	const QString constrShowWrdNdxInSearchResultsRefs("ShowWrdNdxInSearchResultsRefs");

	// Browser View:
	const QString constrBrowserViewGroup("Browser");
	const QString constrLastReferenceKey("LastReference");
	const QString constrLastSelectionSizeKey("SelectionSize");
	//const QString constrHasFocusKey("HasFocus");
	//const QString constrFontKey("Font");				// Deprecated entry name -- here only for old value deletion (replaced by FontName and FontSize)
	//const QString constrFontNameKey("FontName");
	//const QString constrFontSizeKey("FontSize");
	const QString constrNavigationActivationDelayKey("NavigationActivationDelay");
	const QString constrPassageReferenceActivationDelayKey("PassageReferenceActivationDelay");
	const QString constrShowExcludedSearchResultsKey("ShowExcludedSearchResults");
	const QString constrChapterScrollbarModeKey("ChapterScrollbarMode");
	const QString constrVerseRenderingModeKey("VerseRenderingMode");
	const QString constrShowPilcrowMarkersKey("ShowPilcrowMarkers");
	const QString constrLineHeightKey("LineHeight");
	const QString constrBrowserNavigationPaneModeKey("BrowserNavigationPaneMode");
	const QString constrBrowserDisplayModeKey("BrowserDisplayMode");
	const QString constrRandomPassageWeightModeKey("RandomPassageWeightMode");
	const QString constrFootnoteRenderingModeKey("FootnoteRenderingMode");

	// Dictionary Widget:
	const QString constrDictionaryGroup("Dictionary");
	//const QString constrFontKey("Font");				// Deprecated entry name -- here only for old value deletion (replaced by FontName and FontSize)
	//const QString constrFontNameKey("FontName");
	//const QString constrFontSizeKey("FontSize");
	const QString constrDictionaryCompleterFilterModeKey("DictionaryCompleterFilterMode");
	const QString constrDictionaryActivationDelayKey("DictionaryActivationDelay");

	// Copy Options:
	const QString constrCopyOptionsGroup("CopyOptions");
	const QString constrReferenceDelimiterModeKey("ReferenceDelimiterMode");
	const QString constrReferencesAbbreviatedBookNamesKey("ReferencesAbbreviatedBookNames");
	const QString constrReferencesInBoldKey("ReferencesInBold");
	const QString constrReferencesAtEndKey("ReferencesAtEnd");
	const QString constrVerseNumberDelimiterModeKey("VerseNumberDelimiterMode");
	const QString constrVerseNumbersAbbreviatedBookNamesKey("VerseNumbersAbbreviatedBookNames");
	const QString constrVerseNumbersInBoldKey("VerseNumbersInBold");
	const QString constrAddQuotesAroundVerseKey("AddQuotesAroundVerse");
	const QString constrTransChangeAddWordModeKey("TransChangeAddWordMode");
	//const QString constrVerseRenderingModeKey("VerseRenderingMode");
	const QString constrCopyPilcrowMarkersKey("CopyPilcrowMarkers");
	const QString constrCopyColophonsKey("CopyColophons");
	const QString constrCopySuperscriptionsKey("CopySuperscriptions");
	const QString constrCopyFontSelectionKey("CopyFontSelection");
	const QString constrCopyFontKey("CopyFont");		// Deprecated entry name -- here only for old value deletion (replaced by CopyFontName and CopyFontSize)
	const QString constrCopyFontNameKey("CopyFontName");
	const QString constrCopyFontSizeKey("CopyFontSize");
	const QString constrCopyMimeTypeKey("CopyMimeType");
	const QString constrCopySearchResultsAddBlankLineBetweenVersesKey("SearchResultsAddBlankLineBetweenVerses");
	const QString constrSearchResultsVerseCopyOrderKey("SearchResultsVerseCopyOrder");
	const QString constrCopyOCntInSearchResultsRefs("CopyOCntInSearchResultsRefs");
	const QString constrCopyWrdNdxInSearchResultsRefs("CopyWrdNdxInSearchResultsRefs");

	// UserNoteEditor Dialog:
	const QString constrUserNoteEditorGroup("UserNoteEditor");

	// CrossRefsEditor Dialog:
	const QString constrCrossRefsEditorGroup("CrossRefsEditor");

	// Bible Database Settings:
	const QString constrBibleDatabaseSettingsGroup("BibleDatabaseSettings");
	//const QString constrDatabaseUUIDKey("UUID");
	const QString constrLoadOnStartKey("LoadOnStart");
	const QString constrHideHyphensKey("HideHyphens");
	const QString constrHyphenSensitiveKey("HyphenSensitive");
	const QString constrHideCantillationMarksKey("HideCantillationMarks");
	const QString constrVersificationKey("Versification");
	const QString constrCategoryGroupKey("CategoryGroup");

	// Dictionary Database Settings:
	const QString constrDictDatabaseSettingsGroup("DictionaryDatabaseSettings");
	//const QString constrDatabaseUUIDKey("UUID");									// Entries in the dictionary settings signify selecting it for the corresponding language
	//const QString constrLoadOnStartKey("LoadOnStart");
}

// ============================================================================

CKJVCanOpener::CKJVCanOpener(CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QMainWindow(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_bDoingUpdate(false),
	// ----
	m_pActionPassageBrowserEditMenu(nullptr),
	m_pActionPassageReferenceEditMenu(nullptr),
	m_pActionSearchResultsEditMenu(nullptr),
	m_pActionSearchPhraseEditMenu(nullptr),
	m_pActionDictionaryEditMenu(nullptr),
	m_pActionDictWordEditMenu(nullptr),
	// ----
	m_pViewMenu(nullptr),
	m_pActionGroupViewMode(nullptr),
	m_pActionGroupDisplayMode(nullptr),
	m_pActionGroupTreeMode(nullptr),
	m_pActionShowMissingLeafs(nullptr),
	m_pActionExpandAll(nullptr),
	m_pActionCollapseAll(nullptr),
	m_pActionViewDetails(nullptr),
#ifdef USE_GEMATRIA
	m_pActionViewGematria(nullptr),
#endif
	// ----
	m_pActionBookBackward(nullptr),
	m_pActionBookForward(nullptr),
	m_pActionChapterBackward(nullptr),
	m_pActionChapterForward(nullptr),
	m_pActionNavBackward(nullptr),
	m_pActionNavForward(nullptr),
	m_pActionNavHome(nullptr),
	m_pActionNavClear(nullptr),
	m_pActionJump(nullptr),
	m_pActionRefresh(nullptr),
	// ----
	m_pActionBibleDatabasesList(nullptr),
	m_pActionSearchWindowList(nullptr),
	// ----
	m_pActionAbout(nullptr),
	// ----
	m_pSpeechToolbar(nullptr),
	m_pActionSpeechPlay(nullptr),
	m_pActionSpeechPause(nullptr),
	m_pActionSpeechStop(nullptr),
	m_pActionSpeakSelection(nullptr),
	// ----
	m_bBrowserActive(false),
	m_bSearchResultsActive(false),
	m_bPhraseEditorActive(false),
	m_bDictionaryActive(false),
	m_bCanClose(true),
	m_bIsClosing(false),
	m_pSearchSpecWidget(nullptr),
	m_pSplitter(nullptr),
	m_pSplitterDictionary(nullptr),
	m_pSearchResultWidget(nullptr),
	m_pBrowserWidget(nullptr),
	m_pDictionaryWidget(nullptr),
	m_pUserNoteEditorDlg(nullptr),
	m_pCrossRefsEditorDlg(nullptr),
	m_pHighlighterButtons(nullptr),
	m_pActionUserNoteEditor(nullptr),
	m_pActionCrossRefsEditor(nullptr)
{
	Q_ASSERT(!g_pMyApplication.isNull());
	Q_ASSERT(!m_pBibleDatabase.isNull());

	ui.setupUi(this);

	// Seed our random number generator for launching random passages:
	srand(time(nullptr));

	QAction *pAction;

	// -------------------- User Notes/Highlighter/References Toolbar:

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)

	// Note: Must set this up before creating CBrowserWidget, or else our toolbar
	//			will be null when its constructor is building menus:
	m_pHighlighterButtons = new CHighlighterButtons(this);
	m_pHighlighterButtons->addHighlighterButtonsToToolBar(ui.usernotesToolBar);

	ui.usernotesToolBar->addSeparator();

	m_pActionUserNoteEditor = new QAction(QIcon(":/res/App-edit-icon-128.png"), tr("Add/Edit/Remove Note...", "MainMenu"), this);
	m_pActionUserNoteEditor->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
	m_pActionUserNoteEditor->setStatusTip(tr("Add/Edit/Remove Note to current verse or passage", "MainMenu"));
	m_pActionUserNoteEditor->setToolTip(tr("Add/Edit/Remove Note to current verse or passage", "MainMenu"));
	m_pActionUserNoteEditor->setEnabled(false);		// Will get enabled on proper focus-in to Search Results and/or Scripture Browser
	ui.usernotesToolBar->addAction(m_pActionUserNoteEditor);

	ui.usernotesToolBar->addSeparator();

	m_pActionCrossRefsEditor = new QAction(QIcon(":/res/insert-cross-reference.png"), tr("Add/Edit/Remove Cross Reference...", "MainMenu"), this);
	m_pActionCrossRefsEditor->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
	m_pActionCrossRefsEditor->setStatusTip(tr("Add/Edit/Remove Cross Reference to link this verse or passage with another", "MainMenu"));
	m_pActionCrossRefsEditor->setToolTip(tr("Add/Edit/Remove Cross Reference to link this verse or passage with another", "MainMenu"));
	m_pActionCrossRefsEditor->setEnabled(false);		// Will get enabled on proper focus-in to Search Results and/or Scripture Browser
	ui.usernotesToolBar->addAction(m_pActionCrossRefsEditor);

#else
	removeToolBar(ui.usernotesToolBar);
	delete ui.usernotesToolBar;
	ui.usernotesToolBar = nullptr;
#endif

	// -------------------- Speech Toolbar:

#ifdef USING_QT_SPEECH
	// Accelerators: Ctrl-Shift (Meta-Shift on Mac): Y-U-I-O-P (Back-Stop-Pause-Play-Forward)
	m_pSpeechToolbar = new QToolBar(this);
	m_pSpeechToolbar->setObjectName(QLatin1String("speechToolBar"));
	addToolBar(Qt::TopToolBarArea, m_pSpeechToolbar);
	m_pSpeechToolbar->setWindowTitle(QApplication::translate("CKJVCanOpener", "Text-To-&Speech Toolbar", nullptr));	// Keep this in QApplication namespace to be consistent with the ToolBars in the UI file

	m_pActionSpeechPlay = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play", "MainMenu"), this);
#ifndef Q_OS_MAC
	m_pActionSpeechPlay->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
#else
	m_pActionSpeechPlay->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_O));
#endif
	m_pActionSpeechPlay->setStatusTip(tr("Play Text-To-Speech for Current Selection and/or Chapter", "MainMenu"));
	m_pActionSpeechPlay->setToolTip(tr("Play Text-To-Speech", "MainMenu"));
	m_pActionSpeechPlay->setEnabled(false);		// Will get enabled on proper focus-in to Search Results and/or Scripture Browser w/selection
	m_pSpeechToolbar->addAction(m_pActionSpeechPlay);

#if 0
	m_pActionSpeechPause = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause", "MainMenu"), this);
#ifndef Q_OS_MAC
	m_pActionSpeechPause->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I));
#else
	m_pActionSpeechPause->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_I));
#endif
	m_pActionSpeechPause->setStatusTip(tr("Pause Text-To-Speech currently in progress", "MainMenu"));
	m_pActionSpeechPause->setToolTip(tr("Pause Text-To-Speech", "MainMenu"));
	m_pActionSpeechPause->setEnabled(false);		// Will get enabled on proper focus-in to Search Results and/or Scripture Browser w/selection
	m_pSpeechToolbar->addAction(m_pActionSpeechPause);
#endif

	m_pActionSpeechStop = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop", "MainMenu"), this);
#ifndef Q_OS_MAC
	m_pActionSpeechStop->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
#else
	m_pActionSpeechStop->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_U));
#endif
	m_pActionSpeechStop->setStatusTip(tr("Stop Text-To-Speech currently in progress", "MainMenu"));
	m_pActionSpeechStop->setToolTip(tr("Stop Text-To-Speech", "MainMenu"));
	m_pActionSpeechStop->setEnabled(false);			// Will get enabled on proper focus-in to Search Results and/or Scripture Browser w/selection
	m_pSpeechToolbar->addAction(m_pActionSpeechStop);

	m_pActionSpeakSelection = new QAction("speakSelection", this);
#ifndef Q_OS_MAC
	m_pActionSpeakSelection->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X));
#else
	m_pActionSpeakSelection->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_X));
#endif

	if (actionSpeechPause())
		connect(actionSpeechPause(), SIGNAL(triggered()), this, SLOT(en_speechPause()));
	if (actionSpeechStop())
		connect(actionSpeechStop(), SIGNAL(triggered()), this, SLOT(en_speechStop()));

	QtSpeech *pSpeech = g_pMyApplication->speechSynth();

	if (pSpeech != nullptr) {
		connect(pSpeech, SIGNAL(beginning()), this, SLOT(setSpeechActionEnables()));
		connect(pSpeech, SIGNAL(finished(bool)), this, SLOT(setSpeechActionEnables()));
	}

#endif	// USING_QT_SPEECH

	// -------------------- Setup the Three Panes:

	m_pSplitter = new QSplitter(ui.centralWidget);
	m_pSplitter->setObjectName(QString::fromUtf8("splitter"));
	m_pSplitter->setOrientation(Qt::Horizontal);
	m_pSplitter->setChildrenCollapsible(true);

	m_pSplitterDictionary = new QSplitter(m_pSplitter);
	m_pSplitterDictionary->setObjectName(QString::fromUtf8("splitterDictionary"));
	m_pSplitterDictionary->setOrientation(Qt::Vertical);
	m_pSplitterDictionary->setChildrenCollapsible(true);

	m_pSearchSpecWidget = new CSearchSpecWidget(m_pBibleDatabase, m_pSplitter);
	m_pSearchSpecWidget->setObjectName(QString::fromUtf8("SearchSpecWidget"));
	m_pSplitter->addWidget(m_pSearchSpecWidget);

	m_pSearchResultWidget = new CSearchResults(m_pBibleDatabase, m_pSplitter);
	m_pSearchResultWidget->setObjectName(QString::fromUtf8("SearchResultsWidget"));
	m_pSplitter->addWidget(m_pSearchResultWidget);

	m_pBrowserWidget = new CBrowserWidget(m_pSearchResultWidget->vlmodel(), m_pBibleDatabase, m_pSplitterDictionary);
	m_pBrowserWidget->setObjectName(QString::fromUtf8("BrowserWidget"));
	QSizePolicy aSizePolicyBrowser(QSizePolicy::Expanding, QSizePolicy::Expanding);
	aSizePolicyBrowser.setHorizontalStretch(20);
	aSizePolicyBrowser.setVerticalStretch(20);
	aSizePolicyBrowser.setHeightForWidth(m_pBrowserWidget->sizePolicy().hasHeightForWidth());
	m_pBrowserWidget->setSizePolicy(aSizePolicyBrowser);
	m_pSplitterDictionary->addWidget(m_pBrowserWidget);

#if defined(USING_DICTIONARIES) && !defined(IS_CONSOLE_APP)
	CDictionaryDatabasePtr pDictionary = TDictionaryDatabaseList::locateAndLoadDictionary(m_pBibleDatabase->language(), this);
	if (!pDictionary.isNull()) {
		m_pDictionaryWidget = new CDictionaryWidget(pDictionary, m_pBibleDatabase->language(), m_pSplitterDictionary);
		m_pDictionaryWidget->setObjectName(QString::fromUtf8("DictionaryWidget"));
		QSizePolicy aSizePolicyDictionary(QSizePolicy::Expanding, QSizePolicy::Expanding);
		aSizePolicyDictionary.setHorizontalStretch(20);
		aSizePolicyDictionary.setVerticalStretch(0);
		aSizePolicyDictionary.setHeightForWidth(m_pDictionaryWidget->sizePolicy().hasHeightForWidth());
		m_pDictionaryWidget->setSizePolicy(aSizePolicyDictionary);
		m_pSplitterDictionary->addWidget(m_pDictionaryWidget);
	}
#endif

	m_pSplitter->addWidget(m_pSplitterDictionary);

	m_pSplitter->setStretchFactor(0, 0);
	m_pSplitter->setStretchFactor(1, 20);
	m_pSplitter->setStretchFactor(2, 20);

	m_pSplitterDictionary->setStretchFactor(0, 10);
	if (m_pSplitterDictionary->count() > 1) m_pSplitterDictionary->setStretchFactor(1, 1);

	ui.horizontalLayout->addWidget(m_pSplitter);

	// --------------------

#ifdef Q_OS_WIN32
	setWindowIcon(QIcon(":/res/bible.ico"));
#else
	setWindowIcon(QIcon(":/res/bible_48.png"));
#endif

	// --------------------

#ifdef WORKAROUND_QTBUG_13768
// The following is supposed to be another workaround for QTBUG-13768
//	m_pSplitter->setStyleSheet("QSplitterHandle:hover {}  QSplitter::handle:hover { background-color: palette(highlight); }");
	m_pSplitter->handle(1)->setAttribute(Qt::WA_Hover);		// Work-Around QTBUG-13768
	if (m_pSplitter->count() > 2) m_pSplitter->handle(2)->setAttribute(Qt::WA_Hover);
	if (m_pSplitterDictionary->count() > 1) m_pSplitterDictionary->handle(1)->setAttribute(Qt::WA_Hover);
	setStyleSheet("QSplitter::handle:hover { background-color: palette(highlight); }");
#endif

	CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode = m_pSearchResultWidget->viewMode();
	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode = m_pSearchResultWidget->displayMode();
	CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode = m_pSearchResultWidget->treeMode();
	bool bShowMissingLeafs = m_pSearchResultWidget->showMissingLeafs();

	// --------------------

	// --- File Menu
	QMenu *pFileMenu = ui.menuBar->addMenu(tr("&File", "MainMenu"));

	pAction = pFileMenu->addAction(QIcon(":/res/file-new-icon2.png"), tr("&New Search", "MainMenu"), this, SLOT(en_NewSearch()));
	pAction->setStatusTip(tr("Clear All Search Phrases, Search Scope, and Search Within Settings, and Begin New Search", "MainMenu"));
	pAction->setToolTip(tr("Clear All Search Phrases, Search Scope, and Search Within Settings, and Begin New Search", "MainMenu"));
	ui.mainToolBar->addAction(pAction);

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	pAction = pFileMenu->addAction(QIcon(":/res/open-file-icon3.png"), tr("L&oad Search File...", "MainMenu"), this, SLOT(en_OpenSearch()), QKeySequence(Qt::CTRL | Qt::Key_O));
	pAction->setStatusTip(tr("Load Search Phrases from a previously saved King James Search File", "MainMenu"));
	pAction->setToolTip(tr("Load Search Phrases from a previously saved King James Search File", "MainMenu"));
	ui.mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/save-file-icon3.png"), tr("&Save Search File...", "MainMenu"), this, SLOT(en_SaveSearch()), QKeySequence(Qt::CTRL | Qt::Key_S));
	pAction->setStatusTip(tr("Save current Search Phrases to a King James Search File", "MainMenu"));
	pAction->setToolTip(tr("Save current Search Phrases to a King James Search File", "MainMenu"));
	ui.mainToolBar->addAction(pAction);
#endif

	pFileMenu->addSeparator();
	ui.mainToolBar->addSeparator();

	pAction = pFileMenu->addAction(QIcon(":/res/edit_clear.png"), tr("Cl&ear Search Phrases", "MainMenu"), this, SLOT(en_ClearSearchPhrases()));
	QList<QKeySequence> lstShortcuts;
	lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_E));
	lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
	pAction->setShortcuts(lstShortcuts);
	pAction->setStatusTip(tr("Clear All Search Phrases, but keep Search Scope and Search Within Settings", "MainMenu"));
	pAction->setToolTip(tr("Clear All Search Phrases, but keep Search Scope and Search Within Settings", "MainMenu"));
	ui.mainToolBar->addAction(pAction);

	pFileMenu->addSeparator();

	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit", "MainMenu"), g_pMyApplication.data(), SLOT(closeAllCanOpeners()), QKeySequence(Qt::CTRL | Qt::Key_Q));
	pAction->setStatusTip(tr("Exit the King James Pure Bible Search Application", "MainMenu"));
	pAction->setToolTip(tr("Exit Application", "MainMenu"));
	pAction->setMenuRole(QAction::QuitRole);
	pAction->setEnabled(g_pMyApplication->canQuit());
	connect(g_pMyApplication.data(), SIGNAL(canQuitChanged(bool)), pAction, SLOT(setEnabled(bool)));

	// --- Edit Menu
	connect(m_pBrowserWidget, SIGNAL(activatedBrowser(bool)), this, SLOT(en_activatedBrowser(bool)));
	connect(m_pSearchResultWidget, SIGNAL(activatedSearchResults()), this, SLOT(en_activatedSearchResults()));
	connect(m_pSearchSpecWidget, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)), this, SLOT(en_activatedPhraseEditor(const CPhraseLineEdit *)));
#if defined(USING_DICTIONARIES) && !defined(IS_CONSOLE_APP)
	if (m_pDictionaryWidget != nullptr)
		connect(m_pDictionaryWidget, SIGNAL(activatedDictionary(bool)), this, SLOT(en_activatedDictionary(bool)));
#endif

	// --- View Menu
	m_pViewMenu = ui.menuBar->addMenu(tr("&View", "MainMenu"));

	QMenu *pViewToolbarsMenu = m_pViewMenu->addMenu(tr("&Toolbars", "MainMenu"));
	pViewToolbarsMenu->addAction(ui.mainToolBar->toggleViewAction());
	ui.mainToolBar->toggleViewAction()->setStatusTip(tr("Show/Hide Main Tool Bar", "MainMenu"));
	pViewToolbarsMenu->addAction(ui.browserNavigationToolBar->toggleViewAction());
	ui.browserNavigationToolBar->toggleViewAction()->setStatusTip(tr("Show/Hide the Scripture Browser Navigation Tool Bar", "MainMenu"));
	if (ui.usernotesToolBar != nullptr) {
		pViewToolbarsMenu->addAction(ui.usernotesToolBar->toggleViewAction());
		ui.usernotesToolBar->toggleViewAction()->setStatusTip(tr("Show/Hide Highlighter/Notes/References Tool Bar", "MainMenu"));
	}

	pAction = m_pViewMenu->addSeparator();
	pAction->setText(tr("View Mode", "MainMenu") + QString(" (%1):").arg(QKeySequence(Qt::Key_F6).toString(QKeySequence::NativeText)));
	pAction = m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());
	pAction->setText(tr("View Mode", "MainMenu") + QString(" (%1):").arg(QKeySequence(Qt::Key_F6).toString(QKeySequence::NativeText)));

	m_pActionGroupViewMode = new QActionGroup(this);
	m_pActionGroupViewMode->setExclusive(true);

	pAction = m_pActionGroupViewMode->addAction(tr("View S&earch Results", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_SEARCH_RESULTS);
	pAction->setStatusTip(tr("View Search Results from Search Phrases", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_SEARCH_RESULTS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupViewMode->addAction(tr("View E&xcluded Search Results", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED);
	pAction->setStatusTip(tr("View Excluded Search Results from Search Phrases", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	pAction = m_pActionGroupViewMode->addAction(tr("View &Highlighters", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_HIGHLIGHTERS);
	pAction->setStatusTip(tr("View Highlighted Passages", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_HIGHLIGHTERS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupViewMode->addAction(tr("View &Notes", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_USERNOTES);
	pAction->setStatusTip(tr("View All Notes", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_USERNOTES);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupViewMode->addAction(tr("View Cross Re&ferences", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VVME_CROSSREFS);
	pAction->setStatusTip(tr("View Cross References", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nViewMode == CVerseListModel::VVME_CROSSREFS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);
#endif

	connect(m_pActionGroupViewMode, SIGNAL(triggered(QAction*)), this, SLOT(en_viewModeChange(QAction*)));

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::Key_F6));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_nextViewMode()));

	pAction = m_pViewMenu->addSeparator();
	pAction->setText(tr("Tree Mode", "MainMenu") + QString(" (%1):").arg(QKeySequence(Qt::Key_F7).toString(QKeySequence::NativeText)));
	pAction = m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());
	pAction->setText(tr("Tree Mode", "MainMenu") + QString(" (%1):").arg(QKeySequence(Qt::Key_F7).toString(QKeySequence::NativeText)));

	m_pActionGroupTreeMode = new QActionGroup(this);
	m_pActionGroupTreeMode->setExclusive(true);

	pAction = m_pActionGroupTreeMode->addAction(tr("View as &List", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VTME_LIST);
	pAction->setStatusTip(tr("Show Search Results as a List", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nTreeMode == CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupTreeMode->addAction(tr("View as Tree by &Book", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VTME_TREE_BOOKS);
	pAction->setStatusTip(tr("Show Search Results in a Tree by Book", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nTreeMode == CVerseListModel::VTME_TREE_BOOKS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupTreeMode->addAction(tr("View as Tree by Book/&Chapter", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VTME_TREE_CHAPTERS);
	pAction->setStatusTip(tr("Show Search Results in a Tree by Book and Chapter", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nTreeMode == CVerseListModel::VTME_TREE_CHAPTERS);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	connect(m_pActionGroupTreeMode, SIGNAL(triggered(QAction*)), this, SLOT(en_treeModeChange(QAction*)));

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::Key_F7));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_nextTreeMode()));

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());

	m_pActionShowMissingLeafs = m_pViewMenu->addAction(tr("View &Missing Books/Chapters", "MainMenu"), this, SLOT(en_viewShowMissingsLeafs()), QKeySequence(Qt::Key_F4));
	m_pActionShowMissingLeafs->setStatusTip(tr("Show Missing Books and/or Chapters in the Tree (ones that had no matching Search Results)", "MainMenu"));
	m_pActionShowMissingLeafs->setCheckable(true);
	m_pActionShowMissingLeafs->setChecked(bShowMissingLeafs);
	m_pActionShowMissingLeafs->setEnabled(nTreeMode != CVerseListModel::VTME_LIST);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), m_pActionShowMissingLeafs);

	m_pActionExpandAll = m_pViewMenu->addAction(tr("E&xpand All", "MainMenu"), m_pSearchResultWidget, SIGNAL(expandAll()), QKeySequence(Qt::CTRL | Qt::Key_U));
	m_pActionExpandAll->setStatusTip(tr("Expand all tree nodes in Search Results (Warning: May be slow if there are a lot of search results!)", "MainMenu"));
	m_pActionExpandAll->setEnabled(false);
	connect(m_pSearchResultWidget, SIGNAL(canExpandAll(bool)), m_pActionExpandAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), m_pActionExpandAll);

	m_pActionCollapseAll = m_pViewMenu->addAction(tr("Collap&se All", "MainMenu"), m_pSearchResultWidget, SIGNAL(collapseAll()), QKeySequence(Qt::CTRL | Qt::Key_I));
	m_pActionCollapseAll->setStatusTip(tr("Collapse all tree nodes in Search Results", "MainMenu"));
	m_pActionCollapseAll->setEnabled(false);
	connect(m_pSearchResultWidget, SIGNAL(canCollapseAll(bool)), m_pActionCollapseAll, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), m_pActionCollapseAll);

	pAction = m_pViewMenu->addSeparator();
	pAction->setText(tr("Display Mode", "MainMenu") + QString(" (%1):").arg(QKeySequence(Qt::Key_F8).toString(QKeySequence::NativeText)));
	pAction = m_pSearchResultWidget->getLocalEditMenu()->insertSeparator(m_pSearchResultWidget->getLocalEditMenuInsertionPoint());
	pAction->setText(tr("Display Mode", "MainMenu") + QString(" (%1):").arg(QKeySequence(Qt::Key_F8).toString(QKeySequence::NativeText)));

	m_pActionGroupDisplayMode = new QActionGroup(this);
	m_pActionGroupDisplayMode->setExclusive(true);

	pAction = m_pActionGroupDisplayMode->addAction(tr("View &References Only", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VDME_HEADING);
	pAction->setStatusTip(tr("Show Search Results Verse References Only", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nDisplayMode == CVerseListModel::VDME_HEADING);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	pAction = m_pActionGroupDisplayMode->addAction(tr("View Verse &Preview", "MainMenu"));
	m_pViewMenu->addAction(pAction);
	pAction->setData(CVerseListModel::VDME_RICHTEXT);
	pAction->setStatusTip(tr("Show Search Results as Rich Text Verse Preview", "MainMenu"));
	pAction->setCheckable(true);
	pAction->setChecked(nDisplayMode == CVerseListModel::VDME_RICHTEXT);
	m_pSearchResultWidget->getLocalEditMenu()->insertAction(m_pSearchResultWidget->getLocalEditMenuInsertionPoint(), pAction);

	connect(m_pActionGroupDisplayMode, SIGNAL(triggered(QAction*)), this, SLOT(en_displayModeChange(QAction*)));

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::Key_F8));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_nextDisplayMode()));

	m_pViewMenu->addSeparator();
	m_pSearchResultWidget->getLocalEditMenu()->addSeparator();			// Put details at the end

	m_pActionViewDetails = m_pViewMenu->addAction(QIcon(":/res/Windows-View-Detail-icon-48.png"), tr("View &Details...", "MainMenu"), this, SLOT(en_viewDetails()), QKeySequence(Qt::CTRL | Qt::Key_D));
	m_pActionViewDetails->setStatusTip(tr("View Passage Details", "MainMenu"));
	m_pActionViewDetails->setEnabled(false);
	connect(this, SIGNAL(canShowDetails(bool)), m_pActionViewDetails, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionViewDetails);

#ifdef USE_GEMATRIA
	m_pActionViewGematria = m_pViewMenu->addAction(QIcon(":/res/Gematria-icon-2.jpg"), tr("View &Gematria...", "MainMenu"), this, SLOT(en_viewGematria()));
	m_pActionViewGematria->setStatusTip(tr("View Passage Gematria", "MainMenu"));
	m_pActionViewGematria->setEnabled(false);
	connect(this, SIGNAL(canShowGematria(bool)), m_pActionViewGematria, SLOT(setEnabled(bool)));
	m_pSearchResultWidget->getLocalEditMenu()->addAction(m_pActionViewGematria);
#endif

	// --- Navigate Menu
	QMenu *pNavMenu = ui.menuBar->addMenu(tr("&Navigate", "MainMenu"));

	pAction = pNavMenu->addAction(tr("Beginning of Bible", "MainMenu"), m_pBrowserWidget, SLOT(en_Bible_Beginning()), QKeySequence(Qt::ALT | Qt::Key_Home));
	pAction->setStatusTip(tr("Goto the very Beginning of the Bible", "MainMenu"));
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	pAction = pNavMenu->addAction(tr("Ending of Bible", "MainMenu"), m_pBrowserWidget, SLOT(en_Bible_Ending()), QKeySequence(Qt::ALT | Qt::Key_End));
	pAction->setStatusTip(tr("Goto the very End of the Bible", "MainMenu"));
	connect(pAction, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionBookBackward = pNavMenu->addAction(tr("Book Backward", "MainMenu"), m_pBrowserWidget, SLOT(en_Book_Backward()), QKeySequence(Qt::CTRL | Qt::Key_PageUp));
	m_pActionBookBackward->setStatusTip(tr("Move Backward one Book", "MainMenu"));
	connect(m_pActionBookBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionBookForward = pNavMenu->addAction(tr("Book Forward", "MainMenu"), m_pBrowserWidget, SLOT(en_Book_Forward()), QKeySequence(Qt::CTRL | Qt::Key_PageDown));
	m_pActionBookForward->setStatusTip(tr("Move Forward one Book", "MainMenu"));
	connect(m_pActionBookForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionChapterBackward = pNavMenu->addAction(tr("Chapter Backward", "MainMenu"), m_pBrowserWidget, SLOT(en_ChapterBackward()), QKeySequence(Qt::ALT | Qt::Key_PageUp));
	m_pActionChapterBackward->setStatusTip(tr("Move Backward one Chapter", "MainMenu"));
	connect(m_pActionChapterBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionChapterForward = pNavMenu->addAction(tr("Chapter Forward", "MainMenu"), m_pBrowserWidget, SLOT(en_ChapterForward()), QKeySequence(Qt::ALT | Qt::Key_PageDown));
	m_pActionChapterForward->setStatusTip(tr("Move Forward one Chapter", "MainMenu"));
	connect(m_pActionChapterForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	connect(m_pBrowserWidget, SIGNAL(en_gotoIndex(const TPhraseTag &)), this, SLOT(en_gotoIndex(const TPhraseTag &)));

	pNavMenu->addSeparator();

	m_pActionNavBackward = new QAction(QIcon(":/res/Nav3_Arrow_Left.png"), tr("History &Backward", "MainMenu"), this);
	m_pActionNavBackward->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
	m_pActionNavBackward->setStatusTip(tr("Go Backward in Navigation History", "MainMenu"));
	ui.browserNavigationToolBar->addAction(m_pActionNavBackward);
	connect(m_pBrowserWidget, SIGNAL(backwardAvailable(bool)), m_pActionNavBackward, SLOT(setEnabled(bool)));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget, SIGNAL(backward()));
	connect(m_pActionNavBackward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionNavBackward->setEnabled(m_pBrowserWidget->isBackwardAvailable());
	pNavMenu->addAction(m_pActionNavBackward);

	m_pActionNavForward = new QAction(QIcon(":/res/Nav3_Arrow_Right.png"), tr("History &Forward", "MainMenu"), this);
	m_pActionNavForward->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));
	m_pActionNavForward->setStatusTip(tr("Go Forward in Navigation History", "MainMenu"));
	ui.browserNavigationToolBar->addAction(m_pActionNavForward);
	connect(m_pBrowserWidget, SIGNAL(forwardAvailable(bool)), m_pActionNavForward, SLOT(setEnabled(bool)));
	connect(m_pActionNavForward, SIGNAL(triggered()), m_pBrowserWidget, SIGNAL(forward()));
	connect(m_pActionNavForward, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
	m_pActionNavForward->setEnabled(m_pBrowserWidget->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavForward);

	m_pActionNavHome = pNavMenu->addAction(QIcon(":/res/go_home.png"), tr("History &Home", "MainMenu"), m_pBrowserWidget, SIGNAL(home()), QKeySequence(Qt::ALT | Qt::Key_Up));
	m_pActionNavHome->setStatusTip(tr("Jump to History Home Passage", "MainMenu"));
	m_pActionNavHome->setEnabled(m_pBrowserWidget->isBackwardAvailable() ||
									m_pBrowserWidget->isForwardAvailable());

	m_pActionNavClear = new QAction(QIcon(":/res/Actions-edit-clear-icon-128.png"), tr("&Clear Navigation History", "MainMenu"), this);
	m_pActionNavClear->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Delete));
	m_pActionNavClear->setStatusTip(tr("Clear All Passage Navigation History", "MainMenu"));
	ui.browserNavigationToolBar->addAction(m_pActionNavClear);
	connect(m_pActionNavClear, SIGNAL(triggered()), this, SLOT(en_clearBrowserHistory()));
	m_pActionNavClear->setEnabled(m_pBrowserWidget->isBackwardAvailable() ||
									m_pBrowserWidget->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavClear);

	connect(m_pBrowserWidget, SIGNAL(historyChanged()), this, SLOT(en_browserHistoryChanged()));

	pNavMenu->addSeparator();
	ui.browserNavigationToolBar->addSeparator();

	m_pActionRefresh = new QAction(QIcon(":/res/refresh-128.png"), tr("&Refresh Scripture Browser", "MainMenu"), this);
	m_pActionRefresh->setShortcut(QKeySequence(Qt::Key_F5));
	m_pActionRefresh->setStatusTip(tr("Refresh/Reload the Current Passage in the Scripture Browser", "MainMenu"));
	ui.browserNavigationToolBar->addAction(m_pActionRefresh);
	connect(m_pActionRefresh, SIGNAL(triggered()), m_pBrowserWidget, SIGNAL(rerender()));
	pNavMenu->addAction(m_pActionRefresh);

//	m_pActionJump = new QAction(QIcon(":/res/go_jump2.png"), tr("Passage &Navigator", "MainMenu"), this);
	m_pActionJump = new QAction(QIcon(":/res/green_arrow.png"), tr("Passage &Navigator", "MainMenu"), this);
	m_pActionJump->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
	m_pActionJump->setStatusTip(tr("Display the Passage Navigator Widget", "MainMenu"));
	ui.browserNavigationToolBar->addAction(m_pActionJump);
	connect(m_pActionJump, SIGNAL(triggered()), this, SLOT(en_PassageNavigatorTriggered()));
	pNavMenu->addAction(m_pActionJump);

//	pAction = pNavMenu->addAction(QIcon(":/res/Games-icon-dice-128.png"), tr("Goto Ran&dom Passage", "MainMenu"));
	pAction = pNavMenu->addAction(QIcon(":/res/random_128.png"), tr("Goto Ran&dom Passage", "MainMenu"));
	pAction->setStatusTip(tr("Goto a Random Bible Passage", "MainMenu"));
	pAction->setToolTip(tr("Goto Random Passage", "MainMenu"));
	ui.browserNavigationToolBar->addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_gotoRandomPassage()));

	ui.browserNavigationToolBar->addSeparator();
	ui.browserNavigationToolBar->addAction(m_pActionViewDetails);
#ifdef USE_GEMATRIA
	ui.browserNavigationToolBar->addAction(m_pActionViewGematria);
#endif

#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
	// --- Settings Menu
	QMenu *pSettingsMenu = ui.menuBar->addMenu(tr("Se&ttings", "MainMenu"));

	pAction = pSettingsMenu->addAction(QIcon(":/res/Settings-icon2-128.png"), tr("Configure...", "MainMenu"), this, SLOT(en_Configure()));
	pAction->setStatusTip(tr("Configure the King James Pure Bible Search Application", "MainMenu"));
	pAction->setToolTip(tr("Configure King James Pure Bible Search", "MainMenu"));
	pAction->setMenuRole(QAction::PreferencesRole);

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F1));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchGeneralSettingsConfig()));

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F2));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchCopyOptionsConfig()));

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F3));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchTextColorAndFontsConfig()));

#if !defined(VNCSERVER) && !defined(EMSCRIPTEN)
	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F4));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchNotesFileSettingsConfig()));
#endif

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F5));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchBibleDatabaseConfig()));

#if defined(USING_DICTIONARIES)
	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F6));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchDictDatabaseConfig()));
#endif

	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F7));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchLocaleSettingsConfig()));

#ifndef VNCSERVER
	pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F8));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), this, SLOT(en_LaunchTTSOptionsConfig()));
#endif

#endif	// (!EMSCRIPTEN && !IS_CONSOLE_APP) || Q_OS_WASM


	// --- Window Menu
	QMenu *pWindowMenu = ui.menuBar->addMenu(tr("&Window", "MainMenu"));

	m_pActionBibleDatabasesList = new QAction(QIcon(":/res/gnome_window_new.png"), tr("&New Search Window...", "MainMenu"), this);
	m_pActionBibleDatabasesList->setStatusTip(tr("Create a New King James Pure Bible Search Window", "MainMenu"));
	m_pActionBibleDatabasesList->setToolTip(tr("Create New Search Window", "MainMenu"));
	if (TBibleDatabaseList::availableBibleDatabases().size() == 1) {
		// If we only have a single available database, treat it in the old fashion of a single "New Search Window" button:
		m_pActionBibleDatabasesList->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
		connect(m_pActionBibleDatabasesList, SIGNAL(triggered()), this, SLOT(en_NewCanOpener()));
	} else {
		// Otherwise, we'll setup a submenu:
		m_pActionBibleDatabasesList->setMenu(new QMenu);			// The action will take ownership via setOverrideMenuAction()
		en_updateBibleDatabasesList();
		//	Do the update via a QueuedConnection so that KJVCanOpeners coming/going during opening other search windows
		//	that have to open new Bible Databases won't crash if the menu that was triggering it gets yanked out from under it:
		connect(TBibleDatabaseList::instance(), SIGNAL(changedBibleDatabaseList()), this, SLOT(en_updateBibleDatabasesList()), Qt::QueuedConnection);
	}
	pWindowMenu->addAction(m_pActionBibleDatabasesList);

	pAction = pWindowMenu->addAction(QIcon(":/res/window_app_list_close.png"), tr("&Close this Search Window", "MainMenu"), this, SLOT(close()), QKeySequence(Qt::CTRL | Qt::Key_W));
	pAction->setStatusTip(tr("Close this King James Pure Bible Search Window", "MainMenu"));
	pAction->setToolTip(tr("Close this Search Window", "MainMenu"));

	pWindowMenu->addSeparator();

	m_pActionSearchWindowList = new QAction(tr("&Open Search Windows", "MainMenu"), this);
	m_pActionSearchWindowList->setStatusTip(tr("List of Open Search Windows", "MainMenu"));
	m_pActionSearchWindowList->setToolTip(tr("Open Search Window List", "MainMenu"));
	m_pActionSearchWindowList->setMenu(new QMenu);			// The action will take ownership via setOverrideMenuAction()
	pWindowMenu->addAction(m_pActionSearchWindowList);
	// Note: This action's menu will be automatically updated by our application object
	//	Do this via a QueuedConnection so that KJVCanOpeners coming/going during opening other search windows
	//	won't crash if the menu that was triggering it gets yanked out from under it:
	connect(g_pMyApplication.data(), SIGNAL(updateSearchWindowList()), this, SLOT(en_updateSearchWindowList()), Qt::QueuedConnection);

	// --- Help Menu
	ui.mainToolBar->addSeparator();

	QMenu *pHelpMenu = ui.menuBar->addMenu(tr("&Help", "MainMenu"));

#ifndef VNCSERVER
	pAction = pHelpMenu->addAction(QIcon(":/res/help_book.png"), tr("&Help", "MainMenu"), this, SLOT(en_HelpManual()), QKeySequence(Qt::SHIFT | Qt::Key_F1));
	pAction->setStatusTip(tr("Display the Users Manual", "MainMenu"));

	pAction = pHelpMenu->addAction(QIcon(":/res/package_network-128.png"), tr("Goto PureBibleSearch.com...", "MainMenu"), this, SLOT(en_PureBibleSearchDotCom()), QKeySequence(Qt::Key_F2));
	pAction->setStatusTip(tr("Open a Web Browser and Navigate to www.PureBibleSearch.com", "MainMenu"));
	pAction->setToolTip(tr("Goto www.PureBibleSearch.com Home Page", "MainMenu"));
	ui.mainToolBar->addAction(pAction);
#endif

	m_pActionAbout = new QAction(QIcon(":/res/help_icon1.png"), tr("About...", "MainMenu"), this);
	m_pActionAbout->setShortcut(QKeySequence(Qt::Key_F1));
	m_pActionAbout->setStatusTip(tr("About the King James Pure Bible Search", "MainMenu"));
	m_pActionAbout->setToolTip(tr("About the King James Pure Bible Search...", "MainMenu"));
	m_pActionAbout->setMenuRole(QAction::AboutRole);
	connect(m_pActionAbout, SIGNAL(triggered()), this, SLOT(en_HelpAbout()));
	ui.mainToolBar->addAction(m_pActionAbout);
	pHelpMenu->addAction(m_pActionAbout);

	// -------------------- Quick Activate:

	for (int ndx=0; ndx<NUM_QUICK_ACTIONS; ++ndx) {
		m_lstpQuickActivate.append(new QAction(this));
		m_lstpQuickActivate.at(ndx)->setShortcut(QKeySequence(Qt::CTRL | static_cast<Qt::Key>(Qt::Key_0 + ndx)));
		connect(m_lstpQuickActivate.at(ndx), SIGNAL(triggered()), this, SLOT(en_QuickActivate()));
	}
	addActions(m_lstpQuickActivate);

	// -------------------- Search Phrase Widgets:

	connect(m_pSearchSpecWidget, SIGNAL(closingSearchPhrase(CSearchPhraseEdit*)), this, SLOT(en_closingSearchPhrase(CSearchPhraseEdit*)));
	connect(m_pSearchSpecWidget, SIGNAL(phraseChanged(CSearchPhraseEdit *)), this, SLOT(en_phraseChanged(CSearchPhraseEdit *)));
	connect(m_pSearchSpecWidget, SIGNAL(copySearchPhraseSummary()), this, SLOT(en_copySearchPhraseSummary()));
	connect(m_pSearchSpecWidget, SIGNAL(triggeredSearchWithinGotoIndex(const CRelIndex &)), this, SLOT(en_triggeredSearchWithinGotoIndex(const CRelIndex &)));

	// -------------------- Search Spec:

	connect(m_pSearchSpecWidget, SIGNAL(changedSearchSpec(const CSearchResultsData &)), this, SLOT(en_changedSearchSpec(const CSearchResultsData &)));

	// -------------------- Search Results List View:

	connect(m_pSearchResultWidget, SIGNAL(searchResultActivated(const QModelIndex &)), this, SLOT(en_SearchResultActivated(const QModelIndex &)));
	connect(m_pSearchResultWidget, SIGNAL(gotoIndex(const TPhraseTag &)), m_pBrowserWidget, SLOT(gotoIndex(const TPhraseTag &)));
	connect(m_pSearchResultWidget, SIGNAL(setDetailsEnable()), this, SLOT(setDetailsEnable()));
	connect(m_pSearchResultWidget, SIGNAL(setGematriaEnable()), this, SLOT(setGematriaEnable()));

	// -------------------- Search Results to Search Spec pass-through:

	connect(m_pSearchResultWidget, SIGNAL(searchResultsReady()), m_pSearchSpecWidget, SLOT(en_searchResultsReady()));

	// -------------------- Scripture Browser:

#if defined(USING_DICTIONARIES) && !defined(IS_CONSOLE_APP)
	if (m_pDictionaryWidget != nullptr) {
		connect(m_pBrowserWidget, SIGNAL(wordUnderCursorChanged(CBibleDatabasePtr,const TPhraseTag &)), m_pDictionaryWidget, SLOT(setWord(CBibleDatabasePtr,const TPhraseTag &)));
		connect(m_pDictionaryWidget, SIGNAL(gotoPassageReference(const QString &)), m_pBrowserWidget, SLOT(gotoPassageReference(const QString &)));
	}
#endif

	// -------------------- UserNoteEditor Dialog:
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	m_pUserNoteEditorDlg = new CNoteEditDlg(m_pBibleDatabase, g_pUserNotesDatabase, this);
	m_pUserNoteEditorDlg->setModal(true);
	connect(m_pActionUserNoteEditor, SIGNAL(triggered()), this, SLOT(en_userNoteEditorTriggered()));
#endif


	// -------------------- CrossRefsEditor Dialog:
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	m_pCrossRefsEditorDlg = new CCrossRefEditDlg(m_pBibleDatabase, g_pUserNotesDatabase, this);
	m_pCrossRefsEditorDlg->setModal(true);
	connect(m_pActionCrossRefsEditor, SIGNAL(triggered()), this, SLOT(en_crossRefsEditorTriggered()));
#endif


	// -------------------- Persistent Settings:
	// Do this as a singleShot to delay it until after we get out of the constructor.
	//		This is necessary because that function can cause modal message boxes, etc,
	//		to be displayed (such as missing/broken notes file).  And we will assert
	//		in either the Search Results Tree or the Scripture Browser trying to call
	//		findParentCanOpener() when the parent pointers haven't been set yet:
#if QT_VERSION >= 0x050400
	bool bAppRestarting = g_pMyApplication->areRestarting();
	BROWSER_DISPLAY_MODE_ENUM nBrowserDisplayMode = CPersistentSettings::instance()->browserDisplayMode();
	if (bAppRestarting) {
		// We should have 2 CanOpeners at this point : the first is
		//	the original CanOpener and the second is this CanOpener
		Q_ASSERT(g_pMyApplication->canOpeners().size() >= 1);
		nBrowserDisplayMode = g_pMyApplication->canOpeners().at(0)->m_pBrowserWidget->browserDisplayMode();
	}
	QTimer::singleShot(0, this, [this, bAppRestarting, nBrowserDisplayMode]() {
		// When restarting the application, tell restorePersistentSettings that
		//	we are creating the FirstCanOpener so that the geometry is restored
		//	back to the startup conditions for it.  However, preserve and restore
		//	the DisplayMode so that we maintain it across the restart, rather than
		//	returning to the initial mode from the persistent settings:
		restorePersistentSettings(bAppRestarting);
		if (bAppRestarting) {
			QTimer::singleShot(1, this, [this, nBrowserDisplayMode]() {
				m_pBrowserWidget->setBrowserDisplayMode(nBrowserDisplayMode);
			} );
		}
	});
#else
	QTimer::singleShot(0, this, SLOT(restorePersistentSettings()));
#endif
}

CKJVCanOpener::~CKJVCanOpener()
{

}

CDictionaryDatabasePtr CKJVCanOpener::dictionaryDatabase() const
{
#if defined(USING_DICTIONARIES) && !defined(IS_CONSOLE_APP)
	if (m_pDictionaryWidget != nullptr) {
		return m_pDictionaryWidget->dictionaryDatabase();
	} else {
		return CDictionaryDatabasePtr();
	}
#else
	return CDictionaryDatabasePtr();
#endif
}

void CKJVCanOpener::initialize()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Default for unset key (Genesis 1 or start of Bible for partial databases):
	CRelIndex ndxBibleStart = m_pBibleDatabase->calcRelIndex(CRelIndex(1, 1, 0, 0), CBibleDatabase::RIME_Absolute);
	ndxBibleStart.setVerse(0);		// Select chapter only so browser will display headings
	ndxBibleStart.setWord(0);
	TPhraseTag tag(ndxBibleStart, 0);

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());

		settings.beginGroup(constrBrowserViewGroup);
		// Read last location : Default initial location is Genesis 1
		CRelIndex ndxLastRef = CRelIndex(settings.value(constrLastReferenceKey, tag.relIndex().asAnchor()).toString());
		unsigned int nCount = settings.value(constrLastSelectionSizeKey, 0).toUInt();
		if ((ndxLastRef.isSet()) &&
			(m_pBibleDatabase->NormalizeIndex(ndxLastRef) != 0)) {			// Make sure the reference is part of our database's versification/content
			tag = TPhraseTag(ndxLastRef, nCount);
		}
		settings.endGroup();

		setWindowTitle(windowTitle() + " - " + m_pBibleDatabase->description());
	} else {
		setWindowTitle(windowTitle() + " - " + m_pBibleDatabase->description() + " (" + tr("Stealth Mode", "KJVCanOpener") + ")");
	}
#else
	setWindowTitle(windowTitle() + " - " + m_pBibleDatabase->description() + " (" + tr("Lite Version", "KJVCanOpener") + ")");
#endif

	// If there is no selection to highlight, default to the first sub-entity
	//		of the index specified:
	if (tag.count() == 0) {
		tag.relIndex() = CRelIndex::navigationIndexFromLogicalIndex(tag.relIndex());
	}

	m_pBrowserWidget->gotoIndex(tag);
}

void CKJVCanOpener::savePersistentSettings(bool bSaveLastSearchOnly)
{
	if (CPersistentSettings::instance()->settings() == nullptr) return;
	QSettings &settings(*CPersistentSettings::instance()->settings());

	if (!bSaveLastSearchOnly) {
		// Main App and Toolbars RestoreState:
#if defined(PRESERVE_MAINWINDOW_GEOMETRY) || defined(PRESERVE_MAINWINDOW_STATE)
		settings.beginGroup(constrMainAppRestoreStateGroup);
#ifdef PRESERVE_MAINWINDOW_GEOMETRY
		settings.setValue(constrGeometryKey, saveGeometry());
#endif
#ifdef PRESERVE_MAINWINDOW_STATE
		settings.setValue(constrWindowStateKey, saveState(KJVAPP_REGISTRY_VERSION));
#endif
		settings.endGroup();
#endif

		// Main App General Settings:
		settings.beginGroup(constrMainAppControlGroup);
		settings.setValue(constrInvertTextBrightnessKey, CPersistentSettings::instance()->invertTextBrightness());
		settings.setValue(constrTextBrightnessKey, CPersistentSettings::instance()->textBrightness());
		settings.setValue(constrAdjustDialogElementBrightnessKey, CPersistentSettings::instance()->adjustDialogElementBrightness());
		settings.endGroup();

		// Main App Bible Database Settings:
		settings.beginGroup(constrMainAppBibleDatabaseGroup);
		settings.setValue(constrDatabaseUUIDKey, CPersistentSettings::instance()->mainBibleDatabaseUUID());
		settings.endGroup();

		// Main App Dictionary Database Settings:
		settings.beginGroup(constrMainAppDictDatabaseGroup);
		settings.setValue(constrDatabaseUUIDKey, CPersistentSettings::instance()->mainDictDatabaseUUID());
		settings.endGroup();

		// Colors:
		settings.beginGroup(constrColorsGroup);
		if (CPersistentSettings::instance()->colorWordsOfJesus().isValid()) {
			settings.setValue(constrWordsOfJesusColorKey, CPersistentSettings::instance()->colorWordsOfJesus().name());
		} else {
			settings.setValue(constrWordsOfJesusColorKey, "");
		}
		settings.setValue(constrSearchResultsColorKey, CPersistentSettings::instance()->colorSearchResults().name());
		settings.setValue(constrCursorTrackerColorKey, CPersistentSettings::instance()->colorCursorFollow().name());
		settings.endGroup();

#ifdef PRESERVE_MAINWINDOW_SPLITTER_STATE
		// Splitter:
		if (m_pSplitter != nullptr) {
			settings.beginGroup(constrSplitterRestoreStateGroup);
			settings.setValue(constrStateVersionKey, PS_SPLITTER_VERSION);
			settings.setValue(constrWindowStateKey, m_pSplitter->saveState());
			settings.endGroup();
		}

		// Splitter Dictionary:
		if ((m_pDictionaryWidget != nullptr) && (m_pSplitterDictionary != nullptr)) {
			settings.beginGroup(constrSplitterDictionaryRestoreStateGroup);
			settings.setValue(constrStateVersionKey, PS_SPLITTER_VERSION);
			settings.setValue(constrWindowStateKey, m_pSplitterDictionary->saveState());
			settings.endGroup();
		}
#endif

		// User Notes Database:
		Q_ASSERT(!g_pUserNotesDatabase.isNull());
		settings.beginGroup(constrUserNotesDatabaseGroup);
		settings.setValue(constrFilePathNameKey, g_pUserNotesDatabase->filePathName());
		settings.setValue(constrKeepBackupKey, CPersistentSettings::instance()->keepNotesBackup());
		settings.setValue(constrBackupFilenamePostfixKey, CPersistentSettings::instance()->notesBackupFilenamePostfix());
		settings.setValue(constrAutoSaveTimeKey, CPersistentSettings::instance()->notesFileAutoSaveTime());
		settings.setValue(constrDefaultNoteBackgroundColorKey, CPersistentSettings::instance()->colorDefaultNoteBackground().name());
		settings.endGroup();

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
		m_pUserNoteEditorDlg->writeSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrUserNoteEditorGroup));
		m_pCrossRefsEditorDlg->writeSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrCrossRefsEditorGroup));
#endif

		// Highlighter Tool Bar:
		if (m_pHighlighterButtons != nullptr) {
			settings.beginWriteArray(groupCombine(constrColorsGroup, constrColorsHighlightersSubgroup));
			settings.remove("");
			for (int ndxColor = 0; ndxColor < m_pHighlighterButtons->count(); ++ndxColor) {
				settings.setArrayIndex(ndxColor);
				settings.setValue(constrHighlighterNameKey, m_pHighlighterButtons->highlighter(ndxColor));
			}
			settings.endArray();
		}

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
		settings.remove(constrFontKey);		// Remove deprecated font value
		settings.setValue(constrFontNameKey, CPersistentSettings::instance()->fontSearchResults().family());
		settings.setValue(constrFontSizeKey, CPersistentSettings::instance()->fontSearchResults().pointSize());

		settings.setValue(constrAutoExpandSearchResultsTreeViewKey, CPersistentSettings::instance()->autoExpandSearchResultsTree());
		settings.setValue(constrHideNotFoundInStatisticsKey, CPersistentSettings::instance()->hideNotFoundInStatistcs());
		settings.setValue(constrShowHighlightersInSearchResultsKey, m_pSearchResultWidget->showHighlightersInSearchResults());
		settings.setValue(constrShowOCntInSearchResultsRefs, CPersistentSettings::instance()->showOCntInSearchResultsRefs());
		settings.setValue(constrShowWrdNdxInSearchResultsRefs, CPersistentSettings::instance()->showWrdNdxInSearchResultsRefs());
		settings.endGroup();

		// Search Phrases Settings:
		settings.beginGroup(constrSearchPhrasesGroup);
		settings.setValue(constrSearchActivationDelayKey, CPersistentSettings::instance()->searchActivationDelay());
		settings.setValue(constrAutoCompleterActivationDelayKey, CPersistentSettings::instance()->autoCompleterActivationDelay());
		settings.setValue(constrSearchPhraseCompleterFilterModeKey, CPersistentSettings::instance()->searchPhraseCompleterFilterMode());
		settings.setValue(constrInitialNumberOfSearchPhrasesKey, CPersistentSettings::instance()->initialNumberOfSearchPhrases());
		settings.setValue(constrHideMatchingPhrasesListsKey, CPersistentSettings::instance()->hideMatchingPhrasesLists());
		settings.endGroup();
	}

	// Last Search:
	if (m_pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV).m_strUUID, Qt::CaseInsensitive) == 0) {
		// Remove old top-level, if this is the KJV:
		settings.beginWriteArray(constrUserSearchPhrasesGroup);
		settings.remove("");
		settings.endArray();
		settings.remove(constrUserSearchPhrasesGroup);

		settings.remove(constrLastSearchGroup);
	}

	m_pSearchSpecWidget->writeKJVSearchFile(settings, groupCombine(m_pBibleDatabase->compatibilityUUID(), constrLastSearchGroup));

	// User Search Phrases Settings:
	CPhraseList phrases;
	phrases.append(CPersistentSettings::instance()->userPhrases(m_pBibleDatabase->compatibilityUUID()));
	CPhraseListModel mdlPhrases(phrases);
	mdlPhrases.sort(0, Qt::AscendingOrder);
	phrases = mdlPhrases.phraseList();

	settings.beginWriteArray(groupCombine(m_pBibleDatabase->compatibilityUUID(), constrUserSearchPhrasesGroup));
	settings.remove("");
	for (int ndx = 0; ndx < phrases.size(); ++ndx) {
		settings.setArrayIndex(ndx);
		settings.setValue("Phrase", phrases.at(ndx).text());
		settings.setValue("CaseSensitive", phrases.at(ndx).caseSensitive());
		settings.setValue("AccentSensitive", phrases.at(ndx).accentSensitive());
		settings.setValue("Exclude", phrases.at(ndx).isExcluded());
	}
	settings.endArray();

	if (!bSaveLastSearchOnly) {
		// Current Browser Reference and Browser Settings:
		settings.beginGroup(constrBrowserViewGroup);
		TPhraseTag tag = m_pBrowserWidget->selection().primarySelection();
		settings.setValue(constrLastReferenceKey, tag.relIndex().asAnchor());
		settings.setValue(constrLastSelectionSizeKey, tag.count());
		settings.setValue(constrHasFocusKey, m_pBrowserWidget->hasFocusBrowser());
		settings.remove(constrFontKey);		// Remove deprecated font value
		settings.setValue(constrFontNameKey, CPersistentSettings::instance()->fontScriptureBrowser().family());
		settings.setValue(constrFontSizeKey, CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
		settings.setValue(constrNavigationActivationDelayKey, CPersistentSettings::instance()->navigationActivationDelay());
		settings.setValue(constrPassageReferenceActivationDelayKey, CPersistentSettings::instance()->passageReferenceActivationDelay());
		settings.setValue(constrShowExcludedSearchResultsKey, CPersistentSettings::instance()->showExcludedSearchResultsInBrowser());
		settings.setValue(constrChapterScrollbarModeKey, CPersistentSettings::instance()->chapterScrollbarMode());
		settings.setValue(constrVerseRenderingModeKey, CPersistentSettings::instance()->verseRenderingMode());
		settings.setValue(constrShowPilcrowMarkersKey, CPersistentSettings::instance()->showPilcrowMarkers());
		settings.setValue(constrLineHeightKey, CPersistentSettings::instance()->scriptureBrowserLineHeight());
		settings.setValue(constrBrowserNavigationPaneModeKey, CPersistentSettings::instance()->browserNavigationPaneMode());
		settings.setValue(constrBrowserDisplayModeKey, CPersistentSettings::instance()->browserDisplayMode());
		settings.setValue(constrRandomPassageWeightModeKey, CPersistentSettings::instance()->randomPassageWeightMode());
		settings.setValue(constrFootnoteRenderingModeKey, static_cast<int>(CPersistentSettings::instance()->footnoteRenderingMode()));
		settings.endGroup();

		// Browser Object (used for Subwindows: FindDialog, etc):
		m_pBrowserWidget->savePersistentSettings(constrBrowserViewGroup);

		// Dictionary Widget Settings:
		settings.beginGroup(constrDictionaryGroup);
		settings.remove(constrFontKey);		// Remove deprecated font value
		settings.setValue(constrFontNameKey, CPersistentSettings::instance()->fontDictionary().family());
		settings.setValue(constrFontSizeKey, CPersistentSettings::instance()->fontDictionary().pointSize());
		settings.setValue(constrDictionaryActivationDelayKey, CPersistentSettings::instance()->dictionaryActivationDelay());
		settings.setValue(constrDictionaryCompleterFilterModeKey, CPersistentSettings::instance()->dictionaryCompleterFilterMode());
		settings.endGroup();

		// Copy Options:
		settings.beginGroup(constrCopyOptionsGroup);
		settings.setValue(constrReferenceDelimiterModeKey, CPersistentSettings::instance()->referenceDelimiterMode());
		settings.setValue(constrReferencesAbbreviatedBookNamesKey, CPersistentSettings::instance()->referencesUseAbbreviatedBookNames());
		settings.setValue(constrReferencesInBoldKey, CPersistentSettings::instance()->referencesInBold());
		settings.setValue(constrReferencesAtEndKey, CPersistentSettings::instance()->referencesAtEnd());
		settings.setValue(constrVerseNumberDelimiterModeKey, CPersistentSettings::instance()->verseNumberDelimiterMode());
		settings.setValue(constrVerseNumbersAbbreviatedBookNamesKey, CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames());
		settings.setValue(constrVerseNumbersInBoldKey, CPersistentSettings::instance()->verseNumbersInBold());
		settings.setValue(constrAddQuotesAroundVerseKey, CPersistentSettings::instance()->addQuotesAroundVerse());
		settings.setValue(constrTransChangeAddWordModeKey, CPersistentSettings::instance()->transChangeAddWordMode());
		settings.setValue(constrVerseRenderingModeKey, CPersistentSettings::instance()->verseRenderingModeCopying());
		settings.setValue(constrCopyPilcrowMarkersKey, CPersistentSettings::instance()->copyPilcrowMarkers());
		settings.setValue(constrCopyColophonsKey, CPersistentSettings::instance()->copyColophons());
		settings.setValue(constrCopySuperscriptionsKey, CPersistentSettings::instance()->copySuperscriptions());
		settings.setValue(constrCopyFontSelectionKey, CPersistentSettings::instance()->copyFontSelection());
		settings.remove(constrCopyFontKey);		// Remove deprecated font value
		settings.setValue(constrCopyFontNameKey, CPersistentSettings::instance()->fontCopyFont().family());
		settings.setValue(constrCopyFontSizeKey, CPersistentSettings::instance()->fontCopyFont().pointSize());
		settings.setValue(constrCopyMimeTypeKey, CPersistentSettings::instance()->copyMimeType());
		settings.setValue(constrCopySearchResultsAddBlankLineBetweenVersesKey, CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses());
		settings.setValue(constrSearchResultsVerseCopyOrderKey, CPersistentSettings::instance()->searchResultsVerseCopyOrder());
		settings.setValue(constrCopyOCntInSearchResultsRefs, CPersistentSettings::instance()->copyOCntInSearchResultsRefs());
		settings.setValue(constrCopyWrdNdxInSearchResultsRefs, CPersistentSettings::instance()->copyWrdNdxInSearchResultsRefs());
		settings.endGroup();

		// Bible Database Settings:
		settings.beginWriteArray(constrBibleDatabaseSettingsGroup);
		settings.remove("");
		QStringList lstBibleDatabaseUUIDs = CPersistentSettings::instance()->bibleDatabaseSettingsUUIDList();
		for (int ndxDB = 0; ndxDB < lstBibleDatabaseUUIDs.size(); ++ndxDB) {
			const TBibleDatabaseSettings bdbSettings = CPersistentSettings::instance()->bibleDatabaseSettings(lstBibleDatabaseUUIDs.at(ndxDB));
			settings.setArrayIndex(ndxDB);
			settings.setValue(constrDatabaseUUIDKey, lstBibleDatabaseUUIDs.at(ndxDB));
			settings.setValue(constrLoadOnStartKey, bdbSettings.loadOnStart());
			settings.setValue(constrHideHyphensKey, static_cast<int>(bdbSettings.hideHyphens()));
			settings.setValue(constrHyphenSensitiveKey, bdbSettings.hyphenSensitive());
			settings.setValue(constrHideCantillationMarksKey, bdbSettings.hideCantillationMarks());
			settings.setValue(constrVersificationKey, static_cast<int>(bdbSettings.versification()));
			settings.setValue(constrCategoryGroupKey, static_cast<int>(bdbSettings.categoryGroup()));
		}
		settings.endArray();

		// Dictionary Database Settings:
		settings.beginWriteArray(constrDictDatabaseSettingsGroup);
		settings.remove("");
		QStringList lstDictDatabaseUUIDs = CPersistentSettings::instance()->dictionaryDatabaseSettingsUUIDList();
		for (int ndxDB = 0; ndxDB < lstDictDatabaseUUIDs.size(); ++ndxDB) {
			const TDictionaryDatabaseSettings ddbSettings = CPersistentSettings::instance()->dictionaryDatabaseSettings(lstDictDatabaseUUIDs.at(ndxDB));
			settings.setArrayIndex(ndxDB);
			settings.setValue(constrDatabaseUUIDKey, lstDictDatabaseUUIDs.at(ndxDB));
			settings.setValue(constrLoadOnStartKey, ddbSettings.loadOnStart());
		}
		settings.endArray();
	}
}

void CKJVCanOpener::restorePersistentSettings(bool bAppRestarting)
{
	Q_ASSERT(!g_pMyApplication.isNull());
	Q_ASSERT(!m_pBibleDatabase.isNull());

	bool bIsFirstCanOpener = bAppRestarting || g_pMyApplication->isFirstCanOpener(false);
	bool bIsFirstCanOpenerForThisBibleDB = bAppRestarting || g_pMyApplication->isFirstCanOpener(false, m_pBibleDatabase->compatibilityUUID());
	if (bIsFirstCanOpener) {
		Q_ASSERT(bIsFirstCanOpenerForThisBibleDB);
	}

	bool bFocusSearchResults = false;
	bool bFocusBrowser = false;
	bool bLaunchNotesSetupConfig = false;

	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());

		// Main App and Toolbars RestoreState:
		if (bIsFirstCanOpener) {
#if defined(PRESERVE_MAINWINDOW_GEOMETRY) || defined(PRESERVE_MAINWINDOW_STATE)
			settings.beginGroup(constrMainAppRestoreStateGroup);
#ifdef PRESERVE_MAINWINDOW_GEOMETRY
			restoreGeometry(settings.value(constrGeometryKey).toByteArray());
#endif
#ifdef PRESERVE_MAINWINDOW_STATE
			restoreState(settings.value(constrWindowStateKey).toByteArray(), KJVAPP_REGISTRY_VERSION);
#endif
			settings.endGroup();
#endif
		} else {
#if defined(PRESERVE_MAINWINDOW_GEOMETRY) || defined(PRESERVE_MAINWINDOW_STATE)
			CKJVCanOpener *pPrimaryCanOpener = g_pMyApplication->canOpeners().at(0);
			Q_ASSERT(pPrimaryCanOpener != nullptr);
#endif
#ifdef PRESERVE_MAINWINDOW_STATE
			restoreState(pPrimaryCanOpener->saveState(KJVAPP_REGISTRY_VERSION), KJVAPP_REGISTRY_VERSION);
#endif
#ifdef PRESERVE_MAINWINDOW_GEOMETRY
			resize(pPrimaryCanOpener->size());
#endif
		}

		// Main App General Settings:
		if (bIsFirstCanOpener) {
			settings.beginGroup(constrMainAppControlGroup);
			bool bInvertTextBrightness = settings.value(constrInvertTextBrightnessKey, CPersistentSettings::instance()->invertTextBrightness()).toBool();
			int nTextBrightness = settings.value(constrTextBrightnessKey, CPersistentSettings::instance()->textBrightness()).toInt();
			CPersistentSettings::instance()->setAdjustDialogElementBrightness(settings.value(constrAdjustDialogElementBrightnessKey, CPersistentSettings::instance()->adjustDialogElementBrightness()).toBool());
			CPersistentSettings::instance()->setTextBrightness(bInvertTextBrightness, nTextBrightness);
			settings.endGroup();
		}

		// Colors:
		if (bIsFirstCanOpener) {
			settings.beginGroup(constrColorsGroup);
			QColor clrTemp;
			QString strWordsOfJesusColor = settings.value(constrWordsOfJesusColorKey, CPersistentSettings::instance()->colorWordsOfJesus().name()).toString();
			if (!strWordsOfJesusColor.isEmpty()) {
				clrTemp.setNamedColor(strWordsOfJesusColor);
			} else {
				clrTemp = QColor();
			}
			CPersistentSettings::instance()->setColorWordsOfJesus(clrTemp);
			clrTemp.setNamedColor(settings.value(constrSearchResultsColorKey, CPersistentSettings::instance()->colorSearchResults().name()).toString());
			CPersistentSettings::instance()->setColorSearchResults(clrTemp);
			clrTemp.setNamedColor(settings.value(constrCursorTrackerColorKey, CPersistentSettings::instance()->colorCursorFollow().name()).toString());
			CPersistentSettings::instance()->setColorCursorFollow(clrTemp);
			settings.endGroup();
		}

#ifdef PRESERVE_MAINWINDOW_SPLITTER_STATE
		unsigned int nSplitterVersion;

		// Splitter:
		if (m_pSplitter != nullptr) {
			settings.beginGroup(constrSplitterRestoreStateGroup);
			nSplitterVersion = settings.value(constrStateVersionKey).toUInt();
			if (nSplitterVersion == PS_SPLITTER_VERSION) {
				m_pSplitter->restoreState(settings.value(constrWindowStateKey).toByteArray());
			}
			settings.endGroup();
		}

		// Splitter Dictionary:
		if ((m_pDictionaryWidget != nullptr) && (m_pSplitterDictionary != nullptr)) {
			settings.beginGroup(constrSplitterDictionaryRestoreStateGroup);
			nSplitterVersion = settings.value(constrStateVersionKey).toUInt();
			if (nSplitterVersion == PS_SPLITTER_VERSION) {
				m_pSplitterDictionary->restoreState(settings.value(constrWindowStateKey).toByteArray());
			}
			settings.endGroup();
		}
#endif

		// User Notes Database:
		Q_ASSERT(!g_pUserNotesDatabase.isNull());
		if (bIsFirstCanOpener) {
			settings.beginGroup(constrUserNotesDatabaseGroup);
			g_pUserNotesDatabase->setFilePathName(settings.value(constrFilePathNameKey, QString()).toString());
			CPersistentSettings::instance()->setKeepNotesBackup(settings.value(constrKeepBackupKey, CPersistentSettings::instance()->keepNotesBackup()).toBool());
			CPersistentSettings::instance()->setNotesBackupFilenamePostfix(settings.value(constrBackupFilenamePostfixKey, CPersistentSettings::instance()->notesBackupFilenamePostfix()).toString());
			CPersistentSettings::instance()->setNotesFileAutoSaveFile(settings.value(constrAutoSaveTimeKey, CPersistentSettings::instance()->notesFileAutoSaveTime()).toInt());
			QColor clrTemp;
			clrTemp.setNamedColor(settings.value(constrDefaultNoteBackgroundColorKey, CPersistentSettings::instance()->colorDefaultNoteBackground().name()).toString());
			CPersistentSettings::instance()->setColorDefaultNoteBackground(clrTemp);
			settings.endGroup();
		}

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
		m_pUserNoteEditorDlg->readSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrUserNoteEditorGroup));
		m_pCrossRefsEditorDlg->readSettings(settings, groupCombine(constrUserNotesDatabaseGroup, constrCrossRefsEditorGroup));
#endif

		if (bIsFirstCanOpener) {
			if (!g_pUserNotesDatabase->filePathName().isEmpty()) {
				if (!g_pUserNotesDatabase->load()) {
					show();
					displayWarning(this, tr("King James Notes File Error", "Errors"),  g_pUserNotesDatabase->lastLoadSaveError() + QString("\n\n") + tr("Check File existence and Program Settings!", "Errors"));
					// Leave the isDirty flag set, but clear the filename to force the user to re-navigate to
					//		it, or else we may accidentally overwrite the file if it happens to be "fixed" by
					//		the time we exit.  But save a reference to it so we can get the user navigated back there:
					g_pUserNotesDatabase->setErrorFilePathName(g_pUserNotesDatabase->filePathName());
					g_pUserNotesDatabase->setFilePathName(QString());
					bLaunchNotesSetupConfig = true;
				} else {
					if (g_pUserNotesDatabase->version() < KJN_FILE_VERSION) {
						show();
						displayWarning(this, tr("Loading King James Notes File", "Errors"), tr("Warning: The King James Notes File being loaded was last saved on "
													"an older version of King James Pure Bible Search.  It will automatically be updated to this version of "
													"King James Pure Bible Search.  However, if you wish to keep a copy of your Notes File in the old format, you must "
													"manually save a copy of your file now BEFORE you continue!\n\nFilename: \"%1\"", "Errors").arg(g_pUserNotesDatabase->filePathName()));
					} else if (g_pUserNotesDatabase->version() > KJN_FILE_VERSION) {
						show();
						displayWarning(this, tr("Loading King James Notes File", "Errors"), tr("Warning: The King James Notes File being loaded was created on "
													"a newer version of King James Pure Bible Search.  It may contain data or settings for things not "
													"supported on this version of King James Pure Bible Search.  If so, those new things will be LOST the "
													"next time your Notes Files is saved.  If you wish to keep a copy of your original Notes File and not "
													"risk losing any data from it, you must manually save a copy of your file now BEFORE you continue!"
													"\n\nFilename: \"%1\"", "Errors").arg(g_pUserNotesDatabase->filePathName()));
					}
				}
			}
		}

		// Highlighter Tool Bar (must be after loading the User Notes Database):
		if (m_pHighlighterButtons != nullptr) {
			int nColors = settings.beginReadArray(groupCombine(constrColorsGroup, constrColorsHighlightersSubgroup));
			if (nColors != 0) {
				for (int ndxColor = 0; ((ndxColor < nColors) && (ndxColor < m_pHighlighterButtons->count())); ++ndxColor) {
					settings.setArrayIndex(ndxColor);
					QString strHighlighterName = settings.value(constrHighlighterNameKey, QString()).toString();
					m_pHighlighterButtons->setHighlighterList(ndxColor, strHighlighterName);
				}
			} else {
				// For a new (empty) User Notes Database, set the ToolBar to the initial file default highlighters:
				if (g_pUserNotesDatabase->filePathName().isEmpty()) {
					const TUserDefinedColorMap mapHighlighters = g_pUserNotesDatabase->highlighterDefinitionsMap();
					int ndxColor = 0;
					for (TUserDefinedColorMap::const_iterator itrHighlighters = mapHighlighters.constBegin();
									((itrHighlighters != mapHighlighters.constEnd()) && (ndxColor < m_pHighlighterButtons->count()));
									++itrHighlighters) {
						m_pHighlighterButtons->setHighlighterList(ndxColor, itrHighlighters.key());
						ndxColor++;
					}
				}
			}
			settings.endArray();
		}

		// Set Search Results View Keyword Filter:
		m_pSearchResultWidget->keywordListChanged(true);

		if (bIsFirstCanOpener) {
			// Search Phrases Settings:
			settings.beginGroup(constrSearchPhrasesGroup);
			CPersistentSettings::instance()->setSearchActivationDelay(settings.value(constrSearchActivationDelayKey, CPersistentSettings::instance()->searchActivationDelay()).toInt());
			CPersistentSettings::instance()->setAutoCompleterActivationDelay(settings.value(constrAutoCompleterActivationDelayKey, CPersistentSettings::instance()->autoCompleterActivationDelay()).toInt());
			CPersistentSettings::instance()->setSearchPhraseCompleterFilterMode(static_cast<SEARCH_COMPLETION_FILTER_MODE_ENUM>(settings.value(constrSearchPhraseCompleterFilterModeKey, CPersistentSettings::instance()->searchPhraseCompleterFilterMode()).toUInt()));
			CPersistentSettings::instance()->setInitialNumberOfSearchPhrases(settings.value(constrInitialNumberOfSearchPhrasesKey, CPersistentSettings::instance()->initialNumberOfSearchPhrases()).toInt());
			CPersistentSettings::instance()->setHideMatchingPhrasesLists(settings.value(constrHideMatchingPhrasesListsKey, CPersistentSettings::instance()->hideMatchingPhrasesLists()).toBool());
			settings.endGroup();
		}

		if ((bIsFirstCanOpener) || (bIsFirstCanOpenerForThisBibleDB) || (!g_pMyApplication->fileToLoad().isEmpty())) {
			// Read Last Search before setting Search Results mode or else the last settings
			//	won't get restored -- they will be overriden by the loading of the Last Search...
			//	But, we first need to disable our SearchActivationDelay so that the updates
			//	will happen immediately -- otherwise, Search Results mode settings won't
			//	restore properly:
			int nSaveSearchActivationDelay = CPersistentSettings::instance()->searchActivationDelay();
			CPersistentSettings::instance()->setSearchActivationDelay(-1);

			// Last Search or passed KJS file:
			if (!g_pMyApplication->fileToLoad().isEmpty()) {
				openKJVSearchFile(g_pMyApplication->fileToLoad());
				g_pMyApplication->setFileToLoad(QString());
			} else if (bIsFirstCanOpenerForThisBibleDB) {
				m_pSearchSpecWidget->readKJVSearchFile(settings, groupCombine(m_pBibleDatabase->compatibilityUUID(), constrLastSearchGroup));
			} else {
				Q_ASSERT(false);			// Should never end up here...
				m_pSearchSpecWidget->readKJVSearchFile(settings, constrLastSearchGroup);
			}

			if ((bIsFirstCanOpener) || (bIsFirstCanOpenerForThisBibleDB)) {
				// User Search Phrases Settings:
				CPhraseList lstUserPhrases;
				int nPhrases;
				if (m_pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV).m_strUUID, Qt::CaseInsensitive) == 0) {
					// Move old Bible Database settings to new UUID;
					nPhrases = settings.beginReadArray(constrUserSearchPhrasesGroup);
					if (nPhrases != 0) {
						lstUserPhrases.reserve(lstUserPhrases.size() + nPhrases);
						for (int ndx = 0; ndx < nPhrases; ++ndx) {
							CPhraseEntry phrase;
							settings.setArrayIndex(ndx);
							phrase.setText(settings.value("Phrase", QString()).toString());
							phrase.setCaseSensitive(settings.value("CaseSensitive", false).toBool());
							phrase.setAccentSensitive(settings.value("AccentSensitive", false).toBool());
							phrase.setExclude(settings.value("Exclude", false).toBool());
							if (phrase.text().isEmpty()) continue;
							lstUserPhrases.append(phrase);
						}
					}
					settings.endArray();
				}
				nPhrases = settings.beginReadArray(groupCombine(m_pBibleDatabase->compatibilityUUID(), constrUserSearchPhrasesGroup));
				if (nPhrases != 0) {
					lstUserPhrases.reserve(lstUserPhrases.size() + nPhrases);
					for (int ndx = 0; ndx < nPhrases; ++ndx) {
						CPhraseEntry phrase;
						settings.setArrayIndex(ndx);
						phrase.setText(settings.value("Phrase", QString()).toString());
						phrase.setCaseSensitive(settings.value("CaseSensitive", false).toBool());
						phrase.setAccentSensitive(settings.value("AccentSensitive", false).toBool());
						phrase.setExclude(settings.value("Exclude", false).toBool());
						if (phrase.text().isEmpty()) continue;
						lstUserPhrases.append(phrase);
					}
				}
				settings.endArray();
				if (lstUserPhrases.size()) CPersistentSettings::instance()->setUserPhrases(m_pBibleDatabase->compatibilityUUID(), lstUserPhrases);
			}

			// Restore our activation delay:
			CPersistentSettings::instance()->setSearchActivationDelay(nSaveSearchActivationDelay);
		} else {
			m_pSearchSpecWidget->reset();
		}

		// Search Results mode:
		settings.beginGroup(constrSearchResultsViewGroup);
		if (bIsFirstCanOpener) {
			// Restore auto-expand before we set the view mode so that it will update correctly:
			CPersistentSettings::instance()->setAutoExpandSearchResultsTree(settings.value(constrAutoExpandSearchResultsTreeViewKey, CPersistentSettings::instance()->autoExpandSearchResultsTree()).toBool());
			// Same for hideNotFoundInStatistics:
			CPersistentSettings::instance()->setHideNotFoundInStatistics(settings.value(constrHideNotFoundInStatisticsKey, CPersistentSettings::instance()->hideNotFoundInStatistcs()).toBool());
		}
		setViewMode(static_cast<CVerseListModel::VERSE_VIEW_MODE_ENUM>(settings.value(constrResultsViewModeKey, m_pSearchResultWidget->viewMode()).toUInt()));
		setDisplayMode(static_cast<CVerseListModel::VERSE_DISPLAY_MODE_ENUM>(settings.value(constrVerseDisplayModeKey, m_pSearchResultWidget->displayMode()).toUInt()));
		setTreeMode(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(settings.value(constrVerseTreeModeKey, m_pSearchResultWidget->treeMode()).toUInt()));
		setShowMissingLeafs(settings.value(constrViewMissingNodesKey, m_pSearchResultWidget->showMissingLeafs()).toBool());
		m_pSearchResultWidget->setShowHighlightersInSearchResults(settings.value(constrShowHighlightersInSearchResultsKey, m_pSearchResultWidget->showHighlightersInSearchResults()).toBool());
		CPersistentSettings::instance()->setShowOCntInSearchResultsRefs(settings.value(constrShowOCntInSearchResultsRefs, CPersistentSettings::instance()->showOCntInSearchResultsRefs()).toBool());
		CPersistentSettings::instance()->setShowWrdNdxInSearchResultsRefs(settings.value(constrShowWrdNdxInSearchResultsRefs, CPersistentSettings::instance()->showWrdNdxInSearchResultsRefs()).toBool());
		CRelIndex ndxLastCurrentIndex(settings.value(constrCurrentIndexKey, CRelIndex().asAnchor()).toString());
		QString strHighlighterName = settings.value(constrCurrentHighlighterKey, QString()).toString();
		if (m_pSearchResultWidget->viewMode() != CVerseListModel::VVME_HIGHLIGHTERS) strHighlighterName.clear();		// Make sure we load the correct verseIndex below for Search Results and UserNotes, etc
		bFocusSearchResults = settings.value(constrHasFocusKey, false).toBool();
		if (bIsFirstCanOpener) {
			QString strFontName = settings.value(constrFontNameKey, CPersistentSettings::instance()->fontSearchResults().family()).toString();
			int nFontSize = settings.value(constrFontSizeKey, CPersistentSettings::instance()->fontSearchResults().pointSize()).toInt();

			if ((!strFontName.isEmpty()) && (nFontSize>0)) {
				CPersistentSettings::instance()->setFontSearchResults(QFont(strFontName, nFontSize));
			}
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
		bFocusBrowser = settings.value(constrHasFocusKey, false).toBool();
		if (!bLastSet) {
			CRelIndex ndxLastBrowsed = CRelIndex(settings.value(constrLastReferenceKey, CRelIndex().asAnchor()).toString());
			if (ndxLastBrowsed.isSet()) m_pSearchResultWidget->setCurrentIndex(m_pSearchResultWidget->vlmodel()->resolveVerseIndex(ndxLastBrowsed, strHighlighterName), false);
		}
		if (bIsFirstCanOpener) {
			QString strFontName = settings.value(constrFontNameKey, CPersistentSettings::instance()->fontScriptureBrowser().family()).toString();
			int nFontSize = settings.value(constrFontSizeKey, CPersistentSettings::instance()->fontScriptureBrowser().pointSize()).toInt();

			if ((!strFontName.isEmpty()) && (nFontSize>0)) {
				CPersistentSettings::instance()->setFontScriptureBrowser(QFont(strFontName, nFontSize));
			}

			CPersistentSettings::instance()->setNavigationActivationDelay(settings.value(constrNavigationActivationDelayKey, CPersistentSettings::instance()->navigationActivationDelay()).toInt());
			CPersistentSettings::instance()->setPassageReferenceActivationDelay(settings.value(constrPassageReferenceActivationDelayKey, CPersistentSettings::instance()->passageReferenceActivationDelay()).toInt());
			CPersistentSettings::instance()->setShowExcludedSearchResultsInBrowser(settings.value(constrShowExcludedSearchResultsKey, CPersistentSettings::instance()->showExcludedSearchResultsInBrowser()).toBool());
			CPersistentSettings::instance()->setChapterScrollbarMode(static_cast<CHAPTER_SCROLLBAR_MODE_ENUM>(settings.value(constrChapterScrollbarModeKey, CPersistentSettings::instance()->chapterScrollbarMode()).toUInt()));
			CPersistentSettings::instance()->setVerseRenderingMode(static_cast<CPhraseNavigator::VERSE_RENDERING_MODE_ENUM>(settings.value(constrVerseRenderingModeKey, CPersistentSettings::instance()->verseRenderingMode()).toUInt()));
			CPersistentSettings::instance()->setShowPilcrowMarkers(settings.value(constrShowPilcrowMarkersKey, CPersistentSettings::instance()->showPilcrowMarkers()).toBool());
			CPersistentSettings::instance()->setScriptureBrowserLineHeight(settings.value(constrLineHeightKey, CPersistentSettings::instance()->scriptureBrowserLineHeight()).toDouble());
			CPersistentSettings::instance()->setBrowserNavigationPaneMode(static_cast<BROWSER_NAVIGATION_PANE_MODE_ENUM>(settings.value(constrBrowserNavigationPaneModeKey, CPersistentSettings::instance()->browserNavigationPaneMode()).toInt()));
			CPersistentSettings::instance()->setBrowserDisplayMode(static_cast<BROWSER_DISPLAY_MODE_ENUM>(settings.value(constrBrowserDisplayModeKey, CPersistentSettings::instance()->browserDisplayMode()).toInt()));
			CPersistentSettings::instance()->setRandomPassageWeightMode(static_cast<RANDOM_PASSAGE_WEIGHT_ENUM>(settings.value(constrRandomPassageWeightModeKey, CPersistentSettings::instance()->randomPassageWeightMode()).toInt()));
			CPersistentSettings::instance()->setFootnoteRenderingMode(static_cast<CPhraseNavigator::FootnoteRenderingModeFlags>(settings.value(constrFootnoteRenderingModeKey, static_cast<int>(CPersistentSettings::instance()->footnoteRenderingMode())).toInt()));

#if QT_VERSION >= 0x050400
			QTimer::singleShot(1, this, [this]() { m_pBrowserWidget->setBrowserDisplayMode(CPersistentSettings::instance()->browserDisplayMode()); } );
#endif
		}
		settings.endGroup();

		// Browser Object (used for Subwindows: FindDialog, etc):
		m_pBrowserWidget->restorePersistentSettings(constrBrowserViewGroup);

		// Dictionary Widget Settings:
		if (bIsFirstCanOpener) {
			settings.beginGroup(constrDictionaryGroup);

			QString strFontName = settings.value(constrFontNameKey, CPersistentSettings::instance()->fontDictionary().family()).toString();
			int nFontSize = settings.value(constrFontSizeKey, CPersistentSettings::instance()->fontDictionary().pointSize()).toInt();

			if ((!strFontName.isEmpty()) && (nFontSize>0)) {
				CPersistentSettings::instance()->setFontDictionary(QFont(strFontName, nFontSize));
			}

			CPersistentSettings::instance()->setDictionaryActivationDelay(settings.value(constrDictionaryActivationDelayKey, CPersistentSettings::instance()->dictionaryActivationDelay()).toInt());
			CPersistentSettings::instance()->setDictionaryCompleterFilterMode(static_cast<SEARCH_COMPLETION_FILTER_MODE_ENUM>(settings.value(constrDictionaryCompleterFilterModeKey, CPersistentSettings::instance()->dictionaryCompleterFilterMode()).toUInt()));
			settings.endGroup();
		}

		// Copy Options:
		if (bIsFirstCanOpener) {
			settings.beginGroup(constrCopyOptionsGroup);
			CPersistentSettings::instance()->setReferenceDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(settings.value(constrReferenceDelimiterModeKey, CPersistentSettings::instance()->referenceDelimiterMode()).toUInt()));
			CPersistentSettings::instance()->setReferencesUseAbbreviatedBookNames(settings.value(constrReferencesAbbreviatedBookNamesKey, CPersistentSettings::instance()->referencesUseAbbreviatedBookNames()).toBool());
			CPersistentSettings::instance()->setReferencesInBold(settings.value(constrReferencesInBoldKey, CPersistentSettings::instance()->referencesInBold()).toBool());
			CPersistentSettings::instance()->setReferencesAtEnd(settings.value(constrReferencesAtEndKey, CPersistentSettings::instance()->referencesAtEnd()).toBool());
			CPersistentSettings::instance()->setVerseNumberDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(settings.value(constrVerseNumberDelimiterModeKey, CPersistentSettings::instance()->verseNumberDelimiterMode()).toUInt()));
			CPersistentSettings::instance()->setVerseNumbersUseAbbreviatedBookNames(settings.value(constrVerseNumbersAbbreviatedBookNamesKey, CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames()).toBool());
			CPersistentSettings::instance()->setVerseNumbersInBold(settings.value(constrVerseNumbersInBoldKey, CPersistentSettings::instance()->verseNumbersInBold()).toBool());
			CPersistentSettings::instance()->setAddQuotesAroundVerse(settings.value(constrAddQuotesAroundVerseKey, CPersistentSettings::instance()->addQuotesAroundVerse()).toBool());
			CPersistentSettings::instance()->setTransChangeAddWordMode(static_cast<CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM>(settings.value(constrTransChangeAddWordModeKey, CPersistentSettings::instance()->transChangeAddWordMode()).toUInt()));
			CPersistentSettings::instance()->setVerseRenderingModeCopying(static_cast<CPhraseNavigator::VERSE_RENDERING_MODE_ENUM>(settings.value(constrVerseRenderingModeKey, CPersistentSettings::instance()->verseRenderingModeCopying()).toUInt()));
			CPersistentSettings::instance()->setCopyPilcrowMarkers(settings.value(constrCopyPilcrowMarkersKey, CPersistentSettings::instance()->copyPilcrowMarkers()).toBool());
			CPersistentSettings::instance()->setCopyColophons(settings.value(constrCopyColophonsKey, CPersistentSettings::instance()->copyColophons()).toBool());
			CPersistentSettings::instance()->setCopySuperscriptions(settings.value(constrCopySuperscriptionsKey, CPersistentSettings::instance()->copySuperscriptions()).toBool());
			CPersistentSettings::instance()->setCopyFontSelection(static_cast<CPhraseNavigator::COPY_FONT_SELECTION_ENUM>(settings.value(constrCopyFontSelectionKey, CPersistentSettings::instance()->copyFontSelection()).toUInt()));

			QString strFontName = settings.value(constrCopyFontNameKey, CPersistentSettings::instance()->fontCopyFont().family()).toString();
			int nFontSize = settings.value(constrCopyFontSizeKey, CPersistentSettings::instance()->fontCopyFont().pointSize()).toInt();

			if ((!strFontName.isEmpty()) && (nFontSize>0)) {
				CPersistentSettings::instance()->setFontCopyFont(QFont(strFontName, nFontSize));
			}

			CPersistentSettings::instance()->setCopyMimeType(static_cast<COPY_MIME_TYPE_ENUM>(settings.value(constrCopyMimeTypeKey, CPersistentSettings::instance()->copyMimeType()).toUInt()));
			CPersistentSettings::instance()->setSearchResultsAddBlankLineBetweenVerses(settings.value(constrCopySearchResultsAddBlankLineBetweenVersesKey, CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses()).toBool());
			CPersistentSettings::instance()->setSearchResultsVerseCopyOrder(static_cast<VERSE_COPY_ORDER_ENUM>(settings.value(constrSearchResultsVerseCopyOrderKey, CPersistentSettings::instance()->searchResultsVerseCopyOrder()).toUInt()));
			CPersistentSettings::instance()->setCopyOCntInSearchResultsRefs(settings.value(constrCopyOCntInSearchResultsRefs, CPersistentSettings::instance()->copyOCntInSearchResultsRefs()).toBool());
			CPersistentSettings::instance()->setCopyWrdNdxInSearchResultsRefs(settings.value(constrCopyWrdNdxInSearchResultsRefs, CPersistentSettings::instance()->copyWrdNdxInSearchResultsRefs()).toBool());
			settings.endGroup();
		}
	} else {
		// If we aren't using Persistent Settings:

		Q_ASSERT(!g_pUserNotesDatabase.isNull());

		// Set the ToolBar to the initial file default highlighters:
		if (m_pHighlighterButtons != nullptr) {
			const TUserDefinedColorMap mapHighlighters = g_pUserNotesDatabase->highlighterDefinitionsMap();
			int ndxColor = 0;
			for (TUserDefinedColorMap::const_iterator itrHighlighters = mapHighlighters.constBegin();
							((itrHighlighters != mapHighlighters.constEnd()) && (ndxColor < m_pHighlighterButtons->count()));
							++itrHighlighters) {
				m_pHighlighterButtons->setHighlighterList(ndxColor, itrHighlighters.key());
				ndxColor++;
			}
		}

		// Reset our search phrases or load file if we are supposed to:
		if (!g_pMyApplication->fileToLoad().isEmpty()) {
			openKJVSearchFile(g_pMyApplication->fileToLoad());
			g_pMyApplication->setFileToLoad(QString());
		} else {
			m_pSearchSpecWidget->reset();
		}
	}

	show();			// Now that we've restored our settings and geometry, show our window...
	initialize();	// Navigate to our restored location, now that we've finished showing our window (must be done after show so ScriptureBrowser textSelect works)
	raise();
	activateWindow();

	if (bIsFirstCanOpener) {
		// If the Search Result was focused last time, focus it again, else if
		//	the browser was focus last time, focus it again.  Otherwise, leave
		//	the phrase editor focus:
		if (bFocusSearchResults) {
			QTimer::singleShot(1, m_pSearchResultWidget, SLOT(setFocusSearchResult()));
		} else if (bFocusBrowser) {
			QTimer::singleShot(1, m_pBrowserWidget, SLOT(setFocusBrowser()));
		}

		if (bLaunchNotesSetupConfig) QTimer::singleShot(10, this, SLOT(en_LaunchNotesFileSettingsConfig()));
	} else {
		// For secondary search windows, activate the search window:
		if (m_lstpQuickActivate.size() >= 2) m_lstpQuickActivate.at(1)->trigger();
	}
}

void CKJVCanOpener::closeEvent(QCloseEvent *event)
{
	// If we are already closing, ignore this event.  For some reason on Mac with
	//		Qt 5.3.0, using the application quit from the taskbar context menu
	//		causes us to receive multiple close events.  This will safe-guard
	//		against multiple events and keep us from having redundant attempts
	//		at saving options and notes files...
	if (m_bIsClosing) return;

	Q_ASSERT(canClose());
	if (!canClose()) {
		event->ignore();
		return;
	}

	Q_ASSERT(!g_pMyApplication.isNull());

	if (g_pMyApplication->isLastCanOpener()) {
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined (IS_CONSOLE_APP)
		int nResult;
		bool bPromptFilename = false;

		Q_ASSERT(!g_pUserNotesDatabase.isNull());
		if (g_pUserNotesDatabase->isDirty()) {
			// If we don't have a file name, yet made some change to the KJN, prompt them for a path:
			if (g_pUserNotesDatabase->filePathName().isEmpty()) {
				if (g_pUserNotesDatabase->errorFilePathName().isEmpty()) {
					// If we don't have a filename at all, prompt for new setup:
					nResult = displayWarning(this, windowTitle(), tr("You have edited Notes, Highlighters, and/or References, but don't yet have a King James Notes File setup.\n\n"
																			 "Do you wish to setup a Notes File and save your changes??\nWarning: If you select 'No', then your changes will be lost.", "Errors"),
															(QMessageBox::Yes  | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
				} else {
					// If we originally had a filename, but failed in opening it, just prompt the user about saving it since it's
					//		possible they don't want to attempt to overwrite the one that failed since we couldn't load it:
					nResult = displayWarning(this, windowTitle(), tr("The previous attempt to load your King James Notes File failed.\n"
																		   "Do you wish to save the changes you've made?\n"
																		   "Warning, if you save this file overtop of your original file, you will "
																		   "lose all ability to recover the remaining data in your original file.  It's "
																		   "recommended that you save it to a new file.\n\n"
																		   "Click 'Yes' to enter a filename and save your new changes, or\n"
																		   "Click 'No' to lose your changes and exit, or\n"
																		   "Click 'Cancel' to return to King James Pure Bible Search...", "Errors"),
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
						QString strFilePathName = CSaveLoadFileDialog::getSaveFileName(this, tr("Save King James Notes File", "FileFilters"), g_pUserNotesDatabase->errorFilePathName(), tr("King James Notes Files (*.kjn)", "FileFilters"), "kjn", nullptr, QFileDialog::Options());
						if (!strFilePathName.isEmpty()) {
							g_pUserNotesDatabase->setFilePathName(strFilePathName);
						} else {
							// If the user aborted treating after the path after all:
							event->ignore();
							return;
						}
					}

					if (!g_pUserNotesDatabase->save()) {
						nResult = displayWarning(this, tr("King James Notes File Error", "Errors"),  g_pUserNotesDatabase->lastLoadSaveError() + QString("\n\n") +
															tr("Unable to save the King James Notes File!\n\n"
															   "Click 'Yes' to try again, or\n"
															   "Click 'No' to lose your changes and exit, or\n"
															   "Click 'Cancel' to return to King James Pure Bible Search...", "Errors"),
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
#endif

		savePersistentSettings(false);
	} else if ((!m_pBibleDatabase.isNull()) && (g_pMyApplication->isLastCanOpener(m_pBibleDatabase->compatibilityUUID()))) {
		savePersistentSettings(true);
	}

	m_bIsClosing = true;
	emit isClosing(this);
//	QMainWindow::closeEvent(event);
	deleteLater();
}

bool CKJVCanOpener::event(QEvent *pEvent)
{
	if (!m_bIsClosing) {
		if (pEvent->type() == QEvent::WindowActivate) emit windowActivated(this);
	}
	return QMainWindow::event(pEvent);
}

// ------------------------------------------------------------------

QString CKJVCanOpener::searchWindowDescription() const
{
	return m_pSearchSpecWidget->searchWindowDescription();
}

// ------------------------------------------------------------------

void CKJVCanOpener::setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode, bool bFocusTree)
{
	Q_ASSERT(m_pActionGroupViewMode != nullptr);

	QList<QAction *> lstActions = m_pActionGroupViewMode->actions();

	for (int i = 0; i < lstActions.size(); ++i) {
		if (static_cast<CVerseListModel::VERSE_VIEW_MODE_ENUM>(lstActions.at(i)->data().toUInt()) == nViewMode) {
			lstActions.at(i)->setChecked(true);
			en_viewModeChange(lstActions.at(i), bFocusTree);
			break;
		}
	}
}

void CKJVCanOpener::setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	Q_ASSERT(m_pActionGroupDisplayMode != nullptr);

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
	Q_ASSERT(m_pActionGroupTreeMode != nullptr);

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
	Q_ASSERT(m_pActionShowMissingLeafs != nullptr);
	m_pActionShowMissingLeafs->setChecked(bShowMissing);
	en_viewShowMissingsLeafs();
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_NewSearch()
{
	m_pSearchSpecWidget->reset();
	if (m_lstpQuickActivate.size() >= 2) m_lstpQuickActivate.at(1)->trigger();
}

void CKJVCanOpener::en_OpenSearch()
{
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	QString strFilePathName = CSaveLoadFileDialog::getOpenFileName(this, tr("Open KJV Search File", "FileFilters"), QString(), tr("KJV Search Files (*.kjs)", "FileFilters"), nullptr, QFileDialog::ReadOnly);
	if (!strFilePathName.isEmpty())
		if (!openKJVSearchFile(strFilePathName))
			displayWarning(this, tr("KJV Search File Open Failed", "Errors"), tr("Failed to open and read the specified KJV Search File!", "Errors"));
#else
//	// Note: This still doesn't work -- it looks up not being able to instantiate the OpenFileDialog.
//	//	Left here for reference for how to do the open asynchronously:
//	QFileDialog *pDlg = new QFileDialog(this, tr("Open KJV Search File", "FileFilters"), QString(), tr("KJV Search Files (*.kjs)", "FileFilters"));
//	pDlg->setOptions(QFileDialog::ReadOnly);
//	pDlg->setAttribute(Qt::WA_DeleteOnClose);
//	pDlg->setModal(true);
//	pDlg->setFileMode(QFileDialog::ExistingFile);
//	pDlg->setAcceptMode(QFileDialog::AcceptOpen);
//	connect(pDlg, SIGNAL(fileSelected(const QString &)), this, SLOT(openKJVSearchFile(const QString &)));
//	pDlg->show();
#endif
}

void CKJVCanOpener::en_SaveSearch()
{
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	QString strFilePathName = CSaveLoadFileDialog::getSaveFileName(this, tr("Save KJV Search File", "FileFilters"), QString(), tr("KJV Search Files (*.kjs)", "FileFilters"), "kjs", nullptr, QFileDialog::Options());
	if (!strFilePathName.isEmpty())
		if (!saveKJVSearchFile(strFilePathName))
			displayWarning(this, tr("KJV Search File Save Failed", "Errors"), tr("Failed to save the specified KJV Search File!", "Errors"));
#else
//	// Note: This still doesn't work -- it looks up not being able to instantiate the OpenFileDialog.
//	//	Left here for reference for how to do the open asynchronously:
//	QFileDialog *pDlg = new QFileDialog(this, tr("Save KJV Search File", "FileFilters"), QString(), tr("KJV Search Files (*.kjs)", "FileFilters"));
//	pDlg->setAttribute(Qt::WA_DeleteOnClose);
//	pDlg->setModal(true);
//	pDlg->setFileMode(QFileDialog::AnyFile);
//	pDlg->setAcceptMode(QFileDialog::AcceptSave);
//	connect(pDlg, SIGNAL(fileSelected(const QString &)), this, SLOT(saveKJVSearchFile(const QString &)));
//	pDlg->show();
#endif
}

void CKJVCanOpener::en_ClearSearchPhrases()
{
	m_pSearchSpecWidget->clearAllSearchPhrases();
	if (m_lstpQuickActivate.size() >= 2) m_lstpQuickActivate.at(1)->trigger();
}

QString CKJVCanOpener::determineBibleUUIDForKJVSearchFile(const QString &strFilePathName)
{
	if (strFilePathName.isEmpty()) return QString();

	QSettings kjsFile(strFilePathName, QSettings::IniFormat);
	if (kjsFile.status() != QSettings::NoError) return QString();
#if QT_VERSION < 0x060000
	kjsFile.setIniCodec("UTF-8");
#endif

	QString strBblUUID;
	kjsFile.beginGroup("KJVPureBibleSearch");
	strBblUUID = kjsFile.value("BibleDatabase/UUID", bibleDescriptor(BDE_KJV).m_strUUID).toString();	// Read Bible UUID for this file (default to KJV if not specified since early files didn't)
	kjsFile.endGroup();

	return strBblUUID;
}

bool CKJVCanOpener::openKJVSearchFile(const QString &strFilePathName)
{
	if (strFilePathName.isEmpty()) return true;						// Empty is no-file-selected (cancel), treat it as "OK"

	Q_ASSERT(!m_pBibleDatabase.isNull());

	QSettings kjsFile(strFilePathName, QSettings::IniFormat);
	if (kjsFile.status() != QSettings::NoError) return false;
#if QT_VERSION < 0x060000
	kjsFile.setIniCodec("UTF-8");
#endif

	unsigned int nFileVersion = 0;
	QString strBblUUID;
	QString strBblLang;

	kjsFile.beginGroup("KJVPureBibleSearch");
	nFileVersion = kjsFile.value("KJSFileVersion", 0).toUInt();
	strBblUUID = kjsFile.value("BibleDatabase/UUID", bibleDescriptor(BDE_KJV).m_strUUID).toString();	// Read Bible UUID for this file (default to KJV if not specified since early files didn't)
	kjsFile.endGroup();

	// Determine Bible Language for this file (default to KJV if not specified since early files didn't specify)
	TBibleDescriptor bblDesc = TBibleDatabaseList::availableBibleDatabaseDescriptor(strBblUUID);
	if (!bblDesc.isValid()) bblDesc = bibleDescriptor(BDE_KJV);
	strBblLang = bblDesc.m_strLanguage;

	if (nFileVersion < KJS_FILE_VERSION) {
		show();		// Make sure we are visible if this was during construction
		displayWarning(this, tr("Opening King James Search File", "Errors"), tr("Warning: The file you are opening was saved on "
									"an older version of King James Pure Bible Search.  Some manual editing may be necessary "
									"to configure any new search options added since that older version.\n\n"
									"To avoid this message when opening this file in the future, then resave your "
									"search phrases over top of this file, replacing this old version.", "Errors"));
	} else if (nFileVersion > KJS_FILE_VERSION) {
		show();		// Make sure we are visible if this was during construction
		displayWarning(this, tr("Opening King James Search File", "Errors"), tr("Warning: The file you are opening was created on "
									"a newer version of King James Pure Bible Search.  It may contain settings for options not "
									"available on this version of King James Pure Bible Search.  If so, those options will be "
									"ignored.", "Errors"));
	}

	if (toLanguageID(strBblLang) != m_pBibleDatabase->langID()) {
		show();		// Make sure we are visible if this was during construction
		displayWarning(this, tr("Opening King James Search File", "Errors"), tr("Warning: The file you are opening is for a "
									"different language Bible Database and will most likely not display the Search Results "
									"that were intended to have been saved in the KJS file.", "Errors"));
	} else if (strBblUUID.compare(m_pBibleDatabase->compatibilityUUID(), Qt::CaseInsensitive) != 0) {
		show();		// Make sure we are visible if this was during construction
		displayWarning(this, tr("Opening King James Search File", "Errors"), tr("Warning: The file you are opening was created with "
									"a different Bible Database and might have incompatible Search Specification options, potentially yielding "
									"different Search Results from that which was intended to have been saved in the KJS file.", "Errors"));
	}

	m_pSearchSpecWidget->readKJVSearchFile(kjsFile);

	return (kjsFile.status() == QSettings::NoError);
}

bool CKJVCanOpener::saveKJVSearchFile(const QString &strFilePathName) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	QSettings kjsFile(strFilePathName, QSettings::IniFormat);
	if (kjsFile.status() != QSettings::NoError) return false;
#if QT_VERSION < 0x060000
	kjsFile.setIniCodec("UTF-8");
#endif

	kjsFile.clear();

	kjsFile.beginGroup("KJVPureBibleSearch");
	kjsFile.setValue("AppVersion", VER_QT);
	kjsFile.setValue("KJSFileVersion", KJS_FILE_VERSION);
	kjsFile.setValue("BibleDatabase/UUID", m_pBibleDatabase->compatibilityUUID());
	kjsFile.endGroup();

	m_pSearchSpecWidget->writeKJVSearchFile(kjsFile);

	kjsFile.sync();

	return (kjsFile.status() == QSettings::NoError);
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_closingSearchPhrase(CSearchPhraseEdit *pSearchPhrase)
{
	Q_ASSERT(pSearchPhrase != nullptr);

	// If this search phrase's editor was currently active, remove it or else
	//		we'll crash later accessing data for a deleted object:
	if ((m_bPhraseEditorActive) && ((m_pActionSearchPhraseEditMenu != nullptr) &&
									(m_pActionSearchPhraseEditMenu->menu() == pSearchPhrase->phraseEditor()->getEditMenu()))) {
		en_addSearchPhraseEditMenu(false);
	}
}

void CKJVCanOpener::en_phraseChanged(CSearchPhraseEdit *pSearchPhrase)
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

void CKJVCanOpener::en_triggeredSearchWithinGotoIndex(const CRelIndex &relIndex)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	if ((m_pBrowserWidget != nullptr) && (relIndex.isSet())) {
		m_pBrowserWidget->gotoIndex(TPhraseTag(relIndex));
		m_pBrowserWidget->setFocusBrowser();
	}
}

void CKJVCanOpener::en_changedSearchSpec(const CSearchResultsData &searchResultsData)
{
	m_pSearchResultWidget->setParsedPhrases(searchResultsData);		// Setting the phrases will build all of the results and set the verse list on the model
	m_pSearchSpecWidget->enableCopySearchPhraseSummary(true);
	// Auto-switch to Search Results mode:
	if (m_pSearchResultWidget->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS)
		setViewMode(CVerseListModel::VVME_SEARCH_RESULTS, false);

	emit triggerUpdateSearchWindowList();				// Updates this and all other KJVCanOpener lists via myApplication -- it needs to be there so it can also update KJVCanOpeners created or destroyed also
}

void CKJVCanOpener::en_updateBibleDatabasesList()
{
	Q_ASSERT(m_pActionBibleDatabasesList != nullptr);
	Q_ASSERT(m_pActionBibleDatabasesList->menu() != nullptr);

	if (!m_pActionGroupBibleDatabasesList.isNull()) delete m_pActionGroupBibleDatabasesList;
	m_pActionGroupBibleDatabasesList = new QActionGroup(this);

#ifdef ENABLE_ONLY_LOADED_BIBLE_DATABASES
	for (int ndx = 0; ndx < TBibleDatabaseList::instance()->size(); ++ndx) {
		if (TBibleDatabaseList::instance()->at(ndx).isNull()) continue;
		QAction *pAction = new QAction(TBibleDatabaseList::instance()->at(ndx)->description(), m_pActionGroupBibleDatabasesList);
		pAction->setData(TBibleDatabaseList::instance()->at(ndx)->compatibilityUUID());
		if (TBibleDatabaseList::instance()->at(ndx)->compatibilityUUID().compare(m_pBibleDatabase->compatibilityUUID(), Qt::CaseInsensitive) == 0) {
			pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
		}
		m_pActionBibleDatabasesList->menu()->addAction(pAction);
	}
#else
	const QList<TBibleDescriptor> &lstAvailableBBLDescs = TBibleDatabaseList::availableBibleDatabases();
	for (int ndx = 0; ndx < lstAvailableBBLDescs.size(); ++ndx) {
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(lstAvailableBBLDescs.at(ndx).m_strUUID);

		if (!pBibleDatabase.isNull()) {
			QAction *pAction = new QAction(pBibleDatabase->description(), m_pActionGroupBibleDatabasesList);
			pAction->setData(pBibleDatabase->compatibilityUUID());
			if (pBibleDatabase->compatibilityUUID().compare(m_pBibleDatabase->compatibilityUUID(), Qt::CaseInsensitive) == 0) {
				pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
			}
			m_pActionBibleDatabasesList->menu()->addAction(pAction);
		} else {
			Q_ASSERT(lstAvailableBBLDescs.at(ndx).isValid());
			QAction *pAction = new QAction(lstAvailableBBLDescs.at(ndx).m_strDBDesc, m_pActionGroupBibleDatabasesList);
			pAction->setData(lstAvailableBBLDescs.at(ndx).m_strUUID);
			m_pActionBibleDatabasesList->menu()->addAction(pAction);
		}
	}
#endif

	connect(m_pActionGroupBibleDatabasesList.data(), SIGNAL(triggered(QAction*)), this, SLOT(en_NewCanOpener(QAction*)));
}

void CKJVCanOpener::en_updateSearchWindowList()
{
	Q_ASSERT(m_pActionSearchWindowList != nullptr);
	Q_ASSERT(m_pActionSearchWindowList->menu() != nullptr);

	if (m_pActionGroupSearchWindowList != nullptr) delete m_pActionGroupSearchWindowList;
	m_pActionGroupSearchWindowList = new QActionGroup(this);

	const QList<CKJVCanOpener *> &lstCanOpeners = g_pMyApplication->canOpeners();
	for (int ndx = 0; ndx < lstCanOpeners.size(); ++ndx) {
		QAction *pAction = new QAction(QString("%1 [%2]").arg(lstCanOpeners.at(ndx)->searchWindowDescription()).arg(lstCanOpeners.at(ndx)->m_pBibleDatabase->description()), m_pActionGroupSearchWindowList);
		pAction->setData(ndx);
		m_pActionSearchWindowList->menu()->addAction(pAction);
	}
	connect(m_pActionGroupSearchWindowList.data(), SIGNAL(triggered(QAction*)), g_pMyApplication.data(), SLOT(en_triggeredKJVCanOpener(QAction*)));
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_addPassageBrowserEditMenu(bool bAdd, bool bPassageReferenceEditor)
{
	m_bBrowserActive = bAdd;

	if (!bPassageReferenceEditor) {
		if (bAdd) {
			if (m_pActionPassageBrowserEditMenu == nullptr) {
				m_pActionPassageBrowserEditMenu = ui.menuBar->insertMenu(m_pViewMenu->menuAction(), m_pBrowserWidget->getEditMenu(false));
				connect(m_pActionPassageBrowserEditMenu, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusBrowser()));
			}
		} else {
			if (m_pActionPassageBrowserEditMenu) {
				// The following 'if' is needed for insert race conditions to
				//		keep us from crashing:
				if (ui.menuBar->actions().contains(m_pActionPassageBrowserEditMenu))
					ui.menuBar->removeAction(m_pActionPassageBrowserEditMenu);
				m_pActionPassageBrowserEditMenu = nullptr;
			}
		}
	} else {
		if (bAdd) {
			if (m_pActionPassageReferenceEditMenu == nullptr) {
				m_pActionPassageReferenceEditMenu = ui.menuBar->insertMenu(m_pViewMenu->menuAction(), m_pBrowserWidget->getEditMenu(true));
				connect(m_pActionPassageReferenceEditMenu, SIGNAL(triggered()), m_pBrowserWidget, SLOT(setFocusPassageReferenceEditor()));
			}
		} else {
			if (m_pActionPassageReferenceEditMenu) {
				// The following 'if' is needed for insert race conditions to
				//		keep us from crashing:
				if (ui.menuBar->actions().contains(m_pActionPassageReferenceEditMenu))
					ui.menuBar->removeAction(m_pActionPassageReferenceEditMenu);
				m_pActionPassageReferenceEditMenu = nullptr;
			}
		}
	}
}

void CKJVCanOpener::en_addSearchResultsEditMenu(bool bAdd)
{
	m_bSearchResultsActive = bAdd;

	if (bAdd) {
		if (m_pActionSearchResultsEditMenu == nullptr) {
			m_pActionSearchResultsEditMenu = ui.menuBar->insertMenu(m_pViewMenu->menuAction(), m_pSearchResultWidget->getEditMenu());
		}
	} else {
		if (m_pActionSearchResultsEditMenu) {
			// The following 'if' is needed for insert race conditions to
			//		keep us from crashing:
			if (ui.menuBar->actions().contains(m_pActionSearchResultsEditMenu))
				ui.menuBar->removeAction(m_pActionSearchResultsEditMenu);
			m_pActionSearchResultsEditMenu = nullptr;
		}
	}
}

void CKJVCanOpener::en_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor)
{
	m_bPhraseEditorActive = bAdd;

	if (m_pActionSearchPhraseEditMenu) {
		// The following 'if' is needed for insert race conditions to
		//		keep us from crashing:
		if (ui.menuBar->actions().contains(m_pActionSearchPhraseEditMenu))
			ui.menuBar->removeAction(m_pActionSearchPhraseEditMenu);
		m_pActionSearchPhraseEditMenu = nullptr;
	}
	if ((bAdd) && (pEditor != nullptr)) {
		m_pActionSearchPhraseEditMenu = ui.menuBar->insertMenu(m_pViewMenu->menuAction(), pEditor->getEditMenu());
	}
}

void CKJVCanOpener::en_addDictionaryEditMenu(bool bAdd, bool bWordEditor)
{
	m_bDictionaryActive = bAdd;

	if (m_pDictionaryWidget == nullptr) {
		Q_ASSERT(bAdd == false);			// We shouldn't be receiving a dictionary activiation menu add if we don't even have a dictionary widget
		return;
	}

#if defined(USING_DICTIONARIES) && !defined(IS_CONSOLE_APP)
	if (!bWordEditor) {
		if (bAdd) {
			if (m_pActionDictionaryEditMenu == nullptr) {
				m_pActionDictionaryEditMenu = ui.menuBar->insertMenu(m_pViewMenu->menuAction(), m_pDictionaryWidget->getEditMenu(false));
			}
		} else {
			if (m_pActionDictionaryEditMenu) {
				// The following 'if' is needed for insert race conditions to
				//		keep us from crashing:
				if (ui.menuBar->actions().contains(m_pActionDictionaryEditMenu))
					ui.menuBar->removeAction(m_pActionDictionaryEditMenu);
				m_pActionDictionaryEditMenu = nullptr;
			}
		}
	} else {
		if (bAdd) {
			if (m_pActionDictWordEditMenu == nullptr) {
				m_pActionDictWordEditMenu = ui.menuBar->insertMenu(m_pViewMenu->menuAction(), m_pDictionaryWidget->getEditMenu(true));
			}
		} else {
			if (m_pActionDictWordEditMenu) {
				// The following 'if' is needed for insert race conditions to
				//		keep us from crashing:
				if (ui.menuBar->actions().contains(m_pActionDictWordEditMenu))
					ui.menuBar->removeAction(m_pActionDictWordEditMenu);
				m_pActionDictWordEditMenu = nullptr;
			}
		}
	}
#else
	Q_UNUSED(bWordEditor);
#endif
}

void CKJVCanOpener::en_activatedBrowser(bool bPassageReferenceEditor)
{
	m_pSearchSpecWidget->en_activatedPhraseEditor(nullptr);		// Notify that we have no search phrase editor active
	en_addPassageBrowserEditMenu(false, !bPassageReferenceEditor);
	en_addPassageBrowserEditMenu(true, bPassageReferenceEditor);
	en_addSearchResultsEditMenu(false);
	en_addSearchPhraseEditMenu(false);
	en_addDictionaryEditMenu(false, false);
	en_addDictionaryEditMenu(false, true);
	setDetailsEnable();
	setGematriaEnable();
}

void CKJVCanOpener::en_activatedSearchResults()
{
	m_pSearchSpecWidget->en_activatedPhraseEditor(nullptr);		// Notify that we have no search phrase editor active
	en_addPassageBrowserEditMenu(false, false);
	en_addPassageBrowserEditMenu(false, true);
	en_addSearchResultsEditMenu(true);
	en_addSearchPhraseEditMenu(false);
	en_addDictionaryEditMenu(false, false);
	en_addDictionaryEditMenu(false, true);
	setDetailsEnable();
	setGematriaEnable();
}

void CKJVCanOpener::en_activatedPhraseEditor(const CPhraseLineEdit *pEditor)
{
	en_addPassageBrowserEditMenu(false, false);
	en_addPassageBrowserEditMenu(false, true);
	en_addSearchResultsEditMenu(false);
	en_addSearchPhraseEditMenu(true, pEditor);
	en_addDictionaryEditMenu(false, false);
	en_addDictionaryEditMenu(false, true);
	setDetailsEnable();
	setGematriaEnable();
}

void CKJVCanOpener::en_activatedDictionary(bool bWordEditor)
{
	m_pSearchSpecWidget->en_activatedPhraseEditor(nullptr);		// Notify that we have no search phrase editor active
	en_addPassageBrowserEditMenu(false, false);
	en_addPassageBrowserEditMenu(false, true);
	en_addSearchResultsEditMenu(false);
	en_addSearchPhraseEditMenu(false);
	en_addDictionaryEditMenu(false, !bWordEditor);
	en_addDictionaryEditMenu(true, bWordEditor);
	setDetailsEnable();
	setGematriaEnable();
}

bool CKJVCanOpener::isBrowserFocusedOrActive() const
{
	Q_ASSERT(m_pBrowserWidget != nullptr);

	return (m_pBrowserWidget->hasFocusBrowser() || m_pBrowserWidget->hasFocusPassageReferenceEditor() || isBrowserActive());
}

bool CKJVCanOpener::isSearchResultsFocusedOrActive() const
{
	Q_ASSERT(m_pSearchResultWidget != nullptr);

	return (m_pSearchResultWidget->hasFocusSearchResult() || isSearchResultsActive());
}

bool CKJVCanOpener::isPhraseEditorFocusedOrActive() const
{
	return (isPhraseEditorActive());			// TODO : Add PhraseEditor hasFocus() when it's actually needed
}

bool CKJVCanOpener::isDictionaryFocusedOrActive() const
{
	return (isDictionaryActive());				// TODO : Add Dictionary hasFocus() when it's actually needed
}

// ------------------------------------------------------------------

void CKJVCanOpener::en_viewModeChange(QAction *pAction, bool bFocusTree)
{
	Q_ASSERT(pAction != nullptr);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	TVerseIndex ndxCurrent(m_pSearchResultWidget->currentVerseIndex());

	m_pSearchResultWidget->setViewMode(static_cast<CVerseListModel::VERSE_VIEW_MODE_ENUM>(pAction->data().toUInt()));

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent, bFocusTree);
}

void CKJVCanOpener::en_nextViewMode()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CVerseListModel::VERSE_VIEW_MODE_ENUM nNewMode = m_pSearchResultWidget->viewMode();

	switch (m_pSearchResultWidget->viewMode()) {
		case CVerseListModel::VVME_SEARCH_RESULTS:
			nNewMode = CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED;
			break;
		case CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED:
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
			nNewMode = CVerseListModel::VVME_HIGHLIGHTERS;
#else
			nNewMode = CVerseListModel::VVME_SEARCH_RESULTS;
#endif
			break;
		case CVerseListModel::VVME_HIGHLIGHTERS:
			nNewMode = CVerseListModel::VVME_USERNOTES;
			break;
		case CVerseListModel::VVME_USERNOTES:
			nNewMode = CVerseListModel::VVME_CROSSREFS;
			break;
		case CVerseListModel::VVME_CROSSREFS:
			nNewMode = CVerseListModel::VVME_SEARCH_RESULTS;
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	setViewMode(nNewMode, false);
	m_pSearchResultWidget->setViewMode(nNewMode);

	m_bDoingUpdate = false;
}

void CKJVCanOpener::en_displayModeChange(QAction *pAction)
{
	Q_ASSERT(pAction != nullptr);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	TVerseIndex ndxCurrent(m_pSearchResultWidget->currentVerseIndex());

	m_pSearchResultWidget->setDisplayMode(static_cast<CVerseListModel::VERSE_DISPLAY_MODE_ENUM>(pAction->data().toUInt()));

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::en_nextDisplayMode()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nNewMode = m_pSearchResultWidget->displayMode();

	switch (m_pSearchResultWidget->displayMode()) {
		case CVerseListModel::VDME_HEADING:
			nNewMode = CVerseListModel::VDME_RICHTEXT;
			break;
		case CVerseListModel::VDME_RICHTEXT:
			nNewMode = CVerseListModel::VDME_HEADING;
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	setDisplayMode(nNewMode);
	m_pSearchResultWidget->setDisplayMode(nNewMode);

	m_bDoingUpdate = false;
}

void CKJVCanOpener::en_treeModeChange(QAction *pAction)
{
	Q_ASSERT(pAction != nullptr);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	TVerseIndex ndxCurrent(m_pSearchResultWidget->currentVerseIndex());

	m_pSearchResultWidget->setTreeMode(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(pAction->data().toUInt()));
	m_pActionShowMissingLeafs->setEnabled(static_cast<CVerseListModel::VERSE_TREE_MODE_ENUM>(pAction->data().toUInt()) != CVerseListModel::VTME_LIST);

	m_bDoingUpdate = false;

	m_pSearchResultWidget->setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::en_nextTreeMode()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;


	CVerseListModel::VERSE_TREE_MODE_ENUM nNewMode = m_pSearchResultWidget->treeMode();

	switch (m_pSearchResultWidget->treeMode()) {
		case CVerseListModel::VTME_LIST:
			nNewMode = CVerseListModel::VTME_TREE_BOOKS;
			break;
		case CVerseListModel::VTME_TREE_BOOKS:
			nNewMode = CVerseListModel::VTME_TREE_CHAPTERS;
			break;
		case CVerseListModel::VTME_TREE_CHAPTERS:
			nNewMode = CVerseListModel::VTME_LIST;
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	setTreeMode(nNewMode);
	m_pSearchResultWidget->setTreeMode(nNewMode);
	m_pActionShowMissingLeafs->setEnabled(nNewMode != CVerseListModel::VTME_LIST);

	m_bDoingUpdate = false;
}

void CKJVCanOpener::en_viewShowMissingsLeafs()
{
	Q_ASSERT(m_pActionShowMissingLeafs != nullptr);

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
	Q_ASSERT(m_pActionBookBackward != nullptr);
	Q_ASSERT(m_pActionBookForward != nullptr);
	Q_ASSERT(m_pActionChapterBackward != nullptr);
	Q_ASSERT(m_pActionChapterForward != nullptr);
	if ((m_pActionBookBackward == nullptr) ||
		(m_pActionBookForward == nullptr) ||
		(m_pActionChapterBackward == nullptr) ||
		(m_pActionChapterForward == nullptr)) return;

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
			m_pActionNavBackward->setStatusTip(tr("Go to: %1", "MainMenu").arg(m_pBrowserWidget->historyTitle(-1)));
		} else {
			m_pActionNavBackward->setStatusTip(tr("Go Backward in Navigation History", "MainMenu"));
		}
	}
	if (m_pActionNavForward) {
		m_pActionNavForward->setToolTip(m_pBrowserWidget->historyTitle(+1));
		if (m_pBrowserWidget->isForwardAvailable()) {
			m_pActionNavForward->setStatusTip(tr("Go to: %1", "MainMenu").arg(m_pBrowserWidget->historyTitle(+1)));
		} else {
			m_pActionNavForward->setStatusTip(tr("Go Forward in Navigation History", "MainMenu"));
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
	Q_ASSERT(!m_pBibleDatabase.isNull());

	if (!index.isValid()) return;

	CRelIndex ndxRel(m_pSearchResultWidget->vlmodel()->navigationIndexForModelIndex(index));

	if (!ndxRel.isSet()) return;				// If user double-clicks on a highligher, there will be no RelIndex

	m_pBrowserWidget->gotoIndex(TPhraseTag(ndxRel));
	m_pBrowserWidget->setFocusBrowser();
}

void CKJVCanOpener::en_PassageNavigatorTriggered()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	if (isBrowserFocusedOrActive()) {
		m_pBrowserWidget->showPassageNavigator();
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->editableNodeSelected())) {
		m_pSearchResultWidget->showPassageNavigator();
	} else {
#ifndef USE_ASYNC_DIALOGS
		CKJVCanOpenerCloseGuard closeGuard(this);
		CPassageNavigatorDlgPtr pDlg(m_pBibleDatabase, this);

		if (pDlg->exec() == QDialog::Accepted) {
			if (pDlg != nullptr) {			// Could get deleted during execution
				m_pBrowserWidget->gotoIndex(pDlg->passage());
				m_pBrowserWidget->setFocusBrowser();
			}
		}
#else
		CPassageNavigatorDlg *pDlg = new CPassageNavigatorDlg(m_pBibleDatabase, this);
		connect(pDlg, SIGNAL(gotoIndex(const TPhraseTag &)), m_pBrowserWidget, SLOT(gotoIndex(const TPhraseTag &)));
		connect(pDlg, SIGNAL(gotoIndex(const TPhraseTag &)), m_pBrowserWidget, SLOT(setFocusBrowser()));
		pDlg->show();
#endif
	}
}

void CKJVCanOpener::en_gotoRandomPassage()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	CRelIndex ndxPassage;

#ifdef USE_STD_RANDOM
	static std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
#endif

	bool bDone = false;
	while (!bDone) {
		if (CPersistentSettings::instance()->randomPassageWeightMode() == RPWE_VERSE_WEIGHT) {
			// The following version creates random passage evenly weighting all verses, which causes
			//		books with more verses to be weighted higher than those with less:
#ifdef USE_STD_RANDOM
			std::uniform_int_distribution<unsigned int> distributionPassage(1, m_pBibleDatabase->bibleEntry().m_nNumWrd);
			unsigned int nPassage = distributionPassage(generator);
#else
			unsigned int nPassage = ((static_cast<unsigned int>(rand()) % (m_pBibleDatabase->bibleEntry().m_nNumWrd * 2)) / 2) + 1;
#endif
			ndxPassage = m_pBibleDatabase->DenormalizeIndex(nPassage);
			if (!ndxPassage.isSet()) continue;
			ndxPassage.setWord(0);
		} else if (CPersistentSettings::instance()->randomPassageWeightMode() == RPWE_EVEN_WEIGHT) {
			// Create Random Passage by even distribution of book, chapter, verse:
#ifdef USE_STD_RANDOM
			std::uniform_int_distribution<unsigned int> distributionBook(1, m_pBibleDatabase->bibleEntry().m_nNumBk);
			unsigned int nBook = distributionBook(generator);
#else
			unsigned int nBook = ((static_cast<unsigned int>(rand()) % (m_pBibleDatabase->bibleEntry().m_nNumBk * 2)) / 2) + 1;
#endif
			ndxPassage.setBook(nBook);
			const CBookEntry *pBook = m_pBibleDatabase->bookEntry(nBook);
			if (pBook == nullptr) continue;

#ifdef USE_STD_RANDOM
			std::uniform_int_distribution<unsigned int> distributionChapter(1, pBook->m_nNumChp);
			unsigned int nChapter = distributionChapter(generator);
#else
			unsigned int nChapter = ((static_cast<unsigned int>(rand()) % (pBook->m_nNumChp * 2)) / 2) + 1;
#endif
			ndxPassage.setChapter(nChapter);
			const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndxPassage);
			if (pChapter == nullptr) continue;

#ifdef USE_STD_RANDOM
			std::uniform_int_distribution<unsigned int> distributionVerse(1, pChapter->m_nNumVrs);
			unsigned int nVerse = distributionVerse(generator);
#else
			unsigned int nVerse = ((static_cast<unsigned int>(rand()) % (pChapter->m_nNumVrs * 2)) / 2) + 1;
#endif
			ndxPassage.setVerse(nVerse);
			if (m_pBibleDatabase->NormalizeIndex(ndxPassage) == 0) continue;
		} else {
			// Unknown Random Passage Weight Mode
			Q_ASSERT(false);
		}

		bDone = true;
	}

	if (ndxPassage.isSet()) {
		m_pBrowserWidget->gotoIndex(TPhraseTag(ndxPassage, 0));
		m_pBrowserWidget->setFocusBrowser();
	}
}

void CKJVCanOpener::en_userNoteEditorTriggered()
{
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	if (!isActiveWindow()) return;
	if ((!isBrowserFocusedOrActive()) && (!isSearchResultsFocusedOrActive())) return;

	Q_ASSERT(m_pUserNoteEditorDlg != nullptr);
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	if ((m_pUserNoteEditorDlg == nullptr) || (g_pUserNotesDatabase.isNull())) return;

	CRelIndex indexNote;

	if (isBrowserFocusedOrActive()) {
		indexNote = m_pBrowserWidget->selection().primarySelection().relIndex();
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->editableNodeSelected()))  {
		indexNote = m_pSearchResultWidget->vlmodel()->logicalIndexForModelIndex(m_pSearchResultWidget->currentIndex());
	}

	if (!indexNote.isSet()) return;
	m_pUserNoteEditorDlg->setLocationIndex(indexNote);
#ifndef USE_ASYNC_DIALOGS
	CKJVCanOpenerCloseGuard closeGuard(this);
	m_pUserNoteEditorDlg->exec();
#else
	// TODO: Add closeGuard logic for this:
	m_pUserNoteEditorDlg->show();
#endif

#endif
}

void CKJVCanOpener::en_crossRefsEditorTriggered()
{
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	if (!isActiveWindow()) return;
	if ((!isBrowserFocusedOrActive()) && (!isSearchResultsFocusedOrActive())) return;

	Q_ASSERT(m_pCrossRefsEditorDlg != nullptr);
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	if ((m_pCrossRefsEditorDlg == nullptr) || (g_pUserNotesDatabase.isNull())) return;

	TPassageTag tagCrossRef;

	if (isBrowserFocusedOrActive()) {
		tagCrossRef.setFromPhraseTag(m_pBibleDatabase.data(), m_pBrowserWidget->selection().primarySelection());
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->editableNodeSelected())) {
		// Unlike editing notes and passage navigation, editing cross-references should bring up the "Source" Cross-Reference:
		tagCrossRef = TPassageTag(m_pSearchResultWidget->currentVerseIndex().relIndex());
	}

	if (!tagCrossRef.isSet()) return;
	m_pCrossRefsEditorDlg->setSourcePassage(tagCrossRef);
#ifndef USE_ASYNC_DIALOGS
	CKJVCanOpenerCloseGuard closeGuard(this);
	m_pCrossRefsEditorDlg->exec();
#else
	// TODO: Add closeGuard logic for this:
	m_pCrossRefsEditorDlg->show();
#endif

#endif
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

void CKJVCanOpener::en_viewGematria()
{
#ifdef USE_GEMATRIA
	if ((isBrowserFocusedOrActive()) && (m_pBrowserWidget->haveGematria())) {
		m_pBrowserWidget->showGematria();
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->haveGematria())) {
		m_pSearchResultWidget->showGematria();
	}
#endif
}

void CKJVCanOpener::setGematriaEnable()
{
	bool bGematriaEnable = false;

#ifdef USE_GEMATRIA
	if ((isBrowserFocusedOrActive()) && (m_pBrowserWidget->haveGematria())) {
		bGematriaEnable = true;
	} else if ((isSearchResultsFocusedOrActive()) && (m_pSearchResultWidget->haveGematria())) {
		bGematriaEnable = true;
	}
#endif

	emit canShowGematria(bGematriaEnable);
}

// ------------------------------------------------------------------

int CKJVCanOpener::confirmFollowLink()
{
	return QMessageBox::question(this, windowTitle(), tr("Following this link will launch an external browser on your system.  "
														"Doing so may incur extra charges from your service provider.\n\n"
														"Do you wish to follow this link?", "Errors"),
														QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
														QMessageBox::No);
}

void CKJVCanOpener::en_HelpManual()
{
#if defined(EMSCRIPTEN)
	QDesktopServices::openUrl(QUrl(g_constrHelpDocFilename));
#elif defined(VNCSERVER)
#elif defined(Q_OS_ANDROID)
	if (confirmFollowLink() == QMessageBox::Yes) {
		QDesktopServices::openUrl(QUrl(g_constrHelpDocFilename));
	}
#else
	QFileInfo fiHelpDoc(initialAppDirPath(), g_constrHelpDocFilename);
	if ((!fiHelpDoc.exists()) || (!QDesktopServices::openUrl(QUrl::fromLocalFile(fiHelpDoc.absoluteFilePath())))) {
		displayWarning(this, windowTitle(), tr("Unable to open the King James Pure Bible Search Users Manual.\n"
													 "Verify that you have a PDF Viewer, such as Adobe Acrobat, installed.\n"
													 "And check installation of King James Pure Bible Search User Manual at:\n\n"
													 "%1", "Errors").arg(QDir::toNativeSeparators(fiHelpDoc.absoluteFilePath())));
	}
#endif
}

void CKJVCanOpener::en_HelpAbout()
{
#ifndef USE_ASYNC_DIALOGS
	CKJVCanOpenerCloseGuard closeGuard(this);
	CAboutDlgPtr pDlg(this);
	pDlg->exec();
#else
	CAboutDlg *pDlg = new CAboutDlg(this);
	pDlg->show();
#endif
}

void CKJVCanOpener::en_PureBibleSearchDotCom()
{
#ifndef VNCSERVER
	if (confirmFollowLink() == QMessageBox::Yes) {
		if (!QDesktopServices::openUrl(QUrl(g_constrPureBibleSearchURL))) {
#ifndef EMSCRIPTEN
			displayWarning(this, windowTitle(), tr("Unable to open a System Web Browser for\n\n"
														 "%1", "Errors").arg(g_constrPureBibleSearchURL));
#endif
		}
	}
#endif
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
					Q_ASSERT(false);
					break;
			}
			bServiced = true;
			break;
		}
	}

	Q_ASSERT(bServiced);
}

void CKJVCanOpener::en_Configure(int nInitialPage)
{
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
	Q_ASSERT(!g_pMyApplication.isNull());

	const QList<CKJVCanOpener *> &lstCanOpeners = g_pMyApplication->canOpeners();

	for (int ndxCanOpener = 0; ndxCanOpener < lstCanOpeners.size(); ++ndxCanOpener) {
		CHighlighterButtons *pHighlighterButtons = lstCanOpeners.at(ndxCanOpener)->highlighterButtons();
		if (pHighlighterButtons != nullptr) pHighlighterButtons->enterConfigurationMode();
	}

	QPointer<CConfigurationDialog> pDlgConfigure = new CConfigurationDialog(m_pBibleDatabase, ((m_pDictionaryWidget != nullptr) ? m_pDictionaryWidget->dictionaryDatabase() : CDictionaryDatabasePtr()), this, static_cast<CONFIGURATION_PAGE_SELECTION_ENUM>(nInitialPage));

	auto &&fnCompletion = [this, lstCanOpeners, pDlgConfigure](int nResult)->void {
		Q_UNUSED(nResult);
		for (int ndxCanOpener = 0; ndxCanOpener < lstCanOpeners.size(); ++ndxCanOpener) {
			CHighlighterButtons *pHighlighterButtons = lstCanOpeners.at(ndxCanOpener)->highlighterButtons();
			if (pHighlighterButtons != nullptr) pHighlighterButtons->leaveConfigurationMode();
		}

		Q_ASSERT(!pDlgConfigure.isNull());
		if (pDlgConfigure) {
			if (pDlgConfigure->restartApp()) {
#if QT_VERSION >= 0x050400		// Functor calls was introduced in Qt 5.4
				QTimer::singleShot(10, this, [this]()->void { g_pMyApplication->restartApp(this); });
#else
				Q_UNUSED(this);
				QTimer::singleShot(10, g_pMyApplication.data(), SLOT(restartApp()));
#endif
			}
			pDlgConfigure->deleteLater();
		}
	};

#ifndef USE_ASYNC_DIALOGS

	pDlgConfigure->exec();
	fnCompletion(0);

#else

	connect(pDlgConfigure, &CConfigurationDialog::finished, fnCompletion);
	pDlgConfigure->setAttribute(Qt::WA_DeleteOnClose, false);
	pDlgConfigure->setAttribute(Qt::WA_ShowModal, true);
	pDlgConfigure->show();

#endif

#else
	Q_UNUSED(nInitialPage);
#endif
}

void CKJVCanOpener::en_LaunchGeneralSettingsConfig()
{
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
	en_Configure(CPSE_GENERAL_SETTINGS);
#endif
}

void CKJVCanOpener::en_LaunchCopyOptionsConfig()
{
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
	en_Configure(CPSE_COPY_OPTIONS);
#endif
}

void CKJVCanOpener::en_LaunchTextColorAndFontsConfig()
{
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
	en_Configure(CPSE_TEXT_FORMAT);
#endif
}

void CKJVCanOpener::en_LaunchNotesFileSettingsConfig()
{
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) && !defined(VNCSERVER)
	en_Configure(CPSE_USER_NOTES_DATABASE);
#endif
}

void CKJVCanOpener::en_LaunchBibleDatabaseConfig()
{
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
	en_Configure(CPSE_BIBLE_DATABASE);
#endif
}

void CKJVCanOpener::en_LaunchDictDatabaseConfig()
{
#if defined(USING_DICTIONARIES) && !defined(IS_CONSOLE_APP)
	en_Configure(CPSE_DICT_DATABASE);
#endif
}

void CKJVCanOpener::en_LaunchLocaleSettingsConfig()
{
#if (!defined(EMSCRIPTEN) && !defined(IS_CONSOLE_APP)) || defined(Q_OS_WASM)
	en_Configure(CPSE_LOCALE);
#endif
}

void CKJVCanOpener::en_LaunchTTSOptionsConfig()
{
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	en_Configure(CPSE_TTS_OPTIONS);
#endif
}

void CKJVCanOpener::en_NewCanOpener(QAction *pAction)
{
	Q_ASSERT(!g_pMyApplication.isNull());

	BROWSER_DISPLAY_MODE_ENUM nBrowserDisplayMode = m_pBrowserWidget->browserDisplayMode();
	CKJVCanOpener *pNewCanOpener = nullptr;

	if (pAction != nullptr) {
		QString strUUID = pAction->data().toString();

		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strUUID);
#ifndef ENABLE_ONLY_LOADED_BIBLE_DATABASES
		if (pBibleDatabase.isNull()) {
			if (TBibleDatabaseList::instance()->loadBibleDatabase(strUUID, false, this)) {
				pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strUUID);
				Q_ASSERT(!pBibleDatabase.isNull());
			} else {
				return;
			}
		}
#else
		Q_ASSERT(!pBibleDatabase.isNull());
#endif

		pNewCanOpener = g_pMyApplication->createKJVCanOpener(pBibleDatabase);
	} else {
		Q_ASSERT(!TBibleDatabaseList::instance()->mainBibleDatabase().isNull());
		pNewCanOpener = g_pMyApplication->createKJVCanOpener(TBibleDatabaseList::instance()->mainBibleDatabase());
	}

	// Set the DisplayMode of the new browser to the same as this browser:
	Q_ASSERT(pNewCanOpener != nullptr);
#if QT_VERSION >= 0x050400
	QTimer::singleShot(1, this, [pNewCanOpener, nBrowserDisplayMode]() {
		Q_ASSERT(pNewCanOpener->m_pBrowserWidget != nullptr);
		pNewCanOpener->m_pBrowserWidget->setBrowserDisplayMode(nBrowserDisplayMode);
	} );
#else
	Q_UNUSED(nBrowserDisplayMode);
#endif
}

#ifdef USING_QT_SPEECH

void CKJVCanOpener::en_speechPause()
{
	// TODO ?
}

void CKJVCanOpener::en_speechStop()
{
	Q_ASSERT(!g_pMyApplication.isNull());
	QtSpeech *pSpeech = g_pMyApplication->speechSynth();
	if ((pSpeech != nullptr) && (pSpeech->isTalking())) pSpeech->clearQueue();
}

void CKJVCanOpener::setSpeechActionEnables()
{
	Q_ASSERT(!g_pMyApplication.isNull());
	QtSpeech *pSpeech = g_pMyApplication->speechSynth();

	if (pSpeech != nullptr) {
		if (actionSpeechStop() != nullptr) {
			actionSpeechStop()->setEnabled(pSpeech->isTalking());
		}
	}
}

#endif
