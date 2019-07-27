/****************************************************************************
**
** Copyright (C) 2013-2019 Donna Whisnant, a.k.a. Dewtronics.
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

#include <QCoreApplication>

#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include <assert.h>
#include <string>
#include <iostream>
#include <algorithm>

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

	int nBibleDescriptor = -1;
	int nDictDescriptor = -1;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	TDictionaryDescriptor dictDescriptor;
	bool bUnknownOption = false;
	bool bUseConcordanceList = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nBibleDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				nDictDescriptor = strArg.toInt();
			}
		} else if (strArg.compare("-c") == 0) {
			bUseConcordanceList = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("KJVDictWord Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <Bible-UUID-Index> <Dictionary-UUID-Index>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified Bible Database and Dictionary Database and\n").toUtf8().data();
		std::cerr << QString("    searches every word in the Bible to see if it's found in\n").toUtf8().data();
		std::cerr << QString("    the dictionary\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -c = Use full Concordance List\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Bible-UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		for (unsigned int ndx = 0; ndx < dictionaryDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(dictionaryDescriptor(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		return -1;
	}

	if ((nBibleDescriptor >= 0) && (static_cast<unsigned int>(nBibleDescriptor) < bibleDescriptorCount())) {
		bblDescriptor = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nBibleDescriptor));
	} else {
		std::cerr << "Unknown Bible-UUID-Index\n";
		return -1;
	}

	if ((nDictDescriptor >= 0) && (static_cast<unsigned int>(nDictDescriptor) < dictionaryDescriptorCount())) {
		dictDescriptor = dictionaryDescriptor(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(nDictDescriptor));
	} else {
		std::cerr << "Unknown Dictionary-UUID-Index\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	QFileInfo fiBblDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);
	QFileInfo fiDictDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);

	CReadDatabase rdbMain(fiBblDBPath.absoluteFilePath(), fiDictDBPath.absoluteFilePath(), NULL);

	std::cerr << QString("Reading Bible Database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();
	if (!rdbMain.haveBibleDatabaseFiles(bblDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Bible Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDescriptor, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Bible Database!\n").toUtf8().data();
		return -3;
	}

	std::cerr << QString("Reading Dictionary Database: %1\n").arg(dictDescriptor.m_strDBName).toUtf8().data();
	if (!rdbMain.haveDictionaryDatabaseFiles(dictDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Dictionary Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadDictionaryDatabase(dictDescriptor, false, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Dictionary Database!\n").toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();
	CDictionaryDatabasePtr pDictDatabase = TDictionaryDatabaseList::instance()->mainDictionaryDatabase();

	std::cout << "Bible-Word,Dictionary-Word\n";
	for (int ndx = 0; ndx < (bUseConcordanceList ?	pBibleDatabase->concordanceWordList().size() :
													pBibleDatabase->lstWordList().size()); ++ndx) {
		QString strWord = bUseConcordanceList ? pBibleDatabase->concordanceWordList().at(ndx).decomposedWord() :
												pBibleDatabase->lstWordList().at(ndx);

		if (pDictDatabase->wordExists(strWord)) {
			std::cout << strWord.toUtf8().data() << "," << strWord.toUtf8().data() << "\n";
		} else {
			std::cout << strWord.toUtf8().data() << ",\n";
		}
	}

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}
