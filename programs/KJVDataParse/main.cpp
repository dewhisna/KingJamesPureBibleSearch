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

#include "../KJVCanOpener/dbstruct.h"
#include "../KJVCanOpener/BuildDB.h"

#include <QCoreApplication>
#include <QObject>
#include <QMainWindow>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtXml>
//#include <QtXml/QXmlInputSource>
//#include <QtXml/QXmlSimpleReader>
//#include <QtXml/QXmlDefaultHandler>
//#include <QtXml/QXmlAttributes>
//#include <QtXml/QXmlParseException>
#include <QStringList>
#include <QtGlobal>

#include <iostream>

QMainWindow *g_pMainWindow = NULL;

#define NUM_BK 66
//#define NUM_BK_OT 39
//#define NUM_BK_NT 27
#define NUM_TST 2

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

const QString g_arrstrTstNames[NUM_TST] =
		{	QObject::tr("Old Testament"),
			QObject::tr("New Testament")
		};


const QString g_strSpecChar = "'-";				// Special characters that are to remain as word characters and not treated as symbols in the text.  UTF-8 versions of these (like "en dash") will be folded into these

//const char *g_strCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-";		// Accept: [alphanumeric, -, '], we'll handle UTF-8 conversion and translate those to ASCII as appropriate



class COSISXmlHandler : public QXmlDefaultHandler
{
public:
	COSISXmlHandler(const QString &strNamespace)
		:	m_strNamespace(strNamespace),
			m_bCaptureTitle(false),
			m_bInVerse(false),
			m_nLevel(0)
	{
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(QString(), QString()));		// Note: We'll set the name and description later in the reading of the data
		for (int i=0; i<NUM_BK; ++i) {
			m_lstOsisBookList.append(g_arrBooks[i].m_strOsisAbbr);
		}
	}

	~COSISXmlHandler()
	{

	}

	QStringList elementNames() const { return m_lstElementNames; }
	QStringList attrNames() const { return m_lstAttrNames; }

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
	virtual bool characters(const QString &ch);
	virtual bool error(const QXmlParseException &exception);

	const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase.data(); }

protected:
	int findAttribute(const QXmlAttributes &attr, const QString &strName) {
		for (int i = 0; i < attr.count(); ++i) {
			if (attr.localName(i).compare(strName, Qt::CaseInsensitive) == 0) return i;
		}
		return -1;
	}

private:
	QString m_strNamespace;
	QStringList m_lstElementNames;
	QStringList m_lstAttrNames;

	CRelIndex m_ndxCurrent;
	bool m_bCaptureTitle;
	bool m_bInVerse;
	int m_nLevel;			// Parse depth level

	CBibleDatabasePtr m_pBibleDatabase;
	QStringList m_lstOsisBookList;
};

bool COSISXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

