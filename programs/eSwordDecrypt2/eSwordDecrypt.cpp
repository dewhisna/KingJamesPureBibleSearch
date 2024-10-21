/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
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

#include <QCoreApplication>

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

#include <QObject>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QByteArray>
#include <QtSql>
#include <QSqlQuery>

#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/BibleLayout.h"

#include "twofish_opt2.h"

// ============================================================================

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

// ============================================================================

namespace {
	const unsigned int VERSION = 10000;		// Version 1.0.0

	const QString g_constrDatabaseType = "QSQLITE";
	const QString g_constrMainReadConnection = "MainReadConnection";
	const QString g_constrMainWriteConnection = "MainWriteConnection";
}

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

	int nArgsFound = 0;
	bool bUnknownOption = false;
	bool bOverwrite = false;
	QString strCryptKey = "Heb_4:12";
	QString strSQLInFilename;
	QString strSQLOutFilename;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				strSQLInFilename = strArg;
			} else if (nArgsFound == 2) {
				strSQLOutFilename = strArg;
			}
		} else if (strArg.compare("-o") == 0) {
			bOverwrite = true;
		} else if (strArg.startsWith("-p")) {
			strCryptKey = strArg.mid(2);
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption) || (strSQLInFilename.isEmpty()) || (strSQLOutFilename.isEmpty())) {
		std::cerr << QString("eSwordDecrypt Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [Options] <eSword-SQL-in-file> <eSword-SQL-out-file>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("<eSword-SQL-in-file>  = Encrypted e-Sword .bblx SQL Database File\n").toUtf8().data();
		std::cerr << QString("<eSword-SQL-out-file> = Decrypted e-Sword .bblx SQL Database File\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Options\n").toUtf8().data();
		std::cerr << QString("    -o  =  Overwrite <eSword-SQL-out-file> if it exists\n").toUtf8().data();
		std::cerr << QString("    -p<password> = Password or Crypt-Key override\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();

		return -1;
	}

	CSQLitePlusDecryptor decryptor(strCryptKey.toStdString());
	QFile fileIn(strSQLInFilename);
	QFile fileOut(strSQLOutFilename);

	QSqlDatabase dbSource;
	dbSource = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrMainReadConnection);
	dbSource.setDatabaseName(fileIn.fileName());
	dbSource.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!dbSource.open()) {
		std::cerr << QString("Error: Couldn't open SQL database file \"%1\".\n\n%2\n").arg(fileIn.fileName()).arg(dbSource.lastError().text()).toUtf8().data();
		return -2;
	}

	if (fileOut.exists()) {
		if (!bOverwrite) {
			std::cerr << QString("Output database, \"%1\", already exists.  Delete it first or use '-o' option.\n").arg(fileOut.fileName()).toUtf8().data();
			return -3;
		} else {
			if (!fileOut.remove()) {
				std::cerr << QString("Unable to overwrite \"%1\"\n").arg(fileOut.fileName()).toUtf8().data();
				return -3;
			}
		}
	}

	QSqlDatabase dbDecrypt;
	dbDecrypt = QSqlDatabase::addDatabase(g_constrDatabaseType, g_constrMainWriteConnection);
	dbDecrypt.setDatabaseName(fileOut.fileName());
	if (!dbDecrypt.open()) {
		std::cerr << QString("Unable to open \"%1\" for writing\n").arg(fileOut.fileName()).toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	std::cout << QString("Reading: %1\n").arg(strSQLInFilename).toUtf8().data();
	std::cout << QString("Writing: %1\n").arg(strSQLOutFilename).toUtf8().data();
	std::cout.flush();

	// ------------------------------------------------------------------------

	QSqlQuery querySource(dbSource);
	QSqlQuery queryDecrypt(dbDecrypt);

	// Create tables in our decrypted database:
	if (!queryDecrypt.exec("CREATE TABLE Bible (Book INTEGER, Chapter INTEGER, Verse INTEGER, Scripture TEXT)")) {
		std::cerr << "*** Error Creating Bible Table in decrypted database.\n";
		return -4;
	}

	if (!queryDecrypt.exec("CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL)")) {
		std::cerr << "*** Error Creating Details Table in decrypted database.\n";
		return -4;
	}

	if (!queryDecrypt.exec("CREATE TABLE sqliteplus_license (Name TEXT UNIQUE)")) {
		std::cerr << "*** Error Creating sqliteplus_license Table in decrypted database.\n";
		return -4;
	}

	// ------------------------------------------------------------------------

	// Copy the sqliteplus_license Table:
	querySource.exec("SELECT Name from sqliteplus_license");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nCopying License";
	std::cout.flush();
	while (querySource.next()) {
		queryDecrypt.prepare("INSERT INTO sqliteplus_license "
							 "(Name) "
							 "VALUES (:Name)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			queryDecrypt.bindValue(nField, querySource.record().value(nField));
		}

		queryDecrypt.exec();
	}
	queryDecrypt.exec("COMMIT");
	std::cout << "\n";
	std::cout.flush();

	// ------------------------------------------------------------------------

	// Copy the Details Table:
	querySource.exec("SELECT Description, Abbreviation, Comments, Version, Font, RightToLeft, OT, NT, Apocrypha, Strong from Details");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nCopying Details";
	std::cout.flush();
	while (querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Details "
							 "(Description, Abbreviation, Comments, Version, Font, RightToLeft, OT, NT, Apocrypha, Strong) "
							 "VALUES (:Description, :Abbreviation, :Comments, :Version, :Font, :RightToLeft, :OT, :NT, :Apocrypha, :Strong)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			queryDecrypt.bindValue(nField, querySource.record().value(nField));
		}

		queryDecrypt.exec();
	}
	queryDecrypt.exec("COMMIT");
	std::cout << "\n";
	std::cout.flush();

	// ------------------------------------------------------------------------

	bool bGood = true;
	int nBk = 0;
	int nChp = 0;
	int nVrs = 0;

	// Copy the main Bible Table:
	querySource.exec("SELECT Book, Chapter, Verse, Scripture from Bible");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nDecrypting Bible";
	std::cout.flush();
	while (bGood && querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Bible "
							 "(Book, Chapter, Verse, Scripture) "
							 "VALUES (:Book, :Chapter, :Verse, :Scripture)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			if (nField == 3) {
				QByteArray baSource = querySource.record().value(nField).toByteArray();
				std::string strPlainText;
				std::stringstream ssSource(baSource.toStdString());
				int nResult = decryptor.decrypt(ssSource, strPlainText);
				if (nResult == 0) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else {
					std::cerr << "\n*** Decryption failed on Book " << nBk << " Chapter " << nChp << " Verse " << nVrs << " with Code " << nResult << std::endl;
					bGood = false;
				}
			} else {
				int nValue = querySource.record().value(nField).toInt();
				queryDecrypt.bindValue(nField, querySource.record().value(nField));

				if (nField == 0) {			// Book
					if (nBk != nValue) {
						nBk = nValue;
						nChp = 0;
						nVrs = 0;
						std::cout << "\nBook " << nBk;
						std::cout.flush();
					}
				} else if (nField == 1) {	// Chapter
					if (nChp != nValue) {
						nChp = nValue;
						nVrs = 0;
						std::cout << ".";
						std::cout.flush();
					}
				} else if (nField == 2) {	// Verse
					if (nVrs != nValue) {
						nVrs = nValue;
					}
				}
			}
		}

		queryDecrypt.exec();
	}
	queryDecrypt.exec("COMMIT");
	std::cout << "\n";
	std::cout.flush();

	// ------------------------------------------------------------------------

	std::cout << "\nCreating Indexes\n";
	if (!queryDecrypt.exec("CREATE INDEX BookChapterVerseIndex ON Bible (Book, Chapter, Verse)")) {
		std::cerr << "*** Error creating Indexes in decrypted database.\n";
		return -4;
	}

	// ------------------------------------------------------------------------

	if (bGood) std::cout << "\nDone\n";

	// ------------------------------------------------------------------------

	dbSource.close();
	dbSource = QSqlDatabase();

	dbDecrypt.close();
	dbDecrypt = QSqlDatabase();

	if (QSqlDatabase::contains(g_constrMainReadConnection)) QSqlDatabase::removeDatabase(g_constrMainReadConnection);
	if (QSqlDatabase::contains(g_constrMainWriteConnection)) QSqlDatabase::removeDatabase(g_constrMainWriteConnection);

	return bGood ? 0 : -5;
}

// ============================================================================

