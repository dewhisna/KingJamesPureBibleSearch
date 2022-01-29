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

#include "webChannelServer.h"
#include "webChannelObjects.h"
#include "websockettransport.h"
#include "webChannelGeoLocate.h"
#include <QUrl>
#include <QWebSocket>
#include <QNetworkRequest>
#include <QCoreApplication>

#include <QDateTime>

#ifdef IS_CONSOLE_APP
#include <iostream>
#endif

#define DEBUG_WEBCHANNEL_SERVER_CONNECTIONS 0

#define CLIENT_STATE_CHECK_RATE			60000			// Check clients every 60 seconds

// ============================================================================

CWebChannelClient::CWebChannelClient(CWebChannelServer *pParent)
	:	QObject(pParent),
		m_nChannelState(WCCS_CREATED),
		m_pWebChannelServer(pParent)
{
	Q_ASSERT(m_pWebChannelServer != nullptr);

	m_strConnectionTime = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

	m_pWebChannelObjects = new CWebChannelObjects(this);
	registerObject("kjpbs", m_pWebChannelObjects);
}

CWebChannelClient::~CWebChannelClient()
{

}

QString CWebChannelClient::connectionTime() const
{
	return m_strConnectionTime;
}

int CWebChannelClient::threadIndex() const
{
	if (!m_pWebChannelObjects.isNull()) return m_pWebChannelObjects->threadIndex();
	return -1;
}

bool CWebChannelClient::isIdle() const
{
	if (!m_pWebChannelObjects.isNull()) return m_pWebChannelObjects->isIdle();
	return true;		// So idle it's missing, LOL
}

bool CWebChannelClient::isAdmin() const
{
	if (!m_pWebChannelObjects.isNull()) return m_pWebChannelObjects->isAdmin();
	return false;
}

QString CWebChannelClient::userAgent() const
{
	if (!m_pWebChannelObjects.isNull()) return m_pWebChannelObjects->userAgent();
	return QString();
}

QString CWebChannelClient::bibleUUID() const
{
	if (!m_pWebChannelObjects.isNull()) return m_pWebChannelObjects->bibleUUID();
	return QString();
}

void CWebChannelClient::connectTo(WebSocketTransport* pClient)
{
	m_channel.connectTo(pClient);
}

void CWebChannelClient::registerObject(const QString &strID, QObject *pObject)
{
	m_channel.registerObject(strID, pObject);
}

void CWebChannelClient::deregisterObject(QObject *pObject)
{
	m_channel.deregisterObject(pObject);
}

void CWebChannelClient::sendBroadcast(const QString &strMessage)
{
	if (!m_pWebChannelObjects.isNull()) m_pWebChannelObjects->sendBroadcast(strMessage);
}

void CWebChannelClient::setThreadIndex()
{
	Q_ASSERT(m_pWebChannelServer != nullptr);
	m_pWebChannelServer->setClientThreadIndex(this);
}

void CWebChannelClient::setIdle()
{
	Q_ASSERT(m_pWebChannelServer != nullptr);
	m_pWebChannelServer->setClientIdle(this);
}

void CWebChannelClient::killWebChannel()
{
	Q_ASSERT(m_pWebChannelServer != nullptr);
	m_pWebChannelServer->killClient(this);
}

void CWebChannelClient::setUserAgent()
{
	Q_ASSERT(m_pWebChannelServer != nullptr);
	m_pWebChannelServer->setClientUserAgent(this);
}

void CWebChannelClient::setBibleUUID()
{
	Q_ASSERT(m_pWebChannelServer != nullptr);
	m_pWebChannelServer->setClientBibleUUID(this);
}

// ============================================================================

