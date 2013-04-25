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

// BuildDB.cpp -- Code to build the initial database from csv data files
//

#include "BuildDB.h"
#include "dbstruct.h"
#include "CSV.h"
#include "PhraseEdit.h"
#include "PhraseListModel.h"

#include <assert.h>

#include <QObject>
#include <QFile>

#include <QtSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QByteArray>

QSqlDatabase g_sqldbBuildMain;
QSqlDatabase g_sqldbBuildUser;

namespace {
//:BuildDB
	const QString g_constrBuildDatabase = QObject::tr("Building Database");

	// Book counts are here only on the building process as
	//  the reading side gets this data entirely from the
	//  database
	#define NUM_BK 66
	#define NUM_BK_OT 39
	#define NUM_BK_NT 27

	const char *g_arrstrBkTblNames[NUM_BK] =
			{	"GEN",
				"EXOD",
				"LEV",
				"NUM",
				"DEUT",
				"JOSH",
				"JUDG",
				"RUTH",
				"SAM1",
				"SAM2",
				"KGS1",
				"KGS2",
				"CHR1",
				"CHR2",
				"EZRA",
				"NEH",
				"ESTH",
				"JOB",
				"PS",
				"PROV",
				"ECCL",
				"SONG",
				"ISA",
				"JER",
				"LAM",
				"EZEK",
				"DAN",
				"HOS",
				"JOEL",
				"AMOS",
				"OBAD",
				"JONAH",
				"MIC",
				"NAH",
				"HAB",
				"ZEPH",
				"HAG",
				"ZECH",
				"MAL",
				"MATT",
				"MARK",
				"LUKE",
				"JOHN",
				"ACTS",
				"ROM",
				"COR1",
				"COR2",
				"GAL",
				"EPH",
				"PHIL",
				"COL",
				"THESS1",
				"THESS2",
				"TIM1",
				"TIM2",
				"TITUS",
				"PHLM",
				"HEB",
				"JAS",
				"PET1",
				"PET2",
				"JOHN1",
				"JOHN2",
				"JOHN3",
				"JUDE",
				"REV"
			};

}		// Namespace

// ============================================================================

CBuildDatabase::CBuildDatabase(QWidget *pParent)
	:	m_pParent(pParent)
{
	if (!g_sqldbBuildMain.contains("MainBuildConnection")) {
		g_sqldbBuildMain = QSqlDatabase::addDatabase("QSQLITE", "MainBuildConnection");
	}

	if (!g_sqldbBuildUser.contains("UserBuildConnection")) {
		g_sqldbBuildUser = QSqlDatabase::addDatabase("QSQLITE", "UserBuildConnection");
	}
}

// ============================================================================

bool CBuildDatabase::BuildTestamentTable()
{
	// Build the TESTAMENT table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TESTAMENT'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"TESTAMENT\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileTestament(QString("../KJVCanOpener/db/data/TESTAMENT.csv"));
			while (1) {
				if (!fileTestament.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QObject::tr("Failed to open %1 for reading.").arg(fileTestament.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Create the table in the database:
			strCmd = QString("create table TESTAMENT "
							"(TstNdx INTEGER PRIMARY KEY, TstName TEXT)");

			if (!queryCreate.exec(strCmd)) {
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for TESTAMENT\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
				// Read file and populate table:
				CCSVStream csv(&fileTestament);

				QStringList slHeaders;
				csv >> slHeaders;              // Read Headers (verify and discard)

				if ((slHeaders.size()!=2) ||
					(slHeaders.at(0).compare("TstNdx") != 0) ||
					(slHeaders.at(1).compare("TstName") != 0)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for TESTAMENT data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileTestament.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");

				while (!csv.atEndOfStream()) {
					QStringList sl;
					csv >> sl;

					assert(sl.count() == 2);
					if (sl.count() < 2) continue;

					strCmd = QString("INSERT INTO TESTAMENT "
										"(TstNdx, TstName) "
										"VALUES (:TstNdx, :TstName)");
					queryInsert.prepare(strCmd);
					queryInsert.bindValue(":TstNdx", sl.at(0).toUInt());
					queryInsert.bindValue(":TstName", sl.at(1));

					if (!queryInsert.exec()) {
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for TESTAMENT!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(1)),
												QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
							fileTestament.close();
							return false;
						}
					}
				}
				queryInsert.exec("COMMIT");

				fileTestament.close();
			}
		}
	}

	return true;
}