/*
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
*/

	int ndx = -1;
	int nTst = m_pBibleDatabase->m_lstTestaments.size();
	int nBk = -1;

	if (localName.compare("osisText", Qt::CaseInsensitive) == 0)  {
		ndx = findAttribute(atts, "osisIDWork");
		if (ndx != -1) m_pBibleDatabase->m_strName = atts.value(ndx);
		std::cerr << "Work: " << atts.value(ndx).toStdString() << "\n";
	} else if ((!m_ndxCurrent.isSet()) && (localName.compare("title", Qt::CaseInsensitive) == 0)) {
		m_bCaptureTitle = true;
	} else if ((!m_ndxCurrent.isSet()) && (localName.compare("div", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "type");
		if ((ndx != -1) && (atts.value(ndx).compare("x-testament", Qt::CaseInsensitive) == 0)) {
			CTestamentEntry aTestament((m_pBibleDatabase->m_lstTestaments.size() < NUM_TST) ? g_arrstrTstNames[m_pBibleDatabase->m_lstTestaments.size()] : QString());
			m_pBibleDatabase->m_EntireBible.m_nNumTst++;
			m_pBibleDatabase->m_lstTestaments.push_back(aTestament);
			nTst++;
			std::cerr << "Testament: " << ((m_pBibleDatabase->m_lstTestaments.size() <= NUM_TST) ? aTestament.m_strTstName.toStdString() : "<Unknown>") << "\n";
		}
	} else if ((!m_ndxCurrent.isSet()) && (localName.compare("chapter", Qt::CaseInsensitive) == 0)) {
		if (nTst == 0) {
			std::cerr << "\n*** Found book/chapter before testament marker!\n";
		} else {
			ndx = findAttribute(atts, "osisID");
			if (ndx != -1) {
				QStringList lstOsisID = atts.value(ndx).split('.');
				if ((lstOsisID.size() != 2) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
					m_ndxCurrent = CRelIndex();
					 std::cerr << "\n*** Unknown Chapter osisID : " << atts.value(ndx).toStdString() << "\n";
				} else {
					std::cerr << "Book: " << lstOsisID.at(0).toStdString() << " Chapter: " << lstOsisID.at(1).toStdString();
					m_ndxCurrent = CRelIndex(nBk+1, lstOsisID.at(1).toUInt(), 0, 0);
					m_pBibleDatabase->m_EntireBible.m_nNumChp++;
					m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumChp++;
					if (lstOsisID.at(1).toUInt() == 1) {
						m_pBibleDatabase->m_EntireBible.m_nNumBk++;
						m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumBk++;
						m_pBibleDatabase->m_lstBooks.resize(qMax(static_cast<unsigned int>(nBk+1), static_cast<unsigned int>(m_pBibleDatabase->m_lstBooks.size())));
						m_pBibleDatabase->m_lstBooks[nBk].m_nTstBkNdx = m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumBk;
						m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx = nTst;
						m_pBibleDatabase->m_lstBooks[nBk].m_strBkName = g_arrBooks[nBk].m_strName;
						m_pBibleDatabase->m_lstBooks[nBk].m_strBkAbbr = g_arrBooks[nBk].m_strOsisAbbr;
						m_pBibleDatabase->m_lstBooks[nBk].m_strTblName = g_arrBooks[nBk].m_strTableName;
						m_pBibleDatabase->m_lstBooks[nBk].m_strCat = g_arrBooks[nBk].m_strCategory;
						m_pBibleDatabase->m_lstBooks[nBk].m_strDesc = g_arrBooks[nBk].m_strDescription;
					}
					assert(m_pBibleDatabase->m_lstBooks.size() > static_cast<unsigned int>(nBk));
					m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp++;
				}
			} else {
				m_ndxCurrent = CRelIndex();
				std::cerr << "\n*** Chapter with no osisID : ";
				for (int i=0; i<atts.count(); ++i) {
					if (i) std::cerr << ",";
					std::cerr << atts.localName(i).toStdString() << "=" << atts.value(i).toStdString();
				}
				std::cerr << "\n";
			}
		}
	} else if ((m_ndxCurrent.isSet()) && (localName.compare("verse", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "osisID");
		if (ndx != -1) {
			QStringList lstOsisID = atts.value(ndx).split('.');
			if ((lstOsisID.size() != 3) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
				m_ndxCurrent.setVerse(0);
				m_ndxCurrent.setWord(0);
				std::cerr << "\n*** Unknown Verse osisID : " << atts.value(ndx).toStdString() << "\n";
			} else if ((m_ndxCurrent.book() != static_cast<unsigned int>(nBk+1)) || (m_ndxCurrent.chapter() != lstOsisID.at(1).toUInt())) {
				m_ndxCurrent.setVerse(0);
				m_ndxCurrent.setWord(0);
				std::cerr << "\n*** Verse osisID doesn't match Chapter osisID : " << atts.value(ndx).toStdString() << "\n";
			} else {
				m_ndxCurrent.setVerse(lstOsisID.at(2).toUInt());
				m_ndxCurrent.setWord(0);
				std::cerr << ".";
				m_bInVerse = true;
				m_pBibleDatabase->m_EntireBible.m_nNumVrs++;
				assert(static_cast<unsigned int>(nTst) <= m_pBibleDatabase->m_lstTestaments.size());
				m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumVrs++;
				assert(m_pBibleDatabase->m_lstBooks.size() > static_cast<unsigned int>(nBk));
				m_pBibleDatabase->m_lstBooks[nBk].m_nNumVrs++;
			}
		}
	}



/*
	m_lstElementNames.append(localName);
	std::cout << "{" << localName.toStdString() << "}";
	std::cout << "[";
	for (int i = 0; i < atts.count(); ++i) {
		if (i) std::cout << ",";
		std::cout << atts.localName(i).toStdString() << "=" << atts.value(i).toStdString();
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


	if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		m_bCaptureTitle = false;
	} else if (localName.compare("chapter", Qt::CaseInsensitive) == 0) {
		m_ndxCurrent = CRelIndex();
		std::cerr << "\n";
		assert(m_bInVerse == false);
	} else if (localName.compare("verse", Qt::CaseInsensitive) == 0) {
		m_ndxCurrent.setVerse(0);
		m_ndxCurrent.setWord(0);
		m_bInVerse = false;
	}



//	std::cout << "{/" << localName.toStdString() << "}\n";

	return true;
}

bool COSISXmlHandler::characters(const QString &ch)
{
	QString strTemp = ch;
	strTemp.replace('\n', ' ');

	if (m_bCaptureTitle) {
		m_pBibleDatabase->m_strDescription = strTemp;
		std::cerr << "Title: " << strTemp.toStdString() << "\n";
	} else if (m_bInVerse) {
		// TODO : Process verse text here


/*
		nWrd = 0;
		pTemp = strpbrk(buffPlain, g_strCharset);
		while (pTemp) {								// pTemp = start of word
			nWrd++;
			pTemp2 = pTemp+1;
			while ((*pTemp2 != 0) && (strchr(g_strCharset, *pTemp2) != NULL)) ++pTemp2;
			memcpy(word, pTemp, pTemp2-pTemp);
			word[pTemp2-pTemp] = 0;
			if ((bDoingWordDump) && ((nBkNdx == 0) || (nBk == nBkNdx))) fprintf(stdout, "%s\r\n", word);
			pTemp = strpbrk(pTemp2, g_strCharset);
		}		// Here, nWrd = Number of words in this verse
		countWrd[nBk-1] += nWrd;			// Add in the number of words we found

		if ((bDoingBook) && ((nBkNdx == 0) || (nBk == nBkNdx))) {
			fprintf(stdout, "%d,%d,%d,\"%s\",\"%s\",\"%s\"\r\n", nChp*256+nVrs, nWrd, (bIsPilcrow ? 1 : 0), buffPlain, buffRich, buffFootnote);
		}
*/



	}



//	std::cout << ch.toStdString();

	return true;
}

