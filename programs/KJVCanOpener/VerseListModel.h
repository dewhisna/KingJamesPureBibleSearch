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

#ifndef VERSELISTMODEL_H
#define VERSELISTMODEL_H

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "KJVSearchCriteria.h"
#include "VerseRichifier.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <QPair>
#include <QObject>
#include <QSize>
#include <QFont>
#include <QSet>
#include <QSharedPointer>

#include <assert.h>

// ============================================================================

// Verse List Model Results Type Enum:
//		Note: Either keep this in-sync with VERSE_VIEW_MODE_ENUM or
//				update VVME_to_VLMRTE and VLMRTE_to_VVME functions:
enum VERSE_LIST_MODEL_RESULTS_TYPE_ENUM {
	VLMRTE_UNDEFINED = -1,							// Undefined Index (used for defaults)
	VLMRTE_SEARCH_RESULTS = 0,						// Search Results Display Index
	VLMRTE_HIGHLIGHTERS = 1,						// Highlighter Display Index
	VLMRTE_USER_NOTES = 2							// User Notes Display Index
};

#define VLM_SI_UNDEFINED -1							// Undefined Highlighter Index
#define VLM_SI_BOOK_TERMINATOR_NODE -2				// Terminator Node for Tree Book nodes
#define VLM_SI_CHAPTER_TERMINATOR_NODE -3			// Terminator Node for Tree Chapter nodes

class TVerseIndex {
public:
	TVerseIndex(const CRelIndex &ndx = CRelIndex(), VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultType = VLMRTE_UNDEFINED, int nSpecialIndex = VLM_SI_UNDEFINED)
		:	m_nRelIndex(ndx),
			m_nResultsType(nResultType),
			m_nSpecialIndex(nSpecialIndex)
	{ }

	TVerseIndex(const TVerseIndex &aVerseIndex)
		:	m_nRelIndex(aVerseIndex.m_nRelIndex),
			m_nResultsType(aVerseIndex.m_nResultsType),
			m_nSpecialIndex(aVerseIndex.m_nSpecialIndex)
	{

	}

	const CRelIndex relIndex() const { return m_nRelIndex; }
	VERSE_LIST_MODEL_RESULTS_TYPE_ENUM resultsType() const { return m_nResultsType; }
	int specialIndex() const { return m_nSpecialIndex; }

	bool operator <(const TVerseIndex &other) const
	{
		return ((m_nResultsType < other.m_nResultsType) ||
				((m_nResultsType == other.m_nResultsType) && (m_nSpecialIndex < other.m_nSpecialIndex)) ||
				((m_nResultsType == other.m_nResultsType) && (m_nSpecialIndex == other.m_nSpecialIndex) && (m_nRelIndex < other.m_nRelIndex)));
	}

protected:
	friend class CVerseListModel;

	CRelIndex m_nRelIndex;					// Relative Bible index
	VERSE_LIST_MODEL_RESULTS_TYPE_ENUM m_nResultsType;		// Type of index this is
	int m_nSpecialIndex;				// Index into list of highlighters (0 to n) or VLM_SI_UNDEFINED for Results Types that don't use indexes
};
typedef QSharedPointer<TVerseIndex> TVerseIndexPtr;
typedef QList<TVerseIndex> TVerseIndexList;
typedef QMap<CRelIndex, TVerseIndexPtr> TVerseIndexPtrMap;

struct TVerseIndexListSortPredicate {
	static bool ascendingLessThan(const TVerseIndex &s1, const TVerseIndex &s2)
	{
		return (s1 < s2);
	}
};

// ============================================================================

class CVerseListItem
{
public:
	explicit CVerseListItem()
	{

	}

//	explicit CVerseListItem(CBibleDatabasePtr pBibleDatabase = CBibleDatabasePtr(),
//							const CRelIndex &ndx = CRelIndex(),
//							const unsigned int nPhraseSize = 1)
//		:	m_pBibleDatabase(pBibleDatabase),
//			m_ndxRelative(ndx)
//	{
//		m_ndxRelative.setWord(0);								// Primary index will have a zero word index
//		if (nPhraseSize > 0)
//			m_lstTags.push_back(TPhraseTag(ndx, nPhraseSize));		// But the corresponding tag will have non-zero word index
//	}

