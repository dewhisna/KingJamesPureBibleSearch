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

#include "webChannelSearchResults.h"
#include "webChannelObjects.h"
#include "webChannelBibleAudio.h"

#include "UserNotesDatabase.h"
#include "PhraseEdit.h"
#include "Highlighter.h"

#include "CSV.h"

#include <QStringList>
#include <QTextDocument>

#ifdef IS_CONSOLE_APP
#include <QDateTime>
#include <iostream>
#endif

#define DEBUG_WEBCHANNEL_SEARCH 0
#define DEBUG_WEBCHANNEL_AUTOCORRECT 0
#define DEBUG_WEBCHANNEL_THREAD_ANALYSIS 0
#define MINIMUM_THREAD_COUNT 2					// Minimum number of worker threads to create if ideal number of threads is less than this value or ideal number of threads can't be determined

// ============================================================================

// --------------
// Search Limits:
// --------------
#define MAX_SEARCH_PHRASES 5
#define RESULTS_FIRST_BATCH_SIZE 1000		// Number of results for first page batch to send to client
#define RESULTS_BATCH_SIZE 3000				// Number of results per additional batch to send to client per page
#define TIME_UNTIL_IDLE 600000				// 10 Minutes (1000 milliseconds/second * 60 seconds/minute * 10 minutes)
#define TIME_UNTIL_DEAD 3600000				// 60 Minutes (1000 milliseconds/second * 60 seconds/minute * 60 minutes)
#define RETRIGGER_TIME 250					// Number of milliseconds a function that woke us up gets retriggered after updating search results

// ============================================================================

//
// CWebChannelSearchResults
//

void CWebChannelSearchResults::initialize(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase)
{
	// Make sure input valid:
	assert(!pBibleDatabase.isNull());
	assert(!pUserNotesDatabase.isNull());

	// If calling init again, delete current data and start over:
	if (!m_pVerseListModel.isNull()) delete m_pVerseListModel;
	if (!m_pPhraseNavigator.isNull()) delete m_pPhraseNavigator;
	if (!m_pRefResolver.isNull()) delete m_pRefResolver;

	// Setup idle/dead timers.  These will be reused with future inits:
	if (m_pIdleTimer.isNull()) {
		m_pIdleTimer = new DelayedExecutionTimer(-1, TIME_UNTIL_IDLE, this);
		connect(m_pIdleTimer.data(), SIGNAL(triggered()), this, SLOT(en_idleDetected()));
	}
	if (m_pDeadTimer.isNull()) {
		m_pDeadTimer = new DelayedExecutionTimer(-1, TIME_UNTIL_DEAD, this);
		connect(m_pDeadTimer.data(), SIGNAL(triggered()), this, SLOT(en_deadDetected()));
	}
	if (m_pRetriggerGetMoreSearchResults.isNull()) {
		m_pRetriggerGetMoreSearchResults = new DelayedExecutionTimer(-1, RETRIGGER_TIME, this);
		connect(m_pRetriggerGetMoreSearchResults.data(), SIGNAL(triggered()), this, SLOT(getMoreSearchResults()));
	}
	if (m_pRetriggerGetSearchResultDetails.isNull()) {
		m_pRetriggerGetSearchResultDetails = new DelayedExecutionTimer(-1, RETRIGGER_TIME, this);
		connect(m_pRetriggerGetSearchResultDetails.data(), SIGNAL(triggered(uint32_t)), this, SLOT(getSearchResultDetails(uint32_t)));
	}
	if (m_pRetriggerGotoIndex.isNull()) {
		m_pRetriggerGotoIndex = new DelayedExecutionTimer(-1, RETRIGGER_TIME, this);
		connect(m_pRetriggerGotoIndex.data(), SIGNAL(triggered(const QString &)), this, SLOT(en_retriggerGotoIndex(const QString &)));
	}

	m_strLastPhrases.clear();
	m_strLastSearchWithin.clear();
	m_nLastSearchScope = static_cast<int>(CSearchCriteria::SSME_UNSCOPED);
	setIdle(false);

	m_pBibleDatabase = pBibleDatabase;

	m_nNextResultIndex = 0;
	m_bSearchInProgress = false;
	m_searchResultsData.m_lstParsedPhrases.clear();
	m_lstParsedPhrases.clear();
	m_scriptureText.clear();

	m_pVerseListModel = new CVerseListModel(pBibleDatabase, pUserNotesDatabase, this);

	m_pVerseListModel->setViewMode(CVerseListModel::VVME_SEARCH_RESULTS);
	m_pVerseListModel->setTreeMode(CVerseListModel::VTME_LIST);
	m_pVerseListModel->setDisplayMode(CVerseListModel::VDME_RICHTEXT);

	connect(m_pVerseListModel.data(), SIGNAL(searchResultsReady()), this, SLOT(en_searchResultsReady()));

	m_pPhraseNavigator = new CPhraseNavigator(pBibleDatabase, m_scriptureText, this);
	m_pRefResolver = new CPassageReferenceResolver(pBibleDatabase, this);

	m_searchResultsData.m_SearchCriteria.setSearchWithin(pBibleDatabase);		// Initially search within entire Bible, leave scope as previously selected
	CSearchWithinModel swim(pBibleDatabase, m_searchResultsData.m_SearchCriteria);
	emit searchWithinModelChanged(swim.toWebChannelJson(), static_cast<int>(m_searchResultsData.m_SearchCriteria.searchScopeMode()));
	emit bibleSelected(true, pBibleDatabase->compatibilityUUID(), pBibleDatabase->toJsonBkChpStruct());
}

