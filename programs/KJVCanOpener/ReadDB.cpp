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
#include "SearchCompleter.h"

#include <assert.h>

#include <QFile>

#include <QtSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QObject>

#include <QString>

// Used for debugging:
#ifdef NEVER
#include <QTextStream>
#endif

QSqlDatabase g_sqldbReadMain;
QSqlDatabase g_sqldbReadUser;

namespace {
	const QString g_constrReadDatabase = QObject::tr("Reading Database");

	const QString g_constrDatabaseType = "QSQLITE";
	const QString g_constrMainReadConnection = "MainReadConnection";
	const QString g_constrUserReadConnection = "UserReadConnection";
}		// Namespace

// ============================================================================

CReadDatabase::CReadDatabase(QWidget *pParent)
	:	m_pParent(pParent)
{
	if (!g_sqldbReadMain.contains(g_constrMainReadConnection)) {
		g_sqldbReadMain = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrMainReadConnection);
	}

	if (!g_sqldbReadUser.contains(g_constrUserReadConnection)) {
		g_sqldbReadUser = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrUserReadConnection);
	}
}

CReadDatabase::~CReadDatabase()
{
	if (g_sqldbReadMain.contains(g_constrMainReadConnection)) {
		g_sqldbReadMain = QSqlDatabase();
		QSqlDatabase::removeDatabase(g_constrMainReadConnection);
	}

	if (g_sqldbReadUser.contains(g_constrUserReadConnection)) {
		g_sqldbReadUser = QSqlDatabase();
		QSqlDatabase::removeDatabase(g_constrUserReadConnection);
	}
}


// ============================================================================

bool CReadDatabase::ReadTestamentTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Testament Table

	QSqlQuery queryTable(m_myDatabase);

	// Check to see if the table exists:
	if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TESTAMENT'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"TESTAMENT\" Failed!\n%1").arg(queryTable.lastError().text()));
		return false;
	}
	queryTable.next();
	if (!queryTable.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"TESTAMENT\" Table in database!"));
		return false;
	}
	queryTable.finish();

	m_pBibleDatabase->m_lstTestaments.clear();

	QSqlQuery queryData(m_myDatabase);
	queryData.setForwardOnly(true);
	queryData.exec("SELECT * FROM TESTAMENT");
	while (queryData.next()) {
		unsigned int nTstNdx = queryData.value(0).toUInt();
		if (nTstNdx > m_pBibleDatabase->m_lstTestaments.size()) m_pBibleDatabase->m_lstTestaments.resize(nTstNdx);
		CTestamentEntry &entryTestament = m_pBibleDatabase->m_lstTestaments[nTstNdx-1];
		entryTestament.m_strTstName = queryData.value(1).toString();
	}
	queryData.finish();

	return true;
}

