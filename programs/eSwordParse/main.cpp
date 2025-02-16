/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

// Note: This tool works for MySword databases as well as eSword databases

#include <QCoreApplication>

#include <iostream>

#include <QObject>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QtSql>
#include <QSqlQuery>

#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/BibleLayout.h"

#include "version.h"

// ============================================================================

static inline QString htmlEscape(const QString &aString)
{
	return aString.toHtmlEscaped();
}

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

// ============================================================================


QSqlDatabase g_sqldbReadMain;

// ============================================================================

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

class CRTFParseBaton
{
public:
	CRTFParseBaton(bool bConvertAmpersand)
		:	m_bConvertAmpersand(bConvertAmpersand)
	{

	}

	QString parseToOsis(const QString &strRTFText);

private:
	bool m_bConvertAmpersand;
};

typedef struct {
	bool m_bRemoveTextBetween;
	const QString m_strStartTag;
	const QString m_strEndTag;
	const QString m_strNewStartTag;
	const QString m_strNewEndTag;
} TParseMatrix;

TParseMatrix g_lstParseMatrix[] =
{
	{ false, "{\\i ", "}", "<transChange type=\"added\">", "</transChange>" },				// TransChange
	{ false, "{\\cf15 ", "}", "<div type=\"colophon\">", "</div>" },						// Colophons
	{ false, "{\\b ", "}", "<title type=\"psalm\" canonical=\"true\">", "</title>" },		// Superscriptions
	{ true, "{\\qc ", "}", "", "" },														// Psalm 119 Foreign Language Tags
	{ false, "\\par\\par", "", "<milestone type=\"x-extra-p\"/>", "" },						// Paragraph markers (extra space -- not pilcrows)
	{ false, "\\par", "", "", "" },															// "Eat" stray single "\par" tags

	// TODO : We need to parse the \cf2 tags for doing the Apocrypha as e-Sword
	//		strangely has it mixed up and not properly segregated from the main
	//		Bible text.  For now, we'll just strip it out:
	{ true, "{\\cf2", "}", "", "" },														// Eat the apocrypha text for now
	{ false, "\\line", "", "", "" }															// Ignore special line (embedded verse) tags for now...
};

QString CRTFParseBaton::parseToOsis(const QString &strRTFText)
{
	QString strOutText = strRTFText;
	int ndxStart;
	int ndxEnd;
	int nCount = _countof(g_lstParseMatrix);
	bool bUnbalanced = false;

	for (int nMatrix = 0; nMatrix < nCount; ++nMatrix) {
		Q_ASSERT(!g_lstParseMatrix[nMatrix].m_strStartTag.isEmpty());
		while ((ndxStart = strOutText.indexOf(g_lstParseMatrix[nMatrix].m_strStartTag)) != -1) {
			if (!g_lstParseMatrix[nMatrix].m_strEndTag.isEmpty()) {
				ndxEnd = strOutText.indexOf(g_lstParseMatrix[nMatrix].m_strEndTag, ndxStart+g_lstParseMatrix[nMatrix].m_strStartTag.size());
				if (ndxEnd == -1) {
					bUnbalanced = true;
					break;
				}
				strOutText.replace(ndxEnd, g_lstParseMatrix[nMatrix].m_strEndTag.size(), g_lstParseMatrix[nMatrix].m_strNewEndTag);
				if (g_lstParseMatrix[nMatrix].m_bRemoveTextBetween) {
					strOutText.remove(ndxStart + g_lstParseMatrix[nMatrix].m_strStartTag.size(), ndxEnd - (ndxStart + g_lstParseMatrix[nMatrix].m_strStartTag.size()));
				}
			}
			strOutText.replace(ndxStart, g_lstParseMatrix[nMatrix].m_strStartTag.size(), g_lstParseMatrix[nMatrix].m_strNewStartTag);
		}

		if (bUnbalanced) break;
	}

	if (bUnbalanced) {
		std::cerr << QString("\n*** Unbalanced Verse Parse Tags: \"%1\" => \"%2\"\n\n").arg(strRTFText).arg(strOutText).toUtf8().data();
	}

	if (strOutText.contains("{\\")) {
		std::cerr << QString("\n*** Verse contains unknown Parse Tag: \"%1\"\n\n").arg(strOutText).toUtf8().data();
	}

	if (m_bConvertAmpersand) {
		strOutText.replace(QChar('&'), QObject::tr("and"));
	}

	return htmlEscape(strOutText);
}


// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(eSwordParse_VERSION);

	int nArgsFound = 0;
	bool bConvertAmpersand = false;
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
		} else if (strArg.compare("-a") == 0) {
			bConvertAmpersand = true;
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 3) || (bUnknownOption)) {
		std::cerr << eSwordParse_APPNAME << " Version " << eSwordParse_VERSION_SEMVER << "\n\n";
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <eSword-SQL-in-file> <OSIS-out-file>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("<eSword-SQL-in-file> = e-Sword .bblx SQL Database File\n").toUtf8().data();
		std::cerr << QString("<OSIS-out-file>      = OSIS XML Output File\n\n").toUtf8().data();
		std::cerr << QString("Options\n").toUtf8().data();
		std::cerr << QString("    -a  =  Convert Ampersands to \"and\"\n").toUtf8().data();
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

	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='Bible'")) {
		std::cerr << QString("Table Lookup for \"Bible\" Failed!\n%1\n").arg(query.lastError().text()).toUtf8().data();
		return -4;
	}
	query.next();
	if (!query.value(0).toInt()) {
		std::cerr << QString("Unable to find \"Bible\" Table in database!\n").arg(query.lastError().text()).toUtf8().data();
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

	CRTFParseBaton rtfParseBaton(bConvertAmpersand);

	fileOut.write(QString("<div type=\"x-testament\">\n").toUtf8().data());

	query.setForwardOnly(true);
	query.exec("SELECT Book,Chapter,Verse,Scripture FROM Bible");
	while (query.next()) {
		nNextBk = query.value(0).toUInt();
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
			fileOut.write(QString("<div type=\"book\" osisID=\"%1\">\n").arg(g_arrBibleBooks[nNextBk-1].m_lstOsisAbbr.at(0)).toUtf8().data());
			nChp = 0;
			nVrs = 0;
			std::cerr << QString("\nBook: %1").arg(g_arrBibleBooks[nNextBk-1].m_lstOsisAbbr.at(0)).toUtf8().data();
		}
		nBk = nNextBk;

		if (nChp != nNextChp) {
			if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
			if (nChp != 0) fileOut.write(QString("</chapter>\n").toUtf8().data());
			fileOut.write(QString("<chapter osisID=\"%1.%2\">\n").arg(g_arrBibleBooks[nBk-1].m_lstOsisAbbr.at(0)).arg(nNextChp).toUtf8().data());
			nVrs = 0;
			std::cerr << QString("\nChapter: %1 %2").arg(g_arrBibleBooks[nNextBk-1].m_lstOsisAbbr.at(0)).arg(nNextChp).toUtf8().data();
		}
		nChp = nNextChp;

		if (nVrs != nNextVrs) {
			if (nVrs != 0) fileOut.write(QString("</verse>\n").toUtf8().data());
			fileOut.write(QString("<verse osisID=\"%1.%2.%3\">%4").arg(g_arrBibleBooks[nBk-1].m_lstOsisAbbr.at(0)).arg(nChp).arg(nNextVrs).arg(rtfParseBaton.parseToOsis(strVerseText)).toUtf8().data());
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

