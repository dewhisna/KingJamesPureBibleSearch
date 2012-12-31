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

#include <QSqlDatabase>
#include <QWidget>
#include <QByteArray>
#include <QString>
#include "dbstruct.h"

class CBuildDatabase
{
public:
	CBuildDatabase(QWidget *pParent = NULL)
		:	m_pParent(pParent)
	{ }
	~CBuildDatabase() { }

	bool BuildDatabase(const QString &strDatabaseFilename);
	bool BuildUserDatabase(const QString &strDatabaseFilename);

	static QByteArray CSVStringToIndexBlob(const QString &str);

protected:
	bool BuildTestamentTable();
	bool BuildTOCTable();
	bool BuildLAYOUTTable();
	bool BuildBookTables();
	bool BuildWORDSTable();
	bool BuildPHRASESTable(bool bUserPhrases);

private:
	QWidget *m_pParent;
	QSqlDatabase m_myDatabase;
};

#endif // BUILDDB_H
