// BuildDB.cpp -- Code to build the initial database from csv data files
//

#include "BuildDB.h"
#include "dbstruct.h"
#include "CSV.h"

#include <assert.h>

#include <QFile>

#include <QtSql>
#include <QSqlQuery>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QByteArray>

namespace {
	const char *g_constrBuildDatabase = "Building Database";

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

bool CBuildDatabase::BuildTestamentTable()
{
	// Build the TESTAMENT table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TESTAMENT'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Table Lookup for \"TESTAMENT\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {

			// Create the table in the database:
			strCmd = QString("create table TESTAMENT "
							"(TstNdx INTEGER PRIMARY KEY, TstName TEXT)");

			if (!queryCreate.exec(strCmd)) {
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QString("Failed to create table for TESTAMENT\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
				// Populate table:
				QSqlQuery queryInsert(m_myDatabase);
				bool bSuccess = true;
				bSuccess = queryInsert.exec("BEGIN TRANSACTION");
				strCmd = QString("INSERT INTO TESTAMENT "
									"(TstNdx, TstName) "
									"VALUES (:TstNdx, :TstName)");

				if (bSuccess) {
					queryInsert.prepare(strCmd);
					queryInsert.bindValue(":TstNdx", 1);
					queryInsert.bindValue(":TstName", QString("Old Testament"));
					bSuccess = queryInsert.exec();
				}

				if (bSuccess) {
					queryInsert.prepare(strCmd);
					queryInsert.bindValue(":TstNdx", 2);
					queryInsert.bindValue(":TstName", QString("New Testament"));
					bSuccess = queryInsert.exec();
				}

				bSuccess = bSuccess && queryInsert.exec("COMMIT");

				if (!bSuccess) {
					QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Insert Failed for TESTAMENT!\n%1").arg(queryInsert.lastError().text()));
					return false;
				}
			}
		}
	}

	return true;
}

