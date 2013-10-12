/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "../KJVCanOpener/ParseSymbols.h"

#include <QCoreApplication>

#include <QFile>
#include <QString>
#include <QStringList>

#include <QtSql>
#include <QSqlQuery>

#include <assert.h>
#include <string>
#include <iostream>

namespace {
	const char *g_constrKJVDatabaseFilename = "../../KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrWeb1828DatabaseFilename = "../../KJVCanOpener/db/dct-web1828.s3db";
}

// ============================================================================

static QString deApostrHyphen(const QString &strWord);

static QString decompose(const QString &strWord)
{
	QString strDecomposed = strWord.normalized(QString::NormalizationForm_KD);

	strDecomposed.replace(QChar(0x00C6), "Ae");				// U+00C6	&#198;		AE character
	strDecomposed.replace(QChar(0x00E6), "ae");				// U+00E6	&#230;		ae character
	strDecomposed.replace(QChar(0x0132), "IJ");				// U+0132	&#306;		IJ character
	strDecomposed.replace(QChar(0x0133), "ij");				// U+0133	&#307;		ij character
	strDecomposed.replace(QChar(0x0152), "Oe");				// U+0152	&#338;		OE character
	strDecomposed.replace(QChar(0x0153), "oe");				// U+0153	&#339;		oe character

	strDecomposed = deApostrHyphen(strDecomposed);

	// There are two possible ways to remove accent marks:
	//
	//		1) strDecomposed.remove(QRegExp("[^a-zA-Z\\s]"));
	//
	//		2) Remove characters of class "Mark" (QChar::Mark_NonSpacing,
	//				QChar::Mark_SpacingCombining, QChar::Mark_Enclosing),
	//				which can be done by checking isMark()
	//

	for (int nPos = strDecomposed.size()-1; nPos >= 0; --nPos) {
		if (strDecomposed.at(nPos).isMark()) strDecomposed.remove(nPos, 1);
	}

	return strDecomposed;
}

static QString deApostrHyphen(const QString &strWord)
{
	static const QString strApostropheRegExp = QChar('[') + QRegExp::escape(g_strApostrophes) + QChar(']');
	static const QString strHyphenRegExp = QChar('[') + QRegExp::escape(g_strHyphens) + QChar(']');

	QString strDecomposed = strWord;

	strDecomposed.replace(QRegExp(strApostropheRegExp), "'");
	strDecomposed.replace(QRegExp(strHyphenRegExp), "-");

	return strDecomposed;
}

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QSqlDatabase myKJVDatabase;
	myKJVDatabase = QSqlDatabase::addDatabase("QSQLITE", "ReadKJVConnection");
	myKJVDatabase.setDatabaseName(g_constrKJVDatabaseFilename);
	myKJVDatabase.setConnectOptions("QSQLITE_OPEN_READONLY");

	QSqlDatabase myWeb1828Database;
	myWeb1828Database = QSqlDatabase::addDatabase("QSQLITE", "ReadWeb1828Connection");
	myWeb1828Database.setDatabaseName(g_constrWeb1828DatabaseFilename);
	myWeb1828Database.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!myKJVDatabase.open()) {
		std::cerr << QString("*** Failed to open Bible Database: %1\nError: %2\n\n").arg(g_constrKJVDatabaseFilename).arg(myKJVDatabase.lastError().text()).toUtf8().data();
		return -1;
	}

	if (!myWeb1828Database.open()) {
		std::cerr << QString("*** Failed to open Dictionary Database: %1\nError: %2\n\n").arg(g_constrWeb1828DatabaseFilename).arg(myWeb1828Database.lastError().text()).toUtf8().data();
		myKJVDatabase.close();
		return -2;
	}

	// ------------------------------------------------------------------------

	QStringList lstDictWords;

	QSqlQuery queryWeb1828Table(myWeb1828Database);

	// Check to see if the table exists:
	if (!queryWeb1828Table.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='dictionary'")) {
		std::cerr << QString("Table Lookup for \"dictioanry\" Failed!\nError: %1\n\n").arg(queryWeb1828Table.lastError().text()).toUtf8().data();
		return -3;
	}
	queryWeb1828Table.next();
	if (!queryWeb1828Table.value(0).toInt()) {
		std::cerr << QString("Unable to find \"dictionary\" Table in database!\n\n").toUtf8().data();
		return -4;
	}
	queryWeb1828Table.finish();

	QSqlQuery queryWeb1828Data(myWeb1828Database);
	queryWeb1828Data.setForwardOnly(true);
	queryWeb1828Data.exec("SELECT id,topic,definition FROM dictionary");
	while (queryWeb1828Data.next()) {
		lstDictWords.append(decompose(queryWeb1828Data.value(1).toString()).toLower());
	}
	queryWeb1828Data.finish();

	// ------------------------------------------------------------------------

	QStringList lstKJVWords;

	QSqlQuery queryKJVTable(myKJVDatabase);

	// Check to see if the table exists:
	if (!queryKJVTable.exec("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='WORDS'")) {
		std::cerr << QString("Table Lookup for \"WORDS\" Failed!\nError: %1\n\n").arg(queryKJVTable.lastError().text()).toUtf8().data();
		return -5;
	}
	queryKJVTable.next();
	if (!queryKJVTable.value(0).toInt()) {
		std::cerr << QString("Unable to find \"WORDS\" Table in database!\n\n").toUtf8().data();
		return -6;
	}
	queryKJVTable.finish();

	QSqlQuery queryKJVData(myKJVDatabase);
	queryKJVData.setForwardOnly(true);
	queryKJVData.exec("SELECT * FROM WORDS");
	while (queryKJVData.next()) {
		lstKJVWords.append(decompose(queryKJVData.value(1).toString()).toLower());
	}
	queryKJVData.finish();

	qSort(lstKJVWords);

	std::cout << "KJVWord,DictWord\n";
	for (int ndx = 0; ndx < lstKJVWords.size(); ++ndx) {
		QString strWord = lstKJVWords.at(ndx);
		if (lstDictWords.contains(strWord)) {
			std::cout << strWord.toUtf8().data() << "," << strWord.toUtf8().data() << "\n";
		} else {
			std::cout << strWord.toUtf8().data() << ",\n";
		}
	}

	myWeb1828Database.close();
	myKJVDatabase.close();
	return 0;
}
