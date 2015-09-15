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

#ifndef WEBCHANNEL_OBJECTS_H
#define WEBCHANNEL_OBJECTS_H

#include "dbstruct.h"
#include "VerseListModel.h"
#include "webChannelSearchResults.h"

#include <QObject>

// ============================================================================

// Forward declarations:
class CWebChannelClient;
class CWebChannelServer;
class CWebChannelThreadController;

// ============================================================================

//
// CWebChannelObjects
//
class CWebChannelObjects : public QObject
{
	Q_OBJECT

public:
	CWebChannelObjects(CWebChannelClient *pParent);
	virtual ~CWebChannelObjects();

	bool isAdmin() const { return m_bIsAdmin; }
	QString userAgent() const { return m_strUserAgent; }
	int threadIndex() const { return m_nThreadIndex; }
	QString bibleUUID() const { return m_strBibleUUID; }

private:
	friend class CWebChannelThreadController;
	void setThreadIndex(int nThreadIndex);

public slots:
	// ------------ Directly Handled Slots:

	void unlock(const QString &strKey);						// Admin unlock
	void setUserAgent(const QString &strUserAgent);			// Set userAgent data for logging
	void sendBroadcast(const QString &strMessage);			// Transmit broadcast message to connected client (shutdown alert, etc)


	// ------------ Threaded Slots:

	void selectBible(const QString &strUUID);				// Select particular Bible text (empty string is default)

	void setSearchPhrases(const QString &strPhrases, const QString &strSearchWithin, int nSearchScope);		// Phrases, separated by semicolon, to search for.  SearchWithin = comma-separated searchWithinModel keys.  SearchScope = SEARCH_SCOPE_MODE_ENUM value
	void getMoreSearchResults();							// Request next page of search results
	void autoCorrect(const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos);			// Returns HTML Auto-Correction string for passed phrase and triggers autoCompleter list
	void calcUpdatedPhrase(const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos);		// Runs Phrase Parser and determines current subphrase.  Replaces that subphrase with passed strAutoCompleter value

	void getSearchResultDetails(uint32_t ndxLogical);		// Requests data for ToolTip (i.e. search results detail) for the specified logical index

	void resolvePassageReference(const QString &strPassageReference);

	void gotoIndex(uint32_t ndxRel, int nMoveMode, const QString &strParam);			// Passage to navigate Scripture Browser to relative to nMoveMode. strParam is misc parameter sent back to javascript via scriptureBrowserRender()
	void gotoChapter(int nChp, const QString &strParam);	// Passage to navigate Scripture Browser to by chapter index


signals:
	// ------------ Directly Handled Signals:

	void broadcast(const QString &strMessage);				// Display popup message -- used to send shutdown announcements, etc.


	// ------------ Threaded Signals:

	void bibleList(const QString &strJsonBibleList);		// Reports available list of Bible UUIDs after call to selectBible() with default UUID of ("")
	void bibleSelected(bool bSuccess, const QString &strUUID, const QString &strJsonBkChpStruct);		// Generated after selectBible() call to indicate success/fail and to provide the book/chapter layout for navigation (empty if failure)
	void searchWithinModelChanged(const QString &strJsonSearchWithinTree, int nScope);	// Generated after selectBible() call to fill in the searchWithin Tree View

	void searchResultsChanged(const QString &strHtmlSearchResults, const QString &strHtmlSummary, const QString &strlstOccurrences);			// Triggered by en_searchResultsReady() when we have data to send to channel
	void searchResultsAppend(const QString &strHtmlSearchResults, bool bLast);			// Triggered if searchResultsChanged() batch is too large -- used to append remaining text
	void setAutoCorrectText(const QString &strElementID, const QString &strAC);			// Triggered after call to autoCorrect() will return the HTML of the Auto Correct text
	void setAutoCompleter(const QString &strElementID, const QString &strWordList);		// Triggered after call to autoCorrect() will return a list of words separated by ";"
	void updatePhrase(const QString &strElementID, const QString &strNewPhrase);		// Triggered after call to calcUpdatedPhrase() to return new phrase with subphrase substituted

	void searchResultsDetails(uint32_t ndxLogical, const QString &strDetails);			// Triggered after call to getSearchResultDetails() to return search details i.e. tool tip for specified index

	void resolvedPassageReference(uint32_t ndxRel, uint32_t nWordCount);				// Triggered after call to resolvePassageReference with the TPhraseTag equivalent of the passage

	void scriptureBrowserRender(int nChp, uint32_t ndxRel, const QString &strHtmlScripture, const QString &strParam);		// Triggered by scripture browser navigation to display rendered text

private:
	bool m_bIsAdmin;										// Set to true when we receive an admin unlock()
	QString m_strUserAgent;									// Set to userAgent string from client browser
	int m_nThreadIndex;										// Thread Index in CWebChannelThreadController
	QString m_strBibleUUID;									// UUID of currently selected Bible
	CWebChannelClient *m_pWebChannel;						// Parent channel
};

// ============================================================================

//
// CWebChannelAdminObjects
//
class CWebChannelAdminObjects : public QObject
{
	Q_OBJECT

public:
	CWebChannelAdminObjects(CWebChannelServer *pWebServerParent);
	virtual ~CWebChannelAdminObjects();

public slots:
	void sendBroadcast(const QString &strKey, const QString &strMessage);			// Transmit broadcast message to all connected clients (shutdown alert, etc)
	void sendMessage(const QString &strKey, const QString &strClientIP, const QString &strClientPort, const QString &strMessage);	// Transmit message to specific client
	void getConnectionsList(const QString &strKey);									// Request a list of connected clients
	void shutdownServer(const QString &strKey, const QString &strConfirmation);		// Shuts down this server (and exits daemon if a console-build)
	void disconnectClient(const QString &strKey, const QString &strClientIP, const QString &strClientPort);
	void stopListening(const QString &strKey);
	void startListening(const QString &strKey);
	void getIsListening(const QString &strKey);

signals:
	void broadcast(const QString &strMessage);					// Display popup message -- used to send shutdown announcements, etc.
	void connectionsList(const QString &strHtmlConnections);	// Returns list of active client connection data as formatted HTML table
	void sendMessageStatus(bool bSuccess, const QString &strClientIP, const QString &strClientPort, const QString &strMessage);
	void disconnectClientStatus(bool bSuccess, const QString &strClientIP, const QString &strClientPort);
	void serverListeningStatus(bool bIsListening);

private:
	const QString m_strKey;									// Access Key
	CWebChannelServer *m_pWebServer;						// Parent pointer (avoids up-cast)
};

// ============================================================================

#endif	// WEBCHANNEL_OBJECTS_H
