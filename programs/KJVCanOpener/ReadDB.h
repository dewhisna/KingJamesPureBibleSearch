/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef NOT_USING_SQL
#include <QSqlDatabase>
#endif

class QWidget;		// Include QWidget only as forward declaration for non-gui apps

#include <QByteArray>
#include <QString>
#include <QScopedPointer>
#include "dbstruct.h"
#include "dbDescriptors.h"

// ============================================================================

// Forward Declarations:
class CCSVStream;

// ============================================================================

class CReadDatabase
{
public:
	CReadDatabase(QWidget *pParent = nullptr);
	~CReadDatabase();

	bool haveBibleDatabaseFiles(const TBibleDescriptor &bblDesc) const;
	bool haveDictionaryDatabaseFiles(const TDictionaryDescriptor &dctDesc) const;
	bool ReadBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain = false);
	bool ReadSpecialBibleDatabase(const QString &strCCDBPathFilename, bool bSetAsMain = false);		// If an absolute path is given, it's used, else the path is considered relative to m_strBibleDatabasePath
	bool ReadDictionaryDatabase(const TDictionaryDescriptor &dctDesc, bool bLiveDB = true, bool bSetAsMain = false);

	TBibleDescriptor discoverCCDBBibleDatabase(const QString &strFilePathName);
	TBibleDescriptor discoverS3DBBibleDatabase(const QString &strFilePathName);

	TDictionaryDescriptor discoverCCDBDictionaryDatabase(const QString &strFilePathName);
	TDictionaryDescriptor discoverS3DBDictionaryDatabase(const QString &strFilePathName);

	// ------------------------------------------------------------------------

	static QString dictionaryDefinition(const CDictionaryDatabase *pDictionaryDatabase, const CDictionaryWordEntry &wordEntry);

	// ------------------------------------------------------------------------

protected:
	bool ReadDBInfoTable();
	bool ReadTestamentTable();
	bool ReadBooksTable();
	bool ReadChaptersTable();
	bool ReadVerseTables();
	bool ReadWordsTable();
	bool ReadFOOTNOTESTable();
	bool ReadPHRASESTable();
	bool ReadLEMMASTable();
	bool ReadSTRONGSTable();
	bool ValidateData();

	// ------------------------------------------------------------------------

	bool ReadDictionaryDBInfo();
	bool ReadDictionaryWords(bool bLiveDB);

	// ------------------------------------------------------------------------

private:
	bool readBibleStub();
	bool readDictionaryStub(bool bLiveDB);

	bool readCCDBBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain = false);
	bool readS3DBBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain = false);

private:
	QWidget *m_pParent;

#ifndef NOT_USING_SQL
	QSqlDatabase m_myDatabase;
#endif
	QScopedPointer<CCSVStream> m_pCCDatabase;		// KJPBS Format Database .ccdb (compress-csv database)

	CBibleDatabasePtr m_pBibleDatabase;				// Pointer to the Bible Database currently being read -- created in ReadBibleDatabase, used by reader functions
	CDictionaryDatabasePtr m_pDictionaryDatabase;	// Pointer to the Dictionary Database currently being read -- created in ReadDictionaryDatabase, used by reader functions
};

// ============================================================================

#endif // READDB_H
