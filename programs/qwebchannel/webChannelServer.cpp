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
#include <QUrl>
#include <QWebSocket>

#ifdef IS_CONSOLE_APP
#include <QDateTime>
#include <iostream>
#endif

#define DEBUG_WEBCHANNEL_SERVER_CONNECTIONS 0

// ============================================================================

CWebChannelClient::CWebChannelClient(QObject *pParent)
	:	QObject(pParent)
{
	m_pWebChannelObjects = new CWebChannelObjects(this);
	registerObject("kjpbs", m_pWebChannelObjects);
}

CWebChannelClient::~CWebChannelClient()
{

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

// ============================================================================

CWebChannelServer::CWebChannelServer(const QHostAddress &anAddress, quint16 nPort, QObject *pParent)
	:	QObject(pParent),
		m_server("King James Pure Bible Search WebChannel Server", QWebSocketServer::NonSecureMode),
		m_clientWrapper(&m_server)
{
	// setup the QWebSocketServer
	m_server.listen(anAddress, nPort);

	// create the CWebChannelAdminObjects
	m_pWebChannelAdminObjects = new CWebChannelAdminObjects(this);
	connect(m_pWebChannelAdminObjects.data(), SIGNAL(broadcast(const QString &)), this, SLOT(sendBroadcast(const QString &)));

	// Handle connections:
	connect(&m_clientWrapper, SIGNAL(clientConnected(WebSocketTransport*)), this, SLOT(en_clientConnected(WebSocketTransport*)));
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
#endif
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
#endif
	}
}

void CWebChannelServer::close()
{
	// Make server stop listening so we don't get any new connections (eliminate race condition):
	if (m_server.isListening()) m_server.close();

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
#endif
		itrClientMap.key()->socket()->close(QWebSocketProtocol::CloseCodeGoingAway, "Server shutting down");
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) delete pClientChannel;
	}
	m_mapChannels.clear();
}

void CWebChannelServer::sendBroadcast(const QString &strMessage)
{
#ifdef IS_CONSOLE_APP
	std::cout << QString("%1 UTC : Sending Broadcast Message : %2\n").arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).arg(strMessage).toUtf8().data();
#endif
	for (TWebChannelClientMap::iterator itrClientMap = m_mapChannels.begin(); itrClientMap != m_mapChannels.end(); ++itrClientMap) {
		QPointer<CWebChannelClient> pClientChannel = itrClientMap.value();
		if (!pClientChannel.isNull()) pClientChannel->sendBroadcast(strMessage);
	}
}

// ============================================================================
