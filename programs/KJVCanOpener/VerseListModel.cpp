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

#include "VerseListModel.h"

#include "ReportError.h"
#include "ScriptureDocument.h"
#include "SearchCompleter.h"
#include "myApplication.h"
#include "KJVCanOpener.h"
#include "KJVSearchResult.h"		// Needed for the CKJVSearchResult class to keep the CSearchResultsSummary in the same translation context until we can revamp our translation contexts : TODO: Fix translations and Remove this
#include "ModelRowForwardIterator.h"

#ifdef USE_MULTITHREADED_SEARCH_RESULTS
#include "ThreadedSearchResults.h"
#endif

#include <QVector>
#include <QByteArray>
#include <QDataStream>
#include <iterator>
#include <list>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QAtomicInt>
#include <QMessageBox>
#include <BusyCursor.h>

#if 0
#define ASSERT_MODEL_DEBUG(x) Q_ASSERT(x)
#else
#define ASSERT_MODEL_DEBUG(x)
#endif

// ============================================================================

static bool ascendingLessThanVLI(const CVerseListItem &s1, const CVerseListItem &s2)
{
	// Both normalized and denormalized are in order, but it's more expensive
	//	 to convert to normal when we already have relative
	return s1.getIndexDenormalized() < s2.getIndexDenormalized();
}

static bool decendingLessThanVLI(const CVerseListItem &s1, const CVerseListItem &s2)
{
	// Both normalized and denormalized are in order, but it's more expensive
	//	 to convert to normal when we already have relative
	return s1.getIndexDenormalized() > s2.getIndexDenormalized();
}

void sortVerseList(CVerseList &aVerseList, Qt::SortOrder order)
{
	if (order == Qt::AscendingOrder)
		std::sort(aVerseList.begin(), aVerseList.end(), ascendingLessThanVLI);
	else
		std::sort(aVerseList.begin(), aVerseList.end(), decendingLessThanVLI);
}

// ============================================================================

CVerseListModel::TVerseListModelPrivate::TVerseListModelPrivate(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase)
	:	m_pBibleDatabase(pBibleDatabase),
		m_pUserNotesDatabase(pUserNotesDatabase),
		m_nDisplayMode(VDME_RICHTEXT),
		m_nTreeMode(VTME_LIST),
		m_nViewMode(VVME_SEARCH_RESULTS),
		m_bShowMissingLeafs(false),
		m_bShowHighlightersInSearchResults(true)
{

}

CVerseListModel::CVerseListModel(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QObject *pParent)
	:	QAbstractItemModel(pParent),
		m_private(pBibleDatabase, pUserNotesDatabase),
		m_undefinedResults(&m_private, tr("Undefined", "Scope"), VLMRTE_UNDEFINED),
		m_searchResults(&m_private, false),
		m_searchResultsExcluded(&m_private, true),
		m_userNotesResults(&m_private),
		m_crossRefsResults(&m_private)
{
	Q_ASSERT(!pBibleDatabase.isNull());
	Q_ASSERT(!pUserNotesDatabase.isNull());

	m_private.m_richifierTagsDisplay.setFromPersistentSettings(*CPersistentSettings::instance(), false);
	m_private.m_richifierTagsCopying.setFromPersistentSettings(*CPersistentSettings::instance(), true);

	connect(CPersistentSettings::instance(), SIGNAL(changedColorWordsOfJesus(const QColor &)), this, SLOT(en_WordsOfJesusColorChanged(const QColor &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedColorSearchResults(const QColor &)), this, SLOT(en_SearchResultsColorChanged(const QColor &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedShowPilcrowMarkers(bool)), this, SLOT(en_changedShowPilcrowMarkers(bool)));
	connect(CPersistentSettings::instance(), SIGNAL(changedCopyOptions()), this, SLOT(en_changedCopyOptions()));

	if (!m_private.m_pUserNotesDatabase.isNull()) {
		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(highlighterTagsChanged(CBibleDatabasePtr, const QString &)), this, SLOT(en_highlighterTagsChanged(CBibleDatabasePtr, const QString &)));
		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(changedHighlighters()), this, SLOT(en_changedHighlighters()));

		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(changedUserNote(const CRelIndex &)), this, SLOT(en_changedUserNote(const CRelIndex &)));
		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(addedUserNote(const CRelIndex &)), this, SLOT(en_addedUserNote(const CRelIndex &)));
		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(removedUserNote(const CRelIndex &)), this, SLOT(en_removedUserNote(const CRelIndex &)));

		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(changedAllCrossRefs()), this, SLOT(en_changedAllCrossRefs()));
		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(addedCrossRef(const CRelIndex &, const CRelIndex &)), this, SLOT(en_addedCrossRef(const CRelIndex &, const CRelIndex &)));
		connect(m_private.m_pUserNotesDatabase.data(), SIGNAL(removedCrossRef(const CRelIndex &, const CRelIndex &)), this, SLOT(en_removedCrossRef(const CRelIndex &, const CRelIndex &)));
	}

	en_changedHighlighters();			// Make sure we've loaded the initial default highlighters (or from the current set if we are rebuilding this class for some reason)
	en_addedUserNote(CRelIndex());		// Make sure we've loaded the initial notes list from the document
	en_changedAllCrossRefs();			// Make sure we've loaded the initial crossRefs list from the document
}

int CVerseListModel::rowCount(const QModelIndex &zParent) const
{
	// 0 - Root (Follow with: Highlighter or Book Terminator or List Item or CrossRef-Source)
	// 1 - Highlighter (Follow with: Book Terminator or List Item or CrossRef-Source)
	// 2 - Book Terminator (Follow with: Chapter Terminator or List Item or CrossRef-Source or CrossRef-Target)
	// 3 - Chapter Terminator (Follow with: List Item or CrossRef-Source or CrossRef-Target)
	// 4 - CrossRef-Source (Follow with: CrossRef-Target)
	// 5 - CrossRef-Target
	// 6 - List Item

	TVerseIndex *pParentVerseIndex = toVerseIndex(zParent);
	int nLevel = 0;									// 0
	if (zParent.isValid()) {
		nLevel++;									// 1
		if (pParentVerseIndex->nodeType() != VLMNTE_HIGHLIGHTER_NODE) {
			nLevel++;								// 2
			if (pParentVerseIndex->nodeType() != VLMNTE_BOOK_TERMINATOR_NODE) {
				nLevel++;							// 3
				if (pParentVerseIndex->nodeType() != VLMNTE_CHAPTER_TERMINATOR_NODE) {
					nLevel++;						// 4
					if (pParentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_SOURCE_NODE) {
						nLevel++;					// 5
						if (pParentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
							nLevel++;				// 6
						}
					}
				}
			}
		}
	}

	bool bHighlighterNode = ((m_private.m_nViewMode == VVME_HIGHLIGHTERS) && (nLevel == 0));
	bool bSingleCrossRefNode = ((m_private.m_nViewMode == VVME_CROSSREFS) && (m_private.m_ndxSingleCrossRefSource.isSet()) && (nLevel == 0));
	const TVerseListModelResults &zResults = results(zParent);

	if (bHighlighterNode) {
		return m_vlmrListHighlighters.size();
	} else if (bSingleCrossRefNode) {
		if (!m_crossRefsResults.m_mapCrossRefs.haveCrossReferencesFor(m_private.m_ndxSingleCrossRefSource)) return 0;
		return m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(m_private.m_ndxSingleCrossRefSource).size();
	} else {
		switch (m_private.m_nTreeMode) {
			case VTME_LIST:
			{
				if (nLevel < 2) return zResults.m_mapVerses.size();
				ASSERT_MODEL_DEBUG(nLevel != 2);					// Should have no books in list mode
				ASSERT_MODEL_DEBUG(nLevel != 3);					// Should have no chapters in list mode
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					return m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(toVerseIndex(zParent)->m_nRelIndex).size();
				}
				return 0;
			}
			case VTME_TREE_BOOKS:
			{
				if (nLevel < 2) return zResults.GetBookCount();
				if (nLevel == 2) {
					CRelIndex ndxRel(toVerseIndex(zParent)->m_nRelIndex);
					Q_ASSERT(ndxRel.isSet());
					return zResults.GetVerseCount(ndxRel.book());
				}
				ASSERT_MODEL_DEBUG(nLevel != 3);					// Should have no chapters in book mode
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					return m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(toVerseIndex(zParent)->m_nRelIndex).size();
				}
				return 0;
			}
			case VTME_TREE_CHAPTERS:
			{
				if (nLevel < 2) return zResults.GetBookCount();
				CRelIndex ndxRel(toVerseIndex(zParent)->m_nRelIndex);
				Q_ASSERT(ndxRel.isSet());
				if (nLevel == 2) return zResults.GetChapterCount(ndxRel.book());
				if (nLevel == 3) return zResults.GetVerseCount(ndxRel.book(), ndxRel.chapter());
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					return m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(toVerseIndex(zParent)->m_nRelIndex).size();
				}
				return 0;
			}
			default:
				break;
		}
	}

	return 0;
}

int CVerseListModel::columnCount(const QModelIndex &zParent) const
{
	// 0 - Root (Follow with: Highlighter or Book Terminator or List Item or CrossRef-Source)
	// 1 - Highlighter (Follow with: Book Terminator or List Item or CrossRef-Source)
	// 2 - Book Terminator (Follow with: Chapter Terminator or List Item or CrossRef-Source or CrossRef-Target)
	// 3 - Chapter Terminator (Follow with: List Item or CrossRef-Source or CrossRef-Target)
	// 4 - CrossRef-Source (Follow with: CrossRef-Target)
	// 5 - CrossRef-Target
	// 6 - List Item

	TVerseIndex *pParentVerseIndex = toVerseIndex(zParent);
	int nLevel = 0;									// 0
	if (zParent.isValid()) {
		nLevel++;									// 1
		if (pParentVerseIndex->nodeType() != VLMNTE_HIGHLIGHTER_NODE) {
			nLevel++;								// 2
			if (pParentVerseIndex->nodeType() != VLMNTE_BOOK_TERMINATOR_NODE) {
				nLevel++;							// 3
				if (pParentVerseIndex->nodeType() != VLMNTE_CHAPTER_TERMINATOR_NODE) {
					nLevel++;						// 4
					if (pParentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_SOURCE_NODE) {
						nLevel++;					// 5
						if (pParentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
							nLevel++;				// 6
						}
					}
				}
			}
		}
	}

	bool bHighlighterNode = ((m_private.m_nViewMode == VVME_HIGHLIGHTERS) && (nLevel == 0));
	bool bSingleCrossRefNode = ((m_private.m_nViewMode == VVME_CROSSREFS) && (m_private.m_ndxSingleCrossRefSource.isSet()) && (nLevel == 0));
//	const TVerseListModelResults &zResults = results(zParent);

	if (bHighlighterNode) {
		return 1;
	} else if (bSingleCrossRefNode) {
		return 1;
	} else {
		switch (m_private.m_nTreeMode) {
			case VTME_LIST:
			{
				if (nLevel < 2) return 1;				// Root and Highlighter have 1 column
				ASSERT_MODEL_DEBUG(nLevel != 2);		// Should have no books in list mode
				ASSERT_MODEL_DEBUG(nLevel != 3);		// Should have no chapters in list mode
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					return 1;							// Cross-Reference Target Nodes
				}
				return 0;								// Other (real data) Nodes have 0 columns
			}
			case VTME_TREE_BOOKS:
			{
				if (nLevel < 2) return 1;				// Root and Highlighter have 1 column
				if (nLevel == 2) return 1;				// Book Node has 1 column
				ASSERT_MODEL_DEBUG(nLevel != 3);		// Should have no chapters in book mode
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					return 1;							// Cross-Reference Target Nodes
				}
				return 0;								// Other (real data) Nodes have 0 columns
			}
			case VTME_TREE_CHAPTERS:
			{
				if (nLevel < 2) return 1;				// Root and Highlighter have 1 column
				if (nLevel == 2) return 1;				// Book Node has 1 column
				if (nLevel == 3) return 1;				// Chapter Node has 1 column
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					return 1;							// Cross-Reference Target Nodes
				}
				return 0;							// Other (real data) Nodes have 0 columns
			}
			default:
				break;
		}
	}

	return 0;
}

QModelIndex	CVerseListModel::index(int row, int column, const QModelIndex &zParent) const
{
	if (!hasIndex(row, column, zParent)) return QModelIndex();

	// 0 - Root (Follow with: Highlighter or Book Terminator or List Item or CrossRef-Source)
	// 1 - Highlighter (Follow with: Book Terminator or List Item or CrossRef-Source)
	// 2 - Book Terminator (Follow with: Chapter Terminator or List Item or CrossRef-Source or CrossRef-Target)
	// 3 - Chapter Terminator (Follow with: List Item or CrossRef-Source or CrossRef-Target)
	// 4 - CrossRef-Source (Follow with: CrossRef-Target)
	// 5 - CrossRef-Target
	// 6 - List Item

	TVerseIndex *pParentVerseIndex = toVerseIndex(zParent);
	int nLevel = 0;									// 0
	if (zParent.isValid()) {
		nLevel++;									// 1
		if (pParentVerseIndex->nodeType() != VLMNTE_HIGHLIGHTER_NODE) {
			nLevel++;								// 2
			if (pParentVerseIndex->nodeType() != VLMNTE_BOOK_TERMINATOR_NODE) {
				nLevel++;							// 3
				if (pParentVerseIndex->nodeType() != VLMNTE_CHAPTER_TERMINATOR_NODE) {
					nLevel++;						// 4
					if (pParentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_SOURCE_NODE) {
						nLevel++;					// 5
						if (pParentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
							nLevel++;				// 6
						}
					}
				}
			}
		}
	}

	bool bHighlighterNode = ((m_private.m_nViewMode == VVME_HIGHLIGHTERS) && (nLevel == 0));
	bool bSingleCrossRefNode = ((m_private.m_nViewMode == VVME_CROSSREFS) && (m_private.m_ndxSingleCrossRefSource.isSet()) && (nLevel == 0));
	const TVerseListModelResults &zResults = (!bHighlighterNode ? results(zParent) : results(VLMRTE_HIGHLIGHTERS, row));			// If this is the top-level highlighter entry, the given parent will be invalid but our row is our highlighter results index

	if (bHighlighterNode) {
		ASSERT_MODEL_DEBUG(nLevel == 0);
		Q_ASSERT(row < m_vlmrListHighlighters.size());
		if (row < m_vlmrListHighlighters.size()) {
			return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(), VLMNTE_HIGHLIGHTER_NODE).data()));		// Highlighter specialIndex with unset CRelIndex
		}
	} else if (bSingleCrossRefNode) {
		if (!m_crossRefsResults.m_mapCrossRefs.haveCrossReferencesFor(m_private.m_ndxSingleCrossRefSource)) return QModelIndex();
		// For cross-references, the child entries use the parent's ndxRel, but have target nodeType set (it's relIndex comes from row()):
		Q_ASSERT(static_cast<unsigned int>(row) < m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(m_private.m_ndxSingleCrossRefSource).size());
		return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(m_private.m_ndxSingleCrossRefSource, VLMNTE_CROSS_REFERENCE_TARGET_NODE).data()));
	} else {
		switch (m_private.m_nTreeMode) {
			case VTME_LIST:
			{
				ASSERT_MODEL_DEBUG(nLevel != 2);					// Should have no books in list mode
				ASSERT_MODEL_DEBUG(nLevel != 3);					// Should have no chapters in list mode
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					// For cross-references, the child entries use the parent's ndxRel, but have target nodeType set (it's relIndex comes from row()):
					Q_ASSERT(static_cast<unsigned int>(row) < m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(toVerseIndex(zParent)->m_nRelIndex).size());
					return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(toVerseIndex(zParent)->m_nRelIndex, VLMNTE_CROSS_REFERENCE_TARGET_NODE).data()));
				}
				Q_ASSERT(row < zResults.m_mapVerses.size());
				CVerseMap::const_iterator itrVerse = zResults.GetVerse(row);
				if (itrVerse == zResults.m_mapVerses.constEnd()) return QModelIndex();
				return createIndex(row, column, fromVerseIndex(itrVerse->verseIndex().data()));				// Real Data item (using nodeType of model data)
			}
			case VTME_TREE_BOOKS:
			{
				if (nLevel < 2) {
					return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(zResults.BookByIndex(row), 0, 0, 0), VLMNTE_BOOK_TERMINATOR_NODE).data()));
				}
				ASSERT_MODEL_DEBUG(nLevel != 3);					// Should have no chapters in book mode
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					// For cross-references, the child entries use the parent's ndxRel, but have target nodeType set (it's relIndex comes from row()):
					Q_ASSERT(static_cast<unsigned int>(row) < m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(toVerseIndex(zParent)->m_nRelIndex).size());
					return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(toVerseIndex(zParent)->m_nRelIndex, VLMNTE_CROSS_REFERENCE_TARGET_NODE).data()));
				}
				CRelIndex ndxRel(toVerseIndex(zParent)->m_nRelIndex);
				Q_ASSERT(ndxRel.isSet());
				CVerseMap::const_iterator itrVerse = zResults.GetVerse(row, ndxRel.book());
				if (itrVerse == zResults.m_mapVerses.constEnd()) return QModelIndex();
				return createIndex(row, column, fromVerseIndex(itrVerse->verseIndex().data()));			// Real Data item (using nodeType of model mode)
			}
			case VTME_TREE_CHAPTERS:
			{
				if (nLevel < 2) {
					return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(zResults.BookByIndex(row), 0, 0, 0), VLMNTE_BOOK_TERMINATOR_NODE).data()));
				}
				CRelIndex ndxRel(toVerseIndex(zParent)->m_nRelIndex);
				Q_ASSERT(ndxRel.isSet());
				if (nLevel == 2) {
					return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(ndxRel.book(), zResults.ChapterByIndex(zParent.row(), row), 0, 0), VLMNTE_CHAPTER_TERMINATOR_NODE).data()));
				}
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					// For cross-references, the child entries use the parent's ndxRel, but have target nodeType set (it's relIndex comes from row()):
					Q_ASSERT(static_cast<unsigned int>(row) < m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(toVerseIndex(zParent)->m_nRelIndex).size());
					return createIndex(row, column, fromVerseIndex(zResults.extraVerseIndex(toVerseIndex(zParent)->m_nRelIndex, VLMNTE_CROSS_REFERENCE_TARGET_NODE).data()));
				}
				CVerseMap::const_iterator itrVerse = zResults.GetVerse(row, ndxRel.book(), ndxRel.chapter());
				if (itrVerse == zResults.m_mapVerses.constEnd()) return QModelIndex();
				return createIndex(row, column, fromVerseIndex(itrVerse->verseIndex().data()));			// Real Data item (using noteType of model mode)
			}
			default:
				break;
		}
	}

	return QModelIndex();
}

