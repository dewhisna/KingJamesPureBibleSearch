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

#include "UserNotesDatabase.h"
#include "PhraseEdit.h"
#include "Highlighter.h"

#include <QStringList>
#include <QTextDocument>
#include <QTextEdit>

#define DEBUG_WEBCHANNEL_SEARCH 0
#define DEBUG_WEBCHANNEL_AUTOCORRECT 0

// ============================================================================

// --------------
// Search Limits:
// --------------
#define MAX_SEARCH_PHRASES 5

// ============================================================================

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

void CWebChannelObjects::autoCorrect(const QString &strElementID, const QString &strPhrase, int nCursorPos)
{
#if DEBUG_WEBCHANNEL_AUTOCORRECT
	qDebug("ReceivedAC: %s : \"%s\" : Cursor=%d", strElementID.toUtf8().data(), strPhrase.toUtf8().data(), nCursorPos);
#endif

	if (m_pSearchResults.isNull()) return;

	CParsedPhrase thePhrase(m_pSearchResults->vlmodel().bibleDatabase());

	QTextEdit edit(strPhrase);
	CPhraseCursor cursor(edit.textCursor());
	cursor.setPosition(nCursorPos);
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

	// TODO : Decide if we need to check against last cursor position to avoid unnecessary updates
	//			if that's even possible:
//	if ((thePhrase.GetCursorWordPos() != m_nCursorWord) || (bForceUpdate)) {
//		m_nCursorWord = thePhrase.GetCursorWordPos();

//		m_ParsedPhrase.nextWordsList();
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
	emit searchResultsChanged(strResults);
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
}
