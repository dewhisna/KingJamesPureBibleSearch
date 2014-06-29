/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <iostream>
#include <string>
#include <QFile>

#include <QTextDocument>                        // Needed for Qt::escape, which is in this header, not <Qt> as is assistant says

#include <assert.h>


#define NUM_BK 66u
#define NUM_BK_OT 39u
#define NUM_BK_NT 27u
#define NUM_TST 2u

typedef QList<QStringList> TChapterVerseCounts;

const QString g_arrChapterVerseCounts[NUM_BK] =
{
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
	"20,29,22,11,14,17,17,13,21,11,19,17,18,20,8,21,18,24,21,15,27,21"
};

typedef struct {
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
		{ QObject::tr("Genesis", "bookname"), QObject::tr("Gen;Gn", "bookabbr"), "Gen", "GEN", QObject::tr("Law", "bookcategory"), QObject::tr("The First Book of Moses", "bookdesc") },
		{ QObject::tr("Exodus", "bookname"), QObject::tr("Exod;Exo;Ex", "bookabbr"), "Exod", "EXOD", QObject::tr("Law", "bookcategory"), QObject::tr("The Second Book of Moses", "bookdesc") },
		{ QObject::tr("Leviticus", "bookname"), QObject::tr("Lev;Lv", "bookabbr"), "Lev", "LEV", QObject::tr("Law", "bookcategory"), QObject::tr("The Third Book of Moses", "bookdesc") },
		{ QObject::tr("Numbers", "bookname"), QObject::tr("Num;Nm", "bookabbr"), "Num", "NUM", QObject::tr("Law", "bookcategory"), QObject::tr("The Fourth Book of Moses", "bookdesc") },
		{ QObject::tr("Deuteronomy", "bookname"), QObject::tr("Deut;Deu;Dt", "bookabbr"), "Deut", "DEUT", QObject::tr("Law", "bookcategory"), QObject::tr("The Fifth Book of Moses", "bookdesc") },
		{ QObject::tr("Joshua", "bookname"), QObject::tr("Josh;Jos;Jo", "bookabbr"), "Josh", "JOSH", QObject::tr("OT Narative", "bookcategory"), "" },
		{ QObject::tr("Judges", "bookname"), QObject::tr("Judg;Jdg;Jgs", "bookabbr"), "Judg", "JUDG", QObject::tr("OT Narative", "bookcategory"), "" },
		{ QObject::tr("Ruth", "bookname"), QObject::tr("Ruth;Rut;Ru", "bookabbr"), "Ruth", "RUTH", QObject::tr("OT Narative", "bookcategory"), "" },
		{ QObject::tr("1 Samuel", "bookname"), QObject::tr("1Sam;1Sm", "bookabbr"), "1Sam", "SAM1", QObject::tr("OT Narative", "bookcategory"), QObject::tr("The First Book of Samuel Otherwise Called, The First Book of the Kings", "bookdesc") },
		{ QObject::tr("2 Samuel", "bookname"), QObject::tr("2Sam;2Sm", "bookabbr"), "2Sam", "SAM2", QObject::tr("OT Narative", "bookcategory"), QObject::tr("The Second Book of Samuel Otherwise Called, The Second Book of the Kings", "bookdesc") },
		{ QObject::tr("1 Kings", "bookname"), QObject::tr("1Kgs", "bookabbr"), "1Kgs", "KGS1", QObject::tr("OT Narative", "bookcategory"), QObject::tr("The First Book of the Kings Commonly Called, The Third Book of the Kings", "bookdesc") },
		{ QObject::tr("2 Kings", "bookname"), QObject::tr("2Kgs", "bookabbr"), "2Kgs", "KGS2", QObject::tr("OT Narative", "bookcategory"), QObject::tr("The Second Book of the Kings Commonly Called, The Fourth Book of the Kings", "bookdesc") },
		{ QObject::tr("1 Chronicles", "bookname"), QObject::tr("1Chr;1Chron;1Ch", "bookabbr"), "1Chr", "CHR1", QObject::tr("OT Narative", "bookcategory"), QObject::tr("The First Book of the Chronicles", "bookdesc") },
		{ QObject::tr("2 Chronicles", "bookname"), QObject::tr("2Chr;2Chron;2Ch", "bookabbr"), "2Chr", "CHR2", QObject::tr("OT Narative", "bookcategory"), QObject::tr("The Second Book of the Chronicles", "bookdesc") },
		{ QObject::tr("Ezra", "bookname"), QObject::tr("Ezra;Ezr", "bookabbr"), "Ezra", "EZRA", QObject::tr("OT Narative", "bookcategory"), "" },
		{ QObject::tr("Nehemiah", "bookname"), QObject::tr("Neh", "bookabbr"), "Neh", "NEH", QObject::tr("OT Narative", "bookcategory"), "" },
		{ QObject::tr("Esther", "bookname"), QObject::tr("Est;Esth", "bookabbr"), "Esth", "ESTH", QObject::tr("OT Narative", "bookcategory"), "" },
		{ QObject::tr("Job", "bookname"), QObject::tr("Job;Jb", "bookabbr"), "Job", "JOB", QObject::tr("Wisdom", "bookcategory"), "" },
		{ QObject::tr("Psalms", "bookname"), QObject::tr("Ps;Pss", "bookabbr"), "Ps", "PS", QObject::tr("Wisdom", "bookcategory"), "" },
		{ QObject::tr("Proverbs", "bookname"), QObject::tr("Prov;Prv;Pv", "bookabbr"), "Prov", "PROV", QObject::tr("Wisdom", "bookcategory"), "" },
		{ QObject::tr("Ecclesiastes", "bookname"), QObject::tr("Eccl;Eccles", "bookabbr"), "Eccl", "ECCL", QObject::tr("Wisdom", "bookcategory"), QObject::tr("Ecclesiastes; Or, The Preacher", "bookdesc") },
		{ QObject::tr("Song Of Solomon", "bookname"), QObject::tr("Song;Sg", "bookabbr"), "Song", "SONG", QObject::tr("Wisdom", "bookcategory"), "" },
		{ QObject::tr("Isaiah", "bookname"), QObject::tr("Isa;Is", "bookabbr"), "Isa", "ISA", QObject::tr("Major Prophets", "bookcategory"), QObject::tr("The Book of the Prophet Isaiah", "bookdesc") },
		{ QObject::tr("Jeremiah", "bookname"), QObject::tr("Jer", "bookabbr"), "Jer", "JER", QObject::tr("Major Prophets", "bookcategory"), QObject::tr("The Book of the Prophet Jeremiah", "bookdesc") },
		{ QObject::tr("Lamentations", "bookname"), QObject::tr("Lam", "bookabbr"), "Lam", "LAM", QObject::tr("Major Prophets", "bookcategory"), QObject::tr("The Lamentations of Jeremiah", "bookdesc") },
		{ QObject::tr("Ezekiel", "bookname"), QObject::tr("Ezek;Eze;Ez", "bookabbr"), "Ezek", "EZEK", QObject::tr("Major Prophets", "bookcategory"), QObject::tr("The Book of the Prophet Ezekiel", "bookdesc") },
		{ QObject::tr("Daniel", "bookname"), QObject::tr("Dan;Dn", "bookabbr"), "Dan", "DAN", QObject::tr("Major Prophets", "bookcategory"), QObject::tr("The Book of <i>the Prophet</i> Daniel", "bookdesc") },
		{ QObject::tr("Hosea", "bookname"), QObject::tr("Hos", "bookabbr"), "Hos", "HOS", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Joel", "bookname"), QObject::tr("Joel;Joe;Jl", "bookabbr"), "Joel", "JOEL", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Amos", "bookname"), QObject::tr("Amos;Amo;Am", "bookabbr"), "Amos", "AMOS", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Obadiah", "bookname"), QObject::tr("Obad;Oba;Ob", "bookabbr"), "Obad", "OBAD", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Jonah", "bookname"), QObject::tr("Jonah;Jona;Jon", "bookabbr"), "Jonah", "JONAH", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Micah", "bookname"), QObject::tr("Mic;Mi", "bookabbr"), "Mic", "MIC", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Nahum", "bookname"), QObject::tr("Nah;Na", "bookabbr"), "Nah", "NAH", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Habakkuk", "bookname"), QObject::tr("Hab;Hb", "bookabbr"), "Hab", "HAB", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Zephaniah", "bookname"), QObject::tr("Zeph;Zep", "bookabbr"), "Zeph", "ZEPH", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Haggai", "bookname"), QObject::tr("Hag;Hg", "bookabbr"), "Hag", "HAG", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Zechariah", "bookname"), QObject::tr("Zech;Zec", "bookabbr"), "Zech", "ZECH", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Malachi", "bookname"), QObject::tr("Mal", "bookabbr"), "Mal", "MAL", QObject::tr("Minor Prophets", "bookcategory"), "" },
		{ QObject::tr("Matthew", "bookname"), QObject::tr("Matt;Mt", "bookabbr"), "Matt", "MATT", QObject::tr("NT Narative", "bookcategory"), QObject::tr("The Gospel According to Saint Matthew", "bookdesc") },
		{ QObject::tr("Mark", "bookname"), QObject::tr("Mark;Mk", "bookabbr"), "Mark", "MARK", QObject::tr("NT Narative", "bookcategory"), QObject::tr("The Gospel According to Saint Mark", "bookdesc") },
		{ QObject::tr("Luke", "bookname"), QObject::tr("Luke;Lk", "bookabbr"), "Luke", "LUKE", QObject::tr("NT Narative", "bookcategory"), QObject::tr("The Gospel According to Saint Luke", "bookdesc") },
		{ QObject::tr("John", "bookname"), QObject::tr("John;Jhn;Jn", "bookabbr"), "John", "JOHN", QObject::tr("NT Narative", "bookcategory"), QObject::tr("The Gospel According to Saint John", "bookdesc") },
		{ QObject::tr("Acts", "bookname"), QObject::tr("Acts", "bookabbr"), "Acts", "ACTS", QObject::tr("NT Narative", "bookcategory"), QObject::tr("The Acts of the Apostles", "bookdesc") },
		{ QObject::tr("Romans", "bookname"), QObject::tr("Rom", "bookabbr"), "Rom", "ROM", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul the Apostle to the Romans", "bookdesc") },
		{ QObject::tr("1 Corinthians", "bookname"), QObject::tr("1Cor", "bookabbr"), "1Cor", "COR1", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The First Epistle of Paul the Apostle to the Corinthians", "bookdesc") },
		{ QObject::tr("2 Corinthians", "bookname"), QObject::tr("2Cor", "bookabbr"), "2Cor", "COR2", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Second Epistle of Paul the Apostle to the Corinthians", "bookdesc") },
		{ QObject::tr("Galatians", "bookname"), QObject::tr("Gal", "bookabbr"), "Gal", "GAL", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul the Apostle to the Galatians", "bookdesc") },
		{ QObject::tr("Ephesians", "bookname"), QObject::tr("Eph", "bookabbr"), "Eph", "EPH", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul the Apostle to the Ephesians", "bookdesc") },
		{ QObject::tr("Philippians", "bookname"), QObject::tr("Phil", "bookabbr"), "Phil", "PHIL", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul the Apostle to the Philippians", "bookdesc") },
		{ QObject::tr("Colossians", "bookname"), QObject::tr("Col", "bookabbr"), "Col", "COL", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul the Apostle to the Colossians", "bookdesc") },
		{ QObject::tr("1 Thessalonians", "bookname"), QObject::tr("1Thess;1Thes;1Th", "bookabbr"), "1Thess", "THESS1", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The First Epistle of Paul the Apostle to the Thessalonians", "bookdesc") },
		{ QObject::tr("2 Thessalonians", "bookname"), QObject::tr("2Thess;2Thes;2Th", "bookabbr"), "2Thess", "THESS2", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Second Epistle of Paul the Apostle to the Thessalonains", "bookdesc") },
		{ QObject::tr("1 Timothy", "bookname"), QObject::tr("1Tim;1Tm", "bookabbr"), "1Tim", "TIM1", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The First Epistle of Paul the Apostle to Timothy", "bookdesc") },
		{ QObject::tr("2 Timothy", "bookname"), QObject::tr("2Tim;2Tm", "bookabbr"), "2Tim", "TIM2", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Second Epistle of Paul the Apostle to Timothy", "bookdesc") },
		{ QObject::tr("Titus", "bookname"), QObject::tr("Titus;Ti", "bookabbr"), "Titus", "TITUS", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul to Titus", "bookdesc") },
		{ QObject::tr("Philemon", "bookname"), QObject::tr("Phlm;Philem", "bookabbr"), "Phlm", "PHLM", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul to Philemon", "bookdesc") },
		{ QObject::tr("Hebrews", "bookname"), QObject::tr("Heb", "bookabbr"), "Heb", "HEB", QObject::tr("Pauline Epistles", "bookcategory"), QObject::tr("The Epistle of Paul the Apostle to the Hebrews", "bookdesc") },
		{ QObject::tr("James", "bookname"), QObject::tr("Jas", "bookabbr"), "Jas", "JAS", QObject::tr("General Epistles", "bookcategory"), QObject::tr("The General Epistle of James", "bookdesc") },
		{ QObject::tr("1 Peter", "bookname"), QObject::tr("1Pet;1Pt", "bookabbr"), "1Pet", "PET1", QObject::tr("General Epistles", "bookcategory"), QObject::tr("The First General Epistle of Peter", "bookdesc") },
		{ QObject::tr("2 Peter", "bookname"), QObject::tr("2Pet;2Pt", "bookabbr"), "2Pet", "PET2", QObject::tr("General Epistles", "bookcategory"), QObject::tr("The Second General Epistle of Peter", "bookdesc") },
		{ QObject::tr("1 John", "bookname"), QObject::tr("1John;1Jn", "bookabbr"), "1John", "JOHN1", QObject::tr("General Epistles", "bookcategory"), QObject::tr("The First General Epistle of John", "bookdesc") },
		{ QObject::tr("2 John", "bookname"), QObject::tr("2John;2Jn", "bookabbr"), "2John", "JOHN2", QObject::tr("General Epistles", "bookcategory"), QObject::tr("The Second General Epistle of John", "bookdesc") },
		{ QObject::tr("3 John", "bookname"), QObject::tr("3John;3Jn", "bookabbr"), "3John", "JOHN3", QObject::tr("General Epistles", "bookcategory"), QObject::tr("The Third General Epistle of John", "bookdesc") },
		{ QObject::tr("Jude", "bookname"), QObject::tr("Jude", "bookabbr"), "Jude", "JUDE", QObject::tr("General Epistles", "bookcategory"), QObject::tr("The General Epistle of Jude", "bookdesc") },
		{ QObject::tr("Revelation", "bookname"), QObject::tr("Rev;Rv;Apoc", "bookabbr"), "Rev", "REV", QObject::tr("Apocalyptic Epistle", "bookcategory"), QObject::tr("The Revelation of Jesus Christ", "bookdesc") }
	};

	for (unsigned int i=0; i<NUM_BK; ++i) {
		g_arrBooks[i] = arrBooks[i];
	}
}

QString convertVerseText(const QString &strLine)
{
	QString strNewLine = Qt::escape(strLine);

	strNewLine.replace(QString("["), QString("<transChange type=\"added\">"));
	strNewLine.replace(QString("]"), QString("</transChange>"));
	strNewLine.replace(QString::fromUtf8("¶"), QString::fromUtf8("<milestone type=\"x-p\" marker=\"¶\"/>"));
	strNewLine.replace(QString("> "), QString(">"));	// Pilcrow-Spaces above will convert to "> " and since ">" has been escaped to "&gt;", this will remove spaces following pilcrows

	return strNewLine;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	g_setBooks();

	if (argc != 3) {
		std::cerr << QString("Usage: %1 <in-file> <out-file>\n").arg(argv[0]).toStdString();
		return -1;
	}

	QFile fileIn(argv[1]);
	QFile fileOut(argv[2]);


	if (!fileIn.open(QIODevice::ReadOnly)) {
		std::cerr << QString("Unable to open \"%1\" for reading\n").arg(fileIn.fileName()).toUtf8().data();
		return -2;
	}

	if (!fileOut.open(QIODevice::WriteOnly)) {
		std::cerr << QString("Unable to open \"%1\" for writing\n").arg(fileOut.fileName()).toUtf8().data();
		return -3;
	}

	fileOut.write(QString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n").toUtf8());
	fileOut.write(QString("<osis xmlns=\"http://www.bibletechnologies.net/2003/OSIS/namespace\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.bibletechnologies.net/2003/OSIS/namespace http://www.bibletechnologies.net/osisCore.2.1.1.xsd\">\n\n").toUtf8());
	fileOut.write(QString("<osisText osisIDWork=\"RV1865\" osisRefWork=\"defaultReferenceScheme\" lang=\"es\">\n\n").toUtf8());
	fileOut.write(QString("<header>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"RV1865\">\n").toUtf8());
	fileOut.write(QString("<title>Reina-Valera 1865 para Jeff McArdle, La Sociedad Biblica Valera: 22 de Junio de 2014.</title>\n").toUtf8());
	fileOut.write(QString("<identifier type=\"OSIS\">Bible.RV1865</identifier>\n").toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"defaultReferenceScheme\">\n").toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("</header>\n").toUtf8());

	fileOut.write("<div type=\"x-testament\">\n");

	QString strLine;

	strLine = QString::fromUtf8(fileIn.readLine()).trimmed();			// Book Subtitle

	for (unsigned int nBk=0; nBk<NUM_BK; ++nBk) {
		QString strOsisAbbr = g_arrBooks[nBk].m_strOsisAbbr;
		QStringList lstCounts = g_arrChapterVerseCounts[nBk].split(",");

		if (fileIn.atEnd()) {
			std::cerr << "Unexpected end of file!!\n";
			break;
		}

		// Note: Have already read Subtitle either before loop or at bottom of loop (needed for colophon processing)

		fileOut.write(QString("<div type=\"book\" osisID=\"%1\" canonical=\"true\">\n").arg(strOsisAbbr).toUtf8().data());

		if (!strLine.isEmpty()) {
			fileOut.write(QString("<title type=\"main\">%1</title>\n").arg(convertVerseText(strLine)).toUtf8().data());
		}
		strLine = QString::fromUtf8(fileIn.readLine()).trimmed();			// Book Title

		for (int nChp = 0; nChp < lstCounts.size(); ++nChp) {
			if (fileIn.atEnd()) {
				std::cerr << "Unexpected end of file!!\n";
				break;
			}
			strLine = QString::fromUtf8(fileIn.readLine()).trimmed();		// Blank line separates each chapter


			fileOut.write(QString("<chapter osisID=\"%1.%2\">\n").arg(strOsisAbbr).arg(nChp+1).toUtf8().data());

			int nVrsCount = lstCounts.at(nChp).toInt();
			for (int nVrs = 0; nVrs < nVrsCount; ++nVrs) {
				if (fileIn.atEnd()) {
					std::cerr << "Unexpected end of file!!\n";
					break;
				}
				strLine = QString::fromUtf8(fileIn.readLine()).trimmed();

				int nSpaceIndex = strLine.indexOf(' ');
				int nDocVerse = ((nSpaceIndex != -1) ? strLine.left(nSpaceIndex).toInt() : 0);
				if (nDocVerse != 0) {
					strLine = strLine.mid(nSpaceIndex+1);		// note: with the +1, will be whole line if there was no space
				}

				if ((nDocVerse == 0) &&
					(nBk+1 == 19) &&
					(strLine.left(1).compare(QString::fromUtf8("«")) == 0) &&
					(strLine.right(1).compare(QString::fromUtf8("»")) == 0)) {
					fileOut.write("<title type=\"psalm\" canonical=\"true\">");
					fileOut.write(convertVerseText(strLine.mid(1, strLine.length()-2)).toUtf8().data());
					fileOut.write("</title>\n");
					--nVrs;
				} else {
					fileOut.write(QString("<verse osisID=\"%1.%2.%3\" sID=\"%1.%2.%3\"/>").arg(strOsisAbbr).arg(nChp+1).arg(nVrs+1).toUtf8().data());
//					fileOut.write(QString("<verse osisID=\"%1.%2.%3\">").arg(strOsisAbbr).arg(nChp+1).arg(nVrs+1).toUtf8().data());
					fileOut.write(convertVerseText(strLine).toUtf8().data());
					fileOut.write(QString("<verse eID=\"%1.%2.%3\"/>\n").arg(strOsisAbbr).arg(nChp+1).arg(nVrs+1).toUtf8().data());
//					fileOut.write(QString("</verse>\n").toUtf8().data());
					if (nDocVerse != nVrs+1) {
						std::cerr << QString("Error : Expected %1 %2:%3, found %4 %5").arg(strOsisAbbr).arg(nChp+1).arg(nVrs+1).arg(nDocVerse).arg(strLine).toUtf8().data() << std::endl;
					}
				}
			}

			fileOut.write("</chapter>\n");
		}

		strLine = QString::fromUtf8(fileIn.readLine()).trimmed();				// Book Subtitle
		if ((strLine.left(1).compare(QString::fromUtf8("«")) == 0) &&
			(strLine.right(1).compare(QString::fromUtf8("»")) == 0)) {
			fileOut.write(QString("<div type=\"colophon\" osisID=\"%1.c\">").arg(strOsisAbbr).toUtf8().data());
			fileOut.write(convertVerseText(strLine.mid(1, strLine.length()-2)).toUtf8().data());
			fileOut.write("</div>\n");
			strLine = QString::fromUtf8(fileIn.readLine()).trimmed();			// Book Subtitle
		}

		fileOut.write("</div>\n");

		if (nBk == NUM_BK_OT-1) {
			fileOut.write("</div>\n");
			fileOut.write("<div type=\"x-testament\">\n");
		}
	}

	fileOut.write("</div>\n");

	fileOut.write("</osisText>\n");
	fileOut.write("</osis>\n");

	fileOut.close();
	fileIn.close();

//	return a.exec();
	return 0;
}

