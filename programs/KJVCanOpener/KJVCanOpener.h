#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include "dbstruct.h"
#include "KJVSearchPhraseEdit.h"

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

public slots:
	void copy();

signals:
	void activatedSearchResults();

protected:
	virtual void focusInEvent(QFocusEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);

private slots:
	void on_copyReferenceDetails();

private:
	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QAction *m_pActionCopy;			// Edit menu copy
	QAction *m_pActionSelectAll;	// Edit menu select all
	QAction *m_pActionClearSelection;	// Edit menu clear selection
	QAction *m_pActionCopyReferenceDetails;			// Reference ToolTip Copy
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

	void Initialize(CRelIndex nInitialIndex = CRelIndex(1,1,0,0));		// Default initial location is Genesis 1

protected:
	virtual void closeEvent(QCloseEvent * event);
	bool haveUserDatabase() const { return !m_strUserDatabase.isEmpty(); }

protected slots:
	void on_addPassageBrowserEditMenu(bool bAdd);
	void on_addSearchResultsEditMenu(bool bAdd);
	void on_activatedBrowser();
	void on_activatedSearchResults();
	void on_activatedPhraseEditor();

	void on_viewVerseHeading();
	void on_viewVerseRichText();

	void on_indexChanged(const CRelIndex &index);

	void on_browserHistoryChanged();
	void on_clearBrowserHistory();
	void on_phraseChanged(const CParsedPhrase &phrase);
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
	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
