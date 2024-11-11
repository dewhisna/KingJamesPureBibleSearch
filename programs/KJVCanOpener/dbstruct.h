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

#ifndef DATABASE_STRUCT_H
#define DATABASE_STRUCT_H

#include "dbDescriptors.h"
#include "BibleLayout.h"

#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>
#include <stdint.h>
#include <QString>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QList>
#include <QMap>
#include <QMultiMap>
#include <QVariant>
#include <QPair>
#include <QMetaType>
#include <QDataStream>
#include <QSharedPointer>
#include <QObject>
#ifndef NOT_USING_SQL
#include <QSqlDatabase>
#endif

#ifdef USE_GEMATRIA
#include "Gematria.h"
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

// Qt Deprecation/Reorganization-isms so that we can compile without issues
//		on Qt 4, Qt 5, and Qt 6:

#if QT_VERSION >= 0x050F00
#define My_QString_KeepEmptyParts Qt::KeepEmptyParts
#define My_QString_SkipEmptyParts Qt::SkipEmptyParts
#else
#define My_QString_KeepEmptyParts QString::KeepEmptyParts
#define My_QString_SkipEmptyParts QString::SkipEmptyParts
#endif

// ============================================================================

#define KJPBS_CCDB_VERSION		2			// Current version of our CCDB file format

// ============================================================================

extern const QString &initialAppDirPath();		// Initial Application path storage, since QCoreApplication::applicationDirPath() assumes we haven't changed our directory

// ============================================================================

// Forward declarations:
class CBibleDatabase;
class CBasicHighlighter;

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
	CRelIndex(const CRelIndex &ndx) = default;
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

	inline bool isColophon() const {
		return ((book() != 0) && (chapter() == 0) && (verse() == 0) && (word() != 0));
	}
	inline bool isSuperscription() const {
		return ((book() != 0) && (chapter() != 0) && (verse() == 0) && (word() != 0));
	}

	inline QString asAnchor() const {			// Anchor is a text string unique to this reference
		return QString("%1").arg(m_ndx);
	}

	static uint32_t maxBookCount() { return 0xFF; }
	inline uint32_t book() const { return ((m_ndx >> 24) & 0xFF); }
	inline void setBook(uint32_t nBk) {
		m_ndx = ((m_ndx & 0x00FFFFFF) | ((nBk & 0xFF) << 24));
	}
	static uint32_t maxChapterCount() { return 0xFF; }
	inline uint32_t chapter() const { return ((m_ndx >> 16) & 0xFF); }
	inline void setChapter(uint32_t nChp) {
		m_ndx = ((m_ndx & 0xFF00FFFF) | ((nChp & 0xFF) << 16));
	}
	static uint32_t maxVerseCount() { return 0xFF; }
	inline uint32_t verse() const { return ((m_ndx >> 8) & 0xFF); }
	inline void setVerse(uint32_t nVrs) {
		m_ndx = ((m_ndx & 0xFFFF00FF) | ((nVrs & 0xFF) << 8));
	}
	static uint32_t maxWordCount() { return 0xFF; }
	inline uint32_t word() const { return (m_ndx & 0xFF); }
	inline void setWord(uint32_t nWrd) {
		m_ndx = ((m_ndx & 0xFFFFFF00) | (nWrd & 0xFF));
	}
	inline bool isSet() const { return (m_ndx != 0); }
	inline void clear() { m_ndx = 0; }

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
	inline bool operator<=(const CRelIndex &ndx) const {
		return (index() <= ndx.index());
	}
	inline bool operator>(const CRelIndex &ndx) const {
		return (index() > ndx.index());
	}
	inline bool operator>=(const CRelIndex &ndx) const {
		return (index() >= ndx.index());
	}
	inline bool operator==(const CRelIndex &ndx) const {
		return (index() == ndx.index());
	}
	inline bool operator!=(const CRelIndex &ndx) const {
		return (index() != ndx.index());
	}

	static CRelIndex navigationIndexFromLogicalIndex(const CRelIndex &ndxLogical)
	{
		CRelIndex ndxVerse = ndxLogical;

		if (ndxVerse.isSet()) {
			if (((ndxVerse.chapter() == 0) || (ndxVerse.verse() == 0)) &&
				(ndxVerse.word() == 0)) {
				if (ndxVerse.chapter() == 0) ndxVerse.setChapter(1);
				ndxVerse.setVerse(1);
				ndxVerse.setWord(1);
			}
		}

		return ndxVerse;
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
Q_DECLARE_METATYPE(CRelIndex)
Q_DECLARE_METATYPE(uint32_t)

// ============================================================================

#ifdef USE_EXTENDED_INDEXES
class CRelIndexEx : public CRelIndex {
public:
	CRelIndexEx(const CRelIndexEx &ndx) = default;
	CRelIndexEx(const CRelIndex &ndx, uint32_t nLtr = 1)
		:	CRelIndex(ndx),
			m_ndxEx(nLtr)
	{
	}
	CRelIndexEx(uint64_t ndxEx = 0)
	{
		setIndexEx(ndxEx);
	}
	CRelIndexEx(const QString &strAnchor)
		:	CRelIndex(strAnchor),
			m_ndxEx(1)
	{
	}
	CRelIndexEx(uint32_t nBk, uint32_t nChp, uint32_t nVrs, uint32_t nWrd, uint32_t nLtr)
	{
		setIndexEx(nBk, nChp, nVrs, nWrd, nLtr);
	}
	~CRelIndexEx() { }

	static uint32_t maxLetterCount() { return 0xFFFFFFFF; }
	inline uint32_t letter() const { return m_ndxEx; }
	inline void setLetter(uint32_t nLtr) {
		m_ndxEx = nLtr;
	}

	inline bool isSet() const { return ((CRelIndex::isSet()) || (m_ndxEx != 0)); }
	inline void clear() { CRelIndex::clear(); m_ndxEx = 0; }

	inline uint64_t indexEx() const
	{
		return ((static_cast<uint64_t>(index()) << 32) | static_cast<uint64_t>(m_ndxEx));
	}
	inline void setIndexEx(uint32_t nBk, uint32_t nChp, uint32_t nVrs, uint32_t nWrd, uint32_t nLtr)
	{
		setIndex(nBk, nChp, nVrs, nWrd);
		m_ndxEx = nLtr;
	}
	inline void setIndexEx(uint64_t ndx)
	{
		setIndex(static_cast<uint32_t>(ndx >> 32));
		m_ndxEx = static_cast<uint32_t>(ndx & 0xFFFFFFFF);
	}

	inline CRelIndexEx & operator =(const CRelIndexEx &other) = default;

private:
	uint32_t m_ndxEx;				// Extended portion of index
};
#endif

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

	const TCountOf &ofBible() const { return m_nOfBible; }
	const TCountOf &ofTestament() const { return m_nOfTst; }
	const TCountOf &ofBook() const { return m_nOfBk; }
	const TCountOf &ofChapter() const { return m_nOfChp; }
	const TCountOf &ofVerse() const { return m_nOfVrs; }

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

struct RelativeIndexSortPredicate {
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

typedef std::vector<uint32_t> TNormalizedIndexList;			// Normalized Index List for words into book/chapter/verse/word
typedef std::vector<CRelIndex> TRelativeIndexList;			// Relative Index List for words into book/chapter/verse/word
typedef std::set<CRelIndex, RelativeIndexSortPredicate> TRelativeIndexSet;		// Relative Index Set for words into book/chapter/verse/word

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
#ifdef USE_EXTENDED_INDEXES
		,m_nNumLtr(0)
#endif
	{ }
	~CTestamentEntry() { }

	QString  m_strTstName;		// Name of testament (display name)
	unsigned int m_nNumBk;		// Number of Books in this testament
	unsigned int m_nNumChp;		// Number of Chapters in this testament
	unsigned int m_nNumVrs;		// Number of Verses in this testament
	unsigned int m_nNumWrd;		// Number of Words in this testament
#ifdef USE_EXTENDED_INDEXES
	uint32_t m_nNumLtr;		// Number of Letters in this testament
#endif
};

typedef std::vector<CTestamentEntry> TTestamentList;		// Index by nTst-1

// Bible -- Bible Entry (Derived from CTestamentEntry to keep stats for the whole Bible)
class CBibleEntry : public CTestamentEntry
{
public:
	CBibleEntry()
	:	CTestamentEntry(QObject::tr("Entire Bible", "Scope")),
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
		m_nWrdAccum(0),
#ifdef USE_EXTENDED_INDEXES
		m_nNumLtr(0),
		m_nLtrAccum(0),
#endif
#ifdef OSIS_PARSER_BUILD
		m_bCreated(false),
#endif
		m_bHaveColophon(false)
	{ }
	~CBookEntry() { }

	uint32_t m_nTstBkNdx;		// Testament Book Index (Index within the books of the testament) 1-39 or 1-27
	uint32_t m_nTstNdx;			// Testament Index (1=Old, 2=New, etc)
	QString m_strBkName;		// Name of book (display name)
	QStringList m_lstBkAbbr;	// Book Abbreviations (Always at LEAST two entries.  First entry is OSIS Abbreviation.  Second entry is Common Abbreviation used in all Abbreviated Book mode rendering.  Others are common abbreviations used for matching purposes)
	QString m_strTblName;		// Name of Table for this book
	unsigned int m_nNumChp;		// Number of chapters in this book
	unsigned int m_nNumVrs;		// Number of verses in this book
	unsigned int m_nNumWrd;		// Number of words in this book
	unsigned int m_nWrdAccum;	// Number of accumulated words prior to, but not including this book
#ifdef USE_EXTENDED_INDEXES
	uint32_t m_nNumLtr;			// Number of letters in this book
	uint32_t m_nLtrAccum;		// Number of accumulated letters prior to, but not including this book
#endif
	QString m_strDesc;			// Description (subtitle)

#ifdef OSIS_PARSER_BUILD
	bool m_bCreated;			// Set to true in KJVDataParse when this book entry has been created
#endif

	bool m_bHaveColophon;		// True if this book has a Colophon pseudo-verse (will be indexed as [nBk|0|0|0] in the TVerseEntryMap, ie. nChp==0, nVrs==0)
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
		m_nWrdAccum(0),
#ifdef USE_EXTENDED_INDEXES
		m_nNumLtr(0),
		m_nLtrAccum(0),
#endif
		m_bHaveSuperscription(false)
	{ }
	~CChapterEntry() { }

	unsigned int m_nNumVrs;		// Number of verses in this chapter
	unsigned int m_nNumWrd;		// Number of words in this chapter
	unsigned int m_nWrdAccum;	// Number of accumulated words prior to, but not including this chapter
#ifdef USE_EXTENDED_INDEXES
	uint32_t m_nNumLtr;			// Number of letters in this chapter
	uint32_t m_nLtrAccum;		// Number of accumulated letters prior to, but not including this chapter
#endif

	bool m_bHaveSuperscription;	// True if this chapter has a Superscription pseudo-verse (will be indexed as [nBk|nChp|0|0] in the TVerseEntryMap, ie. nVrs==0)
};

typedef std::map<CRelIndex, CChapterEntry, RelativeIndexSortPredicate> TChapterMap;	// Index by [nBk|nChp|0|0]

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
#ifdef USE_EXTENDED_INDEXES
		m_nNumLtr(0),
		m_nLtrAccum(0),
#endif
		m_nPilcrow(PTE_NONE)
	{ }
	~CVerseEntry() { }

	unsigned int m_nNumWrd;			// Number of words in this verse
	unsigned int m_nWrdAccum;		// Number of accumulated words prior to, but not including this verse
#ifdef USE_EXTENDED_INDEXES
	uint32_t m_nNumLtr;				// Number of letters in this verse
	uint32_t m_nLtrAccum;			// Number of accumulated letters prior to, but not including this verse
#endif
	PILCROW_TYPE_ENUM m_nPilcrow;	// Start of verse Pilcrow Flag (and Pilcrow type)
	QString m_strTemplate;			// Rich Text Creation Template

#ifdef OSIS_PARSER_BUILD
	QString m_strText;			// Rich text (or plain if Rich unavailable) for the verse (Note: for mobile versions, this element can be removed and fetched from the database if needed)
	QStringList m_lstWords;			// Word List for parse extraction
	QStringList m_lstRichWords;		// Word List as Rich Text for parse extraction
	QStringList m_lstParseStack;	// Parse operation stack (used to parse red-letter tags, added word tags, morphology, concordance references, etc.
#endif

};

