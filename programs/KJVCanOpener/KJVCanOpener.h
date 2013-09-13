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

#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include "dbstruct.h"

#include "KJVSearchPhraseEdit.h"
#include "VerseListModel.h"
#include "KJVSearchSpec.h"
#include "KJVSearchResult.h"
#include "KJVBrowser.h"

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

#include <assert.h>

// ============================================================================

// Forward Declares:
class CKJVNoteEditDlg;
class CKJVCrossRefEditDlg;
class CHighlighterButtons;
class CTipEdit;

// ============================================================================

#include "ui_KJVCanOpener.h"

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);
	~CKJVCanOpener();

	void initialize();

	bool isBrowserActive() const { return m_bBrowserActive; }
	bool isSearchResultsActive() const { return m_bSearchResultsActive; }
	bool isPhraseEditorActive() const { return m_bPhraseEditorActive; }

	bool isBrowserFocusedOrActive() const;
	bool isSearchResultsFocusedOrActive() const;
	bool isPhraseEditorFocusedOrActive() const;

	CHighlighterButtons *highlighterButtons() const { return m_pHighlighterButtons; }
	QAction *actionUserNoteEditor() const { return m_pActionUserNoteEditor; }
	QAction *actionCrossRefsEditor() const { return m_pActionCrossRefsEditor; }

	bool canClose() const { return m_bCanClose; }
	QString searchWindowDescription() const;		// Return descriptive description for this window for the application search window list

	friend class CKJVCanOpenerCloseGuard;
	class CKJVCanOpenerCloseGuard {
	public:
		CKJVCanOpenerCloseGuard(CKJVCanOpener *pCanOpener)
			:	m_pCanOpener(pCanOpener)
		{
			assert(m_pCanOpener != NULL);
			m_bPreviousCanClose = m_pCanOpener->canClose();
			m_pCanOpener->setCanClose(false);
		}

		~CKJVCanOpenerCloseGuard()
		{
			assert(m_pCanOpener != NULL);
			if (m_pCanOpener != NULL) m_pCanOpener->setCanClose(m_bPreviousCanClose);
		}

	private:
		QPointer<CKJVCanOpener> m_pCanOpener;
		bool m_bPreviousCanClose;
	};

protected slots:
	void savePersistentSettings();
	void restorePersistentSettings();

	void setCanClose(bool bCanClose)
	{
		m_bCanClose = bCanClose;
		emit canCloseChanged(this, m_bCanClose);
	}

protected:
	virtual void closeEvent(QCloseEvent * event);
	virtual bool event(QEvent *pEvent);

	friend class CTipEdit;
	CTipEdit *tipEdit() const { return m_pTipEdit; }
	void setTipEdit(CTipEdit *pTipEdit) { m_pTipEdit = pTipEdit; }

	bool tipEditIsPinned() const { return m_bTipEditIsPinned; }
	void setTipEditIsPinned(bool bIsPinned) { m_bTipEditIsPinned = bIsPinned; }

signals:
	void changedSearchResults();
	void canShowDetails(bool bHaveDetails);
	void windowActivated(CKJVCanOpener *pCanOpener);
	void canCloseChanged(CKJVCanOpener *pCanOpener, bool bCanClose);

public slots:
	bool openKJVSearchFile(const QString &strFilePathName);
	bool saveKJVSearchFile(const QString &strFilePathName) const;

	void setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode);
	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setShowMissingLeafs(bool bShowMissing);

	void en_updateSearchWindowList();

protected slots:
	void en_NewSearch();
	void en_OpenSearch();
	void en_SaveSearch();

	void en_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void en_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
	void en_copySearchPhraseSummary();
	void en_changedSearchSpec(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases);

	void en_addPassageBrowserEditMenu(bool bAdd);
	void en_addSearchResultsEditMenu(bool bAdd);
	void en_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor = NULL);
	void en_activatedBrowser();
	void en_activatedSearchResults();
	void en_activatedPhraseEditor(const CPhraseLineEdit *pEditor);

	void en_viewModeChange(QAction *pAction);
	void en_displayModeChange(QAction *pAction);
	void en_treeModeChange(QAction *pAction);
	void en_viewShowMissingsLeafs();

	void en_gotoIndex(const TPhraseTag &tag);
	void en_browserHistoryChanged();
	void en_clearBrowserHistory();

	void en_SearchResultActivated(const QModelIndex &index);		// Enter or double-click activated

	void en_PassageNavigatorTriggered();

	void en_userNoteEditorTriggered();
	void en_crossRefsEditorTriggered();

	void en_viewDetails();
	void setDetailsEnable();

	void en_HelpManual();
	void en_HelpAbout();

	void en_QuickActivate();

	void en_Configure();

	void en_NewCanOpener();

	void setTextBrightness(bool bInvert, int nBrightness);
	void setAdjustDialogElementBrightness(bool bAdjust);

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;

// UI Private:
private:
	bool m_bDoingUpdate;
	// ----
	QAction *m_pActionPassageBrowserEditMenu;		// Edit Menu from Passage Browser when active
	QAction *m_pActionSearchResultsEditMenu;		// Edit Menu from Search Results when active
	QAction *m_pActionSearchPhraseEditMenu;			// Edit Menu from Search Phrase when active
	// ----
	QMenu *m_pViewMenu;						// View Menu, used for insertion reference for edit menu
	QActionGroup *m_pActionGroupViewMode;		// Group for View Mode (Search Results vs highlighters vs user notes vs references, etc)
	QActionGroup *m_pActionGroupDisplayMode;	// Group for Verse Display Mode (heading vs. richtext)
	QActionGroup *m_pActionGroupTreeMode;		// Group for Tree Mode (List, Tree Books, Tree Chapters)
	QAction *m_pActionShowMissingLeafs;			// Toggle action for tree modes to show missing leafs
	QAction *m_pActionExpandAll;	// View menu Expand All
	QAction *m_pActionCollapseAll;	// View menu Collapse All
	QAction *m_pActionViewDetails;	// View Details
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
	// ----
	QMenu *m_pWindowMenu;
	QAction *m_pActionSearchWindowList;		// Action for Window list of KJVCanOpeners
	QPointer<QActionGroup> m_pActionGroupSearchWindowLists;		// Actual Window List items for Search Window List
	// ----
	QAction *m_pActionAbout;		// About Application
	QList<QAction *> m_lstpQuickActivate;	// Quick activation (Ctrl-1 through Ctrl-8 to activate upto first 8 search phrases, Ctrl-9 to activate Search Results, and Ctrl-0 to activate the browser)

	bool m_bPhraseEditorActive;
	bool m_bSearchResultsActive;
	bool m_bBrowserActive;

	bool m_bCanClose;				// Set to false when displaying a window-modal dialog to keep application from trying to close us

	QString m_strAppStartupStyleSheet;			// Copy of the original StyleSheet from QApp, which will be the user's StyleSheet if they used the "-stylesheet" option
	CKJVSearchSpec *m_pSearchSpecWidget;
	QSplitter *m_pSplitter;
	CKJVSearchResult *m_pSearchResultWidget;
	CKJVBrowser *m_pBrowserWidget;
	CKJVNoteEditDlg *m_pUserNoteEditorDlg;
	CKJVCrossRefEditDlg *m_pCrossRefsEditorDlg;
	CHighlighterButtons *m_pHighlighterButtons;
	QAction *m_pActionUserNoteEditor;
	QAction *m_pActionCrossRefsEditor;
	CTipEdit *m_pTipEdit;
	bool m_bTipEditIsPinned;
	Ui::CKJVCanOpener ui;
};

#endif // KJVCANOPENER_H
