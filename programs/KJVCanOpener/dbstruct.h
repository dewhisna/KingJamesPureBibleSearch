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

#ifndef DBSTRUCT_H
#define DBSTRUCT_H

#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdint.h>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QPair>
#include <QMetaType>
#include <QDataStream>
#include <QSharedPointer>
#include <QObject>

#include <assert.h>

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif


// ============================================================================

// CRelIndex Masks:
#define RIMASK_HEADING	0x10
#define RIMASK_BOOK		0x08
#define RIMASK_CHAPTER	0x04
#define RIMASK_VERSE	0x02
#define RIMASK_WORD		0x01
#define RIMASK_ALL		0x1F

class CRelIndex {
public:
	CRelIndex(const CRelIndex &ndx)
		:	m_ndx(ndx.index())
	{
	}
	CRelIndex(uint32_t ndx = 0)
		:	m_ndx(ndx)
	{
	}
	CRelIndex(const QString &strAnchor)
		:	m_ndx(strAnchor.toUInt())
	{
	}
	CRelIndex(uint32_t nBk, uint32_t nChp, uint32_t nVrs, uint32_t nWrd)
	{
		setIndex(nBk, nChp, nVrs, nWrd);
	}
	~CRelIndex() { }

	inline QString asAnchor() const {			// Anchor is a text string unique to this reference
		return QString("%1").arg(m_ndx);
	}

	inline uint32_t book() const { return ((m_ndx >> 24) & 0xFF); }
	inline void setBook(uint32_t nBk) {
		m_ndx = ((m_ndx & 0x00FFFFFF) | ((nBk & 0xFF) << 24));
	}
	inline uint32_t chapter() const { return ((m_ndx >> 16) & 0xFF); }
	inline void setChapter(uint32_t nChp) {
		m_ndx = ((m_ndx & 0xFF00FFFF) | ((nChp & 0xFF) << 16));
	}
	inline uint32_t verse() const { return ((m_ndx >> 8) & 0xFF); }
	inline void setVerse(uint32_t nVrs) {
		m_ndx = ((m_ndx & 0xFFFF00FF) | ((nVrs & 0xFF) << 8));
	}
	inline uint32_t word() const { return (m_ndx & 0xFF); }
	inline void setWord(uint32_t nWrd) {
		m_ndx = ((m_ndx & 0xFFFFFF00) | (nWrd & 0xFF));
	}
	inline bool isSet() const { return (m_ndx != 0); }

	inline uint32_t index() const { return m_ndx; }
	inline void setIndex(uint32_t nBk, uint32_t nChp, uint32_t nVrs, uint32_t nWrd) {
		m_ndx = (((nBk & 0xFF) << 24) | ((nChp & 0xFF) << 16) | ((nVrs & 0xFF) << 8) | (nWrd & 0xFF));
	}
	inline void setIndex(uint32_t ndx) {
		m_ndx = ndx;
	}

	inline bool operator<(const CRelIndex &ndx) const {
		return (index() < ndx.index());
	}
	inline bool operator>(const CRelIndex &ndx) const {
		return (index() > ndx.index());
	}
	inline bool operator==(const CRelIndex &ndx) const {
		return (index() == ndx.index());
	}

private:
	uint32_t m_ndx;
};
inline QDataStream& operator<<(QDataStream &out, const CRelIndex &ndx) {
	out << ndx.index();
	return out;
}
inline QDataStream& operator>>(QDataStream &in, CRelIndex &ndx) {
	uint32_t anIndex;
	in >> anIndex;
	ndx.setIndex(anIndex);
	return in;
}

// ============================================================================

// Pair representing X (first) of Y (second) things:
class TCountOf : public QPair<unsigned int, unsigned int>
{
public:
	explicit inline TCountOf(unsigned int x = 0, unsigned int y = 0)
		:	QPair<unsigned int, unsigned int>(x, y) { }
};

class CBibleDatabase;		// Forward declaration:

class CRefCountCalc			// Calculates the reference count information for creating ToolTips and indices
{
public:
	enum REF_TYPE_ENUM {
		RTE_TESTAMENT = 0,
		RTE_BOOK = 1,
		RTE_CHAPTER = 2,
		RTE_VERSE = 3,
		RTE_WORD = 4
	};