typedef std::map<CRelIndex, CVerseEntry, RelativeIndexSortPredicate> TVerseEntryMap;		// Index by [nBk|nChp|nVrs|0]

typedef std::vector<TVerseEntryMap> TBookVerseList;		// Index by nBk-1

// ============================================================================

// Words -- Word List and Mapping
//
class CWordEntry
{
public:
	CWordEntry()
	:	m_bCasePreserve(false),
		m_bIsProperWord(false)
	{ }
	~CWordEntry() { }

	QString m_strWord;					// Word Text as in the Bible Database WORDS.csv file (base root for each unique word)
	bool m_bCasePreserve;				// Special Word Case Preserve
	bool m_bIsProperWord;				// Proper Words is set to True if a Word and all its Alternate Word Forms begin with a character in the Letter_Uppercase category (and isn't a special ordinary word, as determined in the KJVDataParse tool)
	QStringList m_lstAltWords;			// Complete exact word of the Bible Database, are related variations of the base root (such as hyphenated and non-hyphenated or capital vs lowercase), these are the exact words of the text
	QStringList m_lstSearchWords;		// Search word reduced from the complete list, for example without cantillation marks
	QStringList m_lstRawAltWords;				// Decomposed Words, without hyphens, without apostrophies, in lowercase -- used for ELS and places where letters-only need to be used
	QStringList m_lstDecomposedAltWords;		// Decomposed Words (used for matching), without hyphens
	QStringList m_lstDecomposedHyphenAltWords;	// Decomposed Words (used for matching), with hyphens
	QStringList m_lstDeApostrAltWords;			// Decomposed Words (used for matching), with apostrophies decomposed without hyphens
	QStringList m_lstDeApostrHyphenAltWords;	// Decomposed Words (used for matching), with apostrophies decomposed with hyphens
	QStringList m_lstRenderedAltWords;			// Alt Words as rendered (hyphen/non-hyphen based on proper/ordinary word rules of Bible Database Setting)
	QList<unsigned int> m_lstAltWordCount;		// Count for each alternate word.  This will be the number of entries for this word in the mapping below
	TNormalizedIndexList m_ndxNormalizedMapping;	// Normalized Indexes Mapping into entire Bible

#ifdef USE_GEMATRIA
	QList<CGematriaCalc> m_lstGematria;	// Per-word gematria calculations
#endif

	struct SortPredicate {
		bool operator() (const QString &s1, const QString &s2) const
		{
			return (s1.compare(s2, Qt::CaseSensitive) < 0);
		}
	};
};

typedef std::set<QString, CWordEntry::SortPredicate> TWordListSet;
typedef std::map<QString, CWordEntry, CWordEntry::SortPredicate> TWordListMap;		// Indexed by lowercase words from word-list

// ============================================================================

// CBasicWordEntry -- Word Entry Virtual base to handle word/decomposedWord/etc commonly for Bible Concordance and Dictionary
//

class CBasicWordEntry
{
public:
	virtual const QString &word() const = 0;
	virtual const QString &searchWord() const = 0;
	virtual const QString &rawWord() const = 0;
	virtual const QString &decomposedWord() const = 0;
	virtual const QString &decomposedHyphenWord() const = 0;
	virtual const QString &deApostrWord() const = 0;
	virtual const QString &deApostrHyphenWord() const = 0;
	virtual const QString &renderedWord() const = 0;
};

// ============================================================================

// Concordance -- Mapping of words and their Normalized positions:
//

enum WORD_TYPE_ENUM {
	WTE_COMPLETE = 0,		// Complete word from the Bible database
	WTE_SEARCH = 1,			// Word as used for search (i.e. without cantillation, etc.)
	WTE_RENDERED = 2,		// Word as rendered (i.e. without hyphen, when disabled, etc. -- applies dehyphen to remove hyphens based on settings)
	WTE_DECOMPOSED = 3,		// Decomposed word (no marks, no ligatures, etc.)
};

class CConcordanceEntry : public CBasicWordEntry
{
public:
	CConcordanceEntry(TWordListMap::const_iterator itrEntryWord, int nAltWordIndex, int nIndex = 0);
	CConcordanceEntry(const CConcordanceEntry &src) = default;
	virtual ~CConcordanceEntry() { }

	CConcordanceEntry & operator=(const CConcordanceEntry &src) = default;

	virtual const QString &word() const override { return m_itrEntryWord->second.m_lstAltWords.at(m_nAltWordIndex); }
	virtual const QString &searchWord() const override { return m_itrEntryWord->second.m_lstSearchWords.at(m_nAltWordIndex); }
	virtual const QString &rawWord() const override { return m_itrEntryWord->second.m_lstRawAltWords.at(m_nAltWordIndex); }
	virtual const QString &decomposedWord() const override { return m_itrEntryWord->second.m_lstDecomposedAltWords.at(m_nAltWordIndex); }
	virtual const QString &decomposedHyphenWord() const override { return m_itrEntryWord->second.m_lstDecomposedHyphenAltWords.at(m_nAltWordIndex); }
	virtual const QString &deApostrWord() const override { return m_itrEntryWord->second.m_lstDeApostrAltWords.at(m_nAltWordIndex); }
	virtual const QString &deApostrHyphenWord() const override { return m_itrEntryWord->second.m_lstDeApostrHyphenAltWords.at(m_nAltWordIndex); }
	virtual const QString &renderedWord() const override { return m_itrEntryWord->second.m_lstRenderedAltWords.at(m_nAltWordIndex); }
	inline bool isProperWord() const { return m_itrEntryWord->second.m_bIsProperWord; }
	inline int index() const { return m_nIndex; }

#ifdef USE_GEMATRIA
	inline const CGematriaCalc &gematria() const { return m_itrEntryWord->second.m_lstGematria.at(m_nAltWordIndex); }
#endif

#ifdef USE_EXTENDED_INDEXES
	uint32_t letterCount() const
	{
		return rawWord().size();
	}
	QChar letter(uint32_t nLtr) const		// one-originated nLtr lookup of letter (to follow pattern of CRelIndex)
	{
		// Note: This function returns the decomposed base-form for the letter,
		//	as generally needed by search/analysis consumers.
		if (nLtr == 0) return QChar();
		--nLtr;					// one-originated
		if (nLtr < static_cast<uint32_t>(rawWord().size())) return rawWord().at(nLtr);
		return QChar();
	}
#endif

	inline bool operator==(const CConcordanceEntry &src) const
	{
		return (word().compare(src.word()) == 0);
	}
	inline bool operator!=(const CConcordanceEntry &src) const
	{
		return (word().compare(src.word()) != 0);
	}

private:
	friend class CReadDatabaseEx;			// Extension used by CLI tools to override functionality of CReadDatabase

	TWordListMap::const_iterator m_itrEntryWord;	// Bible Word Entry from which this was derived (used to lookup details)
	int m_nAltWordIndex;					// Index of Composed Word in the reference CWordEntry (as in the actual text)
	int m_nIndex;							// Index used when sorting and keeping external reference intact (used only by ReadDB)
};

typedef QList<CConcordanceEntry> TConcordanceList;
typedef std::map<QString, QString> TSoundExMap;			// Mapping of Composed Word to SoundEx equivalent, used to minimize calculations

struct TConcordanceListSortPredicate {
	static bool ascendingLessThanWordCaseInsensitive(const CConcordanceEntry &s1, const CConcordanceEntry &s2)
	{
		int nDecompCompare = s1.decomposedWord().compare(s2.decomposedWord(), Qt::CaseInsensitive);
		if (nDecompCompare == 0) {
			return (s1.word().compare(s2.word(), Qt::CaseSensitive) < 0);				// Localized case-sensitive within overall case-insensitive
		}
		return (nDecompCompare < 0);
	}

	static bool ascendingLessThanWordCaseSensitive(const CConcordanceEntry &s1, const CConcordanceEntry &s2)
	{
		int nDecompCompare = s1.decomposedWord().compare(s2.decomposedWord(), Qt::CaseSensitive);
		if (nDecompCompare == 0) {
			return (s1.word().compare(s2.word(), Qt::CaseSensitive) < 0);
		}
		return (nDecompCompare < 0);
	}
};

// ============================================================================

