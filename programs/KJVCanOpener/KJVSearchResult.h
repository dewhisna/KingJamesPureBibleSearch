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

#ifndef KJVSEARCHRESULT_H
#define KJVSEARCHRESULT_H

#include "dbstruct.h"
#include "SearchPhraseListModel.h"
#include "VerseListModel.h"

#include <QWidget>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QFocusEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QString>
#include <QLabel>
#include <QTreeView>

#include <assert.h>

// ============================================================================


// ============================================================================

class CKJVCanOpener;		// Forward declaration

class CSearchResultsTreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit CSearchResultsTreeView(CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);
	virtual ~CSearchResultsTreeView();

	inline QMenu *getEditMenu() { return m_pEditMenu; }
	inline QMenu *getLocalEditMenu() { return m_pEditMenuLocal; }

	bool haveDetails() const;
	bool isActive() const;

	inline CVerseListModel *vlmodel() const {
		assert(model() != NULL);
		return static_cast<CVerseListModel *>(model());
	}

	inline CVerseListModel::VERSE_DISPLAY_MODE_ENUM displayMode() const { return vlmodel()->displayMode(); }
	inline CVerseListModel::VERSE_TREE_MODE_ENUM treeMode() const { return vlmodel()->treeMode(); }
	inline bool showMissingLeafs() const { return vlmodel()->showMissingLeafs(); }

protected slots:
	void on_copyVerseText();
	void on_copyRaw();
	void on_copyVeryRaw();
	void on_copyVerseHeadings();
	void on_copyReferenceDetails();
	void on_copyComplete();

	void on_listChanged();

public slots:
	void showPassageNavigator();
	void showDetails();
	void setFontSearchResults(const QFont& aFont);

signals:
	void activatedSearchResults();
	void gotoIndex(const TPhraseTag &tag);
	void canExpandAll(bool bEnable);
	void canCollapseAll(bool bEnable);
	void currentItemChanged();

protected:
	virtual void focusInEvent(QFocusEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);
	virtual void selectionChanged (const QItemSelection &selected, const QItemSelection &deselected);

	void copyRawCommon(bool bVeryRaw) const;
	void handle_selectionChanged();

	virtual void resizeEvent(QResizeEvent *event);

// Private Data:
private:
	CBibleDatabasePtr m_pBibleDatabase;

// Private UI:
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

class CKJVSearchResult : public QWidget
{
	Q_OBJECT
	
public:
	CKJVSearchResult(CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);
	~CKJVSearchResult();
	
	inline CVerseListModel::VERSE_DISPLAY_MODE_ENUM displayMode() const { return m_pSearchResultsTreeView->displayMode(); }
	inline CVerseListModel::VERSE_TREE_MODE_ENUM treeMode() const { return m_pSearchResultsTreeView->treeMode(); }
	inline bool showMissingLeafs() const { return m_pSearchResultsTreeView->showMissingLeafs(); }

	CRelIndex currentIndex() const;
	bool hasFocusSearchResult() const;
	bool canShowPassageNavigator() const;

	inline QMenu *getEditMenu() { return m_pSearchResultsTreeView->getEditMenu(); }
	inline QMenu *getLocalEditMenu() { return m_pSearchResultsTreeView->getLocalEditMenu(); }

	inline bool haveDetails() const { return m_pSearchResultsTreeView->haveDetails(); }
	inline bool isActive() const { return m_pSearchResultsTreeView->isActive(); }

	inline bool haveResults() const { return (model()->GetResultsCount() > 0); }
	inline const TPhraseTagList &searchResultsTags() const { return m_tagsSearchResults.phraseTags(); }

	QString searchResultsSummaryText() const;

public slots:
	bool setCurrentIndex(const CRelIndex &ndx, bool bFocusTreeView = true);
	void setFocusSearchResult();
	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setShowMissingLeafs(bool bShowMissing);
	const TPhraseTagList &setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases);		// Will build verseList and return the list of tags so they can be passed to a highlighter, etc
	void showPassageNavigator();
	void showDetails();

signals:			// Outgoing Pass-Through:
	void activated(const QModelIndex &);
	void gotoIndex(const TPhraseTag &);
	void changedSearchResults();
	void setDetailsEnable();

	void activatedSearchResults();
	void canExpandAll(bool bEnable);
	void canCollapseAll(bool bEnable);
	void currentItemChanged();

signals:			// Incoming Pass-Through:
	void expandAll();
	void collapseAll();
	void setFontSearchResults(const QFont &aFont);

protected:
	CVerseListModel *model() const {
		assert(m_pSearchResultsTreeView->model() != NULL);
		return static_cast<CVerseListModel *>(m_pSearchResultsTreeView->model());
	}

// Private Data:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		TPhraseTagList &phraseTagsNonConst() { return m_lstPhraseTags; }
		void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_tagsSearchResults;				// Highlight tags from search results
	int m_nLastSearchOccurrences;		// Last search summary of 'n' occurrences in 'x' verses in 'y' chapters in 'z' books
	int m_nLastSearchVerses;
	int m_nLastSearchChapters;
	int m_nLastSearchBooks;
	bool m_bLastCalcSuccess;
	int m_nLastSearchNumPhrases;
	CSearchCriteria m_LastSearchCriteria;

// UI Private:
private:
	CSearchResultsTreeView *m_pSearchResultsTreeView;
	QLabel *m_pSearchResultsCount;
};

#endif // KJVSEARCHRESULT_H
