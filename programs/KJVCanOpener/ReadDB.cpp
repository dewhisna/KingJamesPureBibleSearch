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
#include "ReportError.h"

#include <assert.h>

#ifndef NOT_USING_SQL
#include <QtSql>
#include <QSqlQuery>
#endif

#include <QObject>
#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QtIOCompressor>

#ifdef Q_OS_ANDROID
#include <android/log.h>
#endif

// Used for debugging:
#ifdef NEVER
#include <QTextStream>
#endif

// ============================================================================

namespace {
	const QString g_constrReadDatabase = QObject::tr("Reading Database");

#ifndef NOT_USING_SQL
	const QString g_constrDatabaseType = "QSQLITE";
	const QString g_constrMainReadConnection = "MainReadConnection";
	const QString g_constrUserReadConnection = "UserReadConnection";
#endif

}		// Namespace

// ============================================================================

static void convertIndexListToNormalIndexes(const QString &strIndexList, TNormalizedIndexList &anNormalIndexList)
{
	QStringList lstIndex = strIndexList.split(",");
	anNormalIndexList.clear();
	anNormalIndexList.reserve(lstIndex.size());
	for (int ndx = 0; ndx < lstIndex.size(); ++ndx) {
		anNormalIndexList.push_back(lstIndex.at(ndx).toUInt());
	}
}

#ifndef NOT_USING_SQL
static QString convertIndexBlobToIndexList(const QByteArray &baBlob)
{
	QStringList lstIndexes;

	if ((baBlob.size() % sizeof(uint32_t)) != 0) return QString();

	lstIndexes.reserve(baBlob.size() / sizeof(uint32_t));

	for (unsigned int i=0; i<(baBlob.size()/sizeof(uint32_t)); ++i) {
		uint32_t nValue = 0;
		for (unsigned int j=0; j<sizeof(uint32_t); ++j) {
			nValue = nValue << 8;
			nValue = nValue | (baBlob[static_cast<unsigned int>((i*sizeof(uint32_t))+j)] & 0xFF);
		}
		lstIndexes.append(QString("%1").arg(nValue));
	}

	return lstIndexes.join(",");
}

static bool queryFieldsToStringList(QWidget *pParent, QStringList &lstFields, const QSqlQuery &query, int nMinFields, bool bBlobsAreIndexes = false)
{
	lstFields.clear();
	int nCount = query.record().count();
	lstFields.reserve(nCount);
	for (int ndx = 0; ndx < nCount; ++ndx) {
		QVariant varValue = query.value(ndx);
		if (varValue.type() == QVariant::Bool) {
			lstFields.append(QString(varValue.toBool() ? "1" : "0"));
		} else if (varValue.type() == QVariant::ByteArray) {
			if (bBlobsAreIndexes) {
				lstFields.append(convertIndexBlobToIndexList(varValue.toByteArray()));
			} else {
				lstFields.append(varValue.toString());
			}
		} else {
			lstFields.append(varValue.toString());
		}
	}

	if (lstFields.size() < nMinFields) {
		displayWarning(pParent, g_constrReadDatabase, QObject::tr("Bad Record in Database, expected at least %1 field(s):\n\"%2\"").arg(nMinFields).arg(lstFields.join(",")));
		return false;
	}

	return true;
}
#endif	// !NOT_USING_SQL

static bool readCCDatabaseRecord(QWidget *pParent, QStringList &lstFields, CCSVStream *pStream, int nMinFields)
{
	lstFields.clear();
	if (!pStream->atEndOfStream()) {
		(*pStream) >> lstFields;
	} else {
		displayWarning(pParent, g_constrReadDatabase, QObject::tr("Unexpected end of Bible CCDatabase file"));
		return false;
	}

	if (lstFields.size() < nMinFields) {
		displayWarning(pParent, g_constrReadDatabase, QObject::tr("Bad Record in Database, expected at least %1 field(s):\n\"%2\"").arg(nMinFields).arg(lstFields.join(",")));
		return false;
	}

	return true;
}

// ============================================================================

class CDBTableParser
{
public:
#ifndef NOT_USING_SQL
	CDBTableParser(QWidget *pParentWidget, CCSVStream *pCSVStream, QSqlDatabase &sqlDatabase)
		:	m_nRecordCount(0),
			m_bContinue(false),
			m_pParentWidget(pParentWidget),
			m_pCSVStream(pCSVStream),
			m_myDatabase(sqlDatabase),
			m_queryData(sqlDatabase)
	{

	}

#else
	CDBTableParser(QWidget *pParentWidget, CCSVStream *pCSVStream)
		:	m_nRecordCount(0),
			m_bContinue(false),
			m_pParentWidget(pParentWidget),
			m_pCSVStream(pCSVStream)
	{

	}

#endif	// !NOT_USING_SQL
	bool findTable(const QString &strTableName)
	{
		m_strTableName = strTableName;

		QStringList lstFields;
		m_nRecordCount = 0;

		if (m_pCSVStream != NULL) {
			if (!readCCDatabaseRecord(m_pParentWidget, lstFields, m_pCSVStream, 2)) return false;
			// Format:  <TABLE>,count
			if ((lstFields.size() != 2) ||
				(lstFields.at(0) != strTableName)) {
				displayWarning(m_pParentWidget, g_constrReadDatabase, QObject::tr("Invalid %1 section header in CCDatabase\n\n%2").arg(strTableName).arg(lstFields.join(",")));
				return false;
			}
			m_nRecordCount = lstFields.at(1).toInt();
		} else {
#ifndef NOT_USING_SQL
			QSqlQuery queryTable(m_myDatabase);

			// Check to see if the table exists:
			if (!queryTable.exec(QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(strTableName))) {
				displayWarning(m_pParentWidget, g_constrReadDatabase, QObject::tr("Table Lookup for \"%1\" Failed!\n%2").arg(strTableName).arg(queryTable.lastError().text()));
				return false;
			}
			queryTable.next();
			if (!queryTable.value(0).toInt()) {
				displayWarning(m_pParentWidget, g_constrReadDatabase, QObject::tr("Unable to find \"%1\" Table in database!").arg(strTableName));
				return false;
			}
			queryTable.finish();
#else
			return false;
#endif	// !NOT_USING_SQL
		}

		return true;
	}