QModelIndex CVerseListModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) return QModelIndex();

	// 0 - Highlighter
	// 1 - Book Terminator
	// 2 - Chapter Terminator
	// 3 - CrossRef-Source
	// 4 - CrossRef-Target
	// 5 - List Item

	TVerseIndex *pCurrentVerseIndex = toVerseIndex(index);
	int nLevel = 0;								// 0
	if (pCurrentVerseIndex->nodeType() != VLMNTE_HIGHLIGHTER_NODE) {
		nLevel++;								// 1
		if (pCurrentVerseIndex->nodeType() != VLMNTE_BOOK_TERMINATOR_NODE) {
			nLevel++;							// 2
			if (pCurrentVerseIndex->nodeType() != VLMNTE_CHAPTER_TERMINATOR_NODE) {
				nLevel++;						// 3
				if (pCurrentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_SOURCE_NODE) {
					nLevel++;					// 4
					if (pCurrentVerseIndex->nodeType() != VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
						nLevel++;				// 5
					}
				}
			}
		}
	}

	bool bSingleCrossRefTargetNode = ((m_private.m_nViewMode == VVME_CROSSREFS) && (m_private.m_ndxSingleCrossRefSource.isSet()) && (nLevel == 4));
	const TVerseListModelResults &zResults = results(index);
	CRelIndex ndxRel(toVerseIndex(index)->m_nRelIndex);

	if (nLevel == 0) {
		return QModelIndex();			// Highlighters are always at the top
	} else if (bSingleCrossRefTargetNode) {
		return QModelIndex();			// The single Target Cross Refs are always at the top
	} else {
		switch (m_private.m_nTreeMode) {
			case VTME_LIST:
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					Q_ASSERT(ndxRel.isSet());
					return createIndex(zResults.IndexByVerse(ndxRel), 0, fromVerseIndex(zResults.FindVerseIndex(ndxRel)->verseIndex().data()));				// CROSSREFS_SOURCE nodeType from buildCrossRefs()
				}
				if (nLevel == 3) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					return QModelIndex();						// CrossRef-Source is at top of list
				}
				ASSERT_MODEL_DEBUG(nLevel != 2);					// Should have no chapters in list mode
				ASSERT_MODEL_DEBUG(nLevel != 1);					// Should have no books in list mode
				ASSERT_MODEL_DEBUG(nLevel == 5);
				if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
					return createIndex(zResults.specialIndex(), 0, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(), VLMNTE_HIGHLIGHTER_NODE).data()));		// Highlighter specialIndex with unset CRelIndex
				}
				return QModelIndex();

			case VTME_TREE_BOOKS:
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					Q_ASSERT(ndxRel.isSet());
					return createIndex(zResults.IndexByVerse(ndxRel), 0, fromVerseIndex(zResults.FindVerseIndex(ndxRel)->verseIndex().data()));				// CROSSREFS_SOURCE nodeType from buildCrossRefs()
				}
				if ((nLevel == 5) || (nLevel == 3)) {
					Q_ASSERT(ndxRel.isSet());
					if (zResults.m_mapVerses.contains(ndxRel)) {
						return createIndex(zResults.IndexByBook(ndxRel.book()), 0, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(ndxRel.book(), 0, 0, 0), VLMNTE_BOOK_TERMINATOR_NODE).data()));
					} else {
						Q_ASSERT(false);
					}
				}
				ASSERT_MODEL_DEBUG(nLevel != 2);					// Should have no chapters in book mode
				ASSERT_MODEL_DEBUG(nLevel == 1);
				if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
					return createIndex(zResults.specialIndex(), 0, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(), VLMNTE_HIGHLIGHTER_NODE).data()));		// Highlighter specialIndex with unset CRelIndex
				}
				return QModelIndex();

			case VTME_TREE_CHAPTERS:
				if (nLevel == 4) {
					ASSERT_MODEL_DEBUG(m_private.m_nViewMode == VVME_CROSSREFS);
					Q_ASSERT(ndxRel.isSet());
					return createIndex(zResults.IndexByVerse(ndxRel), 0, fromVerseIndex(zResults.FindVerseIndex(ndxRel)->verseIndex().data()));				// CROSSREFS_SOURCE nodeType from buildCrossRefs()
				}
				if ((nLevel == 5) || (nLevel == 3)) {
					Q_ASSERT(ndxRel.isSet());
					if (zResults.m_mapVerses.contains(ndxRel)) {
						return createIndex(zResults.IndexByChapter(ndxRel.book(), ndxRel.chapter()), 0, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(ndxRel.book(), ndxRel.chapter(), 0, 0), VLMNTE_CHAPTER_TERMINATOR_NODE).data()));
					} else {
						Q_ASSERT(false);
					}
				}
				if (nLevel == 2) {
					return createIndex(zResults.IndexByBook(ndxRel.book()), 0, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(ndxRel.book(), 0, 0, 0), VLMNTE_BOOK_TERMINATOR_NODE).data()));
				}
				ASSERT_MODEL_DEBUG(nLevel == 1);
				if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
					return createIndex(zResults.specialIndex(), 0, fromVerseIndex(zResults.extraVerseIndex(CRelIndex(), VLMNTE_HIGHLIGHTER_NODE).data()));		// Highlighter specialIndex with unset CRelIndex
				}
				return QModelIndex();

			default:
				break;
		}
	}

	return QModelIndex();
}

QVariant CVerseListModel::data(const QModelIndex &index, int role) const
{
	Q_ASSERT(!m_private.m_pBibleDatabase.isNull());

	if (!index.isValid()) return QVariant();

	const TVerseListModelResults &zResults = results(index);

	if (role == Qt::SizeHintRole) return zResults.m_mapSizeHints.value(index, QSize());

	bool bHighlighterNode = ((m_private.m_nViewMode != VVME_HIGHLIGHTERS) ? false : !parent(index).isValid());

	CScriptureTextHtmlBuilder dataGenHTML;

	if (bHighlighterNode) {
		if ((role == Qt::DisplayRole) || (role == Qt::EditRole) || (role == VERSE_COPYING_ROLE)) {
			const TUserDefinedColor udcHighlighter = userNotesDatabase()->highlighterDefinition(zResults.resultsName());
			if (udcHighlighter.isValid()) {
				dataGenHTML.beginParagraph();
				dataGenHTML.beginBold();
				dataGenHTML.beginBackground(udcHighlighter.m_color);
				dataGenHTML.appendLiteralText(zResults.resultsName());
				dataGenHTML.endBackground();
				dataGenHTML.endBold();
				dataGenHTML.endParagraph();
			} else {
				dataGenHTML.beginParagraph();
				dataGenHTML.beginBold();
				dataGenHTML.appendLiteralText(zResults.resultsName());
				dataGenHTML.endBold();
				dataGenHTML.endParagraph();
			}
			return dataGenHTML.getResult();
		}
	} else {
		TVerseIndex *pVerseIndex = toVerseIndex(index);
		Q_ASSERT(pVerseIndex != nullptr);

#ifdef DEBUG_SEARCH_RESULTS_NODE_TOOLTIPS
		if (role == Qt::ToolTipRole) {
			switch (pVerseIndex->nodeType()) {
				case VLMNTE_UNDEFINED:
					return QString("Undefined");
				case VLMNTE_TESTAMENT_TERMINATOR_NODE:
					return QString("Testament Terminator");
				case VLMNTE_CATEGORY_TERMINATOR_NODE:
					return QString("Category Terminator");
				case VLMNTE_BOOK_TERMINATOR_NODE:
					return QString("Book Terminator");
				case VLMNTE_CHAPTER_TERMINATOR_NODE:
					return QString("Chapter Terminator");
				case VLMNTE_VERSE_TERMINATOR_NODE:
					return QString("Verse Terminator");
				case VLMNTE_CROSS_REFERENCE_SOURCE_NODE:
					return QString("Cross-Ref Source");
				case VLMNTE_CROSS_REFERENCE_TARGET_NODE:
					return QString("Highlighter");
				default:
					return QString("Unknown Node Type");
			}
		}
#endif

		CRelIndex ndxRel(pVerseIndex->relIndex());
		CRelIndex ndxVerse(pVerseIndex->relIndex());

		if ((m_private.m_nViewMode == VVME_CROSSREFS) &&
			(pVerseIndex->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE)) {
			// For Cross-Ref targets, swapout the RelIndexes (which will be the Cross-Ref source) for the target:
			const TRelativeIndexSet setCrossRefs = m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(pVerseIndex->relIndex());
			Q_ASSERT((index.row() >= 0) && (static_cast<unsigned int>(index.row()) < setCrossRefs.size()));
			TRelativeIndexSet::const_iterator itrRef = setCrossRefs.begin();
			for (int i = index.row(); i > 0; ++itrRef, --i) { }
			ndxRel = *(itrRef);
			ndxVerse = *(itrRef);
		}

		if ((m_private.m_nViewMode == VVME_CROSSREFS) ||
			(m_private.m_nViewMode == VVME_USERNOTES)) {
			if ((ndxVerse.chapter() != 0) && (ndxVerse.verse() != 0)) {
				ndxVerse.setWord(1);			// Whole verses only
			} else {
				ndxVerse.setWord(0);			// We don't allow cross-refs and notes on colophons and superscriptions
			}
		} else {
			ndxVerse.setWord(1);
		}

		CRelIndex ndxDisplayVerse(ndxVerse);
		ndxDisplayVerse.setWord(0);

		Q_ASSERT(ndxRel.isSet());
		if (!ndxRel.isSet()) return QVariant();

		int nVerses = 0;
		int nResults = 0;

		// Books:
		if (((ndxRel.chapter() == 0) && (ndxRel.verse() == 0) && (ndxRel.word() == 0) && (pVerseIndex->nodeType() != VLMNTE_CHAPTER_TERMINATOR_NODE)) ||
			(pVerseIndex->nodeType() == VLMNTE_BOOK_TERMINATOR_NODE)) {
			nVerses = zResults.GetVerseCount(ndxRel.book());
			nResults = zResults.GetResultsCount(ndxRel.book());
			if ((role == Qt::DisplayRole) || (role == Qt::EditRole) || (role == VERSE_COPYING_ROLE)) {
				if (((nResults) || (nVerses)) &&
					(m_private.m_nViewMode != CVerseListModel::VVME_USERNOTES) &&
					(m_private.m_nViewMode != CVerseListModel::VVME_CROSSREFS)) {
					dataGenHTML.appendLiteralText(QString("{%1} (%2) ").arg(nVerses).arg(nResults));
					dataGenHTML.beginBold();
					dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel));
					dataGenHTML.endBold();
				} else if (m_private.m_nViewMode != CVerseListModel::VVME_USERNOTES) {
					if (((m_private.m_nViewMode == CVerseListModel::VVME_CROSSREFS) &&
						(pVerseIndex->nodeType() != VLMNTE_BOOK_TERMINATOR_NODE)) ||
						(m_private.m_nViewMode != CVerseListModel::VVME_CROSSREFS)) dataGenHTML.beginBold();
					dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel));
					if (((m_private.m_nViewMode == CVerseListModel::VVME_CROSSREFS) &&
						(pVerseIndex->nodeType() != VLMNTE_BOOK_TERMINATOR_NODE)) ||
						(m_private.m_nViewMode != CVerseListModel::VVME_CROSSREFS)) dataGenHTML.endBold();
				} else {
					if (m_userNotesResults.m_mapVerses.contains(ndxVerse)) {
						dataGenHTML.beginBold();
						dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel));
						dataGenHTML.endBold();
						dataGenHTML.addNoteFor(ndxDisplayVerse, false, true);
					} else {
						dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel));
					}
				}
				return dataGenHTML.getResult();
			}
			if ((role == Qt::ToolTipRole) ||
				(role == TOOLTIP_ROLE) ||
				(role == TOOLTIP_PLAINTEXT_ROLE) ||
				(role == TOOLTIP_NOHEADING_ROLE) ||
				(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
				return CPhraseNavigator::getToolTip(m_private.m_pBibleDatabase, TPhraseTag(ndxDisplayVerse), CSelectionPhraseTagList());
			}
			return QVariant();
		}

		// Chapters:
		if (
			((ndxRel.verse() == 0) && (ndxRel.word() == 0)) ||
			(pVerseIndex->nodeType() == VLMNTE_CHAPTER_TERMINATOR_NODE)) {
			nVerses = zResults.GetVerseCount(ndxRel.book(), ndxRel.chapter());
			nResults = zResults.GetResultsCount(ndxRel.book(), ndxRel.chapter());
			if ((role == Qt::DisplayRole) || (role == Qt::EditRole) || (role == VERSE_COPYING_ROLE)) {
				if (((nResults) || (nVerses)) &&
					(m_private.m_nViewMode != CVerseListModel::VVME_USERNOTES) &&
					(m_private.m_nViewMode != CVerseListModel::VVME_CROSSREFS)) {
					QString strChapter = QString("%1").arg(ndxRel.chapter());
					if (ndxRel.chapter() == 0) strChapter = m_private.m_pBibleDatabase->translatedColophonString();
					dataGenHTML.appendLiteralText(QString("{%1} (%2) %3 %4")
													.arg(nVerses)
													.arg(nResults)
													.arg(m_private.m_pBibleDatabase->bookName(ndxRel))
													.arg(strChapter));
				} else if (m_private.m_nViewMode != CVerseListModel::VVME_USERNOTES) {
					if ((m_private.m_nViewMode != CVerseListModel::VVME_CROSSREFS) ||
						((m_private.m_nViewMode == CVerseListModel::VVME_CROSSREFS) &&
						 (pVerseIndex->nodeType() != VLMNTE_CHAPTER_TERMINATOR_NODE))) dataGenHTML.beginBold();
					// Note: Chapter can be zero here to indicate a book at the chapter-node-terminator level:
					if (ndxRel.chapter() != 0) {
						dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel) + QString(" %1").arg(ndxRel.chapter()));
					} else {
						dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel));
					}
					if ((m_private.m_nViewMode != CVerseListModel::VVME_CROSSREFS) ||
						((m_private.m_nViewMode == CVerseListModel::VVME_CROSSREFS) &&
						 (pVerseIndex->nodeType() != VLMNTE_CHAPTER_TERMINATOR_NODE))) dataGenHTML.endBold();
				} else {
					if (m_userNotesResults.m_mapVerses.contains(ndxVerse)) {
						dataGenHTML.beginBold();
					}
					// Note: Chapter can be zero here to indicate a book at the chapter-node-terminator level:
					if (ndxRel.chapter() != 0) {
						dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel) + QString(" %1").arg(ndxRel.chapter()));
					} else {
						dataGenHTML.appendLiteralText(m_private.m_pBibleDatabase->bookName(ndxRel));
					}
					if (m_userNotesResults.m_mapVerses.contains(ndxVerse)) {
						dataGenHTML.endBold();
						dataGenHTML.addNoteFor(ndxDisplayVerse, false, true);
					}
				}
				return dataGenHTML.getResult();
			}
			if ((role == Qt::ToolTipRole) ||
				(role == TOOLTIP_ROLE) ||
				(role == TOOLTIP_PLAINTEXT_ROLE) ||
				(role == TOOLTIP_NOHEADING_ROLE) ||
				(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
				return CPhraseNavigator::getToolTip(m_private.m_pBibleDatabase, TPhraseTag(ndxDisplayVerse), CSelectionPhraseTagList());
			}
			return QVariant();
		}

		if (role == Qt::ToolTipRole) return QString();		// en_viewDetails replaces normal ToolTip

		CVerseMap::const_iterator itrVerse = zResults.m_mapVerses.find(ndxVerse);
		if (itrVerse == zResults.m_mapVerses.constEnd()) {
			Q_ASSERT(false);
			return QVariant();
		}

		// Verses:
		if ((role == Qt::DisplayRole) || (role == Qt::EditRole) || (role == VERSE_COPYING_ROLE)) {
			if (m_private.m_nViewMode == CVerseListModel::VVME_USERNOTES) {
				if (m_private.m_nDisplayMode == VDME_HEADING) {
					dataGenHTML.beginBold();
					dataGenHTML.appendLiteralText(itrVerse->getHeading(role == VERSE_COPYING_ROLE));
					dataGenHTML.endBold();
				} else {
					dataGenHTML.beginBold();
					dataGenHTML.appendLiteralText(QString("%1 ").arg(m_private.m_pBibleDatabase->PassageReferenceText(ndxDisplayVerse, true)));
					dataGenHTML.endBold();
					dataGenHTML.appendRawText(itrVerse->getVerseRichText((role == VERSE_COPYING_ROLE) ? m_private.m_richifierTagsCopying : m_private.m_richifierTagsDisplay, false));
				}
				if (m_userNotesResults.m_mapVerses.contains(ndxVerse)) {
					dataGenHTML.addNoteFor(ndxDisplayVerse, false, true);
				}
				return dataGenHTML.getResult();
			} else if (m_private.m_nDisplayMode == CVerseListModel::VDME_HEADING) {
				dataGenHTML.appendLiteralText(itrVerse->getHeading(role == VERSE_COPYING_ROLE));
				return dataGenHTML.getResult();
			} else {
				QTextDocument doc;
				CPhraseNavigator navigator(m_private.m_pBibleDatabase, doc);

//				if (!bDoingSizeHint) {
					navigator.setDocumentToVerse(itrVerse->getIndex(), itrVerse->phraseTags(), defaultDocumentToVerseFlags | CPhraseNavigator::TRO_SearchResults);
					if ((m_private.m_nViewMode == CVerseListModel::VVME_SEARCH_RESULTS) ||
						(m_private.m_nViewMode == CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED)) {
						CSearchResultHighlighter srHighlighter(itrVerse->phraseTags(), (m_private.m_nViewMode != CVerseListModel::VVME_SEARCH_RESULTS));
						navigator.doHighlighting(srHighlighter);
						if (showHighlightersInSearchResults()) {
							const THighlighterTagMap *pmapHighlighterTags = userNotesDatabase()->highlighterTagsFor(m_private.m_pBibleDatabase);
							if (pmapHighlighterTags) {
								// Note: These are painted in sorted order so they overlay each other with alphabetical precedence:
								//			(the map is already sorted)
								TPhraseTag tagVerse = itrVerse->getWholeVersePhraseTag();
								for (THighlighterTagMap::const_iterator itrHighlighters = pmapHighlighterTags->begin(); itrHighlighters != pmapHighlighterTags->end(); ++itrHighlighters) {
									CUserDefinedHighlighter userHighlighter(itrHighlighters->first, itrHighlighters->second);
									navigator.doHighlighting(userHighlighter, false, tagVerse);
								}
							}
						}
					} else {
						CUserDefinedHighlighter userHighlighter(zResults.resultsName(), itrVerse->phraseTags());
						navigator.doHighlighting(userHighlighter);
					}
//				} else {
//					navigator.setDocumentToVerse(item.getIndex(), item.phraseTags(), CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_SearchResults);		// If not doing highlighting, no need to add anchors (improves search results rendering for size hints)
//				}
				return doc.toHtml();
			}
		}

		if ((role == TOOLTIP_ROLE) ||
			(role == TOOLTIP_PLAINTEXT_ROLE) ||
			(role == TOOLTIP_NOHEADING_ROLE) ||
			(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
			if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
				(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {

				const TVerseListModelSearchResults &zSearchResults = ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ? m_searchResults : m_searchResultsExcluded);

				// Switch to Search Results as our incoming index may not have been for Search Results,
				//		even though we are now in Search Results View Mode:
				itrVerse = zSearchResults.m_mapVerses.find(ndxVerse);
				if (itrVerse == zSearchResults.m_mapVerses.constEnd()) return QVariant();

				bool bHeading = ((role != TOOLTIP_NOHEADING_ROLE) && (role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE));
				QString strToolTip;
				if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
					(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "<qt><pre>";
				if (bHeading) strToolTip += itrVerse->getHeading() + "\n";
				QPair<int, int> nResultsIndexes = zSearchResults.GetResultsIndexes(itrVerse);
				if (nResultsIndexes.first != nResultsIndexes.second) {
					if (m_private.m_nViewMode != VVME_SEARCH_RESULTS_EXCLUDED) {
						strToolTip += QString("%1").arg(bHeading ? "    " : "") +
									tr("Search Results %1-%2 of %3 phrase occurrences", "Statistics")
												.arg(nResultsIndexes.first)
												.arg(nResultsIndexes.second)
												.arg(zSearchResults.GetResultsCount()) + "\n";
					} else {
						strToolTip += QString("%1").arg(bHeading ? "    " : "") +
									tr("Excluded Search Results %1-%2 of %3 phrase occurrences", "Statistics")
												.arg(nResultsIndexes.first)
												.arg(nResultsIndexes.second)
												.arg(zSearchResults.GetResultsCount()) + "\n";
					}
				} else {
					Q_ASSERT(nResultsIndexes.first != 0);		// This will assert if the row was beyond those defined in our list
					if (m_private.m_nViewMode != VVME_SEARCH_RESULTS_EXCLUDED) {
						strToolTip += QString("%1").arg(bHeading ? "    " : "") +
									tr("Search Result %1 of %2 phrase occurrences", "Statistics")
												.arg(nResultsIndexes.first)
												.arg(zSearchResults.GetResultsCount()) + "\n";
					} else {
						strToolTip += QString("%1").arg(bHeading ? "    " : "") +
									tr("Excluded Search Result %1 of %2 phrase occurrences", "Statistics")
												.arg(nResultsIndexes.first)
												.arg(zSearchResults.GetResultsCount()) + "\n";
					}
				}
				QPair<int, int> nVerseResult = zSearchResults.GetVerseIndexAndCount(itrVerse);
				if (nVerseResult.first != 0) {
					strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Verse %1 of %2 in Search Scope", "Statistics").arg(nVerseResult.first).arg(nVerseResult.second) + "\n";
				}
				QPair<int, int> nChapterResult = zSearchResults.GetChapterIndexAndCount(itrVerse);
				strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Chapter %1 of %2 in Search Scope", "Statistics").arg(nChapterResult.first).arg(nChapterResult.second) + "\n";
				QPair<int, int> nBookResult = zSearchResults.GetBookIndexAndCount(itrVerse);
				strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Book %1 of %2 in Search Scope", "Statistics").arg(nBookResult.first).arg(nBookResult.second) + "\n";
				QString strSearchScopeDescription = zSearchResults.m_searchResultsData.m_SearchCriteria.searchScopeDescription();
				if (m_private.m_nViewMode != VVME_SEARCH_RESULTS_EXCLUDED) {
					if (!strSearchScopeDescription.isEmpty()) {
						QString strSearchWithinDescription = zSearchResults.m_searchResultsData.m_SearchCriteria.searchWithinDescription(m_private.m_pBibleDatabase);
						if (!strSearchWithinDescription.isEmpty()) {
							strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Search Scope is: %1 within %2", "Statistics").arg(strSearchScopeDescription).arg(strSearchWithinDescription) + "\n";
						} else {
							strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Search Scope is: anywhere within %1", "Statistics").arg(strSearchScopeDescription) + "\n";
						}
					}
				} else {
					QString strSearchWithinDescription = zSearchResults.m_searchResultsData.m_SearchCriteria.searchWithinDescription(m_private.m_pBibleDatabase);
					if (!strSearchWithinDescription.isEmpty()) {
						strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Selected Search Text is: %1", "Statistics").arg(strSearchWithinDescription) + "\n";
					}
				}
				strToolTip += itrVerse->getToolTip(zSearchResults.m_searchResultsData);
				if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
					(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "</pre></qt>";
				return strToolTip;
			} else {
				return CPhraseNavigator::getToolTip(m_private.m_pBibleDatabase, TPhraseTag(ndxDisplayVerse), CSelectionPhraseTagList(itrVerse->phraseTags()));
			}
		}

		if (role == VERSE_ENTRY_ROLE) {
			return QVariant::fromValue(*itrVerse);
		}
	}

	return QVariant();
}

CRelIndex CVerseListModel::logicalIndexForModelIndex(const QModelIndex &index) const
{
	const TVerseIndex *pVerseIndex = toVerseIndex(index);
	Q_ASSERT(pVerseIndex != nullptr);

	CRelIndex ndxVerse = pVerseIndex->relIndex();
	if (!ndxVerse.isSet()) return CRelIndex();

	if ((m_private.m_nViewMode == VVME_CROSSREFS) &&
		(pVerseIndex->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE)) {
		const TRelativeIndexSet setCrossRefs = m_crossRefsResults.m_mapCrossRefs.crossReferencesFor(pVerseIndex->relIndex());
		Q_ASSERT((index.row() >= 0) && (static_cast<unsigned int>(index.row()) < setCrossRefs.size()));
		TRelativeIndexSet::const_iterator itrRef = setCrossRefs.begin();
		for (int i = index.row(); i > 0; ++itrRef, --i) { }
		ndxVerse = *(itrRef);
	}

	return ndxVerse;
}

QModelIndex CVerseListModel::modelIndexForLogicalIndex(const CRelIndex &ndxLogical) const
{
	if (!ndxLogical.isSet()) return QModelIndex();

	for (CModelRowForwardIterator fwdItr(this); fwdItr; ++fwdItr) {
		const TVerseIndex *pVerseIndex = toVerseIndex(*fwdItr);
		Q_ASSERT(pVerseIndex != nullptr);

		CRelIndex ndxVerse = pVerseIndex->relIndex();
		if (!ndxVerse.isSet()) continue;

		// TODO : Add support for crossrefs??

		if (ndxVerse == ndxLogical) return *fwdItr;
	}

	return QModelIndex();
}

CRelIndex CVerseListModel::navigationIndexForModelIndex(const QModelIndex &index) const
{
	return CRelIndex::navigationIndexFromLogicalIndex(logicalIndexForModelIndex(index));
}

bool CVerseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::SizeHintRole) {
		if (!index.isValid()) {
			// Special Case:  QModelIndex() is "invalidate all":
			clearAllSizeHints();
			emit cachedSizeHintsInvalidated();
			return false;				// But return false because we can't actually set a SizeHint for an invalid index
		}

		TVerseListModelResults &zResults = const_cast<TVerseListModelResults &>(results(index));

		zResults.m_mapSizeHints[index] = value.toSize();
		// Note: Do not fire dataChanged() here, as this is just a cache used by ReflowDelegate
		return true;
	}

