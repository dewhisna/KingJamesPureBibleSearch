/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/PhraseParser.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/SearchCriteria.h"
#include "../KJVCanOpener/BibleLayout.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QSet>

#include <iostream>
#include <set>
#include <algorithm>

#include "../KJVCanOpener/PathConsts.h"

#include "version.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

}	// namespace


// ============================================================================

static bool ascendingLessThan(const CPhraseEntry &s1, const CPhraseEntry &s2)
{
	return (StringParse::decompose(s1.text(), true).compare(StringParse::decompose(s2.text(), true), Qt::CaseSensitive) < 0);
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(KJVPhraseDump_VERSION);

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	int nDescriptor = -1;
	int nMinOccurrences = 2;
	int nMaxOccurrences = -1;
	constexpr int nMinLength = 2;
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
	bool bInvertCriteria = false;
	CReadDatabaseEx::DB_OVERRIDE_ENUM nDBOE = CReadDatabaseEx::DBOE_None;

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
				nMinOccurrences = strArg.toInt();
				if (nMinOccurrences < 2) nMinOccurrences = 2;
			} else if (nArgsFound == 3) {
				nMaxOccurrences = strArg.toInt();
				if ((nMaxOccurrences >= 0) && (nMaxOccurrences < nMinOccurrences)) nMaxOccurrences = nMinOccurrences;
				if (nMaxOccurrences < 0) nMaxOccurrences = -1;
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
			if (bInvertCriteria) {
				setSearchWithin.insert(CSearchCriteria::SSI_COLOPHON);
			} else {
				setSearchWithin.erase(CSearchCriteria::SSI_COLOPHON);
			}
		} else if (strArg.compare("-ss") == 0) {
			if (bInvertCriteria) {
				setSearchWithin.insert(CSearchCriteria::SSI_SUPERSCRIPTION);
			} else {
				setSearchWithin.erase(CSearchCriteria::SSI_SUPERSCRIPTION);
			}
		} else if (strArg.compare("-so") == 0) {
			for (unsigned int nBk = 1; nBk <= NUM_BK_OT; ++nBk) {
				if (bInvertCriteria) {
					setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
				} else {
					setSearchWithin.erase(CRelIndex(nBk, 0, 0, 0));
				}
			}
		} else if (strArg.compare("-sn") == 0) {
			for (unsigned int nBk = 1; nBk <= NUM_BK_NT; ++nBk) {
				if (bInvertCriteria) {
					setSearchWithin.insert(CRelIndex(nBk+NUM_BK_OT, 0, 0, 0));
				} else {
					setSearchWithin.erase(CRelIndex(nBk+NUM_BK_OT, 0, 0, 0));
				}
			}
		} else if (strArg.compare("-sa") == 0) {
			for (unsigned int nBk = 1; nBk <= NUM_BK_APOC; ++nBk) {
				if (bInvertCriteria) {
					setSearchWithin.insert(CRelIndex(nBk+NUM_BK_OT_NT, 0, 0, 0));
				} else {
					setSearchWithin.erase(CRelIndex(nBk+NUM_BK_OT_NT, 0, 0, 0));
				}
			}
		} else if (strArg.startsWith("-s")) {
			unsigned int nBk = strArg.mid(2).toUInt();
			if (bInvertCriteria) {
				setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
			} else {
				setSearchWithin.erase(CRelIndex(nBk, 0, 0, 0));
			}
		} else if (strArg.compare("-i") == 0) {
			bInvertCriteria = true;

			// Invert the current selection:
			TRelativeIndexSet setInvert;
			for (unsigned int nBk = 1; nBk <= NUM_BK; ++nBk) {
				if (setSearchWithin.find(CRelIndex(nBk, 0, 0, 0)) == setSearchWithin.end()) {
					setInvert.insert(CRelIndex(nBk, 0, 0, 0));
				}
			}
			if (setSearchWithin.find(CSearchCriteria::SSI_COLOPHON) == setSearchWithin.end())
				setInvert.insert(CSearchCriteria::SSI_COLOPHON);
			if (setSearchWithin.find(CSearchCriteria::SSI_SUPERSCRIPTION) == setSearchWithin.end())
				setInvert.insert(CSearchCriteria::SSI_SUPERSCRIPTION);

			setSearchWithin = setInvert;

		} else if (strArg.startsWith("-dbo")) {
			nDBOE = static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(strArg.mid(4).toInt());
			if ((nDBOE < 0) || (nDBOE >= CReadDatabaseEx::DBOE_COUNT)) bUnknownOption = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound < 1) || (bUnknownOption)) {
		std::cerr << KJVPhraseDump_APPNAME << " Version " << KJVPhraseDump_VERSION_SEMVER << "\n\n";
		std::cerr << QString("Usage: %1 [options] <UUID-Index> [[<Min-Occurrences>] <Max-Occurrences]\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database, dumps all phrases with at least %1 words and an occurrence\n").arg(nMinLength).toUtf8().data();
		std::cerr << QString("    count greater or equal to that specified (default 2).\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -b  =  Constrain to whole books\n").toUtf8().data();
		std::cerr << QString("  -c  =  Constrain to whole chapters (implies '-b')\n").toUtf8().data();
		std::cerr << QString("  -v  =  Constrain to whole verses (implies '-b' and '-c')\n").toUtf8().data();
		std::cerr << QString("  -pc =  Phrases are Case-Sensitive when compared (default=false)\n").toUtf8().data();
		std::cerr << QString("  -pa =  Phrases are Accent-Sensitive when compared (default=false)\n").toUtf8().data();
		std::cerr << QString("  -ph =  Phrases are Hyphen-Sensitive when compared (default=false)\n").toUtf8().data();
		std::cerr << QString("  -dbo<n> = Database Override Option\n").toUtf8().data();
		std::cerr << QString("          where <n> is one of the following:\n").toUtf8().data();
		for (int ndx = 0; ndx < CReadDatabaseEx::DBOE_COUNT; ++ndx) {
			std::cerr << QString("            %1 : %2%3\n")
							 .arg(ndx)
							 .arg(CReadDatabaseEx::dboeDescription(static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(ndx)))
							 .arg((ndx == CReadDatabaseEx::DBOE_None) ? " (default)" : "").toUtf8().data();
		}
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Search Criteria:\n").toUtf8().data();
		std::cerr << QString("  Default is to search the Entire Bible\n").toUtf8().data();
		std::cerr << QString("  -sc =  Skip Colophons (or Search Colophons if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -ss =  Skip Superscriptions (or Search Superscriptions if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -so =  Skip Old Testament (or Search Old Testament if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -sn =  Skip New Testament (or Search New Testament if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -sa =  Skip Apocrypha (or Search Apocrypha if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -sN =  Skip Book 'N', where 'N' is Book Number in Bible\n").toUtf8().data();
		std::cerr << QString("           (or Search Book 'N' if -i is used)\n").toUtf8().data();
		std::cerr << QString("   -i =  Invert search criteria so that the default is to search\n").toUtf8().data();
		std::cerr << QString("           none of the Bible except when -sX options are used to\n").toUtf8().data();
		std::cerr << QString("           select a specific Book or Testament, etc.\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Min-Occurrences : Minimum Number of occurrences phrase must exist (>=2)\n").toUtf8().data();
		std::cerr << QString("Max-Occurrences : Maximum Number of occurrences for phrase (default: infinite)\n").toUtf8().data();
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

	if (nMaxOccurrences < 0) {
		std::cerr << QString("Dumping all phrases with an occurrence count of at least %1:\n").arg(nMinOccurrences).toUtf8().data();
	} else {
		std::cerr << QString("Dumping all phrases with an occurrence count of at least %1 and no more than %2:\n").arg(nMinOccurrences).arg(nMaxOccurrences).toUtf8().data();
	}

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	TBibleDatabaseSettings bdbSettings = pBibleDatabase->settings();
	bdbSettings.setHyphenSensitive(bHyphenSensitive);
	pBibleDatabase->setSettings(bdbSettings);

	searchCriteria.setSearchWithin(setSearchWithin);
	bSearchWithinIsEntireBible = searchCriteria.withinIsEntireBible(pBibleDatabase, false);
	std::cerr << QString("Searching within %1\n").arg(searchCriteria.searchWithinDescription(pBibleDatabase)).toUtf8().data();

	// Go through entire Bible Database and find all phrases of at least two words
	//		that occur at least nMinOccurrences times:
	typedef QMap< int, CPhraseList > TMapPhraseOccurrences;		// Map of occurrence counts to Phrases found
	TMapPhraseOccurrences mapResults;
	QSet<CPhraseEntry> seen;

	uint32_t nNormalIndex = 1;
	uint32_t nBk = 0;
	uint32_t nChp = 0;
	while (nNormalIndex <= pBibleDatabase->bibleEntry().m_nNumWrd) {
		CRelIndex ndxPhrase(pBibleDatabase->DenormalizeIndex(nNormalIndex));
		if (!searchCriteria.indexIsWithin(CRelIndex(ndxPhrase.book(), 0, 0, 0))) {
			// Skip entire books we aren't searching:
			uint32_t nNewNormal = pBibleDatabase->NormalizeIndex(CRelIndex(ndxPhrase.book()+1, 0, 0, 0));
			if (nNewNormal > nNormalIndex) {
				nNormalIndex = nNewNormal;
			} else {
				// If we are at the end of the text, set the index to one beyond
				//	so we don't loop ad infinitum
				nNormalIndex = pBibleDatabase->bibleEntry().m_nNumWrd + 1;
			}
			continue;
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
		for (int nCount = 0; nCount < nMinLength; ++nCount) {
			const CConcordanceEntry *pConcordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(nNormalIndex+nCount);
			if (pConcordEntry == nullptr) break;
			if (bCaseSensitive) {
				lstPhrase.append(pConcordEntry->renderedWord());
			} else {
				lstPhrase.append(pConcordEntry->word().toLower());
			}
		}
		if (lstPhrase.size() < nMinLength) {
			++nNormalIndex;
			continue;
		}

		// Start with new CParsedPhrase object so that cache is clear for this search cycle:
		CParsedPhrase parsePhrase(pBibleDatabase, bCaseSensitive, bAccentSensitive);
		bool bFirst = true;

		int nOccurrences;
		do {
			nOccurrences = 0;		// Set to zero for checks below so continuation will break out of the loop:

			TPhraseTag phraseTag(ndxPhrase, lstPhrase.size());
			if (!pBibleDatabase->completelyContains(phraseTag)) continue;
			if (bConstrainVerses) {
				if (!pBibleDatabase->versePhraseTag(ndxPhrase).completelyContains(pBibleDatabase.data(), phraseTag)) continue;
			} else if (bConstrainChapters) {
				if (!pBibleDatabase->chapterPhraseTag(ndxPhrase).completelyContains(pBibleDatabase.data(), phraseTag)) continue;
			} else if (bConstrainBooks) {
				if (!pBibleDatabase->bookPhraseTag(ndxPhrase).completelyContains(pBibleDatabase.data(), phraseTag)) continue;
			}

			if (bFirst) {
				// It's slightly faster to call ParsePhrase on the subphrase directly
				//	than use the parsePhrase.ParsePhrase(lstPhrase, true);
				parsePhrase.primarySubPhrase()->ParsePhrase(lstPhrase);
				bFirst = false;
			} else {
				parsePhrase.primarySubPhrase()->AppendPhrase(lstPhrase.last());
			}

			CPhraseEntry phraseEntry(parsePhrase);

			if (!seen.contains(phraseEntry)) {
				parsePhrase.ResumeFindWords();

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

				nOccurrences = lstResults.size();
				if ((nOccurrences >= nMinOccurrences) &&
					((nMaxOccurrences < 0) || (nOccurrences <= nMaxOccurrences))) {
					mapResults[nOccurrences].append(phraseEntry);
					seen.insert(phraseEntry);
				}
			} else {
				// If we've already found it and are skipping our search, we
				//	must fudge our results so we don't break out of the loop.
				//	Otherwise, we'll miss longer phrases that have a similar
				//	start to an already found shorter phrase:
				nOccurrences = nMinOccurrences;
			}

			// Add next word to phrase:
			const CConcordanceEntry *pConcordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(nNormalIndex+lstPhrase.size());
			if (pConcordEntry == nullptr) break;
			if (bCaseSensitive) {
				lstPhrase.append(pConcordEntry->renderedWord());
			} else {
				lstPhrase.append(pConcordEntry->word().toLower());
			}
		} while (nOccurrences >= nMinOccurrences);

		++nNormalIndex;
	}

	std::cerr << "\n";

	for (TMapPhraseOccurrences::iterator itrResults = mapResults.begin(); itrResults != mapResults.end(); ++itrResults) {
		std::sort(itrResults.value().begin(), itrResults.value().end(), ascendingLessThan);

		QString strLastOutput;
		int nUniqueCount = 0;
		for (auto const &phrase : itrResults.value()) {
			if (strLastOutput.isEmpty() || !phrase.text().startsWith(strLastOutput)) {
				strLastOutput = phrase.text();
				++nUniqueCount;
			}
		}

		std::cout << QString("\nFound %1 Phrases (%2 Unique) with %3 occurrences:").arg(itrResults.value().size()).arg(nUniqueCount).arg(itrResults.key()).toUtf8().data();

		strLastOutput.clear();
		for (auto const &phrase : itrResults.value()) {
			if (!strLastOutput.isEmpty() && phrase.text().startsWith(strLastOutput)) {
				std::cout << ", ";
			} else {
				std::cout << "\n    ";
				strLastOutput = phrase.text();
			}
			std::cout << phrase.text().toUtf8().data();
		}
		std::cout << "\n";
	}

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