	CRefCountCalc(const CBibleDatabase *pBibleDatabase, REF_TYPE_ENUM nRefType, const CRelIndex &refIndex);
	~CRefCountCalc() { }

	REF_TYPE_ENUM refType() const { return m_nRefType; }
	CRelIndex refIndex() const { return m_ndxRef; }

	TCountOf ofBible() const { return m_nOfBible; }
	TCountOf ofTestament() const { return m_nOfTst; }
	TCountOf ofBook() const { return m_nOfBk; }
	TCountOf ofChapter() const { return m_nOfChp; }
	TCountOf ofVerse() const { return m_nOfVrs; }

private:
	CRelIndex m_ndxRef;			// Relative Index

	REF_TYPE_ENUM m_nRefType;	// Type of Reference these counts are for

	// All entries above the type specified will be valid:
	TCountOf m_nOfBible;	// Testament, Book, Chapter, Verse, Word of the whole Bible
	TCountOf m_nOfTst;		// Book, Chapter, Verse, Word of the Testament
	TCountOf m_nOfBk;		// Chapter, Verse, Word of the Book
	TCountOf m_nOfChp;		// Verse, Word of the Chapter
	TCountOf m_nOfVrs;		// Word of the Verse
};


// ============================================================================

typedef std::vector<uint32_t> TIndexList;			// Index List for words into book/chapter/verse/word

struct IndexSortPredicate {
	bool operator() (const CRelIndex &v1, const CRelIndex &v2) const
	{
		return (v1.index() < v2.index());
	}
};

struct XformLower {
	int operator()(int c)
	{
		return tolower(c);
	}
};

// ============================================================================

// Testaments -- Table of Testaments:
//
class CTestamentEntry
{
public:
	CTestamentEntry(const QString &strTstName = QString())
	:	m_strTstName(strTstName),
		m_nNumBk(0),
		m_nNumChp(0),
		m_nNumVrs(0),
		m_nNumWrd(0)
	{ }
	~CTestamentEntry() { }

	QString  m_strTstName;		// Name of testament (display name)
	unsigned int m_nNumBk;		// Number of Books in this testament
	unsigned int m_nNumChp;		// Number of Chapters in this testament
	unsigned int m_nNumVrs;		// Number of Verses in this testament
	unsigned int m_nNumWrd;		// Number of Words in this testament
};

typedef std::vector<CTestamentEntry> TTestamentList;		// Index by nTst-1

// Bible -- Bible Entry (Derived from CTestamentEntry to keep stats for the whole Bible)
class CBibleEntry : public CTestamentEntry
{
public:
	CBibleEntry()
	:	CTestamentEntry(QObject::tr("Entire Bible")),
		m_nNumTst(0)
	{ }
	~CBibleEntry() { }

	unsigned int m_nNumTst;		// Number of Testaments
};

// ============================================================================

// Books -- (Table of Contents):
//
class CBookEntry
{
public:
	CBookEntry()
	:   m_nTstBkNdx(0),
		m_nTstNdx(0),
		m_nNumChp(0),
		m_nNumVrs(0),
		m_nNumWrd(0),
		m_nWrdAccum(0)
	{ }
	~CBookEntry() { }

	unsigned int m_nTstBkNdx;	// Testament Book Index (Index within the books of the testament) 1-39 or 1-27
	unsigned int m_nTstNdx;		// Testament Index (1=Old, 2=New, etc)
	QString m_strBkName;		// Name of book (display name)
	QString m_strBkAbbr;		// Book Abbreviation
	QString m_strTblName;		// Name of Table for this book
	unsigned int m_nNumChp;		// Number of chapters in this book
	unsigned int m_nNumVrs;		// Number of verses in this book
	unsigned int m_nNumWrd;		// Number of words in this book
	unsigned int m_nWrdAccum;	// Number of accumulated words prior to, but not including this book
	QString m_strCat;			// Category name
	QString m_strDesc;			// Description (subtitle)
};

typedef std::vector<CBookEntry> TBookList;	// Index by nBk-1

// ============================================================================

// Chapters -- Book/Chapter Layout:
//
class CChapterEntry
{
public:
	CChapterEntry()
	:   m_nNumVrs(0),
		m_nNumWrd(0),
		m_nWrdAccum(0)
	{ }
	~CChapterEntry() { }