	void startQueryLoop(const QString &strFieldNameList)
	{
		assert(!m_strTableName.isEmpty());
		m_bContinue = false;
		if (m_pCSVStream != NULL) {
			m_bContinue = (m_nRecordCount > 0);
		} else {
#ifndef NOT_USING_SQL
			m_queryData.setForwardOnly(true);
			m_queryData.exec(QString("SELECT %1 FROM %2").arg(strFieldNameList).arg(m_strTableName));
			m_bContinue = m_queryData.next();
#else
			Q_UNUSED(strFieldNameList);
#endif	// !NOT_USING_SQL
		}
	}

	bool haveData() const
	{
		return m_bContinue;
	}

	bool readNextRecord(QStringList &lstFields, int nMinFields, bool bTreatBlobsAsIndexes = false)
	{
		if (m_pCSVStream != NULL) {
			if (!readCCDatabaseRecord(m_pParentWidget, lstFields, m_pCSVStream, nMinFields)) return false;
			--m_nRecordCount;
			m_bContinue = (m_nRecordCount > 0);
		} else {
#ifndef NOT_USING_SQL
			if (!queryFieldsToStringList(m_pParentWidget, lstFields, m_queryData, nMinFields, bTreatBlobsAsIndexes)) return false;
			m_bContinue = m_queryData.next();
#else
			Q_UNUSED(bTreatBlobsAsIndexes);
#endif	// !NOT_USING_SQL
		}

		return true;
	}

	void endQueryLoop()
	{
#ifndef NOT_USING_SQL
		if (m_pCSVStream == NULL) m_queryData.finish();
#endif	// !NOT_USING_SQL
	}

private:
	int m_nRecordCount;
	bool m_bContinue;
	QString m_strTableName;
	QWidget *m_pParentWidget;
	CCSVStream *m_pCSVStream;
#ifndef NOT_USING_SQL
	QSqlDatabase &m_myDatabase;
	QSqlQuery m_queryData;
#endif	// !NOT_USING_SQL
};

// ============================================================================

CReadDatabase::CReadDatabase(const QString &strBibleDBPath, const QString &strDictionaryDBPath, QWidget *pParent)
	:	m_pParent(pParent),
		m_strBibleDatabasePath(strBibleDBPath),
		m_strDictionaryDatabasePath(strDictionaryDBPath)
{

}

CReadDatabase::~CReadDatabase()
{
#ifndef NOT_USING_SQL
	if (QSqlDatabase::contains(g_constrMainReadConnection)) QSqlDatabase::removeDatabase(g_constrMainReadConnection);
	if (QSqlDatabase::contains(g_constrUserReadConnection)) QSqlDatabase::removeDatabase(g_constrUserReadConnection);
#endif
}

// ============================================================================

bool CReadDatabase::ReadDBInfoTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Database Info Table:

	bool bDBInfoGood = true;
	QString strError;
	QStringList lstFields;

	if (m_pCCDatabase.data() != NULL) {
		if (!readCCDatabaseRecord(m_pParent, lstFields, m_pCCDatabase.data(), 2)) return false;
	} else {
#ifndef NOT_USING_SQL
		QSqlQuery queryTable(m_myDatabase);

		// Check to see if the table exists:
		if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='DBInfo'")) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"DBInfo\" Failed!\n%1").arg(queryTable.lastError().text()));
			return false;
		}
		queryTable.next();
		if (!queryTable.value(0).toInt()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"DBInfo\" Table in database!"));
			return false;
		}
		queryTable.finish();

		QSqlQuery queryData(m_myDatabase);
		queryData.setForwardOnly(true);
		queryData.exec("SELECT ndx, uuid, Language, Name, Description, Info FROM DBInfo");

		if ((!queryData.next()) || (queryData.value(0).toUInt() != 1)) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database DBInfo Index");
		} else {
			if (!queryFieldsToStringList(m_pParent, lstFields, queryData, 6)) return false;
			// Convert from: ndx, $uuid, $Language, $Name, $Description, $Info
			// to Format:  KJPBSDB, version, $uuid, $Language, $Name, $Description, $Info
			// Note: s3db doesn't currently support the version value and is always '1'
			//			unless we do something in the future to add/change that:
			lstFields[0] = QString("1");		// Convert the ndx to the version
			lstFields.insert(0, "KJPBSDB");		// Add our special "magic" ID
		}
		queryData.finish();
#else
		bDBInfoGood = false;
		strError = QObject::tr("No database reading DBInfo");
