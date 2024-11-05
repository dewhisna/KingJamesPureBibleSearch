/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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
#include "ParseSymbols.h"
#include "ReportError.h"
#include "PersistentSettings.h"

#ifndef NOT_USING_SQL
#include <QtSql>
#include <QSqlQuery>
#endif

#include <QObject>
#include <QFileInfo>
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

#include <algorithm>

#ifdef DUMP_SOUNDEX_LIST
#include "SoundEx.h"
#endif

#define TEST_WORD_COUNTS_IN_VALIDATION 0

// ============================================================================

namespace {
	const QString g_constrReadDatabase = QObject::tr("Reading Database", "ReadDB");

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
#if QT_VERSION >= 0x060000
		if (varValue.metaType().id() == QMetaType::Bool) {
#else
		if (varValue.type() == QVariant::Bool) {
#endif
			lstFields.append(QString(varValue.toBool() ? "1" : "0"));
#if QT_VERSION >= 0x060000
		} else if (varValue.metaType().id() == QMetaType::QByteArray) {
#else
	} else if (varValue.type() == QVariant::ByteArray) {
#endif
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
		displayWarning(pParent, g_constrReadDatabase, QObject::tr("Bad Record in Database, expected at least %1 field(s):\n\"%2\"", "ReadDB").arg(nMinFields).arg(lstFields.join(",")));
		return false;
	}

	return true;
}
#endif	// !NOT_USING_SQL

static bool readCCDatabaseRecord(QWidget *pParent, QStringList &lstFields, CCSVStream *pStream, int nMinFields)
{
	lstFields.clear();
	if (!pStream->atEnd()) {
		(*pStream) >> lstFields;
	} else {
		displayWarning(pParent, g_constrReadDatabase, QObject::tr("Unexpected end of Bible CCDatabase file", "ReadDB"));
		return false;
	}

	if (lstFields.size() < nMinFields) {
		displayWarning(pParent, g_constrReadDatabase, QObject::tr("Bad Record in Database, expected at least %1 field(s):\n\"%2\"", "ReadDB").arg(nMinFields).arg(lstFields.join(",")));
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

	const QString &tableName() const { return m_strTableName; }
	const QString &tableInfo() const { return m_strTableInfo; }

	bool findTable(const QString &strTableName)
	{
		m_strTableName = strTableName;

		QStringList lstFields;
		m_nRecordCount = 0;

		if (m_pCSVStream != nullptr) {
			if (!readCCDatabaseRecord(m_pParentWidget, lstFields, m_pCSVStream, 2)) return false;
			// Format:  <TABLE>,count[,TableInfo]
			//	Where 'TableInfo' is an optional field that carries more information
			//	about the table and is specific to the type of table.  For example,
			//	a VERSIFICATION table might use a UUID that specifically maps the
			//	specific versification.
			if (lstFields.size() < 2) {
				displayWarning(m_pParentWidget, g_constrReadDatabase, QObject::tr("Invalid %1 section header in CCDatabase\n\n%2", "ReadDB").arg(strTableName).arg(lstFields.join(",")));
				return false;
			}
			if (lstFields.at(0) != strTableName) {
				// If the table doesn't match the one expected,
				//	unget it so we can try reading it as another
				//	table type:
				m_pCSVStream->ungetLine(lstFields);
				return false;
			}

			m_nRecordCount = lstFields.at(1).toInt();
			m_bContinue = (m_nRecordCount > 0);
			if (lstFields.size() > 2) {
				m_strTableInfo = lstFields.at(2);
			}
		} else {
#ifndef NOT_USING_SQL
			m_bContinue = false;
			QSqlQuery queryTable(m_myDatabase);

			// Check to see if the table exists:
			if (!queryTable.exec(QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(strTableName))) {
				displayWarning(m_pParentWidget, g_constrReadDatabase, QObject::tr("Table Lookup for \"%1\" Failed!\n%2", "ReadDB").arg(strTableName).arg(queryTable.lastError().text()));
				return false;
			}
			queryTable.next();
			if (!queryTable.value(0).toInt()) {
				displayWarning(m_pParentWidget, g_constrReadDatabase, QObject::tr("Unable to find \"%1\" Table in database!", "ReadDB").arg(strTableName));
				return false;
			}
			queryTable.finish();
			m_bContinue = true;
#else
			return false;
#endif	// !NOT_USING_SQL
		}

		return true;
	}

	void startQueryLoop(const QString &strFieldNameList)
	{
		Q_ASSERT(!m_strTableName.isEmpty());
		m_bContinue = false;
		if (m_pCSVStream != nullptr) {
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

	bool atEnd() const
	{
		if (m_pCSVStream != nullptr) {
			return m_pCSVStream->atEnd();
		}
		return false;
	}

	bool haveData() const
	{
		return m_bContinue;
	}

	bool readNextRecord(QStringList &lstFields, int nMinFields, bool bTreatBlobsAsIndexes = false)
	{
		if (m_pCSVStream != nullptr) {
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
		if (m_pCSVStream == nullptr) m_queryData.finish();
#endif	// !NOT_USING_SQL
	}

private:
	int m_nRecordCount;
	bool m_bContinue;
	QString m_strTableName;
	QString m_strTableInfo;
	QWidget *m_pParentWidget;
	CCSVStream *m_pCSVStream;
#ifndef NOT_USING_SQL
	QSqlDatabase &m_myDatabase;
	QSqlQuery m_queryData;
#endif	// !NOT_USING_SQL
};

// ============================================================================

CReadDatabase::CReadDatabase(QWidget *pParent)
	:	m_pParent(pParent)
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
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Database Info Table:

	bool bDBInfoGood = true;
	QString strError;
	QStringList lstFields;

	if (!m_pCCDatabase.isNull()) {
		if (!readCCDatabaseRecord(m_pParent, lstFields, m_pCCDatabase.data(), 2)) return false;
	} else {
#ifndef NOT_USING_SQL
		QSqlQuery queryTable(m_myDatabase);

		// Check to see if the table exists:
		if (!queryTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='DBInfo'")) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Table Lookup for \"DBInfo\" Failed!\n%1", "ReadDB").arg(queryTable.lastError().text()));
			return false;
		}
		queryTable.next();
		if (!queryTable.value(0).toInt()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find \"DBInfo\" Table in database!", "ReadDB"));
			return false;
		}
		queryTable.finish();

		QSqlQuery queryData(m_myDatabase);
		queryData.setForwardOnly(true);
		queryData.exec("SELECT ndx, uuid, Language, Name, Description, Info FROM DBInfo");

		if (!queryData.next()) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database DBInfo Record", "ReadDB");
		} else {
			if (!queryFieldsToStringList(m_pParent, lstFields, queryData, 6)) return false;
			// Convert from: ndx, $uuid, $Language, $Name, $Description, $Info
			// to Format:  KJPBSDB, version, $uuid, $Language, $Name, $Description, $Info
			// Note: s3db has been redefined so that 'ndx' is the version value:
			lstFields.insert(0, "KJPBSDB");		// Add our special "magic" ID
		}
		queryData.finish();

		if ((bDBInfoGood) && (lstFields.at(1).toInt() == 2)) {
			// If it's version 2, we need to also read the Direction flag:
			queryData.exec("SELECT Direction FROM DBInfo");

			if (!queryData.next()) {
				bDBInfoGood = false;
				strError = QObject::tr("Invalid Bible Database DBInfo V2 Record", "ReadDB");
			} else {
				QStringList lstV2Fields;
				if (!queryFieldsToStringList(m_pParent, lstV2Fields, queryData, 1)) return false;
				// Convert to: KJPBSDB, version, $uuid, $Language, $Name, $Description, $Direction, $Info
				lstFields.insert(6, lstV2Fields.at(0));
			}

			queryData.finish();
		}
#else
		bDBInfoGood = false;
		strError = QObject::tr("No database reading DBInfo", "ReadDB");
#endif	// !NOT_USING_SQL
	}

	int nVersion = (lstFields.size() < 2) ? 0 : lstFields.at(1).toInt();
	if (bDBInfoGood) {
		if ((lstFields.size() < 2) ||						// Must have a minimum of 2 fields until we figure out the version number, then compare from there based on version
			(lstFields.at(0) != "KJPBSDB")) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Database Header/DBInfo record", "ReadDB");
		} else if ((nVersion < 1) || (nVersion > 2 /* KJPBS_CCDB_VERSION */)) {
			bDBInfoGood = false;
			strError = QObject::tr("Unsupported KJPBS Database Version %1", "ReadDB").arg(lstFields.at(1));
		} else if ( ((nVersion == 1) && (lstFields.size() != 7)) ||
					((nVersion == 2) && (lstFields.size() != 8))) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Database Header/DBInfo record for the version (%1) it specifies", "ReadDB").arg(lstFields.at(1));
		} else if (lstFields.at(2).isEmpty()) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database Compatibility UUID", "ReadDB");
		} else if (lstFields.at(3).isEmpty()) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database Language Identifier", "ReadDB");
		} else if (lstFields.at(4).isEmpty()) {
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Bible Database Name", "ReadDB");
		} else if ((nVersion == 2) &&
				   (lstFields.at(6).compare("ltr", Qt::CaseInsensitive) != 0) &&
				   (lstFields.at(6).compare("rtl", Qt::CaseInsensitive) != 0) &&
				   (!lstFields.at(6).isEmpty())) {		// Note: Treat empty direction as a default (i.e. old format norm)
			bDBInfoGood = false;
			strError = QObject::tr("Invalid Text Direction", "ReadDB");
		} else {
			// Note: This overrides any defaults read from internal descriptor list:
			m_pBibleDatabase->m_descriptor.m_strUUID = lstFields.at(2);
			m_pBibleDatabase->m_descriptor.m_strLanguage = lstFields.at(3);
			m_pBibleDatabase->m_descriptor.m_strDBName = lstFields.at(4);
			m_pBibleDatabase->m_descriptor.m_strDBDesc = lstFields.at(5);		// Should we set strDBDesc=strDBName here if strDBDesc is empty??
			if (nVersion == 2) {
				if (lstFields.at(6).compare("ltr", Qt::CaseInsensitive) == 0) {
					m_pBibleDatabase->m_descriptor.m_nTextDir = Qt::LeftToRight;
				} else if (lstFields.at(6).compare("rtl", Qt::CaseInsensitive) == 0) {
					m_pBibleDatabase->m_descriptor.m_nTextDir = Qt::RightToLeft;
				} else {
					m_pBibleDatabase->m_descriptor.m_nTextDir = Qt::LayoutDirectionAuto;
				}
			}
			m_pBibleDatabase->m_strInfo = lstFields.at((nVersion == 1) ? 6 : 7);
		}
	}

	if (!bDBInfoGood) {
		displayWarning(m_pParent, g_constrReadDatabase, strError);
	}

	return bDBInfoGood;
}

