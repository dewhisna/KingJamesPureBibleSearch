/****************************************************************************
**
** Copyright (C) 2016 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/PassageReferenceWidget.h"

#include <QCoreApplication>
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

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0

	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";

	// Use translations from the main app:
	const char *g_constrTranslationsPath = "../../KJVCanOpener/translations/";
	const char *g_constrTranslationFilenamePrefix = "kjpbs";

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
	QString strReference;
	bool bUnknownOption = false;
	unsigned int nHideHyphenOptions = TBibleDatabaseSettings::HHO_None;
	bool bIncludeRef = false;
	bool bUseAbbrRef = false;
	bool bUseHTML = false;
	bool bNoColorizeWordsOfJesus = false;
	bool bHideTransChange = false;
	bool bTransChangeAsBrackets = false;
	bool bHidePilcrows = false;
	bool bHidePs119 = false;
	bool bAddNewline = false;
	bool bQuiet = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				strReference = strArg;
			}
		} else if (strArg.compare("-h0") == 0) {
			nHideHyphenOptions = TBibleDatabaseSettings::HHO_None;
		} else if (strArg.compare("-h1") == 0) {
			nHideHyphenOptions = TBibleDatabaseSettings::HHO_ProperWords;
		} else if (strArg.compare("-h2") == 0) {
			nHideHyphenOptions = TBibleDatabaseSettings::HHO_OrdinaryWords;
		} else if (strArg.compare("-h3") == 0) {
			nHideHyphenOptions = TBibleDatabaseSettings::HHO_ProperWords | TBibleDatabaseSettings::HHO_OrdinaryWords;
		} else if (strArg.compare("-r") == 0) {
			bIncludeRef = true;
		} else if (strArg.compare("-a") == 0) {
			bUseAbbrRef = true;
		} else if (strArg.compare("-m") == 0) {
			bUseHTML = true;
		} else if (strArg.compare("-j") == 0) {
			bNoColorizeWordsOfJesus = true;
		} else if (strArg.compare("-t") == 0) {
			bHideTransChange = true;
		} else if (strArg.compare("-b") == 0) {
			bTransChangeAsBrackets = true;
		} else if (strArg.compare("-p") == 0) {
			bHidePilcrows = true;
		} else if (strArg.compare("-119") == 0) {
			bHidePs119 = true;
		} else if (strArg.compare("-n") == 0) {
			bAddNewline = true;
		} else if (strArg.compare("-q") == 0) {
			bQuiet = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("KJVLookup Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Reference>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database, looks up the specified reference,\n").toUtf8().data();
		std::cerr << QString("    and outputs the corresponding verse text\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -h0 =  Hyphens Hide None (i.e. display all, this is default)\n").toUtf8().data();
		std::cerr << QString("  -h1 =  Hyphens Hide in Proper Words\n").toUtf8().data();
		std::cerr << QString("  -h2 =  Hyphens Hide in Ordinary Words\n").toUtf8().data();
		std::cerr << QString("  -h3 =  Hyphens Hide in Both\n").toUtf8().data();
		std::cerr << QString("  -r  =  Include reference text\n").toUtf8().data();
		std::cerr << QString("  -a  =  Use abbreviated reference (only with -r mode)\n").toUtf8().data();
		std::cerr << QString("  -m  =  Use HTML Markup (default is plain text)\n").toUtf8().data();
		std::cerr << QString("  -j  =  Don't Markup Words of Jesus (only for -m mode)\n").toUtf8().data();
		std::cerr << QString("  -t  =  Hide Translation Change Markup\n").toUtf8().data();
		std::cerr << QString("  -b  =  Use Brackets for TransChange (for -m mode)\n").toUtf8().data();
		std::cerr << QString("  -p  =  Hide Pilcrows\n").toUtf8().data();
		std::cerr << QString("  -119=  Hide Psalm 119 Hebrew Prefixes\n").toUtf8().data();
		std::cerr << QString("  -n  =  Add Trailing Newline\n").toUtf8().data();
		std::cerr << QString("  -q  =  Quiet mode (suppress unnecessary output)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Reference : Can be #nnnn anchor format or Phrase Ref to parse\n").toUtf8().data();
		std::cerr << QString("         Examples:  #721620992\n").toUtf8().data();
		std::cerr << QString("                    \"jn 3:16\"\n\n").toUtf8().data();
		return -1;
	}

	if ((nDescriptor >= 0) && (static_cast<unsigned int>(nDescriptor) < bibleDescriptorCount())) {
		bblDescriptor = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor));
	} else {
		std::cerr << "Unknown UUID-Index\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	if (!bQuiet) {
		std::cerr << QString("Reading database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();
	}

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

	if (!bQuiet) {
		std::cerr << QString("Resolving: \"%1\"\n").arg(strReference).toUtf8().data();
	}

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	TPhraseTag tagRefIndex;

	if (strReference.startsWith(QChar('#'))) {
		tagRefIndex = TPhraseTag(CRelIndex(strReference.mid(1)));
	} else {
		CPassageReferenceResolver resolver(pBibleDatabase);
		tagRefIndex = resolver.resolve(strReference);
	}

	if (!tagRefIndex.isSet() || !pBibleDatabase->completelyContains(tagRefIndex)) {
		std::cerr << QString("\n*** ERROR: Bible Database doesn't contain reference \"%1\"!\n").arg(strReference).toUtf8().data();
		return -4;
	}

	CVerseTextRichifierTags richifierTags = (bUseHTML ? CVerseTextRichifierTags() : CVerseTextPlainRichifierTags());
	if (bNoColorizeWordsOfJesus) richifierTags.setWordsOfJesusTags(QString(), QString());
	richifierTags.setShowPilcrowMarkers(!bHidePilcrows);
	richifierTags.setAddRichPs119HebrewPrefix(!bHidePs119);
	if (bHideTransChange) {
		richifierTags.setTransChangeAddedTags(QString(), QString());
	} else if (bTransChangeAsBrackets) {
		richifierTags.setTransChangeAddedTags("[", "]");
	}

	TBibleDatabaseSettings bdbSettings = pBibleDatabase->settings();
	bdbSettings.setHideHyphens(nHideHyphenOptions);
	pBibleDatabase->setSettings(bdbSettings);

	if (bIncludeRef) {
		if (bUseHTML) std::cout << "<b>";

		CRelIndex relIndex = tagRefIndex.relIndex();
		if (!relIndex.isColophon() && !relIndex.isSuperscription()) relIndex.setWord(0);
		if (bUseAbbrRef) {
			std::cout << pBibleDatabase->PassageReferenceAbbrText(relIndex, true).toUtf8().data();
		} else {
			std::cout << pBibleDatabase->PassageReferenceText(relIndex, true).toUtf8().data();
		}

		if (!bUseHTML) {
			std::cout << ": ";
		} else {
			std::cout << "</b>";
			std::cout << "&nbsp;";
		}
	}
	CRelIndex relIndexWord1 = tagRefIndex.relIndex();
	relIndexWord1.setWord(1);
	const CVerseEntry *pVerse = pBibleDatabase->verseEntry(relIndexWord1);
	if (pVerse) {
		std::cout << CVerseTextRichifier::parse(relIndexWord1, pBibleDatabase.data(), pVerse, richifierTags).toUtf8().data();
	} else {
		if (!bUseHTML) {
			std::cout << "<NULL>";
		} else {
			std::cout << "&lt;NULL&gt;";
		}
	}
	if (bAddNewline) {
		std::cout << "\n";
	}

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

