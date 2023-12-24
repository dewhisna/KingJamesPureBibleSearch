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

// BuildDB.cpp -- Code to build the initial database from csv data files
//

#include "BuildDB.h"
#include "dbstruct.h"
#include "CSV.h"
#include "PhraseListModel.h"
#include "ReportError.h"

#ifndef NOT_USING_SQL
#include <QtSql>
#include <QSqlQuery>
#endif

#include <QObject>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QtIOCompressor>

// ============================================================================

namespace {
//:BuildDB
	const QString g_constrBuildDatabase = QObject::tr("Building Database", "BuildDB");

#ifndef NOT_USING_SQL
	const QString g_constrDatabaseType = "QSQLITE";
	const QString g_constrMainBuildConnection = "MainBuildConnection";
	const QString g_constrUserBuildConnection = "UserBuildConnection";
#endif

}		// Namespace

// ============================================================================

CBuildDatabase::CBuildDatabase(QWidget *pParent)
	:	m_pParent(pParent)
{

}

CBuildDatabase::~CBuildDatabase()
{
#ifndef NOT_USING_SQL
	if (QSqlDatabase::contains(g_constrMainBuildConnection)) QSqlDatabase::removeDatabase(g_constrMainBuildConnection);
	if (QSqlDatabase::contains(g_constrUserBuildConnection)) QSqlDatabase::removeDatabase(g_constrUserBuildConnection);
#endif
}

// ============================================================================

#ifndef NOT_USING_SQL
static QByteArray convertCSVStringToIndexBlob(const QString &str)
{
	QString strBuff = str;
	CCSVStream csv(&strBuff, QIODevice::ReadOnly);
	QStringList slValues;
	csv >> slValues;

	// Special case handle empty lists or else CCSVStream will process it as
	//  a list with one empty member:
	if ((slValues.size() == 1) && (slValues.at(0).isEmpty())) return QByteArray();

	QByteArray baBlob(slValues.size()*sizeof(uint32_t), 0);
	uint32_t nValue;
	for (int i=0; i<slValues.size(); ++i) {
		nValue = slValues.at(i).toUInt();
		Q_ASSERT(nValue != 0);
		for (unsigned int j=1; j<=sizeof(uint32_t); ++j) {
			baBlob[static_cast<unsigned int>((i*sizeof(uint32_t))+(sizeof(uint32_t)-j))] = (nValue & 0xFF);
			nValue = nValue >> 8;
		}
	}
	Q_ASSERT((baBlob.size() % sizeof(uint32_t)) == 0);
	Q_ASSERT(baBlob.size() == static_cast<int>(slValues.size() * sizeof(uint32_t)));

	return baBlob;
}
#endif	// !NOT_USING_SQL

// ============================================================================

bool CBuildDatabase::BuildDBInfoTable()
{
	// Build Database Information Table:

#ifndef NOT_USING_SQL
	QString strCmd;

	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='DBInfo'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"DBInfo\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE DBInfo")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"DBInfo\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table DBInfo "
							"(ndx INTEGER PRIMARY KEY, uuid TEXT, Language TEXT, Name TEXT, Description TEXT, Direction TEXT, Info BLOB)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for 'DBInfo'\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QSettings settingsDBInfo(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/DBInfo.ini")).absoluteFilePath(), QSettings::IniFormat);
#if QT_VERSION < 0x060000
	settingsDBInfo.setIniCodec("UTF-8");
