/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

// ReadDB.cpp -- Code to read out database into our working variables
//

#include "ReadDB.h"
#include "dbstruct.h"
#include "CSV.h"
#include "PhraseEdit.h"

#include <assert.h>

#include <QFile>

#include <QtSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QByteArray>

#include <QString>

// Used for debugging:
#ifdef NEVER
#include <QFile>
#include <QTextStream>
#endif

QSqlDatabase g_sqldbReadMain = QSqlDatabase::addDatabase("QSQLITE", "MainReadConnection");
QSqlDatabase g_sqldbReadUser = QSqlDatabase::addDatabase("QSQLITE", "UserReadConnection");

namespace {
	const char *g_constrReadDatabase = "Reading Database";
}		// Namespace

// ============================================================================

bool CReadDatabase::ReadTestamentTable()
{
	// Read the Testament Table

	QSqlQuery query(m_myDatabase);

	// Check to see if the table exists:
	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TESTAMENT'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Table Lookup for \"TESTAMENT\" Failed!\n%1").arg(query.lastError().text()));
		return false;
	}
	query.next();
	if (!query.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, "Unable to find \"TESTAMENT\" Table in database!");
		return false;
	}

	g_lstTestaments.clear();
	query.setForwardOnly(true);
	query.exec("SELECT * FROM TESTAMENT");
	while (query.next()) {
		unsigned int nTstNdx = query.value(0).toUInt();
		if (nTstNdx > g_lstTestaments.size()) g_lstTestaments.resize(nTstNdx);
		CTestamentEntry &entryTestament = g_lstTestaments[nTstNdx-1];
		entryTestament.m_strTstName = query.value(1).toString();
	}

	return true;
}

bool CReadDatabase::ReadTOCTable()
{
	// Read the TOC Table

	QSqlQuery query(m_myDatabase);

	// Check to see if the table exists:
	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TOC'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Table Lookup for \"TOC\" Failed!\n%1").arg(query.lastError().text()));
		return false;
	}
	query.next();
	if (!query.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, "Unable to find \"TOC\" Table in database!");
		return false;
	}

	g_EntireBible = CTestamentEntry("Entire Bible");

	g_lstTOC.clear();
	query.setForwardOnly(true);
	query.exec("SELECT * FROM TOC");
	while (query.next()) {
		unsigned int nBkNdx = query.value(0).toUInt();
		if (nBkNdx > g_lstTOC.size()) g_lstTOC.resize(nBkNdx);
		CTOCEntry &entryTOC = g_lstTOC[nBkNdx-1];
		entryTOC.m_nTstBkNdx = query.value(1).toUInt();
		entryTOC.m_nTstNdx = query.value(2).toUInt();
		entryTOC.m_strBkName = query.value(3).toString();
		entryTOC.m_strBkAbbr = query.value(4).toString();
		entryTOC.m_strTblName = query.value(5).toString();
		entryTOC.m_nNumChp = query.value(6).toUInt();
		entryTOC.m_nNumVrs = query.value(7).toUInt();
		entryTOC.m_nNumWrd = query.value(8).toUInt();
		entryTOC.m_strCat = query.value(9).toString();
		entryTOC.m_strDesc = query.value(10).toString();

		g_lstTestaments[entryTOC.m_nTstNdx-1].m_nNumBk++;
		g_lstTestaments[entryTOC.m_nTstNdx-1].m_nNumChp += entryTOC.m_nNumChp;
		g_lstTestaments[entryTOC.m_nTstNdx-1].m_nNumVrs += entryTOC.m_nNumVrs;
		g_lstTestaments[entryTOC.m_nTstNdx-1].m_nNumWrd += entryTOC.m_nNumWrd;

		g_EntireBible.m_nNumBk++;
		g_EntireBible.m_nNumChp += entryTOC.m_nNumChp;
		g_EntireBible.m_nNumVrs += entryTOC.m_nNumVrs;
		g_EntireBible.m_nNumWrd += entryTOC.m_nNumWrd;
	}

// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		for (TTOCList::const_iterator itr = g_lstTOC.begin(); itr != g_lstTOC.end(); ++itr) {
			QString strTemp = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10\r\n").arg(itr->m_nTstBkNdx).arg(itr->m_nTstNdx).arg(itr->m_strBkName).arg(itr->m_strBkAbbr)
									.arg(itr->m_strTblName).arg(itr->m_nNumChp).arg(itr->m_nNumVrs).arg(itr->m_nNumWrd).arg(itr->m_strCat).arg(itr->m_strDesc);
			ts << strTemp;
		}
		fileTest.close();
	}
#endif

	return true;
}

bool CReadDatabase::ReadLAYOUTTable()
{
	// Read the LAYOUT table:

	QSqlQuery query(m_myDatabase);

	// Check to see if the table exists:
	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='LAYOUT'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Table Lookup for \"LAYOUT\" Failed!\n%1").arg(query.lastError().text()));
		return false;
	}
	query.next();
	if (!query.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, "Unable to find \"LAYOUT\" Table in database!");
		return false;
	}

	g_mapLayout.clear();

	query.setForwardOnly(true);
	query.exec("SELECT * FROM LAYOUT");
	while (query.next()) {
		uint32_t nBkChpNdx = query.value(0).toUInt();
		CLayoutEntry &entryLayout = g_mapLayout[CRelIndex(nBkChpNdx << 16)];
		entryLayout.m_nNumVrs = query.value(1).toUInt();
		entryLayout.m_nNumWrd = query.value(2).toUInt();
	}

// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		for (TLayoutMap::const_iterator itr = g_mapLayout.begin(); itr != g_mapLayout.end(); ++itr) {
			QString strTemp = QString("%1,%2,%3\r\n").arg(itr->first.index()>>16).arg(itr->second.m_nNumVrs).arg(itr->second.m_nNumWrd);
			ts << strTemp;
		}
		fileTest.close();
	}
#endif

	return true;
}

bool CReadDatabase::ReadBookTables()
{
	// Read the BOOK tables:

	assert(g_lstTOC.size() != 0);		// Must read TOC before BOOKS

	g_lstBooks.clear();
	g_lstBooks.resize(g_lstTOC.size());
	for (unsigned int i=0; i<g_lstTOC.size(); ++i) {
		QSqlQuery query(m_myDatabase);

		// Check to see if the table exists:
		if (!query.exec(QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(g_lstTOC[i].m_strTblName))) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Table Lookup for \"%1\" Failed!\n%2").arg(g_lstTOC[i].m_strTblName).arg(query.lastError().text()));
			return false;
		}
		query.next();
		if (!query.value(0).toInt()) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Unable to find \"%1\" Table in database!").arg(g_lstTOC[i].m_strTblName));
			return false;
		}

		TBookEntryMap &mapBook = g_lstBooks[i];

		mapBook.clear();

		query.setForwardOnly(true);
		query.exec(QString("SELECT * FROM %1").arg(g_lstTOC[i].m_strTblName));
		while (query.next()) {
			QString strVerseText;
			uint32_t nChpVrsNdx = query.value(0).toUInt();
			CBookEntry &entryBook = mapBook[CRelIndex(nChpVrsNdx << 8)];
			entryBook.m_nNumWrd = query.value(1).toUInt();
			entryBook.m_bPilcrow = ((query.value(2).toInt() != 0) ? true : false);
			strVerseText = query.value(4).toString();
			if (strVerseText.isEmpty()) strVerseText = query.value(3).toString();
			entryBook.setText(strVerseText);
		}

// Used for debugging:
#ifdef NEVER
		QFile fileTest(QString("testit%1.txt").arg(i, 2, 10, QChar('0')));
		if (fileTest.open(QIODevice::WriteOnly)) {
			CCSVStream csv(&fileTest);
			for (TBookEntryMap::const_iterator itr = mapBook.begin(); itr != mapBook.end(); ++itr) {
				QStringList sl;
				sl.push_back(QString("%1").arg(itr->first.index()>>8));
				sl.push_back(QString("%1").arg(itr->second.m_nNumWrd));
				sl.push_back(QString("%1").arg(itr->second.m_bPilcrow));
				sl.push_back(itr->second.text());
				csv << sl;
			}
			fileTest.close();
		}