bool CReadDatabase::ReadBooksTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Books Table

	QSqlQuery queryTable(m_myDatabase);

	// Check to see if the table exists:
	if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TOC'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"TOC\" Failed!\n%1").arg(queryTable.lastError().text()));
		return false;
	}
	queryTable.next();
	if (!queryTable.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"TOC\" Table in database!"));
		return false;
	}
	queryTable.finish();

	m_pBibleDatabase->m_EntireBible = CBibleEntry();		// Clear out the main Bible entry, just in case we're being called a second time
	m_pBibleDatabase->m_EntireBible.m_nNumTst = m_pBibleDatabase->m_lstTestaments.size();

	m_pBibleDatabase->m_lstBooks.clear();

	QSqlQuery queryData(m_myDatabase);
	queryData.setForwardOnly(true);
	queryData.exec("SELECT * FROM TOC");
	while (queryData.next()) {
		unsigned int nBkNdx = queryData.value(0).toUInt();
		if (nBkNdx > m_pBibleDatabase->m_lstBooks.size()) m_pBibleDatabase->m_lstBooks.resize(nBkNdx);
		CBookEntry &entryBook = m_pBibleDatabase->m_lstBooks[nBkNdx-1];
		entryBook.m_nTstBkNdx = queryData.value(1).toUInt();
		entryBook.m_nTstNdx = queryData.value(2).toUInt();
		entryBook.m_strBkName = queryData.value(3).toString();
		entryBook.m_strBkAbbr = queryData.value(4).toString();
		entryBook.m_strTblName = queryData.value(5).toString();
		entryBook.m_nNumChp = queryData.value(6).toUInt();
		entryBook.m_nNumVrs = queryData.value(7).toUInt();
		entryBook.m_nNumWrd = queryData.value(8).toUInt();
		entryBook.m_strCat = queryData.value(9).toString();
		entryBook.m_strDesc = queryData.value(10).toString();

		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumBk++;
		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumChp += entryBook.m_nNumChp;
		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumVrs += entryBook.m_nNumVrs;
		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumWrd += entryBook.m_nNumWrd;

		m_pBibleDatabase->m_EntireBible.m_nNumBk++;
		m_pBibleDatabase->m_EntireBible.m_nNumChp += entryBook.m_nNumChp;
		m_pBibleDatabase->m_EntireBible.m_nNumVrs += entryBook.m_nNumVrs;
		m_pBibleDatabase->m_EntireBible.m_nNumWrd += entryBook.m_nNumWrd;
	}
	queryData.finish();

	// Calculate accumulated quick indexes.  Do this here in a separate loop in case database
	//		came to us out of order:
	unsigned int nWrdAccum = 0;
	for (unsigned int nBk = 0; nBk < m_pBibleDatabase->m_lstBooks.size(); ++nBk) {
		m_pBibleDatabase->m_lstBooks[nBk].m_nWrdAccum = nWrdAccum;
		nWrdAccum += m_pBibleDatabase->m_lstBooks[nBk].m_nNumWrd;
	}

	assert(nWrdAccum == m_pBibleDatabase->bibleEntry().m_nNumWrd);		// Our quick indexes should match the count of the Bible as a whole

// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		for (TBookList::const_iterator itr = m_pBibleDatabase->m_lstBooks.begin(); itr != m_pBibleDatabase->m_lstBooks.end(); ++itr) {
			QString strTemp = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10\r\n").arg(itr->m_nTstBkNdx).arg(itr->m_nTstNdx).arg(itr->m_strBkName).arg(itr->m_strBkAbbr)
									.arg(itr->m_strTblName).arg(itr->m_nNumChp).arg(itr->m_nNumVrs).arg(itr->m_nNumWrd).arg(itr->m_strCat).arg(itr->m_strDesc);
			ts << strTemp;
		}
		fileTest.close();
	}
#endif

	return true;
}

bool CReadDatabase::ReadChaptersTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Chapters (LAYOUT) table:

	QSqlQuery queryTable(m_myDatabase);

	// Check to see if the table exists:
	if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='LAYOUT'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"LAYOUT\" Failed!\n%1").arg(queryTable.lastError().text()));
		return false;
	}
	queryTable.next();
	if (!queryTable.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"LAYOUT\" Table in database!"));
		return false;
	}
	queryTable.finish();

	m_pBibleDatabase->m_mapChapters.clear();

	QSqlQuery queryData(m_myDatabase);
	queryData.setForwardOnly(true);
	queryData.exec("SELECT * FROM LAYOUT");
	while (queryData.next()) {
		uint32_t nBkChpNdx = queryData.value(0).toUInt();
		CChapterEntry &entryChapter = m_pBibleDatabase->m_mapChapters[CRelIndex(nBkChpNdx << 16)];
		entryChapter.m_nNumVrs = queryData.value(1).toUInt();
		entryChapter.m_nNumWrd = queryData.value(2).toUInt();
	}
	queryData.finish();

	// Calculate accumulated quick indexes.  Do this here in a separate loop in case database
	//		came to us out of order:
	unsigned int nWrdAccum = 0;
	for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->m_lstBooks.size(); ++nBk) {
		for (unsigned int nChp = 1; nChp <= m_pBibleDatabase->m_lstBooks[nBk-1].m_nNumChp; ++nChp) {
			m_pBibleDatabase->m_mapChapters[CRelIndex(nBk, nChp, 0, 0)].m_nWrdAccum = nWrdAccum;
			nWrdAccum += m_pBibleDatabase->m_mapChapters[CRelIndex(nBk, nChp, 0, 0)].m_nNumWrd;
		}
	}

	assert(nWrdAccum == m_pBibleDatabase->bibleEntry().m_nNumWrd);		// Our quick indexes should match the count of the Bible as a whole

// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		for (TChapterMap::const_iterator itr = m_pBibleDatabase->m_mapChapters.begin(); itr != m_pBibleDatabase->m_mapChapters.end(); ++itr) {
			QString strTemp = QString("%1,%2,%3\r\n").arg(itr->first.index()>>16).arg(itr->second.m_nNumVrs).arg(itr->second.m_nNumWrd);
			ts << strTemp;
		}
		fileTest.close();
	}
#endif

	return true;
}

bool CReadDatabase::ReadVerseTables()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Book Verses tables:

	assert(m_pBibleDatabase->m_lstBooks.size() != 0);		// Must read BookEntries before BookVerses

	unsigned int nWrdAccum = 0;

	m_pBibleDatabase->m_lstBookVerses.clear();
	m_pBibleDatabase->m_lstBookVerses.resize(m_pBibleDatabase->m_lstBooks.size());
	for (unsigned int nBk=1; nBk<=m_pBibleDatabase->m_lstBooks.size(); ++nBk) {
		QSqlQuery queryTable(m_myDatabase);

		// Check to see if the table exists:
		if (!queryTable.exec(QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(m_pBibleDatabase->m_lstBooks[nBk-1].m_strTblName))) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"%1\" Failed!\n%2").arg(m_pBibleDatabase->m_lstBooks[nBk-1].m_strTblName).arg(queryTable.lastError().text()));
			return false;
		}
		queryTable.next();
		if (!queryTable.value(0).toInt()) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"%1\" Table in database!").arg(m_pBibleDatabase->m_lstBooks[nBk-1].m_strTblName));
			return false;
		}
		queryTable.finish();

		TVerseEntryMap &mapVerses = m_pBibleDatabase->m_lstBookVerses[nBk-1];
		mapVerses.clear();

		QSqlQuery queryData(m_myDatabase);
		queryData.setForwardOnly(true);
		queryData.exec(QString("SELECT * FROM %1").arg(m_pBibleDatabase->m_lstBooks[nBk-1].m_strTblName));
		while (queryData.next()) {
			QString strVerseText;
			uint32_t nChpVrsNdx = queryData.value(0).toUInt();
			CVerseEntry &entryVerse = mapVerses[CRelIndex(nChpVrsNdx << 8)];
			entryVerse.m_nNumWrd = queryData.value(1).toUInt();
			entryVerse.m_nPilcrow = static_cast<CVerseEntry::PILCROW_TYPE_ENUM>(queryData.value(2).toInt());
			strVerseText = queryData.value(4).toString();
			if (strVerseText.isEmpty()) strVerseText = queryData.value(3).toString();
			entryVerse.m_strTemplate = queryData.value(5).toString();
		}
		queryData.finish();

		// Calculate accumulated quick indexes.  Do this here in a separate loop in case database
		//		came to us out of order:
		for (unsigned int nChp = 1; nChp <= m_pBibleDatabase->m_lstBooks[nBk-1].m_nNumChp; ++nChp) {
			unsigned int nNumVerses = m_pBibleDatabase->m_mapChapters[CRelIndex(nBk, nChp, 0, 0)].m_nNumVrs;
			for (unsigned int nVrs = 1; nVrs <= nNumVerses; ++nVrs) {
				mapVerses[CRelIndex(0, nChp, nVrs, 0)].m_nWrdAccum = nWrdAccum;
				nWrdAccum += mapVerses[CRelIndex(0, nChp, nVrs, 0)].m_nNumWrd;
			}
		}