CWebChannelSearchResults::~CWebChannelSearchResults()
{

}

// ----------------------------------------------------------------------------

bool CWebChannelSearchResults::setIdle(bool bIsIdle)
{
	assert(!m_pIdleTimer.isNull());
	assert(!m_pDeadTimer.isNull());
	bool bCurrentIdle = m_bIsIdle;
	m_bIsIdle = bIsIdle;
	if (bCurrentIdle != bIsIdle) emit idleStateChanged(bIsIdle);
	if (!bIsIdle) {
		if (!m_pIdleTimer.isNull()) m_pIdleTimer->trigger();
		if (!m_pDeadTimer.isNull()) m_pDeadTimer->trigger();
	}
	return (bCurrentIdle != bIsIdle);
}

bool CWebChannelSearchResults::wakeUp()
{
	if (isIdle()) {
		// No need to call setIdle() here as setSearchPhrases() will do it for us:
		internal_setSearchPhrases(m_strLastPhrases, m_strLastSearchWithin, m_nLastSearchScope);
		return true;
	}
	setIdle(false);
	return false;
}

void CWebChannelSearchResults::en_idleDetected()
{
	if (!m_bSearchInProgress && !isIdle()) {
		// Set our idle flag and clear results only if we aren't in the
		//		middle of a search.  We shouldn't be unless the search took more
		//		time than our timeout, but just in case (yes it means we'll go
		//		through extra timeout periods)...

		setIdle(true);

		// Clear m_pVerseListModel by initiating a search on nothing.  First, make
		//	sure our parsed phrases are clear:
		m_lstParsedPhrases.clear();
		m_searchResultsData.m_lstParsedPhrases.clear();
		m_pVerseListModel->setParsedPhrases(m_searchResultsData);
	} else {
		// Otherwise, re-arm the idle timer if we're not already idle, or else we may not fire again if search was in progress:
		if (!m_pIdleTimer.isNull() && !isIdle()) m_pIdleTimer->trigger();
	}
}

void CWebChannelSearchResults::en_deadDetected()
{
	// Last chance to see if we are no longer idle or are doing a search:
	if (m_bSearchInProgress || !isIdle()) {
		// This path is taken if for some reason dead time is set less than idle time:
		if (!m_pDeadTimer.isNull()) m_pDeadTimer->trigger();
		return;
	}

	emit killWebChannel();
}

// ----------------------------------------------------------------------------

void CWebChannelSearchResults::setSearchPhrases(const QString &strPhrases, const QString &strSearchWithin, int nSearchScope)
{
#if DEBUG_WEBCHANNEL_SEARCH
	qDebug("Received: %s", strPhrases.toUtf8().data());
#endif

	setIdle(false);		// Force set to false so internal_setSearchPhrases() will always do full load from external request, since we aren't regurgitating data
	internal_setSearchPhrases(strPhrases, strSearchWithin, nSearchScope);
}

void CWebChannelSearchResults::internal_setSearchPhrases(const QString &strPhrases, const QString &strSearchWithin, int nSearchScope)
{
	bool bSameSearch = ((m_strLastPhrases == strPhrases) && (m_strLastSearchWithin == strSearchWithin) && (m_nLastSearchScope == nSearchScope));
	m_strLastPhrases = strPhrases;
	m_strLastSearchWithin = strSearchWithin;
	m_nLastSearchScope = nSearchScope;
	bool bWasIdle = setIdle(false) && bSameSearch;

	m_searchResultsData.m_SearchCriteria.setSearchWithin(m_pBibleDatabase, strSearchWithin);
	m_searchResultsData.m_SearchCriteria.setSearchScopeMode(static_cast<CSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(nSearchScope));

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
				m_lstParsedPhrases.append(QSharedPointer<CParsedPhrase>(new CParsedPhrase(m_pBibleDatabase)));
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

	if (!bWasIdle) m_nNextResultIndex = 0;
	m_bSearchInProgress = true;

	m_pVerseListModel->setParsedPhrases(m_searchResultsData);		// Start processing search -- will block if using command-line multi-threaded search phrase mode, else will exit, and either case searchResultsReady() fires
}