	unsigned int m_nNumVrs;		// Number of verses in this chapter
	unsigned int m_nNumWrd;		// Number of words in this chapter
	unsigned int m_nWrdAccum;	// Number of accumulated words prior to, but not including this chapter
};

typedef std::map<CRelIndex, CChapterEntry, IndexSortPredicate> TChapterMap;	// Index by [nBk|nChp|0|0]

// ============================================================================

// Verses -- Chapter/Verse Layout:
//
class CVerseEntry
{
public:
	enum PILCROW_TYPE_ENUM {
		PTE_NONE = 0,
		PTE_MARKER = 1,				// Example: {verse}[osisID=Gen.5.21]{milestone}[marker=¶,type=x-p]{/milestone}
		PTE_MARKER_ADDED = 2,		// Example: {verse}[osisID=Gen.5.3]{milestone}[marker=¶,subType=x-added,type=x-p]{/milestone}
		PTE_EXTRA = 3				// Example: {verse}[osisID=Gen.5.6]{milestone}[type=x-extra-p]{/milestone}
	};

	CVerseEntry()
	:   m_nNumWrd(0),
		m_nWrdAccum(0),
		m_nPilcrow(PTE_NONE)
	{ }
	~CVerseEntry() { }

	unsigned int m_nNumWrd;			// Number of words in this verse
	unsigned int m_nWrdAccum;		// Number of accumulated words prior to, but not including this verse
	PILCROW_TYPE_ENUM m_nPilcrow;	// Start of verse Pilcrow Flag (and Pilcrow type)
	QString m_strTemplate;			// Rich Text Creation Template

#ifdef OSIS_PARSER_BUILD
	QString m_strText;			// Rich text (or plain if Rich unavailable) for the verse (Note: for mobile versions, this element can be removed and fetched from the database if needed)
	QStringList m_lstWords;			// Word List for parse extraction
	QStringList m_lstRichWords;		// Word List as Rich Text for parse extraction
	QStringList m_lstParseStack;	// Parse operation stack (used to parse red-letter tags, added word tags, morphology, concordance references, etc.
#endif

};

typedef std::map<CRelIndex, CVerseEntry, IndexSortPredicate> TVerseEntryMap;		// Index by [0|nChp|nVrs|0]

typedef std::vector<TVerseEntryMap> TBookVerseList;		// Index by nBk-1

#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
typedef std::map<CRelIndex, QString, IndexSortPredicate> TVerseCacheMap;			// Index by [nBk|nChp|nVrs|0]
typedef std::map<uint, TVerseCacheMap> TSpecVerseCacheMap;							// Specific TVerseCacheMap -- Index by CVerseTextRichifierTags hash
#endif

// ============================================================================

// Words -- Word List and Mapping
//
class CWordEntry
{
public:
	CWordEntry()
	:	m_bCasePreserve(false)
	{ }
	~CWordEntry() { }

	QString m_strWord;			// Word Text
	bool m_bCasePreserve;		// Special Word Case Preserve
	QStringList m_lstAltWords;	// List of alternate synonymous words for searching (such as hyphenated and non-hyphenated)
	QList<unsigned int> m_lstAltWordCount;		// Count for each alternate word.  This will be the number of entries for this word in the mapping below
	TIndexList m_ndxNormalizedMapping;	// Normalized Indexes Mapping into entire Bible

	struct SortPredicate {
		bool operator() (const QString &s1, const QString &s2) const
		{
			return (s1.compare(s2, Qt::CaseSensitive) < 0);
		}
	};
};

typedef std::map<QString, CWordEntry, CWordEntry::SortPredicate> TWordListMap;		// Indexed by lowercase words from word-list

// ============================================================================

// Concordance -- Mapping of words and their Normalized positions:
//

class CConcordanceEntry
{
public:
	CConcordanceEntry(const QString &strWord, int nIndex = 0);

	inline const QString &word() const { return m_strWord; }
	inline const QString &decomposedWord() const { return m_strDecomposedWord; }
	inline QString soundEx() const;
//	inline const QString &soundEx() const;
	inline int index() const { return m_nIndex; }

	bool operator==(const CConcordanceEntry &src) const
	{
		return (m_strWord.compare(src.m_strWord) == 0);
	}

private:
	QString m_strWord;						// Composed Word (as in the actual text)
	QString m_strDecomposedWord;			// Decomposed Word (used for matching)
//	mutable QString m_strSoundEx;			// SoundEx for Decomposed Word (for matching)
	int m_nIndex;							// Index used when sorting and keeping external reference intact
};

