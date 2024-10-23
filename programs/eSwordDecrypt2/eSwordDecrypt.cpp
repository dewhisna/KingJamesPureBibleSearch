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
#include <QFileInfo>
#include <QVariant>
#include <QByteArray>
#include <QtSql>
#include <QSqlQuery>

#include <assert.h>

#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/BibleLayout.h"

#include "twofish_opt2.h"
#include <zlib.h>

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

//
// Return -1 for error creating database structure
// Return -2 for error copying/creating data
//

// ============================================================================

int bblxDecrypt(CSQLitePlusDecryptor &decryptor, QSqlDatabase &dbSource, QSqlDatabase &dbDecrypt)
{
	QSqlQuery querySource(dbSource);
	QSqlQuery queryDecrypt(dbDecrypt);

	// Create tables in our decrypted database:
	if (!queryDecrypt.exec("CREATE TABLE Bible (Book INTEGER, Chapter INTEGER, Verse INTEGER, Scripture TEXT)")) {
		std::cerr << "*** Error Creating Bible Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL)")) {
		std::cerr << "*** Error Creating Details Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE sqliteplus_license (Name TEXT UNIQUE)")) {
		std::cerr << "*** Error Creating sqliteplus_license Table in decrypted database.\n";
		return -1;
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
				if (nResult == Z_OK) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else if (nResult == Z_STREAM_END) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
					std::cerr << "Short Record on Book " << nBk << " Chapter " << nChp << " Verse " << nVrs << std::endl;
					nBk = 0;
					nChp = 0;
					nVrs = 0;
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
		return -1;
	}

	// ------------------------------------------------------------------------

	if (bGood) std::cout << "\nDone\n";

	return bGood ? 0 : -2;
}

// ============================================================================

int dctxDecrypt(CSQLitePlusDecryptor &decryptor, QSqlDatabase &dbSource, QSqlDatabase &dbDecrypt)
{
	QSqlQuery querySource(dbSource);
	QSqlQuery queryDecrypt(dbDecrypt);

	// Create tables in our decrypted database:
	if (!queryDecrypt.exec("CREATE TABLE Dictionary (Topic NVARCHAR(100), Definition TEXT)")) {
		std::cerr << "*** Error Creating Dictionary Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE Details (Title NVARCHAR(255), Abbreviation NVARCHAR(50), Information TEXT, Version INT)")) {
		std::cerr << "*** Error Creating Details Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE sqliteplus_license (Name TEXT UNIQUE)")) {
		std::cerr << "*** Error Creating sqliteplus_license Table in decrypted database.\n";
		return -1;
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
	querySource.exec("SELECT Title, Abbreviation, Information, Version from Details");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nCopying Details";
	std::cout.flush();
	while (querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Details "
							 "(Title, Abbreviation, Information, Version) "
							 "VALUES (:Title, :Abbreviation, :Information, :Version)");
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
	std::string strTopic;

	// Copy the main Dictionary Table:
	querySource.exec("SELECT Topic, Definition from Dictionary");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nDecrypting Dictionary";
	std::cout.flush();
	while (bGood && querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Dictionary "
							 "(Topic, Definition) "
							 "VALUES (:Topic, :Definition)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			if (nField == 1) {
				QByteArray baSource = querySource.record().value(nField).toByteArray();
				std::string strPlainText;
				std::stringstream ssSource(baSource.toStdString());
				int nResult = decryptor.decrypt(ssSource, strPlainText);
				if (nResult == Z_OK) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else if (nResult == Z_STREAM_END) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
					std::cerr << "Short Record on Topic \"" << strTopic << "\"\n";
					strTopic.clear();
				} else {
					std::cerr << "\n*** Decryption failed on Topic \"" << strTopic << "\" with Code " << nResult << std::endl;
					bGood = false;
				}
			} else {
				std::string strNextTopic = querySource.record().value(nField).toString().toStdString();
				queryDecrypt.bindValue(nField, querySource.record().value(nField));

				if (strTopic.substr(0, 1).compare(strNextTopic.substr(0, 1)) != 0) {
					std::cout << ".";
					std::cout.flush();
				}
				strTopic = strNextTopic;
			}
		}

		queryDecrypt.exec();
	}
	queryDecrypt.exec("COMMIT");
	std::cout << "\n";
	std::cout.flush();

	// ------------------------------------------------------------------------

	std::cout << "\nCreating Indexes\n";
	if (!queryDecrypt.exec("CREATE INDEX TopicIndex ON Dictionary (Topic)")) {
		std::cerr << "*** Error creating Indexes in decrypted database.\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	if (bGood) std::cout << "\nDone\n";

	return bGood ? 0 : -2;
}

