/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/SearchCompleter.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/CSV.h"

#include "xc_KJVDataParse.h"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtXml>
#include <QStringList>
#include <QtGlobal>
#include <QSettings>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif
#include <QBuffer>
#include <QByteArray>

#include <iostream>
#include <set>
#include <map>

#define CHECK_INDEXES 0

const unsigned int VERSION = 10000;		// Version 1.0.0

#define NUM_BK 80u				// Total Books Defined
#define NUM_BK_OT 39u			// Total Books in Old Testament
#define NUM_BK_NT 27u			// Total Books in New Testament
#define NUM_BK_APOC 14u			// Total Books in Apocrypha (KJVA)
#define NUM_TST 3u				// Total Number of Testaments (or pseudo-testaments, in the case of Apocrypha)

typedef QList<QStringList> TChapterVerseCounts;

const QString g_arrChapterVerseCounts[NUM_BK] =
{
	// ---- Begin Old Testament:
	"31,25,24,26,32,22,24,22,29,32,32,20,18,24,21,16,27,33,38,18,34,24,20,67,34,35,46,22,35,43,55,32,20,31,29,43,36,30,23,23,57,38,34,34,28,34,31,22,33,26",
	"22,25,22,31,23,30,25,32,35,29,10,51,22,31,27,36,16,27,25,26,36,31,33,18,40,37,21,43,46,38,18,35,23,35,35,38,29,31,43,38",
	"17,16,17,35,19,30,38,36,24,20,47,8,59,57,33,34,16,30,37,27,24,33,44,23,55,46,34",
	"54,34,51,49,31,27,89,26,23,36,35,16,33,45,41,50,13,32,22,29,35,41,30,25,18,65,23,31,40,16,54,42,56,29,34,13",
	"46,37,29,49,33,25,26,20,29,22,32,32,18,29,23,22,20,22,21,20,23,30,25,22,19,19,26,68,29,20,30,52,29,12",
	"18,24,17,24,15,27,26,35,27,43,23,24,33,15,63,10,18,28,51,9,45,34,16,33",
	"36,23,31,24,31,40,25,35,57,18,40,15,25,20,20,31,13,31,30,48,25",
	"22,23,18,22",
	"28,36,21,22,12,21,17,22,27,27,15,25,23,52,35,23,58,30,24,42,15,23,29,22,44,25,12,25,11,31,13",
	"27,32,39,12,25,23,29,18,13,19,27,31,39,33,37,23,29,33,43,26,22,51,39,25",
	"53,46,28,34,18,38,51,66,28,29,43,33,34,31,34,34,24,46,21,43,29,53",
	"18,25,27,44,27,33,20,29,37,36,21,21,25,29,38,20,41,37,37,21,26,20,37,20,30",
	"54,55,24,43,26,81,40,40,44,14,47,40,14,17,29,43,27,17,19,8,30,19,32,31,31,32,34,21,30",
	"17,18,17,22,14,42,22,18,31,19,23,16,22,15,19,14,19,34,11,37,20,12,21,27,28,23,9,27,36,27,21,33,25,33,27,23",
	"11,70,13,24,17,22,28,36,15,44",
	"11,20,32,23,19,19,73,18,38,39,36,47,31",
	"22,23,15,17,14,14,10,17,32,3",
	"22,13,26,21,27,30,21,22,35,22,20,25,28,22,35,22,16,21,29,29,34,30,17,25,6,14,23,28,25,31,40,22,33,37,16,33,24,41,30,24,34,17",
	"6,12,8,8,12,10,17,9,20,18,7,8,6,7,5,11,15,50,14,9,13,31,6,10,22,12,14,9,11,12,24,11,22,22,28,12,40,22,13,17,13,11,5,26,17,11,9,14,20,23,19,9,6,7,23,13,11,11,17,12,8,12,11,10,13,20,7,35,36,5,24,20,28,23,10,12,20,72,13,19,16,8,18,12,13,17,7,18,52,17,16,15,5,23,11,13,12,9,9,5,8,28,22,35,45,48,43,13,31,7,10,10,9,8,18,19,2,29,176,7,8,9,4,8,5,6,5,6,8,8,3,18,3,3,21,26,9,8,24,13,10,7,12,15,21,10,20,14,9,6",
	"33,22,35,27,23,35,27,36,18,32,31,28,25,35,33,33,28,24,29,30,31,29,35,34,28,28,27,28,27,33,31",
	"18,26,22,16,20,12,29,17,18,20,10,14",
	"17,17,11,16,16,13,13,14",
	"31,22,26,6,30,13,25,22,21,34,16,6,22,32,9,14,14,7,25,6,17,25,18,23,12,21,13,29,24,33,9,20,24,17,10,22,38,22,8,31,29,25,28,28,25,13,15,22,26,11,23,15,12,17,13,12,21,14,21,22,11,12,19,12,25,24",
	"19,37,25,31,31,30,34,22,26,25,23,17,27,22,21,21,27,23,15,18,14,30,40,10,38,24,22,17,32,24,40,44,26,22,19,32,21,28,18,16,18,22,13,30,5,28,7,47,39,46,64,34",
	"22,22,66,22,22",
	"28,10,27,17,17,14,27,18,11,22,25,28,23,23,8,63,24,32,14,49,32,31,49,27,17,21,36,26,21,26,18,32,33,31,15,38,28,23,29,49,26,20,27,31,25,24,23,35",
	"21,49,30,37,31,28,28,27,27,21,45,13",
	"11,23,5,19,15,11,16,14,17,15,12,14,16,9",
	"20,32,21",
	"15,16,15,13,27,14,17,14,15",
	"21",
	"17,10,10,11",
	"16,13,12,13,15,16,20",
	"15,13,19",
	"17,20,19",
	"18,15,20",
	"15,23",
	"21,13,10,14,11,15,14,23,17,12,17,14,9,21",
	"14,17,18,6",
	// ---- Begin New Testament:
	"25,23,17,25,48,34,29,34,38,42,30,50,58,36,39,28,27,35,30,34,46,46,39,51,46,75,66,20",
	"45,28,35,41,43,56,37,38,50,52,33,44,37,72,47,20",
	"80,52,38,44,39,49,50,56,62,42,54,59,35,35,32,31,37,43,48,47,38,71,56,53",
	"51,25,36,54,47,71,53,59,41,42,57,50,38,31,27,33,26,40,42,31,25",
	"26,47,26,37,42,15,60,40,43,48,30,25,52,28,41,40,34,28,41,38,40,30,35,27,27,32,44,31",
	"32,29,31,25,21,23,25,39,33,21,36,21,14,23,33,27",
	"31,16,23,21,13,20,40,13,27,33,34,31,13,40,58,24",
	"24,17,18,18,21,18,16,24,15,18,33,21,14",
	"24,21,29,31,26,18",
	"23,22,21,32,33,24",
	"30,30,21,23",
	"29,23,25,18",
	"10,20,13,18,28",
	"12,17,18",
	"20,15,16,16,25,21",
	"18,26,17,22",
	"16,15,15",
	"25",
	"14,18,19,16,14,20,28,13,28,39,40,29,25",
	"27,26,18,17,20",
	"25,25,22,19,14",
	"21,22,18",
	"10,29,24,21,21",
	"13",
	"14",
	"25",
	"20,29,22,11,14,17,17,13,21,11,19,17,18,20,8,21,18,24,21,15,27,21",
	// ---- Begin Apocrypha/Deuterocanon:
	"58,30,24,63,73,34,15,96,55",
	"40,48,36,52,56,59,70,63,47,59,46,51,58,48,63,78",
	"22,14,17,21,22,17,18,21,6,12,19,22,18,15",
	"16,28,10,15,24,21,32,36,14,23,23,20,20,19,13,25",
	"0,0,0,0,0,0,0,0,0,13,12,6,18,19,16,24",			// 	AddEsth (starts at 4)
	"16,24,19,20,23,25,30,21,18,21,26,27,19,31,19,29,21,25,22",
	"30,18,31,31,15,37,36,19,18,31,34,18,26,27,20,30,32,33,30,32,28,27,28,34,26,29,30,26,28,25,31,24,31,26,20,26,31,34,35,30,24,25,33,22,26,20,25,25,16,29,30",
	"22,35,37,37,9,73",
	"68",
	"64",
	"42",
	"14",
	"64,70,60,61,68,63,50,32,73,89,74,53,53,49,41,24",
	"36,32,40,50,27,31,42,36,29,38,38,45,26,46,39"
};

typedef struct {
	CRelIndex m_ndxStartingChapterVerse;		// Chapter and Verse this book is supposed to start at (used to handle special case Apocrypha entries, like AddEsther)
	QString m_strName;
	QString m_strCommonAbbr;
	QString m_strOsisAbbr;
	QString m_strTableName;
	QString m_strCategory;
	QString m_strDescription;
} TBook;

// Note: When writing the book TOC, the abbreviation field will now be a semicolon separated list.  The FIRST entry will
//			always be the untranslated OSIS Abbreviation!  The rest will be translated "Common Abbreviation" values.  There
//			may be duplicates, particularly between the OSIS Abbreviation and the Common Abbreviations.  In the Common
//			Abbreviations, the FIRST of that list (i.e. the SECOND entry overall) will be the "preferred" abbreviation that
//			will be rendered when using abbreviated book name mode.

#define PSALMS_BOOK_NUM 19

TBook g_arrBooks[NUM_BK];
static void g_setBooks()
{
	const TBook arrBooks[NUM_BK] =
	{
		// ---- Begin Old Testament:
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Genesis", "bookname"), xc_KJVDataParse::tr("Gen;Gn", "bookabbr"), "Gen", "GEN", xc_KJVDataParse::tr("Law", "bookcategory"), xc_KJVDataParse::tr("The First Book of Moses", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Exodus", "bookname"), xc_KJVDataParse::tr("Exod;Exo;Ex", "bookabbr"), "Exod", "EXOD", xc_KJVDataParse::tr("Law", "bookcategory"), xc_KJVDataParse::tr("The Second Book of Moses", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Leviticus", "bookname"), xc_KJVDataParse::tr("Lev;Lv", "bookabbr"), "Lev", "LEV", xc_KJVDataParse::tr("Law", "bookcategory"), xc_KJVDataParse::tr("The Third Book of Moses", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Numbers", "bookname"), xc_KJVDataParse::tr("Num;Nm", "bookabbr"), "Num", "NUM", xc_KJVDataParse::tr("Law", "bookcategory"), xc_KJVDataParse::tr("The Fourth Book of Moses", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Deuteronomy", "bookname"), xc_KJVDataParse::tr("Deut;Deu;Dt", "bookabbr"), "Deut", "DEUT", xc_KJVDataParse::tr("Law", "bookcategory"), xc_KJVDataParse::tr("The Fifth Book of Moses", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Joshua", "bookname"), xc_KJVDataParse::tr("Josh;Jos;Jo", "bookabbr"), "Josh", "JOSH", xc_KJVDataParse::tr("OT Narative", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Judges", "bookname"), xc_KJVDataParse::tr("Judg;Jdg;Jgs", "bookabbr"), "Judg", "JUDG", xc_KJVDataParse::tr("OT Narative", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Ruth", "bookname"), xc_KJVDataParse::tr("Ruth;Rut;Ru", "bookabbr"), "Ruth", "RUTH", xc_KJVDataParse::tr("OT Narative", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Samuel", "bookname"), xc_KJVDataParse::tr("1Sam;1Sm", "bookabbr"), "1Sam", "SAM1", xc_KJVDataParse::tr("OT Narative", "bookcategory"), xc_KJVDataParse::tr("The First Book of Samuel Otherwise Called, The First Book of the Kings", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Samuel", "bookname"), xc_KJVDataParse::tr("2Sam;2Sm", "bookabbr"), "2Sam", "SAM2", xc_KJVDataParse::tr("OT Narative", "bookcategory"), xc_KJVDataParse::tr("The Second Book of Samuel Otherwise Called, The Second Book of the Kings", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Kings", "bookname"), xc_KJVDataParse::tr("1Kgs", "bookabbr"), "1Kgs", "KGS1", xc_KJVDataParse::tr("OT Narative", "bookcategory"), xc_KJVDataParse::tr("The First Book of the Kings Commonly Called, The Third Book of the Kings", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Kings", "bookname"), xc_KJVDataParse::tr("2Kgs", "bookabbr"), "2Kgs", "KGS2", xc_KJVDataParse::tr("OT Narative", "bookcategory"), xc_KJVDataParse::tr("The Second Book of the Kings Commonly Called, The Fourth Book of the Kings", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Chronicles", "bookname"), xc_KJVDataParse::tr("1Chr;1Chron;1Ch", "bookabbr"), "1Chr", "CHR1", xc_KJVDataParse::tr("OT Narative", "bookcategory"), xc_KJVDataParse::tr("The First Book of the Chronicles", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Chronicles", "bookname"), xc_KJVDataParse::tr("2Chr;2Chron;2Ch", "bookabbr"), "2Chr", "CHR2", xc_KJVDataParse::tr("OT Narative", "bookcategory"), xc_KJVDataParse::tr("The Second Book of the Chronicles", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Ezra", "bookname"), xc_KJVDataParse::tr("Ezra;Ezr", "bookabbr"), "Ezra", "EZRA", xc_KJVDataParse::tr("OT Narative", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Nehemiah", "bookname"), xc_KJVDataParse::tr("Neh", "bookabbr"), "Neh", "NEH", xc_KJVDataParse::tr("OT Narative", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Esther", "bookname"), xc_KJVDataParse::tr("Est;Esth", "bookabbr"), "Esth", "ESTH", xc_KJVDataParse::tr("OT Narative", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Job", "bookname"), xc_KJVDataParse::tr("Job;Jb", "bookabbr"), "Job", "JOB", xc_KJVDataParse::tr("Wisdom", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Psalms", "bookname"), xc_KJVDataParse::tr("Ps;Pss", "bookabbr"), "Ps", "PS", xc_KJVDataParse::tr("Wisdom", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Proverbs", "bookname"), xc_KJVDataParse::tr("Prov;Prv;Pv", "bookabbr"), "Prov", "PROV", xc_KJVDataParse::tr("Wisdom", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Ecclesiastes", "bookname"), xc_KJVDataParse::tr("Eccl;Eccles", "bookabbr"), "Eccl", "ECCL", xc_KJVDataParse::tr("Wisdom", "bookcategory"), xc_KJVDataParse::tr("Ecclesiastes; Or, The Preacher", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Song Of Solomon", "bookname"), xc_KJVDataParse::tr("Song;Sg", "bookabbr"), "Song", "SONG", xc_KJVDataParse::tr("Wisdom", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Isaiah", "bookname"), xc_KJVDataParse::tr("Isa;Is", "bookabbr"), "Isa", "ISA", xc_KJVDataParse::tr("Major Prophets", "bookcategory"), xc_KJVDataParse::tr("The Book of the Prophet Isaiah", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Jeremiah", "bookname"), xc_KJVDataParse::tr("Jer", "bookabbr"), "Jer", "JER", xc_KJVDataParse::tr("Major Prophets", "bookcategory"), xc_KJVDataParse::tr("The Book of the Prophet Jeremiah", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Lamentations", "bookname"), xc_KJVDataParse::tr("Lam", "bookabbr"), "Lam", "LAM", xc_KJVDataParse::tr("Major Prophets", "bookcategory"), xc_KJVDataParse::tr("The Lamentations of Jeremiah", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Ezekiel", "bookname"), xc_KJVDataParse::tr("Ezek;Eze;Ez", "bookabbr"), "Ezek", "EZEK", xc_KJVDataParse::tr("Major Prophets", "bookcategory"), xc_KJVDataParse::tr("The Book of the Prophet Ezekiel", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Daniel", "bookname"), xc_KJVDataParse::tr("Dan;Dn", "bookabbr"), "Dan", "DAN", xc_KJVDataParse::tr("Major Prophets", "bookcategory"), xc_KJVDataParse::tr("The Book of <i>the Prophet</i> Daniel", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Hosea", "bookname"), xc_KJVDataParse::tr("Hos", "bookabbr"), "Hos", "HOS", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Joel", "bookname"), xc_KJVDataParse::tr("Joel;Joe;Jl", "bookabbr"), "Joel", "JOEL", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Amos", "bookname"), xc_KJVDataParse::tr("Amos;Amo;Am", "bookabbr"), "Amos", "AMOS", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Obadiah", "bookname"), xc_KJVDataParse::tr("Obad;Oba;Ob", "bookabbr"), "Obad", "OBAD", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Jonah", "bookname"), xc_KJVDataParse::tr("Jonah;Jona;Jon", "bookabbr"), "Jonah", "JONAH", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Micah", "bookname"), xc_KJVDataParse::tr("Mic;Mi", "bookabbr"), "Mic", "MIC", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Nahum", "bookname"), xc_KJVDataParse::tr("Nah;Na", "bookabbr"), "Nah", "NAH", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Habakkuk", "bookname"), xc_KJVDataParse::tr("Hab;Hb", "bookabbr"), "Hab", "HAB", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Zephaniah", "bookname"), xc_KJVDataParse::tr("Zeph;Zep", "bookabbr"), "Zeph", "ZEPH", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Haggai", "bookname"), xc_KJVDataParse::tr("Hag;Hg", "bookabbr"), "Hag", "HAG", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Zechariah", "bookname"), xc_KJVDataParse::tr("Zech;Zec", "bookabbr"), "Zech", "ZECH", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Malachi", "bookname"), xc_KJVDataParse::tr("Mal", "bookabbr"), "Mal", "MAL", xc_KJVDataParse::tr("Minor Prophets", "bookcategory"), "" },
		// ---- Begin New Testament:
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Matthew", "bookname"), xc_KJVDataParse::tr("Matt;Mt", "bookabbr"), "Matt", "MATT", xc_KJVDataParse::tr("NT Narative", "bookcategory"), xc_KJVDataParse::tr("The Gospel According to Saint Matthew", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Mark", "bookname"), xc_KJVDataParse::tr("Mark;Mk", "bookabbr"), "Mark", "MARK", xc_KJVDataParse::tr("NT Narative", "bookcategory"), xc_KJVDataParse::tr("The Gospel According to Saint Mark", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Luke", "bookname"), xc_KJVDataParse::tr("Luke;Lk", "bookabbr"), "Luke", "LUKE", xc_KJVDataParse::tr("NT Narative", "bookcategory"), xc_KJVDataParse::tr("The Gospel According to Saint Luke", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("John", "bookname"), xc_KJVDataParse::tr("John;Jhn;Jn", "bookabbr"), "John", "JOHN", xc_KJVDataParse::tr("NT Narative", "bookcategory"), xc_KJVDataParse::tr("The Gospel According to Saint John", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Acts", "bookname"), xc_KJVDataParse::tr("Acts", "bookabbr"), "Acts", "ACTS", xc_KJVDataParse::tr("NT Narative", "bookcategory"), xc_KJVDataParse::tr("The Acts of the Apostles", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Romans", "bookname"), xc_KJVDataParse::tr("Rom", "bookabbr"), "Rom", "ROM", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul the Apostle to the Romans", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Corinthians", "bookname"), xc_KJVDataParse::tr("1Cor", "bookabbr"), "1Cor", "COR1", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The First Epistle of Paul the Apostle to the Corinthians", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Corinthians", "bookname"), xc_KJVDataParse::tr("2Cor", "bookabbr"), "2Cor", "COR2", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Second Epistle of Paul the Apostle to the Corinthians", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Galatians", "bookname"), xc_KJVDataParse::tr("Gal", "bookabbr"), "Gal", "GAL", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul the Apostle to the Galatians", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Ephesians", "bookname"), xc_KJVDataParse::tr("Eph", "bookabbr"), "Eph", "EPH", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul the Apostle to the Ephesians", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Philippians", "bookname"), xc_KJVDataParse::tr("Phil", "bookabbr"), "Phil", "PHIL", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul the Apostle to the Philippians", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Colossians", "bookname"), xc_KJVDataParse::tr("Col", "bookabbr"), "Col", "COL", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul the Apostle to the Colossians", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Thessalonians", "bookname"), xc_KJVDataParse::tr("1Thess;1Thes;1Th", "bookabbr"), "1Thess", "THESS1", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The First Epistle of Paul the Apostle to the Thessalonians", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Thessalonians", "bookname"), xc_KJVDataParse::tr("2Thess;2Thes;2Th", "bookabbr"), "2Thess", "THESS2", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Second Epistle of Paul the Apostle to the Thessalonains", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Timothy", "bookname"), xc_KJVDataParse::tr("1Tim;1Tm", "bookabbr"), "1Tim", "TIM1", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The First Epistle of Paul the Apostle to Timothy", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Timothy", "bookname"), xc_KJVDataParse::tr("2Tim;2Tm", "bookabbr"), "2Tim", "TIM2", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Second Epistle of Paul the Apostle to Timothy", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Titus", "bookname"), xc_KJVDataParse::tr("Titus;Ti", "bookabbr"), "Titus", "TITUS", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul to Titus", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Philemon", "bookname"), xc_KJVDataParse::tr("Phlm;Philem", "bookabbr"), "Phlm", "PHLM", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul to Philemon", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Hebrews", "bookname"), xc_KJVDataParse::tr("Heb", "bookabbr"), "Heb", "HEB", xc_KJVDataParse::tr("Pauline Epistles", "bookcategory"), xc_KJVDataParse::tr("The Epistle of Paul the Apostle to the Hebrews", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("James", "bookname"), xc_KJVDataParse::tr("Jas", "bookabbr"), "Jas", "JAS", xc_KJVDataParse::tr("General Epistles", "bookcategory"), xc_KJVDataParse::tr("The General Epistle of James", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Peter", "bookname"), xc_KJVDataParse::tr("1Pet;1Pt", "bookabbr"), "1Pet", "PET1", xc_KJVDataParse::tr("General Epistles", "bookcategory"), xc_KJVDataParse::tr("The First General Epistle of Peter", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Peter", "bookname"), xc_KJVDataParse::tr("2Pet;2Pt", "bookabbr"), "2Pet", "PET2", xc_KJVDataParse::tr("General Epistles", "bookcategory"), xc_KJVDataParse::tr("The Second General Epistle of Peter", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 John", "bookname"), xc_KJVDataParse::tr("1John;1Jn", "bookabbr"), "1John", "JOHN1", xc_KJVDataParse::tr("General Epistles", "bookcategory"), xc_KJVDataParse::tr("The First General Epistle of John", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 John", "bookname"), xc_KJVDataParse::tr("2John;2Jn", "bookabbr"), "2John", "JOHN2", xc_KJVDataParse::tr("General Epistles", "bookcategory"), xc_KJVDataParse::tr("The Second General Epistle of John", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("3 John", "bookname"), xc_KJVDataParse::tr("3John;3Jn", "bookabbr"), "3John", "JOHN3", xc_KJVDataParse::tr("General Epistles", "bookcategory"), xc_KJVDataParse::tr("The Third General Epistle of John", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Jude", "bookname"), xc_KJVDataParse::tr("Jude", "bookabbr"), "Jude", "JUDE", xc_KJVDataParse::tr("General Epistles", "bookcategory"), xc_KJVDataParse::tr("The General Epistle of Jude", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Revelation", "bookname"), xc_KJVDataParse::tr("Rev;Rv;Apoc", "bookabbr"), "Rev", "REV", xc_KJVDataParse::tr("Apocalyptic Epistle", "bookcategory"), xc_KJVDataParse::tr("The Revelation of Jesus Christ", "bookdesc") },
		// ---- Begin Apocrypha/Deuterocanon:
		{ CRelIndex(0, 1, 1, 0),  xc_KJVDataParse::tr("1 Esdras", "bookname"), xc_KJVDataParse::tr("1Esd;1Es", "bookabbr"), "1Esd", "ESD1", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The First Book of Esdras", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Esdras", "bookname"), xc_KJVDataParse::tr("2Esd;2Es", "bookabbr"), "2Esd", "ESD2", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Second Book of Esdras", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Tobit", "bookname"), xc_KJVDataParse::tr("Tob;Tb", "bookabbr"), "Tob", "TOB", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Book of Tobit", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Judith", "bookname"), xc_KJVDataParse::tr("Jdt;Jth", "bookabbr"), "Jdt", "JDT", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Book of Judith", "bookdesc") },
		{ CRelIndex(0, 10, 4, 0), xc_KJVDataParse::tr("Additions to Esther", "bookname"), xc_KJVDataParse::tr("AddEst;AddEsth", "bookabbr"), "AddEsth", "ADDESTH", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Rest of the Chapters of the Book of Esther", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Wisdom", "bookname"), xc_KJVDataParse::tr("Wis;Ws", "bookabbr"), "Wis", "WIS", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Book of Wisdom or The Wisdom of Solomon", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Sirach", "bookname"), xc_KJVDataParse::tr("Sir;Ecclus", "bookabbr"), "Sir", "SIR", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Wisdom of Jesus the Son of Sirach, or Ecclesiasticus", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Baruch", "bookname"), xc_KJVDataParse::tr("Bar;Ba", "bookabbr"), "Bar", "BAR", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Book of Baruch", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Prayer of Azariah", "bookname"), xc_KJVDataParse::tr("PrAzar", "bookabbr"), "PrAzar", "PRAZAR", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Prayer of Azariah", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Susanna", "bookname"), xc_KJVDataParse::tr("Sus", "bookabbr"), "Sus", "SUS", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The History of Susanna [in Daniel]", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Bel and the Dragon", "bookname"), xc_KJVDataParse::tr("Bel", "bookabbr"), "Bel", "BEL", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Book of Bel and the Dragon [in Daniel]", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("Prayer of Manasses", "bookname"), xc_KJVDataParse::tr("PrMan", "bookabbr"), "PrMan", "PRMAN", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Prayer of Manasseh, or, The Prayer of Manasses King of Judah", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("1 Maccabees", "bookname"), xc_KJVDataParse::tr("1Macc;1Mc;1Ma", "bookabbr"), "1Macc", "MACC1", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The First Book of the Maccabees", "bookdesc") },
		{ CRelIndex(0, 1, 1, 0), xc_KJVDataParse::tr("2 Maccabees", "bookname"), xc_KJVDataParse::tr("2Macc;2Mc;2Ma", "bookabbr"), "2Macc", "MACC2", xc_KJVDataParse::tr("Apocrypha", "bookcategory"), xc_KJVDataParse::tr("The Second Book of the Maccabees", "bookdesc") }
	};

	for (unsigned int i=0; i<NUM_BK; ++i) {
		g_arrBooks[i] = arrBooks[i];
	}
}

