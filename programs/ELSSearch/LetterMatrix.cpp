/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
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

// ============================================================================

CLetterMatrix::CLetterMatrix(CBibleDatabasePtr pBibleDatabase, bool bSkipColophons, bool bSkipSuperscriptions)
	:	m_pBibleDatabase(pBibleDatabase),
		m_bSkipColophons(bSkipColophons),
		m_bSkipSuperscriptions(bSkipSuperscriptions)
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
	for (uint32_t normalMatrixCurrent = pBibleDatabase->NormalizeIndex(ndxMatrixCurrent);
		 normalMatrixCurrent <= normalMatrixEnd; ++normalMatrixCurrent) {
		ndxMatrixCurrent = pBibleDatabase->DenormalizeIndex(normalMatrixCurrent);

		// Transfer any pending colophon if book changes:
		if (!lstColophon.isEmpty() && (ndxMatrixLastColophon.book() != ndxMatrixCurrent.book())) {
			if (!m_bSkipColophons) {
				append(lstColophon);
			} else {
				m_mapMatrixIndexToLetterShift[size()] = lstColophon.size();
			}
			lstColophon.clear();
			ndxMatrixLastColophon.clear();
		}

		// Check for skipped superscription to map:
		if (!lstSuperscription.isEmpty() && (ndxMatrixLastSuperscription.verse() != ndxMatrixCurrent.verse())) {
			Q_ASSERT(m_bSkipSuperscriptions);		// Should only be here if actually skipping the superscriptions
			m_mapMatrixIndexToLetterShift[size()] = lstSuperscription.size();
			lstSuperscription.clear();
			ndxMatrixLastSuperscription.clear();
		}

		const CConcordanceEntry *pWordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(ndxMatrixCurrent);
		Q_ASSERT(pWordEntry != nullptr);
		if (pWordEntry) {
			const QString &strWord = pWordEntry->rawWord();

			if (!ndxMatrixCurrent.isColophon()) {
				if (!ndxMatrixCurrent.isSuperscription() || !m_bSkipSuperscriptions) {
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
		}
	}

	// Verify the matrix size as a sanity check for code bugs:
	CRelIndexEx ndxLast = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_End);
	const CConcordanceEntry *pceLastWord = pBibleDatabase->concordanceEntryForWordAtIndex(ndxLast);
	if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
	uint32_t matrixIndexLast = matrixIndexFromRelIndex(ndxLast);
	Q_ASSERT((matrixIndexLast+1) == static_cast<uint32_t>(size()));
}

// ----------------------------------------------------------------------------

uint32_t CLetterMatrix::matrixIndexFromRelIndex(const CRelIndexEx nRelIndexEx) const
{
	CRelIndex relIndex{nRelIndexEx.index()};
	const CBookEntry *pBook = m_pBibleDatabase->bookEntry(relIndex);
	if (pBook == nullptr) return 0;
	uint32_t nMatrixIndex = m_pBibleDatabase->NormalizeIndexEx(nRelIndexEx);
	if (nRelIndexEx.isColophon()) {
		Q_ASSERT(pBook->m_bHaveColophon);
		const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(relIndex);
		Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
		nMatrixIndex += (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);		// Shift colophon to end of book
	} else {
		if (pBook->m_bHaveColophon) {		// If this book has a colophon, but this index isn't it, shift this index ahead of colophon
			const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
			Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
			nMatrixIndex -= pColophonVerse->m_nNumLtr;
		}
	}

	// Note: Since the colophon transform mapping is inserted at the
	//	point where the colophon would be (after moving) in the matrix, then
	//	we must do this index transform after the other colophon transforms:
	// Note: These must be subtracted as we go as these shifts must be cumulative
	//	(unlike the other direction):
	for (TMapMatrixIndexToLetterShift::const_iterator itrXformMatrix = m_mapMatrixIndexToLetterShift.cbegin();
		 itrXformMatrix != m_mapMatrixIndexToLetterShift.cend(); ++itrXformMatrix) {
		if (itrXformMatrix.key() <= nMatrixIndex) {
			nMatrixIndex -= itrXformMatrix.value();
		} else {
			break;
		}
	}

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

	if (pBook->m_bHaveColophon) {		// Must do this even when skipping colophons to shift other indexes after colophon until we catchup with the transform above
		const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
		Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
		uint32_t nMatrixColophonNdx = m_pBibleDatabase->NormalizeIndexEx(CRelIndexEx(relIndex.book(), 0, 0, 1, 1)) +
									  (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);
		if (nMatrixIndex >= nMatrixColophonNdx) {
			if (!m_bSkipColophons) {	// Don't adjust for the colophon here if we skipped it
				nMatrixIndex -= (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);		// Shift colophon back to start of book
			}
		} else {
			nMatrixIndex += pColophonVerse->m_nNumLtr;		// Shift other indexes after colophon
		}
	}
	return m_pBibleDatabase->DenormalizeIndexEx(nMatrixIndex);
}

// ----------------------------------------------------------------------------