#endif
	settingsDBInfo.beginGroup("BibleDBInfo");
	QString strDBLang = settingsDBInfo.value("Language").toString();
	QString strDBDirection = settingsDBInfo.value("Direction").toString();
	QString strDBName = settingsDBInfo.value("Name").toString();
	QString strDBDesc = settingsDBInfo.value("Description").toString();
	QString strDBUUID = settingsDBInfo.value("UUID").toString();
	QString strDBInfoFilename = settingsDBInfo.value("InfoFilename").toString();
	settingsDBInfo.endGroup();

	QByteArray arrDBInfo;
	if (!strDBInfoFilename.isEmpty()) {
		QFile fileDBInfo(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/%1").arg(strDBInfoFilename)).absoluteFilePath());

		while (1) {
			if (!fileDBInfo.open(QIODevice::ReadOnly)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
									QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(fileDBInfo.fileName()),
									(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
			arrDBInfo = fileDBInfo.readAll();
			fileDBInfo.close();
			break;
		}
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		QSqlQuery queryInsert(m_myDatabase);
		queryInsert.exec("BEGIN TRANSACTION");

		// Note: We have redefined 'ndx' to be version number to keep in sync with
		//	the CCDB, etc.
		strCmd = QString("INSERT INTO DBInfo "
							"(ndx, uuid, Language, Name, Description, Direction, Info) "
							"VALUES (:ndx, :uuid, :Language, :Name, :Description, :Direction, :Info)");
		queryInsert.prepare(strCmd);
		queryInsert.bindValue(":ndx", KJPBS_CCDB_VERSION);
		queryInsert.bindValue(":uuid", strDBUUID);
		queryInsert.bindValue(":Language", strDBLang);
		queryInsert.bindValue(":Name", strDBName);
		queryInsert.bindValue(":Description", strDBDesc);
		queryInsert.bindValue(":Direction", strDBDirection);
		queryInsert.bindValue(":Info", arrDBInfo, QSql::In | QSql::Binary);

		if (!queryInsert.exec()) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for 'DBInfo'!\n%1", "BuildDB").arg(queryInsert.lastError().text()),
									(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				return false;
			}
		}

		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Version 1:
		//		Format:  KJPBSDB, version, $uuid, $Language, $Name, $Description, $Info
		// Version 2:
		//		Format:  KJPBSDB, version, $uuid, $Language, $Name, $Description, $Direction, $Info
		QStringList arrCCData;
		arrCCData.append("KJPBSDB");
		arrCCData.append(QString("%1").arg(KJPBS_CCDB_VERSION));
		arrCCData.append(strDBUUID);
		arrCCData.append(strDBLang);
		arrCCData.append(strDBName);
		arrCCData.append(strDBDesc);
		arrCCData.append(strDBDirection);
		arrCCData.append(QString(arrDBInfo));
		(*m_pCCDatabase) << arrCCData;
	}

	return true;
}

bool CBuildDatabase::BuildTestamentTable()
{
	// Build the TESTAMENT table:

#ifndef NOT_USING_SQL
	QString strCmd;

	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TESTAMENT'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"TESTAMENT\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE TESTAMENT")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"TESTAMENT\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table TESTAMENT "
							"(TstNdx INTEGER PRIMARY KEY, TstName TEXT)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for TESTAMENT\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QFile fileTestament(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/TESTAMENT.csv")).absoluteFilePath());
	while (1) {
		if (!fileTestament.open(QIODevice::ReadOnly)) {
			if (displayWarning(m_pParent, g_constrBuildDatabase,
								QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(fileTestament.fileName()),
								(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
		} else break;
	}

	// Read file and populate table:
	CCSVStream csv(&fileTestament);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=2) ||
		(slHeaders.at(0).compare("TstNdx") != 0) ||
		(slHeaders.at(1).compare("TstName") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for TESTAMENT data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			fileTestament.close();
			return false;
		}
	}

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QList<QStringList> lstArrCCData;

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 2) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in TESTAMENT data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileTestament.close();
				return false;
			}
			continue;
		}

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO TESTAMENT "
								"(TstNdx, TstName) "
								"VALUES (:TstNdx, :TstName)");
			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":TstNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":TstName", sl.at(1));

			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for TESTAMENT!\n%1\n  %2  %3", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(1)),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
					fileTestament.close();
					return false;
				}
			}
		}
#endif	// !NOT_USING_SQL

		// Format:  tstNdx,tstName
		lstArrCCData.append(sl);
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Format:  TESTAMENT,count
		QStringList arrCCData;
		arrCCData.append("TESTAMENT");
		arrCCData.append(QString("%1").arg(lstArrCCData.size()));
		(*m_pCCDatabase) << arrCCData;
		m_pCCDatabase->writeAll(lstArrCCData);
	}

	fileTestament.close();

	return true;
}

bool CBuildDatabase::BuildBooksTable()
{
	// Build the Books table:

	m_lststrBkTblNames.clear();

#ifndef NOT_USING_SQL
	QString strCmd;

	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TOC'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"TOC\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE TOC")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"TOC\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table TOC "
							"(BkNdx INTEGER PRIMARY KEY, TstBkNdx NUMERIC, TstNdx NUMERIC, "
							"BkName TEXT, BkAbbr TEXT, TblName TEXT, "
							"NumChp NUMERIC, NumVrs NUMERIC, NumWrd NUMERIC, "
							"Cat TEXT, Desc TEXT)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for TOC\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QFile fileBook(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/TOC.csv")).absoluteFilePath());
	while (1) {
		if (!fileBook.open(QIODevice::ReadOnly)) {
			if (displayWarning(m_pParent, g_constrBuildDatabase,
					QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(fileBook.fileName()),
					(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
		} else break;
	}

	// Read file and populate table:
	CCSVStream csv(&fileBook);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=11) ||
		(slHeaders.at(0).compare("BkNdx") != 0) ||
		(slHeaders.at(1).compare("TstBkNdx") != 0) ||
		(slHeaders.at(2).compare("TstNdx") != 0) ||
		(slHeaders.at(3).compare("BkName") != 0) ||
		(slHeaders.at(4).compare("BkAbbr") != 0) ||
		(slHeaders.at(5).compare("TblName") != 0) ||
		(slHeaders.at(6).compare("NumChp") != 0) ||
		(slHeaders.at(7).compare("NumVrs") != 0) ||
		(slHeaders.at(8).compare("NumWrd") != 0) ||
		(slHeaders.at(9).compare("Cat") != 0) ||
		(slHeaders.at(10).compare("Desc") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for TOC data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			fileBook.close();
			return false;
		}
	}

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QList<QStringList> lstArrCCData;

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 11) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in TOC data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileBook.close();
				return false;
			}
			continue;
		}

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO TOC "
							"(BkNdx, TstBkNdx, TstNdx, BkName, BkAbbr, TblName, "
							"NumChp, NumVrs, NumWrd, Cat, Desc) "
							"VALUES (:BkNdx, :TstBkNdx, :TstNdx, :BkName, :BkAbbr, :TblName, "
							":NumChp, :NumVrs, :NumWrd, :Cat, :Desc)");
			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":BkNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":TstBkNdx", sl.at(1).toUInt());
			queryInsert.bindValue(":TstNdx", sl.at(2).toUInt());
			queryInsert.bindValue(":BkName", sl.at(3));
			queryInsert.bindValue(":BkAbbr", sl.at(4));
			queryInsert.bindValue(":TblName", sl.at(5));
			queryInsert.bindValue(":NumChp", sl.at(6).toUInt());
			queryInsert.bindValue(":NumVrs", sl.at(7).toUInt());
			queryInsert.bindValue(":NumWrd", sl.at(8).toUInt());
			queryInsert.bindValue(":Cat", sl.at(9));
			queryInsert.bindValue(":Desc", sl.at(10));

			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for TOC!\n%1\n  %2  %3", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(3)),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
