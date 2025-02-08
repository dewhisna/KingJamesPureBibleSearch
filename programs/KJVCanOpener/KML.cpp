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

#include "KML.h"

// ============================================================================

CKmlIconStyle *CKmlStyle::iconStyle()
{
	if (this->operator[](CKmlBaseStyle::KSTE_ICON).isNull()) {
		this->operator[](CKmlBaseStyle::KSTE_ICON) = CKmlBaseStylePtr(new CKmlIconStyle());
	}
	return static_cast<CKmlIconStyle*>(this->operator[](CKmlBaseStyle::KSTE_ICON).data());
}

const CKmlIconStyle *CKmlStyle::iconStyle() const
{
	if (contains(CKmlBaseStyle::KSTE_ICON)) {
		return static_cast<const CKmlIconStyle*>(value(CKmlBaseStyle::KSTE_ICON).data());
	}
	return nullptr;
}

CKmlLineStyle *CKmlStyle::lineStyle()
{
	if (this->operator[](CKmlBaseStyle::KSTE_LINE).isNull()) {
		this->operator[](CKmlBaseStyle::KSTE_LINE) = CKmlBaseStylePtr(new CKmlLineStyle());
	}
	return static_cast<CKmlLineStyle*>(this->operator[](CKmlBaseStyle::KSTE_LINE).data());
}

const CKmlLineStyle *CKmlStyle::lineStyle() const
{
	if (contains(CKmlBaseStyle::KSTE_LINE)) {
		return static_cast<const CKmlLineStyle*>(value(CKmlBaseStyle::KSTE_LINE).data());
	}
	return nullptr;
}

CKmlPolyStyle *CKmlStyle::polyStyle()
{
	if (this->operator[](CKmlBaseStyle::KSTE_POLY).isNull()) {
		this->operator[](CKmlBaseStyle::KSTE_POLY) = CKmlBaseStylePtr(new CKmlPolyStyle());
	}
	return static_cast<CKmlPolyStyle*>(this->operator[](CKmlBaseStyle::KSTE_POLY).data());
}

const CKmlPolyStyle *CKmlStyle::polyStyle() const
{
	if (contains(CKmlBaseStyle::KSTE_POLY)) {
		return static_cast<const CKmlPolyStyle*>(value(CKmlBaseStyle::KSTE_POLY).data());
	}
	return nullptr;
}

// ============================================================================

bool CKmlDocument::read(QIODevice *device)
{
	CXmlReader xmlReader(device);
	xmlReader.setXmlHandler(this);
	return xmlReader.parse();
}

bool CKmlDocument::read(const QByteArray &data)
{
	CXmlReader xmlReader(data);
	xmlReader.setXmlHandler(this);
	return xmlReader.parse();
}

bool CKmlDocument::read(const QString &data)
{
	CXmlReader xmlReader(data);
	xmlReader.setXmlHandler(this);
	return xmlReader.parse();
}

bool CKmlDocument::read(const char *data)
{
	CXmlReader xmlReader(data);
	xmlReader.setXmlHandler(this);
	return xmlReader.parse();
}

// ============================================================================

