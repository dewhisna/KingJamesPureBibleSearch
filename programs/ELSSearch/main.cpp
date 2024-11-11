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

#include "../KJVCanOpener/dbstruct.h"
#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/ReadDB.h"
#include "../KJVCanOpener/Translator.h"

#include <QCoreApplication>
#include <QObject>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QElapsedTimer>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QtConcurrent>

#include <iostream>
#include <set>
#include <algorithm>
#include <numeric>
#include <utility>			// for std::pair
#if QT_VERSION < 0x060000
#include <functional>		// Needed for std::bind on Qt5 path
#endif

#include "../KJVCanOpener/PathConsts.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0

}	// namespace


// ============================================================================

// Create giant array of all letters from the Bible text for speed:
QList<QChar> g_lstLetterMatrix;

bool g_bSkipColophons = false;
bool g_bSkipSuperscriptions = false;

// Matrix index to letter count shift for normalize/denormalize computations:
//	When we are skipping colophons and/or superscriptions, the matrix index
//	of what would be the colophon/superscription is set here with the number
//	of letters to skip for it.  When transforming to/from matrix index and
//	database normal/rel index, this map will be scanned for all values of
//	matrix index less than or equal to the one being transformed and the
//	corresponding letter count added or subtracted (depending on direction
//	of the transformation):
typedef QMap<uint32_t, uint32_t> TMapMatrixIndexToLetterShift;	// MatrixIndex -> LetterCount
TMapMatrixIndexToLetterShift g_mapMatrixIndexToLetterShift;

class CELSResult {
public:
	QString m_strWord;
	int m_nSkip = 0;
	CRelIndexEx m_ndxStart;
	Qt::LayoutDirection m_nDirection = Qt::LeftToRight;
};

// ----------------------------------------------------------------------------

uint32_t matrixIndexFromRelIndex(const CBibleDatabase *pBibleDatabase, const CRelIndexEx nRelIndexEx)
{
	CRelIndex relIndex{nRelIndexEx.index()};
	const CBookEntry *pBook = pBibleDatabase->bookEntry(relIndex);
	Q_ASSERT(pBook != nullptr);  if (pBook == nullptr) return 0;
	uint32_t nMatrixIndex = pBibleDatabase->NormalizeIndexEx(nRelIndexEx);
	if (nRelIndexEx.isColophon()) {
		Q_ASSERT(pBook->m_bHaveColophon);
		const CVerseEntry *pColophonVerse = pBibleDatabase->verseEntry(relIndex);
		Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
		nMatrixIndex += (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);		// Shift colophon to end of book
	} else {
		if (pBook->m_bHaveColophon) {		// If this book has a colophon, but this index isn't it, shift this index ahead of colophon
			const CVerseEntry *pColophonVerse = pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
			Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
			nMatrixIndex -= pColophonVerse->m_nNumLtr;
		}
	}

	// Note: Since the colophon transform mapping is inserted at the
	//	point where the colophon would be (after moving) in the matrix, then
	//	we must do this index transform after the other colophon transforms:
	// Note: These must be subtracted as we go as these shifts must be cumulative
	//	(unlike the other direction):
	for (TMapMatrixIndexToLetterShift::const_iterator itrXformMatrix = g_mapMatrixIndexToLetterShift.cbegin();
		 itrXformMatrix != g_mapMatrixIndexToLetterShift.cend(); ++itrXformMatrix) {
		if (itrXformMatrix.key() <= nMatrixIndex) {
			nMatrixIndex -= itrXformMatrix.value();
		} else {
			break;
		}
	}

	return nMatrixIndex;
}

CRelIndexEx relIndexFromMatrixIndex(const CBibleDatabase *pBibleDatabase, uint32_t nMatrixIndex)
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
	for (TMapMatrixIndexToLetterShift::const_iterator itrXformMatrix = g_mapMatrixIndexToLetterShift.cbegin();
		 itrXformMatrix != g_mapMatrixIndexToLetterShift.cend(); ++itrXformMatrix) {
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
	CRelIndex relIndex{pBibleDatabase->DenormalizeIndexEx(nMatrixIndex)};
	const CBookEntry *pBook = pBibleDatabase->bookEntry(relIndex);
	Q_ASSERT(pBook != nullptr);  if (pBook == nullptr) return CRelIndexEx();

	if (pBook->m_bHaveColophon) {		// Must do this even when skipping colophons to shift other indexes after colophon until we catchup with the transform above
		const CVerseEntry *pColophonVerse = pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
		Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
		uint32_t nMatrixColophonNdx = pBibleDatabase->NormalizeIndexEx(CRelIndexEx(relIndex.book(), 0, 0, 1, 1)) +
									  (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);
		if (nMatrixIndex >= nMatrixColophonNdx) {
			if (!g_bSkipColophons) {	// Don't adjust for the colophon here if we skipped it
				nMatrixIndex -= (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);		// Shift colophon back to start of book
			}
		} else {
			nMatrixIndex += pColophonVerse->m_nNumLtr;		// Shift other indexes after colophon
		}
	}
	return pBibleDatabase->DenormalizeIndexEx(nMatrixIndex);
}