// Used for debugging:
#ifdef NEVER
		QFile fileTest(QString("testit%1.txt").arg(nBk, 2, 10, QChar('0')));
		if (fileTest.open(QIODevice::WriteOnly)) {
			CCSVStream csv(&fileTest);
			for (TVerseEntryMap::const_iterator itr = mapVerses.begin(); itr != mapVerses.end(); ++itr) {
				QStringList sl;
				sl.push_back(QString("%1").arg(itr->first.index()>>8));
				sl.push_back(QString("%1").arg(itr->second.m_nNumWrd));
				sl.push_back(QString("%1").arg(itr->second.m_nPilcrow));
				sl.push_back(itr->second.m_strTemplate);
				csv << sl;
			}
			fileTest.close();
		}
#endif

	}

	assert(nWrdAccum == m_pBibleDatabase->bibleEntry().m_nNumWrd);		// Our quick indexes should match the count of the Bible as a whole

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

static bool ascendingLessThanStrings(const QString &s1, const QString &s2)
{
	return (s1.compare(s2, Qt::CaseInsensitive) < 0);
}

bool CReadDatabase::ReadWordsTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Words table:

	QSqlQuery queryTable(m_myDatabase);

	// Check to see if the table exists:
	if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='WORDS'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"WORDS\" Failed!\n%1").arg(queryTable.lastError().text()));
		return false;
	}
	queryTable.next();
	if (!queryTable.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"WORDS\" Table in database!"));
		return false;
	}
	queryTable.finish();

	unsigned int nNumWordsInText = 0;
	for (unsigned int ndxBook=0; ndxBook<m_pBibleDatabase->m_lstBooks.size(); ++ndxBook) {
		nNumWordsInText += m_pBibleDatabase->m_lstBooks[ndxBook].m_nNumWrd;
	}

	m_pBibleDatabase->m_mapWordList.clear();
	m_pBibleDatabase->m_lstWordList.clear();
	m_pBibleDatabase->m_lstConcordanceWords.clear();
	m_pBibleDatabase->m_lstConcordanceMapping.clear();
	m_pBibleDatabase->m_lstConcordanceMapping.resize(nNumWordsInText+1);			// Preallocate our concordance mapping as we know how many words the text contains (+1 for zero position)
	int nConcordanceCount = 0;			// Count of words so we can preallocate our buffers

	QSqlQuery queryData(m_myDatabase);
	queryData.setForwardOnly(true);
	queryData.exec("SELECT * FROM WORDS");
	while (queryData.next()) {
		QString strWord = queryData.value(1).toString();
		bool bCasePreserve = ((queryData.value(2).toInt()) ? true : false);
// TODO : CLEAN
//		QString strKey = strWord.toLower().normalized(QString::NormalizationForm_C);
		QString strKey = CSearchStringListModel::decompose(strWord).toLower();
		CWordEntry &entryWord = m_pBibleDatabase->m_mapWordList[strKey];
		if (!m_pBibleDatabase->m_lstWordList.contains(strKey))			// This check is needed because duplicates can happen from decomposed index keys
			m_pBibleDatabase->m_lstWordList.append(strKey);

		if (entryWord.m_strWord.isEmpty()) {
			entryWord.m_strWord = strKey;
			entryWord.m_bCasePreserve = bCasePreserve;
		} else {
			// If folding duplicate words into single entry from decomposed indexes,
			//		they better be the same exact word:
			assert(entryWord.m_strWord.compare(strKey) == 0);
			assert(entryWord.m_bCasePreserve == bCasePreserve);
			if ((entryWord.m_strWord.compare(strKey) != 0) || (entryWord.m_bCasePreserve != bCasePreserve)) {
				QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Non-unique decomposed word entry error in WORDS table!\n\nWord: \"%1\" with Word: \"%2\"").arg(strWord).arg(entryWord.m_strWord));
				return false;
			}
		}

		QString strAltWords = queryData.value(4).toString();
		CCSVStream csvWord(&strAltWords, QIODevice::ReadOnly);
		while (!csvWord.atEndOfStream()) {
			QString strTemp;
			csvWord >> strTemp;
			if (!strTemp.isEmpty()) entryWord.m_lstAltWords.push_back(strTemp.normalized(QString::NormalizationForm_C));
		}
		QString strAltWordCounts = queryData.value(5).toString();
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
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Mismatch Word Counts for \"%1\" AltWords=%2, AltWordCounts=%3")
							.arg(strWord).arg(entryWord.m_lstAltWords.size()).arg(entryWord.m_lstAltWordCount.size()));
			return false;
		}
		if (nAltCount != queryData.value(3).toUInt()) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Bad AltWordCounts for \"%1\"").arg(strWord));
			return false;
		}
		nConcordanceCount += entryWord.m_lstAltWords.size();		// Note: nConcordanceCount will be slightly too large due to folding of duplicate decomposed indexes, but is sufficient for a reserve()

		TIndexList lstNormalIndexes;
		if (!IndexBlobToIndexList(queryData.value(6).toByteArray(), lstNormalIndexes)) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Bad word indexes for \"%1\"").arg(strWord));
			return false;
		}
		if (lstNormalIndexes.size() != queryData.value(3).toUInt()) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Index/Count consistency error in WORDS table!"));
			return false;
		}
		entryWord.m_ndxNormalizedMapping.insert(entryWord.m_ndxNormalizedMapping.end(), lstNormalIndexes.begin(), lstNormalIndexes.end());
	}
	queryData.finish();

	m_pBibleDatabase->m_lstConcordanceWords.reserve(nConcordanceCount);
	int ndxWord = 0;

	// The following has to be done in a separate loop if we allow duplicate index
	//		key folding caused by decomposing the indexes.  Otherwise, our word indexes
	//		in this loop won't match the word indexes in the loop after the sort!:
	for (TWordListMap::const_iterator itrWordEntry = m_pBibleDatabase->m_mapWordList.begin(); itrWordEntry != m_pBibleDatabase->m_mapWordList.end(); ++itrWordEntry) {
		const CWordEntry &entryWord(itrWordEntry->second);
		// Add this word and alternates to our concordance, and we'll set the normalized indices that refer to it to point
		//		to the specific word below after we've sorted the concordance list.  This sorting allows us to optimize
		//		the completer list and the FindWords sorting:
		for (int ndxAltWord=0; ndxAltWord<entryWord.m_lstAltWords.size(); ++ndxAltWord) {
			QString strAltWord = entryWord.m_lstAltWords.at(ndxAltWord);
			CConcordanceEntry entryConcordance(strAltWord, ndxWord);
			m_pBibleDatabase->m_lstConcordanceWords.append(entryConcordance);
			ndxWord++;
		}
	}

	// Sort all of our word forms since the alternates may sort different with
	//		with respect to the other words:
	qSort(m_pBibleDatabase->m_lstWordList.begin(), m_pBibleDatabase->m_lstWordList.end(), ascendingLessThanStrings);
	qSort(m_pBibleDatabase->m_lstConcordanceWords.begin(), m_pBibleDatabase->m_lstConcordanceWords.end(), TConcordanceListSortPredicate::ascendingLessThanWordCaseInsensitive);

	assert(m_pBibleDatabase->m_lstWordList.size() == static_cast<int>(m_pBibleDatabase->m_mapWordList.size()));

	// Now that we have the sorted indexes, we need to remap back to what came from what for our mapping:
	QVector<int> lstSortIndex;
	lstSortIndex.resize(m_pBibleDatabase->m_lstConcordanceWords.size());
	for (int i = 0; i<m_pBibleDatabase->m_lstConcordanceWords.size(); ++i)
		lstSortIndex[m_pBibleDatabase->m_lstConcordanceWords.at(i).index()] = i;

	ndxWord = 0;

	for (TWordListMap::const_iterator itrWordEntry = m_pBibleDatabase->m_mapWordList.begin(); itrWordEntry != m_pBibleDatabase->m_mapWordList.end(); ++itrWordEntry) {
		const CWordEntry &entryWord(itrWordEntry->second);

		// Now that we've built the concordance list and have sorted it, we'll set our normalized indices:
		unsigned int ndxMapping = 0;
		for (int ndxAltWord=0; ndxAltWord<entryWord.m_lstAltWords.size(); ++ndxAltWord) {
			assert(ndxWord < m_pBibleDatabase->m_lstConcordanceWords.size());
			for (unsigned int ndxAltCount=0; ndxAltCount<entryWord.m_lstAltWordCount.at(ndxAltWord); ++ndxAltCount) {
				assert(ndxMapping < entryWord.m_ndxNormalizedMapping.size());
				if (entryWord.m_ndxNormalizedMapping.at(ndxMapping) > nNumWordsInText) {
					QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Invalid WORDS mapping.  Check database integrity!\n\nWord: \"%1\"  Index: %2").arg(entryWord.m_lstAltWords.at(ndxAltWord)).arg(entryWord.m_ndxNormalizedMapping.at(ndxMapping)));
					return false;
				}
				m_pBibleDatabase->m_lstConcordanceMapping[entryWord.m_ndxNormalizedMapping.at(ndxMapping)] = lstSortIndex.at(ndxWord);
				ndxMapping++;
			}
			ndxWord++;
		}
	}
	assert(m_pBibleDatabase->m_lstConcordanceMapping.size() == (nNumWordsInText + 1));
	assert(ndxWord == m_pBibleDatabase->m_lstConcordanceWords.size());