bool CKmlDocument::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const CXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

	if (!m_bInKml) {
		if (localName.compare("kml", Qt::CaseInsensitive) == 0) {
			m_bInKml = true;
		}	// Ignore other top-level elements so file content can be concatenated
	} else if (!m_bInDocument) {
		if (localName.compare("Document", Qt::CaseInsensitive) == 0) {
			m_bInDocument = true;
		} else {
			m_strErrorString = QString("Unexpected element \"%1\"").arg(localName);
			return false;
		}
	} else if (m_bInName) {
		m_strErrorString = QString("Illegal/unknown nesting in 'name'");
		return false;
	} else if (m_bInDescription) {
		m_strErrorString = QString("Illegal/unknown nesting in 'description'");
		return false;
	} else if (m_bInScale) {
		m_strErrorString = QString("Illegal/unknown nesting in 'scale'");
		return false;
	} else if (m_bInHref) {
		m_strErrorString = QString("Illegal/unknown nesting in 'href'");
		return false;
	} else if (m_bInColor) {
		m_strErrorString = QString("Illegal/unknown nesting in 'color'");
		return false;
	} else if (m_bInWidth) {
		m_strErrorString = QString("Illegal/unknown nesting in 'width'");
		return false;
	} else if (m_bInFill) {
		m_strErrorString = QString("Illegal/unknown nesting in 'fill'");
		return false;
	} else if (m_bInOutline) {
		m_strErrorString = QString("Illegal/unknown nesting in 'outline'");
		return false;
	} else if (m_bInTesselate) {
		m_strErrorString = QString("Illegal/unknown nesting in 'tessalate'");
		return false;
	} else if (m_bInStyleUrl) {
		m_strErrorString = QString("Illegal/unknown nesting in 'styleUrl'");
		return false;
	} else if (m_bInCoordinates) {
		m_strErrorString = QString("Illegal/unknown nesting in 'coordinates'");
		return false;
	} else if (m_bInStyle) {
		if (m_bInIconStyle) {
			if (localName.compare("scale", Qt::CaseInsensitive) == 0) {
				m_bInScale = true;
				m_strParseScale.clear();
			} else if (!m_bInIcon && (localName.compare("icon", Qt::CaseInsensitive) == 0)) {
				m_bInIcon = true;
			} else if (m_bInIcon && (localName.compare("href", Qt::CaseInsensitive) == 0)) {
				m_bInHref = true;
				m_strParseHref.clear();
			} else {
				m_strErrorString = QString("Unknown IconStyle Property: \"%1\"").arg(localName);
				return false;
			}
		} else if (m_bInLineStyle) {
			if (localName.compare("color", Qt::CaseInsensitive) == 0) {
				m_bInColor = true;
				m_strParseColor.clear();
			} else if (localName.compare("width", Qt::CaseInsensitive) == 0) {
				m_bInWidth = true;
				m_strParseValue.clear();
			} else {
				m_strErrorString = QString("Unknown LineStyle Property: \"%1\"").arg(localName);
				return false;
			}
		} else if (m_bInPolyStyle) {
			if (localName.compare("color", Qt::CaseInsensitive) == 0) {
				m_bInColor = true;
				m_strParseColor.clear();
			} else if (localName.compare("fill", Qt::CaseInsensitive) == 0) {
				m_bInFill = true;
				m_strParseValue.clear();
			} else if (localName.compare("outline", Qt::CaseInsensitive) == 0) {
				m_bInOutline = true;
				m_strParseValue.clear();
			} else {
				m_strErrorString = QString("Unknown PolyStyle Property: \"%1\"").arg(localName);
				return false;
			}
		} else if (localName.compare("IconStyle", Qt::CaseInsensitive) == 0) {
			Q_ASSERT(!m_strParseStyleID.isEmpty());
			m_bInIconStyle = true;
			if (m_mapStyles[m_strParseStyleID].iconStyle() == nullptr) {		// Create the iconStyle attribute on this style
				Q_ASSERT(false);
			}
		} else if (localName.compare("LineStyle", Qt::CaseInsensitive) == 0) {
			Q_ASSERT(!m_strParseStyleID.isEmpty());
			m_bInLineStyle = true;
			if (m_mapStyles[m_strParseStyleID].lineStyle() == nullptr) {		// Create the lineStyle attribute on this style
				Q_ASSERT(false);
			}
		} else if (localName.compare("PolyStyle", Qt::CaseInsensitive) == 0) {
			Q_ASSERT(!m_strParseStyleID.isEmpty());
			m_bInPolyStyle = true;
			if (m_mapStyles[m_strParseStyleID].polyStyle() == nullptr) {		// Create the polyStyle attribute on this style
				Q_ASSERT(false);
			}
		} else {
			m_strErrorString = QString("Unknown Style Type \"%1\"").arg(localName);
			return false;
		}
	} else if (m_bInPlacemark) {
		// Check elements inside Placemark:
		if (localName.compare("name", Qt::CaseInsensitive) == 0) {					// Note: Placemark only
			m_bInName = true;
			m_strParseName.clear();
		} else if (localName.compare("description", Qt::CaseInsensitive) == 0) {	// Note: Placemark only
			m_bInDescription = true;
			m_strParseDescription.clear();
		} else if (localName.compare("styleUrl", Qt::CaseInsensitive) == 0) {
			m_bInStyleUrl = true;
			m_strParseHref.clear();
		} else if (m_bInPoint) {
			if (localName.compare("coordinates", Qt::CaseInsensitive) == 0) {
				m_bInCoordinates = true;
				m_strParseCoordinates.clear();
			} else {
				m_strErrorString = QString("Unexpected Element in Point Placemark: \"%1\"").arg(localName);
				return false;
			}
		} else if (m_nInPolygon) {
			if (m_nInPolygon == 1) {
				if (localName.compare("tesselate", Qt::CaseInsensitive) == 0) {
					m_bInTesselate = true;
					m_strParseValue.clear();
				} else if (localName.compare("outerBoundaryIs", Qt::CaseInsensitive) == 0) {
					++m_nInPolygon;
				} else {
					m_strErrorString = QString("Unexpected Element in Polygon Placemark: \"%1\"").arg(localName);
					return false;
				}
			} else if (m_nInPolygon == 2) {
				if (localName.compare("LinearRing", Qt::CaseInsensitive) == 0) {
					++m_nInPolygon;
				} else {
					m_strErrorString = QString("Unexpected Element in outerBoundaryIs Polygon Placemark: \"%1\"").arg(localName);
					return false;
				}
			} else if (m_nInPolygon == 3) {
				if (localName.compare("coordinates", Qt::CaseInsensitive) == 0) {
					m_bInCoordinates = true;
					m_strParseCoordinates.clear();
				} else {
					m_strErrorString = QString("Unexpected Element in LinearRing outerBoundaryIs Polygon Placemark: \"%1\"").arg(localName);
					return false;
				}
			} else {
				Q_ASSERT(false);
			}
		} else if (m_bInLineString) {
			if (localName.compare("tesselate", Qt::CaseInsensitive) == 0) {
				m_bInTesselate = true;
				m_strParseValue.clear();
			} else if (localName.compare("coordinates", Qt::CaseInsensitive) == 0) {
				m_bInCoordinates = true;
				m_strParseCoordinates.clear();
			} else {
				m_strErrorString = QString("Unexpected Element in LineString Placemark: \"%1\"").arg(localName);
				return false;
			}
		} else if (localName.compare("Point", Qt::CaseInsensitive) == 0) {
			Q_ASSERT(!currentFolder().placemarks().isEmpty());
			m_bInPoint = true;
			currentFolder().placemarks().back().setType(CKmlPlacemark::KPTE_POINT);
		} else if (localName.compare("Polygon", Qt::CaseInsensitive) == 0) {
			Q_ASSERT(!currentFolder().placemarks().isEmpty());
			++m_nInPolygon;
			currentFolder().placemarks().back().setType(CKmlPlacemark::KPTE_POLYGON);
		} else if (localName.compare("LineString", Qt::CaseInsensitive) == 0) {
			Q_ASSERT(!currentFolder().placemarks().isEmpty());
			m_bInLineString = true;
			currentFolder().placemarks().back().setType(CKmlPlacemark::KPTE_LINESTRING);
		} else {
			m_strErrorString = QString("Unknown Placemark Type or Property: \"%1\"").arg(localName);
			return false;
		}
	} else if (localName.compare("Placemark", Qt::CaseInsensitive) == 0) {
		m_bInPlacemark = true;
		currentFolder().placemarks().push_back(CKmlPlacemark());	// Add new placemark to our current folder's list
	} else if (localName.compare("Folder", Qt::CaseInsensitive) == 0) {			// Check Nested Folders (Note: Document itself is a folder)
		// Since the name is an element instead of an attribute, we don't
		//	know the folder name (i.e. map key) yet, so put it under
		//	and empty name and then swap it after we resolve the name:
		currentFolder().folders().insert(QString(), CKmlFolder());
		++m_nInFolder;
	} else if (localName.compare("name", Qt::CaseInsensitive) == 0) {			// Note: Document or Folder
		m_bInName = true;
		m_strParseName.clear();
	} else if (localName.compare("description", Qt::CaseInsensitive) == 0) {	// Note: Document or Folder
		m_bInDescription = true;
		m_strParseDescription.clear();
	} else if ((m_nInFolder == 0) && (localName.compare("Style", Qt::CaseInsensitive) == 0)) {	// Note: Style is on Document only (not subfolders)
		m_bInStyle = true;
		int ndxID = atts.index("id", Qt::CaseInsensitive);
		if (ndxID == -1) {
			m_strErrorString = QString("Missing 'id' attribute on 'Style' element");
			return false;
		} else {
			m_strParseStyleID = atts.value(ndxID);
		}
		if (m_strParseStyleID.isEmpty()) {
			m_strErrorString = QString("Invalid 'id' attribute on 'Style' element: \"%1\"").arg(m_strParseStyleID);
			return false;
		}
		if (m_mapStyles.contains(m_strParseStyleID)) {
			m_strErrorString = QString("Duplicate Style id: \"%1\"").arg(m_strParseStyleID);
			return false;
		}
		CKmlStyle aStyle;
		aStyle.setID(m_strParseStyleID);
		m_mapStyles[m_strParseStyleID] = aStyle;
	} else {
		m_strErrorString = QString("Unexpected Element in Document/Folder: \"%1\"").arg(localName);
		return false;
	}

	return true;
}