CWebChannelServer::CWebChannelServer(const QHostAddress &anAddress, quint16 nPort, QObject *pParent)
	:	QObject(pParent),
		m_server("King James Pure Bible Search WebChannel Server", QWebSocketServer::NonSecureMode),
		m_clientWrapper(&m_server),
		m_HostAddress(anAddress),
		m_nHostPort(nPort),
		m_pGeoLocater(nullptr)
{
	m_pGeoLocater = new CWebChannelGeoLocate(this);
	connect(m_pGeoLocater, SIGNAL(locationInfo(const CWebChannelClient *, const QString &, const QString &)), this, SLOT(setClientLocation(const CWebChannelClient *, const QString &, const QString &)));

	// setup the QWebSocketServer
	m_server.listen(anAddress, nPort);

	// create the CWebChannelAdminObjects
	m_pWebChannelAdminObjects = new CWebChannelAdminObjects(this);
	connect(m_pWebChannelAdminObjects.data(), SIGNAL(broadcast(const QString &)), this, SLOT(sendBroadcast(const QString &)));

	// Handle connections:
	connect(&m_clientWrapper, SIGNAL(clientConnected(WebSocketTransport*)), this, SLOT(en_clientConnected(WebSocketTransport*)));

	// Handle client state checking:
	connect(&m_tmrStateCheck, SIGNAL(timeout()), this, SLOT(en_checkClientStates()));
	m_tmrStateCheck.start(CLIENT_STATE_CHECK_RATE);

#ifdef IS_CONSOLE_APP
	if (m_server.isListening()) {
		std::cout << QString("%1 UTC : KJPBS-WebChannel (pid=%2) started on interface \"%3\" port %4\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(QCoreApplication::applicationPid())
							.arg(serverAddress().toString())
							.arg(serverPort())
							.toUtf8().data();
		std::cout.flush();
	}
#endif
}

CWebChannelServer::~CWebChannelServer()
{

}

QString CWebChannelServer::url(const QString &strBaseURLGood, const QString &strBaseURLBad) const
{
	QUrl aURL(m_server.isListening() ? strBaseURLGood : strBaseURLBad);
	if (isListening()) {
		aURL.setQuery(QStringLiteral("webChannelBaseUrl=") + m_server.serverUrl().toString());
	}
	return aURL.toString();
}

void CWebChannelServer::en_clientConnected(WebSocketTransport* pClient)
{
	// Handle disconnections:
	connect(pClient, SIGNAL(clientDisconnected(WebSocketTransport*)), this, SLOT(en_clientDisconnected(WebSocketTransport*)));

	// setup the channel
	QPointer<CWebChannelClient> pClientChannel = new CWebChannelClient(this);
	pClientChannel->registerObject("mosis", m_pWebChannelAdminObjects);
	pClientChannel->connectTo(pClient);
	m_mapChannels[pClient] = pClientChannel;

#if DEBUG_WEBCHANNEL_SERVER_CONNECTIONS
	qDebug("Client Connection from: \"%s\" (%s) port %d  --  %d Connections",
			pClient->socket()->peerName().toUtf8().data(),
			peerAddressOfSocket(pClient).toUtf8().data(),
			pClient->socket()->peerPort(),
			m_mapChannels.size());
#endif
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Connected : \"%2\" (%3) port %4 -- %5 Connections\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(pClient->socket()->peerName())
							.arg(peerAddressOfSocket(pClient))
							.arg(pClient->socket()->peerPort())
							.arg(m_mapChannels.size())
							.toUtf8().data();
	std::cout.flush();
#endif

	// Trigger GeoLocate:
	m_pGeoLocater->locate(pClientChannel, peerAddressOfSocket(pClient));
}

void CWebChannelServer::en_clientDisconnected(WebSocketTransport* pClient)
{
	TWebChannelClientMap::iterator itrClientMap = m_mapChannels.find(pClient);
	Q_ASSERT(itrClientMap != m_mapChannels.end());
	if (itrClientMap != m_mapChannels.end()) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) delete pClientChannel;
		m_mapChannels.remove(pClient);
#if DEBUG_WEBCHANNEL_SERVER_CONNECTIONS
	qDebug("Client Disconnection from: \"%s\" (%s) port %d  --  %d Connections",
			pClient->socket()->peerName().toUtf8().data(),
			peerAddressOfSocket(pClient).toUtf8().data(),
			pClient->socket()->peerPort(),
			m_mapChannels.size());
#endif
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Disconnected : \"%2\" (%3) port %4 -- %5 Connections\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(pClient->socket()->peerName())
							.arg(peerAddressOfSocket(pClient))
							.arg(pClient->socket()->peerPort())
							.arg(m_mapChannels.size())
							.toUtf8().data();
	std::cout.flush();
#endif
	}

#ifdef IS_CONSOLE_APP
	if (m_mapChannels.isEmpty() && !isListening()) {
		// If the last client disconnects and the server was already made deaf,
		//	then exit our daemon or else we'll have a stuck process:
		QCoreApplication::exit(0);
	}
#endif
}