QString g_arrstrTstNames[NUM_TST];
static void g_setTstNames()
{
	const QString arrstrTstNames[NUM_TST] =
		{	xc_KJVDataParse::tr("Old Testament", "testname"),
			xc_KJVDataParse::tr("New Testament", "testname"),
			xc_KJVDataParse::tr("Apocrypha/Deuterocanon", "testname")
		};

	for (unsigned int i=0; i<NUM_TST; ++i) {
		g_arrstrTstNames[i] = arrstrTstNames[i];
	}
}

// ============================================================================
// ============================================================================

// Note: Other Parse Symbols are in ParseSymbols.cpp:

const QChar g_chrParseTag = QChar('|');			// Special tag to put into the verse text to mark parse tags -- must NOT exist in the text

// ============================================================================
// ============================================================================

const char *g_constrTranslationsPath = "../../KJVDataParse/translations/";
const char *g_constrTranslationFilenamePrefix = "kjvdataparse";

// ============================================================================
// ============================================================================

static bool isSpecialWord(BIBLE_DESCRIPTOR_ENUM nBDE, const QString &strLanguage, const CWordEntry &entryWord)
{
	Q_UNUSED(nBDE);

	if (strLanguage.compare("en", Qt::CaseInsensitive) == 0) {
		for (int ndx = 0; ndx < entryWord.m_lstAltWords.size(); ++ndx) {
			QString strDecomposedWord = CSearchStringListModel::decompose(entryWord.m_lstAltWords.at(ndx), true);

			if (strDecomposedWord.compare("abominations", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("am", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("amen", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("ancient", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("and", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("angel", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Babylon", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("bishop", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("branch", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("cherub", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("comforter", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("creator", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("day", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("days", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("devil", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("dominion", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("duke", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("earth", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("elect", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("father", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("father's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("fathers", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("ghost", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("God", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("gods", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("great", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("harlots", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("heaven", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("hell", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("highest", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("him", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("himself", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("his", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("holiness", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("holy", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("is", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Jesus", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Jews", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("judge", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("king", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("kings", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("kings'", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lamb", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("legion", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lion", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lord", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lord's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lords", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lot", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("man", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("man's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("master", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("masters", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("men", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("men's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("mighty", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("moon", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("mother", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("mystery", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Nazareth", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("of", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("one", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("our", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("righteousness", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("sanctuary", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("saviour", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("sceptre", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("shepherd", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("son", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("spirit", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("spirits", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("sun", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("tabernacle", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("that", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("the", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("this", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("thy", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("unknown", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("unto", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("word", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("wormwood", Qt::CaseInsensitive) == 0) return true;
		}
	}

	return false;
}

static bool isProperWord(BIBLE_DESCRIPTOR_ENUM nBDE, const QString &strLanguage, const CWordEntry &entryWord)
{
	bool bIsProperWord = true;

	if ((nBDE == BDE_KJV) || (nBDE == BDE_KJVPCE) || (nBDE == BDE_KJVA)) {
		assert(strLanguage.compare("en" ,Qt::CaseInsensitive) == 0);

		for (int ndx = 0; ((bIsProperWord) && (ndx < entryWord.m_lstAltWords.size())); ++ndx) {
			QString strDecomposedWord = CSearchStringListModel::decompose(entryWord.m_lstAltWords.at(ndx), true);
			if (!strDecomposedWord.at(0).isUpper()) {
				bIsProperWord = false;
			} else {
				//	Lists of "Oridinary" words as extracted:
				//
				//	Sword 1769:				Sword PCE:
				//	-----------				----------
				//	Godward					Godward
				//	jointheirs				jointheirs
				//	theeward
				//	usward					usward
				//	youward					youward

				if (strDecomposedWord.compare("Godward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("jointheirs", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("theeward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("usward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("youward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				}
			}
		}
	} else {
		for (int ndx = 0; ((bIsProperWord) && (ndx < entryWord.m_lstAltWords.size())); ++ndx) {
			QString strDecomposedWord = CSearchStringListModel::decompose(entryWord.m_lstAltWords.at(ndx), true);
			if (!strDecomposedWord.at(0).isUpper()) {
				bIsProperWord = false;
			}
		}
	}

	return bIsProperWord;
}

// ============================================================================
// ============================================================================

// TAltWordSet will be a set containing all of the case-forms of a given word.  It's easier
//		to map them here as a set than in the list that the database itself uses.  The
//		TAltWordListMap will be indexed by the Lower-Case word key and will map to
//		the set of word forms for that key:
typedef std::set<QString, CWordEntry::SortPredicate> TAltWordSet;
typedef std::map<QString, TAltWordSet, CWordEntry::SortPredicate> TAltWordListMap;

// WordFromWordSet - Drives word toward lower-case and returns the resulting word.  The
//		theory is that proper names will always be capitalized and non-proper names will
//		have mixed case, being capital only when they start a new sentence.  Thus, if we
//		drive toward lower-case, we should have an all-lower-case word for non-proper
//		name words and mixed-case for proper names:
static QString WordFromWordSet(const TAltWordSet &setAltWords)
{
	QString strWord;

	for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
		if ((strWord.isEmpty()) ||
			(((*itrAltWrd).compare(strWord)) > 0)) strWord = *itrAltWrd;
	}

	return strWord;
}

// ============================================================================
// ============================================================================

// Like the QXmlAttributes::index() function, but case-insensitive
static int findAttribute(const QXmlAttributes &attr, const QString &strName)
{
	for (int i = 0; i < attr.count(); ++i) {
		if (attr.localName(i).compare(strName, Qt::CaseInsensitive) == 0) return i;
	}
	return -1;
}

static QString stringifyAttributes(const QXmlAttributes &attr)
{
	QString strTemp;
	for (int i=0; i<attr.count(); ++i) {
		if (i) strTemp += ',';
		strTemp += attr.localName(i) + '=' + attr.value(i);
	}
	return strTemp;
}

//static QXmlAttributes attributesFromString(const QString &str)
//{
//	QXmlAttributes attrs;
//	QStringList lstPairs = str.split(',');
//	for (int i=0; i<lstPairs.count(); ++i) {
//		QStringList lstEntry = lstPairs.at(i).split('=');
//		assert(lstEntry.count() == 2);
//		if (lstEntry.count() != 2) {
//			std::cerr << "\n*** Error: Attributes->String failure\n";
//			continue;
//		}
//		attrs.append(lstEntry.at(0), QString(), lstEntry.at(0), lstEntry.at(1));
//	}
//	return attrs;
//}

// ============================================================================
// ============================================================================

class CStrongsImpXmlHandler : public QXmlDefaultHandler
{
	enum STRONGS_IMP_PARSER_STATE {
		SIPSE_ENTRYFREE = 0,
		SIPSE_TITLE = 1,
		SIPSE_ORTH = 2,
		SIPSE_TRANSLITERATION = 3,
		SIPSE_PRONUNCIATION = 4,
		SIPSE_DEFINITION = 5,
		SIPSE_REFERENCE = 6,
		SIPSE_RENDER = 7,
	};

public:
	CStrongsImpXmlHandler(const QString &strExpectedTextIndex)
		:	m_strongsEntry(strExpectedTextIndex),
			m_bRefIsStrongs(false),
			m_strExpectedTextIndex(strExpectedTextIndex)
	{ }

	virtual ~CStrongsImpXmlHandler()
	{ }

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
	virtual bool characters(const QString &ch) override;
	virtual bool error(const QXmlParseException &exception) override;
	virtual QString errorString() const override
	{
		return (!m_strErrorString.isEmpty() ? m_strErrorString : QXmlDefaultHandler::errorString());
	}
	virtual bool endDocument() override;

	const CStrongsEntry &strongsEntry() const { return m_strongsEntry; }

private:
	void beginRenderElement(const QString &strRendType);
	void endRenderElement();

private:
	CStrongsEntry m_strongsEntry;

	QVector<STRONGS_IMP_PARSER_STATE> m_vctParseState;	// Parse State Stack
	bool m_bRefIsStrongs;						// Set to True in the SIPSE_REFERENCE state if the reference is for Strongs
	QString m_strRefText;						// Text inside the <a> tags for references for Strongs format conversion
	QString m_strExpectedTextIndex;				// Strongs Text Index expected from IMP file during construction of this parser (ex: 'G0001')
	QString m_strErrorString;

	QString m_strEntryTextIndex;				// derived from entryFree 'n' attribute, compared with expected TextIndex and with 'title' element (ex: 'G0001')
	QString m_strCurrentMapIndex;				// derived from text on 'title' element (ex: 'G1'), compared with expected TextIndex

	QString m_strEntryOrthographicIndex;		// derived from entryFree 'n' attribute, compared with 'orth' element, is PlainText whereas the 'orth' element in the StrongsEntry is RichText

	QStringList m_lstRenderElementStack;		// Corresponding Render Element to output when we hit endRenderElement(), pushed in beginRenderElement()
};

void CStrongsImpXmlHandler::beginRenderElement(const QString &strRendType)
{
	if (strRendType.compare("bold", Qt::CaseInsensitive) == 0) {
		characters("<b>");
		m_lstRenderElementStack.push_back("</b>");
	} else if (strRendType.compare("italic", Qt::CaseInsensitive) == 0) {
		characters("<i>");
		m_lstRenderElementStack.push_back("</i>");
	} else if (strRendType.compare("super", Qt::CaseInsensitive) == 0) {
		characters("<sup>");
		m_lstRenderElementStack.push_back("</sup>");
	} else if (strRendType.compare("sub", Qt::CaseInsensitive) == 0) {
		characters("<sub>");
		m_lstRenderElementStack.push_back("</sub>");
	} else {
		// Unknown rendering types placeholder:
		m_lstRenderElementStack.push_back(QString());		// Keep stack balanced
	}
	m_vctParseState.push_back(SIPSE_RENDER);
}

void CStrongsImpXmlHandler::endRenderElement()
{
	assert(!m_vctParseState.isEmpty());
	assert(m_vctParseState.back() == SIPSE_RENDER);
	m_vctParseState.pop_back();
	assert(!m_lstRenderElementStack.isEmpty());
	characters(m_lstRenderElementStack.back());
	m_lstRenderElementStack.pop_back();
}

bool CStrongsImpXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

/*

	Strongs IMP Format:
	-------------------
	$$$G0001
	<entryFree n="G0001|Α"> <title>G1</title> <orth>Α</orth> <orth rend="bold" type="trans">A</orth>
	<pron rend="italic">al'-fah</pron><lb/> <def>Of Hebrew origin; the first letter of the alphabet:
	figuratively only (from its use as a numeral) the <hi rend="italic">first</hi>. Often used (usually
	“an” before a vowel) also in composition (as a contraction from <ref target="Strong:G0427">G427</ref>)
	in the sense of <hi rend="italic">privation</hi>; so in many words beginning with this letter;
	occasionally in the sense of <hi rend="italic">union</hi> (as a contraction of
	<ref target="Strong:G0260">G260</ref>): - Alpha.</def> </entryFree>

*/

	int ndx = -1;

	if (localName.compare("entryFree", Qt::CaseInsensitive) == 0) {
		if (!m_vctParseState.isEmpty()) {
			std::cerr << QString("*** Error: Strongs Imp Parse State not empty at entryFree start for %1\n").arg(m_strExpectedTextIndex).toUtf8().data();
			m_vctParseState.clear();
		}

		m_vctParseState.push_back(SIPSE_ENTRYFREE);

		m_strCurrentMapIndex.clear();
		m_strongsEntry.setOrthography(QString());

		ndx = findAttribute(atts, "n");
		if (ndx != -1) {
			QStringList lstIndex = atts.value(ndx).split('|');
			if (lstIndex.size() < 2) {
				std::cerr << QString("*** Warning: On %1 - Strongs Imp entryFree missing proper 'n' attribute: %2\n").arg(m_strExpectedTextIndex).arg(stringifyAttributes(atts)).toUtf8().data();
			} else {
				m_strEntryTextIndex = lstIndex.at(0);
				m_strEntryOrthographicIndex = lstIndex.at(1);
				if (lstIndex.size() > 2) {
					std::cerr << QString("*** Warning: On %1 - Strongs Imp entryFree with extraneous 'n' attribute: %2\n").arg(m_strExpectedTextIndex).arg(atts.value(ndx)).toUtf8().data();
				}
			}
		} else {
			m_strEntryTextIndex.clear();
			m_strEntryOrthographicIndex.clear();
		}
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		m_vctParseState.push_back(SIPSE_TITLE);

		m_strCurrentMapIndex.clear();		// Redundant, but we'll do it in case an entry has multiple 'title' elements
	} else if (localName.compare("orth", Qt::CaseInsensitive) == 0) {
		ndx = findAttribute(atts, "type");
		if ((ndx != -1) && (atts.value(ndx).compare("trans", Qt::CaseInsensitive) == 0)) {
			m_vctParseState.push_back(SIPSE_TRANSLITERATION);
			m_strongsEntry.setTransliteration(QString());
		} else {
			m_vctParseState.push_back(SIPSE_ORTH);
			m_strongsEntry.setOrthography(QString());
		}

		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("pron", Qt::CaseInsensitive) == 0) {
		m_vctParseState.push_back(SIPSE_PRONUNCIATION);
		m_strongsEntry.setPronunciation(QString());

		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("def", Qt::CaseInsensitive) == 0) {
		m_vctParseState.push_back(SIPSE_DEFINITION);
		m_strongsEntry.setDefinition(QString());

		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("hi", Qt::CaseInsensitive) == 0) {
		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("ref", Qt::CaseInsensitive) == 0) {
		ndx = findAttribute(atts, "target");
		if (ndx != -1) {
			// Push the reference attribute into current text element before we push the reference state
			m_bRefIsStrongs = (atts.value(ndx).indexOf("Strong:", 0, Qt::CaseInsensitive) >= 0);
			m_strRefText.clear();
			characters(QString("<a href=\"%1\">").arg(atts.value(ndx).replace("Strong:", "strong://", Qt::CaseInsensitive)));
			m_vctParseState.push_back(SIPSE_REFERENCE);
		}	// Ignore references with no target
	}
	// Ignore other elements, like 'lb' (lb will get converted in the endElement)

	return true;
}

bool CStrongsImpXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

	if (localName.compare("entryFree", Qt::CaseInsensitive) == 0) {
		assert(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_ENTRYFREE) {
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected entryFree endElement";
			return false;
		}
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		assert(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_TITLE) {
			// Make sure all of our indexes match:
			StrongsIndexSortPredicate isp;
			if ((isp(m_strCurrentMapIndex, m_strEntryTextIndex) != false) ||
				(isp(m_strEntryTextIndex, m_strCurrentMapIndex) != false) ||
				(isp(m_strCurrentMapIndex, m_strExpectedTextIndex) != false) ||
				(isp(m_strExpectedTextIndex, m_strCurrentMapIndex) != false)) {
				std::cerr << QString("\n*** Mismatched Current, Title, and Expected Text Indexes : "
										"Current=%1, Title=%2, Expected=%3\n")
										.arg(m_strCurrentMapIndex)
										.arg(m_strEntryTextIndex)
										.arg(m_strExpectedTextIndex).toUtf8().data();
			}
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected title endElement";
			return false;
		}

	} else if (localName.compare("orth", Qt::CaseInsensitive) == 0) {
		assert(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
			assert(!m_vctParseState.isEmpty());
		}
		if ((m_vctParseState.back() == SIPSE_ORTH) ||
			(m_vctParseState.back() == SIPSE_TRANSLITERATION)) {
			// Make sure orthography indexes match:
			if (m_vctParseState.back() == SIPSE_ORTH) {
				if (m_strongsEntry.orthographyPlainText() != m_strEntryOrthographicIndex) {
					std::cerr << QString("\n*** Mismatched Orthography on %1 : n=\"%2\", orth=\"%3\", orthPlainText=\"%4\"\n")
											.arg(m_strExpectedTextIndex)
											.arg(m_strEntryOrthographicIndex)
											.arg(m_strongsEntry.orthography())
											.arg(m_strongsEntry.orthographyPlainText()).toUtf8().data();
				}
			}
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected orth endElement";
			return false;
		}
	} else if (localName.compare("pron", Qt::CaseInsensitive) == 0) {
		assert(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
			assert(!m_vctParseState.isEmpty());
		}
		if (m_vctParseState.back() == SIPSE_PRONUNCIATION) {
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected pron endElement";
			return false;
		}
	} else if (localName.compare("def", Qt::CaseInsensitive) == 0) {
		assert(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
			assert(!m_vctParseState.isEmpty());
		}
		if (m_vctParseState.back() == SIPSE_DEFINITION) {
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected def endElement";
			return false;
		}
	} else if (localName.compare("hi", Qt::CaseInsensitive) == 0) {
		assert(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
		}
	} else if (localName.compare("ref", Qt::CaseInsensitive) == 0) {
		assert(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_REFERENCE) {
			if (m_bRefIsStrongs) {
				m_bRefIsStrongs = false;		// Clear flag so character push goes to parent:
				CStrongsEntry tmpEntry(m_strRefText);
				characters(tmpEntry.strongsTextIndex());
			}
			m_vctParseState.pop_back();
			characters("</a>");
		}
	} else if (localName.compare("lb", Qt::CaseInsensitive) == 0) {
		characters("<br/>");
	}
	// Ignore other elements

	return true;
}

bool CStrongsImpXmlHandler::characters(const QString &ch)
{
	assert(!m_vctParseState.isEmpty());
	STRONGS_IMP_PARSER_STATE parseState = m_vctParseState.back();

	if ((parseState == SIPSE_REFERENCE) && m_bRefIsStrongs) {
		m_strRefText += ch;
		return true;
	}

	// Push data inside references and render elements up to
	//	the parent element:
	int ndxParent = 2;
	while ((parseState == SIPSE_REFERENCE) ||
			(parseState == SIPSE_RENDER)) {
		assert(m_vctParseState.size() >= ndxParent);
		parseState = m_vctParseState.at(m_vctParseState.size()-ndxParent);
		++ndxParent;
	}

	switch (parseState) {
		case SIPSE_ENTRYFREE:
			// Ignore characters (like spaces) in our entry not assigned to a specific function
			break;
		case SIPSE_TITLE:
			m_strCurrentMapIndex.append(ch);
			break;
		case SIPSE_ORTH:
			m_strongsEntry.setOrthography(m_strongsEntry.orthography() + ch);
			break;
		case SIPSE_TRANSLITERATION:
			m_strongsEntry.setTransliteration(m_strongsEntry.transliteration() + ch);
			break;
		case SIPSE_PRONUNCIATION:
			m_strongsEntry.setPronunciation(m_strongsEntry.pronunciation() + ch);
			break;
		case SIPSE_DEFINITION:
			m_strongsEntry.setDefinition(m_strongsEntry.definition() + ch);
			break;
		case SIPSE_REFERENCE:
		case SIPSE_RENDER:
			// This state can't happen due to parent seeking above
			assert(false);
			return false;
	}

	return true;
}

bool CStrongsImpXmlHandler::error(const QXmlParseException &exception)
{
	std::cerr << QString("\n\n*** %1\n").arg(exception.message()).toUtf8().data();
	return true;
}

bool CStrongsImpXmlHandler::endDocument()
{
	return true;
}

// ============================================================================
// ============================================================================

class COSISXmlHandler : public QXmlDefaultHandler
{
public:
	enum XML_FORMAT_TYPE_ENUM {
		XFTE_UNKNOWN = -1,
		XFTE_OSIS = 0,
		XFTE_ZEFANIA = 1
	};

	COSISXmlHandler(const TBibleDescriptor &bblDesc)
		:	m_xfteFormatType(XFTE_UNKNOWN),
			m_bInHeader(false),
			m_bCaptureTitle(false),
			m_bCaptureLang(false),
			m_bOpenEndedChapter(false),
			m_bInVerse(false),
			m_bOpenEndedVerse(false),
			m_bInLemma(false),
			m_bInTransChangeAdded(false),
			m_bInNotes(false),
			m_bInBracketNotes(false),
			m_bInColophon(false),
			m_bOpenEndedColophon(false),
			m_bInSuperscription(false),
			m_bOpenEndedSuperscription(false),
			m_bInForeignText(false),
			m_bInWordsOfJesus(false),
			m_bInDivineName(false),
			m_nDelayedPilcrow(CVerseEntry::PTE_NONE),
			m_strLanguage("en"),
			m_bNoColophonVerses(false),
			m_bUseBracketColophons(false),
			m_bDisableColophons(false),
			m_bNoSuperscriptionVerses(false),
			m_bDisableSuperscriptions(false),
			m_bBracketItalics(false),
			m_bNoArabicNumeralWords(false),
			m_bInlineFootnotes(false),
			m_bUseBracketFootnotes(false),
			m_bUseBracketFootnotesExcluded(false),
			m_bExcludeDeuterocanonical(false),
			m_bFoundSegVariant(false)
	{
		g_setBooks();
		g_setTstNames();
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(bblDesc));		// Note: We'll set the name and description later in the reading of the data
		for (unsigned int i=0; i<NUM_BK; ++i) {
			m_lstOsisBookList.append(g_arrBooks[i].m_strOsisAbbr);
		}
	}

	virtual ~COSISXmlHandler()
	{

	}

	// Properties:
	void setNoColophonVerses(bool bNoColophonVerses) { m_bNoColophonVerses = bNoColophonVerses; }
	bool noColophonVerses() const { return m_bNoColophonVerses; }
	void setUseBracketColophons(bool bUseBracketColophons) { m_bUseBracketColophons = bUseBracketColophons; }
	bool useBracketColophons() const { return m_bUseBracketColophons; }
	void setDisableColophons(bool bDisableColophons) { m_bDisableColophons = bDisableColophons; }
	bool disableColophons() const { return m_bDisableColophons; }
	void setNoSuperscriptionVerses(bool bNoSuperscriptionVerses) { m_bNoSuperscriptionVerses = bNoSuperscriptionVerses; }
	bool noSuperscriptionVerses() const { return m_bNoSuperscriptionVerses; }
	void setDisableSuperscriptions(bool bDisableSuperscriptions) { m_bDisableSuperscriptions = bDisableSuperscriptions; }
	bool disableSuperscriptions() const { return m_bDisableSuperscriptions; }
	void setBracketItalics(bool bBracketItalics) { m_bBracketItalics = bBracketItalics; }
	bool bracketItalics() const { return m_bBracketItalics; }
	void setNoArabicNumeralWords(bool bNoArabicNumeralWords) { m_bNoArabicNumeralWords = bNoArabicNumeralWords; }
	bool noArabicNumeralWords() const { return m_bNoArabicNumeralWords; }
	void setInlineFootnotes(bool bInlineFootnotes) { m_bInlineFootnotes = bInlineFootnotes; }
	bool inlineFootnotes() const { return m_bInlineFootnotes; }
	void setUseBracketFootnotes(bool bUseBracketFootnotes) { m_bUseBracketFootnotes = bUseBracketFootnotes; }
	bool useBracketFootnotes() const { return m_bUseBracketFootnotes; }
	void setUseBracketFootnotesExcluded(bool bUseBracketFootnotesExcluded) { m_bUseBracketFootnotesExcluded = bUseBracketFootnotesExcluded; }
	bool useBracketFootnotesExcluded() const { return m_bUseBracketFootnotesExcluded; }
	void setExcludeDeuterocanonical(bool bExcludeDeuterocanonical) { m_bExcludeDeuterocanonical = bExcludeDeuterocanonical; }
	bool excludeDeuterocanonical() const { return m_bExcludeDeuterocanonical; }
	void setSegVariant(const QString &strSegVariant) { m_strSegVariant = strSegVariant; }
	QString segVariant() const { return m_strSegVariant; }
	bool foundSegVariant() const { return m_bFoundSegVariant; }
	void setStrongsImpFilepath(const QString &strFilepath) { m_strStrongsImpFilepath = strFilepath; }
	QString strongsImpFilepath() const { return m_strStrongsImpFilepath; }

	// Parsing:
	QStringList elementNames() const { return m_lstElementNames; }
	QStringList attrNames() const { return m_lstAttrNames; }

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
	virtual bool characters(const QString &ch) override;
	virtual bool error(const QXmlParseException &exception) override;
	virtual QString errorString() const override
	{
		return (!m_strErrorString.isEmpty() ? m_strErrorString : QXmlDefaultHandler::errorString());
	}
	virtual bool endDocument() override;

	const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase.data(); }

	QString language() const { return m_strLanguage; }

	QString parsedUTF8Chars() const { return m_strParsedUTF8Chars; }

	const CBookEntry *addBookToBibleDatabase(unsigned int nBk);

protected:
	void startVerseEntry(const CRelIndex &relIndex, bool bOpenEnded);
	void charactersVerseEntry(const CRelIndex &relIndex, const QString &strText);
	void endVerseEntry(CRelIndex &relIndex);
	CRelIndex &activeVerseIndex()
	{
		// Note: Can have nested superscriptions/colophon in verse, but not
		//			vice versa.  So, do these in order of precedence:
		if (m_bInColophon) return m_ndxColophon;
		if (m_bInSuperscription) return m_ndxSuperscription;
		assert(m_bInVerse);
		return m_ndxCurrent;
	}
	CVerseEntry &activeVerseEntry()
	{
		CRelIndex &ndxActive = activeVerseIndex();
		return (m_pBibleDatabase->m_lstBookVerses[ndxActive.book()-1])[CRelIndex(ndxActive.book(), ndxActive.chapter(), ndxActive.verse(), 0)];
	}


private:
	XML_FORMAT_TYPE_ENUM m_xfteFormatType;
	QString m_strErrorString;

	QString m_strNamespace;
	QStringList m_lstElementNames;
	QStringList m_lstAttrNames;

	CRelIndex m_ndxCurrent;
	CRelIndex m_ndxColophon;
	CRelIndex m_ndxSuperscription;
	bool m_bInHeader;
	bool m_bCaptureTitle;
	bool m_bCaptureLang;
	bool m_bOpenEndedChapter;
	bool m_bInVerse;
	bool m_bOpenEndedVerse;
	bool m_bInLemma;
	bool m_bInTransChangeAdded;
	bool m_bInNotes;
	bool m_bInBracketNotes;
	bool m_bInColophon;
	bool m_bOpenEndedColophon;				// Open-ended colophons use sID/eID attributes in <div /> tags to start stop them rather than enclosing the whole colophon in a single specific <div></div> section
	bool m_bInSuperscription;
	bool m_bOpenEndedSuperscription;
	bool m_bInForeignText;
	bool m_bInWordsOfJesus;
	bool m_bInDivineName;
	CVerseEntry::PILCROW_TYPE_ENUM m_nDelayedPilcrow;		// Used to flag a pilcrow to appear in the next verse -- used for <CM> tag from German Schlachter text
	QString m_strParsedUTF8Chars;		// UTF-8 (non-Ascii) characters encountered -- used for report

	CBibleDatabasePtr m_pBibleDatabase;
	QString m_strLanguage;
	QStringList m_lstOsisBookList;
	bool m_bNoColophonVerses;			// Note: This is colophons as "pseudo-verses" only not colophons in general, which are also written as footnotes
	bool m_bUseBracketColophons;		// Treat "[" and "]" as a colophon marker
	bool m_bDisableColophons;			// Disable all colophon output (both pseudo-verses and footnote version)
	bool m_bNoSuperscriptionVerses;		// Note: This is superscriptions as "pseudo-verses" only not superscriptions in general, which are also written as footnotes
	bool m_bDisableSuperscriptions;		// Disable all superscription output (both pseudo-verses and footnote version)
	bool m_bBracketItalics;
	bool m_bNoArabicNumeralWords;		// Skip "words" made entirely of Arabic numerals and don't count them as words
	bool m_bInlineFootnotes;			// True if inlining footnotes as uncounted parentheticals
	bool m_bUseBracketFootnotes;		// True if treating "[" and "]" as inline footnote markers
	bool m_bUseBracketFootnotesExcluded;// True if treating "[" and "]" as inline footnote markers and excluding them (m_bUseBracketFootnotes will also be true)
	bool m_bExcludeDeuterocanonical;	// Exclude Apocrypha/Deuterocanonical Text
	QString m_strSegVariant;			// OSIS <seg> tag variant to export (or empty to export all)
	QString m_strCurrentSegVariant;		// Current OSIS <seg> tag variant we are in (or empty if not in a seg)
	bool m_bFoundSegVariant;			// Set to true if any <seg> tag variant found when no SegVariant was specifed.  Otherwise, set to true when the specified Seg Variant was found.
	QString m_strStrongsImpFilepath;	// Strongs Imp Database to parse (if empty, no Strongs Database will be used)
};

static unsigned int bookIndexToTestamentIndex(unsigned int nBk)
{
	unsigned int nTst = 0;
	if (nBk == 0) return 0;			// Special-case for "no book"

	// note: Shift nBk to be an zero based since book indexes are normally one-based:
	--nBk;
	if (nBk < NUM_BK_OT) {
		nTst = 1;
	} else if (nBk < (NUM_BK_OT + NUM_BK_NT)) {
		nTst = 2;
	} else if (nBk < (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC)) {
		nTst = 3;
	} else {
		nTst = 0;
		assert(false);			// Can't happen if our NUM_BK_xx values are correct!
	}

	return nTst;
}

static unsigned int bookIndexToTestamentBookIndex(unsigned int nBk)
{
	// Note: nBk is one-based
	if (nBk <= NUM_BK_OT) {
		return nBk;
	} else if (nBk <= (NUM_BK_OT + NUM_BK_NT)) {
		return nBk-NUM_BK_OT;
	} else if (nBk <= (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC)) {
		return nBk-NUM_BK_OT-NUM_BK_NT;
	} else {
		return nBk-NUM_BK_OT-NUM_BK_NT-NUM_BK_APOC;
	}
}

static bool bookIsDeuterocanonical(unsigned int nBk)
{
	// Note: nBk is one-based
	if (nBk > (NUM_BK_OT + NUM_BK_NT)) return true;
	return false;
}

const CBookEntry *COSISXmlHandler::addBookToBibleDatabase(unsigned int nBk)
{
	unsigned int nTst = bookIndexToTestamentIndex(nBk);

	// Make nBk 0-based to simplify:
	--nBk;

	m_pBibleDatabase->m_EntireBible.m_nNumBk++;
	m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumBk++;
	m_pBibleDatabase->m_lstBooks.resize(qMax(static_cast<unsigned int>(nBk+1), static_cast<unsigned int>(m_pBibleDatabase->m_lstBooks.size())));
	m_pBibleDatabase->m_lstBooks[nBk].m_nTstBkNdx = bookIndexToTestamentBookIndex(nBk+1);
	m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx = nTst;
	m_pBibleDatabase->m_lstBooks[nBk].m_strBkName = g_arrBooks[nBk].m_strName;
	m_pBibleDatabase->m_lstBooks[nBk].m_lstBkAbbr.append(g_arrBooks[nBk].m_strOsisAbbr);
	m_pBibleDatabase->m_lstBooks[nBk].m_lstBkAbbr.append(g_arrBooks[nBk].m_strCommonAbbr.split(QChar(';'), QString::SkipEmptyParts));
	m_pBibleDatabase->m_lstBooks[nBk].m_strTblName = g_arrBooks[nBk].m_strTableName;

	TBookCategoryList::iterator itrCat = m_pBibleDatabase->m_lstBookCategories.begin();
	while (itrCat != m_pBibleDatabase->m_lstBookCategories.end()) {
		if (itrCat->m_strCategoryName.compare(g_arrBooks[nBk].m_strCategory) == 0) break;
		++itrCat;
	}
	if (itrCat == m_pBibleDatabase->m_lstBookCategories.end()) {
		m_pBibleDatabase->m_lstBookCategories.push_back(CBookCategoryEntry(g_arrBooks[nBk].m_strCategory));
		itrCat = m_pBibleDatabase->m_lstBookCategories.end() - 1;
	}
	itrCat->m_setBooksNum.insert(nBk+1);
	m_pBibleDatabase->m_lstBooks[nBk].m_nCatNdx = std::distance(m_pBibleDatabase->m_lstBookCategories.begin(), itrCat) + 1;

	m_pBibleDatabase->m_lstBooks[nBk].m_strDesc = g_arrBooks[nBk].m_strDescription;
	m_pBibleDatabase->m_lstBookVerses.resize(qMax(static_cast<unsigned int>(nBk+1), static_cast<unsigned int>(m_pBibleDatabase->m_lstBookVerses.size())));

	return &m_pBibleDatabase->m_lstBooks[nBk];
}

bool COSISXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

/*

	OSIS File Format Header:
	------------------------

	{osis}[schemaLocation=http://www.bibletechnologies.net/2003/OSIS/namespace http://www.bibletechnologies.net/osisCore.2.1.1.xsd]

	{osisText}[osisIDWork=KJV,osisRefWork=defaultReferenceScheme,lang=en]

		{header}[]
			{work}[osisWork=KJV]
				{title}[]King James Version (1769) with Strongs Numbers and Morphology{/title}

				{identifier}[type=OSIS]Bible.KJV{/identifier}

				{refSystem}[]Bible.KJV{/refSystem}

			{/work}

			{work}[osisWork=defaultReferenceScheme]
				{refSystem}[]Bible.KJV{/refSystem}

			{/work}

		{/header}



	Zefania XML Format Header:
	--------------------------

	<XMLBIBLE xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="zef2005.xsd" version="2.0.1.18" revision="2" status="v" biblename="Luther 1545 mit Strongs" type="x-bible">
	  <INFORMATION>
		<title>Luther 1545 mit Strongs</title>
		<creator></creator>
		<subject>Heilige Schrift</subject>
		<description>Dieser Bibeltext ist eine eingedeutschte Version der letzten von Luther
			 1545 in Druck gegebenen Übersetzung der Bibel. Ursprünglich erschien sie als:
			"Martin Luther: Biblia: das ist: Die gantze Heilige Schrifft. Deudsch Auffs new zugericht.
			Wittenberg: Hans Lufft 1545 (Ausgabe letzter Hand)".
		</description>
		<publisher>FREE BIBLE SOFTWARE GROUP</publisher>
		<contributors>Michael Bolsinger (Michael.Bolsinger@t-online.de)</contributors>
		<date>2009-01-20</date>
		<type>Bible</type>
		<format>Zefania XML Bible Markup Language</format>
		<identifier>luth1545str</identifier>
		<source>Text: http://www.luther-bibel-1545.de/
					 Strongs: http://www.winbibel.de/de/lutherstrong/lu1545adds.htm</source>
		<language>GER</language>
		<coverage>provide the Bible to the nations of the world</coverage>
		<rights>We believe that this Bible is found in the Public Domain.</rights>
	  </INFORMATION>

*/

	int ndx = -1;
	unsigned int nTst = bookIndexToTestamentIndex(m_ndxCurrent.book());
	int nBk = -1;

	if (localName.compare("osis", Qt::CaseInsensitive) == 0) {
		if (m_xfteFormatType != XFTE_UNKNOWN) {
			m_strErrorString = "*** Error: Ambiguous XML File Type!  Check Source!\n\n";
			return false;
		}
		m_strNamespace = "http://www.bibletechnologies.net/2003/OSIS/namespace";		// TODO : Verify Namespace?
		m_xfteFormatType = XFTE_OSIS;
		std::cerr << "XMLType: OSIS\n";
	} else if (localName.compare("XMLBIBLE", Qt::CaseInsensitive) == 0) {
		if (m_xfteFormatType != XFTE_UNKNOWN) {
			m_strErrorString = "*** Error: Ambiguous XML File Type!  Check Source!\n\n";
			return false;
		}
		m_strNamespace = "zef2005.xsd";		// TODO : Verify Namespace?
		m_xfteFormatType = XFTE_ZEFANIA;
		std::cerr << "XMLType: ZEFANIA\n";
	} else if ((m_xfteFormatType == XFTE_OSIS) && (localName.compare("osisText", Qt::CaseInsensitive) == 0))  {
		ndx = findAttribute(atts, "osisIDWork");
		if (ndx != -1) m_pBibleDatabase->m_descriptor.m_strWorkID = atts.value(ndx);
		std::cerr << "Work: " << atts.value(ndx).toUtf8().data() << "\n";
		ndx = findAttribute(atts, "lang");
		if  (ndx != -1) {
			m_strLanguage = atts.value(ndx);
			std::cerr << "Language: " << m_strLanguage.toUtf8().data();
			if (CTranslatorList::instance()->setApplicationLanguage(m_strLanguage)) {
				g_setBooks();
				g_setTstNames();
				std::cerr << " (Loaded Translations)\n";
			} else {
				std::cerr << " (NO Translations Found!)\n";
			}
		}
	} else if (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("header", Qt::CaseInsensitive) == 0)) ||
			   ((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("INFORMATION", Qt::CaseInsensitive) == 0))) {
		m_bInHeader = true;
	} else if ((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("language", Qt::CaseInsensitive) == 0) && (m_bInHeader)) {
		m_bCaptureLang = true;
		m_strLanguage.clear();
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		if (!m_ndxCurrent.isSet()) {
			if (m_bInHeader) m_bCaptureTitle = true;
		} else if (m_xfteFormatType == XFTE_OSIS) {
			// Older format (embedded in closed-form verse tag): canonical="true" subType="x-preverse" type="section":
			//		<chapter osisID="Ps.3">
			//		<verse osisID="Ps.3.1"><title canonical="true" subType="x-preverse" type="section">A Psalm of David, when he fled from Absalom his son.</title>
			//
			// Newer format (using OpenEnded markers, but preceding the verse-tags):
			//		<chapter osisID="Job.42" chapterTitle="CHAPTER 42.">
			//		<title type="chapter">CHAPTER 42.</title>
			//
			// Newer format (using OpenEnded markers, but inside the verse-tags):
			//		<chapter osisID="Ps.7">
			//		<verse osisID="Ps.7.1">
			//		<div type="x-milestone" subType="x-preverse" sID="pv5"/>
			//		<title canonical="true" type="psalm">Shiggaion of David, which he sang unto the <divineName>Lord</divineName>, concerning the words of Cush the Benjamite.</title>
			//		<div type="x-milestone" subType="x-preverse" eID="pv5"/>
			//		O <divineName>Lord</divineName> my God, in thee do I put my trust: save me from all them that persecute me, and deliver me:
			//		<note type="study">words: or, business</note>
			//		</verse>
			//
			//		<chapter osisID="Ps.3" chapterTitle="PSALM 3.">
			//		<title type="chapter">PSALM 3.</title>
			//		<title type="psalm" canonical="true">A Psalm of David, when he fled from Absalom his son.</title>
			//
			//		<verse osisID="Ps.119.1" sID="Ps.119.1"/><title type="acrostic" canonical="true"><foreign n="א">ALEPH.</foreign></title>
			ndx = findAttribute(atts, "type");
			if ((ndx != -1) &&
				((atts.value(ndx).compare("section", Qt::CaseInsensitive) == 0) ||
				 (atts.value(ndx).compare("psalm", Qt::CaseInsensitive) == 0))) {
				m_ndxSuperscription = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0);		// Superscriptions are for the chapter, not the first verse in it, even though that's were this tag exists (in the old closed-form format)
				startVerseEntry(m_ndxSuperscription, false);
			} else if ((ndx != -1) &&
					   ((atts.value(ndx).compare("chapter", Qt::CaseInsensitive) == 0) ||
						(atts.value(ndx).compare("acrostic", Qt::CaseInsensitive) == 0))) {
				// Ignore Chapter titles (as it just has things like "Chapter 1", etc), and is somewhat useless...
				// Ignore verse acrostics on new-format OSIS files (old formats get ignored via foreign language tags below)
			} else {
				std::cerr << QString("\n*** Encountered unknown Title tag inside chapter and/or verse body : %1\n").arg(m_ndxCurrent.index()).toUtf8().data();
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && (localName.compare("foreign", Qt::CaseInsensitive) == 0)) {
		m_bInForeignText = true;				// Old format way of handling acrostics
	} else if ((m_xfteFormatType == XFTE_OSIS) &&
			   (((localName.compare("div", Qt::CaseInsensitive) == 0) && ((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("colophon", Qt::CaseInsensitive) == 0)) ||
				(localName.compare("closer", Qt::CaseInsensitive) == 0))) {
		// Note: This must come here as colophon's may (old form) or may not (new form) have m_ndxCurrent set depending on placement relative to books, chapters, and verses:
		ndx = findAttribute(atts, "osisID");
		if (ndx != -1) {
			QStringList lstOsisID = atts.value(ndx).split('.');
			if ((lstOsisID.size() < 1) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
				std::cerr << "\n*** Unknown Colophon osisID : " << atts.value(ndx).toUtf8().data() << "\n";
				m_ndxColophon = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);
			} else {
				if (!m_bExcludeDeuterocanonical || !bookIsDeuterocanonical(nBk+1)) {
					bool bOK = true;
					unsigned int nChp = 0;
					unsigned int nVrs = 0;
					m_ndxColophon = CRelIndex(nBk+1, 0, 0, 0);
					if ((lstOsisID.size() >= 2) && ((nChp = lstOsisID.at(1).toUInt(&bOK)) != 0) && (bOK)) {
						m_ndxColophon.setChapter(nChp);
						if ((lstOsisID.size() >= 3) && ((nVrs = lstOsisID.at(2).toUInt(&bOK)) != 0) && (bOK)) {
							m_ndxColophon.setVerse(nVrs);
						}
					}
				} else {
					nBk = -1;
				}
			}
		} else {
			m_ndxColophon = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);
		}
		if (findAttribute(atts, "sID") != -1) {
			// Start of open-ended colophon:
			if (m_bInColophon && !m_bUseBracketColophons) {
				std::cerr << "\n*** Start of open-ended colophon before end of colophon : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			startVerseEntry(m_ndxColophon, true);
		} else if (findAttribute(atts, "eID") != -1) {
			// End of open-ended colophon:
			if (!m_bInColophon && !m_bUseBracketColophons) {
				std::cerr << "\n*** End of open-ended colophon before start of colophon : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			} else {
				// We can have nested Words of Jesus with open form:
				if (m_bInWordsOfJesus) {
					CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxColophon.book()-1])[CRelIndex(m_ndxColophon.book(), m_ndxColophon.chapter(), m_ndxColophon.verse(), 0)];
					verse.m_strText += g_chrParseTag;
					verse.m_lstParseStack.push_back("j:");
				}
				endVerseEntry(m_ndxColophon);
			}
		} else {
			// Standard Closed-Form Colophon:
			if (m_bOpenEndedColophon) {
				std::cerr << "\n*** Mixing open-ended and closed form colophons : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			startVerseEntry(m_ndxColophon, false);
		}
	} else if ((!m_ndxCurrent.isSet()) &&
			   (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("div", Qt::CaseInsensitive) == 0)) ||
				((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("BIBLEBOOK", Qt::CaseInsensitive) == 0)))) {
		nBk = -1;
		if (m_xfteFormatType == XFTE_OSIS) {
			ndx = findAttribute(atts, "type");
			if ((ndx != -1) && (atts.value(ndx).compare("x-testament", Qt::CaseInsensitive) == 0)) {
//				std::cerr << "Testament Tag\n";
			} else if ((ndx != -1) && (atts.value(ndx).compare("book", Qt::CaseInsensitive) == 0)) {
				// Some OSIS files just have book tags and no x-testament tags, so we'll try to infer
				//		testament here:
				ndx = findAttribute(atts, "osisID");
				if (ndx != -1) {
					QStringList lstOsisID = atts.value(ndx).split('.');
					if ((lstOsisID.size() != 1) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
						std::cerr << "\n*** Invalid Book osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} // else fall-through and create book with nBk != -1 ...
				}
			}
		} else if (m_xfteFormatType == XFTE_ZEFANIA) {
			ndx = findAttribute(atts, "bnumber");
			if (ndx != -1) {
				nBk = atts.value(ndx).toInt() - 1;			// Note: nBk is index into array, not book number
				if ((nBk < 0) || (nBk >= m_lstOsisBookList.size())) {
					std::cerr << "\n**** Invalid Book Index: " << atts.value(ndx).toUtf8().data() << "\n";
					nBk = -1;
				} // else fall-through and create book with nBk != -1 ...
			} else {
				std::cerr << "\n*** Warning: Found BIBLEBOOK tag without a bnumber\n";
			}
		}
		if (nBk != -1) {
			if (bookIsDeuterocanonical(nBk+1) && m_bExcludeDeuterocanonical) {
				if ((localName.compare("div", Qt::CaseInsensitive) != 0) ||
					((localName.compare("div", Qt::CaseInsensitive) == 0) &&
					 (findAttribute(atts, "eID") == -1))) {
					// Don't log this if this is a <div> tag with "eID" set, as
					//	we will have already written it for "sID":
					std::cerr << "Book: " << m_lstOsisBookList.at(nBk).toUtf8().data();
					std::cerr << "  >>> Skipping Deuterocanonical Book\n";
				}
				nBk = -1;
			} else {
				std::cerr << "Book: " << m_lstOsisBookList.at(nBk).toUtf8().data() << "\n";
				// note: nBk is index into array, not book number:
				nTst = bookIndexToTestamentIndex(nBk+1);
				while (m_pBibleDatabase->m_lstTestaments.size() < nTst) {
					CTestamentEntry aTestament(g_arrstrTstNames[m_pBibleDatabase->m_lstTestaments.size()]);
					m_pBibleDatabase->m_EntireBible.m_nNumTst++;
					m_pBibleDatabase->m_lstTestaments.push_back(aTestament);
					std::cerr << "Adding Testament: " << aTestament.m_strTstName.toUtf8().data() << "\n";
				}
				if (m_xfteFormatType == XFTE_ZEFANIA) m_ndxCurrent.setBook(nBk+1);
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && (m_ndxCurrent.isSet()) && (localName.compare("div", Qt::CaseInsensitive) == 0) && ((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("paragraph", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "sID");			// Paragraph Starts are tagged with sID, Paragraph Ends are tagged with eID -- we only care about the starts for our Pilcrows -- example text: Reina-Valera 1909
		if (ndx != -1) {
			if (m_ndxCurrent.verse() == 0) {
				std::cerr << "\n*** Pilcrow marker outside of verse at: " << m_pBibleDatabase->PassageReferenceText(m_ndxCurrent).toUtf8().data() << "\n";
			} else {
				CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
				verse.m_nPilcrow = CVerseEntry::PTE_MARKER;
			}
		}
	} else if ((localName.compare("chapter", Qt::CaseInsensitive) == 0) &&
			   ((m_xfteFormatType == XFTE_OSIS) ||
				((m_ndxCurrent.isSet()) && (m_ndxCurrent.chapter() == 0) && (m_xfteFormatType == XFTE_ZEFANIA)))) {
		// Note: Coming into this function, either ndxCurrent isn't set and we set both book and chapter (OSIS) or book only is set and we set chapter (ZEFANIA)
		if ((m_xfteFormatType == XFTE_OSIS) && ((ndx = findAttribute(atts, "eID")) != -1)) {
			// End of open-ended chapter:
			if ((m_ndxCurrent.book() != 0) && (m_ndxCurrent.chapter() == 0)) {
				std::cerr << "\n*** End of open-ended chapter before start of chapter : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			m_bOpenEndedChapter = false;
			// At the end of closed-form, leave the chapter number set (indicating bInChapter) and process closing in the endElement for this end-tag.  But don't start new chapter here
		} else {
			if (m_xfteFormatType == XFTE_OSIS) {
				ndx = findAttribute(atts, "osisID");
				if (ndx != -1) {
					QStringList lstOsisID = atts.value(ndx).split('.');
					if ((lstOsisID.size() != 2) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
						m_ndxCurrent = CRelIndex();
						std::cerr << "\n*** Unknown Chapter osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} else {
						if (!m_bExcludeDeuterocanonical || !bookIsDeuterocanonical(nBk+1)) {
							if (findAttribute(atts, "sID") != -1) {
								// Start of open-ended chapter:
								if (m_ndxCurrent.chapter()) {
									std::cerr << "\n*** Start of open-ended chapter before end of chapter : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
								}
								m_bOpenEndedChapter = true;
							} else {
								// Standard Closed-Form chapter:
								if (m_bOpenEndedChapter) {
									std::cerr << "\n*** Mixing open-ended and closed form chapter : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
								}
								m_bOpenEndedChapter = false;
							}
							m_ndxCurrent = CRelIndex(nBk+1, lstOsisID.at(1).toUInt(), 0, 0);
						} else {
							nBk = -1;
						}
					}
				} else {
					m_ndxCurrent = CRelIndex();
					std::cerr << "\n*** Chapter with no osisID : ";
					std::cerr << stringifyAttributes(atts).toUtf8().data() << "\n";
				}
			} else if (m_xfteFormatType == XFTE_ZEFANIA) {
				assert((m_ndxCurrent.verse() == 0) && (m_ndxCurrent.word() == 0));
				ndx = findAttribute(atts, "cnumber");
				if (ndx != -1) {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), atts.value(ndx).toUInt(), 0, 0);
					if (m_ndxCurrent.chapter() == 0) {
						std::cerr << QString("\n*** Invalid Chapter Number: \"%1\"\n").arg(atts.value(ndx)).toUtf8().data();
					}
				} else {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);		// Leave the book set for other chapters...
					std::cerr << "\n*** Warning: Found chapter tag without a cnumber\n";
				}
			}

			if (m_ndxCurrent.chapter() != 0) {
				nBk = m_ndxCurrent.book() - 1;		// Let nBk be our array index for our current book
				std::cerr << "Book: " << m_lstOsisBookList.at(nBk).toUtf8().data() << " Chapter: " << QString("%1").arg(m_ndxCurrent.chapter()).toUtf8().data();
				nTst = bookIndexToTestamentIndex(nBk+1);
				m_pBibleDatabase->m_mapChapters[m_ndxCurrent];			// Make sure the chapter entry is created, even though we have nothing to put in it yet
				if (m_ndxCurrent.chapter() == g_arrBooks[nBk].m_ndxStartingChapterVerse.chapter()) {
					addBookToBibleDatabase(nBk+1);
				}
				assert(m_pBibleDatabase->m_lstBooks.size() > static_cast<unsigned int>(nBk));
				if (m_ndxCurrent.chapter() == g_arrBooks[nBk].m_ndxStartingChapterVerse.chapter()) {
					for (CRelIndex ndxAddChp = CRelIndex(nBk+1, 1, 0, 0); ndxAddChp != m_ndxCurrent; ndxAddChp.setChapter(ndxAddChp.chapter()+1)) {
						m_pBibleDatabase->m_mapChapters[ndxAddChp];			// Make sure the chapter entry is created for the empty chapters
					}
					m_pBibleDatabase->m_EntireBible.m_nNumChp += g_arrBooks[nBk].m_ndxStartingChapterVerse.chapter();
					m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumChp += g_arrBooks[nBk].m_ndxStartingChapterVerse.chapter();
					m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp += g_arrBooks[nBk].m_ndxStartingChapterVerse.chapter();
				} else {
					m_pBibleDatabase->m_EntireBible.m_nNumChp++;
					m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumChp++;
					m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp++;
				}
			}
		}
	} else if ((m_ndxCurrent.isSet()) &&
			   (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("verse", Qt::CaseInsensitive) == 0)) ||
				((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("vers", Qt::CaseInsensitive) == 0)))) {
		if ((m_xfteFormatType == XFTE_OSIS) && ((ndx = findAttribute(atts, "eID")) != -1)) {
			// End of open-ended verse:
			if (!m_bInVerse) {
				std::cerr << "\n*** End of open-ended verse before start of verse : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			m_bOpenEndedVerse = false;
			// We can have nested Words of Jesus with open form:
			if ((m_bInWordsOfJesus) && (m_bInVerse)) {
				CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
				verse.m_strText += g_chrParseTag;
				verse.m_lstParseStack.push_back("j:");
			}
			// At the end of closed-form, leave m_bInVerse set and process closing in the endElement for this end-tag.  But don't start new verse here
		} else {
			// We need a separate flag here to denote new verse and can't just use whether or not
			//		the verse number is set like we could with the others.  This is because this
			//		same code also processes the split sID/eID tags for open-ended verse logic:
			bool bFoundNewVerse = false;
			bool bOpenEnded = false;
			if (m_xfteFormatType == XFTE_OSIS) {
				ndx = findAttribute(atts, "osisID");
				if (ndx != -1) {
					QStringList lstOsisID = atts.value(ndx).split('.');
					if ((lstOsisID.size() != 3) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
						std::cerr << "\n*** Unknown Verse osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} else if ((m_ndxCurrent.book() != static_cast<unsigned int>(nBk+1)) || (m_ndxCurrent.chapter() != lstOsisID.at(1).toUInt())) {
						m_ndxCurrent.setVerse(0);
						m_ndxCurrent.setWord(0);
						std::cerr << "\n*** Verse osisID doesn't match Chapter osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} else {
						if (findAttribute(atts, "sID") != -1) {
							// Start of open-ended verse:
							if (m_bInVerse) {
								std::cerr << "\n*** Start of open-ended verse before end of verse : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
							}
							bOpenEnded = true;
						} else {
							// Standard Closed-Form verse:
							if (m_bOpenEndedVerse) {
								std::cerr << "\n*** Mixing open-ended and closed form verses : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
							}
							m_bOpenEndedVerse = false;
						}

						m_ndxCurrent.setVerse(lstOsisID.at(2).toUInt());
						m_ndxCurrent.setWord(0);
						bFoundNewVerse = true;
					}
				}
			} else if (m_xfteFormatType == XFTE_ZEFANIA) {
				ndx = findAttribute(atts, "vnumber");
				if (ndx != -1) {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), atts.value(ndx).toUInt(), 0);
					if (m_ndxCurrent.verse() == 0) {
						std::cerr << QString("\n*** Invalid Verse Number: \"%1\"\n").arg(atts.value(ndx)).toUtf8().data();
					} else {
						bFoundNewVerse = true;
					}
				} else {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0);		// Leave the book/chapter set for other verses...
					std::cerr << "\n*** Warning: Found verse tag without a vnumber\n";
				}
			}

			if (bFoundNewVerse) {
				if ((m_ndxCurrent.verse() % 5) == 0) {
					std::cerr << QString("%1").arg(m_ndxCurrent.verse() / 5).toUtf8().data();
				} else {
					std::cerr << ".";
				}

				bool bPreExisted = ((m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1]).find(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0))
										!= (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1]).end());

				if (CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0) == g_arrBooks[m_ndxCurrent.book()-1].m_ndxStartingChapterVerse) {
					for (CRelIndex ndxAddVrs = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 0); ndxAddVrs != m_ndxCurrent; ndxAddVrs.setVerse(ndxAddVrs.verse()+1)) {
						// Create all intentionally missing verses:
						(m_pBibleDatabase->m_lstBookVerses[ndxAddVrs.book()-1])[CRelIndex(ndxAddVrs.book(), ndxAddVrs.chapter(), ndxAddVrs.verse(), 0)];
					}
				}

				if (bPreExisted) {
					std::cerr << QString("\n*** Warning: Duplicate Verse Entry: \"%1\"\n").arg(m_pBibleDatabase->PassageReferenceText(m_ndxCurrent)).toUtf8().data();
				}

				startVerseEntry(m_ndxCurrent, bOpenEnded);
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && (m_bInVerse) && (localName.compare("note", Qt::CaseInsensitive) == 0)) {
		m_bInNotes = true;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("N:");
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("milestone", Qt::CaseInsensitive) == 0)) {
		//	Note: If we already have text on this verse, then set a flag to put the pilcrow on the next verse
		//			so we can handle the strange <CM> markers used on the German Schlachter text
		//
		//	PTE_MARKER			Example: {verse}[osisID=Gen.5.21]{milestone}[marker=¶,type=x-p]{/milestone}
		//	PTE_MARKER_ADDED	Example: {verse}[osisID=Gen.5.3]{milestone}[marker=¶,subType=x-added,type=x-p]{/milestone}
		//	PTE_EXTRA			Example: {verse}[osisID=Gen.5.6]{milestone}[type=x-extra-p]{/milestone}

		CVerseEntry::PILCROW_TYPE_ENUM nPilcrow = CVerseEntry::PTE_NONE;

		if (((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("x-p", Qt::CaseInsensitive) == 0)) {
			if (((ndx = findAttribute(atts, "subType")) != -1) && (atts.value(ndx).compare("x-added", Qt::CaseInsensitive) == 0)) {
				nPilcrow = CVerseEntry::PTE_MARKER_ADDED;
			} else{
				nPilcrow = CVerseEntry::PTE_MARKER;
			}
		} else if ((m_xfteFormatType == XFTE_OSIS) && ((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("x-extra-p", Qt::CaseInsensitive) == 0)) {
			nPilcrow = CVerseEntry::PTE_EXTRA;
		}

		CVerseEntry &verse = activeVerseEntry();
		QString strTempText = verse.m_strText;
		strTempText.remove(g_chrParseTag);		// To check for text for delayed pilcrow, remove any parseTag markers as we may have encountered some text modifier tags, but not the actual text yet
		if (strTempText.isEmpty()) {
			verse.m_nPilcrow = nPilcrow;
		} else {
			m_nDelayedPilcrow = nPilcrow;
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("seg", Qt::CaseInsensitive) == 0)) {
		// <seg subType="x-1" type="x-variant">
		ndx = findAttribute(atts, "type");		// TODO : In addition to 'x-variant', add support for full OSIS 'variant', which is currently a work-in-progress
		if ((ndx != -1) && (atts.value(ndx).compare("x-variant", Qt::CaseInsensitive) == 0)) {
			ndx = findAttribute(atts, "subType");
			if (ndx != -1) {
				m_strCurrentSegVariant = atts.value(ndx);
				if ((m_strSegVariant.isEmpty()) || (m_strCurrentSegVariant.compare(m_strSegVariant, Qt::CaseInsensitive) == 0)) {
					m_bFoundSegVariant = true;
				}
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("w", Qt::CaseInsensitive) == 0)) {
		m_bInLemma = true;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("L:" + stringifyAttributes(atts));
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) &&	// Note: Allow transChangeAdded inside of inline bracketed notes
			   ((localName.compare("transChange", Qt::CaseInsensitive) == 0) ||
				(localName.compare("hi", Qt::CaseInsensitive) == 0))) {
		ndx = findAttribute(atts, "type");
		if ((ndx != -1) &&
			(((localName.compare("transChange", Qt::CaseInsensitive) == 0) &&
			  (atts.value(ndx).compare("added", Qt::CaseInsensitive) == 0)) ||		// <transChange type="added">
			 ((localName.compare("hi", Qt::CaseInsensitive) == 0) &&
			  (atts.value(ndx).compare("italic", Qt::CaseInsensitive) == 0)))) {	// <hi type="italic">
			m_bInTransChangeAdded = true;
			CVerseEntry &verse = activeVerseEntry();
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("T:");
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("q", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "who");
		if ((ndx != -1) && (atts.value(ndx).compare("Jesus", Qt::CaseInsensitive) == 0)) {
			m_bInWordsOfJesus = true;
			CVerseEntry &verse = activeVerseEntry();
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("J:");
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("divineName", Qt::CaseInsensitive) == 0)) {
		m_bInDivineName = true;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("D:");
	}

	// Note: In the m_lstParseStack, we'll push values on as follows:
	//			L:<attrs>		-- Lemma Start
	//			l:				-- Lemma End
	//			T:				-- TransChange Added Start
	//			t:				-- TransChange Added End
	//			J:				-- Words of Jesus Start
	//			j:				-- Words of Jesus End
	//			D:				-- Divine Name Start
	//			d:				-- Divine Name End
	//			R:				-- Search Result Start (reserved placeholder, see VerseRichifier)
	//			r:				-- Search Result End (reserved placeholder, see VerseRichifier)
	//			M:				-- Hebrew Psalm 119 Marker
	//			N:				-- Inline Parenthetical Note Start (inline footnote, not counted as verse text proper)
	//			n:				-- Inline Parenthetical Note End



/*
	m_lstElementNames.append(localName);
	std::cout << "{" << localName.toUtf8().data() << "}";
	std::cout << "[";
	for (int i = 0; i < atts.count(); ++i) {
		if (i) std::cout << ",";
		std::cout << atts.localName(i).toUtf8().data() << "=" << atts.value(i).toUtf8().data();
//		if (atts.localName(i).compare("type", Qt::CaseInsensitive) == 0) {
			m_lstAttrNames.append(atts.localName(i) + "=" + atts.value(i));
//		}
	}
	std::cout << "]";
*/



	return true;
}

bool COSISXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);


	if (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("header", Qt::CaseInsensitive) == 0)) ||
		((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("INFORMATION", Qt::CaseInsensitive) == 0))) {
		m_bInHeader = false;
	} else if ((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("language", Qt::CaseInsensitive) == 0) && (m_bInHeader)) {
		// Convert Language:
		if (m_strLanguage.compare("GER", Qt::CaseInsensitive) == 0) m_strLanguage = "de";
		std::cerr << "Language: " << m_strLanguage.toUtf8().data();
		if (CTranslatorList::instance()->setApplicationLanguage(m_strLanguage)) {
			g_setBooks();
			g_setTstNames();
			std::cerr << " (Loaded Translations)\n";
		} else {
			std::cerr << " (NO Translations Found!)\n";
		}
		m_bCaptureLang = false;
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		if (m_bInHeader) {
			std::cerr << "Title: " << m_pBibleDatabase->m_descriptor.m_strDBDesc.toUtf8().data() << "\n";
		}
		m_bCaptureTitle = false;
		if ((m_bInSuperscription) && (!m_bOpenEndedSuperscription)) {
			endVerseEntry(m_ndxSuperscription);
		}
	} else if (localName.compare("foreign", Qt::CaseInsensitive) == 0) {
		m_bInForeignText = false;
	} else if ((m_bInColophon) && (!m_bOpenEndedColophon) && (!m_bUseBracketColophons) && (localName.compare("div", Qt::CaseInsensitive) == 0)) {
		endVerseEntry(m_ndxColophon);
	} else if ((m_xfteFormatType == XFTE_ZEFANIA) && (!m_bInVerse) && (localName.compare("BIBLEBOOK", Qt::CaseInsensitive) == 0)) {
		m_ndxCurrent = CRelIndex();
	} else if ((!m_bInVerse) && (localName.compare("chapter", Qt::CaseInsensitive) == 0) && (!m_bOpenEndedChapter)) {
		if (m_ndxCurrent.book() != 0) {
			std::cerr << "\n";
		}
		if (m_bInBracketNotes) std::cerr << "\n*** Error: Missing end of Bracketed Footnotes within chapter\n";
		m_bInBracketNotes = false;
		if (m_xfteFormatType == XFTE_OSIS) {
			m_ndxCurrent = CRelIndex();
		} else if (m_xfteFormatType == XFTE_ZEFANIA) {
			m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);
		}
// Technically, we shouldn't have a chapter inside verse, but some modules use it as a special inner marking (like FrePGR, for example):
//		assert(m_bInVerse == false);
//		if (m_bInVerse) {
//			std::cerr << "\n*** End-of-Chapter found before End-of-Verse\n";
//			m_bInVerse = false;
//		}
	} else if ((m_bInVerse) && (!m_bOpenEndedVerse) &&
			   (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("verse", Qt::CaseInsensitive) == 0)) ||
				((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("vers", Qt::CaseInsensitive) == 0)))) {
		endVerseEntry(m_ndxCurrent);
	} else if ((m_bInNotes) && (localName.compare("note", Qt::CaseInsensitive) == 0)) {
		m_bInNotes = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("n:");
	} else if (localName.compare("seg", Qt::CaseInsensitive) == 0) {
		m_strCurrentSegVariant.clear();
	} else if ((m_bInLemma) && (localName.compare("w", Qt::CaseInsensitive) == 0)) {
		m_bInLemma = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("l:");
	} else if ((m_bInTransChangeAdded) &&
			   ((localName.compare("transChange", Qt::CaseInsensitive) == 0) ||
				(localName.compare("hi", Qt::CaseInsensitive) == 0))) {
		m_bInTransChangeAdded = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("t:");
	} else if ((m_bInWordsOfJesus) && (localName.compare("q", Qt::CaseInsensitive) == 0)) {
		m_bInWordsOfJesus = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("j:");
	} else if ((m_bInDivineName) && (localName.compare("divineName", Qt::CaseInsensitive) == 0)) {
		m_bInDivineName = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("d:");
	}



//	std::cout << "{/" << localName.toUtf8().data() << "}\n";

	return true;
}

bool COSISXmlHandler::characters(const QString &ch)
{
	QString strTemp = ch;
// TODO : REMOVE
//	strTemp.replace('\n', ' ');

	if (m_bCaptureTitle) {
		m_pBibleDatabase->m_descriptor.m_strDBDesc += strTemp;
	} else if (m_bCaptureLang) {
		m_strLanguage += strTemp;
	} else if ((!m_strCurrentSegVariant.isEmpty()) && (!m_strSegVariant.isEmpty()) && (m_strCurrentSegVariant.compare(m_strSegVariant, Qt::CaseInsensitive) != 0)) {
		// Eat the characters if we are in a <seg> variant other than the one specified to capture,
		//		and we were actually given a <seg> variant to capture...
	} else if (m_bInColophon && !m_bUseBracketColophons) {
		if (!m_bDisableColophons) {
			assert(m_ndxColophon.isSet());
			if (m_ndxColophon.isSet()) {
				// TODO : Eventually remove the "footnote" version of colophon?
				CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[m_ndxColophon];
				footnote.setText(footnote.text() + strTemp);
			}
			charactersVerseEntry(m_ndxColophon, strTemp);
		}
	} else if ((m_bInSuperscription) && (!m_bInForeignText)) {
		if (!m_bDisableSuperscriptions) {
			assert(m_ndxSuperscription.isSet());
			if (m_ndxSuperscription.isSet()) {
				// TODO : Eventually remove the "footnote" version of superscription?
				CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[m_ndxSuperscription];
				footnote.setText(footnote.text() + strTemp);
			}
			charactersVerseEntry(m_ndxSuperscription, strTemp);
		}
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInForeignText)) {
		assert((m_ndxCurrent.book() != 0) && (m_ndxCurrent.chapter() != 0) && (m_ndxCurrent.verse() != 0));
		charactersVerseEntry(m_ndxCurrent, strTemp);
//		std::cout << strTemp.toUtf8().data();
	} else if (((m_bInVerse) || (m_bInColophon && !m_bDisableColophons) || (m_bInSuperscription && !m_bDisableSuperscriptions)) && (m_bInNotes)) {
		if (m_bInVerse || (m_bInColophon && !m_bNoColophonVerses) || (m_bInSuperscription && !m_bNoSuperscriptionVerses)) {
			assert((m_ndxCurrent.book() != 0) && (m_ndxCurrent.chapter() != 0) && (m_ndxCurrent.verse() != 0));
			charactersVerseEntry(m_ndxCurrent, strTemp);
		}
	}



//	std::cout << ch.toUtf8().data();

	return true;
}

bool COSISXmlHandler::error(const QXmlParseException &exception)
{
	std::cerr << QString("\n\n*** %1\n").arg(exception.message()).toUtf8().data();
	return true;
}

bool COSISXmlHandler::endDocument()
{
	// Nothing else needs to be done here for the main database XML,
	//	but now that that's complete, we'll parse any given Strongs
	//	Database here to go with it:
	if (m_strStrongsImpFilepath.isEmpty()) return true;

	QFile fileStrongsImp;

	fileStrongsImp.setFileName(m_strStrongsImpFilepath);
	if (!fileStrongsImp.open(QIODevice::ReadOnly)) {
		m_strErrorString = QString("*** Failed to open Strongs Imp Database \"%1\"\n").arg(m_strStrongsImpFilepath).toUtf8().data();
		return false;
	}

	std::cerr << "Strongs: ";

	int nProgress = 0;
	QString strIndexLine;
	QByteArray baDataLine;
	while (!fileStrongsImp.atEnd()) {
		if ((nProgress % 100) == 0) std::cerr << ".";
		++nProgress;

		strIndexLine = QString(fileStrongsImp.readLine()).trimmed();
		baDataLine = fileStrongsImp.readLine();
		if (strIndexLine.isEmpty()) continue;		// Handle extra newline at end of file

		if (strIndexLine.left(3) != "$$$") {
			std::cerr << QString("\n\n*** Malformed Strongs Index: %1\n").arg(strIndexLine).toUtf8().data();
			continue;
		} else {
			strIndexLine = strIndexLine.mid(3);
		}
		CStrongsImpXmlHandler xmlHandler(strIndexLine);
		if (xmlHandler.strongsEntry().strongsIndex() == 0) {
// Note: All link indexes in database of orthography will generate this warning, so
//	leave this commented out -- but leave the line here in case we need to turn it on
//	to debug a new database:
//			std::cerr << QString("\n*** Unknown Strongs Index: %1\n").arg(strIndexLine).toUtf8().data();
			continue;
		}

		QBuffer xmlBuffer(&baDataLine);
		QXmlInputSource xmlInput(&xmlBuffer);
		QXmlSimpleReader xmlReader;

		xmlReader.setContentHandler(&xmlHandler);
		xmlReader.setErrorHandler(&xmlHandler);
		if (xmlReader.parse(xmlInput)) {
			if (m_pBibleDatabase->m_mapStrongsEntries.find(xmlHandler.strongsEntry().strongsMapIndex()) !=
				m_pBibleDatabase->m_mapStrongsEntries.cend()) {
				std::cerr << QString("\n*** Duplicate Strongs Map Index: %1\n").arg(strIndexLine).toUtf8().data();
			}
			m_pBibleDatabase->m_mapStrongsEntries[xmlHandler.strongsEntry().strongsMapIndex()] = xmlHandler.strongsEntry();
			m_pBibleDatabase->m_mapStrongsOrthographyMap.insert(xmlHandler.strongsEntry().orthographyPlainText(), xmlHandler.strongsEntry().strongsMapIndex());
		} else {
			std::cerr << QString("\n\n*** Failed to parse Strongs Index: %1\n").arg(strIndexLine).toUtf8().data();
		}
	}
	std::cerr << "\n";

	return true;
}

// ----------------------------------------------------------------------------

enum VTYPE_ENUM {
	VT_VERSE = 0,
	VT_SUPERSCRIPTION = 1,
	VT_COLOPHON = 2
};

void COSISXmlHandler::startVerseEntry(const CRelIndex &relIndex, bool bOpenEnded)
{
	VTYPE_ENUM nVT = VT_VERSE;
	if (relIndex.verse() == 0) {
		nVT = ((relIndex.chapter() == 0) ? VT_COLOPHON : VT_SUPERSCRIPTION);
	}

	if (m_bInLemma) std::cerr << "\n*** Error: Missing end of Lemma\n";
	m_bInLemma = false;
	if (m_bInTransChangeAdded) std::cerr << "\n*** Error: Missing end of TransChange Added\n";
	m_bInTransChangeAdded = false;
	if (m_bInNotes) std::cerr << "\n*** Error: Missing end of Notes\n";
	m_bInNotes = false;
	if (m_bInForeignText) std::cerr << "\n*** Error: Missing end of Foreign text\n";
	m_bInForeignText = false;
	if (m_bInDivineName) std::cerr << "\n*** Error: Missing end of Divine Name\n";
	m_bInDivineName = false;

	if ((nVT == VT_COLOPHON) && (m_bNoColophonVerses || m_bDisableColophons)) {
		if (!m_bOpenEndedColophon) {
			if (m_bInWordsOfJesus) std::cerr << "\n*** Error: Missing end of Words-of-Jesus at " << m_pBibleDatabase->PassageReferenceText(relIndex).toUtf8().data() << "\n";
			m_bInWordsOfJesus = false;
		}

		if (m_bInSuperscription) std::cerr << "\n*** Error: Missing end of Superscription\n";
		m_bInSuperscription = false;
		m_bOpenEndedSuperscription = false;

		m_bInColophon = true;
		if (bOpenEnded) m_bOpenEndedColophon = true;

		return;
	}

	if (nVT == VT_SUPERSCRIPTION) {
		bool bLocalDisableSuperscription = false;
		if ((m_ndxCurrent.verse() != 0) && (m_ndxCurrent.verse() != 1)) {
			// Note: Some texts use "in-chapter titles" to break the chapter
			//	into subdivisions.  These are not superscriptions, so only
			//	allow superscriptions if we are at the top of the chapter
			//	itself or in the first verse, such as verse 1 preverse text:
			bLocalDisableSuperscription = true;
		}
		if (bLocalDisableSuperscription || m_bNoSuperscriptionVerses || m_bDisableSuperscriptions) {
			if (!m_bOpenEndedSuperscription) {
				if (m_bInWordsOfJesus) std::cerr << "\n*** Error: Missing end of Words-of-Jesus at " << m_pBibleDatabase->PassageReferenceText(relIndex).toUtf8().data() << "\n";
				m_bInWordsOfJesus = false;
			}

			if (m_bInColophon) std::cerr << "\n*** Error: Missing end of Colophon\n";
			m_bInColophon = false;
			m_bOpenEndedColophon = false;

			m_bInSuperscription = true;
			if (bOpenEnded) m_bOpenEndedSuperscription = true;

			return;
		}
	}

	bool bPreExisted = ((m_pBibleDatabase->m_lstBookVerses[relIndex.book()-1]).find(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0))
							!= (m_pBibleDatabase->m_lstBookVerses[relIndex.book()-1]).end());
	CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)];

	if (m_nDelayedPilcrow != CVerseEntry::PTE_NONE) {
		verse.m_nPilcrow = m_nDelayedPilcrow;
		m_nDelayedPilcrow = CVerseEntry::PTE_NONE;
	}

	if (nVT == VT_VERSE) {
		m_bInVerse = true;
		if (bOpenEnded) m_bOpenEndedVerse = true;
	} else {
// Note: Can have nested Superscriptions/Colophons inside of Verse tag!  So, don't
//		do this check:
//		if (m_bInVerse) std::cerr << "\n*** Error: Missing end of Verse\n";
//		m_bInVerse = false;
//		m_bOpenEndedVerse = false;
	}

	if (nVT == VT_COLOPHON) {
		m_bInColophon = true;
		if (bOpenEnded) m_bOpenEndedColophon = true;
	} else {
		if (m_bInColophon) std::cerr << "\n*** Error: Missing end of Colophon\n";
		m_bInColophon = false;
		m_bOpenEndedColophon = false;
	}

	if (nVT == VT_SUPERSCRIPTION) {
		m_bInSuperscription = true;
		if (bOpenEnded) m_bOpenEndedSuperscription = true;
	} else {
		if (m_bInSuperscription) std::cerr << "\n*** Error: Missing end of Superscription\n";
		m_bInSuperscription = false;
		m_bOpenEndedSuperscription = false;
	}

	if (((nVT == VT_VERSE) && (!m_bOpenEndedVerse)) ||
		((nVT == VT_COLOPHON) && (!m_bOpenEndedColophon)) ||
		((nVT == VT_SUPERSCRIPTION) && (!m_bOpenEndedSuperscription))) {
		if (m_bInWordsOfJesus) std::cerr << "\n*** Error: Missing end of Words-of-Jesus at " << m_pBibleDatabase->PassageReferenceText(relIndex).toUtf8().data() << "\n";
		m_bInWordsOfJesus = false;
	} else {
		if (m_bInWordsOfJesus) {
			// We can have nested Words of Jesus with open form:
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("J:");
		}
	}

	if (m_bInBracketNotes) {
		// If still within BracketNotes from previous verse,
		//	extended it into this one:
		verse.m_strText.append(g_chrParseTag);
		verse.m_lstParseStack.push_back("N:");
	}

	if (nVT == VT_VERSE) {
		if (!bPreExisted) {			// Only increment verse counts if this isn't a duplicate (pre-existing) verse.  Otherwise, we'll crash in the word output and summary phase
			unsigned int nVerseOffset = 1;
			unsigned int nTst = bookIndexToTestamentIndex(relIndex.book());
			if (CRelIndex(0, relIndex.chapter(), relIndex.verse(), 0) == g_arrBooks[relIndex.book()-1].m_ndxStartingChapterVerse) nVerseOffset = g_arrBooks[relIndex.book()-1].m_ndxStartingChapterVerse.verse();
			m_pBibleDatabase->m_EntireBible.m_nNumVrs += nVerseOffset;
			assert(static_cast<unsigned int>(nTst) <= m_pBibleDatabase->m_lstTestaments.size());
			m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumVrs += nVerseOffset;
			assert(m_pBibleDatabase->m_lstBooks.size() > static_cast<unsigned int>(relIndex.book()-1));
			m_pBibleDatabase->m_lstBooks[relIndex.book()-1].m_nNumVrs += nVerseOffset;
			m_pBibleDatabase->m_mapChapters[CRelIndex(relIndex.book(), relIndex.chapter(), 0, 0)].m_nNumVrs += nVerseOffset;
		}
		if ((relIndex.book() == PSALMS_BOOK_NUM) && (relIndex.chapter() == 119) && (((relIndex.verse()-1)%8) == 0)) {
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("M:");
		}
	}

	if (nVT == VT_COLOPHON) {
		assert(m_pBibleDatabase->m_lstBooks.size() > static_cast<unsigned int>(relIndex.book()-1));
		m_pBibleDatabase->m_lstBooks[relIndex.book()-1].m_bHaveColophon = true;
	}

	if (nVT == VT_SUPERSCRIPTION) {
		m_pBibleDatabase->m_mapChapters[CRelIndex(relIndex.book(), relIndex.chapter(), 0, 0)].m_bHaveSuperscription = true;
	}
}

void COSISXmlHandler::charactersVerseEntry(const CRelIndex &relIndex, const QString &strText)
{
	VTYPE_ENUM nVT = VT_VERSE;
	if (relIndex.verse() == 0) {
		nVT = ((relIndex.chapter() == 0) ? VT_COLOPHON : VT_SUPERSCRIPTION);
	}

	if ((nVT == VT_COLOPHON) && (m_bNoColophonVerses || m_bDisableColophons)) {
		return;
	}
	if (nVT == VT_SUPERSCRIPTION) {
		bool bLocalDisableSuperscription = false;
		if ((m_ndxCurrent.verse() != 0) && (m_ndxCurrent.verse() != 1)) {
			// Note: Some texts use "in-chapter titles" to break the chapter
			//	into subdivisions.  These are not superscriptions, so only
			//	allow superscriptions if we are at the top of the chapter
			//	itself or in the first verse, such as verse 1 preverse text:
			bLocalDisableSuperscription = true;
		}
		if (bLocalDisableSuperscription || m_bNoSuperscriptionVerses || m_bDisableSuperscriptions) return;
	}

	assert(!strText.contains(g_chrParseTag, Qt::CaseInsensitive));
	if (strText.contains(g_chrParseTag, Qt::CaseInsensitive)) {
		std::cerr << "\n*** ERROR: Text contains the special parse tag!!  Change the tag in KJVDataParse and try again!\n";
	}

	QString strTempText = strText;

	CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)];

	// Do bracket footnotes ahead of bracket colophons
	if (m_bUseBracketFootnotes) {		// Note: Bracket-Footnotes will appear in normal verse text during parsing
		int nBracketRefCount = 0;
		int ndxStart;
		int ndxEnd;
		while (m_bInBracketNotes || ((ndxStart = strTempText.indexOf(QChar('['))) != -1)) {		// If continued from previous verse or starting in this verse
			if (!m_bInBracketNotes) {
				strTempText.replace(ndxStart, 1, g_chrParseTag);
				verse.m_lstParseStack.push_back("N:");
			} else {
				// Here if this is a continuation from a previous entry.  If
				//	so, the starting parse tag will have already been written.
				//	In this case, set up to find the ending:
				ndxStart = -1;		// Set to -1 so end search is correct below
				m_bInBracketNotes = false;		// Clear flag to break out of this loop
			}
			++nBracketRefCount;
			ndxEnd = strTempText.indexOf(QChar(']'), ndxStart+1);
			if (ndxEnd != -1) {
				strTempText.replace(ndxEnd, 1, g_chrParseTag);
				verse.m_lstParseStack.push_back("n:");
				--nBracketRefCount;
			}
		}
		if (nBracketRefCount) m_bInBracketNotes = true;		// Keep it set if still in bracketing
		// Allow one marker carryover without warning:
		if ((nBracketRefCount > 1) || (strTempText.contains(QChar('['))) || (strTempText.contains(QChar(']')))) {
			std::cerr << "\n*** Warning: Mismatched Bracket-Footnote Markers\n";
		}
	} else
	  // Do bracket colophons ahead of bracket italics
	  if ((m_bUseBracketColophons) && (nVT == VT_VERSE)) {		// Note: Bracket-Colophons will appear in normal verse text during parsing, not a "real colophon"
		if (!m_bInColophon) {
			int ndxStart;
			int ndxEnd;
			if ((ndxStart = strTempText.indexOf(QChar('['))) != -1) {
				ndxEnd = strTempText.mid(ndxStart+1).indexOf(QChar(']'));
				if (!m_bNoColophonVerses) {
					m_pBibleDatabase->m_lstBooks[relIndex.book()-1].m_bHaveColophon = true;
					CVerseEntry &colophonVerse = (m_pBibleDatabase->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), 0, 0, 0)];
					colophonVerse.m_strText += strTempText.mid(ndxStart+1).mid(0, ndxEnd);
				}
				m_ndxColophon = CRelIndex(relIndex.book(), 0, 0, 0);
				if (ndxEnd != -1) {
					m_bInColophon = true;			// Though short lived here, set the flags for endVerseEntry() processing, etc...
					m_bOpenEndedColophon = true;
					strTempText = strTempText.mid(0, ndxStart-1) + strTempText.mid(ndxStart+1).mid(ndxEnd+1);
					endVerseEntry(m_ndxColophon);
					m_ndxColophon.clear();
					m_bInColophon = false;
					m_bOpenEndedColophon = false;
				} else {
					strTempText.clear();
					m_bInColophon = true;
					m_bOpenEndedColophon = true;
				}
			}
		} else if (m_bInColophon) {
			int ndxEnd;
			CVerseEntry &colophonVerse = (m_pBibleDatabase->m_lstBookVerses[relIndex.book()-1])[m_ndxColophon];
			if ((ndxEnd = strTempText.indexOf(QChar(']'))) != -1) {
				if (!m_bNoColophonVerses) {
					colophonVerse.m_strText += strTempText.mid(0, ndxEnd);
				}
				strTempText = strTempText.mid(ndxEnd+1);
				endVerseEntry(m_ndxColophon);
				m_ndxColophon.clear();
				m_bInColophon = false;
				m_bOpenEndedColophon = false;
			} else {
				if (!m_bNoColophonVerses) {
					colophonVerse.m_strText += strTempText;
				}
				strTempText.clear();
			}
		}
	} else if (m_bBracketItalics) {
		int nItalicRefCount = 0;
		int ndxStart;
		int ndxEnd;
		while ((ndxStart = strTempText.indexOf(QChar('['))) != -1) {
			strTempText.replace(ndxStart, 1, g_chrParseTag);
			verse.m_lstParseStack.push_back("T:");
			++nItalicRefCount;
			ndxEnd = strTempText.indexOf(QChar(']'), ndxStart+1);
			if (ndxEnd != -1) {
				strTempText.replace(ndxEnd, 1, g_chrParseTag);
				verse.m_lstParseStack.push_back("t:");
				--nItalicRefCount;
			}
		}
		if ((nItalicRefCount != 0) || (strTempText.contains(QChar('['))) || (strTempText.contains(QChar(']')))) {
			std::cerr << "\n*** Warning: Mismatched Bracket-Italic Markers\n";
		}
	}

	//	verse.m_strText += strText;
	verse.m_strText += (m_bInDivineName ? strTempText.toUpper() : strTempText);
}

void COSISXmlHandler::endVerseEntry(CRelIndex &relIndex)
{
	VTYPE_ENUM nVT = VT_VERSE;
	if (relIndex.verse() == 0) {
		nVT = ((relIndex.chapter() == 0) ? VT_COLOPHON : VT_SUPERSCRIPTION);
	}

	if ((nVT == VT_COLOPHON) && (m_bNoColophonVerses || m_bDisableColophons)) {
		m_bInColophon = false;
		return;
	}
	if (nVT == VT_SUPERSCRIPTION) {
		bool bLocalDisableSuperscription = false;
		if ((m_ndxCurrent.verse() != 0) && (m_ndxCurrent.verse() != 1)) {
			// Note: Some texts use "in-chapter titles" to break the chapter
			//	into subdivisions.  These are not superscriptions, so only
			//	allow superscriptions if we are at the top of the chapter
			//	itself or in the first verse, such as verse 1 preverse text:
			bLocalDisableSuperscription = true;
		}
		if (bLocalDisableSuperscription || m_bNoSuperscriptionVerses || m_bDisableSuperscriptions) {
			m_bInSuperscription = false;
			return;
		}
	}

	CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)];
	QString strTemp = verse.m_strText;

	if (m_bInBracketNotes) {
		// If BracketNotes crosses verse boundary, put matching
		//	tag at the end of the verse:
		strTemp.append(g_chrParseTag);
		verse.m_lstParseStack.push_back("n:");
	}

	unsigned int nWordCount = 0;
	bool bInWord = false;
	bool bInlineNote = false;
	CRelIndex ndxLastFootnoteActive = relIndex;
	CRelIndex ndxFootnoteActive = relIndex;
	ndxFootnoteActive.setWord(nWordCount+1);
	QString strWord;
	QString strRichWord;
	QStringList lstWords;

	TPhraseTag tagLemmaEntry;
	QString strLemmaAttr;

	QStringList lstRichWords;
	bool bHaveDoneTemplateWord = false;				// Used to tag words crossing parse-stack boundary (i.e. half the word is inside the parse operator and half is outside, like the word "inasmuch")
	while (!strTemp.isEmpty()) {
		bool bIsHyphen = g_strHyphens.contains(strTemp.at(0));
		bool bIsApostrophe = g_strApostrophes.contains(strTemp.at(0));
		if (strTemp.at(0) == g_chrParseTag) {
			if (bInWord) {
				if (!bHaveDoneTemplateWord) {
					++nWordCount;
					ndxFootnoteActive.setWord(nWordCount+1);
					verse.m_strTemplate += QString("w");
				}
				bHaveDoneTemplateWord = true;
			}
			assert(!verse.m_lstParseStack.isEmpty());
			if (!verse.m_lstParseStack.isEmpty()) {
				QString strParse = verse.m_lstParseStack.at(0);
				verse.m_lstParseStack.pop_front();
				int nPos = strParse.indexOf(':');
				assert(nPos != -1);		// Every ParseStack entry must contain a ':'
				QString strOp = strParse.left(nPos);
				if (strOp.compare("L") == 0) {
					tagLemmaEntry.setRelIndex(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), nWordCount+1));
					tagLemmaEntry.setCount(1);
					strLemmaAttr = strParse.mid(nPos+1);
				} else if (strOp.compare("l") == 0) {
					tagLemmaEntry.setCount((nWordCount+1)-tagLemmaEntry.relIndex().word());
					m_pBibleDatabase->m_mapLemmaEntries[tagLemmaEntry.relIndex()] = CLemmaEntry(tagLemmaEntry, strLemmaAttr);
				} else if (strOp.compare("T") == 0) {
					if (!bInlineNote) {
						verse.m_strTemplate += "T";
					} else {
						// Convert TransChangeAdded in footnotes to brackets:
						CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
						footnote.setText(footnote.text() + QChar('['));
					}
				} else if (strOp.compare("t") == 0) {
					if (!bInlineNote) {
						verse.m_strTemplate += "t";
					} else {
						// Convert TransChangeAdded in footnotes to brackets:
						CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
						footnote.setText(footnote.text() + QChar(']'));
					}
				} else if (strOp.compare("J") == 0) {
					verse.m_strTemplate += "J";
				} else if (strOp.compare("j") == 0) {
					verse.m_strTemplate += "j";
				} else if (strOp.compare("D") == 0) {
					verse.m_strTemplate += "D";
				} else if (strOp.compare("d") == 0) {
					verse.m_strTemplate += "d";
				} else if (strOp.compare("M") == 0) {
					verse.m_strTemplate += "M";
					// For special Ps 119 Hebrew markers, add x-extra-p Pilcrow to
					//		add a pseudo-paragraph break if there currently isn't
					//		one, as it makes these more readable:
					if (verse.m_nPilcrow == CVerseEntry::PTE_NONE)
						verse.m_nPilcrow = CVerseEntry::PTE_EXTRA;
				} else if (strOp.compare("N") == 0) {
					if ((!m_bUseBracketFootnotes && m_bInlineFootnotes) ||
						(m_bUseBracketFootnotes && !m_bUseBracketFootnotesExcluded)) {
						if (ndxFootnoteActive != ndxLastFootnoteActive) {
							verse.m_strTemplate += "N";
						}
					} else {
						// If not outputting the inline note, remove the
						//	extra space from the text that preceeded it:
						if (!verse.m_strTemplate.isEmpty() &&
							verse.m_strTemplate.at(verse.m_strTemplate.size()-1).isSpace()) {
							verse.m_strTemplate = verse.m_strTemplate.left(verse.m_strTemplate.size()-1);
						}
					}
					bInlineNote = true;

					// If the active footnote index already has text, add a
					//	separator so that this next footnote starting won't
					//	be jammed up against the first.  This is necessary
					//	for texts, like the KJV-1769, where multiple study
					//	notes exists back-to-back at the end of a verse:
					CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
					if (!footnote.text().isEmpty()) {
						footnote.setText(footnote.text() + "; ");
					}
				} else if (strOp.compare("n") == 0) {
					if ((!m_bUseBracketFootnotes && m_bInlineFootnotes) ||
						(m_bUseBracketFootnotes && !m_bUseBracketFootnotesExcluded)) {
						if (ndxFootnoteActive != ndxLastFootnoteActive) {
							verse.m_strTemplate += "n";
						}
					}
					bInlineNote = false;

					ndxLastFootnoteActive = ndxFootnoteActive;
				} else {
					assert(false);		// Unknown ParseStack Operator!
				}
			}
		} else if (bInlineNote) {
			CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
			footnote.setText(footnote.text() + strTemp.at(0));
		} else if ((strTemp.at(0).unicode() < 128) ||
			(g_strNonAsciiNonWordChars.contains(strTemp.at(0))) ||
			(strTemp.at(0) == g_chrPilcrow) ||
			(strTemp.at(0) == g_chrParseTag) ||
			(bIsHyphen) ||
			(bIsApostrophe)) {
			if ((g_strAsciiWordChars.contains(strTemp.at(0))) ||
				((bIsHyphen) && (!strRichWord.isEmpty())) ||				// Don't let words start with hyphen or apostrophe
				((bIsApostrophe) && (!strRichWord.isEmpty()))) {
				bInWord = true;
				if (bIsHyphen) {
					strWord += '-';
				} else if (bIsApostrophe) {
					strWord += '\'';
				} else strWord += strTemp.at(0);
				strRichWord += strTemp.at(0);
			} else {
				if (bInWord) {
					assert(!strRichWord.isEmpty());
					assert(!strWord.isEmpty());

					if ((strRichWord.size() == 1) &&
						((g_strHyphens.contains(strRichWord.at(0))) ||
						 (g_strApostrophes.contains(strRichWord.at(0))))) {
						// Don't count words that are only a hyphen or apostrophe:
						verse.m_strTemplate += strRichWord;
					} else if (m_bNoArabicNumeralWords && (QRegExp("\\d*").exactMatch(strWord))) {
						// If we aren't counting Arabic Numerals as words, move them out to the verse template for rendering but not counting:
						verse.m_strTemplate += strWord;		// It shouldn't matter here if we use Word or RichWord (unlike apostrophes above)
					} else {
						QString strPostTemplate;		// Needed so we get the "w" marker in the correct place
						// Remove trailing hyphens from words and put them in the template.
						//		We'll keep trailing apostophes for posessive words, like: "Jesus'":
						while ((!strRichWord.isEmpty()) && (g_strHyphens.contains(strRichWord.at(strRichWord.size()-1)))) {
							assert(!strWord.isEmpty());
							strPostTemplate += strRichWord.at(strRichWord.size()-1);
							strRichWord = strRichWord.left(strRichWord.size()-1);
							strWord = strWord.left(strWord.size()-1);
						}
						if (!strRichWord.isEmpty()) {
							if (!bHaveDoneTemplateWord) {
								nWordCount++;
								ndxFootnoteActive.setWord(nWordCount+1);
								verse.m_strTemplate += QString("w");
							}
							relIndex.setWord(verse.m_nNumWrd + nWordCount);
							lstWords.append(strWord);
							lstRichWords.append(strRichWord);
						}
						verse.m_strTemplate += strPostTemplate;
					}
					strWord.clear();
					strRichWord.clear();
					bInWord = false;
				}
				if (strTemp.at(0) != g_chrPilcrow) {
					if (strTemp.at(0) == g_chrParseTag) {
						std::cerr << "\n*** WARNING: Text contains our special parse tag character and may cause parsing issues\nTry recompiling using a different g_chrParseTag character!\n";
					}
					verse.m_strTemplate += strTemp.at(0);
				} else {
					// If we see a pilcrow marker in the text, but the OSIS didn't declare it, go ahead and add it
					//	as a marker, but flag it of type "added":
					if (verse.m_nPilcrow == CVerseEntry::PTE_NONE) verse.m_nPilcrow = CVerseEntry::PTE_MARKER_ADDED;
				}
				bHaveDoneTemplateWord = false;
			}
		} else {
			if (!m_strParsedUTF8Chars.contains(strTemp.at(0))) m_strParsedUTF8Chars += strTemp.at(0);

			bInWord = true;
			if (strTemp.at(0) == QChar(0x00C6)) {				// U+00C6	&#198;		AE character
				strWord += "Ae";
			} else if (strTemp.at(0) == QChar(0x00E6)) {		// U+00E6	&#230;		ae character
				strWord += "ae";
			} else if (strTemp.at(0) == QChar(0x0132)) {		// U+0132	&#306;		IJ character
				strWord += "IJ";
			} else if (strTemp.at(0) == QChar(0x0133)) {		// U+0133	&#307;		ij character
				strWord += "ij";
			} else if (strTemp.at(0) == QChar(0x0152)) {		// U+0152	&#338;		OE character
				strWord += "Oe";
			} else if (strTemp.at(0) == QChar(0x0153)) {		// U+0153	&#339;		oe character
				strWord += "oe";
			} else {
				strWord += strTemp.at(0);			// All other UTF-8 leave untranslated
			}
			strRichWord += strTemp.at(0);
		}

		strTemp = strTemp.right(strTemp.size()-1);
	}

	assert(verse.m_lstParseStack.isEmpty());		// We should have exhausted the stack above!

	if (bInWord) {
		if ((strRichWord.size() == 1) &&
			((g_strHyphens.contains(strRichWord.at(0))) ||
			 (g_strApostrophes.contains(strRichWord.at(0))))) {
			// Don't count words that are only a hyphen or apostrophe:
			verse.m_strTemplate += strRichWord;
		} else if (m_bNoArabicNumeralWords && (QRegExp("\\d*").exactMatch(strWord))) {
			// If we aren't counting Arabic Numerals as words, move them out to the verse template for rendering but not counting:
			verse.m_strTemplate += strWord;		// It shouldn't matter here if we use Word or RichWord (unlike apostrophes above)
		} else {
			QString strPostTemplate;		// Needed so we get the "w" marker in the correct place
			// Remove trailing hyphens from words and put them in the template.
			//		We'll keep trailing apostophes for posessive words, like: "Jesus'":
			while ((!strRichWord.isEmpty()) && (g_strHyphens.contains(strRichWord.at(strRichWord.size()-1)))) {
				assert(!strWord.isEmpty());
				strPostTemplate += strRichWord.at(strRichWord.size()-1);
				strRichWord = strRichWord.left(strRichWord.size()-1);
				strWord = strWord.left(strWord.size()-1);
			}
			if (!strRichWord.isEmpty()) {
				if (!bHaveDoneTemplateWord) {
					nWordCount++;
					ndxFootnoteActive.setWord(nWordCount+1);
					verse.m_strTemplate += QString("w");
				}
				relIndex.setWord(verse.m_nNumWrd + nWordCount);
				lstWords.append(strWord);
				lstRichWords.append(strRichWord);
			}
			verse.m_strTemplate += strPostTemplate;
		}
		strWord.clear();
		strRichWord.clear();
		bInWord = false;
	}
	bHaveDoneTemplateWord = false;

	m_pBibleDatabase->m_EntireBible.m_nNumWrd += nWordCount;
	m_pBibleDatabase->m_lstTestaments[m_pBibleDatabase->m_lstBooks.at(relIndex.book()-1).m_nTstNdx-1].m_nNumWrd += nWordCount;
	m_pBibleDatabase->m_lstBooks[relIndex.book()-1].m_nNumWrd += nWordCount;
	if (relIndex.chapter() != 0) {
		m_pBibleDatabase->m_mapChapters[CRelIndex(relIndex.book(), relIndex.chapter(), 0, 0)].m_nNumWrd += nWordCount;
	}
	verse.m_nNumWrd += nWordCount;
	verse.m_lstWords.append(lstWords);
	verse.m_lstRichWords.append(lstRichWords);



//std::cout << m_pBibleDatabase->PassageReferenceText(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)).toUtf8().data() << "\n";
//std::cout << verse..m_strText.toUtf8().data() << "\n" << verse.m_strTemplate.toUtf8().data << "\n" << verse.m_lstWords.join(",").toUtf8().data() << "\n" << QString("Words: %1\n").arg(verse.m_nNumWrd).toUtf8().data();

	assert(static_cast<unsigned int>(verse.m_strTemplate.count('w')) == verse.m_nNumWrd);
	if (static_cast<unsigned int>(verse.m_strTemplate.count('w')) != verse.m_nNumWrd) {
		std::cerr << "\n" << m_pBibleDatabase->PassageReferenceText(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)).toUtf8().data();
		std::cerr << "\n*** Error: Verse word count doesn't match template word count!!!\n";
	}


	relIndex.setVerse(0);
	relIndex.setWord(0);

	if (nVT == VT_VERSE) {
		m_bInVerse = false;
	} else if (nVT == VT_SUPERSCRIPTION) {
		m_bInSuperscription = false;
		m_bInBracketNotes = false;			// Don't carry bracketed footnotes across superscription boundary
	} else if (nVT == VT_COLOPHON) {
		m_bInColophon = false;
		m_bInBracketNotes = false;			// Don't carry bracketed footnotes across colophon boundary
	}
}

// ============================================================================
// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	int nArgsFound = 0;
	bool bUnknownOption = false;
	bool bNoColophonVerses = false;
	bool bUseBracketColophons = false;
	bool bDisableColophons = false;
	bool bNoSuperscriptionVerses = false;
	bool bDisableSuperscriptions = false;
	bool bBracketItalics = false;
	bool bNoArabicNumeralWords = false;
	bool bInlineFootnotes = false;
	bool bUseBracketFootnotes = false;
	bool bUseBracketFootnotesExcluded = false;
	bool bExcludeDeuterocanonical = false;
	int nDescriptor = -1;
	QString strOSISFilename;
	QString strInfoFilename;
	QString strOutputPath;
	QString strStrongsImpPath;
	bool bLookingforSegVariant = false;
	QString strSegVariant;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			if (bLookingforSegVariant) {
				strSegVariant = strArg;
				bLookingforSegVariant = false;
			} else {
				++nArgsFound;
				if (nArgsFound == 1) {
					nDescriptor = strArg.toInt();
				} else if (nArgsFound == 2) {
					strOSISFilename = strArg;
				} else if (nArgsFound == 3) {
					strInfoFilename = strArg;
				} else if (nArgsFound == 4) {
					strOutputPath = strArg;
				} else if (nArgsFound == 5) {
					strStrongsImpPath = strArg;
				}
			}
		} else if (strArg.compare("-c") == 0) {
			bNoColophonVerses = true;
		} else if (strArg.compare("-bc") == 0) {
			bUseBracketColophons = true;
		} else if (strArg.compare("-cd") == 0) {
			bDisableColophons = true;
		} else if (strArg.compare("-s") == 0) {
			bNoSuperscriptionVerses = true;
		} else if (strArg.compare("-sd") == 0) {
			bDisableSuperscriptions = true;
		} else if (strArg.compare("-i") == 0) {
			bBracketItalics = true;
		} else if (strArg.compare("-v") == 0) {
			bLookingforSegVariant = true;
		} else if (strArg.compare("-n") == 0) {
			bNoArabicNumeralWords = true;
		} else if (strArg.compare("-f") == 0) {
			bInlineFootnotes = true;
		} else if (strArg.compare("-bf") == 0) {
			bUseBracketFootnotes = true;
		} else if (strArg.compare("-bfx") == 0) {
			bUseBracketFootnotes = true;
			bUseBracketFootnotesExcluded = true;
		} else if (strArg.compare("-x") == 0) {
			bExcludeDeuterocanonical = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound < 4) || (nArgsFound > 5) || (bUnknownOption)) {
		std::cerr << QString("KJVDataParse Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <OSIS-Database> <infofile> <datafile-path> [<Strongs-Imp-path>]\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads and parses the OSIS database and outputs all of the CSV files\n").toUtf8().data();
		std::cerr << QString("    necessary to import into KJPBS into <datafile-path>\n\n").toUtf8().data();
		std::cerr << QString("<infofile> is the path/filename to the information file to include\n\n").toUtf8().data();
		std::cerr << QString("Options\n").toUtf8().data();
		std::cerr << QString("    -c  =  Don't generate Colophons as pseudo-verses (only as footnotes)\n").toUtf8().data();
		std::cerr << QString("    -bc =  Enable Bracket Colophons (such as used in the TR text)\n").toUtf8().data();
		std::cerr << QString("           (use with -c to find the bracket colophons and remove them)\n").toUtf8().data();
		std::cerr << QString("           (Note: -bc will take precedence over -i)\n").toUtf8().data();
		std::cerr << QString("    -cd =  Disable all Colophon generation (pseudo-verses and footnote form)\n").toUtf8().data();
		std::cerr << QString("    -s  =  Don't generate Superscriptions as pseudo-verses (only as footnotes)\n").toUtf8().data();
		std::cerr << QString("    -sd =  Disable all Superscription generation (pseudo-verses and footnote form)\n").toUtf8().data();
		std::cerr << QString("    -i  =  Enable Bracket Italic detection conversion to TransChange\n").toUtf8().data();
		std::cerr << QString("    -v <variant> = Export only segment variant of <variant>\n").toUtf8().data();
		std::cerr << QString("    -n  =  Don't detect Arabic numerals as words\n").toUtf8().data();
		std::cerr << QString("    -f  =  Inline footnotes as Uncounted Parentheticals\n").toUtf8().data();
		std::cerr << QString("    -bf =  Enable Bracket Inline Footnotes (such as used in the RusSynodal)\n").toUtf8().data();
		std::cerr << QString("           (Note: -bf will take precedence over -i and -bc, and implies -f)\n").toUtf8().data();
		std::cerr << QString("    -bfx=  Enable Bracket Inline Footnotes and Exclude them\n").toUtf8().data();
		std::cerr << QString("           (Identical to -bf, but excludes them, and overrides -f)\n").toUtf8().data();
		std::cerr << QString("    -x  =  Exclude Apocrypha/Deuterocanonical Text\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			BIBLE_DESCRIPTOR_ENUM nBDETemp = static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx);
			std::cerr << QString("    %1 = %2 (%3)\n").arg(ndx).arg(bibleDescriptor(nBDETemp).m_strDBName).arg(bibleDescriptor(nBDETemp).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		return -1;
	}

	if ((nDescriptor < 0) || (static_cast<unsigned int>(nDescriptor) >= bibleDescriptorCount())) {
		std::cerr << "Unknown UUID-Index\n";
		return -1;
	}

	BIBLE_DESCRIPTOR_ENUM nBDE = static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor);
	TBibleDescriptor bblDescriptor = bibleDescriptor(nBDE);

	QDir dirOutput(strOutputPath);
	if (!dirOutput.exists()) {
		std::cerr << QString("\n\n*** Output path \"%1\" doesn't exist\n\n").arg(dirOutput.path()).toUtf8().data();
		return -2;
	}

	QFile fileOSIS;

	fileOSIS.setFileName(strOSISFilename);
	if (!fileOSIS.open(QIODevice::ReadOnly)) {
		std::cerr << QString("\n\n*** Failed to open OSIS database \"%1\"\n").arg(strOSISFilename).toUtf8().data();
		return -3;
	}

	QXmlInputSource xmlInput(&fileOSIS);
	QXmlSimpleReader xmlReader;
	COSISXmlHandler xmlHandler(bblDescriptor);

	xmlHandler.setNoColophonVerses(bNoColophonVerses);
	xmlHandler.setUseBracketColophons(bUseBracketColophons);
	xmlHandler.setDisableColophons(bDisableColophons);
	xmlHandler.setNoSuperscriptionVerses(bNoSuperscriptionVerses);
	xmlHandler.setDisableSuperscriptions(bDisableSuperscriptions);
	xmlHandler.setBracketItalics(bBracketItalics);
	xmlHandler.setNoArabicNumeralWords(bNoArabicNumeralWords);
	xmlHandler.setInlineFootnotes(bInlineFootnotes);
	xmlHandler.setUseBracketFootnotes(bUseBracketFootnotes);
	xmlHandler.setUseBracketFootnotesExcluded(bUseBracketFootnotesExcluded);
	xmlHandler.setExcludeDeuterocanonical(bExcludeDeuterocanonical);
	xmlHandler.setSegVariant(strSegVariant);
	xmlHandler.setStrongsImpFilepath(strStrongsImpPath);

	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.setErrorHandler(&xmlHandler);
//	xmlReader.setFeature("http://www.bibletechnologies.net/2003/OSIS/namespace", true);

	if (!xmlReader.parse(xmlInput)) {
		std::cerr << QString("\n\n*** Failed to parse OSIS database \"%1\"\n%2\n").arg(strOSISFilename).arg(xmlHandler.errorString()).toUtf8().data();
		return -4;
	}

	const CBibleDatabase *pBibleDatabase = xmlHandler.bibleDatabase();

	// ------------------------------------------------------------------------

	QFile fileTestaments;	// Testaments CSV being written
	QFile fileBooks;		// Books CSV being written (Originally known as "TOC")
	QFile fileChapters;		// Chapters CSV being written (Originally known as "Layout")
	QFile fileVerses;		// Verses CSV being written (Originally known as "BOOKS")
	QFile fileWords;		// Words CSV being written
	QFile fileFootnotes;	// Footnotes CSV being written
	QFile fileWordSummary;	// Words Summary CSV being written
	QFile filePhrases;		// Default search phrases CSV being written (deprecated)
	QFile fileLemmas;		// Lemma list being written
	QFile fileStrongs;		// Strongs database list being written

	QFileInfo fiInfoFile(strInfoFilename);
	if (!strInfoFilename.isEmpty()) {
		if ((!fiInfoFile.exists()) || (!fiInfoFile.isFile())) {
			std::cerr << QString("\n\n*** Info Filename \"%1\" doesn't exist\n\n").arg(strInfoFilename).toUtf8().data();
			return -3;
		}
		if (!QFile::copy(fiInfoFile.absoluteFilePath(), dirOutput.absoluteFilePath(fiInfoFile.fileName()))) {
			std::cerr << QString("\n\n*** Failed to copy Info File from \"%1\" to \"%2\"\n\n")
						 .arg(fiInfoFile.absoluteFilePath())
						 .arg(dirOutput.absoluteFilePath(fiInfoFile.fileName()))
						 .toUtf8().data();
		}
	}

	QSettings settingsDBInfo(dirOutput.absoluteFilePath("DBInfo.ini"), QSettings::IniFormat);
	settingsDBInfo.clear();
	settingsDBInfo.beginGroup("BibleDBInfo");
	settingsDBInfo.setValue("Language", xmlHandler.language());
	settingsDBInfo.setValue("Name", bblDescriptor.m_strDBName);
	settingsDBInfo.setValue("Description", bblDescriptor.m_strDBDesc);
	settingsDBInfo.setValue("UUID", bblDescriptor.m_strUUID);
	settingsDBInfo.setValue("InfoFilename", (!strInfoFilename.isEmpty() ? fiInfoFile.fileName() : QString()));
	settingsDBInfo.endGroup();

	fileTestaments.setFileName(dirOutput.absoluteFilePath("TESTAMENT.csv"));
	if (!fileTestaments.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Testament Output File \"%1\"\n").arg(fileTestaments.fileName()).toUtf8().data();
		return -5;
	}

	fileTestaments.write(QString(QChar(0xFEFF)).toUtf8());			// UTF-8 BOM
	fileTestaments.write(QString("TstNdx, TstName\r\n").toUtf8());
	for (unsigned int nTst=1; nTst<=pBibleDatabase->bibleEntry().m_nNumTst; ++nTst) {
		fileTestaments.write(QString("%1,\"%2\"\r\n").arg(nTst).arg(pBibleDatabase->testamentEntry(nTst)->m_strTstName).toUtf8());
	}
	std::cerr << QFileInfo(fileTestaments).fileName().toUtf8().data() << "\n";
	fileTestaments.close();

	fileBooks.setFileName(dirOutput.absoluteFilePath("TOC.csv"));
	if (!fileBooks.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Books Output File \"%1\"\n").arg(fileBooks.fileName()).toUtf8().data();
		return -6;
	}

	fileBooks.write(QString(QChar(0xFEFF)).toUtf8());			// UTF-8 BOM
	fileBooks.write(QString("BkNdx,TstBkNdx,TstNdx,BkName,BkAbbr,TblName,NumChp,NumVrs,NumWrd,Cat,Desc\r\n").toUtf8());

	fileChapters.setFileName(dirOutput.absoluteFilePath("LAYOUT.csv"));
	if (!fileChapters.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Chapters Output File \"%1\"\n").arg(fileChapters.fileName()).toUtf8().data();
		return -7;
	}

	fileChapters.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileChapters.write(QString("BkChpNdx,NumVrs,NumWrd,BkAbbr,ChNdx\r\n").toUtf8());


	TChapterVerseCounts lstChapterVerseCounts;
	// mapWordList will be for ALL forms of all words so that we can get mapping/counts
	//	for all unique forms of words.  Words in this map will NOT be indexed by the
	//	lowercase nor the base-form of the word, but by the actual word itself.
	//	Then, once we've used this to build all of the unique indexes for the different
	//	word forms, we'll consolidate them and ultimately build the database entries which will
	//	contain the wordlists by lowercase word and have the main word value and all
	//	alternates:
	TWordListMap mapWordList;				// mapWordList is indexed by the Word form as-is (no changes in case)
	TAltWordListMap mapAltWordList;			// mapAltWordList is indexed by the LowerCase form of the Word

	unsigned int nWordAccum = 0;
	for (unsigned int nBk=1; nBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		if (nBk > NUM_BK) {
			std::cerr << QString("\n*** WARNING: Module has extra Book : %1\n").arg(nBk).toUtf8().data();
			lstChapterVerseCounts.push_back(QStringList());
		} else {
			// Predefined books from our list:
			lstChapterVerseCounts.push_back(g_arrChapterVerseCounts[nBk-1].split(","));
		}
		const CBookEntry *pBook = pBibleDatabase->bookEntry(nBk);
		bool bHadBook = true;
		if ((pBook == NULL) || (pBook->m_strTblName.isEmpty())) {
			bHadBook = false;
			pBook = xmlHandler.addBookToBibleDatabase(nBk);
			std::cerr << QString("\n*** WARNING: Module is missing Book : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0, 0))).toUtf8().data();
		}
		(const_cast<CBookEntry*>(pBook))->m_nWrdAccum = nWordAccum;

		fileVerses.setFileName(dirOutput.absoluteFilePath(QString("BOOK_%1_%2.csv").arg(nBk, 2, 10, QChar('0')).arg(pBook->m_strTblName)));
		if (!fileVerses.open(QIODevice::WriteOnly)) {
			std::cerr << QString("\n\n*** Failed to open Verses Output File \"%1\"\n").arg(fileVerses.fileName()).toUtf8().data();
			return -8;
		}

		fileVerses.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
		fileVerses.write(QString("ChpVrsNdx,NumWrd,nPilcrow,PText,RText,TText\r\n").toUtf8());

		std::cerr << QFileInfo(fileVerses).fileName().toUtf8().data();

		unsigned int nChapterWordAccum = 0;
		unsigned int nChaptersExpected = qMax(pBook->m_nNumChp, static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size()));
		for (unsigned int nChp=(pBook->m_bHaveColophon ? 0 : 1); nChp<=nChaptersExpected; ++nChp) {
			if ((nChp != 0) && (nChp > static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size()))) {
				std::cerr << QString("\n*** WARNING: Module has extra Chapter : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toUtf8().data();
			}
			const CChapterEntry *pChapter = ((nChp != 0) ? pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0)) : NULL);
			bool bChapterMissing = false;
			Q_UNUSED(bChapterMissing);
			if ((nChp != 0) && (pChapter == NULL)) {
				bChapterMissing = true;
				if ((nChp >= g_arrBooks[nBk-1].m_ndxStartingChapterVerse.chapter()) && (bHadBook)) {
					std::cerr << QString("\n*** WARNING: Module is missing Chapter : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toUtf8().data();
				}
				pChapter = pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0), true);
				if (pChapter == NULL) {
					std::cerr << "*** Unable to create missing chapter\n";
					continue;
				} else {
					(const_cast<CBookEntry*>(pBook))->m_nNumChp++;
				}
			}
			if (pChapter != NULL) {
				(const_cast<CChapterEntry*>(pChapter))->m_nWrdAccum = nWordAccum;

				std::cerr << ".";
			}