bool CReadDatabase::ReadTestamentTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Testament Table

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("TESTAMENT")) return false;

	m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.clear();

	dbParser.startQueryLoop("TstNdx, TstName");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 2)) return false;

		unsigned int nTstNdx = lstFields.at(0).toUInt();
		if (nTstNdx > m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.size()) m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.resize(nTstNdx);
		CTestamentEntry &entryTestament = m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[nTstNdx-1];
		entryTestament.m_strTstName = lstFields.at(1);
	}

	dbParser.endQueryLoop();

	return true;
}

bool CReadDatabase::ReadBooksTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Books Table

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("TOC")) return false;

	m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible = CBibleEntry();		// Clear out the main Bible entry, just in case we're being called a second time
	m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumTst = static_cast<decltype(m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumTst)>(m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.size());
	m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.clear();

	dbParser.startQueryLoop("BkNdx, TstBkNdx, TstNdx, BkName, BkAbbr, TblName, NumChp, NumVrs, NumWrd, Cat, Desc");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 11)) return false;

		unsigned int nBkNdx = lstFields.at(0).toUInt();
		if (nBkNdx == 0) continue;
		if (nBkNdx > m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size()) m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.resize(nBkNdx);
		CBookEntry &entryBook = m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkNdx-1];
		entryBook.m_nTstBkNdx = lstFields.at(1).toUInt();
		entryBook.m_nTstNdx = lstFields.at(2).toUInt();
		entryBook.m_strBkName = lstFields.at(3);
		entryBook.m_lstBkAbbr = lstFields.at(4).split(QChar(';'), My_QString_SkipEmptyParts);
		entryBook.m_strTblName = lstFields.at(5);
		entryBook.m_nNumChp = lstFields.at(6).toUInt();
		entryBook.m_nNumVrs = lstFields.at(7).toUInt();
		entryBook.m_nNumWrd = lstFields.at(8).toUInt();
		//QString strCategory = lstFields.at(9);		// Category is now deprecated and ignored
		entryBook.m_strDesc = lstFields.at(10);

		m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumBk++;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumChp += entryBook.m_nNumChp;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumVrs += entryBook.m_nNumVrs;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[entryBook.m_nTstNdx-1].m_nNumWrd += entryBook.m_nNumWrd;

		m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumBk++;
		m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumChp += entryBook.m_nNumChp;
		m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumVrs += entryBook.m_nNumVrs;
		m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumWrd += entryBook.m_nNumWrd;
	}

	dbParser.endQueryLoop();

	// Calculate accumulated quick indexes.  Do this here in a separate loop in case database
	//		came to us out of order:
	unsigned int nWrdAccum = 0;
	for (unsigned int nBk = 0; nBk < m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size(); ++nBk) {
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk].m_nWrdAccum = nWrdAccum;
		nWrdAccum += m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk].m_nNumWrd;
	}

	Q_ASSERT(nWrdAccum == m_pBibleDatabase->bibleEntry().m_nNumWrd);		// Our quick indexes should match the count of the Bible as a whole

// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		for (TBookList::const_iterator itr = m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.begin(); itr != m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.end(); ++itr) {
			QString strTemp = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9\r\n").arg(itr->m_nTstBkNdx).arg(itr->m_nTstNdx).arg(itr->m_strBkName).arg(itr->m_lstBkAbbr.join(";"))
									.arg(itr->m_strTblName).arg(itr->m_nNumChp).arg(itr->m_nNumVrs).arg(itr->m_nNumWrd).arg(itr->m_strDesc);
			ts << strTemp;
		}
		fileTest.close();
	}
#endif

	return true;
}

bool CReadDatabase::ReadChaptersTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Chapters (LAYOUT) table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("LAYOUT")) return false;

	m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters.clear();

	dbParser.startQueryLoop("BkChpNdx, NumVrs, NumWrd, BkAbbr, ChNdx");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 5)) return false;

		uint32_t nBkChpNdx = lstFields.at(0).toUInt();
		CChapterEntry &entryChapter = m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[CRelIndex(nBkChpNdx << 16)];
		entryChapter.m_nNumVrs = lstFields.at(1).toUInt();
		entryChapter.m_nNumWrd = lstFields.at(2).toUInt();
	}

	dbParser.endQueryLoop();