#endif	// !NOT_USING_SQL

		// Format:  bkNdx, tstBkNdx, tstNdx, bkName, bkAbbr, tblName, numChp, numVrs, numWrd, cat, desc
		lstArrCCData.append(sl);

		unsigned int nBk = sl.at(0).toUInt();	// nBk will be 1-originated
		if (nBk != 0) {
			// Make sure our list contains at least nBk books:
			for (unsigned int ndxBk = m_lststrBkTblNames.size(); ndxBk < nBk; ++ndxBk) {
				m_lststrBkTblNames.append(QString());
			}
			// Set this book's table name:
			m_lststrBkTblNames[nBk-1] = sl.at(5);	// (TblName)
		}
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Format:  TOC,count
		QStringList arrCCData;
		arrCCData.append("TOC");
		arrCCData.append(QString("%1").arg(lstArrCCData.size()));
		(*m_pCCDatabase) << arrCCData;
		m_pCCDatabase->writeAll(lstArrCCData);
	}

	fileBook.close();

	return true;
}

bool CBuildDatabase::BuildChaptersTable()
{
	// Build the Chapters (LAYOUT) table:

#ifndef NOT_USING_SQL
	QString strCmd;
	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='LAYOUT'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"LAYOUT\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE LAYOUT")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"LAYOUT\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table LAYOUT "
							"(BkChpNdx INTEGER PRIMARY KEY, NumVrs NUMERIC, NumWrd NUMERIC, BkAbbr TEXT, ChNdx NUMERIC)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for LAYOUT\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QFile fileBook(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/LAYOUT.csv")).absoluteFilePath());
	while (1) {
		if (!fileBook.open(QIODevice::ReadOnly)) {
			if (displayWarning(m_pParent, g_constrBuildDatabase,
					QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(fileBook.fileName()),
					(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
		} else break;
	}

	// Read file and populate table:
	CCSVStream csv(&fileBook);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=5) ||
		(slHeaders.at(0).compare("BkChpNdx") != 0) ||
		(slHeaders.at(1).compare("NumVrs") != 0) ||
		(slHeaders.at(2).compare("NumWrd") != 0) ||
		(slHeaders.at(3).compare("BkAbbr") != 0) ||
		(slHeaders.at(4).compare("ChNdx") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for LAYOUT data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			fileBook.close();
			return false;
		}
	}

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QList<QStringList> lstArrCCData;

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 5) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in LAYOUT data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileBook.close();
				return false;
			}
			continue;
		}

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO LAYOUT "
							"(BkChpNdx, NumVrs, NumWrd, BkAbbr, ChNdx) "
							"VALUES (:BkChpNdx, :NumVrs, :NumWrd, :BkAbbr, :ChNdx)");
			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":BkChpNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":NumVrs", sl.at(1).toUInt());
			queryInsert.bindValue(":NumWrd", sl.at(2).toUInt());
			queryInsert.bindValue(":BkAbbr", sl.at(3));
			queryInsert.bindValue(":ChNdx", sl.at(4).toUInt());

			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for LAYOUT!\n%1\n  %2  %3", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(3)).arg(sl.at(4)),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
#endif	// !NOT_USING_SQL

		// Format:  bkChpNdx, numVrs, numWrd, bkAbbr, chNdx
		lstArrCCData.append(sl);
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Format:  LAYOUT,count
		QStringList arrCCData;
		arrCCData.append("LAYOUT");
		arrCCData.append(QString("%1").arg(lstArrCCData.size()));
		(*m_pCCDatabase) << arrCCData;
		m_pCCDatabase->writeAll(lstArrCCData);
	}

	fileBook.close();

	return true;
}

bool CBuildDatabase::BuildVerseTables()
{
	// Build the Book Verses tables:

	int nBooksProcessed = 0;
	int nBooksExpected = 0;

	for (int i=0; i<m_lststrBkTblNames.size(); ++i) {
		if (m_lststrBkTblNames.at(i).isEmpty()) continue;
		++nBooksExpected;

		QFileInfo fiBook(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/BOOK_%1_%2.csv").arg(i+1, 2, 10, QChar('0')).arg(m_lststrBkTblNames.at(i)));

#ifndef NOT_USING_SQL
		QString strCmd;
		if (m_myDatabase.isOpen()) {
			QSqlQuery queryCreate(m_myDatabase);

			// Check to see if the table exists already:
			queryCreate.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=:table_name");
			queryCreate.bindValue(":table_name", m_lststrBkTblNames.at(i));
			if (!queryCreate.exec()) {
				displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"%1\" Failed!\n%2", "BuildDB").arg(m_lststrBkTblNames.at(i)).arg(queryCreate.lastError().text()));
				return false;
			}
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec(QString("DROP TABLE %1").arg(m_lststrBkTblNames.at(i)))) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"%1\" table from database!\n%2", "BuildDB").arg(m_lststrBkTblNames.at(i)).arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Don't create the database if we don't have the datafile:
			if ((!fiBook.exists()) || (!fiBook.isFile())) continue;

			// Create the table in the database:
			strCmd = QString("create table %1 "
							"(ChpVrsNdx INTEGER PRIMARY KEY, NumWrd NUMERIC, nPilcrow NUMERIC, PText TEXT, RText TEXT, TText TEXT)").arg(m_lststrBkTblNames.at(i));

			if (!queryCreate.exec(strCmd)) {
				displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to create table for %1\n%2", "BuildDB").arg(m_lststrBkTblNames.at(i)).arg(queryCreate.lastError().text()));
				return false;
			}
		}
