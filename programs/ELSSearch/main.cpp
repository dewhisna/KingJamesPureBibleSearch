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

#include "ELSSearchMainWindow.h"

#include "LetterMatrix.h"
#include "FindELS.h"

#ifndef IS_CONSOLE_APP
#include <QApplication>
#else
#include <QCoreApplication>
#endif
#include <QObject>
#include <QDir>				// Needed for call to QFileInfo
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QElapsedTimer>
#include <QFutureSynchronizer>

#include <iostream>
#include <algorithm>		// for std::sort
#include <utility>			// for std::pair

#include "../KJVCanOpener/PathConsts.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0

}	// namespace


// ============================================================================

void printResult(const CLetterMatrix &letterMatrix, const CELSResult &result, bool bUpperCase)
{
	std::cout << "----------------------------------------\n";
	std::cout << QString("Word: \"%1\"\n").arg(bUpperCase ? result.m_strWord.toUpper() : result.m_strWord).toUtf8().data();
	std::cout << QString("Start Location: %1\n").arg(letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxStart, false)).toUtf8().data();
	std::cout << QString("Skip: %1\n").arg(result.m_nSkip).toUtf8().data();
	std::cout << QString("Direction: %1\n").arg((result.m_nDirection == Qt::LeftToRight) ? "Forward" : "Reverse").toUtf8().data();
	CRelIndex relPassageStart = CRelIndex(result.m_ndxStart.index());
	uint32_t matrixIndexResult = letterMatrix.matrixIndexFromRelIndex(result.m_ndxStart);
	uint32_t matrixIndexStart = letterMatrix.matrixIndexFromRelIndex(CRelIndexEx(CRelIndex(relPassageStart.book(), relPassageStart.chapter(), relPassageStart.verse(), 0), 0));
	uint32_t martixIndexEnd = matrixIndexResult + ((result.m_nSkip+1)*(result.m_strWord.size()));
	martixIndexEnd += (result.m_nSkip+1) - ((martixIndexEnd - matrixIndexStart + 1) % (result.m_nSkip+1));		// Make a whole number of row data
	int nChar = 0;
	for (uint32_t normalIndex = matrixIndexStart; normalIndex <= martixIndexEnd; ++normalIndex) {
		if (normalIndex == matrixIndexStart) {
			std::cout << "\n";
			matrixIndexStart += result.m_nSkip+1;
		}
		if (normalIndex >= static_cast<uint32_t>(letterMatrix.size())) break;
		std::cout << ((normalIndex == matrixIndexResult) ? "[" : " ");
		if (bUpperCase) {
			std::cout << QString(letterMatrix.at(normalIndex).toUpper()).toUtf8().data();
		} else {
			std::cout << QString(letterMatrix.at(normalIndex)).toUtf8().data();
		}
		std::cout << ((normalIndex == matrixIndexResult) ? "]" : " ");
		if ((normalIndex == matrixIndexResult) && (++nChar < result.m_strWord.size())) matrixIndexResult += result.m_nSkip+1;
	}
	std::cout << "\n";
}

// ----------------------------------------------------------------------------

// NOTE: Qt 5 can't use lambdas for the functors in the mappedReduced() call.
//	So this dance works around that by having normal functions for it:
static void reduce(CELSResultList &lstResults, const CELSResultList &result)
{
	lstResults.append(result);
	std::cerr << ".";
}

// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
#ifndef IS_CONSOLE_APP
	QApplication app(argc, argv);
#else
	QCoreApplication app(argc, argv);
#endif
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
	bool bSkipColophons = false;
	bool bSkipSuperscriptions = false;
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
			bSkipColophons = true;
		} else if (strArg.compare("-ss") == 0) {
			bSkipSuperscriptions = true;
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

#ifndef IS_CONSOLE_APP

	CELSSearchMainWindow *pMainWindow = new CELSSearchMainWindow(pBibleDatabase, bSkipColophons, bSkipSuperscriptions);

	pMainWindow->show();

	int nRetVal = app.exec();

	delete pMainWindow;

	return nRetVal;

#else
	CLetterMatrix letterMatrix(pBibleDatabase, bSkipColophons, bSkipSuperscriptions);

	// ------------------------------------------------------------------------

	// Results storage (for all skips):
	CELSResultList lstResults;

	// Perform the Search:
	QElapsedTimer elapsedTime;
	elapsedTime.start();
	std::cerr << "Searching";

	CFindELS elsFinder(letterMatrix, lstSearchWords);
	if (!elsFinder.setBookEnds(nBookStart, nBookEnd)) {
		std::cerr << QString("\n*** ERROR: Invalid Book Begin/End specified.  Database has books 1 through %1!\n")
						 .arg(pBibleDatabase->bibleEntry().m_nNumBk).toUtf8().data();
		return -4;
	}
	nBookStart = elsFinder.bookStart();		// Set local variables to resolved start/end locations for reporting later
	nBookEnd = elsFinder.bookEnd();

	if (bRunMultithreaded) {
		QFutureSynchronizer<CELSResultList> synchronizer;
		synchronizer.setFuture(elsFinder.future(nMinSkip, nMaxSkip, &reduce));
		synchronizer.waitForFinished();
		lstResults = synchronizer.futures().at(0).result();
	} else {
		for (int nSkip = nMinSkip; nSkip <= nMaxSkip; ++nSkip) {
			reduce(lstResults, elsFinder.run(nSkip));
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
	if (bSkipColophons) std::cout << "Skipping Colophons\n";
	if (bSkipSuperscriptions) std::cout << "Skipping Superscriptions\n";

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
		printResult(letterMatrix, result, bOutputWordsAllUppercase);
	}

	// ------------------------------------------------------------------------

	return 0;
#endif
}