void CWebChannelSearchResults::autoCorrect(const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos)
{
#if DEBUG_WEBCHANNEL_AUTOCORRECT
	qDebug("ReceivedAC: %s : \"%s\" : Cursor=%d", strElementID.toUtf8().data(), strPhrase.toUtf8().data(), nCursorPos);
#endif

	wakeUp();
	CParsedPhrase thePhrase(m_pBibleDatabase);

	QTextDocument doc(strPhrase);
	CPhraseCursor cursor(&doc);
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
		CParsedPhrase thePhraseLast(m_pBibleDatabase);
		CPhraseCursor cursorLast(&doc);
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
			(!strCursorWord.isEmpty() && thePhrase.nextWordsList().at(ndx).renderedWord().startsWith(strCursorWord, Qt::CaseInsensitive))) {
			lstNextWords.append(strBasePhrase + thePhrase.nextWordsList().at(ndx).renderedWord());		// TODO: Anyway to make jquery-ui autocompleter learn about decomposed/composed word differences?
		}
	}
	emit setAutoCompleter(strElementID, lstNextWords.join(QChar(';')));
}

void CWebChannelSearchResults::calcUpdatedPhrase(const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos)
{
	wakeUp();
	CParsedPhrase thePhrase(m_pBibleDatabase);

	QTextDocument doc(strPhrase);
	CPhraseCursor cursor(&doc);
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

void CWebChannelSearchResults::en_searchResultsReady()
{
	bool bPrevSearchInProgress = m_bSearchInProgress;
	m_bSearchInProgress = false;
	if (isIdle() && !bPrevSearchInProgress) return;					// A transition to idle clears verses by searching for "nothing".  If we are idle, we're done, unless a previous search result was finishing up...
	if (m_nNextResultIndex == 0) getMoreSearchResults();			// Generate first batch (emits both a searchResultsChanged and searchResultsAppend)

	// Free-up memory for other clients:
	m_lstParsedPhrases.clear();
	m_searchResultsData.m_lstParsedPhrases.clear();
}

void CWebChannelSearchResults::getMoreSearchResults()
{
	// If we fell asleep before this call, restart the search results since the
	//		previous result will have been tossed.  That will automatically reset
	//		us back to the previous NextResultIndex:
	wakeUp();				// This will automatically start a SearchInProgress, fall through to check it.  If it's asynchronous it will still be true...

	// If a search is in-progress (presummably the wakeUp() above), retrigger us if it isn't done yet:
	if (m_bSearchInProgress) {
		if (m_nNextResultIndex == 0) return;
		m_pRetriggerGetMoreSearchResults->trigger();
		return;
	}

	CVerseTextRichifierTags richifierTags;
	richifierTags.setFromPersistentSettings(*CPersistentSettings::instance(), true);

	int nVerses = m_pVerseListModel->rowCount();
#if DEBUG_WEBCHANNEL_SEARCH
	qDebug("Num Verses Matching = %d", nVerses);
#endif

	bool bFirstBatch = (m_nNextResultIndex == 0);
	int nCount = 0;

	QString strOccurrences;
	for (int ndx = 0; ndx < m_lstParsedPhrases.size(); ++ndx) {
		const CParsedPhrase &parsedPhrase = *m_lstParsedPhrases.at(ndx).data();
		if (!strOccurrences.isEmpty()) strOccurrences += ";";
		strOccurrences += QString("%1/%2/%3")
								.arg(!parsedPhrase.isDisabled() ? (parsedPhrase.isExcluded() ? -parsedPhrase.GetContributingNumberOfMatches() : parsedPhrase.GetContributingNumberOfMatches()) : 0)
								.arg(parsedPhrase.GetNumberOfMatchesWithin())
								.arg(parsedPhrase.GetNumberOfMatches());
	}

	CSearchResultsSummary srs(*m_pVerseListModel);

	QString strResults;
	for (int ndx = m_nNextResultIndex; ndx < nVerses; ++ndx) {
		QModelIndex ndxModel = m_pVerseListModel->index(ndx);

		const CVerseListItem &item(m_pVerseListModel->data(ndxModel, CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		if (item.verseIndex().isNull()) continue;
		CSearchResultHighlighter srHighlighter(item.phraseTags());
		CRelIndex ndxVerse = item.getIndex();
		ndxVerse.setWord((ndxVerse.isColophon() || ndxVerse.isSuperscription()) ? 1 : 0);		// Use 1st word anchor on colophons & superscriptions, but verse number only anchors otherwise since we aren't outputting word anchors
		QString strVerse;
		unsigned int nChp = CRefCountCalc(m_pBibleDatabase.data(),
										  CRefCountCalc::RTE_CHAPTER, ndxVerse).ofBible().first;
		if (ndxVerse.isColophon()) {
			// For colophons, find the last chapter of this book, which is where colophons
			//		are actually printed in the text, as the above calculation will be wrong
			//		and generally point to the last chapter of the previous book instead:
			const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxVerse);
			assert(pBook);
			if (pBook) {
				nChp = CRefCountCalc(m_pBibleDatabase.data(),
									 CRefCountCalc::RTE_CHAPTER, CRelIndex(ndxVerse.book(), pBook->m_nNumChp, 0, 0)).ofBible().first;
			}
		}
		strVerse += "<span class=\"verse\">";
		strVerse += "<span class=\"ref\">";
		strVerse += QString("<a href=\"javascript:gotoResult(%1,%2);\">")
									.arg(nChp)
									.arg(ndxVerse.index());
		strVerse += m_pBibleDatabase->PassageReferenceText(ndxVerse, true);
		strVerse += "</a>";
		strVerse += " ";
		strVerse += "</span>";	// Ref
		strVerse += item.getVerseRichText(richifierTags, false, &srHighlighter);
		strVerse += QString("<a href=\"javascript:viewDetails(%1);\"><img src=\"detail.png\" alt=\"Details\" height=\"16\" width=\"16\"></a>")
								.arg(m_pVerseListModel->logicalIndexForModelIndex(ndxModel).index());
		strVerse += "</span>";	// Verse
		strVerse += "<br /><hr />";
		strResults += strVerse;

		++nCount;
		if ((bFirstBatch && (nCount >= RESULTS_FIRST_BATCH_SIZE)) ||
			(!bFirstBatch && (nCount >= RESULTS_BATCH_SIZE))) break;
	}

	m_nNextResultIndex += nCount;

	if (bFirstBatch) {
		emit searchResultsChanged(strResults, srs.summaryDisplayText(m_pBibleDatabase, false, true), strOccurrences);
		emit searchResultsAppend(QString(), (m_nNextResultIndex >= nVerses));
	} else {
		emit searchResultsAppend(strResults, (m_nNextResultIndex >= nVerses));
	}
}

void CWebChannelSearchResults::getSearchResultDetails(uint32_t ndxLogical)
{
	// If we fell asleep before this call, restart the search results since the
	//		previous result will have been tossed:
	wakeUp();				// This will automatically start a SearchInProgress, fall through to check it.  If it's asynchronous it will still be true...

	// If a search is in-progress (presummably the wakeUp() above), retrigger us if it isn't done yet:
	if (m_bSearchInProgress) {
		if (m_nNextResultIndex == 0) return;
		m_pRetriggerGetSearchResultDetails->trigger(ndxLogical);
		return;
	}

	QModelIndex mdlIndex = m_pVerseListModel->modelIndexForLogicalIndex(ndxLogical);
	if (mdlIndex.isValid()) {
		QString strDetails = m_pVerseListModel->data(mdlIndex, CVerseListModel::TOOLTIP_ROLE).toString();
		emit searchResultsDetails(ndxLogical, strDetails);
	}
}

void CWebChannelSearchResults::resolvePassageReference(const QString &strPassageReference)
{
	wakeUp();
	TPhraseTag tagResolved = m_pRefResolver->resolve(strPassageReference);
	emit resolvedPassageReference(tagResolved.relIndex().index(), tagResolved.count());
}

void CWebChannelSearchResults::en_retriggerGotoIndex(const QString &strData)
{
	QString strBuffer = strData;
	QString strNdxRel;
	QString strMoveMode;
	QString strParam;

	CCSVStream csv(&strBuffer, QIODevice::ReadOnly);
	csv >> strNdxRel >> strMoveMode >> strParam;
	gotoIndex(strNdxRel.toUInt(), strMoveMode.toInt(), strParam);
}

void CWebChannelSearchResults::gotoIndex(uint32_t ndxRel, int nMoveMode, const QString &strParam)
{
	// If we fell asleep before this call, restart the search results since the
	//		previous result will have been tossed:
	wakeUp();				// This will automatically start a SearchInProgress, fall through to check it.  If it's asynchronous it will still be true...

	// If a search is in-progress (presummably the wakeUp() above), retrigger us if it isn't done yet:
	if (m_bSearchInProgress) {
		if (m_nNextResultIndex == 0) return;
		QString strData;
		CCSVStream csv(&strData, QIODevice::WriteOnly);
		csv << QString("%1").arg(ndxRel) << QString("%1").arg(nMoveMode) << strParam;
		m_pRetriggerGotoIndex->trigger(strData);
		return;
	}

	CRelIndex ndx(ndxRel);
	CRelIndex ndxDecolophonated(ndxRel);
	if (ndxDecolophonated.isColophon()) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxDecolophonated);
		if (pBook) {
			ndxDecolophonated.setChapter(pBook->m_nNumChp);
		}
	}
	ndx = m_pBibleDatabase->calcRelIndex(ndx, static_cast<CBibleDatabase::RELATIVE_INDEX_MOVE_ENUM>(nMoveMode));
	if (!ndx.isSet()) return;
	ndxDecolophonated = m_pBibleDatabase->calcRelIndex(ndxDecolophonated, static_cast<CBibleDatabase::RELATIVE_INDEX_MOVE_ENUM>(nMoveMode));
	if (!ndxDecolophonated.isSet()) return;
	if (!m_pBibleDatabase->completelyContains(TPhraseTag(ndxDecolophonated))) return;

	// Build a subset list of search results that are only in this chapter (which can't be
	//		any larger than the number of results in this chapter) and use that for doing
	//		the highlighting so that the VerseRichifier doesn't have to search the whole
	//		set, as doing so is slow on large searches:
	TPhraseTagList lstChapterCurrent = m_pPhraseNavigator->currentChapterDisplayPhraseTagList(ndx);
	TPhraseTagList lstSearchResultsSubset;

	CSearchResultHighlighter srHighlighterFull(m_pVerseListModel.data(), false);
	CHighlighterPhraseTagFwdItr itr = srHighlighterFull.getForwardIterator();
	while (!itr.isEnd()) {
		TPhraseTag tagNext = itr.nextTag();
		if (lstChapterCurrent.intersects(m_pBibleDatabase.data(), tagNext))
			lstSearchResultsSubset.append(tagNext);
	}

	CSearchResultHighlighter srHighlighter(lstSearchResultsSubset, false);
	QString strText = m_pPhraseNavigator->setDocumentToChapter(ndxDecolophonated,
							CPhraseNavigator::TextRenderOptionFlags(defaultDocumentToChapterFlags |
							CPhraseNavigator::TRO_InnerHTML |
							CPhraseNavigator::TRO_NoWordAnchors |
							CPhraseNavigator::TRO_SuppressPrePostChapters),
							&srHighlighter);

	strText += "<hr />" + m_pPhraseNavigator->getToolTip(TPhraseTag(CRelIndex(ndxDecolophonated.book(), ndxDecolophonated.chapter(), 0, 0)),
															CSelectionPhraseTagList(),
															CPhraseNavigator::TTE_COMPLETE,
															false);

	ndx.setWord((ndx.isColophon() || ndx.isSuperscription()) ? 1 : 0);				// Use 1st word anchor on colophons & superscriptions, but verse number only anchors otherwise since we aren't outputting word anchors

	emit scriptureBrowserRender(CRefCountCalc(m_pBibleDatabase.data(),
											  CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofBible().first,
								ndx.index(),
								strText,
								strParam);

	emit setBibleAudioURLs(CWebChannelBibleAudio::instance()->urlsForChapterAudio(m_pBibleDatabase, ndx));

	m_pPhraseNavigator->clearDocument();			// Free-up memory for other clients
}

void CWebChannelSearchResults::gotoChapter(unsigned int nChp, const QString &strParam)
{
	CRelIndex ndx = m_pBibleDatabase->calcRelIndex(0, 0, nChp, 0, 0);
	if (ndx.isSet()) {
		// Let gotoIndex() do wakeUp() operation
		gotoIndex(ndx.index(), static_cast<int>(CBibleDatabase::RIME_Absolute), strParam);
	} else {
		wakeUp();
	}
}

// ============================================================================

//
// CWebChannelThread
//

CWebChannelThread::CWebChannelThread(QObject *pParent)
	:	QThread(pParent)
{

}

void CWebChannelThread::attachWebChannelSearchResults(CWebChannelSearchResults *pSearchResults)
{
	assert(pSearchResults != NULL);
	pSearchResults->moveToThread(this);
	connect(this, SIGNAL(finished()), pSearchResults, SLOT(deleteLater()));		// When the thread ends, delete the object since it becomes detached at that point
	connect(pSearchResults, SIGNAL(destroyed(QObject*)), CWebChannelThreadController::instance(), SLOT(en_destroyedWebChannelSearchResults(QObject*)));
}

// ============================================================================

//
// CWebChannelThreadController
//

CWebChannelThreadController::CWebChannelThreadController()
{
	int nIdealThreads = QThread::idealThreadCount();
	if (nIdealThreads < MINIMUM_THREAD_COUNT) nIdealThreads = MINIMUM_THREAD_COUNT;		// If ideal can't be determined (-1) or is less than 'n', use default

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Thread Count : %2\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(nIdealThreads)
							.toUtf8().data();
	std::cout.flush();
#endif

	m_lstThreads.reserve(nIdealThreads);
	m_lstNumWebChannels.reserve(nIdealThreads);
	for (int ndx = 0; ndx < nIdealThreads; ++ndx) {
		CWebChannelThread *pThread = new CWebChannelThread(this);
		m_lstThreads.append(pThread);
		m_lstNumWebChannels.append(0);
		pThread->start();
	}
}

CWebChannelThreadController::~CWebChannelThreadController()
{
	m_mapSearchResultsToThread.clear();
	m_mapSearchResults.clear();
	for (int ndx = 0; ndx < m_lstThreads.size(); ++ndx) {
		if (!m_lstThreads.at(ndx)->isFinished()) {
			m_lstThreads.at(ndx)->quit();
			m_lstThreads.at(ndx)->wait();
		}
		delete m_lstThreads.at(ndx);
	}
	m_lstThreads.clear();
	m_lstNumWebChannels.clear();
}

CWebChannelThreadController *CWebChannelThreadController::instance()
{
	static CWebChannelThreadController theWebChannelThreadController;
	return &theWebChannelThreadController;
}

CWebChannelSearchResults *CWebChannelThreadController::createWebChannelSearchResults(CWebChannelObjects *pChannel,  CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults == NULL) {
		// Select next open thread:
		int ndxThreadToUse = -1;
		int nLowCount = -1;
		assert(m_lstNumWebChannels.size() == m_lstThreads.size());
		for (int ndx = 0; ndx < m_lstThreads.size(); ++ndx) {
			if (m_lstThreads.at(ndx) && m_lstThreads.at(ndx)->isRunning()) {
#if DEBUG_WEBCHANNEL_THREAD_ANALYSIS
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : Analyzing Threads : Thread %2, Load=%3\n")
								.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
								.arg(ndx)
								.arg(m_lstNumWebChannels.at(ndx))
								.toUtf8().data();
		std::cout.flush();
#endif
#endif
				if ((nLowCount == -1) || (m_lstNumWebChannels.at(ndx) < nLowCount)) {
					nLowCount = m_lstNumWebChannels.at(ndx);
					ndxThreadToUse = ndx;
				}
			}
		}
		if (ndxThreadToUse == -1) ndxThreadToUse = 0;		// Safeguard in case for some reason all threads are stopped, we'll just queue it on the first since we are guaranteed to have at least one thread (see constructor)

		pChannel->setThreadIndex(ndxThreadToUse);

		CWebChannelThread *pThread = m_lstThreads.at(ndxThreadToUse);

		pSearchResults = new CWebChannelSearchResults();		// No parent as the parent has to be in the target thread!!
		pThread->attachWebChannelSearchResults(pSearchResults);				// MoveToThread -- do this call prior to any connects, etc.

		m_mapSearchResults[pChannel] = pSearchResults;
		++m_lstNumWebChannels[ndxThreadToUse];
		m_mapSearchResultsToThread[pSearchResults] = ndxThreadToUse;

		connect(pSearchResults, SIGNAL(idleStateChanged(bool)), pChannel, SLOT(en_idleStateChanged(bool)));
		connect(pSearchResults, SIGNAL(killWebChannel()), pChannel, SLOT(en_killWebChannel()));

		connect(pSearchResults, SIGNAL(bibleSelected(bool, const QString &, const QString &)), pChannel, SIGNAL(bibleSelected(bool, const QString &, const QString &)));
		connect(pSearchResults, SIGNAL(searchWithinModelChanged(const QString &,int)), pChannel, SIGNAL(searchWithinModelChanged(const QString &, int)));

		connect(pSearchResults, SIGNAL(searchResultsChanged(const QString &, const QString &, const QString &)), pChannel, SIGNAL(searchResultsChanged(const QString &, const QString &, const QString &)));
		connect(pSearchResults, SIGNAL(searchResultsAppend(const QString &, bool)), pChannel, SIGNAL(searchResultsAppend(const QString &, bool)));
		connect(pSearchResults, SIGNAL(setAutoCorrectText(const QString &, const QString &)), pChannel, SIGNAL(setAutoCorrectText(const QString &, const QString &)));
		connect(pSearchResults, SIGNAL(setAutoCompleter(const QString &, const QString &)), pChannel, SIGNAL(setAutoCompleter(const QString &, const QString &)));
		connect(pSearchResults, SIGNAL(updatePhrase(const QString &, const QString &)), pChannel, SIGNAL(updatePhrase(const QString &, const QString &)));

		connect(pSearchResults, SIGNAL(searchResultsDetails(uint32_t, const QString &)), pChannel, SIGNAL(searchResultsDetails(uint32_t, const QString &)));

		connect(pSearchResults, SIGNAL(resolvedPassageReference(uint32_t, uint32_t)), pChannel, SIGNAL(resolvedPassageReference(uint32_t, uint32_t)));

		connect(pSearchResults, SIGNAL(scriptureBrowserRender(unsigned int, uint32_t, const QString &, const QString &)), pChannel, SIGNAL(scriptureBrowserRender(unsigned int, uint32_t, const QString &, const QString &)));
		connect(pSearchResults, SIGNAL(setBibleAudioURLs(const QString &)), pChannel, SIGNAL(setBibleAudioURLs(const QString &)));
	}

	// Initialize after connects in case pSearchResults wants to emit any signals during init:
	bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
												"initialize",
												Qt::QueuedConnection,
												Q_ARG(CBibleDatabasePtr, pBibleDatabase),
												Q_ARG(CUserNotesDatabasePtr, pUserNotesDatabase));
	assert(bSuccess);

	return pSearchResults;
}

