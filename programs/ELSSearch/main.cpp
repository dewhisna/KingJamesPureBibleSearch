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
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/PersistentSettings.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif
#include <QElapsedTimer>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QtConcurrent>

#include <iostream>
#include <set>
#include <algorithm>
#include <numeric>
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

class CELSResult {
public:
	QString m_strWord;
	int m_nSkip = 0;
	CRelIndexEx m_ndxStart;
	Qt::LayoutDirection m_nDirection = Qt::LeftToRight;
};

uint64_t matrixIndexFromRelIndex(const CBibleDatabase *pBibleDatabase, const CRelIndexEx nRelIndexEx)
{
	return pBibleDatabase->NormalizeIndexEx(nRelIndexEx);
}

CRelIndexEx relIndexFromMatrixIndex(const CBibleDatabase *pBibleDatabase, uint64_t nMatrixIndex)
{
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
	uint64_t matrixIndexResult = matrixIndexFromRelIndex(pBibleDatabase, result.m_ndxStart);
	uint64_t matrixIndexStart = matrixIndexFromRelIndex(pBibleDatabase, CRelIndexEx(CRelIndex(relPassageStart.book(), relPassageStart.chapter(), relPassageStart.verse(), 0), 0));
	uint64_t martixIndexEnd = matrixIndexResult + ((result.m_nSkip+1)*(result.m_strWord.size()));
	martixIndexEnd += (result.m_nSkip+1) - ((martixIndexEnd - matrixIndexStart + 1) % (result.m_nSkip+1));		// Make a whole number of row data
	int nChar = 0;
	for (uint64_t normalIndex = matrixIndexStart; normalIndex <= martixIndexEnd; ++normalIndex) {
		if (normalIndex == matrixIndexStart) {
			std::cout << "\n";
			matrixIndexStart += result.m_nSkip+1;
		}
		if (normalIndex >= static_cast<uint64_t>(g_lstLetterMatrix.size())) break;
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
auto findELS = [](int nSkip, const CBibleDatabase *pBibleDatabase, const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev) {
	// Results storage (for this skip):
	QList<CELSResult> lstResults;

	// Get maximum search word lengths to know bounds of search:
	int nMaxLength = lstSearchWords.last().size();

	// Compute starting index for the first letter in the Bible:
	CRelIndexEx ndxCurrent = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_Start);
	uint64_t matrixIndexCurrent = matrixIndexFromRelIndex(pBibleDatabase, ndxCurrent);

	// Compute ending index for the last letter in the Bible:
	CRelIndexEx ndxLast = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_End);
	const CConcordanceEntry *pceLastWord = pBibleDatabase->concordanceEntryForWordAtIndex(ndxLast);
	if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
	uint64_t matrixIndexLast = matrixIndexFromRelIndex(pBibleDatabase, ndxLast);
	Q_ASSERT((matrixIndexLast+1) == static_cast<uint64_t>(g_lstLetterMatrix.size()));

	while (matrixIndexCurrent <= matrixIndexLast) {
		int ndxSearchWord = 0;				// Index to current search word being tested
		for (int nLen = lstSearchWords.at(ndxSearchWord).size(); nLen <= nMaxLength; ++nLen) {
			if (ndxSearchWord >= lstSearchWords.size()) break;
			while ((ndxSearchWord < lstSearchWords.size()) &&					// Find next word in list at least nLen long
				   (lstSearchWords.at(ndxSearchWord).size() < nLen)) {
				++ndxSearchWord;
			}
			if (lstSearchWords.at(ndxSearchWord).size() > nLen) continue;		// Find length of next longest word in the list
			uint64_t matrixIndexNext = matrixIndexCurrent + (nSkip*(nLen-1)) + nLen - 1;
			if (matrixIndexNext > matrixIndexLast) continue;		// Stop if the search would run off the end of the text

			QString strWord;
			uint64_t matrixIndexLetter = matrixIndexCurrent;		// MatrixIndex for the current letter being extracted
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

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

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
	bool bSkipColophons = false;
	bool bSkipSuperscriptions = false;
	bool bOutputWordsAllUppercase = false;

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
			bSkipColophons = true;
		} else if (strArg.compare("-ss") == 0) {
			bSkipSuperscriptions = true;
		} else if (strArg.compare("-u") == 0) {
			bOutputWordsAllUppercase = true;
		} else {
			bUnknownOption = true;
		}
	}

	for (auto const &strSearchWord : lstSearchWords) if (strSearchWord.size() < 2) bUnknownOption = true;	// Each word must have at least two characters
	if (lstSearchWords.isEmpty()) bUnknownOption = true;		// Must have at least one search word

	if ((nArgsFound != 4) || (bUnknownOption)) {
		std::cerr << QString("ELSSearch Version %1\n\n").arg(app.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Min-Letter-Skip> <Max-Letter-Skip> <Words>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database and searches for the specified <Words> at ELS\n").toUtf8().data();
		std::cerr << QString("    skip-distances from <Min-Letter-Skip> to <Max-Letter-Skip>\n\n").toUtf8().data();
		std::cerr << QString("<Words> = Comma separated list of words to search (each must be at least two characters)\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -mt =  Run Multi-Threaded\n").toUtf8().data();
// TODO : Figure out how to skip colophons and superscriptions and how to properly define their search order:
//		std::cerr << QString("  -sc =  Skip Colophons\n").toUtf8().data();
//		std::cerr << QString("  -ss =  Skip Superscriptions\n").toUtf8().data();
		std::cerr << QString("  -u  =  Print Output Text in all uppercase (default is lowercase)\n").toUtf8().data();
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

	// Create giant array of all letters from the Bible text for speed:
	g_lstLetterMatrix.reserve(pBibleDatabase->bibleEntry().m_nNumLtr + 1);		// +1 since we reserve the 0 entry
	g_lstLetterMatrix.append(QChar());
	CRelIndex ndxMatrixStart = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_Start);
	uint32_t normalMatrixStart = pBibleDatabase->NormalizeIndex(ndxMatrixStart);
	CRelIndex ndxMatrixEnd = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_End);
	uint32_t normalMatrixEnd = pBibleDatabase->NormalizeIndex(ndxMatrixEnd);
	for (uint32_t ndx = normalMatrixStart; ndx <= normalMatrixEnd; ++ndx) {
		const CConcordanceEntry *pWordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(ndx);
		Q_ASSERT(pWordEntry != nullptr);
		if (pWordEntry) {
			const QString &strWord = pWordEntry->rawWord();
			for (auto const &chrLetter : strWord) g_lstLetterMatrix.append(chrLetter);
		}
	}

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
			FutureRunner(const CBibleDatabase *pBibleDatabase, const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev)
				:	m_pBibleDatabase(pBibleDatabase),
					m_lstSearchWords(lstSearchWords),
					m_lstSearchWordsRev(lstSearchWordsRev)
			{ }
			QList<CELSResult> run(int nSkip)
			{
				return findELS(nSkip, m_pBibleDatabase, m_lstSearchWords, m_lstSearchWordsRev);
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
		} runner(pBibleDatabase.data(), lstSearchWords, lstSearchWordsRev);
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
					return findELS(nSkip, pBibleDatabase.data(), lstSearchWords, lstSearchWordsRev);
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
			lstResults.append(findELS(nSkip, pBibleDatabase.data(), lstSearchWords, lstSearchWordsRev));
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

	// Print Summary:
	std::cout << "Word Occurrence Count:\n";
	for (int i = 0; i < lstSearchWords.size(); ++i) {
		std::cout << QString("%1 : Forward: %2, Reverse: %3\n").arg(bOutputWordsAllUppercase ? lstSearchWords.at(i).toUpper() : lstSearchWords.at(i))
						 .arg(mapResultsWordCountForward[lstSearchWords.at(i)])
						 .arg(mapResultsWordCountReverse[lstSearchWords.at(i)]).toUtf8().data();
	}
	std::cout << "\n";

	// Sort results based on word, then skip, then Bible passage reference:
	std::sort(lstResults.begin(), lstResults.end(),
			[](const CELSResult &r1, const CELSResult &r2)->bool {
				int nRetVal = r1.m_strWord.compare(r2.m_strWord);
				if (nRetVal < 0) return true;
				if (nRetVal == 0) {
					if (r1.m_nSkip < r2.m_nSkip) return true;
					if (r1.m_nSkip == r2.m_nSkip) return (r1.m_ndxStart.indexEx() < r2.m_ndxStart.indexEx());
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