#endif	// !NOT_USING_SQL

		// Open the table data file:
		if ((!fiBook.exists()) || (!fiBook.isFile())) continue;
		QFile fileBook(fiBook.absoluteFilePath());
		while (1) {
			if (!fileBook.open(QIODevice::ReadOnly)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(fileBook.fileName()),
						(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else break;
		}
		++nBooksProcessed;

		// Read file and populate table:
		CCSVStream csv(&fileBook);

		QStringList slHeaders;
		csv >> slHeaders;              // Read Headers (verify and discard)

		if ((slHeaders.size()!=6) ||
			(slHeaders.at(0).compare("ChpVrsNdx") != 0) ||
			(slHeaders.at(1).compare("NumWrd") != 0) ||
			(slHeaders.at(2).compare("nPilcrow") != 0) ||
			(slHeaders.at(3).compare("PText") != 0) ||
			(slHeaders.at(4).compare("RText") != 0) ||
			(slHeaders.at(5).compare("TText") != 0)) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for %1 data file!", "BuildDB").arg(m_lststrBkTblNames.at(i)),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileBook.close();
				return false;
			}
			fileBook.close();
			continue;
		}

#ifndef NOT_USING_SQL
		QSqlQuery queryInsert(m_myDatabase);
		if (m_myDatabase.isOpen()) {
			queryInsert.exec("BEGIN TRANSACTION");
		}
#endif	// !NOT_USING_SQL

		QList<QStringList> lstArrCCData;

		while (!csv.atEnd()) {
			QStringList sl;
			csv >> sl;

			if (sl.count() != 6) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in %1 data file!", "BuildDB").arg(m_lststrBkTblNames.at(i)),
									(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
					fileBook.close();
					return false;
				}
				continue;
			}

			// No need to have both plain text and rich text in our database, since
			//	we only need one data source for these.  So if we have Rich, we'll
			//	use that, but leave a placeholder for Plain.  Otherwise, we'll leave
			//	the placeholder for Rich and use Plain:
			sl[3] = sl[3].trimmed();
			sl[4] = sl[4].trimmed();
			if (!sl.at(4).isEmpty()) sl[3].clear();

#ifndef NOT_USING_SQL
			if (m_myDatabase.isOpen()) {
				strCmd = QString("INSERT INTO %1 "
							"(ChpVrsNdx, NumWrd, nPilcrow, PText, RText, TText) "
							"VALUES (:ChpVrsNdx, :NumWrd, :nPilcrow, :PText, :RText, :TText)").arg(m_lststrBkTblNames.at(i));

				queryInsert.prepare(strCmd);
				queryInsert.bindValue(":ChpVrsNdx", sl.at(0).toUInt());
				queryInsert.bindValue(":NumWrd", sl.at(1).toUInt());
				queryInsert.bindValue(":nPilcrow", sl.at(2).toInt());
				queryInsert.bindValue(":PText", sl.at(3));
				queryInsert.bindValue(":RText", sl.at(4));
				queryInsert.bindValue(":TText", sl.at(5));
				if (!queryInsert.exec()) {
					if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed!\n%1\n  %2  %3", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(3)),
											(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
				}
			}
#endif	// !NOT_USING_SQL

			// Format:  chpVrsNdx, numWrd, nPilcrow, PText, RText, TText
			lstArrCCData.append(sl);
		}

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			queryInsert.exec("COMMIT");
		}
#endif	// !NOT_USING_SQL

		if (!m_pCCDatabase.isNull()) {
			// Format:  tblName,count
			QStringList arrCCData;
			arrCCData.append(QString("%1").arg(m_lststrBkTblNames.at(i)));
			arrCCData.append(QString("%1").arg(lstArrCCData.size()));
			(*m_pCCDatabase) << arrCCData;
			m_pCCDatabase->writeAll(lstArrCCData);
		}

		fileBook.close();
	}

	if (nBooksProcessed != nBooksExpected) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Processed %1 Books.  Expected %2 books!", "BuildDB")
							.arg(nBooksProcessed).arg(nBooksExpected),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	}

	return true;
}

