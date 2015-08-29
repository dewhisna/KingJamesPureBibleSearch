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

#ifndef WEBCHANNEL_SERVER_H
#define WEBCHANNEL_SERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebChannel>
#include <QMap>
#include <QPointer>
#include "websocketclientwrapper.h"

// Forward declarations:
class CWebChannelObjects;
class CWebChannelAdminObjects;

// ============================================================================

//
// CWebChannelClient
//
class CWebChannelClient : public QObject
{
	Q_OBJECT

public:
	CWebChannelClient(QObject *pParent = NULL);
	virtual ~CWebChannelClient();

public slots:
	void registerObject(const QString &strID, QObject *pObject);
	void deregisterObject(QObject *pObject);

	void sendBroadcast(const QString &strMessage);

protected:
	friend class CWebChannelServer;

	void connectTo(WebSocketTransport* pClient);

private:
	QWebChannel m_channel;
	QPointer<CWebChannelObjects> m_pWebChannelObjects;
};

typedef QMap<WebSocketTransport *, QPointer<CWebChannelClient> > TWebChannelClientMap;

// ============================================================================

//
// CWebChannelServer
//
class CWebChannelServer : public QObject
{
	Q_OBJECT

public:
	CWebChannelServer(const QHostAddress &anAddress = QHostAddress::Any, quint16 nPort = 0, QObject *pParent = NULL);
	virtual ~CWebChannelServer();

	bool isListening() const { return m_server.isListening(); }
	QString url(const QString &strBaseURLGood, const QString &strBaseURLBad) const;		// The Good URL is used if we are listening for connections, the Bad URL is used if not (to direct user to the correct target if our server fails to start)

	QHostAddress serverAddress() const { return m_server.serverAddress(); }
	quint16 serverPort() const { return m_server.serverPort(); }

protected:
	friend class CWebChannelAdminObjects;

	const TWebChannelClientMap &channelMap() const { return m_mapChannels; }

public slots:
	void close();										// Hangup all clients and shutdown server
	void sendBroadcast(const QString &strMessage);		// Broadcast message to all connected clients

private slots:
	void en_clientConnected(WebSocketTransport* pClient);
	void en_clientDisconnected(WebSocketTransport* pClient);

protected:
	QWebSocketServer m_server;						// Server to host i/o
	WebSocketClientWrapper m_clientWrapper;			// wrap WebSocket clients in QWebChannelAbstractTransport objects
	TWebChannelClientMap m_mapChannels;				// Channels for exposing QObjects to HTML to connected clients
	QPointer<CWebChannelAdminObjects> m_pWebChannelAdminObjects;		// Admin objects attached to all clients to do "hidden" messaging
};

// ============================================================================

#endif	// WEBCHANNEL_SERVER_H