	CVerseListItem(const TVerseIndex &aVerseIndex, CBibleDatabasePtr pBibleDatabase, const TPhraseTag &tag = TPhraseTag())
		:	m_pBibleDatabase(pBibleDatabase)
	{
		m_pVerseIndex = TVerseIndexPtr(new TVerseIndex(aVerseIndex));
		if (tag.relIndex().isSet()) m_lstTags.push_back(tag);
	}

	CVerseListItem(const TVerseIndex &aVerseIndex, CBibleDatabasePtr pBibleDatabase, const TPhraseTagList &lstTags)
		:	m_pBibleDatabase(pBibleDatabase),
			m_lstTags(lstTags)
	{
		m_pVerseIndex = TVerseIndexPtr(new TVerseIndex(aVerseIndex));
	}
	~CVerseListItem()
	{ }

	TVerseIndexPtr verseIndex() const { return m_pVerseIndex; }

	inline QString getHeading() const {
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QString();
		bool bSearchRefs = (verseIndex()->resultsType() == VLMRTE_SEARCH_RESULTS);		// For Search Results, show word positions too
		QString strHeading;
		if ((m_lstTags.size() > 0) && (verseIndex()->resultsType() != VLMRTE_USER_NOTES)) {
			strHeading += QString("(%1) ").arg(m_lstTags.size());
			for (int ndx = 0; ndx < m_lstTags.size(); ++ndx) {
				if (ndx == 0) {
					if (bSearchRefs) {
						strHeading += m_pBibleDatabase->PassageReferenceText(m_lstTags.at(ndx).relIndex());
					} else {
						strHeading += m_pBibleDatabase->PassageReferenceText(getIndex());
					}
				} else {
					if (bSearchRefs) strHeading += QString("[%1]").arg(m_lstTags.at(ndx).relIndex().word());
				}
			}
		} else {
			strHeading += m_pBibleDatabase->PassageReferenceText(getIndex());
		}
		return strHeading;
	}