/*
	if (index.row() < 0 || index.row() >= m_lstVerses.size()) return false;

	if ((role == Qt::EditRole) || (role == Qt::DisplayRole)) {
		switch (m_private.m_nDisplayMode) {
			case VDME_HEADING:
			case VDME_VERYPLAIN:
			case VDME_RICHTEXT:
			case VDME_COMPLETE:
				return false;		// read-only
		}
	}

	if ((role == TOOLTIP_ROLE) ||
		(role == TOOLTIP_PLAINTEXT_ROLE) ||
		(role == TOOLTIP_NOHEADING_ROLE) ||
		(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
		return false;				// read-only
	}

	if (role == VERSE_ENTRY_ROLE) {
		m_lstVerses.replace(index.row(), value.value<CVerseListItem>());
		emit dataChanged(index, index);
		return true;
	}
*/

	return false;
}

bool CVerseListModel::ascendingLessThanModelIndex(const QModelIndex &ndx1, const QModelIndex &ndx2)
{
	const TVerseIndex *vi1 = toVerseIndex(ndx1);
	const TVerseIndex *vi2 = toVerseIndex(ndx2);

	return (*vi1 < *vi2);
}

bool CVerseListModel::ascendingLessThanXRefTargets(const QModelIndex &ndx1, const QModelIndex &ndx2)
{
	const TVerseIndex *vi1 = toVerseIndex(ndx1);
	const TVerseIndex *vi2 = toVerseIndex(ndx2);

	CRelIndex ndxRel1(toVerseIndex(ndx1)->relIndex());
	if (vi1->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
		const TRelativeIndexSet setCrossRefs1 = ms_pCrossRefsMap->crossReferencesFor(ndxRel1);
		Q_ASSERT((ndx1.row() >= 0) && (static_cast<unsigned int>(ndx1.row()) < setCrossRefs1.size()));
		TRelativeIndexSet::const_iterator itrRef1 = setCrossRefs1.begin();
		for (int i = ndx1.row(); i > 0; ++itrRef1, --i) { }
		ndxRel1 = *(itrRef1);
	}
	CRelIndex ndxRel2(toVerseIndex(ndx2)->relIndex());
	if (vi2->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
		const TRelativeIndexSet setCrossRefs2 = ms_pCrossRefsMap->crossReferencesFor(ndxRel2);
		Q_ASSERT((ndx2.row() >= 0) && (static_cast<unsigned int>(ndx2.row()) < setCrossRefs2.size()));
		TRelativeIndexSet::const_iterator itrRef2 = setCrossRefs2.begin();
		for (int i = ndx2.row(); i > 0; ++itrRef2, --i) { }
		ndxRel2 = *(itrRef2);
	}
	return (ndxRel1 < ndxRel2);
}

bool CVerseListModel::descendingLessThanModelIndex(const QModelIndex &ndx1, const QModelIndex &ndx2)
{
	const TVerseIndex *vi1 = toVerseIndex(ndx1);
	const TVerseIndex *vi2 = toVerseIndex(ndx2);

	return (*vi2 < *vi1);
}

bool CVerseListModel::descendingLessThanXRefTargets(const QModelIndex &ndx1, const QModelIndex &ndx2)
{
	const TVerseIndex *vi1 = toVerseIndex(ndx1);
	const TVerseIndex *vi2 = toVerseIndex(ndx2);

	CRelIndex ndxRel1(toVerseIndex(ndx1)->relIndex());
	if (vi1->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
		const TRelativeIndexSet setCrossRefs1 = ms_pCrossRefsMap->crossReferencesFor(ndxRel1);
		Q_ASSERT((ndx1.row() >= 0) && (static_cast<unsigned int>(ndx1.row()) < setCrossRefs1.size()));
		TRelativeIndexSet::const_iterator itrRef1 = setCrossRefs1.begin();
		for (int i = ndx1.row(); i > 0; ++itrRef1, --i) { }
		ndxRel1 = *(itrRef1);
	}
	CRelIndex ndxRel2(toVerseIndex(ndx2)->relIndex());
	if (vi2->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE) {
		const TRelativeIndexSet setCrossRefs2 = ms_pCrossRefsMap->crossReferencesFor(ndxRel2);
		Q_ASSERT((ndx2.row() >= 0) && (static_cast<unsigned int>(ndx2.row()) < setCrossRefs2.size()));
		TRelativeIndexSet::const_iterator itrRef2 = setCrossRefs2.begin();
		for (int i = ndx2.row(); i > 0; ++itrRef2, --i) { }
		ndxRel2 = *(itrRef2);
	}
	return (ndxRel2 < ndxRel1);
}

// Static thread-locked data, locked in sortModelIndexList:
const TCrossReferenceMap *CVerseListModel::ms_pCrossRefsMap = nullptr;

void CVerseListModel::sortModelIndexList(QModelIndexList &lstIndexes, bool bUseCopySortOption) const
{
	static QAtomicInt nThreadLock(0);
	while (!nThreadLock.testAndSetRelaxed(0, 1)) {
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}	// Mutex wait for thread if someone is currently running this function
	Q_ASSERT(ms_pCrossRefsMap == nullptr);					// Thread-safeguard check
	ms_pCrossRefsMap = &m_crossRefsResults.m_mapCrossRefs;
	if (m_private.m_nViewMode == VVME_CROSSREFS) {
		if (bUseCopySortOption) {
			switch (CPersistentSettings::instance()->searchResultsVerseCopyOrder()) {
				case VCOE_SELECTED:
					break;
				case VCOE_BIBLE_ASCENDING:
					std::sort(lstIndexes.begin(), lstIndexes.end(), ascendingLessThanXRefTargets);
					break;
				case VCOE_BIBLE_DESCENDING:
					std::sort(lstIndexes.begin(), lstIndexes.end(), descendingLessThanXRefTargets);
					break;
				default:
					Q_ASSERT(false);
					break;
			}
		} else {
			std::sort(lstIndexes.begin(), lstIndexes.end(), ascendingLessThanXRefTargets);
		}
	} else {
		if (bUseCopySortOption) {
			switch (CPersistentSettings::instance()->searchResultsVerseCopyOrder()) {
				case VCOE_SELECTED:
					break;
				case VCOE_BIBLE_ASCENDING:
					std::sort(lstIndexes.begin(), lstIndexes.end(), ascendingLessThanModelIndex);
					break;
				case VCOE_BIBLE_DESCENDING:
					std::sort(lstIndexes.begin(), lstIndexes.end(), descendingLessThanModelIndex);
					break;
			}
		} else {
			std::sort(lstIndexes.begin(), lstIndexes.end(), ascendingLessThanModelIndex);
		}
	}
	ms_pCrossRefsMap = nullptr;
	nThreadLock = 0;
}

Qt::ItemFlags CVerseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	TVerseIndex *pVerseIndex = CVerseListModel::toVerseIndex(index);
	Q_ASSERT(pVerseIndex != nullptr);

	CRelIndex ndxRel(logicalIndexForModelIndex(index));
	if (m_private.m_nViewMode != VVME_CROSSREFS) {
		if ((ndxRel.isSet()) &&
			((ndxRel.verse() != 0)  ||
			 ((ndxRel.verse() == 0) && (ndxRel.word() != 0)) ||
			 ((m_private.m_nViewMode == VVME_USERNOTES) && (m_private.m_pUserNotesDatabase->existsNoteFor(ndxRel))))) {
			if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
#ifndef IS_MOBILE_APP
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled /* | Qt::ItemIsEditable */;		// | Qt::ItemIsDropEnabled;
#else
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable /* | Qt::ItemIsEditable */;		// | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
#endif
		}
	} else {
		if ((pVerseIndex->nodeType() == VLMNTE_CROSS_REFERENCE_SOURCE_NODE) ||
			(pVerseIndex->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE))
#ifndef IS_MOBILE_APP
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled /* | Qt::ItemIsEditable */;		// | Qt::ItemIsDropEnabled;
#else
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable /* | Qt::ItemIsEditable */;		// | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
#endif
	}

	if ((m_private.m_nViewMode == VVME_HIGHLIGHTERS) &&
		(pVerseIndex->nodeType() == VLMNTE_HIGHLIGHTER_NODE)) return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
	return Qt::ItemIsEnabled;		// | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool CVerseListModel::insertRows(int row, int count, const QModelIndex &zParent)
{
	Q_UNUSED(row);
	Q_UNUSED(count);
	Q_UNUSED(zParent);

	return false;
/*
	if (count < 1 || row < 0 || row > m_lstVerses.size())
		return false;
	if (zParent.isValid()) return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.insert(row, CVerseListItem());

	endInsertRows();

	return true;
*/
}

bool CVerseListModel::removeRows(int row, int count, const QModelIndex &zParent)
{
	Q_UNUSED(row);
	Q_UNUSED(count);
	Q_UNUSED(zParent);

	return false;

/*
	if (count <= 0 || row < 0 || (row + count) > m_lstVerses.size())
		return false;
	if (zParent.isValid()) return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.removeAt(row);

	endRemoveRows();

	return true;
*/
}

/*
static bool ascendingLessThan(const QPair<CVerseListItem, int> &s1, const QPair<CVerseListItem, int> &s2)
{
	// Both normalized and denormalized are in order, but it's more expensive
	//	 to convert to normal when we already have relative
	return s1.first.getIndexDenormalized() < s2.first.getIndexDenormalized();
}

static bool decendingLessThan(const QPair<CVerseListItem, int> &s1, const QPair<CVerseListItem, int> &s2)
{
	// Both normalized and denormalized are in order, but it's more expensive
	//	 to convert to normal when we already have relative
	return s1.first.getIndexDenormalized() > s2.first.getIndexDenormalized();
}
*/

void CVerseListModel::sort(int /* column */, Qt::SortOrder order)
{
	Q_UNUSED(order);
	Q_ASSERT(false);

/*

	emit layoutAboutToBeChanged();

	QList<QPair<CVerseListItem, int> > list;
	for (int i = 0; i < m_lstVerses.count(); ++i)
		list.append(QPair<CVerseListItem, int>(m_lstVerses.at(i), i));

	if (order == Qt::AscendingOrder)
		std::sort(list.begin(), list.end(), ascendingLessThan);
	else
		std::sort(list.begin(), list.end(), decendingLessThan);

	m_lstVerses.clear();
	QVector<int> forwarding(list.count());
	for (int i = 0; i < list.count(); ++i) {
		m_lstVerses.append(list.at(i).first);
		forwarding[list.at(i).second] = i;
	}

	QModelIndexList oldList = persistentIndexList();
	QModelIndexList newList;
	for (int i = 0; i < oldList.count(); ++i)
		newList.append(index(forwarding.at(oldList.at(i).row()), 0));
	changePersistentIndexList(oldList, newList);

	emit layoutChanged();
*/
}

// ----------------------------------------------------------------------------

Qt::DropActions CVerseListModel::supportedDropActions() const
{
	if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
		return Qt::MoveAction;
	} else {
#ifdef WORKAROUND_QTBUG_ABSLIST_DROP_ACTIONS
		return QAbstractItemModel::supportedDropActions();
#else
		return Qt::IgnoreAction;
#endif
	}
}

Qt::DropActions CVerseListModel::supportedDragActions() const
{
	if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
		return Qt::MoveAction | Qt::CopyAction;
	} else {
		return Qt::CopyAction;
	}
}

QStringList CVerseListModel::mimeTypes() const
{
	QStringList lstTypes;
	if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
		lstTypes << g_constrHighlighterPhraseTagListMimeType;
	}
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_TEXT)) {
		lstTypes << g_constrPlainTextMimeType;
	}
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_HTML)) {
		lstTypes << g_constrHTMLTextMimeType;
	}

	return lstTypes;
}

QMimeData *CVerseListModel::mimeData(const QModelIndexList &indexes) const
{
	if (indexes.isEmpty()) return nullptr;

	CBusyCursor iAmBusy(nullptr);

	QMimeData *pMimeData = nullptr;

	switch (m_private.m_nDisplayMode) {
		case VDME_COMPLETE:
			pMimeData = mimeDataFromCompleteVerseDetails(indexes);
			break;

		case VDME_HEADING:
			pMimeData = mimeDataFromVerseHeadings(indexes, false);
			break;

		case VDME_RICHTEXT:
			pMimeData = mimeDataFromVerseText(indexes, false);
			break;

		case VDME_VERYPLAIN:
			pMimeData = mimeDataFromRawVerseText(indexes, true);
			break;
	}

	if ((pMimeData != nullptr) &&
		(m_private.m_nViewMode == VVME_HIGHLIGHTERS)) {
		QByteArray baEncodedData;

		QDataStream aStream(&baEncodedData, QIODevice::WriteOnly);

		// Format is:  HighlighterName, VerseReference
		//	That way, we can move passages from any highlighter to any
		//		other target highlighter without them getting mixed up,
		//		and the highlighters will have already been masked by
		//		the verse-span:

		for (int ndx = 0; ndx < indexes.size(); ++ndx) {
			if (indexes.at(ndx).isValid()) {
				TVerseIndex *pVerseIndex = toVerseIndex(indexes.at(ndx));
				Q_ASSERT(pVerseIndex != nullptr);
				if (pVerseIndex == nullptr) continue;
				Q_ASSERT(pVerseIndex->resultsType() == VLMRTE_HIGHLIGHTERS);
				if (pVerseIndex->resultsType() != VLMRTE_HIGHLIGHTERS) continue;
				Q_ASSERT(pVerseIndex->nodeType() == VLMNTE_UNDEFINED);
				if (pVerseIndex->nodeType() != VLMNTE_UNDEFINED) continue;

				const CVerseListModel::TVerseListModelResults &zResults = results(indexes.at(ndx));
				aStream << zResults.resultsName();					// Highlighter name
				aStream << pVerseIndex->relIndex().asAnchor();		// Verse location
				Q_ASSERT(pVerseIndex->relIndex().word() == 1);
			}
		}

		pMimeData->setData(g_constrHighlighterPhraseTagListMimeType, baEncodedData);
	}

	return pMimeData;
}

