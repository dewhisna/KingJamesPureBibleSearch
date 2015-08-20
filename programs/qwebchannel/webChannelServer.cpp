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
#include "websockettransport.h"
#include <QUrl>

CWebChannelServer::CWebChannelServer(const QHostAddress &anAddress, quint16 nPort, QObject *pParent)
	:	QObject(pParent),
		m_server("King James Pure Bible Search WebChannel Server", QWebSocketServer::NonSecureMode),
		m_clientWrapper(&m_server)
{
	// setup the QWebSocketServer
	m_server.listen(anAddress, nPort);

	// setup the channel
	connect(&m_clientWrapper, &WebSocketClientWrapper::clientConnected, &m_channel, &QWebChannel::connectTo);

}

CWebChannelServer::~CWebChannelServer()
{

}

void CWebChannelServer::registerObject(const QString &strID, QObject *pObject)
{
	m_channel.registerObject(strID, pObject);
}

void CWebChannelServer::deregisterObject(QObject *pObject)
{
	m_channel.deregisterObject(pObject);
}

QString CWebChannelServer::url(const QString &strBaseURLGood, const QString &strBaseURLBad) const
{
	QUrl aURL(m_server.isListening() ? strBaseURLGood : strBaseURLBad);
	if (isListening()) {
		aURL.setQuery(QStringLiteral("webChannelBaseUrl=") + m_server.serverUrl().toString());
	}
	return aURL.toString();
}