// Footnotes -- Footnote List and Mapping
//		Note: This works consistently for book-only footnotes or colophons,
//		chapter footnotes or superscriptions, verse footnotes, and even word
//		footnotes (or translation lemmas) if we wish.  The index into the
//		map is the complete CRelIndex style.  For book-only, for example, the
//		chapter, verse, and word will be 0.  For a chapter note, the verse and
//		word will be 0, etc.  This allows us to easily and quickly query for the
//		type of note we need.
//

class CFootnoteEntry
{
public:
	CFootnoteEntry()
	{ }
	~CFootnoteEntry() { }

	QString htmlText(const CBibleDatabase *pBibleDatabase = nullptr) const;		// Formatted HTML to insert into Scripture Browser (Database is needed only if doing footnotes with embedded scripture or cross-refs, etc)
	QString plainText(const CBibleDatabase *pBibleDatabase = nullptr) const;		// Formatted PlainText rendering (Database is needed only if doing footnotes with embedded scripture or cross-refs, etc)

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

typedef std::map<CRelIndex, CFootnoteEntry, RelativeIndexSortPredicate> TFootnoteEntryMap;		// Index by [nBk|nChp|nVrs|nWrd]

// ============================================================================

// Phrases -- Common and Saved Search Phrase Lists:
//

class CParsedPhrase;		// Forward declaration

class CPhraseEntry
{
public:
	CPhraseEntry(const QString &strEncodedText = QString(), const QVariant &varExtraInfo = QVariant());
	CPhraseEntry(const CParsedPhrase &aPhrase);
	~CPhraseEntry();

	void clear();

	void setFromPhrase(const CParsedPhrase &aPhrase);

	inline const QString &text() const { return m_strPhrase; }
	QString textEncoded() const;
	void setText(const QString &strText);
	void setTextEncoded(const QString &strText);

	inline bool caseSensitive() const { return m_bCaseSensitive; }
	inline void setCaseSensitive(bool bCaseSensitive) { m_bCaseSensitive = bCaseSensitive; }

	inline bool accentSensitive() const { return m_bAccentSensitive; }
	inline void setAccentSensitive(bool bAccentSensitive) { m_bAccentSensitive = bAccentSensitive; }

	inline bool isExcluded() const { return m_bExclude; }
	inline void setExclude(bool bExclude) { m_bExclude = bExclude; }

	inline bool isDisabled() const { return m_bDisabled; }
	inline void setDisabled(bool bDisabled) { m_bDisabled = bDisabled; }

	inline QVariant extraInfo() const { return m_varExtraInfo; }
	inline void setExtraInfo(const QVariant &varExtraInfo) { m_varExtraInfo = varExtraInfo; }

	inline bool operator==(const CPhraseEntry &src) const
	{
		return ((m_bCaseSensitive == src.m_bCaseSensitive) &&
				(m_bAccentSensitive == src.m_bAccentSensitive) &&
				(m_bExclude == src.m_bExclude) &&
				// Don't compare m_bDisabled because that doesn't affect "equality"
				(m_strPhrase.compare(src.m_strPhrase, Qt::CaseSensitive) == 0));
	}
	inline bool operator!=(const CPhraseEntry &src) const
	{
		return (!(operator==(src)));
	}

	bool operator==(const CParsedPhrase &src) const;		// Implemented in PhraseParser.cpp, where CParsedPhrase is defined
	bool operator!=(const CParsedPhrase &src) const;		// Implemented in PhraseParser.cpp, where CParsedPhrase is defined

	static const QChar encCharCaseSensitive() { return QChar(0xA7); } 			// Section Sign = Case-Sensitive
	static const QChar encCharAccentSensitive() { return QChar(0xA4); }			// Current Sign = Accent-Sensitive
	static const QChar encCharExclude() { return QChar(0x2209); }				// Not an Element of = Exclude
	static const QChar encCharDisabled() { return QChar(0xAC); }				// Not Sign = Disable flag

	// From Qt 5.13.0 QtPrivate in qhashfunctions.h
	struct QHashCombineCommutative {
		// QHashCombine is a good hash combiner, but is not commutative,
		// ie. it depends on the order of the input elements. That is
		// usually what we want: {0,1,3} should hash differently than
		// {1,3,0}. Except when it isn't (e.g. for QSet and
		// QHash). Therefore, provide a commutative combiner, too.
#if QT_VERSION < 0x060000
		typedef uint result_type;
#else
		// Qt6 changes the signature of this from uint to size_t
		typedef size_t result_type;
#endif
		template <typename T>
#if QT_VERSION >= 0x050000
		Q_DECL_CONSTEXPR result_type operator()(result_type seed, const T &t) const Q_DECL_NOEXCEPT_EXPR(noexcept(qHash(t)))
#else
		// Backward compatibility for old VNC 4.8.7 build:
		constexpr result_type operator()(uint seed, const T &t) const noexcept(noexcept(qHash(t)))
#endif
		{ return seed + qHash(t); } // don't use xor!
	};

private:
	bool m_bCaseSensitive;
	bool m_bAccentSensitive;
	bool m_bExclude;
	bool m_bDisabled;
	QString m_strPhrase;
	QVariant m_varExtraInfo;	// Extra user info for specific uses of this structure
};

Q_DECLARE_METATYPE(CPhraseEntry)

class CPhraseList : public QList<CPhraseEntry>
{
public:
	inline CPhraseList() : QList<CPhraseEntry>() { }
	inline explicit CPhraseList(const CPhraseEntry &i) : QList<CPhraseEntry>() { append(i); }
	inline CPhraseList(const CPhraseList &l) : QList<CPhraseEntry>(l) { }
	inline CPhraseList &operator =(const CPhraseList &l) { QList<CPhraseEntry>::operator =(l); return *this; }

	int removeDuplicates();
};

#if QT_VERSION >= 0x050000
Q_DECL_CONST_FUNCTION inline CPhraseEntry::QHashCombineCommutative::result_type qHash(const CPhraseEntry &key, CPhraseEntry::QHashCombineCommutative::result_type seed = 0) Q_DECL_NOTHROW
#else
// Backward compatibility for old VNC 4.8.7 build:
__attribute__((const)) inline uint qHash(const CPhraseEntry &key, uint seed = 0) noexcept
#endif
{
	// Note: Aren't hashing "disable" because it doesn't affect the main key value equality
#if QT_VERSION >= 0x060000
	std::vector<CPhraseEntry::QHashCombineCommutative::result_type> vctHashes = { qHash(key.text()), qHash((key.caseSensitive() ? 4u : 0u) + (key.accentSensitive() ? 2u : 0u) + (key.isExcluded() ? 1u : 0u)) };
#else
	std::vector<uint> vctHashes = { qHash(key.text()), qHash((key.caseSensitive() ? 4u : 0u) + (key.accentSensitive() ? 2u : 0u) + (key.isExcluded() ? 1u : 0u)) };
#endif
	return std::accumulate(vctHashes.begin(), vctHashes.end(), seed, CPhraseEntry::QHashCombineCommutative());
}

// ============================================================================

// Forward declarations:
class TPhraseTag;
class TPhraseTagList;
class TPassageTag;
class TPassageTagList;

// Class to hold the Normalized Lo and Hi indexes covered by a tag
//		with basic manipulations:
class TTagBoundsPair
{
public:
	TTagBoundsPair(uint32_t nNormalLo, uint32_t nNormalHi, bool bHadCount = true);
	TTagBoundsPair(const TTagBoundsPair &tbpSrc);
	TTagBoundsPair(const TPhraseTag &aTag, const CBibleDatabase *pBibleDatabase);

	TTagBoundsPair & operator=(const TTagBoundsPair &src)
	{
		m_pairNormals = src.m_pairNormals;
		m_bHadCount = src.m_bHadCount;
		return *this;
	}

	bool isValid() const { return ((m_pairNormals.first != 0) && (m_pairNormals.second != 0)); }

	inline uint32_t lo() const { return m_pairNormals.first; }
	void setLo(uint32_t nNormal) { m_pairNormals.first = nNormal; }
	inline uint32_t hi() const { return m_pairNormals.second; }
	void setHi(uint32_t nNormal) { m_pairNormals.second = nNormal; }
	inline bool hadCount() const { return m_bHadCount; }
	void setHadCount(bool bHadCount) { m_bHadCount = bHadCount; }

	bool completelyContains(const TTagBoundsPair &tbpSrc) const;
	bool intersects(const TTagBoundsPair &tbpSrc) const;
	bool intersectingInsert(const TTagBoundsPair &tbpSrc);
	bool intersectingTrim(const TTagBoundsPair &tbpSrc);

private:
	typedef QPair<uint32_t, uint32_t> TNormalPair;
	TNormalPair m_pairNormals;
	bool m_bHadCount;							// True if the range had a count of words rather than being a reference to a location without any content
};

// ============================================================================

// Relative Index and Word Count pair used for highlighting phrases:
class TPhraseTag
{
public:
	explicit inline TPhraseTag(const CRelIndex &ndx = CRelIndex(), unsigned int nCount = 0)
		:	m_RelIndex(ndx),
			m_nCount(nCount)
	{ }

	TPhraseTag(const CBibleDatabase *pBibleDatabase, const TTagBoundsPair &tbpSrc);

	TPhraseTag(const CBibleDatabase *pBibleDatabase, const TPassageTag &tagPassage)
		:	m_nCount(0)
	{
		setFromPassageTag(pBibleDatabase, tagPassage);
	}

	inline const CRelIndex &relIndex() const { return m_RelIndex; }
	inline CRelIndex &relIndex() { return m_RelIndex; }						// Needed for >> operator
	inline void setRelIndex(const CRelIndex &ndx) { m_RelIndex = ndx; }
	inline const unsigned int &count() const { return m_nCount; }
	inline unsigned int &count() { return m_nCount; }						// Needed for >> operator
	inline void setCount(unsigned int nCount) { m_nCount = nCount; }

	void setFromPassageTag(const CBibleDatabase *pBibleDatabase, const TPassageTag &tagPassage);
	QString PassageReferenceRangeText(const CBibleDatabase *pBibleDatabase) const;

	bool isSet() const {
		return (m_RelIndex.isSet());
	}

	bool haveSelection() const {
		return ((m_RelIndex.isSet()) && (m_nCount != 0));
	}

	bool operator==(const TPhraseTag &otherTag) const {
		return ((m_RelIndex.index() == otherTag.relIndex().index()) &&
				(m_nCount == otherTag.count()));
	}