#endif

	}

	return true;
}

bool CReadDatabase::IndexBlobToIndexList(const QByteArray &baBlob, TIndexList &anIndexList)
{
	if ((baBlob.size() % sizeof(uint32_t)) != 0) return false;

	anIndexList.clear();
	for (unsigned int i=0; i<(baBlob.size()/sizeof(uint32_t)); ++i) {
		uint32_t nValue = 0;
		for (unsigned int j=0; j<sizeof(uint32_t); ++j) {
			nValue = nValue << 8;
			nValue = nValue | (baBlob[static_cast<unsigned int>((i*sizeof(uint32_t))+j)] & 0xFF);
		}
		anIndexList.push_back(nValue);
	}

	return true;
}

bool CReadDatabase::ReadWORDSTable()
{
	// Read the WORDS table:

	QSqlQuery query(m_myDatabase);

	// Check to see if the table exists:
	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='WORDS'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Table Lookup for \"WORDS\" Failed!\n%1").arg(query.lastError().text()));
		return false;
	}
	query.next();
	if (!query.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, "Unable to find \"WORDS\" Table in database!");
		return false;
	}

	unsigned int nNumWordsInText = 0;
	for (unsigned int ndxTOC=0; ndxTOC<g_lstTOC.size(); ++ndxTOC) {
		nNumWordsInText += g_lstTOC[ndxTOC].m_nNumWrd;
	}

	g_mapWordList.clear();
	g_lstConcordanceWords.clear();
	g_lstConcordanceMapping.clear();
	g_lstConcordanceMapping.resize(nNumWordsInText+1);			// Preallocate our concordance mapping as we know how many words the text contains (+1 for zero position)

	query.setForwardOnly(true);
	query.exec("SELECT * FROM WORDS");
	while (query.next()) {
		QString strWord = query.value(1).toString();
		QString strKey = strWord.toLower();
		CWordEntry &entryWord = g_mapWordList[strKey];
		entryWord.m_strWord = strWord;
		entryWord.m_bCasePreserve = ((query.value(2).toInt()) ? true : false);

		QString strAltWords = query.value(4).toString();
		CCSVStream csvWord(&strAltWords, QIODevice::ReadOnly);
		while (!csvWord.atEndOfStream()) {
			QString strTemp;
			csvWord >> strTemp;
			if (!strTemp.isEmpty()) entryWord.m_lstAltWords.push_back(strTemp);
		}
		QString strAltWordCounts = query.value(5).toString();
		CCSVStream csvWordCount(&strAltWordCounts, QIODevice::ReadOnly);
		unsigned int nAltCount = 0;
		while (!csvWordCount.atEndOfStream()) {
			QString strTemp;
			csvWordCount >> strTemp;
			if (!strTemp.isEmpty()) {
				entryWord.m_lstAltWordCount.push_back(strTemp.toUInt());
				nAltCount += strTemp.toUInt();
			}
		}
		if (entryWord.m_lstAltWords.size() != entryWord.m_lstAltWordCount.size()) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Mismatch Word Counts for \"%1\" AltWords=%2, AltWordCounts=%3")
							.arg(strWord).arg(entryWord.m_lstAltWords.size()).arg(entryWord.m_lstAltWordCount.size()));
			return false;
		}
		if (nAltCount != query.value(3).toUInt()) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Bad AltWordCounts for \"%1\"").arg(strWord));
			return false;
		}

		if (!IndexBlobToIndexList(query.value(6).toByteArray(), entryWord.m_ndxNormalizedMapping)) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Bad word indexes for \"%1\"").arg(strWord));
			return false;
		}
		if (entryWord.m_ndxNormalizedMapping.size() != query.value(3).toUInt()) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, "Index/Count consistency error in WORDS table!");
			return false;
		}
		// Add this word and alternates to our concordance, and set all normalized indices that refer it to
		//	point to the the specific word:
		unsigned int ndxMapping=0;
		for (int ndxAltWord=0; ndxAltWord<entryWord.m_lstAltWords.size(); ++ndxAltWord) {
			g_lstConcordanceWords.push_back(entryWord.m_lstAltWords.at(ndxAltWord));
			for (unsigned int ndxAltCount=0; ndxAltCount<entryWord.m_lstAltWordCount.at(ndxAltWord); ++ndxAltCount) {
				if (entryWord.m_ndxNormalizedMapping[ndxMapping] > nNumWordsInText) {
					QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Invalid WORDS mapping.  Check database integrity!\n\nWord: \"%1\"  Index: %2").arg(entryWord.m_lstAltWords.at(ndxAltWord)).arg(entryWord.m_ndxNormalizedMapping[ndxMapping]));
					return false;
				}
				g_lstConcordanceMapping[entryWord.m_ndxNormalizedMapping[ndxMapping]] = g_lstConcordanceWords.size();
				ndxMapping++;
			}
		}
	}

// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		int cnt = 0;
		for (TWordListMap::const_iterator itr = g_mapWordList.begin(); itr != g_mapWordList.end(); ++itr) {
			cnt++;
			ts << QString("%1,").arg(cnt);
//			ts << "\"" + itr->first + "\",";
			ts << "\"" + itr->second.m_strWord + "\",";
			if (strcmp(itr->first.c_str(), itr->second.m_strWord.c_str()) == 0) {
				ts << "1,";
			} else {
				ts << "0,";
			}
			ts << QString("%1,").arg(itr->second.m_ndxNormalizedMapping.size());
			ts << "\"";
			for (unsigned int i=0; i<itr->second.m_lstAltWords.size(); ++i) {
				if (i!=0) ts << ",";
				ts << itr->second.m_lstAltWords.at(i);
			}
			ts << "\",\"";
			for (unsigned int i=0; i<itr->second.m_ndxNormalizedMapping.size(); ++i) {
				if (i!=0) ts << ",";
				ts << QString("%1").arg(itr->second.m_ndxNormalizedMapping.at(i));
			}
			ts << "\"\n";
		}
		fileTest.close();
	}
#endif

	return true;
}

bool CReadDatabase::ReadFOOTNOTESTable()
{
	// Read the FOOTNOTES table:

	QSqlQuery query(m_myDatabase);

	// Check to see if the table exists:
	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='FOOTNOTES'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Table Lookup for \"FOOTNOTES\" Failed!\n%1").arg(query.lastError().text()));
		return false;
	}
	query.next();
	if (!query.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, "Unable to find \"FOOTNOTES\" Table in database!");
		return false;
	}

	g_mapFootnotes.clear();

	query.setForwardOnly(true);
	query.exec("SELECT * FROM FOOTNOTES");
	while (query.next()) {
		QString strFootnoteText;
		CFootnoteEntry footnote;
		CRelIndex ndxRel(query.value(0).toUInt());
		assert(ndxRel.isSet());
		if (!ndxRel.isSet()) continue;
		strFootnoteText = query.value(2).toString();
		if (strFootnoteText.isEmpty()) strFootnoteText = query.value(1).toString();
		if (!strFootnoteText.isEmpty()) {
			footnote.setText(strFootnoteText);
			g_mapFootnotes[ndxRel] = footnote;
		}
	}

	return true;
}

bool CReadDatabase::ReadPHRASESTable(bool bUserPhrases)
{
	// Read the PHRASES table:

	QSqlQuery query(m_myDatabase);

	// Check to see if the table exists:
	if (!query.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='PHRASES'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Table Lookup for \"PHRASES\" Failed!\n%1").arg(query.lastError().text()));
		return false;
	}
	query.next();
	if (!query.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, "Unable to find \"PHRASES\" Table in database!");
		return false;
	}

	if (bUserPhrases) {
		g_lstUserPhrases.clear();
	} else {
		g_lstCommonPhrases.clear();
	}

	query.setForwardOnly(true);
	query.exec("SELECT * FROM PHRASES");
	while (query.next()) {
		CPhraseEntry phrase;
		phrase.m_strPhrase = query.value(1).toString();
		phrase.m_bCaseSensitive = ((query.value(2).toInt() != 0) ? true : false);
		if (!phrase.m_strPhrase.isEmpty()) {
			CParsedPhrase parsedPhrase;
			parsedPhrase.ParsePhrase(phrase.m_strPhrase);
			phrase.m_nNumWrd = parsedPhrase.phraseSize();
			if (bUserPhrases) {
				g_lstUserPhrases.push_back(phrase);
			} else {
				g_lstCommonPhrases.push_back(phrase);
			}
		}
	}

	return true;
}

