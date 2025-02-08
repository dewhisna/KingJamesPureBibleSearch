/****************************************************************************
**
** Copyright (C) 2022-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef KML_H
#define KML_H

#include "XML.h"

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QGeoCoordinate>
#include <QList>
#include <QMap>
#include <QColor>
#include <QSharedPointer>

// Forward Declarations:
class QIODevice;
class QByteArray;

// ============================================================================

class CKmlContainer
{
public:
	const QString &name() const { return m_strName; }
	void setName(const QString &strName) { m_strName = strName; }

	const QString &description() const { return m_strDescription; }
	void setDescription(const QString &strDesc) { m_strDescription = strDesc; }

private:
	QString m_strName;
	QString m_strDescription;
};

// ============================================================================

typedef QList<QGeoCoordinate> TCoordinateList;

class CKmlPlacemark : public CKmlContainer
{
public:
	enum KML_PLACEMARK_TYPE_ENUM {
		KPTE_POINT = 0,
		KPTE_POLYGON = 1,
		KPTE_LINESTRING = 2,
	};

	KML_PLACEMARK_TYPE_ENUM type() const { return m_nType; }
	void setType(KML_PLACEMARK_TYPE_ENUM nType) { m_nType = nType; }

	const QUrl &urlStyle() const { return m_urlStyle; }
	void setUrlStyle(const QUrl &urlStyle) { m_urlStyle = urlStyle; }

	unsigned int tesselate() const { return m_nTesselate; }
	void setTesselate(unsigned int nTesselate) { m_nTesselate = nTesselate; }

	const TCoordinateList &coordinates() const { return m_lstCoordinates; }
	TCoordinateList &coordinates() { return m_lstCoordinates; }
	void setCoordinates(const TCoordinateList &lstCoordinates) { m_lstCoordinates = lstCoordinates; }

private:
	KML_PLACEMARK_TYPE_ENUM m_nType = KPTE_POINT;
	QUrl m_urlStyle;
	unsigned int m_nTesselate = 1;					// Only on Polygon and LineString
	TCoordinateList m_lstCoordinates;
};

typedef QList<CKmlPlacemark> TKmlPlacemarkList;

// ============================================================================

class CKmlBaseStyle
{
public:
	enum KML_STYLE_TYPE_ENUM {
		KSTE_ICON = 0,
		KSTE_LINE = 1,
		KSTE_POLY = 2,
	};

	virtual KML_STYLE_TYPE_ENUM type() const = 0;
};
typedef QSharedPointer<CKmlBaseStyle> CKmlBaseStylePtr;

// ----------------------------------------------------------------------------

class CKmlIconStyle : public CKmlBaseStyle
{
public:
	virtual KML_STYLE_TYPE_ENUM type() const override { return KSTE_ICON; }

	const QUrl &urlIcon() const { return m_urlIcon; }
	void setUrlIcon(const QUrl &urlIcon) { m_urlIcon = urlIcon; }

	double scale() const { return m_nScale; }
	void setScale(double nScale) { m_nScale = nScale; }

private:
	QUrl m_urlIcon;
	double m_nScale = 1.0;
};

// ----------------------------------------------------------------------------

class CKmlLineStyle : public CKmlBaseStyle
{
public:
	virtual KML_STYLE_TYPE_ENUM type() const override { return KSTE_LINE; }

	QColor color() const { return m_color; }
	void setColor(const QColor &color) { m_color = color; }

	unsigned int width() const { return m_nWidth; }
	void setWidth(unsigned int nWidth) { m_nWidth = nWidth; }

private:
	QColor m_color;
	unsigned int m_nWidth = 1;
};

// ----------------------------------------------------------------------------

class CKmlPolyStyle : public CKmlBaseStyle
{
public:
	virtual KML_STYLE_TYPE_ENUM type() const override { return KSTE_POLY; }

	QColor color() const { return m_color; }
	void setColor(const QColor &color) { m_color = color; }

	unsigned int fill() const { return m_nFill; }
	void setFill(unsigned int nFill) { m_nFill = nFill; }

	unsigned int outline() const { return m_nOutline; }
	void setOutline(unsigned int nOutline) { m_nOutline = nOutline; }

private:
	QColor m_color;
	unsigned int m_nFill = 1;
	unsigned int m_nOutline = 0;
};

// ----------------------------------------------------------------------------

// Map of CKmlBaseStyle types to create a single CKmlStyle by type attribute:
class CKmlStyle : public QMap<CKmlBaseStyle::KML_STYLE_TYPE_ENUM, CKmlBaseStylePtr>
{
public:
	const QString &id() const { return m_strID; }
	void setID(const QString &strID) { m_strID = strID; }

	CKmlIconStyle *iconStyle();
	const CKmlIconStyle *iconStyle() const;
	CKmlLineStyle *lineStyle();
	const CKmlLineStyle *lineStyle() const;
	CKmlPolyStyle *polyStyle();
	const CKmlPolyStyle *polyStyle() const;

private:
	QString m_strID;
};

typedef QMap<QString, CKmlStyle> TStyleMap;		// Map of Style ID to CKmlStyle

// ============================================================================

class CKmlFolder : public CKmlContainer
{
public:
	typedef QMap<QString, CKmlFolder> TFolderMap;		// Map of Folder Name to CKmlFolder

	TKmlPlacemarkList &placemarks() { return m_lstPlacemarks; }
	const TKmlPlacemarkList &placemarks() const { return m_lstPlacemarks; }

	TFolderMap &folders() { return m_mapFolders; }
	const TFolderMap &folders() const { return m_mapFolders; }

private:
	TKmlPlacemarkList m_lstPlacemarks;
	TFolderMap m_mapFolders;
};

// ============================================================================

class CKmlDocument : public CKmlFolder, protected CXmlDefaultHandler
{
public:
	bool read(QIODevice *device);
    bool read(const QByteArray &data);
    bool read(const QString &data);
    bool read(const char *data);

	const CXmlParseException &parseException() const { return m_xmlParseException; }

	TStyleMap &styles() { return m_mapStyles; }
	const TStyleMap &styles() const { return m_mapStyles; }

protected:
	CKmlFolder &currentFolder();

	// XML Overridates:
	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const CXmlAttributes &atts) override;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
	virtual bool characters(const QString &ch) override;
	virtual bool error(const CXmlParseException &exception) override;
	virtual QString errorString() const override
	{
		return (!m_strErrorString.isEmpty() ? m_strErrorString : CXmlDefaultHandler::errorString());
	}

private:
	TStyleMap m_mapStyles;
	// ----
	// Parsing Variables:
	QString m_strErrorString;
	CXmlParseException m_xmlParseException;		// Stored XML parse exception for when read() returns false
	//
	bool m_bInKml = false;
	bool m_bInDocument = false;
	int m_nInFolder = 0;				// Since Folders can be nested, this number will be the index for the current folder (0==Document)
	bool m_bInName = false;
	bool m_bInDescription = false;
	bool m_bInStyle = false;
	bool m_bInIconStyle = false;
	bool m_bInLineStyle = false;
	bool m_bInPolyStyle = false;
	bool m_bInScale = false;
	bool m_bInIcon = false;
	bool m_bInHref = false;
	bool m_bInColor = false;
	bool m_bInWidth = false;
	bool m_bInFill = false;
	bool m_bInOutline = false;
	bool m_bInTesselate = false;
	bool m_bInPlacemark = false;
	bool m_bInStyleUrl = false;
	int m_nInPolygon = 0;				// Non-zero is true, will count elements within: Polygon->outerBoundaryIs->LinearRing->Coordinates
	bool m_bInPoint = false;
	bool m_bInLineString = false;
	bool m_bInCoordinates = false;
	//
	QString m_strParseName;
	QString m_strParseDescription;
	QString m_strParseStyleID;
	QString m_strParseScale;
	QString m_strParseHref;				// Used for icon href and styleUrl
	QString m_strParseColor;
	QString m_strParseValue;			// Used for Width, Fill, Outline, and Tesselate
	QString m_strParseCoordinates;
};

// ============================================================================

#endif	// KML_H

