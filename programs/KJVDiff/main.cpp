/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/SearchCompleter.h"
#include "../KJVCanOpener/PhraseEdit.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QObject>
#include <QMainWindow>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
//#include <QtXml>
#include <QStringList>
#include <QtGlobal>
#include <QSettings>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif
#include <QRegExp>

#include <iostream>
#include <set>

QMainWindow *g_pMainWindow = NULL;
QTranslator g_qtTranslator;

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0


	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";

}	// namespace


static int readDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain)
{
	std::cerr << QString::fromUtf8("Reading database: %1\n").arg(bblDesc.m_strDBDesc).toUtf8().data();

	QFileInfo fiDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);

	CReadDatabase rdbMain(fiDBPath.absoluteFilePath(), QString(), NULL);
	if (!rdbMain.haveBibleDatabaseFiles(bblDesc)) {
		std::cerr << QString::fromUtf8("\n*** ERROR: Unable to locate Bible Database Files for %1!\n").arg(bblDesc.m_strDBDesc).toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDesc, bSetAsMain)) {
		std::cerr << QString::fromUtf8("\n*** ERROR: Failed to Read the Bible Database for %1!\n").arg(bblDesc.m_strDBDesc).toUtf8().data();
		return -3;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationName("KJVDiff");
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	int nDescriptor1 = -1;
	int nDescriptor2 = -1;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor1;
	TBibleDescriptor bblDescriptor2;
	bool bUnknownOption = false;
	bool bIgnoreWordsOfJesus = false;
	bool bIgnoreDivineNames = false;
	bool bIgnoreTransChange = false;
	bool bIgnorePilcrows = false;
	bool bExactPilcrows = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor1 = strArg.toInt();
			} else if (nArgsFound == 2) {
				nDescriptor2 = strArg.toInt();
			}
		} else if (strArg.compare("-j") == 0) {
			bIgnoreWordsOfJesus = true;
		} else if (strArg.compare("-d") == 0) {
			bIgnoreDivineNames = true;
		} else if (strArg.compare("-t") == 0) {
			bIgnoreTransChange = true;
		} else if (strArg.compare("-p") == 0) {
			bIgnorePilcrows = true;
		} else if (strArg.compare("-e") == 0) {
			bExactPilcrows = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("%1 Version %2\n\n").arg(a.applicationName()).arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index-1> <UUID-Index-2>\n\n").arg(a.applicationName()).toUtf8().data();
		std::cerr << QString("Reads the specified databases and does a comparison for pertinent differences\n").toUtf8().data();
		std::cerr << QString("    and outputs the diff results...\n\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -j  =  Ignore Words of Jesus\n").toUtf8().data();
		std::cerr << QString("  -d  =  Ignore Divine Names Markup\n").toUtf8().data();
		std::cerr << QString("  -t  =  Ignore Translation Change/Added Markup\n").toUtf8().data();
		std::cerr << QString("  -p  =  Ignore Pilcrows\n").toUtf8().data();
		std::cerr << QString("  -e  =  Match Exact Pilcrows (ignored if -p is set)\n\n").toUtf8().data();
//		std::cerr << QString("  -c  =  Case-Sensitive\n").toUtf8().data();
//		std::cerr << QString("  -a  =  Accent-Sensitive\n").toUtf8().data();
//		std::cerr << QString("  -h  =  Human readable reference text (default is normal index values)\n").toUtf8().data();
//		std::cerr << QString("  -w  =  No word index (only when using '-h')\n").toUtf8().data();
//		std::cerr << QString("  -b  =  Use Abbreviated Book names (only when using '-h')\n").toUtf8().data();
//		std::cerr << QString("  -s  =  Separate Lines (default is comma separated)\n\n").toUtf8().data();
		std::cerr << QString("UUID-Index Values:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			const TBibleDescriptor &bblDesc(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)));
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bblDesc.m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		return -1;
	}

	if ((nDescriptor1 >= 0) && (static_cast<unsigned int>(nDescriptor1) < bibleDescriptorCount())) {
		bblDescriptor1 = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor1));
	} else {
		std::cerr << QString::fromUtf8("Unknown UUID-Index: %1\n").arg(nDescriptor1).toUtf8().data();
		return -1;
	}

	if ((nDescriptor2 >= 0) && (static_cast<unsigned int>(nDescriptor2) < bibleDescriptorCount())) {
		bblDescriptor2 = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor2));
	} else {
		std::cerr << QString::fromUtf8("Unknown UUID-Index: %1\n").arg(nDescriptor2).toUtf8().data();
		return -1;
	}

	// ------------------------------------------------------------------------

	int nReadStatus;

	nReadStatus = readDatabase(bblDescriptor1, true);
	if (nReadStatus != 0) return nReadStatus;

	nReadStatus = readDatabase(bblDescriptor2, false);
	if (nReadStatus != 0) return nReadStatus;

	assert(g_lstBibleDatabases.size() == 2);
	CBibleDatabasePtr pBible1 = g_lstBibleDatabases.at(0);
	CBibleDatabasePtr pBible2 = g_lstBibleDatabases.at(1);

	// ------------------------------------------------------------------------

	const CBibleEntry &bblEntry1(pBible1->bibleEntry());
	const CBibleEntry &bblEntry2(pBible2->bibleEntry());

	if ((bblEntry1.m_nNumTst != bblEntry2.m_nNumTst)) {
		std::cout << QString("Number of Testaments don't match can't compare!  %1 <=> %2\n").arg(bblEntry1.m_nNumTst).arg(bblEntry2.m_nNumTst).toUtf8().data();
		return 1;
	}

	if ((bblEntry1.m_nNumBk != bblEntry2.m_nNumBk)) {
		std::cout << QString("Number of Books don't match can't compare!  %1 <=> %2\n").arg(bblEntry1.m_nNumBk).arg(bblEntry2.m_nNumBk).toUtf8().data();
		return 2;
	}

	CVerseTextRichifierTags vtfTags;
	vtfTags.setAddRichPs119HebrewPrefix(false);
	vtfTags.setTransChangeAddedTags("[", "]");
	vtfTags.setWordsOfJesusTags("<", ">");
	vtfTags.setDivineNameTags("/", "\\");
	vtfTags.setShowPilcrowMarkers(false);

	for (unsigned int nBk = 0; nBk < bblEntry1.m_nNumBk; ++nBk) {
		const CBookEntry *pBook1 = pBible1->bookEntry(nBk+1);
		const CBookEntry *pBook2 = pBible2->bookEntry(nBk+1);
		unsigned int nChp1;
		unsigned int nChp2;
		for (nChp1=nChp2=0; ((nChp1 < pBook1->m_nNumChp) && (nChp2 < pBook2->m_nNumChp)); ++nChp1, ++nChp2) {
			CRelIndex ndxChapter1 = CRelIndex(nBk+1, nChp1+1, 0, 0);
			CRelIndex ndxChapter2 = CRelIndex(nBk+1, nChp2+1, 0, 0);
			const CChapterEntry *pChapter1 = pBible1->chapterEntry(ndxChapter1);
			const CChapterEntry *pChapter2 = pBible2->chapterEntry(ndxChapter2);
			unsigned int nVrs1;
			unsigned int nVrs2;
			for (nVrs1=nVrs2=0; ((nVrs1 < pChapter1->m_nNumVrs) && (nVrs2 < pChapter2->m_nNumVrs)); ++nVrs1, ++nVrs2) {
				CRelIndex ndxVerse1 = CRelIndex(nBk+1, nChp1+1, nVrs1+1, 0);
				CRelIndex ndxVerse2 = CRelIndex(nBk+1, nChp2+1, nVrs2+1, 0);
				const CVerseEntry *pVerse1 = pBible1->verseEntry(ndxVerse1);
				const CVerseEntry *pVerse2 = pBible2->verseEntry(ndxVerse2);
				QString strRef1 = pBible1->PassageReferenceText(ndxVerse1);
				QString strRef2 = pBible2->PassageReferenceText(ndxVerse2);
				QString strDiffText = QString("%1 : %2\n").arg(strRef1).arg(strRef2);
				bool bHaveDiff = false;
				if (pVerse1->m_nNumWrd != pVerse2->m_nNumWrd) {
					strDiffText += QString("    WordCount: %1 <=> %2\n").arg(pVerse1->m_nNumWrd).arg(pVerse2->m_nNumWrd);
					bHaveDiff = true;
				}
				if ((!bIgnorePilcrows) && (pVerse1->m_nPilcrow != pVerse2->m_nPilcrow)) {
					if ((bExactPilcrows) ||
						(!(((pVerse1->m_nPilcrow == CVerseEntry::PTE_NONE) && (pVerse2->m_nPilcrow == CVerseEntry::PTE_EXTRA)) ||
						   ((pVerse1->m_nPilcrow == CVerseEntry::PTE_EXTRA) && (pVerse2->m_nPilcrow == CVerseEntry::PTE_NONE)) ||
						   ((pVerse1->m_nPilcrow == CVerseEntry::PTE_MARKER) && (pVerse2->m_nPilcrow == CVerseEntry::PTE_MARKER_ADDED)) ||
						   ((pVerse1->m_nPilcrow == CVerseEntry::PTE_MARKER_ADDED) && (pVerse2->m_nPilcrow == CVerseEntry::PTE_MARKER))))) {
						strDiffText += QString("    Pilcrows: %1 <=> %2\n").arg(pVerse1->m_nPilcrow).arg(pVerse2->m_nPilcrow);
						bHaveDiff = true;
					}
				}
				QString strTemplate1 = pVerse1->m_strTemplate;
				QString strTemplate2 = pVerse2->m_strTemplate;
				if (bIgnoreDivineNames) {
					strTemplate1.remove(QRegExp("[Dd]"));
					strTemplate2.remove(QRegExp("[Dd]"));
				}
				if (bIgnoreTransChange) {
					strTemplate1.remove(QRegExp("[Tt]"));
					strTemplate2.remove(QRegExp("[Tt]"));
				}
				if (bIgnoreWordsOfJesus) {
					strTemplate1.remove(QRegExp("[Jj]"));
					strTemplate2.remove(QRegExp("[Jj]"));
				}
				if (strTemplate1.compare(strTemplate2) != 0) {
					strDiffText += QString("    Template1: %1\n").arg(pVerse1->m_strTemplate);
					strDiffText += QString("    Template2: %1\n").arg(pVerse2->m_strTemplate);
					strDiffText += QString("    Text1: %1\n").arg(CVerseTextRichifier::parse(ndxVerse1, pBible1.data(), pVerse1, vtfTags)).toUtf8().data();
					strDiffText += QString("    Text2: %1\n").arg(CVerseTextRichifier::parse(ndxVerse2, pBible2.data(), pVerse2, vtfTags)).toUtf8().data();
					bHaveDiff = true;
				}
				if (bHaveDiff) {
					std::cout << QString("%1--------------------\n").arg(strDiffText).toUtf8().data();
				}
			}
			while ((nVrs1 < pChapter1->m_nNumVrs) || (nVrs2 < pChapter2->m_nNumVrs)) {
				CRelIndex ndxVerse1 = CRelIndex(nBk+1, nChp1+1, nVrs1+1, 0);
				CRelIndex ndxVerse2 = CRelIndex(nBk+1, nChp2+1, nVrs2+1, 0);
				if (nVrs1 >= pChapter1->m_nNumVrs) {
					std::cout << QString("<<missing>> : %1\n").arg(pBible2->PassageReferenceText(ndxVerse2)).toUtf8().data();
					++nVrs2;
				} else if (nVrs2 >= pChapter2->m_nNumVrs) {
					std::cout << QString("%1 : <<missing>>\n").arg(pBible1->PassageReferenceText(ndxVerse1)).toUtf8().data();
					++nVrs1;
				} else {
					assert(false);
				}
			}
		}
		while ((nChp1 < pBook1->m_nNumChp) || (nChp2 < pBook2->m_nNumChp)) {
			CRelIndex ndxChapter1 = CRelIndex(nBk+1, nChp1+1, 0, 0);
			CRelIndex ndxChapter2 = CRelIndex(nBk+1, nChp2+1, 0, 0);
			if (nChp1 >= pBook1->m_nNumChp) {
				std::cout << QString("<<missing>> : %1\n").arg(pBible2->PassageReferenceText(ndxChapter2)).toUtf8().data();
				++nChp2;
			} else if (nChp2 >= pBook2->m_nNumChp) {
				std::cout << QString("%1 : <<missing>>\n").arg(pBible1->PassageReferenceText(ndxChapter1)).toUtf8().data();
				++nChp1;
			} else {
				assert(false);
			}
		}
	}

//	return a.exec();
	return 0;
}