//			std::cout << QString("%1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toUtf8().data();
			unsigned int nVerseWordAccum = 0;
			unsigned int nVersesExpected = ((pChapter != NULL) ? qMax(pChapter->m_nNumVrs, static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0)) : 0);

			// Remove empty non-canonical verses that got added posthumous during parsing
			//	that are trailing at the end of the chapter (leave any in the middle of
			//	the chapter, as they are placeholders):
			if (nChp > 0) {		// Do this only for non-colophons
				for (unsigned int nVrs=nVersesExpected; nVrs > static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0); --nVrs) {
					const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
					if ((pVerse != NULL) && (pVerse->m_nNumWrd == 0) && (pVerse->m_strTemplate.trimmed().isEmpty())) {
						// Ideally, we would also delete the verse here from pBibleDatabase->m_lstBookVerses,
						//	but that's a private member, and since we are using the counts to output the
						//	verses below, rather than the extents of that member, we can simply decrement
						//	the counts and it will be correctly removed when writing:
						std::cerr << QString("\n*** Removing empty extra verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
						(const_cast<CBookEntry*>(pBook))->m_nNumVrs--;
						(const_cast<CChapterEntry*>(pChapter))->m_nNumVrs--;
						nVersesExpected--;		// This count is used for the loop below, so it needs decrementing too
					} else {
						break;		// Stop when we find the first one that isn't empty as any other empties are needed as placeholders
					}
				}
			}

			for (unsigned int nVrs=((pChapter != NULL) ? (pChapter->m_bHaveSuperscription ? 0 : 1) : 0); nVrs<=nVersesExpected; ++nVrs) {
				if ((nVrs != 0) && (nVrs > static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0))) {
					std::cerr << QString("\n*** WARNING: Module has extra Verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
				}
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
				bool bVerseMissing = false;
				if (pVerse == NULL) {
					if ((nChp == 0) || (nVrs == 0)) assert(false);
					bVerseMissing = true;
					if ((CRelIndex(0, nChp, nVrs, 0) >= g_arrBooks[nBk-1].m_ndxStartingChapterVerse) && (bHadBook)) {
						std::cerr << QString("\n*** WARNING: Module is missing Verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
					}
					pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0), true);
					if (pVerse == NULL) {
						std::cerr << "*** Unable to create missing verse\n";
						continue;
					} else {
						(const_cast<CBookEntry*>(pBook))->m_nNumVrs++;
						(const_cast<CChapterEntry*>(pChapter))->m_nNumVrs++;
					}
				}
