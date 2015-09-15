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

#ifndef WEBCHANNEL_GEOLOCATE_H
#define WEBCHANNEL_GEOLOCATE_H

#include <QObject>
#include <QString>
#include <QMap>

// ============================================================================

// Forward declares:
class QNetworkAccessManager;
class QNetworkReply;
class CWebChannelClient;

// ============================================================================

class CWebChannelGeoLocate : public QObject
{
	Q_OBJECT

protected:
	enum GEOLOCATE_SERVER_ENUM {
		GSE_NONE = -1,					// Placeholder for startup
		GSE_TELIZE = 0,					// Use telize.com
		GSE_FREEGEOIP = 1,				// Use freegeoip.net
		GSE_NEKUDO = 2,					// Use nekudo.com
		GSE_END_OF_LIST = 3				// Size of GEOLOCATE_SERVER_ENUM
	};

	struct TGeoLocateClient {			// Client Baton
		TGeoLocateClient()
			:	m_pChannel(NULL),
				m_nLocateServer(GSE_NONE)
		{ }

		const CWebChannelClient *m_pChannel;
		QString m_strIPAddress;
		GEOLOCATE_SERVER_ENUM m_nLocateServer;
	};

	typedef QMap<QNetworkReply *, TGeoLocateClient> TNetworkReplyToChannelMap;

public:
	CWebChannelGeoLocate(QObject *pParent = NULL);
	virtual ~CWebChannelGeoLocate();

public slots:
	void locate(const CWebChannelClient *pChannel, const QString &strIPAddress);

signals:
	void locationInfo(const CWebChannelClient *pChannel, const QString &strLocationInfo);

protected slots:
	void locateRequest(TGeoLocateClient theClient);
	void en_requestComplete(QNetworkReply *pReply);

private:
	QNetworkAccessManager *m_pNetManager;
	TNetworkReplyToChannelMap m_mapChannels;
};

// ============================================================================

#endif	// WEBCHANNEL_GEOLOCATE_H