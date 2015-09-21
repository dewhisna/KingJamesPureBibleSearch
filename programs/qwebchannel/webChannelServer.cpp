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

#include "webChannelServer.h"
#include "webChannelObjects.h"
#include "websockettransport.h"
#include "webChannelGeoLocate.h"
#include <QUrl>
#include <QWebSocket>
#include <QCoreApplication>

#include <QDateTime>

#ifdef IS_CONSOLE_APP
#include <iostream>
#endif

#define DEBUG_WEBCHANNEL_SERVER_CONNECTIONS 0

// ============================================================================

CWebChannelClient::CWebChannelClient(CWebChannelServer *pParent)
	:	QObject(pParent),
		m_pWebChannelServer(pParent)
{
	assert(m_pWebChannelServer != NULL);

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
	assert(m_pWebChannelServer != NULL);
	m_pWebChannelServer->setClientThreadIndex(this);
}

void CWebChannelClient::setIdle()
{
	assert(m_pWebChannelServer != NULL);
	m_pWebChannelServer->setClientIdle(this);
}

void CWebChannelClient::killWebChannel()
{
	assert(m_pWebChannelServer != NULL);
	m_pWebChannelServer->killClient(this);
}

void CWebChannelClient::setUserAgent()
{
	assert(m_pWebChannelServer != NULL);
	m_pWebChannelServer->setClientUserAgent(this);
}

void CWebChannelClient::setBibleUUID()
{
	assert(m_pWebChannelServer != NULL);
	m_pWebChannelServer->setClientBibleUUID(this);
}

// ============================================================================

CWebChannelServer::CWebChannelServer(const QHostAddress &anAddress, quint16 nPort, QObject *pParent)
	:	QObject(pParent),
		m_server("King James Pure Bible Search WebChannel Server", QWebSocketServer::NonSecureMode),
		m_clientWrapper(&m_server),
		m_HostAddress(anAddress),
		m_nHostPort(nPort)
{
	m_pGeoLocater = new CWebChannelGeoLocate(this);
	connect(m_pGeoLocater, SIGNAL(locationInfo(const CWebChannelClient *, const QString &)), this, SLOT(setClientLocation(const CWebChannelClient *, const QString &)));

	// setup the QWebSocketServer
	m_server.listen(anAddress, nPort);

	// create the CWebChannelAdminObjects
	m_pWebChannelAdminObjects = new CWebChannelAdminObjects(this);
	connect(m_pWebChannelAdminObjects.data(), SIGNAL(broadcast(const QString &)), this, SLOT(sendBroadcast(const QString &)));

	// Handle connections:
	connect(&m_clientWrapper, SIGNAL(clientConnected(WebSocketTransport*)), this, SLOT(en_clientConnected(WebSocketTransport*)));

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
			pClient->socket()->peerAddress().toString().toUtf8().data(),
			pClient->socket()->peerPort(),
			m_mapChannels.size());
#endif
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Connected : \"%2\" (%3) port %4 -- %5 Connections\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(pClient->socket()->peerName())
							.arg(pClient->socket()->peerAddress().toString())
							.arg(pClient->socket()->peerPort())
							.arg(m_mapChannels.size())
							.toUtf8().data();
	std::cout.flush();
#endif

	// Trigger GeoLocate:
	m_pGeoLocater->locate(pClientChannel, pClient->socket()->peerAddress().toString());
}

void CWebChannelServer::en_clientDisconnected(WebSocketTransport* pClient)
{
	TWebChannelClientMap::iterator itrClientMap = m_mapChannels.find(pClient);
	assert(itrClientMap != m_mapChannels.end());
	if (itrClientMap != m_mapChannels.end()) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) delete pClientChannel;
		m_mapChannels.remove(pClient);
#if DEBUG_WEBCHANNEL_SERVER_CONNECTIONS
	qDebug("Client Disconnection from: \"%s\" (%s) port %d  --  %d Connections",
			pClient->socket()->peerName().toUtf8().data(),
			pClient->socket()->peerAddress().toString().toUtf8().data(),
			pClient->socket()->peerPort(),
			m_mapChannels.size());
#endif
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Disconnected : \"%2\" (%3) port %4 -- %5 Connections\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(pClient->socket()->peerName())
							.arg(pClient->socket()->peerAddress().toString())
							.arg(pClient->socket()->peerPort())
							.arg(m_mapChannels.size())
							.toUtf8().data();
	std::cout.flush();
#endif
	}

#ifdef IS_CONSOLE_APP
	if (m_mapChannels.isEmpty() && !isListening()) {
		// If the last client disconnects and the server was already made deaf,
		//	the exit our daemon or else we'll have a stuck process:
		QCoreApplication::exit(0);
	}
#endif
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
				itrClientMap.key()->socket()->peerAddress().toString().toUtf8().data(),
				itrClientMap.key()->socket()->peerPort(),
				nNumConnections);
#endif
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Disconnected : \"%2\" (%3) port %4 -- %5 Connections\n")
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
							.arg(itrClientMap.key()->socket()->peerName())
							.arg(itrClientMap.key()->socket()->peerAddress().toString())
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
		if ((itrClientMap.key()->socket()->peerAddress().toString().compare(strClientIP, Qt::CaseInsensitive) == 0) &&
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
		if ((itrClientMap.key()->socket()->peerAddress().toString().compare(strClientIP, Qt::CaseInsensitive) == 0) &&
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
									.arg(itrClientMap.key()->socket()->peerAddress().toString())
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
									.arg(itrClientMap.key()->socket()->peerAddress().toString())
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
									.arg(itrClientMap.key()->socket()->peerAddress().toString())
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
									.arg(itrClientMap.key()->socket()->peerAddress().toString())
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
									.arg(itrClientMap.key()->socket()->peerAddress().toString())
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

void CWebChannelServer::setClientLocation(const CWebChannelClient *pClient, const QString &strLocationInfo)
{
	for (TWebChannelClientMap::const_iterator itrClientMap = m_mapChannels.constBegin(); itrClientMap != m_mapChannels.constEnd(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if ((!pClientChannel.isNull()) && (pClient == pClientChannel.data())) {
#ifdef IS_CONSOLE_APP
			std::cout << QString("%1 UTC : GeoLocate : \"%2\" (%3) port %4 : %5\n")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate))
									.arg(itrClientMap.key()->socket()->peerName())
									.arg(itrClientMap.key()->socket()->peerAddress().toString())
									.arg(itrClientMap.key()->socket()->peerPort())
									.arg(strLocationInfo)
									.toUtf8().data();
			std::cout.flush();
#else
			Q_UNUSED(strLocationInfo);
#endif
			break;
		}
	}
}

void CWebChannelServer::stopListening()
{
	if (m_server.isListening()) m_server.close();
}

void CWebChannelServer::startListening()
{
	if (!m_server.isListening()) m_server.listen(m_HostAddress, m_nHostPort);
}

// ============================================================================