// Used for debugging:
#ifdef NEVER
	QFile fileTest("testit.txt");
	if (fileTest.open(QIODevice::WriteOnly)) {
		QTextStream ts(&fileTest);
		for (TChapterMap::const_iterator itr = m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters.begin(); itr != m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters.end(); ++itr) {
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
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Book Verses tables:

	Q_ASSERT(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size() != 0);		// Must read BookEntries before BookVerses

	unsigned int nWrdAccum = 0;			// Accum by Verse
	unsigned int nWrdAccum2 = 0;		// Accum by Chapter

	m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses.clear();
	m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses.resize(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size());

	for (unsigned int nBk=1; nBk<=m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size(); ++nBk) {
		CBookEntry &theBook = m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1];
		if (theBook.m_strTblName.isEmpty()) continue;

#ifndef NOT_USING_SQL
		CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
		CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

		if (!dbParser.findTable(theBook.m_strTblName)) return false;

		TVerseEntryMap &mapVerses = m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[nBk-1];
		mapVerses.clear();

		dbParser.startQueryLoop("ChpVrsNdx, NumWrd, nPilcrow, PText, RText, TText");

		while (dbParser.haveData()) {
			QStringList lstFields;
			if (!dbParser.readNextRecord(lstFields, 6)) return false;

			QString strVerseText;
			uint32_t nChpVrsNdx = lstFields.at(0).toUInt();
			CRelIndex ndxVerse(CRelIndex((nChpVrsNdx << 8) | (nBk << 24)));
			CVerseEntry &entryVerse = mapVerses[ndxVerse];
			entryVerse.m_nNumWrd = lstFields.at(1).toUInt();
			entryVerse.m_nPilcrow = static_cast<CVerseEntry::PILCROW_TYPE_ENUM>(lstFields.at(2).toInt());
			strVerseText = lstFields.at(4);
			if (strVerseText.isEmpty()) strVerseText = lstFields.at(3);
			entryVerse.m_strTemplate = lstFields.at(5);
			if ((entryVerse.m_nNumWrd > 0) && (ndxVerse.verse() == 0)) {
				if (ndxVerse.chapter() == 0) {
					theBook.m_bHaveColophon = true;
				} else {
					CChapterEntry &theChapter = m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[ndxVerse];
					theChapter.m_bHaveSuperscription = true;
				}
			}
		}

		dbParser.endQueryLoop();

		// Calculate accumulated quick indexes.  Do this here in a separate loop in case database
		//		came to us out of order:
		if (theBook.m_bHaveColophon) {
			// Colophons before book chapters -- even though it will be rendered after all chapters
			//		and verses, the index values are numerically less-than all other chapter data!
			//		We must insert it here so that normalize/denormalize works correctly:
			CVerseEntry &entryVerse = mapVerses[CRelIndex(nBk, 0, 0, 0)];
			entryVerse.m_nWrdAccum = nWrdAccum;
			nWrdAccum += entryVerse.m_nNumWrd;
			nWrdAccum2 += entryVerse.m_nNumWrd;
		}
		for (unsigned int nChp = 1; nChp <= theBook.m_nNumChp; ++nChp) {
			unsigned int nWrdAccum3 = 0;		// Accum by Verse for each chapter
			CRelIndex ndxChapter(nBk, nChp, 0, 0);
			CChapterEntry &theChapter = m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[ndxChapter];
			theChapter.m_nWrdAccum = nWrdAccum;
			nWrdAccum2 += theChapter.m_nNumWrd;		// Note: nWrdAccum gets updated below by verses, nWrdAccum2 gets updated here by chapter, except for colophons, which are handled above by book!
			if (theChapter.m_bHaveSuperscription) {
				// Superscriptions before chapter verses:
				CVerseEntry &entryVerse = mapVerses[ndxChapter];
				entryVerse.m_nWrdAccum = nWrdAccum;
				nWrdAccum += entryVerse.m_nNumWrd;
				nWrdAccum3 += entryVerse.m_nNumWrd;
			}
			for (unsigned int nVrs = 1; nVrs <= theChapter.m_nNumVrs; ++nVrs) {
				CVerseEntry &entryVerse = mapVerses[CRelIndex(nBk, nChp, nVrs, 0)];
				entryVerse.m_nWrdAccum = nWrdAccum;
				nWrdAccum += entryVerse.m_nNumWrd;
				nWrdAccum3 += entryVerse.m_nNumWrd;
			}
			Q_ASSERT(nWrdAccum3 == theChapter.m_nNumWrd);		// Quick check to make sure the word count of the verses in the chapter matches the word count for the whole chapter
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

	Q_ASSERT(nWrdAccum == m_pBibleDatabase->bibleEntry().m_nNumWrd);		// Our quick indexes should match the count of the Bible as a whole
	Q_ASSERT(nWrdAccum2 == m_pBibleDatabase->bibleEntry().m_nNumWrd);		// Our quick indexes should match the count of the Bible as a whole

	return true;
}

static bool ascendingLessThanStrings(const QString &s1, const QString &s2)
{
	return (s1.compare(s2, Qt::CaseInsensitive) < 0);
}

bool CReadDatabase::ReadWordsTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Words table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("WORDS")) return false;

	unsigned int nNumWordsInText = 0;
	for (unsigned int ndxBook=0; ndxBook<m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size(); ++ndxBook) {
		nNumWordsInText += m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[ndxBook].m_nNumWrd;
	}

	m_pBibleDatabase->m_mapWordList.clear();
	m_pBibleDatabase->m_lstWordList.clear();
	m_pBibleDatabase->m_lstConcordanceWords.clear();
	m_pBibleDatabase->m_lstConcordanceMapping.clear();
	m_pBibleDatabase->m_lstConcordanceMapping.resize(nNumWordsInText+1);			// Preallocate our concordance mapping as we know how many words the text contains (+1 for zero position)
	int nConcordanceCount = 0;			// Count of words so we can preallocate our buffers

	// NOTE: bIndexCasePreserve is no longer just the SpecialWord CasePreserve flag.  It's now a special word processing
	//			bit-field.  For backward compatibility, the low-bit is still the boolean flag for SpecialWord CasePreserve.
	//			The next to lsbit is the IsProperWord flag for words which have all alternate forms with initial uppercase,
	//			excluding specialized hyphen formed "Ordinary Words", like "God-ward", as mapped out in the KJVDataParse tool.

	dbParser.startQueryLoop("WrdNdx, Word, bIndexCasePreserve, NumTotal, AltWords, AltWordCounts, NormalMap");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 7, true)) return false;

		QString strWord = lstFields.at(1);
		bool bCasePreserve = ((lstFields.at(2).toInt() & 0x01) ? true : false);
		bool bIsProperWord = ((lstFields.at(2).toInt() & 0x02) ? true : false);
		QString strKey = StringParse::decompose(strWord, true).toLower();
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
			entryWord.m_bIsProperWord = bIsProperWord;
		} else {
			// If folding duplicate words into single entry from decomposed indexes,
			//		they better be the same exact word:
			Q_ASSERT(entryWord.m_strWord.compare(strKey) == 0);
			// Combine special flags to form overall logic:
			if (entryWord.m_bCasePreserve != bCasePreserve) entryWord.m_bCasePreserve = true;		// Special word of either form is still the "special word"
			if (entryWord.m_bIsProperWord != bIsProperWord) entryWord.m_bIsProperWord = false;		// If two words that are considered the same have different "proper" status, fold it into being "ordinary"
			if (entryWord.m_strWord.compare(strKey) != 0) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Non-unique decomposed word entry error in WORDS table!\n\nWord: \"%1\" with Word: \"%2\"", "ReadDB").arg(strWord).arg(entryWord.m_strWord));
				return false;
			}
		}

		QString strAltWords = lstFields.at(4);
		CCSVStream csvWord(&strAltWords, QIODevice::ReadOnly);
		while (!csvWord.atEnd()) {
			QString strTemp;
			csvWord >> strTemp;
			if (!strTemp.isEmpty()) {
				strTemp = strTemp.normalized(QString::NormalizationForm_C);
				QString strSearchWord = StringParse::deCantillate(strTemp);
				entryWord.m_lstAltWords.push_back(strTemp);
				entryWord.m_lstSearchWords.push_back(strSearchWord);
				entryWord.m_lstRawAltWords.push_back(StringParse::decompose(StringParse::deApostrophe(strWord, true), true).toLower());
				entryWord.m_lstDecomposedAltWords.push_back(StringParse::decompose(strTemp, true));
				entryWord.m_lstDecomposedHyphenAltWords.push_back(StringParse::decompose(strTemp, false));
				entryWord.m_lstDeApostrAltWords.push_back(StringParse::deApostrHyphen(strSearchWord, true));
				entryWord.m_lstDeApostrHyphenAltWords.push_back(StringParse::deApostrHyphen(strSearchWord, false));
				entryWord.m_lstRenderedAltWords.push_back(QString());		// Placeholder to be filled in below with the call to setRenderedWords()

#ifdef USE_GEMATRIA
				if (TBibleDatabaseList::useGematria()) {
					entryWord.m_lstGematria.push_back(CGematriaCalc(m_pBibleDatabase->langID(), strTemp));
				}
#endif
			}
		}
		m_pBibleDatabase->setRenderedWords(entryWord);
		QString strAltWordCounts = lstFields.at(5);
		CCSVStream csvWordCount(&strAltWordCounts, QIODevice::ReadOnly);
		unsigned int nAltCount = 0;
		while (!csvWordCount.atEnd()) {
			QString strTemp;
			csvWordCount >> strTemp;
			if (!strTemp.isEmpty()) {
				entryWord.m_lstAltWordCount.push_back(strTemp.toUInt());
				nAltCount += strTemp.toUInt();
			}
		}
		if (entryWord.m_lstAltWords.size() != entryWord.m_lstAltWordCount.size()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Mismatch Word Counts for \"%1\" AltWords=%2, AltWordCounts=%3", "ReadDB")
							.arg(strWord).arg(entryWord.m_lstAltWords.size()).arg(entryWord.m_lstAltWordCount.size()));
			return false;
		}
		if (nAltCount != lstFields.at(3).toUInt()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Bad AltWordCounts for \"%1\"", "ReadDB").arg(strWord));
			return false;
		}
		nConcordanceCount += entryWord.m_lstAltWords.size();		// Note: nConcordanceCount will be slightly too large due to folding of duplicate decomposed indexes, but is sufficient for a reserve()

		if (lstFields.at(6).isEmpty()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Bad word indexes for \"%1\"", "ReadDB").arg(strWord));
			return false;
		}
		TNormalizedIndexList lstNormalIndexes;
		convertIndexListToNormalIndexes(lstFields.at(6), lstNormalIndexes);
		if (lstNormalIndexes.size() != lstFields.at(3).toUInt()) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Index/Count consistency error in WORDS table!", "ReadDB"));
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
		TWordListSet setConcordanceAltSearchWords;
		for (int ndxAltWord=0; ndxAltWord<entryWord.m_lstAltWords.size(); ++ndxAltWord) {
			if (setConcordanceAltSearchWords.find(entryWord.m_lstSearchWords.at(ndxAltWord)) == setConcordanceAltSearchWords.cend()) {
				setConcordanceAltSearchWords.insert(entryWord.m_lstSearchWords.at(ndxAltWord));
			} else {
				// If we encounter a search word entry that's a duplicate here, then
				//	the search space and the complete concordance aren't the same space:
				m_pBibleDatabase->m_bSearchSpaceIsCompleteConcordance = false;
			}
			CConcordanceEntry entryConcordance(itrWordEntry, ndxAltWord, ndxWord);
			m_pBibleDatabase->soundEx(entryConcordance.decomposedWord());		// Pre-compute cached soundEx values for all words so we don't have to do it over and over again later
			m_pBibleDatabase->m_lstConcordanceWords.append(entryConcordance);
			ndxWord++;
		}
	}

	// Sort all of our word forms since the alternates may sort different with
	//		with respect to the other words:
	std::sort(m_pBibleDatabase->m_lstWordList.begin(), m_pBibleDatabase->m_lstWordList.end(), ascendingLessThanStrings);
	std::sort(m_pBibleDatabase->m_lstConcordanceWords.begin(), m_pBibleDatabase->m_lstConcordanceWords.end(), TConcordanceListSortPredicate::ascendingLessThanWordCaseInsensitive);

	Q_ASSERT(m_pBibleDatabase->m_lstWordList.size() == static_cast<int>(m_pBibleDatabase->m_mapWordList.size()));

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
			Q_ASSERT(ndxWord < m_pBibleDatabase->m_lstConcordanceWords.size());
			for (unsigned int ndxAltCount=0; ndxAltCount<entryWord.m_lstAltWordCount.at(ndxAltWord); ++ndxAltCount) {
				Q_ASSERT(ndxMapping < entryWord.m_ndxNormalizedMapping.size());
				if (entryWord.m_ndxNormalizedMapping.at(ndxMapping) > nNumWordsInText) {
					displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Invalid WORDS mapping.  Check database integrity!\n\nWord: \"%1\"  Index: %2", "ReadDB").arg(entryWord.m_lstAltWords.at(ndxAltWord)).arg(entryWord.m_ndxNormalizedMapping.at(ndxMapping)));
					return false;
				}
				m_pBibleDatabase->m_lstConcordanceMapping[entryWord.m_ndxNormalizedMapping.at(ndxMapping)] = lstSortIndex.at(ndxWord);
				ndxMapping++;
			}
			ndxWord++;
		}
	}
	Q_ASSERT(m_pBibleDatabase->m_lstConcordanceMapping.size() == (nNumWordsInText + 1));
	Q_ASSERT(ndxWord == m_pBibleDatabase->m_lstConcordanceWords.size());


// Code to dump all of the SoundEx values for the words in this database, used
//	to verify SoundEx equations:
#ifdef DUMP_SOUNDEX_LIST
	QFile fileSoundEx("soundex.txt");
	fileSoundEx.open(QIODevice::WriteOnly);
	fileSoundEx.write(QString(QChar(0xFEFF)).toUtf8());
	QTextStream ts(&fileSoundEx);

	for (TWordListMap::const_iterator itrWordEntry = m_pBibleDatabase->m_mapWordList.begin(); itrWordEntry != m_pBibleDatabase->m_mapWordList.end(); ++itrWordEntry) {
		const CWordEntry &entryWord(itrWordEntry->second);

		ts << entryWord.m_strWord << "," << SoundEx::soundEx(entryWord.m_strWord) << "\n";
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
	Q_ASSERT(!m_pBibleDatabase.isNull());

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
		Q_ASSERT(ndxRel.isSet());
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

bool CReadDatabase::ReadPHRASESTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Phrases table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (!dbParser.findTable("PHRASES")) return false;

	m_pBibleDatabase->m_lstCommonPhrases.clear();

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
			m_pBibleDatabase->m_lstCommonPhrases.push_back(phrase);
		}
	}

	dbParser.endQueryLoop();

	return true;
}

