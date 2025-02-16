/****************************************************************************
**
** Copyright (C) 2014-2025 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/ReadDBEx.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/PhraseParser.h"
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

#include <iostream>
#include <set>

#include "../KJVCanOpener/PathConsts.h"

#include "version.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

}	// namespace

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(KJVSearch_VERSION);

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	int nDescriptor = -1;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	QString strPhrase;
	bool bUnknownOption = false;
	bool bConstrainBooks = false;
	bool bConstrainChapters = false;
	bool bConstrainVerses = false;
	bool bCaseSensitive = false;
	bool bAccentSensitive = false;
	bool bHyphenSensitive = false;
	bool bHumanReadable = false;
	bool bNoWordIndex = false;
	bool bUseAbbreviated = false;
	bool bSeparateLines = false;
	bool bRenderText = false;
	bool bNoDuplicateVerses = false;
	CReadDatabaseEx::DB_OVERRIDE_ENUM nDBOE = CReadDatabaseEx::DBOE_None;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				strPhrase = strArg;
			}
		} else if (strArg.compare("-cb") == 0) {
			bConstrainBooks = true;
		} else if (strArg.compare("-cc") == 0) {
			bConstrainChapters = true;
		} else if (strArg.compare("-cv") == 0) {
			bConstrainVerses = true;
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
		} else if (strArg.compare("-t") == 0) {
			bRenderText = true;
			bSeparateLines = true;
		} else if (strArg.compare("-d") == 0) {
			bNoDuplicateVerses = true;
			bNoWordIndex = true;
		} else if (strArg.startsWith("-dbo")) {
			nDBOE = static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(strArg.mid(4).toInt());
			if ((nDBOE < 0) || (nDBOE >= CReadDatabaseEx::DBOE_COUNT)) bUnknownOption = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << KJVSearch_APPNAME << " Version " << KJVSearch_VERSION_SEMVER << "\n\n";
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Phrase>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database, searches for the specified Phrase\n").toUtf8().data();
		std::cerr << QString("    and outputs Normal Indexes for all found matching references\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -cb =  Constrain to whole books\n").toUtf8().data();
		std::cerr << QString("  -cc =  Constrain to whole chapters (implies '-b')\n").toUtf8().data();
		std::cerr << QString("  -cv =  Constrain to whole verses (implies '-b' and '-c')\n").toUtf8().data();
		std::cerr << QString("  -c  =  Case-Sensitive\n").toUtf8().data();
		std::cerr << QString("  -a  =  Accent-Sensitive\n").toUtf8().data();
		std::cerr << QString("  -y  =  Hyphen-Sensitive\n").toUtf8().data();
		std::cerr << QString("  -h  =  Human readable reference text (default is normal index values)\n").toUtf8().data();
		std::cerr << QString("  -w  =  No word index (only when using '-h')\n").toUtf8().data();
		std::cerr << QString("  -b  =  Use Abbreviated Book names (only when using '-h')\n").toUtf8().data();
		std::cerr << QString("  -s  =  Separate Lines (default is comma separated)\n").toUtf8().data();
		std::cerr << QString("  -t  =  Render verse text (implies '-s')\n").toUtf8().data();
		std::cerr << QString("  -d  =  Removed Duplicate verses (implies '-w')\n").toUtf8().data();
		std::cerr << QString("  -dbo<n> = Database Override Option\n").toUtf8().data();
		std::cerr << QString("          where <n> is one of the following:\n").toUtf8().data();
		for (int ndx = 0; ndx < CReadDatabaseEx::DBOE_COUNT; ++ndx) {
			std::cerr << QString("            %1 : %2%3\n")
							.arg(ndx)
							.arg(CReadDatabaseEx::dboeDescription(static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(ndx)))
							.arg((ndx == CReadDatabaseEx::DBOE_None) ? " (default)" : "").toUtf8().data();
		}
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Phrase : Text phrase to search, supports all KJPBS notation\n\n").toUtf8().data();
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

	std::cerr << QString("Reading Bible Database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();
	std::cerr << QString("Database Override Option: %1\n").arg(CReadDatabaseEx::dboeDescription(nDBOE)).toUtf8().data();

	CReadDatabaseEx rdbMain(nDBOE);
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

	CVerseTextPlainRichifierTags tagsPlain;
	TBibleDatabaseSettings bdbSettings = pBibleDatabase->settings();
	bdbSettings.setHyphenSensitive(bHyphenSensitive);
	pBibleDatabase->setSettings(bdbSettings);

	parsePhrase.ParsePhrase(strPhrase);
	parsePhrase.FindWords();

	TPhraseTagList lstResults(parsePhrase.GetPhraseTagSearchResults());

	if (bNoDuplicateVerses) {
		CRelIndex relIndexLast;
		for (int ndx = 0; ndx < lstResults.size(); ++ndx) {
			CRelIndex relIndex = lstResults.at(ndx).relIndex();
			relIndex.setWord(1);
			lstResults[ndx].setRelIndex(relIndex);
			if (relIndexLast.isSet()) {
				if (relIndex == relIndexLast) {
					lstResults.removeAt(ndx);
					--ndx;
				}
			}
			relIndexLast = relIndex;
		}
	}

	for (int ndx = 0; ndx < lstResults.size(); ++ndx) {
		bool bRemove = false;
		if (!pBibleDatabase->completelyContains(lstResults.at(ndx))) {
			bRemove = true;
		} else if (bConstrainVerses) {
			if (!pBibleDatabase->versePhraseTag(lstResults.at(ndx).relIndex()).completelyContains(pBibleDatabase.data(), lstResults.at(ndx))) {
				bRemove = true;
			}
		} else if (bConstrainChapters) {
			if (!pBibleDatabase->chapterPhraseTag(lstResults.at(ndx).relIndex()).completelyContains(pBibleDatabase.data(), lstResults.at(ndx))) {
				bRemove = true;
			}
		} else if (bConstrainBooks) {
			if (!pBibleDatabase->bookPhraseTag(lstResults.at(ndx).relIndex()).completelyContains(pBibleDatabase.data(), lstResults.at(ndx))) {
				bRemove = true;
			}
		}
		if (bRemove) {
			lstResults.removeAt(ndx);
			--ndx;
		}
	}

	std::cerr << QString("Found %1 matches\n").arg(lstResults.size()).toUtf8().data();
	for (int ndx = 0; ndx < lstResults.size(); ++ndx) {
		CRelIndex relIndex = lstResults.at(ndx).relIndex();
		if ((ndx > 0) && (!bSeparateLines)) std::cout << ",";
		if (!bHumanReadable) {
			std::cout << relIndex.asAnchor().toUtf8().data();
		} else {
			if ((bNoWordIndex) && (!relIndex.isColophon()) && (!relIndex.isSuperscription())) relIndex.setWord(0);
			if (bUseAbbreviated) {
				std::cout << pBibleDatabase->PassageReferenceAbbrText(relIndex, bNoWordIndex).toUtf8().data();
			} else {
				std::cout << pBibleDatabase->PassageReferenceText(relIndex, bNoWordIndex).toUtf8().data();
			}
		}
		if (bRenderText) {
			CRelIndex relIndexWord1 = relIndex;
			relIndexWord1.setWord(1);
			const CVerseEntry *pVerse = pBibleDatabase->verseEntry(relIndexWord1);
			std::cout << ": ";
			if (pVerse != nullptr) {
				std::cout << CVerseTextRichifier::parse(relIndexWord1, pBibleDatabase.data(), pVerse, tagsPlain).toUtf8().data();
			} else {
				std::cout << ": <NULL>";
			}
		}
		if (bSeparateLines) std::cout << "\n";
	}
	if (!bSeparateLines) std::cout << "\n";

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

