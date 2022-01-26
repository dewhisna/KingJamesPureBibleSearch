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

#include "webChannelGeoLocate.h"
#include "mmdblookup.h"

#include "CSV.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <DelayedExecutionTimer.h>

#define DEBUG_WEBCHANNEL_GEOLOCATE_REQUESTS 0
#define DEBUG_WEBCHANNEL_MMDB_REQUESTS 0

#define INTERNAL_MMDB_RETRY_COUNT 3			// Set to the number of retries for the internal MMDB database (used to retry when database is busy during upgrade)
#define INTERNAL_MMDB_RETRY_DELAY 30000		// Set to the retry delay in milliseconds

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
	Q_ASSERT(pChannel != nullptr);

	TGeoLocateClient theClient;
	theClient.m_nLocateServer = GSE_NONE;
	theClient.m_pChannel = pChannel;
	theClient.m_strIPAddress = strIPAddress;
	theClient.m_nRetries = 0;		// Force loading the first time
	locateRequest(theClient);
}

void CWebChannelGeoLocate::locateRequest(TGeoLocateClient theClient)
{
	QString strURL;
	bool bIsRetry = false;

	do {
		if (theClient.m_nRetries == 0) {
			theClient.m_nLocateServer = static_cast<GEOLOCATE_SERVER_ENUM>(int(theClient.m_nLocateServer) + 1);
			if (int(theClient.m_nLocateServer) >= int(GSE_END_OF_LIST)) {
				emit locationInfo(theClient.m_pChannel, theClient.m_strIPAddress, QString("*** GeoLocateHosts Error: All GeoLocate servers have failed, out of GeoLocate servers to try"));
				return;
			}
		} else {
			--theClient.m_nRetries;
			bIsRetry = true;
		}

		switch (theClient.m_nLocateServer) {
			case GSE_INTERNAL:
#ifdef USING_MMDB
				strURL = "internal";
				if (!bIsRetry) theClient.m_nRetries = INTERNAL_MMDB_RETRY_COUNT;
#endif
				break;

			case GSE_TELIZE:
				// Note: The free-access version of telize.com is now offline indefinitely
				//strURL = QString("http://www.telize.com/geoip/%1").arg(theClient.m_strIPAddress);
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
	} while (strURL.isEmpty());

#if DEBUG_WEBCHANNEL_GEOLOCATE_REQUESTS
	qDebug("Sending GeoLocate Request to: %s", strURL.toUtf8().data());
#endif

	// Note: The internal lookup doesn't use a network session, but we can't just
	//		use NULL for it, because we would have a collision in our map from
	//		multiple lookups running on various threads.  Instead, we'll create
	//		a dummy QObject and force pass it as a QNetworkReply with an immediate
	//		call to en_requestComplete(), which will delete it.  This will keep
	//		every entry unique when running multiple threads:
	bool bIsInternal = (theClient.m_nLocateServer == GSE_INTERNAL);
	QNetworkReply *pReply = (!bIsInternal ? m_pNetManager->get(QNetworkRequest(QUrl(strURL))) : reinterpret_cast<QNetworkReply*>(new QObject(this)));
	m_mapChannels[pReply] = theClient;
	if (bIsInternal) {
		// Create a delayed trigger to launch the actual request.  For initial requests, this is
		//		set to a tiny 1ms delay, but retries will be by the specified retry delay:
		DelayedExecutionTimer *pTrigger = new DelayedExecutionTimer(-1, (bIsRetry ? INTERNAL_MMDB_RETRY_DELAY : 1), this);
		connect(pTrigger, SIGNAL(triggered(QObject*)), this, SLOT(triggerInternalRequest(QObject*)));
		connect(pTrigger, SIGNAL(triggered()), pTrigger, SLOT(deleteLater()));
		pTrigger->trigger(pReply);
	}
}

void CWebChannelGeoLocate::triggerInternalRequest(QObject *pInternal)
{
	en_requestComplete(reinterpret_cast<QNetworkReply*>(pInternal));
}

void CWebChannelGeoLocate::en_requestComplete(QNetworkReply *pReply)
{
	Q_ASSERT(pReply != nullptr);
	TGeoLocateClient theClient = m_mapChannels.value(pReply);
	m_mapChannels.remove(pReply);

	// If this is an internal lookup, pReply was used above to resolve our
	//		map.  But we need to delete and destroy our dummy QObject and
	//		clear pReply so as not to treat it as a QNetworkReply object:
	if (theClient.m_nLocateServer == GSE_INTERNAL) {
		reinterpret_cast<QObject *>(pReply)->deleteLater();
		pReply = nullptr;
	}
	// After this point, pReply will either be NULL or a real QNetworkReply object...

	if ((pReply) && (pReply->error() != QNetworkReply::NoError)) {
		// Handle error:
		emit locationInfo(theClient.m_pChannel, theClient.m_strIPAddress, QString("*** Network Error: %1").arg(pReply->errorString()));
		pReply->deleteLater();
		locateRequest(theClient);		// Try next host
		return;
	}

	QByteArray baData;

	if (theClient.m_nLocateServer != GSE_INTERNAL) {
		if (pReply) baData = pReply->readAll();
	} else {
		CMMDBLookup mmdb;
		QString strJSON;
		if (mmdb.lookup(strJSON, theClient.m_strIPAddress)) {
			baData = strJSON.toUtf8();
#if DEBUG_WEBCHANNEL_MMDB_REQUESTS
			qDebug("MMDB Data:\n%s", baData.data());
#endif
		} else {
			emit locationInfo(theClient.m_pChannel, theClient.m_strIPAddress, QString("*** Internal MMDB Error: %1").arg(mmdb.lastError()));
			if (pReply) pReply->deleteLater();		// pReply should be NULL, but include this for completeness/consistency
			locateRequest(theClient);				// Try next host
			return;
		}
	}

#if DEBUG_WEBCHANNEL_GEOLOCATE_REQUESTS
	qDebug("Received GeoLocate Data: %s", baData.data());
#endif

	QJsonParseError jsonError;
	QJsonDocument json = QJsonDocument::fromJson(baData, &jsonError);
	if (jsonError.error != QJsonParseError::NoError) {
		// Handle error:
		if (!baData.isEmpty()) {		// Report the JSON error only if we actually received data
			emit locationInfo(theClient.m_pChannel, theClient.m_strIPAddress, QString("*** JSON Error: %1").arg(jsonError.errorString()));
		}
		if (pReply) pReply->deleteLater();
		locateRequest(theClient);		// Try next host
		return;
	}

	QString strInformation = serverName(theClient) + " : " + jsonToCSV(json, theClient);
	emit locationInfo(theClient.m_pChannel, theClient.m_strIPAddress, strInformation);
	if (pReply) pReply->deleteLater();
}

// ----------------------------------------------------------------------------

QString CWebChannelGeoLocate::serverName(GEOLOCATE_SERVER_ENUM nServer)
{
	QString strServer;
	switch (nServer) {
		case GSE_INTERNAL:
			strServer = "internal";
			break;
		case GSE_TELIZE:
			strServer = "telize.com";
			break;
		case GSE_FREEGEOIP:
			strServer = "freegeoip.net";
			break;
		case GSE_NEKUDO:
			strServer = "nekudo.com";
			break;
		default:
			strServer = "unknown";
			break;
	}
	return strServer;
}

QString CWebChannelGeoLocate::jsonToCSV(const QJsonDocument &json, const TGeoLocateClient &theClient)
{
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
		case GSE_INTERNAL:
			//		{"city":{"geoname_id":4850751,"names":{"en":"Cedar Rapids","ja":"\u30b7\u30fc\u30c0\u30fc\u30e9\u30d4\u30c3\u30ba","ru":"\u0421\u0438\u0434\u0430\u0440-\u0420\u0430\u043f\u0438\u0434\u0441"}},
			//		"continent":{"code":"NA","geoname_id":6255149,"names":{"de":"Nordamerika","en":"North America","es":"Norteam\u00e9rica","fr":"Am\u00e9rique du Nord","ja":"\u5317\u30a2\u30e1\u30ea\u30ab","pt-BR":"Am\u00e9rica do Norte","ru":"\u0421\u0435\u0432\u0435\u0440\u043d\u0430\u044f \u0410\u043c\u0435\u0440\u0438\u043a\u0430","zh-CN":"\u5317\u7f8e\u6d32"}},
			//		"country":{"geoname_id":6252001,"iso_code":"US","names":{"de":"USA","en":"United States","es":"Estados Unidos","fr":"\u00c9tats-Unis","ja":"\u30a2\u30e1\u30ea\u30ab\u5408\u8846\u56fd","pt-BR":"Estados Unidos","ru":"\u0421\u0448\u0430","zh-CN":"\u7f8e\u56fd"}},
			//		"location":{"latitude":41.9731,"longitude":-91.5767,"metro_code":637,"time_zone":"America\/Chicago"},
			//		"postal":{"code":"52403"},
			//		"registered_country":{"geoname_id":6252001,"iso_code":"US","names":{"de":"USA","en":"United States","es":"Estados Unidos","fr":"\u00c9tats-Unis","ja":"\u30a2\u30e1\u30ea\u30ab\u5408\u8846\u56fd","pt-BR":"Estados Unidos","ru":"\u0421\u0448\u0430","zh-CN":"\u7f8e\u56fd"}},
			//		"subdivisions":[{"geoname_id":4862182,"iso_code":"IA","names":{"de":"Iowa","en":"Iowa","es":"Iowa","fr":"Iowa","ja":"\u30a2\u30a4\u30aa\u30ef\u5dde","pt-BR":"Iowa","ru":"\u0410\u0439\u043e\u0432\u0430","zh-CN":"\u827e\u5965\u74e6\u5dde"}}],
			//		"traits":{"ip_address":"12.34.56.78"}}

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

	QString strInformation;
	CCSVStream csv(&strInformation, QIODevice::WriteOnly);
	csv << strIP << strCountryCode << strCountry << strRegionCode << strRegion << strCity;
	csv << strPostalCode << strTimeZone << strLat << strLong << strMetroCode << strISP;
	// Not calling endLine here since we don't want newline characters (just raw string)

	return strInformation;
}

// ============================================================================