bool CReadDatabase::ReadLEMMASTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Lemmas table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (dbParser.atEnd()) return true;		// Lemmas are optional and old databases won't have them

	if (!dbParser.findTable("LEMMAS")) {
		if (!m_pCCDatabase.isNull()) {
			return false;
		} else {
			return true;			// If this is an SQL-only database, SQL Files won't report EndOfStream above, but Lemmas are optional
		}
	}

	m_pBibleDatabase->m_mapLemmaEntries.clear();

	dbParser.startQueryLoop("BkChpVrsWrdNdx, Count, Attrs");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 3)) return false;

		CLemmaEntry lemma(TPhraseTag(lstFields.at(0).toUInt(), lstFields.at(1).toUInt()), lstFields.at(2));
		Q_ASSERT(lemma.tag().isSet());
		if (!lemma.tag().isSet() || (lemma.count() == 0)) continue;
		m_pBibleDatabase->m_mapLemmaEntries[lemma.tag().relIndex()] = lemma;
	}

	dbParser.endQueryLoop();

	return true;
}

bool CReadDatabase::ReadSTRONGSTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Read the Strongs table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (dbParser.atEnd()) return true;		// Strongs is optional and old databases won't have it
	if (!dbParser.findTable("STRONGS") || !dbParser.haveData()) return true;		// Strongs is optional, old databases won't have it at all, and new databases may have zero entries

	m_pBibleDatabase->m_mapStrongsEntries.clear();
	m_pBibleDatabase->m_mapStrongsOrthographyMap.clear();

	dbParser.startQueryLoop("StrongsMapNdx, Orth, Trans, Pron, Def");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 5)) return false;

		CStrongsEntry strongsEntry(lstFields.at(0));
		strongsEntry.setOrthography(lstFields.at(1));
		strongsEntry.setTransliteration(lstFields.at(2));
		strongsEntry.setPronunciation(lstFields.at(3));
		strongsEntry.setDefinition(lstFields.at(4));
		m_pBibleDatabase->m_mapStrongsEntries[strongsEntry.strongsMapIndex()] = strongsEntry;
		m_pBibleDatabase->m_mapStrongsOrthographyMap.insert(strongsEntry.orthographyPlainText(), strongsEntry.strongsMapIndex());
	}

	dbParser.endQueryLoop();

	return true;
}

bool CReadDatabase::ReadMORPHOLOGYTable()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

// Read the Morphology table:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	if (dbParser.atEnd()) return true;		// Morphology is optional and old databases won't have it
	if (!dbParser.findTable("MORPHOLOGY") || !dbParser.haveData()) return true;		// Morphology is optional, old databases won't have it at all, and new databases may have zero entries

	m_pBibleDatabase->m_mapMorphDatabaseMap.clear();

	dbParser.startQueryLoop("MorphNdx, MorphSrc, Key, Desc");

	while (dbParser.haveData()) {
		QStringList lstFields;
		if (!dbParser.readNextRecord(lstFields, 4)) return false;

		MORPH_SOURCE_ENUM nMorphSrc = MSE_NONE;
		if (lstFields.at(1).compare("oshm", Qt::CaseInsensitive) == 0) {
			nMorphSrc = MSE_OSHM;
		} else if (lstFields.at(1).compare("robinson", Qt::CaseInsensitive) == 0) {
			nMorphSrc = MSE_ROBINSON;
		} else if (lstFields.at(1).compare("packard", Qt::CaseInsensitive) == 0) {
			nMorphSrc = MSE_PACKARD;
		} else if (lstFields.at(1).compare("thayers", Qt::CaseInsensitive) == 0) {
			nMorphSrc = MSE_THAYERS;
		} else {
			// TODO : Add additional types here as needed.
			//	Note: Not throwing on unknown types for backward compatibility
			//		purposes, so we can introduce new databases with new types
			//		and they still read on older app versions, only without the
			//		new data types.
			continue;
		}
		CMorphEntry morph(lstFields.at(2), lstFields.at(3));

		m_pBibleDatabase->m_mapMorphDatabaseMap[nMorphSrc][lstFields.at(2).toUpper()] = morph;
	}

	dbParser.endQueryLoop();

	return true;
}

bool CReadDatabase::ReadVersificationTables()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Versification Table Format:
	//	BkChpVrsNdx, BkAbbr, NumChp, NumVrs, NumWrd, nPilcrow, TText
	//	16777216,Gen,50,1533,20704,0,""			-- 0x01000000 : Book-only: NumChp=Book_Chapter_Count, NumVrs=Book_Verse_Count, NumWrd=Book_Word_Count, nPilcrow/TText = Colophon Template, Empty=None
	//	16842752,Gen,1,31,440,0,""				-- 0x01010000 : Book/Chapter: NumChp=Chapter_Index, NumVrs=Chapter_Verse_Count, NumWrd=Chapter_Word_Count, nPilcrow/TText = Superscription Template, Empty=None
	//	16843008,Gen,1,1,7,0,""					-- 0x01010100 : Book/Chapter/Verse: NumChp=Chapter_Index, NumVrs=Verse_Index, NumWrd=Verse_Word_Count, nPilcrow/TText = Verse Template
	//	16843264,Gen,1,2,14,0,""				-- 0x01010200 : ""
	//	16843520,Gen,1,3,6,0,""					-- 0x01010300 : ""
	//	...

	// SQL Databases can't support Versification Tables:
	if (m_pCCDatabase.isNull()) return true;		// Return true since Versifications are optional

	// Read the Versification tables:

#ifndef NOT_USING_SQL
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data(), m_myDatabase);
#else
	CDBTableParser dbParser(m_pParent, m_pCCDatabase.data());