	inline QString getToolTip(const TParsedPhrasesList &phrases = TParsedPhrasesList()) const {
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QString();
		QString strToolTip;
		strToolTip += m_pBibleDatabase->SearchResultToolTip(getIndex(), RIMASK_BOOK | RIMASK_CHAPTER | RIMASK_VERSE);
		for (int ndx = 0; ndx < phraseTags().size(); ++ndx) {
			const CRelIndex &ndxTag(phraseTags().at(ndx).relIndex());
			if (phraseTags().size() > 1) {
				strToolTip += QString("(%1)[%2] \"%3\" %4 ").arg(ndx+1).arg(ndxTag.word()).arg(getPhrase(ndx)).arg(QObject::tr("is"));
			} else {
				strToolTip += QString("[%1] \"%2\" %3 ").arg(ndxTag.word()).arg(getPhrase(ndx)).arg(QObject::tr("is"));
			}
			strToolTip += m_pBibleDatabase->SearchResultToolTip(ndxTag, RIMASK_WORD);
			for (int ndxPhrase = 0; ndxPhrase < phrases.size(); ++ndxPhrase) {
				const CParsedPhrase *pPhrase = phrases.at(ndxPhrase);
				assert(pPhrase != NULL);
				if (pPhrase == NULL) continue;
				if (pPhrase->GetPhraseTagSearchResults().contains(phraseTags().at(ndx))) {
					strToolTip += "    " + QObject::tr("%1 of %2 of Search Phrase \"%3\" Results in Selected Search Text")
										.arg(pPhrase->GetPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
										.arg(pPhrase->GetPhraseTagSearchResults().size())
										.arg(pPhrase->phrase()) + "\n";
				}
				if (pPhrase->GetScopedPhraseTagSearchResults().contains(phraseTags().at(ndx))) {
					strToolTip += "    " + QObject::tr("%1 of %2 of Search Phrase \"%3\" Results in Search Scope")
										.arg(pPhrase->GetScopedPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
										.arg(pPhrase->GetScopedPhraseTagSearchResults().size())
										.arg(pPhrase->phrase()) + "\n";
				}
			}
		}

		return strToolTip;
	}

	inline bool isSet() const {
		return (m_pVerseIndex->relIndex().isSet());
	}

	inline uint32_t getBook() const { return m_pVerseIndex->relIndex().book(); }		// Book Number (1-n)
	inline uint32_t getChapter() const { return m_pVerseIndex->relIndex().chapter(); }	// Chapter Number within Book (1-n)
	inline uint32_t getVerse() const { return m_pVerseIndex->relIndex().verse(); }		// Verse Number within Chapter (1-n)
	uint32_t getIndexNormalized() const {
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return 0;
		return m_pBibleDatabase->NormalizeIndex(m_pVerseIndex->relIndex());
	}
	inline uint32_t getIndexDenormalized() const { return m_pVerseIndex->relIndex().index(); }
	inline CRelIndex getIndex() const { return m_pVerseIndex->relIndex(); }
	inline unsigned int getPhraseSize(int nTag) const {
		assert((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((nTag < 0) || (nTag >= m_lstTags.size())) return 0;
		return m_lstTags.at(nTag).count();
	}
	inline CRelIndex getPhraseReference(int nTag) const {
		assert((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((nTag < 0) || (nTag >= m_lstTags.size())) return CRelIndex();
		return m_lstTags.at(nTag).relIndex();
	}
	void addPhraseTag(const CRelIndex &ndx, unsigned int nPhraseSize) { m_lstTags.push_back(TPhraseTag(ndx, nPhraseSize)); }
	void addPhraseTag(const TPhraseTag &tag) { m_lstTags.push_back(tag); }
	const TPhraseTagList &phraseTags() const { return m_lstTags; }

	QStringList getWordList(int nTag) const
	{
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QStringList();
		assert((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((!isSet()) || (nTag < 0) || (nTag >= m_lstTags.size())) return QStringList();
		QStringList strWords;
		unsigned int nNumWords = m_lstTags.at(nTag).count();
		uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(m_lstTags.at(nTag).relIndex());
		while (nNumWords) {
			strWords.push_back(m_pBibleDatabase->wordAtIndex(ndxNormal));
			ndxNormal++;
			nNumWords--;
		}
		return strWords;
	}
	QString getPhrase(int nTag) const
	{
		return getWordList(nTag).join(" ");
	}

	QStringList getVerseAsWordList() const
	{
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QStringList();
		if (!isSet()) return QStringList();
		QStringList strWords;
		const CVerseEntry *pVerseEntry = m_pBibleDatabase->verseEntry(CRelIndex(getBook(), getChapter(), getVerse(), 0));
		unsigned int nNumWords = (pVerseEntry ? pVerseEntry->m_nNumWrd : 0);
		uint32_t ndxNormal = getIndexNormalized();
		while (nNumWords) {
			strWords.push_back(m_pBibleDatabase->wordAtIndex(ndxNormal));
			ndxNormal++;
			nNumWords--;
		}
		return strWords;
	}
	QString getVerseVeryPlainText() const		// Very Plain has no punctuation!
	{
#ifdef VERSE_LIST_PLAIN_TEXT_CACHE
		if (!m_strVeryPlainTextCache.isEmpty()) return m_strVeryPlainTextCache;
		m_strVeryPlainTextCache = getVerseAsWordList().join(" ");
		return m_strVeryPlainTextCache;
#else
		return getVerseAsWordList().join(" ");
#endif
	}
	QString getVerseRichText(const CVerseTextRichifierTags &richifierTags) const
	{
#ifdef VERSE_LIST_RICH_TEXT_CACHE
		if (!m_strRichTextCache.isEmpty()) return m_strRichTextCache;
#endif
		assert(m_pBibleDatabase.data() != NULL);
		if (m_pBibleDatabase.data() == NULL) return QString();
		if (!isSet()) return QString();
#ifdef VERSE_LIST_RICH_TEXT_CACHE
		m_strRichTextCache = m_pBibleDatabase->richVerseText(m_pVerseIndex->relIndex(), richifierTags, false);
		return m_strRichTextCache;
#else
		return m_pBibleDatabase->richVerseText(m_pVerseIndex->relIndex(), richifierTags, false);
#endif
	}

private:
	CBibleDatabasePtr m_pBibleDatabase;
	TPhraseTagList m_lstTags;		// Phrase Tags to highlight in this object

	TVerseIndexPtr m_pVerseIndex;	// Used for QModelIndex Internal object

// Caches filled in during first fetch:
#ifdef VERSE_LIST_RICH_TEXT_CACHE
	mutable QString m_strRichTextCache;
#endif
#ifdef VERSE_LIST_PLAIN_TEXT_CACHE
	mutable QString m_strVeryPlainTextCache;
#endif
};

Q_DECLARE_METATYPE(CVerseListItem)

typedef QList<CVerseListItem> CVerseList;
extern void sortVerseList(CVerseList &aVerseList, Qt::SortOrder order);

typedef QMap<CRelIndex, CVerseListItem> CVerseMap;

// ============================================================================

class CVerseListModel : public QAbstractItemModel
{
	Q_OBJECT
public:

	enum VERSE_DISPLAY_MODE_ENUM {
		VDME_HEADING = 0,
		VDME_VERYPLAIN = 1,
		VDME_RICHTEXT = 2,
		VDME_COMPLETE = 3
	};

	enum VERSE_TREE_MODE_ENUM {
		VTME_LIST = 0,					// Display as a linear list (like old QListView used to)
		VTME_TREE_BOOKS = 1,			// Tree by Books = Branch verses under Books w/o Chapters
		VTME_TREE_CHAPTERS = 2			// Tree by Chapters = Branch verses under Chapters under Books
	};

	enum VERSE_VIEW_MODE_ENUM {
		VVME_SEARCH_RESULTS = 0,		// Display Search Results in the Tree View
		VVME_HIGHLIGHTERS = 1,			// Display Tree of Highlighter Tags
		VVME_USERNOTES = 2				// Display Tree of User Notes
	};
	static VERSE_LIST_MODEL_RESULTS_TYPE_ENUM VVME_to_VLMRTE(VERSE_VIEW_MODE_ENUM nViewMode)
	{
		return static_cast<VERSE_LIST_MODEL_RESULTS_TYPE_ENUM>(nViewMode);
	}
	static VERSE_VIEW_MODE_ENUM VLMRTE_to_VVME(VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType)
	{
		return static_cast<VERSE_VIEW_MODE_ENUM>(nResultsType);
	}

	enum VERSE_DATA_ROLES_ENUM {
		VERSE_ENTRY_ROLE = Qt::UserRole + 0,				// Full verse text display mode
		VERSE_HEADING_ROLE = Qt::UserRole + 1,				// Verse heading display text
		TOOLTIP_ROLE = Qt::UserRole + 2,					// Use our own ToolTip role so we can have user click Ctrl-D to see details ToolTip.  Qt::ToolTip will be a message telling them to do that.
		TOOLTIP_PLAINTEXT_ROLE = Qt::UserRole + 3,			// Same as TOOLTIP_ROLE, but as PlainText instead of RichText
		TOOLTIP_NOHEADING_ROLE = Qt::UserRole + 4,			// Same as TOOLTIP_ROLE, but without Verse Reference Heading
		TOOLTIP_NOHEADING_PLAINTEXT_ROLE = Qt::UserRole + 5	// Same as TOOLTIP_PLAINTEXT_ROLE, but without Verse Reference Heading
	};

	// ------------------------------------------------------------------------

	class TVerseListModelPrivate {
	public:
		TVerseListModelPrivate(CBibleDatabasePtr pBibleDatabase);

		CBibleDatabasePtr m_pBibleDatabase;
		VERSE_DISPLAY_MODE_ENUM m_nDisplayMode;		// Headings vs RichText, etc
		VERSE_TREE_MODE_ENUM m_nTreeMode;			// List, Tree by Books, Tree by Chapters, etc.
		VERSE_VIEW_MODE_ENUM m_nViewMode;			// Search Results vs Highlighters, etc
		bool m_bShowMissingLeafs;					// Shows the missing leafs in book or book/chapter modes
		CVerseTextRichifierTags m_richifierTags;	// Richifier tags used to render the results in this list
		QFont m_font;								// Normally we wouldn't keep this here in the model, but this is directly accessible to the delegate showing us and we have to trigger the model anyway to update sizeHints()
	};

	// ------------------------------------------------------------------------

	static TVerseIndex *toVerseIndex(const QModelIndex &ndx) {
		static TVerseIndex nullVerseIndex;				// Empty VerseIndex to use for parent entries where QModelIndex->NULL
		return ((ndx.internalPointer() != NULL) ? reinterpret_cast<TVerseIndex *>(ndx.internalPointer()) : &nullVerseIndex);
	}
	static void *fromVerseIndex(const TVerseIndex *ndx) { return reinterpret_cast<void *>(const_cast<TVerseIndex *>(ndx)); }

	// Data for one parsed TPhraseTagList (Used one for Search Results and one for each Highlighter)
	class TVerseListModelResults {
	public:
		TVerseListModelResults(const TVerseListModelResults &other)
			:	m_mapVerses(other.m_mapVerses),
				m_lstVerseIndexes(other.m_lstVerseIndexes),
				m_mapExtraVerseIndexes(other.m_mapExtraVerseIndexes),
				m_mapSizeHints(other.m_mapSizeHints),
				m_private(other.m_private),
				m_strResultsName(other.m_strResultsName),
				m_nResultsType(other.m_nResultsType),
				m_nSpecialIndex(other.m_nSpecialIndex)
		{

		}

	protected:
		friend class CVerseListModel;

		TVerseListModelResults(TVerseListModelPrivate *priv, const QString &strResultsName, VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType, int nSpecialIndex = VLM_SI_UNDEFINED)
			:	m_private(priv),
				m_strResultsName(strResultsName),
				m_nResultsType(nResultsType),
				m_nSpecialIndex(nSpecialIndex)
		{ }

		CVerseMap m_mapVerses;						// Map of Verse Search Results by CRelIndex [nBk|nChp|nVrs|0].  Set in buildScopedResultsFromParsedPhrases()
		QList<CRelIndex> m_lstVerseIndexes;			// List of CRelIndexes in CVerseMap -- needed because index lookup within the QMap is time-expensive
		mutable TVerseIndexPtrMap m_mapExtraVerseIndexes;	// Used to store VerseIndex objects we give out for items with no data, like Book/Chapter headings (cleared in buildScopedResultsFromParsedPhrases() and created on demand).  Objects we give out are in CVerseListModel.
		QMap<TVerseIndex, QSize> m_mapSizeHints;	// Map of TVerseIndex (CRelIndex [nBk|nChp|nVrs|0] and Results Type and Special Index) to SizeHint -- used for ReflowDelegate caching (Note: This only needs to be cleared if we change databases or display modes!)

		// --------------------------------------

		TVerseIndexPtr extraVerseIndex(const TVerseIndex &aVerseIndex) const
		{
			TVerseIndexPtrMap::const_iterator itr = m_mapExtraVerseIndexes.find(aVerseIndex.relIndex());
			if (itr != m_mapExtraVerseIndexes.constEnd()) return itr.value();

			return m_mapExtraVerseIndexes.insert(aVerseIndex.relIndex(), TVerseIndexPtr(new TVerseIndex(aVerseIndex))).value();
		}

		// --------------------------------------

		int GetBookCount() const;						// Returns the number of books in the model based on mode
		int IndexByBook(unsigned int nBk) const;		// Returns the index (in the number of books) for the specified Book number
		unsigned int BookByIndex(int ndxBook) const;	// Returns the Book Number for the specified index (in the number of books)
		int GetChapterCount(unsigned int nBk) const;	// Returns the number of chapters in the specified book number based on the current mode
		int IndexByChapter(unsigned int nBk, unsigned int nChp) const;	// Returns the index (in the number of chapters) for the specified Chapter number
		unsigned int ChapterByIndex(int ndxBook, int ndxChapter) const;		// Returns the Chapter Number for the specified index (in the number of chapters)
		CVerseMap::const_iterator FindVerseIndex(const CRelIndex &ndxRel) const;	// Looks for the specified CRelIndex in m_mapVerses and returns its index
		CVerseMap::const_iterator GetVerse(int ndxVerse, int nBk = -1, int nChp = -1) const;	// Returns index into m_mapVerses based on relative index of Verse for specified Book and/or Book/Chapter
	public:
		int GetVerseCount(int nBk = -1, int nChp = -1) const;
		int GetResultsCount(unsigned int nBk = 0, unsigned int nChp = 0) const;				// Calculates the total number of results from the Results Phrase Tags (can be limited to book or book/chapter)

		const CVerseMap &verseMap() const { return m_mapVerses; }
		const QString resultsName() const { return m_strResultsName; }
		VERSE_LIST_MODEL_RESULTS_TYPE_ENUM resultsType() const { return m_nResultsType; }
		int specialIndex() const { return m_nSpecialIndex; }
		const TVerseIndex makeVerseIndex(const CRelIndex &ndxRel) const
		{
			return TVerseIndex(ndxRel, resultsType(), specialIndex());
		}
	protected:
		TVerseListModelPrivate *m_private;
	private:
		QString m_strResultsName;						// Name of the Highlighter or "Search Results" or "Notes", etc...
		VERSE_LIST_MODEL_RESULTS_TYPE_ENUM m_nResultsType;
		int m_nSpecialIndex;
	};
	typedef QList<TVerseListModelResults> THighlighterVLMRList;

	class TVerseListModelSearchResults : public TVerseListModelResults {
	protected:
		friend class CVerseListModel;

		TVerseListModelSearchResults(TVerseListModelPrivate *priv)
			:	TVerseListModelResults(priv, tr("Search Results"), VLMRTE_SEARCH_RESULTS)
		{ }

		TParsedPhrasesList m_lstParsedPhrases;		// Parsed phrases, updated by KJVCanOpener en_phraseChanged (used to build Search Results and for displaying tooltips)
		CSearchCriteria m_SearchCriteria;			// Search criteria set during setParsedPhrases

		// --------------------------------------

	public:
		QPair<int, int> GetResultsIndexes(CVerseMap::const_iterator itrVerse) const;		// Calculates the starting and ending results indexes for the specified Verse List entry index
		QPair<int, int> GetBookIndexAndCount(CVerseMap::const_iterator itrVerse = CVerseMap::const_iterator()) const;		// Returns the Search Result Book number and total number of books with results
		QPair<int, int> GetChapterIndexAndCount(CVerseMap::const_iterator itrVerse = CVerseMap::const_iterator()) const;	// Returns the Search Result Chapter and total number of chapters with results
		QPair<int, int> GetVerseIndexAndCount(CVerseMap::const_iterator itrVerse = CVerseMap::const_iterator()) const;		// Returns the Search Result Verse and total number of verses with results

		using TVerseListModelResults::GetVerseCount;
		using TVerseListModelResults::verseMap;
	};

	class TVerseListModelNotesResults : public TVerseListModelResults {
	protected:
		friend class CVerseListModel;

		TVerseListModelNotesResults(TVerseListModelPrivate *priv)
			:	TVerseListModelResults(priv, tr("Notes"), VLMRTE_USER_NOTES)
		{ }

		// --------------------------------------
	};

	// ------------------------------------------------------------------------

	CVerseListModel(CBibleDatabasePtr pBibleDatabase, QObject *pParent = 0);

	inline CBibleDatabasePtr bibleDatabase() const { return m_private.m_pBibleDatabase; }

	virtual int rowCount(const QModelIndex &zParent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &zParent = QModelIndex()) const;

	virtual QModelIndex	index(int row, int column = 0, const QModelIndex &zParent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	QVariant dataForVerse(const TVerseIndex *pVerseIndex, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool insertRows(int row, int count, const QModelIndex &zParent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &zParent = QModelIndex());

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	virtual Qt::DropActions supportedDropActions() const;

	QModelIndex locateIndex(const TVerseIndex &ndxVerse) const;
	TVerseIndex resolveVerseIndex(const CRelIndex &ndxRel, const QString &strResultsName, VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType = VLMRTE_UNDEFINED) const;			// Note: Pass strHighlighterName for strResultsName or Empty string for types that use no specialIndex (nResultsType == VLMRTE_UNDEFINED uses ViewMode of model)

	TParsedPhrasesList parsedPhrases() const;
	void setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases);		// Will build verseList and the list of tags so they can be iterated in a highlighter, etc

	VERSE_DISPLAY_MODE_ENUM displayMode() const { return m_private.m_nDisplayMode; }
	VERSE_TREE_MODE_ENUM treeMode() const { return m_private.m_nTreeMode; }
	VERSE_VIEW_MODE_ENUM viewMode() const { return m_private.m_nViewMode; }
	bool showMissingLeafs() const { return m_private.m_bShowMissingLeafs; }

// TODO : CLEAN
//	const TVerseListModelResults &results(int ndxResults) const
//	{
//		if (ndxResults == VLM_HI_UNDEFINED) return m_undefinedResults;
//		if (ndxResults == VLM_HI_SEARCH_RESULTS) return m_searchResults;
//		if (ndxResults == VLM_HI_USER_NOTES) return m_userNotesResults;
//		assert((ndxResults >= 0) && (ndxResults < m_vlmrListHighlighters.size()));
//		return m_vlmrListHighlighters.at(ndxResults);
//	}

	const TVerseListModelResults &results(VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType, int nSpecialIndex) const
	{
		switch (nResultsType) {
			case VLMRTE_UNDEFINED:
				return m_undefinedResults;
			case VLMRTE_SEARCH_RESULTS:
				return m_searchResults;
			case VLMRTE_USER_NOTES:
				return m_userNotesResults;
			case VLMRTE_HIGHLIGHTERS:
				break;				// Fall through and lookup
			default:
				assert(false);
		}

		assert((nSpecialIndex >= 0) && (nSpecialIndex < m_vlmrListHighlighters.size()));
		return m_vlmrListHighlighters.at(nSpecialIndex);
	}
	const TVerseListModelResults &results(const TVerseIndex &ndxVerse) const
	{
		return results(ndxVerse.resultsType(), ndxVerse.specialIndex());
	}
	const TVerseListModelResults &results(const QModelIndex &index) const
	{
		// Note: Invalid QModelIndex() will have a results type of undefined.  Only those types with
		//			Pseudo-top-labels, like highlighters, will use the undefinedResults to complete them.
		//			The rest will use the results for the model's current mode.  As more things are added
		//			with pseudo-labels using the undefinedResults, add additional logic here for them:
		TVerseIndex *pVerseIndex = toVerseIndex(index);
		if (m_private.m_nViewMode != VVME_HIGHLIGHTERS) {
			return results(VVME_to_VLMRTE(m_private.m_nViewMode), pVerseIndex->specialIndex());
		}

		return results(*toVerseIndex(index));
	}
	const TVerseListModelSearchResults &searchResults() const { return m_searchResults; }

	inline const QFont &font() const { return m_private.m_font; }

signals:
	void cachedSizeHintsInvalidated();
	void verseListAboutToChange();					// Emitted just before our verse phrase tags change so that users (KJVBrowser, etc) can clear highlighting with the old tags
	void verseListChanged();						// Emitted just after our verse phrase tags change so that users (KJVBrowser, etc) can set highlighting with the new tags

public slots:
	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode);
	void setShowMissingLeafs(bool bShowMissing);
	virtual void setFont(const QFont& aFont);

protected slots:
	void en_WordsOfJesusColorChanged(const QColor &color);

	void en_highlighterTagsChanged(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName);
	void en_changedHighlighters();

	void en_changedUserNote(const CRelIndex &ndx);
	void en_addedUserNote(const CRelIndex &ndx);
	void en_removedUserNote(const CRelIndex &ndx);

public:
	// Total Verse/Result count for the whole model for the current mode:
	int GetVerseCount(int nBk = -1, int nChp = -1) const;
	int GetResultsCount(unsigned int nBk = 0, unsigned int nChp = 0) const;

private:
	void clearAllSizeHints();
	void clearAllExtraVerseIndexes();

	void buildScopedResultsFromParsedPhrases();
	CRelIndex ScopeIndex(const CRelIndex &index, CSearchCriteria::SEARCH_SCOPE_MODE_ENUM nMode);

	void buildHighlighterResults(int ndxHighlighter = -1);								// Note: index of -1 = All Highlighters
	void buildHighlighterResults(int ndxHighlighter, const TPhraseTagList *pTags);		// Here, ndxHighlighter must NOT be -1 !!

	void buildUserNotesResults(const CRelIndex &ndx, bool bAdd);

private:
	Q_DISABLE_COPY(CVerseListModel)
	TVerseListModelPrivate m_private;

	THighlighterVLMRList m_vlmrListHighlighters;	// Per-Highlighter VerseListModelResults
	TVerseListModelResults m_undefinedResults;		// VerseListModelResults for Undefined Results -- Used for generating extraVerseIndexes for parent entries where QModelIndex->NULL
	TVerseListModelSearchResults m_searchResults;	// VerseListModelResults for Search Results
	TVerseListModelNotesResults m_userNotesResults;	// VerseListModelResults for User Notes
};

// ============================================================================

#endif // VERSELISTMODEL_H
