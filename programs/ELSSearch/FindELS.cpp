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

#include <algorithm>		// for std::sort
#include <functional>		// for std::bind

// ============================================================================

int CFindELS::g_conFibonacciCast9[8][24] = { };

static int castOut9(int nVal)
{
	while (nVal >= 10) {
		nVal = (nVal / 10) + (nVal % 10);
	}
	return nVal;
}

void CFindELS::initFibonacciCast9Table()
{
	if (g_conFibonacciCast9[0][0]) return;		// Non-zero values means it's already initialized

	// Compute base fibonacci values:
	int n[3] = { 1, 1, 2 };
	for (int j = 0; j < 24; ++j) {
		g_conFibonacciCast9[0][j] = castOut9(n[0]);
		n[0] = n[1];
		n[1] = n[2];
		n[2] = n[0] + n[1];
	}

	// Compute C9 multipliers:
	for (int i = 1; i < 8; ++i) {
		for (int j = 0; j < 24; ++j) {
			g_conFibonacciCast9[i][j] = castOut9(g_conFibonacciCast9[0][j] * (i+1));
		}
	}
}

// Compute max distance for a given word length for a given search type:
int CFindELS::maxDistance(int nSkip, int nLen, ELS_SEARCH_TYPE_ENUM nSearchType)
{
	Q_ASSERT(nLen > 1);
	int nDist = 0;

	if (nSearchType == ESTE_ELS) {
		nDist = (nSkip*(nLen-1));
	} else if (nSearchType == ESTE_FLS) {
		int n[3] = { 1, 1, 2 };
		for (int ndx = 0; ndx < (nLen-1); ++ndx) {
			nDist += (n[0] * nSkip);
			n[0] = n[1];
			n[1] = n[2];
			n[2] = n[0] + n[1];
		}
	} else {				// FLS Vortex Types:
		for (int ndx = 0; ndx < (nLen-1); ++ndx) {
			nDist += g_conFibonacciCast9[(nSkip-1) % 8][ndx % 24];
		}
	}

	nDist += nLen;			// Source letters themselves

	return nDist;
}

// Compute next skip for a given word letter index for a given search type:
int CFindELS::nextOffset(int nSkip, int nLetterPos, ELS_SEARCH_TYPE_ENUM nSearchType)
{
	int nOffset = 1;		// 1 since offset is at least to next letter in word

	if (nSearchType == ESTE_ELS) {
		nOffset += nSkip;
	} else if (nSearchType == ESTE_FLS) {
		int n[3] = { 1, 1, 2 };
		for (int ndx = 0; ndx < nLetterPos; ++ndx) {
			n[0] = n[1];
			n[1] = n[2];
			n[2] = n[0] + n[1];
		}
		nOffset += n[0] * nSkip;
	} else {				// FLS Vortex Types:
		nOffset += g_conFibonacciCast9[(nSkip-1) % 8][nLetterPos % 24];
	}

	return nOffset;
}