void CWebChannelServer::en_checkClientStates()
{
	// To keep from blowing away our iterator by closing the socket directly
	//		in this loop, make a list of clients to disconnect:
	QList<WebSocketTransport *> lstClientsToDrop;

	// Loop through clients and check status:
	for (TWebChannelClientMap::iterator itrClientMap = m_mapChannels.begin(); itrClientMap != m_mapChannels.end(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) {
			switch (pClientChannel->channelState()) {
				case CWebChannelClient::WCCS_DEAD:
					// If the channel was last deemed as dead, give one final check
					//	to see if it has become active.  If not, drop the connection:
					if (!pClientChannel->isAdmin()) {
						if (pClientChannel->threadIndex() == -1) {
							// Disconnect and remove dead client:
							lstClientsToDrop.append(itrClientMap.key());
						} else {
							// Client seems to be active now, as it's been assigned to a worker thread, so revive it:
							pClientChannel->setChannelState(CWebChannelClient::WCCS_ACTIVE);
						}
					} else {
						// Note: Admin connections should never be dead, as it wouldn't be deemed
						//	admin unless it made it through init.  But if it somehow got tagged as
						//	dead, move it to active:
						pClientChannel->setChannelState(CWebChannelClient::WCCS_ACTIVE);
					}
					break;

				case CWebChannelClient::WCCS_CREATED:
					// If the channel was created, see if it's become active (i.e. assigned to
					//	a worker thread.  If not, move to the dead state to possibly drop next time:
					if (!pClientChannel->isAdmin()) {
						if (pClientChannel->threadIndex() == -1) {
							// If the client object was created, but not assigned to a worker
							//	thread, then it means that there's a chance the connection (while
							//	still active) has failed WebChannel registration and initialization.
							//	Flag as dead to check it next time, and if it's still inactive,
							//	we'll drop the connection then:
							pClientChannel->setChannelState(CWebChannelClient::WCCS_DEAD);
						} else {
							// Once the connection is assigned to a worker thread, move it to the
							//	active state.  WebChannel registration and initialization was
							//	successful and the search objects will be responsible for dropping
							//	it should it become idle:
							pClientChannel->setChannelState(CWebChannelClient::WCCS_ACTIVE);
						}
					} else {
						// Note: Admin connections will have (by definition) made it through init
						//	already, even though it doesn't get assigned to a worker thread.  Therefore,
						//	move it straight to the active state:
						pClientChannel->setChannelState(CWebChannelClient::WCCS_ACTIVE);
					}
					break;

				case CWebChannelClient::WCCS_ACTIVE:
					// Leave active clients alone.  The search objects on the worker threads
					//	will be responsible for dropping inactive users should they stop
					//	interacting with the server
					break;
			}
		} else {
			// Disconnect and remove objects that have gotten deleted:
			lstClientsToDrop.append(itrClientMap.key());
		}
	}

	// Close and remove prescribed clients (removal happens in disconnect signal processing):
	for (int i = 0; i < lstClientsToDrop.size(); ++i) {
		Q_ASSERT(lstClientsToDrop.at(i) != nullptr);
		lstClientsToDrop.at(i)->socket()->close(QWebSocketProtocol::CloseCodeGoingAway, "disconnectClient");
	}
}

void CWebChannelServer::close()
{
	// Make server stop listening so we don't get any new connections (eliminate race condition):
	if (m_server.isListening()) {
		m_server.close();
	}

	// To keep from blowing our iterator, first disconnect the close events from all clients:
	for (TWebChannelClientMap::iterator itrClientMap = m_mapChannels.begin(); itrClientMap != m_mapChannels.end(); ++itrClientMap) {
		disconnect(itrClientMap.key(), SIGNAL(clientDisconnected(WebSocketTransport*)), this, SLOT(en_clientDisconnected(WebSocketTransport*)));
	}

	// Now, close them:
	int nNumConnections = m_mapChannels.size();
	for (TWebChannelClientMap::iterator itrClientMap = m_mapChannels.begin(); itrClientMap != m_mapChannels.end(); ++itrClientMap) {
		--nNumConnections;
#if DEBUG_WEBCHANNEL_SERVER_CONNECTIONS
		qDebug("Client Disconnection from: \"%s\" (%s) port %d  --  %d Connections",
				itrClientMap.key()->socket()->peerName().toUtf8().data(),
				peerAddressOfSocket(itrClientMap.key()).toUtf8().data(),
				itrClientMap.key()->socket()->peerPort(),
				nNumConnections);
#endif
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Disconnected : \"%2\" (%3) port %4 -- %5 Connections\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(itrClientMap.key()->socket()->peerName())
							.arg(peerAddressOfSocket(itrClientMap.key()))
							.arg(itrClientMap.key()->socket()->peerPort())
							.arg(nNumConnections)
							.toUtf8().data();
	std::cout.flush();
#endif
		itrClientMap.key()->socket()->close(QWebSocketProtocol::CloseCodeGoingAway, "Server shutting down");
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) delete pClientChannel;
	}
	m_mapChannels.clear();