bool CVerseListModel::dropMimeData(const QMimeData *pData, Qt::DropAction nAction, int nRow, int nColumn, const QModelIndex &zParent)
{
	if (nAction == Qt::IgnoreAction) return true;

	Q_UNUSED(nRow);
	Q_UNUSED(nColumn);

	Q_ASSERT(pData != nullptr);
	if (pData == nullptr) return false;

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	if ((nAction == Qt::MoveAction) &&
		(m_private.m_nViewMode == VVME_HIGHLIGHTERS) &&
		(pData->hasFormat(g_constrHighlighterPhraseTagListMimeType))) {
		if (!zParent.isValid()) return false;

		TVerseIndex *pVerseIndex = toVerseIndex(zParent);
		Q_ASSERT(pVerseIndex != nullptr);
		if (pVerseIndex == nullptr) return false;
		Q_ASSERT(pVerseIndex->nodeType() == VLMNTE_HIGHLIGHTER_NODE);
		const CVerseListModel::TVerseListModelResults &zResults = results(zParent);
		QString strTargetHighlighter = zResults.resultsName();
		Q_ASSERT(m_private.m_pUserNotesDatabase->existsHighlighter(strTargetHighlighter));
		if (!m_private.m_pUserNotesDatabase->existsHighlighter(strTargetHighlighter)) return false;
		Q_ASSERT(zResults.resultsType() == VLMRTE_HIGHLIGHTERS);
		if (zResults.resultsType() != VLMRTE_HIGHLIGHTERS) return false;

		QByteArray baEncodedData = pData->data(g_constrHighlighterPhraseTagListMimeType);
		QDataStream stream(&baEncodedData, QIODevice::ReadOnly);
		QList< QPair<QString, CRelIndex> > lstHighlighterIndexPairs;

		bool bHaveAtLeastOneUniqueSource = false;			// Set to true if at least one source highlighter is different from the target (i.e. if we have something to move)
		while (!stream.atEnd()) {
			QString strHighlighter;
			QString strTagAnchor;
			stream >> strHighlighter;
			stream >> strTagAnchor;
			CRelIndex ndxTagAnchor(strTagAnchor);
			Q_ASSERT(ndxTagAnchor.isSet());
			if (!ndxTagAnchor.isSet()) continue;
			lstHighlighterIndexPairs << qMakePair(strHighlighter, ndxTagAnchor);
			if (strHighlighter.compare(strTargetHighlighter) != 0) bHaveAtLeastOneUniqueSource = true;
			Q_ASSERT(m_private.m_pUserNotesDatabase->existsHighlighter(strHighlighter));
			if (!m_private.m_pUserNotesDatabase->existsHighlighter(strHighlighter)) return false;
			const CVerseListModel::TVerseListModelResults &zTargetResults = highlighterResults(strHighlighter);
			Q_ASSERT(zTargetResults.resultsType() == VLMRTE_HIGHLIGHTERS);
			if (zTargetResults.resultsType() != VLMRTE_HIGHLIGHTERS) return false;
			Q_ASSERT(zTargetResults.m_mapVerses.contains(ndxTagAnchor));
			if (!zTargetResults.m_mapVerses.contains(ndxTagAnchor)) return false;
		}
		if (!bHaveAtLeastOneUniqueSource) return false;

		Q_ASSERT(!g_pMyApplication.isNull());
		CKJVCanOpener *pCanOpener = g_pMyApplication->activeCanOpener();
		Q_ASSERT(pCanOpener != nullptr);

		int nResult = displayInformation(pCanOpener, tr("Moving Highlighter Tags", "Errors"),
											   tr("You are about to move the selected verse highlighting to the \"%1\" highlighter.  This will "
												  "merge those passages into this target highlighter, changing their color to match the target "
												  "highlighter.  This operation cannot be undone!\n\n"
												  "Are you sure you wish to move the selected verse highlighting to \"%1\"?", "Errors").arg(strTargetHighlighter),
										(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
		if (nResult != QMessageBox::Yes) return false;

		for (int ndxVerse = 0; ndxVerse < lstHighlighterIndexPairs.size(); ++ndxVerse) {
			if (lstHighlighterIndexPairs.at(ndxVerse).first.compare(strTargetHighlighter) == 0) continue;		// No need to copy to self

			const CVerseListModel::TVerseListModelResults &zTargetResults = highlighterResults(lstHighlighterIndexPairs.at(ndxVerse).first);
			Q_ASSERT(zTargetResults.resultsType() == VLMRTE_HIGHLIGHTERS);
			if (zTargetResults.resultsType() != VLMRTE_HIGHLIGHTERS) continue;

			CVerseMap::const_iterator itrVerse = zTargetResults.m_mapVerses.find(lstHighlighterIndexPairs.at(ndxVerse).second);
			Q_ASSERT(itrVerse != zTargetResults.m_mapVerses.constEnd());
			if (itrVerse == zTargetResults.m_mapVerses.constEnd()) continue;

			// Note: MUST build this as a copy or else we must do the append before the remove below
			//			or else the highlighter change notification will change our list and cause the
			//			list to change on us before the append!
			TPhraseTagList lstTags(itrVerse->phraseTags());
			m_private.m_pUserNotesDatabase->removeHighlighterTagsFor(m_private.m_pBibleDatabase, lstHighlighterIndexPairs.at(ndxVerse).first, lstTags);
			m_private.m_pUserNotesDatabase->appendHighlighterTagsFor(m_private.m_pBibleDatabase, strTargetHighlighter, lstTags);
		}

		return true;
	}
#endif

	return false;
}

// ----------------------------------------------------------------------------

QMimeData *CVerseListModel::mimeDataFromVerseText(const QModelIndexList &lstVersesUnsorted, bool bVerseTextOnly) const
{
	Q_ASSERT(!m_private.m_pBibleDatabase.isNull());

	QModelIndexList lstVerses = lstVersesUnsorted;
	sortModelIndexList(lstVerses, true);

	CScriptureTextHtmlBuilder docVerseHTML;

	bool bUsingData = (!bVerseTextOnly) && ((m_private.m_nViewMode == VVME_USERNOTES) || (m_private.m_nDisplayMode != VDME_RICHTEXT));

	if (bUsingData) {
		QString strCopyFont= "font-size:medium;";
		switch (CPersistentSettings::instance()->copyFontSelection()) {
			case CPhraseNavigator::CFSE_NONE:
				break;
			case CPhraseNavigator::CFSE_COPY_FONT:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
				break;
			case CPhraseNavigator::CFSE_SCRIPTURE_BROWSER:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
				break;
			case CPhraseNavigator::CFSE_SEARCH_RESULTS:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
				break;
			default:
				Q_ASSERT(false);
				break;
		}

		docVerseHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
											"<style type=\"text/css\">\n"
											"body, p, li, .bodyIndent { white-space: pre-wrap; %1 }\n"
											"</style></head><body>\n")
											.arg(strCopyFont));																// Copy Font
	}

	QTextDocument docList;
	QTextCursor cursorDocList(&docList);
	bool bFirst = true;
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		if (!bUsingData) {
			const CVerseListItem &item(data(lstVerses.at(ndx), VERSE_ENTRY_ROLE).value<CVerseListItem>());
			if (item.verseIndex().isNull()) continue;

			QTextDocument docVerse;
			CPhraseNavigator navigator(m_private.m_pBibleDatabase, docVerse);

//			if (!bFirst) cursorDocList.insertHtml("<hr />\n");
			if (!bFirst) {
				cursorDocList.insertHtml("<br />\n");
				if (CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses()) cursorDocList.insertHtml("<br />\n");
			}

			// Note:  Qt bug with fragments causes leading <hr /> tags
			//		to get converted to <br /> tags.  Since this may
			//		change on us if/when they get it fixed, we'll pass
			//		TRO_None here and set our <hr /> or <br /> below as
			//		desired:
			CPhraseNavigator::TextRenderOptionFlags troCopy = defaultDocumentToVerseFlags | CPhraseNavigator::TRO_Copying | CPhraseNavigator::TRO_SearchResults;
			if ((!bVerseTextOnly) && (m_private.m_nViewMode == VVME_USERNOTES)) troCopy |= CPhraseNavigator::TRO_UserNotes | CPhraseNavigator::TRO_UserNotesForceVisible;
			navigator.setDocumentToVerse(item.getIndex(), item.phraseTags(), troCopy);
			if (!bVerseTextOnly) {
				if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
					(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {
					CSearchResultHighlighter highlighter(item.phraseTags(), (m_private.m_nViewMode != VVME_SEARCH_RESULTS));
					navigator.doHighlighting(highlighter);
				} else if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
					CUserDefinedHighlighter highlighter(results(*item.verseIndex()).resultsName(), item.phraseTags());
					navigator.doHighlighting(highlighter);
				}
			}
			navigator.removeAnchors();

			QTextDocumentFragment fragment(&docVerse);
			cursorDocList.insertFragment(fragment);
		} else {
			if (!bFirst) {
				if (m_private.m_nViewMode == VVME_USERNOTES) {
					docVerseHTML.insertHorizontalRule();
				} else {
					docVerseHTML.addLineBreak();
					if (CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses()) docVerseHTML.addLineBreak();
				}
			}
			docVerseHTML.appendRawText(data(lstVerses.at(ndx), VERSE_COPYING_ROLE).toString());
		}

		bFirst = false;
	}

	if (bUsingData) {
		docVerseHTML.appendRawText("</body></html>");
		QString strHTML = docVerseHTML.getResult();
		strHTML.remove(QChar('\n'));			// Keep text-only version from having extra newlines
		docList.setHtml(strHTML);
	}

	QMimeData *mime = new QMimeData();
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_TEXT)) {
		mime->setText(docList.toPlainText());
	}
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_HTML)) {
		mime->setHtml(docList.toHtml());
	}
	return mime;
}

QMimeData *CVerseListModel::mimeDataFromRawVerseText(const QModelIndexList &lstVersesUnsorted, bool bVeryRaw) const
{
	Q_ASSERT(!m_private.m_pBibleDatabase.isNull());

	QModelIndexList lstVerses = lstVersesUnsorted;
	sortModelIndexList(lstVerses, true);

	QString strText;
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(data(lstVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		if (item.verseIndex().isNull()) continue;

		if (!bVeryRaw) {
			strText += item.getVersePlainText(true) + "\n";
		} else {
			strText += item.getVerseVeryPlainText() + "\n";
		}

		if (CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses()) strText += "\n";
	}

	QMimeData *mime = new QMimeData();
	mime->setText(strText);
	return mime;
}

QMimeData *CVerseListModel::mimeDataFromVerseHeadings(const QModelIndexList &lstVersesUnsorted, bool bHeadingTextOnly) const
{
	Q_ASSERT(!m_private.m_pBibleDatabase.isNull());

	QModelIndexList lstVerses = lstVersesUnsorted;
	sortModelIndexList(lstVerses, true);

	QTextDocument docHeadings;
	CScriptureTextHtmlBuilder docHeadingsHTML;
	bool bFirst = true;

	QString strCopyFont= "font-size:medium;";
	switch (CPersistentSettings::instance()->copyFontSelection()) {
		case CPhraseNavigator::CFSE_NONE:
			break;
		case CPhraseNavigator::CFSE_COPY_FONT:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
			break;
		case CPhraseNavigator::CFSE_SCRIPTURE_BROWSER:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
			break;
		case CPhraseNavigator::CFSE_SEARCH_RESULTS:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	docHeadingsHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
										"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
										"<style type=\"text/css\">\n"
										"body, p, li, .bodyIndent { white-space: pre-wrap; %1 }\n"
										"</style></head><body>\n")
										.arg(strCopyFont));																// Copy Font

	if ((m_private.m_nViewMode == VVME_CROSSREFS) ||
		(m_private.m_nViewMode == VVME_USERNOTES)) {

		for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
			CRelIndex ndxRel = logicalIndexForModelIndex(lstVerses.at(ndx));
			if (ndxRel.isSet()) {
				//	strVerseHeadings += m_private.m_pBibleDatabase->PassageReferenceText(ndxRel) + "\n";

				if (!bFirst) {
					if ((m_private.m_nViewMode == VVME_CROSSREFS) || (bHeadingTextOnly)) {
						if (CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses()) docHeadingsHTML.addLineBreak();
					} else if (m_private.m_nViewMode == VVME_USERNOTES) {
						docHeadingsHTML.insertHorizontalRule();
					}
				}
				bFirst = false;

				CRelIndex ndxDisplayVerse(ndxRel);
				ndxDisplayVerse.setWord(0);				// Note we don't allow notes/crossrefs on colophons and superscriptions, so this is safe

				if (!bHeadingTextOnly) docHeadingsHTML.beginBold();
				docHeadingsHTML.appendLiteralText(m_private.m_pBibleDatabase->PassageReferenceText(ndxDisplayVerse));
				if (!bHeadingTextOnly) docHeadingsHTML.endBold();
				docHeadingsHTML.addLineBreak();

				if ((!bHeadingTextOnly) && (m_private.m_nViewMode == VVME_USERNOTES)) {
					if (m_userNotesResults.m_mapVerses.contains(ndxRel)) {
						docHeadingsHTML.addNoteFor(ndxDisplayVerse, false, true);
					}
				}
			}
		}
	} else {
		for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
			const CVerseListItem &item(data(lstVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
			if (item.verseIndex().isNull()) continue;

			if ((!bFirst) && (CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses())) docHeadingsHTML.addLineBreak();
			bFirst = false;

			docHeadingsHTML.appendLiteralText(item.getHeading(true));
			docHeadingsHTML.addLineBreak();
		}
	}

	docHeadingsHTML.appendRawText("</body></html>");
	QString strHTML = docHeadingsHTML.getResult();
	strHTML.remove(QChar('\n'));			// Keep text-only version from having extra newlines
	docHeadings.setHtml(strHTML);

	QMimeData *mime = new QMimeData();
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_TEXT)) {
		mime->setText(docHeadings.toPlainText());
	}
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_HTML)) {
		mime->setHtml(docHeadings.toHtml());
	}
	return mime;
}

QMimeData *CVerseListModel::mimeDataFromReferenceDetails(const QModelIndexList &lstVersesUnsorted) const
{
	QModelIndexList lstVerses = lstVersesUnsorted;
	sortModelIndexList(lstVerses, true);

	QString strPlainText;
	QString strRichText;
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		if (ndx > 0) {
			strPlainText += "--------------------\n";
			strRichText += "<hr />\n";
		}
		strPlainText += data(lstVerses.at(ndx), CVerseListModel::TOOLTIP_PLAINTEXT_ROLE).toString();
		strRichText += data(lstVerses.at(ndx), CVerseListModel::TOOLTIP_ROLE).toString();
	}

	QMimeData *mime = new QMimeData();
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_TEXT)) {
		mime->setText(strPlainText);
	}
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_HTML)) {
		mime->setHtml(strRichText);
	}
	return mime;
}

QMimeData *CVerseListModel::mimeDataFromCompleteVerseDetails(const QModelIndexList &lstVersesUnsorted) const
{
	Q_ASSERT(!m_private.m_pBibleDatabase.isNull());

	QModelIndexList lstVerses = lstVersesUnsorted;
	sortModelIndexList(lstVerses, true);

	QTextDocument docList;
	QTextCursor cursorDocList(&docList);
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(data(lstVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		if (item.verseIndex().isNull()) continue;
		QTextDocument docVerse;
		CPhraseNavigator navigator(m_private.m_pBibleDatabase, docVerse);

		// Note:  Qt bug with fragments causes leading <hr /> tags
		//		to get converted to <br /> tags.  Since this may
		//		change on us if/when they get it fixed, we'll pass
		//		TRO_None here and set our <hr /> or <br /> below as
		//		desired:
		navigator.setDocumentToVerse(item.getIndex(), item.phraseTags(), defaultDocumentToVerseFlags | CPhraseNavigator::TRO_Copying | CPhraseNavigator::TRO_SearchResults);
		if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
			(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {
			CSearchResultHighlighter highlighter(item.phraseTags(), (m_private.m_nViewMode != VVME_SEARCH_RESULTS));
			navigator.doHighlighting(highlighter);
		} else if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
			CUserDefinedHighlighter highlighter(results(*item.verseIndex()).resultsName(), item.phraseTags());
			navigator.doHighlighting(highlighter);
		}
		navigator.removeAnchors();

		QTextDocumentFragment fragment(&docVerse);
		cursorDocList.insertFragment(fragment);

		if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
			(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {
			cursorDocList.insertHtml("<br />\n<pre>" + data(lstVerses.at(ndx), TOOLTIP_NOHEADING_PLAINTEXT_ROLE).toString() + "</pre>\n");
			if (ndx != (lstVerses.size()-1)) cursorDocList.insertHtml("\n<hr /><br />\n");
		} else {
			cursorDocList.insertHtml("<br />\n");
		}
	}

	QMimeData *mime = new QMimeData();
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_TEXT)) {
		mime->setText(docList.toPlainText());
	}
	if ((CPersistentSettings::instance()->copyMimeType() == CMTE_ALL) ||
		(CPersistentSettings::instance()->copyMimeType() == CMTE_HTML)) {
		mime->setHtml(docList.toHtml());
	}
	return mime;
}

// ----------------------------------------------------------------------------

QModelIndex CVerseListModel::locateIndex(const TVerseIndex &ndxVerse) const
{
	const CRelIndex &ndxRel = ndxVerse.relIndex();
	if (!ndxRel.isSet()) return QModelIndex();

	const TVerseListModelResults &zResults = results(resolveVerseIndex(ndxRel, results(ndxVerse).resultsName(), VLMRTE_UNDEFINED));

	// See if this is a verse (search result) reference.  If so resolve:
	if ((ndxRel.verse() != 0) ||
		((ndxRel.verse() == 0) && (ndxRel.word() != 0))) {
		CVerseMap::const_iterator itrFirst;
		CVerseMap::const_iterator itrTarget;

		// Set ndxVerse to first verse m_lstVerses array for this parent node:
		if (m_private.m_nTreeMode == VTME_LIST) {
			itrFirst = zResults.m_mapVerses.constBegin();		// For list mode, the list includes everything, so start with the first index
		} else {
			itrFirst = zResults.GetVerse(0, ndxRel.book(), ((m_private.m_nTreeMode == VTME_TREE_CHAPTERS ) ? ndxRel.chapter() : -1));
		}
		if (itrFirst == zResults.m_mapVerses.constEnd()) return QModelIndex();
		itrTarget = zResults.FindVerseIndex(ndxRel);
		if (itrTarget == zResults.m_mapVerses.constEnd()) return QModelIndex();
		return createIndex(std::distance(itrFirst, itrTarget), 0, fromVerseIndex(itrTarget->verseIndex().data()));		// Use index from actual verse instead of ndxRel since word() isn't required to match
	}

	// If we are in list mode and caller only gave us a book/chapter reference,
	//		then we simply don't have an index to return.  That makes more
	//		sense than possibly returning the first one that might match:
	if (m_private.m_nTreeMode == VTME_LIST) return QModelIndex();

	if (((ndxRel.chapter() != 0) ||
		 ((ndxRel.chapter() == 0) && (ndxRel.word() != 0))) &&
		(m_private.m_nTreeMode == VTME_TREE_CHAPTERS)) {
		// If this is a book/chapter reference, resolve it:
		int ndxTarget = zResults.IndexByChapter(ndxRel.book(), ndxRel.chapter());
		if (ndxTarget == -1) return QModelIndex();
		CRelIndex ndxChapter(ndxRel.book(), ndxRel.chapter(), 0, 0);			// Create CRelIndex rather than using ndxRel, since we aren't requiring word() to match
		if (zResults.FindVerseIndex(ndxChapter) == zResults.m_mapVerses.constEnd()) return QModelIndex();
		return createIndex(ndxTarget, 0, fromVerseIndex(zResults.extraVerseIndex(ndxChapter, VLMNTE_CHAPTER_TERMINATOR_NODE).data()));
	} else {
		// If this is a book-only reference, resolve it:
		int ndxTarget = zResults.IndexByBook(ndxRel.book());
		if (ndxTarget == -1) return QModelIndex();
		CRelIndex ndxBook(ndxRel.book(), 0, 0, 0);			// Create CRelIndex rather than using ndxRel, since we aren't requiring word() to match
		if (zResults.FindVerseIndex(ndxBook) == zResults.m_mapVerses.constEnd()) return QModelIndex();
		return createIndex(ndxTarget, 0, fromVerseIndex(zResults.extraVerseIndex(ndxBook, VLMNTE_BOOK_TERMINATOR_NODE).data()));
	}

	return QModelIndex();
}