	bool operator!=(const TPhraseTag &otherTag) const {
		return ((m_RelIndex.index() != otherTag.relIndex().index()) ||
				(m_nCount != otherTag.count()));
	}

	TTagBoundsPair bounds(const CBibleDatabase *pBibleDatabase) const;			// Returns a pair containing the Normalized Lo and Hi indexes covered by this tag

	bool completelyContains(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;
	bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;
	bool intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag);
	TPhraseTag mask(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;		// Creates a new tag that's the content of this tag masked by the specified aTag.
	friend class TPhraseTagList;

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
const QString g_constrHighlighterPhraseTagListMimeType("application/vnd.dewtronics.kjvcanopener.highlighter.phrasetaglist");
const QString g_constrPlainTextMimeType("text/plain;charset=utf-8");
const QString g_constrHTMLTextMimeType("text/html;charset=utf-8");

// ----------------------------------------------------------------------------

// List of tags used for highlighting found phrases, etc:
class TPhraseTagList : public QList<TPhraseTag>
{
public:
	TPhraseTagList();
	TPhraseTagList(const TPhraseTag &aTag);
	TPhraseTagList(const TPhraseTagList &src) = default;
	TPhraseTagList(const CBibleDatabase *pBibleDatabase, const TPassageTagList &lstPassageTags);
	TPhraseTagList& operator=(const TPhraseTagList &src) = default;

	bool isSet() const;

	void setFromPassageTagList(const CBibleDatabase *pBibleDatabase, const TPassageTagList &lstPassageTags);

	bool completelyContains(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;
	bool completelyContains(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &aTagList) const;
	bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;
	void intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag);
	void intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &aTagList);		// Note: Both lists MUST be sorted before calling this function!  The resulting list will be sorted...
	bool removeIntersection(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag);
	int findIntersectingIndex(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag, int nStartIndex = 0) const;

	bool isEquivalent(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &aTagList) const
	{
		return (completelyContains(pBibleDatabase, aTagList) && aTagList.completelyContains(pBibleDatabase, *this));
	}
};

typedef QList<TPhraseTagList> TPhraseTagListList;		// List of tag lists, use to keep tag lists for multiple phrases

struct TPhraseTagListSortPredicate {
	static bool ascendingLessThan(const TPhraseTag &s1, const TPhraseTag &s2)
	{
		return (s1.relIndex().index() < s2.relIndex().index());
	}
};

// ============================================================================

// Relative Index and VERSE Count pair used for highlighting Passages:

class TPassageTag
{
public:
	explicit inline TPassageTag(const CRelIndex &ndx = CRelIndex(), unsigned int nVerseCount = 0)
		:	m_RelIndex(ndx),
			m_nVerseCount(nVerseCount)
	{
		if (m_RelIndex.isSet()) m_RelIndex.setWord(1);
	}
	TPassageTag(const CBibleDatabase *pBibleDatabase, const TPhraseTag &tagPhrase)
		:	m_nVerseCount(0)
	{
		setFromPhraseTag(pBibleDatabase, tagPhrase);
	}

	inline const CRelIndex &relIndex() const { return m_RelIndex; }
	inline CRelIndex &relIndex() { return m_RelIndex; }							// Needed for >> operator
	inline void setRelIndex(const CRelIndex &ndx) { m_RelIndex = ndx; }
	inline const unsigned int &verseCount() const { return m_nVerseCount; }
	inline unsigned int &verseCount() { return m_nVerseCount; }					// Needed for >> operator
	inline void setVerseCount(unsigned int nVerseCount) { m_nVerseCount = nVerseCount; }

	void setFromPhraseTag(const CBibleDatabase *pBibleDatabase, const TPhraseTag &tagPhrase);
	QString PassageReferenceRangeText(const CBibleDatabase *pBibleDatabase) const;

	bool isSet() const {
		return (m_RelIndex.isSet());
	}

	bool haveSelection() const {
		return ((m_RelIndex.isSet()) && (m_nVerseCount != 0));
	}

	bool operator==(const TPassageTag &otherTag) const {
		return ((m_RelIndex.index() == otherTag.relIndex().index()) &&
				(m_nVerseCount == otherTag.verseCount()));
	}

	bool operator!=(const TPassageTag &otherTag) const {
		return ((m_RelIndex.index() != otherTag.relIndex().index()) ||
				(m_nVerseCount != otherTag.verseCount()));
	}

//	bool completelyContains(const CBibleDatabase *pBibleDatabase, const TPassageTag &aTag) const;
//	bool intersects(const CBibleDatabase *pBibleDatabase, const TPassageTag &aTag) const;
//	bool intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPassageTag &aTag);
	friend class TPassageTagList;

private:
	CRelIndex m_RelIndex;
	unsigned int m_nVerseCount;
};
inline QDataStream& operator<<(QDataStream &out, const TPassageTag &ndx) {
	out << ndx.relIndex() << ndx.verseCount();
	return out;
}
inline QDataStream& operator>>(QDataStream &in, TPassageTag &ndx) {
	in >> ndx.relIndex() >> ndx.verseCount();
	return in;
}
Q_DECLARE_METATYPE(TPassageTag)

const QString g_constrPassageTagMimeType("application/vnd.dewtronics.kjvcanopener.passagetag");

// List of tags used for highlighting found phrases, etc:
class TPassageTagList : public QList<TPassageTag>
{
public:
	TPassageTagList()
		:	QList<TPassageTag>()
	{ }

	TPassageTagList(const TPassageTag &aTag)
		:	QList<TPassageTag>()
	{
		append(aTag);
	}

	TPassageTagList(const TPassageTagList &src)
		:	QList<TPassageTag>(src)
	{ }

	TPassageTagList(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &lstPhraseTags)
		:	QList<TPassageTag>()
	{
		setFromPhraseTagList(pBibleDatabase, lstPhraseTags);
	}

	void setFromPhraseTagList(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &lstPhraseTags);
	unsigned int verseCount() const;

//	bool completelyContains(const CBibleDatabase *pBibleDatabase, const TPassageTag &aTag) const;
//	void intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPassageTag &aTag);
//	bool removeIntersection(const CBibleDatabase *pBibleDatabase, const TPassageTag &aTag);
};

struct TPassageTagListSortPredicate {
	static bool ascendingLessThan(const TPassageTag &s1, const TPassageTag &s2)
	{
		return (s1.relIndex().index() < s2.relIndex().index());
	}
};

// ============================================================================

enum MORPH_SOURCE_ENUM {		// Enum of source data tags
	MSE_NONE = 0,				// None or Unknown
	MSE_OSHM = 1,				// Open Scriptures Hebrew Morphology
	MSE_THAYERS = 2,			// Thayer's (Hebrew/Greek)
	MSE_ROBINSON = 3,			// Robinson (Greek)
	MSE_PACKARD = 4,			// Packard (Greek)
	// ----
	MSE_COUNT
};

struct TMorphTag
{
	MORPH_SOURCE_ENUM m_nSource = MSE_NONE;		// Source Database Type
	QString m_strEntryKey;						// Entry Key in Database
};

typedef QList<TMorphTag> TMorphTagList;		// List of Morph Tags in CLemmaEntry

class CLemmaEntry
{
public:
	CLemmaEntry() { }
	CLemmaEntry(const TPhraseTag &tag, const QString &strLemmaAttrs);
	~CLemmaEntry() { }

	inline TPhraseTag tag() const { return m_tagEntry; }
#ifdef OSIS_PARSER_BUILD
	inline QString lemmaAttrs() const { return m_strLemmaAttrs; }
#endif

	inline int count() const { return m_lstStrongs.size(); }
	inline bool isValid() const
	{
		return ((m_lstStrongs.size() == m_lstText.size()) &&
				(m_lstStrongs.size() == m_lstMorphology.size()) &&
				!m_lstStrongs.isEmpty());
	}

	const QStringList &strongs() const { return m_lstStrongs; }
	const QStringList &text() const { return m_lstText; }
	const TMorphTagList &morph() const { return m_lstMorphology; }

private:
	TPhraseTag m_tagEntry;			// Phrase of words corresponding to this entry
#ifdef OSIS_PARSER_BUILD
	QString m_strLemmaAttrs;		// Lemma Attributes from OSIS.  These will be parsed into the data below, but this member is only needed during OSIS parsing.
#endif
	// ----
	QStringList m_lstStrongs;		// Array of Strongs Indexes -- count of this is the Lemma count
	QStringList m_lstText;			// Masoretic or Textus-Receptus Words (paired with Strongs Indexes) -- count of this is the Lemma count
	TMorphTagList m_lstMorphology;	// Array of morph codes for this phrase -- count of this is the Lemma count
};

typedef std::map<CRelIndex, CLemmaEntry, RelativeIndexSortPredicate> TLemmaEntryMap;	// Index by [nBk|nChp|nVrs|nWrd]

// ============================================================================

class CStrongsEntry
{
public:
	CStrongsEntry() { }
	CStrongsEntry(QChar chrLangCode, unsigned int nStrongsIndex)
		:	m_chrLangCode(chrLangCode.toUpper()),
			m_nStrongsIndex(nStrongsIndex)
	{ }
	CStrongsEntry(const QString &strStrongsTextIndex)
		:	m_chrLangCode(!strStrongsTextIndex.isEmpty() ? strStrongsTextIndex.at(0).toUpper() : '?'),
			m_nStrongsIndex(strStrongsTextIndex.mid(1).toUInt())
	{ }

	QChar langCode() const { return m_chrLangCode; }
	unsigned int strongsIndex() const { return m_nStrongsIndex; }
	QString strongsMapIndex() const { return QString("%1%2").arg(m_chrLangCode).arg(m_nStrongsIndex); }
	QString strongsTextIndex() const { return QString("%1%2").arg(m_chrLangCode).arg(m_nStrongsIndex, 4, 10, QChar('0')); }

	QString orthography() const { return m_strOrthography; }
#if QT_VERSION >= 0x050000
	QString orthographyPlainText() const { QString strResult = m_strOrthography; return strResult.remove(QRegularExpression("<[^>]*>")); }
#else
	QString orthographyPlainText() const { QString strResult = m_strOrthography; return strResult.remove(QRegExp("<[^>]*>")); }
#endif
	void setOrthography(const QString &strOrthography) { m_strOrthography = strOrthography; }

	QString transliteration() const { return m_strTransliteration; }
	void setTransliteration(const QString &strTransliteration) { m_strTransliteration = strTransliteration; }