void CWebChannelThreadController::en_destroyedWebChannelSearchResults(QObject *pObject)
{
	CWebChannelSearchResults *pSearchResults = static_cast<CWebChannelSearchResults *>(pObject);
	assert(pSearchResults != NULL);

	int nThread = m_mapSearchResultsToThread.value(pSearchResults, -1);
	assert(nThread != -1);
	if (nThread != -1) {
		--m_lstNumWebChannels[nThread];
		m_mapSearchResultsToThread.remove(pSearchResults);
	}

	CWebChannelObjects *pChannel = NULL;
	for (TWebChannelSearchResultsMap::const_iterator itrSR = m_mapSearchResults.constBegin(); itrSR != m_mapSearchResults.constEnd(); ++itrSR) {
		if (itrSR.value() == pSearchResults) {
			pChannel = itrSR.key();
		}
	}
	// Note: pChannel can be NULL here if object was deleted via a call to
	//			destroyWebChannelSearchResults() rather than a thread finish
	//			signal, so don't assert on (pChannel != NULL) here.

	if (pChannel) m_mapSearchResults.remove(pChannel);
}

void CWebChannelThreadController::destroyWebChannelSearchResults(CWebChannelObjects *pChannel)
{
	assert(pChannel != NULL);

	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	// Must remove the channel here as selectBible() will call this function to release
	//		the current object and then create a new one to map to the channel, so we
	//		should go ahead and deselect it here since the threads are asynchronous:
	if ((pChannel) && m_mapSearchResults.remove(pChannel)) pChannel->setThreadIndex(-1);

	if (pSearchResults == NULL) return;			// Allow a null case here as this function may be called just to ensure disconnection of channel's SearchResults

	// In case the other thread is in the process of returning pending calculations, disconnect
	//		all signals from CWebChannelSearchResults and CWebChannelObjects:
	disconnect(pSearchResults, SIGNAL(idleStateChanged(bool)), pChannel, SLOT(en_idleStateChanged(bool)));
	disconnect(pSearchResults, SIGNAL(killWebChannel()), pChannel, SLOT(en_killWebChannel()));
	disconnect(pSearchResults, SIGNAL(bibleSelected(bool, const QString &, const QString &)), pChannel, SIGNAL(bibleSelected(bool, const QString &, const QString &)));
	disconnect(pSearchResults, SIGNAL(searchWithinModelChanged(const QString &,int)), pChannel, SIGNAL(searchWithinModelChanged(const QString &, int)));
	disconnect(pSearchResults, SIGNAL(searchResultsChanged(const QString &, const QString &, const QString &)), pChannel, SIGNAL(searchResultsChanged(const QString &, const QString &, const QString &)));
	disconnect(pSearchResults, SIGNAL(searchResultsAppend(const QString &, bool)), pChannel, SIGNAL(searchResultsAppend(const QString &, bool)));
	disconnect(pSearchResults, SIGNAL(setAutoCorrectText(const QString &, const QString &)), pChannel, SIGNAL(setAutoCorrectText(const QString &, const QString &)));
	disconnect(pSearchResults, SIGNAL(setAutoCompleter(const QString &, const QString &)), pChannel, SIGNAL(setAutoCompleter(const QString &, const QString &)));
	disconnect(pSearchResults, SIGNAL(updatePhrase(const QString &, const QString &)), pChannel, SIGNAL(updatePhrase(const QString &, const QString &)));
	disconnect(pSearchResults, SIGNAL(searchResultsDetails(uint32_t, const QString &)), pChannel, SIGNAL(searchResultsDetails(uint32_t, const QString &)));
	disconnect(pSearchResults, SIGNAL(resolvedPassageReference(uint32_t, uint32_t)), pChannel, SIGNAL(resolvedPassageReference(uint32_t, uint32_t)));
	disconnect(pSearchResults, SIGNAL(scriptureBrowserRender(unsigned int, uint32_t, const QString &, const QString &)), pChannel, SIGNAL(scriptureBrowserRender(unsigned int, uint32_t, const QString &, const QString &)));
	disconnect(pSearchResults, SIGNAL(setBibleAudioURLs(const QString &)), pChannel, SIGNAL(setBibleAudioURLs(const QString &)));

	pSearchResults->deleteLater();
}

