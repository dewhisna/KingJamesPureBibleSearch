/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef BIBLE_LAYOUT_H
#define BIBLE_LAYOUT_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <map>

// ============================================================================

constexpr uint32_t NUM_BK_OT = 39u;			// Total Books in Old Testament
constexpr uint32_t NUM_BK_NT = 27u;			// Total Books in New Testament
constexpr uint32_t NUM_BK_OT_NT = (NUM_BK_OT + NUM_BK_NT);				// Total Books OT/NT Only (no Aprocrypha)
constexpr uint32_t NUM_BK_APOC = 19u;		// Total Books in Apocrypha (First 14 are in KJVA, Others are not)
constexpr uint32_t NUM_BK = (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC);		// Total Books Defined
constexpr uint32_t NUM_TST = 3u;			// Total Number of Testaments (or pseudo-testaments, in the case of Apocrypha)

// Common OSIS names to use in call to CBibleDatabase->bookIndexFromOSISAbbr() to
//	get the book number, as mainly needed in webChannelBibleAudio.
//	NOTE: The Psalms book number index is hardcoded here because it's
//	needed in parsing tools where a database doesn't exist.  I don't like that,
//	but it isn't easy to eliminate.  It MUST, however, be in sync with the
//	index in g_arrBibleBooks, when 1-originated.  But, do NOT add any other
//	hardcoded indexes here.  Call CBibleDatabase->bookIndexFromOSISAbbr() instead:
constexpr uint32_t PSALMS_BOOK_NUM = 19;	// CRelIndex Book Number for the Book of Psalms
constexpr char OSISNAME_NUMBERS[] = "Num";
constexpr char OSISNAME_PSALMS[] = "Ps";
constexpr char OSISNAME_OBADIAH[] = "Obad";
constexpr char OSISNAME_ZEPHANIAH[] = "Zeph";
constexpr char OSISNAME_PHILEMON[] = "Phlm";
constexpr char OSISNAME_2PETER[] = "2Pet";
constexpr char OSISNAME_2JOHN[] = "2John";
constexpr char OSISNAME_3JOHN[] = "3John";
constexpr char OSISNAME_JUDE[] = "Jude";

// ============================================================================

// Versifications:
enum BIBLE_VERSIFICATION_TYPE_ENUM {		// Note: This list cannot change without breaking settings
	BVTE_KJV = 0,							// KJV Standard Versification
	BVTE_HEBREW_MASORETIC = 1,				// Hebrew Masoretic Standard
	// ----
	BVTE_COUNT
};

// QObject namespace for Bible Versifications Translation Context:
class CBibleVersifications : public QObject
{
	Q_OBJECT

	struct TVersificationEntry {
		QString m_strName;
		QString m_strUUID;
	};
	typedef QList<TVersificationEntry> TBibleVersificationList;	// List of BIBLE_VERSIFICATION_TYPE_ENUM Versifications

public:
	static TBibleVersificationList::size_type count()
	{
		Q_ASSERT(g_arrBibleVersifications.size() == BVTE_COUNT);
		return g_arrBibleVersifications.size();
	}
	static QString name(BIBLE_VERSIFICATION_TYPE_ENUM nVersificationType)
	{
		Q_ASSERT((nVersificationType >= 0) && (nVersificationType < count()));
		return g_arrBibleVersifications.at(nVersificationType).m_strName;
	}
	static QString uuid(BIBLE_VERSIFICATION_TYPE_ENUM nVersificationType)
	{
		Q_ASSERT((nVersificationType >= 0) && (nVersificationType < count()));
		return g_arrBibleVersifications.at(nVersificationType).m_strUUID;
	}
	static BIBLE_VERSIFICATION_TYPE_ENUM lookup(const QString &strUUID)
	{
		for (int ndx = 0; ndx < count(); ++ndx) {
			if (strUUID.compare(g_arrBibleVersifications.at(ndx).m_strUUID, Qt::CaseInsensitive) == 0)
				return static_cast<BIBLE_VERSIFICATION_TYPE_ENUM>(ndx);
		}
		return BVTE_KJV;		// Default to KJV Standard Versification as fallback
	}

private:
	static const TBibleVersificationList g_arrBibleVersifications;
};

// ============================================================================

// Testaments:
// QObject namespace for Bible Testament Translation Context:
class CBibleTestaments : public QObject
{
	Q_OBJECT
	typedef QStringList TBibleTestamentNameList;

public:
	static TBibleTestamentNameList::size_type count()
	{
		Q_ASSERT(g_arrBibleTestamentNames.size() == NUM_TST);
		return g_arrBibleTestamentNames.size();
	}
	static QString name(TBibleTestamentNameList::size_type nTst)
	{
		if ((nTst < 1) || (nTst > static_cast<TBibleTestamentNameList::size_type>(g_arrBibleTestamentNames.size()))) return QString();
		return g_arrBibleTestamentNames.at(nTst-1);			// NOTE: Unlike Enum variants, nTst is the 1-based Testament Index!
	}

private:
	static const TBibleTestamentNameList g_arrBibleTestamentNames;
};

// ============================================================================

// Category Groups:
enum BIBLE_BOOK_CATEGORY_GROUP_ENUM {		// Note: This list cannot change without breaking settings
	BBCGE_KJV = 0,							// KJV Standard Book Groups
	BBCGE_HEBREW_MASORETIC = 1,				// Hebrew Masoretic Book Groups
	// ----
	BBCGE_COUNT
};

