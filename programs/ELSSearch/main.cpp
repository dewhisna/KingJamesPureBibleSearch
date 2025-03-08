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
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QWindow>
#include "../KJVCanOpener/BusyCursor.h"
#else
#include <QCoreApplication>
#endif
#include <QObject>
#include <QDir>				// Needed for call to QFileInfo
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QList>
#include <QMap>
#include <QElapsedTimer>
#include <QFutureSynchronizer>

#include <iostream>
#include <algorithm>		// for std::sort
#include <utility>			// for std::pair and std::swap

#include "../KJVCanOpener/PathConsts.h"

#include "version.h"

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

}	// namespace

// ============================================================================

int runTests(CBibleDatabasePtr pBibleDatabase)
{
	std::cerr << "RelIndexEx Roundtrip Testing";
	for (uint32_t ndx = 0; ndx <= pBibleDatabase->bibleEntry().m_nNumLtr; ++ndx) {
		if ((ndx % 10000) == 0) {
			std::cerr << ".";
			std::cerr.flush();
		}
		CRelIndexEx relIndex = pBibleDatabase->DenormalizeIndexEx(ndx);
		uint32_t ndxTest = pBibleDatabase->NormalizeIndexEx(relIndex);		// Separate variable for debug viewing
		if (ndx != ndxTest) {
			std::cerr << "\n";
			std::cerr << "Normal Index: " << (unsigned int)(ndx) << std::endl;
			std::cerr << "relIndex    : 0x" << QString("%1").arg(relIndex.index(), 8, 16, QChar('0')).toUpper().toUtf8().data()
					  << " +0x" << QString("%1").arg(relIndex.letter(), 2, 16, QChar('0')).toUpper().toUtf8().data() << std::endl;

			std::cerr << "Roundtrip   : " << (unsigned int)(ndxTest) << std::endl;

			// Perform again as a convenient place to attach a breakpoint and watch what happens:
			CRelIndexEx relRedo = pBibleDatabase->DenormalizeIndexEx(ndx);
			uint32_t ndxTest2 = pBibleDatabase->NormalizeIndexEx(relRedo);
			Q_UNUSED(ndxTest2);

			return -1;
		}
	}
	std::cerr << std::endl;

	for (LetterMatrixTextModifierOptionFlags flags = LMTMO_None; flags <= LMTMO_ALL;
		 flags=static_cast<LetterMatrixTextModifierOptionFlags>(static_cast<int>(flags)+1)) {
		std::cerr << "LetterMatrix Roundtrip Test\n";
		std::cerr << "Options: ";
		if (flags == LMTMO_None) {
			std::cerr << "None";
		} else {
			bool bOutput = false;
			if (flags.testFlag(LMTMO_WordsOfJesusOnly)) {
				if (bOutput) std::cerr << ", ";
				std::cerr << "Words of Jesus";
				bOutput = true;
			}
			if (flags.testFlag(LMTMO_RemoveColophons)) {
				if (bOutput) std::cerr << ", ";
				std::cerr << "Remove Colophons";
				bOutput = true;
			}
			if (flags.testFlag(LMTMO_RemoveSuperscriptions)) {
				if (bOutput) std::cerr << ", ";
				std::cerr << "Remove Superscriptions";
				bOutput = true;
			}
			if (flags.testFlag(LMTMO_IncludeBookPrologues)) {
				if (bOutput) std::cerr << ", ";
				std::cerr << "Include Book Prologues";
				bOutput = true;
			}
			if (flags.testFlag(LMTMO_IncludeChapterPrologues)) {
				if (bOutput) std::cerr << ", ";
				std::cerr << "Include Chapter Prologues";
				bOutput = true;
			}
			if (flags.testFlag(LMTMO_IncludeVersePrologues)) {
				if (bOutput) std::cerr << ", ";
				std::cerr << "Include Verse Prologues";
				bOutput = true;
			}
		}
		std::cerr << std::endl;

		CLetterMatrix letterMatrix(pBibleDatabase, flags, LMBPO_None, LMCPO_None, LMVPO_None);
		if (!letterMatrix.runMatrixIndexRoundtripTest()) return -2;
		std::cerr << std::endl;
	}

	std::cerr << "Tests Complete" << std::endl;

	return 0;			// All good
}

// ============================================================================

