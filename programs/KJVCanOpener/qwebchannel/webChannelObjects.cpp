/****************************************************************************
**
** Copyright (C) 2015-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#include "webChannelGeoLocate.h"
#include "mmdblookup.h"
#include "CSV.h"

#include <QWebSocket>
#include <QVector>

#include <QDateTime>

#ifdef IS_CONSOLE_APP
#include <iostream>
#include <QCoreApplication>
#endif

// External keys from webChannelKeys.cpp as generated via webChannelKeyGen.sh
extern const char *g_pWebChannelAdminKey;

// ============================================================================

//
// CWebChannelObjects
//

CWebChannelObjects::CWebChannelObjects(CWebChannelClient *pParent)
	:	QObject(pParent),
		m_bIsAdmin(false),
		m_nThreadIndex(-1),
		m_bIsIdle(false),
		m_bHasSelectedBible(false),
		m_pWebChannel(pParent)
{
	Q_ASSERT(m_pWebChannel != nullptr);
}

CWebChannelObjects::~CWebChannelObjects()
{
	CWebChannelThreadController::instance()->destroyWebChannelSearchResults(this);
}

void CWebChannelObjects::setThreadIndex(int nThreadIndex)
{
	Q_ASSERT(m_pWebChannel != nullptr);

	m_nThreadIndex = nThreadIndex;
	m_pWebChannel->setThreadIndex();
}

void CWebChannelObjects::en_idleStateChanged(bool bIsIdle)
{
	Q_ASSERT(m_pWebChannel != nullptr);

	m_bIsIdle = bIsIdle;
	m_pWebChannel->setIdle();
}

void CWebChannelObjects::en_killWebChannel()
{
	Q_ASSERT(m_pWebChannel != nullptr);
	m_pWebChannel->killWebChannel();
}

// ----------------------------------------------------------------------------

void CWebChannelObjects::unlock(const QString &strKey, const QString &strConfirmation)
{
	if ((strKey.compare(g_pWebChannelAdminKey) == 0) &&
		(strConfirmation.compare("A609FDFD-BB3C-4BEB-AFDA-9A839F940346") == 0)) {
		m_bIsAdmin = true;
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : Admin : WebChannelObjects Unlock Successful\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8().data();
		std::cout.flush();
#endif
	} else {
		// An invalid key forces disconnection:
		en_killWebChannel();
		m_bIsAdmin = false;
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key Received: \"%2\"  Confirmation: \"%3\".  Terminating Connection\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).arg(strConfirmation).toUtf8().data();
		std::cout.flush();
#endif
	}
}

void CWebChannelObjects::setUserAgent(const QString &strUserAgent)
{
	Q_ASSERT(m_pWebChannel != nullptr);

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
	Q_ASSERT(m_pWebChannel != nullptr);

	if (CWebChannelThreadController::instance()->selectBible(this, strUUID)) {
		m_strBibleUUID = strUUID;
		m_bHasSelectedBible = true;
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

void CWebChannelObjects::gotoChapter(unsigned int nChp, const QString &strParam)
{
	CWebChannelThreadController::instance()->gotoChapter(this, nChp, strParam);
}

// ============================================================================

//
// CWebChannelAdminObjects
//

CWebChannelAdminObjects::CWebChannelAdminObjects(CWebChannelServer *pWebServerParent)
	:	QObject(pWebServerParent),
		m_strKey(g_pWebChannelAdminKey),
		m_pWebServer(pWebServerParent)
{

}

CWebChannelAdminObjects::~CWebChannelAdminObjects()
{

}

void CWebChannelAdminObjects::sendBroadcast(const QString &strKey, const QString &strMessage)
{
	if (strKey != m_strKey) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key on Send Broadcast.  Key: \"%2\"  Message: \"%3\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).arg(strMessage).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Send Broadcast.  Message: \"%2\"\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
					.arg(strMessage).toUtf8().data();
	std::cout.flush();
#endif
	emit broadcast(strMessage);
}

void CWebChannelAdminObjects::sendMessage(const QString &strKey, const QString &strClientIP, const QString &strClientPort, const QString &strMessage)
{
	if (strKey != m_strKey) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key on Send Message.  Key: \"%2\"  Message: \"%3\"  Client: \"%4\"  Port: \"%5\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).arg(strMessage).arg(strClientIP).arg(strClientPort).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Send Message.  Message: \"%2\"  Client: \"%3\"  Port: \"%4\"\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
					.arg(strMessage).arg(strClientIP).arg(strClientPort).toUtf8().data();
	std::cout.flush();
#endif

	bool bStatus = false;
	if (!strMessage.isEmpty()) {
		bStatus = m_pWebServer->sendMessage(strClientIP, strClientPort, strMessage);
	}
	emit sendMessageStatus(bStatus, strClientIP, strClientPort, strMessage);
}

void CWebChannelAdminObjects::getConnectionsList(const QString &strKey)
{
	if (strKey != m_strKey) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key on Get Connections list.  Key: \"%2\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Get Connections list\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8().data();
	std::cout.flush();
#endif

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
		QString strGeoIP;
		QString strStatus;
#ifdef USING_MMDB
		CMMDBLookup mmdb;
		QString strJSON;
		QByteArray baData;
		QString strIPAddress = CWebChannelServer::peerAddressOfSocket(itrChannels.key());
		if (mmdb.lookup(strJSON, strIPAddress)) {
			baData = strJSON.toUtf8();
			QJsonParseError jsonError;
			QJsonDocument json = QJsonDocument::fromJson(baData, &jsonError);
			if (jsonError.error == QJsonParseError::NoError) {
				CWebChannelGeoLocate::TGeoLocateClient theClient;
				theClient.m_nLocateServer = CWebChannelGeoLocate::GSE_INTERNAL;
				theClient.m_strIPAddress = strIPAddress;
				QString strCSV = CWebChannelGeoLocate::jsonToCSV(json, theClient);
				CCSVStream csv(&strCSV, QIODevice::ReadOnly);
				QString strValue;
				QString strLat;
				bool bHaveData = false;
				strGeoIP = "<table style=\"width:100%;\">";
				csv >> strValue;			// strIP;
				csv >> strValue; if (!strValue.isEmpty()) { strGeoIP += "<tr><td style=\"text-align:right;\">CountryCode:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">Country:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">RegionCode:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">Region:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">City:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">PostalCode:</td><td>" + strValue.toHtmlEscaped() + " </td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">TimeZone:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">Latitude:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				strLat = strValue;
				csv >> strValue;
				if (!strValue.isEmpty()) {
					strGeoIP += "<tr><td style=\"text-align:right;\">Longitude:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">";
					if (!strLat.isEmpty() && !strValue.isEmpty()) {
						strGeoIP += "<button type=\"button\" onclick=\"javascript:mapLatLong(" + strLat + ", " + strValue + ");\">Display Map</button>";
					} else {
						strGeoIP += "&nbsp;";
					}
					strGeoIP += "</td></tr>";
					bHaveData = true;
				}
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">MetroCode:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				csv >> strValue; if (!strValue.isEmpty()) {  strGeoIP += "<tr><td style=\"text-align:right;\">ISP:</td><td>" + strValue.toHtmlEscaped() + "</td><td style=\"width:99%\">&nbsp;</td></tr>"; bHaveData = true; }
				strGeoIP += "</table>";
				if (!bHaveData) strGeoIP.clear();
			}
		}
#endif
		if (bAdmin) {
			strStatus = "n/a";
		} else {
			if (!pClientChannel.isNull()) {
				switch (pClientChannel->channelState()) {
					case CWebChannelClient::WCCS_CREATED:
						strStatus = "Created";
						break;
					case CWebChannelClient::WCCS_ACTIVE:
						strStatus = (!pClientChannel->isIdle() ? "Active" : "Idle");
						break;
					case CWebChannelClient::WCCS_DEAD:
						strStatus = "Dead";
						break;
				}
			} else {
				strStatus = "Dropped";
			}
		}

		strClients += QString("<tr><td>%1</td><td>%2</td><td>:%3</td><td style=\"text-align:center;\">%4</td><td>%5</td><td style=\"text-align:center;\">%6</td><td style=\"white-space:nowrap;\">%7</td><td style=\"white-space:nowrap;\">%8</td></tr>\n")
					.arg(itrChannels.key()->socket()->peerName() + (bAdmin ? QString("%1(Admin)").arg(!itrChannels.key()->socket()->peerName().isEmpty() ? " " : "") : ""))
					.arg(CWebChannelServer::peerAddressOfSocket(itrChannels.key()))
					.arg(itrChannels.key()->socket()->peerPort())
					.arg(!pClientChannel.isNull() ? pClientChannel->threadIndex() : -1)
					.arg(!pClientChannel.isNull() ? pClientChannel->connectionTime() : "")
					.arg(strStatus)
					.arg(bIsSearching ? pBibleDatabase->description() : QString())
					.arg(!pClientChannel.isNull() ? pClientChannel->userAgent().toHtmlEscaped() : QString());
		if (!strGeoIP.isEmpty()) {
			strClients += QString("<tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>%1</td></tr>\n")
						.arg(strGeoIP);
		}
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
		(strConfirmation != "9BF89B76-45B2-46EB-B95C-79D460F702BD")) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key or Confirmation on Shutdown Server.  Key: \"%2\"  Confirmation: \"%3\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).arg(strConfirmation).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Shutdown Server\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8().data();
	std::cout.flush();
#endif

#ifdef IS_CONSOLE_APP
	QCoreApplication::exit(0);			// For console-build (i.e. daemon), exit.  Server closing will happen on exit
#else
	m_pWebServer->close();				// For GUI build, just close the webserver
#endif
}

void CWebChannelAdminObjects::disconnectClient(const QString &strKey, const QString &strClientIP, const QString &strClientPort)
{
	if (strKey != m_strKey) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key on Disconnect Client.  Key: \"%2\"  Client: \"%3\"  Port: \"%4\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).arg(strClientIP).arg(strClientPort).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Disconnect Client.  Client: \"%2\"  Port: \"%3\"\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
					.arg(strClientIP).arg(strClientPort).toUtf8().data();
	std::cout.flush();
#endif

	bool bStatus = m_pWebServer->disconnectClient(strClientIP, strClientPort);
	emit disconnectClientStatus(bStatus, strClientIP, strClientPort);
}

void CWebChannelAdminObjects::stopListening(const QString &strKey)
{
	if (strKey != m_strKey) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key on Stop Listening.  Key: \"%2\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Stop Listening\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8().data();
	std::cout.flush();
#endif

	m_pWebServer->stopListening();
	getIsListening(strKey);
}

void CWebChannelAdminObjects::startListening(const QString &strKey)
{
	if (strKey != m_strKey) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key on Start Listening.  Key: \"%2\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Start Listening\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8().data();
	std::cout.flush();
#endif

	m_pWebServer->startListening();
	getIsListening(strKey);
}

void CWebChannelAdminObjects::getIsListening(const QString &strKey)
{
	if (strKey != m_strKey) {
#ifdef IS_CONSOLE_APP
		std::cout << QString("%1 UTC : *** Invalid Admin Key on Get IsListening Status.  Key: \"%2\".  Ignoring\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(strKey).toUtf8().data();
		std::cout.flush();
#endif
		return;
	}

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Admin : Get IsListening Status\n")
					.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8().data();
	std::cout.flush();
#endif

	emit serverListeningStatus(m_pWebServer->isListening());
}

// ============================================================================