// ============================================================================

int cmtxDecrypt(CSQLitePlusDecryptor &decryptor, QSqlDatabase &dbSource, QSqlDatabase &dbDecrypt)
{
	QSqlQuery querySource(dbSource);
	QSqlQuery queryDecrypt(dbDecrypt);

	// Create tables in our decrypted database:
	if (!queryDecrypt.exec("CREATE TABLE BookCommentary (Book INT, Comments TEXT)")) {
		std::cerr << "*** Error Creating BookCommentary Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE ChapterCommentary (Book INT, Chapter INT, Comments TEXT)")) {
		std::cerr << "*** Error Creating ChapterCommentary Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE VerseCommentary (Book INT, ChapterBegin INT, VerseBegin INT, ChapterEnd INT, VerseEnd INT, Comments TEXT)")) {
		std::cerr << "*** Error Creating VerseCommentary Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE Details (Title NVARCHAR(255), Abbreviation NVARCHAR(50), Information TEXT, Version INT)")) {
		std::cerr << "*** Error Creating Details Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE sqliteplus_license (Name TEXT UNIQUE)")) {
		std::cerr << "*** Error Creating sqliteplus_license Table in decrypted database.\n";
		return -1;
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
	querySource.exec("SELECT Title, Abbreviation, Information, Version from Details");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nCopying Details";
	std::cout.flush();
	while (querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Details "
							 "(Title, Abbreviation, Information, Version) "
							 "VALUES (:Title, :Abbreviation, :Information, :Version)");
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

	// Copy the main BookCommentary Table:
	querySource.exec("SELECT Book, Comments from BookCommentary");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nDecrypting Book Commentary";
	std::cout.flush();
	while (bGood && querySource.next()) {
		queryDecrypt.prepare("INSERT INTO BookCommentary "
							 "(Book, Comments) "
							 "VALUES (:Book, :Comments)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			if (nField == 1) {
				QByteArray baSource = querySource.record().value(nField).toByteArray();
				std::string strPlainText;
				std::stringstream ssSource(baSource.toStdString());
				int nResult = decryptor.decrypt(ssSource, strPlainText);
				if (nResult == Z_OK) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else if (nResult == Z_STREAM_END) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
					std::cerr << "Short Record on Book " << nBk << std::endl;
					nBk = 0;
				} else {
					std::cerr << "\n*** Decryption failed on Book " << nBk << " with Code " << nResult << std::endl;
					bGood = false;
				}
			} else {
				int nValue = querySource.record().value(nField).toInt();
				queryDecrypt.bindValue(nField, querySource.record().value(nField));

				if (nBk != nValue) {
					nBk = nValue;
					std::cout << ".";
					std::cout.flush();
				}
			}
		}

		queryDecrypt.exec();
	}
	queryDecrypt.exec("COMMIT");
	std::cout << "\n";
	std::cout.flush();

	// ------------------------------------------------------------------------

	// Copy the main ChapterCommentary Table:
	querySource.exec("SELECT Book, Chapter, Comments from ChapterCommentary");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nDecrypting Chapter Commentary";
	std::cout.flush();
	while (bGood && querySource.next()) {
		queryDecrypt.prepare("INSERT INTO ChapterCommentary "
							 "(Book, Chapter, Comments) "
							 "VALUES (:Book, :Chapter, :Comments)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			if (nField == 2) {
				QByteArray baSource = querySource.record().value(nField).toByteArray();
				std::string strPlainText;
				std::stringstream ssSource(baSource.toStdString());
				int nResult = decryptor.decrypt(ssSource, strPlainText);
				if (nResult == Z_OK) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else if (nResult == Z_STREAM_END) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
					std::cerr << "Short Record on Book " << nBk << " Chapter " << nChp << std::endl;
					nBk = 0;
					nChp = 0;
				} else {
					std::cerr << "\n*** Decryption failed on Book " << nBk << " Chapter " << nChp << " with Code " << nResult << std::endl;
					bGood = false;
				}
			} else {
				int nValue = querySource.record().value(nField).toInt();
				queryDecrypt.bindValue(nField, querySource.record().value(nField));

				if (nField == 0) {			// Book
					if (nBk != nValue) {
						nBk = nValue;
						nChp = 0;
						std::cout << "\nBook " << nBk;
						std::cout.flush();
					}
				} else if (nField == 1) {	// Chapter
					if (nChp != nValue) {
						nChp = nValue;
						std::cout << ".";
						std::cout.flush();
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

	// Copy the main VerseCommentary Table:
	querySource.exec("SELECT Book, ChapterBegin, VerseBegin, ChapterEnd, VerseEnd, Comments from VerseCommentary");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nDecrypting Verse Commentary";
	std::cout.flush();
	while (bGood && querySource.next()) {
		queryDecrypt.prepare("INSERT INTO VerseCommentary "
							 "(Book, ChapterBegin, VerseBegin, ChapterEnd, VerseEnd, Comments) "
							 "VALUES (:Book, :ChapterBegin, :VerseBegin, :ChapterEnd, :VerseEnd, :Comments)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			if (nField == 5) {
				QByteArray baSource = querySource.record().value(nField).toByteArray();
				std::string strPlainText;
				std::stringstream ssSource(baSource.toStdString());
				int nResult = decryptor.decrypt(ssSource, strPlainText);
				if (nResult == Z_OK) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else if (nResult == Z_STREAM_END) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
					std::cerr << "Short Record on Book " << nBk << " Chapter " << nChp << " Verse " << nVrs << std::endl;
					nBk = 0;
					nChp = 0;
					nVrs = 0;
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
	if (!queryDecrypt.exec("CREATE INDEX BookChapterIndex ON ChapterCommentary (Book, Chapter)") ||
		!queryDecrypt.exec("CREATE INDEX BookChapterVerseIndex ON VerseCommentary (Book, ChapterBegin, VerseBegin)") ||
		!queryDecrypt.exec("CREATE INDEX BookIndex ON BookCommentary (Book)")) {
		std::cerr << "*** Error creating Indexes in decrypted database.\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	if (bGood) std::cout << "\nDone\n";

	return bGood ? 0 : -2;
}

// ============================================================================

int lexxDecrypt(CSQLitePlusDecryptor &decryptor, QSqlDatabase &dbSource, QSqlDatabase &dbDecrypt)
{
	QSqlQuery querySource(dbSource);
	QSqlQuery queryDecrypt(dbDecrypt);

	// Create tables in our decrypted database:
	if (!queryDecrypt.exec("CREATE TABLE Lexicon (Topic NVARCHAR(100), Definition TEXT)")) {
		std::cerr << "*** Error Creating Lexicon Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE Details (Title NVARCHAR(255), Abbreviation NVARCHAR(50), Information TEXT, Version INT)")) {
		std::cerr << "*** Error Creating Details Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE sqliteplus_license (Name TEXT UNIQUE)")) {
		std::cerr << "*** Error Creating sqliteplus_license Table in decrypted database.\n";
		return -1;
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
	querySource.exec("SELECT Title, Abbreviation, Information, Version from Details");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nCopying Details";
	std::cout.flush();
	while (querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Details "
							 "(Title, Abbreviation, Information, Version) "
							 "VALUES (:Title, :Abbreviation, :Information, :Version)");
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
	std::string strTopic;
	int nCount = 0;

	// Copy the main Lexicon Table:
	querySource.exec("SELECT Topic, Definition from Lexicon");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nDecrypting Lexicon";
	std::cout.flush();
	while (bGood && querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Lexicon "
							 "(Topic, Definition) "
							 "VALUES (:Topic, :Definition)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			if (nField == 1) {
				QByteArray baSource = querySource.record().value(nField).toByteArray();
				std::string strPlainText;
				std::stringstream ssSource(baSource.toStdString());
				int nResult = decryptor.decrypt(ssSource, strPlainText);
				if (nResult == Z_OK) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else if (nResult == Z_STREAM_END) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
					std::cerr << "Short Record on Topic \"" << strTopic << "\"\n";
					strTopic.clear();
				} else {
					std::cerr << "\n*** Decryption failed on Topic \"" << strTopic << "\" with Code " << nResult << std::endl;
					bGood = false;
				}
			} else {
				std::string strNextTopic = querySource.record().value(nField).toString().toStdString();
				queryDecrypt.bindValue(nField, querySource.record().value(nField));

				if (strTopic.substr(0, 1).compare(strNextTopic.substr(0, 1)) != 0) {
					nCount = 0;
					std::cout << "\n" << strNextTopic.substr(0, 1);
					std::cout.flush();
				} else {
					if ((nCount % 100) == 0) {
						std::cout << ".";
						std::cout.flush();
					}
					++nCount;
				}
				strTopic = strNextTopic;
			}
		}

		queryDecrypt.exec();
	}
	queryDecrypt.exec("COMMIT");
	std::cout << "\n";
	std::cout.flush();

	// ------------------------------------------------------------------------

	std::cout << "\nCreating Indexes\n";
	if (!queryDecrypt.exec("CREATE INDEX TopicIndex ON Lexicon (Topic)")) {
		std::cerr << "*** Error creating Indexes in decrypted database.\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	if (bGood) std::cout << "\nDone\n";

	return bGood ? 0 : -2;
}

// ============================================================================

int devxDecrypt(CSQLitePlusDecryptor &decryptor, QSqlDatabase &dbSource, QSqlDatabase &dbDecrypt)
{
	QSqlQuery querySource(dbSource);
	QSqlQuery queryDecrypt(dbDecrypt);

	// Create tables in our decrypted database:
	if (!queryDecrypt.exec("CREATE TABLE Devotional (Month INT, Day INT, Devotion TEXT)")) {
		std::cerr << "*** Error Creating Devotional Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE Details (Title NVARCHAR(255), Abbreviation NVARCHAR(50), Information TEXT, Version INT)")) {
		std::cerr << "*** Error Creating Details Table in decrypted database.\n";
		return -1;
	}

	if (!queryDecrypt.exec("CREATE TABLE sqliteplus_license (Name TEXT UNIQUE)")) {
		std::cerr << "*** Error Creating sqliteplus_license Table in decrypted database.\n";
		return -1;
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
	querySource.exec("SELECT Title, Abbreviation, Information, Version from Details");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nCopying Details";
	std::cout.flush();
	while (querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Details "
							 "(Title, Abbreviation, Information, Version) "
							 "VALUES (:Title, :Abbreviation, :Information, :Version)");
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
	int nMonth = 0;
	int nDay = 0;

	// Copy the main Devotional Table:
	querySource.exec("SELECT Month, Day, Devotion from Devotional");
	queryDecrypt.exec("BEGIN TRANSACTION");

	std::cout << "\nDecrypting Devotional";
	std::cout.flush();
	while (bGood && querySource.next()) {
		queryDecrypt.prepare("INSERT INTO Devotional "
							 "(Month, Day, Devotion) "
							 "VALUES (:Month, :Day, :Devotion)");
		for (int nField = 0; nField < querySource.record().count(); ++nField) {
			if (nField == 2) {
				QByteArray baSource = querySource.record().value(nField).toByteArray();
				std::string strPlainText;
				std::stringstream ssSource(baSource.toStdString());
				int nResult = decryptor.decrypt(ssSource, strPlainText);
				if (nResult == Z_OK) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
				} else if (nResult == Z_STREAM_END) {
					queryDecrypt.bindValue(nField, QString::fromStdString(strPlainText));
					std::cerr << "Short Record on Month " << nMonth << " Day " << nDay << std::endl;
					nMonth = 0;
					nDay = 0;
				} else {
					std::cerr << "\n*** Decryption failed on Month " << nMonth << " Day " << nDay << " with Code " << nResult << std::endl;
					bGood = false;
				}
			} else {
				int nValue = querySource.record().value(nField).toInt();
				queryDecrypt.bindValue(nField, querySource.record().value(nField));

				if (nField == 0) {			// Month
					if (nMonth != nValue) {
						nMonth = nValue;
						nDay = 0;
						std::cout << "\nMonth " << nMonth;
						std::cout.flush();
					}
				} else if (nField == 1) {	// Day
					if (nDay != nValue) {
						nDay = nValue;
						std::cout << ".";
						std::cout.flush();
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
	if (!queryDecrypt.exec("CREATE INDEX MonthDayIndex ON Devotional (Month, Day)")) {
		std::cerr << "*** Error creating Indexes in decrypted database.\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	if (bGood) std::cout << "\nDone\n";

	return bGood ? 0 : -2;
}

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

	enum DB_TYPE_ENUM {
		DTE_Unknown = 0,			// Unknown or not yet defined
		DTE_Bible = 1,				// Bible Database (bblx)
		DTE_Dictionary = 2,			// Dictionary Database (dctx)
		DTE_Commentary = 3,			// Commentary Database (cmtx)
		DTE_Lexicon = 4,			// Lexicon Database (lexx)
		DTE_Devotional = 5,			// Devotional Database (devx)
	};

	int nArgsFound = 0;
	bool bUnknownOption = false;
	bool bOverwrite = false;
	QString strCryptKey = "Heb_4:12";
	QString strSQLInFilename;
	QString strSQLOutFilename;
	DB_TYPE_ENUM nDBType = DTE_Unknown;

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
		} else if (strArg.startsWith("-db")) {
			nDBType = DTE_Bible;
		} else if (strArg.startsWith("-dd")) {
			nDBType = DTE_Dictionary;
		} else if (strArg.startsWith("-dc")) {
			nDBType = DTE_Commentary;
		} else if (strArg.startsWith("-dl")) {
			nDBType = DTE_Lexicon;
		} else if (strArg.startsWith("-dv")) {
			nDBType = DTE_Devotional;
		} else {
			bUnknownOption = true;
		}
	}

	if (nDBType == DTE_Unknown) {
		QFileInfo fiFileIn(strSQLInFilename);
		if (fiFileIn.suffix().compare("bblx") == 0) {
			nDBType = DTE_Bible;
		} else if (fiFileIn.suffix().compare("dctx") == 0) {
			nDBType = DTE_Dictionary;
		} else if (fiFileIn.suffix().compare("cmtx") == 0) {
			nDBType = DTE_Commentary;
		} else if (fiFileIn.suffix().compare("lexx") == 0) {
			nDBType = DTE_Lexicon;
		} else if (fiFileIn.suffix().compare("devx") == 0) {
			nDBType = DTE_Devotional;
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
		std::cerr << QString("    -db =  Bible Database (bblx file)\n").toUtf8().data();
		std::cerr << QString("    -dd =  Dictionary Database (dctx file)\n").toUtf8().data();
		std::cerr << QString("    -dc =  Commentary Database (cmtx file)\n").toUtf8().data();
		std::cerr << QString("    -dl =  Lexicon Database (lexx file)\n").toUtf8().data();
		std::cerr << QString("    -dv =  Devotional Database (devx file)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("  Note: If the database type isn't specified, it will be inferred from the\n").toUtf8().data();
		std::cerr << QString("     <eSword-SQL-in-file> extension if possible.\n").toUtf8().data();
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

	int nRetVal = 0;
	switch (nDBType) {
		case DTE_Bible:
			nRetVal = bblxDecrypt(decryptor, dbSource, dbDecrypt);
			break;
		case DTE_Dictionary:
			nRetVal = dctxDecrypt(decryptor, dbSource, dbDecrypt);
			break;
		case DTE_Commentary:
			nRetVal = cmtxDecrypt(decryptor, dbSource, dbDecrypt);
			break;
		case DTE_Lexicon:
			nRetVal = lexxDecrypt(decryptor, dbSource, dbDecrypt);
			break;
		case DTE_Devotional:
			nRetVal = devxDecrypt(decryptor, dbSource, dbDecrypt);
			break;
		case DTE_Unknown:
			assert(false);
			std::cerr << "*** Unknown/Unspecified database type\n";
			nRetVal = -1;
			break;
	}

	// ------------------------------------------------------------------------

	dbSource.close();
	dbSource = QSqlDatabase();

	dbDecrypt.close();
	dbDecrypt = QSqlDatabase();

	if (QSqlDatabase::contains(g_constrMainReadConnection)) QSqlDatabase::removeDatabase(g_constrMainReadConnection);
	if (QSqlDatabase::contains(g_constrMainWriteConnection)) QSqlDatabase::removeDatabase(g_constrMainWriteConnection);

	return nRetVal ? (nRetVal-3) : 0;		// -1 -> -4, -2 -> -5
}

// ============================================================================

