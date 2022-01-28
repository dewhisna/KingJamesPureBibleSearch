/****************************************************************************
**
** Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/Translator.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include <iostream>
#include <set>

#include "../KJVCanOpener/PathConsts.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0

}	// namespace


// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	int nDescriptor = -1;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	bool bUnknownOption = false;
	bool bSkipColophons = false;
	bool bSkipSuperscriptions = false;
	bool bPrintReference = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			}
		} else if (strArg.compare("-sc") == 0) {
			bSkipColophons = true;
		} else if (strArg.compare("-ss") == 0) {
			bSkipSuperscriptions = true;
		} else if (strArg.compare("-r") == 0) {
			bPrintReference = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 1) || (bUnknownOption)) {
		std::cerr << QString("KJVDataDump Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database and dumps the template for each verse\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -sc =  Skip Colophons\n").toUtf8().data();
		std::cerr << QString("  -ss =  Skip Superscriptions\n").toUtf8().data();
		std::cerr << QString("  -r  =  Print Reference\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		return -1;
	}

	// TODO : Add support for raw-UUID as well as indexes

	if ((nDescriptor >= 0) && (static_cast<unsigned int>(nDescriptor) < bibleDescriptorCount())) {
		bblDescriptor = TBibleDatabaseList::availableBibleDatabaseDescriptor(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor)).m_strUUID);
	} else {
		std::cerr << "Unknown UUID-Index\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	std::cerr << QString("Reading database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();

	CReadDatabase rdbMain;
	if (!rdbMain.haveBibleDatabaseFiles(bblDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Bible Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDescriptor, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Bible Database!\n").toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	CRelIndex ndxVerse = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_Start);

	while (ndxVerse.isSet()) {
		if ((bSkipColophons && ndxVerse.isColophon()) ||
			(bSkipSuperscriptions && ndxVerse.isSuperscription())) {
			// Must increment to next physical word index instead of using calculator movement
			//	in order to properly traverse colophons and superscriptions:
//			ndxVerse = pBibleDatabase->calcRelIndex(ndxVerse, CBibleDatabase::RIME_NextVerse);
			ndxVerse = pBibleDatabase->calcRelIndex(ndxVerse, CBibleDatabase::RIME_EndOfVerse);
			ndxVerse = pBibleDatabase->DenormalizeIndex(pBibleDatabase->NormalizeIndex(ndxVerse)+1);
			continue;
		}

		const CVerseEntry *pVerse = pBibleDatabase->verseEntry(ndxVerse);
		if (pVerse) {
			if (bPrintReference) {
				std::cout << pBibleDatabase->PassageReferenceText(ndxVerse).toUtf8().data() << " : " << pVerse->m_strTemplate.toUtf8().data() << std::endl;
			} else{
				std::cout << pVerse->m_strTemplate.toUtf8().data() << std::endl;
			}
		}

		// Must increment to next physical word index instead of using calculator movement
		//	in order to properly traverse colophons and superscriptions:
//		ndxVerse = pBibleDatabase->calcRelIndex(ndxVerse, CBibleDatabase::RIME_NextVerse);
		ndxVerse = pBibleDatabase->calcRelIndex(ndxVerse, CBibleDatabase::RIME_EndOfVerse);
		ndxVerse = pBibleDatabase->DenormalizeIndex(pBibleDatabase->NormalizeIndex(ndxVerse)+1);
	}

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}
