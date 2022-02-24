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

#include <QCoreApplication>

#include <iostream>

#include <QObject>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

#define NUM_BK 66u
#define NUM_BK_OT 39u
#define NUM_BK_NT 27u
#define NUM_TST 2u

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

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (argc < 3) {
		std::cerr << QString("Usage: %1 <in-file> <out-file>\n").arg(argv[0]).toStdString();
		return -1;
	}

	QFile fileIn(argv[1]);
	QFile fileOut(argv[2]);

	if (!fileIn.open(QIODevice::ReadOnly)) {
		std::cerr << QString("Unable to open \"%1\" for reading\n").arg(fileIn.fileName()).toStdString();
		return -2;
	}

	if (!fileOut.open(QIODevice::WriteOnly)) {
		std::cerr << QString("Unable to open \"%1\" for writing\n").arg(fileOut.fileName()).toStdString();
		return -3;
	}

	fileOut.write(QString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n").toUtf8());
	fileOut.write(QString("<osis xmlns=\"http://www.bibletechnologies.net/2003/OSIS/namespace\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.bibletechnologies.net/2003/OSIS/namespace http://www.bibletechnologies.net/osisCore.2.1.1.xsd\">\n\n").toUtf8());
	fileOut.write(QString("<osisText osisIDWork=\"RV1865\" osisRefWork=\"defaultReferenceScheme\" lang=\"es\">\n\n").toUtf8());
	fileOut.write(QString("<header>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"RV1865\">\n").toUtf8());
	fileOut.write(QString("<title>The Reina Valera 1865 text revised by Dr. Ángel H. de Mora</title>\n").toUtf8());
	fileOut.write(QString("<identifier type=\"OSIS\">Bible.RV1865</identifier>\n").toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"defaultReferenceScheme\">\n").toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("</header>\n").toUtf8());

	QJsonParseError jsonParseError;
	QJsonDocument jsonRV1865(QJsonDocument::fromJson(fileIn.readAll(), &jsonParseError));

	if (jsonParseError.error != QJsonParseError::NoError) {
		std::cerr << "*** JSON Error: " << jsonParseError.errorString().toUtf8().data() << "\n";
		return -4;
	}

	unsigned int nTst = 1;
	unsigned int nBk = 0;
	unsigned int nChp = 0;
	unsigned int nVrs = 0;
	unsigned int nNextTst = 0;
	unsigned int nNextBk = 0;
	unsigned int nNextChp = 0;
	unsigned int nNextVrs = 0;

	fileOut.write(QString("<div type=\"x-testament\">\n").toUtf8().data());

	Q_ASSERT(jsonRV1865.isArray());
	unsigned int nVerseIndex = 0;
	QJsonArray arrRV1865(jsonRV1865.array());
	for (QJsonArray::const_iterator itr = arrRV1865.constBegin(); itr != arrRV1865.constEnd(); ++itr) {
		const QJsonValue valEntry = *itr;
		Q_ASSERT(valEntry.isObject());
		const QJsonObject objEntry(valEntry.toObject());
		nVerseIndex++;
		Q_ASSERT(objEntry.value("pk").toVariant().toUInt() == nVerseIndex);
		const QJsonObject objVerseEntry(objEntry.value("fields").toObject());
		nNextBk = objVerseEntry.value("book_id").toVariant().toUInt();
		nNextChp = objVerseEntry.value("chapter_id").toVariant().toUInt();
		nNextVrs = objVerseEntry.value("verse_id").toVariant().toUInt();

		Q_ASSERT((nNextBk > 0) && (nNextBk <= NUM_BK));

		if (nNextBk > 0) nNextTst = 1;
		if (nNextBk > NUM_BK_OT) nNextTst = 2;
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
			fileOut.write(QString("<verse osisID=\"%1.%2.%3\">%4").arg(g_arrBooks[nBk-1].m_strOsisAbbr).arg(nChp).arg(nNextVrs).arg(objVerseEntry.value("text").toString()).toUtf8().data());
			std::cerr << ".";
		} else {
			Q_ASSERT(false);
		}
		nVrs = nNextVrs;
	}
	std::cerr << "\n";

	if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
	if (nChp != 0) fileOut.write(QString("</chapter>\n").toUtf8().data());

	if (nBk != 0) fileOut.write(QString("</div>\n").toUtf8().data());			// End of Book

	if (nTst != 0) fileOut.write(QString("</div>\n").toUtf8().data());			// End of testament

	fileOut.write(QString("\n</osisText>\n").toUtf8());
	fileOut.write(QString("\n</osis>\n").toUtf8());

	fileOut.close();
	fileIn.close();

//	return a.exec();
	return 0;
}

