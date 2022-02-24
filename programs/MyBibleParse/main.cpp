/****************************************************************************
**
** Copyright (C) 2013-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include <iostream>

#include <QObject>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QtSql>
#include <QSqlQuery>

#include "../KJVCanOpener/dbDescriptors.h"

// ============================================================================

static inline QString htmlEscape(const QString &aString)
{
	return aString.toHtmlEscaped();
}

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

// ============================================================================

const unsigned int VERSION = 10000;		// Version 1.0.0

// TODO : Update this for apocrypha:
#define NUM_BK 66u				// Total Books Defined
//#define NUM_BK 80u				// Total Books Defined
#define NUM_BK_OT 39u			// Total Books in Old Testament
#define NUM_BK_NT 27u			// Total Books in New Testament
#define NUM_BK_APOC 14u			// Total Books in Apocrypha (KJVA)
#define NUM_TST 3u				// Total Number of Testaments (or pseudo-testaments, in the case of Apocrypha)

typedef struct {
	const QString m_strName;
	const QString m_strOsisAbbr;
	const QString m_strBookName;
	const int m_nBookNumber;			// In MyBible App Database number system
	const QString m_strCategory;
	const QString m_strDescription;
} TBook;

// ============================================================================

TBook g_arrBooks[NUM_BK] =				// NOTE: Reworked to match MyBible App Format and Naming
{
	// ---- Begin Old Testament:
	{ QObject::tr("Genesis"), "Gen", "Gen", 10, QObject::tr("Law"), QObject::tr("The First Book of Moses") },
	{ QObject::tr("Exodus"), "Exod", "Exo", 20, QObject::tr("Law"), QObject::tr("The Second Book of Moses") },
	{ QObject::tr("Leviticus"), "Lev", "Lev", 30, QObject::tr("Law"), QObject::tr("The Third Book of Moses") },
	{ QObject::tr("Numbers"), "Num", "Num", 40, QObject::tr("Law"), QObject::tr("The Fourth Book of Moses") },
	{ QObject::tr("Deuteronomy"), "Deut", "Deu", 50, QObject::tr("Law"), QObject::tr("The Fifth Book of Moses") },
	{ QObject::tr("Joshua"), "Josh", "Josh", 60, QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Judges"), "Judg", "Judg", 70, QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Ruth"), "Ruth", "Ruth", 80, QObject::tr("OT Narrative"), "" },
	{ QObject::tr("1 Samuel"), "1Sam", "1Sam", 90, QObject::tr("OT Narrative"), QObject::tr("The First Book of Samuel Otherwise Called, The First Book of the Kings") },
	{ QObject::tr("2 Samuel"), "2Sam", "2Sam", 100, QObject::tr("OT Narrative"), QObject::tr("The Second Book of Samuel Otherwise Called, The Second Book of the Kings") },
	{ QObject::tr("1 Kings"), "1Kgs", "1Kgs", 110, QObject::tr("OT Narrative"), QObject::tr("The First Book of the Kings Commonly Called, The Third Book of the Kings") },
	{ QObject::tr("2 Kings"), "2Kgs", "2Kgs", 120, QObject::tr("OT Narrative"), QObject::tr("The Second Book of the Kings Commonly Called, The Fourth Book of the Kings") },
	{ QObject::tr("1 Chronicles"), "1Chr", "1Chr", 130, QObject::tr("OT Narrative"), QObject::tr("The First Book of the Chronicles") },
	{ QObject::tr("2 Chronicles"), "2Chr", "2Chr", 140, QObject::tr("OT Narrative"), QObject::tr("The Second Book of the Chronicles") },
	{ QObject::tr("Ezra"), "Ezra", "Ezr", 150, QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Nehemiah"), "Neh", "Neh", 160, QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Esther"), "Esth", "Esth", 190, QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Job"), "Job", "Job", 220, QObject::tr("Wisdom"), "" },
	{ QObject::tr("Psalms"), "Ps", "Ps", 230, QObject::tr("Wisdom"), "" },
	{ QObject::tr("Proverbs"), "Prov", "Prov", 240, QObject::tr("Wisdom"), "" },
	{ QObject::tr("Ecclesiastes"), "Eccl", "Eccl", 250, QObject::tr("Wisdom"), QObject::tr("Ecclesiastes; Or, The Preacher") },
	{ QObject::tr("Song Of Solomon"), "Song", "Song", 260, QObject::tr("Wisdom"), "" },
	{ QObject::tr("Isaiah"), "Isa", "Isa", 290, QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Isaiah") },
	{ QObject::tr("Jeremiah"), "Jer", "Jer", 300, QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Jeremiah") },
	{ QObject::tr("Lamentations"), "Lam", "Lam", 310, QObject::tr("Major Prophets"), QObject::tr("The Lamentations of Jeremiah") },
	{ QObject::tr("Ezekiel"), "Ezek", "Ezek", 330, QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Ezekiel") },
	{ QObject::tr("Daniel"), "Dan", "Dan", 340, QObject::tr("Major Prophets"), QObject::tr("The Book of <i>the Prophet</i> Daniel") },
	{ QObject::tr("Hosea"), "Hos", "Hos", 350, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Joel"), "Joel", "Joel", 360, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Amos"), "Amos", "Am", 370, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Obadiah"), "Obad", "Oba", 380, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Jonah"), "Jonah", "Jona", 390, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Micah"), "Mic", "Mic", 400, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Nahum"), "Nah", "Nah", 410, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Habakkuk"), "Hab", "Hab", 420, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Zephaniah"), "Zeph", "Zeph", 430, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Haggai"), "Hag", "Hag", 440, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Zechariah"), "Zech", "Zech", 450, QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Malachi"), "Mal", "Mal", 460, QObject::tr("Minor Prophets"), "" },
	// ---- Begin New Testament:
	{ QObject::tr("Matthew"), "Matt", "Mat", 470, QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint Matthew") },
	{ QObject::tr("Mark"), "Mark", "Mar", 480, QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint Mark") },
	{ QObject::tr("Luke"), "Luke", "Luk", 490, QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint Luke") },
	{ QObject::tr("John"), "John", "John", 500, QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint John") },
	{ QObject::tr("Acts"), "Acts", "Acts", 510, QObject::tr("NT Narrative"), QObject::tr("The Acts of the Apostles") },
	{ QObject::tr("Romans"), "Rom", "Rom", 520, QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Romans") },
	{ QObject::tr("1 Corinthians"), "1Cor", "1Cor", 530, QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to the Corinthians") },
	{ QObject::tr("2 Corinthians"), "2Cor", "2Cor", 540, QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to the Corinthians") },
	{ QObject::tr("Galatians"), "Gal", "Gal", 550, QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Galatians") },
	{ QObject::tr("Ephesians"), "Eph", "Eph", 560, QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Ephesians") },
	{ QObject::tr("Philippians"), "Phil", "Phil", 570, QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Philippians") },
	{ QObject::tr("Colossians"), "Col", "Col", 580, QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Colossians") },
	{ QObject::tr("1 Thessalonians"), "1Thess", "1Ths", 590, QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to the Thessalonians") },
	{ QObject::tr("2 Thessalonians"), "2Thess", "2Ths", 600, QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to the Thessalonains") },
	{ QObject::tr("1 Timothy"), "1Tim", "1Tim", 610, QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to Timothy") },
	{ QObject::tr("2 Timothy"), "2Tim", "2Tim", 620, QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to Timothy") },
	{ QObject::tr("Titus"), "Titus", "Tit", 630, QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul to Titus") },
	{ QObject::tr("Philemon"), "Phlm", "Phlm", 640, QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul to Philemon") },
	{ QObject::tr("Hebrews"), "Heb", "Heb", 650, QObject::tr("General Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Hebrews") },
	{ QObject::tr("James"), "Jas", "Jam", 660, QObject::tr("General Epistles"), QObject::tr("The General Epistle of James") },
	{ QObject::tr("1 Peter"), "1Pet", "1Pet", 670, QObject::tr("General Epistles"), QObject::tr("The First General Epistle of Peter") },
	{ QObject::tr("2 Peter"), "2Pet", "2Pet", 680, QObject::tr("General Epistles"), QObject::tr("The Second General Epistle of Peter") },
	{ QObject::tr("1 John"), "1John", "1Jn", 690, QObject::tr("General Epistles"), QObject::tr("The First General Epistle of John") },
	{ QObject::tr("2 John"), "2John", "2Jn", 700, QObject::tr("General Epistles"), QObject::tr("The Second General Epistle of John") },
	{ QObject::tr("3 John"), "3John", "3Jn", 710, QObject::tr("General Epistles"), QObject::tr("The Third General Epistle of John") },
	{ QObject::tr("Jude"), "Jude", "Jud", 720, QObject::tr("General Epistles"), QObject::tr("The General Epistle of Jude") },
	{ QObject::tr("Revelation"), "Rev", "Rev", 730, QObject::tr("Apocalyptic Epistle"), QObject::tr("The Revelation of Jesus Christ") },
	// ---- Begin Apocrypha/Deuterocanon:
// TODO : Rework this for the Apocrypha once we've remapped their implementation to ours:
//	{ QObject::tr("1 Esdras"), "1Esd", "1Esd", 165, QObject::tr("Apocrypha"), QObject::tr("The First Book of Esdras") },
//	{ QObject::tr("2 Esdras"), "2Esd", "2Esd", 468, QObject::tr("Apocrypha"), QObject::tr("The Second Book of Esdras") },
//	{ QObject::tr("Tobit"), "Tob", "Tob", 170, QObject::tr("Apocrypha"), QObject::tr("The Book of Tobit") },
//	{ QObject::tr("Judith"), "Jdt", "Jdt", 180, QObject::tr("Apocrypha"), QObject::tr("The Book of Judith") },
//	{ QObject::tr("Additions to Esther"), "AddEsth", "AddEsth", 0, QObject::tr("Apocrypha"), QObject::tr("The Rest of the Chapters of the Book of Esther") },
//	{ QObject::tr("Wisdom"), "Wis", "Wis", 270, QObject::tr("Apocrypha"), QObject::tr("The Book of Wisdom or The Wisdom of Solomon") },
//	{ QObject::tr("Sirach"), "Sir", "Sir", 280, QObject::tr("Apocrypha"), QObject::tr("The Wisdom of Jesus the Son of Sirach, or Ecclesiasticus") },
//	{ QObject::tr("Baruch"), "Bar", "Bar", 320, QObject::tr("Apocrypha"), QObject::tr("The Book of Baruch") },
//	{ QObject::tr("Prayer of Azariah"), "PrAzar", "PrAz", 305, QObject::tr("Apocrypha"), QObject::tr("The Prayer of Azariah") },
//	{ QObject::tr("Susanna"), "Sus", "Sus", 325, QObject::tr("Apocrypha"), QObject::tr("The History of Susanna [in Daniel]") },
//	{ QObject::tr("Bel and the Dragon"), "Bel", "Bel", 345, QObject::tr("Apocrypha"), QObject::tr("The Book of Bel and the Dragon [in Daniel]") },
//	{ QObject::tr("Prayer of Manasses"), "PrMan", "PrMan", 790, QObject::tr("Apocrypha"), QObject::tr("The Prayer of Manasseh, or, The Prayer of Manasses King of Judah") },
//	{ QObject::tr("1 Maccabees"), "1Macc", "1Mac", 462, QObject::tr("Apocrypha"), QObject::tr("The First Book of the Maccabees") },
//	{ QObject::tr("2 Maccabees"), "2Macc", "2Mac", 464, QObject::tr("Apocrypha"), QObject::tr("The Second Book of the Maccabees") },
//	{ QObject::tr("3 Maccabees"), "3Macc", "3Mac", 466, QObject::tr("Apocrypha"), QObject::tr("The Third Book of the Maccabees") },
//	{ QObject::tr("4 Maccabees"), "4Macc", "4Mac", 467, QObject::tr("Apocrypha"), QObject::tr("The Fourth Book of the Maccabees") },
//	{ QObject::tr("Letter of Jeremiah"), "EpJer", "EpJer", 315, QObject::tr("Apocrypha"), QObject::tr("Letter (Epistle) of Jeremiah") },
//	{ QObject::tr("Psalm 151"), "Ps151", "Ps151", 232, QObject::tr("Apocrypha"), QObject::tr("Psalm 151") },
//	{ QObject::tr("Song of the Three Young Men"), "Sg3", "Sg3", 323, QObject::tr("Apocrypha"), QObject::tr("Song of the Three Young Men") },
//	{ QObject::tr("Greek Esther"), "EstGr", "EstGr", 192, QObject::tr("Apocrypha"), QObject::tr("Greek Esther") },
};

QSqlDatabase g_sqldbReadMain;

// ============================================================================

static unsigned int bookNumberToArrayIndex(int nBookNum)
{
	for (unsigned int ndx = 0; (ndx < NUM_BK); ++ndx) {
		if (nBookNum == g_arrBooks[ndx].m_nBookNumber) return ndx+1;
	}

	return 0;
}

static unsigned int bookIndexToTestamentIndex(unsigned int nBk)
{
	unsigned int nTst = 0;
	if (nBk == 0) return 0;			// Special-case for "no book"

	// note: Shift nBk to be an zero based since book indexes are normally one-based:
	--nBk;
	if (static_cast<unsigned int>(nBk) < NUM_BK_OT) {
		nTst = 1;
	} else if (static_cast<unsigned int>(nBk) < (NUM_BK_OT + NUM_BK_NT)) {
		nTst = 2;
	} else if (static_cast<unsigned int>(nBk) < (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC)) {
		nTst = 3;
	} else {
		nTst = 0;
		Q_ASSERT(false);			// Can't happen if our NUM_BK_xx values are correct!
	}

	return nTst;
}

// ============================================================================

class CMyBibleTextParser
{
public:
	CMyBibleTextParser()
	{ }

	QString parseToOsis(const QString &strText);

private:
};

typedef struct {
	bool m_bRemoveTextBetween;
	const QString m_strStartTag;
	const QString m_strEndTag;
	const QString m_strNewStartTag;
	const QString m_strNewEndTag;
	bool m_bAllowSplit;				// If true, the start/end can be split from previous parse
	bool m_bSplitTrack;				// If true at the end of a parse, the end tag is output before exiting, and if true at the start of a parse, the start tag is output
} TParseMatrix;

TParseMatrix g_lstParseMatrix[] =
{
	{ true, "&lt;f&gt;", "&lt;/f&gt;", "", "", false, false },								// Footnote markers
	{ true, "&lt;n&gt;", "&lt;/n&gt;", "" ,"", false, false },								// Endnote markers
	{ true, "&lt;S&gt;", "&lt;/S&gt;", "" ,"", false, false },								// Strongs numbers (remove)
	{ false, "&lt;/S&gt;", "", "" ,"", false, false },										// Stray Strongs numbers closing markers (remove)
	{ false, "&lt;e&gt;",  "", "&lt;em&gt;",  "", false, false },							// Emphasis Start
	{ false, "&lt;/e&gt;", "", "&lt;/em&gt;", "", false, false },							// Emphasis End

	{ false, "&lt;t&gt;", "", "", "", false, false },										// Text quote marker for formatting
	{ false, "&lt;/t&gt;", "", "" ,"", false, false },										// Text quote marker for formatting

	{ false, "&lt;i&gt;&lt;J&gt;", "", "&lt;J&gt;&lt;i&gt;", "", false, false },			// Put Words of Jesus markers on the output of TransChange
	{ false, "&lt;/J&gt;&lt;/i&gt;", "", "&lt;/i&gt;&lt;/J&gt;", "", false, false },

	{ false, "&lt;J&gt;", "&lt;/J&gt;", "<q who=\"Jesus\" marker=\"\">", "</q>", true, false },	// Words of Jesus Start/End (allow split)
//	{ false, "&lt;J&gt;", "", "", "", false, false },										// Words of Jesus Start (ignore markup)
	{ false, "&lt;/J&gt;", "", "", "", false, false },										// Words of Jesus End (ignore markup)

	{ false, "&lt;i&gt;", "&lt;/i&gt;", "<transChange type=\"added\">", "</transChange>", false, false },	// TransChange (or at least italics)
// Can't enable brackets here due to other note types using them (like double brackets):
//	{ false, "[", "]", "<transChange type=\"added\">", "</transChange>", false, false },	// TransChange

	{ false, "&lt;br/&gt;", "" ,"", "", false, false },										// Line break formatting (remove cause it doesn't work)
	{ false, "&lt;em&gt;", "", "", "", false, false },										// Emphasis start (remove)
	{ false, "&lt;/em&gt;", "", "", "", false, false },										// Emphasis end (remove)

	{ false, "&lt;/pb&gt;", "&lt;pb/&gt;", "<milestone type=\"x-p\" marker=\"¶\"/>", "", false, false },	// Paragraph markers w/pilcrow
	{ false, "&lt;pb/&gt;", "", "<milestone type=\"x-extra-p\"/>", "", false, false },		// Paragraph markers (extra space -- not pilcrows)

//	{ false, "&lt;/pb&gt;", "" ,"" ,"", false, false },										// Stray closing </pb> marker

};


QString CMyBibleTextParser::parseToOsis(const QString &strText)
{
	QString strOutText = htmlEscape(strText);
	int ndxStart;
	int ndxEnd;
	int nCount = _countof(g_lstParseMatrix);
	bool bUnbalanced = false;

	// Output pseudo-start for restarting a split:
	for (int nMatrix = 0; nMatrix < nCount; ++nMatrix) {
		if (g_lstParseMatrix[nMatrix].m_bSplitTrack) {
			Q_ASSERT(g_lstParseMatrix[nMatrix].m_bAllowSplit);
			strOutText.prepend(g_lstParseMatrix[nMatrix].m_strNewStartTag);
		}
	}

	for (int nMatrix = 0; nMatrix < nCount; ++nMatrix) {
		Q_ASSERT(!g_lstParseMatrix[nMatrix].m_strStartTag.isEmpty());

		// First terminate any split tracking end tags:
		if (g_lstParseMatrix[nMatrix].m_bSplitTrack) {
			Q_ASSERT(g_lstParseMatrix[nMatrix].m_bAllowSplit);
			ndxEnd = strOutText.indexOf(g_lstParseMatrix[nMatrix].m_strEndTag);
			ndxStart = strOutText.indexOf(g_lstParseMatrix[nMatrix].m_strStartTag);
			if (ndxEnd != -1) {
				if ((ndxStart == -1) || (ndxStart > ndxEnd)) {
					strOutText.replace(ndxEnd, g_lstParseMatrix[nMatrix].m_strEndTag.size(), g_lstParseMatrix[nMatrix].m_strNewEndTag);
					g_lstParseMatrix[nMatrix].m_bSplitTrack = false;
				} else {
					// If the next end overlapped next start, then end and start again (but leave start unprocessed so we will find its end below):
					strOutText.replace(ndxStart, g_lstParseMatrix[nMatrix].m_strStartTag.size(), g_lstParseMatrix[nMatrix].m_strNewEndTag + g_lstParseMatrix[nMatrix].m_strStartTag);
					g_lstParseMatrix[nMatrix].m_bSplitTrack = false;
				}
			}
		}

		// Find/replace start/end pairs:
		while ((ndxStart = strOutText.indexOf(g_lstParseMatrix[nMatrix].m_strStartTag)) != -1) {
			if (!g_lstParseMatrix[nMatrix].m_strEndTag.isEmpty()) {
				ndxEnd = strOutText.indexOf(g_lstParseMatrix[nMatrix].m_strEndTag, ndxStart+g_lstParseMatrix[nMatrix].m_strStartTag.size());
				if (ndxEnd == -1) {
					if (!g_lstParseMatrix[nMatrix].m_bAllowSplit) {
						bUnbalanced = true;
						break;
					} else {
						g_lstParseMatrix[nMatrix].m_bSplitTrack = true;		// Flag we have no end tag
					}
				} else {
					strOutText.replace(ndxEnd, g_lstParseMatrix[nMatrix].m_strEndTag.size(), g_lstParseMatrix[nMatrix].m_strNewEndTag);
					if (g_lstParseMatrix[nMatrix].m_bRemoveTextBetween) {
						strOutText.remove(ndxStart + g_lstParseMatrix[nMatrix].m_strStartTag.size(), ndxEnd - (ndxStart + g_lstParseMatrix[nMatrix].m_strStartTag.size()));
					}
				}
			}
			strOutText.replace(ndxStart, g_lstParseMatrix[nMatrix].m_strStartTag.size(), g_lstParseMatrix[nMatrix].m_strNewStartTag);
		}

		if (bUnbalanced) break;
	}

	// Output pseudo-end for ending a split:
	for (int nMatrix = 0; nMatrix < nCount; ++nMatrix) {
		if (g_lstParseMatrix[nMatrix].m_bSplitTrack) {
			Q_ASSERT(g_lstParseMatrix[nMatrix].m_bAllowSplit);
			strOutText.append(g_lstParseMatrix[nMatrix].m_strNewEndTag);
		}
	}

	if (bUnbalanced) {
		std::cerr << QString("\n*** Unbalanced Verse Parse Tags: \"%1\" => \"%2\"\n\n").arg(strText).arg(strOutText).toUtf8().data();
	}

	if (strOutText.contains("&lt;")) {
		QString strTemp = strOutText;
		// Note: It doesn't work to keep these:
//		strTemp.replace("&lt;br/&gt;", "");			// Keep embedded line-breaks for aesthetics
//		strTemp.replace("&lt;em&gt;", "");			// Keep emphasis for aesthetics
//		strTemp.replace("&lt;/em&gt;", "");			// Keep emphasis for aesthetics
		if (strTemp.contains("&lt;")) {
			std::cerr << QString("\n*** Verse contains unknown Parse Tag: \"%1\"\n\n").arg(strOutText).toUtf8().data();
		}
	}

	return strOutText;
}


// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

	int nArgsFound = 0;
	bool bUnknownOption = false;
	int nDescriptor = -1;
	QString strSQLFilename;
	QString strOSISFilename;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				strSQLFilename = strArg;
			} else if (nArgsFound == 3) {
				strOSISFilename = strArg;
			}
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 3) || (bUnknownOption)) {
		std::cerr << QString("MyBibleParse Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 <UUID-Index> <MyBible-SQL-in-file> <OSIS-out-file>\n\n").arg(argv[0]).toUtf8().data();
//		std::cerr << QString("Usage: %1 [options] <UUID-Index> <MyBible-SQL-in-file> <OSIS-out-file>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("<MyBible-SQL-in-file> = MyBible .SQLite3 Database File\n").toUtf8().data();
		std::cerr << QString("<OSIS-out-file>       = OSIS XML Output File\n\n").toUtf8().data();
//		std::cerr << QString("Options\n").toUtf8().data();
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

	g_sqldbReadMain = QSqlDatabase::addDatabase("QSQLITE", "MainReadConnection");

	QFile fileIn(strSQLFilename);
	QFile fileOut(strOSISFilename);

	QSqlDatabase myDatabase;
	myDatabase = g_sqldbReadMain;
	myDatabase.setDatabaseName(fileIn.fileName());
	myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!myDatabase.open()) {
		std::cerr << QString("Error: Couldn't open SQL database file \"%1\".\n\n%2\n").arg(fileIn.fileName()).arg(myDatabase.lastError().text()).toUtf8().data();
		return -2;
	}

	if (!fileOut.open(QIODevice::WriteOnly)) {
		std::cerr << QString("Unable to open \"%1\" for writing\n").arg(fileOut.fileName()).toUtf8().data();
		return -3;
	}

	fileOut.write(QString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n").toUtf8());
	fileOut.write(QString("<osis xmlns=\"http://www.bibletechnologies.net/2003/OSIS/namespace\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.bibletechnologies.net/2003/OSIS/namespace http://www.bibletechnologies.net/osisCore.2.1.1.xsd\">\n\n").toUtf8());
	fileOut.write(QString("<osisText osisIDWork=\"%1\" osisRefWork=\"defaultReferenceScheme\" lang=\"%2\">\n\n").arg(bblDescriptor.m_strWorkID).arg(bblDescriptor.m_strLanguage).toUtf8());
	fileOut.write(QString("<header>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"%1\">\n").arg(bblDescriptor.m_strWorkID).toUtf8());
	fileOut.write(QString("<title>%1</title>\n").arg(htmlEscape(bblDescriptor.m_strDBDesc)).toUtf8());
	fileOut.write(QString("<identifier type=\"OSIS\">Bible.%1</identifier>\n").arg(bblDescriptor.m_strWorkID).toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"defaultReferenceScheme\">\n").toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("</header>\n").toUtf8());

	QSqlQuery query(myDatabase);

	if(!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='info'")) {
		std::cerr << QString("Table Lookup for \"info\" Failed!\n%1\n").arg(query.lastError().text()).toUtf8().data();
		return -4;
	}
	query.next();
	if (!query.value(0).toInt()) {
		std::cerr << QString("Unable to find \"info\" Table in database!\n").arg(query.lastError().text()).toUtf8().data();
		return -5;
	}

	query.setForwardOnly(true);
	query.exec("SELECT name,value FROM info");

	while (query.next()) {
		std::cerr << query.value(0).toString().toUtf8().data() << " : " << query.value(1).toString().toUtf8().data() << "\n";
		if (query.value(0).toString().compare("language", Qt::CaseInsensitive) == 0) {
			if (query.value(1).toString().compare(bblDescriptor.m_strLanguage) != 0) {
				std::cerr << "   *** Expected language of \"" << bblDescriptor.m_strLanguage.toUtf8().data() << "\", but got \"" << query.value(1).toString().toUtf8().data() << "\"\n";
			}
		}
	}

	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='books'")) {
		std::cerr << QString("Table Lookup for \"books\" Failed!\n%1\n").arg(query.lastError().text()).toUtf8().data();
		return -4;
	}
	query.next();
	if (!query.value(0).toInt()) {
		std::cerr << QString("Unable to find \"books\" Table in database!\n").arg(query.lastError().text()).toUtf8().data();
		return -5;
	}

	query.setForwardOnly(true);
	query.exec("SELECT book_number,short_name,long_name FROM books");
	while (query.next()) {
		int nBookNum = query.value(0).toInt();
		int ndx = bookNumberToArrayIndex(nBookNum);
		if (ndx) {
			std::cerr << query.value(2).toString().trimmed().toUtf8().data() << " -> " << g_arrBooks[ndx-1].m_strName.toUtf8().data() << std::endl;
		} else {
			std::cerr << "*** Book: \"" << query.value(2).toString().toUtf8().data() << "\" Unknown or Wrong Mapping!" << std::endl;
		}
	}

	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='verses'")) {
		std::cerr << QString("Table Lookup for \"verses\" Failed!\n%1\n").arg(query.lastError().text()).toUtf8().data();
		return -4;
	}
	query.next();
	if (!query.value(0).toInt()) {
		std::cerr << QString("Unable to find \"verses\" Table in database!\n").arg(query.lastError().text()).toUtf8().data();
		return -5;
	}

	unsigned int nTst = 1;
	unsigned int nBk = 0;
	unsigned int nChp = 0;
	unsigned int nVrs = 0;
	unsigned int nNextTst = 0;
	unsigned int nNextBk = 0;
	unsigned int nNextChp = 0;
	unsigned int nNextVrs = 0;
	bool bTooManyBooks = false;

	CMyBibleTextParser parser;

	fileOut.write(QString("<div type=\"x-testament\">\n").toUtf8().data());

	query.setForwardOnly(true);
	query.exec("SELECT book_number,chapter,verse,text FROM verses");

	while (query.next()) {
		nNextBk = bookNumberToArrayIndex(query.value(0).toUInt());
		nNextChp = query.value(1).toUInt();
		nNextVrs = query.value(2).toUInt();
		QString strVerseText = query.value(3).toString().trimmed();

		if ((nNextBk <= 0) || (nNextBk > NUM_BK)) {
			if (nBk != nNextBk) {
				if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
				if (nChp != 0) fileOut.write(QString("</chapter>\n").toUtf8().data());
				if ((!bTooManyBooks) && (nBk != 0)) fileOut.write(QString("</div>\n").toUtf8().data());
				nChp = 0;
				nVrs = 0;

				std::cerr << QString("\nUnknown Book: %1\n").arg(nNextBk).toUtf8().data();
				nBk = nNextBk;
			}
			bTooManyBooks = true;
			continue;
		}

		nNextTst = bookIndexToTestamentIndex(nNextBk);
		if (nTst != nNextTst) {
			if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
			if (nChp != 0) fileOut.write(QString("</chapter>\n").toUtf8().data());
			if (nBk != 0) fileOut.write(QString("</div>\n").toUtf8().data());
			if (nTst != 0) fileOut.write(QString("</div>\n").toUtf8().data());
			fileOut.write(QString("<div type=\"x-testament\">\n").toUtf8().data());
			std::cerr << QString("\nTestament: %1\n").arg(nNextTst).toUtf8().data();
			nBk = 0;
			nChp = 0;
			nVrs = 0;
		}
		nTst = nNextTst;

		if (nBk != nNextBk) {
			if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
			if (nChp != 0) fileOut.write(QString("</chapter>\n").toUtf8().data());
			if (nBk != 0) fileOut.write(QString("</div>\n").toUtf8().data());
			fileOut.write(QString("<div type=\"book\" osisID=\"%1\">\n").arg(g_arrBooks[nNextBk-1].m_strOsisAbbr).toUtf8().data());
			nChp = 0;
			nVrs = 0;
			std::cerr << QString("\nBook: %1").arg(g_arrBooks[nNextBk-1].m_strOsisAbbr).toUtf8().data();
		}
		nBk = nNextBk;

		if (nChp != nNextChp) {
			if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
			if (nChp != 0) fileOut.write(QString("</chapter>\n").toUtf8().data());
			fileOut.write(QString("<chapter osisID=\"%1.%2\">\n").arg(g_arrBooks[nBk-1].m_strOsisAbbr).arg(nNextChp).toUtf8().data());
			nVrs = 0;
			std::cerr << QString("\nChapter: %1 %2").arg(g_arrBooks[nNextBk-1].m_strOsisAbbr).arg(nNextChp).toUtf8().data();
		}
		nChp = nNextChp;

		if (nVrs != nNextVrs) {
			if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
			fileOut.write(QString("<verse osisID=\"%1.%2.%3\">%4").arg(g_arrBooks[nBk-1].m_strOsisAbbr).arg(nChp).arg(nNextVrs).arg(parser.parseToOsis(strVerseText)).toUtf8().data());
			std::cerr << ".";
		} else {
			Q_ASSERT(false);
		}
		nVrs = nNextVrs;
	}
	std::cerr << "\n";

	if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
	if (nChp != 0) fileOut.write(QString("</chapter>\n").toUtf8().data());

	if ((!bTooManyBooks) && (nBk != 0)) fileOut.write(QString("</div>\n").toUtf8().data());			// End of Book

	if (nTst != 0) fileOut.write(QString("</div>\n").toUtf8().data());			// End of testament

	fileOut.write(QString("\n</osisText>\n").toUtf8());
	fileOut.write(QString("\n</osis>\n").toUtf8());

	fileOut.close();

	myDatabase.close();
	myDatabase = QSqlDatabase();

//	return a.exec();
	return 0;
}

// ============================================================================