TVerseIndex CVerseListModel::resolveVerseIndex(const CRelIndex &ndxRel, const QString &strResultsName, VERSE_LIST_MODEL_RESULTS_TYPE_ENUM nResultsType) const
{
	VERSE_VIEW_MODE_ENUM nViewMode = ((nResultsType != VLMRTE_UNDEFINED) ? VLMRTE_to_VVME(nResultsType) : m_private.m_nViewMode);
	switch (nViewMode) {
		case VVME_SEARCH_RESULTS:
			return TVerseIndex(ndxRel, VLMRTE_SEARCH_RESULTS);
		case VVME_SEARCH_RESULTS_EXCLUDED:
			return TVerseIndex(ndxRel, VLMRTE_SEARCH_RESULTS_EXCLUDED);
		case VVME_HIGHLIGHTERS:
			if (!strResultsName.isEmpty()) {
				for (int ndxHighlighter = 0; ndxHighlighter < m_vlmrListHighlighters.size(); ++ndxHighlighter) {
					if (m_vlmrListHighlighters.at(ndxHighlighter).resultsName().compare(strResultsName, Qt::CaseInsensitive) == 0)
						return TVerseIndex(ndxRel, VLMRTE_HIGHLIGHTERS, VLMNTE_UNDEFINED, ndxHighlighter);
				}
			}
			break;
		case VVME_USERNOTES:
			return TVerseIndex(ndxRel, VLMRTE_USER_NOTES);
		case VVME_CROSSREFS:
			return TVerseIndex(ndxRel, VLMRTE_CROSS_REFS);
		default:
			Q_ASSERT(false);
	}

	return TVerseIndex(ndxRel);			// with: VLMRTE_UNDEFINED and VLMNTE_UNDEFINED
}

// ----------------------------------------------------------------------------

void CVerseListModel::setParsedPhrases(const CSearchResultsData &searchResultsData)
{
	// Note: Basic setting of this list doesn't change the model, as the phrases
	//		themselves are used primarily for building of tooltips that are
	//		appropriate for the entire search scope.  However, once these are
	//		set, we'll call the buildVerseListFromParsedPhrases function that
	//		will build and set the VerseList, which will change the model.
	//		Therefore, the beginResetModel/endResetModel calls don't exist here,
	//		but down in buildScopedResultsFromParsedPhrases():
	buildScopedResultsFromParsedPhrases(searchResultsData);
}

// ----------------------------------------------------------------------------

void CVerseListModel::en_highlighterTagsChanged(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName)
{
	if ((pBibleDatabase.isNull()) ||
		(pBibleDatabase->highlighterUUID().compare(m_private.m_pBibleDatabase->highlighterUUID(), Qt::CaseInsensitive) == 0)) {
		if (strUserDefinedHighlighterName.isEmpty()) {
			// Rebuild all highlighters if this is a broadcast for all highlighters
			buildHighlighterResults();
		} else {
			TVerseIndex verseIndex = resolveVerseIndex(CRelIndex(), strUserDefinedHighlighterName, VLMRTE_HIGHLIGHTERS);
			buildHighlighterResults(verseIndex.specialIndex());
		}
	}
}

void CVerseListModel::en_changedHighlighters()
{
	buildHighlighterResults();
}

void CVerseListModel::buildHighlighterResults(int ndxHighlighter)
{
	if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
		emit verseListAboutToChange();
		emit beginResetModel();
	} else if ((m_private.m_bShowHighlightersInSearchResults) && ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) || (m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED))) {
		emit layoutAboutToBeChanged();
	}

	if (ndxHighlighter == -1) {
		m_vlmrListHighlighters.clear();
		const TUserDefinedColorMap mapHighlighters = m_private.m_pUserNotesDatabase->highlighterDefinitionsMap();
		ndxHighlighter = 0;
		for (TUserDefinedColorMap::const_iterator itrHighlighters = mapHighlighters.constBegin(); itrHighlighters != mapHighlighters.constEnd(); ++itrHighlighters) {
			// Must add it to our list before calling buildHighlighterResults(ndx):
			m_vlmrListHighlighters.push_back(TVerseListModelResults(&m_private, itrHighlighters.key(), VLMRTE_HIGHLIGHTERS, VLMNTE_UNDEFINED, ndxHighlighter));
			buildHighlighterResults(ndxHighlighter, m_private.m_pUserNotesDatabase->highlighterTagsFor(m_private.m_pBibleDatabase, itrHighlighters.key()));
			ndxHighlighter++;
		}

//		const THighlighterTagMap *pHighlighters = m_private.m_pUserNotesDatabase->highlighterTagsFor(m_private.m_pBibleDatabase);
//		if (pHighlighters != nullptr) {
//			ndxHighlighter = 0;
//			for (THighlighterTagMap::const_iterator itrHighlighters = pHighlighters->begin(); itrHighlighters != pHighlighters->end(); ++itrHighlighters) {
//				// Must add it to our list before calling buildHighlighterResults(ndx):
//				m_vlmrListHighlighters.push_back(TVerseListModelResults(&m_private, itrHighlighters->first, ndxHighlighter));
//				buildHighlighterResults(ndxHighlighter, &(itrHighlighters->second));
//				ndxHighlighter++;
//			}
//		}
	} else {
		Q_ASSERT((ndxHighlighter >= 0) && (ndxHighlighter < m_vlmrListHighlighters.size()));
		TVerseListModelResults &zResults = const_cast<TVerseListModelResults &>(results(VLMRTE_HIGHLIGHTERS, ndxHighlighter));

		buildHighlighterResults(ndxHighlighter, m_private.m_pUserNotesDatabase->highlighterTagsFor(m_private.m_pBibleDatabase, zResults.resultsName()));
	}

	if (m_private.m_nViewMode == VVME_HIGHLIGHTERS) {
		emit endResetModel();
		emit verseListChanged();
	} else if ((m_private.m_bShowHighlightersInSearchResults) && ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) || (m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED))) {
		emit layoutChanged();
	}
}

void CVerseListModel::buildHighlighterResults(int ndxHighlighter, const TPhraseTagList *pTags)
{
	Q_ASSERT((ndxHighlighter >= 0) && (ndxHighlighter < m_vlmrListHighlighters.size()));
	TVerseListModelResults &zResults = const_cast<TVerseListModelResults &>(results(VLMRTE_HIGHLIGHTERS, ndxHighlighter));

	zResults.m_mapVerses.clear();
	zResults.m_lstVerseIndexes.clear();
	zResults.m_mapExtraVerseIndexes.clear();
	zResults.m_mapSizeHints.clear();

	if (pTags) {
		for (TPhraseTagList::const_iterator itrTags = pTags->constBegin(); itrTags != pTags->constEnd(); ++itrTags) {
			CRelIndex ndxNextRelative = itrTags->relIndex();
			unsigned int nWordCount = itrTags->count();
			Q_ASSERT(nWordCount != 0);			// Shouldn't have any highlighter tags with empty ranges
			nWordCount += ((ndxNextRelative.word() != 0) ? (ndxNextRelative.word() - 1) : 0);					// Calculate back to start of verse to figure out how many verses this tag encompasses
			ndxNextRelative.setWord(1);
			uint32_t ndxNormalNext = m_private.m_pBibleDatabase->NormalizeIndex(ndxNextRelative);
			if (ndxNormalNext == 0) continue;												// Handle cases for higlighters from other databases that extend beyond this one (and highlighter isn't part of this)
			if (m_private.m_pBibleDatabase->verseEntry(ndxNextRelative) == nullptr) continue;	// Handle cases for highlighters from othe database beginning in a colophon/superscription not supported in this database
			while (nWordCount > 0) {
				// Mask the highlighter tags for this verse and just insert the tags corresponding to this verse:
				const CVerseEntry *pVerseEntry = m_private.m_pBibleDatabase->verseEntry(ndxNextRelative);
				unsigned int nNumWordsInVerse = ((pVerseEntry != nullptr) ? pVerseEntry->m_nNumWrd : 0);
				TPhraseTag tagMasked = itrTags->mask(m_private.m_pBibleDatabase.data(), TPhraseTag(ndxNextRelative, nNumWordsInVerse));
				if (zResults.m_mapVerses.contains(ndxNextRelative)) {
					zResults.m_mapVerses[ndxNextRelative].addPhraseTag(tagMasked);
				} else {
					zResults.m_mapVerses.insert(ndxNextRelative, CVerseListItem(zResults.makeVerseIndex(ndxNextRelative), m_private.m_pBibleDatabase, tagMasked));
				}
				if (nNumWordsInVerse >= nWordCount) {
					nWordCount = 0;
				} else {
					nWordCount -= nNumWordsInVerse;
					// Add number of words in verse to find start of next verse:
					ndxNormalNext += nNumWordsInVerse;
					ndxNextRelative = m_private.m_pBibleDatabase->DenormalizeIndex(ndxNormalNext);
					Q_ASSERT((ndxNextRelative.word() == 1) ||
						   (!ndxNextRelative.isSet()));			// We better end up at the first word of the next verse (or out of our text, in cases of cross-database highlighter sharing), or something bad happened
					if (!ndxNextRelative.isSet()) nWordCount = 0;
					ndxNextRelative.setWord(1);					// But, add as whole verse
				}
			}
		}
	}

	zResults.m_lstVerseIndexes.reserve(zResults.m_mapVerses.size());
	for (CVerseMap::const_iterator itr = zResults.m_mapVerses.constBegin(); (itr != zResults.m_mapVerses.constEnd()); ++itr) {
		zResults.m_lstVerseIndexes.append(itr.key());
	}
}

// ----------------------------------------------------------------------------

void CVerseListModel::setUserNoteKeywordFilter(const QStringList &lstKeywordFilter)
{
	m_lstUserNoteKeywordFilter = lstKeywordFilter;
	if (m_private.m_nViewMode == VVME_USERNOTES) buildUserNotesResults();
}

void CVerseListModel::en_changedUserNote(const CRelIndex &ndx)
{
	Q_UNUSED(ndx);				// TODO : Add logic to use ndx to insert/remove a single note if we can
	if (m_private.m_nViewMode == VVME_USERNOTES) {
		emit layoutAboutToBeChanged();
		m_userNotesResults.m_mapSizeHints.clear();
		emit layoutChanged();
	}
}

void CVerseListModel::en_addedUserNote(const CRelIndex &ndx)
{
	buildUserNotesResults(ndx, true);
}

void CVerseListModel::en_removedUserNote(const CRelIndex &ndx)
{
	buildUserNotesResults(ndx, false);
}

void CVerseListModel::buildUserNotesResults(const CRelIndex &ndx, bool bAdd)
{
	Q_UNUSED(ndx);				// TODO : Add logic to use ndx to insert/remove a single note if we can
	Q_UNUSED(bAdd);

	TVerseListModelNotesResults &zResults = m_userNotesResults;

	if (m_private.m_nViewMode == VVME_USERNOTES) {
		emit verseListAboutToChange();
		emit beginResetModel();
	}

	bool bShowNotesWithoutKeywords = m_lstUserNoteKeywordFilter.contains(QString());

	zResults.m_mapVerses.clear();
	zResults.m_lstVerseIndexes.clear();
	zResults.m_mapExtraVerseIndexes.clear();
	zResults.m_mapSizeHints.clear();

	const CUserNoteEntryMap &mapNotes = m_private.m_pUserNotesDatabase->notesMap();
	zResults.m_lstVerseIndexes.reserve(mapNotes.size());

	for (CUserNoteEntryMap::const_iterator itrNote = mapNotes.begin(); itrNote != mapNotes.end(); ++itrNote) {
		CRelIndex ndxNote = (itrNote->first);
		Q_ASSERT(ndxNote.isSet());
		if ((ndxNote.chapter() != 0) && (ndxNote.verse() != 0)) {
			ndxNote.setWord(1);			// Whole verses only
		} else {
			ndxNote.setWord(0);			// We don't allow notes on colophons and superscriptions
		}

		if (m_private.m_pBibleDatabase->NormalizeIndex(ndxNote) == 0) continue;		// Don't include notes for references outside our database (like Aprocrypha notes outside on database without Apocrypha)

		const QStringList &noteKeywordList = (itrNote->second).keywordList();

		bool bInclude = false;

		if ((m_lstUserNoteKeywordFilter.isEmpty()) ||
			((bShowNotesWithoutKeywords) && (noteKeywordList.isEmpty()))) {
			bInclude = true;
		} else {
			for (int n = 0; n < noteKeywordList.size(); ++n) {
				if (m_lstUserNoteKeywordFilter.contains(CSearchStringListModel::decompose(noteKeywordList.at(n), false), Qt::CaseInsensitive)) {
					bInclude = true;
					break;
				}
			}
		}

		if (bInclude) {
			Q_ASSERT(!zResults.m_mapVerses.contains(ndxNote));
			zResults.m_mapVerses.insert(ndxNote, CVerseListItem(zResults.makeVerseIndex(ndxNote), m_private.m_pBibleDatabase));
			zResults.m_lstVerseIndexes.append(ndxNote);
		}
	}

	if (m_private.m_nViewMode == VVME_USERNOTES) {
		emit endResetModel();
		emit verseListChanged();
	}
}

// ----------------------------------------------------------------------------

void CVerseListModel::en_addedCrossRef(const CRelIndex &ndxRef1, const CRelIndex &ndxRef2)
{
	Q_UNUSED(ndxRef1);			// TODO : Add logic to use ndxRef1/ndxRef2 to insert/remove a single cross-ref if we can
	Q_UNUSED(ndxRef2);
	buildCrossRefsResults();
}

void CVerseListModel::en_removedCrossRef(const CRelIndex &ndxRef1, const CRelIndex &ndxRef2)
{
	Q_UNUSED(ndxRef1);			// TODO : Add logic to use ndxRef1/ndxRef2 to insert/remove a single cross-ref if we can
	Q_UNUSED(ndxRef2);
	buildCrossRefsResults();
}

void CVerseListModel::en_changedAllCrossRefs()
{
	buildCrossRefsResults();
}

void CVerseListModel::buildCrossRefsResults()
{
	TVerseListModelCrossRefsResults &zResults = m_crossRefsResults;

	if (m_private.m_nViewMode == VVME_CROSSREFS) {
		emit verseListAboutToChange();
		emit beginResetModel();
	}

	zResults.m_mapVerses.clear();
	zResults.m_lstVerseIndexes.clear();
	zResults.m_mapExtraVerseIndexes.clear();
	zResults.m_mapSizeHints.clear();

	zResults.m_mapCrossRefs.clear();
	zResults.m_mapCrossRefs = m_private.m_pUserNotesDatabase->crossRefsMap().createScopedMap(m_private.m_pBibleDatabase.data());

	zResults.m_lstVerseIndexes.reserve(zResults.m_mapCrossRefs.size());
	for (TCrossReferenceMap::const_iterator itrCrossRef = zResults.m_mapCrossRefs.begin(); itrCrossRef != zResults.m_mapCrossRefs.end(); ++itrCrossRef) {
		CRelIndex ndxCrossRef = (itrCrossRef->first);
		Q_ASSERT(ndxCrossRef.isSet());
		if ((ndxCrossRef.chapter() != 0) && (ndxCrossRef.verse() != 0)) {
			ndxCrossRef.setWord(1);			// Whole verses only
		} else {
			ndxCrossRef.setWord(0);			// We don't allow cross-refs to colophons and superscriptions
		}

		Q_ASSERT(!zResults.m_mapVerses.contains(ndxCrossRef));
		zResults.m_mapVerses.insert(ndxCrossRef, CVerseListItem(zResults.makeVerseIndex(ndxCrossRef, VLMNTE_CROSS_REFERENCE_SOURCE_NODE), m_private.m_pBibleDatabase));
		zResults.m_lstVerseIndexes.append(ndxCrossRef);
	}

	if (m_private.m_nViewMode == VVME_CROSSREFS) {
		emit endResetModel();
		emit verseListChanged();
	}
}

// ----------------------------------------------------------------------------

void CVerseListModel::setDisplayMode(VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	if (m_private.m_nDisplayMode == nDisplayMode) return;

	clearAllSizeHints();
	emit layoutAboutToBeChanged();
	m_private.m_nDisplayMode = nDisplayMode;
	emit layoutChanged();
}

void CVerseListModel::setTreeMode(VERSE_TREE_MODE_ENUM nTreeMode)
{
	if (m_private.m_nTreeMode == nTreeMode) return;

	clearAllSizeHints();
	emit beginResetModel();

	// The following is especially needed for Cross-Ref's which completely revamps the model,
	//		but we'll do it for all in case we change how they are generated:
	clearAllExtraVerseIndexes();

	m_private.m_nTreeMode = nTreeMode;
	emit endResetModel();
}

void CVerseListModel::setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode)
{
	if (m_private.m_nViewMode == nViewMode) return;

	// Don't allow Highlighters, UserNotes, or Cross-Refs if we have no UserNotesDatabase:
	if (((nViewMode == VVME_HIGHLIGHTERS) || (nViewMode == VVME_USERNOTES) || (nViewMode == VVME_CROSSREFS)) &&
		(m_private.m_pUserNotesDatabase.isNull())) {
		Q_ASSERT(false);
		return;
	}

	clearAllSizeHints();
	emit beginResetModel();

	// Need to dump the undefinedResults list so we don't have VerseIndexes of the wrong Results Type.
	//		This should really only affect types having abstract root types, like the highlighters.
	//		Those with just basic Book/Chapter/Verse structure, like Search Results, shouldn't have
	//		any entries in the undefined list:
	m_undefinedResults.m_mapExtraVerseIndexes.clear();

	m_private.m_nViewMode = nViewMode;
	emit endResetModel();
}

void CVerseListModel::setShowMissingLeafs(bool bShowMissing)
{
	if (m_private.m_bShowMissingLeafs == bShowMissing) return;

	// Note: No need to clear sizeHints on this mode change as the size of existing items shouldn't change
	if ((m_private.m_nTreeMode != VTME_LIST) || (m_private.m_nViewMode == VVME_HIGHLIGHTERS)) beginResetModel();
	m_private.m_bShowMissingLeafs = bShowMissing;
	if ((m_private.m_nTreeMode != VTME_LIST) || (m_private.m_nViewMode == VVME_HIGHLIGHTERS)) endResetModel();
}

void CVerseListModel::setShowHighlightersInSearchResults(bool bShowHighlightersInSearchResults)
{
	if (m_private.m_bShowHighlightersInSearchResults == bShowHighlightersInSearchResults) return;

	emit layoutAboutToBeChanged();
	m_private.m_bShowHighlightersInSearchResults = bShowHighlightersInSearchResults;
	emit layoutChanged();
}

void CVerseListModel::setSingleCrossRefSourceIndex(const CRelIndex &ndx)
{
	if (m_private.m_ndxSingleCrossRefSource == ndx) return;

	if (m_private.m_nViewMode == VVME_CROSSREFS) {
		clearAllSizeHints();
		beginResetModel();

		m_crossRefsResults.m_mapExtraVerseIndexes.clear();
	}

	m_private.m_ndxSingleCrossRefSource = ndx;

	if (m_private.m_nViewMode == VVME_CROSSREFS) {
		endResetModel();
	}
}

// ----------------------------------------------------------------------------