bool CBuildDatabase::BuildBooksTable()
{
	// Build the Books table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TOC'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"TOC\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileBook(QString("../KJVCanOpener/db/data/TOC.csv"));
			while (1) {
				if (!fileBook.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QObject::tr("Failed to open %1 for reading.").arg(fileBook.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Create the table in the database:
			strCmd = QString("create table TOC "
							"(BkNdx INTEGER PRIMARY KEY, TstBkNdx NUMERIC, TstNdx NUMERIC, "
							"BkName TEXT, BkAbbr TEXT, TblName TEXT, "
							"NumChp NUMERIC, NumVrs NUMERIC, NumWrd NUMERIC, "
							"Cat TEXT, Desc TEXT)");

			if (!queryCreate.exec(strCmd)) {
				fileBook.close();
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for TOC\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
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
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for TOC data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileBook.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");
				while (!csv.atEndOfStream()) {
					QStringList sl;
					csv >> sl;

					assert(sl.count() == 11);
					if (sl.count() < 11) continue;

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
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for TOC!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(3)),
												QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
					}
				}
				queryInsert.exec("COMMIT");

				fileBook.close();
			}
		}
	}

	return true;
}

bool CBuildDatabase::BuildChaptersTable()
{
	// Build the Chapters (LAYOUT) table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='LAYOUT'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"LAYOUT\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileBook(QString("../KJVCanOpener/db/data/LAYOUT.csv"));
			while (1) {
				if (!fileBook.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QObject::tr("Failed to open %1 for reading.").arg(fileBook.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Create the table in the database:
			strCmd = QString("create table LAYOUT "
							"(BkChpNdx INTEGER PRIMARY KEY, NumVrs NUMERIC, NumWrd NUMERIC, BkAbbr TEXT, ChNdx NUMERIC)");

			if (!queryCreate.exec(strCmd)) {
				fileBook.close();
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for LAYOUT\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
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
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for LAYOUT data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileBook.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");
				while (!csv.atEndOfStream()) {
					QStringList sl;
					csv >> sl;

					assert(sl.count() == 5);
					if (sl.count() < 5) continue;

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
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for LAYOUT!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(3)).arg(sl.at(4)),
												QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
					}
				}
				queryInsert.exec("COMMIT");

				fileBook.close();
			}
		}
	}

	return true;
}

