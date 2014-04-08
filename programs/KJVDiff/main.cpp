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
#include <QList>
#include <QMap>
#include <QSet>
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

// ============================================================================

QMainWindow *g_pMainWindow = NULL;
QTranslator g_qtTranslator;

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0


	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";

}	// namespace

// ============================================================================

class CWordDiffEntry
{
public:
	CWordDiffEntry(const QString &strWord)			// Created with a partially decomposed word
		:	m_strWord(strWord),
			m_strDecomposedWord(CSearchStringListModel::decompose(strWord, true).toLower())
	{
	}

	QString word() const { return m_strWord; }
	QString decomposedWord() const { return m_strDecomposedWord; }

private:
	QString m_strWord;
	QString m_strDecomposedWord;
};

typedef QList<CWordDiffEntry> TWordDiffEntryList;

static int TWordDiffEntryList_removeDuplicates(TWordDiffEntryList &aList)
{
	int n = aList.size();
	int j = 0;
	QSet<QString> seen;
	seen.reserve(n);
	for (int i = 0; i < n; ++i) {
		const CWordDiffEntry &s = aList.at(i);
		if (seen.contains(s.word())) continue;
		seen.insert(s.word());
		if (j != i) aList[j] = s;
		++j;
	}
	if (n != j) aList.erase(aList.begin() + j, aList.end());
	return (n - j);
}

struct TWordDiffEntryListSortPredicate {
	static bool ascendingLessThanWordCaseInsensitive(const CWordDiffEntry &s1, const CWordDiffEntry &s2)
	{
		int nDecompCompare = s1.decomposedWord().compare(s2.decomposedWord(), Qt::CaseInsensitive);
		if (nDecompCompare == 0) {
			return (s1.word().compare(s2.word(), Qt::CaseSensitive) < 0);				// Localized case-sensitive within overall case-insensitive
		}
		return (nDecompCompare < 0);
	}
};

// ============================================================================

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

static int readDatabaseByFilename(const QString &strFilename, bool bSetAsMain)
{
	QFileInfo fiDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);
	QFileInfo fiFile(QDir(fiDBPath.absoluteFilePath()), strFilename);

	std::cerr << QString::fromUtf8("Reading database: %1\n").arg(fiFile.absoluteFilePath()).toUtf8().data();

	CReadDatabase rdbMain(fiDBPath.absoluteFilePath(), QString(), NULL);
	if (!fiFile.exists() || !fiFile.isFile()) {
		std::cerr << QString::fromUtf8("\n*** ERROR: Unable to locate Bible Database File \"%1\"!\n").arg(fiFile.absoluteFilePath()).toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadSpecialBibleDatabase(strFilename, bSetAsMain)) {
		std::cerr << QString::fromUtf8("\n*** ERROR: Failed to Read Bible Database File \"%1\"!\n").arg(fiFile.absoluteFilePath()).toUtf8().data();
		return -3;
	}

	return 0;
}

static QString passageReference(CBibleDatabasePtr pBibleDatabase, bool bAbbrev, const CRelIndex &relIndex)
{
	if (bAbbrev) {
		return pBibleDatabase->PassageReferenceAbbrText(relIndex);
	} else {
		return pBibleDatabase->PassageReferenceText(relIndex);
	}
}