#endif

	while (!dbParser.atEnd() &&				// Versification Tables are optional and old databases won't have it
		   (dbParser.findTable("VERSIFICATION") && dbParser.haveData())) {		// And new databases may have zero entries
		dbParser.startQueryLoop("BkChpVrsNdx, BkAbbr, NumChp, NumVrs, NumWrd, nPilcrow, TText");

		BIBLE_VERSIFICATION_TYPE_ENUM nVersificationType = CBibleVersifications::lookup(dbParser.tableInfo());
		if (m_pBibleDatabase->hasVersificationType(nVersificationType)) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Duplicate or Invalid Versification Layout ID: \"%1\"", "ReadDB").arg(dbParser.tableInfo()));
			return false;
		}

		// The Main Database contains common details for Testaments, number
		//	of books, etc., which must match across versifications:
		CBibleDatabase::TVersificationLayoutMap::const_iterator itrMain = static_cast<CBibleDatabase::TVersificationLayoutMap::const_iterator>(m_pBibleDatabase->m_itrMainLayout);

		m_pBibleDatabase->m_mapVersificationLayouts[nVersificationType] = CBibleDatabase::TVersificationLayout();
		CBibleDatabase::TVersificationLayoutMap::iterator itrNewV11n = m_pBibleDatabase->m_mapVersificationLayouts.find(nVersificationType);

		// Setup the Testament List with things common from Main:
		itrNewV11n->m_lstTestaments.reserve(itrMain->m_lstTestaments.size());
		for (TTestamentList::size_type nTst = 0; nTst < itrMain->m_lstTestaments.size(); ++nTst) {
			const CTestamentEntry &entryMain = itrMain->m_lstTestaments.at(nTst);
			CTestamentEntry entryNew;
			entryNew.m_strTstName = entryMain.m_strTstName;
			entryNew.m_nNumBk = entryMain.m_nNumBk;
			itrNewV11n->m_lstTestaments.push_back(entryNew);
		}
		itrNewV11n->m_EntireBible.m_nNumTst = itrMain->m_EntireBible.m_nNumTst;

		// Setup the Book List with things common from Main:
		itrNewV11n->m_lstBookVerses.reserve(itrMain->m_lstBooks.size());
		for (TBookList::size_type nBk = 0; nBk < itrMain->m_lstBooks.size(); ++nBk) {
			const CBookEntry &entryMain = itrMain->m_lstBooks.at(nBk);
			CBookEntry entryNew;
			entryNew.m_nTstBkNdx = entryMain.m_nTstBkNdx;
			entryNew.m_nTstNdx = entryMain.m_nTstNdx;
			entryNew.m_strBkName = entryMain.m_strBkName;
			entryNew.m_lstBkAbbr = entryMain.m_lstBkAbbr;
			entryNew.m_strTblName = entryMain.m_strTblName;
			entryNew.m_strDesc = entryMain.m_strDesc;
			itrNewV11n->m_lstBooks.push_back(entryNew);
			// ----
			itrNewV11n->m_lstBookVerses.push_back(TVerseEntryMap());
		}
		itrNewV11n->m_EntireBible.m_nNumBk = itrMain->m_EntireBible.m_nNumBk;

		unsigned int nTotalChp = 0;
		unsigned int nTotalVrs = 0;
		unsigned int nTotalWrds = 0;
		// ----
		unsigned int nTotalChpBooks = 0;		// Cross-check count of chapters from book entries
		unsigned int nTotalVrsBooks = 0;		// Cross-check count of verses from book entries
		unsigned int nTotalWrdsBooks = 0;		// Cross-check count of words from book entries
		// ----
		unsigned int nTotalVrsChapters = 0;		// Cross-check count of verses from chapter entries
		unsigned int nTotalWrdsChapters = 0;	// Cross-check count of words from chapter entries

		while (dbParser.haveData()) {
			QStringList lstFields;
			if (!dbParser.readNextRecord(lstFields, 7)) return false;

			CRelIndex ndxRel(lstFields.at(0).toUInt());
			Q_ASSERT(ndxRel.isSet());
			if (!ndxRel.isSet()) continue;

			unsigned int nNumChp = lstFields.at(2).toUInt();
			unsigned int nNumVrs = lstFields.at(3).toUInt();
			unsigned int nNumWrd = lstFields.at(4).toUInt();
			CVerseEntry::PILCROW_TYPE_ENUM nPilcrow = static_cast<CVerseEntry::PILCROW_TYPE_ENUM>(lstFields.at(5).toUInt());
			QString strTemplate = lstFields.at(6);

			Q_ASSERT(ndxRel.book() != 0);
			Q_ASSERT(ndxRel.word() == 0);
			if ((ndxRel.book() != 0) && (ndxRel.word() == 0)) {
				if (ndxRel.chapter() == 0) {
					// Book Entry:
					CBookEntry &bookEntry = itrNewV11n->m_lstBooks[ndxRel.book()-1];
					bookEntry.m_nNumChp = nNumChp;			// Chapters in this book
					bookEntry.m_nNumVrs = nNumVrs;			// Verses in this book
					bookEntry.m_nNumWrd = nNumWrd;			// Words in this book
					bookEntry.m_nWrdAccum = nTotalWrds;
					// TODO : Add support for extended indexes and letter counts?
					CTestamentEntry &testamentEntry = itrNewV11n->m_lstTestaments[bookEntry.m_nTstNdx-1];
					testamentEntry.m_nNumChp += nNumChp;
					testamentEntry.m_nNumVrs += nNumVrs;
					testamentEntry.m_nNumWrd += nNumWrd;
					// TODO : Add support for extended indexes and letter counts?

					nTotalChpBooks += nNumChp;
					nTotalVrsBooks += nNumVrs;
					nTotalWrdsBooks += nNumWrd;

					// Zero chapter/Verse = Optional Colophon for Book:
					if (!strTemplate.isEmpty()) {
						bookEntry.m_bHaveColophon = true;
						QStringList lstWords = strTemplate.split('w');		// We don't have word counts for this pseudo-verse, so compute it from the template
						Q_ASSERT(!lstWords.empty());
						CVerseEntry &verseEntry = itrNewV11n->m_lstBookVerses[ndxRel.book()-1][ndxRel];
						verseEntry.m_nPilcrow = nPilcrow;
						verseEntry.m_strTemplate = strTemplate;
						verseEntry.m_nNumWrd = lstWords.size()-1;		// split will create one too many word entries, so subtract 1
						verseEntry.m_nWrdAccum = nTotalWrds;
						// TODO : Add support for extended indexes and letter counts?
						++nTotalVrs;
						nTotalWrds += verseEntry.m_nNumWrd;
					}
				} else {
					Q_ASSERT(ndxRel.chapter() == nNumChp);
					if (ndxRel.chapter() == nNumChp) {
						if (ndxRel.verse() == 0) {
							// Chapter Entry:
							CChapterEntry &chapterEntry = itrNewV11n->m_mapChapters[ndxRel];
							chapterEntry.m_nNumVrs = nNumVrs;		// Verses in this chapter
							chapterEntry.m_nNumWrd = nNumWrd;		// Words in this chapter
							chapterEntry.m_nWrdAccum = nTotalWrds;
							// TODO : Add support for extended indexes and letter counts?
							++nTotalChp;

							nTotalVrsChapters += nNumVrs;
							nTotalWrdsChapters += nNumWrd;

							// Zero Verse = Optional Superscription for Chapter:
							if (!strTemplate.isEmpty()) {
								chapterEntry.m_bHaveSuperscription = true;
								QStringList lstWords = strTemplate.split('w');		// We don't have word counts for this pseudo-verse, so compute it from the template
								Q_ASSERT(!lstWords.empty());
								CVerseEntry &verseEntry = itrNewV11n->m_lstBookVerses[ndxRel.book()-1][ndxRel];
								verseEntry.m_nPilcrow = nPilcrow;
								verseEntry.m_strTemplate = strTemplate;
								verseEntry.m_nNumWrd = lstWords.size()-1;		// split will create one too many word entries, so subtract 1
								verseEntry.m_nWrdAccum = nTotalWrds;
								// TODO : Add support for extended indexes and letter counts?
								++nTotalVrs;
								nTotalWrds += verseEntry.m_nNumWrd;
							}
						} else {
							Q_ASSERT(ndxRel.verse() == nNumVrs);
							if (ndxRel.verse() == nNumVrs) {
								// Verse Entry:
								CVerseEntry &verseEntry  = itrNewV11n->m_lstBookVerses[ndxRel.book()-1][ndxRel];
								verseEntry.m_nPilcrow = nPilcrow;
								verseEntry.m_strTemplate = strTemplate;
								verseEntry.m_nNumWrd = nNumWrd;
								verseEntry.m_nWrdAccum = nTotalWrds;
								// TODO : Add support for extended indexes and letter counts?
								++nTotalVrs;
								nTotalWrds += verseEntry.m_nNumWrd;
							} else {
								// Verse doesn't match:
								displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Versification: %1\nLine: %2\nIndex Verse: %3 doesn't match given Verse: %4!", "ReadDB")
												.arg(dbParser.tableInfo()).arg(lstFields.join(",")).arg(ndxRel.verse()).arg(nNumVrs));
								return false;
							}
						}
					} else {
						// Chapter doesn't match:
						displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Versification: %1\nLine: %2\nIndex Chapter: %3 doesn't match given Chapter: %4!", "ReadDB")
										.arg(dbParser.tableInfo()).arg(lstFields.join(",")).arg(ndxRel.chapter()).arg(nNumChp));
						return false;
					}
				}
			} else {
				// Zero-Book or Non-Zero-Word:
				if (ndxRel.book() == 0) {
					displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Versification: %1\nLine: %2\nIndex Book is Zero!", "ReadDB")
									.arg(dbParser.tableInfo()).arg(lstFields.join(",")));
					return false;
				}
				// Silently ignore Non-Zero-Word so we can reserve it for future expansion.
				//	The assert above the 'if' should catch errors until we repurpose it.
			}
		}

		itrNewV11n->m_EntireBible.m_nNumChp = nTotalChp;
		itrNewV11n->m_EntireBible.m_nNumVrs = nTotalVrs;
		itrNewV11n->m_EntireBible.m_nNumWrd = nTotalWrds;

		if ((nTotalChp != nTotalChpBooks) ||
			((nTotalVrs != nTotalVrsBooks) || (nTotalVrs != nTotalVrsChapters)) ||
			((nTotalWrds != nTotalWrdsBooks) || (nTotalWrds != nTotalWrdsChapters))) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Versification: %1 Validation Failed!\n"
																		"Total Chapters: %2,  Book Chapters Specified: %3\n"
																		"Total Verses: %4, Book Verses Specified: %5, Chapter Verses Specified: %6\n"
																		"Total Words: %7, Book Words Specified: %8, Chapter Words Specified: %9", "ReadDB")
																		.arg(dbParser.tableInfo())
																		.arg(nTotalChp).arg(nTotalChpBooks)
																		.arg(nTotalVrs).arg(nTotalVrsBooks).arg(nTotalVrsChapters)
																		.arg(nTotalWrds).arg(nTotalWrdsBooks).arg(nTotalWrdsChapters));
			return false;
		}

		dbParser.endQueryLoop();
	}

	return true;
}

bool CReadDatabase::ValidateData()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

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

	if (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses.size() != m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size()) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book List and Table of Contents have different sizes!\nCheck the database!", "ReadDB"));
		return false;
	}

	ncntTstTot = static_cast<decltype(ncntTstTot)>(m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.size());
	for (unsigned int nBk=1; nBk<=m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size(); ++nBk) {		// Books
		if ((m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nTstNdx < 1) || (m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nTstNdx > ncntTstTot)) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) References Invalid Testament %3", "ReadDB")
							.arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_strBkName).arg(nBk).arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nTstNdx));
			return false;
		}
		ncntChp_Bk = 0;
		ncntVrs_Bk = 0;
		ncntWrd_Bk = 0;
		ncntBkTot++;
		// Colophons are part of the book's words, but not part of the chapter's:
		if (m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_bHaveColophon) {
			const TVerseEntryMap &aBookVerses = m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[nBk-1];
			TVerseEntryMap::const_iterator itrVerses = aBookVerses.find(CRelIndex(nBk,0,0,0));
			if (itrVerses != aBookVerses.end()) {
				ncntWrdTot += itrVerses->second.m_nNumWrd;
				ncntWrd_Bk += itrVerses->second.m_nNumWrd;
			} // Should we assert on the else??
		}
		for (unsigned int nChp=1; nChp<=m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nNumChp; ++nChp) {	// Chapters
			ncntVrs_Chp = 0;
			ncntWrd_Chp = 0;
			TChapterMap::const_iterator itrChapters = m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters.find(CRelIndex(nBk,nChp,0,0));
			if (itrChapters == m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters.end()) continue;
			ncntChpTot++;
			ncntChp_Bk++;
			for (unsigned int nVrs=(itrChapters->second.m_bHaveSuperscription ? 0 : 1); nVrs<=itrChapters->second.m_nNumVrs; ++nVrs) {	// Verses
				const TVerseEntryMap &aBookVerses = m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[nBk-1];
				TVerseEntryMap::const_iterator itrVerses = aBookVerses.find(CRelIndex(nBk,nChp,nVrs,0));
				if (itrVerses == aBookVerses.end()) continue;
				if (nVrs != 0) {
					ncntVrsTot++;
					ncntVrs_Chp++;
					ncntVrs_Bk++;
				}
				ncntWrdTot += itrVerses->second.m_nNumWrd;			// Words
				ncntWrd_Chp += itrVerses->second.m_nNumWrd;
				ncntWrd_Bk += itrVerses->second.m_nNumWrd;
#if TEST_WORD_COUNTS_IN_VALIDATION
				QStringList lstWordTemplate = itrVerses->second.m_strTemplate.split('w');
				if (itrVerses->second.m_nNumWrd != (lstWordTemplate.size()-1)) {
					displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) Chapter %3, Verse %4 contains %5 Words, expected %6 Words!", "ReadDB")
									.arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_strBkName).arg(nBk).arg(nChp).arg(nVrs).arg(lstWordTemplate.size()-1).arg(itrVerses->second.m_nNumWrd));
					return false;
				}