//				std::cout << QString("%1 : \"%2\"\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).arg(pVerse->m_strTemplate).toUtf8().data();

				(const_cast<CVerseEntry*>(pVerse))->m_nWrdAccum = nWordAccum;
				nVerseWordAccum += pVerse->m_nNumWrd;
				nWordAccum += pVerse->m_nNumWrd;

				if (pVerse->m_nNumWrd > 0) {
					uint32_t nNormal = pBibleDatabase->NormalizeIndexNoAccum(CRelIndex(nBk, nChp, nVrs, 1));
					if (nNormal != pVerse->m_nWrdAccum+1) {
						std::cerr << QString("\n**** Error: Normal for CRelIndex(%1, %2, %3, 1)->%4 != %5\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nNormal).arg(pVerse->m_nWrdAccum+1).toUtf8().data();
						assert(nNormal == (pVerse->m_nWrdAccum+1));
					}
					CRelIndex ndxDenormal = pBibleDatabase->DenormalizeIndexNoAccum(pVerse->m_nWrdAccum+1);
					if (ndxDenormal != CRelIndex(nBk, nChp, nVrs, 1)) {
						std::cerr << QString("\n*** Error: Denormal for %1  !=  CRelIndex(%2, %3, %4, 1)->%5\n")
									 .arg(ndxDenormal.index()).arg(nBk).arg(nChp).arg(nVrs).arg(CRelIndex(nBk, nChp, nVrs, 1).index()).toUtf8().data();
						assert(ndxDenormal == CRelIndex(nBk, nChp, nVrs, 1));
					}
				} else {
					if (!bVerseMissing) {
						if (nVrs == 0) {
							if (nChp == 0) {
								std::cerr << QString("\n*** Warning: Colophon has no text: %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
							} else {
								std::cerr << QString("\n*** Warning: Superscription has no text: %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
							}
						} else {
							std::cerr << QString("\n*** Warning: Verse has no text: %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
						}
					}
				}

//				QStringList lstTempRich = CVerseTextRichifier::parse(CRelIndex(nBk,nChp, nVrs, 0), pBibleDatabase, pVerse, CVerseTextRichifierTags(), false).split('\"');
//				QString strBuffRich = lstTempRich.join("\"\"");
//				QStringList lstTempPlain = CVerseTextRichifier::parse(CRelIndex(nBk,nChp, nVrs, 0), pBibleDatabase, pVerse, CVerseTextPlainRichifierTags(), false).split('\"');
//				QString strBuffPlain = lstTempPlain.join("\"\"");

				QStringList lstTempTemplate = pVerse->m_strTemplate.trimmed().split('\"');
				QString strBuffTemplate = lstTempTemplate.join("\"\"");

				// ChpVrsNdx,NumWrd,nPilcrow,PText,RText,TText
				fileVerses.write(QString("%1,%2,%3,\"%4\",\"%5\",\"%6\"\r\n")
								 .arg(CRelIndex(0,0,nChp,nVrs).index())		// 1
								 .arg(pVerse->m_nNumWrd)					// 2
								 .arg(pVerse->m_nPilcrow)					// 3
								 .arg(QString())							// 4	(PlainText)
								 .arg(QString())							// 5	(RichText)
								 .arg(strBuffTemplate)						// 6	(TemplateText)
								 .toUtf8());


				// Needs to be after we calculate nWordAccum above so we can output anchor tags:
//				QString strTemp = CVerseTextRichifier::parse(pVerse, CVerseTextRichifierTags(), false);
//				std::cout << QString("%1 : %2\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).arg(strTemp).toUtf8().data();


/*
				std::cout << g_arrBooks[nBk-1].m_strOsisAbbr.toUtf8().data() << QString(" %1:%2 : ").arg(nChp).arg(nVrs).toUtf8().data();
				if (pVerse->m_nPilcrow == CVerseEntry::PTE_NONE) {
					std::cout << "false \n";
				} else {
					std::cout << "true ";
					switch (pVerse->m_nPilcrow) {
						case CVerseEntry::PTE_MARKER:
							std::cout << "(Marker)";
							break;
						case CVerseEntry::PTE_MARKER_ADDED:
							std::cout << "(Added)";
							break;
						case CVerseEntry::PTE_EXTRA:
							std::cout << "(Extra)";
							break;
						default:
							break;
					}
					std::cout << "\n";
				}
*/

				// Now use the words we've gathered from this verse to build the Word Lists and Concordance:
				assert(pVerse->m_nNumWrd == static_cast<unsigned int>(pVerse->m_lstWords.size()));
				assert(pVerse->m_nNumWrd == static_cast<unsigned int>(pVerse->m_lstRichWords.size()));
				for (unsigned int nWrd=1; nWrd<=pVerse->m_nNumWrd; ++nWrd) {
					QString strWord = pVerse->m_lstWords.at(nWrd-1);
					QString strRichWord = pVerse->m_lstRichWords.at(nWrd-1);
					CWordEntry &wordEntry = mapWordList[strRichWord];
					TAltWordSet &wordSet = mapAltWordList[strRichWord.toLower()];
					wordSet.insert(strRichWord);
					wordEntry.m_ndxNormalizedMapping.push_back(pVerse->m_nWrdAccum+nWrd);
				}

				if (pVerse->m_nNumWrd > CRelIndex::maxWordCount()) {
					std::cerr << QString("\n*** Warning: Verse word count (%1) exceeds maximum allowed (%2) : ").arg(pVerse->m_nNumWrd).arg(CRelIndex::maxWordCount()).toUtf8().data()
							  << pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0)).toUtf8().data() << "\n";
				}
			}
			if (pChapter != NULL) {
				if (nVerseWordAccum != pChapter->m_nNumWrd) {
					std::cerr << QString("\n*** Error: %1 Chapter Word Count (%2) doesn't match sum of Verse Word Counts (%3)!\n")
													.arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0)))
													.arg(pChapter->m_nNumWrd)
													.arg(nVerseWordAccum)
													.toUtf8().data();
				}
				nChapterWordAccum += pChapter->m_nNumWrd;

				if (pChapter->m_nNumVrs > CRelIndex::maxVerseCount()) {
					std::cerr << QString("\n*** Warning: Chapter verse count (%1) exceeds maximum allowed (%2) : ").arg(pChapter->m_nNumVrs).arg(CRelIndex::maxVerseCount()).toUtf8().data()
							  << pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0)).toUtf8().data() << "\n";
				}
			} else {
				nChapterWordAccum += nVerseWordAccum;		// Book has words of Colophons that aren't part of chapters proper
			}

			if (pChapter != NULL) {
				// BkChpNdx,NumVrs,NumWrd,BkAbbr,ChNdx
				fileChapters.write(QString("%1,%2,%3,%4,%5\r\n")
								   .arg(CRelIndex(0,0,nBk,nChp).index())		// 1
								   .arg(pChapter->m_nNumVrs)					// 2
								   .arg(pChapter->m_nNumWrd)					// 3
								   .arg(pBook->m_lstBkAbbr.at(0))				// 4 -- OSIS Abbr Only!
								   .arg(nChp)									// 5
								   .toUtf8());
			}

		}
		if (nChapterWordAccum != pBook->m_nNumWrd) {
			std::cerr << QString("\n*** Error: %1 Book Word Count (%2) doesn't match sum of Chapter Word Counts (%3)!\n")
												.arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0 ,0)))
												.arg(pBook->m_nNumWrd)
												.arg(nChapterWordAccum)
												.toUtf8().data();
		}
		if (pBook->m_nNumChp > CRelIndex::maxChapterCount()) {
			std::cerr << QString("\n*** Warning: Book chapter count (%1) exceeds maximum allowed (%2) : ").arg(pBook->m_nNumChp).arg(CRelIndex::maxChapterCount()).toUtf8().data()
					  << pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0, 0)).toUtf8().data() << "\n";
		}

		fileVerses.close();

		// BkNdx,TstBkNdx,TstNdx,BkName,BkAbbr,TblName,NumChp,NumVrs,NumWrd,Cat,Desc
		fileBooks.write(QString("%1,%2,%3,\"%4\",%5,%6,%7,%8,%9,\"%10\",\"%11\"\r\n")
						.arg(nBk)							// 1
						.arg(pBook->m_nTstBkNdx)			// 2
						.arg(pBook->m_nTstNdx)				// 3
						.arg(pBook->m_strBkName)			// 4
						.arg(pBook->m_lstBkAbbr.join(";"))	// 5
						.arg(pBook->m_strTblName)			// 6
						.arg(pBook->m_nNumChp)				// 7
						.arg(pBook->m_nNumVrs)				// 8
						.arg(pBook->m_nNumWrd)				// 9
						.arg(pBibleDatabase->bookCategoryEntry(pBook->m_nCatNdx)->m_strCategoryName)				// 10
						.arg(pBook->m_strDesc)				// 11
						.toUtf8());


		std::cerr << "\n";
	}
	if (pBibleDatabase->bibleEntry().m_nNumBk > CRelIndex::maxBookCount()) {
		std::cerr << QString("\n*** Warning: Total book count (%1) exceeds maximum allowed (%2)!\n").arg(pBibleDatabase->bibleEntry().m_nNumBk).arg(CRelIndex::maxBookCount()).toUtf8().data();
	}

	std::cerr << QFileInfo(fileChapters).fileName().toUtf8().data() << "\n";
	fileChapters.close();

	std::cerr << QFileInfo(fileBooks).fileName().toUtf8().data() << "\n";
	fileBooks.close();

	// ------------------------------------------------------------------------

	fileWords.setFileName(dirOutput.absoluteFilePath("WORDS.csv"));
	if (!fileWords.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Words Output File \"%1\"\n").arg(fileWords.fileName()).toUtf8().data();
		return -9;
	}
	std::cerr << QFileInfo(fileWords).fileName().toUtf8().data();

	fileWords.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileWords.write(QString("WrdNdx,Word,bIndexCasePreserve,NumTotal,AltWords,AltWordCounts,NormalMap\r\n").toUtf8());

	unsigned int nWordIndex = 0;

	// We've now built a list of word indexes and alternate forms, and now we
	//	need to take this list and convert it to the form of the database:
	TWordListMap &mapDbWordList = const_cast<TWordListMap &>(pBibleDatabase->mapWordList());
	for (TAltWordListMap::const_iterator itrUniqWrd = mapAltWordList.begin(); itrUniqWrd != mapAltWordList.end(); ++itrUniqWrd) {
		const TAltWordSet &setAltWords = itrUniqWrd->second;
		CWordEntry &wordEntryDb = mapDbWordList[itrUniqWrd->first];
		for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
			TWordListMap::const_iterator itrWrd = mapWordList.find(*itrAltWrd);
			if (itrWrd == mapWordList.end()) {
				std::cerr << QString("\n*** Error: %1 -> %2 -- Couldn't Find it (something bad happened!)\n").arg(itrUniqWrd->first).arg(*itrAltWrd).toUtf8().data();
				continue;
			}
			wordEntryDb.m_lstAltWords.push_back(*itrAltWrd);
			wordEntryDb.m_lstAltWordCount.push_back(itrWrd->second.m_ndxNormalizedMapping.size());
			wordEntryDb.m_ndxNormalizedMapping.insert(wordEntryDb.m_ndxNormalizedMapping.end(), itrWrd->second.m_ndxNormalizedMapping.begin(), itrWrd->second.m_ndxNormalizedMapping.end());
			wordEntryDb.m_strWord = WordFromWordSet(setAltWords);
		}
		wordEntryDb.m_bCasePreserve = isSpecialWord(nBDE, xmlHandler.language(), wordEntryDb);
		wordEntryDb.m_bIsProperWord = isProperWord(nBDE, xmlHandler.language(), wordEntryDb);

		assert(wordEntryDb.m_lstAltWords.size() == wordEntryDb.m_lstAltWordCount.size());
		assert(wordEntryDb.m_lstAltWords.size() > 0);

		if ((nWordIndex % 100) == 0) std::cerr << ".";

		// WrdNdx,Word,bIndexCasePreserve,NumTotal,AltWords,AltWordCounts,NormalMap

		nWordIndex++;
		int nSpecFlags = (wordEntryDb.m_bCasePreserve ? 1 :0) + (wordEntryDb.m_bIsProperWord ? 2 :0);		// Setup sepcial bit-flags field
		fileWords.write(QString("%1,\"%2\",%3,%4,").arg(nWordIndex).arg(wordEntryDb.m_strWord).arg(nSpecFlags).arg(wordEntryDb.m_ndxNormalizedMapping.size()).toUtf8());
		for (int i=0; i<wordEntryDb.m_lstAltWords.size(); ++i) {
			fileWords.write(QString((i == 0) ? "\"" : ",").toUtf8());
			fileWords.write(wordEntryDb.m_lstAltWords.at(i).toUtf8());
		}
		fileWords.write(QString("\",").toUtf8());
		for (int i=0; i<wordEntryDb.m_lstAltWordCount.size(); ++i) {
			fileWords.write(QString((i == 0) ? "\"" : ",").toUtf8());
			fileWords.write(QString("%1").arg(wordEntryDb.m_lstAltWordCount.at(i)).toUtf8());
		}
		fileWords.write(QString("\",").toUtf8());
		for (unsigned int i=0; i<wordEntryDb.m_ndxNormalizedMapping.size(); ++i) {
			fileWords.write(QString((i == 0) ? "\"" : ",").toUtf8());
			fileWords.write(QString("%1").arg(wordEntryDb.m_ndxNormalizedMapping.at(i)).toUtf8());
		}
		fileWords.write(QString("\"\r\n").toUtf8());
	}

	fileWords.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	fileWordSummary.setFileName(dirOutput.absoluteFilePath("WORDS_summary.csv"));
	if (!fileWordSummary.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Words Summary Output File \"%1\"\n").arg(fileWordSummary.fileName()).toUtf8().data();
		return -9;
	}
	std::cerr << QFileInfo(fileWordSummary).fileName().toUtf8().data();

	unsigned int nTotalWordCount = 0;
	unsigned int arrTotalTestamentWordCounts[NUM_TST];
	memset(arrTotalTestamentWordCounts, 0, sizeof(arrTotalTestamentWordCounts));
	unsigned int arrTotalBookWordCounts[NUM_BK];
	memset(arrTotalBookWordCounts, 0, sizeof(arrTotalBookWordCounts));

	bool bHaveApoc = (pBibleDatabase->bibleEntry().m_nNumBk > (NUM_BK_OT + NUM_BK_NT));
	unsigned int nRepTst = (bHaveApoc ? NUM_TST : (NUM_TST-1));
	unsigned int nRepBk = (bHaveApoc ? NUM_BK : (NUM_BK_OT + NUM_BK_NT));

	fileWordSummary.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileWordSummary.write(QString("\"Word\",\"AltWords\",\"Entire\nBible\"").toUtf8());
	for (unsigned int nTst=0; nTst<nRepTst; ++nTst) {
		QString strTemp = g_arrstrTstNames[nTst];
		strTemp.replace(' ', '\n');
		fileWordSummary.write(QString(",\"%1\"").arg(strTemp).toUtf8());
	}
	for (unsigned int nBk=0; nBk<nRepBk; ++nBk) {
		fileWordSummary.write(QString(",\"%1\"").arg(g_arrBooks[nBk].m_strName).toUtf8());
	}
	fileWordSummary.write(QString("\r\n").toUtf8());

	nWordIndex = 0;

