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
#include <QCloseEvent>
#include <QString>
#include <QSplitter>

#include <assert.h>

// ============================================================================

namespace Ui {
class CKJVCanOpener;
}

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(CBibleDatabasePtr pBibleDatabase, const QString &strUserDatabase = QString(), QWidget *parent = 0);
	~CKJVCanOpener();

	void initialize();

	bool isBrowserActive() const { return m_bBrowserActive; }
	bool isSearchResultsActive() const { return m_bSearchResultsActive; }
	bool isPhraseEditorActive() const { return m_bPhraseEditorActive; }

protected:
	void savePersistentSettings();
	void restorePersistentSettings();
	virtual void closeEvent(QCloseEvent * event);
	bool haveUserDatabase() const { return !m_strUserDatabase.isEmpty(); }

signals:
	void changedSearchResults();
	void canShowDetails(bool bHaveDetails);

public slots:
	bool openKJVSearchFile(const QString &strFilePathName);
	bool saveKJVSearchFile(const QString &strFilePathName) const;

	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setShowMissingLeafs(bool bShowMissing);

protected slots:
	void on_NewSearch();
	void on_OpenSearch();
	void on_SaveSearch();

	void on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
	void on_copySearchPhraseSummary();
	void on_changedSearchSpec(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases);

	void on_addPassageBrowserEditMenu(bool bAdd);
	void on_addSearchResultsEditMenu(bool bAdd);
	void on_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor = NULL);
	void on_activatedBrowser();
	void on_activatedSearchResults();
	void on_activatedPhraseEditor(const CPhraseLineEdit *pEditor);

	void on_viewVerseHeading();
	void on_viewVerseRichText();

	void on_viewAsList();
	void on_viewAsTreeBooks();
	void on_viewAsTreeChapters();
	void on_viewShowMissingsLeafs();

	void on_gotoIndex(const TPhraseTag &tag);
	void on_browserHistoryChanged();
	void on_clearBrowserHistory();

	void on_SearchResultActivated(const QModelIndex &index);		// Enter or double-click activated

	void on_PassageNavigatorTriggered();

	void on_viewDetails();
	void setDetailsEnable();

	void on_HelpManual();
	void on_HelpAbout();

	void on_QuickActivate();

	void on_setFontScriptureBrowser();
	void on_setFontSearchResults();

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	QString m_strUserDatabase;

// UI Private:
private:
	bool m_bDoingUpdate;
	QAction *m_pActionPassageBrowserEditMenu;		// Edit Menu from Passage Browser when active
	QAction *m_pActionSearchResultsEditMenu;		// Edit Menu from Search Results when active
	QAction *m_pActionSearchPhraseEditMenu;			// Edit Menu from Search Phrase when active
	QMenu *m_pViewMenu;						// View Menu, used for insertion reference for edit menu
	QAction *m_pActionShowVerseHeading;		// Toggle action to show verse heading only
	QAction *m_pActionShowVerseRichText;	// Toggle action to show verse richtext
	QAction *m_pActionShowAsList;			// Toggle action to show in list mode
	QAction *m_pActionShowAsTreeBooks;		// Toggle action to show in tree books mode
	QAction *m_pActionShowAsTreeChapters;	// Toggle action to show in tree chapters mode
	QAction *m_pActionShowMissingLeafs;		// Toggle action for tree modes to show missing leafs
	QAction *m_pActionExpandAll;	// View menu Expand All
	QAction *m_pActionCollapseAll;	// View menu Collapse All
	QAction *m_pActionViewDetails;	// View Details
	QAction *m_pActionBookBackward;	// Navigate Book Backward
	QAction *m_pActionBookForward;	// Navigate Book Forward
	QAction *m_pActionChapterBackward;	// Navigate Chapter Backward
	QAction *m_pActionChapterForward;	// Navigate Chapter Forward
	QAction *m_pActionNavBackward;	// Browser Navigate Backward
	QAction *m_pActionNavForward;	// Browser Navigate Forward
	QAction *m_pActionNavHome;		// Browser Navigate to History Home
	QAction *m_pActionNavClear;		// Clear Navigation History
	QAction *m_pActionJump;			// Jump to passage via Passage Navigator
	QAction *m_pActionAbout;		// About Application
	QList<QAction *> m_lstpQuickActivate;	// Quick activation (Ctrl-1 through Ctrl-8 to activate upto first 8 search phrases, Ctrl-9 to activate Search Results, and Ctrl-0 to activate the browser)

	bool m_bPhraseEditorActive;
	bool m_bSearchResultsActive;
	bool m_bBrowserActive;

	CKJVSearchSpec *m_pSearchSpecWidget;
	QSplitter *m_pSplitter;
	CKJVSearchResult *m_pSearchResultWidget;
	CKJVBrowser *m_pBrowserWidget;
	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