// Code to dump all of the SoundEx values for the words in this database, used
//	to verify SoundEx equations:
#ifdef DUMP_SOUNDEX_LIST
	QFile fileSoundEx("soundex.txt");
	fileSoundEx.open(QIODevice::WriteOnly);
	fileSoundEx.write(QString(QChar(0xFEFF)).toUtf8());
	QTextStream ts(&fileSoundEx);

	for (TWordListMap::const_iterator itrWordEntry = m_pBibleDatabase->m_mapWordList.begin(); itrWordEntry != m_pBibleDatabase->m_mapWordList.end(); ++itrWordEntry) {
		const CWordEntry &entryWord(itrWordEntry->second);

		ts << entryWord.m_strWord << "," << CSoundExSearchCompleterFilter::soundEx(entryWord.m_strWord) << "\n";
	}

	fileSoundEx.close();
#endif


// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		int cnt = 0;
		for (TWordListMap::const_iterator itr = m_pBibleDatabase->m_mapWordList.begin(); itr != m_pBibleDatabase->m_mapWordList.end(); ++itr) {
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
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Footnotes table:

	QSqlQuery queryTable(m_myDatabase);

	// Check to see if the table exists:
	if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='FOOTNOTES'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"FOOTNOTES\" Failed!\n%1").arg(queryTable.lastError().text()));
		return false;
	}
	queryTable.next();
	if (!queryTable.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"FOOTNOTES\" Table in database!"));
		return false;
	}
	queryTable.finish();

	m_pBibleDatabase->m_mapFootnotes.clear();

	QSqlQuery queryData(m_myDatabase);
	queryData.setForwardOnly(true);
	queryData.exec("SELECT * FROM FOOTNOTES");
	while (queryData.next()) {
		QString strFootnoteText;
		CFootnoteEntry footnote;
		CRelIndex ndxRel(queryData.value(0).toUInt());
		assert(ndxRel.isSet());
		if (!ndxRel.isSet()) continue;
		strFootnoteText = queryData.value(2).toString();
		if (strFootnoteText.isEmpty()) strFootnoteText = queryData.value(1).toString();
		if (!strFootnoteText.isEmpty()) {
			footnote.setText(strFootnoteText);
			m_pBibleDatabase->m_mapFootnotes[ndxRel] = footnote;
		}
	}
	queryData.finish();

	return true;
}