// Use previously defined mapDbWordList:
//	TWordListMap &mapDbWordList = const_cast<TWordListMap &>(pBibleDatabase->mapWordList());
	for (TAltWordListMap::const_iterator itrUniqWrd = mapAltWordList.begin(); itrUniqWrd != mapAltWordList.end(); ++itrUniqWrd) {
		const TAltWordSet &setAltWords = itrUniqWrd->second;
		CWordEntry &wordEntryDb = mapDbWordList[itrUniqWrd->first];
		QString strAltWords;
		for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
			if (!strAltWords.isEmpty()) strAltWords += ",";
			strAltWords += *itrAltWrd;
		}

		fileWordSummary.write(QString("\"%1\",\"%2\",%3").arg(wordEntryDb.m_strWord).arg(strAltWords).arg(wordEntryDb.m_ndxNormalizedMapping.size()).toUtf8());

		assert(wordEntryDb.m_lstAltWords.size() == wordEntryDb.m_lstAltWordCount.size());
		assert(wordEntryDb.m_lstAltWords.size() > 0);

		if ((nWordIndex % 100) == 0) std::cerr << ".";
		nWordIndex++;

		unsigned int arrTestamentWordCounts[NUM_TST];
		memset(arrTestamentWordCounts, 0, sizeof(arrTestamentWordCounts));
		unsigned int arrBookWordCounts[NUM_BK];
		memset(arrBookWordCounts, 0, sizeof(arrBookWordCounts));

		for (TNormalizedIndexList::const_iterator itr = wordEntryDb.m_ndxNormalizedMapping.begin(); itr != wordEntryDb.m_ndxNormalizedMapping.end(); ++itr) {
			CRelIndex ndx(pBibleDatabase->DenormalizeIndex(*itr));
			assert(ndx.isSet());
			assert(ndx.book() != 0);
			if (ndx.book() <= NUM_BK_OT) {
				arrTestamentWordCounts[0]++;
				arrTotalTestamentWordCounts[0]++;
			} else if (ndx.book() <= (NUM_BK_OT + NUM_BK_NT)) {
				arrTestamentWordCounts[1]++;
				arrTotalTestamentWordCounts[1]++;
			} else if (ndx.book() <= (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC)) {
				arrTestamentWordCounts[2]++;
				arrTotalTestamentWordCounts[2]++;
			} else {
				// Word in unknown Testament -- assert here??
			}
			if (ndx.book() <= NUM_BK) {
				arrBookWordCounts[ndx.book()-1]++;
				arrTotalBookWordCounts[ndx.book()-1]++;
			} else {
				// Word in unknwon Book -- assert here??
			}
			nTotalWordCount++;
		}

		for (unsigned int nTst=0; nTst<nRepTst; ++nTst) {
			fileWordSummary.write(QString(",%1").arg(arrTestamentWordCounts[nTst]).toUtf8());
		}
		for (unsigned int nBk=0; nBk<nRepBk; ++nBk) {
			fileWordSummary.write(QString(",%1").arg(arrBookWordCounts[nBk]).toUtf8());
		}

		fileWordSummary.write(QString("\r\n").toUtf8());
	}
	fileWordSummary.write(QString("\"\",\"\",%1").arg(nTotalWordCount).toUtf8());
	for (unsigned int nTst=0; nTst<nRepTst; ++nTst) {
		fileWordSummary.write(QString(",%1").arg(arrTotalTestamentWordCounts[nTst]).toUtf8());
	}
	for (unsigned int nBk=0; nBk<nRepBk; ++nBk) {
		fileWordSummary.write(QString(",%1").arg(arrTotalBookWordCounts[nBk]).toUtf8());
	}
	fileWordSummary.write(QString("\r\n").toUtf8());

	fileWordSummary.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	if (strSegVariant.isEmpty() && xmlHandler.foundSegVariant()) {
		std::cerr << "\n"
					 "*** WARNING: Text contains seg variant text tags and no variant to parse was specified!\n"
					 "             Resulting database file will contain all variants run together!!\n\n";
	} else if (!strSegVariant.isEmpty() && !xmlHandler.foundSegVariant()) {
		std::cerr << "\n"
					 "*** WARNING: Specified seg variant wasn't found!  Resulting database file may be missing text!!\n\n";
	}

	// ------------------------------------------------------------------------

	unsigned int nFootnoteIndex = 0;

	fileFootnotes.setFileName(dirOutput.absoluteFilePath("FOOTNOTES.csv"));
	if (!fileFootnotes.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Footnotes Output File \"%1\"\n").arg(fileFootnotes.fileName()).toUtf8().data();
		return -9;
	}
	std::cerr << QFileInfo(fileFootnotes).fileName().toUtf8().data();

	fileFootnotes.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileFootnotes.write(QString("BkChpVrsWrdNdx,PFootnote,RFootnote\r\n").toUtf8());

	const TFootnoteEntryMap &mapFootnotes = pBibleDatabase->footnotesMap();
	for (TFootnoteEntryMap::const_iterator itrFootnotes = mapFootnotes.begin(); itrFootnotes != mapFootnotes.end(); ++itrFootnotes) {
		QStringList lstTempFootnote = (itrFootnotes->second).text().split('\"');
		QString strTempFootnote = lstTempFootnote.join("\"\"");
		// BkChpVrsWrdNdx,PFootnote,RFootnote
		fileFootnotes.write(QString("%1,\"%2\",\"%3\"\r\n")
							.arg((itrFootnotes->first).index())			// 1
							.arg(strTempFootnote)						// 2			-- TODO : FIX
							.arg(strTempFootnote)						// 3			-- TODO : FIX
							.toUtf8());

		if ((nFootnoteIndex % 100) == 0) std::cerr << ".";
		nFootnoteIndex++;
	}

	fileFootnotes.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	// Phrases are somewhat deprecated.  Write an empty PHRASES file so that the
	//	KJPBS build will succeed.  The person doing the build can always override
	//	it with a meaningful phrases file.
	filePhrases.setFileName(dirOutput.absoluteFilePath("PHRASES.csv"));
	if (!filePhrases.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Phrases Output File \"%1\"\n").arg(filePhrases.fileName()).toUtf8().data();
		return -10;
	}
	std::cerr << QFileInfo(filePhrases).fileName().toUtf8().data();

	filePhrases.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	filePhrases.write(QString("Ndx,Phrase,CaseSensitive,AccentSensitive,Exclude\r\n").toUtf8());

	filePhrases.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	// Write Lemmas:
	fileLemmas.setFileName(dirOutput.absoluteFilePath("LEMMAS.csv"));
	if (!fileLemmas.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Lemma Output File \"%1\"\n").arg(fileLemmas.fileName()).toUtf8().data();
		return -11;
	}
	std::cerr << QFileInfo(fileLemmas).fileName().toUtf8().data();

	fileLemmas.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileLemmas.write(QString("BkChpVrsWrdNdx,Count,Attrs\r\n").toUtf8());

	unsigned int nLemmaBk = 0;
	const TLemmaEntryMap &mapLemmas = pBibleDatabase->lemmaMap();
	for (TLemmaEntryMap::const_iterator itrLemmas = mapLemmas.cbegin();
			itrLemmas != mapLemmas.cend(); ++itrLemmas) {
		if (nLemmaBk != itrLemmas->second.tag().relIndex().book()) {
			nLemmaBk = itrLemmas->second.tag().relIndex().book();
			std::cerr << ".";
		}
		QStringList lstTempLemma = (itrLemmas->second).lemmaAttrs().split('\"');
		QString strTempLemma = lstTempLemma.join("\"\"");
		// Ndx,Count,Attrs
		fileLemmas.write(QString("%1,%2,\"%3\"\r\n")
							.arg(itrLemmas->second.tag().relIndex().index())
							.arg(itrLemmas->second.tag().count())
							.arg(strTempLemma)
							.toUtf8());
	}

	fileLemmas.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	// Write Strongs Database:
	fileStrongs.setFileName(dirOutput.absoluteFilePath("STRONGS.csv"));
	if (!fileStrongs.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Strongs Output File \"%1\"\n").arg(fileStrongs.fileName()).toUtf8().data();
		return -11;
	}
	std::cerr << QFileInfo(fileStrongs).fileName().toUtf8().data();

	fileStrongs.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM

	CCSVStream csvFileStrongs(&fileStrongs);
	csvFileStrongs << QStringList({ "StrongsMapNdx", "Orth", "Trans", "Pron", "Def" });

	int nStrongsProgress = 0;
	for (TStrongsIndexMap::const_iterator itrStrongs = pBibleDatabase->strongsIndexMap().cbegin();
			itrStrongs != pBibleDatabase->strongsIndexMap().cend();
			++itrStrongs) {
		csvFileStrongs << QStringList({ itrStrongs->second.strongsMapIndex(),
										itrStrongs->second.orthography(),
										itrStrongs->second.transliteration(),
										itrStrongs->second.pronunciation(),
										itrStrongs->second.definition() });
		if ((nStrongsProgress % 100) == 0) std::cerr << ".";
		++nStrongsProgress;
	}

	fileStrongs.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