// ============================================================================

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
	QString strFilePathname1;
	QString strFilePathname2;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor1;
	TBibleDescriptor bblDescriptor2;
	bool bUnknownOption = false;
	bool bIgnoreWordsOfJesus = false;
	bool bIgnoreDivineNames = false;
	bool bIgnoreTransChange = false;
	bool bIgnorePilcrows = false;
	bool bExactPilcrows = false;
	bool bIgnoreLemmas = false;
	bool bIgnoreHebrewPs119 = false;
	bool bIgnoreRendering = false;
	bool bIgnoreVerseText = false;
	bool bCaseInsensitive = false;
	bool bAccentInsensitive = false;
	bool bHyphenInsensitive = false;
	bool bUseAbbrevRefs = false;
	bool bUseConcordanceWords = false;
	bool bAllDiffs = true;
	bool bTextDiffs = false;
	bool bWordDiffs = false;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor1 = strArg.toInt();
				strFilePathname1 = strArg;
			} else if (nArgsFound == 2) {
				nDescriptor2 = strArg.toInt();
				strFilePathname2 = strArg;
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
		} else if (strArg.compare("-l") == 0) {
			bIgnoreLemmas = true;
		} else if (strArg.compare("-s") == 0) {
			bIgnoreHebrewPs119 = true;
		} else if (strArg.compare("-r") == 0) {
			bIgnoreRendering = true;
		} else if (strArg.compare("-v") == 0) {
			bIgnoreVerseText = true;
		} else if (strArg.compare("-c") == 0) {
			bCaseInsensitive = true;
		} else if (strArg.compare("-a") == 0) {
			bAccentInsensitive = true;
		} else if (strArg.compare("-h") == 0) {
			bHyphenInsensitive = true;
		} else if (strArg.compare("-b") == 0) {
			bUseAbbrevRefs = true;
		} else if (strArg.compare("-n") == 0) {
			bUseConcordanceWords = true;
		} else if (strArg.compare("-m") == 0) {
			bAllDiffs = false;
			bTextDiffs = true;
		} else if (strArg.compare("-w") == 0) {
			bAllDiffs = false;
			bWordDiffs = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption)) {
		std::cerr << QString("%1 Version %2\n\n").arg(a.applicationName()).arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <File/UUID-Index-1> <File/UUID-Index-2>\n\n").arg(a.applicationName()).toUtf8().data();
		std::cerr << QString("Reads the specified databases and does a comparison for pertinent differences\n").toUtf8().data();
		std::cerr << QString("    and outputs the diff results...\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Databases may be specified by either a file pathname and/or UUID-Index\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("------------\n").toUtf8().data();
		std::cerr << QString("  -j  =  Ignore Words of Jesus\n").toUtf8().data();
		std::cerr << QString("  -d  =  Ignore Divine Names Markup\n").toUtf8().data();
		std::cerr << QString("  -t  =  Ignore Translation Change/Added Markup\n").toUtf8().data();
		std::cerr << QString("  -p  =  Ignore Pilcrows\n").toUtf8().data();
		std::cerr << QString("  -e  =  Match Exact Pilcrows (ignored if -p is set)\n").toUtf8().data();
		std::cerr << QString("  -l  =  Ignore Lemma Tags\n").toUtf8().data();
		std::cerr << QString("  -s  =  Ignore Psalm 119 Hebrew Letter Tags\n").toUtf8().data();
		std::cerr << QString("  -r  =  Ignore rendering differences of punctuation, spaces, etc\n").toUtf8().data();
		std::cerr << QString("  -v  =  Ignore verse text diffs (Verses where words are different)\n").toUtf8().data();
		std::cerr << QString("  -c  =  Case-Insensitive (i.e. Discard case rather than compare them)\n").toUtf8().data();
		std::cerr << QString("  -a  =  Accent-Insensitive (i.e. Discard accents rather than compare them)\n").toUtf8().data();
		std::cerr << QString("  -h  =  Hyphen-Insensitive (i.e. Discard hyphens rather than compare them)\n").toUtf8().data();
		std::cerr << QString("  -b  =  Use Abbreviated Book names when outputting references\n").toUtf8().data();
		std::cerr << QString("  -n  =  Use Concordance Word List instead of General Word List (Word Diffs Only)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Diffs to run:\n").toUtf8().data();
		std::cerr << QString("  -m  =  Text Markup Diffs\n").toUtf8().data();
		std::cerr << QString("  -w  =  Word List Diffs\n").toUtf8().data();
		std::cerr << QString("  (By default All Diffs are run, unless one or more specific diffs is specified)\n\n").toUtf8().data();
		std::cerr << QString("UUID-Index Values:\n").toUtf8().data();
		for (unsigned int ndx = 1; ndx < bibleDescriptorCount(); ++ndx) {
			const TBibleDescriptor &bblDesc(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)));
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bblDesc.m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		return -1;
	}

	if ((nDescriptor1 > 0) && (static_cast<unsigned int>(nDescriptor1) < bibleDescriptorCount())) {
		bblDescriptor1 = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor1));
	} else {
		if (nDescriptor1 != 0) {
			std::cerr << QString::fromUtf8("Unknown UUID-Index: %1\n").arg(nDescriptor1).toUtf8().data();
			return -1;
		}
	}

	if ((nDescriptor2 > 0) && (static_cast<unsigned int>(nDescriptor2) < bibleDescriptorCount())) {
		bblDescriptor2 = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor2));
	} else {
		if (nDescriptor2 != 0) {
			std::cerr << QString::fromUtf8("Unknown UUID-Index: %1\n").arg(nDescriptor2).toUtf8().data();
			return -1;
		}
	}

	// ------------------------------------------------------------------------

	int nReadStatus;

	if (nDescriptor1 != 0) {
		nReadStatus = readDatabase(bblDescriptor1, true);
	} else {
		nReadStatus = readDatabaseByFilename(strFilePathname1, true);
	}
	if (nReadStatus != 0) return nReadStatus;

	if (nDescriptor2 != 0) {
		nReadStatus = readDatabase(bblDescriptor2, false);
	} else {
		nReadStatus = readDatabaseByFilename(strFilePathname2, false);
	}
	if (nReadStatus != 0) return nReadStatus;

	assert(TBibleDatabaseList::instance()->size() == 2);
	CBibleDatabasePtr pBible1 = TBibleDatabaseList::instance()->at(0);
	CBibleDatabasePtr pBible2 = TBibleDatabaseList::instance()->at(1);

	// ------------------------------------------------------------------------

	const CBibleEntry &bblEntry1(pBible1->bibleEntry());
	const CBibleEntry &bblEntry2(pBible2->bibleEntry());

	unsigned int nChkTst = qMin(bblEntry1.m_nNumTst, bblEntry2.m_nNumTst);
	if (bblEntry1.m_nNumTst != bblEntry2.m_nNumTst) {
		std::cout << QString("Number of Testaments don't match!  %1 <=> %2\nComparing only %3 Testaments\n").arg(bblEntry1.m_nNumTst).arg(bblEntry2.m_nNumTst).arg(nChkTst).toUtf8().data();
	}

	unsigned int nChkBk = qMin(bblEntry1.m_nNumBk, bblEntry2.m_nNumBk);
	if (bblEntry1.m_nNumBk != bblEntry2.m_nNumBk) {
		std::cout << QString("Number of Books don't match!  %1 <=> %2\nComparing only %3 Books\n").arg(bblEntry1.m_nNumBk).arg(bblEntry2.m_nNumBk).arg(nChkBk).toUtf8().data();
	}

	CVerseTextRichifierTags vtfTags;
	vtfTags.setAddRichPs119HebrewPrefix(false);
	if (!bIgnoreTransChange) {
		vtfTags.setTransChangeAddedTags("[", "]");
	} else {
		vtfTags.setTransChangeAddedTags(QString(), QString());
	}
	if (!bIgnoreWordsOfJesus) {
		vtfTags.setWordsOfJesusTags("<", ">");
	} else {
		vtfTags.setWordsOfJesusTags(QString(), QString());
	}
	if (!bIgnoreDivineNames) {
		vtfTags.setDivineNameTags("/", "\\");
	} else {
		vtfTags.setDivineNameTags(QString(), QString());
	}
	vtfTags.setShowPilcrowMarkers(false);

	bool bOutputedTextDiffs = false;

	if (bAllDiffs || bTextDiffs) {
		for (unsigned int nBk = 0; nBk < nChkBk; ++nBk) {
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
					QString strRef1 = passageReference(pBible1, bUseAbbrevRefs, ndxVerse1);
					QString strRef2 = passageReference(pBible2, bUseAbbrevRefs, ndxVerse2);
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
					bool bHaveTextDiff = false;
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
					if (bIgnoreLemmas) {
						strTemplate1.remove(QRegExp("[Ll]"));
						strTemplate2.remove(QRegExp("[Ll]"));
					}
					if (bIgnoreHebrewPs119) {
						strTemplate1.remove(QRegExp("[M]"));
						strTemplate2.remove(QRegExp("[M]"));
					}
					if (bIgnoreVerseText) {
						// Leave the "w" in place so that rendering checks will work correctly!
						// strTemplate1.remove(QRegExp("[w]"));
						// strTemplate2.remove(QRegExp("[w]"));
					} else {
						CVerseEntry veNewVerseWords1(*pVerse1);
						QString strWordTemplate1;
						for (unsigned int nWrd = 0; nWrd < veNewVerseWords1.m_nNumWrd; ++nWrd) {
							if (nWrd != 0) {
								strWordTemplate1 += " w";
							} else {
								strWordTemplate1 += "w";
							}
						}
						veNewVerseWords1.m_strTemplate = strWordTemplate1;

						CVerseEntry veNewVerseWords2(*pVerse2);
						QString strWordTemplate2;
						for (unsigned int nWrd = 0; nWrd < veNewVerseWords2.m_nNumWrd; ++nWrd) {
							if (nWrd != 0) {
								strWordTemplate2 += " w";
							} else {
								strWordTemplate2 += "w";
							}
						}
						veNewVerseWords2.m_strTemplate = strWordTemplate2;

						QString strVerseText1 = CVerseTextRichifier::parse(ndxVerse1, pBible1.data(), &veNewVerseWords1, vtfTags);
						QString strVerseText2 = CVerseTextRichifier::parse(ndxVerse2, pBible2.data(), &veNewVerseWords2, vtfTags);
						if (bAccentInsensitive) {
							strVerseText1 = CSearchStringListModel::decompose(strVerseText1, bHyphenInsensitive);
							strVerseText2 = CSearchStringListModel::decompose(strVerseText2, bHyphenInsensitive);
						} else {
							strVerseText1 = CSearchStringListModel::deApostrHyphen(strVerseText1, bHyphenInsensitive);
							strVerseText2 = CSearchStringListModel::deApostrHyphen(strVerseText2, bHyphenInsensitive);
						}
						if (bCaseInsensitive) {
							strVerseText1 = strVerseText1.toLower();
							strVerseText2 = strVerseText2.toLower();
						}
						if (strVerseText1 != strVerseText2) bHaveTextDiff = true;
					}
					if (bIgnoreRendering) {
						strTemplate1.remove(QRegExp("[^DdTtJjLlMw]"));
						strTemplate2.remove(QRegExp("[^DdTtJjLlMw]"));
					}
					CVerseEntry veNewVerse1(*pVerse1);
					veNewVerse1.m_strTemplate = strTemplate1;
					CVerseEntry veNewVerse2(*pVerse2);
					veNewVerse2.m_strTemplate = strTemplate2;
					// Note: deApostrHyphen is used here so that hyphen and apostrophy differences in the rendering markup (like the weird extra hyphen
					//			that exists in Exodus 32:32 doesn't trigger unsubstantiated diffs):
					if (CSearchStringListModel::deApostrHyphen(strTemplate1, false) != CSearchStringListModel::deApostrHyphen(strTemplate2, false)) {
						if (pVerse1->m_strTemplate != strTemplate1) {
							strDiffText += QString("    Template1: \"%1\" <= \"%2\"\n").arg(strTemplate1).arg(pVerse1->m_strTemplate);
						} else {
							strDiffText += QString("    Template1: \"%1\"\n").arg(strTemplate1);
						}
						if (pVerse2->m_strTemplate != strTemplate2) {
							strDiffText += QString("    Template2: \"%1\" <= \"%2\"\n").arg(strTemplate2).arg(pVerse2->m_strTemplate);
						} else {
							strDiffText += QString("    Template2: \"%1\"\n").arg(strTemplate2);
						}
						strDiffText += QString("    Text1: %1\n").arg(CVerseTextRichifier::parse(ndxVerse1, pBible1.data(), &veNewVerse1, vtfTags)).toUtf8().data();
						strDiffText += QString("    Text2: %1\n").arg(CVerseTextRichifier::parse(ndxVerse2, pBible2.data(), &veNewVerse2, vtfTags)).toUtf8().data();
						bHaveDiff = true;
					} else if (bHaveTextDiff) {
						strDiffText += QString("    Text1: %1\n").arg(CVerseTextRichifier::parse(ndxVerse1, pBible1.data(), &veNewVerse1, vtfTags)).toUtf8().data();
						strDiffText += QString("    Text2: %1\n").arg(CVerseTextRichifier::parse(ndxVerse2, pBible2.data(), &veNewVerse2, vtfTags)).toUtf8().data();
						bHaveDiff = true;
					}
					if (bHaveDiff) {
						std::cout << QString("%1%2\n").arg(bOutputedTextDiffs ? QString("--------------------") : QString()).arg(strDiffText).toUtf8().data();
						bOutputedTextDiffs = true;
					}
				}
				while ((nVrs1 < pChapter1->m_nNumVrs) || (nVrs2 < pChapter2->m_nNumVrs)) {
					CRelIndex ndxVerse1 = CRelIndex(nBk+1, nChp1+1, nVrs1+1, 0);
					CRelIndex ndxVerse2 = CRelIndex(nBk+1, nChp2+1, nVrs2+1, 0);
					if (nVrs1 >= pChapter1->m_nNumVrs) {
						std::cout << QString("<<missing>> : %1\n").arg(passageReference(pBible2, bUseAbbrevRefs, ndxVerse2)).toUtf8().data();
						++nVrs2;
					} else if (nVrs2 >= pChapter2->m_nNumVrs) {
						std::cout << QString("%1 : <<missing>>\n").arg(passageReference(pBible1, bUseAbbrevRefs, ndxVerse1)).toUtf8().data();
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
					std::cout << QString("<<missing>> : %1\n").arg(passageReference(pBible2, bUseAbbrevRefs, ndxChapter2)).toUtf8().data();
					++nChp2;
				} else if (nChp2 >= pBook2->m_nNumChp) {
					std::cout << QString("%1 : <<missing>>\n").arg(passageReference(pBible1, bUseAbbrevRefs, ndxChapter1)).toUtf8().data();
					++nChp1;
				} else {
					assert(false);
				}
			}
		}
	}

	if (bAllDiffs || bWordDiffs) {
#define COLUMN_SPACE 2
		int nMaxWordSize = 0;
		for (TConcordanceList::const_iterator itr = pBible1->concordanceWordList().constBegin(); itr != pBible1->concordanceWordList().constEnd(); ++itr) {
			nMaxWordSize = qMax(nMaxWordSize, itr->word().size());
		}

		QString strWordDiffOutput;

		if (bUseConcordanceWords) {
			TWordDiffEntryList lstWordDiffs1;
			lstWordDiffs1.reserve(pBible1->concordanceWordList().size());
			for (TConcordanceList::const_iterator itr = pBible1->concordanceWordList().constBegin(); itr != pBible1->concordanceWordList().constEnd(); ++itr) {
				QString strWord = itr->word();
				if (bAccentInsensitive) {
					strWord = CSearchStringListModel::decompose(strWord, bHyphenInsensitive);
				} else {
					strWord = CSearchStringListModel::deApostrHyphen(strWord, bHyphenInsensitive);
				}
				if (bCaseInsensitive) {
					strWord = strWord.toLower();
				}
				lstWordDiffs1.append(strWord);
			}
			qSort(lstWordDiffs1.begin(), lstWordDiffs1.end(), TWordDiffEntryListSortPredicate::ascendingLessThanWordCaseInsensitive);
			TWordDiffEntryList_removeDuplicates(lstWordDiffs1);

			TWordDiffEntryList lstWordDiffs2;
			lstWordDiffs2.reserve(pBible2->concordanceWordList().size());
			for (TConcordanceList::const_iterator itr = pBible2->concordanceWordList().constBegin(); itr != pBible2->concordanceWordList().constEnd(); ++itr) {
				QString strWord = itr->word();
				if (bAccentInsensitive) {
					strWord = CSearchStringListModel::decompose(strWord, bHyphenInsensitive);
				} else {
					strWord = CSearchStringListModel::deApostrHyphen(strWord, bHyphenInsensitive);
				}
				if (bCaseInsensitive) {
					strWord = strWord.toLower();
				}
				lstWordDiffs2.append(strWord);
			}
			qSort(lstWordDiffs2.begin(), lstWordDiffs2.end(), TWordDiffEntryListSortPredicate::ascendingLessThanWordCaseInsensitive);
			TWordDiffEntryList_removeDuplicates(lstWordDiffs2);

			TWordDiffEntryList::const_iterator itrWordEntry1 = lstWordDiffs1.constBegin();
			TWordDiffEntryList::const_iterator itrWordEntry2 = lstWordDiffs2.constBegin();
			while ((itrWordEntry1 != lstWordDiffs1.constEnd()) || (itrWordEntry2 != lstWordDiffs2.constEnd())) {
				bool bEOL1 = (itrWordEntry1 == lstWordDiffs1.constEnd());
				bool bEOL2 = (itrWordEntry2 == lstWordDiffs2.constEnd());
				QString strKeyWord1 = (!bEOL1 ? (itrWordEntry1->decomposedWord()) : QString());
				QString strKeyWord2 = (!bEOL2 ? (itrWordEntry2->decomposedWord()) : QString());
				QString strLookAhead1 = ((!bEOL1 && ((itrWordEntry1+1) != lstWordDiffs1.constEnd())) ? (itrWordEntry1+1)->decomposedWord() : QString());
				QString strLookAhead2 = ((!bEOL2 && ((itrWordEntry2+1) != lstWordDiffs2.constEnd())) ? (itrWordEntry2+1)->decomposedWord() : QString());

				int nComp = strKeyWord1.compare(strKeyWord2);
				if (nComp == 0) {
					assert(!strKeyWord1.isEmpty() && !strKeyWord2.isEmpty());
					if (itrWordEntry1->word() != itrWordEntry2->word()) {
						strWordDiffOutput += itrWordEntry1->word() + QString(" ").repeated(COLUMN_SPACE + nMaxWordSize - itrWordEntry1->word().size()) + itrWordEntry2->word() + "\n";
					}
					if ((!bEOL1) && ((strKeyWord1 != strLookAhead2) || (strLookAhead1 == strLookAhead2))) ++itrWordEntry1;
					if ((!bEOL2) && ((strKeyWord2 != strLookAhead1) || (strLookAhead1 == strLookAhead2))) ++itrWordEntry2;
				} else if ((nComp < 0) && (!strKeyWord1.isEmpty())) {
					strWordDiffOutput += itrWordEntry1->word() + "\n";
					if (!bEOL1) ++itrWordEntry1;
				} else if ((nComp > 0) && (!strKeyWord2.isEmpty())) {
					strWordDiffOutput += QString(" ").repeated(nMaxWordSize + COLUMN_SPACE) + itrWordEntry2->word() + "\n";
					if (!bEOL2) ++itrWordEntry2;
				} else {
					// We can only be here if nothing is greater than something or we
					//		ran out of input on both sides and yet didn't exit the loop:
					assert(false);
				}
			}
		} else {
			TWordListMap::const_iterator itrWordEntry1 = pBible1->mapWordList().begin();
			TWordListMap::const_iterator itrWordEntry2 = pBible2->mapWordList().begin();
			while ((itrWordEntry1 != pBible1->mapWordList().end()) || (itrWordEntry2 != pBible2->mapWordList().end())) {
				bool bEOL1 = (itrWordEntry1 == pBible1->mapWordList().end());
				bool bEOL2 = (itrWordEntry2 == pBible2->mapWordList().end());
				QString strKeyWord1 = (!bEOL1 ? (itrWordEntry1->first) : QString());
				QString strKeyWord2 = (!bEOL2 ? (itrWordEntry2->first) : QString());

				int nComp = strKeyWord1.compare(strKeyWord2);
				if (nComp == 0) {
					assert(!strKeyWord1.isEmpty() && !strKeyWord2.isEmpty());
					if (!bEOL1) ++itrWordEntry1;
					if (!bEOL2) ++itrWordEntry2;
				} else if ((nComp < 0) && (!strKeyWord1.isEmpty())) {
					strWordDiffOutput += (itrWordEntry1->second).m_strWord + "\n";
					if (!bEOL1) ++itrWordEntry1;
				} else if ((nComp > 0) && (!strKeyWord2.isEmpty())) {
					strWordDiffOutput += QString(" ").repeated(nMaxWordSize + COLUMN_SPACE) + (itrWordEntry2->second).m_strWord + "\n";
					if (!bEOL2) ++itrWordEntry2;
				} else {
					// We can only be here if nothing is greater than something or we
					//		ran out of input on both sides and yet didn't exit the loop:
					assert(false);
				}
			}
		}

		if (!strWordDiffOutput.isEmpty()) {
			if (bOutputedTextDiffs) {
				std::cout << QString("============================================================\n").toUtf8().data();
			}
			std::cout << strWordDiffOutput.toUtf8().data();
		}
	}

//	return a.exec();
	return 0;
}