bool CReadDatabase::ReadPHRASESTable(bool bUserPhrases)
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Phrases table:

	QSqlQuery queryTable(m_myDatabase);

	// Check to see if the table exists:
	if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='PHRASES'")) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"PHRASES\" Failed!\n%1").arg(queryTable.lastError().text()));
		return false;
	}
	queryTable.next();
	if (!queryTable.value(0).toInt()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"PHRASES\" Table in database!"));
		return false;
	}
	queryTable.finish();

	if (bUserPhrases) {
		g_lstUserPhrases.clear();
	} else {
		m_pBibleDatabase->m_lstCommonPhrases.clear();
	}

	QSqlQuery queryData(m_myDatabase);
	queryData.setForwardOnly(true);
	queryData.exec("SELECT * FROM PHRASES");
	while (queryData.next()) {
		CPhraseEntry phrase;
		phrase.setText(queryData.value(1).toString());
		phrase.setCaseSensitive((queryData.value(2).toInt() != 0) ? true : false);
		phrase.setAccentSensitive((queryData.value(3).toInt() != 0) ? true : false);
		if (!phrase.text().isEmpty()) {
			if (bUserPhrases) {
				g_lstUserPhrases.push_back(phrase);
			} else {
				m_pBibleDatabase->m_lstCommonPhrases.push_back(phrase);
			}
		}
	}
	queryData.finish();

	return true;
}

