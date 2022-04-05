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

#ifndef KJV_CAN_OPENER_H
#define KJV_CAN_OPENER_H

#include "dbstruct.h"

#include "SearchPhraseEdit.h"
#include "VerseListModel.h"
#include "SearchSpecWidget.h"
#include "SearchResults.h"
#include "BrowserWidget.h"

#include <QMainWindow>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QEvent>
#include <QString>
#include <QSplitter>
#include <QPointer>
#include <QToolBar>

// ============================================================================

// Forward Declares:
class CNoteEditDlg;
class CCrossRefEditDlg;
class CHighlighterButtons;
class CTipEdit;
class CDictionaryWidget;
#ifdef USE_GEOMAP
class CGeoMap;
#endif

// ============================================================================

#include "ui_KJVCanOpener.h"

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(CBibleDatabasePtr pBibleDatabase, QWidget *parent = nullptr);
	~CKJVCanOpener();

	CBibleDatabasePtr bibleDatabase() const { return m_pBibleDatabase; }
	CDictionaryDatabasePtr dictionaryDatabase() const;

	bool isBrowserActive() const { return m_bBrowserActive; }
	bool isSearchResultsActive() const { return m_bSearchResultsActive; }
	bool isPhraseEditorActive() const { return m_bPhraseEditorActive; }
	bool isDictionaryActive() const { return m_bDictionaryActive; }

	bool isBrowserFocusedOrActive() const;
	bool isSearchResultsFocusedOrActive() const;
	bool isPhraseEditorFocusedOrActive() const;
	bool isDictionaryFocusedOrActive() const;

	CHighlighterButtons *highlighterButtons() const { return m_pHighlighterButtons; }
	QAction *actionUserNoteEditor() const { return m_pActionUserNoteEditor; }
	QAction *actionCrossRefsEditor() const { return m_pActionCrossRefsEditor; }
	QAction *actionSpeechPlay() const { return m_pActionSpeechPlay; }
	QAction *actionSpeechPause() const { return m_pActionSpeechPause; }
	QAction *actionSpeechStop() const { return m_pActionSpeechStop; }
	QAction *actionSpeakSelection() const { return m_pActionSpeakSelection; }

	bool canClose() const { return m_bCanClose; }
	QString searchWindowDescription() const;		// Return descriptive description for this window for the application search window list

	friend class CKJVCanOpenerCloseGuard;
	class CKJVCanOpenerCloseGuard {
	public:
		CKJVCanOpenerCloseGuard(CKJVCanOpener *pCanOpener)
			:	m_pCanOpener(pCanOpener)
		{
			Q_ASSERT(!m_pCanOpener.isNull());
			m_bPreviousCanClose = m_pCanOpener->canClose();
			m_pCanOpener->setCanClose(false);
		}

		~CKJVCanOpenerCloseGuard()
		{
			Q_ASSERT(!m_pCanOpener.isNull());
			if (!m_pCanOpener.isNull()) m_pCanOpener->setCanClose(m_bPreviousCanClose);
		}

	private:
		QPointer<CKJVCanOpener> m_pCanOpener;
		bool m_bPreviousCanClose;
	};

protected slots:
	void savePersistentSettings(bool bSaveLastSearchOnly = false);
	void restorePersistentSettings(bool bAppRestarting = false);

	void setCanClose(bool bCanClose)
	{
		m_bCanClose = bCanClose;
		emit canCloseChanged(this, m_bCanClose);
	}

protected:
	void initialize();

	virtual void closeEvent(QCloseEvent * event) override;
	virtual bool event(QEvent *pEvent) override;

	friend class CTipEdit;
	CTipEdit *tipEdit(TIP_EDIT_TYPE_ENUM nTipType) const { return m_pTipEdit[nTipType]; }
	void setTipEdit(TIP_EDIT_TYPE_ENUM nTipType, CTipEdit *pTipEdit) { m_pTipEdit[nTipType] = pTipEdit; }

	bool tipEditIsPinned(TIP_EDIT_TYPE_ENUM nTipType) const { return m_bTipEditIsPinned[nTipType]; }
	void setTipEditIsPinned(TIP_EDIT_TYPE_ENUM nTipType, bool bIsPinned) { m_bTipEditIsPinned[nTipType] = bIsPinned; }

