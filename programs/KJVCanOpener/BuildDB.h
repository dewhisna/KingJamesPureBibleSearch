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
