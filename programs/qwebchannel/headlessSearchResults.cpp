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

#include "headlessSearchResults.h"
#include "PhraseEdit.h"

CHeadlessSearchResults::CHeadlessSearchResults(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QObject *pParent)
	:	QObject(pParent),
		m_verseListModel(pBibleDatabase, pUserNotesDatabase),
		m_refResolver(pBibleDatabase)
{
	m_verseListModel.setViewMode(CVerseListModel::VVME_SEARCH_RESULTS);
	m_verseListModel.setTreeMode(CVerseListModel::VTME_LIST);
	m_verseListModel.setDisplayMode(CVerseListModel::VDME_RICHTEXT);

	connect(&m_verseListModel, SIGNAL(searchResultsReady()), this, SIGNAL(searchResultsReady()));

	m_pPhraseNavigator = new CPhraseNavigator(pBibleDatabase, m_scriptureText, this);
}

CHeadlessSearchResults::~CHeadlessSearchResults()
{

}

void CHeadlessSearchResults::setParsedPhrases(const CSearchResultsData &searchResultsData)
{
	m_verseListModel.setParsedPhrases(searchResultsData);
}

TPhraseTag CHeadlessSearchResults::resolvePassageReference(const QString &strPassageReference) const
{
	return m_refResolver.resolve(strPassageReference);
}