typedef QList<CConcordanceEntry> TConcordanceList;

struct TConcordanceListSortPredicate {
	static bool ascendingLessThanWordCaseInsensitive(const CConcordanceEntry &s1, const CConcordanceEntry &s2)
	{
		return (s1.decomposedWord().compare(s2.decomposedWord(), Qt::CaseInsensitive) < 0);
	}

	static bool ascendingLessThanWordCaseSensitive(const CConcordanceEntry &s1, const CConcordanceEntry &s2)
	{
		return (s1.decomposedWord().compare(s2.decomposedWord(), Qt::CaseSensitive) < 0);
	}
};

// ============================================================================

// Footnotes -- Footnote List and Mapping
//		Note: This works consistently for book-only footnotes, chapter footnotes,
//		verse footnotes, and even word footnotes if we wish.  The index into the
//		map is the complete CRelIndex style.  For book-only, for example, the
//		chapter, verse, and word will be 0.  For a chapter note, the verse and
//		word will be 0, etc.  This allows us to easily and quickly query for the
//		type of note we need.
//

class CFootnoteEntry
{
public:
	CFootnoteEntry() { }
	~CFootnoteEntry() { }

	QString text() const		// We'll use a function to fetch the text (on mobile this can be a database lookup if need be)
	{
		return m_strText;
	}
	void setText(const QString &strText)		// This can be a no-op on mobile if doing direct database lookups
	{
		m_strText = strText;
	}

private:
	QString m_strText;			// Rich text (or plain if Rich unavailable) for the footnote (Note: for mobile versions, this element can be removed and fetched from the database if needed)
};

typedef std::map<CRelIndex, CFootnoteEntry, IndexSortPredicate> TFootnoteEntryMap;		// Index by [nBk|nChp|nVrs|nWrd]

// ============================================================================

// Phrases -- Common and Saved Search Phrase Lists:
//

class CParsedPhrase;		// Forward declaration

class CPhraseEntry
{
public:
	CPhraseEntry(const QString &strEncodedText = QString(), const QVariant &varExtraInfo = QVariant());
	~CPhraseEntry();

	void clear();

	void setFromPhrase(const CParsedPhrase *pPhrase);

	inline const QString &text() const { return m_strPhrase; }
	QString textEncoded() const;
	void setText(const QString &strText);
	void setTextEncoded(const QString &strText);

	inline bool caseSensitive() const { return m_bCaseSensitive; }
	inline void setCaseSensitive(bool bCaseSensitive) { m_bCaseSensitive = bCaseSensitive; }

	inline bool accentSensitive() const { return m_bAccentSensitive; }
	inline void setAccentSensitive(bool bAccentSensitive) { m_bAccentSensitive = bAccentSensitive; }

	inline bool isDisabled() const { return m_bDisabled; }
	inline void setDisabled(bool bDisabled) { m_bDisabled = bDisabled; }

	inline QVariant extraInfo() const { return m_varExtraInfo; }
	inline void setExtraInfo(const QVariant &varExtraInfo) { m_varExtraInfo = varExtraInfo; }

	inline int wordCount() const { return m_nNumWrd; }

	bool operator==(const CPhraseEntry &src) const
	{
		return ((m_bCaseSensitive == src.m_bCaseSensitive) &&
				(m_bAccentSensitive == src.m_bAccentSensitive) &&
				// Don't compare m_bDisabled because that doesn't affect "equality"
				(m_strPhrase.compare(src.m_strPhrase, Qt::CaseSensitive) == 0));
	}

	bool operator==(const CParsedPhrase &src) const;		// Implemented in PhraseEdit.cpp, where CParsedPhrase is defined

	static const QChar encCharCaseSensitive() { return QChar(0xA7); } 			// Section Sign = Case-Sensitive
	static const QChar encCharAccentSensitive() { return QChar(0xA4); }			// Current Sign = Accent-Sensitive
	static const QChar encCharDisabled() { return QChar(0xAC); }					// Not Sign = Disable flag

private:
	bool m_bCaseSensitive;
	bool m_bAccentSensitive;
	bool m_bDisabled;
	QString m_strPhrase;
	unsigned int m_nNumWrd;		// Number of words in phrase
	QVariant m_varExtraInfo;	// Extra user info for specific uses of this structure
};