#endif	// !NOT_USING_SQL
	}

	if (bDBInfoGood) {
		if ((lstFields.size() < 2) ||						// Must have a minimum of 2 fields until we figure out the version number, then compare from there based on version
			(lstFields.at(0) != "KJPBSDB")) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Database Header/DBInfo record");
		} else if (lstFields.at(1).toInt() != KJPBS_CCDB_VERSION) {
			bDBInfoGood = false;
			strError = QObject::tr("Unsupported KJPBS Database Version %1").arg(lstFields.at(1));
		} else if (lstFields.size() != 7) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Database Header/DBInfo record for the version (%1) it specifies").arg(lstFields.at(1));
		} else if (lstFields.at(2).isEmpty()) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database Compatibility UUID");
		} else if (lstFields.at(3).isEmpty()) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database Language Identifier");
		} else if (lstFields.at(4).isEmpty()) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database Name");
		} else {
			m_pBibleDatabase->m_strCompatibilityUUID = lstFields.at(2);
			m_pBibleDatabase->m_strLanguage = lstFields.at(3);
			m_pBibleDatabase->m_strName = lstFields.at(4);
			m_pBibleDatabase->m_strDescription = lstFields.at(5);
			m_pBibleDatabase->m_strInfo = lstFields.at(6);
		}
	}

	if (!bDBInfoGood) {
		displayWarning(m_pParent, g_constrReadDatabase, strError);
	}

	return bDBInfoGood;
}

bool CReadDatabase::ReadTestamentTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Testament Table

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("TESTAMENT")) return false;

	m_pBibleDatabase->m_lstTestaments.clear();

	dbParser.startQueryLoop("TstNdx, TstName");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 2)) return false;

		unsigned int nTstNdx = lstFields.at(0).toUInt();
		if (nTstNdx > m_pBibleDatabase->m_lstTestaments.size()) m_pBibleDatabase->m_lstTestaments.resize(nTstNdx);
		CTestamentEntry &entryTestament = m_pBibleDatabase->m_lstTestaments[nTstNdx-1];
		entryTestament.m_strTstName = lstFields.at(1);
	}

	dbParser.endQueryLoop();

	return true;
}

bool CReadDatabase::ReadBooksTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Books Table

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("TOC")) return false;

	m_pBibleDatabase->m_EntireBible = CBibleEntry();		// Clear out the main Bible entry, just in case we're being called a second time
	m_pBibleDatabase->m_EntireBible.m_nNumTst = m_pBibleDatabase->m_lstTestaments.size();
	m_pBibleDatabase->m_lstBooks.clear();

	dbParser.startQueryLoop("BkNdx, TstBkNdx, TstNdx, BkName, BkAbbr, TblName, NumChp, NumVrs, NumWrd, Cat, Desc");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 11)) return false;

		unsigned int nBkNdx = lstFields.at(0).toUInt();
		if (nBkNdx > m_pBibleDatabase->m_lstBooks.size()) m_pBibleDatabase->m_lstBooks.resize(nBkNdx);
		CBookEntry &entryBook = m_pBibleDatabase->m_lstBooks[nBkNdx-1];
		entryBook.m_nTstBkNdx = lstFields.at(1).toUInt();
		entryBook.m_nTstNdx = lstFields.at(2).toUInt();
		entryBook.m_strBkName = lstFields.at(3);
		entryBook.m_lstBkAbbr = lstFields.at(4).split(QChar(';'), QString::SkipEmptyParts);
		entryBook.m_strTblName = lstFields.at(5);
		entryBook.m_nNumChp = lstFields.at(6).toUInt();
		entryBook.m_nNumVrs = lstFields.at(7).toUInt();
		entryBook.m_nNumWrd = lstFields.at(8).toUInt();
		QString strCategory = lstFields.at(9);
		entryBook.m_strDesc = lstFields.at(10);

		TBookCategoryList::iterator itrCat = m_pBibleDatabase->m_lstBookCategories.begin();
		while (itrCat != m_pBibleDatabase->m_lstBookCategories.end()) {
			if (itrCat->m_strCategoryName.compare(strCategory) == 0) break;
			++itrCat;
		}
		if (itrCat == m_pBibleDatabase->m_lstBookCategories.end()) {
			m_pBibleDatabase->m_lstBookCategories.push_back(CBookCategoryEntry(strCategory));
			itrCat = m_pBibleDatabase->m_lstBookCategories.end() - 1;
		}
		itrCat->m_setBooksNum.insert(nBkNdx);
		entryBook.m_nCatNdx = std::distance(m_pBibleDatabase->m_lstBookCategories.begin(), itrCat) + 1;

		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumBk++;
		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumChp += entryBook.m_nNumChp;
		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumVrs += entryBook.m_nNumVrs;
		m_pBibleDatabase->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumWrd += entryBook.m_nNumWrd;

		m_pBibleDatabase->m_EntireBible.m_nNumBk++;
		m_pBibleDatabase->m_EntireBible.m_nNumChp += entryBook.m_nNumChp;
		m_pBibleDatabase->m_EntireBible.m_nNumVrs += entryBook.m_nNumVrs;
		m_pBibleDatabase->m_EntireBible.m_nNumWrd += entryBook.m_nNumWrd;
	}

	dbParser.endQueryLoop();

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
			QString strTemp = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10\r\n").arg(itr->m_nTstBkNdx).arg(itr->m_nTstNdx).arg(itr->m_strBkName).arg(itr->m_lstBkAbbr.join(";"))
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

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("LAYOUT")) return false;

	m_pBibleDatabase->m_mapChapters.clear();

	dbParser.startQueryLoop("BkChpNdx, NumVrs, NumWrd, BkAbbr, ChNdx");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 5)) return false;

		uint32_t nBkChpNdx = lstFields.at(0).toUInt();
		CChapterEntry &entryChapter = m_pBibleDatabase->m_mapChapters[CRelIndex(nBkChpNdx << 16)];
		entryChapter.m_nNumVrs = lstFields.at(1).toUInt();
		entryChapter.m_nNumWrd = lstFields.at(2).toUInt();
	}

	dbParser.endQueryLoop();

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