	QString pronunciation() const { return m_strPronunciation; }
	void setPronunciation(const QString &strPronunciation) { m_strPronunciation = strPronunciation; }

	QString definition() const { return m_strDefinition; }
	void setDefinition(const QString &strDefinition) { m_strDefinition = strDefinition; }

private:
	QChar m_chrLangCode;						// Language code: 'G' or 'H'
	unsigned int m_nStrongsIndex = 0;			// Numeric Index
	QString m_strOrthography;					// Word(s) in original language
	QString m_strTransliteration;				// English Transliteration of Word(s)
	QString m_strPronunciation;					// Entry Word(s) Pronunciation.  Note: Will be RichText in some cases as some Hebrew entries have a superscripted 'o' for some reason
	QString m_strDefinition;					// Entry Definition as Rich Text
};

struct StrongsIndexSortPredicate {
	bool operator() (const QString &v1, const QString &v2) const
	{
		if (v1.left(1) < v2.left(1)) return true;
		if (v1.left(1) == v2.left(1)) return (v1.mid(1).toUInt() < v2.mid(1).toUInt());
		return false;
	}
};

typedef std::map<QString, CStrongsEntry, StrongsIndexSortPredicate> TStrongsIndexMap;		// Mapping of StrongsMapIndex to StrongsEntry
typedef QMultiMap<QString, QString> TStrongsOrthographyMap;		// Mapping of Orthography word(s) to StrongsMapIndex -- NOTE: This is a MultiMap, as multiple Strongs Indexes can be mapped to one orthography

// ============================================================================

class CMorphEntry
{
public:
	CMorphEntry() { }
	CMorphEntry(const QString &strKey, const QString &strDescription = QString())
		:	m_strKey(strKey),
			m_strDescription(strDescription)
	{ }
	~CMorphEntry() { }

	const QString &key() const { return m_strKey; }
	const QString &description() const { return m_strDescription; }
	void setDescription(const QString &strDescription) { m_strDescription = strDescription; }

private:
	QString m_strKey;				// Plain text key value from database (mixed case)
	QString m_strDescription;		// HTML rendered description to display
};

typedef std::map<QString, CMorphEntry> TMorphEntryMap;					// Map of Morph Uppercase Key values to Entry
typedef std::map<MORPH_SOURCE_ENUM, TMorphEntryMap> TMorphDatabaseMap;	// Map of Morph Database type to MorphEntryMap

// ============================================================================

class TBibleDatabaseSettings
{
public:
	// NOTE: This enum is needed in order to keep
	//	backward compatibility with existing
	//	Persistent Bible Database Settings:
	enum HideHyphensOptions {					// <<Bitfields>>
		HHO_None = 0x0,							// Default for no options (i.e. don't hide anything)
		HHO_ProperWords = 0x1,					// Hide Hyphens in "Proper Words"
		HHO_OrdinaryWords = 0x2					// Hide Hyphens in non-"Proper Words"
	};
	Q_DECLARE_FLAGS(HideHyphensOptionFlags, HideHyphensOptions)

	enum BibleDatabaseOptions {					// <<Bitfields>>
		BDO_None = 0x00,						// Default for no options (i.e. don't hide anything)
		// ----
		BDO_LoadOnStart = 0x01,					// Load Database on application start
		// ----
		BDO_HideHyphens_ProperWords = 0x02,		// Hide Hyphens in "Proper Words"	-- WARNING: Changing these will require changing the hideHyphens and setHideHyphens functions!
		BDO_HideHyphens_OrdinaryWords = 0x04,	// Hide Hyphens in non-"Proper Words"
		BDO_HideHyphens_Flags = 0x06,			// All Flags related to Hiding Hyphens
		// ----
		BDO_HyphenSensitiveSearch = 0x08,		// True to make word searches hyphen sensitive
		// ----
		BDO_HideCantillationMarks = 0x10,		// Hide Cantillations markings (Hebrew OSHB and similar texts)
		// ----
	};
	Q_DECLARE_FLAGS(BibleDatabaseOptionFlags, BibleDatabaseOptions)

	explicit TBibleDatabaseSettings()
		:	m_flagsOptions(BDO_None),
			m_nVersification(BVTE_KJV),
			m_nCategoryGroup(BBCGE_KJV)
	{ }

	bool isValid() const { return true; }

	inline bool operator==(const TBibleDatabaseSettings &other) const {
		return ((m_flagsOptions == other.m_flagsOptions) &&
				(m_nCategoryGroup == other.m_nCategoryGroup) &&
				(m_nVersification == other.m_nVersification));
	}
	inline bool operator!=(const TBibleDatabaseSettings &other) const {
		return (!operator==(other));
	}

	bool loadOnStart() const { return (m_flagsOptions & BDO_LoadOnStart); }
	void setLoadOnStart(bool bLoadOnStart)
	{
		m_flagsOptions &= ~BDO_LoadOnStart;
		if (bLoadOnStart) m_flagsOptions |= BDO_LoadOnStart;
	}

	HideHyphensOptionFlags hideHyphens() const
	{
		// Note: Use static_cast in these functions instead of fromInt()
		//	for compatibility with Qt4/Qt5.
		return static_cast<HideHyphensOptionFlags>((m_flagsOptions & BDO_HideHyphens_Flags) >> 1);
	}
	void setHideHyphens(HideHyphensOptionFlags nHHO)
	{
		// Note: Use static_cast in these functions instead of fromInt()
		//	for compatibility with Qt4/Qt5.
		BibleDatabaseOptionFlags nBDO = static_cast<BibleDatabaseOptionFlags>(nHHO << 1);
		Q_ASSERT((nBDO & ~BDO_HideHyphens_Flags) == 0);		// Make sure nothing is passing non-hyphen flags
		m_flagsOptions &= ~BDO_HideHyphens_Flags;
		m_flagsOptions |= (nBDO & BDO_HideHyphens_Flags);
	}

	bool hyphenSensitive() const { return (m_flagsOptions & BDO_HyphenSensitiveSearch); }
	void setHyphenSensitive(bool bHyphenSensitive)
	{
		m_flagsOptions &= ~BDO_HyphenSensitiveSearch;
		if (bHyphenSensitive) m_flagsOptions |= BDO_HyphenSensitiveSearch;
	}

	bool hideCantillationMarks() const { return (m_flagsOptions & BDO_HideCantillationMarks); }
	void setHideCantillationMarks(bool bHideCantillationMarks)
	{
		m_flagsOptions &= ~BDO_HideCantillationMarks;
		if (bHideCantillationMarks) m_flagsOptions |= BDO_HideCantillationMarks;
	}

	BIBLE_VERSIFICATION_TYPE_ENUM versification() const { return m_nVersification; }
	void setVersification(BIBLE_VERSIFICATION_TYPE_ENUM nVersification) { m_nVersification = nVersification; }

	BIBLE_BOOK_CATEGORY_GROUP_ENUM categoryGroup() const { return m_nCategoryGroup; }
	void setCategoryGroup(BIBLE_BOOK_CATEGORY_GROUP_ENUM nCategoryGroup) { m_nCategoryGroup = nCategoryGroup; }

private:
	BibleDatabaseOptionFlags m_flagsOptions;
	BIBLE_VERSIFICATION_TYPE_ENUM m_nVersification;
	BIBLE_BOOK_CATEGORY_GROUP_ENUM m_nCategoryGroup;
};

typedef QMap<QString, TBibleDatabaseSettings> TBibleDatabaseSettingsMap;		// Map of Bible UUIDs to settings for saving/preserving

// ============================================================================

class CReadDatabase;			// Forward declaration for class friendship
class COSISXmlHandler;

class CVerseTextRichifierTags;
enum RichifierRenderOptions : uint32_t;
typedef QFlags<RichifierRenderOptions> RichifierRenderOptionFlags;
class CKJPBSWordScriptureObject;
class QAbstractTextDocumentLayout;

// CBibleDatabase - Class to define a Bible Database file
class CBibleDatabase
{
private:
	CBibleDatabase(const TBibleDescriptor &bblDesc);		// Creatable by CReadDatabase
public:
	~CBibleDatabase();

	TBibleDatabaseSettings settings() const;
	void setSettings(const TBibleDatabaseSettings &aSettings);

	BibleTypeOptionsFlags flags() const { return m_descriptor.m_btoFlags; }
	QString language() const { return m_descriptor.m_strLanguage; }
	LANGUAGE_ID_ENUM langID() const { return toLanguageID(m_descriptor.m_strLanguage); }
	Qt::LayoutDirection direction() const { return m_descriptor.m_nTextDir; }
	QString name() const { return m_descriptor.m_strDBName; }
	QString description() const { return m_descriptor.m_strDBDesc; }
	QString compatibilityUUID() const { return m_descriptor.m_strUUID; }
	QString highlighterUUID() const { return m_descriptor.m_strHighlighterUUID; }
	const TBibleDescriptor &descriptor() const { return m_descriptor; }

	QString info() const { return m_strInfo; }

	QString translatedColophonString() const;				// Text "Colophon"
	QString translatedSuperscriptionString() const;			// Text "Superscription"

	bool hasColophons() const;								// Returns true if any book has colophons in this Bible
	bool hasSuperscriptions() const;						// Returns true if any chapter has superscriptions in this Bible

	bool completelyContains(const TPhraseTag &aPhraseTag) const;		// Returns true if this Bible database completely contains the specified tag (i.e. none of it lies outside the database text)
	TTagBoundsPair bounds() const;

	TPhraseTag bookPhraseTag(const CRelIndex &nRelIndex) const;
	TPhraseTag chapterPhraseTag(const CRelIndex &nRelIndex) const;
	TPhraseTag versePhraseTag(const CRelIndex &nRelIndex) const;

	void registerTextLayoutHandlers(QAbstractTextDocumentLayout *pDocLayout);

#ifdef USING_WEBCHANNEL
	QString toJsonBkChpStruct() const;		// Generate Book/Chapter Structure as JSON for WebChannel
#endif