Q_DECLARE_METATYPE(CPhraseEntry)

class CPhraseList : public QList<CPhraseEntry>
{
public:
	inline CPhraseList() { }
	inline explicit CPhraseList(const CPhraseEntry &i) { append(i); }
	inline CPhraseList(const CPhraseList &l) : QList<CPhraseEntry>(l) { }
	inline CPhraseList(const QList<CPhraseEntry> &l) : QList<CPhraseEntry>(l) { }

	int removeDuplicates();
};

inline uint qHash(const CPhraseEntry &key)
{
	// Note: Aren't hasing "disable" because it doesn't affect the main key value equality
	uint nHash = (qHash(key.text()) << 2) + (key.caseSensitive() ? 2 : 0) + (key.accentSensitive() ? 1 : 0);
	return nHash;
}

extern CPhraseList g_lstUserPhrases;			// User-defined phrases read from optional user database
extern bool g_bUserPhrasesDirty;				// True if user has edited the phrase list


// ============================================================================

class CReadDatabase;			// Forward declaration for class friendship
class COSISXmlHandler;

class CVerseTextRichifierTags;

// CBibleDatabase - Class to define a Bible Database file
class CBibleDatabase
{
private:
	CBibleDatabase(const QString &strName, const QString &strDescription);		// Creatable by CReadDatabase
public:
	~CBibleDatabase();

	QString name() const { return m_strName; }
	QString description() const { return m_strDescription; }

	// CRelIndex Name/Report Functions:
	QString SearchResultToolTip(const CRelIndex &nRelIndex, unsigned int nRIMask = RIMASK_ALL, unsigned int nSelectionSize = 1) const;		// Create complete reference statistics report
	QString PassageReferenceText(const CRelIndex &nRelIndex) const;		// Creates a reference text string like "Genesis 1:1 [5]"

	QString testamentName(const CRelIndex &nRelIndex) const;
	uint32_t testament(const CRelIndex &nRelIndex) const;

	QString bookName(const CRelIndex &nRelIndex) const;

	// CRelIndex Transformation Functions:
#ifdef OSIS_PARSER_BUILD
	inline uint32_t NormalizeIndexNoAccum(const CRelIndex &nRelIndex) const { return NormalizeIndexNoAccum(nRelIndex.index()); }
	uint32_t NormalizeIndexNoAccum(uint32_t nRelIndex) const;
	uint32_t DenormalizeIndexNoAccum(uint32_t nNormalIndex) const;
#endif
	inline uint32_t NormalizeIndex(const CRelIndex &nRelIndex) const { return NormalizeIndex(nRelIndex.index()); }
	uint32_t NormalizeIndex(uint32_t nRelIndex) const;
	uint32_t DenormalizeIndex(uint32_t nNormalIndex) const;

	// calcRelIndex - Calculates a relative index from counts.  For example, starting from (0,0,0,0):
	//			calcRelIndex(1, 1, 666, 0, 1);						// Returns (21,7,1,1) or Ecclesiastes 7:1 [1], Word 1 of Verse 1 of Chapter 666 of the Bible
	//			calcRelIndex(1, 393, 0, 5, 0);						// Returns (5, 13, 13, 1) or Deuteronomy 13:13 [1], Word 1 of Verse 393 of Book 5 of the Bible
	//			calcRelIndex(1, 13, 13, 5, 0);						// Returns (5, 13, 13, 1) or Deuteronomy 13:13 [1], Word 1 of Verse 13 of Chapter 13 of Book 5 of the Bible
	//			calcRelIndex(1, 13, 13, 5, 1);						// Returns (5, 13, 13, 1) or Deuteronomy 13:13 [1], Word 1 of Verse 13 of Chapter 13 of Book 5 of the Old Testament
	//			calcRelIndex(1, 13, 13, 5, 2);						// Returns (44, 13, 13, 1) or Acts 13:13 [1], Word 1 of Verse 13 of Chapter 13 of Book 5 of the New Testament
	//			calcRelIndex(0, 13, 13, 5, 2);						// Returns (44, 13, 13, 1) or Acts 13:13 [1], Word 1 of Verse 13 of Chapter 13 of Book 5 of the New Testament
	CRelIndex calcRelIndex(
					unsigned int nWord, unsigned int nVerse, unsigned int nChapter,
					unsigned int nBook, unsigned int nTestament,
					CRelIndex ndxStart = CRelIndex(),
					bool bReverse = false) const;

