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

#ifndef WEBCHANNEL_GEOLOCATE_H
#define WEBCHANNEL_GEOLOCATE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonDocument>

// ============================================================================

// Forward declares:
class QNetworkAccessManager;
class QNetworkReply;
class CWebChannelClient;

// ============================================================================

class CWebChannelGeoLocate : public QObject
{
	Q_OBJECT

public:
	enum GEOLOCATE_SERVER_ENUM {
		GSE_NONE = -1,					// Placeholder for startup
		GSE_INTERNAL = 0,				// Internal MaxMind Database lookup
		GSE_TELIZE = 1,					// Use telize.com
		GSE_FREEGEOIP = 2,				// Use freegeoip.net
		GSE_NEKUDO = 3,					// Use nekudo.com
		GSE_END_OF_LIST = 4				// Size of GEOLOCATE_SERVER_ENUM
	};

	struct TGeoLocateClient {			// Client Baton
		TGeoLocateClient()
			:	m_pChannel(nullptr),
				m_nLocateServer(GSE_NONE),
				m_nRetries(0)
		{ }

		const CWebChannelClient *m_pChannel;
		QString m_strIPAddress;
		GEOLOCATE_SERVER_ENUM m_nLocateServer;
		int m_nRetries;					// Number of Retries Remaining (used for multiple retries on internal MMDB database which may be busy during database update)
	};

	static QString serverName(const TGeoLocateClient &theClient)
	{
		return serverName(theClient.m_nLocateServer);
	}
	static QString serverName(GEOLOCATE_SERVER_ENUM nServer);
	static QString jsonToCSV(const QJsonDocument &json, const TGeoLocateClient &theClient);

public:
	CWebChannelGeoLocate(QObject *pParent = nullptr);
	virtual ~CWebChannelGeoLocate();

public slots:
	void locate(const CWebChannelClient *pChannel, const QString &strIPAddress);

signals:
	void locationInfo(const CWebChannelClient *pChannel, const QString &strIPAddress, const QString &strLocationInfo);

protected slots:
	void locateRequest(CWebChannelGeoLocate::TGeoLocateClient theClient);
	void triggerInternalRequest(QObject *pInternal);
	void en_requestComplete(QNetworkReply *pReply);

protected:
	typedef QMap<QNetworkReply *, TGeoLocateClient> TNetworkReplyToChannelMap;

private:
	QNetworkAccessManager *m_pNetManager;
	TNetworkReplyToChannelMap m_mapChannels;
};

// ============================================================================

#endif	// WEBCHANNEL_GEOLOCATE_H
