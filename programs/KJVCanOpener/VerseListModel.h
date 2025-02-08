/****************************************************************************
**
** Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef VERSE_LIST_MODEL_H
#define VERSE_LIST_MODEL_H

#include "dbstruct.h"
#include "PhraseParser.h"
#include "SearchCriteria.h"
#include "VerseRichifier.h"
#include "UserNotesDatabase.h"
#include "PersistentSettings.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPersistentModelIndex>
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
#include <QMimeData>
#include <QPointer>

#include <algorithm>

// ============================================================================

// Verse List Model Results Type Enum:
//		Note: Either keep this in-sync with VERSE_VIEW_MODE_ENUM or
//				update VVME_to_VLMRTE and VLMRTE_to_VVME functions:
enum VERSE_LIST_MODEL_RESULTS_TYPE_ENUM {
	VLMRTE_UNDEFINED = -1,							// Undefined Index (used for defaults)
	VLMRTE_SEARCH_RESULTS = 0,						// Search Results Display Index
	VLMRTE_HIGHLIGHTERS = 1,						// Highlighter Display Index
	VLMRTE_USER_NOTES = 2,							// User Notes Display Index
	VLMRTE_CROSS_REFS = 3,							// Cross References Index
	VLMRTE_SEARCH_RESULTS_EXCLUDED = 4				// Excluded Search Results Display Index
};

// Verse List Model Node Type Enum:
enum VERSE_LIST_MODEL_NODE_TYPE_ENUM {
	VLMNTE_UNDEFINED = -1,							// Undefined/Unset Node
	VLMNTE_TESTAMENT_TERMINATOR_NODE = 0,			// Terminator Node for Tree Testament nodes
	VLMNTE_CATEGORY_TERMINATOR_NODE = 1,			// Terminator Node for Tree Category nodes
	VLMNTE_BOOK_TERMINATOR_NODE = 2,				// Terminator Node for Tree Book nodes
	VLMNTE_CHAPTER_TERMINATOR_NODE = 3,				// Terminator Node for Tree Chapter nodes
	VLMNTE_VERSE_TERMINATOR_NODE = 4,				// Terminator Node for Tree Verse nodes
	VLMNTE_CROSS_REFERENCE_SOURCE_NODE = 5,			// Source Reference Node for Tree Cross References
	VLMNTE_CROSS_REFERENCE_TARGET_NODE = 6,			// Target Reference Node for Tree Cross References
	VLMNTE_HIGHLIGHTER_NODE = 7						// Highlighter Name Node
};

class TVerseIndex {
public:
	TVerseIndex(const CRelIndex &ndx = CRelIndex(),
				VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultType = VLMRTE_UNDEFINED,
				VERSE_LIST_MODEL_NODE_TYPE_ENUM nNodeType = VLMNTE_UNDEFINED,
				int nSpecialIndex = -1)
		:	m_nRelIndex(ndx),
			m_nResultsType(nResultType),
			m_nNodeType(nNodeType),
			m_nSpecialIndex(nSpecialIndex)
	{ }

	TVerseIndex(const TVerseIndex &aVerseIndex)
		:	m_nRelIndex(aVerseIndex.m_nRelIndex),
			m_nResultsType(aVerseIndex.m_nResultsType),
			m_nNodeType(aVerseIndex.m_nNodeType),
			m_nSpecialIndex(aVerseIndex.m_nSpecialIndex)
	{ }

	const CRelIndex &relIndex() const { return m_nRelIndex; }
	VERSE_LIST_MODEL_RESULTS_TYPE_ENUM resultsType() const { return m_nResultsType; }
	VERSE_LIST_MODEL_NODE_TYPE_ENUM nodeType() const { return m_nNodeType; }
	int specialIndex() const { return m_nSpecialIndex; }

	bool operator <(const TVerseIndex &other) const
	{
		return ((m_nResultsType < other.m_nResultsType) ||
				((m_nResultsType == other.m_nResultsType) && (m_nSpecialIndex < other.m_nSpecialIndex)) ||
				((m_nResultsType == other.m_nResultsType) && (m_nSpecialIndex == other.m_nSpecialIndex) && (m_nNodeType < other.m_nNodeType)) ||
				((m_nResultsType == other.m_nResultsType) && (m_nSpecialIndex == other.m_nSpecialIndex) && (m_nNodeType == other.m_nNodeType) && (m_nRelIndex < other.m_nRelIndex)));
	}

protected:
	friend class CVerseListModel;

	CRelIndex m_nRelIndex;					// Relative Bible index
	VERSE_LIST_MODEL_RESULTS_TYPE_ENUM m_nResultsType;		// Type of index this is (i.e. which Search Results structure)
	VERSE_LIST_MODEL_NODE_TYPE_ENUM m_nNodeType;			// Type of node this is (view node type)
	int m_nSpecialIndex;				// Index into list of highlighters (0 to n) or -1 for Results Types that don't use indexes
};
typedef QSharedPointer<TVerseIndex> TVerseIndexPtr;
typedef QList<TVerseIndex> TVerseIndexList;

struct TExtraVerseIndexKey
{
	TExtraVerseIndexKey(const CRelIndex &ndxVerse, VERSE_LIST_MODEL_NODE_TYPE_ENUM nNodeType)
		:	m_nRelIndex(ndxVerse),
			m_nNodeType(nNodeType)
	{ }

	bool operator <(const TExtraVerseIndexKey &other) const
	{
		return ((m_nNodeType < other.m_nNodeType) ||
				((m_nNodeType == other.m_nNodeType) && (m_nRelIndex < other.m_nRelIndex)));
	}

	CRelIndex m_nRelIndex;
	VERSE_LIST_MODEL_NODE_TYPE_ENUM m_nNodeType;
};
typedef QMap<TExtraVerseIndexKey, TVerseIndexPtr> TExtraVerseIndexPtrMap;

struct TVerseIndexListSortPredicate {
	static bool ascendingLessThan(const TVerseIndex &s1, const TVerseIndex &s2)
	{
		return (s1 < s2);
	}
};

// ============================================================================

class CSearchResultsData
{
public:
	// For SearchResults, this list will be the Included phrases.  For
	//		ExcludedSearchResults, this list will be the Excluded phrases:
	CSearchCriteria m_SearchCriteria;			// Search criteria set at the time of generating these results
	TParsedPhrasesList m_lstParsedPhrases;		// Pointer to Parsed phrases
};

// ============================================================================

// Forward declarations
class CBasicHighlighter;

class CVerseListItem
{
public:
	explicit CVerseListItem()
	{

	}

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

	inline QString getHeading(bool bForCopying = false) const {		// bForCopying = false for Showing/Displaying the heading, true for Copy mode
		Q_ASSERT(!m_pBibleDatabase.isNull());
		if (m_pBibleDatabase.isNull()) return QString();
		bool bSearchRefs = ((verseIndex()->resultsType() == VLMRTE_SEARCH_RESULTS) ||
							(verseIndex()->resultsType() == VLMRTE_SEARCH_RESULTS_EXCLUDED));		// For Search Results, show word positions too
		QString strHeading;
		if ((m_lstTags.size() > 0) &&
			(verseIndex()->resultsType() != VLMRTE_USER_NOTES) &&
			(verseIndex()->resultsType() != VLMRTE_CROSS_REFS)) {
			if (((bForCopying) && (CPersistentSettings::instance()->copyOCntInSearchResultsRefs())) ||
				((!bForCopying) && (CPersistentSettings::instance()->showOCntInSearchResultsRefs()))) {
				strHeading += QString("(%1) ").arg(m_lstTags.size());
			}
			for (int ndx = 0; ndx < m_lstTags.size(); ++ndx) {
				if (ndx == 0) {
					CRelIndex relNdxTag;
					bool bCopyWrdNdx = true;
					if (bSearchRefs) {
						relNdxTag = m_lstTags.at(ndx).relIndex();
					} else {
						relNdxTag = getIndex();
					}
					if ((!bSearchRefs) ||
						((bForCopying) && (!CPersistentSettings::instance()->copyWrdNdxInSearchResultsRefs())) ||
						((!bForCopying) && (!CPersistentSettings::instance()->showWrdNdxInSearchResultsRefs()))) {
						if (relNdxTag.verse() != 0) relNdxTag.setWord(0);
						bCopyWrdNdx = false;
					}
					strHeading += m_pBibleDatabase->PassageReferenceText(relNdxTag, !bCopyWrdNdx);
				} else {
					if ((bSearchRefs) &&
						(((bForCopying) && (CPersistentSettings::instance()->copyWrdNdxInSearchResultsRefs())) ||
						((!bForCopying) && (CPersistentSettings::instance()->showWrdNdxInSearchResultsRefs())))) {
						strHeading += QString("[%1]").arg(m_lstTags.at(ndx).relIndex().word());
					}
				}
			}
		} else {
			CRelIndex ndxRel = getIndex();
			if ((verseIndex()->resultsType() == VLMRTE_USER_NOTES) ||
				(verseIndex()->resultsType() == VLMRTE_CROSS_REFS)) ndxRel.setWord(0);		// Note we don't allow notes/cross-refs on colophons/superscriptions
			strHeading += m_pBibleDatabase->PassageReferenceText(ndxRel);
		}
		return strHeading;
	}

	inline QString getToolTip(const CSearchResultsData &searchResultsData) const {
		Q_ASSERT(!m_pBibleDatabase.isNull());
		if (m_pBibleDatabase.isNull()) return QString();
		QString strToolTip;
		strToolTip += m_pBibleDatabase->SearchResultToolTip(getIndex(), RIMASK_BOOK | RIMASK_CHAPTER | RIMASK_VERSE);
		for (int ndx = 0; ndx < phraseTags().size(); ++ndx) {
			const CRelIndex &ndxTag(phraseTags().at(ndx).relIndex());
			if (phraseTags().size() > 1) {
				strToolTip += QString("(%1)[%2] \"%3\" %4 ").arg(ndx+1).arg(ndxTag.word()).arg(getPhrase(ndx)).arg(QObject::tr("is", "Statistics"));
			} else {
				strToolTip += QString("[%1] \"%2\" %3 ").arg(ndxTag.word()).arg(getPhrase(ndx)).arg(QObject::tr("is", "Statistics"));
			}
			strToolTip += m_pBibleDatabase->SearchResultToolTip(ndxTag, RIMASK_WORD);
			for (int ndxPhrase = 0; ndxPhrase < searchResultsData.m_lstParsedPhrases.size(); ++ndxPhrase) {
				const CParsedPhrase *pPhrase = searchResultsData.m_lstParsedPhrases.at(ndxPhrase);
				// Note: This CAN legitimately be NULL if we are intentionally dumping our SearchPhrase
				//		data.  See CWebChannelObjects::en_searchResultsReady() and the clearing of the
				//		Parsed Phrases in order to free memory for additional users.  So don't assert here:
				//Q_ASSERT(pPhrase != nullptr);
				if (pPhrase == nullptr) continue;
				if (verseIndex()->resultsType() != VLMRTE_SEARCH_RESULTS_EXCLUDED) {
					QString strSearchWithinDescription = searchResultsData.m_SearchCriteria.searchWithinDescription(m_pBibleDatabase);
					QString strSearchScopeDescription = searchResultsData.m_SearchCriteria.searchScopeDescription();
					if (pPhrase->GetPhraseTagSearchResults().contains(phraseTags().at(ndx))) {
						strToolTip += "    " + QObject::tr("%1 of %2 of Search Phrase \"%3\" Results in Entire Bible", "Statistics")
											.arg(pPhrase->GetPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
											.arg(pPhrase->GetPhraseTagSearchResults().size())
											.arg(pPhrase->phrase()) + "\n";
					}
					if ((!searchResultsData.m_SearchCriteria.withinIsEntireBible(m_pBibleDatabase)) &&
						(!strSearchWithinDescription.isEmpty()) &&
						(pPhrase->GetWithinPhraseTagSearchResults().contains(phraseTags().at(ndx)))) {
						strToolTip += "    " + QObject::tr("%1 of %2 of Search Phrase \"%3\" Results within %4", "Statistics")
											.arg(pPhrase->GetWithinPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
											.arg(pPhrase->GetWithinPhraseTagSearchResults().size())
											.arg(pPhrase->phrase())
											.arg(strSearchWithinDescription) + "\n";
					}
					if (strSearchScopeDescription.isEmpty()) strSearchScopeDescription = QObject::tr("in Search Scope", "Statistics");
					if ((pPhrase->GetScopedPhraseTagSearchResults().contains(phraseTags().at(ndx))) &&
						(searchResultsData.m_SearchCriteria.searchScopeMode() != CSearchCriteria::SSME_UNSCOPED) &&
						(!strSearchScopeDescription.isEmpty())) {
						strToolTip += "    " + QObject::tr("%1 of %2 of Search Phrase \"%3\" Results %4", "Statistics")
											.arg(pPhrase->GetScopedPhraseTagSearchResults().indexOf(phraseTags().at(ndx)) + 1)
											.arg(pPhrase->GetScopedPhraseTagSearchResults().size())
											.arg(pPhrase->phrase())
											.arg(strSearchScopeDescription) + "\n";
					}
				} else {
					int nResultsIndex = pPhrase->GetPhraseTagSearchResults().findIntersectingIndex(m_pBibleDatabase.data(), phraseTags().at(ndx));
					if (nResultsIndex != -1) {
						strToolTip += "    " + QObject::tr("%1 of %2 of Excluded Search Phrase \"%3\" Results in Entire Bible", "Statistics")
											.arg(nResultsIndex + 1)
											.arg(pPhrase->GetPhraseTagSearchResults().size())
											.arg(pPhrase->phrase()) + "\n";
					}
					if (!searchResultsData.m_SearchCriteria.withinIsEntireBible(m_pBibleDatabase)) {
						int nResultsWithinIndex = pPhrase->GetWithinPhraseTagSearchResults().findIntersectingIndex(m_pBibleDatabase.data(), phraseTags().at(ndx));
						if (nResultsWithinIndex != -1) {
							strToolTip += "    " + QObject::tr("%1 of %2 of Excluded Search Phrase \"%3\" Results in Selected Search Text", "Statistics")
												.arg(nResultsWithinIndex + 1)
												.arg(pPhrase->GetWithinPhraseTagSearchResults().size())
												.arg(pPhrase->phrase()) + "\n";
						}
					}
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
		Q_ASSERT(!m_pBibleDatabase.isNull());
		if (m_pBibleDatabase.isNull()) return 0;
		return m_pBibleDatabase->NormalizeIndex(m_pVerseIndex->relIndex());
	}
	inline uint32_t getIndexDenormalized() const { return m_pVerseIndex->relIndex().index(); }
	inline CRelIndex getIndex() const { return m_pVerseIndex->relIndex(); }
	inline TPhraseTag getWholeVersePhraseTag() const
	{
		return TPhraseTag(m_pVerseIndex->relIndex(), m_pBibleDatabase->verseEntry(m_pVerseIndex->relIndex())->m_nNumWrd);
	}
	inline unsigned int getPhraseSize(int nTag) const {
		Q_ASSERT((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((nTag < 0) || (nTag >= m_lstTags.size())) return 0;
		return m_lstTags.at(nTag).count();
	}
	inline CRelIndex getPhraseReference(int nTag) const {
		Q_ASSERT((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((nTag < 0) || (nTag >= m_lstTags.size())) return CRelIndex();
		return m_lstTags.at(nTag).relIndex();
	}
	void addPhraseTag(const CRelIndex &ndx, unsigned int nPhraseSize) { m_lstTags.push_back(TPhraseTag(ndx, nPhraseSize)); }
	void addPhraseTag(const TPhraseTag &tag) { m_lstTags.push_back(tag); }
	const TPhraseTagList &phraseTags() const { return m_lstTags; }

	QStringList getWordList(int nTag) const
	{
		Q_ASSERT(!m_pBibleDatabase.isNull());
		if (m_pBibleDatabase.isNull()) return QStringList();
		Q_ASSERT((nTag >= 0) && (nTag < m_lstTags.size()));
		if ((!isSet()) || (nTag < 0) || (nTag >= m_lstTags.size())) return QStringList();
		QStringList strWords;
		unsigned int nNumWords = m_lstTags.at(nTag).count();
		uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(m_lstTags.at(nTag).relIndex());
		while (nNumWords) {
			strWords.push_back(m_pBibleDatabase->wordAtIndex(ndxNormal, WTE_RENDERED));
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
		return getVerseAsWordList(getIndex(), m_pBibleDatabase);
	}
	static QStringList getVerseAsWordList(const CRelIndex &ndx, CBibleDatabasePtr pBibleDatabase)
	{
		Q_ASSERT(!pBibleDatabase.isNull());
		if (pBibleDatabase.isNull()) return QStringList();
		if (!ndx.isSet()) return QStringList();
		QStringList strWords;
		const CVerseEntry *pVerseEntry = pBibleDatabase->verseEntry(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), 0));
		unsigned int nNumWords = (pVerseEntry ? pVerseEntry->m_nNumWrd : 0);
		uint32_t ndxNormal = pBibleDatabase->NormalizeIndex(ndx);
		while (nNumWords) {
			strWords.push_back(pBibleDatabase->wordAtIndex(ndxNormal, WTE_RENDERED));
			ndxNormal++;
			nNumWords--;
		}
		return strWords;
	}

	QString getVerseVeryPlainText() const		// Very Plain has no punctuation!
	{
		return getVerseAsWordList().join(" ");
	}
	static QString getVerseVeryPlainText(const CRelIndex &ndx, CBibleDatabasePtr pBibleDatabase)
	{
		return getVerseAsWordList(ndx, pBibleDatabase).join(" ");
	}

	QString getVersePlainText(bool bSuppressHeadings) const			// Plain (with punctuation)
	{
		CVerseTextPlainRichifierTags plainRichifierTags;
		return getVerseRichText(plainRichifierTags, bSuppressHeadings);
	}

	QString getVerseRichText(const CVerseTextRichifierTags &richifierTags,
								bool bSuppressHeadings,
								const CBasicHighlighter *pSRHighlighter = nullptr,
								const CBasicHighlighter *pSRExclHighlighter = nullptr) const
	{
		QString strVerseRichText;
		CRelIndex ndxCurrent = getIndex();
		bool bExtended = false;			// True if result extends to multiple verses
		do {
			if (bExtended) {
				if (richifierTags.usesHTML()) {
					strVerseRichText += QString("&nbsp;&nbsp;");
				} else {
					strVerseRichText += QString("  ");
				}

				if (!bSuppressHeadings) {
					if ((ndxCurrent.book() == getIndex().book()) && (ndxCurrent.chapter() == getIndex().chapter()) &&
						(ndxCurrent.verse() != 0)) {
						strVerseRichText += QString("(%1) ").arg(ndxCurrent.verse());
					} else if ((ndxCurrent.book() == getIndex().book()) && (ndxCurrent.chapter() != 0) && (ndxCurrent.verse() != 0)) {
						strVerseRichText += QString("(%1:%2) ").arg(ndxCurrent.chapter()).arg(ndxCurrent.verse());
					} else {
						CRelIndex ndxPrint = ndxCurrent;
						if ((ndxPrint.chapter() != 0) && (ndxPrint.verse() != 0)) ndxPrint.setWord(0);
						strVerseRichText += QString("(%1) ").arg(m_pBibleDatabase->PassageReferenceText(ndxPrint, true));
					}
				}
			}
			strVerseRichText += getVerseRichText(ndxCurrent, m_pBibleDatabase, richifierTags, pSRHighlighter, pSRExclHighlighter);
			ndxCurrent = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, ndxCurrent, false);		// See if next verse intersects our results of this verse (i.e. spills to next verse)
			bExtended = true;
		} while (m_lstTags.intersects(m_pBibleDatabase.data(), TPhraseTag(ndxCurrent)));

		return strVerseRichText;
	}
	static QString getVerseRichText(const CRelIndex &ndx, CBibleDatabasePtr pBibleDatabase,
									const CVerseTextRichifierTags &richifierTags,
									const CBasicHighlighter *pSRHighlighter = nullptr,
									const CBasicHighlighter *pSRExclHighlighter = nullptr)
	{
		Q_ASSERT(!pBibleDatabase.isNull());
		if (pBibleDatabase.isNull()) return QString();
		if (!ndx.isSet()) return QString();
		return pBibleDatabase->richVerseText(ndx, richifierTags, RichifierRenderOptionFlags(), pSRHighlighter, pSRExclHighlighter);
	}

	void sortPhraseTags()
	{
		std::sort(m_lstTags.begin(), m_lstTags.end(), TPhraseTagListSortPredicate::ascendingLessThan);
	}

private:
	CBibleDatabasePtr m_pBibleDatabase;
	TPhraseTagList m_lstTags;		// Phrase Tags to highlight in this object

	TVerseIndexPtr m_pVerseIndex;	// Used for QModelIndex Internal object
};

Q_DECLARE_METATYPE(CVerseListItem)

typedef QList<CVerseListItem> CVerseList;
extern void sortVerseList(CVerseList &aVerseList, Qt::SortOrder order);

typedef QMap<CRelIndex, CVerseListItem> CVerseMap;

// ============================================================================

// Forward Declaration:
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
class CThreadedSearchResultCtrl;
#endif

class CSearchResultsProcess : private CSearchResultsData
{
public:
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	CSearchResultsProcess(CBibleDatabasePtr pBibleDatabase, const CSearchResultsData &searchResultsData);
#endif
	CSearchResultsProcess(CBibleDatabasePtr pBibleDatabase, const CSearchResultsData &searchResultsData, CVerseMap *pMapVersesIncl, CVerseMap *pMapVersesExcl);
	void commonConstruct();

	bool canCopyBack() const;		// Returns true if the copied phrases match the source (i.e. data hasn't changed)
	void copyBackInclusionData(CSearchResultsData &searchResultsData, CVerseMap &mapVerseData, QList<CRelIndex> &lstVerseIndexes) const;
	void copyBackExclusionData(CSearchResultsData &searchResultsData, CVerseMap &mapVerseData, QList<CRelIndex> &lstVerseIndexes) const;

	void buildScopedResultsFromParsedPhrases();

protected:
	bool checkExclusion(QList<TPhraseTagList::const_iterator> &lstItrExclNext, const TPhraseTag &tag, bool bPreserveLastItr = false);			// Finds next possible intersection of the specified tag in the m_searchResultsExcluded indexed with the list of iterators
	void buildWithinResultsInParsedPhrase(const CParsedPhrase *pParsedPhrase) const;
	CRelIndex ScopeIndex(const CRelIndex &index);

private:
	CBibleDatabasePtr m_pBibleDatabase;
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	TSharedParsedPhrasesList m_lstCopyParsedPhrasesIncl;		// Copy of Parsed Phrases for inclusions
	TSharedParsedPhrasesList m_lstCopyParsedPhrasesExcl;		// Copy of Parsed Phrases for exclusions
#endif
	TParsedPhrasesList m_lstParsedPhrasesIncl;	// Pointer to Phrases for inclusions
	TParsedPhrasesList m_lstParsedPhrasesExcl;	// Pointer to Phrases for exclusions
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	CVerseMap m_mapVersesInclLocal;				// Map of Included Verse Search Results by CRelIndex [nBk|nChp|nVrs|0].  Set in buildScopedResultsFromParsedPhrases()
	CVerseMap m_mapVersesExclLocal;				// Map of Excluded Verse Search Results by CRelIndex [nBk|nChp|nVrs|0].  Set in buildScopedResultsFromParsedPhrases()
#endif
	CVerseMap *m_pMapVersesIncl;				// Map of Included Verse Search Results by CRelIndex [nBk|nChp|nVrs|0].  Set in buildScopedResultsFromParsedPhrases()
	CVerseMap *m_pMapVersesExcl;				// Map of Excluded Verse Search Results by CRelIndex [nBk|nChp|nVrs|0].  Set in buildScopedResultsFromParsedPhrases()
};

class CVerseListModel : public QAbstractItemModel
{
	Q_OBJECT
public:

	// Note: Don't change the order of these enums.  Doing so will break
	//			preserved settings save/restore, etc, making them incompatible:

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
		VVME_SEARCH_RESULTS = 0,			// Display Search Results in the Tree View
		VVME_HIGHLIGHTERS = 1,				// Display Tree of Highlighter Tags
		VVME_USERNOTES = 2,					// Display Tree of User Notes
		VVME_CROSSREFS = 3,					// Display Tree of User Cross References
		VVME_SEARCH_RESULTS_EXCLUDED = 4	// Display Excluded Search Results in the Tree View
	};
	static VERSE_LIST_MODEL_RESULTS_TYPE_ENUM VVME_to_VLMRTE(VERSE_VIEW_MODE_ENUM nViewMode)
	{
		return static_cast<VERSE_LIST_MODEL_RESULTS_TYPE_ENUM>(nViewMode);
	}
	static VERSE_VIEW_MODE_ENUM VLMRTE_to_VVME(VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType)
	{
		Q_ASSERT(nResultsType != VLMRTE_UNDEFINED);
		return static_cast<VERSE_VIEW_MODE_ENUM>(nResultsType);
	}

	enum VERSE_DATA_ROLES_ENUM {
		VERSE_ENTRY_ROLE = Qt::UserRole + 0,					// Return CVerseListItem object
		VERSE_COPYING_ROLE = Qt::UserRole + 1,					// Same as Qt::DisplayRole, but for Copying mode (i.e. copy options used in generation)
		TOOLTIP_ROLE = Qt::UserRole + 2,						// Use our own ToolTip role so we can have user click Ctrl-D to see details ToolTip.  Qt::ToolTip will be a message telling them to do that.
		TOOLTIP_PLAINTEXT_ROLE = Qt::UserRole + 3,				// Same as TOOLTIP_ROLE, but as PlainText instead of RichText
		TOOLTIP_NOHEADING_ROLE = Qt::UserRole + 4,				// Same as TOOLTIP_ROLE, but without Verse Reference Heading
		TOOLTIP_NOHEADING_PLAINTEXT_ROLE = Qt::UserRole + 5,	// Same as TOOLTIP_PLAINTEXT_ROLE, but without Verse Reference Heading
		TOOLTIP_GEMATRIA_ROLE = Qt::UserRole + 6,				// Gematria ToolTip role
	};

	// ------------------------------------------------------------------------

	class TVerseListModelPrivate {
	public:
		TVerseListModelPrivate(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase);

		CBibleDatabasePtr m_pBibleDatabase;
		CUserNotesDatabasePtr m_pUserNotesDatabase;
		VERSE_DISPLAY_MODE_ENUM m_nDisplayMode;		// Headings vs RichText, etc
		VERSE_TREE_MODE_ENUM m_nTreeMode;			// List, Tree by Books, Tree by Chapters, etc.
		VERSE_VIEW_MODE_ENUM m_nViewMode;			// Search Results vs Highlighters, etc
		bool m_bShowMissingLeafs;					// Shows the missing leafs in book or book/chapter modes
		bool m_bShowHighlightersInSearchResults;	// True if VerseDelegate will paint highlighters in Search Results verses
		CRelIndex m_ndxSingleCrossRefSource;		// If Set, will be in special Cross-Reference Display mode, which is limited to this single source reference (used for the Cross-Reference Editor)
		CVerseTextRichifierTags m_richifierTagsDisplay;	// Richifier tags used to render the results in this list for display
		CVerseTextRichifierTags m_richifierTagsCopying;	// Richifier tags used to render the results in this list for copying
		QFont m_font;								// Normally we wouldn't keep this here in the model, but this is directly accessible to the delegate showing us and we have to trigger the model anyway to update sizeHints()
	};

	// ------------------------------------------------------------------------

	static TVerseIndex *toVerseIndex(const QModelIndex &ndx) {
		static TVerseIndex nullVerseIndex;				// Empty VerseIndex to use for parent entries where QModelIndex->NULL
		return ((ndx.internalPointer() != nullptr) ? reinterpret_cast<TVerseIndex *>(ndx.internalPointer()) : &nullVerseIndex);
	}
	static void *fromVerseIndex(const TVerseIndex *ndx) { return reinterpret_cast<void *>(const_cast<TVerseIndex *>(ndx)); }

	// Data for one parsed TPhraseTagList (Used one for Search Results and one for each Highlighter)
	class TVerseListModelResults {
	public:
		TVerseListModelResults(const TVerseListModelResults &other) = default;
		TVerseListModelResults& operator=(const TVerseListModelResults &other) = default;

	protected:
		friend class CVerseListModel;

		TVerseListModelResults(TVerseListModelPrivate *priv,
								const QString &strResultsName,
								VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType,
								VERSE_LIST_MODEL_NODE_TYPE_ENUM nDefaultNodeType = VLMNTE_UNDEFINED,
								int nSpecialIndex = -1)
			:	m_private(priv),
				m_strResultsName(strResultsName),
				m_nResultsType(nResultsType),
				m_nDefaultNodeType(nDefaultNodeType),
				m_nSpecialIndex(nSpecialIndex)
		{ }

		CVerseMap m_mapVerses;						// Map of Verse Search Results by CRelIndex [nBk|nChp|nVrs|0].  Set in buildScopedResultsFromParsedPhrases()
		QList<CRelIndex> m_lstVerseIndexes;			// List of CRelIndexes in CVerseMap -- needed because index lookup within the QMap is time-expensive
		mutable TExtraVerseIndexPtrMap m_mapExtraVerseIndexes;	// Used to store VerseIndex objects we give out for items with no data, like Book/Chapter headings (cleared in buildScopedResultsFromParsedPhrases() and created on demand).  Objects we give out are in CVerseListModel.
		QMap<QPersistentModelIndex, QSize> m_mapSizeHints;	// Map of QModelIndex to SizeHint -- used for ReflowDelegate caching (Note: This only needs to be cleared if we change databases or display modes!)

		// --------------------------------------

		TVerseIndexPtr extraVerseIndex(const TVerseIndex &aVerseIndex) const
		{
			TExtraVerseIndexKey keyExtraVerse(aVerseIndex.relIndex(), aVerseIndex.nodeType());
			TExtraVerseIndexPtrMap::const_iterator itr = m_mapExtraVerseIndexes.constFind(keyExtraVerse);
			if (itr == m_mapExtraVerseIndexes.constEnd()) itr = m_mapExtraVerseIndexes.constFind(TExtraVerseIndexKey(aVerseIndex.relIndex(), VLMNTE_UNDEFINED));
			if (itr != m_mapExtraVerseIndexes.constEnd()) return itr.value();

			return m_mapExtraVerseIndexes.insert(keyExtraVerse, TVerseIndexPtr(new TVerseIndex(aVerseIndex))).value();
		}

		TVerseIndexPtr extraVerseIndex(const CRelIndex &ndxRel, VERSE_LIST_MODEL_NODE_TYPE_ENUM nNodeType = VLMNTE_UNDEFINED) const
		{
			return extraVerseIndex(makeVerseIndex(ndxRel, nNodeType));
		}

		// --------------------------------------

		int GetBookCount() const;						// Returns the number of books in the model based on mode
		int IndexByBook(unsigned int nBk) const;		// Returns the index (in the number of books) for the specified Book number
		unsigned int BookByIndex(int ndxBook) const;	// Returns the Book Number for the specified index (in the number of books)
		int GetChapterCount(unsigned int nBk) const;	// Returns the number of chapters in the specified book number based on the current mode
		int IndexByChapter(unsigned int nBk, unsigned int nChp) const;	// Returns the index (in the number of chapters) for the specified Chapter number
		unsigned int ChapterByIndex(int ndxBook, int ndxChapter) const;		// Returns the Chapter Number for the specified index (in the number of chapters)
		CVerseMap::const_iterator FindVerseIndex(const CRelIndex &ndxRel) const;	// Looks for the specified CRelIndex in m_mapVerses and returns its index
		int IndexByVerse(const CRelIndex &ndxRel) const;	// Returns the index in the list for specified CRelIndex in m_mapVerses
		CVerseMap::const_iterator GetVerse(int ndxVerse, int nBk = -1, int nChp = -1) const;	// Returns index into m_mapVerses based on relative index of Verse for specified Book and/or Book/Chapter
	public:
		int GetVerseCount(int nBk = -1, int nChp = -1) const;
		int GetResultsCount(int nBk = -1, int nChp = -1) const;				// Calculates the total number of results from the Results Phrase Tags (can be limited to book or book/chapter)

		const CVerseMap &verseMap() const { return m_mapVerses; }
		const QString resultsName() const { return m_strResultsName; }
		VERSE_LIST_MODEL_RESULTS_TYPE_ENUM resultsType() const { return m_nResultsType; }
		VERSE_LIST_MODEL_NODE_TYPE_ENUM defaultNodeType() const { return m_nDefaultNodeType; }
		int specialIndex() const { return m_nSpecialIndex; }
		const TVerseIndex makeVerseIndex(const CRelIndex &ndxRel, VERSE_LIST_MODEL_NODE_TYPE_ENUM nNodeTypeOverride = VLMNTE_UNDEFINED) const
		{
			return TVerseIndex(ndxRel, resultsType(), ((nNodeTypeOverride != VLMNTE_UNDEFINED) ? nNodeTypeOverride : defaultNodeType()), specialIndex());
		}
	protected:
		TVerseListModelPrivate *m_private;
	private:
		QString m_strResultsName;						// Name of the Highlighter or "Search Results" or "Notes", etc...
		VERSE_LIST_MODEL_RESULTS_TYPE_ENUM m_nResultsType;
		VERSE_LIST_MODEL_NODE_TYPE_ENUM m_nDefaultNodeType;
		int m_nSpecialIndex;
	};
	typedef QList<TVerseListModelResults> THighlighterVLMRList;

	class TVerseListModelSearchResults : public TVerseListModelResults {
	protected:
		friend class CVerseListModel;

		TVerseListModelSearchResults(TVerseListModelPrivate *priv, bool bExcluded)
			:	TVerseListModelResults(priv, (!bExcluded ? tr("Search Results", "MainMenu") : tr("Excluded Search Results", "MainMenu")), (!bExcluded ? VLMRTE_SEARCH_RESULTS : VLMRTE_SEARCH_RESULTS_EXCLUDED))
		{ }

		// For SearchResults, this list will be the Included phrases.  For
		//		ExcludedSearchResults, this list will be the Excluded phrases:
		CSearchResultsData m_searchResultsData;				// Set via setParsedPhrases via KJVCanOpener en_phraseChanged (used to build Search Results and for displaying tooltips)

		// --------------------------------------

	public:
		QPair<int, int> GetResultsIndexes(CVerseMap::const_iterator itrVerse) const;		// Calculates the starting and ending results indexes for the specified Verse List entry index
		QPair<int, int> GetBookIndexAndCount(CVerseMap::const_iterator itrVerse) const;		// Returns the Search Result Book number and total number of books with results
		inline QPair<int, int> GetBookIndexAndCount() const { return GetBookIndexAndCount(m_mapVerses.constEnd()); }
		QPair<int, int> GetChapterIndexAndCount(CVerseMap::const_iterator itrVerse) const;	// Returns the Search Result Chapter and total number of chapters with results
		inline QPair<int, int> GetChapterIndexAndCount() const { return GetChapterIndexAndCount(m_mapVerses.constEnd()); }
		QPair<int, int> GetVerseIndexAndCount(CVerseMap::const_iterator itrVerse) const;	// Returns the Search Result Verse and total number of verses with results
		inline QPair<int, int> GetVerseIndexAndCount() const { return GetVerseIndexAndCount(m_mapVerses.constEnd()); }

		const CSearchResultsData &searchResultsData() const { return m_searchResultsData; }

		using TVerseListModelResults::GetVerseCount;
		using TVerseListModelResults::verseMap;
	};

	class TVerseListModelNotesResults : public TVerseListModelResults {
	protected:
		friend class CVerseListModel;

		TVerseListModelNotesResults(TVerseListModelPrivate *priv)
			:	TVerseListModelResults(priv, tr("Notes", "MainMenu"), VLMRTE_USER_NOTES)
		{ }

		// --------------------------------------
	};

	class TVerseListModelCrossRefsResults : public TVerseListModelResults {
	protected:
		friend class CVerseListModel;

		TVerseListModelCrossRefsResults(TVerseListModelPrivate *priv)
			:	TVerseListModelResults(priv, tr("Cross References", "MainMenu"), VLMRTE_CROSS_REFS, VLMNTE_CROSS_REFERENCE_SOURCE_NODE)
		{ }

		TCrossReferenceMap m_mapCrossRefs;		// Created from UserNotesDatabase CrossRefsMap by buildCrossRefsResults, scoped to the current database (i.e. references not part of the database are excluded, like Apocrypha references with non-Apocrypha database)

		// --------------------------------------
	};

	// ------------------------------------------------------------------------

	CVerseListModel(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QObject *pParent = nullptr);

	inline CBibleDatabasePtr bibleDatabase() const { return m_private.m_pBibleDatabase; }
	inline CUserNotesDatabasePtr userNotesDatabase() const { return m_private.m_pUserNotesDatabase; }

	virtual int rowCount(const QModelIndex &zParent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &zParent = QModelIndex()) const override;

	virtual QModelIndex	index(int row, int column = 0, const QModelIndex &zParent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex &index) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;
	CRelIndex logicalIndexForModelIndex(const QModelIndex &index) const;
	QModelIndex modelIndexForLogicalIndex(const CRelIndex &ndxLogical) const;
	CRelIndex navigationIndexForModelIndex(const QModelIndex &index) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	void sortModelIndexList(QModelIndexList &lstIndexes, bool bUseCopySortOption) const;				// Sorts a list of model indexes for the current set of model settings
	static bool ascendingLessThanModelIndex(const QModelIndex &ndx1, const QModelIndex &ndx2);
	static bool ascendingLessThanXRefTargets(const QModelIndex &ndx1, const QModelIndex &ndx2);
	static bool descendingLessThanModelIndex(const QModelIndex &ndx1, const QModelIndex &ndx2);
	static bool descendingLessThanXRefTargets(const QModelIndex &ndx1, const QModelIndex &ndx2);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual bool insertRows(int row, int count, const QModelIndex &zParent = QModelIndex()) override;
	virtual bool removeRows(int row, int count, const QModelIndex &zParent = QModelIndex()) override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	virtual Qt::DropActions supportedDropActions() const override;
#if QT_VERSION >= 0x050000
	virtual Qt::DropActions supportedDragActions() const override;
#else
	// On Qt4, this function isn't virtual and can't be overridden.
	//	Instead, we have to call setSupportedDragActions() in the
	//	constructor:
	Qt::DropActions supportedDragActions() const;
#endif
	virtual QStringList mimeTypes() const override;
	virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
	virtual bool dropMimeData(const QMimeData *pData, Qt::DropAction nAction, int nRow, int nColumn, const QModelIndex &zParent) override;

	QMimeData *mimeDataFromVerseText(const QModelIndexList &lstVersesUnsorted, bool bVerseTextOnly) const;
	QMimeData *mimeDataFromRawVerseText(const QModelIndexList &lstVersesUnsorted, bool bVeryRaw) const;
	QMimeData *mimeDataFromVerseHeadings(const QModelIndexList &lstVersesUnsorted, bool bHeadingTextOnly) const;
	QMimeData *mimeDataFromReferenceDetails(const QModelIndexList &lstVersesUnsorted) const;
	QMimeData *mimeDataFromCompleteVerseDetails(const QModelIndexList &lstVersesUnsorted) const;

	QModelIndex locateIndex(const TVerseIndex &ndxVerse) const;
	TVerseIndex resolveVerseIndex(const CRelIndex &ndxRel, const QString &strResultsName, VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType = VLMRTE_UNDEFINED) const;			// Note: Pass strHighlighterName for strResultsName or Empty string for types that use no specialIndex (nResultsType == VLMRTE_UNDEFINED uses ViewMode of model)

	void setParsedPhrases(const CSearchResultsData &searchResultsData);		// Will build verseList and the list of tags so they can be iterated in a highlighter, etc

	VERSE_DISPLAY_MODE_ENUM displayMode() const { return m_private.m_nDisplayMode; }
	VERSE_TREE_MODE_ENUM treeMode() const { return m_private.m_nTreeMode; }
	VERSE_VIEW_MODE_ENUM viewMode() const { return m_private.m_nViewMode; }
	bool showMissingLeafs() const { return m_private.m_bShowMissingLeafs; }
	bool showHighlightersInSearchResults() const { return m_private.m_bShowHighlightersInSearchResults; }
	CRelIndex singleCrossRefSourceIndex() const { return m_private.m_ndxSingleCrossRefSource; }

	const TVerseListModelResults &results(VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType, int nSpecialIndex) const
	{
		switch (nResultsType) {
			case VLMRTE_UNDEFINED:
				return m_undefinedResults;
			case VLMRTE_SEARCH_RESULTS:
				return m_searchResults;
			case VLMRTE_SEARCH_RESULTS_EXCLUDED:
				return m_searchResultsExcluded;
			case VLMRTE_USER_NOTES:
				return m_userNotesResults;
			case VLMRTE_HIGHLIGHTERS:
				break;				// Fall through and lookup
			case VLMRTE_CROSS_REFS:
				return m_crossRefsResults;
			default:
				Q_ASSERT(false);
		}

		if (nSpecialIndex == -1) return m_undefinedResults;
		Q_ASSERT((nSpecialIndex >= 0) && (nSpecialIndex < m_vlmrListHighlighters.size()));
		return m_vlmrListHighlighters.at(nSpecialIndex);
	}
	const TVerseListModelResults &results(const TVerseIndex &ndxVerse) const
	{
		return results(ndxVerse.resultsType(), ndxVerse.specialIndex());
	}
	const TVerseListModelResults &results(const QModelIndex &index) const
	{
		if (!index.isValid()) return results(VVME_to_VLMRTE(m_private.m_nViewMode), -1);
		TVerseIndex *pVerseIndex = toVerseIndex(index);
		Q_ASSERT(pVerseIndex->resultsType() == VVME_to_VLMRTE(m_private.m_nViewMode));

		return results(*pVerseIndex);
	}
	const TVerseListModelSearchResults &searchResults(bool bExcluded) const
	{
		if (bExcluded) return m_searchResultsExcluded;
		return m_searchResults;
	}

	const TVerseListModelResults &highlighterResults(const QString &strHighlighterName) const
	{
		for (int ndx = 0; ndx < m_vlmrListHighlighters.size(); ++ndx) {
			if (m_vlmrListHighlighters.at(ndx).resultsName().compare(strHighlighterName) == 0) return m_vlmrListHighlighters.at(ndx);
		}
		return m_undefinedResults;
	}

	inline const QFont &font() const { return m_private.m_font; }

signals:
	void cachedSizeHintsInvalidated();
	void verseListAboutToChange();					// Emitted just before our verse phrase tags change so that users (BrowserWidget, etc) can clear highlighting with the old tags
	void verseListChanged();						// Emitted just after our verse phrase tags change so that users (BrowserWidget, etc) can set highlighting with the new tags
	void searchResultsReady();						// Emitted when we receive en_searchResultsReady() during multithreaded or in the buildScopedResultsFromParsedPhrases() for non-multithreaded

public slots:
	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode);
	void setShowMissingLeafs(bool bShowMissing);
	void setShowHighlightersInSearchResults(bool bShowHighlightersInSearchResults);
	void setSingleCrossRefSourceIndex(const CRelIndex &ndx);
	virtual void setFont(const QFont& aFont);
	void setUserNoteKeywordFilter(const QStringList &lstKeywordFilter);			// Note: An empty string is a special "show notes without keywords" entry.  This list should be DECOMPOSED words!

protected slots:
	void en_WordsOfJesusColorChanged(const QColor &color);
	void en_SearchResultsColorChanged(const QColor &color);
	void en_changedShowPilcrowMarkers(bool bShowPilcrowMarkers);
	void en_changedCopyOptions();

	void en_highlighterTagsChanged(const CBibleDatabase *pBibleDatabase, const QString &strUserDefinedHighlighterName);
	void en_changedHighlighters();

	void en_changedUserNote(BIBLE_VERSIFICATION_TYPE_ENUM nVersification, const CRelIndex &ndx);
	void en_addedUserNote(BIBLE_VERSIFICATION_TYPE_ENUM nVersification, const CRelIndex &ndx);
	void en_removedUserNote(BIBLE_VERSIFICATION_TYPE_ENUM nVersification, const CRelIndex &ndx);
	void en_changedAllUserNotes();

	void en_addedCrossRef(BIBLE_VERSIFICATION_TYPE_ENUM nVersification, const CRelIndex &ndxRef1, const CRelIndex &ndxRef2);
	void en_removedCrossRef(BIBLE_VERSIFICATION_TYPE_ENUM nVersification, const CRelIndex &ndxRef1, const CRelIndex &ndxRef2);
	void en_changedAllCrossRefs();

public:
	// Total Verse/Result count for the whole model for the current mode:
	int GetVerseCount(int nBk = -1, int nChp = -1) const;
	int GetResultsCount(int nBk = -1, int nChp = -1) const;

private:
	void clearAllSizeHints();
	void clearAllExtraVerseIndexes();

	void buildHighlighterResults(int ndxHighlighter = -1);								// Note: index of -1 = All Highlighters
	void buildHighlighterResults(int ndxHighlighter, const TPhraseTagList *pTags);		// Here, ndxHighlighter must NOT be -1 !!

	void buildUserNotesResults(const CRelIndex &ndx = CRelIndex(), bool bAdd = true);

	void buildCrossRefsResults();

	void buildScopedResultsFromParsedPhrases(const CSearchResultsData &searchResultsData);

#ifdef USE_MULTITHREADED_SEARCH_RESULTS
private slots:
	void en_searchResultsReady(const CThreadedSearchResultCtrl *theThreadedSearchResult);
#endif

private:
	Q_DISABLE_COPY(CVerseListModel)
	TVerseListModelPrivate m_private;
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	QPointer<CThreadedSearchResultCtrl> m_pSearchResultsThreadCtrl;		// Thread Controller for multi-threading Search Results
#endif

	THighlighterVLMRList m_vlmrListHighlighters;		// Per-Highlighter VerseListModelResults
	TVerseListModelResults m_undefinedResults;			// VerseListModelResults for Undefined Results -- Used for generating extraVerseIndexes for parent entries where QModelIndex->NULL
	TVerseListModelSearchResults m_searchResults;		// VerseListModelResults for Search Results
	TVerseListModelSearchResults m_searchResultsExcluded;	// VerseListModelResults for Excluded Search Results
	TVerseListModelNotesResults m_userNotesResults;		// VerseListModelResults for User Notes
	TVerseListModelCrossRefsResults m_crossRefsResults;	// VerseListModelResults for Cross References
	QStringList m_lstUserNoteKeywordFilter;				// User Note filter set by Search Results view via call to setUserNoteKeywordFilter().  Note: An empty string is a special "show notes without keywords" entry
// ---
	// Special statics needed for sorting (mutexed in sorting function to be thread-safe):
	static const TCrossReferenceMap *ms_pCrossRefsMap;
};

// ============================================================================

//
// CSearchResultsSummary
//		Search results summary of 'n' occurrences in 'x' verses in 'y' chapters in 'z' books
//
class CSearchResultsSummary
{
public:
	CSearchResultsSummary()
	{
		reset();
	}

	CSearchResultsSummary(const CVerseListModel &verseModel)
	{
		setFromVerseListModel(verseModel);
	}

	CSearchResultsSummary & operator=(const CVerseListModel &src)
	{
		setFromVerseListModel(src);
		return *this;
	}

	void reset()
	{
		m_nSearchOccurrences = 0;
		m_nSearchVerses = 0;
		m_nSearchChapters = 0;
		m_nSearchBooks = 0;
		m_nNumSearchPhrases = 0;
		// ----
		m_nExcludedSearchOccurrences = 0;
		m_nExcludedSearchVerses = 0;
		m_nExcludedSearchChapters = 0;
		m_nExcludedSearchBooks = 0;
		m_nNumExcludedSearchPhrases = 0;
		// ----
		m_SearchCriteria.clear();
		m_bValid = false;
	}

	void setFromVerseListModel(const CVerseListModel &verseModel)
	{
		m_nSearchOccurrences = verseModel.searchResults(false).GetResultsCount();
		m_nSearchVerses = verseModel.searchResults(false).GetVerseIndexAndCount().second;
		m_nSearchChapters = verseModel.searchResults(false).GetChapterIndexAndCount().second;
		m_nSearchBooks = verseModel.searchResults(false).GetBookIndexAndCount().second;
		m_nNumSearchPhrases = verseModel.searchResults(false).searchResultsData().m_lstParsedPhrases.size();
		// ----
		m_nExcludedSearchOccurrences = verseModel.searchResults(true).GetResultsCount();
		m_nExcludedSearchVerses = verseModel.searchResults(true).GetVerseIndexAndCount().second;
		m_nExcludedSearchChapters = verseModel.searchResults(true).GetChapterIndexAndCount().second;
		m_nExcludedSearchBooks = verseModel.searchResults(true).GetBookIndexAndCount().second;
		m_nNumExcludedSearchPhrases = verseModel.searchResults(true).searchResultsData().m_lstParsedPhrases.size();
		// ----
		m_SearchCriteria = verseModel.searchResults(false).searchResultsData().m_SearchCriteria;		// Should match excluded as well
		m_bValid = true;
	}

	QString summaryDisplayText(CBibleDatabasePtr pBibleDatabase, bool bExcluded = false, bool bWebChannelHTML = false) const;
	QString summaryCopyText(CBibleDatabasePtr pBibleDatabase) const;

	// ----
	bool isValid() const { return m_bValid; }
	// ---- included:
	int searchOccurrences() const { return m_nSearchOccurrences; }
	int searchVerses() const { return m_nSearchVerses; }
	int searchChapters() const { return m_nSearchChapters; }
	int searchBooks() const { return m_nSearchBooks; }
	int numSearchPhrases() const { return m_nNumSearchPhrases; }
	// ---- excluded:
	int excludedSearchOccurrences() const { return m_nExcludedSearchOccurrences; }
	int excludedSearchVerses() const { return m_nExcludedSearchVerses; }
	int excludedSearchChapters() const { return m_nExcludedSearchChapters; }
	int excludedSearchBooks() const { return m_nExcludedSearchBooks; }
	int numExcludedSearchPhrases() const { return m_nNumExcludedSearchPhrases; }
	// ----
	int numTotalSearchPhrases() const { return m_nNumSearchPhrases + m_nNumExcludedSearchPhrases; }
	// ----
	const CSearchCriteria &searchCriteria() const { return m_SearchCriteria; }

private:
	int m_nSearchOccurrences;
	int m_nSearchVerses;
	int m_nSearchChapters;
	int m_nSearchBooks;
	// ----
	int m_nExcludedSearchOccurrences;
	int m_nExcludedSearchVerses;
	int m_nExcludedSearchChapters;
	int m_nExcludedSearchBooks;
	// ----
	int m_nNumSearchPhrases;
	int m_nNumExcludedSearchPhrases;
	CSearchCriteria m_SearchCriteria;
	// ----
	bool m_bValid;						// Set to true when set from a search source
};

// ============================================================================

#endif // VERSE_LIST_MODEL_H