QPair<int, int> CVerseListModel::TVerseListModelSearchResults::GetResultsIndexes(CVerseMap::const_iterator itrVerse) const
{
	QPair<int, int> nResultsIndexes;
	nResultsIndexes.first = 0;
	nResultsIndexes.second = 0;

	Q_ASSERT(itrVerse != CVerseMap::const_iterator());

	for (CVerseMap::const_iterator itr = m_mapVerses.constBegin(); ((itr != itrVerse) && (itr != m_mapVerses.constEnd())); ++itr) {
		nResultsIndexes.first += itr->phraseTags().size();
	}
	nResultsIndexes.second = nResultsIndexes.first;
	if (itrVerse != m_mapVerses.constEnd()) {
		nResultsIndexes.first++;
		nResultsIndexes.second += itrVerse->phraseTags().size();
	}

	return nResultsIndexes;		// Result first = first result index, second = last result index for specified row
}

QPair<int, int> CVerseListModel::TVerseListModelSearchResults::GetBookIndexAndCount(CVerseMap::const_iterator itrVerse) const
{
	int ndxBook = 0;		// Index into Books
	int nBooks = 0;			// Results counts in Books
	bool bFlag = false;

	for (CVerseMap::const_iterator itr = m_mapVerses.constBegin(); (itr != m_mapVerses.constEnd()); ++itr) {
		nBooks++;			// Count the book we are on and skip the ones that are on the same book:
		if (!bFlag) ndxBook++;
		if (itr == itrVerse) bFlag = true;
		uint32_t nCurrentBook = itr.key().book();
		for (CVerseMap::const_iterator itr2 = std::next(itr); (itr2 != m_mapVerses.constEnd()); ++itr2) {
			if (itr2.key().book() != nCurrentBook) break;			// Look ahead at next entry and see if it's the same book.  If not, move on to count it...
			++itr;
			if (itr == itrVerse) bFlag = true;
		}
	}

	return QPair<int, int>(ndxBook, nBooks);
}

QPair<int, int> CVerseListModel::TVerseListModelSearchResults::GetChapterIndexAndCount(CVerseMap::const_iterator itrVerse) const
{
	int ndxChapter = 0;		// Index into Chapters
	int nChapters = 0;		// Results counts in Chapters
	bool bFlag = false;

	for (CVerseMap::const_iterator itr = m_mapVerses.constBegin(); (itr != m_mapVerses.constEnd()); ++itr) {
		if (itr.key().chapter() == 0) continue;			// Exclude colophons
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same book/chapter:
		if (!bFlag) ndxChapter++;
		if (itr == itrVerse) bFlag = true;
		uint32_t nCurrentBook = itr.key().book();
		uint32_t nCurrentChapter = itr.key().chapter();
		for (CVerseMap::const_iterator itr2 = std::next(itr); (itr2 != m_mapVerses.constEnd()); ++itr2) {
			if ((itr2.key().book() != nCurrentBook) ||
				(itr2.key().chapter() != nCurrentChapter)) break;		// Look ahead at next entry and see if it's the same book/chapter.  If not, move on to count it...
			++itr;
			if (itr == itrVerse) bFlag = true;
		}
	}

	return QPair<int, int>(ndxChapter, nChapters);
}

QPair<int, int> CVerseListModel::TVerseListModelSearchResults::GetVerseIndexAndCount(CVerseMap::const_iterator itrVerse) const
{
	int nIndex = 0;
	int nCount = 0;
	for (CVerseMap::const_iterator itr = m_mapVerses.constBegin(); itr != m_mapVerses.constEnd(); ++itr) {
		if (itr->getIndex().verse() != 0) {
			++nCount;
			if (itr == itrVerse) nIndex = nCount;
		}
	}
	return QPair<int, int>(nIndex, nCount);
}

// ----------------------------------------------------------------------------

int CVerseListModel::TVerseListModelResults::GetBookCount() const
{
	Q_ASSERT(!m_private->m_pBibleDatabase.isNull());

	if (m_private->m_bShowMissingLeafs) return m_private->m_pBibleDatabase->bibleEntry().m_nNumBk;

	CVerseMap::const_iterator itrVerseMapBookFirst = m_mapVerses.begin();
	CVerseMap::const_iterator itrVerseMapBookLast = m_mapVerses.end();

	int nCount = 0;
	while (itrVerseMapBookFirst != itrVerseMapBookLast) {
		// Find next book (bypassing any chapters/verses in the current book):
		itrVerseMapBookFirst = m_mapVerses.lowerBound(CRelIndex(itrVerseMapBookFirst.key().book() + 1, 0, 0, 0));
		++nCount;
	}
	return nCount;
}

int CVerseListModel::TVerseListModelResults::IndexByBook(unsigned int nBk) const
{
	Q_ASSERT(!m_private->m_pBibleDatabase.isNull());

	if (m_private->m_bShowMissingLeafs) {
		if ((nBk < 1) || (nBk > m_private->m_pBibleDatabase->bibleEntry().m_nNumBk)) return -1;
		return (nBk-1);
	}

	// Find the first entry with the correct Book number:
	CVerseMap::const_iterator itrVerseMapBookFirst = m_mapVerses.begin();
	CVerseMap::const_iterator itrVerseMapBookLast = m_mapVerses.lowerBound(CRelIndex(nBk, 0, 0, 0));

	// If we didn't find the book, return -1 (not found):
	if (itrVerseMapBookLast == m_mapVerses.end()) return -1;
	if (itrVerseMapBookLast.key().book() != nBk) return -1;

	int nIndex = 0;
	while (itrVerseMapBookFirst != itrVerseMapBookLast) {
		// Find next book (bypassing any chapters/verses in the current book):
		itrVerseMapBookFirst = m_mapVerses.lowerBound(CRelIndex(itrVerseMapBookFirst.key().book() + 1, 0, 0, 0));
		Q_ASSERT(itrVerseMapBookFirst != m_mapVerses.end());		// Shouldn't hit the end because we already know the correct book exists
		++nIndex;
	}
	return nIndex;
}

unsigned int CVerseListModel::TVerseListModelResults::BookByIndex(int ndxBook) const
{
	Q_ASSERT(!m_private->m_pBibleDatabase.isNull());

	if (m_private->m_bShowMissingLeafs) {
		if ((ndxBook < 0) || (static_cast<unsigned int>(ndxBook) >= m_private->m_pBibleDatabase->bibleEntry().m_nNumBk)) return 0;
		return (ndxBook+1);
	}

	CVerseMap::const_iterator itrVerseMapBookFirst = m_mapVerses.begin();
	CVerseMap::const_iterator itrVerseMapBookLast = m_mapVerses.end();

	int nIndex = 0;
	while (itrVerseMapBookFirst != itrVerseMapBookLast) {
		if (nIndex == ndxBook) return itrVerseMapBookFirst.key().book();			// If we've found the right index, return the book
		// Find next book (bypassing any chapters/verses in the current book):
		itrVerseMapBookFirst = m_mapVerses.lowerBound(CRelIndex(itrVerseMapBookFirst.key().book() + 1, 0, 0, 0));
		++nIndex;
	}
	Q_ASSERT(false);
	return 0;				// Should have already returned a chapter above, but 0 if we're given an index beyond the list
}

int CVerseListModel::TVerseListModelResults::GetChapterCount(unsigned int nBk) const
{
	Q_ASSERT(!m_private->m_pBibleDatabase.isNull());

	int nFirstChp = 0;
	if ((resultsType() == VLMRTE_USER_NOTES) ||
		(resultsType() == VLMRTE_CROSS_REFS)) {
		nFirstChp = 1;
	}

	if (nBk == 0) return 0;
	if (m_private->m_bShowMissingLeafs) {
		if (nBk > m_private->m_pBibleDatabase->bibleEntry().m_nNumBk) return 0;
		const CBookEntry *pBookEntry = m_private->m_pBibleDatabase->bookEntry(nBk);
		Q_ASSERT(pBookEntry != nullptr);
		return pBookEntry->m_nNumChp + ((pBookEntry->m_bHaveColophon && (nFirstChp==0)) ? 1 : 0);
	}

	// Find the first and last entries with the correct Book number:
	CVerseMap::const_iterator itrVerseMapBookChapterFirst;
	CVerseMap::const_iterator itrVerseMapBookChapterLast;
	itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, nFirstChp, 0, 1));			// This will be the first verse of the first chapter of this book
	itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0));			// This will be the first verse of the next book/chapter

	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return 0;
	if (itrVerseMapBookChapterFirst.key().book() != nBk) return 0;

	int nCount = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		// Find next chapter (bypassing any verses in the current chapter):
		itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, itrVerseMapBookChapterFirst.key().chapter() + 1, 0, 0));
		++nCount;
	}
	return nCount;
}

int CVerseListModel::TVerseListModelResults::IndexByChapter(unsigned int nBk, unsigned int nChp) const
{
	Q_ASSERT(!m_private->m_pBibleDatabase.isNull());

	int nFirstChp = 0;
	if ((resultsType() == VLMRTE_USER_NOTES) ||
		(resultsType() == VLMRTE_CROSS_REFS)) {
		nFirstChp = 1;
	}

	if (nBk == 0) return -1;
	if (m_private->m_bShowMissingLeafs) {
		if (nBk > m_private->m_pBibleDatabase->bibleEntry().m_nNumBk) return -1;
		const CBookEntry *pBookEntry = m_private->m_pBibleDatabase->bookEntry(nBk);
		Q_ASSERT(pBookEntry != nullptr);
		unsigned int nChpCount = pBookEntry->m_nNumChp + ((pBookEntry->m_bHaveColophon && (nFirstChp==0)) ? 1 : 0);
		if ((!pBookEntry->m_bHaveColophon) || (nFirstChp==1)) {
			Q_ASSERT(nChp > 0);
		}
		if (nChp > nChpCount) return -1;
		return ((pBookEntry->m_bHaveColophon && (nFirstChp==0)) ? nChp : (nChp-1));
	}

	// Find the first and last entries with the correct Book number:
	CVerseMap::const_iterator itrVerseMapBookChapterFirst;
	CVerseMap::const_iterator itrVerseMapBookChapterLast;
	itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, nFirstChp, 0, 1));			// This will be the first verse of the first chapter of this book
	itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0));			// This will be the first verse of the next book/chapter

	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return -1;
	if (itrVerseMapBookChapterFirst.key().book() != nBk) return -1;

	int nIndex = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		if (itrVerseMapBookChapterFirst.key().chapter() == nChp) return nIndex;

		// Find next chapter (bypassing any verses in the current chapter):
		itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, itrVerseMapBookChapterFirst.key().chapter() + 1, 0, 0));
		++nIndex;
	}
	return -1;
}

unsigned int CVerseListModel::TVerseListModelResults::ChapterByIndex(int ndxBook, int ndxChapter) const
{
	Q_ASSERT(!m_private->m_pBibleDatabase.isNull());

	int nFirstChp = 0;
	if ((resultsType() == VLMRTE_USER_NOTES) ||
		(resultsType() == VLMRTE_CROSS_REFS)) {
		nFirstChp = 1;
	}

	if ((ndxBook < 0) || (ndxChapter < 0)) return 0;
	if (m_private->m_bShowMissingLeafs) {
		if (static_cast<unsigned int>(ndxBook) >= m_private->m_pBibleDatabase->bibleEntry().m_nNumBk) return 0;
		const CBookEntry *pBookEntry = m_private->m_pBibleDatabase->bookEntry(ndxBook+1);
		Q_ASSERT(pBookEntry != nullptr);
		unsigned int nChpCount = pBookEntry->m_nNumChp + ((pBookEntry->m_bHaveColophon && (nFirstChp==0)) ? 1 : 0);
		if (static_cast<unsigned int>(ndxChapter) >= nChpCount) return 0;
		return ((pBookEntry->m_bHaveColophon && (nFirstChp==0)) ? ndxChapter : (ndxChapter+1));
	}

	unsigned int nBk = BookByIndex(ndxBook);
	if (nBk == 0) return 0;

	// Find the first and last entries with the correct Book number:
	CVerseMap::const_iterator itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, nFirstChp, 0, 1));		// This will be the first verse of the first chapter of this book
	CVerseMap::const_iterator itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0));		// This will be the first verse of the next book/chapter

	// We should have found the book, because of the above BookByIndex() call and nBk check, but safe-guard:
	Q_ASSERT(itrVerseMapBookChapterFirst != m_mapVerses.end());
	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return 0;

	int nIndex = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		if (nIndex == ndxChapter) return itrVerseMapBookChapterFirst.key().chapter();			// If we've found the right index, return the chapter

		// Find next chapter (bypassing any verses in the current chapter):
		itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, itrVerseMapBookChapterFirst.key().chapter() + 1, 0, 0));
		++nIndex;
	}
	return 0;				// Should have already returned a chapter above, but 0 if we're given an index beyond the list
}

CVerseMap::const_iterator CVerseListModel::TVerseListModelResults::FindVerseIndex(const CRelIndex &ndxRel) const
{
	if (!ndxRel.isSet()) return m_mapVerses.constEnd();

	CRelIndex ndxSearch(ndxRel);

	if ((m_private->m_nViewMode == VVME_CROSSREFS) ||
		(m_private->m_nViewMode == VVME_USERNOTES)) {
		if ((ndxSearch.chapter() != 0) && (ndxSearch.verse() != 0)) {
			ndxSearch.setWord(1);			// Whole verses only
		} else {
			ndxSearch.setWord(0);			// We don't allow cross-refs and notes on colophons and superscriptions
		}
	} else {
		ndxSearch.setWord(1);
	}

	return m_mapVerses.find(ndxSearch);
}

int CVerseListModel::TVerseListModelResults::IndexByVerse(const CRelIndex &ndxRel) const
{
	CVerseMap::const_iterator itrVerseMap = FindVerseIndex(ndxRel);
	Q_ASSERT(itrVerseMap != m_mapVerses.constEnd());

	switch (m_private->m_nTreeMode) {
		case VTME_LIST:
			return m_lstVerseIndexes.indexOf(ndxRel);
		case VTME_TREE_BOOKS:
		{
			CVerseMap::const_iterator itrFirstVerse = GetVerse(0, ndxRel.book(), -1);
			Q_ASSERT(itrFirstVerse != m_mapVerses.end());
			if (itrFirstVerse == m_mapVerses.end()) return -1;
			return std::distance(itrFirstVerse, itrVerseMap);
		}
		case VTME_TREE_CHAPTERS:
		{
			CVerseMap::const_iterator itrFirstVerse = GetVerse(0, ndxRel.book(),  ndxRel.chapter());
			Q_ASSERT(itrFirstVerse != m_mapVerses.end());
			if (itrFirstVerse == m_mapVerses.end()) return -1;
			return std::distance(itrFirstVerse, itrVerseMap);
		}
		default:
			Q_ASSERT(false);
			break;
	}

	return -1;
}

CVerseMap::const_iterator CVerseListModel::TVerseListModelResults::GetVerse(int ndxVerse, int nBk, int nChp) const
{
	// Note: This function has a special case for nBk == -1 and nChp == -1 (unlike the other index functions)

	int nFirstChp = 0;
	if ((resultsType() == VLMRTE_USER_NOTES) ||
		(resultsType() == VLMRTE_CROSS_REFS)) {
		nFirstChp = 1;
	}

	if (ndxVerse < 0) return m_mapVerses.constEnd();

	if ((nBk == -1) && (nChp == -1)) {
		if (ndxVerse >= m_lstVerseIndexes.size()) return m_mapVerses.constEnd();	// Note: (ndxVerse < 0) is handled above for both Map/List methods
		return m_mapVerses.find(m_lstVerseIndexes.at(ndxVerse));
	}
	Q_ASSERT(nBk != -1);

	// Find the first and last entries with the correct Book/Chapter number:
	CVerseMap::const_iterator itrVerseMapBookChapterFirst;
	CVerseMap::const_iterator itrVerseMapBookChapterLast;
	itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, ((nChp > 0) ? nChp : nFirstChp), 0, 0));			// This will be the first verse of this chapter of this book
	if (nChp >= nFirstChp) {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk, nChp+1, 0, 0));		// This will be the first verse of the next book/chapter
	} else {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0));
	}

	// If we didn't find the book and/or book/chapter, return (not found):
	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return m_mapVerses.constEnd();
	if ((itrVerseMapBookChapterFirst.key().book() != static_cast<unsigned int>(nBk)) ||
		((nChp >= nFirstChp) && (itrVerseMapBookChapterFirst.key().chapter() != static_cast<unsigned int>(nChp)))) return m_mapVerses.constEnd();

	int nVerses = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		// Don't count non-verse entries for Tree-by-Chapters mode since we will roll them
		//		into the Chapter entry:
		if ((m_private->m_nTreeMode != VTME_TREE_CHAPTERS) ||
			((m_private->m_nTreeMode == VTME_TREE_CHAPTERS) &&
			 ((itrVerseMapBookChapterFirst->verseIndex()->nodeType() == VLMNTE_CROSS_REFERENCE_SOURCE_NODE) ||
			  (itrVerseMapBookChapterFirst->verseIndex()->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE) ||
			  (itrVerseMapBookChapterFirst.key().verse() != 0) ||
			  ((itrVerseMapBookChapterFirst.key().verse() == 0) && (itrVerseMapBookChapterFirst.key().word() != 0))))) {
			if (nVerses == ndxVerse) return itrVerseMapBookChapterFirst;
			++nVerses;
		}

		++itrVerseMapBookChapterFirst;
	}
	return m_mapVerses.constEnd();			// Should have already returned a verse above, but end() if we're given an index beyond the list
}

// ----------------------------------------------------------------------------

int CVerseListModel::TVerseListModelResults::GetVerseCount(int nBk, int nChp) const
{
	// Note: This function has special cases for nBk == -1 and nChp == -1 (unlike the other count functions)

	int nFirstChp = 0;
	if ((resultsType() == VLMRTE_USER_NOTES) ||
		(resultsType() == VLMRTE_CROSS_REFS)) {
		nFirstChp = 1;
	}

	if (nBk == -1) return m_mapVerses.size();		// Quick special-case

	// Find the first and last entries with the correct Book/Chapter number:
	CVerseMap::const_iterator itrVerseMapBookChapterFirst;
	CVerseMap::const_iterator itrVerseMapBookChapterLast;
	itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, ((nChp > 0) ? nChp : nFirstChp), 0, 0));			// This will be the first verse of this chapter of this book
	if (nChp >= nFirstChp) {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk, nChp+1, 0, 0));		// This will be the first verse of the next book/chapter
	} else {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0));			// This will be the first verse of the next book
	}

	// If we didn't find the book and/or book/chapter, return none found:
	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return 0;
	if ((itrVerseMapBookChapterFirst.key().book() != static_cast<unsigned int>(nBk)) ||
		((nChp >= nFirstChp) && (itrVerseMapBookChapterFirst.key().chapter() != static_cast<unsigned int>(nChp)))) return 0;

	int nVerses = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		// Don't count non-verse entries for Tree-by-Chapters mode since we will roll them
		//		into the Chapter entry:
		if ((m_private->m_nTreeMode != VTME_TREE_CHAPTERS) ||
			((m_private->m_nTreeMode == VTME_TREE_CHAPTERS) &&
			 ((itrVerseMapBookChapterFirst->verseIndex()->nodeType() == VLMNTE_CROSS_REFERENCE_SOURCE_NODE) ||
			  (itrVerseMapBookChapterFirst->verseIndex()->nodeType() == VLMNTE_CROSS_REFERENCE_TARGET_NODE) ||
			  (itrVerseMapBookChapterFirst.key().verse() != 0) ||
			  ((itrVerseMapBookChapterFirst.key().verse() == 0) && (itrVerseMapBookChapterFirst.key().word() != 0))))) {
			++nVerses;
		}
		++itrVerseMapBookChapterFirst;
	}
	return nVerses;
}