	// CRelIndex Name/Report Functions:
	QString SearchResultToolTip(const CRelIndex &nRelIndex, unsigned int nRIMask = RIMASK_ALL, unsigned int nSelectionSize = 1) const;		// Create complete reference statistics report
	QString PassageReferenceText(const CRelIndex &nRelIndex, bool bSuppressWordOnPseudoVerse = false) const;		// Creates a reference text string like "Genesis 1:1 [5]"
	QString PassageReferenceAbbrText(const CRelIndex &nRelIndex, bool bSuppressWordOnPseudoVerse = false) const;	// Creates a reference abbreviated text string like "Gen 1:1 [5]"
#ifdef USE_EXTENDED_INDEXES
	QString PassageReferenceText(const CRelIndexEx &nRelIndex, bool bSuppressWordOnPseudoVerse = false) const;		// Creates a reference text string like "Genesis 1:1 [5]"
	QString PassageReferenceAbbrText(const CRelIndexEx &nRelIndex, bool bSuppressWordOnPseudoVerse = false) const;	// Creates a reference abbreviated text string like "Gen 1:1 [5]"
#endif

	QString testamentName(const CRelIndex &nRelIndex) const;
	uint32_t testament(const CRelIndex &nRelIndex) const;

	QString bookCategoryName(const CRelIndex &nRelIndex) const;
	BIBLE_BOOK_CATEGORIES_ENUM bookCategory(const CRelIndex &nRelIndex) const;

	QString bookName(const CRelIndex &nRelIndex) const;
	QString bookNameAbbr(const CRelIndex &nRelIndex) const;
	QString bookOSISAbbr(const CRelIndex &nRelIndex) const;

	CRelIndex bookIndexFromOSISAbbr(const QString &strOSISAbbr) const;

	// CRelIndex Transformation Functions:
#ifdef OSIS_PARSER_BUILD
	inline uint32_t NormalizeIndexNoAccum(const CRelIndex &ndxRelIndex) const
	{
		return m_itrCurrentLayout->NormalizeIndexNoAccum(ndxRelIndex);
	}
	inline CRelIndex DenormalizeIndexNoAccum(uint32_t nNormalIndex) const
	{
		return m_itrCurrentLayout->DenormalizeIndexNoAccum(nNormalIndex);
	}
#endif
	inline uint32_t NormalizeIndex(const CRelIndex &ndxRelIndex) const
	{
		return m_itrCurrentLayout->NormalizeIndex(ndxRelIndex);
	}
	inline CRelIndex DenormalizeIndex(uint32_t nNormalIndex) const
	{
		return m_itrCurrentLayout->DenormalizeIndex(nNormalIndex);
	}
#ifdef USE_EXTENDED_INDEXES
	uint32_t NormalizeIndexEx(const CRelIndexEx &ndxRelIndexEx) const
	{
		return m_itrCurrentLayout->NormalizeIndexEx(ndxRelIndexEx);
	}
	CRelIndexEx DenormalizeIndexEx(uint32_t nNormalIndexEx) const
	{
		return m_itrCurrentLayout->DenormalizeIndexEx(nNormalIndexEx);
	}
#endif

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
					const CRelIndex &ndxStart = CRelIndex(),
					bool bReverse = false) const;

	// Note: Changing the following will require updating and redeploying
	//		of the WebChannel pages, in addition to CBrowserWidget, etc:
	enum RELATIVE_INDEX_MOVE_ENUM {
		RIME_Absolute = 0,				// Move to Absolute Index or NoMove (default entry for doing no relative calculation except for checking validity and renormalizing to next valid location)
		RIME_Start = 1,					// Move to Beginning of the Bible
		RIME_StartOfBook = 2,			// Move to Beginning of current Book
		RIME_StartOfChapter = 3,		// Move to Beginning of current Chapter
		RIME_StartOfVerse = 4,			// Move to Beginning of current Verse
		RIME_End = 5,					// Move to Ending of the Bible
		RIME_EndOfBook = 6,				// Move to Ending of current Book
		RIME_EndOfChapter = 7,			// Move to Ending of current Chapter
		RIME_EndOfVerse = 8,			// Move to Ending of current Verse
		RIME_PreviousBook = 9,			// Move Backward one Book (to the start of it)
		RIME_PreviousChapter = 10,		// Move Backward one Chapter (to the start of it)
		RIME_PreviousVerse = 11,		// Move Backward one Verse (to the start of it)
		RIME_PreviousWord = 12,			// Move Backward one Word
		RIME_NextBook = 13,				// Move Forward one Book (to the start of it)
		RIME_NextChapter = 14,			// Move Forward one Chapter (to the start of it)
		RIME_NextVerse = 15,			// Move Forward one Verse (to the start of it)
		RIME_NextWord = 16				// Move Forward one Word
	};
	CRelIndex calcRelIndex(const CRelIndex &ndxStart, RELATIVE_INDEX_MOVE_ENUM nMoveMode) const;	// Calculates new index as per MoveMode.  Returns 0 (Not Set) if result is invalid, such as PreviousBook from Genesis 1, for example.

	unsigned int bookWordCountProper(unsigned int nBook) const;
	unsigned int chapterWordCountProper(unsigned int nBook, unsigned int nChapter) const;

	inline bool hasVersificationType(BIBLE_VERSIFICATION_TYPE_ENUM nVersification) const
	{
		return m_mapVersificationLayouts.contains(nVersification);
	}
	bool setVersificationType();
	inline bool isVersificationRemapped() const { return (m_itrCurrentLayout != m_itrMainLayout); }
	inline BIBLE_VERSIFICATION_TYPE_ENUM versification() const { return m_itrCurrentLayout.key(); }

	inline const CBibleEntry &bibleEntry() const						// Bible stats entry
	{
		return m_itrCurrentLayout->m_EntireBible;
	}
	const CTestamentEntry *testamentEntry(uint32_t nTst) const;			// Testament stats/data entry
	const CBookEntry *bookEntry(uint32_t nBk) const;					// Book Data or Table of Contents [Book]
	const CBookEntry *bookEntry(const CRelIndex &ndx) const;			// Book Data or Table of Contents Use CRelIndex:[Book | 0 | 0 | 0]
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
	inline bool searchSpaceIsCompleteConcordance() const { return m_bSearchSpaceIsCompleteConcordance; }
	void setRenderedWords();
	void setRenderedWords(CWordEntry &aWordEntry) const;
	int concordanceIndexForWordAtIndex(uint32_t ndxNormal) const;			// Returns the concordanceWordList() index for the Word at the specified Bible Normalized Index (or -1 if not found)
	int concordanceIndexForWordAtIndex(const CRelIndex &relIndex) const;	// Returns the concordanceWordList() index for the Word at the specified Bible Normalized Index (or -1 if not found)
	const CConcordanceEntry *concordanceEntryForWordAtIndex(uint32_t ndxNormal) const;			// Returns the CConcordanceEntry object for the word at the specified index -- like concordanceIndexForWordAtIndex, but returns underlying CConcordanceEntry
	const CConcordanceEntry *concordanceEntryForWordAtIndex(const CRelIndex &relIndex) const;	// Returns the CConcordanceEntry object for the word at the specified index -- like concordanceIndexForWordAtIndex, but returns underlying CConcordanceEntry
	QString wordAtIndex(uint32_t ndxNormal, WORD_TYPE_ENUM nWordType) const;			// Returns word of the Bible based on Normalized Index (1 to Max) -- Automatically does ConcordanceMapping Lookups -- If bAsRendered=true, applies dehyphen to remove hyphens based on settings
	QString wordAtIndex(const CRelIndex &relIndex, WORD_TYPE_ENUM nWordType) const;		// Returns word of the Bible based on Relative Index (Denormalizes and calls wordAtIndex() for normal above) -- If bAsRendered=true, applies dehyphen to remove hyphens based on settings
#ifdef USE_EXTENDED_INDEXES
	QChar letterAtIndex(uint32_t ndxNormalEx) const;
#endif
	const CFootnoteEntry *footnoteEntry(const CRelIndex &ndx) const;	// Footnote Data Entry, Used CRelIndex:[Book | Chapter | Verse | Word], for unused, set to 0, example: [1 | 1 | 0 | 0] for Genesis 1 (See TFootnoteEntryMap above)
	bool haveFootnotes() const
	{
		return !m_mapFootnotes.empty();
	}
#ifdef OSIS_PARSER_BUILD
	// Note: Footnotes are indexed via primary KJV versification and not
	//	alternate versification.  Therefore, keep the map private (only
	//	accessible by CBibleDatabase and OSIS_PARSER) (i.e. KJVDataParse)
	//	to help prevent accidentally using the content incorrectly:
	inline const TFootnoteEntryMap &footnotesMap() const				// Entire Footnote Map, needed for database generation
	{
		return m_mapFootnotes;
	}
#endif
	inline const CPhraseList &phraseList() const						// Returns the Common Phrases List from the Main Database for this Bible Database
	{
		return m_lstCommonPhrases;
	}
	const CLemmaEntry *lemmaEntry(const CRelIndex &ndx) const;			// Lemma Data Entry, Used CRelIndex: [Book | Chapter | Verse | Word], supports Colophons and Superscription indexes
	bool haveLemmas() const
	{
		return !m_mapLemmaEntries.empty();
	}
#ifdef OSIS_PARSER_BUILD
	// Note: Lemmas are indexed via primary KJV versification and not
	//	alternate versification.  Therefore, keep the map private (only
	//	accessible by CBibleDatabase and OSIS_PARSER) (i.e. KJVDataParse)
	//	to help prevent accidentally using the content incorrectly:
	inline const TLemmaEntryMap &lemmaMap() const						// Entire Lemma Map, needed for database generation
	{
		return m_mapLemmaEntries;
	}
#endif
	const CStrongsEntry *strongsEntryByIndex(const QString &strIndex) const;		// Strongs Entry by Map Index (G1, H22, etc);
	QList<const CStrongsEntry *> strongsEntriesByOthography(const QString &strOrth) const;	// Strongs Entries from Orthographic Word
	QStringList strongsIndexesFromOrthograph(const QString &strOrth) const;		// Lookup Orthographic Word and return a List of Strongs Map Indexes
	inline const TStrongsIndexMap &strongsIndexMap() const
	{
		return m_mapStrongsEntries;
	}
	inline const TStrongsOrthographyMap &strongsOrthographyMap() const
	{
		return m_mapStrongsOrthographyMap;
	}
	CMorphEntry lookupMorphology(MORPH_SOURCE_ENUM nSource, const QString &strKey) const;
	inline const TMorphDatabaseMap &morphologyDatabaseMap() const
	{
		return m_mapMorphDatabaseMap;
	}
	QString soundEx(const QString &strDecomposedConcordanceWord, bool bCache = true) const;		// Return and/or calculate soundEx for the specified Concordance Word (calculations done based on this Bible Database language)