#ifndef NOT_USING_SQL
		CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
		CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

		if (!dbParser.findTable(m_pBibleDatabase->m_lstBooks[nBk-1].m_strTblName)) return false;

		TVerseEntryMap &mapVerses = m_pBibleDatabase->m_lstBookVerses[nBk-1];
		mapVerses.clear();

		dbParser.startQueryLoop("ChpVrsNdx, NumWrd, nPilcrow, PText, RText, TText");

		while (dbParser.haveData()) {
			QStringList lstFields;
			if (!dbParser.readNextRecord(lstFields, 6)) return false;

			QString strVerseText;
			uint32_t nChpVrsNdx = lstFields.at(0).toUInt();
			CVerseEntry &entryVerse = mapVerses[CRelIndex(nChpVrsNdx << 8)];
			entryVerse.m_nNumWrd = lstFields.at(1).toUInt();
			entryVerse.m_nPilcrow = static_cast<CVerseEntry::PILCROW_TYPE_ENUM>(lstFields.at(2).toInt());
			strVerseText = lstFields.at(4);
			if (strVerseText.isEmpty()) strVerseText = lstFields.at(3);
			entryVerse.m_strTemplate = lstFields.at(5);
		}

		dbParser.endQueryLoop();

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

static bool ascendingLessThanStrings(const QString &s1, const QString &s2)
{
	return (s1.compare(s2, Qt::CaseInsensitive) < 0);
}

bool CReadDatabase::ReadWordsTable()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Read the Words table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("WORDS")) return false;

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

	dbParser.startQueryLoop("WrdNdx, Word, bIndexCasePreserve, NumTotal, AltWords, AltWordCounts, NormalMap");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 7, true)) return false;

		QString strWord = lstFields.at(1);
		bool bCasePreserve = ((lstFields.at(2).toInt()) ? true : false);
		QString strKey = CSearchStringListModel::decompose(strWord, true).toLower();
		// This check is needed because duplicates can happen from decomposed index keys.
		//		Note: It's less computationally expensive to search the map for it than
		//				to do a .contains() call on the m_lstWordList below, even though
		//				it's the list we want to keep it out of.  Searching the list used
		//				over 50% of the database load time!:
		bool bIsNewWord = (m_pBibleDatabase->m_mapWordList.find(strKey) == m_pBibleDatabase->m_mapWordList.end());
		CWordEntry &entryWord = m_pBibleDatabase->m_mapWordList[strKey];
		if (bIsNewWord) m_pBibleDatabase->m_lstWordList.append(strKey);

		if (entryWord.m_strWord.isEmpty()) {
			entryWord.m_strWord = strKey;
			entryWord.m_bCasePreserve = bCasePreserve;
		} else {
			// If folding duplicate words into single entry from decomposed indexes,
			//		they better be the same exact word:
			assert(entryWord.m_strWord.compare(strKey) == 0);
			assert(entryWord.m_bCasePreserve == bCasePreserve);
			if ((entryWord.m_strWord.compare(strKey) != 0) || (entryWord.m_bCasePreserve != bCasePreserve)) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Non-unique decomposed word entry error in WORDS table!\n\nWord: \"%1\" with Word: \"%2\"").arg(strWord).arg(entryWord.m_strWord));
				return false;
			}
		}

		QString strAltWords = lstFields.at(4);
		CCSVStream csvWord(&strAltWords, QIODevice::ReadOnly);
		while (!csvWord.atEndOfStream()) {
			QString strTemp;
			csvWord >> strTemp;
			if (!strTemp.isEmpty()) entryWord.m_lstAltWords.push_back(strTemp.normalized(QString::NormalizationForm_C));
		}
		QString strAltWordCounts = lstFields.at(5);
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
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Mismatch Word Counts for \"%1\" AltWords=%2, AltWordCounts=%3")
							.arg(strWord).arg(entryWord.m_lstAltWords.size()).arg(entryWord.m_lstAltWordCount.size()));
			return false;
		}
		if (nAltCount != lstFields.at(3).toUInt()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Bad AltWordCounts for \"%1\"").arg(strWord));
			return false;
		}
		nConcordanceCount += entryWord.m_lstAltWords.size();		// Note: nConcordanceCount will be slightly too large due to folding of duplicate decomposed indexes, but is sufficient for a reserve()

		if (lstFields.at(6).isEmpty()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Bad word indexes for \"%1\"").arg(strWord));
			return false;
		}
		TNormalizedIndexList lstNormalIndexes;
		convertIndexListToNormalIndexes(lstFields.at(6), lstNormalIndexes);
		if (lstNormalIndexes.size() != lstFields.at(3).toUInt()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Index/Count consistency error in WORDS table!"));
			return false;
		}
		entryWord.m_ndxNormalizedMapping.insert(entryWord.m_ndxNormalizedMapping.end(), lstNormalIndexes.begin(), lstNormalIndexes.end());
	}

	dbParser.endQueryLoop();

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
		bool bIsProperWord = true;
		for (int ndxAltWord=0; ndxAltWord<entryWord.m_lstAltWords.size(); ++ndxAltWord) {
			if (!entryWord.m_lstAltWords.at(ndxAltWord).at(0).isUpper()) {
				bIsProperWord = false;
				break;
			}
		}
		for (int ndxAltWord=0; ndxAltWord<entryWord.m_lstAltWords.size(); ++ndxAltWord) {
			QString strAltWord = entryWord.m_lstAltWords.at(ndxAltWord);
			CConcordanceEntry entryConcordance(strAltWord, bIsProperWord, ndxWord);
			m_pBibleDatabase->soundEx(entryConcordance.decomposedWord());		// Pre-compute cached soundEx values for all words so we don't have to do it over and over again later
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
					displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Invalid WORDS mapping.  Check database integrity!\n\nWord: \"%1\"  Index: %2").arg(entryWord.m_lstAltWords.at(ndxAltWord)).arg(entryWord.m_ndxNormalizedMapping.at(ndxMapping)));
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

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("FOOTNOTES")) return false;

	m_pBibleDatabase->m_mapFootnotes.clear();

	dbParser.startQueryLoop("BkChpVrsWrdNdx, PFootnote, RFootnote");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 3)) return false;

		QString strFootnoteText;
		CFootnoteEntry footnote;
		CRelIndex ndxRel(lstFields.at(0).toUInt());
		assert(ndxRel.isSet());
		if (!ndxRel.isSet()) continue;
		strFootnoteText = lstFields.at(2);
		if (strFootnoteText.isEmpty()) strFootnoteText = lstFields.at(1);
		if (!strFootnoteText.isEmpty()) {
			footnote.setText(strFootnoteText);
			m_pBibleDatabase->m_mapFootnotes[ndxRel] = footnote;
		}
	}

	dbParser.endQueryLoop();

	return true;
}