int CWebChannelThreadController::threadCount() const
{
	return m_lstThreads.size();
}

int CWebChannelThreadController::threadWebChannelCount(int nThreadIndex) const
{
	assert((nThreadIndex >= 0) && (nThreadIndex < m_lstNumWebChannels.size()));
	if ((nThreadIndex < 0) || (nThreadIndex >= m_lstNumWebChannels.size())) return 0;
	return m_lstNumWebChannels.at(nThreadIndex);
}

bool CWebChannelThreadController::selectBible(CWebChannelObjects *pChannel, const QString &strUUID)
{
	assert(pChannel != NULL);

	bool bSuccess = true;

	// Report list of available texts if we haven't yet selected the Bible on this client (i.e. first selection):
	if (!pChannel->hasSelectedBible()) {
		bSuccess = QMetaObject::invokeMethod(pChannel,
									"bibleList",
									Qt::DirectConnection,		// This should make sure the list fires before the default selection propagates
									Q_ARG(const QString &, TBibleDatabaseList::instance()->availableBibleDatabasesAsJson()));
		assert(bSuccess);
	}

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strUUID);
#ifndef ENABLE_ONLY_LOADED_BIBLE_DATABASES
	if ((pBibleDatabase.isNull()) && (TBibleDatabaseList::instance()->loadBibleDatabase(strUUID, false))) {
		pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strUUID);
		assert(!pBibleDatabase.isNull());
	}