bool CBuildDatabase::BuildTOCTable()
{
	// Build the TOC table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TOC'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Table Lookup for \"TOC\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileBook(QString("../KJVCanOpener/db/data/TOC.csv"));
			while (1) {
				if (!fileBook.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QString("Failed to open %1 for reading.").arg(fileBook.fileName()),
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
						QString("Failed to create table for TOC\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
				// Read file and populate table:
				CSVstream csv(&fileBook);

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
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Unexpected Header Layout for TOC data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileBook.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");
				while (!csv.atEnd()) {
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
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Insert Failed for TOC!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(3)),
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

bool CBuildDatabase::BuildLAYOUTTable()
{
	// Build the LAYOUT table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='LAYOUT'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Table Lookup for \"LAYOUT\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileBook(QString("../KJVCanOpener/db/data/LAYOUT.csv"));
			while (1) {
				if (!fileBook.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QString("Failed to open %1 for reading.").arg(fileBook.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Create the table in the database:
			strCmd = QString("create table LAYOUT "
							"(BkChpNdx INTEGER PRIMARY KEY, NumVrs NUMERIC, NumWrd NUMERIC, BkAbbr TEXT, ChNdx NUMERIC)");

			if (!queryCreate.exec(strCmd)) {
				fileBook.close();
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QString("Failed to create table for LAYOUT\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
				// Read file and populate table:
				CSVstream csv(&fileBook);

				QStringList slHeaders;
				csv >> slHeaders;              // Read Headers (verify and discard)

				if ((slHeaders.size()!=5) ||
					(slHeaders.at(0).compare("BkChpNdx") != 0) ||
					(slHeaders.at(1).compare("NumVrs") != 0) ||
					(slHeaders.at(2).compare("NumWrd") != 0) ||
					(slHeaders.at(3).compare("BkAbbr") != 0) ||
					(slHeaders.at(4).compare("ChNdx") != 0)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Unexpected Header Layout for LAYOUT data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileBook.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");
				while (!csv.atEnd()) {
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
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Insert Failed for LAYOUT!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(3)).arg(sl.at(4)),
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

bool CBuildDatabase::BuildBookTables()
{
	QString strCmd;

	// Build the BOOK tables:
	for (int i=0; i<NUM_BK; ++i) {
		QSqlQuery queryCreate(m_myDatabase);

		// Check to see if the table exists already:
		queryCreate.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=:table_name");
		queryCreate.bindValue(":table_name", g_arrstrBkTblNames[i]);
		if (!queryCreate.exec()) {
			if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Table Lookup for \"%1\" Failed!\n%2").arg(g_arrstrBkTblNames[i]).arg(queryCreate.lastError().text()),
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
						QString("Failed to open %1 for reading.").arg(fileBook.fileName()),
						QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else break;
		}

		// Create the table in the database:
		strCmd = QString("create table %1 "
						"(ChpVrsNdx INTEGER PRIMARY KEY, NumWrd NUMERIC, bPilcrow NUMERIC, PText TEXT, RText TEXT, Footnote TEXT)").arg(g_arrstrBkTblNames[i]);

		if (!queryCreate.exec(strCmd)) {
			fileBook.close();
			if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
					QString("Failed to create table for %1\n%2").arg(g_arrstrBkTblNames[i]).arg(queryCreate.lastError().text()),
					QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			continue;
		}

		// Read file and populate table:
		CSVstream csv(&fileBook);

		QStringList slHeaders;
		csv >> slHeaders;              // Read Headers (verify and discard)

		if ((slHeaders.size()!=6) ||
			(slHeaders.at(0).compare("ChpVrsNdx") != 0) ||
			(slHeaders.at(1).compare("NumWrd") != 0) ||
			(slHeaders.at(2).compare("bPilcrow") != 0) ||
			(slHeaders.at(3).compare("PText") != 0) ||
			(slHeaders.at(4).compare("RText") != 0) ||
			(slHeaders.at(5).compare("Footnote") != 0)) {
			if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Unexpected Header Layout for %1 data file!").arg(g_arrstrBkTblNames[i]),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
				fileBook.close();
				return false;
			}
			fileBook.close();
			continue;
		}

		QSqlQuery queryInsert(m_myDatabase);
		queryInsert.exec("BEGIN TRANSACTION");
		while (!csv.atEnd()) {
			QStringList sl;
			csv >> sl;

			assert(sl.count() == 6);
			if (sl.count() < 6) continue;

			strCmd = QString("INSERT INTO %1 "
						"(ChpVrsNdx, NumWrd, bPilcrow, PText, RText, Footnote) "
						"VALUES (:ChpVrsNdx, :NumWrd, :bPilcrow, :PText, :RText, :Footnote)").arg(g_arrstrBkTblNames[i]);

			queryInsert.prepare(strCmd);
			queryInsert.bindValue(":ChpVrsNdx", sl.at(0).toUInt());
			queryInsert.bindValue(":NumWrd", sl.at(1).toUInt());
			queryInsert.bindValue(":bPilcrow", sl.at(2).toInt());
			queryInsert.bindValue(":PText", sl.at(3));
			queryInsert.bindValue(":RText", sl.at(4));
			queryInsert.bindValue(":Footnote", sl.at(5));
			if (!queryInsert.exec()) {
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Insert Failed!\n%1\n  %2  %3").arg(queryInsert.lastError().text()).arg(sl.at(0)).arg(sl.at(3)),
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
	QString strBuff = str + '\n';
	CSVstream csv(&strBuff, QIODevice::ReadOnly);
	QStringList slValues;
	csv >> slValues;

	// Special case handle empty lists or else CSVstream will process it as
	//  a list with one empty member:
	if ((slValues.size() == 1) && (slValues.at(0).isEmpty())) return QByteArray();

	QByteArray baBlob(slValues.size()*sizeof(uint32_t), 0);
	uint32_t nValue;
	for (int i=0; i<slValues.size(); ++i) {
		nValue = slValues.at(i).toUInt();
		assert(nValue != 0);
		for (unsigned int j=1; j<=sizeof(uint32_t); ++j) {
			baBlob[(i*sizeof(uint32_t))+(sizeof(uint32_t)-j)] = (nValue & 0xFF);
			nValue = nValue >> 8;
		}
	}
	assert((baBlob.size() % sizeof(uint32_t)) == 0);
	assert(baBlob.size() == (slValues.size() * sizeof(uint32_t)));

	return baBlob;
}

bool CBuildDatabase::BuildWORDSTable()
{
	// Build the WORDS table:

	QString strCmd;
	QSqlQuery queryCreate(m_myDatabase);

	// Check to see if the table exists already:
	if (!queryCreate.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='WORDS'")) {
		if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Table Lookup for \"WORDS\" Failed!\n%1").arg(queryCreate.lastError().text()),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
	} else {
		queryCreate.next();
		if (!queryCreate.value(0).toInt()) {
			// Open the table data file:
			QFile fileBook(QString("../KJVCanOpener/db/data/WORDS.csv"));
			while (1) {
				if (!fileBook.open(QIODevice::ReadOnly)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
							QString("Failed to open %1 for reading.").arg(fileBook.fileName()),
							QMessageBox::Retry, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
				} else break;
			}

			// Create the table in the database:
			strCmd = QString("create table WORDS "
							"(WrdNdx INTEGER PRIMARY KEY, Word TEXT, bIndexCasePreserve NUMERIC, NumTotal NUMERIC, AltWords TEXT, AltWordCounts TEXT, Mapping BLOB, NormalMap BLOB)");

			if (!queryCreate.exec(strCmd)) {
				fileBook.close();
				if (QMessageBox::warning(m_pParent, g_constrBuildDatabase,
						QString("Failed to create table for WORDS\n%1").arg(queryCreate.lastError().text()),
						QMessageBox::Ignore, QMessageBox::Cancel) == QMessageBox::Cancel) return false;
			} else {
				// Read file and populate table:
				CSVstream csv(&fileBook);

				QStringList slHeaders;
				csv >> slHeaders;              // Read Headers (verify and discard)

				if ((slHeaders.size()!=8) ||
					(slHeaders.at(0).compare("WrdNdx") != 0) ||
					(slHeaders.at(1).compare("Word") != 0) ||
					(slHeaders.at(2).compare("bIndexCasePreserve") != 0) ||
					(slHeaders.at(3).compare("NumTotal") != 0) ||
					(slHeaders.at(4).compare("AltWords") != 0) ||
					(slHeaders.at(5).compare("AltWordCounts") != 0) ||
					(slHeaders.at(6).compare("Mapping") != 0) ||
					(slHeaders.at(7).compare("NormalMap") != 0)) {
					if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Unexpected Header Layout for WORDS data file!"),
										QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
						fileBook.close();
						return false;
					}
				}

				QSqlQuery queryInsert(m_myDatabase);
				queryInsert.exec("BEGIN TRANSACTION");
				while (!csv.atEnd()) {
					QStringList sl;
					csv >> sl;

					assert(sl.count() == 8);
					if (sl.count() < 8) continue;

					strCmd = QString("INSERT INTO WORDS "
									"(WrdNdx, Word, bIndexCasePreserve, NumTotal, AltWords, AltWordCounts, Mapping, NormalMap) "
									"VALUES (:WrdNdx, :Word, :bIndexCasePreserve, :NumTotal, :AltWords, :AltWordCounts, :Mapping, :NormalMap)");
					queryInsert.prepare(strCmd);
					queryInsert.bindValue(":WrdNdx", sl.at(0).toUInt());
					queryInsert.bindValue(":Word", sl.at(1));
					queryInsert.bindValue(":bIndexCasePreserve", sl.at(2).toInt());
					queryInsert.bindValue(":NumTotal", sl.at(3).toUInt());
					queryInsert.bindValue(":AltWords", sl.at(4));
					queryInsert.bindValue(":AltWordCounts", sl.at(5));
					queryInsert.bindValue(":Mapping", CSVStringToIndexBlob(sl.at(6)), QSql::In | QSql::Binary);
					queryInsert.bindValue(":NormalMap", CSVStringToIndexBlob(sl.at(7)), QSql::In | QSql::Binary);

					if (!queryInsert.exec()) {
						if (QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Insert Failed for WORDS!\n%1\n  %2  (%3)").arg(queryInsert.lastError().text()).arg(sl.at(1)).arg(sl.at(4)),
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

bool CBuildDatabase::BuildDatabase(const char *pstrDatabaseFilename)
{
	m_myDatabase = QSqlDatabase::addDatabase("QSQLITE");
	m_myDatabase.setDatabaseName(pstrDatabaseFilename);

//	QMessageBox::information(m_pParent, g_constrBuildDatabase, m_myDatabase.databaseName());

	if (!m_myDatabase.open()) {
		QMessageBox::warning(m_pParent, g_constrBuildDatabase, QString("Error: Couldn't open database file \"%1\".").arg(m_myDatabase.databaseName()));
		return false;
	}

	bool bSuccess = true;

	if ((!BuildTestamentTable()) ||
		(!BuildTOCTable()) ||
		(!BuildLAYOUTTable()) ||
		(!BuildBookTables()) ||
		(!BuildWORDSTable())) bSuccess = false;

	m_myDatabase.close();

	if (bSuccess) QMessageBox::information(m_pParent, g_constrBuildDatabase, "Build Complete!");
	return bSuccess;
}