void printResult(const CLetterMatrix &letterMatrix, const CELSResult &result, LETTER_CASE_ENUM nLetterCase)
{
	QString strWord;
	switch (nLetterCase) {
		case LCE_LOWER:
			strWord = result.m_strWord.toLower();
			break;
		case LCE_UPPER:
			strWord = result.m_strWord.toUpper();
			break;
		case LCE_ORIGINAL:
			strWord = result.m_strWord;
			break;
	}
	std::cout << "----------------------------------------\n";
	std::cout << QString("Word: \"%1\"\n").arg(strWord).toUtf8().data();
	std::cout << QString("Start Location: %1\n").arg(letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxStart, false)).toUtf8().data();
	std::cout << QString("Nominal Location: %1\n").arg(letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxNominal, false)).toUtf8().data();
	std::cout << QString("End Location: %1\n").arg(letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxEnd, false)).toUtf8().data();
	std::cout << QString("Search Type: %1\n").arg(elsSearchTypeDescription(result.m_nSearchType)).toUtf8().data();
	std::cout << QString("Skip: %1\n").arg(result.m_nSkip).toUtf8().data();
	std::cout << QString("Direction: %1\n").arg((result.m_nDirection == Qt::LeftToRight) ? "Forward" : "Reverse").toUtf8().data();
	CRelIndex relPassageStart = CRelIndex(result.m_ndxStart.index());
	uint32_t matrixIndexResult = letterMatrix.matrixIndexFromRelIndex(result.m_ndxStart);
	uint32_t matrixIndexStart = letterMatrix.matrixIndexFromRelIndex(CRelIndexEx(CRelIndex(relPassageStart.book(), relPassageStart.chapter(), relPassageStart.verse(), 0), 0));
	int nMaxDistance = CFindELS::maxDistance(result.m_nSkip, result.m_strWord.size(), result.m_nSearchType);
	int nAvgDistance = (result.m_strWord.size() >= 2) ? nMaxDistance/(result.m_strWord.size() - 1) : nMaxDistance;
	uint32_t martixIndexEnd = matrixIndexResult + nMaxDistance - 1;
	bool bNeedExtraLine = ((martixIndexEnd - matrixIndexStart + 1) % nAvgDistance) != 0;
	martixIndexEnd += nAvgDistance - ((martixIndexEnd - matrixIndexStart + 1) % nAvgDistance);		// Make a whole number of row data
	if (bNeedExtraLine) martixIndexEnd += nAvgDistance;		// Add extra line for consistency, as above always will for the zero case
	int nChar = 0;
	for (uint32_t matrixIndex = matrixIndexStart; matrixIndex <= martixIndexEnd; ++matrixIndex) {
		if (matrixIndex == matrixIndexStart) {
			std::cout << "\n";
			matrixIndexStart += nAvgDistance;
		}
		if (matrixIndex >= static_cast<uint32_t>(letterMatrix.size())) break;
		std::cout << (((matrixIndex == matrixIndexResult) && (nChar < result.m_strWord.size())) ? "[" : " ");
		switch (nLetterCase) {
			case LCE_LOWER:
				std::cout << QString(letterMatrix.at(matrixIndex).toLower()).toUtf8().data();
				break;
			case LCE_UPPER:
				std::cout << QString(letterMatrix.at(matrixIndex).toUpper()).toUtf8().data();
				break;
			case LCE_ORIGINAL:
				std::cout << QString(letterMatrix.at(matrixIndex)).toUtf8().data();
				break;
		}
		std::cout << (((matrixIndex == matrixIndexResult) && (nChar < result.m_strWord.size())) ? "]" : " ");
		if ((matrixIndex == matrixIndexResult) && (nChar < result.m_strWord.size())) {
			matrixIndexResult += CFindELS::nextOffset(result.m_nSkip, nChar, result.m_nSearchType);
			++nChar;
		}
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

class CSplashLauncher : public QSplashScreen
{
	Q_OBJECT
public:
	CSplashLauncher(const QString  &strBibleUUID,
					LetterMatrixTextModifierOptionFlags flagsLMTMO,
					LMBookPrologueOptionFlags flagsLMBPO,
					LMChapterPrologueOptionFlags flagsLMCPO,
					LMVersePrologueOptionFlags flagsLMVPO)
		:	QSplashScreen(QPixmap(":/res/BeholdtheStone.png")),
			m_rdb(this),
			m_strBibleUUID(strBibleUUID),
			m_flagsLMTMO(flagsLMTMO),
			m_flagsLMBPO(flagsLMBPO),
			m_flagsLMCPO(flagsLMCPO),
			m_flagsLMVPO(flagsLMVPO)
	{
		QTimer::singleShot(10, this, SLOT(doLaunch()));
	}

	bool haveMainWindow() const { return (m_pMainWindow != nullptr); }

	void deleteMainWindow()
	{
		if (m_pMainWindow) {
			delete m_pMainWindow;
			m_pMainWindow = nullptr;
		}
	}

protected slots:
	void doLaunch()
	{
		// Based on waitForWindowExposed() function logic used in
		//	QSplashScreen::finish() of Qt Source:
		if (!windowHandle()) createWinId();
		QWindow *pWindow = windowHandle();
		if (!pWindow->isExposed()) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
			QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
			QTimer::singleShot(10, this, SLOT(doLaunch()));
		} else {
			CBusyCursor iAmBusy(this);
			if (!m_rdb.ReadBibleDatabase(TBibleDatabaseList::availableBibleDatabaseDescriptor(m_strBibleUUID), true)) {
				QMessageBox::critical(nullptr, QApplication::applicationName(), QObject::tr("*** ERROR: Failed to Read the Bible Database!", "ELSSearch"));
				close();
			} else {
				// When the database read finishes, show the main window and close the progress dialog:
				if (!TBibleDatabaseList::instance()->mainBibleDatabase().isNull()) {
					m_pMainWindow = new CELSSearchMainWindow(TBibleDatabaseList::instance()->mainBibleDatabase(),
															 m_flagsLMTMO,
															 m_flagsLMBPO,
															 m_flagsLMCPO,
															 m_flagsLMVPO);

					m_pMainWindow->show();
					finish(m_pMainWindow);
				}
			}
		}
	}

private:
	CELSSearchMainWindow *m_pMainWindow = nullptr;
	CReadDatabase m_rdb;
	QString m_strBibleUUID;
	LetterMatrixTextModifierOptionFlags m_flagsLMTMO = LMTMO_None;
	LMBookPrologueOptionFlags m_flagsLMBPO = LMBPO_None;
	LMChapterPrologueOptionFlags m_flagsLMCPO = LMCPO_None;
	LMVersePrologueOptionFlags m_flagsLMVPO = LMVPO_None;
};