bool CReadDatabase::ReadPHRASESTable(bool bUserPhrases)
{
	if (!bUserPhrases) {
		assert(m_pBibleDatabase.data() != NULL);
	}

	// Read the Phrases table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("PHRASES")) return false;

	CPhraseList lstUserPhrases;
	if (!bUserPhrases) {
		m_pBibleDatabase->m_lstCommonPhrases.clear();
	}

	dbParser.startQueryLoop("Ndx, Phrase, CaseSensitive, AccentSensitive, Exclude");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 5)) return false;

		CPhraseEntry phrase;
		phrase.setText(lstFields.at(1));
		phrase.setCaseSensitive((lstFields.at(2).toInt() != 0) ? true : false);
		phrase.setAccentSensitive((lstFields.at(3).toInt() != 0) ? true : false);
		phrase.setExclude((lstFields.at(4).toInt() != 0) ? true : false);
		if (!phrase.text().isEmpty()) {
			if (bUserPhrases) {
				lstUserPhrases.append(phrase);
			} else {
				m_pBibleDatabase->m_lstCommonPhrases.push_back(phrase);
			}
		}
	}

	dbParser.endQueryLoop();

	if (bUserPhrases) setUserPhrases(lstUserPhrases);

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
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book List and Table of Contents have different sizes!\nCheck the database!"));
		return false;
	}

	ncntTstTot = m_pBibleDatabase->m_lstTestaments.size();
	for (unsigned int nBk=0; nBk<m_pBibleDatabase->m_lstBooks.size(); ++ nBk) {		// Books
		if ((m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx < 1) || (m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx > ncntTstTot)) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) References Invalid Testament %3")
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
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) Chapter %3 contains %4 Verses, expected %5 Verses!")
								.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(nChp+1).arg(ncntVrs_Chp).arg(itrChapters->second.m_nNumVrs));
				return false;
			}
			if (ncntWrd_Chp != itrChapters->second.m_nNumWrd) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) Chapter %3 contains %4 Words, expected %5 Words!")
								.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(nChp+1).arg(ncntWrd_Chp).arg(itrChapters->second.m_nNumWrd));
				return false;
			}
		}
		if (ncntChp_Bk != m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Chapters, expected %4 Chapters!")
							.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(ncntChp_Bk).arg(m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp));
			return false;
		}
		if (ncntVrs_Bk != m_pBibleDatabase->m_lstBooks[nBk].m_nNumVrs) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Verses, expected %4 Verses!")
							.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(ncntVrs_Bk).arg(m_pBibleDatabase->m_lstBooks[nBk].m_nNumVrs));
			return false;
		}
		if (ncntWrd_Bk != m_pBibleDatabase->m_lstBooks[nBk].m_nNumWrd) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Words, expected %4 Words!")
							.arg(m_pBibleDatabase->m_lstBooks[nBk].m_strBkName).arg(nBk+1).arg(ncntWrd_Bk).arg(m_pBibleDatabase->m_lstBooks[nBk].m_nNumWrd));
			return false;
		}
	}

	unsigned int nWordListTot = 0;
	for (TWordListMap::const_iterator itrWords = m_pBibleDatabase->m_mapWordList.begin(); itrWords != m_pBibleDatabase->m_mapWordList.end(); ++itrWords) {
		nWordListTot += itrWords->second.m_ndxNormalizedMapping.size();
	}
	if (nWordListTot != ncntWrdTot) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Word List contains %1 indexes, expected %2!").arg(nWordListTot).arg(ncntWrdTot));
		return false;
	}

	// Check concordance:
	if ((nWordListTot+1) != m_pBibleDatabase->m_lstConcordanceMapping.size()) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Word List contains %1 indexes, but Concordance Mapping contains %2 entries!").arg(nWordListTot+1).arg(m_pBibleDatabase->m_lstConcordanceMapping.size()));
		return false;
	}

	// Check overall count values:
	if ((ncntTstTot != m_pBibleDatabase->bibleEntry().m_nNumTst) ||
		(ncntBkTot != m_pBibleDatabase->bibleEntry().m_nNumBk) ||
		(ncntChpTot != m_pBibleDatabase->bibleEntry().m_nNumChp) ||
		(ncntVrsTot != m_pBibleDatabase->bibleEntry().m_nNumVrs) ||
		(ncntWrdTot != m_pBibleDatabase->bibleEntry().m_nNumWrd)) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Overall Bible Entry Data Counts are inconsistent!  Check database!"));
		return false;
	}

	// Check Normalize/Denormalize functions:
#ifdef TEST_INDEXING
	for (unsigned int nWrd = 1; nWrd <= m_pBibleDatabase->bibleEntry().m_nNumWrd; ++nWrd) {
		uint32_t ndxRel = m_pBibleDatabase->DenormalizeIndex(nWrd);
		if (m_pBibleDatabase->NormalizeIndex(ndxRel) != nWrd) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Normalize/Denormalize Index Check Failed!\n\nNormal->Relative->Normal:\n%1->%2->%3").arg(nWrd).arg(ndxRel).arg(m_pBibleDatabase->NormalizeIndex(ndxRel)));
			assert(false);
		}
	}
#endif

	return true;
}

// ============================================================================

bool CReadDatabase::ReadDictionaryDBInfo()
{
	assert(m_pDictionaryDatabase.data() != NULL);

	// Read the Dictionary Info table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_pDictionaryDatabase->m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("title")) return false;

	dbParser.startQueryLoop("abbr, desc, info");

	bool bFound = false;

	if (dbParser.haveData()) {
		QStringList lstFields;
		if (dbParser.readNextRecord(lstFields, 3)) {
			m_pDictionaryDatabase->m_strInfo = lstFields.at(2);
			bFound = true;
		}
	}

	dbParser.endQueryLoop();

	if (!bFound) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find Dictionary information record!"));
		return false;
	}

	return true;
}

bool CReadDatabase::ReadDictionaryWords(bool bLiveDB)
{
	assert(m_pDictionaryDatabase.data() != NULL);

	// Read the Dictionary Defintions table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_pDictionaryDatabase->m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("dictionary")) return false;

	m_pDictionaryDatabase->m_mapWordDefinitions.clear();

	dbParser.startQueryLoop("id, topic, definition");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 3)) return false;

		int nIndex = lstFields.at(0).toInt();
		QString strWord = lstFields.at(1);
		QString strDefinition = (bLiveDB ? QString() : lstFields.at(2));
		CDictionaryWordEntry wordEntry(strWord, strDefinition, nIndex);
		assert(m_pDictionaryDatabase->m_mapWordDefinitions.find(wordEntry.decomposedWord()) == m_pDictionaryDatabase->m_mapWordDefinitions.end());
		m_pDictionaryDatabase->m_mapWordDefinitions[wordEntry.decomposedWord()] = wordEntry;
	}

	dbParser.endQueryLoop();

	m_pDictionaryDatabase->m_lstWordList.clear();
	m_pDictionaryDatabase->m_lstWordList.reserve(m_pDictionaryDatabase->m_mapWordDefinitions.size());
	for (TDictionaryWordListMap::const_iterator itrWordList = m_pDictionaryDatabase->m_mapWordDefinitions.begin(); itrWordList != m_pDictionaryDatabase->m_mapWordDefinitions.end(); ++itrWordList) {
		m_pDictionaryDatabase->m_lstWordList.append(itrWordList->first);
	}
	qSort(m_pDictionaryDatabase->m_lstWordList.begin(), m_pDictionaryDatabase->m_lstWordList.end());

	return true;
}

QString CReadDatabase::dictionaryDefinition(const CDictionaryDatabase *pDictionaryDatabase, const CDictionaryWordEntry &wordEntry)
{
	assert(pDictionaryDatabase != NULL);
	assert(pDictionaryDatabase->isLiveDatabase());

	QString strDefinition;

#ifndef NOT_USING_SQL
	QSqlQuery queryData(pDictionaryDatabase->m_myDatabase);
	queryData.setForwardOnly(true);
	if ((queryData.exec(QString("SELECT definition FROM dictionary WHERE id=%1").arg(wordEntry.index()))) &&
		(queryData.next())) {
		strDefinition = queryData.value(0).toString();
	}
	queryData.finish();
#else
	Q_UNUSED(wordEntry);
#endif

	return strDefinition;
}

// ============================================================================

bool CReadDatabase::haveBibleDatabaseFiles(const TBibleDescriptor &bblDesc) const
{
	QFileInfo fiSQL(bibleDBFileInfo(DTE_SQL, bblDesc));
	QFileInfo fiCC(bibleDBFileInfo(DTE_CC, bblDesc));
	return ((fiCC.exists() && fiCC.isFile()) ||
			(fiSQL.exists() && fiSQL.isFile()));
}