bool CReadDatabase::ValidateData()
{
	unsigned int ncntTstTot = 0;	// Total number of Testaments
	unsigned int ncntBkTot = 0;		// Total number of Books (all Testaments)
	unsigned int ncntChpTot = 0;	// Total number of Chapters (all Books)
	unsigned int ncntVrsTot = 0;	// Total number of Verses (all Chapters)
	unsigned int ncntWrdTot = 0;	// Total number of Words (all verses)

	unsigned int ncntChp_Bk = 0;	// Chapter count in current Book
	unsigned int ncntVrs_Bk = 0;	// Verse count in current Book
	unsigned int ncntWrd_Bk = 0;	// Word count in current Book

	unsigned int ncntVrs_Chp = 0;	// Verse count in current Chapter
	unsigned int ncntWrd_Chp = 0;	// Word count in current Chapter

	unsigned int ncntWrd_Vrs = 0;	// Word count in current Verse

	if (g_lstBooks.size() != g_lstTOC.size()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, "Error: Book List and Table of Contents have different sizes!\nCheck the database!");
		return false;
	}

	ncntTstTot = g_lstTestaments.size();
	for (unsigned int nBk=0; nBk<g_lstTOC.size(); ++ nBk) {		// Books
		if ((g_lstTOC[nBk].m_nTstNdx < 1) || (g_lstTOC[nBk].m_nTstNdx > ncntTstTot)) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Book \"%1\" (%2) References Invalid Testament %3")
								.arg(g_lstTOC[nBk].m_strBkName).arg(nBk+1).arg(g_lstTOC[nBk].m_nTstNdx));
			return false;
		}
		ncntChp_Bk = 0;
		ncntVrs_Bk = 0;
		ncntWrd_Bk = 0;
		ncntBkTot++;
		for (unsigned int nChp=0; nChp<g_lstTOC[nBk].m_nNumChp; ++nChp) {	// Chapters
			ncntVrs_Chp = 0;
			ncntWrd_Chp = 0;
			TLayoutMap::const_iterator itrLayout = g_mapLayout.find(CRelIndex(nBk+1,nChp+1,0,0));
			if (itrLayout == g_mapLayout.end()) continue;
			ncntChpTot++;
			ncntChp_Bk++;
			for (unsigned int nVrs=0; nVrs<itrLayout->second.m_nNumVrs; ++nVrs) {	// Verses
				ncntWrd_Vrs = 0;
				const TBookEntryMap &aBook = g_lstBooks[nBk];
				TBookEntryMap::const_iterator itrBook = aBook.find(CRelIndex(0,nChp+1,nVrs+1,0));
				if (itrBook == aBook.end()) continue;
				ncntVrsTot++;
				ncntVrs_Chp++;
				ncntVrs_Bk++;
				ncntWrdTot += itrBook->second.m_nNumWrd;			// Words
				ncntWrd_Vrs += itrBook->second.m_nNumWrd;
				ncntWrd_Chp += itrBook->second.m_nNumWrd;
				ncntWrd_Bk += itrBook->second.m_nNumWrd;
			}
			if (ncntVrs_Chp != itrLayout->second.m_nNumVrs) {
				QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Book \"%1\" (%2) Chapter %3 contains %4 Verses, expected %5 Verses!")
									.arg(g_lstTOC[nBk].m_strBkName).arg(nBk+1).arg(nChp+1).arg(ncntVrs_Chp).arg(itrLayout->second.m_nNumVrs));
				return false;
			}
			if (ncntWrd_Chp != itrLayout->second.m_nNumWrd) {
				QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Book \"%1\" (%2) Chapter %3 contains %4 Words, expected %5 Words!")
									.arg(g_lstTOC[nBk].m_strBkName).arg(nBk+1).arg(nChp+1).arg(ncntWrd_Chp).arg(itrLayout->second.m_nNumWrd));
				return false;
			}
		}
		if (ncntChp_Bk != g_lstTOC[nBk].m_nNumChp) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Book \"%1\" (%2) contains %3 Chapters, expected %4 Chapters!")
									.arg(g_lstTOC[nBk].m_strBkName).arg(nBk+1).arg(ncntChp_Bk).arg(g_lstTOC[nBk].m_nNumChp));
			return false;
		}
		if (ncntVrs_Bk != g_lstTOC[nBk].m_nNumVrs) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Book \"%1\" (%2) contains %3 Verses, expected %4 Verses!")
									.arg(g_lstTOC[nBk].m_strBkName).arg(nBk+1).arg(ncntVrs_Bk).arg(g_lstTOC[nBk].m_nNumVrs));
			return false;
		}
		if (ncntWrd_Bk != g_lstTOC[nBk].m_nNumWrd) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Book \"%1\" (%2) contains %3 Words, expected %4 Words!")
									.arg(g_lstTOC[nBk].m_strBkName).arg(nBk+1).arg(ncntWrd_Bk).arg(g_lstTOC[nBk].m_nNumWrd));
			return false;
		}
	}

	unsigned int nWordListTot = 0;
	for (TWordListMap::const_iterator itrWords = g_mapWordList.begin(); itrWords != g_mapWordList.end(); ++itrWords) {
		nWordListTot += itrWords->second.m_ndxNormalizedMapping.size();
	}
	if (nWordListTot != ncntWrdTot) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Word List contains %1 indexes, expected %2!").arg(nWordListTot).arg(ncntWrdTot));
		return false;
	}

	// Check concordance:
	if ((nWordListTot+1) != g_lstConcordanceMapping.size()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Word List contains %1 indexes, but Concordance Mapping contains %2 entries!").arg(nWordListTot+1).arg(g_lstConcordanceMapping.size()));
		return false;
	}

	return true;
}

