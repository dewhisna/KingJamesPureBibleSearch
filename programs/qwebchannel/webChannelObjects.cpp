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
#include "webChannelServer.h"
#include "websockettransport.h"

#include "UserNotesDatabase.h"
#include "PhraseEdit.h"
#include "Highlighter.h"

#include <QStringList>
#include <QTextDocument>
#include <QTextEdit>
#include <QWebSocket>

#define DEBUG_WEBCHANNEL_SEARCH 0
#define DEBUG_WEBCHANNEL_AUTOCORRECT 0

// ============================================================================

// --------------
// Search Limits:
// --------------
#define MAX_SEARCH_PHRASES 5

// ============================================================================

//
// CWebChannelObjects
//

CWebChannelObjects::CWebChannelObjects(QObject *pParent)
	:	QObject(pParent)
{

}

CWebChannelObjects::~CWebChannelObjects()
{

}

void CWebChannelObjects::selectBible(const QString &strUUID)
{
	if (!m_pSearchResults.isNull()) delete m_pSearchResults;

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strUUID);
	if (!pBibleDatabase.isNull()) {
		m_pSearchResults = new CHeadlessSearchResults(pBibleDatabase, g_pUserNotesDatabase, this);
		connect(m_pSearchResults.data(), SIGNAL(searchResultsReady()), this, SLOT(en_searchResultsReady()));

		m_searchResultsData.m_SearchCriteria.setSearchWithin(pBibleDatabase);		// Initially search within entire Bible
		CSearchWithinModel swim(pBibleDatabase, m_searchResultsData.m_SearchCriteria);
		emit searchWithinModelChanged(swim.toWebChannelJson());
	}

	emit bibleSelected(!m_pSearchResults.isNull());
}

void CWebChannelObjects::setSearchPhrases(const QString &strPhrases)
{
#if DEBUG_WEBCHANNEL_SEARCH
	qDebug("Received: %s", strPhrases.toUtf8().data());
#endif

	if (m_pSearchResults.isNull()) return;

	QStringList lstPhrases = strPhrases.split(";", QString::SkipEmptyParts);
	if (lstPhrases.size() > MAX_SEARCH_PHRASES) {
		lstPhrases.erase(lstPhrases.begin() + MAX_SEARCH_PHRASES, lstPhrases.end());
	}
	if (lstPhrases.isEmpty()) {
		m_searchResultsData.m_lstParsedPhrases.clear();
		m_lstParsedPhrases.clear();
	} else {
		int ndxUsed = 0;
		for (int ndx = 0; ndx < lstPhrases.size(); ++ndx) {
			CPhraseEntry aPhraseEntry(lstPhrases.at(ndx));
			if (aPhraseEntry.isDisabled()) continue;
			if (m_lstParsedPhrases.size() <= ndxUsed) {
				m_lstParsedPhrases.append(QSharedPointer<CParsedPhrase>(new CParsedPhrase(m_pSearchResults->vlmodel().bibleDatabase())));
				m_searchResultsData.m_lstParsedPhrases.append(m_lstParsedPhrases.last().data());
			}
			assert(ndxUsed < m_searchResultsData.m_lstParsedPhrases.size());
			m_lstParsedPhrases[ndxUsed]->setFromPhraseEntry(aPhraseEntry, true);		// Set each phrase and search it
			++ndxUsed;
		}
		for (int ndx = m_lstParsedPhrases.size(); ndx > ndxUsed; --ndx) {
			m_lstParsedPhrases.removeLast();
			m_searchResultsData.m_lstParsedPhrases.removeLast();
		}
	}
	m_pSearchResults->setParsedPhrases(m_searchResultsData);		// Start processing search -- will block if not multi-threaded, else will exit, and either case searchResultsReady() fires
}