bool CReadDatabase::haveDictionaryDatabaseFiles(const TDictionaryDescriptor &dctDesc) const
{
	QFileInfo fiSQL(dictDBFileInfo(DTE_SQL, dctDesc));
	QFileInfo fiCC(dictDBFileInfo(DTE_CC, dctDesc));
	return ((fiCC.exists() && fiCC.isFile()) ||
			(fiSQL.exists() && fiSQL.isFile()));
}

QFileInfo CReadDatabase::bibleDBFileInfo(DATABASE_TYPE_ENUM nDatabaseType, const TBibleDescriptor &bblDesc) const
{
	switch (nDatabaseType) {
		case DTE_SQL:
			return QFileInfo(QDir(m_strBibleDatabasePath), bblDesc.m_strS3DBFilename);
		case DTE_CC:
			return QFileInfo(QDir(m_strBibleDatabasePath), bblDesc.m_strCCDBFilename);
		default:
			assert(false);
			return QFileInfo();
	}
}

QFileInfo CReadDatabase::dictDBFileInfo(DATABASE_TYPE_ENUM nDatabaseType, const TDictionaryDescriptor &dctDesc) const
{
	switch (nDatabaseType) {
		case DTE_SQL:
			return QFileInfo(QDir(m_strDictionaryDatabasePath), dctDesc.m_strS3DBFilename);
		case DTE_CC:
			return QFileInfo(QDir(m_strDictionaryDatabasePath), dctDesc.m_strCCDBFilename);
		default:
			assert(false);
			return QFileInfo();
	}
}

// ============================================================================

bool CReadDatabase::readBibleStub()
{
	if ((!ReadDBInfoTable()) ||
		(!ReadTestamentTable()) ||
		(!ReadBooksTable()) ||
		(!ReadChaptersTable()) ||
		(!ReadVerseTables()) ||
		(!ReadWordsTable()) ||
		(!ReadFOOTNOTESTable()) ||
		(!ReadPHRASESTable(false)) ||
		(!ValidateData())) return false;
	return true;
}

bool CReadDatabase::ReadBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain)
{
	bool bSuccess = false;

	m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(bblDesc));
	assert(m_pBibleDatabase.data() != NULL);

	QFileInfo fiSQL(bibleDBFileInfo(DTE_SQL, bblDesc));
	QFileInfo fiCC(bibleDBFileInfo(DTE_CC, bblDesc));

	// Prefer CC database over SQL in our search order:
	if (!bSuccess && fiCC.exists() && fiCC.isFile()) {
		QFile fileCCDB;
		fileCCDB.setFileName(fiCC.absoluteFilePath());
		if (!fileCCDB.open(QIODevice::ReadOnly)) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open CC database file \"%1\".").arg(fiCC.absoluteFilePath()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open CC database file \"%1\".").arg(fiCC.absoluteFilePath()));
		} else {
			QtIOCompressor compCCDB(&fileCCDB);
			compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
			if (!compCCDB.open(QIODevice::ReadOnly)) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Failed to open i/o compressor for file \"%1\".").arg(fiCC.absoluteFilePath()));
			} else {
				CScopedCSVStream ccdb(m_pCCDatabase, new CCSVStream(&compCCDB));
				if (readBibleStub()) bSuccess = true;
			}
		}
	}

	// Try SQL secondarily if we support SQL:
	if (!bSuccess && fiSQL.exists() && fiSQL.isFile()) {
#ifndef NOT_USING_SQL
		m_myDatabase = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrMainReadConnection);
		m_myDatabase.setDatabaseName(fiSQL.absoluteFilePath());
		m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

//		displayInformation(m_pParent, g_constrReadDatabase, m_myDatabase.databaseName());

		if (!m_myDatabase.open()) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2").arg(fiSQL.absoluteFilePath()).arg(m_myDatabase.lastError().text()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2").arg(fiSQL.absoluteFilePath()).arg(m_myDatabase.lastError().text()));
		} else {
			if (readBibleStub()) bSuccess = true;
			m_myDatabase.close();
		}

		m_myDatabase = QSqlDatabase();
		QSqlDatabase::removeDatabase(g_constrMainReadConnection);
#else
#endif	// !NOT_USING_SQL
	}

	if (bSuccess) {
		TBibleDatabaseList::instance()->addBibleDatabase(m_pBibleDatabase, bSetAsMain);
	}

	return bSuccess;
}

bool CReadDatabase::readUserStub()
{
	if (!ReadPHRASESTable(true)) return false;
	return true;
}

bool CReadDatabase::ReadUserDatabase(DATABASE_TYPE_ENUM nDatabaseType, const QString &strDatabaseFilename, bool bHideWarnings)
{
	bool bSuccess = true;

	if (nDatabaseType == DTE_SQL) {
#ifndef NOT_USING_SQL
		m_myDatabase = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrUserReadConnection);
		m_myDatabase.setDatabaseName(strDatabaseFilename);
		m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

		if (!m_myDatabase.open()) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open database file \"%1\".\n\n%2").arg(strDatabaseFilename).arg(m_myDatabase.lastError().text()).toUtf8().data());
