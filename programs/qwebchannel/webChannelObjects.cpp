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

#include "webChannelObjects.h"

#include <QStringList>

#define DEBUG_WEBCHANNEL 1

CWebChannelObjects::CWebChannelObjects(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QObject *pParent)
	:	QObject(pParent)
{
	assert(!pBibleDatabase.isNull());
	assert(!pUserNotesDatabase.isNull());

	m_pSearchResults = new CHeadlessSearchResults(pBibleDatabase, pUserNotesDatabase, this);
	connect(m_pSearchResults.data(), SIGNAL(searchResultsReady()), this, SLOT(en_searchResultsReady()));

	m_searchResultsData.m_SearchCriteria.setSearchWithin(pBibleDatabase);		// Initially search within entire Bible
}

CWebChannelObjects::~CWebChannelObjects()
{

}

void CWebChannelObjects::setSearchPhrases(const QString &strPhrases)
{
#if DEBUG_WEBCHANNEL
	qDebug("Received: %s", strPhrases.toUtf8().data());
#endif

	QStringList lstPhrases = strPhrases.split(";", QString::SkipEmptyParts);
	if (lstPhrases.isEmpty()) {
		m_searchResultsData.m_lstParsedPhrases.clear();
		m_lstParsedPhrases.clear();
	} else {
		for (int ndx = 0; ndx < lstPhrases.size(); ++ndx) {
			if (m_lstParsedPhrases.size() <= ndx) {
				m_lstParsedPhrases.append(QSharedPointer<CParsedPhrase>(new CParsedPhrase(m_pSearchResults->vlmodel().bibleDatabase())));
				m_searchResultsData.m_lstParsedPhrases.append(m_lstParsedPhrases.last().data());
			}
			assert(ndx < m_searchResultsData.m_lstParsedPhrases.size());
			m_lstParsedPhrases[ndx]->setFromPhraseEntry(CPhraseEntry(lstPhrases.at(ndx)), true);		// Set each phrase and search it
		}
		for (int ndx = m_lstParsedPhrases.size(); ndx > lstPhrases.size(); --ndx) {
			m_lstParsedPhrases.removeLast();
			m_searchResultsData.m_lstParsedPhrases.removeLast();
		}
	}
	m_pSearchResults->setParsedPhrases(m_searchResultsData);		// Start processing search -- will block if not multi-threaded, else will exit, and either case searchResultsReady() fires
}

void CWebChannelObjects::en_searchResultsReady()
{
	CVerseTextRichifierTags richifierTags;
	richifierTags.setFromPersistentSettings(*CPersistentSettings::instance(), true);

	int nVerses = m_pSearchResults->vlmodel().rowCount();
#if DEBUG_WEBCHANNEL
	qDebug("Num Verses Matching = %d", nVerses);
#endif
	QString strResults;
	for (int ndx = 0; ndx < nVerses; ++ndx) {
		QModelIndex ndxModel = m_pSearchResults->vlmodel().index(ndx);

		const CVerseListItem &item(m_pSearchResults->vlmodel().data(ndxModel, CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		if (item.verseIndex().isNull()) continue;
		CRelIndex ndxVerse = item.getIndex();
		ndxVerse.setWord(0);
		QString strVerse;
		strVerse += "<li>";
		strVerse += m_pSearchResults->vlmodel().bibleDatabase()->PassageReferenceText(ndxVerse, true);
		strVerse += " ";
		strVerse += item.getVerseRichText(richifierTags, item.phraseTags());
		strVerse += "</li>";
		strResults += strVerse;
	}

#if DEBUG_WEBCHANNEL
	qDebug("Sending Results");
#endif
	emit searchResultsChanged(strResults);
}