// Concurrent Threading function to locate the ELS entries for a single skip distance:
CELSResultList CFindELS::findELS(int nSkip, const CLetterMatrix &letterMatrix,
				  const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev,
				  unsigned int nBookStart, unsigned int nBookEnd, ELS_SEARCH_TYPE_ENUM nSearchType)
{
	// Results storage (for this skip):
	CELSResultList lstResults;

	// Get maximum search word lengths to know bounds of search:
	int nMaxLength = lstSearchWords.last().size();

	// Compute starting index for first letter in the search range:
	uint32_t matrixIndexCurrent = letterMatrix.matrixIndexFromRelIndex(CRelIndexEx(nBookStart, 0, 0, 0, 1));
	if (matrixIndexCurrent == 0) return CELSResultList();

	// Compute ending index for the last letter in the search range:
	CRelIndexEx ndxLast = letterMatrix.bibleDatabase()->calcRelIndex(CRelIndex(nBookEnd, 0, 0, 0), CBibleDatabase::RIME_EndOfBook);
	const CConcordanceEntry *pceLastWord = letterMatrix.bibleDatabase()->concordanceEntryForWordAtIndex(ndxLast);
	if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
	const CBookEntry *pBook = letterMatrix.bibleDatabase()->bookEntry(ndxLast);
	Q_ASSERT(pBook != nullptr);
	if (pBook && pBook->m_bHaveColophon && !letterMatrix.skipColophons()) {
		// If this book has a colophon and we aren't skipping them, then the
		//	matrix will have moved the colophon to the end of the book so instead
		//	of the last letter of the last word of the last verse of the last
		//	chapter of the book (as above), we need to move to the last letter of
		//	the last word of the colophon:
		ndxLast = letterMatrix.bibleDatabase()->calcRelIndex(CRelIndex(nBookEnd, 0, 0, 1), CBibleDatabase::RIME_EndOfVerse);
		pceLastWord = letterMatrix.bibleDatabase()->concordanceEntryForWordAtIndex(ndxLast);
		if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
	}
	uint32_t matrixIndexLast = letterMatrix.matrixIndexFromRelIndex(ndxLast);
	if (matrixIndexCurrent == 0) return CELSResultList();

	while (matrixIndexCurrent <= matrixIndexLast) {
		int ndxSearchWord = 0;				// Index to current search word being tested
		for (int nLen = lstSearchWords.at(ndxSearchWord).size(); nLen <= nMaxLength; ++nLen) {
			if (ndxSearchWord >= lstSearchWords.size()) break;
			while ((ndxSearchWord < lstSearchWords.size()) &&					// Find next word in list at least nLen long
				   (lstSearchWords.at(ndxSearchWord).size() < nLen)) {
				++ndxSearchWord;
			}
			if (lstSearchWords.at(ndxSearchWord).size() > nLen) continue;		// Find length of next longest word in the list
			uint32_t matrixIndexNext = matrixIndexCurrent + maxDistance(nSkip, nLen, nSearchType) - 1;
			if (matrixIndexNext > matrixIndexLast) continue;		// Stop if the search would run off the end of the text

			QString strWord;
			uint32_t matrixIndexLetter = matrixIndexCurrent;		// MatrixIndex for the current letter being extracted
			for (int i = 0; i < nLen; ++i) {
				strWord += letterMatrix.at(matrixIndexLetter);
				matrixIndexLetter += nextOffset(nSkip, i, nSearchType);
			}

			for (int ndxWord = ndxSearchWord; ndxWord < lstSearchWords.size(); ++ndxWord) {
				if (lstSearchWords.at(ndxWord).size() != nLen) break;				// Check all words of this length only and exit when we hit a longer word
				if (strWord.compare(lstSearchWords.at(ndxWord)) == 0) {				// Check forward direction
					CELSResult result;
					result.m_strWord = strWord;
					result.m_nSkip = nSkip;
					result.m_nSearchType = nSearchType;
					result.m_ndxStart = letterMatrix.relIndexFromMatrixIndex(matrixIndexCurrent);
					result.m_nDirection = Qt::LeftToRight;
					lstResults.append(result);
				} else if (strWord.compare(lstSearchWordsRev.at(ndxWord)) == 0) {	// Check reverse direction
					CELSResult result;
					result.m_strWord = lstSearchWords.at(ndxWord);		// Result is always forward ordered word
					result.m_nSkip = nSkip;
					result.m_nSearchType = nSearchType;
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

CFindELS::CFindELS(const CLetterMatrix &letterMatrix, const QStringList &lstSearchWords, ELS_SEARCH_TYPE_ENUM nSearchType)
	:	m_letterMatrix(letterMatrix),
		m_lstSearchWords(lstSearchWords),
		m_lstSearchWordsRev(lstSearchWords),
		m_nSearchType(nSearchType)
{
	initFibonacciCast9Table();

	// Make all search words lower case and sort by ascending word length:
	for (auto &strSearchWord : m_lstSearchWords) strSearchWord = strSearchWord.toLower();
	std::sort(m_lstSearchWords.begin(), m_lstSearchWords.end(), [](const QString &s1, const QString &s2)->bool {
		return (s1.size() < s2.size());
	});

	// Create reversed word list so we can also search for ELS occurrences in both directions:
	for (auto &strSearchWord : m_lstSearchWordsRev) std::reverse(strSearchWord.begin(), strSearchWord.end());

	// Set default bookends to full Bible:
	setBookEnds();
}

// ----------------------------------------------------------------------------

bool CFindELS::setBookEnds(unsigned int nBookStart, unsigned int nBookEnd)
{
	if (nBookStart > m_letterMatrix.bibleDatabase()->bibleEntry().m_nNumBk) return false;

	if ((nBookStart > nBookEnd) && (nBookEnd != 0)) {			// Put starting/ending book indexes in order
		unsigned int nTemp = nBookEnd;
		nBookEnd = nBookStart;
		nBookStart = nTemp;
	}

	if (nBookStart == 0) nBookStart = 1;
	if ((nBookEnd == 0) || (nBookEnd > m_letterMatrix.bibleDatabase()->bibleEntry().m_nNumBk)) nBookEnd = m_letterMatrix.bibleDatabase()->bibleEntry().m_nNumBk;

	m_nBookStart = nBookStart;
	m_nBookEnd = nBookEnd;

	return true;
}

// ----------------------------------------------------------------------------

CELSResultList CFindELS::run(int nSkip) const
{
	return findELS(nSkip, m_letterMatrix, m_lstSearchWords, m_lstSearchWordsRev, m_nBookStart, m_nBookEnd, m_nSearchType);
}

// ============================================================================