#endif

	if (!pBibleDatabase.isNull()) {
		// Note: create calls init on searchResults which will emit bibleSelected signal
		bSuccess = (createWebChannelSearchResults(pChannel, pBibleDatabase, g_pUserNotesDatabase) != NULL);
	} else {
		bSuccess = QMetaObject::invokeMethod(pChannel,
												"bibleSelected",
												Qt::QueuedConnection,
												Q_ARG(bool, false),
												Q_ARG(const QString &, QString()),
												Q_ARG(const QString &, QString()));
		assert(bSuccess);
	}

	return bSuccess;
}

void CWebChannelThreadController::setSearchPhrases(CWebChannelObjects *pChannel, const QString &strPhrases, const QString &strSearchWithin, int nSearchScope)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"setSearchPhrases",
													Qt::QueuedConnection,
													Q_ARG(const QString &, strPhrases),
													Q_ARG(const QString &, strSearchWithin),
													Q_ARG(int, nSearchScope));
		assert(bSuccess);
	}
}

void CWebChannelThreadController::getMoreSearchResults(CWebChannelObjects *pChannel)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"getMoreSearchResults",
													Qt::QueuedConnection);
		assert(bSuccess);
	}
}

void CWebChannelThreadController::autoCorrect(CWebChannelObjects *pChannel, const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"autoCorrect",
													Qt::QueuedConnection,
													Q_ARG(const QString &, strElementID),
													Q_ARG(const QString &, strPhrase),
													Q_ARG(int, nCursorPos),
													Q_ARG(const QString &, strLastPhrase),
													Q_ARG(int,  nLastCursorPos));
		assert(bSuccess);
	}
}