	QString richVerseText(const CRelIndex &ndxRel,
							const CVerseTextRichifierTags &tags,
							RichifierRenderOptionFlags flagsRRO = RichifierRenderOptionFlags(),
							const CBasicHighlighter *aSRHighlighter = nullptr,
							const CBasicHighlighter *aSRExclHighlighter = nullptr) const;	// Generate and return verse text for specified index: [Book | Chapter | Verse | 0]

private:
	// CReadDatabase needed to load the database.  After that everything
	//	is read-only.  Database building is done directly from the CSV files
	//
	friend class CReadDatabase;
	friend class CReadDatabaseEx;			// Extension used by CLI tools to override functionality of CReadDatabase
	friend class COSISXmlHandler;			// COSISXmlHandler - Used by KJVDataParse for OSIS XML File processing to build KJPBS databases

// Main Database Data:
	struct TVersificationLayout {
		TVersificationLayout(CBibleDatabase *pBibleDatabase = nullptr)
			:	m_pParentBibleDatabase(pBibleDatabase)
		{ }
		bool isValid() const { return m_pParentBibleDatabase != nullptr; }
#ifdef OSIS_PARSER_BUILD
		uint32_t NormalizeIndexNoAccum(const CRelIndex &ndxRelIndex) const;
		CRelIndex DenormalizeIndexNoAccum(uint32_t nNormalIndex) const;
#endif
		uint32_t NormalizeIndex(const CRelIndex &ndxRelIndex) const;
		CRelIndex DenormalizeIndex(uint32_t nNormalIndex) const;
#ifdef USE_EXTENDED_INDEXES
		uint32_t NormalizeIndexEx(const CRelIndexEx &ndxRelIndexEx) const;
		CRelIndexEx DenormalizeIndexEx(uint32_t nNormalIndexEx) const;
#endif
		// ----
		CBibleEntry m_EntireBible;				// Entire Bible stats, calculated from testament stats in ReadDB.
		TTestamentList m_lstTestaments;			// Testament List: List(nTst-1)
		TBookList m_lstBooks;					// Books (Table of Contents): List(nBk-1)
		TChapterMap m_mapChapters;				// Chapter Entries Map: Map(CRelIndex[nBk | nChp | 0 | 0])
		TBookVerseList m_lstBookVerses;			// Book Verse Entries List: List(nBk-1) -> Map(CRelIndex[nBk | nChp | nVrs | 0])
	private:
		CBibleDatabase *m_pParentBibleDatabase;	// Parent Bible Database to which this versification layout belongs
	};
	typedef QMap<BIBLE_VERSIFICATION_TYPE_ENUM, TVersificationLayout> TVersificationLayoutMap;
	TVersificationLayoutMap m_mapVersificationLayouts;		// We will always have a Main Versification.  Others are optional and dependent on specific Bible database
	TVersificationLayoutMap::iterator m_itrCurrentLayout;	// Currently active versification layout for this database. This will always be a valid enumerator. It's not const so that ReadDB and KJVDataParse don't have to be special-cased (and it's private anyway)
	TVersificationLayoutMap::iterator m_itrMainLayout;		// Pointer to Main Versification Layout for quick comparison against current

	TWordListMap m_mapWordList;				// Master word-list Map (Indexed by lowercase word)
	QStringList m_lstWordList;				// Master word-list List as lowercase, used for searching lower/upper-bound for m_mapWordList
	bool m_bSearchSpaceIsCompleteConcordance;	// True if all of the words in the concordance list are searchable (i.e. there are no words with renderings that aren't searchable -- like cantillation marks).  This is used to speed things up and reduce memory usage when the space is the same.
	TConcordanceList m_lstConcordanceWords;	// List (QStringList) of all Unique Words as Composed UTF8 in the order for the concordance with names of the TWordListMap key (starts at index 0)
	TNormalizedIndexList m_lstConcordanceMapping;	// List of WordNdx# (in ConcordanceWords) for all 789629 words of the text (starts at index 1)
	TFootnoteEntryMap m_mapFootnotes;		// Footnotes (typed by index - See notes above with TFootnoteEntryMap)
	CPhraseList m_lstCommonPhrases;			// Common phrases read from database
	TLemmaEntryMap m_mapLemmaEntries;		// Lemmas (typed by index - See notes above with TLemmaEntryMap)
	TStrongsIndexMap m_mapStrongsEntries;	// Strongs Entries mapped by StrongsMapIndex
	TStrongsOrthographyMap m_mapStrongsOrthographyMap;		// Map of Strongs Orthography word(s) to StrongsMapIndex
	TMorphDatabaseMap m_mapMorphDatabaseMap;	// Mapping of Morphography database type to data
	mutable TSoundExMap m_mapSoundEx;		// SoundEx map of Decomposed words (from m_lstConcordanceWords) to SoundEx equivalent, used to minimize calculations

// Local Data:
	TBibleDescriptor m_descriptor;			// Bible Descriptor Record
	QString m_strInfo;						// Information about this database (copyright details, etc)

	CKJPBSWordScriptureObject *m_pKJPBSWordScriptureObject;		// Object used to render the words from this database in the Scripture Editor/Browser
};

Q_DECLARE_METATYPE(CBibleDatabase *)
typedef QSharedPointer<CBibleDatabase> CBibleDatabasePtr;
Q_DECLARE_METATYPE(CBibleDatabasePtr)

class TBibleDatabaseList : public QObject, protected QList<CBibleDatabasePtr>
{
	Q_OBJECT

private:				// Enforce Singleton:
	TBibleDatabaseList(QObject *pParent = nullptr);

public:
	virtual ~TBibleDatabaseList();
	static TBibleDatabaseList *instance();

	static bool useGematria() { return g_bUseGematria; }
	static void setUseGematria(bool bUseGematria) { g_bUseGematria = bUseGematria; }

	static const QString &bibleDatabasePath()
	{
		return instance()->m_strBibleDatabasePath;
	}

	void setBibleDatabasePath(bool bBuildDB);

#ifdef USING_WEBCHANNEL
	static QString availableBibleDatabasesAsJson();
#endif
	static bool loadBibleDatabase(const QString &strUUID, bool bAutoSetAsMain = false, QWidget *pParent = nullptr);
	static bool loadBibleDatabase(const TBibleDescriptor &bblDesc, bool bAutoSetAsMain = false, QWidget *pParent = nullptr);
	CBibleDatabasePtr mainBibleDatabase() const { return m_pMainBibleDatabase; }
	void setMainBibleDatabase(const QString &strUUID);
	bool haveMainBibleDatabase() const { return (!m_pMainBibleDatabase.isNull()); }
	void removeBibleDatabase(const QString &strUUID);
	void clear();
	int size() const { return QList<CBibleDatabasePtr>::size(); }
	CBibleDatabasePtr at(int i) const { return QList<CBibleDatabasePtr>::at(i); }
	CBibleDatabasePtr atUUID(const QString &strUUID) const;

	static const QList<TBibleDescriptor> &availableBibleDatabases()		// List of Bible Descriptors of available Bible Databases
	{
		instance()->findBibleDatabases();
		return instance()->m_lstAvailableDatabaseDescriptors;
	}
	static const TBibleDescriptor availableBibleDatabaseDescriptor(const QString &strUUID)
	{
		const QList<TBibleDescriptor> &lstDesc = availableBibleDatabases();
		for (int ndx = 0; ndx < lstDesc.size(); ++ndx) {
			if (strUUID.compare(lstDesc.at(ndx).m_strUUID, Qt::CaseInsensitive) == 0) return lstDesc.at(ndx);
		}
		return TBibleDescriptor();
	}

protected:
	void findBibleDatabases();

	friend class CReadDatabase;
	void addBibleDatabase(CBibleDatabasePtr pBibleDatabase, bool bSetAsMain);			// Added via CReadDatabase

signals:
	void loadedBibleDatabase(CBibleDatabasePtr pBibleDatabase);
	void removingBibleDatabase(CBibleDatabasePtr pBibleDatabase);
	void changedMainBibleDatabase(CBibleDatabasePtr pBibleDatabase);
	void changedBibleDatabaseList();
	void changedAvailableBibleDatabaseList();
	void beginChangeBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &oldSettings,
											const TBibleDatabaseSettings &newSettings, bool bForce);		// bForce = Force Update even if settings aren't actually different
	void endChangeBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &oldSettings,
											const TBibleDatabaseSettings &newSettings, bool bForce);		// bForce = Force Update even if settings aren't actually different

protected slots:
	// en_changedBibleDatabaseSettings is triggered from CPersistentSettings:
	void en_changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &oldSettings,
											const TBibleDatabaseSettings &newSettings, bool bForce);

private:
	CBibleDatabasePtr m_pMainBibleDatabase;
	bool m_bHaveSearchedAvailableDatabases;							// True when we've done at least one find operation
	QList<TBibleDescriptor> m_lstAvailableDatabaseDescriptors;		// List of descriptors for available Bible databases

	QString m_strBibleDatabasePath;
	static bool g_bUseGematria;						// Set to true when the command-line -gematria option is specified (here so that it works with all applications, not just the main KJPBS GUI)
};

// ============================================================================

// Dictionary Word Entry -- Mapping of words and their Definitions:
//

class CDictionaryWordEntry : public CBasicWordEntry
{
public:
	CDictionaryWordEntry();
	CDictionaryWordEntry(const QString &strWord);
	virtual ~CDictionaryWordEntry() { }

	CDictionaryWordEntry & operator=(const CDictionaryWordEntry &src)
	{
		m_strWord = src.m_strWord;
		m_strDecomposedWord = src.m_strDecomposedWord;
		m_lstDefinitions = src.m_lstDefinitions;
		m_lstIndexes = src.m_lstIndexes;
		return *this;
	}

	virtual const QString &word() const override { return m_strWord; }
	virtual const QString &searchWord() const override { return m_strWord; }
	virtual const QString &rawWord() const override { return m_strDecomposedWord; }
	virtual const QString &decomposedWord() const override { return m_strDecomposedWord; }
	virtual const QString &decomposedHyphenWord() const override { return m_strDecomposedWord; }
	virtual const QString &deApostrWord() const override { return m_strDecomposedWord; }
	virtual const QString &deApostrHyphenWord() const override { return m_strDecomposedWord; }
	virtual const QString &renderedWord() const override { return m_strWord; }
	const QStringList &definitions() const { return m_lstDefinitions; }
	const QList<int> &indexes() const { return m_lstIndexes; }
	void addDefinition(int nIndex, const QString &strDefinition)
	{
		if (!m_lstIndexes.contains(nIndex)) {
			m_lstIndexes.append(nIndex);
			m_lstDefinitions.append(strDefinition);
		}
	}