int CVerseListModel::TVerseListModelResults::GetResultsCount(int nBk, int nChp) const
{
	int nResults = 0;

	for (CVerseMap::const_iterator itrVerse = m_mapVerses.constBegin(); itrVerse != m_mapVerses.constEnd(); ++itrVerse) {
		if ((nBk != -1) && (itrVerse.key().book() != static_cast<unsigned int>(nBk))) continue;
		if ((nBk != -1) && (nChp != -1) && (itrVerse.key().chapter() != static_cast<unsigned int>(nChp))) continue;
		if (itrVerse->phraseTags().size()) {
			nResults += itrVerse->phraseTags().size();
		} else {
			// User Notes have results but no tags:
			++nResults;
		}
	}

	return nResults;
}

// ----------------------------------------------------------------------------

int CVerseListModel::GetVerseCount(int nBk, int nChp) const
{
	if (m_private.m_nViewMode == VVME_SEARCH_RESULTS) return m_searchResults.GetVerseCount(nBk, nChp);
	if (m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED) return m_searchResultsExcluded.GetVerseCount(nBk, nChp);
	if (m_private.m_nViewMode == VVME_USERNOTES) return m_userNotesResults.GetVerseCount(nBk, nChp);

	int nCount = 0;
	for (THighlighterVLMRList::const_iterator itrHighlighter = m_vlmrListHighlighters.constBegin(); itrHighlighter != m_vlmrListHighlighters.constEnd(); ++itrHighlighter) {
		nCount += itrHighlighter->GetVerseCount(nBk, nChp);
	}

	return nCount;
}

int CVerseListModel::GetResultsCount(int nBk, int nChp) const
{
	if (m_private.m_nViewMode == VVME_SEARCH_RESULTS) return m_searchResults.GetResultsCount(nBk, nChp);
	if (m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED) return m_searchResultsExcluded.GetResultsCount(nBk, nChp);
	if (m_private.m_nViewMode == VVME_USERNOTES) return m_userNotesResults.GetResultsCount(nBk, nChp);

	int nCount = 0;
	for (THighlighterVLMRList::const_iterator itrHighlighter = m_vlmrListHighlighters.constBegin(); itrHighlighter != m_vlmrListHighlighters.constEnd(); ++itrHighlighter) {
		nCount += itrHighlighter->GetResultsCount(nBk, nChp);
	}

	return nCount;
}

// ----------------------------------------------------------------------------

void CVerseListModel::clearAllSizeHints()
{
	m_undefinedResults.m_mapSizeHints.clear();
	m_searchResults.m_mapSizeHints.clear();
	m_userNotesResults.m_mapSizeHints.clear();
	for (THighlighterVLMRList::iterator itrHighlighter = m_vlmrListHighlighters.begin(); itrHighlighter != m_vlmrListHighlighters.end(); ++itrHighlighter) {
		itrHighlighter->m_mapSizeHints.clear();
	}
	m_crossRefsResults.m_mapSizeHints.clear();
}

void CVerseListModel::clearAllExtraVerseIndexes()
{
	m_undefinedResults.m_mapExtraVerseIndexes.clear();
	m_searchResults.m_mapExtraVerseIndexes.clear();
	m_userNotesResults.m_mapExtraVerseIndexes.clear();
	for (THighlighterVLMRList::iterator itrHighlighter = m_vlmrListHighlighters.begin(); itrHighlighter != m_vlmrListHighlighters.end(); ++itrHighlighter) {
		itrHighlighter->m_mapExtraVerseIndexes.clear();
	}
	m_crossRefsResults.m_mapExtraVerseIndexes.clear();
}

// ----------------------------------------------------------------------------

void CVerseListModel::setFont(const QFont& aFont)
{
	m_private.m_font = aFont;
	emit layoutAboutToBeChanged();
	setData(QModelIndex(), QSize(), Qt::SizeHintRole);			// Invalidate all sizeHints on fontChange
	emit layoutChanged();
}

void CVerseListModel::en_WordsOfJesusColorChanged(const QColor &color)
{
	m_private.m_richifierTagsDisplay.setWordsOfJesusTagsByColor(color);
	m_private.m_richifierTagsCopying.setWordsOfJesusTagsByColor(color);
}

void CVerseListModel::en_SearchResultsColorChanged(const QColor &color)
{
	m_private.m_richifierTagsDisplay.setSearchResultsTagsByColor(color);
}

void CVerseListModel::en_changedShowPilcrowMarkers(bool bShowPilcrowMarkers)
{
	m_private.m_richifierTagsDisplay.setShowPilcrowMarkers(bShowPilcrowMarkers);
}

void CVerseListModel::en_changedCopyOptions()
{
	m_private.m_richifierTagsCopying.setFromPersistentSettings(*CPersistentSettings::instance(), true);
}

// ----------------------------------------------------------------------------


