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

#ifndef KJVSEARCHSPEC_H
#define KJVSEARCHSPEC_H

#include "dbstruct.h"
#include "KJVSearchCriteria.h"
#include "KJVSearchPhraseEdit.h"
#include "SearchPhraseListModel.h"

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSettings>
#include <QEvent>

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

namespace Ui {
	class CKJVSearchSpec;
}

class CKJVSearchSpec : public QWidget
{
	Q_OBJECT
	
public:
	explicit CKJVSearchSpec(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase = true, QWidget *parent = 0);
	virtual ~CKJVSearchSpec();

	QString searchPhraseSummaryText() const;

signals:
	void changedSearchSpec(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases);

signals:				// Outgoing Pass-Through:
	void closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);
	void copySearchPhraseSummary();

public slots:
	void reset();
	void readKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup = QString());
	void writeKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup = QString()) const;

	void setFocusSearchPhrase(int nIndex);
	void setFocusSearchPhrase(const CKJVSearchPhraseEdit *pSearchPhrase);

public slots:			// Incoming Pass-Through:
	void enableCopySearchPhraseSummary(bool bEnable);
	void setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);

protected slots:
	void closeAllSearchPhrases();

	CKJVSearchPhraseEdit *addSearchPhrase();
	void ensureSearchPhraseVisible(int nIndex);
	void ensureSearchPhraseVisible(const CKJVSearchPhraseEdit *pSearchPhrase);
	void on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void on_changedSearchCriteria();
	void on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
public slots:
	void on_activatedPhraseEditor(const CPhraseLineEdit *pEditor);

protected:
	bool haveUserDatabase() const { return m_bHaveUserDatabase; }

	virtual bool eventFilter(QObject *obj, QEvent *ev);

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	bool m_bHaveUserDatabase;				// True if there is a user database defined (used for enabling add/remove icons)

// UI Private:
private:
	QVBoxLayout *m_pLayoutPhrases;
//	CSearchPhraseListModel m_modelSearchPhraseEditors;
	CSearchPhraseEditList m_lstSearchPhraseEditors;
	const CPhraseLineEdit *m_pLastEditorActive;		// Used to reactivate when the Search Spec Layout pane become active
	bool m_bDoneActivation;							// Set to True when we've triggered activation
	bool m_bCloseAllSearchPhrasesInProgress;		// Set to True when the closeAllSearchPhrases() has been triggered and is processing, so the we don't emit extra phraseChanged() notifications

	Ui::CKJVSearchSpec *ui;
};

#endif // KJVSEARCHSPEC_H