signals:
	void changedSearchResults();
	void canShowDetails(bool bHaveDetails);
	void canShowGematria(bool bHaveGematria);
	void windowActivated(CKJVCanOpener *pCanOpener);
	void canCloseChanged(CKJVCanOpener *pCanOpener, bool bCanClose);
	void isClosing(CKJVCanOpener *pCanOpener);
	void triggerUpdateSearchWindowList();

public:
	static QString determineBibleUUIDForKJVSearchFile(const QString &strFilePathName);
	int confirmFollowLink();					// Returns either QMessageBox::Yes or QMessageBox::No

public slots:
	bool openKJVSearchFile(const QString &strFilePathName);
	bool saveKJVSearchFile(const QString &strFilePathName) const;

	void setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode, bool bFocusTree = true);
	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setShowMissingLeafs(bool bShowMissing);

	void en_updateBibleDatabasesList();
	void en_updateSearchWindowList();

protected slots:
	void en_NewSearch();
	void en_OpenSearch();
	void en_SaveSearch();
	void en_ClearSearchPhrases();

	void en_closingSearchPhrase(CSearchPhraseEdit *pSearchPhrase);
	void en_phraseChanged(CSearchPhraseEdit *pSearchPhrase);
	void en_copySearchPhraseSummary();
	void en_triggeredSearchWithinGotoIndex(const CRelIndex &relIndex);
	void en_changedSearchSpec(const CSearchResultsData &searchResultsData);

	void en_addPassageBrowserEditMenu(bool bAdd, bool bPassageReferenceEditor);
	void en_addSearchResultsEditMenu(bool bAdd);
	void en_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor = nullptr);
	void en_addDictionaryEditMenu(bool bAdd, bool bWordEditor);
	void en_activatedBrowser(bool bPassageReferenceEditor);
	void en_activatedSearchResults();
	void en_activatedPhraseEditor(const CPhraseLineEdit *pEditor);
	void en_activatedDictionary(bool bWordEditor);

	void en_viewModeChange(QAction *pAction, bool bFocusTree = true);
	void en_nextViewMode();
	void en_displayModeChange(QAction *pAction);
	void en_nextDisplayMode();
	void en_treeModeChange(QAction *pAction);
	void en_nextTreeMode();
	void en_viewShowMissingsLeafs();

	void en_gotoIndex(const TPhraseTag &tag);
	void en_browserHistoryChanged();
	void en_clearBrowserHistory();

	void en_SearchResultActivated(const QModelIndex &index);		// Enter or double-click activated

	void en_PassageNavigatorTriggered();
	void en_gotoRandomPassage();

	void en_userNoteEditorTriggered();
	void en_crossRefsEditorTriggered();

	void en_viewDetails();
	void setDetailsEnable();

	void en_viewGematria();
	void setGematriaEnable();

	void en_viewGeoMap();
	void setGeoMapEnable(bool bEnable);

	void en_HelpManual();
	void en_HelpAbout();
	void en_PureBibleSearchDotCom();

	void en_QuickActivate();

	void en_Configure(int nInitialPage = -1);
	void en_LaunchGeneralSettingsConfig();
	void en_LaunchCopyOptionsConfig();
	void en_LaunchTextColorAndFontsConfig();
	void en_LaunchNotesFileSettingsConfig();
	void en_LaunchBibleDatabaseConfig();
	void en_LaunchDictDatabaseConfig();
	void en_LaunchLocaleSettingsConfig();
	void en_LaunchTTSOptionsConfig();

	void en_NewCanOpener(QAction *pAction = nullptr);

	void en_changeActiveCanOpener(CKJVCanOpener *pNewActiveCanOpener, CKJVCanOpener *pOldActiveCanOpener);

#ifdef USING_QT_SPEECH
	void en_speechPause();
	void en_speechStop();
	void setSpeechActionEnables();
#endif

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;