bool CBuildDatabase::BuildVerseTables()
{
	QString strCmd;

	// Build the Book Verses tables:
	for (int i=0; i<NUM_BK; ++i) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		queryCreate.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=:table_name");
		queryCreate.bindValue(":table_name", g_arrstrBkTblNames[i]);
		if (!queryCreate.exec()) {
			if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"%1\" Failed!\n%2").arg(g_arrstrBkTblNames[i]).arg(queryCreate.lastError().text()),
									QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			continue;
		}
		queryCreate.next();
		if (queryCreate.value(0).toInt()) continue;             // If the table exists, skip it

		// Open the table data file:
		QFile fileBook(QString("../KJVCanOpener/db/data/BOOK_%1_%2.csv").arg(i+1, 2, 10, QChar('0')).arg(g_arrstrBkTblNames[i]));
		while (1) {
			if (!fileBook.open(QIODevice::ReadOnly)) {
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to open %1 for reading.").arg(fileBook.fileName()),
						QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else break;
		}

		// Create the table in the database:
		strCmd = QString("create table %1 "
						"(ChpVrsNdx INTEGER PRIMARY KEY, NumWrd NUMERIC, nPilcrow NUMERIC, PText TEXT, RText TEXT)").arg(g_arrstrBkTblNames[i]);

		if (!queryCreate.exec(strCmd)) {
			fileBook.close();
			if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
					QObject::tr("Failed to create table for %1\n%2").arg(g_arrstrBkTblNames[i]).arg(queryCreate.lastError().text()),
					QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			continue;
		}

		// Read file and populate table:
		CCSVStream csv(&fileBook);

		QStringList slHeaders;
		csv >> slHeaders;              // Read Headers (verify and discard)

		if ((slHeaders.size()!=5) ||
			(slHeaders.at(0).compare("ChpVrsNdx") != 0) ||
			(slHeaders.at(1).compare("NumWrd") != 0) ||
			(slHeaders.at(2).compare("nPilcrow") != 0) ||
			(slHeaders.at(3).compare("PText") != 0) ||
			(slHeaders.at(4).compare("RText") != 0)) {
			if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for %1 data file!").arg(g_arrstrBkTblNames[i]),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileBook.close();
				return false;
			}
			fileBook.close();
			continue;
		}

		QSqlQuery queryInsert(m_myDatabase);
		queryInsert.exec("BEGIN TRANSACTION");
		while (!csv.atEndOfStream()) {
			QStringList sl;
			csv >> sl;

			assert(sl.count() == 5);
			if (sl.count() < 5) continue;

			strCmd = QString("INSERT INTO %1 "
						"(ChpVrsNdx, NumWrd, nPilcrow, PText, RText) "
						"VALUES (:ChpVrsNdx, :NumWrd, :nPilcrow, :PText, :RText)").arg(g_arrstrBkTblNames[i]);

			// No need to have both plain text and rich text in our database, since
			//	we only need one data source for these.  So if we have Rich, we'll
			//	use that, but leave a placeholder for Plain.  Otherwise, we'll leave
			//	the placeholder for Rich and use Plain:
			sl[3] = sl[3].trimmed();
			sl[4] = sl[4].trimmed();
			if (!sl.at(4).isEmpty()) sl[3].clear();

			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":ChpVrsNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":NumWrd", sl.at(1).toUInt());
			queryInsert.bindValue(":nPilcrow", sl.at(2).toInt());
			queryInsert.bindValue(":PText", sl.at(3));
			queryInsert.bindValue(":RText", sl.at(4));
			if (!queryInsert.exec()) {
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(3)),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
		queryInsert.exec("COMMIT");

		fileBook.close();
	}

	return true;
}

QByteArray CBuildDatabase::CSVStringToIndexBlob(const QString &str)
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
		assert(nValue != 0);
		for (unsigned int j=1; j<=sizeof(uint32_t); ++j) {
			baBlob[static_cast<unsigned int>((i*sizeof(uint32_t))+(sizeof(uint32_t)-j))] = (nValue & 0xFF);
			nValue = nValue >> 8;
		}
	}
	assert((baBlob.size() % sizeof(uint32_t)) == 0);
	assert(baBlob.size() == static_cast<int>(slValues.size() * sizeof(uint32_t)));

	return baBlob;
}