bool CBuildDatabase::BuildWordsTable()
{
	// Build the Words table:

#ifndef NOT_USING_SQL
	QString strCmd;
	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='WORDS'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"WORDS\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE WORDS")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"WORDS\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table WORDS "
							"(WrdNdx INTEGER PRIMARY KEY, Word TEXT, bIndexCasePreserve NUMERIC, NumTotal NUMERIC, AltWords TEXT, AltWordCounts TEXT, NormalMap BLOB)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for WORDS\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QFile fileBook(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/WORDS.csv")).absoluteFilePath());
	while (1) {
		if (!fileBook.open(QIODevice::ReadOnly)) {
			if (displayWarning(m_pParent, g_constrBuildDatabase,
					QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(fileBook.fileName()),
					(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
		} else break;
	}

	// Read file and populate table:
	CCSVStream csv(&fileBook);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=7) ||
		(slHeaders.at(0).compare("WrdNdx") != 0) ||
		(slHeaders.at(1).compare("Word") != 0) ||
		(slHeaders.at(2).compare("bIndexCasePreserve") != 0) ||
		(slHeaders.at(3).compare("NumTotal") != 0) ||
		(slHeaders.at(4).compare("AltWords") != 0) ||
		(slHeaders.at(5).compare("AltWordCounts") != 0) ||
		(slHeaders.at(6).compare("NormalMap") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for WORDS data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			fileBook.close();
			return false;
		}
	}

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QList<QStringList> lstArrCCData;

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 7) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in WORDS data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileBook.close();
				return false;
			}
			continue;
		}

		// NOTE: bIndexCasePreserve is no longer just the SpecialWord CasePreserve flag.  It's now a special word processing
		//			bit-field.  For backward compatibility, the low-bit is still the boolean flag for SpecialWord CasePreserve.
		//			The next to lsbit is the IsProperWord flag for words which have all alternate forms with initial uppercase,
		//			excluding specialized hyphen formed "Ordinary Words", like "God-ward", as mapped out in the KJVDataParse tool.

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO WORDS "
							"(WrdNdx, Word, bIndexCasePreserve, NumTotal, AltWords, AltWordCounts, NormalMap) "
							"VALUES (:WrdNdx, :Word, :bIndexCasePreserve, :NumTotal, :AltWords, :AltWordCounts, :NormalMap)");
			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":WrdNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":Word", sl.at(1));
			queryInsert.bindValue(":bIndexCasePreserve", sl.at(2).toInt());
			queryInsert.bindValue(":NumTotal", sl.at(3).toUInt());
			queryInsert.bindValue(":AltWords", sl.at(4));
			queryInsert.bindValue(":AltWordCounts", sl.at(5));
			queryInsert.bindValue(":NormalMap", convertCSVStringToIndexBlob(sl.at(6)), QSql::In | QSql::Binary);

			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for WORDS!\n%1\n  %2  (%3)", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(1)).arg(sl.at(4)),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
#endif	// !NOT_USING_SQL

		// Format:  WrdNdx, Word, bIndexCasePreserve, NumTotal, AltWords, AltWordCounts, NormalMap
		lstArrCCData.append(sl);
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Format:  WORDS,count
		QStringList arrCCData;
		arrCCData.append("WORDS");
		arrCCData.append(QString("%1").arg(lstArrCCData.size()));
		(*m_pCCDatabase) << arrCCData;
		m_pCCDatabase->writeAll(lstArrCCData);
	}

	fileBook.close();

	return true;
}

bool CBuildDatabase::BuildFootnotesTables()
{
	// Build the Footnotes table:

#ifndef NOT_USING_SQL
	QString strCmd;
	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='FOOTNOTES'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"FOOTNOTES\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE FOOTNOTES")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"FOOTNOTES\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table FOOTNOTES "
							"(BkChpVrsWrdNdx INTEGER PRIMARY KEY, PFootnote TEXT, RFootnote TEXT)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for FOOTNOTES\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QFile fileFootnotes(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/FOOTNOTES.csv")).absoluteFilePath());
	while (1) {
		if (!fileFootnotes.open(QIODevice::ReadOnly)) {
			if (displayWarning(m_pParent, g_constrBuildDatabase,
					QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(fileFootnotes.fileName()),
					(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
		} else break;
	}

	// Read file and populate table:
	CCSVStream csv(&fileFootnotes);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=3) ||
		(slHeaders.at(0).compare("BkChpVrsWrdNdx") != 0) ||
		(slHeaders.at(1).compare("PFootnote") != 0) ||
		(slHeaders.at(2).compare("RFootnote") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for FOOTNOTES data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			fileFootnotes.close();
			return false;
		}
	}

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QList<QStringList> lstArrCCData;

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 3) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in FOOTNOTES data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileFootnotes.close();
				return false;
			}
			continue;
		}

		// No need to have both plain text and rich text in our database, since
		//	we only need one data source for these.  So if we have Rich, we'll
		//	use that, but leave a placeholder for Plain.  Otherwise, we'll leave
		//	the placeholder for Rich and use Plain:
		sl[1] = sl[1].trimmed();
		sl[2] = sl[2].trimmed();
		if (!sl.at(2).isEmpty()) sl[1].clear();

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO FOOTNOTES "
							"(BkChpVrsWrdNdx, PFootnote, RFootnote) "
							"VALUES (:BkChpVrsWrdNdx, :PFootnote, :RFootnote)");

			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":BkChpVrsWrdNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":PFootnote", sl.at(1));
			queryInsert.bindValue(":RFootnote", sl.at(2));

			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for FOOTNOTES!\n%1\n%2\n%3\n%4", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(1)).arg(sl.at(2)),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
