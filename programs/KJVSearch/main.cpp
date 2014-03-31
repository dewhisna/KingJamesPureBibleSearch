/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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

#include "../KJVCanOpener/dbstruct.h"
#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/ReadDB.h"
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/SearchCompleter.h"
#include "../KJVCanOpener/PhraseEdit.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QObject>
#include <QMainWindow>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
//#include <QtXml>
#include <QStringList>
#include <QtGlobal>
#include <QSettings>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include <iostream>
#include <set>

QMainWindow *g_pMainWindow = NULL;
QTranslator g_qtTranslator;

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0



	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";

}	// namespace


// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	int nDescriptor = -1;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	QString strPhrase;
	bool bUnknownOption = false;
	bool bCaseSensitive = false;
	bool bAccentSensitive = false;
	bool bHyphenSensitive = false;
	bool bHumanReadable = false;
	bool bNoWordIndex = false;
	bool bUseAbbreviated = false;
	bool bSeparateLines = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				strPhrase = strArg;
			}
		} else if (strArg.compare("-c") == 0) {
			bCaseSensitive = true;
		} else if (strArg.compare("-a") == 0) {
			bAccentSensitive = true;
		} else if (strArg.compare("-y") == 0) {
			bHyphenSensitive = true;
		} else if (strArg.compare("-h") == 0) {
			bHumanReadable = true;
		} else if (strArg.compare("-w") == 0) {
			bNoWordIndex = true;
		} else if (strArg.compare("-b") == 0) {
			bUseAbbreviated = true;
		} else if (strArg.compare("-s") == 0) {
			bSeparateLines = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("KJVSearch Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Phrase>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database, searches for the specified Phrase\n").toUtf8().data();
		std::cerr << QString("    and outputs Normal Indexes for all found matching references\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -c  =  Case-Sensitive\n").toUtf8().data();
		std::cerr << QString("  -a  =  Accent-Sensitive\n").toUtf8().data();
		std::cerr << QString("  -y  =  Hyphen-Sensitive\n").toUtf8().data();
		std::cerr << QString("  -h  =  Human readable reference text (default is normal index values)\n").toUtf8().data();
		std::cerr << QString("  -w  =  No word index (only when using '-h')\n").toUtf8().data();
		std::cerr << QString("  -b  =  Use Abbreviated Book names (only when using '-h')\n").toUtf8().data();
		std::cerr << QString("  -s  =  Separate Lines (default is comma separated)\n\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Phrase : Text phrase to search, supports all KJPBS notation\n\n").toUtf8().data();
		return -1;
	}

	if ((nDescriptor >= 0) && (static_cast<unsigned int>(nDescriptor) < bibleDescriptorCount())) {
		bblDescriptor = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor));
	} else {
		std::cerr << "Unknown UUID-Index\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	std::cerr << QString("Reading database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();

	QFileInfo fiDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);

	CReadDatabase rdbMain(fiDBPath.absoluteFilePath(), QString(), NULL);
	if (!rdbMain.haveBibleDatabaseFiles(bblDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Bible Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDescriptor, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Bible Database!\n").toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	std::cerr << QString("Searching for: \"%1\"\n").arg(strPhrase).toUtf8().data();

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	CParsedPhrase parsePhrase(pBibleDatabase, bCaseSensitive, bAccentSensitive);

	TBibleDatabaseSettings bdbSettings = pBibleDatabase->settings();
	bdbSettings.setHyphenSensitive(bHyphenSensitive);
	pBibleDatabase->setSettings(bdbSettings);

	parsePhrase.ParsePhrase(strPhrase);
	parsePhrase.FindWords();

	const TPhraseTagList &lstResults = parsePhrase.GetPhraseTagSearchResults();

	std::cerr << QString("Found %1 matches\n").arg(lstResults.size()).toUtf8().data();
	for (int ndx = 0; ndx < lstResults.size(); ++ndx) {
		CRelIndex relIndex = lstResults.at(ndx).relIndex();
		if ((ndx > 0) && (!bSeparateLines)) std::cout << ",";
		if (!bHumanReadable) {
			std::cout << relIndex.asAnchor().toUtf8().data();
		} else {
			if (bNoWordIndex) relIndex.setWord(0);
			if (bUseAbbreviated) {
				std::cout << pBibleDatabase->PassageReferenceAbbrText(relIndex).toUtf8().data();
			} else {
				std::cout << pBibleDatabase->PassageReferenceText(relIndex).toUtf8().data();
			}
		}
		if (bSeparateLines) std::cout << "\n";
	}
	if (!bSeparateLines) std::cout << "\n";

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

