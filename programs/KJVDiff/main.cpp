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


static int readDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain)
{
	std::cerr << QString::fromUtf8("Reading database: %1\n").arg(bblDesc.m_strDBDesc).toUtf8().data();

	QFileInfo fiDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);

	CReadDatabase rdbMain(fiDBPath.absoluteFilePath(), QString(), NULL);
	if (!rdbMain.haveBibleDatabaseFiles(bblDesc)) {
		std::cerr << QString::fromUtf8("\n*** ERROR: Unable to locate Bible Database Files for %1!\n").arg(bblDesc.m_strDBDesc).toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDesc, bSetAsMain)) {
		std::cerr << QString::fromUtf8("\n*** ERROR: Failed to Read the Bible Database for %1!\n").arg(bblDesc.m_strDBDesc).toUtf8().data();
		return -3;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationName("KJVDiff");
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	int nDescriptor1 = -1;
	int nDescriptor2 = -1;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor1;
	TBibleDescriptor bblDescriptor2;
	bool bUnknownOption = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor1 = strArg.toInt();
			} else if (nArgsFound == 2) {
				nDescriptor2 = strArg.toInt();
			}
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("%1 Version %2\n\n").arg(a.applicationName()).arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index-1> <UUID-Index-2>\n\n").arg(a.applicationName()).toUtf8().data();
		std::cerr << QString("Reads the specified databases and does a comparison for pertinent differences\n").toUtf8().data();
		std::cerr << QString("    and outputs the diff results...\n\n").toUtf8().data();
		std::cerr << QString("Currently, no special [options] are supported\n\n").toUtf8().data();
//		std::cerr << QString("Options are:\n").toUtf8().data();
//		std::cerr << QString("  -c  =  Case-Sensitive\n").toUtf8().data();
//		std::cerr << QString("  -a  =  Accent-Sensitive\n").toUtf8().data();
//		std::cerr << QString("  -h  =  Human readable reference text (default is normal index values)\n").toUtf8().data();
//		std::cerr << QString("  -w  =  No word index (only when using '-h')\n").toUtf8().data();
//		std::cerr << QString("  -b  =  Use Abbreviated Book names (only when using '-h')\n").toUtf8().data();
//		std::cerr << QString("  -s  =  Separate Lines (default is comma separated)\n\n").toUtf8().data();
		std::cerr << QString("UUID-Index Values:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			const TBibleDescriptor &bblDesc(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)));
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bblDesc.m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		return -1;
	}

	if ((nDescriptor1 >= 0) && (static_cast<unsigned int>(nDescriptor1) < bibleDescriptorCount())) {
		bblDescriptor1 = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor1));
	} else {
		std::cerr << QString::fromUtf8("Unknown UUID-Index: %1\n").arg(nDescriptor1).toUtf8().data();
		return -1;
	}

	if ((nDescriptor2 >= 0) && (static_cast<unsigned int>(nDescriptor2) < bibleDescriptorCount())) {
		bblDescriptor2 = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor2));
	} else {
		std::cerr << QString::fromUtf8("Unknown UUID-Index: %1\n").arg(nDescriptor2).toUtf8().data();
		return -1;
	}

	// ------------------------------------------------------------------------

	int nReadStatus;

	nReadStatus = readDatabase(bblDescriptor1, true);
	if (nReadStatus != 0) return nReadStatus;

	nReadStatus = readDatabase(bblDescriptor2, false);
	if (nReadStatus != 0) return nReadStatus;

	// ------------------------------------------------------------------------




//	return a.exec();
	return 0;
}