#endif	// !NOT_USING_SQL

		// Format:  BkChpVrsWrdNdx, PFootnote, RFootnote
		lstArrCCData.append(sl);
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Format:  FOOTNOTES,count
		QStringList arrCCData;
		arrCCData.append("FOOTNOTES");
		arrCCData.append(QString("%1").arg(lstArrCCData.size()));
		(*m_pCCDatabase) << arrCCData;
		m_pCCDatabase->writeAll(lstArrCCData);
	}

	fileFootnotes.close();

	return true;
}

bool CBuildDatabase::BuildPhrasesTable()
{
	// Build the Phrases table:

#ifndef NOT_USING_SQL
	QString strCmd;
	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='PHRASES'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"PHRASES\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE PHRASES")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"PHRASES\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table PHRASES "
							"(Ndx INTEGER PRIMARY KEY, Phrase TEXT, CaseSensitive NUMERIC, AccentSensitive NUMERIC, Exclude NUMERIC)");
			if (!queryCreate.exec(strCmd)) {
				displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for PHRASES\n%1", "BuildDB").arg(queryCreate.lastError().text()));
				return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	CPhraseList phrases;

	// If this is the main phrases table, open our data file for populating it:
	QFile filePhrases(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/PHRASES.csv")).absoluteFilePath());
	while (1) {
		if (!filePhrases.open(QIODevice::ReadOnly)) {
			if (displayWarning(m_pParent, g_constrBuildDatabase,
					QObject::tr("Failed to open %1 for reading.", "BuildDB").arg(filePhrases.fileName()),
					(QMessageBox::Retry | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
		} else break;
	}

	// Read file and populate phrase list:
	CCSVStream csv(&filePhrases);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=5) ||
		(slHeaders.at(0).compare("Ndx") != 0) ||
		(slHeaders.at(1).compare("Phrase") != 0) ||
		(slHeaders.at(2).compare("CaseSensitive") != 0) ||
		(slHeaders.at(3).compare("AccentSensitive") != 0) ||
		(slHeaders.at(4).compare("Exclude") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for PHRASES data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			filePhrases.close();
			return false;
		}
	}

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 5) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in PHRASES data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				filePhrases.close();
				return false;
			}
			continue;
		}

		CPhraseEntry phrase;
		phrase.setText(sl.at(1));
		phrase.setCaseSensitive((sl.at(2).toInt() != 0) ? true : false);
		phrase.setAccentSensitive((sl.at(3).toInt() != 0) ? true : false);
		phrase.setExclude((sl.at(4).toInt() != 0) ? true: false);
		if (!phrase.text().isEmpty()) {
			phrases.push_back(phrase);
		}
	}

	filePhrases.close();

	// Use the CPhraseListModel to sort it (since that will be displaying it later):
	CPhraseListModel mdlPhrases(phrases);
	mdlPhrases.sort(0, Qt::AscendingOrder);
	phrases = mdlPhrases.phraseList();

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QStringList arrCCData;

	if (!m_pCCDatabase.isNull()) {
		// Format:  PHRASES,count
		arrCCData.clear();
		arrCCData.append("PHRASES");
		arrCCData.append(QString("%1").arg(phrases.size()));
		(*m_pCCDatabase) << arrCCData;
	}

	for (int ndx=0; ndx<phrases.size(); ++ndx) {
#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO PHRASES "
									"(Ndx, Phrase, CaseSensitive, AccentSensitive, Exclude) "
									"VALUES (:Ndx, :Phrase, :CaseSensitive, :AccentSensitive, :Exclude)");
			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":Ndx", ndx+1);
			queryInsert.bindValue(":Phrase", phrases.at(ndx).text());
			queryInsert.bindValue(":CaseSensitive", (phrases.at(ndx).caseSensitive() ? 1 : 0));
			queryInsert.bindValue(":AccentSensitive", (phrases.at(ndx).accentSensitive() ? 1 : 0));
			queryInsert.bindValue(":Exclude", (phrases.at(ndx).isExcluded() ? 1 : 0));
			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for PHRASES!\n%1\n  %2  (%3)  (%4)  (%5)", "BuildDB")
																				.arg(queryInsert.lastError().text())
																				.arg(phrases.at(ndx).text())
																				.arg(phrases.at(ndx).caseSensitive() ? QObject::tr("Case", "BuildDB") : QObject::tr("NoCase", "BuildDB"))
																				.arg(phrases.at(ndx).accentSensitive() ? QObject::tr("Accent", "BuildDB") : QObject::tr("NoAccent", "BuildDB"))
																				.arg(phrases.at(ndx).isExcluded() ? QObject::tr("Exclude", "BuildDB") : QObject::tr("Include", "BuildDB")),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
#endif	// !NOT_USING_SQL

		if (!m_pCCDatabase.isNull()) {
			// Format:  Ndx, Phrase, CaseSensitive, AccentSensitive, Exclude
			arrCCData.clear();
			arrCCData.append(QString("%1").arg(ndx+1));
			arrCCData.append(phrases.at(ndx).text());
			arrCCData.append(QString("%1").arg(phrases.at(ndx).caseSensitive() ? 1 : 0));
			arrCCData.append(QString("%1").arg(phrases.at(ndx).accentSensitive() ? 1 : 0));
			arrCCData.append(QString("%1").arg(phrases.at(ndx).isExcluded() ? 1 : 0));
			(*m_pCCDatabase) << arrCCData;
		}
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	return true;
}