// UI Private:
private:
	bool m_bDoingUpdate;
	// ----
	QAction *m_pActionPassageBrowserEditMenu;		// Edit Menu from Passage Browser when active
	QAction *m_pActionPassageReferenceEditMenu;		// Edit Menu from Passage Reference Editor (in browser) when active
	QAction *m_pActionSearchResultsEditMenu;		// Edit Menu from Search Results when active
	QAction *m_pActionSearchPhraseEditMenu;			// Edit Menu from Search Phrase when active
	QAction *m_pActionDictionaryEditMenu;			// Edit Menu from Dictionary when active
	QAction *m_pActionDictWordEditMenu;				// Edit Menu from Dictionary Word Editor when active
	// ----
	QMenu *m_pViewMenu;						// View Menu, used for insertion reference for edit menu
	QActionGroup *m_pActionGroupViewMode;		// Group for View Mode (Search Results vs highlighters vs user notes vs references, etc)
	QActionGroup *m_pActionGroupDisplayMode;	// Group for Verse Display Mode (heading vs. richtext)
	QActionGroup *m_pActionGroupTreeMode;		// Group for Tree Mode (List, Tree Books, Tree Chapters)
	QAction *m_pActionShowMissingLeafs;			// Toggle action for tree modes to show missing leafs
	QAction *m_pActionExpandAll;	// View menu Expand All
	QAction *m_pActionCollapseAll;	// View menu Collapse All
	QAction *m_pActionViewDetails;	// View Details
#ifdef USE_GEMATRIA
	QAction *m_pActionViewGematria;	// View Gematria
#endif
#ifdef USE_GEOMAP
	QAction *m_pActionViewGeoMap;	// View GeoMap
#endif
	// ----
	QAction *m_pActionBookBackward;	// Navigate Book Backward
	QAction *m_pActionBookForward;	// Navigate Book Forward
	QAction *m_pActionChapterBackward;	// Navigate Chapter Backward
	QAction *m_pActionChapterForward;	// Navigate Chapter Forward
	QAction *m_pActionNavBackward;	// Browser Navigate Backward
	QAction *m_pActionNavForward;	// Browser Navigate Forward
	QAction *m_pActionNavHome;		// Browser Navigate to History Home
	QAction *m_pActionNavClear;		// Clear Navigation History
	QAction *m_pActionJump;			// Jump to passage via Passage Navigator
	QAction *m_pActionRefresh;		// Refresh Scripture Browser
	// ----
	QAction *m_pActionBibleDatabasesList;	// Action for Loaded Bible Databases list from which the user can open New KJVCanOpeners
	QPointer<QActionGroup> m_pActionGroupBibleDatabasesList;	// Actual Bible Databases List items for the New Search Window List
	QAction *m_pActionSearchWindowList;		// Action for Window list of KJVCanOpeners
	QPointer<QActionGroup> m_pActionGroupSearchWindowList;		// Actual Window List items for Search Window List
	// ----
	QAction *m_pActionAbout;		// About Application
	QList<QAction *> m_lstpQuickActivate;	// Quick activation (Ctrl-1 through Ctrl-8 to activate upto first 8 search phrases, Ctrl-9 to activate Search Results, and Ctrl-0 to activate the browser)
	// ----
	QToolBar *m_pSpeechToolbar;		// Text-To-Speech Playback actions toolbar -- these will be NULL if Text-To-Speech isn't supported
	QAction *m_pActionSpeechPlay;
	QAction *m_pActionSpeechPause;
	QAction *m_pActionSpeechStop;
	QAction *m_pActionSpeakSelection;

	bool m_bBrowserActive;
	bool m_bSearchResultsActive;
	bool m_bPhraseEditorActive;
	bool m_bDictionaryActive;

	bool m_bCanClose;				// Set to false when displaying a window-modal dialog to keep application from trying to close us
	bool m_bIsClosing;				// True when window has issued an isClosing signal and set a deleteLater(), used as a guard for our event handler

#ifdef USE_GEOMAP
	QPointer<CGeoMap> m_pGeoMap;
#endif

	CSearchSpecWidget *m_pSearchSpecWidget;
	QSplitter *m_pSplitter;
	QSplitter *m_pSplitterDictionary;
	CSearchResults *m_pSearchResultWidget;
	CBrowserWidget *m_pBrowserWidget;
	CDictionaryWidget *m_pDictionaryWidget;
	CNoteEditDlg *m_pUserNoteEditorDlg;
	CCrossRefEditDlg *m_pCrossRefsEditorDlg;
	CHighlighterButtons *m_pHighlighterButtons;
	QAction *m_pActionUserNoteEditor;
	QAction *m_pActionCrossRefsEditor;
	CTipEdit *m_pTipEdit[TETE_COUNT] = {};
	bool m_bTipEditIsPinned[TETE_COUNT] = {};
	Ui::CKJVCanOpener ui;
};

#endif // KJV_CAN_OPENER_H