bool CReadDatabase::ValidateData()
{
	assert(m_pBibleDatabase.data() != NULL);

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

	if (m_pBibleDatabase->m_lstBookVerses.size() != m_pBibleDatabase->m_lstBooks.size()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book List and Table of Contents have different sizes!\nCheck the database!"));
		return false;
	}

	ncntTstTot = m_pBibleDatabase->m_lstTestaments.size();
	for (unsigned int nBk=0; nBk<m_pBibleDatabase->m_lstBooks.size(); ++ nBk) {		// Books
		if ((m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx < 1) || (m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx > ncntTstTot)) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) References Invalid Testament %3")
								.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx));
			return false;
		}
		ncntChp_Bk = 0;
		ncntVrs_Bk = 0;
		ncntWrd_Bk = 0;
		ncntBkTot++;
		for (unsigned int nChp=0; nChp<m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp; ++nChp) {	// Chapters
			ncntVrs_Chp = 0;
			ncntWrd_Chp = 0;
			TChapterMap::const_iterator itrChapters = m_pBibleDatabase->m_mapChapters.find(CRelIndex(nBk+1,nChp+1,0,0));
			if (itrChapters == m_pBibleDatabase->m_mapChapters.end()) continue;
			ncntChpTot++;
			ncntChp_Bk++;
			for (unsigned int nVrs=0; nVrs<itrChapters->second.m_nNumVrs; ++nVrs) {	// Verses
				ncntWrd_Vrs = 0;
				const TVerseEntryMap &aBookVerses = m_pBibleDatabase->m_lstBookVerses[nBk];
				TVerseEntryMap::const_iterator itrBook = aBookVerses.find(CRelIndex(0,nChp+1,nVrs+1,0));
				if (itrBook == aBookVerses.end()) continue;
				ncntVrsTot++;
				ncntVrs_Chp++;
				ncntVrs_Bk++;
				ncntWrdTot += itrBook->second.m_nNumWrd;			// Words
				ncntWrd_Vrs += itrBook->second.m_nNumWrd;
				ncntWrd_Chp += itrBook->second.m_nNumWrd;
				ncntWrd_Bk += itrBook->second.m_nNumWrd;
			}
			if (ncntVrs_Chp != itrChapters->second.m_nNumVrs) {
				QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) Chapter %3 contains %4 Verses, expected %5 Verses!")
									.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(nChp+1).arg(ncntVrs_Chp).arg(itrChapters->second.m_nNumVrs));
				return false;
			}
			if (ncntWrd_Chp != itrChapters->second.m_nNumWrd) {
				QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) Chapter %3 contains %4 Words, expected %5 Words!")
									.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(nChp+1).arg(ncntWrd_Chp).arg(itrChapters->second.m_nNumWrd));
				return false;
			}
		}
		if (ncntChp_Bk != m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Chapters, expected %4 Chapters!")
									.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(ncntChp_Bk).arg(m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp));
			return false;
		}
		if (ncntVrs_Bk != m_pBibleDatabase->m_lstBooks[nBk].m_nNumVrs) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Verses, expected %4 Verses!")
									.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(ncntVrs_Bk).arg(m_pBibleDatabase->m_lstBooks[nBk].m_nNumVrs));
			return false;
		}
		if (ncntWrd_Bk != m_pBibleDatabase->m_lstBooks[nBk].m_nNumWrd) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Words, expected %4 Words!")
									.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(ncntWrd_Bk).arg(m_pBibleDatabase->m_lstBooks[nBk].m_nNumWrd));
			return false;
		}
	}

	unsigned int nWordListTot = 0;
	for (TWordListMap::const_iterator itrWords = m_pBibleDatabase->m_mapWordList.begin(); itrWords != m_pBibleDatabase->m_mapWordList.end(); ++itrWords) {
		nWordListTot += itrWords->second.m_ndxNormalizedMapping.size();
	}
	if (nWordListTot != ncntWrdTot) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Word List contains %1 indexes, expected %2!").arg(nWordListTot).arg(ncntWrdTot));
		return false;
	}

	// Check concordance:
	if ((nWordListTot+1) != m_pBibleDatabase->m_lstConcordanceMapping.size()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Word List contains %1 indexes, but Concordance Mapping contains %2 entries!").arg(nWordListTot+1).arg(m_pBibleDatabase->m_lstConcordanceMapping.size()));
		return false;
	}

	// Check overall count values:
	if ((ncntTstTot != m_pBibleDatabase->bibleEntry().m_nNumTst) ||
		(ncntBkTot != m_pBibleDatabase->bibleEntry().m_nNumBk) ||
		(ncntChpTot != m_pBibleDatabase->bibleEntry().m_nNumChp) ||
		(ncntVrsTot != m_pBibleDatabase->bibleEntry().m_nNumVrs) ||
		(ncntWrdTot != m_pBibleDatabase->bibleEntry().m_nNumWrd)) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Overall Bible Entry Data Counts are inconsistent!  Check database!"));
		return false;
	}

	// Check Normalize/Denormalize functions:
#ifdef TEST_INDEXING
	for (unsigned int nWrd = 1; nWrd <= m_pBibleDatabase->bibleEntry().m_nNumWrd; ++nWrd) {
		uint32_t ndxRel = m_pBibleDatabase->DenormalizeIndex(nWrd);
		if (m_pBibleDatabase->NormalizeIndex(ndxRel) != nWrd) {
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Normalize/Denormalize Index Check Failed!\n\nNormal->Relative->Normal:\n%1->%2->%3").arg(nWrd).arg(ndxRel).arg(m_pBibleDatabase->NormalizeIndex(ndxRel)));
			assert(false);
		}
	}
#endif

	return true;
}

// ============================================================================

bool CReadDatabase::ReadDatabase(const QString &strDatabaseFilename, const QString &strName, const QString &strDescription, bool bSetAsMain)
{
	m_myDatabase = g_sqldbReadMain;
	m_myDatabase.setDatabaseName(strDatabaseFilename);
	m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

//	QMessageBox::information(m_pParent, g_constrReadDatabase, m_myDatabase.databaseName());

	m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(strName, strDescription));
	assert(m_pBibleDatabase.data() != NULL);

	if (!m_myDatabase.open()) {
		QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open database file \"%1\".\n\n%2").arg(strDatabaseFilename).arg(m_myDatabase.lastError().text()));
		return false;
	}

	bool bSuccess = true;

	if ((!ReadTestamentTable()) ||
		(!ReadBooksTable()) ||
		(!ReadChaptersTable()) ||
		(!ReadVerseTables()) ||
		(!ReadWordsTable()) ||
		(!ReadFOOTNOTESTable()) ||
		(!ReadPHRASESTable(false)) ||
		(!ValidateData())) bSuccess = false;

	m_myDatabase.close();
	m_myDatabase = QSqlDatabase();

	if (bSuccess) {
		g_lstBibleDatabases.push_back(m_pBibleDatabase);
		if (bSetAsMain) g_pMainBibleDatabase = m_pBibleDatabase;
	}

	return bSuccess;
}

bool CReadDatabase::ReadUserDatabase(const QString &strDatabaseFilename, bool bHideWarnings)
{
	m_myDatabase = g_sqldbReadUser;
	m_myDatabase.setDatabaseName(strDatabaseFilename);
	m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!m_myDatabase.open()) {
		if (!bHideWarnings)
			QMessageBox::warning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open database file \"%1\".\n\n%2").arg(strDatabaseFilename).arg(m_myDatabase.lastError().text()));
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