	inline const CBibleEntry &bibleEntry() const						// Bible stats entry
	{
		return m_EntireBible;
	}
	const CTestamentEntry *testamentEntry(uint32_t nTst) const;			// Testament stats/data entry
	const CBookEntry *bookEntry(uint32_t nBk) const;					// Book Data or Table of Contents [Books]
#ifdef OSIS_PARSER_BUILD
	const CChapterEntry *chapterEntry(const CRelIndex &ndx, bool bForceCreate = false) const;		// Chapter Data Use CRelIndex:[Book | Chapter | 0 | 0]
	const CVerseEntry *verseEntry(const CRelIndex &ndx, bool bForceCreate = false) const;			// Verse Data Entry Use CRelIndex:[Book | Chapter | Verse | 0]
#else
	const CChapterEntry *chapterEntry(const CRelIndex &ndx) const;		// Chapter Data Use CRelIndex:[Book | Chapter | 0 | 0]
	const CVerseEntry *verseEntry(const CRelIndex &ndx) const;			// Verse Data Entry Use CRelIndex:[Book | Chapter | Verse | 0]
#endif
	const CWordEntry *wordlistEntry(const QString &strWord) const;		// WordList Data Entry: Index by lowercase keyword
	inline const TWordListMap &mapWordList() const						// Master word-list Map
	{
		return m_mapWordList;
	}
	inline const QStringList &lstWordList() const						// List version of Master word-list (used for quick matching)
	{
		return m_lstWordList;
	}
	inline const TConcordanceList &concordanceWordList() const			// List of all words as composed UTF8 in sorted order.  Used for index mapping, lookup and searching
	{
		return m_lstConcordanceWords;
	}
	QString wordAtIndex(uint32_t ndxNormal) const;						// Returns word of the Bible based on Normalized Index (1 to Max) -- Automatically does ConcordanceMapping Lookups
	QString decomposedWordAtIndex(uint32_t ndxNormal) const;			// Returns word of the Bible (decomposed) based on Normalized Index (1 to Max) -- Automatically does ConcordanceMapping Lookups
	const CFootnoteEntry *footnoteEntry(const CRelIndex &ndx) const;	// Footnote Data Entry, Used CRelIndex:[Book | Chapter | Verse | Word], for unused, set to 0, example: [1 | 1 | 0 | 0] for Genesis 1 (See TFootnoteEntryMap above)
	inline const TFootnoteEntryMap &footnotesMap() const				// Entire Footnote Map, needed for database generation
	{
		return m_mapFootnotes;
	}
	inline const CPhraseList &phraseList() const						// Returns the Common Phrases List from the Main Database for this Bible Database
	{
		return m_lstCommonPhrases;
	}

	QString richVerseText(const CRelIndex &ndxRel, const CVerseTextRichifierTags &tags, bool bAddAnchors = false) const;	// Generate and return verse text for specified index: [Book | Chapter | Verse | 0]
#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
	void dumpRichVerseTextCache(uint nTextRichifierTagHash = 0);		// Dump the cache for a specific CVerseTextRichifierTags object (pass its hash) or all data (pass 0)
#endif

private:
	// CReadDatabase needed to load the database.  After that everything
	//	is read-only.  Database building is done directly from the CSV files
	//
	friend class CReadDatabase;
	friend class COSISXmlHandler;			// COSISXmlHandler - Used by KJVDataParse for OSIS XML File processing to build KJPBS databases

// Main Database Data:
	CBibleEntry m_EntireBible;				// Entire Bible stats, calculated from testament stats in ReadDB.
	TTestamentList m_lstTestaments;			// Testament List: List(nTst-1)
	TBookList m_lstBooks;					// Books (Table of Contents): List(nBk-1)
	TChapterMap m_mapChapters;				// Chapter Entries Map: Map(CRelIndex[nBk | nChp | 0 | 0])
	TBookVerseList m_lstBookVerses;			// Book Verse Entries List: List(nBk-1) -> Map(CRelIndex[0 | nChp | nVrs | 0])
	TWordListMap m_mapWordList;				// Master word-list Map (Indexed by lowercase word)
	QStringList m_lstWordList;				// Master word-list List as lowercase, used for searching lower/upper-bound for m_mapWordList
	TConcordanceList m_lstConcordanceWords;	// List (QStringList) of all Unique Words as Composed UTF8 in the order for the concordance with names of the TWordListMap key (starts at index 0)
	TIndexList m_lstConcordanceMapping;		// List of WordNdx# (in ConcordanceWords) for all 789629 words of the text (starts at index 1)
	TFootnoteEntryMap m_mapFootnotes;		// Footnotes (typed by index - See notes above with TFootnoteEntryMap)
	CPhraseList m_lstCommonPhrases;			// Common phrases read from database

// Local Data:
	QString m_strName;						// Name for this database
	QString m_strDescription;				// Database description

// Cache:
#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
	mutable TSpecVerseCacheMap m_mapVerseCacheWithAnchors;		// Map of Verse Cache Maps to store rendered rich text if g_nRichTextCachingMode is as RTCME_FULL
	mutable TSpecVerseCacheMap m_mapVerseCacheNoAnchors;		//  "(ditto)" (but without anchors)
#endif
};