#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : KJPBS-WebChannel (pid=%2) stopped\n")
						.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
						.arg(QCoreApplication::applicationPid())
						.toUtf8().data();
	std::cout.flush();
#endif
}

bool CWebChannelServer::disconnectClient(const QString &strClientIP, const QString &strClientPort)
{
	for (TWebChannelClientMap::iterator itrClientMap = m_mapChannels.begin(); itrClientMap != m_mapChannels.end(); ++itrClientMap) {
		if ((peerAddressOfSocket(itrClientMap.key()).compare(strClientIP, Qt::CaseInsensitive) == 0) &&
			(itrClientMap.key()->socket()->peerPort() == strClientPort.toUInt())) {
			itrClientMap.key()->socket()->close(QWebSocketProtocol::CloseCodeGoingAway, "disconnectClient");
			return true;
		}
	}
	return false;
}

void CWebChannelServer::sendBroadcast(const QString &strMessage)
{
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Sending Broadcast Message : %2\n").arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).arg(strMessage).toUtf8().data();
	std::cout.flush();
#endif
	for (TWebChannelClientMap::iterator itrClientMap = m_mapChannels.begin(); itrClientMap != m_mapChannels.end(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) pClientChannel->sendBroadcast(strMessage);
	}
}

bool CWebChannelServer::sendMessage(const QString &strClientIP, const QString &strClientPort, const QString &strMessage)
{
	TWebChannelClientMap::iterator itrClientMap = m_mapChannels.begin();
	do {
		if ((peerAddressOfSocket(itrClientMap.key()).compare(strClientIP, Qt::CaseInsensitive) == 0) &&
			(itrClientMap.key()->socket()->peerPort() == strClientPort.toUInt())) {
			break;
		}
		++itrClientMap;
	} while (itrClientMap != m_mapChannels.end());
	if (itrClientMap != m_mapChannels.end()) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) {
			pClientChannel->sendBroadcast(strMessage);
			return true;
		}
	}
	return false;
}

