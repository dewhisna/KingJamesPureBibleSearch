/****************************************************************************
**
** Copyright (C) 2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "../KJVCanOpener/KML.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

#include <iostream>
#include <functional>

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	int nArgsFound = 0;
	bool bUnknownOption = false;
	QString strKmlPath;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				strKmlPath = strArg;
			}
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 1) ||
		(strKmlPath.isEmpty()) ||
		(bUnknownOption)) {
		std::cerr << "kmltest\n";
		std::cerr << QString("Usage: %1 [options] <KMLFileInput>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << "Where:\n";
		std::cerr << "    <KMLFileInput> = Source KML File to Parse\n\n";
		std::cerr << "    (Output will be to STDOUT)\n";
		std::cerr << "\n";
		return -1;
	}

	QFile fileKML(strKmlPath);

	if (!fileKML.open(QIODevice::ReadOnly)) {
		std::cerr << QString("\n\n*** Failed to open KML File \"%1\" for reading.\n").arg(strKmlPath).toUtf8().data();
		return -2;
	}

	CKmlDocument kmlDocument;

	if (!kmlDocument.read(&fileKML)) {
		std::cerr << QString("\n\n*** %1\n").arg(kmlDocument.parseException().message()).toUtf8().data();
		std::cerr << QString("Line: %1  Column: %2\nErrorCode: %3\n")
						.arg(kmlDocument.parseException().lineNumber()).arg(kmlDocument.parseException().columnNumber())
						.arg(kmlDocument.parseException().error()).toUtf8().data();
		return -3;
	}

	fileKML.close();

	std::cout << "Styles:\n";
	for (auto const &itrStyle : kmlDocument.styles()) {
		std::cout << "  ID: " << itrStyle.id().toUtf8().data() << "\n";
		for (auto const &itrStyleType : itrStyle) {
			std::cout << "  type: ";
			switch (itrStyleType->type()) {
				case CKmlBaseStyle::KSTE_ICON:
					std::cout << "icon\n";
					std::cout << "scale: " << itrStyle.iconStyle()->scale() << "\n";
					std::cout << "url: " << itrStyle.iconStyle()->urlIcon().toString().toUtf8().data() << "\n";
					break;
				case CKmlBaseStyle::KSTE_LINE:
					std::cout << "line\n";
					std::cout << "color: " << itrStyle.lineStyle()->color().name(QColor::HexArgb).toUtf8().data() << "\n";
					std::cout << "width: " << itrStyle.lineStyle()->width() << "\n";
					break;
				case CKmlBaseStyle::KSTE_POLY:
					std::cout << "poly\n";
					std::cout << "color: " << itrStyle.polyStyle()->color().name(QColor::HexArgb).toUtf8().data() << "\n";
					std::cout << "fill: " << itrStyle.polyStyle()->fill() << "\n";
					std::cout << "outline: " << itrStyle.polyStyle()->outline() << "\n";
					break;
			}
		}
	}
	std::cout << "\n";

	std::function<void(int, const CKmlFolder&)>fnDumpFolder = [&fnDumpFolder](int nLevel, const CKmlFolder &folder)->void {
		QString strSpacer = QString("  ").repeated(nLevel);
		std::cout << strSpacer.toUtf8().data() << "Name: " << folder.name().toUtf8().data() << "\n";
		std::cout << strSpacer.toUtf8().data() << "Desc: " << folder.description().toUtf8().data() << "\n";
		for (auto const &itrPlacemark : folder.placemarks()) {
			std::cout << strSpacer.toUtf8().data() << "Placemark: ";
			switch (itrPlacemark.type()) {
				case CKmlPlacemark::KPTE_POINT:
					std::cout << "point\n";
					break;
				case CKmlPlacemark::KPTE_POLYGON:
					std::cout << "polygon\n";
					break;
				case CKmlPlacemark::KPTE_LINESTRING:
					std::cout << "linestring\n";
					break;
			}
			std::cout << strSpacer.toUtf8().data() << "  name: " << itrPlacemark.name().toUtf8().data() << "\n";
			std::cout << strSpacer.toUtf8().data() << "  desc: " << itrPlacemark.description().toUtf8().data() << "\n";
			std::cout << strSpacer.toUtf8().data() << "  style: " << itrPlacemark.urlStyle().toString().toUtf8().data() << "\n";
			if ((itrPlacemark.type() == CKmlPlacemark::KPTE_POLYGON) ||
				(itrPlacemark.type() == CKmlPlacemark::KPTE_LINESTRING)) {
				std::cout << strSpacer.toUtf8().data() << "  tesselate: " << itrPlacemark.tesselate() << "\n";
			}
			std::cout << strSpacer.toUtf8().data() << "  coords:";
			for (auto const &itrCoord : itrPlacemark.coordinates()) {
				std::cout << " " << itrCoord.toString(QGeoCoordinate::Degrees).toUtf8().data();
			}
			std::cout << "\n";
		}
		for (auto const &itrFolder : folder.folders()) {
			fnDumpFolder(nLevel + 1, itrFolder);
		}
	};

	std::cout << "Folders:\n";
	fnDumpFolder(0, kmlDocument);

	return 0;
}

// ============================================================================
