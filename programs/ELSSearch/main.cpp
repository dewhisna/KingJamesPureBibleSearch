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

#include "LetterMatrix.h"
#include "FindELS.h"

#ifndef IS_CONSOLE_APP
#include "ELSSearchMainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include "ELSBibleDatabaseSelectDlg.h"
#include <QProgressDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
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

#ifndef IS_CONSOLE_APP

class CProgressLauncher : public QProgressDialog
{
public:
	CProgressLauncher(CReadDatabase &rdbMain, const QString  &strBibleUUID,
					bool bSkipColophons, bool bSkipSuperscriptions)
		:	QProgressDialog(QObject::tr("Reading Bible Database", "ELSSearch"), QString(), 0, 0, nullptr,
			Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint),		// Get rid of the frame to prevent user from closing, as it will crash us
			m_bSkipColophons(bSkipColophons),
			m_bSkipSuperscriptions(bSkipSuperscriptions)
	{
		QFutureWatcher<void> *pWatcher = new QFutureWatcher<void>(this);
		connect(pWatcher, &QFutureWatcher<void>::finished, this, [&]()->void {
			// When the database read finishes, show the main window and close the progress dialog:
			if (!TBibleDatabaseList::instance()->mainBibleDatabase().isNull()) {
				m_pMainWindow = new CELSSearchMainWindow(TBibleDatabaseList::instance()->mainBibleDatabase(),
														 m_bSkipColophons, m_bSkipSuperscriptions);

				m_pMainWindow->show();
				this->close();
			}
		});

		// Read the database on a separate thread while this thread runs the progress dialog:
		pWatcher->setFuture(
			QtConcurrent::run([&]()->void {
				if (!rdbMain.ReadBibleDatabase(TBibleDatabaseList::availableBibleDatabaseDescriptor(strBibleUUID), true)) {
					QMessageBox::critical(nullptr, QApplication::applicationName(), QObject::tr("*** ERROR: Failed to Read the Bible Database!", "ELSSearch"));
					this->close();
				}
			})
		);
	}

	void deleteMainWindow()
	{
		if (m_pMainWindow) delete m_pMainWindow;
	}

private:
	CELSSearchMainWindow *m_pMainWindow = nullptr;
	bool m_bSkipColophons = false;
	bool m_bSkipSuperscriptions = false;
};

#endif

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
	bool bShowUsageHelp = false;
	// ----
	bool bRunMultithreaded = false;
	bool bSkipColophons = false;
	bool bSkipSuperscriptions = false;
	bool bOutputWordsAllUppercase = false;
	unsigned int nBookStart = 0;
	unsigned int nBookEnd = 0;
	// ----
	ELSRESULT_SORT_ORDER_ENUM nSortOrder = ESO_WSR;

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
				bShowUsageHelp = true;
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
			nSortOrder = ESO_WSR;
		} else if (strArg.compare("-owrs") == 0) {
			nSortOrder = ESO_WRS;
		} else if (strArg.compare("-orws") == 0) {
			nSortOrder = ESO_RWS;
		} else if (strArg.compare("-orsw") == 0) {
			nSortOrder = ESO_RSW;
		} else if (strArg.compare("-osrw") == 0) {
			nSortOrder = ESO_SRW;
		} else if (strArg.compare("-oswr") == 0) {
			nSortOrder = ESO_SWR;
		} else if ((strArg.compare("-h") == 0) || (strArg.compare("--help") == 0)) {
			bShowUsageHelp = true;
		} else {
			bShowUsageHelp = true;
		}
	}

#ifndef IS_CONSOLE_APP
	// Launch as a GUI unless user specified enough arguments for console app:
	if ((nArgsFound <= 1) && (!bShowUsageHelp)) {		// Allow Bible Database argument to dialog if passed
		QString strBibleUUID{bibleDescriptor(BDE_KJV).m_strUUID};

		if ((nArgsFound == 1) && (nDescriptor >= 0) &&
			(static_cast<unsigned int>(nDescriptor) < bibleDescriptorCount())) {
			strBibleUUID = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor)).m_strUUID;
		}

		CELSBibleDatabaseSelectDlg dlgBibleSelect{strBibleUUID, bSkipColophons, bSkipSuperscriptions};
		if (dlgBibleSelect.exec() == QDialog::Rejected) return -1;

		CReadDatabase rdbMain;
		if (!rdbMain.haveBibleDatabaseFiles(TBibleDatabaseList::availableBibleDatabaseDescriptor(dlgBibleSelect.bibleUUID()))) {
			QMessageBox::critical(nullptr, QApplication::applicationName(), QObject::tr("*** ERROR: Unable to locate Bible Database Files!", "ELSSearch"));
			return -2;
		}

		CProgressLauncher launcher(rdbMain, dlgBibleSelect.bibleUUID(), dlgBibleSelect.removeColophons(), dlgBibleSelect.removeSuperscriptions());
		launcher.show();
		launcher.ensurePolished();
		launcher.raise();

		int nRetVal = app.exec();

		launcher.deleteMainWindow();

		return nRetVal;
	}
#endif

	for (auto const &strSearchWord : lstSearchWords) if (strSearchWord.size() < 2) bShowUsageHelp = true;	// Each word must have at least two characters
	if (lstSearchWords.isEmpty()) bShowUsageHelp = true;		// Must have at least one search word

	if ((nArgsFound != 4) || (bShowUsageHelp)) {
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
	std::cout << elsresultSortOrderDescription(nSortOrder).toUtf8().data() << "\n";
	std::cout << "\n";

	// Sort results based on sort order:
	sortELSResultList(nSortOrder, lstResults);

	// Print Results:
	std::cout << QString("Found %1 Results:\n").arg(lstResults.size()).toUtf8().data();
	for (auto const & result : lstResults) {
		printResult(letterMatrix, result, bOutputWordsAllUppercase);
	}

	// ------------------------------------------------------------------------

	return 0;
}

