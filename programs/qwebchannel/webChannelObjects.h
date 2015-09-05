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
#include "headlessSearchResults.h"

#include <QObject>
#include <QPointer>

// ============================================================================

// Forward declarations:
class CWebChannelServer;

// ============================================================================

//
// CWebChannelObjects
//
class CWebChannelObjects : public QObject
{
	Q_OBJECT

public:
	CWebChannelObjects(QObject *pParent = NULL);
	virtual ~CWebChannelObjects();

public slots:
	void selectBible(const QString &strUUID);

	void setSearchPhrases(const QString &strPhrases, const QString &strSearchWithin, int nSearchScope);		// Phrases, separated by semicolon, to search for.  SearchWithin = comma-separated searchWithinModel keys.  SearchScope = SEARCH_SCOPE_MODE_ENUM value
	void autoCorrect(const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos);			// Returns HTML Auto-Correction string for passed phrase and triggers autoCompleter list
	void calcUpdatedPhrase(const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos);		// Runs Phrase Parser and determines current subphrase.  Replaces that subphrase with passed strAutoCompleter value

	void gotoIndex(uint32_t ndxRel, int nMoveMode, const QString &strParam);			// Passage to navigate Scripture Browser to relative to nMoveMode. strParam is misc parameter sent back to javascript via scriptureBrowserRender()
	void gotoChapter(int nChp, const QString &strParam);	// Passage to navigate Scripture Browser to by chapter index

	void sendBroadcast(const QString &strMessage);			// Transmit broadcast message to connected client (shutdown alert, etc)

signals:
	void bibleSelected(bool bSuccess, const QString &strJsonBkChpStruct);				// Generated after selectBible() call to indicate success/fail and to provide the book/chapter layout for navigation (empty if failure)
	void searchWithinModelChanged(const QString &strJsonSearchWithinTree, int nScope);	// Generated after selectBible() call to fill in the searchWithin Tree View

	void searchResultsChanged(const QString &strHtmlSearchResults, const QString &strHtmlSummary, const QString &strlstOccurrences);			// Triggered by en_searchResultsReady() when we have data to send to channel
	void setAutoCorrectText(const QString &strElementID, const QString &strAC);			// Triggered after call to autoCorrect() will return the HTML of the Auto Correct text
	void setAutoCompleter(const QString &strElementID, const QString &strWordList);		// Triggered after call to autoCorrect() will return a list of words separated by ";"
	void updatePhrase(const QString &strElementID, const QString &strNewPhrase);		// Triggered after call to calcUpdatedPhrase() to return new phrase with subphrase substituted

	void scriptureBrowserRender(int nChp, uint32_t ndxRel, const QString &strHtmlScripture, const QString &strParam);		// Triggered by scripture browser navigation to display rendered text

	void broadcast(const QString &strMessage);				// Display popup message -- used to send shutdown announcements, etc.

protected:

protected slots:
	void en_searchResultsReady();							// Called by verseListModel in CHeadlessSearchResults when results have been updated


private:
	QPointer<CHeadlessSearchResults> m_pSearchResults;		// Search Results that we are controlling
	CSearchResultsData m_searchResultsData;					// Data (phrases and criteria) that we are using
	TSharedParsedPhrasesList m_lstParsedPhrases;			// Phrase parsers
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