#include "main.moc"

#endif

int main(int argc, char *argv[])
{
#ifndef IS_CONSOLE_APP
	QApplication app(argc, argv);
#else
	QCoreApplication app(argc, argv);
#endif
	app.setApplicationVersion(ELSSearch_VERSION);

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	int nDescriptor = -1;
	int nMinSkip = 0;
	int nMaxSkip = 0;
	ELS_SEARCH_TYPE_ENUM nSearchType = ESTE_ELS;
	QStringList lstSearchWords;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	bool bShowUsageHelp = false;
	bool bTestMode = false;
	// ----
	bool bRunMultithreaded = false;
	LetterMatrixTextModifierOptionFlags flagsLMTMO = LMTMO_None;
	LMBookPrologueOptionFlags flagsLMBPO = LMBPO_None;
	LMChapterPrologueOptionFlags flagsLMCPO = LMCPO_None;
	LMVersePrologueOptionFlags flagsLMVPO = LMVPO_None;
	LETTER_CASE_ENUM nLetterCase = LCE_LOWER;
	unsigned int nBookStart = 0;
	unsigned int nBookEnd = 0;
	// ----
	ELSRESULT_SORT_ORDER_ENUM nSortOrder = ESO_RWS;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				static const QRegularExpression regExWordSplit = QRegularExpression("[\\s,]+");
				lstSearchWords = strArg.split(regExWordSplit, Qt::SkipEmptyParts);
			} else if (nArgsFound == 3) {
				nMinSkip = strArg.toInt();
			} else if (nArgsFound == 4) {
				nMaxSkip = strArg.toInt();
			} else {
				bShowUsageHelp = true;
			}
		} else if (strArg.compare("-mt") == 0) {
			bRunMultithreaded = true;
		} else if (strArg.compare("-sc") == 0) {
			flagsLMTMO.setFlag(LMTMO_RemoveColophons, true);
		} else if (strArg.compare("-ss") == 0) {
			flagsLMTMO.setFlag(LMTMO_RemoveSuperscriptions, true);
		} else if (strArg.compare("-sj") == 0) {
			flagsLMTMO.setFlag(LMTMO_WordsOfJesusOnly, true);
		} else if (strArg.compare("-sbp") == 0) {
			flagsLMTMO.setFlag(LMTMO_IncludeBookPrologues, true);
		} else if (strArg.compare("-scp") == 0) {
			flagsLMTMO.setFlag(LMTMO_IncludeChapterPrologues, true);
		} else if (strArg.compare("-u") == 0) {
			nLetterCase = LCE_UPPER;
		} else if (strArg.compare("-o") == 0) {
			nLetterCase = LCE_ORIGINAL;
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
		} else if (strArg.startsWith("-t")) {
			nSearchType = static_cast<ELS_SEARCH_TYPE_ENUM>(strArg.mid(2).toInt());
			if ((nSearchType < ESTE_FIRST) || (nSearchType >= ESTE_COUNT)) bShowUsageHelp = true;
		} else if ((strArg.compare("-h") == 0) || (strArg.compare("--help") == 0)) {
			bShowUsageHelp = true;
		} else if (strArg.compare("--test") == 0) {
			bTestMode = true;
		} else {
			bShowUsageHelp = true;
		}
	}

	if (nSearchType != ESTE_ELS) {
		if (nMinSkip < 1) nMinSkip = 1;
		if (nMaxSkip < 1) nMaxSkip = 1;
	} else {
		if (nMinSkip < 0) nMinSkip = 0;
		if (nMaxSkip < 0) nMaxSkip = 0;
	}
	if (nMaxSkip < nMinSkip) std::swap(nMinSkip, nMaxSkip);