#endif
			}
			if (ncntVrs_Chp != itrChapters->second.m_nNumVrs) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) Chapter %3 contains %4 Verses, expected %5 Verses!", "ReadDB")
								.arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_strBkName).arg(nBk).arg(nChp).arg(ncntVrs_Chp).arg(itrChapters->second.m_nNumVrs));
				return false;
			}
			if (ncntWrd_Chp != itrChapters->second.m_nNumWrd) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) Chapter %3 contains %4 Words, expected %5 Words!", "ReadDB")
								.arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_strBkName).arg(nBk).arg(nChp).arg(ncntWrd_Chp).arg(itrChapters->second.m_nNumWrd));
				return false;
			}
		}
		if (ncntChp_Bk != m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nNumChp) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Chapters, expected %4 Chapters!", "ReadDB")
							.arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_strBkName).arg(nBk).arg(ncntChp_Bk).arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nNumChp));
			return false;
		}
		if (ncntVrs_Bk != m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nNumVrs) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Verses, expected %4 Verses!", "ReadDB")
							.arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_strBkName).arg(nBk).arg(ncntVrs_Bk).arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nNumVrs));
			return false;
		}
		if (ncntWrd_Bk != m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nNumWrd) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Book \"%1\" (%2) contains %3 Words, expected %4 Words!", "ReadDB")
							.arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_strBkName).arg(nBk).arg(ncntWrd_Bk).arg(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1].m_nNumWrd));
			return false;
		}
	}

	unsigned int nWordListTot = 0;
	for (TWordListMap::const_iterator itrWords = m_pBibleDatabase->m_mapWordList.begin(); itrWords != m_pBibleDatabase->m_mapWordList.end(); ++itrWords) {
		nWordListTot += static_cast<decltype(nWordListTot)>(itrWords->second.m_ndxNormalizedMapping.size());
	}
	if (nWordListTot != ncntWrdTot) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Word List contains %1 indexes, expected %2!", "ReadDB").arg(nWordListTot).arg(ncntWrdTot));
		return false;
	}

	// Check concordance:
	if ((nWordListTot+1) != m_pBibleDatabase->m_lstConcordanceMapping.size()) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Word List contains %1 indexes, but Concordance Mapping contains %2 entries!", "ReadDB").arg(nWordListTot+1).arg(m_pBibleDatabase->m_lstConcordanceMapping.size()));
		return false;
	}

	// Check overall count values:
	if ((ncntTstTot != m_pBibleDatabase->bibleEntry().m_nNumTst) ||
		(ncntBkTot != m_pBibleDatabase->bibleEntry().m_nNumBk) ||
		(ncntChpTot != m_pBibleDatabase->bibleEntry().m_nNumChp) ||
		(ncntVrsTot != m_pBibleDatabase->bibleEntry().m_nNumVrs) ||
		(ncntWrdTot != m_pBibleDatabase->bibleEntry().m_nNumWrd)) {
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Overall Bible Entry Data Counts are inconsistent!  Check database!", "ReadDB"));
		return false;
	}

	// Check Normalize/Denormalize functions:
#ifdef TEST_INDEXING
	for (unsigned int nWrd = 1; nWrd <= m_pBibleDatabase->bibleEntry().m_nNumWrd; ++nWrd) {
		CRelIndex ndxRel = m_pBibleDatabase->DenormalizeIndex(nWrd);
		if (m_pBibleDatabase->NormalizeIndex(ndxRel.index()) != nWrd) {
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Normalize/Denormalize Index Check Failed!\n\nNormal->Relative->Normal:\n%1->%2->%3", "ReadDB").arg(nWrd).arg(ndxRel.index()).arg(m_pBibleDatabase->NormalizeIndex(ndxRel)));
			Q_ASSERT(false);
		}
	}
#endif

	return true;
}

// ============================================================================

bool CReadDatabase::ReadDictionaryDBInfo()
{
	Q_ASSERT(!m_pDictionaryDatabase.isNull());

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
		displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Unable to find Dictionary information record!", "ReadDB"));
		return false;
	}

	return true;
}

bool CReadDatabase::ReadDictionaryWords(bool bLiveDB)
{
	Q_ASSERT(!m_pDictionaryDatabase.isNull());

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
		CDictionaryWordEntry wordEntryNew(strWord);
		if (m_pDictionaryDatabase->m_mapWordDefinitions.find(wordEntryNew.decomposedWord()) == m_pDictionaryDatabase->m_mapWordDefinitions.end()) {
			m_pDictionaryDatabase->m_mapWordDefinitions[wordEntryNew.decomposedWord()] = wordEntryNew;
		}
		CDictionaryWordEntry &wordEntry = m_pDictionaryDatabase->m_mapWordDefinitions[wordEntryNew.decomposedWord()];
		wordEntry.addDefinition(nIndex, strDefinition);
	}

	dbParser.endQueryLoop();

	m_pDictionaryDatabase->m_lstWordList.clear();
	m_pDictionaryDatabase->m_lstWordList.reserve(static_cast<decltype(m_pDictionaryDatabase->m_lstWordList)::size_type>(m_pDictionaryDatabase->m_mapWordDefinitions.size()));
	for (TDictionaryWordListMap::const_iterator itrWordList = m_pDictionaryDatabase->m_mapWordDefinitions.begin(); itrWordList != m_pDictionaryDatabase->m_mapWordDefinitions.end(); ++itrWordList) {
		m_pDictionaryDatabase->m_lstWordList.append(itrWordList->first);
	}
	std::sort(m_pDictionaryDatabase->m_lstWordList.begin(), m_pDictionaryDatabase->m_lstWordList.end());

	return true;
}

QString CReadDatabase::dictionaryDefinition(const CDictionaryDatabase *pDictionaryDatabase, const CDictionaryWordEntry &wordEntry)
{
	Q_ASSERT(pDictionaryDatabase != nullptr);

	QStringList lstDefinitions;

	if (!pDictionaryDatabase->isLiveDatabase()) {
		// If we have loaded the whole database in memory, just return it:
		return wordEntry.definitions().join(QString::fromLatin1("<hr>"))
				.replace("bible://", "bible/://", Qt::CaseInsensitive)
				.replace("strong://", "strong/://", Qt::CaseInsensitive);
	} else {
		// Otherwise, do SQL Query:
#ifndef NOT_USING_SQL
		for (int ndx=0; ndx < wordEntry.indexes().size(); ++ndx) {
			QSqlQuery queryData(pDictionaryDatabase->m_myDatabase);
			queryData.setForwardOnly(true);
			if ((queryData.exec(QString("SELECT definition FROM dictionary WHERE id=%1").arg(wordEntry.indexes().at(ndx)))) &&
				(queryData.next())) {
				lstDefinitions.append(queryData.value(0).toString());
			}
			queryData.finish();
		}
#else
	Q_UNUSED(wordEntry);
#endif
	}

	return lstDefinitions.join(QString::fromLatin1("<hr>"))
			.replace("bible://", "bible/://", Qt::CaseInsensitive)
			.replace("strong://", "strong/://", Qt::CaseInsensitive);
}

// ============================================================================

bool CReadDatabase::haveBibleDatabaseFiles(const TBibleDescriptor &bblDesc) const
{
	QFileInfo fiSQL(bblDesc.m_strS3DBFilename);
	QFileInfo fiCC(bblDesc.m_strCCDBFilename);
#ifdef Q_OS_ANDROID
	if (!fiCC.exists()) {
		__android_log_print(ANDROID_LOG_WARN, "KJPBS", QObject::tr("Warning: Couldn't find CC database file \"%1\".", "ReadDB").arg(fiCC.absoluteFilePath()).toUtf8().data());
	}
#endif
	return ((fiCC.exists() && fiCC.isFile()) ||
			(fiSQL.exists() && fiSQL.isFile()));
}

bool CReadDatabase::haveDictionaryDatabaseFiles(const TDictionaryDescriptor &dctDesc) const
{
	if (dctDesc.m_strCCDBFilename.isEmpty() && dctDesc.m_strS3DBFilename.isEmpty()) {
		// Empty filenames is special-case to refer back to a dictionary integrated in
		//	the Bible Database.  So see if we can read the Bible Database, and if not,
		//	we can't read the Dictionary Database either:
		if (!TBibleDatabaseList::availableBibleDatabaseDescriptor(dctDesc.m_strUUID).isValid()) return false;
		return true;
	}

	QFileInfo fiSQL(dctDesc.m_strS3DBFilename);
	QFileInfo fiCC(dctDesc.m_strCCDBFilename);
	return ((fiCC.exists() && fiCC.isFile()) ||
			(fiSQL.exists() && fiSQL.isFile()));
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
		(!ReadPHRASESTable()) ||
		(!ReadLEMMASTable()) ||
		(!ReadSTRONGSTable()) ||
		(!ReadMORPHOLOGYTable()) ||
		(!ReadVersificationTables()) ||
		(!ValidateData())) return false;
