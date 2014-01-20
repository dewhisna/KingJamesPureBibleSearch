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

#ifndef BUILDDB_H
#define BUILDDB_H

#ifndef NOT_USING_SQL
#include <QSqlDatabase>
#endif

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QScopedPointer>
#include "dbstruct.h"

// ============================================================================

// Forward Declarations:
class CCSVStream;

// ============================================================================

class CBuildDatabase
{
public:
	CBuildDatabase(QWidget *pParent = NULL);
	~CBuildDatabase();

	bool BuildDatabase(const QString &strSQLDatabaseFilename, const QString &strCCDatabaseFilename);
	bool BuildUserDatabase(const QString &strDatabaseFilename, bool bHideWarnings = false);

protected:
	bool BuildDBInfoTable();
	bool BuildTestamentTable();
	bool BuildBooksTable();
	bool BuildChaptersTable();
	bool BuildVerseTables();
	bool BuildWordsTable();
	bool BuildFootnotesTables();
	bool BuildPhrasesTable(bool bUserPhrases);

private:
	QWidget *m_pParent;
#ifndef NOT_USING_SQL
	QSqlDatabase m_myDatabase;
#endif
	QScopedPointer<CCSVStream> m_pCCDatabase;				// KJPBS Format Database .ccdb (compress-csv database)
};

// ============================================================================

#endif // BUILDDB_H
