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

#include "LetterMatrix.h"

#include "../KJVCanOpener/VerseRichifier.h"

#include <QStringList>

#include <iostream>

#define TEST_MATRIX_INDEXING 0		// Set to '1' to enable matrix index roundtrip testing -- WARNING: This is VERY slow test!

// ============================================================================

// Words of Jesus extractor variant of Verse Text Richifier Tags:
class CWordsOfJesusExtractor : public CVerseTextPlainRichifierTags
{
public:
	CWordsOfJesusExtractor(QStringList &lstWordsOfJesus)
		:	m_lstWordsOfJesus(lstWordsOfJesus)
	{
		setTransChangeAddedTags(QString(), QString());
	}
protected:
	virtual void wordCallback(const QString &strWord, VerseWordTypeFlags nWordTypes) const override
	{
		if (nWordTypes & VWT_WordsOfJesus) {
			m_lstWordsOfJesus.append(strWord);
		} else {
			m_lstWordsOfJesus.append(QString());
		}
	}
private:
	QStringList &m_lstWordsOfJesus;
};

// ----------------------------------------------------------------------------

CLetterMatrix::CLetterMatrix(CBibleDatabasePtr pBibleDatabase, LetterMatrixTextModifierOptionFlags flagsLMTMO)
	:	m_pBibleDatabase(pBibleDatabase),
		m_flagsLMTMO(flagsLMTMO)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Create giant array of all letters from the Bible text for speed:
	//	NOTE: This is with the entire Bible content (sans colophons/
	//	superscriptions when they are skipped) and not the search span
	//	so that we don't have to convert the matrix index based on the
	//	search span.
	reserve(pBibleDatabase->bibleEntry().m_nNumLtr + 1);		// +1 since we reserve the 0 entry
	append(QChar());
	CRelIndex ndxMatrixCurrent = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_Start);
	CRelIndex ndxMatrixEnd = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_End);
	uint32_t normalMatrixEnd = pBibleDatabase->NormalizeIndex(ndxMatrixEnd);
	QList<QChar> lstColophon;
	CRelIndex ndxMatrixLastColophon;
	QList<QChar> lstSuperscription;
	CRelIndex ndxMatrixLastSuperscription;
	uint32_t nWordsOfJesusLetterSkip = 0;		// Number of letters to skip prior to words of Jesus
	QStringList lstWordsOfJesus;
	CWordsOfJesusExtractor vtrtWordsOfJesus(lstWordsOfJesus);
	CRelIndex ndxMatrixLastPrologue;
	for (uint32_t normalMatrixCurrent = pBibleDatabase->NormalizeIndex(ndxMatrixCurrent);
		 normalMatrixCurrent <= normalMatrixEnd; ++normalMatrixCurrent) {
		ndxMatrixCurrent = pBibleDatabase->DenormalizeIndex(normalMatrixCurrent);

		// Transfer any pending colophon if book changes:
		if (!lstColophon.isEmpty() && (ndxMatrixLastColophon.book() != ndxMatrixCurrent.book())) {
			if (!m_flagsLMTMO.testFlag(LMTMO_RemoveColophons)) {
				append(lstColophon);
			} else {
				m_mapMatrixIndexToLetterShift[size()] += lstColophon.size();
			}
			lstColophon.clear();
			ndxMatrixLastColophon.clear();
		}

		// Check for skipped superscription to map:
		if (!lstSuperscription.isEmpty() && (ndxMatrixLastSuperscription.verse() != ndxMatrixCurrent.verse())) {
			Q_ASSERT(m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions));		// Should only be here if actually skipping the superscriptions
			m_mapMatrixIndexToLetterShift[size()] += lstSuperscription.size();
			lstSuperscription.clear();
			ndxMatrixLastSuperscription.clear();
		}

		// Add prologues first:
		bool bAddedPrologue = false;
		if (ndxMatrixCurrent.book() != ndxMatrixLastPrologue.book()) {		// Check entering new book
			const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxMatrixCurrent);
			Q_ASSERT(pBook != nullptr);
			if (pBook && !pBook->m_strPrologue.isEmpty()) {
				if (m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues) && !m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
					for (auto const &chrLetter : pBook->m_strPrologue) append(chrLetter);
				} else {
					if (!pBook->m_strPrologue.isEmpty()) {
						m_mapMatrixIndexToLetterShift[size()] += pBook->m_strPrologue.size();
					}
				}
			}
			bAddedPrologue = true;
			ndxMatrixLastPrologue.setChapter(0);		// Force enter new chapter if this is a new book -- without this, we miss chapters of books with only 1 chapter
		}
		if ((ndxMatrixCurrent.chapter() != ndxMatrixLastPrologue.chapter()) &&
			(ndxMatrixCurrent.chapter() != 0)) {							// Check entering new chapter
			const CChapterEntry *pChapter = pBibleDatabase->chapterEntry(ndxMatrixCurrent);
			Q_ASSERT(pChapter != nullptr);
			if (pChapter && !pChapter->m_strPrologue.isEmpty()) {
				if (m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues) && !m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
					for (auto const &chrLetter : pChapter->m_strPrologue) append(chrLetter);
				} else {
					if (!pChapter->m_strPrologue.isEmpty()) {
						m_mapMatrixIndexToLetterShift[size()] += pChapter->m_strPrologue.size();
					}
				}
			}
			bAddedPrologue = true;
		}
		if (bAddedPrologue) ndxMatrixLastPrologue = ndxMatrixCurrent;

		const CConcordanceEntry *pWordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(ndxMatrixCurrent);
		Q_ASSERT(pWordEntry != nullptr);
		if (pWordEntry) {
			const QString &strWord = pWordEntry->rawWord();

			if (!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
				// Since the Words of Jesus can never be in Colophons or Superscriptions,
				//	we only need to shift the colophons to their correct position when
				//	searching everything:
				if (!ndxMatrixCurrent.isColophon()) {
					if (!ndxMatrixCurrent.isSuperscription() || !m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions)) {
						// Output the book as-is without shuffling:
						for (auto const &chrLetter : strWord) append(chrLetter);
					} else {
						// If skipping superscriptions, put them on a local buffer like
						//	we did with colophons so that when we hit the next "verse"
						//	(i.e. exit the superscription), we can insert a MatrixIndex
						//	to LetterShift mapping.  Note that unlike colophons, since
						//	we aren't moving the superscription itself, this buffer is
						//	never transferred to the matrix:
						for (auto const &chrLetter : strWord) lstSuperscription.append(chrLetter);
						ndxMatrixLastSuperscription = ndxMatrixCurrent;
					}
				} else {
					// Output colophon to temp buff:
					for (auto const &chrLetter : strWord) lstColophon.append(chrLetter);
					ndxMatrixLastColophon = ndxMatrixCurrent;
				}
			} else {
				// Trigger per-verse rendering on first word of verse:
				if (ndxMatrixCurrent.word() == 1) {
					const CVerseEntry *pVerse = pBibleDatabase->verseEntry(ndxMatrixCurrent);
					Q_ASSERT(pVerse != nullptr);
					if (pVerse == nullptr) continue;

					lstWordsOfJesus.clear();
					pBibleDatabase->richVerseText(ndxMatrixCurrent, vtrtWordsOfJesus);
					Q_ASSERT(pVerse->m_nNumWrd == static_cast<unsigned int>(lstWordsOfJesus.size()));
				}
				Q_ASSERT(static_cast<unsigned int>(lstWordsOfJesus.size()) >= ndxMatrixCurrent.word());
				Q_ASSERT(ndxMatrixCurrent.word() >= 1);

				if (!lstWordsOfJesus.at(ndxMatrixCurrent.word()-1).isEmpty()) {
					// If in Words of Jesus, output letter skip for main text and append words to matrix:
					if (nWordsOfJesusLetterSkip) {
						m_mapMatrixIndexToLetterShift[size()] += nWordsOfJesusLetterSkip;
						nWordsOfJesusLetterSkip = 0;
					}
					for (auto const &chrLetter : strWord) append(chrLetter);
				} else {
					// Otherwise, plan to skip the letters that aren't:
					nWordsOfJesusLetterSkip += strWord.size();
				}
			}
		}
	}
	if (m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && (nWordsOfJesusLetterSkip != 0)) {
		m_mapMatrixIndexToLetterShift[size()] += nWordsOfJesusLetterSkip;
		nWordsOfJesusLetterSkip = 0;
	}

	// Verify the matrix size as a sanity check for code bugs:
	CRelIndexEx ndxLast = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_End);
	const CConcordanceEntry *pceLastWord = pBibleDatabase->concordanceEntryForWordAtIndex(ndxLast);
	if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
	uint32_t matrixIndexLast = matrixIndexFromRelIndex(ndxLast);
	Q_ASSERT((matrixIndexLast+1) == static_cast<uint32_t>(size()));

	// Additional Matrix Index Roundtrip Test for debugging:
#if TEST_MATRIX_INDEXING
	runMatrixIndexRoundtripTest();
#endif
}

bool CLetterMatrix::runMatrixIndexRoundtripTest() const
{
	for (uint32_t ndx = 1; ndx < static_cast<uint32_t>(size()); ++ndx) {
#if !TEST_MATRIX_INDEXING
		if ((ndx % 10000) == 0) {
			std::cerr << ".";
			std::cerr.flush();
		}
#endif
		CRelIndexEx relIndex = relIndexFromMatrixIndex(ndx);
		uint32_t ndxTest = matrixIndexFromRelIndex(relIndex);		// Separate variable for debug viewing
		if (ndx != ndxTest) {
			QString strTemp;
			for (uint32_t i = 0; ((i < 32) && (i < static_cast<uint32_t>(size()))); ++i) strTemp += at(ndx+i);
#if TEST_MATRIX_INDEXING
			qDebug("Real Index:     %d : %s", ndx, strTemp.toUtf8().data());
			qDebug("relIndex: 0x%s", QString("%1").arg(relIndex.index(), 8, 16, QChar('0')).toUpper().toUtf8().data());
#else
			std::cerr << "\n";
			std::cerr << "Real Index:     " << (unsigned int)(ndx) << " : " << strTemp.toUtf8().data() << std::endl;
			std::cerr << "relIndex: 0x" << QString("%1").arg(relIndex.index(), 8, 16, QChar('0')).toUpper().toUtf8().data() << std::endl;
#endif
			strTemp.clear();
			for (uint32_t i = 0; ((i < 32) && (i < static_cast<uint32_t>(size()))); ++i) strTemp += at(ndxTest+i);
#if TEST_MATRIX_INDEXING
			qDebug("Resolved Index: %d : %s", ndxTest, strTemp.toUtf8().data());
#else
			std::cerr << "Resolved Index: " << (unsigned int)(ndxTest) << " : " << strTemp.toUtf8().data() << std::endl;
#endif

			// Perform again as a convenient place to attach a breakpoint and watch what happens:
			CRelIndexEx relRedo = relIndexFromMatrixIndex(ndx);
			uint32_t ndxTest2 = matrixIndexFromRelIndex(relRedo);
			Q_UNUSED(ndxTest2);
#if TEST_MATRIX_INDEXING
			Q_ASSERT(false);
#endif
			return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------------

uint32_t CLetterMatrix::matrixIndexFromRelIndex(const CRelIndexEx nRelIndexEx) const
{
	CRelIndex relIndex{nRelIndexEx.index()};
	const CBookEntry *pBook = m_pBibleDatabase->bookEntry(relIndex);
	if (pBook == nullptr) return 0;
	// Note: MatrixIndex must be signed here in case the part being eliminated from
	//	the text is larger than the text before the part being kept (like in the
	//	Words of Jesus only mode):
	int32_t nMatrixIndex = m_pBibleDatabase->NormalizeIndexEx(nRelIndexEx);
	if (!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
		if (nRelIndexEx.isColophon()) {
			Q_ASSERT(pBook->m_bHaveColophon);
			const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(relIndex);
			Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
			// Note: BookPrologue must be included since it is included in the book letter
			//	count, but not any verse letter count, including the colophon:
			uint32_t nColophonShift = (pBook->m_nNumLtr - pBook->m_strPrologue.size() - pColophonVerse->m_nNumLtr);
			nMatrixIndex += nColophonShift;		// Shift colophon to end of book
		} else {
			if (pBook->m_bHaveColophon && !nRelIndexEx.isBookPrologue()) {		// If this book has a colophon, but this index isn't it, shift this index ahead of colophon
				const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
				Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
				nMatrixIndex -= pColophonVerse->m_nNumLtr;
			}
		}

		if (!m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues) && nRelIndexEx.isBookPrologue()) {
			// If not including the book prologues, add any letters that
			//	would normally be processed there so that the shift below
			//	doesn't move the index backward into the previous book:
			nMatrixIndex += pBook->m_strPrologue.size();
		}

		if (!m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues) && nRelIndexEx.isChapterPrologue()) {
			// If not including the chapter prologues, add any letters that
			//	would normally be processed there so that the shift below
			//	doesn't move the index backward into the previous book:
			const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(relIndex);
			Q_ASSERT(pChapter != nullptr);
			nMatrixIndex += pChapter->m_strPrologue.size();
		}
	}

	// Note: Since the colophon transform mapping is inserted at the
	//	point where the colophon would be (after moving) in the matrix, then
	//	we must do this index transform after the other colophon transforms:
	// Note: These must be subtracted as we go as these shifts must be cumulative
	//	(unlike the other direction):
	for (TMapMatrixIndexToLetterShift::const_iterator itrXformMatrix = m_mapMatrixIndexToLetterShift.cbegin();
		 itrXformMatrix != m_mapMatrixIndexToLetterShift.cend(); ++itrXformMatrix) {
		if (static_cast<int32_t>(itrXformMatrix.key()) <= nMatrixIndex) {
			nMatrixIndex -= itrXformMatrix.value();
		} else {
			break;
		}
	}

	if (nMatrixIndex < 0) nMatrixIndex = 1;		// Return first active position if real position is before anything in the matrix
	return nMatrixIndex;
}

CRelIndexEx CLetterMatrix::relIndexFromMatrixIndex(uint32_t nMatrixIndex) const
{
	// Note: Since the colophon transform mapping is inserted at the
	//	point where the colophon would be (after moving) in the matrix, then
	//	we must do this index transform prior to other colophon transforms.
	//	This is also needed for the logic below where we do a denormalization
	//	of MatrixIndex and it needs to have the correct number of letters prior
	//	to this point:
	// Note: These must be added after we've accumulated the total as these
	//	shouldn't be cumulative (unlike the other direction):
	uint32_t nLetterShift = 0;
	for (TMapMatrixIndexToLetterShift::const_iterator itrXformMatrix = m_mapMatrixIndexToLetterShift.cbegin();
		 itrXformMatrix != m_mapMatrixIndexToLetterShift.cend(); ++itrXformMatrix) {
		if (itrXformMatrix.key() <= nMatrixIndex) {
			nLetterShift += itrXformMatrix.value();
		} else {
			break;
		}
	}
	nMatrixIndex += nLetterShift;

	// Since we are only shifting the colophons from the beginning of each book
	//	to the end of each book, the number of letters in each book should be
	//	the same.  Therefore, the standard Denormalize call should give the same
	//	book:
	CRelIndex relIndex{m_pBibleDatabase->DenormalizeIndexEx(nMatrixIndex)};
	const CBookEntry *pBook = m_pBibleDatabase->bookEntry(relIndex);
	Q_ASSERT(pBook != nullptr);  if (pBook == nullptr) return CRelIndexEx();

	if (!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
		if (pBook->m_bHaveColophon) {		// Must do this even when skipping colophons to shift other indexes after colophon until we catchup with the transform above
			const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
			Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
			uint32_t nMatrixOldColophonNdx = pColophonVerse->m_nLtrAccum + 1;
			// Note: BookPrologue must be included since it is included in the book letter
			//	count, but not any verse letter count, including the colophon:
			uint32_t nColophonShift = (pBook->m_nNumLtr - pBook->m_strPrologue.size() - pColophonVerse->m_nNumLtr);
			uint32_t nMatrixNewColophonNdx = nMatrixOldColophonNdx + nColophonShift;
			if (nMatrixIndex >= nMatrixNewColophonNdx) {
				if (!m_flagsLMTMO.testFlag(LMTMO_RemoveColophons)) {	// Don't adjust for the colophon here if we skipped it
					nMatrixIndex -= nColophonShift;						// Shift colophon back to start of book
				}
			} else {
				if (nMatrixIndex >= nMatrixOldColophonNdx) {
					nMatrixIndex += pColophonVerse->m_nNumLtr;		// Shift other indexes after colophon (as long as the index isn't part of the prologue)
				}
			}
		}
	}
	return m_pBibleDatabase->DenormalizeIndexEx(nMatrixIndex);
}

// ----------------------------------------------------------------------------

