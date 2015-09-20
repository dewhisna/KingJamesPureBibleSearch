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

#include <QCoreApplication>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtGui/QTextDocument>

#include "../../KJVCanOpener/CSV.h"

#include <iostream>

// ============================================================================

static const QString escape( const QString &s )
{
#if QT_VERSION < 0x050000
	return Qt::escape( s );
#else
	return s.toHtmlEscaped();
#endif
}

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	QString strFileIn;
	QString strFileOut;
	bool bShowHelp = false;
	bool bUseIPAsName = false;
	bool bIncludeISP = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg(argv[ndx]);
		if (!strArg.startsWith("-")) {
			if (strFileIn.isEmpty()) {
				strFileIn = strArg;
			} else if (strFileOut.isEmpty()) {
				strFileOut = strArg;
			} else {
				bShowHelp = true;
				break;
			}
		} else {
			if ((strArg.compare("-h", Qt::CaseInsensitive) == 0) ||
				(strArg.compare("--help", Qt::CaseInsensitive) == 0)) {
				bShowHelp = true;
				break;
			} else if (strArg.compare("-i", Qt::CaseSensitive) == 0) {
				bUseIPAsName = true;
			} else if (strArg.compare("-s", Qt::CaseSensitive) == 0) {
				bIncludeISP = true;
			} else {	// TODO : Add other command-line options
				bShowHelp = true;
				break;
			}
		}
	}

	if (bShowHelp) {
		std::cout << QString("%1 [options] [<In-File>] [[<Out-File>]]\n\n").arg(QFileInfo(QCoreApplication::applicationFilePath()).fileName()).toUtf8().data();
		std::cout << "Input is from the 'webchannel-geolocate' script'\n";
		std::cout << "Output is a Google Maps KML File\n\n";
		std::cout << "In-File and Out-File are both optional.  If omitted, stdin and stdout\n";
		std::cout << "    are used, respectively.  If only one filename is given, it will be\n";
		std::cout << "    considered the input file (to avoid accidentally overwriting something.\n\n";
		std::cout << "\n";
		std::cout << "Options:\n";
		std::cout << "\n";
		std::cout << "  -h or --help : Show this help information\n";
		std::cout << "\n";
		std::cout << "  -i  :  Include IP Address as the name rather than site description\n";
		std::cout << "\n";
		std::cout << "  -s  :  Add Service Provider details to the description\n";
		std::cout << "\n";
		return -1;
	}

	//
	// Input format expected:
	//
	// " telize.com : 12.34.56.78,US,United States,IA,Iowa,Cedar Rapids,52403,America/Chicago,41.973100000000002,-91.576700000000002,0,Overpriced ISP Corp"
	//

	QTextStream tsIn(stdin);
	QTextStream tsOut(stdout);
	QFile fileIn(strFileIn);
	QFile fileOut(strFileOut);
	if (!strFileIn.isEmpty()) {
		if (!fileIn.open(QIODevice::ReadOnly)) {
			std::cerr << QString("*** Failed to open \"%1\" for reading!\n").arg(strFileIn).toUtf8().data();
			return -2;
		}
		tsIn.setDevice(&fileIn);
	}
	if (!strFileOut.isEmpty()) {
		if (!fileOut.open(QIODevice::WriteOnly)) {
			std::cerr << QString("*** Failed to open \"%1\" for writing!\n").arg(strFileOut).toUtf8().data();
			return -3;
		}
		tsOut.setDevice(&fileOut);
	}

	tsIn.setCodec("UTF-8");
	tsOut.setCodec("UTF-8");

	tsOut << QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	tsOut << QString("<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\">\n");
	tsOut << QString("\t<Document>\n");
	tsOut << QString("\t\t<Style id=\"pinned_scroll\">\n");
	tsOut << QString("\t\t\t<IconStyle>\n");
	tsOut << QString("\t\t\t\t<Icon>\n");
	tsOut << QString("\t\t\t\t\t<href>http://webchannel.purebiblesearch.com/pinned-scroll.png</href>\n");
	tsOut << QString("\t\t\t\t</Icon>\n");
	tsOut << QString("\t\t\t</IconStyle>\n");
	tsOut << QString("\t\t</Style>\n");

	while (!tsIn.atEnd()) {
		QString strLine = tsIn.readLine().trimmed();
		if (strLine.isEmpty()) continue;
		int nPos = strLine.indexOf(QChar(':'));
		if (nPos == -1) {
			std::cerr << QString("*** Invalid line: \"%1\"\n").arg(strLine).toUtf8().data();
			continue;
		}
		strLine = strLine.mid(nPos+1).trimmed();
		CCSVStream csv(& strLine, QIODevice::ReadOnly);

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

		csv >> strIP >> strCountryCode >> strCountry >> strRegionCode >> strRegion >> strCity;
		csv >> strPostalCode >> strTimeZone >> strLat >> strLong >> strMetroCode >> strISP;

		QString strDesc;
		if (!strCity.isEmpty()) strDesc += QString("%1 ").arg(strCity);
		if ((strCountryCode == "US") && (!strCity.isEmpty())) {
			if (!strRegionCode.isEmpty()) {
				strDesc += QString("%1 ").arg(strRegionCode);
			} else if (!strRegion.isEmpty()) {
				strDesc += QString("%1 ").arg(strRegion);
			}
			strDesc += QString("%1 ").arg(strCountryCode);
		} else {
			if (!strRegion.isEmpty()) {
				strDesc += QString("%1 ").arg(strRegion);
			} else if (!strRegionCode.isEmpty()) {
				strDesc += QString("%1 ").arg(strRegionCode);
			}
			if (!strCountry.isEmpty()) {
				strDesc += QString("%1 ").arg(strCountry);
			} else if (!strCountryCode.isEmpty()) {
				strDesc += QString("%1 ").arg(strCountryCode);
			}
		}
		if (!strPostalCode.isEmpty()) strDesc += QString("%1 ").arg(strPostalCode);
		strDesc.remove(QChar('\0'));
		strDesc = strDesc.trimmed();

		tsOut << QString("\t\t<Placemark>\n");
		if (bUseIPAsName) {
			tsOut << QString("\t\t\t<name>%1</name>\n").arg(escape(strIP));
		} else {
			tsOut << QString("\t\t\t<name>%1</name>\n").arg(escape(strDesc));
		}

		tsOut << QString("\t\t\t<description>\n");
		if (bUseIPAsName) {
			tsOut << QString("\t\t\t\t%1\n").arg(escape(strDesc));
		}
		if (!strISP.isEmpty() && bIncludeISP) tsOut << QString("\t\t\t\t%1\n").arg(escape(strISP.remove(QChar('\0'))));
		tsOut << QString("\t\t\t</description>\n");
		tsOut << QString("\t\t\t<styleUrl>#pinned_scroll</styleUrl>\n");
		tsOut << QString("\t\t\t<Point>\n");
		tsOut << QString("\t\t\t\t<coordinates>%1,%2</coordinates>\n").arg(strLong).arg(strLat);
		tsOut << QString("\t\t\t</Point>\n");
		tsOut << QString("\t\t</Placemark>\n");
	}

	tsOut << QString("\t</Document>\n");
	tsOut << QString("</kml>\n");

	return 0;
}
