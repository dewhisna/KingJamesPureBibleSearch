/****************************************************************************
**
** Copyright (C) 2023 Donna Whisnant, a.k.a. Dewtronics.
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

// ReadDBEx.cpp -- Code to extend ReadDB for doing special database overrides
//		and modifications during reading to experiment with various search
//		types.
//

#include "ReadDBEx.h"
#include "ParseSymbols.h"

#include <algorithm>

// ============================================================================

QString CReadDatabaseEx::dboeDescription(DB_OVERRIDE_ENUM nDBOE)
{
	static const QString arrDesc[] =
		{
			QObject::tr("Normal Unaltered Database", "ReadDBEx"),
			QObject::tr("UPPERCASE first word of each chapter", "ReadDBEx"),
			QObject::tr("UPPERCASE first word of each chapter and second word too if first word only one character", "ReadDBEx"),
		};
	static_assert(_countof(arrDesc) == DBOE_COUNT, "Invalid DB Override Descriptor Count");
	assert((nDBOE >= 0) && (nDBOE < DBOE_COUNT));
	if ((nDBOE < 0) || (nDBOE >= DBOE_COUNT)) return QString();
	return arrDesc[nDBOE];
}

// ----------------------------------------------------------------------------

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
		decltype(wordEntry.m_ndxNormalizedMapping)::size_type nSizeOld = wordEntry.m_ndxNormalizedMapping.size();
		wordEntry.m_ndxNormalizedMapping.erase(
			std::find(wordEntry.m_ndxNormalizedMapping.begin(), wordEntry.m_ndxNormalizedMapping.end(), ndxNormal));
		assert(nSizeOld == wordEntry.m_ndxNormalizedMapping.size()+1);
		Q_UNUSED(nSizeOld);		// Avoid unused warning on non-debug builds
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
	if (m_nDBOE == DBOE_None) return true;

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
		if ((m_nDBOE == DBOE_ChapterFirstWordCapitalExt) && (strFirstWordUpper.size() == 1) && (ndxSecondWord.isSet())) {
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

	// TODO :Should we resort the concordance here??  If we don't, then any UPPERCASE alternate
	//	forms will automatically be at the bottom of the sort order, which may actually
	//	be preferred so the user can see they aren't normal.

	bibleDatabase()->setRenderedWords();		// Need to update new word changes

// Dumping cache isn't needed for this app, but if we use this in the main
//	GUI, it might be:
#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
	bibleDatabase()->dumpRichVerseTextCache();
#endif

	return true;
}

// ============================================================================