#endif
			if (!bHideWarnings)
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open database file \"%1\".\n\n%2").arg(strDatabaseFilename).arg(m_myDatabase.lastError().text()));
			bSuccess = false;
		}

		if (bSuccess) {
			if (!readUserStub()) bSuccess = false;
			m_myDatabase.close();
		}

		m_myDatabase = QSqlDatabase();
		QSqlDatabase::removeDatabase(g_constrUserReadConnection);
#else
		return false;
#endif	// !NOT_USING_SQL
	} else if (nDatabaseType == DTE_CC) {
		QFile fileCCDB;
		if ((bSuccess) && (!strDatabaseFilename.isEmpty())) {
			fileCCDB.setFileName(strDatabaseFilename);
			if (!fileCCDB.open(QIODevice::ReadOnly)) {
				if (!bHideWarnings)
					displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open CC database file \"%1\".").arg(strDatabaseFilename));
				bSuccess = false;
			}
		}

		QtIOCompressor compCCDB(&fileCCDB);
		if ((bSuccess) && (fileCCDB.isOpen())) {
			compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
			if (!compCCDB.open(QIODevice::ReadOnly)) {
				if (!bHideWarnings)
					displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Failed to open i/o compressor for file \"%1\".").arg(strDatabaseFilename));
				bSuccess = false;
			}
		}

		CScopedCSVStream ccdb(m_pCCDatabase, ((bSuccess && compCCDB.isOpen()) ? new CCSVStream(&compCCDB) : NULL));

		if (bSuccess) {
			if (!readUserStub()) bSuccess = false;
		}
	} else {
		assert(false);
		return false;
	}

	return bSuccess;
}

bool CReadDatabase::readDictionaryStub(bool bLiveDB)
{
	if ((!ReadDictionaryDBInfo()) ||
		(!ReadDictionaryWords(bLiveDB))) return false;
	return true;
}

bool CReadDatabase::ReadDictionaryDatabase(const TDictionaryDescriptor &dctDesc, bool bLiveDB, bool bSetAsMain)
{
	bool bSuccess = false;

	m_pDictionaryDatabase = QSharedPointer<CDictionaryDatabase>(new CDictionaryDatabase(dctDesc));
	assert(m_pDictionaryDatabase.data() != NULL);

	m_pDictionaryDatabase->m_strLanguage = dctDesc.m_strLanguage;

	QFileInfo fiSQL(dictDBFileInfo(DTE_SQL, dctDesc));
	QFileInfo fiCC(dictDBFileInfo(DTE_CC, dctDesc));

	// Prefer CC database over SQL in our search order:
	if (!bSuccess && fiCC.exists() && fiCC.isFile()) {
		QFile fileCCDB;
		fileCCDB.setFileName(fiCC.absoluteFilePath());
		if (!fileCCDB.open(QIODevice::ReadOnly)) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open CC database file \"%1\".").arg(fiCC.absoluteFilePath()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open CC database file \"%1\".").arg(fiCC.absoluteFilePath()));
		} else {
			QtIOCompressor compCCDB(&fileCCDB);
			compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
			if (!compCCDB.open(QIODevice::ReadOnly)) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Failed to open i/o compressor for file \"%1\".").arg(fiCC.absoluteFilePath()));
			} else {
				CScopedCSVStream ccdb(m_pCCDatabase, new CCSVStream(&compCCDB));
				if (readDictionaryStub(false)) bSuccess = true;				// CC Database can't be live by definition
			}
		}
	}

	// Try SQL secondarily if we support SQL:
	if (!bSuccess && fiSQL.exists() && fiSQL.isFile()) {
#ifndef NOT_USING_SQL
		if (!m_pDictionaryDatabase->m_myDatabase.contains(dctDesc.m_strUUID)) {
			m_pDictionaryDatabase->m_myDatabase = QSqlDatabase::addDatabase(g_constrDatabaseType, dctDesc.m_strUUID);
		}

		m_pDictionaryDatabase->m_myDatabase.setDatabaseName(fiSQL.absoluteFilePath());
		m_pDictionaryDatabase->m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

//		displayInformation(m_pParent, g_constrReadDatabase, m_pDictionaryDatabase->m_myDatabase.databaseName());

		if (!m_pDictionaryDatabase->m_myDatabase.open()) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2").arg(fiSQL.absoluteFilePath()).arg(m_pDictionaryDatabase->m_myDatabase.lastError().text()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2").arg(fiSQL.absoluteFilePath()).arg(m_pDictionaryDatabase->m_myDatabase.lastError().text()));
			m_pDictionaryDatabase->m_myDatabase = QSqlDatabase();
			QSqlDatabase::removeDatabase(dctDesc.m_strUUID);
		} else {
			if (readDictionaryStub(bLiveDB)) bSuccess = true;
		}

		if ((!bLiveDB) || (!bSuccess)) {
			assert(m_pDictionaryDatabase->m_myDatabase.contains(dctDesc.m_strUUID));
			m_pDictionaryDatabase->m_myDatabase.close();
			m_pDictionaryDatabase->m_myDatabase = QSqlDatabase();
			QSqlDatabase::removeDatabase(dctDesc.m_strUUID);
		}
#else
		Q_UNUSED(bLiveDB)
#endif	// !NOT_USING_SQL
	}

	if (bSuccess) {
		TDictionaryDatabaseList::instance()->addDictionaryDatabase(m_pDictionaryDatabase, bSetAsMain);
	}

	return bSuccess;
}

// ============================================================================

