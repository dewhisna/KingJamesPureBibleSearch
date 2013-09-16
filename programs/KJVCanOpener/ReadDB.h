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

#ifndef READDB_H
#define READDB_H

#include <QSqlDatabase>
#include <QWidget>
#include <QByteArray>
#include <QString>
#include "dbstruct.h"

// ============================================================================

class CReadDatabase
{
public:
	CReadDatabase(QWidget *pParent = NULL);
	~CReadDatabase();

	bool ReadBibleDatabase(const QString &strDatabaseFilename, const QString &strName, const QString &strDescription, const QString &strCompatUUID, bool bSetAsMain = false);
	bool ReadUserDatabase(const QString &strDatabaseFilename, bool bHideWarnings = false);
	bool ReadDictionaryDatabase(const QString &strDatabaseFilename, const QString &strName, const QString &strDescription, const QString &strCompatUUID, bool bLiveDB = true, bool bSetAsMain = false);

	static bool IndexBlobToIndexList(const QByteArray &baBlob, TNormalizedIndexList &anIndexList);

	// ------------------------------------------------------------------------

	static QString dictionaryDefinition(const CDictionaryDatabase *pDictionaryDatabase, const CDictionaryWordEntry &wordEntry);

	// ------------------------------------------------------------------------

protected:
	bool ReadTestamentTable();
	bool ReadBooksTable();
	bool ReadChaptersTable();
	bool ReadVerseTables();
	bool ReadWordsTable();
	bool ReadFOOTNOTESTable();
	bool ReadPHRASESTable(bool bUserPhrases = false);
	bool ValidateData();

	// ------------------------------------------------------------------------

	bool ReadDictionaryDBInfo();
	bool ReadDictionaryWords(bool bLiveDB);

	// ------------------------------------------------------------------------

private:
	QWidget *m_pParent;
	QSqlDatabase m_myDatabase;
	CBibleDatabasePtr m_pBibleDatabase;				// Pointer to the Bible Database currently being read -- created in ReadBibleDatabase, used by reader functions
	CDictionaryDatabasePtr m_pDictionaryDatabase;	// Pointer to the Dictionary Database currently being read -- created in ReadDictionaryDatabase, used by reader functions
};

// ============================================================================

#endif // READDB_H
