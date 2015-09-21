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
#include "webChannelSearchResults.h"

#include <QWebSocket>
#include <QVector>

// ============================================================================

//
// CWebChannelObjects
//

CWebChannelObjects::CWebChannelObjects(CWebChannelClient *pParent)
	:	QObject(pParent),
		m_bIsAdmin(false),
		m_nThreadIndex(-1),
		m_bIsIdle(false),
		m_pWebChannel(pParent)
{
	assert(m_pWebChannel != NULL);
}

CWebChannelObjects::~CWebChannelObjects()
{
	CWebChannelThreadController::instance()->destroyWebChannelSearchResults(this);
}

void CWebChannelObjects::setThreadIndex(int nThreadIndex)
{
	assert(m_pWebChannel != NULL);

	m_nThreadIndex = nThreadIndex;
	m_pWebChannel->setThreadIndex();
}

void CWebChannelObjects::en_idleStateChanged(bool bIsIdle)
{
	assert(m_pWebChannel != NULL);

	m_bIsIdle = bIsIdle;
	m_pWebChannel->setIdle();
}

void CWebChannelObjects::en_killWebChannel()
{
	assert(m_pWebChannel != NULL);
	m_pWebChannel->killWebChannel();
}

// ----------------------------------------------------------------------------

void CWebChannelObjects::unlock(const QString &strKey)
{
	if (strKey.compare("A609FDFD-BB3C-4BEB-AFDA-9A839F940346") == 0) m_bIsAdmin = true;
}

void CWebChannelObjects::setUserAgent(const QString &strUserAgent)
{
	assert(m_pWebChannel != NULL);

	m_strUserAgent = strUserAgent;
	m_pWebChannel->setUserAgent();
}

void CWebChannelObjects::sendBroadcast(const QString &strMessage)
{
	if (!strMessage.isEmpty()) emit broadcast(strMessage);
}

// ----------------------------------------------------------------------------

void CWebChannelObjects::selectBible(const QString &strUUID)
{
	assert(m_pWebChannel != NULL);

	if (CWebChannelThreadController::instance()->selectBible(this, strUUID)) {
		m_strBibleUUID = strUUID;
	} else {
		m_strBibleUUID.clear();
	}
	m_pWebChannel->setBibleUUID();
}

void CWebChannelObjects::setSearchPhrases(const QString &strPhrases, const QString &strSearchWithin, int nSearchScope)
{
	CWebChannelThreadController::instance()->setSearchPhrases(this, strPhrases, strSearchWithin, nSearchScope);
}

void CWebChannelObjects::getMoreSearchResults()
{
	CWebChannelThreadController::instance()->getMoreSearchResults(this);
}

void CWebChannelObjects::autoCorrect(const QString &strElementID, const QString &strPhrase, int nCursorPos, const QString &strLastPhrase, int nLastCursorPos)
{
	CWebChannelThreadController::instance()->autoCorrect(this, strElementID, strPhrase, nCursorPos, strLastPhrase, nLastCursorPos);
}

void CWebChannelObjects::calcUpdatedPhrase(const QString &strElementID, const QString &strPhrase, const QString &strAutoCompleter, int nCursorPos)
{
	CWebChannelThreadController::instance()->calcUpdatedPhrase(this, strElementID, strPhrase, strAutoCompleter, nCursorPos);
}

void CWebChannelObjects::getSearchResultDetails(uint32_t ndxLogical)
{
	CWebChannelThreadController::instance()->getSearchResultDetails(this, ndxLogical);
}

void CWebChannelObjects::resolvePassageReference(const QString &strPassageReference)
{
	CWebChannelThreadController::instance()->resolvePassageReference(this, strPassageReference);
}

void CWebChannelObjects::gotoIndex(uint32_t ndxRel, int nMoveMode, const QString &strParam)
{
	CWebChannelThreadController::instance()->gotoIndex(this, ndxRel, nMoveMode, strParam);
}