void CWebChannelServer::setClientThreadIndex(const CWebChannelClient *pClient)
{
	for (TWebChannelClientMap::const_iterator itrClientMap = m_mapChannels.constBegin(); itrClientMap != m_mapChannels.constEnd(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if ((!pClientChannel.isNull()) && (pClient == pClientChannel.data())) {
#ifdef IS_CONSOLE_APP
			std::cout << QString("%1 UTC : Setting Thread : \"%2\" (%3) port %4 : Index %5\n")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
									.arg(itrClientMap.key()->socket()->peerName())
									.arg(peerAddressOfSocket(itrClientMap.key()))
									.arg(itrClientMap.key()->socket()->peerPort())
									.arg(pClientChannel->threadIndex())
									.toUtf8().data();
			std::cout.flush();
#endif
			break;
		}
	}
}

void CWebChannelServer::setClientIdle(const CWebChannelClient *pClient)
{
	for (TWebChannelClientMap::const_iterator itrClientMap = m_mapChannels.constBegin(); itrClientMap != m_mapChannels.constEnd(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if ((!pClientChannel.isNull()) && (pClient == pClientChannel.data())) {
#ifdef IS_CONSOLE_APP
			std::cout << QString("%1 UTC : Idle Status : \"%2\" (%3) port %4 : %5\n")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
									.arg(itrClientMap.key()->socket()->peerName())
									.arg(peerAddressOfSocket(itrClientMap.key()))
									.arg(itrClientMap.key()->socket()->peerPort())
									.arg(pClientChannel->isIdle() ? "Idle" : "Active")
									.toUtf8().data();
			std::cout.flush();
#endif
			break;
		}
	}
}

void CWebChannelServer::killClient(const CWebChannelClient *pClient)
{
	for (TWebChannelClientMap::const_iterator itrClientMap = m_mapChannels.constBegin(); itrClientMap != m_mapChannels.constEnd(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if ((!pClientChannel.isNull()) && (pClient == pClientChannel.data())) {
#ifdef IS_CONSOLE_APP
			std::cout << QString("%1 UTC : Killing Unresponsive Client : \"%2\" (%3) port %4\n")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
									.arg(itrClientMap.key()->socket()->peerName())
									.arg(peerAddressOfSocket(itrClientMap.key()))
									.arg(itrClientMap.key()->socket()->peerPort())
									.toUtf8().data();
			std::cout.flush();
#endif
			itrClientMap.key()->socket()->close(QWebSocketProtocol::CloseCodeNormal, "Killing unresponsive client");
			break;
		}
	}
}

void CWebChannelServer::setClientUserAgent(const CWebChannelClient *pClient)
{
	for (TWebChannelClientMap::const_iterator itrClientMap = m_mapChannels.constBegin(); itrClientMap != m_mapChannels.constEnd(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if ((!pClientChannel.isNull()) && (pClient == pClientChannel.data())) {
#ifdef IS_CONSOLE_APP
			std::cout << QString("%1 UTC : UserAgent : \"%2\" (%3) port %4 : %5\n")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
									.arg(itrClientMap.key()->socket()->peerName())
									.arg(peerAddressOfSocket(itrClientMap.key()))
									.arg(itrClientMap.key()->socket()->peerPort())
									.arg(pClientChannel->userAgent())
									.toUtf8().data();
			std::cout.flush();
#endif
			break;
		}
	}
}

void CWebChannelServer::setClientBibleUUID(const CWebChannelClient *pClient)
{
	for (TWebChannelClientMap::const_iterator itrClientMap = m_mapChannels.constBegin(); itrClientMap != m_mapChannels.constEnd(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if ((!pClientChannel.isNull()) && (pClient == pClientChannel.data())) {
#ifdef IS_CONSOLE_APP
			CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(pClientChannel->bibleUUID());
			bool bValid = !pBibleDatabase.isNull();
			std::cout << QString(bValid ?	"%1 UTC : Select Bible : \"%2\" (%3) port %4 : {%5} %6\n" :
											"%1 UTC : Select Bible : \"%2\" (%3) port %4 : invalid/unknown%5%6\n")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
									.arg(itrClientMap.key()->socket()->peerName())
									.arg(peerAddressOfSocket(itrClientMap.key()))
									.arg(itrClientMap.key()->socket()->peerPort())
									.arg(bValid ? pClientChannel->bibleUUID() : QString())
									.arg(bValid ? pBibleDatabase->description() : QString())
									.toUtf8().data();
			std::cout.flush();
#endif
			break;
		}
	}
}

void CWebChannelServer::setClientLocation(const CWebChannelClient *pClient, const QString &strIPAddress, const QString &strLocationInfo)
{
#ifdef IS_CONSOLE_APP

	bool bFound = false;

	for (TWebChannelClientMap::const_iterator itrClientMap = m_mapChannels.constBegin(); itrClientMap != m_mapChannels.constEnd(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if ((!pClientChannel.isNull()) && (pClient == pClientChannel.data())) {
			bFound = true;
			std::cout << QString("%1 UTC : GeoLocate : \"%2\" (%3) port %4 : %5\n")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
									.arg(itrClientMap.key()->socket()->peerName())
									.arg(peerAddressOfSocket(itrClientMap.key()))
									.arg(itrClientMap.key()->socket()->peerPort())
									.arg(strLocationInfo)
									.toUtf8().data();
			std::cout.flush();
			break;
		}
	}
	if (!bFound) {
		// If the client disconnected and is no longer in our map, go ahead and log the GeoIP data
		//		just without the port detail:
		std::cout << QString("%1 UTC : GeoLocate : \"\" (%2) port 0 : %3\n")
								.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
								.arg(strIPAddress)
								.arg(strLocationInfo)
								.toUtf8().data();
		std::cout.flush();
	}

#else
	Q_UNUSED(pClient);
	Q_UNUSED(strIPAddress);
	Q_UNUSED(strLocationInfo);
#endif
}

void CWebChannelServer::stopListening()
{
	if (m_server.isListening()) m_server.close();
}

void CWebChannelServer::startListening()
{
	if (!m_server.isListening()) m_server.listen(m_HostAddress, m_nHostPort);
}

// ----------------------------------------------------------------------------

// Helper function to resolve the Client IP address.  This is needed to support
//	connections through webserver reverse proxy so that we return the IP address
//	of the client connection and not the websever.
//	This will return the regular peerAddress unless there's an X-Forwarded-For
//	header present:
QString CWebChannelServer::peerAddressOfSocket(const WebSocketTransport *pClient)
{
	Q_ASSERT(pClient != nullptr);
	QString strPeerAddress = pClient->socket()->peerAddress().toString();

#if QT_VERSION >= 0x050600		// QWebSocket::request() was added in Qt 5.6
	if (pClient->socket()->request().hasRawHeader(QString("X-Forwarded-For").toUtf8())) {
		strPeerAddress = pClient->socket()->request().rawHeader(QString("X-Forwarded-For").toUtf8());
	}
#endif

	return strPeerAddress;
}

// ============================================================================