void CWebChannelThreadController::calcUpdatedPhrase(CWebChannelObjects *pChannel, const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"calcUpdatedPhrase",
													Qt::QueuedConnection,
													Q_ARG(const QString &, strElementID),
													Q_ARG(const QString &, strPhrase),
													Q_ARG(const QString &, strAutoCompleter),
													Q_ARG(int, nCursorPos));
		assert(bSuccess);
	}
}

void CWebChannelThreadController::getSearchResultDetails(CWebChannelObjects *pChannel, uint32_t ndxLogical)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"getSearchResultDetails",
													Qt::QueuedConnection,
													Q_ARG(uint32_t, ndxLogical));
		assert(bSuccess);
	}
}

void CWebChannelThreadController::resolvePassageReference(CWebChannelObjects *pChannel, const QString &strPassageReference)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"resolvePassageReference",
													Qt::QueuedConnection,
													Q_ARG(const QString &, strPassageReference));
		assert(bSuccess);
	}
}

void CWebChannelThreadController::gotoIndex(CWebChannelObjects *pChannel, uint32_t ndxRel, int nMoveMode, const QString &strParam)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"gotoIndex",
													Qt::QueuedConnection,
													Q_ARG(uint32_t, ndxRel),
													Q_ARG(int, nMoveMode),
													Q_ARG(const QString &, strParam));
		assert(bSuccess);
	}
}

void CWebChannelThreadController::gotoChapter(CWebChannelObjects *pChannel, unsigned int nChp, const QString &strParam)
{
	assert(pChannel != NULL);
	CWebChannelSearchResults *pSearchResults = m_mapSearchResults.value(pChannel, NULL);
	if (pSearchResults) {
		bool bSuccess = QMetaObject::invokeMethod(pSearchResults,
													"gotoChapter",
													Qt::QueuedConnection,
													Q_ARG(unsigned int, nChp),
													Q_ARG(const QString &, strParam));
		assert(bSuccess);
	}
}

// ============================================================================
