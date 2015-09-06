/****************************************************************************
**
** Copyright (C) 2015 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef HEADLESS_SEARCH_RESULTS_H
#define HEADLESS_SEARCH_RESULTS_H

#include "dbstruct.h"
#include "UserNotesDatabase.h"
#include "VerseListModel.h"
#include "PassageReferenceWidget.h"

#include <QPointer>
#include <QTextDocument>

// Forward declarations:
class CPhraseNavigator;

//
// CHeadlessSearchResults
//
class CHeadlessSearchResults : public QObject
{
	Q_OBJECT

public:
	CHeadlessSearchResults(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QObject *pParent = 0);
	virtual ~CHeadlessSearchResults();

	inline const CVerseListModel &vlmodel() const { return m_verseListModel; }
	CPhraseNavigator &phraseNavigator() const
	{
		assert(!m_pPhraseNavigator.isNull());
		return *m_pPhraseNavigator.data();
	}

	TPhraseTag resolvePassageReference(const QString &strPassageReference) const;

public slots:
	void setParsedPhrases(const CSearchResultsData &searchResultsData);

signals:
	// Pass-through:
	void searchResultsReady();

private:
	CVerseListModel m_verseListModel;
	QPointer<CPhraseNavigator> m_pPhraseNavigator;
	QTextDocument m_scriptureText;
	CPassageReferenceResolver m_refResolver;
};

#endif	// HEADLESS_SEARCH_RESULTS_H