#ifdef USE_EXTENDED_INDEXES
	// Build Letter counts.  Do this here after reading the
	//	tables because the Letter Counts aren't stored in
	//	the other tables and can't be computed until the
	//	WordsTable is read:
	uint32_t nLtrCount = 0;
	for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size(); ++nBk) {
		CBookEntry &theBook = m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBk-1];
		TVerseEntryMap &mapVerses = m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[nBk-1];

		theBook.m_nLtrAccum = nLtrCount;
		if (theBook.m_bHaveColophon) {
			// Handle Colophon separately, as there won't be a real chapter 0 to index:
			CVerseEntry &theVerse = mapVerses[CRelIndex(nBk, 0, 0, 0)];

			theVerse.m_nLtrAccum = nLtrCount;
			for (unsigned int nWrd = 1; nWrd <= theVerse.m_nNumWrd; ++nWrd) {
				CRelIndex ndxBkWrd(nBk, 0, 0, nWrd);
				const CConcordanceEntry *pConcordanceEntry = m_pBibleDatabase->concordanceEntryForWordAtIndex(ndxBkWrd);
				if (pConcordanceEntry == nullptr) continue;
				theVerse.m_nNumLtr += pConcordanceEntry->letterCount();
			}
			nLtrCount += theVerse.m_nNumLtr;
			theBook.m_nNumLtr += theVerse.m_nNumLtr;
		}
		for (unsigned int nChp = 1; nChp <= theBook.m_nNumChp; ++nChp) {
			CRelIndex ndxBkChp(nBk, nChp, 0, 0);
			if (m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters.find(ndxBkChp) == m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters.end()) continue;
			CChapterEntry &theChapter = m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[ndxBkChp];

			theChapter.m_nLtrAccum = nLtrCount;
			for (unsigned int nVrs = (theChapter.m_bHaveSuperscription ? 0 : 1);
					nVrs <= theChapter.m_nNumVrs; ++nVrs) {
				CRelIndex ndxBkChpVrs(nBk, nChp, nVrs, 0);
				if (mapVerses.find(ndxBkChpVrs) == mapVerses.end()) continue;
				CVerseEntry &theVerse = mapVerses[ndxBkChpVrs];

				theVerse.m_nLtrAccum = nLtrCount;
				for (unsigned int nWrd = 1; nWrd <= theVerse.m_nNumWrd; ++nWrd) {
					CRelIndex ndxBkChpVrsWrd(nBk, nChp, nVrs, nWrd);
					const CConcordanceEntry *pConcordanceEntry = m_pBibleDatabase->concordanceEntryForWordAtIndex(ndxBkChpVrsWrd);
					if (pConcordanceEntry == nullptr) continue;
					theVerse.m_nNumLtr += pConcordanceEntry->letterCount();
				}
				nLtrCount += theVerse.m_nNumLtr;
				theChapter.m_nNumLtr += theVerse.m_nNumLtr;
			}

			theBook.m_nNumLtr += theChapter.m_nNumLtr;
		}

		m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[theBook.m_nTstNdx-1].m_nNumLtr += theBook.m_nNumLtr;
	}
	m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumLtr = nLtrCount;
#endif
	return true;
}

bool CReadDatabase::ReadBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain)
{
	bool bSuccess = false;

	// Prefer CC database over SQL in our search order:
	if (!bSuccess) {
		bSuccess = readCCDBBibleDatabase(bblDesc, bSetAsMain);
	}

	// Try SQL secondarily if we support SQL:
	if (!bSuccess) {
		bSuccess = readS3DBBibleDatabase(bblDesc, bSetAsMain);
	}

	return bSuccess;
}

// Note: Primary use of ReadSpecialBibleDatabase is in external command-line
//	applications like KJVDiff where a specific non-registered (i.e. no internal
//	descriptor) database is explicitly loaded by filename:
bool CReadDatabase::ReadSpecialBibleDatabase(const QString &strCCDBPathFilename, bool bSetAsMain)
{
	QFileInfo fiCCDB(QDir(TBibleDatabaseList::bibleDatabasePath()), strCCDBPathFilename);
	TBibleDescriptor bblDesc = bibleDescriptor(BDE_SPECIAL_TEST);
	bblDesc.m_strCCDBFilename = fiCCDB.absoluteFilePath();
	return readCCDBBibleDatabase(bblDesc, bSetAsMain);
}

bool CReadDatabase::readCCDBBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain)
{
	bool bSuccess = false;

	QFileInfo fiCCDB(bblDesc.m_strCCDBFilename);
	if (fiCCDB.exists() && fiCCDB.isFile()) {
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(bblDesc));
		Q_ASSERT(!m_pBibleDatabase.isNull());

		QFile fileCCDB;
		fileCCDB.setFileName(fiCCDB.absoluteFilePath());
		if (!fileCCDB.open(QIODevice::ReadOnly)) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open CC database file \"%1\".", "ReadDB").arg(fiCCDB.absoluteFilePath()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open CC database file \"%1\".", "ReadDB").arg(fiCCDB.absoluteFilePath()));
		} else {
			QtIOCompressor compCCDB(&fileCCDB);
			compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
			if (!compCCDB.open(QIODevice::ReadOnly)) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Failed to open i/o compressor for file \"%1\".", "ReadDB").arg(fiCCDB.absoluteFilePath()));
			} else {
				CScopedCSVStream ccdb(m_pCCDatabase, new CCSVStream(&compCCDB));
				if (readBibleStub()) bSuccess = true;
			}
		}
	}

	// If this Bible contained a Strong Dictionary, set the flag in its descriptor,
	//	even if the default descriptor that loaded us didn't have it set.  Do this
	//	before adding the BibleDatabase so our descriptors will get updated:
	if (m_pBibleDatabase->m_mapStrongsEntries.size()) {
		m_pBibleDatabase->m_descriptor.m_btoFlags |= BTO_HasStrongs;
	}

	// If this Bible contained a Morphology Database, set the flag in its descriptor.
	//	Do this before adding the BibleDatabase so our descriptors will get updated:
	if (m_pBibleDatabase->m_mapMorphDatabaseMap.size()) {
		m_pBibleDatabase->m_descriptor.m_btoFlags |= BTO_HasMorphology;
	}

	if (bSuccess) {
		TBibleDatabaseList::instance()->addBibleDatabase(m_pBibleDatabase, bSetAsMain);

		TBibleDatabaseSettings currentSettings = m_pBibleDatabase->settings();
		if (!m_pBibleDatabase->hasVersificationType(currentSettings.versification())) {
			// If the current settings versification isn't in the database, switch
			//	to the current settings (which should match the descriptor during the
			//	Bible database construction).  Note: this will trigger a settings
			//	change event so we don't need to do it twice (hence the else):
			currentSettings.setVersification(m_pBibleDatabase->m_itrCurrentLayout.key());
			m_pBibleDatabase->setSettings(currentSettings);
		} else {
			// Trigger settings change to make sure rendering is updated, since
			//	things like versification won't be valid at database construction:
			CPersistentSettings::instance()->triggerForcedChangeBibleDatabaseSettings(m_pBibleDatabase->compatibilityUUID());
		}
	} else {
		m_pBibleDatabase.clear();
	}

	return bSuccess;
}

TBibleDescriptor CReadDatabase::discoverCCDBBibleDatabase(const QString &strFilePathName)
{
	Q_ASSERT(m_pBibleDatabase.isNull());		// Must be run on a new CReadDatabase object

	TBibleDescriptor bblDesc;
	bblDesc.m_strCCDBFilename = strFilePathName;
	QFileInfo fiCCDB(bblDesc.m_strCCDBFilename);
	if (fiCCDB.exists() && fiCCDB.isFile()) {
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(bblDesc));
		Q_ASSERT(!m_pBibleDatabase.isNull());

		QFile fileCCDB;
		fileCCDB.setFileName(fiCCDB.absoluteFilePath());
		if (fileCCDB.open(QIODevice::ReadOnly)) {
			QtIOCompressor compCCDB(&fileCCDB);
			compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
			if (compCCDB.open(QIODevice::ReadOnly)) {
				CScopedCSVStream ccdb(m_pCCDatabase, new CCSVStream(&compCCDB));
				if (ReadDBInfoTable()) {
					m_pBibleDatabase->m_descriptor.m_btoFlags |= BTO_Discovered;
					bblDesc = m_pBibleDatabase->m_descriptor;
				}
			}
		}

		m_pBibleDatabase.clear();
	}

	return bblDesc;
}

bool CReadDatabase::readS3DBBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain)
{
	bool bSuccess = false;

	QFileInfo fiS3DB(bblDesc.m_strS3DBFilename);
	if (fiS3DB.exists() && fiS3DB.isFile()) {
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(bblDesc));
		Q_ASSERT(!m_pBibleDatabase.isNull());

#ifndef NOT_USING_SQL
		m_myDatabase = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrMainReadConnection);
		m_myDatabase.setDatabaseName(fiS3DB.absoluteFilePath());
		m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

//		displayInformation(m_pParent, g_constrReadDatabase, m_myDatabase.databaseName());

		if (!m_myDatabase.open()) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2", "ReadDB").arg(fiS3DB.absoluteFilePath()).arg(m_myDatabase.lastError().text()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2", "ReadDB").arg(fiS3DB.absoluteFilePath()).arg(m_myDatabase.lastError().text()));
		} else {
			if (readBibleStub()) bSuccess = true;
			m_myDatabase.close();
		}

		m_myDatabase = QSqlDatabase();
		QSqlDatabase::removeDatabase(g_constrMainReadConnection);
#else
#endif	// !NOT_USING_SQL
	}

	// If this Bible contained a Strong Dictionary, set the flag in its descriptor,
	//	even if the default descriptor that loaded us didn't have it set.  Do this
	//	before adding the BibleDatabase so our descriptors will get updated:
	if (m_pBibleDatabase->m_mapStrongsEntries.size()) {
		m_pBibleDatabase->m_descriptor.m_btoFlags |= BTO_HasStrongs;
	}

	// If this Bible contained a Morphology Database, set the flag in its descriptor.
	//	Do this before adding the BibleDatabase so our descriptors will get updated:
	if (m_pBibleDatabase->m_mapMorphDatabaseMap.size()) {
		m_pBibleDatabase->m_descriptor.m_btoFlags |= BTO_HasMorphology;
	}

	if (bSuccess) {
		TBibleDatabaseList::instance()->addBibleDatabase(m_pBibleDatabase, bSetAsMain);

		TBibleDatabaseSettings currentSettings = m_pBibleDatabase->settings();
		if (!m_pBibleDatabase->hasVersificationType(currentSettings.versification())) {
			// If the current settings versification isn't in the database, switch
			//	to the current settings (which should match the descriptor during the
			//	Bible database construction).  Note: this will trigger a settings
			//	change event so we don't need to do it twice (hence the else):
			currentSettings.setVersification(m_pBibleDatabase->m_itrCurrentLayout.key());
			m_pBibleDatabase->setSettings(currentSettings);
		} else {
			// Trigger settings change to make sure rendering is updated, since
			//	things like versification won't be valid at database construction:
			CPersistentSettings::instance()->triggerForcedChangeBibleDatabaseSettings(m_pBibleDatabase->compatibilityUUID());
		}
	} else {
		m_pBibleDatabase.clear();
	}

	return bSuccess;
}