typedef QSharedPointer<CBibleDatabase> CBibleDatabasePtr;

typedef QList<CBibleDatabasePtr> TBibleDatabaseList;

extern CBibleDatabasePtr g_pMainBibleDatabase;		// Main Database (database currently active for main navigation)
extern TBibleDatabaseList g_lstBibleDatabases;

// ============================================================================

// Relative Index and Word Count pair used for highlight phrases found:
class TPhraseTag
{
public:
	explicit inline TPhraseTag(const CRelIndex &ndx = CRelIndex(), unsigned int nCount = 0)
		:	m_RelIndex(ndx),
			m_nCount(nCount)
	{ }

	inline const CRelIndex &relIndex() const { return m_RelIndex; }
	inline CRelIndex &relIndex() { return m_RelIndex; }
	inline const unsigned int &count() const { return m_nCount; }
	inline unsigned int &count() { return m_nCount; }

	QString PassageReferenceRangeText(CBibleDatabasePtr pBibleDatabase) const {
		assert(pBibleDatabase.data() != NULL);

		if (pBibleDatabase.data() == NULL) return QString();
		QString strReferenceRangeText = pBibleDatabase->PassageReferenceText(m_RelIndex);
		if (m_nCount > 1) {
			uint32_t nNormal = pBibleDatabase->NormalizeIndex(m_RelIndex);
			strReferenceRangeText += " - " + pBibleDatabase->PassageReferenceText(CRelIndex(pBibleDatabase->DenormalizeIndex(nNormal + m_nCount - 1)));
		}
		return strReferenceRangeText;
	}

	bool haveSelection() const {
		return ((m_RelIndex.isSet()) && (m_nCount != 0));
	}

	bool operator==(const TPhraseTag &otherTag) {
		return ((m_RelIndex.index() == otherTag.relIndex().index()) &&
				(m_nCount == otherTag.count()));
	}

	bool operator!=(const TPhraseTag &otherTag) {
		return ((m_RelIndex.index() != otherTag.relIndex().index()) ||
				(m_nCount != otherTag.count()));
	}

private:
	CRelIndex m_RelIndex;
	unsigned int m_nCount;
};
inline QDataStream& operator<<(QDataStream &out, const TPhraseTag &ndx) {
	out << ndx.relIndex() << ndx.count();
	return out;
}
inline QDataStream& operator>>(QDataStream &in, TPhraseTag &ndx) {
	in >> ndx.relIndex() >> ndx.count();
	return in;
}
Q_DECLARE_METATYPE(TPhraseTag)

const QString g_constrPhraseTagMimeType("application/vnd.dewtronics.kjvcanopener.phrasetag");

typedef QList<TPhraseTag> TPhraseTagList;				// List of tags used for highlighting found phrases, etc.
typedef QList<TPhraseTagList> TPhraseTagListList;		// List of tag lists, use to keep tag lists for multiple phrases

struct TPhraseTagListSortPredicate {
	static bool ascendingLessThan(const TPhraseTag &s1, const TPhraseTag &s2)
	{
		return (s1.relIndex().index() < s2.relIndex().index());
	}
};

// ============================================================================

// Global Settings:


// ============================================================================

#endif // DBSTRUCT_H