void CVerseListModel::buildScopedResultsFromParsedPhrases(const CSearchResultsData &searchResultsData)
{
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	if (!g_pMyApplication->isSingleThreadedSearchResults()) {
		if (!m_pSearchResultsThreadCtrl.isNull()) {
			m_pSearchResultsThreadCtrl->disconnect(this);		// Deactivate future notifications from current worker thread
		}

		m_pSearchResultsThreadCtrl = new CThreadedSearchResultCtrl(m_private.m_pBibleDatabase, searchResultsData);
		connect(m_pSearchResultsThreadCtrl.data(), SIGNAL(resultsReady(const CThreadedSearchResultCtrl *)), this, SLOT(en_searchResultsReady(const CThreadedSearchResultCtrl *)));
		m_pSearchResultsThreadCtrl->startWorking();
	} else {
#endif

		if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
			(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {
			emit verseListAboutToChange();
			emit beginResetModel();
		}

		CSearchResultsProcess srp(m_private.m_pBibleDatabase, searchResultsData, &m_searchResults.m_mapVerses, &m_searchResultsExcluded.m_mapVerses);
		srp.buildScopedResultsFromParsedPhrases();
		srp.copyBackInclusionData(m_searchResults.m_searchResultsData, m_searchResults.m_mapVerses, m_searchResults.m_lstVerseIndexes);
		srp.copyBackExclusionData(m_searchResultsExcluded.m_searchResultsData, m_searchResultsExcluded.m_mapVerses, m_searchResultsExcluded.m_lstVerseIndexes);

		m_searchResults.m_mapExtraVerseIndexes.clear();
		m_searchResults.m_mapSizeHints.clear();

		m_searchResultsExcluded.m_mapExtraVerseIndexes.clear();
		m_searchResultsExcluded.m_mapSizeHints.clear();

		if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
			(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {
			emit endResetModel();
			emit verseListChanged();
		}

		emit searchResultsReady();

#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	}
#endif

}

#ifdef USE_MULTITHREADED_SEARCH_RESULTS

void CVerseListModel::en_searchResultsReady(const CThreadedSearchResultCtrl *theThreadedSearchResult)
{
	// If a signal from another (now stale) thread helper sneaks through, ignore it:
	if (theThreadedSearchResult != m_pSearchResultsThreadCtrl.data()) return;

	// If the phrases changed out from under us, we can't copy back.  We'll eventually
	//		get another change notification to handle it:
	if (!theThreadedSearchResult->searchResultsProcess()->canCopyBack()) {
		m_pSearchResultsThreadCtrl = nullptr;			// Object has its own deleteLater()
		return;
	}

	if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
		(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {
		emit verseListAboutToChange();
		emit beginResetModel();
	}

	theThreadedSearchResult->searchResultsProcess()->copyBackInclusionData(m_searchResults.m_searchResultsData, m_searchResults.m_mapVerses, m_searchResults.m_lstVerseIndexes);
	theThreadedSearchResult->searchResultsProcess()->copyBackExclusionData(m_searchResultsExcluded.m_searchResultsData, m_searchResultsExcluded.m_mapVerses, m_searchResultsExcluded.m_lstVerseIndexes);

	m_searchResults.m_mapExtraVerseIndexes.clear();
	m_searchResults.m_mapSizeHints.clear();

	m_searchResultsExcluded.m_mapExtraVerseIndexes.clear();
	m_searchResultsExcluded.m_mapSizeHints.clear();

	if ((m_private.m_nViewMode == VVME_SEARCH_RESULTS) ||
		(m_private.m_nViewMode == VVME_SEARCH_RESULTS_EXCLUDED)) {
		emit endResetModel();
		emit verseListChanged();
	}

	m_pSearchResultsThreadCtrl = nullptr;		// Object has its own deleteLater()

	emit searchResultsReady();
}

#endif

// ============================================================================

#ifdef USE_MULTITHREADED_SEARCH_RESULTS

CSearchResultsProcess::CSearchResultsProcess(CBibleDatabasePtr pBibleDatabase, const CSearchResultsData &searchResultsData)
	:	CSearchResultsData(searchResultsData),
		m_pBibleDatabase(pBibleDatabase)
{
	Q_ASSERT(!pBibleDatabase.isNull());
	m_pMapVersesIncl = &m_mapVersesInclLocal;
	m_pMapVersesExcl = &m_mapVersesExclLocal;

	commonConstruct();
}

#endif

CSearchResultsProcess::CSearchResultsProcess(CBibleDatabasePtr pBibleDatabase, const CSearchResultsData &searchResultsData, CVerseMap *pMapVersesIncl, CVerseMap *pMapVersesExcl)
	:	CSearchResultsData(searchResultsData),
		m_pBibleDatabase(pBibleDatabase),
		m_pMapVersesIncl(pMapVersesIncl),
		m_pMapVersesExcl(pMapVersesExcl)
{
	Q_ASSERT(!pBibleDatabase.isNull());
	Q_ASSERT(pMapVersesIncl != nullptr);
	Q_ASSERT(pMapVersesExcl != nullptr);

	commonConstruct();
}

void CSearchResultsProcess::commonConstruct()
{
	for (int ndx=0; ndx<m_lstParsedPhrases.size(); ++ndx) {
		Q_ASSERT(!m_lstParsedPhrases.at(ndx).isNull());
		m_lstParsedPhrases.at(ndx)->setHasChanged(false);
		if (!m_lstParsedPhrases.at(ndx)->isExcluded()) {
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
			if (!g_pMyApplication->isSingleThreadedSearchResults()) {
				m_lstCopyParsedPhrasesIncl.append(QSharedPointer<CParsedPhrase>(new CParsedPhrase(*m_lstParsedPhrases.at(ndx))));
				m_lstParsedPhrasesIncl.append(m_lstCopyParsedPhrasesIncl.last().data());
			} else {
				m_lstParsedPhrasesIncl.append(m_lstParsedPhrases.at(ndx));
			}
#else
			m_lstParsedPhrasesIncl.append(m_lstParsedPhrases.at(ndx));
#endif
		} else {
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
			if (!g_pMyApplication->isSingleThreadedSearchResults()) {
				m_lstCopyParsedPhrasesExcl.append(QSharedPointer<CParsedPhrase>(new CParsedPhrase(*m_lstParsedPhrases.at(ndx))));
				m_lstParsedPhrasesExcl.append(m_lstCopyParsedPhrasesExcl.last().data());
			} else {
				m_lstParsedPhrasesExcl.append(m_lstParsedPhrases.at(ndx));
			}
#else
			m_lstParsedPhrasesExcl.append(m_lstParsedPhrases.at(ndx));
#endif
		}
	}
}

bool CSearchResultsProcess::canCopyBack() const
{
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	if (!g_pMyApplication->isSingleThreadedSearchResults()) {
		int ndxIncl = 0;
		int ndxExcl = 0;
		if (m_lstParsedPhrases.size() != (m_lstCopyParsedPhrasesIncl.size() + m_lstCopyParsedPhrasesExcl.size())) return false;

		// Check for phrases that have changed:
		for (int ndx=0; ndx<m_lstParsedPhrases.size(); ++ndx) {
			if ((m_lstParsedPhrases.at(ndx).isNull()) ||
				(m_lstParsedPhrases.at(ndx)->hasChanged())) return false;
			if (!m_lstParsedPhrases.at(ndx)->isExcluded()) {
				if (ndxIncl >= m_lstCopyParsedPhrasesIncl.size()) return false;
				if ((*m_lstParsedPhrases.at(ndx)) != (*m_lstCopyParsedPhrasesIncl.at(ndxIncl).data())) return false;
				++ndxIncl;
			} else {
				if (ndxExcl >= m_lstCopyParsedPhrasesExcl.size()) return false;
				if ((*m_lstParsedPhrases.at(ndx)) != (*m_lstCopyParsedPhrasesExcl.at(ndxExcl).data())) return false;
				++ndxExcl;
			}
		}
	}
#endif

	return true;
}

void CSearchResultsProcess::copyBackInclusionData(CSearchResultsData &searchResultsData, CVerseMap &mapVerseData, QList<CRelIndex> &lstVerseIndexes) const
{
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	int ndxIncl = 0;
	if (!g_pMyApplication->isSingleThreadedSearchResults()) {
		Q_ASSERT(m_lstParsedPhrases.size() == (m_lstCopyParsedPhrasesIncl.size() + m_lstCopyParsedPhrasesExcl.size()));
	}
#endif
	searchResultsData.m_lstParsedPhrases.clear();
	for (int ndx=0; ndx<m_lstParsedPhrases.size(); ++ndx) {
		Q_ASSERT(!m_lstParsedPhrases.at(ndx).isNull());
		if (!m_lstParsedPhrases.at(ndx)->isExcluded()) {
			searchResultsData.m_lstParsedPhrases.append(m_lstParsedPhrases.at(ndx));
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
			if (!g_pMyApplication->isSingleThreadedSearchResults()) {
				//	Assert if phrase changed because caller should have called canCopyBack():
				Q_ASSERT((*m_lstParsedPhrases.at(ndx)) == (*m_lstCopyParsedPhrasesIncl.at(ndxIncl).data()));
				m_lstParsedPhrases.at(ndx)->GetScopedPhraseTagSearchResultsNonConst() = m_lstCopyParsedPhrasesIncl.at(ndxIncl)->GetScopedPhraseTagSearchResults();
				m_lstParsedPhrases.at(ndx)->GetWithinPhraseTagSearchResultsNonConst() = m_lstCopyParsedPhrasesIncl.at(ndxIncl)->GetWithinPhraseTagSearchResults();
			}
			++ndxIncl;
#endif
		}
	}
	searchResultsData.m_SearchCriteria = m_SearchCriteria;
	lstVerseIndexes.clear();
	lstVerseIndexes.reserve(m_pMapVersesIncl->size());
	for (CVerseMap::const_iterator itr = m_pMapVersesIncl->begin(); (itr != m_pMapVersesIncl->end()); ++itr) {
		lstVerseIndexes.append(itr.key());
	}
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	if (!g_pMyApplication->isSingleThreadedSearchResults()) {
		Q_ASSERT(m_pMapVersesIncl != nullptr);
		mapVerseData = *m_pMapVersesIncl;
	}
#else
	Q_UNUSED(mapVerseData);
#endif
}

void CSearchResultsProcess::copyBackExclusionData(CSearchResultsData &searchResultsData, CVerseMap &mapVerseData, QList<CRelIndex> &lstVerseIndexes) const
{
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	int ndxExcl = 0;
	if (!g_pMyApplication->isSingleThreadedSearchResults()) {
		Q_ASSERT(m_lstParsedPhrases.size() == (m_lstCopyParsedPhrasesIncl.size() + m_lstCopyParsedPhrasesExcl.size()));
	}
#endif
	searchResultsData.m_lstParsedPhrases.clear();
	for (int ndx=0; ndx<m_lstParsedPhrases.size(); ++ndx) {
		Q_ASSERT(!m_lstParsedPhrases.at(ndx).isNull());
		if (m_lstParsedPhrases.at(ndx)->isExcluded()) {
			searchResultsData.m_lstParsedPhrases.append(m_lstParsedPhrases.at(ndx));
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
			if (!g_pMyApplication->isSingleThreadedSearchResults()) {
				//	Assert if phrase changed because caller should have called canCopyBack():
				Q_ASSERT((*m_lstParsedPhrases.at(ndx)) == (*m_lstCopyParsedPhrasesExcl.at(ndxExcl).data()));
				m_lstParsedPhrases.at(ndx)->GetScopedPhraseTagSearchResultsNonConst() = m_lstCopyParsedPhrasesExcl.at(ndxExcl)->GetScopedPhraseTagSearchResults();
				m_lstParsedPhrases.at(ndx)->GetWithinPhraseTagSearchResultsNonConst() = m_lstCopyParsedPhrasesExcl.at(ndxExcl)->GetWithinPhraseTagSearchResults();
			}
			++ndxExcl;
#endif
		}
	}
	searchResultsData.m_SearchCriteria = m_SearchCriteria;
	lstVerseIndexes.clear();
	lstVerseIndexes.reserve(m_pMapVersesExcl->size());
	for (CVerseMap::const_iterator itr = m_pMapVersesExcl->begin(); (itr != m_pMapVersesExcl->end()); ++itr) {
		lstVerseIndexes.append(itr.key());
	}
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
	if (!g_pMyApplication->isSingleThreadedSearchResults()) {
		Q_ASSERT(m_pMapVersesExcl != nullptr);
		mapVerseData = *m_pMapVersesExcl;
	}
#else
	Q_UNUSED(mapVerseData);
#endif
}

void CSearchResultsProcess::buildScopedResultsFromParsedPhrases()
{
	m_pMapVersesIncl->clear();
	m_pMapVersesExcl->clear();

	QList<TPhraseTagList::const_iterator> lstItrStart;
	QList<TPhraseTagList::const_iterator> lstItrEnd;
	QList<CRelIndex> lstScopedRefs;
	QList<bool> lstNeedScope;
	int nNumPhrases = m_lstParsedPhrasesIncl.size();
	QList< QList<TPhraseTagList::const_iterator> > lstlstItrExclNext;		// List of List Exclusion Next Iterators so we can check for intersection without comparing the whole list (inner list is exclusions, outer list is inclusions)
	QList<bool> lstHitExclusion;
	int nNumExcludedPhrases = m_lstParsedPhrasesExcl.size();

	// Note: We'll still build the "within" results even for excluded phrases.
	//		We have to do this first so we can populate the list of iterators per inclusion phrase:
	for (int ndx=0; ndx<nNumExcludedPhrases; ++ndx) {
		buildWithinResultsInParsedPhrase(m_lstParsedPhrasesExcl.at(ndx));
	}

	lstItrStart.reserve(nNumPhrases);
	lstItrEnd.reserve(nNumPhrases);
	lstScopedRefs.reserve(nNumPhrases);
	lstNeedScope.reserve(nNumPhrases);
	lstlstItrExclNext.reserve(nNumPhrases);
	lstHitExclusion.reserve(nNumPhrases);

	// Fetch results from all phrases and build a list of lists, denormalizing entries, and
	//		setting the phrase size details:
	for (int ndx=0; ndx<nNumPhrases; ++ndx) {
		buildWithinResultsInParsedPhrase(m_lstParsedPhrasesIncl.at(ndx));
		const TPhraseTagList &lstSearchResultsPhraseTags = m_lstParsedPhrasesIncl.at(ndx)->GetWithinPhraseTagSearchResults();
		lstItrStart.append(lstSearchResultsPhraseTags.constBegin());
		lstItrEnd.append(lstSearchResultsPhraseTags.constBegin());
		lstScopedRefs.append(CRelIndex());
		lstNeedScope.append(true);
		QList<TPhraseTagList::const_iterator> lstItrExclNext;
		lstItrExclNext.reserve(nNumExcludedPhrases);
		for (int ndxExclusion=0; ndxExclusion<nNumExcludedPhrases; ++ndxExclusion) {
			const TPhraseTagList &lstExcludedSearchResultsPhraseTags = m_lstParsedPhrasesExcl.at(ndxExclusion)->GetWithinPhraseTagSearchResults();
			lstItrExclNext.append(lstExcludedSearchResultsPhraseTags.constBegin());
		}
		lstlstItrExclNext.append(lstItrExclNext);
		lstHitExclusion.append(false);
	}

	// Now, we'll go through our lists and compress the results to the scope specified
	//		for each phrase.  We'll then find the lowest valued one and see if the others
	//		match.  If they do, we'll push all of those results onto the output.  If not,
	//		we'll toss results for the lowest until we get a match.  When any list hits
	//		its end, we're done and can break out since we have no more matches

	bool bDone = (nNumPhrases == 0);		// We're done if we have no phrases (or phrases with results)
	while (!bDone) {
		uint32_t nMaxScope = 0;
		bool bDoneAll = true;
		for (int ndx=0; ndx<nNumPhrases; ++ndx) {
			const CParsedPhrase *phrase = m_lstParsedPhrasesIncl.at(ndx);
			const TPhraseTagList &lstSearchResultsPhraseTags = phrase->GetWithinPhraseTagSearchResults();
			QList<TPhraseTagList::const_iterator> &lstItrExclNext = lstlstItrExclNext[ndx];
			if (!lstNeedScope[ndx]) {
				nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
				continue;		// Only find next scope for a phrase if we need it
			}
			lstItrStart[ndx] = lstItrEnd[ndx];		// Begin at the last ending position

			// Go until we find one that isn't excluded:
			while ((lstItrStart[ndx] != lstSearchResultsPhraseTags.constEnd()) &&
				   (checkExclusion(lstItrExclNext, *lstItrStart[ndx]))) {
				++lstItrStart[ndx];
			}

			if (lstItrStart[ndx] == lstSearchResultsPhraseTags.constEnd()) {
				if (m_SearchCriteria.searchScopeMode() != CSearchCriteria::SSME_UNSCOPED) {
					bDone = true;
					break;
				}
				// For unscoped, keep going and process other phrases when this one is done (no short-circuit like scoped)
			} else {
				bDoneAll = false;
			}

			lstHitExclusion[ndx] = false;
			if (lstItrStart[ndx] != lstSearchResultsPhraseTags.constEnd()) {
				lstScopedRefs[ndx] = ScopeIndex(lstItrStart[ndx]->relIndex());
				for (lstItrEnd[ndx] = lstItrStart[ndx]+1; lstItrEnd[ndx] != lstSearchResultsPhraseTags.constEnd(); ++lstItrEnd[ndx]) {
					CRelIndex ndxScopedTemp = ScopeIndex(lstItrEnd[ndx]->relIndex());
					if (lstScopedRefs[ndx].index() != ndxScopedTemp.index()) break;

					// Check for exclusion, but don't advance and count exclusions as we'll need to do
					//		that when we come back through the loop via our start index:
					if (checkExclusion(lstItrExclNext, *lstItrEnd[ndx], true)) {
						lstHitExclusion[ndx] = true;
						break;
					}
				}
			} else {
				lstScopedRefs[ndx] = ScopeIndex(CRelIndex(1, 0, 0, 0));
				lstItrEnd[ndx] = lstSearchResultsPhraseTags.constEnd();
			}
			// Here lstItrEnd will be one more than the number of matching, either the next index
			//		off the end of the array, an exclusion, or the first non-matching entry.  So the scoped
			//		area is from lstItrStart to lstItrEnd-1.
			nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
			lstNeedScope[ndx] = false;
		}
		if (m_SearchCriteria.searchScopeMode() != CSearchCriteria::SSME_UNSCOPED) {
			if (bDone) continue;		// For scoped, if we run out of phrase matches on any phrase, we're done
		} else {
			// For unscoped, if we run out of matches on all phrases, we're done
			if (bDoneAll) {
				bDone = true;
				continue;
			}
		}
		// Now, check the scoped references.  If they match for all indexes, we'll push the
		//	results to our output and set flags to get all new scopes.  Otherwise, compare them
		//	all against our maximum scope value and tag any that's less than that as needing a
		//	new scope (they weren't matches).  Then loop back until we've either pushed all
		//	results or run out of matches.
		bool bMatch = true;
		bool bHitExclusion = false;
		for (int ndx=0; ndx<nNumPhrases; ++ndx) {
			if (lstScopedRefs[ndx].index() != nMaxScope) {
				lstNeedScope[ndx] = true;
				bMatch = false;
			}
			if (lstHitExclusion[ndx]) bHitExclusion = true;
		}
		if (bMatch) {
			// We got a match, so push results to output and flag for new scopes:
			for (int ndx=0; ndx<nNumPhrases; ++ndx) {
				TPhraseTagList &lstScopedPhraseTags = m_lstParsedPhrasesIncl.at(ndx)->GetScopedPhraseTagSearchResultsNonConst();
				lstScopedPhraseTags.reserve(lstScopedPhraseTags.size() + std::distance(lstItrStart[ndx], lstItrEnd[ndx]));
				for (TPhraseTagList::const_iterator itr = lstItrStart[ndx]; itr != lstItrEnd[ndx]; ++itr) {
					lstScopedPhraseTags.append(*itr);
					CRelIndex ndxNextRelative = itr->relIndex();
					ndxNextRelative.setWord(1);
					if (m_pMapVersesIncl->contains(ndxNextRelative)) {
						(*m_pMapVersesIncl)[ndxNextRelative].addPhraseTag(*itr);
					} else {
						m_pMapVersesIncl->insert(ndxNextRelative, CVerseListItem(TVerseIndex(ndxNextRelative, VLMRTE_SEARCH_RESULTS), m_pBibleDatabase, *itr));
					}
				}
				if (!bHitExclusion) {
					lstNeedScope[ndx] = true;
				} else {
					// If we hit an exclusion on one list, the others don't need to move to next
					//		scope, only the one we hit the exclusion on:
					if (!lstHitExclusion[ndx]) {
						lstNeedScope[ndx] = false;
						lstItrStart[ndx] = lstItrEnd[ndx];		// But we've already consumed the search results (don't keep them or we'll get an extra copy)
					} else {
						lstNeedScope[ndx] = true;
					}
				}
			}
		}
	}

	for (CVerseMap::iterator itr = m_pMapVersesIncl->begin(); (itr != m_pMapVersesIncl->end()); ++itr) {
		itr->sortPhraseTags();
	}
	for (CVerseMap::iterator itr = m_pMapVersesExcl->begin(); (itr != m_pMapVersesExcl->end()); ++itr) {
		itr->sortPhraseTags();
	}
}

bool CSearchResultsProcess::checkExclusion(QList<TPhraseTagList::const_iterator> &lstItrExclNext, const TPhraseTag &tag, bool bPreserveLastItr)
{
	bool bExclude = false;

	TTagBoundsPair tbpTag(tag, m_pBibleDatabase.data());

	for (int ndx=0; ndx<lstItrExclNext.size(); ++ndx) {
		// If inclusion tag is less than exclusion target, it can't possibly be completely contained in exclusion:
		while ((lstItrExclNext.at(ndx) != m_lstParsedPhrasesExcl.at(ndx)->GetWithinPhraseTagSearchResults().constEnd()) &&
			   (tbpTag.lo() >= m_pBibleDatabase->NormalizeIndex(lstItrExclNext.at(ndx)->relIndex()))) {
			if (lstItrExclNext.at(ndx)->bounds(m_pBibleDatabase.data()).completelyContains(tbpTag)) {
				bExclude = true;
				if (!bPreserveLastItr) {
					m_lstParsedPhrasesExcl.at(ndx)->GetScopedPhraseTagSearchResultsNonConst().append(tag);
				} else {
					break;		// Don't increment last index when we're preserving so we can use it again in Start logic
				}

			}
			++lstItrExclNext[ndx];
		}
	}

	if (bExclude && !bPreserveLastItr) {
		CRelIndex ndxNextRelative = tag.relIndex();
		ndxNextRelative.setWord(1);
		// Note: The tag has already been added to the ScopedPhraseTags above in the
		//			check for intersection.
		if (m_pMapVersesExcl->contains(ndxNextRelative)) {
			(*m_pMapVersesExcl)[ndxNextRelative].addPhraseTag(tag);
		} else {
			m_pMapVersesExcl->insert(ndxNextRelative, CVerseListItem(TVerseIndex(ndxNextRelative, VLMRTE_SEARCH_RESULTS_EXCLUDED), m_pBibleDatabase, tag));
		}
	}

	return bExclude;
}

void CSearchResultsProcess::buildWithinResultsInParsedPhrase(const CParsedPhrase *pParsedPhrase) const
{
	const TPhraseTagList &lstPhraseTags = pParsedPhrase->GetPhraseTagSearchResults();
	TPhraseTagList &lstWithinPhraseTags = pParsedPhrase->GetWithinPhraseTagSearchResultsNonConst();

	if (m_SearchCriteria.withinIsEntireBible(m_pBibleDatabase)) {
		lstWithinPhraseTags = lstPhraseTags;
		return;
	}

	lstWithinPhraseTags.reserve(lstPhraseTags.size());
	for (TPhraseTagList::const_iterator itrTags = lstPhraseTags.constBegin(); itrTags != lstPhraseTags.constEnd(); ++itrTags) {
		if (m_SearchCriteria.indexIsWithin(itrTags->relIndex())) lstWithinPhraseTags.append(*itrTags);
	}
}

CRelIndex CSearchResultsProcess::ScopeIndex(const CRelIndex &index)
{
	CRelIndex indexScoped;

	switch (m_SearchCriteria.searchScopeMode()) {
		case (CSearchCriteria::SSME_UNSCOPED):
		case (CSearchCriteria::SSME_WHOLE_BIBLE):
			// For Whole Bible and Unscoped, we'll set the Book to 1 so that anything in the Bible matches:
			if (index.isSet()) indexScoped = CRelIndex(1, 0, 0, 0);
			break;
		case (CSearchCriteria::SSME_TESTAMENT):
			// For Testament, set the Book to the 1st Book of the corresponding Testament:
			if (index.book()) {
				if (index.book() <= m_pBibleDatabase->bibleEntry().m_nNumBk) {
					const CBookEntry &book = *m_pBibleDatabase->bookEntry(index.book());
					unsigned int nTestament = book.m_nTstNdx;
					unsigned int nBook = 1;
					for (unsigned int i=1; i<nTestament; ++i)
						nBook += m_pBibleDatabase->testamentEntry(i)->m_nNumBk;
					indexScoped = CRelIndex(nBook, 0, 0 ,0);
				}
			}
			break;
		case (CSearchCriteria::SSME_CATEGORY):
			// For Category, set the Book to the 1st Book of the corresponding Category:
			if (index.book()) {
				uint32_t nCat = m_pBibleDatabase->bookCategory(index);
				if (nCat) {
					Q_ASSERT(m_pBibleDatabase->bookCategoryEntry(nCat) != nullptr);
					const CBookCategoryEntry &category = *m_pBibleDatabase->bookCategoryEntry(nCat);
					if (category.m_setBooksNum.find(index.book()) != category.m_setBooksNum.end()) {
						// Get first book of the category for the scope:
						indexScoped = CRelIndex(*(category.m_setBooksNum.begin()), 0, 0, 0);
					}
				}
			}
			break;
		case (CSearchCriteria::SSME_BOOK):
			// For Book, mask off Chapter, Verse, and Word:
			indexScoped = CRelIndex(index.book(), 0, 0, 0);
			break;
		case (CSearchCriteria::SSME_CHAPTER):
			// For Chapter, mask off Verse and Word:
			indexScoped = CRelIndex(index.book(), index.chapter(), 0, 0);
			break;
		case (CSearchCriteria::SSME_VERSE):
			// For Verse, mask off word:
			indexScoped = CRelIndex(index.book(), index.chapter(), index.verse(), 0);
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	return indexScoped;
}

// ============================================================================

QString CSearchResultsSummary::summaryDisplayText(CBibleDatabasePtr pBibleDatabase, bool bExcluded, bool bWebChannelHTML) const
{
	QString strResults;

	// TODO : Fix Translation Contexts Here...

	// ------------------------------------------------------------------------

	if (!bExcluded) {
		strResults += CKJVSearchResult::tr("Found %n Occurrence(s)", "Statistics", m_nSearchOccurrences) + "\n";
		strResults += "    " + CKJVSearchResult::tr("in %n Verse(s)", "Statistics", m_nSearchVerses) +
						" " + CKJVSearchResult::tr("in %n Chapter(s)", "Statistics", m_nSearchChapters) +
						" " + CKJVSearchResult::tr("in %n Book(s)", "Statistics", m_nSearchBooks);
		if (m_SearchCriteria.withinIsEntireBible(pBibleDatabase, true)) {
			if (!CPersistentSettings::instance()->hideNotFoundInStatistcs()) {
				if (m_nSearchOccurrences > 0) {
					if (bWebChannelHTML) {
						QString strSearchWithinDescription = m_SearchCriteria.searchWithinDescription(pBibleDatabase);
						if (!strSearchWithinDescription.isEmpty()) {
							strResults += " " + CKJVSearchResult::tr("within", "Statistics") + " " + strSearchWithinDescription;
						}
						strResults += "<br />\n<br /><span style=\"text-align: center;\">\n";
						strResults += CKJVSearchResult::tr("Not found at all", "Statistics");
						strResults += " " + CKJVSearchResult::tr("in %n Verse(s)", "Statistics", pBibleDatabase->bibleEntry().m_nNumVrs - m_nSearchVerses) +
										" " + CKJVSearchResult::tr("in %n Chapter(s)", "Statistics", pBibleDatabase->bibleEntry().m_nNumChp - m_nSearchChapters) +
										" " + CKJVSearchResult::tr("in %n Book(s)", "Statistics", pBibleDatabase->bibleEntry().m_nNumBk - m_nSearchBooks);
						strResults += "<br /></span>\n";
					} else {
						strResults += "\n";
						strResults += "    " + CKJVSearchResult::tr("Not found at all in %n Verse(s) of the Bible", "Statistics", pBibleDatabase->bibleEntry().m_nNumVrs - m_nSearchVerses) + "\n";
						strResults += "    " + CKJVSearchResult::tr("Not found at all in %n Chapter(s) of the Bible", "Statistics", pBibleDatabase->bibleEntry().m_nNumChp - m_nSearchChapters) + "\n";
						strResults += "    " + CKJVSearchResult::tr("Not found at all in %n Book(s) of the Bible", "Statistics", pBibleDatabase->bibleEntry().m_nNumBk - m_nSearchBooks);
					}
				}
			}
		} else {
			QString strSearchWithinDescription = m_SearchCriteria.searchWithinDescription(pBibleDatabase);
			if (!strSearchWithinDescription.isEmpty()) {
				strResults += " " + CKJVSearchResult::tr("within", "Statistics") + " " + strSearchWithinDescription;
			}
		}
	} else {
		strResults += CKJVSearchResult::tr("Excluded %n Occurrence(s)", "Statistics", m_nExcludedSearchOccurrences) + "\n";
		strResults += "    " + CKJVSearchResult::tr("in %n Verse(s)", "Statistics", m_nExcludedSearchVerses) +
						" " + CKJVSearchResult::tr("in %n Chapter(s)", "Statistics", m_nExcludedSearchChapters) +
						" " + CKJVSearchResult::tr("in %n Book(s)", "Statistics", m_nExcludedSearchBooks);
		if (!m_SearchCriteria.withinIsEntireBible(pBibleDatabase)) {
			QString strSearchWithinDescription = m_SearchCriteria.searchWithinDescription(pBibleDatabase);
			if (!strSearchWithinDescription.isEmpty()) {
				strResults += " " + CKJVSearchResult::tr("within", "Statistics") + " " + strSearchWithinDescription;
			}
		}
	}

	// ------------------------------------------------------------------------

	return strResults;
}

QString CSearchResultsSummary::summaryCopyText(CBibleDatabasePtr pBibleDatabase) const
{
	QString strSummary;

	// TODO : Fix Translation Contexts Here...

	if (m_bValid) {
		strSummary += CKJVSearchResult::tr("Found %n%1Occurrence(s)", "Statistics", m_nSearchOccurrences).arg((numTotalSearchPhrases() > 1) ? (" " + CKJVSearchResult::tr("Combined", "Statistics", m_nSearchOccurrences) + " ") : " ").trimmed();
		strSummary += (m_SearchCriteria.withinIsEntireBible(pBibleDatabase) ? "" : (" " + CKJVSearchResult::tr("in the Selected Search Text", "Statistics"))) + "\n";
		strSummary += "    " + CKJVSearchResult::tr("in %n Verse(s)", "Statistics", m_nSearchVerses) + "\n";
		strSummary += "    " + CKJVSearchResult::tr("in %n Chapter(s)", "Statistics", m_nSearchChapters) + "\n";
		strSummary += "    " + CKJVSearchResult::tr("in %n Book(s)", "Statistics", m_nSearchBooks) + "\n";
		if (m_SearchCriteria.withinIsEntireBible(pBibleDatabase, true)) {
			strSummary += "\n";
			strSummary += CKJVSearchResult::tr("Not found%1at all in %n Verse(s) of the Bible", "Statistics", pBibleDatabase->bibleEntry().m_nNumVrs - m_nSearchVerses).arg(((numTotalSearchPhrases() > 1) && (m_SearchCriteria.searchScopeMode() != CSearchCriteria::SSME_UNSCOPED)) ? (" " + CKJVSearchResult::tr("together", "Statistics") + " ") : " ") + "\n";
			strSummary += CKJVSearchResult::tr("Not found%1at all in %n Chapter(s) of the Bible", "Statistics", pBibleDatabase->bibleEntry().m_nNumChp - m_nSearchChapters).arg(((numTotalSearchPhrases() > 1) && (m_SearchCriteria.searchScopeMode() != CSearchCriteria::SSME_UNSCOPED)) ? (" " + CKJVSearchResult::tr("together", "Statistics") + " ") : " ") + "\n";
			strSummary += CKJVSearchResult::tr("Not found%1at all in %n Book(s) of the Bible", "Statistics", pBibleDatabase->bibleEntry().m_nNumBk - m_nSearchBooks).arg(((numTotalSearchPhrases() > 1) && (m_SearchCriteria.searchScopeMode() != CSearchCriteria::SSME_UNSCOPED)) ? (" " + CKJVSearchResult::tr("together", "Statistics") + " ") : " ") + "\n";
		} else {
			QString strSearchWithinDescription = m_SearchCriteria.searchWithinDescription(pBibleDatabase);
			if (!strSearchWithinDescription.isEmpty()) {
				strSummary += "    " + CKJVSearchResult::tr("within", "Statistics") + " " + strSearchWithinDescription + "\n";
			}
		}
		if (m_nExcludedSearchOccurrences > 0) {
			strSummary += "\n";
			strSummary += CKJVSearchResult::tr("Excluded %n%1Occurrence(s)", "Statistics", m_nExcludedSearchOccurrences).arg((numTotalSearchPhrases() > 1) ? (" " + CKJVSearchResult::tr("Combined", "Statistics", m_nExcludedSearchOccurrences) + " ") : " ").trimmed() + "\n";
			strSummary += "    " + CKJVSearchResult::tr("in %n Verse(s)", "Statistics", m_nExcludedSearchVerses) + "\n";
			strSummary += "    " + CKJVSearchResult::tr("in %n Chapter(s)", "Statistics", m_nExcludedSearchChapters) + "\n";
			strSummary += "    " + CKJVSearchResult::tr("in %n Book(s)", "Statistics", m_nExcludedSearchBooks) + "\n";
			if (!m_SearchCriteria.withinIsEntireBible(pBibleDatabase)) {
				QString strSearchWithinDescription = m_SearchCriteria.searchWithinDescription(pBibleDatabase);
				if (!strSearchWithinDescription.isEmpty()) {
					strSummary += "    " + CKJVSearchResult::tr("within", "Statistics") + " " + strSearchWithinDescription + "\n";
				}
			}
		}
	} else {
		strSummary += CKJVSearchResult::tr("Search was incomplete -- too many possible matches", "Statistics") + "\n";
	}

	return strSummary;
}

// ============================================================================