bool CBuildDatabase::BuildLemmasTable()
{
	// Build the Lemmas table:

#ifndef NOT_USING_SQL
	QString strCmd;
	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='LEMMAS'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"LEMMAS\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE LEMMAS")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"LEMMAS\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table LEMMAS "
							"(BkChpVrsWrdNdx INTEGER PRIMARY KEY, Count NUMERIC, Attrs TEXT)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for LEMMAS\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QFile fileLemmas(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/LEMMAS.csv")).absoluteFilePath());
	if (!fileLemmas.open(QIODevice::ReadOnly)) {
		displayInformation(m_pParent, g_constrBuildDatabase,
			QObject::tr("Failed to open %1 for reading.  Skipping Lemma Generation.", "BuildDB").arg(fileLemmas.fileName()));
		return true;
	}

	// Read file and populate table:
	CCSVStream csv(&fileLemmas);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=3) ||
		(slHeaders.at(0).compare("BkChpVrsWrdNdx") != 0) ||
		(slHeaders.at(1).compare("Count") != 0) ||
		(slHeaders.at(2).compare("Attrs") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for LEMMAS data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			fileLemmas.close();
			return false;
		}
	}

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QList<QStringList> lstArrCCData;

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 3) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in LEMMAS data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileLemmas.close();
				return false;
			}
			continue;
		}

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO LEMMAS "
							"(BkChpVrsWrdNdx, Count, Attrs) "
							"VALUES (:BkChpVrsWrdNdx, :Count, :Attrs)");

			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":BkChpVrsWrdNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":Count", sl.at(1).toUInt());
			queryInsert.bindValue(":Attrs", sl.at(2));

			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for LEMMAS!\n%1\n%2\n%3\n%4", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(1)).arg(sl.at(2)),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
#endif	// !NOT_USING_SQL

		// Format:  BkChpVrsWrdNdx, Count, Attrs
		lstArrCCData.append(sl);
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Format:  LEMMAS,count
		QStringList arrCCData;
		arrCCData.append("LEMMAS");
		arrCCData.append(QString("%1").arg(lstArrCCData.size()));
		(*m_pCCDatabase) << arrCCData;
		m_pCCDatabase->writeAll(lstArrCCData);
	}

	fileLemmas.close();

	return true;
}

bool CBuildDatabase::BuildStrongsTable()
{
	// Build the Strongs table:

#ifndef NOT_USING_SQL
	QString strCmd;
	if (m_myDatabase.isOpen()) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='STRONGS'")) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"STRONGS\" Failed!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
			return false;
		} else {
			queryCreate.next();
			if (queryCreate.value(0).toInt()) {
				// If we found it, drop it so we can recreate it:
				if (!queryCreate.exec("DROP TABLE STRONGS")) {
					displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"STRONGS\" table from database!\n%1", "BuildDB").arg(queryCreate.lastError().text()));
					return false;
				}
			}

			// Create the table in the database:
			strCmd = QString("create table STRONGS "
							"(StrongsMapNdx TEXT PRIMARY KEY, Orth TEXT, Trans TEXT, Pron TEXT, Def TEXT)");

			if (!queryCreate.exec(strCmd)) {
				if (displayWarning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for STRONGS\n%1", "BuildDB").arg(queryCreate.lastError().text()),
						(QMessageBox::Ignore | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			}
		}
	}
#endif	// !NOT_USING_SQL

	// Open the table data file:
	QFile fileStrongs(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/STRONGS.csv")).absoluteFilePath());
	if (!fileStrongs.open(QIODevice::ReadOnly)) {
		displayInformation(m_pParent, g_constrBuildDatabase,
			QObject::tr("Failed to open %1 for reading.  Skipping Strongs Generation.", "BuildDB").arg(fileStrongs.fileName()));
		return true;
	}

	// Read file and populate table:
	CCSVStream csv(&fileStrongs);

	QStringList slHeaders;
	csv >> slHeaders;              // Read Headers (verify and discard)

	if ((slHeaders.size()!=5) ||
		(slHeaders.at(0).compare("StrongsMapNdx") != 0) ||
		(slHeaders.at(1).compare("Orth") != 0) ||
		(slHeaders.at(2).compare("Trans") != 0) ||
		(slHeaders.at(3).compare("Pron") != 0) ||
		(slHeaders.at(4).compare("Def") != 0)) {
		if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for STRONGS data file!", "BuildDB"),
							(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
			fileStrongs.close();
			return false;
		}
	}

#ifndef NOT_USING_SQL
	QSqlQuery queryInsert(m_myDatabase);
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("BEGIN TRANSACTION");
	}
#endif	// !NOT_USING_SQL

	QList<QStringList> lstArrCCData;

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;

		if (sl.count() != 5) {
			if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Bad table data in STRONGS data file!", "BuildDB"),
								(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileStrongs.close();
				return false;
			}
			continue;
		}