#if CHECK_INDEXES
	std::cerr << "Checking Indexes";
	for (unsigned int nBk=1; nBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(nBk);
		assert(pBook != NULL);
		for (unsigned int nChp=(pBook->m_bHaveColophon ? 0 : 1); nChp<=pBook->m_nNumChp; ++nChp) {
			const CChapterEntry *pChapter = ((nChp != 0) ? pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0)) : NULL);
			unsigned int nStartVerse = 1;
			if (nChp != 0) {
				assert(pChapter != NULL);
				nStartVerse = (pChapter->m_bHaveSuperscription ? 0 : 1);
			} else {
				nStartVerse = 0;
			}

			for (unsigned int nVrs=nStartVerse; nVrs<= ((pChapter != NULL) ? pChapter->m_nNumVrs : 0); ++nVrs) {
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
				assert(pVerse != NULL);
				for (unsigned int nWrd=1; nWrd<=pVerse->m_nNumWrd; ++nWrd) {
					uint32_t nNormalAccum = pBibleDatabase->NormalizeIndex(CRelIndex(nBk, nChp, nVrs, nWrd));
					uint32_t nNormalNoAccum = pBibleDatabase->NormalizeIndexNoAccum(CRelIndex(nBk, nChp, nVrs, nWrd));
					if (nNormalAccum != nNormalNoAccum) {
						std::cerr << QString("\n*** Error: CRelIndex(%1, %2, %3, %4) : NormalAccum->%5 != NormalNoAccum->%6\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(nNormalAccum).arg(nNormalNoAccum).toUtf8().data();
						assert(nNormalAccum == nNormalNoAccum);
					}
					CRelIndex ndxDenormalAccum = pBibleDatabase->DenormalizeIndex(pVerse->m_nWrdAccum+nWrd);
					CRelIndex ndxDenormalNoAccum = pBibleDatabase->DenormalizeIndexNoAccum(pVerse->m_nWrdAccum+nWrd);
					if (ndxDenormalAccum != ndxDenormalNoAccum) {
						std::cerr << QString("\n*** Error: CRelIndex(%1, %2, %3, %4)->Accum:%5  DenormalAccum->%6 != DenormalNoAccum=%7\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(pVerse->m_nWrdAccum+nWrd).arg(ndxDenormalAccum.index()).arg(ndxDenormalNoAccum.index()).toUtf8().data();
						assert(ndxDenormalAccum == ndxDenormalNoAccum);
					}
					CRelIndex ndxTest = CRelIndex(nBk, nChp, nVrs, nWrd);
					uint32_t nNormal = pBibleDatabase->NormalizeIndex(ndxTest);
					CRelIndex ndxDenormal = pBibleDatabase->DenormalizeIndex(nNormal);
					if (ndxDenormal != ndxTest) {
						std::cerr << QString("\n*** Error: Roundtrip : CRelIndex(%1, %2, %3, %4)=%5 -> Normal=%6 -> Denormal=%7\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(ndxTest.index()).arg(nNormal).arg(ndxDenormal.index()).toUtf8().data();
						assert(ndxDenormal == ndxTest);
					}
				}
			}
		}
		std::cerr << ".";
	}
	std::cerr << "\n";
#endif

	// ------------------------------------------------------------------------

/*
	std::cout << QString("Bible:  Testaments: %1  Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->bibleEntry().m_nNumTst)
						.arg(pBibleDatabase->bibleEntry().m_nNumBk)
						.arg(pBibleDatabase->bibleEntry().m_nNumChp)
						.arg(pBibleDatabase->bibleEntry().m_nNumVrs)
						.arg(pBibleDatabase->bibleEntry().m_nNumWrd)
						.toUtf8().data();

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumTst; ++i) {
		std::cout << QString("%1 : Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->testamentEntry(i)->m_strTstName)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumBk)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumWrd)
						.toUtf8().data();
	}

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumBk; ++i) {
		std::cout << QString("%1 : Chapters: %2  Verses: %3  Words: %4\n")
						.arg(pBibleDatabase->bookEntry(i)->m_strBkName)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumWrd)
						.toUtf8().data();
	}

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumBk; ++i) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(i);
		assert(pBook != NULL);
		for (unsigned int j=1; j<=pBook->m_nNumChp; ++j) {
			std::cout << QString("%1 Chapter %2 : Verses: %3  Words: %4\n")
						.arg(pBook->m_strBkName)
						.arg(j)
						.arg(pBibleDatabase->chapterEntry(CRelIndex(i, j, 0, 0))->m_nNumVrs)
						.arg(pBibleDatabase->chapterEntry(CRelIndex(i, j, 0, 0))->m_nNumWrd)
						.toUtf8().data();
		}
	}
*/

	// ------------------------------------------------------------------------


/*
	std::cout << "\n============================ Element Names  =================================\n";
	QStringList lstElements = xmlHandler.elementNames();
	lstElements.sort();
	lstElements.removeDuplicates();
	for (int i = 0; i < lstElements.count(); ++i) {
		std::cout << lstElements.at(i).toUtf8().data() << "\n";
	}

	std::cout << "\n\n============================ Attribute Names  =================================\n";
	QStringList lstAttrib = xmlHandler.attrNames();
	lstAttrib.sort();
	lstAttrib.removeDuplicates();
	for (int i = 0; i < lstAttrib.count(); ++i) {
		std::cout << lstAttrib.at(i).toUtf8().data() << "\n";
	}

*/

	// ------------------------------------------------------------------------

	QString strParsedUTF8 = xmlHandler.parsedUTF8Chars();
	std::cerr << "UTF8 Characters Parsed: \"" << strParsedUTF8.toUtf8().data() << "\"\n";
	for (int i = 0; i<strParsedUTF8.size(); ++i) {
		std::cerr << "    \"" << QString(strParsedUTF8.at(i)).toUtf8().data() << "\" (" << QString("%1").arg(strParsedUTF8.at(i).unicode(), 4, 16, QChar('0')).toUtf8().data() << ")\n";
	}

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}

