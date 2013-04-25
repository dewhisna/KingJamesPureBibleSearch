#include <QCoreApplication>

#include <iostream>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>

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
	{ QObject::tr("Joshua"), "Josh", "JOSH", QObject::tr("OT Narative"), "" },
	{ QObject::tr("Judges"), "Judg", "JUDG", QObject::tr("OT Narative"), "" },
	{ QObject::tr("Ruth"), "Ruth", "RUTH", QObject::tr("OT Narative"), "" },
	{ QObject::tr("1 Samuel"), "1Sam", "SAM1", QObject::tr("OT Narative"), QObject::tr("The First Book of Samuel Otherwise Called, The First Book of the Kings") },
	{ QObject::tr("2 Samuel"), "2Sam", "SAM2", QObject::tr("OT Narative"), QObject::tr("The Second Book of Samuel Otherwise Called, The Second Book of the Kings") },
	{ QObject::tr("1 Kings"), "1Kgs", "KGS1", QObject::tr("OT Narative"), QObject::tr("The First Book of the Kings Commonly Called, The Third Book of the Kings") },
	{ QObject::tr("2 Kings"), "2Kgs", "KGS2", QObject::tr("OT Narative"), QObject::tr("The Second Book of the Kings Commonly Called, The Fourth Book of the Kings") },
	{ QObject::tr("1 Chronicles"), "1Chr", "CHR1", QObject::tr("OT Narative"), QObject::tr("The First Book of the Chronicles") },
	{ QObject::tr("2 Chronicles"), "2Chr", "CHR2", QObject::tr("OT Narative"), QObject::tr("The Second Book of the Chronicles") },
	{ QObject::tr("Ezra"), "Ezra", "EZRA", QObject::tr("OT Narative"), "" },
	{ QObject::tr("Nehemiah"), "Neh", "NEH", QObject::tr("OT Narative"), "" },
	{ QObject::tr("Esther"), "Esth", "ESTH", QObject::tr("OT Narative"), "" },
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
	{ QObject::tr("Matthew"), "Matt", "MATT", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint Matthew") },
	{ QObject::tr("Mark"), "Mark", "MARK", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint Mark") },
	{ QObject::tr("Luke"), "Luke", "LUKE", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint Luke") },
	{ QObject::tr("John"), "John", "JOHN", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint John") },
	{ QObject::tr("Acts"), "Acts", "ACTS", QObject::tr("NT Narative"), QObject::tr("The Acts of the Apostles") },
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
	fileOut.write(QString("<osisText osisIDWork=\"KJF\" osisRefWork=\"defaultReferenceScheme\" lang=\"fr\">\n\n").toUtf8());
	fileOut.write(QString("<header>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"KJF\">\n").toUtf8());
	fileOut.write(QString("<title>la Bible King James Française, édition 2006</title>\n").toUtf8());
	fileOut.write(QString("<identifier type=\"OSIS\">Bible.KJF</identifier>\n").toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("<work osisWork=\"defaultReferenceScheme\">\n").toUtf8());
	fileOut.write(QString("<refSystem>Bible.KJV</refSystem>\n").toUtf8());
	fileOut.write(QString("</work>\n").toUtf8());
	fileOut.write(QString("</header>\n").toUtf8());

	bool bInBook = false;
	unsigned int nBk = 0;
	unsigned int nChp = 0;
	unsigned int nVrs = 0;
	QString strLastBookName;
	QString strVerseText;

	while (!fileIn.atEnd()) {
		QByteArray arrLine;
		QString strLine;

		arrLine = fileIn.readLine();
		strLine = QString::fromUtf8(arrLine.data(), arrLine.size());

		if (!bInBook) {
			if ((strLine.isEmpty()) || (strLine.at(0) == '\n')) continue;
			QStringList lstLine = strLine.split(' ');
			QString strBookName;
			for (int i = 0; i < qMax((lstLine.size()-1), 1); ++i) {
				if (!strBookName.isEmpty()) strBookName += ' ';
				strBookName += strLine.split(' ').at(i).trimmed();
			}

			if (strBookName.compare(strLastBookName, Qt::CaseInsensitive) != 0) {
				if (nBk) {
					fileOut.write(QString("</div>\n").toUtf8());
				}
				nBk++;
				strLastBookName = strBookName;
				std::cout << QString("Book: %1\n").arg(strBookName).toUtf8().data();

				fileOut.write(QString("<div type=\"book\" osisID=\"%1\">\n").arg(g_arrBooks[nBk-1].m_strOsisAbbr).toUtf8());
			}

			bInBook = true;
			nChp = 0;
			nVrs = 0;
			strVerseText.clear();

			std::cout << '\t' << arrLine.data();
			if (lstLine.size() >= 2) {
				fileOut.write(QString("<chapter osisID=\"%1.%2\">\n").arg(g_arrBooks[nBk-1].m_strOsisAbbr).arg(lstLine.at(lstLine.size()-1).trimmed()).toUtf8());
			} else {
				fileOut.write(QString("<chapter osisID=\"%1.1\">\n").arg(g_arrBooks[nBk-1].m_strOsisAbbr).toUtf8());
			}
		} else {
			if ((strLine.isEmpty()) || (strLine.at(0) == '\n')) {
				if (!strVerseText.isEmpty()) {
					fileOut.write(QString("<verse osisID=\"%1.%2.%3\">%4</verse>\n").arg(g_arrBooks[nBk-1].m_strOsisAbbr).arg(nChp).arg(nVrs).arg(strVerseText).toUtf8());
					strVerseText.clear();
				}
				fileOut.write(QString("</chapter>\n").toUtf8());
				bInBook = false;
				continue;
			}

			if (strLine.at(0).isDigit()) {
				if (!strVerseText.isEmpty()) {
					fileOut.write(QString("<verse osisID=\"%1.%2.%3\">%4</verse>\n").arg(g_arrBooks[nBk-1].m_strOsisAbbr).arg(nChp).arg(nVrs).arg(strVerseText).toUtf8());
					strVerseText.clear();
				}

				nChp = strLine.split(' ').at(0).split(':').at(0).toUInt();
				nVrs = strLine.split(' ').at(0).split(':').at(1).toUInt();
				strVerseText = strLine.mid(strLine.indexOf(' ')+1);
				strVerseText.remove('\n');
			} else {
				strVerseText += QChar(' ') + strLine;
				strVerseText.remove('\n');
			}
		}
	}
	if (!strVerseText.isEmpty()) {
		fileOut.write(QString("<verse osisID=\"%1.%2.%3\">%4</verse>\n").arg(g_arrBooks[nBk-1].m_strOsisAbbr).arg(nChp).arg(nVrs).arg(strVerseText).toUtf8());
		strVerseText.clear();
	}
	fileOut.write(QString("</chapter>\n").toUtf8());
	fileOut.write(QString("</div>\n").toUtf8());
	fileOut.write(QString("\n</osisText>\n").toUtf8());
	fileOut.write(QString("\n</osis>\n").toUtf8());

	fileOut.close();
	fileIn.close();

//	return a.exec();
	return 0;
}