bool COSISXmlHandler::error(const QXmlParseException &exception)
{
	std::cerr << QString("\n\n*** %1\n").arg(exception.message()).toStdString();
	return true;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	const char *pstrFilename = NULL;

	if (argc < 2) {
		std::cerr << QString("Usage: %1 <OSIS-Database>\n").arg(argv[0]).toStdString();
		return -1;
	}

	pstrFilename = argv[1];

	QFile fileOSIS;

	fileOSIS.setFileName(QString(pstrFilename));
	if (!fileOSIS.open(QIODevice::ReadOnly)) {
		std::cerr << QString("\n\n*** Failed to open OSIS database \"%1\"\n").arg(pstrFilename).toStdString();
		return -2;
	}

	QXmlInputSource xmlInput(&fileOSIS);
	QXmlSimpleReader xmlReader;
	COSISXmlHandler xmlHandler("http://www.bibletechnologies.net/2003/OSIS/namespace");

	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.setErrorHandler(&xmlHandler);
//	xmlReader.setFeature("http://www.bibletechnologies.net/2003/OSIS/namespace", true);

	bool bOK = xmlReader.parse(xmlInput);
	if (!bOK) {
		std::cerr << QString("\n\n*** Failed to parse OSIS database \"%1\"\n%2\n").arg(pstrFilename).arg(xmlHandler.errorString()).toStdString();
		return -3;
	}

	const CBibleDatabase *pBibleDatabase = xmlHandler.bibleDatabase();

	std::cout << QString("Bible:  Testaments: %1  Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->bibleEntry().m_nNumTst)
						.arg(pBibleDatabase->bibleEntry().m_nNumBk)
						.arg(pBibleDatabase->bibleEntry().m_nNumChp)
						.arg(pBibleDatabase->bibleEntry().m_nNumVrs)
						.arg(pBibleDatabase->bibleEntry().m_nNumWrd)
						.toStdString();

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumTst; ++i) {
		std::cout << QString("%1 : Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->testamentEntry(i)->m_strTstName)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumBk)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumWrd)
						.toStdString();
	}

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumBk; ++i) {
		std::cout << QString("%1 : Chapters: %2  Verses: %3  Words: %4\n")
						.arg(pBibleDatabase->bookEntry(i)->m_strBkName)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumWrd)
						.toStdString();
	}

/*
	std::cout << "\n============================ Element Names  =================================\n";
	QStringList lstElements = xmlHandler.elementNames();
	lstElements.sort();
	lstElements.removeDuplicates();
	for (int i = 0; i < lstElements.count(); ++i) {
		std::cout << lstElements.at(i).toStdString() << "\n";
	}

	std::cout << "\n\n============================ Attribute Names  =================================\n";
	QStringList lstAttrib = xmlHandler.attrNames();
	lstAttrib.sort();
	lstAttrib.removeDuplicates();
	for (int i = 0; i < lstAttrib.count(); ++i) {
		std::cout << lstAttrib.at(i).toStdString() << "\n";
	}

*/

//	return a.exec();
	return 0;
}

