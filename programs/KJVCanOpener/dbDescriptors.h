/****************************************************************************
**
** Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef DATABASE_DESCRIPTORS_H
#define DATABASE_DESCRIPTORS_H

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QFlags>

// ============================================================================

enum LANGUAGE_ID_ENUM {
	LIDE_UNKNOWN = 0,					// Unknown Language
	LIDE_ENGLISH = 1,					// English, "en", or "eng"
	LIDE_FRENCH = 2,					// French, "fr", "fre", or "fra"
	LIDE_SPANISH = 3,					// Spanish, "es", or "spa"
	LIDE_GERMAN = 4,					// German, "de", "deu", or "ger"
	LIDE_RUSSIAN = 5,					// Russian, "ru", or "rus"
	LIDE_HEBREW = 6,					// Hebrew, "he", "heb", "hbo" or "iw"
	LIDE_GREEK = 7,						// Greek, "el", "ell", "grc", or "gre"
};

LANGUAGE_ID_ENUM toLanguageID(const QString &strLang);		// Return Bible Language ID from database language string
QString toQtLanguageName(LANGUAGE_ID_ENUM nID);				// Returns Qt Language name for Bible Language ID

// ============================================================================

enum BibleTypeOptions {
	BTO_None = 0x0,						// Default for no options
	BTO_SpecialTest = 0x1,				// Is Special Test Database entry
	BTO_AutoLoad = 0x2,					// AutoLoad database at app start
	BTO_Discovered = 0x4,				// Discovered via disk search instead of internal descriptor list
	BTO_Preferred = 0x8,				// Bible is a preferred choice, for example Full KJV Database vs Standard KJV Database
	BTO_HasStrongs = 0x10,				// True if this Bible Database has a Strongs Dictionary (Used when resolving ambiguous Bible Databases, like full vs standard KJV)
};
Q_DECLARE_FLAGS(BibleTypeOptionsFlags, BibleTypeOptions)

#define defaultBibleTypeFlags			(BTO_None)

typedef struct TBibleDescriptor {
	BibleTypeOptionsFlags m_btoFlags = defaultBibleTypeFlags;	// Bible database flags
	Qt::LayoutDirection m_nTextDir = Qt::LayoutDirectionAuto;	// Text Direction (ltr or rtl)
	QString m_strWorkID;				// OSIS Work Identifier
	QString m_strLanguage;				// Two-Character international language ID (note: some language codes are more than 2-characters, like ancient Greek is "grc" and Hebrew is "hbo")
	QString m_strDBName;				// Short Database Name
	QString m_strDBDesc;				// Long Database Name/Description
	QString m_strUUID;					// Bible Database UUID
	QString m_strS3DBFilename;			// Sqlite3 Database filename
	QString m_strCCDBFilename;			// Compressed-CSV Database filename
	QString m_strHighlighterUUID;		// Master Highlighter UUID for databases with compatible versification to point to their master

	bool isValid() const { return !m_strUUID.isEmpty(); }
} TBibleDescriptor;

// Must match constBibleDescriptors[]!!  This is also the order we attempt to read/load them in:
enum BIBLE_DESCRIPTOR_ENUM {
	BDE_UNKNOWN = -1,
	BDE_SPECIAL_TEST = 0,
	BDE_KJV = 1,
	BDE_RVG2010_20140126 = 2,
	BDE_KJF2006 = 3,
	BDE_KJVPCE = 4,
	BDE_KJVA = 5,
	BDE_UKJV = 6,
	BDE_GERLUT1545 = 7,
	BDE_RV1865lcbp20100713 = 8,
	BDE_RV1602Prrb20110825 = 9,
	BDE_GERSCH2000 = 10,
	BDE_KJV1611A = 11,
	BDE_KJV1611 = 12,
	BDE_RV1865sbv20140622 = 13,
	BDE_RVG2010_20140705 = 14,
	BDE_KJF2015 = 15,
	BDE_RVG2010_20150120 = 16,
	BDE_TR_20140413_X1 = 17,		// OSIS seg variant x-1 of TR-20140413 OSIS (Stephens 1550)
	BDE_TR_20140413_X2 = 18,		// OSIS seg variant x-2 of TR-20140413 OSIS (Scrivener 1894)
	BDE_SPMT_20120627 = 19,
	BDE_LXX_20080722 = 20,
	BDE_RV1865mv20180504 = 21,
	BDE_RUSSYNODAL_20101106 = 22,
	// ----
	BDE_ASV_20061025 = 23,			// Other Translation for comparative studies
	BDE_ISV_20100807 = 24,			// Other Translation for comparative studies
	// ----
	BDE_KJV_FULL = 25,				// Full KJV database with Strongs Lemma/Morph
	BDE_RUSSYNODAL_20201221 = 26,
	// ----
	BDE_MYBIBLE_ESV_2001 = 27,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	BDE_MYBIBLE_NASB_1971 = 28,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	BDE_MYBIBLE_NASB_2020 = 29,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	BDE_MYBIBLE_NIV_1978 = 30,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	BDE_MYBIBLE_NIV_1984 = 31,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	BDE_MYBIBLE_NIV_2011 = 32,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	BDE_MYBIBLE_NKJV_1982 = 33,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	BDE_MYBIBLE_TNIV_2005 = 34,		// Other Translation for comparative studies -- from Android MyBible App SQLite3
	// ----
	BDE_OSHB = 35,					// Open Scriptures Hebrew Bible
	// ----
};
Q_DECLARE_METATYPE(BIBLE_DESCRIPTOR_ENUM)

// ============================================================================

enum DictionaryTypeOptions {
	DTO_None = 0x0,						// Default for no options
	DTO_SpecialTest = 0x1,				// Is Special Test Database entry
	DTO_AutoLoad = 0x2,					// AutoLoad database at app start
	DTO_IgnoreLang = 0x4,				// Ignore language code (causes the language check to be ignored allowing the dictionary to be used with Bible databases of unmatching languages)
	DTO_DisableTracking = 0x8,			// Disable auto tracking of Bible text word in the dictionary
	DTO_Topical = 0x10,					// Dictionary is a Topical Index/Dictionary
	DTO_TimeLineDictionary = 0x20,		// Timeline dictionary, such as Ussher's Annals of the World
	DTO_Strongs = 0x40,					// Strongs Concordance
	DTO_Preferred = 0x80,				// Dictionary is a preferred choice when auto-finding companion dictionary, such as having multiple Strongs Concordances
	DTO_Discovered = 0x100,				// Discovered via disk search instead of internal descriptor list
};
Q_DECLARE_FLAGS(DictionaryTypeOptionsFlags, DictionaryTypeOptions)

#define defaultDictionaryTypeFlags		(DTO_None)
#define defaultTopicalDctTypeFlags		(DTO_IgnoreLang | DTO_DisableTracking | DTO_Topical)
#define defaultStrongsDctTypeFlags		(DTO_IgnoreLang | DTO_Strongs)

typedef struct TDictionaryDescriptor {
	DictionaryTypeOptionsFlags m_dtoFlags;
	QString m_strLanguage;
	QString m_strDBName;
	QString m_strDBDesc;
	QString m_strUUID;
	QString m_strS3DBFilename;			// Sqlite3 Database filename
	QString m_strCCDBFilename;			// Compressed-CSV Database filename

	bool isValid() const { return !m_strUUID.isEmpty(); }
} TDictionaryDescriptor;

// Must match coonstDictionaryDescriptors[]!!  This is also the order we attempt to read/load them in:
enum DICTIONARY_DESCRIPTOR_ENUM {
	DDE_UNKNOWN = -1,
	DDE_SPECIAL_TEST = 0,
	DDE_WEB1828 = 1,
	DDE_WEB1913 = 2,
	DDE_WEB1806 = 3,
	DDE_USSHER = 4,
	DDE_T_NAVE = 5,
	DDE_T_THOMPSON = 6,
	DDE_T_TOPICAL = 7,
	DDE_T_TORREY = 8
};
Q_DECLARE_METATYPE(DICTIONARY_DESCRIPTOR_ENUM)

// ============================================================================

extern unsigned int bibleDescriptorCount();
extern const TBibleDescriptor &bibleDescriptor(BIBLE_DESCRIPTOR_ENUM nIndex);
extern BIBLE_DESCRIPTOR_ENUM bibleDescriptorFromUUID(const QString &strUUID);

extern unsigned int dictionaryDescriptorCount();
extern const TDictionaryDescriptor &dictionaryDescriptor(DICTIONARY_DESCRIPTOR_ENUM nIndex);
extern DICTIONARY_DESCRIPTOR_ENUM dictionaryDescriptorFromUUID(const QString &strUUID);

// ============================================================================

#endif	// DATABASE_DESCRIPTORS_H
