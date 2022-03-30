/****************************************************************************
**
** Copyright (C) 2015-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef WEBCHANNEL_SEARCH_RESULTS_H
#define WEBCHANNEL_SEARCH_RESULTS_H

#include "dbstruct.h"
#include "UserNotesDatabase.h"
#include "VerseListModel.h"
#include "PassageReferenceResolver.h"
#include "DelayedExecutionTimer.h"

#include <QPointer>
#include <QString>
#include <QTextDocument>
#include <QVector>
#include <QMap>
#include <QThread>

// Forward declarations:
class CPhraseNavigator;
class CWebChannelObjects;
class CWebChannelThreadController;

// ============================================================================

//
// CWebChannelSearchResults
//
class CWebChannelSearchResults : public QObject
{
	Q_OBJECT

	// Creatable only by CWebChannelThreadController
	friend class CWebChannelThreadController;
	CWebChannelSearchResults()					// Don't allow parenting as the parent must be in the thread the object is running in
		:	QObject(),
			m_nNextResultIndex(0),
			m_bSearchInProgress(false),
			m_bIsIdle(false),
			m_nLastSearchScope(-1)
	{ }
public:
	virtual ~CWebChannelSearchResults();

public slots:
	void initialize(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase);
	void setSearchPhrases(const QString &strPhrases, const QString &strSearchWithin, int nSearchScope);		// Phrases, separated by semicolon, to search for.  SearchWithin = comma-separated searchWithinModel keys.  SearchScope = SEARCH_SCOPE_MODE_ENUM value
	void getMoreSearchResults();							// Request next page of search results
	void autoCorrect(const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos);			// Returns HTML Auto-Correction string for passed phrase and triggers autoCompleter list
	void calcUpdatedPhrase(const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos);		// Runs Phrase Parser and determines current subphrase.  Replaces that subphrase with passed strAutoCompleter value

	void getSearchResultDetails(uint32_t ndxLogical);	// Requests data for ToolTip (i.e. search results detail) for the specified logical index

	void resolvePassageReference(const QString &strPassageReference);

	void gotoIndex(uint32_t ndxRel, int nMoveMode, const QString &strParam);		// Passage to navigate Scripture Browser to relative to nMoveMode. strParam is misc parameter sent back to javascript via scriptureBrowserRender()
	void gotoChapter(unsigned int nChp, const QString &strParam);	// Passage to navigate Scripture Browser to by chapter index

signals:
	void bibleSelected(bool bSuccess, const QString &strUUID, const QString &strJsonBkChpStruct);		// Generated after selectBible() call to indicate success/fail and to provide the book/chapter layout for navigation (empty if failure)
	void searchWithinModelChanged(const QString &strJsonSearchWithinTree, int nScope);	// Generated after selectBible() call to fill in the searchWithin Tree View

	void searchResultsChanged(const QString &strHtmlSearchResults, const QString &strHtmlSummary, const QString &strlstOccurrences);			// Triggered by en_searchResultsReady() when we have data to send to channel
	void searchResultsAppend(const QString &strHtmlSearchResults, bool bLast);			// Triggered if searchResultsChanged() batch is too large -- used to append remaining text
	void setAutoCorrectText(const QString &strElementID, const QString &strAC);			// Triggered after call to autoCorrect() will return the HTML of the Auto Correct text
	void setAutoCompleter(const QString &strElementID, const QString &strWordList);		// Triggered after call to autoCorrect() will return a list of words separated by ";"
	void updatePhrase(const QString &strElementID, const QString &strNewPhrase);		// Triggered after call to calcUpdatedPhrase() to return new phrase with subphrase substituted

	void searchResultsDetails(uint32_t ndxLogical, const QString &strDetails);		// Triggered after call to getSearchResultDetails() to return search details i.e. tool tip for specified index

	void resolvedPassageReference(uint32_t ndxRel, uint32_t nWordCount);		// Triggered after call to resolvePassageReference with the TPhraseTag equivalent of the passage

	void scriptureBrowserRender(unsigned int nChp, uint32_t ndxRel, const QString &strHtmlScripture, const QString &strParam);		// Triggered by scripture browser navigation to display rendered text
	void setBibleAudioURLs(const QString &strURLListJson);								// Triggered by scripture browser navigation to set a list of valid URLs for Bible Audio

	void idleStateChanged(bool bIsIdle);					// Triggered when connection either goes idle or wakes up
	void killWebChannel();									// Kill the WebChannel connection as the client is dead (hasn't responded in the dead-time limit)

private slots:
	void en_idleDetected();
	void en_deadDetected();
	void en_retriggerGotoIndex(const QString &strData);

private:
	void internal_setSearchPhrases(const QString &strPhrases, const QString &strSearchWithin, int nSearchScope);

	CPhraseNavigator &phraseNavigator() const
	{
		Q_ASSERT(!m_pPhraseNavigator.isNull());
		return *m_pPhraseNavigator.data();
	}

	bool setIdle(bool bIsIdle);								// Returns True if Idle State Changed
	bool wakeUp();											// Returns True if we were previously asleep (idle)
	bool isIdle() const { return m_bIsIdle; }

private slots:
	void en_searchResultsReady();

private:
	// QObject derived:
	CBibleDatabasePtr m_pBibleDatabase;
	QPointer<CVerseListModel> m_pVerseListModel;
	QPointer<CPhraseNavigator> m_pPhraseNavigator;
	QTextDocument m_scriptureText;
	QPointer<CPassageReferenceResolver> m_pRefResolver;
	QPointer<DelayedExecutionTimer> m_pIdleTimer;
	QPointer<DelayedExecutionTimer> m_pDeadTimer;
	QPointer<DelayedExecutionTimer> m_pRetriggerGetMoreSearchResults;
	QPointer<DelayedExecutionTimer> m_pRetriggerGetSearchResultDetails;
	QPointer<DelayedExecutionTimer> m_pRetriggerGotoIndex;

	// Non-QObject:
	int m_nNextResultIndex;									// Index of next output index when generating output (used for pagination)
	bool m_bSearchInProgress;								// Set to true while verseListModel is calculating results so that getMoreSearchResults() can be blocked during search
	bool m_bIsIdle;											// Set to true when this client becomes idle
	// ----
	CSearchResultsData m_searchResultsData;					// Data (phrases and criteria) that we are using
	TSharedParsedPhrasesList m_lstParsedPhrases;			// Phrase parsers
	// ----
	QString m_strLastPhrases;								// Last set of Search Phrases used to redo search if client wakes up from being idle
	QString m_strLastSearchWithin;							// Last Search Within Setting used to redo search if client wakes up from being idle
	int m_nLastSearchScope;									// Last Search Scope value used to redo search if client wakes up from being idle
};

typedef QMap<CWebChannelObjects *, CWebChannelSearchResults *> TWebChannelSearchResultsMap;
typedef QMap<CWebChannelSearchResults *, int> TSearchResultsToThreadMap;

// ============================================================================

//
// CWebChannelThread
//
class CWebChannelThread : public QThread
{
	Q_OBJECT

public:
	CWebChannelThread(QObject *pParent = nullptr);

	void attachWebChannelSearchResults(CWebChannelSearchResults *pSearchResults);
};

// ============================================================================

//
// CWebChannelThreadController
//
class CWebChannelThreadController : public QObject
{
	Q_OBJECT

	CWebChannelThreadController();			// Creatable by global singleton only

public:
	virtual ~CWebChannelThreadController();
	static CWebChannelThreadController *instance();

	void destroyWebChannelSearchResults(CWebChannelObjects *pChannel);

	int threadCount() const;
	int threadWebChannelCount(int nThreadIndex) const;

public:
	// These functions are called by the CWebChannelObjects slots to process the slot on a particular thread
	bool selectBible(CWebChannelObjects *pChannel, const QString &strUUID);

	void setSearchPhrases(CWebChannelObjects *pChannel, const QString &strPhrases, const QString &strSearchWithin, int nSearchScope);		// Phrases, separated by semicolon, to search for.  SearchWithin = comma-separated searchWithinModel keys.  SearchScope = SEARCH_SCOPE_MODE_ENUM value
	void getMoreSearchResults(CWebChannelObjects *pChannel);								// Request next page of search results
	void autoCorrect(CWebChannelObjects *pChannel, const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos);			// Returns HTML Auto-Correction string for passed phrase and triggers autoCompleter list
	void calcUpdatedPhrase(CWebChannelObjects *pChannel, const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos);		// Runs Phrase Parser and determines current subphrase.  Replaces that subphrase with passed strAutoCompleter value

	void getSearchResultDetails(CWebChannelObjects *pChannel, uint32_t ndxLogical);		// Requests data for ToolTip (i.e. search results detail) for the specified logical index

	void resolvePassageReference(CWebChannelObjects *pChannel, const QString &strPassageReference);

	void gotoIndex(CWebChannelObjects *pChannel, uint32_t ndxRel, int nMoveMode, const QString &strParam);			// Passage to navigate Scripture Browser to relative to nMoveMode. strParam is misc parameter sent back to javascript via scriptureBrowserRender()
	void gotoChapter(CWebChannelObjects *pChannel, unsigned int nChp, const QString &strParam);	// Passage to navigate Scripture Browser to by chapter index

private:
	CWebChannelSearchResults *createWebChannelSearchResults(CWebChannelObjects *pChannel,  CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase);

private slots:
	void en_destroyedWebChannelSearchResults(QObject *pObject);

private:
	TWebChannelSearchResultsMap m_mapSearchResults;
	TSearchResultsToThreadMap m_mapSearchResultsToThread;		// Mapping of Search Result to thread index (used for counting on SearchResult object destruction)
	QVector<CWebChannelThread *> m_lstThreads;
	QVector<int> m_lstNumWebChannels;
};

// ============================================================================

#endif	// WEBCHANNEL_SEARCH_RESULTS_H
