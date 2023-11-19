/****************************************************************************
**
** Copyright (C) 2014-2022 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/PhraseNavigator.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/ParseSymbols.h"

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
#include <algorithm>

#include "../KJVCanOpener/PathConsts.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 20000;		// Version 1.0.0

}	// namespace

// ============================================================================

class CReadDatabaseEx : public CReadDatabase
{
public:
	enum DB_OVERRIDE_ENUM {
		DOE_None = 0,
		DOE_ChapterFirstWordCapital = 1,		// UPPERCASE first word of each chapter
		DOE_ChapterFirstWordCapitalExt = 2,		// UPPERCASE first word of each chapter and second word too if first word is only a single character
		// ----
		DOE_COUNT
	};

	static QString doeDescription(DB_OVERRIDE_ENUM nOverride)
	{
		static const QString arrDesc[] =
		{
				"Normal Unaltered Database",
				"UPPERCASE first word of each chapter",
				"UPPERCASE first word of each chapter and second word too if first word only one character",
		};
		static_assert(_countof(arrDesc) == DOE_COUNT, "Invalid DB Override Descriptor Count");
		assert((nOverride >= 0) && (nOverride < DOE_COUNT));
		if ((nOverride < 0) || (nOverride >= DOE_COUNT)) return QString();
		return arrDesc[nOverride];
	}

	// ------------------------------------------------------------------------

	CReadDatabaseEx(DB_OVERRIDE_ENUM nDOE)
		:	m_nDOE(nDOE)
	{ }
	virtual bool ReadBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain = false) override;

protected:
	DB_OVERRIDE_ENUM m_nDOE = DOE_None;
};

bool CReadDatabaseEx::ReadBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain)
{
	// fnChangeWordAt changes the wordEntry at ndxNormal from strOldWord to strNewWord:
	//	Return std::pair of a boolean flag that's true if this is a brand new word form (else false)
	//		and the index of altWord in the updated wordEntry:
	typedef std::pair<bool, int> TWordChange;
	auto &&fnChangeWordAt = [](CWordEntry &wordEntry, const QString &strOldWord, const QString &strNewWord, uint32_t ndxNormal)->TWordChange {
		int ndxOldWord = wordEntry.m_lstAltWords.indexOf(strOldWord);
		assert(ndxOldWord != -1);			// Error if OldWord not found!

		if (strOldWord == strNewWord) return TWordChange(false, ndxOldWord);			// Change only if it needs it

		// Find and remove the index for the OldWord:
		int ndxOldNormalStart = 0;
		for (int ndx = 0; ndx < ndxOldWord; ++ndx) ndxOldNormalStart += wordEntry.m_lstAltWordCount.at(ndx);
		// Make sure the specified normal index is associated with the OldWord:
		assert(std::find(wordEntry.m_ndxNormalizedMapping.begin()+ndxOldNormalStart,
					wordEntry.m_ndxNormalizedMapping.begin()+ndxOldNormalStart+wordEntry.m_lstAltWordCount.at(ndxOldWord),
					ndxNormal) != wordEntry.m_ndxNormalizedMapping.begin()+ndxOldNormalStart+wordEntry.m_lstAltWordCount.at(ndxOldWord));
		// Remove Old Word Normal Entry:
		unsigned int nSizeOld = wordEntry.m_ndxNormalizedMapping.size();
		wordEntry.m_ndxNormalizedMapping.erase(
				std::find(wordEntry.m_ndxNormalizedMapping.begin(), wordEntry.m_ndxNormalizedMapping.end(), ndxNormal));
		assert(nSizeOld == wordEntry.m_ndxNormalizedMapping.size()+1);
		// Decrement count for this word:
		wordEntry.m_lstAltWordCount[ndxOldWord] = wordEntry.m_lstAltWordCount.at(ndxOldWord) - 1;

		// Add the index for the NewWord:
		int ndxNewWord = wordEntry.m_lstAltWords.indexOf(strNewWord);
		bool bIsNewWord = false;
		if (ndxNewWord != -1) {
			// If the new form is already an alternate, set its index:
			int ndxNewNormalStart = 0;
			for (int ndx = 0; ndx < ndxNewWord; ++ndx) ndxNewNormalStart += wordEntry.m_lstAltWordCount.at(ndx);
			for (unsigned int ndx = 0; ndx < wordEntry.m_lstAltWordCount.at(ndxNewWord); ++ndx) {
				if (wordEntry.m_ndxNormalizedMapping.at(ndxNewNormalStart) > ndxNormal) break;
				++ndxNewNormalStart;
			}
			// Insert New Word Normal Entry:
			wordEntry.m_ndxNormalizedMapping.insert(wordEntry.m_ndxNormalizedMapping.begin()+ndxNewNormalStart, ndxNormal);
			// Increment count for this word:
			wordEntry.m_lstAltWordCount[ndxNewWord] = wordEntry.m_lstAltWordCount.at(ndxNewWord) + 1;
		} else {
			// If the new form isn't in the list, add it:
			bIsNewWord = true;
			ndxNewWord = wordEntry.m_lstAltWords.size();
			wordEntry.m_lstAltWords.push_back(strNewWord);
			wordEntry.m_lstAltWordCount.push_back(1);			// One Normalized index for this new word
			wordEntry.m_ndxNormalizedMapping.push_back(ndxNormal);

			QString strSearchWord = StringParse::deCantillate(strNewWord);
			wordEntry.m_lstSearchWords.push_back(strSearchWord);
			wordEntry.m_lstDecomposedAltWords.push_back(StringParse::decompose(strNewWord, true));
			wordEntry.m_lstDecomposedHyphenAltWords.push_back(StringParse::decompose(strNewWord, false));
			wordEntry.m_lstDeApostrAltWords.push_back(StringParse::deApostrHyphen(strSearchWord, true));
			wordEntry.m_lstDeApostrHyphenAltWords.push_back(StringParse::deApostrHyphen(strSearchWord, false));
			wordEntry.m_lstRenderedAltWords.push_back(QString());		// Placeholder to be filled in below with the call to setRenderedWords()

#ifdef USE_GEMATRIA
			if (TBibleDatabaseList::useGematria()) {
				wordEntry.m_lstGematria.push_back(CGematriaCalc(bibleDatabase()->langID(), strNewWord));
			}
#endif
		}

		return TWordChange(bIsNewWord, ndxNewWord);
	};


	if (!CReadDatabase::ReadBibleDatabase(bblDesc, bSetAsMain)) return false;
	if (m_nDOE == DOE_None) return true;

	// Modify database based on override option:
	// Step through each chapter of the Bible and modify initial word to be all UPPERCASE:
	CRelIndex ndxCurrent = bibleDatabase()->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_Start);		// Index into first word of each chapter

	while (ndxCurrent.isSet()) {
		uint32_t ndxCurrentNormalized = bibleDatabase()->NormalizeIndex(ndxCurrent);
		// Make the first word of the chapter UPPERCASE:
		int nFirstWordConcordanceIndex = bibleDatabase()->concordanceIndexForWordAtIndex(ndxCurrentNormalized);
		assert(nFirstWordConcordanceIndex != -1);
		CConcordanceEntry &firstWordConcordanceEntry = bibleDatabase()->m_lstConcordanceWords[nFirstWordConcordanceIndex];
		QString strFirstWord = bibleDatabase()->wordAtIndex(ndxCurrentNormalized, WTE_COMPLETE);		// Get first word of chapter
		QString strFirstWordUpper = strFirstWord.toUpper();				// Make it UPPERCASE
		TWordListMap::iterator itrFirstWordEntry = bibleDatabase()->m_mapWordList.find(firstWordConcordanceEntry.m_itrEntryWord->first);
		assert(itrFirstWordEntry != bibleDatabase()->m_mapWordList.end());
		if (itrFirstWordEntry != bibleDatabase()->m_mapWordList.end()) {
			// Modify the wordEntry to rewrite this word to an alternate form of all UPPERCASE
			TWordChange wcNewAltWord = fnChangeWordAt(itrFirstWordEntry->second, strFirstWord, strFirstWordUpper, ndxCurrentNormalized);
			// Update concordance if the word was changed:
			if (wcNewAltWord.first) {
				// If this is a new word form, add new concordance entry for it:
				bibleDatabase()->m_lstConcordanceMapping[ndxCurrentNormalized] = bibleDatabase()->m_lstConcordanceWords.size();
				bibleDatabase()->m_lstConcordanceWords.push_back(CConcordanceEntry(itrFirstWordEntry, wcNewAltWord.second));
			} else {
				// If not a new form, then we must already have a concordance entry for this
				//	form, so find it and point this one to it:
				for (TNormalizedIndexList::size_type ndxConcord = 0;
							(ndxConcord < itrFirstWordEntry->second.m_ndxNormalizedMapping.size()); ++ndxConcord) {
					if (bibleDatabase()->m_lstConcordanceWords.at(bibleDatabase()->m_lstConcordanceMapping.at(itrFirstWordEntry->second.m_ndxNormalizedMapping.at(ndxConcord))).m_nAltWordIndex == wcNewAltWord.second) {
						bibleDatabase()->m_lstConcordanceMapping[ndxCurrentNormalized] = bibleDatabase()->m_lstConcordanceMapping.at(itrFirstWordEntry->second.m_ndxNormalizedMapping.at(ndxConcord));
						break;
					}
				}
			}
		}

		// Check if doing the extended replacement to the second word:
		CRelIndex ndxSecondWord = bibleDatabase()->calcRelIndex(ndxCurrent, CBibleDatabase::RIME_NextWord);
		uint32_t ndxSecondNormalized = bibleDatabase()->NormalizeIndex(ndxSecondWord);
		if ((m_nDOE == DOE_ChapterFirstWordCapitalExt) && (strFirstWordUpper.size() == 1) && (ndxSecondWord.isSet())) {
			int nSecondWordConcordanceIndex = bibleDatabase()->concordanceIndexForWordAtIndex(ndxSecondNormalized);
			assert(nSecondWordConcordanceIndex != -1);
			CConcordanceEntry &secondWordConcordanceEntry = bibleDatabase()->m_lstConcordanceWords[nSecondWordConcordanceIndex];
			QString strSecondWord = bibleDatabase()->wordAtIndex(ndxSecondNormalized, WTE_COMPLETE);		// Get first word of chapter
			QString strSecondWordUpper = strSecondWord.toUpper();				// Make it UPPERCASE
			TWordListMap::iterator itrSecondWordEntry = bibleDatabase()->m_mapWordList.find(secondWordConcordanceEntry.m_itrEntryWord->first);
			assert(itrSecondWordEntry != bibleDatabase()->m_mapWordList.end());
			if (itrSecondWordEntry != bibleDatabase()->m_mapWordList.end()) {
				// Modify the wordEntry to rewrite this word to an alternate form of all UPPERCASE
				TWordChange wcNewAltWord = fnChangeWordAt(itrSecondWordEntry->second, strSecondWord, strSecondWordUpper, ndxSecondNormalized);
				// Update concordance if the word was changed:
				if (wcNewAltWord.first) {
					// If this is a new word form, add new concordance entry for it:
					bibleDatabase()->m_lstConcordanceMapping[ndxSecondNormalized] = bibleDatabase()->m_lstConcordanceWords.size();
					bibleDatabase()->m_lstConcordanceWords.push_back(CConcordanceEntry(itrSecondWordEntry, wcNewAltWord.second));
				} else {
					// If not a new form, then we must already have a concordance entry for this
					//	form, so find it and point this one to it:
					for (TNormalizedIndexList::size_type ndxConcord = 0;
								(ndxConcord < itrSecondWordEntry->second.m_ndxNormalizedMapping.size()); ++ndxConcord) {
						if (bibleDatabase()->m_lstConcordanceWords.at(bibleDatabase()->m_lstConcordanceMapping.at(itrSecondWordEntry->second.m_ndxNormalizedMapping.at(ndxConcord))).m_nAltWordIndex == wcNewAltWord.second) {
							bibleDatabase()->m_lstConcordanceMapping[ndxSecondNormalized] = bibleDatabase()->m_lstConcordanceMapping.at(itrSecondWordEntry->second.m_ndxNormalizedMapping.at(ndxConcord));
							break;
						}
					}
				}
			}
		}

		// Goto Next Chapter:
		ndxCurrent = bibleDatabase()->calcRelIndex(ndxCurrent, CBibleDatabase::RIME_NextChapter);
	}

	// Should we resort the concordance here??  If we don't, then any UPPERCASE alternate
	//	forms will automatically be at the bottom of the sort order, which may actually
	//	be preferred so the user can see they aren't normal.

	bibleDatabase()->setRenderedWords();		// Need to update new word changes

	// Dumping cache isn't needed for this app, but if we move this to the main
	//	GUI, it might be:
#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
	bibleDatabase()->dumpRichVerseTextCache();
#endif

	return true;
}

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
	CReadDatabaseEx::DB_OVERRIDE_ENUM nDOE = CReadDatabaseEx::DOE_None;

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
		} else if (strArg.startsWith("-do")) {
			nDOE = static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(strArg.mid(3).toInt());
			if ((nDOE < 0) || (nDOE >= CReadDatabaseEx::DOE_COUNT)) bUnknownOption = true;
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
		std::cerr << QString("  -do<n> = Database Override Option\n").toUtf8().data();
		std::cerr << QString("          where <n> is one of the following:\n").toUtf8().data();
		for (int ndx = 0; ndx < CReadDatabaseEx::DOE_COUNT; ++ndx) {
			std::cerr << QString("            %1 : %2%3\n")
							.arg(ndx)
							.arg(CReadDatabaseEx::doeDescription(static_cast<CReadDatabaseEx::DB_OVERRIDE_ENUM>(ndx)))
							.arg((ndx == CReadDatabaseEx::DOE_None) ? " (default)" : "").toUtf8().data();
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

	std::cerr << QString("Reading database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();
	std::cerr << QString("Database Option: %1\n").arg(CReadDatabaseEx::doeDescription(nDOE)).toUtf8().data();

	CReadDatabaseEx rdbMain(nDOE);
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

