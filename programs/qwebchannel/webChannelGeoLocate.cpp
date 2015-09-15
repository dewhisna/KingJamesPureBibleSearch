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

#include "webChannelGeoLocate.h"

#include "CSV.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <assert.h>

#define DEBUG_WEBCHANNEL_GEOLOCATE_REQUESTS 0

// ============================================================================

CWebChannelGeoLocate::CWebChannelGeoLocate(QObject *pParent)
	:	QObject(pParent)
{
	m_pNetManager = new QNetworkAccessManager(this);
	connect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(en_requestComplete(QNetworkReply*)));
}

CWebChannelGeoLocate::~CWebChannelGeoLocate()
{

}

void CWebChannelGeoLocate::locate(const CWebChannelClient *pChannel, const QString &strIPAddress)
{
	assert(pChannel != NULL);

	TGeoLocateClient theClient;
	theClient.m_nLocateServer = GSE_NONE;
	theClient.m_pChannel = pChannel;
	theClient.m_strIPAddress = strIPAddress;
	locateRequest(theClient);
}

void CWebChannelGeoLocate::locateRequest(TGeoLocateClient theClient)
{
	theClient.m_nLocateServer = static_cast<GEOLOCATE_SERVER_ENUM>(int(theClient.m_nLocateServer) + 1);
	if (int(theClient.m_nLocateServer) >= int(GSE_END_OF_LIST)) {
		emit locationInfo(theClient.m_pChannel, QString("*** GeoLocateHosts Error: All GeoLocate servers have failed, out of GeoLocate servers to try"));
		return;
	}

	QString strURL;

	switch (theClient.m_nLocateServer) {
		case GSE_TELIZE:
			strURL = QString("http://www.telize.com/geoip/%1").arg(theClient.m_strIPAddress);
			break;

		case GSE_FREEGEOIP:
			strURL = QString("https://freegeoip.net/json/%1").arg(theClient.m_strIPAddress);
			break;

		case GSE_NEKUDO:
			strURL = QString("http://geoip.nekudo.com/api/%1/full").arg(theClient.m_strIPAddress);
			break;

		default:
			break;
	}

	if (strURL.isEmpty()) {
		assert(false);			// shouldn't happen unless our sequential host trying mechanism is broken
		return;
	}

#if DEBUG_WEBCHANNEL_GEOLOCATE_REQUESTS
	qDebug("Sending GeoLocate Request to: %s", strURL.toUtf8().data());
#endif

	QNetworkReply *pReply = m_pNetManager->get(QNetworkRequest(QUrl(strURL)));
	m_mapChannels[pReply] = theClient;
}

