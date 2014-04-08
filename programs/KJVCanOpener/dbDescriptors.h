/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef DB_DESCRIPTORS_H
#define DB_DESCRIPTORS_H

#include <QMetaType>
#include <QString>
#include <QStringList>

// ============================================================================

typedef struct {
	bool m_bAutoLoad;					// If true, the database will be autoloaded at application startup
	QString m_strLanguage;
	QString m_strDBName;
	QString m_strDBDesc;
	QString m_strUUID;
	QString m_strDBInfoFilename;		// Database Information filename (used during build process)
	QString m_strS3DBFilename;			// Sqlite3 Database filename
	QString m_strCCDBFilename;			// Compressed-CSV Database filename
	QStringList m_lstCompatUUID;		// List of UUIDs for databases with compatible versification
} TBibleDescriptor;

// Must match constBibleDescriptors[]!!  This is also the order we attempt to read/load them in:
enum BIBLE_DESCRIPTOR_ENUM {
	BDE_UNKNOWN = -1,
	BDE_SPECIAL_TEST = 0,
	BDE_KJV = 1,
	BDE_RVG2010 = 2,
	BDE_KJF2006 = 3,
	BDE_KJVPCE = 4,
	BDE_KJVA = 5
};
Q_DECLARE_METATYPE(BIBLE_DESCRIPTOR_ENUM)

typedef struct {
	bool m_bAutoLoad;					// If true, the database will be autoloaded at application startup
	QString m_strLanguage;
	QString m_strDBName;
	QString m_strDBDesc;
	QString m_strUUID;
	QString m_strDBInfoFilename;		// Database Information filename (used during build process)
	QString m_strS3DBFilename;			// Sqlite3 Database filename
	QString m_strCCDBFilename;			// Compressed-CSV Database filename
} TDictionaryDescriptor;

// Must match coonstDictionaryDescriptors[]!!  This is also the order we attempt to read/load them in:
enum DICTIONARY_DESCRIPTOR_ENUM {
	DDE_UNKNOWN = -1,
	DDE_SPECIAL_TEST = 0,
	DDE_WEB1828 = 1
};
Q_DECLARE_METATYPE(DICTIONARY_DESCRIPTOR_ENUM)

extern QString g_strBibleDatabasePath;
extern QString g_strDictionaryDatabasePath;

extern unsigned int bibleDescriptorCount();
extern const TBibleDescriptor &bibleDescriptor(BIBLE_DESCRIPTOR_ENUM nIndex);
extern BIBLE_DESCRIPTOR_ENUM bibleDescriptorFromUUID(const QString &strUUID);
extern unsigned int dictionaryDescriptorCount();
extern const TDictionaryDescriptor &dictionaryDescriptor(DICTIONARY_DESCRIPTOR_ENUM nIndex);
extern DICTIONARY_DESCRIPTOR_ENUM dictionaryDescriptorFromUUID(const QString &strUUID);

// ============================================================================

#endif	// DB_DESCRIPTORS_H
