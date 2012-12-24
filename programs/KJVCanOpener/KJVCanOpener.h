#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include "dbstruct.h"
#include "KJVSearchPhraseEdit.h"
#include "SearchPhraseListModel.h"
#include "KJVSearchCriteria.h"

#include <QMainWindow>
#include <QModelIndex>
#include <QScrollArea>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QFocusEvent>
#include <QContextMenuEvent>
#include <QString>
#include <QListView>
#include <QVBoxLayout>

#include <assert.h>

// ============================================================================

class CSearchPhraseScrollArea : public QScrollArea
{
public:
	CSearchPhraseScrollArea( QWidget *parent=NULL)
		: QScrollArea(parent)
	{ }
	virtual ~CSearchPhraseScrollArea() { }

	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
};

// ============================================================================

class CSearchResultsListView : public QListView
{
	Q_OBJECT
public:
	explicit CSearchResultsListView(QWidget *parent = 0);
	virtual ~CSearchResultsListView();

	QMenu *getEditMenu() { return m_pEditMenu; }
	QMenu *getLocalEditMenu() { return m_pEditMenuLocal; }

public slots:
	void on_copyVerseText();
	void on_copyRaw();
	void on_copyVeryRaw();
	void on_copyVerseHeadings();
	void on_copyReferenceDetails();
	void on_copyComplete();
	void on_passageNavigator();

	void on_listChanged();

signals:
	void activatedSearchResults();
	void gotoIndex(const TPhraseTag &tag);

protected:
	virtual void focusInEvent(QFocusEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);

	void copyRawCommon(bool bVeryRaw) const;
	void handle_selectionChanged();

private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog and we don't want the highlight to disable
	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QMenu *m_pEditMenuLocal;		// Edit menu for local popup when user right-clicks -- like above but includes view toggles
	// ----
	QAction *m_pActionCopyVerseText;			// Edit menu copy text
	QAction *m_pActionCopyRaw;		// Edit menu copy raw phrase text
	QAction *m_pActionCopyVeryRaw;	// Edit menu copy very (no punctuation) raw phrase text
	// ----
	QAction *m_pActionCopyVerseHeadings;		// Edit menu copy headings
	QAction *m_pActionCopyReferenceDetails;		// Edit menu Reference ToolTip Copy
	QAction *m_pActionCopyComplete;				// Edit menu copy everything
	// ----
	QAction *m_pActionSelectAll;	// Edit menu select all
	QAction *m_pActionClearSelection;	// Edit menu clear selection
	// ----
	QAction *m_pActionNavigator;	// Launch Passage Navigator for Search Result
	// ----
	QAction *m_pStatusAction;		// Used to update the status bar without an enter/leave sequence
};


// ============================================================================

namespace Ui {
class CKJVCanOpener;
}

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(const QString &strUserDatabase = QString(), QWidget *parent = 0);
	~CKJVCanOpener();

	void Initialize(const TPhraseTag &nInitialIndex = TPhraseTag(CRelIndex(1,1,0,0)));		// Default initial location is Genesis 1

protected:
	virtual void closeEvent(QCloseEvent * event);
	bool haveUserDatabase() const { return !m_strUserDatabase.isEmpty(); }

signals:
	void changedSearchResults();

protected slots:
	CKJVSearchPhraseEdit *addSearchPhrase();
	void on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void on_changedSearchCriteria();
	void on_copySearchPhraseSummary();

	void on_addPassageBrowserEditMenu(bool bAdd);
	void on_addSearchResultsEditMenu(bool bAdd);
	void on_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor = NULL);
	void on_activatedBrowser();
	void on_activatedSearchResults();
	void on_activatedPhraseEditor(const CPhraseLineEdit *pEditor);

	void on_viewVerseHeading();
	void on_viewVerseRichText();

	void on_indexChanged(const TPhraseTag &tag);

	void on_browserHistoryChanged();
	void on_clearBrowserHistory();
	void on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
	void on_SearchResultActivated(const QModelIndex &index);		// Enter or double-click activated

	void on_PassageNavigatorTriggered();

	void on_HelpManual();
	void on_HelpAbout();

// Data Private:
private:
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

	bool m_bBrowserActive;
	bool m_bSearchResultsActive;
	bool m_bPhraseEditorActive;

	QVBoxLayout *m_pLayoutPhrases;
//	CSearchPhraseListModel m_modelSearchPhraseEditors;
	CSearchPhraseEditList m_lstSearchPhraseEditors;

	int m_nLastSearchOccurrences;		// Last search summary of 'n' occurrences in 'x' verses in 'y' chapters in 'z' books
	int m_nLastSearchVerses;
	int m_nLastSearchChapters;
	int m_nLastSearchBooks;
	bool m_bLastCalcSuccess;

	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