void CWebChannelObjects::gotoChapter(int nChp, const QString &strParam)
{
	CWebChannelThreadController::instance()->gotoChapter(this, nChp, strParam);
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

void CWebChannelAdminObjects::sendMessage(const QString &strKey, const QString &strClientIP, const QString &strClientPort, const QString &strMessage)
{
	if (strKey != m_strKey) return;

	bool bStatus = false;
	if (!strMessage.isEmpty()) {
		bStatus = m_pWebServer->sendMessage(strClientIP, strClientPort, strMessage);
	}
	emit sendMessageStatus(bStatus, strClientIP, strClientPort, strMessage);
}

void CWebChannelAdminObjects::getConnectionsList(const QString &strKey)
{
	if (strKey != m_strKey) return;

	const TWebChannelClientMap &mapChannels = m_pWebServer->channelMap();

	QString strClients;
	int nNotSearching = 0;

	QVector<int> lstThreadActivity;		// Number of Active (non-idle) clients per thread

	lstThreadActivity.resize(CWebChannelThreadController::instance()->threadCount());
	for (int n = 0; n < CWebChannelThreadController::instance()->threadCount(); ++n) lstThreadActivity[n] = 0;

	strClients += "<table id=\"connectionsListTable\"><thead>\n";
	strClients += "<th>Name</th><th>IP Address</th><th>Port</th><th>Thread</th><th>Connection Time</th><th>Status</th><th>Bible</th><th>User Agent</th>\n";
	strClients += "</thead><tbody>\n";
	for (TWebChannelClientMap::const_iterator itrChannels = mapChannels.constBegin(); itrChannels != mapChannels.constEnd(); ++itrChannels) {
		QPointer<CWebChannelClient> pClientChannel = itrChannels.value();
		bool bAdmin = ((!pClientChannel.isNull()) ? pClientChannel->isAdmin() : false);
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(!pClientChannel.isNull() ? pClientChannel->bibleUUID() : QString());
		bool bIsSearching = ((!pBibleDatabase.isNull()) && ((!pClientChannel.isNull() ? pClientChannel->threadIndex() : -1) != -1));
		strClients += QString("<tr><td>%1</td><td>%2</td><td>:%3</td><td style=\"text-align:center;\">%4</td><td>%5</td><td style=\"text-align:center;\">%6</td><td style=\"white-space:nowrap;\">%7</td><td style=\"white-space:nowrap;\">%8</td></tr>\n")
					.arg(itrChannels.key()->socket()->peerName() + (bAdmin ? QString("%1(Admin)").arg(!itrChannels.key()->socket()->peerName().isEmpty() ? " " : "") : ""))
					.arg(itrChannels.key()->socket()->peerAddress().toString())
					.arg(itrChannels.key()->socket()->peerPort())
					.arg(!pClientChannel.isNull() ? pClientChannel->threadIndex() : -1)
					.arg(!pClientChannel.isNull() ? pClientChannel->connectionTime() : "")
					.arg(!bAdmin ? ((!pClientChannel.isNull() && !pClientChannel->isIdle()) ? "Active" : "Idle") : "n/a")
					.arg(bIsSearching ? pBibleDatabase->description() : QString())
					.arg(!pClientChannel.isNull() ? pClientChannel->userAgent() : QString());
		if ((pClientChannel.isNull()) || (pClientChannel->threadIndex() == -1)) {
			++nNotSearching;
		} else {
			if (!pClientChannel->isIdle()) ++lstThreadActivity[pClientChannel->threadIndex()];
		}
	}
	strClients += "</tbody></table>\n";
	strClients += QString("<br />Connections: %1<br /><hr /><br />\n").arg(mapChannels.size());

	strClients += QString("Thread Count: %1<br /><br />\n").arg(CWebChannelThreadController::instance()->threadCount());
	strClients += "<table id=\"threadListTable\"><thead>\n";
	strClients += "<th>Thread</th><th>Connections</th><th>Active</th><th>Idle</th>\n";
	strClients += "</thead><tbody style=\"text-align:center;\">\n";
	for (int ndx = -1; ndx < CWebChannelThreadController::instance()->threadCount(); ++ndx) {
		if (ndx != -1) {
			strClients += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>\n")
										.arg(ndx)
										.arg(CWebChannelThreadController::instance()->threadWebChannelCount(ndx))
										.arg(lstThreadActivity.at(ndx))
										.arg(CWebChannelThreadController::instance()->threadWebChannelCount(ndx) - lstThreadActivity.at(ndx));
		} else {
			strClients += QString("<tr><td>%1</td><td>%2</td><td>n/a</td><td>n/a</td></tr>\n").arg(ndx).arg(nNotSearching);
		}
	}
	strClients += "</tbody></table>\n";

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

void CWebChannelAdminObjects::disconnectClient(const QString &strKey, const QString &strClientIP, const QString &strClientPort)
{
	if (strKey != m_strKey) return;

	bool bStatus = m_pWebServer->disconnectClient(strClientIP, strClientPort);
	emit disconnectClientStatus(bStatus, strClientIP, strClientPort);
}

void CWebChannelAdminObjects::stopListening(const QString &strKey)
{
	if (strKey != m_strKey) return;

	m_pWebServer->stopListening();
	getIsListening(strKey);
}

void CWebChannelAdminObjects::startListening(const QString &strKey)
{
	if (strKey != m_strKey) return;

	m_pWebServer->startListening();
	getIsListening(strKey);
}

void CWebChannelAdminObjects::getIsListening(const QString &strKey)
{
	if (strKey != m_strKey) return;

	emit serverListeningStatus(m_pWebServer->isListening());
}

// ============================================================================
