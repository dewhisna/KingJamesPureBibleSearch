/****************************************************************************
**
** Copyright (C) 2018 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/KJVSearchCriteria.h"

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

#define NUM_BK 80u				// Total Books Defined
#define NUM_BK_OT 39u			// Total Books in Old Testament
#define NUM_BK_NT 27u			// Total Books in New Testament
#define NUM_BK_APOC 14u			// Total Books in Apocrypha (KJVA)
#define NUM_TST 3u				// Total Number of Testaments (or pseudo-testaments, in the case of Apocrypha)

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

static bool ascendingLessThan(const CPhraseEntry &s1, const CPhraseEntry &s2)
{
	return (CSearchStringListModel::decompose(s1.text(), true).compare(CSearchStringListModel::decompose(s2.text(), true), Qt::CaseSensitive) < 0);
}

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
	int nOccurrences = 2;
	int nMinLen = 2;
	int nMaxLen = 10;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	bool bUnknownOption = false;
	bool bHyphenSensitive = false;
	bool bCaseSensitive = false;
	bool bAccentSensitive = false;
	bool bConstrainBooks = false;
	bool bConstrainChapters = false;
	bool bConstrainVerses = false;
	CSearchCriteria searchCriteria;
	TRelativeIndexSet setSearchWithin;
	bool bSearchWithinIsEntireBible = true;

	// Default to searching Entire Bible:
	for (unsigned int nBk = 1; nBk <= NUM_BK; ++nBk) {
		setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
	}
	setSearchWithin.insert(CSearchCriteria::SSI_COLOPHON);
	setSearchWithin.insert(CSearchCriteria::SSI_SUPERSCRIPTION);

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				nOccurrences = strArg.toInt();
				if (nOccurrences < 2) nOccurrences = 2;
			} else if (nArgsFound == 3) {
				nMinLen = strArg.toInt();
				if (nMinLen < 2) nMinLen = 2;
				if (nMinLen > nMaxLen) nMaxLen = nMinLen;		// initial default for max
			} else if (nArgsFound == 4) {
				nMaxLen = strArg.toInt();
				if (nMaxLen < nMinLen) nMaxLen = nMinLen;
			} else {
				bUnknownOption = true;
			}
		} else if (strArg.compare("-b") == 0) {
			bConstrainBooks = true;
		} else if (strArg.compare("-c") == 0) {
			bConstrainChapters = true;
			bConstrainBooks = true;
		} else if (strArg.compare("-v") == 0) {
			bConstrainVerses = true;
			bConstrainChapters = true;
			bConstrainBooks = true;
		} else if (strArg.compare("-pc") == 0) {
			bCaseSensitive = true;
		} else if (strArg.compare("-pa") == 0) {
			bAccentSensitive = true;
		} else if (strArg.compare("-ph") == 0) {
			bHyphenSensitive = true;
		} else if (strArg.compare("-sc") == 0) {
			setSearchWithin.erase(CSearchCriteria::SSI_COLOPHON);
			bSearchWithinIsEntireBible = false;
		} else if (strArg.compare("-ss") == 0) {
			setSearchWithin.erase(CSearchCriteria::SSI_SUPERSCRIPTION);
			bSearchWithinIsEntireBible = false;
		} else if (strArg.compare("-so") == 0) {
			for (unsigned int nBk = 1; nBk <= NUM_BK_OT; ++nBk) {
				setSearchWithin.erase(CRelIndex(nBk, 0, 0, 0));
			}
			bSearchWithinIsEntireBible = false;
		} else if (strArg.compare("-sn") == 0) {
			for (unsigned int nBk = 1; nBk <= NUM_BK_NT; ++nBk) {
				setSearchWithin.erase(CRelIndex(nBk+NUM_BK_OT, 0, 0, 0));
			}
			bSearchWithinIsEntireBible = false;
		} else if (strArg.startsWith("-s")) {
			unsigned int nBk = strArg.mid(2).toUInt();
			setSearchWithin.erase(CRelIndex(nBk, 0, 0, 0));
			bSearchWithinIsEntireBible = false;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound < 2) || (bUnknownOption)) {
		std::cerr << QString("KJVPhraseSearch Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Occurrences> [<MinLen> [<MaxLen>]]\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database, searches for all phrases of the\n").toUtf8().data();
		std::cerr << QString("    specified length range that occur the specified number of times.\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -b  =  Constrain to whole books\n").toUtf8().data();
		std::cerr << QString("  -c  =  Constrain to whole chapters (implies '-b')\n").toUtf8().data();
		std::cerr << QString("  -v  =  Constrain to whole verses (implies '-b' and '-c')\n").toUtf8().data();
		std::cerr << QString("  -pc =  Phrases are Case-Sensitive when compared (default=false)\n").toUtf8().data();
		std::cerr << QString("  -pa =  Phrases are Accent-Sensitive when compared (default=false)\n").toUtf8().data();
		std::cerr << QString("  -ph =  Phrases are Hyphen-Sensitive when compared (default=false)\n").toUtf8().data();
		std::cerr << QString("  -sc =  Skip Colophons\n").toUtf8().data();
		std::cerr << QString("  -ss =  Skip Superscriptions\n").toUtf8().data();
		std::cerr << QString("  -so =  Skip Old Testament\n").toUtf8().data();
		std::cerr << QString("  -sn =  Skip New Testament\n").toUtf8().data();
		std::cerr << QString("  -sN =  Skip Book 'N', where 'N' is Book Number in Bible\n").toUtf8().data();
		std::cerr << QString("           (Default is to search the Entire Bible)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Occurrences : Number of occurrences phrase must exist (>=2)\n").toUtf8().data();
		std::cerr << QString("MinLen : Minimum Phrase Length (>=2), defaults to 2\n").toUtf8().data();
		std::cerr << QString("MaxLen : Maximum Phrase Length (>=MinLen), defaults to 10 or MinLen\n").toUtf8().data();
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

	std::cerr << QString("Searching for phrases of length %1-%2 words that occur %3 times:\n").arg(nMinLen).arg(nMaxLen).arg(nOccurrences).toUtf8().data();

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	TBibleDatabaseSettings bdbSettings = pBibleDatabase->settings();
	bdbSettings.setHyphenSensitive(bHyphenSensitive);
	pBibleDatabase->setSettings(bdbSettings);

	searchCriteria.setSearchWithin(setSearchWithin);

	// Go through entire Bible Database and find all phrases whose length is between
	//		MinLen and MaxLen:
	uint32_t nNormalIndex = 1;
	CPhraseList lstOverallResults;
	uint32_t nBk = 0;
	uint32_t nChp = 0;
	while (nNormalIndex <= pBibleDatabase->bibleEntry().m_nNumWrd) {
		CRelIndex ndxPhrase(pBibleDatabase->DenormalizeIndex(nNormalIndex));
		if (!searchCriteria.indexIsWithin(CRelIndex(ndxPhrase.book(), 0, 0, 0))) {
			nNormalIndex = pBibleDatabase->NormalizeIndex(CRelIndex(ndxPhrase.book()+1, 0, 0, 0));
			continue;	// Skip entire books we aren't searching
		}
		if (nBk != ndxPhrase.book()) {
			nBk = ndxPhrase.book();
			nChp = 0;
			std::cerr << "\n" << pBibleDatabase->bookName(ndxPhrase).toUtf8().data();
		}
		if (nChp != ndxPhrase.chapter()) {
			nChp = ndxPhrase.chapter();
			std::cerr << ".";
		}
		QStringList lstPhrase;
		for (int nCount = 0; nCount < nMinLen-1; ++nCount) {
			const CConcordanceEntry *pConcordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(nNormalIndex+nCount);
			if (pConcordEntry == NULL) break;
			if (bCaseSensitive) {
				lstPhrase.append(pConcordEntry->renderedWord());
			} else {
				lstPhrase.append(pConcordEntry->word().toLower());
			}
		}

		// Start with new CParsedPhrase object so that cache is clear for this search cycle:
		CParsedPhrase parsePhrase(pBibleDatabase, bCaseSensitive, bAccentSensitive);
		CSubPhrase *pSubPhrase = new CSubPhrase;	// Warning: This object gets deleted when the above parsePhrase does
		parsePhrase.attachSubPhrase(pSubPhrase);	//		as the parsePhrase takes ownership OR if calling something like ParsePhrase() which nukes it!

		int nLastOccurrences = nOccurrences;		// Previous phrase search of this entity
		for (int nCount = nMinLen; nCount < nMaxLen; ++nCount) {
			if (nLastOccurrences < nOccurrences) {
				// If not enough occurrences were found with less words,
				//		they surely won't be found with more words:
				break;
			}
			const CConcordanceEntry *pConcordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(nNormalIndex+nCount);
			if (pConcordEntry == NULL) break;
			if (bCaseSensitive) {
				lstPhrase.append(pConcordEntry->renderedWord());
			} else {
				lstPhrase.append(pConcordEntry->word().toLower());
			}

			TPhraseTag phraseTag(ndxPhrase, nCount);
			if (!pBibleDatabase->completelyContains(phraseTag)) continue;
			if (bConstrainVerses) {
				if (!pBibleDatabase->versePhraseTag(ndxPhrase).completelyContains(pBibleDatabase.data(), phraseTag)) continue;
			} else if (bConstrainChapters) {
				if (!pBibleDatabase->chapterPhraseTag(ndxPhrase).completelyContains(pBibleDatabase.data(), phraseTag)) continue;
			} else if (bConstrainBooks) {
				if (!pBibleDatabase->bookPhraseTag(ndxPhrase).completelyContains(pBibleDatabase.data(), phraseTag)) continue;
			}

			if ((nCount == nMinLen) && (nCount >= 2)) {
				pSubPhrase->ParsePhrase(lstPhrase);		// Must be SubPhrase->ParsePhrase, not parsePhrase->ParsePhrase() or else our SubPhrase object gets deleted!
				parsePhrase.FindWords();
			} else {
				pSubPhrase->AppendPhrase(lstPhrase.last());
				parsePhrase.ResumeFindWords();
			}

			TPhraseTagList lstResults(parsePhrase.GetPhraseTagSearchResults());

			// Filter for search criteria:
			if (!bSearchWithinIsEntireBible) {
				for (int nResult = lstResults.size()-1; nResult >= 0; --nResult) {		// Search backwards so we don't nullify indexes when removing entries
					if (!searchCriteria.indexIsWithin(lstResults.at(nResult).relIndex())) {
						lstResults.removeAt(nResult);
					}
				}
			}

			// Check bounds of each result for crossing book/chapter/verse constraint:
			for (int nResult = lstResults.size()-1; nResult >= 0; --nResult) {		// Search backwards so we don't nullify indexes when removing entries
				if (bConstrainVerses) {
					if (!pBibleDatabase->versePhraseTag(lstResults.at(nResult).relIndex()).completelyContains(pBibleDatabase.data(), lstResults.at(nResult))) {
						lstResults.removeAt(nResult);
					}
				} else if (bConstrainChapters) {
					if (!pBibleDatabase->chapterPhraseTag(lstResults.at(nResult).relIndex()).completelyContains(pBibleDatabase.data(), lstResults.at(nResult))) {
						lstResults.removeAt(nResult);
					}
				} else if (bConstrainBooks) {
					if (!pBibleDatabase->bookPhraseTag(lstResults.at(nResult).relIndex()).completelyContains(pBibleDatabase.data(), lstResults.at(nResult))) {
						lstResults.removeAt(nResult);
					}
				}
			}

			nLastOccurrences = lstResults.size();			// Save number of occurrences for optimizing longer phrases
			if (lstResults.size() != nOccurrences) continue;

			// If found the specified number of times, add to overall list as a PhraseEntry so we can
			//		remove duplicates, etc:
			lstOverallResults.append(lstPhrase.join(' '));
		}
		++nNormalIndex;
	}

	lstOverallResults.removeDuplicates();
	qSort(lstOverallResults.begin(), lstOverallResults.end(), ascendingLessThan);

	std::cerr << QString("\n\nFound %1 phrases having %2 occurrences:\n").arg(lstOverallResults.size()).arg(nOccurrences).toUtf8().data();
	QString strLastOutput;
	int nUniqueCount = 0;
	for (int ndx = 0; ndx < lstOverallResults.size(); ++ndx) {
		if ((!strLastOutput.isEmpty()) && (lstOverallResults.at(ndx).text().startsWith(strLastOutput))) {
			std::cout << ", ";
		} else {
			std::cout << "\n";
			strLastOutput = lstOverallResults.at(ndx).text();
			++nUniqueCount;
		}
		std::cout << lstOverallResults.at(ndx).text().toUtf8().data();
	}
	std::cout << "\n";
	std::cerr << QString("\n%1 Unique phrases found\n").arg(nUniqueCount).toUtf8().data();

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

