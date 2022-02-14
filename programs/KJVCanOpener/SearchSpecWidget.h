/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SEARCH_SPEC_WIDGET_H
#define SEARCH_SPEC_WIDGET_H

#include "dbstruct.h"
#include "SearchCriteria.h"
#include "SearchPhraseEdit.h"
#include "SearchPhraseListModel.h"
#include "DelayedExecutionTimer.h"
#include "VerseListModel.h"

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSettings>
#include <QEvent>
#include <QFrame>
#include <QPushButton>

// ============================================================================

class CSearchPhraseScrollArea : public QScrollArea
{
public:
	CSearchPhraseScrollArea( QWidget *parent=nullptr)
		: QScrollArea(parent)
	{ }
	virtual ~CSearchPhraseScrollArea() { }

	virtual QSize minimumSizeHint() const override;
	virtual QSize sizeHint() const override;
};

// ============================================================================

#include "ui_SearchSpecWidget.h"

class CSearchSpecWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CSearchSpecWidget(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase = true, QWidget *parent = nullptr);
	virtual ~CSearchSpecWidget();

	QString searchPhraseSummaryText() const;
	QString searchWindowDescription() const;		// Return descriptive description for this window for the application search window list

	int searchPhraseCount() const { return m_lstSearchPhraseEditors.size(); }

	int searchResultsUpdateDelay() const { return m_dlySearchResultsUpdate.minimumDelay()/2; }			// 2-multiplier is to avoid a race condition between search phrase change delay and search results update delay

signals:
	void changedSearchSpec(const CSearchResultsData &searchResultsData);
	void copySearchPhraseSummary();

signals:				// Outgoing Pass-Through:
	void closingSearchPhrase(CSearchPhraseEdit *pSearchPhrase);
	void phraseChanged(CSearchPhraseEdit *pSearchPhrase);
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);
	void triggeredSearchWithinGotoIndex(const CRelIndex &relIndex);

public slots:
	void reset();
	void clearAllSearchPhrases();
	void readKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup = QString());
	void writeKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup = QString()) const;

	CSearchPhraseEdit *addSearchPhrase();

	CSearchPhraseEdit *setFocusSearchPhrase(int nIndex = 0);
	void setFocusSearchPhrase(const CSearchPhraseEdit *pSearchPhrase);

	void setSearchResultsUpdateDelay(int nDelay) { m_dlySearchResultsUpdate.setMinimumDelay(nDelay*2); }		// 2-multiplier is to avoid a race condition between search phrase change delay and search results update delay

public slots:			// Incoming Pass-Through:
	void enableCopySearchPhraseSummary(bool bEnable);
	void setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);
	void processAllPendingUpdateCompleter();

protected slots:
	void closeAllSearchPhrases();

	void ensureSearchPhraseVisible(int nIndex);
	void ensureSearchPhraseVisible(const CSearchPhraseEdit *pSearchPhrase);
	void en_phraseResizing(CSearchPhraseEdit *pSearchPhrase);
	void en_changingShowMatchingPhrases(CSearchPhraseEdit *pSearchPhrase);
	void resizeScrollAreaLayout();
	void en_closingSearchPhrase(CSearchPhraseEdit *pSearchPhrase);
	void en_changedSearchCriteria();
	void en_phraseChanged(CSearchPhraseEdit *pSearchPhrase = nullptr);
public slots:
	void en_searchResultsReady();
	void en_activatedPhraseEditor(const CPhraseLineEdit *pEditor);
protected slots:
	void en_changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings);
	void en_changedHideMatchingPhrasesLists(bool bHideMatchingPhrasesLists);

protected:
	bool haveUserDatabase() const { return m_bHaveUserDatabase; }

	virtual bool eventFilter(QObject *obj, QEvent *ev) override;

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	bool m_bHaveUserDatabase;				// True if there is a user database defined (used for enabling add/remove icons)

// UI Private:
private:
	QVBoxLayout *m_pLayoutPhrases;
//	CSearchPhraseListModel m_modelSearchPhraseEditors;
	CSearchPhraseEditList m_lstSearchPhraseEditors;
	QPushButton m_buttonAddSearchPhrase;
	QFrame m_frameAddCopySeparator;
	QPushButton m_buttonCopySummary;
	bool m_bDoingResizing;
	const CPhraseLineEdit *m_pLastEditorActive;		// Used to reactivate when the Search Spec Layout pane become active
	bool m_bDoneActivation;							// Set to True when we've triggered activation
	bool m_bCloseAllSearchPhrasesInProgress;		// Set to True when the closeAllSearchPhrases() has been triggered and is processing, so the we don't emit extra phraseChanged() notifications
	bool m_bReadingSearchFile;						// Set to True while we are reading a search file to keep updates to a minimum
	DelayedExecutionTimer m_dlySearchResultsUpdate;	// Staged delay for Search Results Update after Phrase Change notification

	Ui::CSearchSpecWidget ui;
};

#endif // SEARCH_SPEC_WIDGET_H