bool CBuildDatabase::BuildWordsTable()
{
	// Build the Words table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='WORDS'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"WORDS\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileBook(QString("../KJVCanOpener/db/data/WORDS.csv"));
			while (1) {
				if (!fileBook.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QObject::tr("Failed to open %1 for reading.").arg(fileBook.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Create the table in the database:
			strCmd = QString("create table WORDS "
							"(WrdNdx INTEGER PRIMARY KEY, Word TEXT, bIndexCasePreserve NUMERIC, NumTotal NUMERIC, AltWords TEXT, AltWordCounts TEXT, NormalMap BLOB)");

			if (!queryCreate.exec(strCmd)) {
				fileBook.close();
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for WORDS\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
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
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for WORDS data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileBook.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");
				while (!csv.atEndOfStream()) {
					QStringList sl;
					csv >> sl;

					assert(sl.count() == 7);
					if (sl.count() < 7) continue;

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
					queryInsert.bindValue(":NormalMap", CSVStringToIndexBlob(sl.at(6)), QSql::In | QSql::Binary);

					if (!queryInsert.exec()) {
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for WORDS!\n%1\n  %2  (%3)").arg(queryInsert.lastError().text()).arg(sl.at(1)).arg(sl.at(4)),
												QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
					}
				}
				queryInsert.exec("COMMIT");

				fileBook.close();
			}
		}
	}

	return true;
}

bool CBuildDatabase::BuildFootnotesTables()
{
	// Build the Footnotes table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='FOOTNOTES'")) {
		QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"FOOTNOTES\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok);
		return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileBook(QString("../KJVCanOpener/db/data/FOOTNOTES.csv"));
			while (1) {
				if (!fileBook.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QObject::tr("Failed to open %1 for reading.").arg(fileBook.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Create the table in the database:
			strCmd = QString("create table FOOTNOTES "
							"(BkChpVrsWrdNdx INTEGER PRIMARY KEY, PFootnote TEXT, RFootnote TEXT)");

			if (!queryCreate.exec(strCmd)) {
				fileBook.close();
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QObject::tr("Failed to create table for FOOTNOTES\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
				// Read file and populate table:
				CCSVStream csv(&fileBook);

				QStringList slHeaders;
				csv >> slHeaders;              // Read Headers (verify and discard)

				if ((slHeaders.size()!=3) ||
					(slHeaders.at(0).compare("BkChpVrsWrdNdx") != 0) ||
					(slHeaders.at(1).compare("PFootnote") != 0) ||
					(slHeaders.at(2).compare("RFootnote") != 0)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for FOOTNOTES data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileBook.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");
				while (!csv.atEndOfStream()) {
					QStringList sl;
					csv >> sl;

					assert(sl.count() == 3);
					if (sl.count() < 3) continue;

					strCmd = QString("INSERT INTO FOOTNOTES "
									"(BkChpVrsWrdNdx, PFootnote, RFootnote) "
									"VALUES (:BkChpVrsWrdNdx, :PFootnote, :RFootnote)");

					// No need to have both plain text and rich text in our database, since
					//	we only need one data source for these.  So if we have Rich, we'll
					//	use that, but leave a placeholder for Plain.  Otherwise, we'll leave
					//	the placeholder for Rich and use Plain:
					sl[1] = sl[1].trimmed();
					sl[2] = sl[2].trimmed();
					if (!sl.at(2).isEmpty()) sl[1].clear();

					queryInsert.prepare(strCmd);
					queryInsert.bindValue(":BkChpVrsWrdNdx", sl.at(0).toUInt());
					queryInsert.bindValue(":PFootnote", sl.at(1));
					queryInsert.bindValue(":RFootnote", sl.at(2));

					if (!queryInsert.exec()) {
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for FOOTNOTES!\n%1\n%2\n%3\n%4").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(1)).arg(sl.at(2)),
												QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
					}
				}
				queryInsert.exec("COMMIT");

				fileBook.close();
			}
		}
	}

	return true;
}

bool CBuildDatabase::BuildPhrasesTable(bool bUserPhrases)
{
	// Build the Phrases table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='PHRASES'")) {
		QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Table Lookup for \"PHRASES\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok);
		return false;
	} else {
		CPhraseList phrases;

		queryCreate.next();
		if (queryCreate.value(0).toInt()) {
			// If we found it, drop it so we can recreate it:
			if (!queryCreate.exec("DROP TABLE PHRASES")) {
				QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Failed to drop old \"PHRASES\" table from database!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok);
				return false;
			}
		}
		if (!bUserPhrases) {
			// If this is the main phrases table, open our data file for populating it:
			QFile filePhrases(QString("../KJVCanOpener/db/data/PHRASES.csv"));
			while (1) {
				if (!filePhrases.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QObject::tr("Failed to open %1 for reading.").arg(filePhrases.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Read file and populate phrase list:
			CCSVStream csv(&filePhrases);

			QStringList slHeaders;
			csv >> slHeaders;              // Read Headers (verify and discard)

			if ((slHeaders.size()!=3) ||
				(slHeaders.at(0).compare("Ndx") != 0) ||
				(slHeaders.at(1).compare("Phrase") != 0) ||
				(slHeaders.at(2).compare("CaseSensitive") != 0)) {
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Unexpected Header Layout for PHRASES data file!"),
									QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
					filePhrases.close();
					return false;
				}
			}

			while (!csv.atEndOfStream()) {
				QStringList sl;
				csv >> sl;

				assert(sl.count() == 3);
				if (sl.count() < 3) continue;

				CPhraseEntry phrase;
				phrase.m_strPhrase = sl.at(1);
				phrase.m_bCaseSensitive = ((sl.at(2).toInt() != 0) ? true : false);
				if (!phrase.m_strPhrase.isEmpty()) {
					CParsedPhrase parsedPhrase(CBibleDatabasePtr(), false);			// Note: the ParsePhrase() function doesn't need the datbase.  If that ever changes, this must change (TODO)
					parsedPhrase.ParsePhrase(phrase.m_strPhrase);
					phrase.m_nNumWrd = parsedPhrase.phraseSize();

					phrases.push_back(phrase);
				}
			}

			filePhrases.close();
		}

		// Create the table in the database:
		strCmd = QString("create table PHRASES "
						"(Ndx INTEGER PRIMARY KEY, Phrase TEXT, CaseSensitive NUMERIC)");
		if (!queryCreate.exec(strCmd)) {
			QMessageBox::warning(m_pParent, g_constrBuildDatabase,
					QObject::tr("Failed to create table for PHRASES\n%1").arg(queryCreate.lastError().text()),
					QMessageBox::Ok);
			return false;
		}

		// Get the phrases and use the CPhraseListModel to sort it (since that will be displaying it later):
		if (bUserPhrases) {
			phrases.append(g_lstUserPhrases);
		}
		CPhraseListModel mdlPhrases(phrases);
		mdlPhrases.sort(0, Qt::AscendingOrder);
		phrases = mdlPhrases.phraseList();

		QSqlQuery queryInsert(m_myDatabase);
		queryInsert.exec("BEGIN TRANSACTION");
		for (int ndx=0; ndx<phrases.size(); ++ndx) {
			strCmd = QString("INSERT INTO PHRASES "
									"(Ndx, Phrase, CaseSensitive) "
									"VALUES (:Ndx, :Phrase, :CaseSensitive)");
			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":Ndx", ndx+1);
			queryInsert.bindValue(":Phrase", phrases.at(ndx).m_strPhrase);
			queryInsert.bindValue(":CaseSensitive", (phrases.at(ndx).m_bCaseSensitive ? 1 : 0));
			if (!queryInsert.exec()) {
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Insert Failed for PHRASES!\n%1\n  %2  (%3)")
																				.arg(queryInsert.lastError().text())
																				.arg(phrases.at(ndx).m_strPhrase)
																				.arg(phrases.at(ndx).m_bCaseSensitive ? QObject::tr("Case") : QObject::tr("NoCase")),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) break;
			}
		}
		queryInsert.exec("COMMIT");
	}

	return true;
}

bool CBuildDatabase::BuildDatabase(const QString &strDatabaseFilename)
{
	m_myDatabase = g_sqldbBuildMain;
	m_myDatabase.setDatabaseName(strDatabaseFilename);

//	QMessageBox::information(m_pParent, g_constrBuildDatabase, m_myDatabase.databaseName());

	if (!m_myDatabase.open()) {
		QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Error: Couldn't open database file \"%1\".").arg(strDatabaseFilename));
		return false;
	}

	bool bSuccess = true;

	if ((!BuildTestamentTable()) ||
		(!BuildBooksTable()) ||
		(!BuildChaptersTable()) ||
		(!BuildVerseTables()) ||
		(!BuildWordsTable()) ||
		(!BuildFootnotesTables()) ||
		(!BuildPhrasesTable(false))) bSuccess = false;

	m_myDatabase.close();

	if (bSuccess) QMessageBox::information(m_pParent, g_constrBuildDatabase, QObject::tr("Build Complete!"));
	return bSuccess;
}

bool CBuildDatabase::BuildUserDatabase(const QString &strDatabaseFilename, bool bHideWarnings)
{
	m_myDatabase = g_sqldbBuildUser;
	m_myDatabase.setDatabaseName(strDatabaseFilename);

	if (!m_myDatabase.open()) {
		if (!bHideWarnings)
			QMessageBox::warning(m_pParent, g_constrBuildDatabase, QObject::tr("Error: Couldn't open database file \"%1\".").arg(strDatabaseFilename));
		return false;
	}

	bool bSuccess = true;

	if (!BuildPhrasesTable(true)) {
		bSuccess = false;
	} else {
		g_bUserPhrasesDirty = false;
	}

	m_myDatabase.close();

	return bSuccess;
}