bool CKmlDocument::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

	if (localName.compare("name", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInName);
		if (m_strParseName.trimmed().isEmpty()) {
			if (m_bInPlacemark) {
				m_strErrorString = "Invalid Empty Placemark Name";
			} else {
				m_strErrorString = "Invalid Empty Document/Folder Name";
			}
			return false;
		}
		if (m_bInPlacemark) {
			Q_ASSERT(!currentFolder().placemarks().isEmpty());
			currentFolder().placemarks().back().setName(m_strParseName.trimmed());
		} else {
			// Folders and Document:
			currentFolder().setName(m_strParseName.trimmed());
		}
		m_strParseName.clear();
		m_bInName = false;
	} else if (localName.compare("description", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInDescription);
		if (m_bInPlacemark) {
			Q_ASSERT(!currentFolder().placemarks().isEmpty());
			currentFolder().placemarks().back().setDescription(m_strParseDescription.trimmed());
		} else {
			currentFolder().setDescription(m_strParseDescription.trimmed());
		}
		m_strParseDescription.clear();
		m_bInDescription = false;
	} else if (localName.compare("scale", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInScale);
		Q_ASSERT(!m_strParseStyleID.isEmpty());
		Q_ASSERT(m_bInIconStyle);
		m_mapStyles[m_strParseStyleID].iconStyle()->setScale(m_strParseScale.trimmed().toDouble());
		m_strParseScale.clear();
		m_bInScale = false;
	} else if (localName.compare("href", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInHref);
		Q_ASSERT(!m_strParseStyleID.isEmpty());
		Q_ASSERT(m_bInIconStyle);
		m_mapStyles[m_strParseStyleID].iconStyle()->setUrlIcon(QUrl(m_strParseHref.trimmed()));
		m_strParseHref.clear();
		m_bInHref = false;
	} else if (localName.compare("color", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInColor);
		Q_ASSERT(!m_strParseStyleID.isEmpty());
		Q_ASSERT(m_bInLineStyle || m_bInPolyStyle);
		if (m_bInLineStyle) {
			m_mapStyles[m_strParseStyleID].lineStyle()->setColor(QColor("#" + m_strParseColor.trimmed()));
		} else if (m_bInPolyStyle) {
			m_mapStyles[m_strParseStyleID].polyStyle()->setColor(QColor("#" + m_strParseColor.trimmed()));
		}
		m_strParseColor.clear();
		m_bInColor = false;
	} else if (localName.compare("width", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInWidth);
		Q_ASSERT(!m_strParseStyleID.isEmpty());
		Q_ASSERT(m_bInLineStyle);
		m_mapStyles[m_strParseStyleID].lineStyle()->setWidth(m_strParseValue.trimmed().toUInt());
		m_strParseValue.clear();
		m_bInWidth = false;
	} else if (localName.compare("fill", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInFill);
		Q_ASSERT(!m_strParseStyleID.isEmpty());
		Q_ASSERT(m_bInPolyStyle);
		m_mapStyles[m_strParseStyleID].polyStyle()->setFill(m_strParseValue.trimmed().toUInt());
		m_strParseValue.clear();
		m_bInFill = false;
	} else if (localName.compare("outline", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInOutline);
		Q_ASSERT(!m_strParseStyleID.isEmpty());
		Q_ASSERT(m_bInPolyStyle);
		m_mapStyles[m_strParseStyleID].polyStyle()->setOutline(m_strParseValue.trimmed().toUInt());
		m_strParseValue.clear();
		m_bInOutline = false;
	} else if (localName.compare("tesselate", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInTesselate);
		Q_ASSERT(!currentFolder().placemarks().isEmpty());
		Q_ASSERT(m_bInPlacemark);
		Q_ASSERT(m_nInPolygon || m_bInLineString);
		currentFolder().placemarks().back().setTesselate(m_strParseValue.trimmed().toUInt());
		m_strParseValue.clear();
		m_bInTesselate = false;
	} else if (localName.compare("styleUrl", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInStyleUrl);
		Q_ASSERT(!currentFolder().placemarks().isEmpty());
		Q_ASSERT(m_bInPlacemark);
		currentFolder().placemarks().back().setUrlStyle(QUrl(m_strParseHref.trimmed()));
		m_strParseHref.clear();
		m_bInStyleUrl = false;
	} else if (localName.compare("coordinates", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInCoordinates);
		Q_ASSERT(!currentFolder().placemarks().isEmpty());
		Q_ASSERT(m_bInPlacemark);

		QStringList lstCoords = m_strParseCoordinates.split(' ', Qt::SkipEmptyParts);
		for (auto const &strCoord : lstCoords) {
			Q_ASSERT(!strCoord.isEmpty());
			QStringList lstLatLong = strCoord.split(',', Qt::SkipEmptyParts);
			if (lstLatLong.size() != 2) {
				m_strErrorString = QString("Invalid Coordinates: %1").arg(strCoord);
				return false;
			}
			currentFolder().placemarks().back().coordinates().append(QGeoCoordinate(lstLatLong.at(0).trimmed().toDouble(),
																					lstLatLong.at(1).trimmed().toDouble()));
		}
		m_strParseCoordinates.clear();
		m_bInCoordinates = false;
		// -------------------------------------------------------------------- End of individual value elements
	} else if (localName.compare("icon", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInIcon);
		m_bInIcon = false;
	} else if (localName.compare("IconStyle", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInIconStyle);
		m_bInIconStyle = false;
	} else if (localName.compare("LineStyle", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInLineStyle);
		m_bInLineStyle = false;
	} else if (localName.compare("PolyStyle", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInPolyStyle);
		m_bInPolyStyle = false;
	} else if (localName.compare("Style", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInStyle);
		m_bInStyle = false;
	} else if (localName.compare("Point", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInPoint);
		m_bInPoint = false;
	} else if (localName.compare("Polygon", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_nInPolygon == 1);
		--m_nInPolygon;
	} else if (localName.compare("outerBoundaryIs", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_nInPolygon == 2);
		--m_nInPolygon;
	} else if (localName.compare("LinearRing", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_nInPolygon == 3);
		--m_nInPolygon;
	} else if (localName.compare("LineString", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInLineString);
		m_bInLineString = false;
	} else if (localName.compare("Placemark", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInPlacemark);
		m_bInPlacemark = false;
	} else if (localName.compare("Folder", Qt::CaseInsensitive) == 0) {			// Check Nested Folders (Note: Document itself is a folder)
		Q_ASSERT(m_nInFolder > 0);
		// The new folder was originally inserted with an empty name.
		//	But it should now have a name from its 'name' element.  Now,
		//	we will swap them before popping the folder:
		CKmlFolder &realFolder = currentFolder();		// This is the real folder with an empty name
		--m_nInFolder;
		TFolderMap::iterator itrNoName = currentFolder().folders().find(QString());	// Iterator to folder with empty name
		Q_ASSERT(itrNoName != currentFolder().folders().end());
		Q_ASSERT(&itrNoName.value() == &realFolder);
		currentFolder().folders().insert(realFolder.name(), itrNoName.value());		// Insert the folder content with the real name
		currentFolder().folders().remove(QString());	// Remove the extra copy with the empty name
	} else if (localName.compare("Document", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInDocument);
		m_bInDocument = false;
	} else if (localName.compare("kml", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(m_bInKml);
		m_bInKml = false;
	}

	return true;
}

bool CKmlDocument::characters(const QString &ch)
{
	if (m_bInName) {
		m_strParseName += ch;
	} else if (m_bInDescription) {
		m_strParseDescription += ch;
	} else if (m_bInScale) {
		m_strParseScale += ch;
	} else if (m_bInHref || m_bInStyleUrl) {
		m_strParseHref += ch;
	} else if (m_bInColor) {
		m_strParseColor += ch;
	} else if (m_bInWidth || m_bInFill || m_bInOutline || m_bInTesselate) {
		m_strParseValue += ch;
	} else if (m_bInCoordinates) {
		m_strParseCoordinates += ch;
	}

	return true;
}

bool CKmlDocument::error(const CXmlParseException &exception)
{
	m_xmlParseException = exception;
	return true;
}

// ----------------------------------------------------------------------------

CKmlFolder &CKmlDocument::currentFolder()
{
	CKmlFolder *pFolder = this;
	for (int ndx = 0; ndx < m_nInFolder; ++ndx) {
		TFolderMap::iterator itrFolder = pFolder->folders().find(QString());
		Q_ASSERT(itrFolder != folders().end());
		pFolder = &itrFolder.value();
	}
	return *pFolder;
}

// ============================================================================