// QObject namespace for Bible Book Category Groups Translation Context:
class CBibleBookCategoryGroups : public QObject
{
	Q_OBJECT
	typedef QStringList TBibleBookCategoryGroupList;	// List of BIBLE_BOOK_CATEGORY_GROUP_ENUM Groups

public:
	static TBibleBookCategoryGroupList::size_type count()
	{
		Q_ASSERT(g_arrBibleBookCategoryGroups.size() == BBCGE_COUNT);
		return g_arrBibleBookCategoryGroups.size();
	}
	static QString name(BIBLE_BOOK_CATEGORY_GROUP_ENUM nCatGroup)
	{
		Q_ASSERT((nCatGroup >= 0) && (nCatGroup < count()));
		return g_arrBibleBookCategoryGroups.at(nCatGroup);
	}

private:
	static const TBibleBookCategoryGroupList g_arrBibleBookCategoryGroups;
};

// ============================================================================

// Categories:
enum BIBLE_BOOK_CATEGORIES_ENUM {
	BBCE_UNKNOWN = 0,
	BBCE_LAW = 1,
	BBCE_HISTORICAL = 2,
	BBCE_WISDOM_POETIC = 3,
	BBCE_MAJOR_PROPHETS = 4,
	BBCE_MINOR_PROPHETS = 5,
	BBCE_PROPHETS = 6,
	BBCE_FORMER_PROPHETS = 7,
	BBCE_LATTER_PROPHETS = 8,
	BBCE_BOOK_OF_THE_TWELVE = 9,		// i.e. Minor Prophets (Hebrew)
	BBCE_WRITINGS = 10,					// After prophets for sort order control
	BBCE_GOSPELS = 11,
	BBCE_PAULINE_EPISTLES = 12,
	BBCE_GENERAL_EPISTLES = 13,
	BBCE_APOCALYPTIC_EPISTLE = 14,
	// ----
	BBCE_COUNT
};

// QObject namespace for Bible Book Category Translation Context:
class CBibleBookCategories : public QObject
{
	Q_OBJECT
	typedef QStringList TBibleBookCategoryList;		// List of BIBLE_BOOK_CATEGORIES_ENUM Categories
public:
	static TBibleBookCategoryList::size_type count()
	{
		Q_ASSERT(g_arrBibleBookCategories.size() == BBCE_COUNT);
		return g_arrBibleBookCategories.size();
	}
	static QString name(BIBLE_BOOK_CATEGORIES_ENUM nCat)
	{
		Q_ASSERT((nCat >= 0) && (nCat < count()));
		return g_arrBibleBookCategories.at(nCat);
	}

private:
	static const TBibleBookCategoryList g_arrBibleBookCategories;
};

// Use std::map here instead of QMap so we can use bracket-initializers
//	in .cpp and support Qt4:
typedef std::map<QString, BIBLE_BOOK_CATEGORIES_ENUM> TBibleBookCategoryMap;	// Mapping of OSIS ID to Category
extern const TBibleBookCategoryMap g_mapKJVBookCategories;						// BBCGE_KJV
extern const TBibleBookCategoryMap g_mapHebrewMasoreticBookCategories;			// BBCGE_HEBREW_MASORETIC

// ============================================================================
// ============================================================================

//
// Everything below this point is only for external CLI tools, like
//	KJVDataParse for creating databases and parsing external files!
//

// QObject namespace for Bible Book Names Translation Context:
class CBibleBookNames : public QObject
{
	Q_OBJECT
};

// ============================================================================

// QObject namespace for Bible Book Abbreviations Translation Context:
class CBibleBookAbbr : public QObject
{
	Q_OBJECT
};

// ============================================================================

// QObject namespace for Bible Book Description Translation Context:
class CBibleBookDescription : public QObject
{
	Q_OBJECT
};

// ============================================================================

// Books:
// TBibleBook Structure -- Used for Parsing Bible Database Files (KJVDataParse, etc)
typedef struct TBibleBook {
	uint32_t m_ndxStartingChapterVerse;		// CRelIndex of Chapter and Verse this book is supposed to start at (used to handle special case Apocrypha entries, like AddEsther), Stored as uint32_t instead of CRelIndex to avoid circular include
	QStringList m_lstOsisAbbr;				// List of OSIS IDs to apply for this book (to allow for things like EsthGr and AddEsth to be synonyms).  Only the FIRST will be used in our databases!
	QString m_strTableName;					// Database Table Name for this book
	QString m_strName;						// BookName (Translated)
	QString m_strCommonAbbr;				// Semicolon separated list of Abbreviation Strings (Translated)
	QString m_strDescription;				// Book Description (Translated)
} TBibleBook;

typedef QList<TBibleBook> TBibleBookList;
extern const TBibleBookList g_arrBibleBooks;

// ============================================================================

// Bible Book->Chapter/Verse Mapping:
typedef QList<QStringList> TBibleChapterVerseCounts;

class CKJVBibleChapterVerseCounts : public TBibleChapterVerseCounts
{
private:
	CKJVBibleChapterVerseCounts();

public:
	static const CKJVBibleChapterVerseCounts *instance();
};

// ============================================================================

#endif	// BIBLE_LAYOUT_H