// ============================================================================

bool CReadDatabase::ReadDatabase(const QString &strDatabaseFilename)
{
	m_myDatabase = g_sqldbReadMain;
	m_myDatabase.setDatabaseName(strDatabaseFilename);
	m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

//	QMessageBox::information(m_pParent, g_constrReadDatabase, m_myDatabase.databaseName());

	if (!m_myDatabase.open()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Couldn't open database file \"%1\".\n\n%2").arg(strDatabaseFilename).arg(m_myDatabase.lastError().text()));
		return false;
	}

	bool bSuccess = true;

	if ((!ReadTestamentTable()) ||
		(!ReadTOCTable()) ||
		(!ReadLAYOUTTable()) ||
		(!ReadBookTables()) ||
		(!ReadWORDSTable()) ||
		(!ReadFOOTNOTESTable()) ||
		(!ReadPHRASESTable(false)) ||
		(!ValidateData())) bSuccess = false;

	m_myDatabase.close();

	m_myDatabase = QSqlDatabase();

	return bSuccess;
}

bool CReadDatabase::ReadUserDatabase(const QString &strDatabaseFilename)
{
	m_myDatabase = g_sqldbReadUser;
	m_myDatabase.setDatabaseName(strDatabaseFilename);
	m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!m_myDatabase.open()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QString("Error: Couldn't open database file \"%1\".\n\n%2").arg(strDatabaseFilename).arg(m_myDatabase.lastError().text()));
		return false;
	}

	bool bSuccess = true;

	if (!ReadPHRASESTable(true)) {
		bSuccess = false;
	} else {
		g_bUserPhrasesDirty = false;
	}

	m_myDatabase.close();

	m_myDatabase = QSqlDatabase();

	return bSuccess;
}

