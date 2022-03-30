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

//#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>


#define NUM_BK 66u
#define NUM_BK_OT 39u
#define NUM_BK_NT 27u
#define NUM_TST 2u

#define PSALMS_BOOK_NUM 19

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
	const QString m_strName;
	const QString m_strOsisAbbr;
	const QString m_strTableName;
	const QString m_strCategory;
	const QString m_strDescription;
} TBook;

TBook g_arrBooks[NUM_BK] =
{
	{ QObject::tr("Genesis"), "Gen", "GEN", QObject::tr("Law"), QObject::tr("The First Book of Moses") },
	{ QObject::tr("Exodus"), "Exod", "EXOD", QObject::tr("Law"), QObject::tr("The Second Book of Moses") },
	{ QObject::tr("Leviticus"), "Lev", "LEV", QObject::tr("Law"), QObject::tr("The Third Book of Moses") },
	{ QObject::tr("Numbers"), "Num", "NUM", QObject::tr("Law"), QObject::tr("The Fourth Book of Moses") },
	{ QObject::tr("Deuteronomy"), "Deut", "DEUT", QObject::tr("Law"), QObject::tr("The Fifth Book of Moses") },
	{ QObject::tr("Joshua"), "Josh", "JOSH", QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Judges"), "Judg", "JUDG", QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Ruth"), "Ruth", "RUTH", QObject::tr("OT Narrative"), "" },
	{ QObject::tr("1 Samuel"), "1Sam", "SAM1", QObject::tr("OT Narrative"), QObject::tr("The First Book of Samuel Otherwise Called, The First Book of the Kings") },
	{ QObject::tr("2 Samuel"), "2Sam", "SAM2", QObject::tr("OT Narrative"), QObject::tr("The Second Book of Samuel Otherwise Called, The Second Book of the Kings") },
	{ QObject::tr("1 Kings"), "1Kgs", "KGS1", QObject::tr("OT Narrative"), QObject::tr("The First Book of the Kings Commonly Called, The Third Book of the Kings") },
	{ QObject::tr("2 Kings"), "2Kgs", "KGS2", QObject::tr("OT Narrative"), QObject::tr("The Second Book of the Kings Commonly Called, The Fourth Book of the Kings") },
	{ QObject::tr("1 Chronicles"), "1Chr", "CHR1", QObject::tr("OT Narrative"), QObject::tr("The First Book of the Chronicles") },
	{ QObject::tr("2 Chronicles"), "2Chr", "CHR2", QObject::tr("OT Narrative"), QObject::tr("The Second Book of the Chronicles") },
	{ QObject::tr("Ezra"), "Ezra", "EZRA", QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Nehemiah"), "Neh", "NEH", QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Esther"), "Esth", "ESTH", QObject::tr("OT Narrative"), "" },
	{ QObject::tr("Job"), "Job", "JOB", QObject::tr("Wisdom"), "" },
	{ QObject::tr("Psalms"), "Ps", "PS", QObject::tr("Wisdom"), "" },
	{ QObject::tr("Proverbs"), "Prov", "PROV", QObject::tr("Wisdom"), "" },
	{ QObject::tr("Ecclesiastes"), "Eccl", "ECCL", QObject::tr("Wisdom"), QObject::tr("Ecclesiastes; Or, The Preacher") },
	{ QObject::tr("Song Of Solomon"), "Song", "SONG", QObject::tr("Wisdom"), "" },
	{ QObject::tr("Isaiah"), "Isa", "ISA", QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Isaiah") },
	{ QObject::tr("Jeremiah"), "Jer", "JER", QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Jeremiah") },
	{ QObject::tr("Lamentations"), "Lam", "LAM", QObject::tr("Major Prophets"), QObject::tr("The Lamentations of Jeremiah") },
	{ QObject::tr("Ezekiel"), "Ezek", "EZEK", QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Ezekiel") },
	{ QObject::tr("Daniel"), "Dan", "DAN", QObject::tr("Major Prophets"), QObject::tr("The Book of <i>the Prophet</i> Daniel") },
	{ QObject::tr("Hosea"), "Hos", "HOS", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Joel"), "Joel", "JOEL", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Amos"), "Amos", "AMOS", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Obadiah"), "Obad", "OBAD", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Jonah"), "Jonah", "JONAH", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Micah"), "Mic", "MIC", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Nahum"), "Nah", "NAH", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Habakkuk"), "Hab", "HAB", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Zephaniah"), "Zeph", "ZEPH", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Haggai"), "Hag", "HAG", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Zechariah"), "Zech", "ZECH", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Malachi"), "Mal", "MAL", QObject::tr("Minor Prophets"), "" },
	{ QObject::tr("Matthew"), "Matt", "MATT", QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint Matthew") },
	{ QObject::tr("Mark"), "Mark", "MARK", QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint Mark") },
	{ QObject::tr("Luke"), "Luke", "LUKE", QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint Luke") },
	{ QObject::tr("John"), "John", "JOHN", QObject::tr("NT Narrative"), QObject::tr("The Gospel According to Saint John") },
	{ QObject::tr("Acts"), "Acts", "ACTS", QObject::tr("NT Narrative"), QObject::tr("The Acts of the Apostles") },
	{ QObject::tr("Romans"), "Rom", "ROM", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Romans") },
	{ QObject::tr("1 Corinthians"), "1Cor", "COR1", QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to the Corinthians") },
	{ QObject::tr("2 Corinthians"), "2Cor", "COR2", QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to the Corinthians") },
	{ QObject::tr("Galatians"), "Gal", "GAL", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Galatians") },
	{ QObject::tr("Ephesians"), "Eph", "EPH", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Ephesians") },
	{ QObject::tr("Philippians"), "Phil", "PHIL", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Philippians") },
	{ QObject::tr("Colossians"), "Col", "COL", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Colossians") },
	{ QObject::tr("1 Thessalonians"), "1Thess", "THESS1", QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to the Thessalonians") },
	{ QObject::tr("2 Thessalonians"), "2Thess", "THESS2", QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to the Thessalonains") },
	{ QObject::tr("1 Timothy"), "1Tim", "TIM1", QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to Timothy") },
	{ QObject::tr("2 Timothy"), "2Tim", "TIM2", QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to Timothy") },
	{ QObject::tr("Titus"), "Titus", "TITUS", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul to Titus") },
	{ QObject::tr("Philemon"), "Phlm", "PHLM", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul to Philemon") },
	{ QObject::tr("Hebrews"), "Heb", "HEB", QObject::tr("General Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Hebrews") },
	{ QObject::tr("James"), "Jas", "JAS", QObject::tr("General Epistles"), QObject::tr("The General Epistle of James") },
	{ QObject::tr("1 Peter"), "1Pet", "PET1", QObject::tr("General Epistles"), QObject::tr("The First General Epistle of Peter") },
	{ QObject::tr("2 Peter"), "2Pet", "PET2", QObject::tr("General Epistles"), QObject::tr("The Second General Epistle of Peter") },
	{ QObject::tr("1 John"), "1John", "JOHN1", QObject::tr("General Epistles"), QObject::tr("The First General Epistle of John") },
	{ QObject::tr("2 John"), "2John", "JOHN2", QObject::tr("General Epistles"), QObject::tr("The Second General Epistle of John") },
	{ QObject::tr("3 John"), "3John", "JOHN3", QObject::tr("General Epistles"), QObject::tr("The Third General Epistle of John") },
	{ QObject::tr("Jude"), "Jude", "JUDE", QObject::tr("General Epistles"), QObject::tr("The General Epistle of Jude") },
	{ QObject::tr("Revelation"), "Rev", "REV", QObject::tr("Apocalyptic Epistle"), QObject::tr("The Revelation of Jesus Christ") }
};


static QString convertVerseText(const QString &strVerseText, unsigned int nBook, unsigned int nChapter, unsigned int nVerse)		// note: nVerse 0 = first verse and means use subtitles.  Others mean last verse of chapter and use colophons
{
	Q_ASSERT(nBook < NUM_BK);

	// Markup is as follows:
	//	[brackets] are for Italicized words
	//	<brackets> are for the Words of Christ in Red
	//	«brackets» are for the Titles in the Book of Psalms, and the Ending tags of some of the Epistles in the New Testament.

	// In Psalms 119, remove the built-in Hebrew Transliterations as KJVCanOpener will render these for us with the Hebrew Letters:
	QString strVerse = strVerseText;
	if ((nBook == (PSALMS_BOOK_NUM-1)) && (nChapter == 118) && ((nVerse % 8) == 0)) {
		int ndxOf = strVerse.indexOf(".");
		Q_ASSERT(ndxOf != -1);
		strVerse = strVerse.mid(ndxOf+1);
	}

	QString strTemp;
	int nBracketsAdded = 0;				// Integrity check for Added text brackets (should balance to 0)
	int nBracketsWordsOfJesus = 0;		// Integrity check for Words of Jesus text brackets (should balance to 0)
	int nBracketsColophon = 0;			// Integrity check for Colophons/Subtitles (should balance to 0)

	for (int ndx = 0; ndx < strVerse.size(); ++ndx) {
		if (strVerse.at(ndx) == QChar('[')) {
			strTemp += "<transChange type=\"added\">";
			nBracketsAdded++;
		} else if (strVerse.at(ndx) == QChar(']')) {
			strTemp += "</transChange>";
			nBracketsAdded--;
		} else if (strVerse.at(ndx) == QChar('<')) {
			strTemp += "<q who=\"Jesus\">";
			nBracketsWordsOfJesus++;
		} else if (strVerse.at(ndx) == QChar('>')) {
			strTemp += "</q>";
			nBracketsWordsOfJesus--;
		} else if (strVerse.at(ndx) == QChar(0x00AB)) {			// '«'
			if (nVerse == 0) {
				strTemp += "<title canonical=\"true\" subType=\"x-preverse\" type=\"section\">";
			} else {
				strTemp += QString("<div osisID=\"%1.c\" type=\"colophon\">").arg(g_arrBooks[nBook].m_strOsisAbbr);
			}
			nBracketsColophon++;
		} else if (strVerse.at(ndx) == QChar(0x00BB)) {			// '»'
			if (nVerse == 0) {
				strTemp += "</title>";
			} else {
				strTemp += "</div>";
			}
			nBracketsColophon--;
		} else if (!strVerse.at(ndx).isDigit()) {			// For some reason, four verses end with a number: 1 Chronicles 6:62, Nehemiah 7:64, Job 32:2, 2 Chronicles 5:2 ?? Not sure why, but they shouldn't be there!
			strTemp += strVerse.at(ndx);
		}
	}

	Q_ASSERT(nBracketsAdded == 0);
	Q_ASSERT(nBracketsWordsOfJesus == 0);
	Q_ASSERT(nBracketsColophon == 0);

	return strTemp;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (argc < 3) {
		std::cerr << QString::fromUtf8("Usage: %1 <in-file> <out-file>\n").arg(argv[0]).toStdString();
		return -1;
	}

	QFile fileIn(argv[1]);
	QFile fileOut(argv[2]);

	if (!fileIn.open(QIODevice::ReadOnly)) {
		std::cerr << QString::fromUtf8("Unable to open \"%1\" for reading\n").arg(fileIn.fileName()).toUtf8().data();
		return -2;
	}

	if (!fileOut.open(QIODevice::WriteOnly)) {
		std::cerr << QString::fromUtf8("Unable to open \"%1\" for writing\n").arg(fileOut.fileName()).toUtf8().data();
		return -3;
	}

	fileOut.write(QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n"));
	fileOut.write(QByteArray("<osis xmlns=\"http://www.bibletechnologies.net/2003/OSIS/namespace\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.bibletechnologies.net/2003/OSIS/namespace http://www.bibletechnologies.net/osisCore.2.1.1.xsd\">\n\n"));
	fileOut.write(QByteArray("<osisText osisIDWork=\"KJVPureCambridge\" osisRefWork=\"defaultReferenceScheme\" lang=\"en\">\n\n"));
	fileOut.write(QByteArray("<header>\n"));
	fileOut.write(QByteArray("<work osisWork=\"KJVPureCambridge\">\n"));
	fileOut.write(QByteArray("<title>KJV Pure Cambridge</title>\n"));
	fileOut.write(QByteArray("<identifier type=\"OSIS\">Bible.KJVPureCambridge</identifier>\n"));
	fileOut.write(QByteArray("<refSystem>Bible.KJV</refSystem>\n"));
	fileOut.write(QByteArray("</work>\n"));
	fileOut.write(QByteArray("<work osisWork=\"defaultReferenceScheme\">\n"));
	fileOut.write(QByteArray("<refSystem>Bible.KJV</refSystem>\n"));
	fileOut.write(QByteArray("</work>\n"));
	fileOut.write(QByteArray("</header>\n"));

	unsigned int nVerseCount = 0;
	unsigned int nTst = 0;

	for (unsigned int nBk = 0; nBk < NUM_BK; ++nBk) {
		if ((nBk == 0) || (nBk == NUM_BK_OT))
			fileOut.write(QByteArray("<div type=\"x-testament\">\n"));


		QStringList lstVerseCounts = g_arrChapterVerseCounts[nBk].split(",");		// Number of chapters = size of lstVerseCounts, each entry is number of verses in chapter

		std::cout << QString::fromUtf8("Book: %1\n").arg(g_arrBooks[nBk].m_strName).toUtf8().data();
		fileOut.write(QString::fromUtf8("<div type=\"book\" osisID=\"%1\">\n").arg(g_arrBooks[nBk].m_strOsisAbbr).toUtf8());

		for (unsigned int nChp = 0; nChp < static_cast<unsigned int>(lstVerseCounts.size()); ++nChp) {
			unsigned int nVerses = lstVerseCounts.at(nChp).toUInt();

			std::cout << QString::fromUtf8("    Chapter: %1").arg(nChp+1).toUtf8().data();
			fileOut.write(QString::fromUtf8("<chapter osisID=\"%1.%2\">\n").arg(g_arrBooks[nBk].m_strOsisAbbr).arg(nChp+1).toUtf8().data());

			for (unsigned int nVrs = 0; nVrs < nVerses; ++nVrs) {
				if (fileIn.atEnd()) {
					std::cerr << "Unexpected end of input file!";
					break;
				}

				++nVerseCount;

				std::cout << ".";
				fileOut.write(QString::fromUtf8("<verse osisID=\"%1.%2.%3\">%4").arg(g_arrBooks[nBk].m_strOsisAbbr).arg(nChp+1).arg(nVrs+1).arg(convertVerseText(QString::fromUtf8(fileIn.readLine()), nBk, nChp, nVrs).trimmed()).toUtf8().data());
				fileOut.write(QByteArray("</verse>\n"));
			}

			std::cout << "\n";
			fileOut.write(QByteArray("</chapter>\n"));
		}

		fileOut.write(QByteArray("</div>\n"));				// End of Book

		if ((nBk == (NUM_BK_OT - 1)) || (nBk == (NUM_BK - 1))) {
			fileOut.write(QByteArray("</div>\n"));			// End of testament
			nTst++;
		}
	}

	if (nVerseCount != 31102) {
		std::cerr << QString::fromUtf8("Error: Expected 31102 verses, but found %1").arg(nVerseCount).toUtf8().data();
	}

	fileOut.write(QByteArray("\n</osisText>\n"));
	fileOut.write(QByteArray("\n</osis>\n"));

	fileOut.close();
	fileIn.close();

//	return a.exec();
	return 0;
}