void CWebChannelObjects::autoCorrect(const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos)
{
#if DEBUG_WEBCHANNEL_AUTOCORRECT
	qDebug("ReceivedAC: %s : \"%s\" : Cursor=%d", strElementID.toUtf8().data(), strPhrase.toUtf8().data(), nCursorPos);
#endif

	if (m_pSearchResults.isNull()) return;

	CParsedPhrase thePhrase(m_pSearchResults->vlmodel().bibleDatabase());

	QTextEdit edit(strPhrase);
	CPhraseCursor cursor(edit.textCursor());
	if (nCursorPos < 0) nCursorPos = 0;
	// Sometimes html doc sends cursor position outside the bounds of the doc, this is a safe-guard:
	cursor.movePosition(QTextCursor::End);
	if (nCursorPos < cursor.position()) {
		cursor.setPosition(nCursorPos);
	}
	thePhrase.ParsePhrase(cursor, true);

	QString strParsedPhrase;

	for (int nSubPhrase = 0; nSubPhrase < thePhrase.subPhraseCount(); ++nSubPhrase) {
		if (nSubPhrase) strParsedPhrase += " | ";
		const CSubPhrase *pSubPhrase = thePhrase.subPhrase(nSubPhrase);
		int nPhraseSize = pSubPhrase->phraseSize();
		for (int nWord = 0; nWord < nPhraseSize; ++nWord) {
			if (nWord) strParsedPhrase += " ";
			if ((pSubPhrase->GetMatchLevel() <= nWord) &&
				(pSubPhrase->GetCursorMatchLevel() <= nWord) &&
				((nWord != pSubPhrase->GetCursorWordPos()) ||
				 ((!pSubPhrase->GetCursorWord().isEmpty()) && (nWord == pSubPhrase->GetCursorWordPos()))
				)
				) {
				strParsedPhrase += "<span style=\"text-decoration-line: underline line-through; text-decoration-style: wavy; text-decoration-color: red;\">";
				strParsedPhrase += pSubPhrase->phraseWords().at(nWord);
				strParsedPhrase += "</span>";
			} else {
				strParsedPhrase += pSubPhrase->phraseWords().at(nWord);
			}
		}
	}

#if DEBUG_WEBCHANNEL_AUTOCORRECT
	qDebug("AC: \"%s\"", strParsedPhrase.toUtf8().data());
#endif

	emit setAutoCorrectText(strElementID, strParsedPhrase);

	bool bNeedUpdate = false;

	if ((strLastPhrase == strPhrase) &&
		(nLastCursorPos == nCursorPos)) return;			// If the text and cursor didn't change, no need to update
	if ((nLastCursorPos < 0) ||
		(strLastPhrase != strPhrase)) {
		bNeedUpdate = true;								// TextChanged or LastCursorPos==-1 => Always update
		nLastCursorPos = 0;
	}

	if (!bNeedUpdate) {
		CParsedPhrase thePhraseLast(m_pSearchResults->vlmodel().bibleDatabase());
		CPhraseCursor cursorLast(edit.textCursor());
		cursorLast.movePosition(QTextCursor::End);
		if (nLastCursorPos <= cursorLast.position()) {
			cursorLast.setPosition(nLastCursorPos);
			thePhraseLast.ParsePhrase(cursorLast, false);		// Break phrase into subphrases and calculate cursor word, but don't find word matches as that's redundant
			if ((thePhraseLast.currentSubPhrase() != thePhrase.currentSubPhrase()) ||
				(thePhraseLast.GetCursorWordPos() != thePhrase.GetCursorWordPos())) {
				bNeedUpdate = true;
			}
		} else {
			// If last cursor position was beyond the length of the
			//		phrase text, an update is automatically needed:
			bNeedUpdate = true;
		}
	}
	// Avoid unnecessary updates by not sending new completer list if the
	//		cursor word hasn't changed:
	if (!bNeedUpdate) return;

	QStringList lstNextWords;
	QString strBasePhrase;
	int nCurrentSubPhrase = thePhrase.currentSubPhrase();
	if (nCurrentSubPhrase == -1) return;
	const CSubPhrase *pCurrentSubPhrase = thePhrase.subPhrase(nCurrentSubPhrase);
	for (int ndx = 0; ndx < pCurrentSubPhrase->GetCursorWordPos(); ++ndx) {
		strBasePhrase += pCurrentSubPhrase->phraseWords().at(ndx) + " ";
	}
	QString strCursorWord = pCurrentSubPhrase->GetCursorWord();
	int nPreRegExp = strCursorWord.indexOf(QRegExp("[\\[\\]\\*\\?]"));
	if (nPreRegExp != -1) strCursorWord = strCursorWord.left(nPreRegExp);
	lstNextWords.reserve(thePhrase.nextWordsList().size());
	for (int ndx = 0; ndx < thePhrase.nextWordsList().size(); ++ndx) {
		if ((strCursorWord.isEmpty() && (pCurrentSubPhrase->GetCursorWordPos() > 0)) ||
			(!strCursorWord.isEmpty() && thePhrase.nextWordsList().at(ndx).decomposedWord().startsWith(strCursorWord, Qt::CaseInsensitive))) {
			lstNextWords.append(strBasePhrase + thePhrase.nextWordsList().at(ndx).decomposedWord());		// TODO: Anyway to make jquery-ui autocompleter learn about decomposed/composed word differences?
		}
	}
	emit setAutoCompleter(strElementID, lstNextWords.join(QChar(';')));
}

