#ifndef DBSTRUCT_H
#define DBSTRUCT_H

#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdint.h>
#include <QString>

#ifndef uint32_t
#define uint32_t unsigned int
#endif

// ============================================================================

typedef struct TRelIndex {
	uint32_t m_nN3;
	uint32_t m_nN2;
	uint32_t m_nN1;
	uint32_t m_nN0;
} TRelIndex;

extern uint32_t MakeIndex(uint32_t nN3, uint32_t nN2, uint32_t nN1, uint32_t nN0);
extern uint32_t MakeIndex(const TRelIndex &relIndex);
extern TRelIndex DecomposeIndex(uint32_t nIndex);
extern uint32_t NormalizeIndex(uint32_t nRelIndex);
extern uint32_t DenormalizeIndex(uint32_t nNormalIndex);

// ============================================================================

typedef std::vector<uint32_t> TIndexList;			// Index List for words into book/chapter/verse/word

struct IndexSortPredicate {
	bool operator() (const uint32_t &v1, const uint32_t &v2) const
	{
		return (v1 < v2);
	}
};

typedef std::vector<QString> TStringList;

struct XformLower {
	int operator()(int c)
	{
		return tolower(c);
	}
};

// ============================================================================

// TESTAMENT -- Table of Testaments:
//
class CTestamentEntry
{
public:
	CTestamentEntry() { }
	~CTestamentEntry() { }

	QString  m_strTstName;		// Name of testament (display name)
};

typedef std::vector<CTestamentEntry> TTestamentList;		// Index by nTst-1

extern TTestamentList g_lstTestaments;		// Global Testament List

// ============================================================================

// TOC -- Table of Contents:
//
class CTOCEntry
{
public:
	CTOCEntry()
	:   m_nTstBkNdx(0),
		m_nTstNdx(0),
		m_nNumChp(0),
		m_nNumVrs(0),
		m_nNumWrd(0)
	{ }
	~CTOCEntry() { }

	unsigned int m_nTstBkNdx;	// Testament Book Index (Index within the books of the testament) 1-39 or 1-27
	unsigned int m_nTstNdx;		// Testament Index (1=Old, 2=New, etc)
	QString m_strBkName;		// Name of book (display name)
	QString m_strBkAbbr;		// Book Abbreviation
	QString m_strTblName;		// Name of Table for this book
	unsigned int m_nNumChp;		// Number of chapters in this book
	unsigned int m_nNumVrs;		// Number of verses in this book
	unsigned int m_nNumWrd;		// Number of words in this book
	QString m_strCat;			// Category name
	QString m_strDesc;			// Description (subtitle)
};

typedef std::vector<CTOCEntry> TTOCList;	// Index by nBk-1

extern TTOCList g_lstTOC;		// Global Table of Contents

// ============================================================================

// LAYOUT -- Book/Chapter Layout:
//
class CLayoutEntry
{
public:
	CLayoutEntry()
	:   m_nNumVrs(0),
		m_nNumWrd(0)
	{ }
	~CLayoutEntry() { }

	unsigned int m_nNumVrs;		// Number of verses in this chapter
	unsigned int m_nNumWrd;		// Number of words in this chapter
};

typedef std::map<uint32_t, CLayoutEntry, IndexSortPredicate> TLayoutMap;	// Index by [nBk|nChp]

extern TLayoutMap g_mapLayout;	// Global Layout

// ============================================================================

// BOOK -- Chapter/Verse Layout:
//
class CBookEntry
{
public:
	CBookEntry()
	:   m_nNumWrd(0),
		m_bPilcrow(false)
	{ }
	~CBookEntry() { }

	unsigned int m_nNumWrd;		// Number of words in this verse
	bool m_bPilcrow;			// Start of verse Pilcrow Flag
	QString GetRichText() const		// We'll use a function to fetch the rich text (on mobile this will be a database lookup)
	{
		return m_strRichText;
	}
	void SetRichText(const QString &strRichText)		// This will be a noop on mobile
	{
		m_strRichText = strRichText;
	}
	QString m_strFootnote;		// Footnote text for this verse (if any)

private:
	QString m_strRichText;		// Rich text for the verse (Note: for mobile versions, this element will be removed and fetched from the database)
};

typedef std::map<uint32_t, CBookEntry, IndexSortPredicate> TBookEntryMap;		// Index by [nChp|nVrs]

typedef std::vector<TBookEntryMap> TBookList;	// Index by nBk-1

extern TBookList g_lstBooks;	// Global Books

// ============================================================================

// WORDS -- Word List and Mapping
//
class CWordEntry
{
public:
	CWordEntry() { }
	~CWordEntry() { }

	QString m_strWord;			// Word Text
	TStringList m_lstAltWords;	// List of alternate synonymous words for searching (such as hyphenated and non-hyphenated)
	TIndexList m_ndxMapping;	// Indexes
	TIndexList m_ndxNormalized;	// Normalized index into entire Bible (Number of entries here should match number of indexes in Mapping above)

	struct SortPredicate {
		bool operator() (const QString &s1, const QString &s2) const
		{
			return (s1.compare(s2) < 0);
		}
	};
};

typedef std::map<QString, CWordEntry, CWordEntry::SortPredicate> TWordListMap;

extern TWordListMap g_mapWordList;	// Our one and only master word list (Indexed by lowercase word except those that case-preserve)

// ============================================================================


#endif // DBSTRUCT_H