#ifndef IS_CONSOLE_APP

#ifdef Q_OS_MAC
	// On the Mac/Cocoa platform this attribute is enabled by default
	// We override it to ensure shortcuts show in context menus on that platform
	QApplication::setAttribute(Qt::AA_DontShowShortcutsInContextMenus, false);
#endif

	// Launch as a GUI unless user specified enough arguments for console app or is running tests:
	if ((nArgsFound <= 1) && !bShowUsageHelp && !bTestMode) {		// Allow Bible Database argument to dialog if passed
		QString strBibleUUID{bibleDescriptor(BDE_KJV).m_strUUID};

		if ((nArgsFound == 1) && (nDescriptor >= 0) &&
			(static_cast<unsigned int>(nDescriptor) < bibleDescriptorCount())) {
			strBibleUUID = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor)).m_strUUID;
		}

		CELSBibleDatabaseSelectDlg dlgBibleSelect{strBibleUUID, flagsLMTMO, flagsLMBPO, flagsLMCPO, flagsLMVPO};
		if (dlgBibleSelect.exec() == QDialog::Rejected) return -1;

		CReadDatabase rdbMain;
		if (!rdbMain.haveBibleDatabaseFiles(TBibleDatabaseList::availableBibleDatabaseDescriptor(dlgBibleSelect.bibleUUID()))) {
			QMessageBox::critical(nullptr, QApplication::applicationName(), QObject::tr("*** ERROR: Unable to locate Bible Database Files!", "ELSSearch"));
			return -2;
		}

		CSplashLauncher launcher{dlgBibleSelect.bibleUUID(),
								 dlgBibleSelect.textModifierOptions(),
								 dlgBibleSelect.bookPrologueOptions(),
								 dlgBibleSelect.chapterPrologueOptions(),
								 dlgBibleSelect.versePrologueOptions()};
		launcher.show();
		launcher.ensurePolished();
		launcher.raise();

		int nRetVal = app.exec();

		if (!launcher.haveMainWindow()) nRetVal = -3;		// If the main window didn't launch, it's equivalent to failing to read the database in the CLI path
		launcher.deleteMainWindow();

		return nRetVal;
	}