void CWebChannelObjects::calcUpdatedPhrase(const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos)
{
	if (m_pSearchResults.isNull()) return;

	CParsedPhrase thePhrase(m_pSearchResults->vlmodel().bibleDatabase());

	QTextEdit edit(strPhrase);
	CPhraseCursor cursor(edit.textCursor());
	cursor.setPosition(nCursorPos);
	thePhrase.ParsePhrase(cursor, false);
	int nCurrentSubPhrase = thePhrase.currentSubPhrase();

	QString strNewPhrase;

	for (int nSubPhrase = 0; nSubPhrase < thePhrase.subPhraseCount(); ++nSubPhrase) {
		if (nSubPhrase) strNewPhrase += " | ";
		if (nSubPhrase == nCurrentSubPhrase) {
			strNewPhrase += strAutoCompleter;
		} else {
			strNewPhrase += thePhrase.subPhrase(nSubPhrase)->phrase();
		}
	}

	emit updatePhrase(strElementID, strNewPhrase);
}

void CWebChannelObjects::en_searchResultsReady()
{
	assert(!m_pSearchResults.isNull());

	CVerseTextRichifierTags richifierTags;
	richifierTags.setFromPersistentSettings(*CPersistentSettings::instance(), true);

	int nVerses = m_pSearchResults->vlmodel().rowCount();
#if DEBUG_WEBCHANNEL_SEARCH
	qDebug("Num Verses Matching = %d", nVerses);
#endif
	QString strResults;
	for (int ndx = 0; ndx < nVerses; ++ndx) {
		QModelIndex ndxModel = m_pSearchResults->vlmodel().index(ndx);

		const CVerseListItem &item(m_pSearchResults->vlmodel().data(ndxModel, CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		if (item.verseIndex().isNull()) continue;
		CSearchResultHighlighter srHighlighter(item.phraseTags());
		CRelIndex ndxVerse = item.getIndex();
		ndxVerse.setWord(0);
		QString strVerse;
		strVerse += QString("<a href=\"javascript:gotoIndex(%1);\">").arg(ndxVerse.index());
		strVerse += m_pSearchResults->vlmodel().bibleDatabase()->PassageReferenceText(ndxVerse, true);
		strVerse += "</a>";
		strVerse += " ";
		strVerse += item.getVerseRichText(richifierTags, &srHighlighter);
		strVerse += "<br /><hr />";
		strResults += strVerse;
	}

#if DEBUG_WEBCHANNEL_SEARCH
	qDebug("Sending Results");
#endif

	QString strOccurrences;
	for (int ndx = 0; ndx < m_lstParsedPhrases.size(); ++ndx) {
		const CParsedPhrase &parsedPhrase = *m_lstParsedPhrases.at(ndx).data();
		if (!strOccurrences.isEmpty()) strOccurrences += ";";
		strOccurrences += QString("%1/%2/%3")
								.arg(!parsedPhrase.isDisabled() ? (parsedPhrase.isExcluded() ? -parsedPhrase.GetContributingNumberOfMatches() : parsedPhrase.GetContributingNumberOfMatches()) : 0)
								.arg(parsedPhrase.GetNumberOfMatchesWithin())
								.arg(parsedPhrase.GetNumberOfMatches());
	}

	CSearchResultsSummary srs(m_pSearchResults->vlmodel());

	emit searchResultsChanged(strResults, srs.summaryDisplayText(m_pSearchResults->vlmodel().bibleDatabase(), false, true), strOccurrences);

	// Free-up memory for other clients:
	m_lstParsedPhrases.clear();
	m_searchResultsData.m_lstParsedPhrases.clear();
	// TODO : Figure out how to clear out the VerseListModel without causing
	//		this function to get run again and send empty results (keeping in
	//		mind this can be multithreaded).
}

void CWebChannelObjects::gotoIndex(uint32_t ndxRel)
{
	if (m_pSearchResults.isNull()) return;

	// Build a subset list of search results that are only in this chapter (which can't be
	//		any larger than the number of results in this chapter) and use that for doing
	//		the highlighting so that the VerseRichifier doesn't have to search the whole
	//		set, as doing so is slow on large searches:
	TPhraseTagList lstChapterCurrent = m_pSearchResults->phraseNavigator().currentChapterDisplayPhraseTagList(ndxRel);
	TPhraseTagList lstSearchResultsSubset;

	CSearchResultHighlighter srHighlighterFull(&m_pSearchResults->vlmodel(), false);
	CHighlighterPhraseTagFwdItr itr = srHighlighterFull.getForwardIterator();
	while (!itr.isEnd()) {
		TPhraseTag tagNext = itr.nextTag();
		if (lstChapterCurrent.intersects(m_pSearchResults->vlmodel().bibleDatabase().data(), tagNext))
			lstSearchResultsSubset.append(tagNext);
	}

	CSearchResultHighlighter srHighlighter(lstSearchResultsSubset, false);
	QString strText = m_pSearchResults->phraseNavigator().setDocumentToChapter(CRelIndex(ndxRel),
							CPhraseNavigator::TextRenderOptionFlags(defaultDocumentToChapterFlags |
							CPhraseNavigator::TRO_InnerHTML |
							CPhraseNavigator::TRO_NoWordAnchors |
							CPhraseNavigator::TRO_SuppressPrePostChapters),
							&srHighlighter);
	emit scriptureBrowserRender(ndxRel, strText);
	m_pSearchResults->phraseNavigator().clearDocument();			// Free-up memory for other clients
}

void CWebChannelObjects::sendBroadcast(const QString &strMessage)
{
	if (!strMessage.isEmpty()) emit broadcast(strMessage);
}

// ============================================================================

//
// CWebChannelAdminObjects
//

CWebChannelAdminObjects::CWebChannelAdminObjects(CWebChannelServer *pWebServerParent)
	:	QObject(pWebServerParent),
		m_strKey("76476F14-F3A9-42AC-9443-4A7445154EC7"),
		m_pWebServer(pWebServerParent)
{

}

CWebChannelAdminObjects::~CWebChannelAdminObjects()
{

}

void CWebChannelAdminObjects::sendBroadcast(const QString &strKey, const QString &strMessage)
{
	if (strKey == m_strKey) {
		emit broadcast(strMessage);
	}
}

void CWebChannelAdminObjects::getConnectionsList(const QString &strKey)
{
	if (strKey != m_strKey) return;

	const TWebChannelClientMap &mapChannels = m_pWebServer->channelMap();

	QString strClients;

	strClients += "<table><thead><tr>\n";
	strClients += "<th>Name</th><th>IP Address</th><th>Port</th>\n";
	strClients += "</tr></thead><tbody>\n";
	for (TWebChannelClientMap::const_iterator itrChannels = mapChannels.constBegin(); itrChannels != mapChannels.constEnd(); ++itrChannels) {
		strClients += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>\n")
								.arg(itrChannels.key()->socket()->peerName())
								.arg(itrChannels.key()->socket()->peerAddress().toString())
								.arg(itrChannels.key()->socket()->peerPort());
	}
	strClients += "</tbody></table>\n";
	strClients += QString("<br/>Connections: %1\n").arg(mapChannels.size());

	emit connectionsList(strClients);
}

void CWebChannelAdminObjects::shutdownServer(const QString &strKey, const QString &strConfirmation)
{
	if ((strKey != m_strKey) ||
		(strConfirmation != "9BF89B76-45B2-46EB-B95C-79D460F702BD")) return;

#ifdef IS_CONSOLE_APP
	QCoreApplication::exit(0);			// For console-build (i.e. daemon), exit.  Server closing will happen on exit
#else
	m_pWebServer->close();				// For GUI build, just close the webserver
#endif
}

// ============================================================================