void CWebChannelGeoLocate::en_requestComplete(QNetworkReply *pReply)
{
	assert(pReply != NULL);
	TGeoLocateClient theClient = m_mapChannels.value(pReply);
	m_mapChannels.remove(pReply);

	if (pReply->error() != QNetworkReply::NoError) {
		// Handle error:
		emit locationInfo(theClient.m_pChannel, QString("*** Network Error: %1").arg(pReply->errorString()));
		pReply->deleteLater();
		locateRequest(theClient);		// Try next host
		return;
	}

	QString strInformation;
	QByteArray baData = pReply->readAll();

#if DEBUG_WEBCHANNEL_GEOLOCATE_REQUESTS
	qDebug("Received GeoLocate Data: %s", baData.data());
#endif

	QJsonParseError jsonError;
	QJsonDocument json = QJsonDocument::fromJson(baData, &jsonError);
	if (jsonError.error != QJsonParseError::NoError) {
		// Handle error:
		emit locationInfo(theClient.m_pChannel, QString("*** JSON Error: %1").arg(jsonError.errorString()));
		pReply->deleteLater();
		locateRequest(theClient);		// Try next host
		return;
	}

	QString strServer;
	QString strIP;
	QString strCountryCode;
	QString strCountry;
	QString strRegionCode;
	QString strRegion;
	QString strCity;
	QString strPostalCode;
	QString strTimeZone;
	QString strLat;
	QString strLong;
	QString strMetroCode;
	QString strISP;

	QJsonObject objJson = json.object();
	switch (theClient.m_nLocateServer) {
		case GSE_TELIZE:
			//		{"dma_code":"0",
			//		"ip":"12.34.56.78",
			//		"asn":"AS30036",
			//		"city":"Cedar Rapids",
			//		"latitude":41.9731,
			//		"country_code":"US",
			//		"offset":"-5",
			//		"country":"United States",
			//		"region_code":"IA",
			//		"isp":"Overpriced Internet Inc.",
			//		"timezone":"America\/Chicago",
			//		"area_code":"0",
			//		"continent_code":"NA",
			//		"longitude":-91.5767,
			//		"region":"Iowa",
			//		"postal_code":"52403",
			//		"country_code3":"USA"}

			strServer = "telize.com";
			strIP = objJson.value("ip").toString();
			strCountryCode = objJson.value("country_code").toString();
			strCountry = objJson.value("country").toString();
			strRegionCode = objJson.value("region_code").toString();
			strRegion = objJson.value("region").toString();
			strCity = objJson.value("city").toString();
			strPostalCode = objJson.value("postal_code").toString();
			strTimeZone = objJson.value("timezone").toString();
			strLat = objJson.value("latitude").toVariant().toString();
			strLong = objJson.value("longitude").toVariant().toString();
			strMetroCode = objJson.value("area_code").toString();		// ?? Not 100% certain this is metro code
			strISP = objJson.value("isp").toString();
			break;

		case GSE_FREEGEOIP:
			//		{"ip":"12.34.56.78",
			//		"country_code":"US",
			//		"country_name":"United States",
			//		"region_code":"IA",
			//		"region_name":"Iowa",
			//		"city":"Cedar Rapids",
			//		"zip_code":"52403",
			//		"time_zone":"America/Chicago",
			//		"latitude":41.973,
			//		"longitude":-91.577,
			//		"metro_code":637}

			strServer = "freegeoip.net";
			strIP = objJson.value("ip").toString();
			strCountryCode = objJson.value("country_code").toString();
			strCountry = objJson.value("country_name").toString();
			strRegionCode = objJson.value("region_code").toString();
			strRegion = objJson.value("region_name").toString();
			strCity = objJson.value("city").toString();
			strPostalCode = objJson.value("zip_code").toString();
			strTimeZone = objJson.value("time_zone").toString();
			strLat = objJson.value("latitude").toVariant().toString();
			strLong = objJson.value("longitude").toVariant().toString();
			strMetroCode = objJson.value("metro_code").toVariant().toString();
			strISP = QString();					// No ISP string
			break;

		case GSE_NEKUDO:
			//		{"city":{"geoname_id":4850751,"names":{"en":"Cedar Rapids","ja":"\u30b7\u30fc\u30c0\u30fc\u30e9\u30d4\u30c3\u30ba","ru":"\u0421\u0438\u0434\u0430\u0440-\u0420\u0430\u043f\u0438\u0434\u0441"}},
			//		"continent":{"code":"NA","geoname_id":6255149,"names":{"de":"Nordamerika","en":"North America","es":"Norteam\u00e9rica","fr":"Am\u00e9rique du Nord","ja":"\u5317\u30a2\u30e1\u30ea\u30ab","pt-BR":"Am\u00e9rica do Norte","ru":"\u0421\u0435\u0432\u0435\u0440\u043d\u0430\u044f \u0410\u043c\u0435\u0440\u0438\u043a\u0430","zh-CN":"\u5317\u7f8e\u6d32"}},
			//		"country":{"geoname_id":6252001,"iso_code":"US","names":{"de":"USA","en":"United States","es":"Estados Unidos","fr":"\u00c9tats-Unis","ja":"\u30a2\u30e1\u30ea\u30ab\u5408\u8846\u56fd","pt-BR":"Estados Unidos","ru":"\u0421\u0448\u0430","zh-CN":"\u7f8e\u56fd"}},
			//		"location":{"latitude":41.9731,"longitude":-91.5767,"metro_code":637,"time_zone":"America\/Chicago"},
			//		"postal":{"code":"52403"},
			//		"registered_country":{"geoname_id":6252001,"iso_code":"US","names":{"de":"USA","en":"United States","es":"Estados Unidos","fr":"\u00c9tats-Unis","ja":"\u30a2\u30e1\u30ea\u30ab\u5408\u8846\u56fd","pt-BR":"Estados Unidos","ru":"\u0421\u0448\u0430","zh-CN":"\u7f8e\u56fd"}},
			//		"subdivisions":[{"geoname_id":4862182,"iso_code":"IA","names":{"de":"Iowa","en":"Iowa","es":"Iowa","fr":"Iowa","ja":"\u30a2\u30a4\u30aa\u30ef\u5dde","pt-BR":"Iowa","ru":"\u0410\u0439\u043e\u0432\u0430","zh-CN":"\u827e\u5965\u74e6\u5dde"}}],
			//		"traits":{"ip_address":"12.34.56.78"}}

			strServer = "nekudo.com";
			strIP = objJson.value("traits").toObject().value("ip_address").toString();
			strCountryCode = objJson.value("country").toObject().value("iso_code").toString();
			if (strCountryCode.isEmpty()) strCountryCode = objJson.value("registered_country").toObject().value("iso_code").toString();
			strCountry = objJson.value("country").toObject().value("names").toObject().value("en").toString();
			if (strCountry.isEmpty()) strCountry = objJson.value("registered_country").toObject().value("names").toObject().value("en").toString();
			if (objJson.value("subdivisions").toArray().size()) {
				strRegionCode = objJson.value("subdivisions").toArray().first().toObject().value("iso_code").toString();
				strRegion = objJson.value("subdivisions").toArray().first().toObject().value("names").toObject().value("en").toString();
			}
			strCity = objJson.value("city").toObject().value("names").toObject().value("en").toString();
			strPostalCode = objJson.value("postal").toObject().value("code").toString();
			strTimeZone = objJson.value("location").toObject().value("time_zone").toString();
			strLat = objJson.value("location").toObject().value("latitude").toVariant().toString();
			strLong = objJson.value("location").toObject().value("longitude").toVariant().toString();
			strMetroCode = objJson.value("location").toObject().value("metro_code").toVariant().toString();
			strISP = QString();					// No ISP string
			break;

		default:
			break;
	}

	if (strIP.isEmpty()) strIP = theClient.m_strIPAddress;

	CCSVStream csv(&strInformation, QIODevice::WriteOnly);
	csv << strIP << strCountryCode << strCountry << strRegionCode << strRegion << strCity;
	csv << strPostalCode << strTimeZone << strLat << strLong << strMetroCode << strISP;
	// Not caling endLine here since we don't want newline characters (just raw string)

	strInformation = strServer + " : " + strInformation;

	emit locationInfo(theClient.m_pChannel, strInformation);
	pReply->deleteLater();
}

// ============================================================================