#endif

	// TODO : Finish options for BPO/CPO/VPO

	for (auto const &strSearchWord : lstSearchWords) if (strSearchWord.size() < 2) bShowUsageHelp = true;	// Each word must have at least two characters
	if (lstSearchWords.isEmpty() && !bTestMode) bShowUsageHelp = true;		// Must have at least one search word

	if (((nArgsFound != 4) && ((nSearchType == ESTE_ELS) || (nSearchType == ESTE_FLS)) && !bTestMode) ||
		((nArgsFound != 2) && ((nSearchType != ESTE_ELS) && (nSearchType != ESTE_FLS)) && !bTestMode) ||
		((nArgsFound != 1) && bTestMode) ||
		(bShowUsageHelp)) {
		std::cerr << ELSSearch_APPNAME << " Version " << ELSSearch_VERSION_SEMVER << "\n\n";
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <Words> [<Min-Letter-Skip> <Max-Letter-Skip>]\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads the specified database and apophenic searches for the specified <Words> at\n").toUtf8().data();
		std::cerr << QString("    ELS/FLS skip-distances from <Min-Letter-Skip> to <Max-Letter-Skip>.\n").toUtf8().data();
		std::cerr << QString("    Letter skips are required for ELS and FLS searches, but are optional and\n").toUtf8().data();
		std::cerr << QString("    aren't used for Vortex-Based FLS searches.\n\n").toUtf8().data();
		std::cerr << QString("<Words> = Comma separated list of words to search (each must be at least two characters)\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -h, --help =  Show this usage information\n").toUtf8().data();
		std::cerr << QString("  --test     =  Run regression tests (preempts all search option, only pass <UUID-Index>)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("  -mt    =  Run Multi-Threaded\n").toUtf8().data();
		std::cerr << QString("  -sc    =  Skip Colophons\n").toUtf8().data();
		std::cerr << QString("  -ss    =  Skip Superscriptions\n").toUtf8().data();
		std::cerr << QString("  -sj    =  Search Words of Jesus Only\n").toUtf8().data();
		std::cerr << QString("  -sbp   =  Search Book Prologues (Book Title, Subtitle, etc.)\n").toUtf8().data();
		std::cerr << QString("  -scp   =  Search Chapter Prologues (Chapter Number, etc.)\n").toUtf8().data();
		std::cerr << QString("  -u     =  Print Output Text in all uppercase (default is lowercase)\n").toUtf8().data();
		std::cerr << QString("  -o     =  Print Output Text in original case (default is lowercase)\n").toUtf8().data();
		std::cerr << QString("  -bb<n> =  Begin Searching in Book <n> (defaults to first)\n").toUtf8().data();
		std::cerr << QString("  -be<n> =  End Searching in Book <n>   (defaults to last)\n").toUtf8().data();
		std::cerr << QString("  -owsr  =  Order output by word, skip, then reference\n").toUtf8().data();
		std::cerr << QString("  -owrs  =  Order output by word, reference, then skip\n").toUtf8().data();
		std::cerr << QString("  -orws  =  Order output by reference, word, then skip (this is the default)\n").toUtf8().data();
		std::cerr << QString("  -orsw  =  Order output by reference, skip, then word\n").toUtf8().data();
		std::cerr << QString("  -osrw  =  Order output by skip, reference, then word\n").toUtf8().data();
		std::cerr << QString("  -oswr  =  Order output by skip, word, then reference\n").toUtf8().data();
		std::cerr << QString("  -t<n>  =  Search Type to perform for <n> values as follows:\n").toUtf8().data();
		for (int i = ESTE_FIRST; i < ESTE_COUNT; ++i) {
			std::cerr << QString("            %1 = %2%3\n")
							 .arg(i)
							 .arg(elsSearchTypeDescription(static_cast<ELS_SEARCH_TYPE_ENUM>(i)))
							 .arg((i == ESTE_ELS) ? "  [Default]" : "").toUtf8().data();
		}
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

	if (bTestMode) return runTests(pBibleDatabase);

	CLetterMatrix letterMatrix{pBibleDatabase, flagsLMTMO, flagsLMBPO, flagsLMCPO, flagsLMVPO};

	// ------------------------------------------------------------------------

	// Results storage (for all skips):
	CELSResultList lstResults;

	// Perform the Search:
	QElapsedTimer elapsedTime;
	elapsedTime.start();
	std::cerr << "Searching";

	CFindELS elsFinder(letterMatrix, lstSearchWords, nSearchType);
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
	if (nSearchType == ESTE_ELS) {
		std::cout << "Searching for ELS skips from " << nMinSkip << " to " << nMaxSkip;
		std::cout << " in " << strBookRange.toUtf8().data() << "\n";
	} else if (nSearchType == ESTE_FLS) {
		std::cout << "Searching with FLS multipliers of " << nMinSkip << " to " << nMaxSkip;
		std::cout << " in " << strBookRange.toUtf8().data() << "\n";
	} else {
		std::cout << "Searching in " << strBookRange.toUtf8().data() << "\n";
	}

	QString strTemp = letterMatrix.getOptionDescription(false);
	if (!strTemp.isEmpty()) std::cout << strTemp.toUtf8().data();

	// Print Summary:
	std::cout << "\nWord Occurrence Counts:\n";
	for (int i = 0; i < lstSearchWords.size(); ++i) {
		QString strWord;
		switch (nLetterCase) {
			case LCE_LOWER:
				strWord = lstSearchWords.at(i).toLower();
				break;
			case LCE_UPPER:
				strWord = lstSearchWords.at(i).toUpper();
				break;
			case LCE_ORIGINAL:
				strWord = lstSearchWords.at(i);
				break;
		}
		std::cout << QString("%1 : Forward: %2, Reverse: %3\n").arg(strWord)
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
		printResult(letterMatrix, result, nLetterCase);
	}

	// ------------------------------------------------------------------------

	return 0;
}