// ----------------------------------------------------------------------------

void printResult(const CBibleDatabase *pBibleDatabase, const CELSResult &result, bool bUpperCase)
{
	std::cout << "----------------------------------------\n";
	std::cout << QString("Word: \"%1\"\n").arg(bUpperCase ? result.m_strWord.toUpper() : result.m_strWord).toUtf8().data();
	std::cout << QString("Start Location: %1\n").arg(pBibleDatabase->PassageReferenceText(result.m_ndxStart, false)).toUtf8().data();
	std::cout << QString("Skip: %1\n").arg(result.m_nSkip).toUtf8().data();
	std::cout << QString("Direction: %1\n").arg((result.m_nDirection == Qt::LeftToRight) ? "Forward" : "Reverse").toUtf8().data();
	CRelIndex relPassageStart = CRelIndex(result.m_ndxStart.index());
	uint32_t matrixIndexResult = matrixIndexFromRelIndex(pBibleDatabase, result.m_ndxStart);
	uint32_t matrixIndexStart = matrixIndexFromRelIndex(pBibleDatabase, CRelIndexEx(CRelIndex(relPassageStart.book(), relPassageStart.chapter(), relPassageStart.verse(), 0), 0));
	uint32_t martixIndexEnd = matrixIndexResult + ((result.m_nSkip+1)*(result.m_strWord.size()));
	martixIndexEnd += (result.m_nSkip+1) - ((martixIndexEnd - matrixIndexStart + 1) % (result.m_nSkip+1));		// Make a whole number of row data
	int nChar = 0;
	for (uint32_t normalIndex = matrixIndexStart; normalIndex <= martixIndexEnd; ++normalIndex) {
		if (normalIndex == matrixIndexStart) {
			std::cout << "\n";
			matrixIndexStart += result.m_nSkip+1;
		}
		if (normalIndex >= static_cast<uint32_t>(g_lstLetterMatrix.size())) break;
		std::cout << ((normalIndex == matrixIndexResult) ? "[" : " ");
		if (bUpperCase) {
			std::cout << QString(g_lstLetterMatrix.at(normalIndex).toUpper()).toUtf8().data();
		} else {
			std::cout << QString(g_lstLetterMatrix.at(normalIndex)).toUtf8().data();
		}
		std::cout << ((normalIndex == matrixIndexResult) ? "]" : " ");
		if ((normalIndex == matrixIndexResult) && (++nChar < result.m_strWord.size())) matrixIndexResult += result.m_nSkip+1;
	}
	std::cout << "\n";
}

// ----------------------------------------------------------------------------

// Concurrent Threading function to locate the ELS entries for a single skip distance:
auto findELS = [](int nSkip, const CBibleDatabase *pBibleDatabase,
					const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev,
					unsigned int nBookStart, unsigned int nBookEnd) {
	// Results storage (for this skip):
	QList<CELSResult> lstResults;

	// Get maximum search word lengths to know bounds of search:
	int nMaxLength = lstSearchWords.last().size();

	// Compute starting index for first letter in the search range:
	uint32_t matrixIndexCurrent = matrixIndexFromRelIndex(pBibleDatabase, CRelIndexEx(nBookStart, 1, 0, 1, 1));

	// Compute ending index for the last letter in the search range:
	CRelIndexEx ndxLast = pBibleDatabase->calcRelIndex(CRelIndex(nBookEnd, 0, 0, 0), CBibleDatabase::RIME_EndOfBook);
	const CConcordanceEntry *pceLastWord = pBibleDatabase->concordanceEntryForWordAtIndex(ndxLast);
	if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
	uint32_t matrixIndexLast = matrixIndexFromRelIndex(pBibleDatabase, ndxLast);

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
				strWord += g_lstLetterMatrix.at(matrixIndexLetter);
				matrixIndexLetter += nSkip + 1;
			}

			for (int ndxWord = ndxSearchWord; ndxWord < lstSearchWords.size(); ++ndxWord) {
				if (lstSearchWords.at(ndxWord).size() != nLen) break;				// Check all words of this length only and exit when we hit a longer word
				if (strWord.compare(lstSearchWords.at(ndxWord)) == 0) {				// Check forward direction
					CELSResult result;
					result.m_strWord = strWord;
					result.m_nSkip = nSkip;
					result.m_ndxStart = relIndexFromMatrixIndex(pBibleDatabase, matrixIndexCurrent);
					result.m_nDirection = Qt::LeftToRight;
					lstResults.append(result);
				} else if (strWord.compare(lstSearchWordsRev.at(ndxWord)) == 0) {	// Check reverse direction
					CELSResult result;
					result.m_strWord = lstSearchWords.at(ndxWord);		// Result is always forward ordered word
					result.m_nSkip = nSkip;
					result.m_ndxStart = relIndexFromMatrixIndex(pBibleDatabase, matrixIndexCurrent);
					result.m_nDirection = Qt::RightToLeft;
					lstResults.append(result);
				}
			}
		}

		++matrixIndexCurrent;
	}

	return lstResults;
};

// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	app.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	int nDescriptor = -1;
	int nMinSkip = 0;
	int nMaxSkip = 0;
	QStringList lstSearchWords;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	bool bUnknownOption = false;
	// ----
	bool bRunMultithreaded = false;
	bool bOutputWordsAllUppercase = false;
	unsigned int nBookStart = 0;
	unsigned int nBookEnd = 0;
	// ----
	enum OUTPUT_SORT_ORDER_ENUM {
		OSO_WSR,
		OSO_WRS,
		OSO_RWS,
		OSO_RSW,
		OSO_SRW,
		OSO_SWR,
	} nSortOrder = OSO_WSR;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				nMinSkip = strArg.toInt();
			} else if (nArgsFound == 3) {
				nMaxSkip = strArg.toInt();
			} else if (nArgsFound == 4) {
				lstSearchWords = strArg.split(',', Qt::SkipEmptyParts);
			} else {
				bUnknownOption = true;
			}
		} else if (strArg.compare("-mt") == 0) {
			bRunMultithreaded = true;
		} else if (strArg.compare("-sc") == 0) {
			g_bSkipColophons = true;
		} else if (strArg.compare("-ss") == 0) {
			g_bSkipSuperscriptions = true;
		} else if (strArg.compare("-u") == 0) {
			bOutputWordsAllUppercase = true;
		} else if (strArg.startsWith("-bb")) {
			nBookStart = strArg.mid(3).toUInt();
		} else if (strArg.startsWith("-be")) {
			nBookEnd = strArg.mid(3).toUInt();
		} else if (strArg.compare("-owsr") == 0) {
			nSortOrder = OSO_WSR;
		} else if (strArg.compare("-owrs") == 0) {
			nSortOrder = OSO_WRS;
		} else if (strArg.compare("-orws") == 0) {
			nSortOrder = OSO_RWS;
		} else if (strArg.compare("-orsw") == 0) {
			nSortOrder = OSO_RSW;
		} else if (strArg.compare("-osrw") == 0) {
			nSortOrder = OSO_SRW;
		} else if (strArg.compare("-oswr") == 0) {
			nSortOrder = OSO_SWR;
		} else {
			bUnknownOption = true;
		}
	}

	for (auto const &strSearchWord : lstSearchWords) if (strSearchWord.size() < 2) bUnknownOption = true;	// Each word must have at least two characters
	if (lstSearchWords.isEmpty()) bUnknownOption = true;		// Must have at least one search word
	if ((nBookStart > nBookEnd) && (nBookEnd != 0)) {			// Put starting/ending book indexes in order
		unsigned int nTemp = nBookEnd;
		nBookEnd = nBookStart;
		nBookStart = nTemp;
	}

	if ((nArgsFound != 4) || (bUnknownOption)) {
		std::cerr << QString("ELSSearch Version %1\n\n").arg(app.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Min-Letter-Skip> <Max-Letter-Skip> <Words>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database and searches for the specified <Words> at ELS\n").toUtf8().data();
		std::cerr << QString("    skip-distances from <Min-Letter-Skip> to <Max-Letter-Skip>\n\n").toUtf8().data();
		std::cerr << QString("<Words> = Comma separated list of words to search (each must be at least two characters)\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -mt    =  Run Multi-Threaded\n").toUtf8().data();
		std::cerr << QString("  -sc    =  Skip Colophons\n").toUtf8().data();
		std::cerr << QString("  -ss    =  Skip Superscriptions\n").toUtf8().data();
		std::cerr << QString("  -u     =  Print Output Text in all uppercase (default is lowercase)\n").toUtf8().data();
		std::cerr << QString("  -bb<N> =  Begin Searching in Book <N> (defaults to first)\n").toUtf8().data();
		std::cerr << QString("  -be<N> =  End Searching in Book <N>   (defaults to last)\n").toUtf8().data();
		std::cerr << QString("  -owsr  =  Order output by word, skip, then reference (this is the default)\n").toUtf8().data();
		std::cerr << QString("  -owrs  =  Order output by word, reference, then skip\n").toUtf8().data();
		std::cerr << QString("  -orws  =  Order output by reference, word, then skip\n").toUtf8().data();
		std::cerr << QString("  -orsw  =  Order output by reference, skip, then word\n").toUtf8().data();
		std::cerr << QString("  -osrw  =  Order output by skip, reference, then word\n").toUtf8().data();
		std::cerr << QString("  -oswr  =  Order output by skip, word, then reference\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
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

	CReadDatabase rdbMain;
	if (!rdbMain.haveBibleDatabaseFiles(bblDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Bible Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDescriptor, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Bible Database!\n").toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	// Set book search span:
	if (nBookStart && (nBookStart > pBibleDatabase->bibleEntry().m_nNumBk)) {
		std::cerr << QString("\n*** ERROR: Book %1 specified as starting book, but database only has %2 books!\n")
						.arg(nBookStart).arg(pBibleDatabase->bibleEntry().m_nNumBk).toUtf8().data();
		return -4;
	}
	if (nBookStart == 0) nBookStart = 1;
	if ((nBookEnd == 0) || (nBookEnd > pBibleDatabase->bibleEntry().m_nNumBk)) nBookEnd = pBibleDatabase->bibleEntry().m_nNumBk;

	// Create giant array of all letters from the Bible text for speed:
	//	NOTE: This is with the entire Bible content (sans colophons/
	//	superscriptions when they are skipped) and not the search span
	//	so that we don't have to convert the matrix index based on the
	//	search span.
	g_lstLetterMatrix.reserve(pBibleDatabase->bibleEntry().m_nNumLtr + 1);		// +1 since we reserve the 0 entry
	g_lstLetterMatrix.append(QChar());
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
			if (!g_bSkipColophons) {
				g_lstLetterMatrix.append(lstColophon);
			} else {
				g_mapMatrixIndexToLetterShift[g_lstLetterMatrix.size()] = lstColophon.size();
			}
			lstColophon.clear();
			ndxMatrixLastColophon.clear();
		}

		// Check for skipped superscription to map:
		if (!lstSuperscription.isEmpty() && (ndxMatrixLastSuperscription.verse() != ndxMatrixCurrent.verse())) {
			Q_ASSERT(g_bSkipSuperscriptions);		// Should only be here if actually skipping the superscriptions
			g_mapMatrixIndexToLetterShift[g_lstLetterMatrix.size()] = lstSuperscription.size();
			lstSuperscription.clear();
			ndxMatrixLastSuperscription.clear();
		}

		const CConcordanceEntry *pWordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(ndxMatrixCurrent);
		Q_ASSERT(pWordEntry != nullptr);
		if (pWordEntry) {
			const QString &strWord = pWordEntry->rawWord();

			if (!ndxMatrixCurrent.isColophon()) {
				if (!ndxMatrixCurrent.isSuperscription() || !g_bSkipSuperscriptions) {
					// Output the book as-is without shuffling:
					for (auto const &chrLetter : strWord) g_lstLetterMatrix.append(chrLetter);
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
	uint32_t matrixIndexLast = matrixIndexFromRelIndex(pBibleDatabase.data(), ndxLast);
	Q_ASSERT((matrixIndexLast+1) == static_cast<uint32_t>(g_lstLetterMatrix.size()));

	// ------------------------------------------------------------------------

	// Make all search words lower case and sort by ascending word length:
	for (auto &strSearchWord : lstSearchWords) strSearchWord = strSearchWord.toLower();
	std::sort(lstSearchWords.begin(), lstSearchWords.end(), [](const QString &s1, const QString &s2)->bool {
		return (s1.size() < s2.size());
	});

	// Create reversed word list so we can also search for ELS occurrences in both directions:
	QStringList lstSearchWordsRev = lstSearchWords;
	for (auto &strSearchWord : lstSearchWordsRev) std::reverse(strSearchWord.begin(), strSearchWord.end());

	// Results storage (for all skips):
	QList<CELSResult> lstResults;

	// Perform the Search:
	QElapsedTimer elapsedTime;
	elapsedTime.start();
	std::cerr << "Searching";

	// Build list of skips to search:
#if QT_VERSION < 0x060000
	// NOTE: Unlike Qt6, Qt 5 has no constructor to prepopulate the list:
	QList<int> lstSkips;
	lstSkips.reserve(nMaxSkip - nMinSkip + 1);
	for (int nSkip = nMinSkip; nSkip <= nMaxSkip; ++nSkip) lstSkips.append(0);
#else
	QList<int> lstSkips(nMaxSkip - nMinSkip + 1);
#endif
	std::iota(lstSkips.begin(), lstSkips.end(), nMinSkip);

	if (bRunMultithreaded) {

#if QT_VERSION < 0x060000
		// NOTE: Qt 5 can't use lambdas for the functors in the mappedReduced() call.
		//	So this dance works around that by having normal functions for it:
		class FutureRunner {
		public:
			FutureRunner(const CBibleDatabase *pBibleDatabase,
						 const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev,
						 unsigned int nBookStart, unsigned int nBookEnd)
				:	m_pBibleDatabase(pBibleDatabase),
					m_lstSearchWords(lstSearchWords),
					m_lstSearchWordsRev(lstSearchWordsRev),
					m_nBookStart(nBookStart),
					m_nBookEnd(nBookEnd)
			{ }
			QList<CELSResult> run(int nSkip)
			{
				return findELS(nSkip, m_pBibleDatabase, m_lstSearchWords, m_lstSearchWordsRev, m_nBookStart, m_nBookEnd);
			}
			static void reduce(QList<CELSResult> &lstResults, const QList<CELSResult> &result)
			{
				lstResults.append(result);
				std::cerr << ".";
			}

		private:
			const CBibleDatabase *m_pBibleDatabase;
			const QStringList &m_lstSearchWords;
			const QStringList &m_lstSearchWordsRev;
			unsigned int m_nBookStart;
			unsigned int m_nBookEnd;
		} runner(pBibleDatabase.data(), lstSearchWords, lstSearchWordsRev, nBookStart, nBookEnd);
#endif

		QFutureSynchronizer< QList<CELSResult> > synchronizer;
#if QT_VERSION < 0x060000
		synchronizer.setFuture(
			QtConcurrent::mappedReduced(lstSkips,
										std::bind(&FutureRunner::run, &runner, std::placeholders::_1),
										&FutureRunner::reduce)
		);
#else
		synchronizer.setFuture(
			QtConcurrent::mappedReduced(lstSkips,
				[&](int nSkip)->QList<CELSResult>
				{
					return findELS(nSkip, pBibleDatabase.data(), lstSearchWords, lstSearchWordsRev, nBookStart, nBookEnd);
				},
				[](QList<CELSResult> &lstResults, const QList<CELSResult> &result)
				{
					lstResults.append(result);
					std::cerr << ".";
				}
			)
		);
#endif
		synchronizer.waitForFinished();
		lstResults = synchronizer.futures().at(0).result();
	} else {
		for (auto const & nSkip : lstSkips) {
			lstResults.append(findELS(nSkip, pBibleDatabase.data(), lstSearchWords, lstSearchWordsRev, nBookStart, nBookEnd));
			std::cerr << ".";
		}
	}

	// Results counts:
	QMap<QString, int> mapResultsWordCountForward;
	QMap<QString, int> mapResultsWordCountReverse;

	for (auto const & result : lstResults) {
		if (result.m_nDirection == Qt::LeftToRight) {
			mapResultsWordCountForward[result.m_strWord]++;
		} else {
			mapResultsWordCountReverse[result.m_strWord]++;
		}
	}

	std::cerr << "\n";

	std::cerr << QString("Search Time: %1 secs\n\n").arg(elapsedTime.elapsed() / 1000.0).toUtf8().data();

	// Print Search Spec:
	QString strBookRange;
	if ((nBookStart == 1) && (nBookEnd == pBibleDatabase->bibleEntry().m_nNumBk)) {
		strBookRange = "Entire Bible";
	} else {
		strBookRange = QString("%1 through %2")
								.arg(pBibleDatabase->bookName(CRelIndex(nBookStart, 0, 0, 0)))
								.arg(pBibleDatabase->bookName(CRelIndex(nBookEnd, 0, 0, 0)));
	}
	std::cout << "Searching for ELS skips from " << nMinSkip << " to " << nMaxSkip;
	std::cout << " in " << strBookRange.toUtf8().data() << "\n";
	if (g_bSkipColophons) std::cout << "Skipping Colophons\n";
	if (g_bSkipSuperscriptions) std::cout << "Skipping Superscriptions\n";

	// Print Summary:
	std::cout << "\nWord Occurrence Counts:\n";
	for (int i = 0; i < lstSearchWords.size(); ++i) {
		std::cout << QString("%1 : Forward: %2, Reverse: %3\n").arg(bOutputWordsAllUppercase ? lstSearchWords.at(i).toUpper() : lstSearchWords.at(i))
						 .arg(mapResultsWordCountForward[lstSearchWords.at(i)])
						 .arg(mapResultsWordCountReverse[lstSearchWords.at(i)]).toUtf8().data();
	}
	std::cout << "\n";

	std::cout << "Sort Order: ";
	switch (nSortOrder) {
		case OSO_WSR:
			std::cout << "Word, Skip, Ref\n";
			break;
		case OSO_WRS:
			std::cout << "Word, Ref, Skip\n";
			break;
		case OSO_RWS:
			std::cout << "Ref, Word, Skip\n";
			break;
		case OSO_RSW:
			std::cout << "Ref, Skip, Word\n";
			break;
		case OSO_SRW:
			std::cout << "Skip, Ref, Word\n";
			break;
		case OSO_SWR:
			std::cout << "Skip, Word, Ref\n";
			break;
	}
	std::cout << "\n";

	// Sort results based on sort order:
	std::sort(lstResults.begin(), lstResults.end(),
			[nSortOrder](const CELSResult &r1, const CELSResult &r2)->bool {
				auto fnWord = [](const CELSResult &r1, const CELSResult &r2)->std::pair<bool,bool> {
					int nComp = r1.m_strWord.compare(r2.m_strWord);
					return std::pair<bool,bool>(nComp < 0, nComp == 0);
				};
				auto fnSkip = [](const CELSResult &r1, const CELSResult &r2)->std::pair<bool,bool> {
					return std::pair<bool,bool>(r1.m_nSkip < r2.m_nSkip, r1.m_nSkip == r2.m_nSkip);
				};
				auto fnRef = [](const CELSResult &r1, const CELSResult &r2)->std::pair<bool,bool> {
					return std::pair<bool,bool>(r1.m_ndxStart.indexEx() < r2.m_ndxStart.indexEx(),
												r1.m_ndxStart.indexEx() == r2.m_ndxStart.indexEx());
				};
				struct TFuncs {
					std::pair<bool,bool> (*m_first)(const CELSResult &, const CELSResult &);
					std::pair<bool,bool> (*m_second)(const CELSResult &, const CELSResult &);
					std::pair<bool,bool> (*m_third)(const CELSResult &, const CELSResult &);
				} sortFuncs[] = {			// Order must match OUTPUT_SORT_ORDER_ENUM
					{ fnWord, fnSkip, fnRef },	// OSO_WSR
					{ fnWord, fnRef, fnSkip },	// OSO_WRS
					{ fnRef, fnWord, fnSkip },	// OSO_RWS
					{ fnRef, fnSkip, fnWord },	// OSO_RSW
					{ fnSkip, fnRef, fnWord },	// OSO_SRW
					{ fnSkip, fnWord, fnRef },	// OSO_SWR
				};
				std::pair<bool,bool> cmpFirst = sortFuncs[nSortOrder].m_first(r1, r2);
				if (cmpFirst.first) return true;
				if (cmpFirst.second) {
					std::pair<bool,bool> cmpSecond = sortFuncs[nSortOrder].m_second(r1, r2);
					if (cmpSecond.first) return true;
					if (cmpSecond.second) {
						std::pair<bool,bool> cmpThird = sortFuncs[nSortOrder].m_third(r1, r2);
						if (cmpThird.first) return true;
					}
				}
				return false;
			});

	// Print Results:
	std::cout << QString("Found %1 Results:\n").arg(lstResults.size()).toUtf8().data();
	for (auto const & result : lstResults) {
		printResult(pBibleDatabase.data(), result, bOutputWordsAllUppercase);
	}

	// ------------------------------------------------------------------------

	return 0;
}

