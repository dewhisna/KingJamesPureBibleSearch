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

#include "FindELS.h"
#include "LetterMatrix.h"

// ============================================================================

// Concurrent Threading function to locate the ELS entries for a single skip distance:
CELSResultList findELS(int nSkip, const CLetterMatrix &letterMatrix,
				  const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev,
				  unsigned int nBookStart, unsigned int nBookEnd)
{
	// Results storage (for this skip):
	CELSResultList lstResults;

	// Get maximum search word lengths to know bounds of search:
	int nMaxLength = lstSearchWords.last().size();

	// Compute starting index for first letter in the search range:
	uint32_t matrixIndexCurrent = letterMatrix.matrixIndexFromRelIndex(CRelIndexEx(nBookStart, 1, 0, 1, 1));

	// Compute ending index for the last letter in the search range:
	CRelIndexEx ndxLast = letterMatrix.bibleDatabase()->calcRelIndex(CRelIndex(nBookEnd, 0, 0, 0), CBibleDatabase::RIME_EndOfBook);
	const CConcordanceEntry *pceLastWord = letterMatrix.bibleDatabase()->concordanceEntryForWordAtIndex(ndxLast);
	if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
	uint32_t matrixIndexLast = letterMatrix.matrixIndexFromRelIndex(ndxLast);

	while (matrixIndexCurrent <= matrixIndexLast) {
		int ndxSearchWord = 0;				// Index to current search word being tested
		for (int nLen = lstSearchWords.at(ndxSearchWord).size(); nLen <= nMaxLength; ++nLen) {
			if (ndxSearchWord >= lstSearchWords.size()) break;
			while ((ndxSearchWord < lstSearchWords.size()) &&					// Find next word in list at least nLen long
				   (lstSearchWords.at(ndxSearchWord).size() < nLen)) {
				++ndxSearchWord;
			}
			if (lstSearchWords.at(ndxSearchWord).size() > nLen) continue;		// Find length of next longest word in the list
			uint32_t matrixIndexNext = matrixIndexCurrent + (nSkip*(nLen-1)) + nLen - 1;
			if (matrixIndexNext > matrixIndexLast) continue;		// Stop if the search would run off the end of the text

			QString strWord;
			uint32_t matrixIndexLetter = matrixIndexCurrent;		// MatrixIndex for the current letter being extracted
			for (int i = 0; i < nLen; ++i) {
				strWord += letterMatrix.at(matrixIndexLetter);
				matrixIndexLetter += nSkip + 1;
			}

			for (int ndxWord = ndxSearchWord; ndxWord < lstSearchWords.size(); ++ndxWord) {
				if (lstSearchWords.at(ndxWord).size() != nLen) break;				// Check all words of this length only and exit when we hit a longer word
				if (strWord.compare(lstSearchWords.at(ndxWord)) == 0) {				// Check forward direction
					CELSResult result;
					result.m_strWord = strWord;
					result.m_nSkip = nSkip;
					result.m_ndxStart = letterMatrix.relIndexFromMatrixIndex(matrixIndexCurrent);
					result.m_nDirection = Qt::LeftToRight;
					lstResults.append(result);
				} else if (strWord.compare(lstSearchWordsRev.at(ndxWord)) == 0) {	// Check reverse direction
					CELSResult result;
					result.m_strWord = lstSearchWords.at(ndxWord);		// Result is always forward ordered word
					result.m_nSkip = nSkip;
					result.m_ndxStart = letterMatrix.relIndexFromMatrixIndex(matrixIndexCurrent);
					result.m_nDirection = Qt::RightToLeft;
					lstResults.append(result);
				}
			}
		}

		++matrixIndexCurrent;
	}

	return lstResults;
}

// ============================================================================