	bool operator==(const CDictionaryWordEntry &src) const
	{
		return (m_strDecomposedWord.compare(src.m_strDecomposedWord) == 0);
	}
	bool operator!=(const CDictionaryWordEntry &src) const
	{
		return (m_strDecomposedWord.compare(src.m_strDecomposedWord) != 0);
	}

private:
	QString m_strWord;						// Composed Word (as in the actual text)
	QString m_strDecomposedWord;			// Lowercase Decomposed Word (used for matching)
	QStringList m_lstDefinitions;			// Rich-Text Definitions of the Word
	QList<int> m_lstIndexes;				// Database indexes -- used for live database lookup
};

typedef std::map<QString, CDictionaryWordEntry> TDictionaryWordListMap;		// Indexed by lower-case decomposed words from word-list

// ============================================================================

class TDictionaryDatabaseSettings
{
public:
	explicit TDictionaryDatabaseSettings()
		:	m_bLoadOnStart(false)
	{ }

	bool isValid() const { return true; }

	inline bool operator==(const TDictionaryDatabaseSettings &other) const {
		return (m_bLoadOnStart == other.m_bLoadOnStart);
	}
	inline bool operator!=(const TDictionaryDatabaseSettings &other) const {
		return (!operator==(other));
	}

	bool loadOnStart() const { return m_bLoadOnStart; }
	void setLoadOnStart(bool bLoadOnStart) { m_bLoadOnStart = bLoadOnStart; }

private:
	bool m_bLoadOnStart;
};

typedef QMap<QString, TDictionaryDatabaseSettings> TDictionaryDatabaseSettingsMap;		// Map of Dictionary UUIDs to settings for saving/preserving

// ============================================================================

// CDictionaryDatabase - Class to define a Dictionary Database file
class CDictionaryDatabase
{
protected:
	CDictionaryDatabase(const TDictionaryDescriptor &dctDesc);		// Creatable by CReadDatabase
public:
	virtual ~CDictionaryDatabase();

	DictionaryTypeOptionsFlags flags() const { return m_descriptor.m_dtoFlags; }
	QString language() const { return m_descriptor.m_strLanguage; }
	LANGUAGE_ID_ENUM langID() const { return toLanguageID(m_descriptor.m_strLanguage); }
	QString name() const { return m_descriptor.m_strDBName; }
	QString description() const { return m_descriptor.m_strDBDesc; }
	QString compatibilityUUID() const { return m_descriptor.m_strUUID; }
	const TDictionaryDescriptor &descriptor() const { return m_descriptor; }

	QString info() const { return m_strInfo; }

	bool isLiveDatabase() const {
#ifndef NOT_USING_SQL
		return m_myDatabase.isOpen();
#else
		return false;
#endif
	}

	virtual QString soundEx(const QString &strDecomposedDictionaryWord, bool bCache = true) const;		// Return and/or calculate soundEx for the specified Dictionary Word (calculations done based on this Dictionary Database language)

	virtual QString definition(const QString &strWord) const;		// Lookup and return definition for word
	virtual bool wordExists(const QString &strWord) const;			// Lookup word and return true/false on its existence

	virtual const CDictionaryWordEntry &wordDefinitionsEntry(const QString &strKeyWord) const;		// Lookup by lower-case decomposed word from word-list map
	virtual int wordCount() const;							// Count of words in m_lstWordList
	virtual QString wordEntry(int ndx) const;				// Entry from m_lstWordList

private:
	// CReadDatabase needed to load the database.  After that everything
	//	is read-only.
	//
	friend class CReadDatabase;

// Main Database Data:
	TDictionaryWordListMap m_mapWordDefinitions;
	QStringList m_lstWordList;				// List of decomposed lower-case keywords from map, stored in list for quick enumeration
	mutable TSoundExMap m_mapSoundEx;		// SoundEx map of Decomposed words (from m_mapWordDefinitions) to SoundEx equivalent, used to minimize calculations

// Local Data:
	TDictionaryDescriptor m_descriptor;		// Dictionary Descriptor Record
	QString m_strInfo;						// Information about this database (copyright details, etc)
#ifndef NOT_USING_SQL
	QSqlDatabase m_myDatabase;				// Open SQL for this dictionary
#endif
};

Q_DECLARE_METATYPE(CDictionaryDatabase *)
typedef QSharedPointer<CDictionaryDatabase> CDictionaryDatabasePtr;
Q_DECLARE_METATYPE(CDictionaryDatabasePtr)

// ----------------------------------------------------------------------------

// CStrongsDictionaryDatabase - Class to define a Strongs Dictionary Database file
class CStrongsDictionaryDatabase : public CDictionaryDatabase
{
protected:
	CStrongsDictionaryDatabase(const TDictionaryDescriptor &dctDesc, CBibleDatabasePtr pBibleDatabase)		// Creatable by CReadDatabase
		:	CDictionaryDatabase(dctDesc),
			m_pBibleDatabase(pBibleDatabase)
	{
		Q_ASSERT(!m_pBibleDatabase.isNull());
	}
public:
	virtual ~CStrongsDictionaryDatabase() { }

	virtual QString soundEx(const QString &strDecomposedDictionaryWord, bool bCache = true) const override;		// Return and/or calculate soundEx for the specified Dictionary Word (calculations done based on this Dictionary Database language)

	virtual QString definition(const QString &strWord) const override;		// Lookup and return definition for word
	virtual bool wordExists(const QString &strWord) const override;			// Lookup word and return true/false on its existence

	// WARNING: wordDefinitionsEntry() is NOT thread-safe in CStrongsDictionaryDatabase!!
	//	As such, use the CSearchStrongsDictionaryListModel and not the CSearchDictionaryListModel.
	//	the wordDefinitionsEntry() is here only as a placeholder so compilation will succeed.
	//	Since there are no actual words in the Strongs Database (only indexes), it would be a
	//	waste of resouces to use CSearchDictionaryListModel anyway (i.e. it's really slow!)
	virtual const CDictionaryWordEntry &wordDefinitionsEntry(const QString &strKeyWord) const override;		// Lookup by lower-case decomposed word from word-list map
	virtual int wordCount() const override;							// Count of words in m_lstWordList
	virtual QString wordEntry(int ndx) const override;				// Entry from m_lstWordList

private:
	// CReadDatabase needed to load the database.  After that everything
	//	is read-only.
	//
	friend class CReadDatabase;

	CBibleDatabasePtr m_pBibleDatabase;
	mutable CDictionaryWordEntry m_tmpDictionaryWordEntry;			// Temporary word return for wordDefinitionsEntry().
};

// ----------------------------------------------------------------------------

class  TDictionaryDatabaseList : public QObject, protected QList<CDictionaryDatabasePtr>
{
	Q_OBJECT

private:				// Enforce Singleton:
	TDictionaryDatabaseList(QObject *pParent = nullptr);

public:
	virtual ~TDictionaryDatabaseList();
	static TDictionaryDatabaseList *instance();

	static const QString &dictionaryDatabasePath()
	{
		return instance()->m_strDictionaryDatabasePath;
	}

	static CDictionaryDatabasePtr locateAndLoadDictionary(const QString &strLanguage, QWidget *pParentWidget = nullptr);		// Locates and loads the best candidate dictionary for the specified language based on MainDictionary and DictionaryLoad settings
	static CDictionaryDatabasePtr locateAndLoadStrongsDictionary(const QString &strBibleUUID, const QString &strLanguage, QWidget *pParentWidget = nullptr);		// Locates and loads the best candidate Strong's Dictionary for the specified language and DictionaryLoad settings

	static CDictionaryDatabasePtr loadDictionaryDatabase(const QString &strUUID, bool bAutoSetAsMain = false, QWidget *pParent = nullptr);
	CDictionaryDatabasePtr mainDictionaryDatabase() const { return m_pMainDictionaryDatabase; }
	void setMainDictionaryDatabase(const QString &strUUID);
	bool haveMainDictionaryDatabase() const { return (!m_pMainDictionaryDatabase.isNull()); }
	void removeDictionaryDatabase(const QString &strUUID);
	void clear();
	int size() const { return QList<CDictionaryDatabasePtr>::size(); }
	CDictionaryDatabasePtr at(int i) const { return QList<CDictionaryDatabasePtr>::at(i); }
	CDictionaryDatabasePtr atUUID(const QString &strUUID) const;

	static const QList<TDictionaryDescriptor> &availableDictionaryDatabases()		// List of Dictionary Descriptors of available Dictionary Databases
	{
		instance()->findDictionaryDatabases();
		return instance()->m_lstAvailableDatabaseDescriptors;
	}
	static const TDictionaryDescriptor availableDictionaryDatabaseDescriptor(const QString &strUUID)
	{
		const QList<TDictionaryDescriptor> &lstDesc = availableDictionaryDatabases();
		for (int ndx = 0; ndx < lstDesc.size(); ++ndx) {
			if (strUUID.compare(lstDesc.at(ndx).m_strUUID, Qt::CaseInsensitive) == 0) return lstDesc.at(ndx);
		}
		return TDictionaryDescriptor();
	}

protected:
	void findDictionaryDatabases();

	friend class CReadDatabase;
	void addDictionaryDatabase(CDictionaryDatabasePtr pDictionaryDatabase, bool bSetAsMain);		// Added via CReadDatabase

signals:
	void loadedDictionaryDatabase(CDictionaryDatabasePtr pDictionaryDatabase);
	void removingDictionaryDatabase(CDictionaryDatabasePtr pDictionaryDatabase);
	void changedMainDictionaryDatabase(CDictionaryDatabasePtr pDictionaryDatabase);
	void changedDictionaryDatabaseList();
	void changedAvailableDictionaryDatabaseList();

private:
	CDictionaryDatabasePtr m_pMainDictionaryDatabase;
	bool m_bHaveSearchedAvailableDatabases;							// True when we've done at least one find operation
	QList<TDictionaryDescriptor> m_lstAvailableDatabaseDescriptors;		// List of descriptors for available Dictionary databases

	QString m_strDictionaryDatabasePath;
};


// ============================================================================

#endif // DATABASE_STRUCT_H