TBibleDescriptor CReadDatabase::discoverS3DBBibleDatabase(const QString &strFilePathName)
{
	Q_ASSERT(m_pBibleDatabase.isNull());		// Must be run on a new CReadDatabase object

	TBibleDescriptor bblDesc;
	bblDesc.m_strS3DBFilename = strFilePathName;
#ifndef NOT_USING_SQL
	QFileInfo fiS3DB(bblDesc.m_strS3DBFilename);
	if (fiS3DB.exists() && fiS3DB.isFile()) {
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(bblDesc));
		Q_ASSERT(!m_pBibleDatabase.isNull());

		m_myDatabase = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrMainReadConnection);
		m_myDatabase.setDatabaseName(fiS3DB.absoluteFilePath());
		m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

		if (m_myDatabase.open()) {
			if (ReadDBInfoTable()) {
				m_pBibleDatabase->m_descriptor.m_btoFlags |= BTO_Discovered;
				bblDesc = m_pBibleDatabase->m_descriptor;
			}
			m_myDatabase.close();
		}

		m_myDatabase = QSqlDatabase();
		QSqlDatabase::removeDatabase(g_constrMainReadConnection);

		m_pBibleDatabase.clear();
	}
#else
#endif	// !NOT_USING_SQL

	return bblDesc;
}

// ============================================================================

bool CReadDatabase::readDictionaryStub(bool bLiveDB)
{
	if ((!ReadDictionaryDBInfo()) ||
		(!ReadDictionaryWords(bLiveDB))) return false;
	return true;
}

bool CReadDatabase::ReadDictionaryDatabase(const TDictionaryDescriptor &dctDesc, bool bLiveDB, bool bSetAsMain)
{
	bool bSuccess = false;

	if (dctDesc.m_strCCDBFilename.isEmpty() && dctDesc.m_strS3DBFilename.isEmpty()) {
		// Empty filenames is special-case to refer back to a dictionary integrated in
		//	the Bible Database:
		TBibleDescriptor bblDesc = TBibleDatabaseList::availableBibleDatabaseDescriptor(dctDesc.m_strUUID);
		if (!bblDesc.isValid()) return false;		// We can't read the Dictionary Database if we can't read the Bible Database
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(dctDesc.m_strUUID);
		if (pBibleDatabase.isNull()) {
			if (!ReadBibleDatabase(bblDesc)) return false;		// Read the Bible Database to get the Dictionary Database, if it fails the Dictionary failed too
		}

		// Logic for Reading Specific Special Bible Database Internal Dictionaries:
		if (dctDesc.m_dtoFlags & DTO_Strongs) {		// Handle Strongs Dictionary:
			m_pDictionaryDatabase = QSharedPointer<CStrongsDictionaryDatabase>(new CStrongsDictionaryDatabase(dctDesc, pBibleDatabase));
			Q_ASSERT(!m_pDictionaryDatabase.isNull());

			m_pDictionaryDatabase->m_mapWordDefinitions.clear();		// These will remain empty and lookups done via the Strongs portion of the Bible Database
			m_pDictionaryDatabase->m_lstWordList.clear();

			TDictionaryDatabaseList::instance()->addDictionaryDatabase(m_pDictionaryDatabase, bSetAsMain);
			return true;
		}

		return false;
	}

	m_pDictionaryDatabase = QSharedPointer<CDictionaryDatabase>(new CDictionaryDatabase(dctDesc));
	Q_ASSERT(!m_pDictionaryDatabase.isNull());

	QFileInfo fiSQL(dctDesc.m_strS3DBFilename);
	QFileInfo fiCC(dctDesc.m_strCCDBFilename);

	// Prefer CC database over SQL in our search order:
	if (!bSuccess && fiCC.exists() && fiCC.isFile()) {
		QFile fileCCDB;
		fileCCDB.setFileName(fiCC.absoluteFilePath());
		if (!fileCCDB.open(QIODevice::ReadOnly)) {
#ifdef Q_OS_ANDROID
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open CC database file \"%1\".", "ReadDB").arg(fiCC.absoluteFilePath()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open CC database file \"%1\".", "ReadDB").arg(fiCC.absoluteFilePath()));
		} else {
			QtIOCompressor compCCDB(&fileCCDB);
			compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
			if (!compCCDB.open(QIODevice::ReadOnly)) {
				displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Failed to open i/o compressor for file \"%1\".", "ReadDB").arg(fiCC.absoluteFilePath()));
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
			__android_log_print(ANDROID_LOG_FATAL, "KJPBS", QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2", "ReadDB").arg(fiSQL.absoluteFilePath()).arg(m_pDictionaryDatabase->m_myDatabase.lastError().text()).toUtf8().data());
#endif
			displayWarning(m_pParent, g_constrReadDatabase, QObject::tr("Error: Couldn't open SQL database file \"%1\".\n\n%2", "ReadDB").arg(fiSQL.absoluteFilePath()).arg(m_pDictionaryDatabase->m_myDatabase.lastError().text()));
			m_pDictionaryDatabase->m_myDatabase = QSqlDatabase();
			QSqlDatabase::removeDatabase(dctDesc.m_strUUID);
		} else {
			if (readDictionaryStub(bLiveDB)) bSuccess = true;
		}

		if ((!bLiveDB) || (!bSuccess)) {
			Q_ASSERT(m_pDictionaryDatabase->m_myDatabase.contains(dctDesc.m_strUUID));
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

TDictionaryDescriptor CReadDatabase::discoverCCDBDictionaryDatabase(const QString &strFilePathName)
{
	Q_ASSERT(m_pDictionaryDatabase.isNull());		// Must be run on a new CReadDatabase object

	TDictionaryDescriptor dctDesc = { DTO_None, "", "", "", "", "", "" };
	dctDesc.m_strCCDBFilename = strFilePathName;

	// TODO : NOTE: For Dictionary Databases the UUID, Language, DBName,
	//	and DBDesc are NOT contained in the database file itself like
	//	it is for the Bible Databases.  This currently means we don't
	//	know how to do discovery for Dictionary Databases in any
	//	meaningful way without having an internal descriptor.
	//	Figure out how to work around this!!

#if (0)
	QFileInfo fiCCDB(dctDesc.m_strCCDBFilename);
	if (fiCCDB.exists() && fiCCDB.isFile()) {
		m_pDictionaryDatabase = QSharedPointer<CDictionaryDatabase>(new CDictionaryDatabase(dctDesc));
		Q_ASSERT(!m_pDictionaryDatabase.isNull());

		QFile fileCCDB;
		fileCCDB.setFileName(fiCCDB.absoluteFilePath());
		if (fileCCDB.open(QIODevice::ReadOnly)) {
			QtIOCompressor compCCDB(&fileCCDB);
			compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
			if (compCCDB.open(QIODevice::ReadOnly)) {
				CScopedCSVStream ccdb(m_pCCDatabase, new CCSVStream(&compCCDB));
				if (ReadDictionaryDBInfo()) {
					m_pDictionaryDatabase->m_descriptor.m_dtoFlags |= DTO_Discovered;
					dctDesc = m_pDictionaryDatabase->m_descriptor;
				}
			}
		}

		m_pDictionaryDatabase.clear();
	}
#endif

	return dctDesc;
}

TDictionaryDescriptor CReadDatabase::discoverS3DBDictionaryDatabase(const QString &strFilePathName)
{
	Q_ASSERT(m_pDictionaryDatabase.isNull());		// Must be run on a new CReadDatabase object

	TDictionaryDescriptor dctDesc = { DTO_None, "", "", "", "", "", "" };
	dctDesc.m_strS3DBFilename = strFilePathName;

	// TODO : NOTE: For Dictionary Databases the UUID, Language, DBName,
	//	and DBDesc are NOT contained in the database file itself like
	//	it is for the Bible Databases.  This currently means we don't
	//	know how to do discovery for Dictionary Databases in any
	//	meaningful way without having an internal descriptor.
	//	Figure out how to work around this!!

#if (0)

#ifndef NOT_USING_SQL
	QFileInfo fiS3DB(dctDesc.m_strS3DBFilename);
	if (fiS3DB.exists() && fiS3DB.isFile()) {
		m_pDictionaryDatabase = QSharedPointer<CDictionaryDatabase>(new CDictionaryDatabase(dctDesc));
		Q_ASSERT(!m_pDictionaryDatabase.isNull());

		m_pDictionaryDatabase->m_myDatabase = QSqlDatabase::addDatabase(g_constrDatabaseType, dctDesc.m_strUUID);
		m_pDictionaryDatabase->m_myDatabase.setDatabaseName(fiS3DB.absoluteFilePath());
		m_pDictionaryDatabase->m_myDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

		if (m_pDictionaryDatabase->m_myDatabase.open()) {
			if (ReadDictionaryDBInfo()) {
				m_pDictionaryDatabase->m_descriptor.m_dtoFlags |= DTO_Discovered;
				dctDesc = m_pDictionaryDatabase->m_descriptor;
			}
			m_pDictionaryDatabase->m_myDatabase.close();
		}

		m_pDictionaryDatabase->m_myDatabase = QSqlDatabase();
		QSqlDatabase::removeDatabase(dctDesc.m_strUUID);

		m_pDictionaryDatabase.clear();
	}
#else
#endif	// !NOT_USING_SQL

#endif

	return dctDesc;
}

// ============================================================================