#ifndef NOT_USING_SQL
		if (m_myDatabase.isOpen()) {
			strCmd = QString("INSERT INTO STRONGS "
							"(StrongsMapNdx, Orth, Trans, Pron, Def) "
							"VALUES (:StrongsMapNdx, :Orth, :Trans, :Pron, :Def)");

			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":StrongsMapNdx", sl.at(0));
			queryInsert.bindValue(":Orth", sl.at(1));
			queryInsert.bindValue(":Trans", sl.at(2));
			queryInsert.bindValue(":Pron", sl.at(3));
			queryInsert.bindValue(":Def", sl.at(4));

			if (!queryInsert.exec()) {
				if (displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for STRONGS!\n%1\n%2\n%3\n%4\n%5\n%6", "BuildDB").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(1)).arg(sl.at(2)).arg(sl.at(3)).arg(sl.at(4)).arg(sl.at(5)),
										(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
#endif	// !NOT_USING_SQL

		// Format:  StrongsMapNdx,Orth,Trans,Pron,Def
		lstArrCCData.append(sl);
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) {
		queryInsert.exec("COMMIT");
	}
#endif	// !NOT_USING_SQL

	if (!m_pCCDatabase.isNull()) {
		// Format:  STRONGS,count
		QStringList arrCCData;
		arrCCData.append("STRONGS");
		arrCCData.append(QString("%1").arg(lstArrCCData.size()));
		(*m_pCCDatabase) << arrCCData;
		m_pCCDatabase->writeAll(lstArrCCData);
	}

	fileStrongs.close();

	return true;
}

bool CBuildDatabase::BuildVersificationTables()
{
	// Build the Versification tables:
	// The Versification Tables are already in the final
	//	format coming out of KJVDataParse and can just be
	//	copied straight in to the CCDB (not supported by
	//	the SQL databases).

	if (m_pCCDatabase.isNull()) return true;		// SQL doesn't support Versification tables

	// Open the table data file:
	QFile fileVersification(QFileInfo(QDir(TBibleDatabaseList::bibleDatabasePath()), QString("data/VERSIFICATION.csv")).absoluteFilePath());
	if (!fileVersification.exists()) return true;		// Versification is optional.  Silently skip if the file doesn't exist
	if (!fileVersification.open(QIODevice::ReadOnly)) {
		displayInformation(m_pParent, g_constrBuildDatabase,
			QObject::tr("Failed to open %1 for reading.  Skipping Versification Generation.", "BuildDB").arg(fileVersification.fileName()));
		return true;
	}

	// Read file and populate table:
	CCSVStream csv(&fileVersification);

	while (!csv.atEnd()) {
		QStringList sl;
		csv >> sl;
		(*m_pCCDatabase) << sl;
	}

	fileVersification.close();

	return true;
}

bool CBuildDatabase::BuildDatabase(const QString &strSQLDatabaseFilename, const QString &strCCDatabaseFilename)
{
#ifndef NOT_USING_SQL
	m_myDatabase = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrMainBuildConnection);
	m_myDatabase.setDatabaseName(strSQLDatabaseFilename);

//	displayInformation(m_pParent, g_constrBuildDatabase, m_myDatabase.databaseName());
#else
	Q_UNUSED(strSQLDatabaseFilename);
#endif

	bool bSuccess = true;

#ifndef NOT_USING_SQL
	if ((bSuccess) && (!strSQLDatabaseFilename.isEmpty()) && (!m_myDatabase.open())) {
		displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Error: Couldn't open SQL database file \"%1\".", "BuildDB").arg(strSQLDatabaseFilename));
		bSuccess = false;
	}
#endif

	QFile fileCCDB;
	if ((bSuccess) && (!strCCDatabaseFilename.isEmpty())) {
		fileCCDB.setFileName(strCCDatabaseFilename);
		if (!fileCCDB.open(QIODevice::WriteOnly)) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Error: Couldn't open CC database file \"%1\".", "BuildDB").arg(strCCDatabaseFilename));
			bSuccess = false;
		}
	}

	QtIOCompressor compCCDB(&fileCCDB);
	if ((bSuccess) && (fileCCDB.isOpen())) {
		compCCDB.setStreamFormat(QtIOCompressor::ZlibFormat);
		if (!compCCDB.open(QIODevice::WriteOnly)) {
			displayWarning(m_pParent, g_constrBuildDatabase, QObject::tr("Error: Failed to open i/o compressor for file \"%1\".", "BuildDB").arg(strCCDatabaseFilename));
			bSuccess = false;
		}
	}

	CScopedCSVStream ccdb(m_pCCDatabase, ((bSuccess && compCCDB.isOpen()) ? new CCSVStream(&compCCDB) : nullptr));

	if (bSuccess) {
		if ((!BuildDBInfoTable()) ||
			(!BuildTestamentTable()) ||
			(!BuildBooksTable()) ||
			(!BuildChaptersTable()) ||
			(!BuildVerseTables()) ||
			(!BuildWordsTable()) ||
			(!BuildFootnotesTables()) ||
			(!BuildPhrasesTable()) ||
			(!BuildLemmasTable()) ||
			(!BuildStrongsTable()) ||
			(!BuildVersificationTables())) bSuccess = false;
	}

#ifndef NOT_USING_SQL
	if (m_myDatabase.isOpen()) m_myDatabase.close();
	m_myDatabase = QSqlDatabase();
	QSqlDatabase::removeDatabase(g_constrMainBuildConnection);
#endif

	if (bSuccess) displayInformation(m_pParent, g_constrBuildDatabase, QObject::tr("Build Complete!", "BuildDB"));
	return bSuccess;
}

// ============================================================================

