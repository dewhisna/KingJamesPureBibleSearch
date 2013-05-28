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

#include "VerseListModel.h"

#include <QVector>
#include <QModelIndexList>

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
		qSort(aVerseList.begin(), aVerseList.end(), ascendingLessThanVLI);
	else
		qSort(aVerseList.begin(), aVerseList.end(), decendingLessThanVLI);
}

// ============================================================================

CVerseListModel::CVerseListModel(CBibleDatabasePtr pBibleDatabase, QObject *parent)
	:	QAbstractItemModel(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_nDisplayMode(VDME_HEADING),
		m_nTreeMode(VTME_LIST),
		m_bShowMissingLeafs(false)
{
}

CVerseListModel::CVerseListModel(CBibleDatabasePtr pBibleDatabase, const CVerseList &verses, QObject *parent)
	:	QAbstractItemModel(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_nDisplayMode(VDME_HEADING),
		m_nTreeMode(VTME_LIST),
		m_bShowMissingLeafs(false)
{
	setVerseList(verses);
}

int CVerseListModel::rowCount(const QModelIndex &parent) const
{
	switch (m_nTreeMode) {
		case VTME_LIST:
		{
			if (parent.isValid()) return 0;
			return m_lstVerses.count();
		}
		case VTME_TREE_BOOKS:
		{
			if (!parent.isValid()) return GetBookCount();
			CRelIndex ndxRel(parent.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.chapter() == 0) return GetVerseCount(ndxRel.book());
			return 0;
		}
		case VTME_TREE_CHAPTERS:
		{
			if (!parent.isValid()) return GetBookCount();
			CRelIndex ndxRel(parent.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.chapter() == 0) return GetChapterCount(ndxRel.book());
			if (ndxRel.verse() == 0) return GetVerseCount(ndxRel.book(), ndxRel.chapter());
			return 0;
		}
		default:
			break;
	}

	return 0;
}

int CVerseListModel::columnCount(const QModelIndex &parent) const
{
	switch (m_nTreeMode) {
		case VTME_LIST:
		{
			if (!parent.isValid()) return 1;
			return 0;
		}
		case VTME_TREE_BOOKS:
		{
			if (!parent.isValid()) return 1;
			CRelIndex ndxRel(parent.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.chapter() == 0) return 1;
			return 0;
		}
		case VTME_TREE_CHAPTERS:
		{
			if (!parent.isValid()) return 1;
			CRelIndex ndxRel(parent.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.chapter() == 0) return 1;
			if (ndxRel.verse() == 0) return 1;
			return 0;
		}
		default:
			break;
	}

	return 0;
}

QModelIndex	CVerseListModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent)) return QModelIndex();

	switch (m_nTreeMode) {
		case VTME_LIST:
		{
			return createIndex(row, column, m_lstVerses.at(row).getIndex().index());
		}
		case VTME_TREE_BOOKS:
		{
			if (!parent.isValid()) {
				return createIndex(row, column, CRelIndex(BookByIndex(row), 0, 0, 0).index());
			}
			CRelIndex ndxRel(parent.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.chapter() == 0) {
				int nVerse = GetVerse(row, ndxRel.book());
				if (nVerse == -1) return QModelIndex();
				return createIndex(row, column, m_lstVerses.at(nVerse).getIndex().index());
			}
			return QModelIndex();
		}
		case VTME_TREE_CHAPTERS:
		{
			if (!parent.isValid()) {
				return createIndex(row, column, CRelIndex(BookByIndex(row), 0, 0, 0).index());
			}
			CRelIndex ndxRel(parent.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.chapter() == 0) {
				return createIndex(row, column, CRelIndex(ndxRel.book(), ChapterByIndex(parent.row(), row), 0, 0).index());
			}
			if (ndxRel.verse() == 0) {
				int nVerse = GetVerse(row, ndxRel.book(), ndxRel.chapter());
				if (nVerse == -1) return QModelIndex();
				return createIndex(row, column, m_lstVerses.at(nVerse).getIndex().index());
			}
			return QModelIndex();
		}
		default:
			break;
	}

	return QModelIndex();
}

QModelIndex CVerseListModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) return QModelIndex();

	switch (m_nTreeMode) {
		case VTME_LIST:
		{
			return QModelIndex();
		}
		case VTME_TREE_BOOKS:
		{
			CRelIndex ndxRel(index.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.verse() != 0) {
				if (!m_mapVerses.contains(ndxRel.index())) return QModelIndex();
				return createIndex(IndexByBook(ndxRel.book()), 0, CRelIndex(ndxRel.book(), 0, 0, 0).index());
			}
			return QModelIndex();
		}
		case VTME_TREE_CHAPTERS:
		{
			CRelIndex ndxRel(index.internalId());
			assert(ndxRel.isSet());
			if (ndxRel.verse() != 0) {
				if (!m_mapVerses.contains(ndxRel.index())) return QModelIndex();
				return createIndex(IndexByChapter(ndxRel.book(), ndxRel.chapter()), 0, CRelIndex(ndxRel.book(), ndxRel.chapter(), 0, 0).index());
			}
			if (ndxRel.chapter() != 0) {
				return createIndex(IndexByBook(ndxRel.book()), 0, CRelIndex(ndxRel.book(), 0, 0, 0).index());
			}
			return QModelIndex();
		}
		default:
			break;
	}

	return QModelIndex();
}

QVariant CVerseListModel::data(const QModelIndex &index, int role) const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (!index.isValid()) return QVariant();

	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());
	if (!ndxRel.isSet()) return QVariant();

	if (role == Qt::SizeHintRole) return m_mapSizeHints.value(ndxRel.index(), QSize());

	if ((ndxRel.chapter() == 0) && (ndxRel.verse() == 0)) {
		if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
			QString strBookText = m_pBibleDatabase->bookName(ndxRel);
			if (m_nDisplayMode != VDME_HEADING) return strBookText;		// For Rich Text, Let delegate add results so it can be formatted
			int nVerses = GetVerseCount(ndxRel.book());
			int nResults = GetResultsCount(ndxRel.book());
			if ((nResults) || (nVerses)) strBookText = QString("{%1} (%2) ").arg(nVerses).arg(nResults) + strBookText;
			return strBookText;
		}
		if ((role == Qt::ToolTipRole) ||
			(role == TOOLTIP_ROLE) ||
			(role == TOOLTIP_PLAINTEXT_ROLE) ||
			(role == TOOLTIP_NOHEADING_ROLE) ||
			(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
			return QString();
		}
		return QVariant();
	}

	if (ndxRel.verse() == 0) {
		if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
			QString strChapterText = m_pBibleDatabase->bookName(ndxRel) + QString(" %1").arg(ndxRel.chapter());
			if (m_nDisplayMode != VDME_HEADING) return strChapterText;	// For Rich Text, Let delegate add results so it can be formatted
			int nVerses = GetVerseCount(ndxRel.book(), ndxRel.chapter());
			int nResults = GetResultsCount(ndxRel.book(), ndxRel.chapter());
			if ((nResults) || (nVerses)) strChapterText = QString("{%1} (%2) ").arg(nVerses).arg(nResults) + strChapterText;
			return strChapterText;
		}
		if ((role == Qt::ToolTipRole) ||
			(role == TOOLTIP_ROLE) ||
			(role == TOOLTIP_PLAINTEXT_ROLE) ||
			(role == TOOLTIP_NOHEADING_ROLE) ||
			(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
			return QString();
		}
		return QVariant();
	}

	if (!m_mapVerses.contains(ndxRel.index())) return QVariant();
	int nVerse = m_mapVerses[ndxRel.index()];
	assert((nVerse>=0) && (nVerse<m_lstVerses.size()));

	if (nVerse < 0 || nVerse >= m_lstVerses.size())
		return QVariant();

	if (role == Qt::ToolTipRole) return QString();		// on_viewDetails replaces normal ToolTip

	return dataForVerse(m_lstVerses.at(nVerse), role);
}

QVariant CVerseListModel::dataForVerse(const CVerseListItem &aVerse, int role) const
{
	if (!m_mapVerses.contains(aVerse.getIndex().index())) return QVariant();
	int nVerse = m_mapVerses[aVerse.getIndex().index()];
	assert((nVerse>=0) && (nVerse<m_lstVerses.size()));

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		switch (m_nDisplayMode) {
			case VDME_HEADING:
				return aVerse.getHeading();
			case VDME_VERYPLAIN:
				return aVerse.getVerseVeryPlainText();
			case VDME_RICHTEXT:
				return aVerse.getVerseRichText();
			case VDME_COMPLETE:
				return aVerse.getVerseRichText();		// TODO : FINISH THIS ONE!!!
			default:
				return QString();
		}
	}

	if ((role == TOOLTIP_ROLE) ||
		(role == TOOLTIP_PLAINTEXT_ROLE) ||
		(role == TOOLTIP_NOHEADING_ROLE) ||
		(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
		bool bHeading = ((role != TOOLTIP_NOHEADING_ROLE) && (role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE));
		QString strToolTip;
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "<qt><pre>";
		if (bHeading) strToolTip += aVerse.getHeading() + "\n";
		QPair<int, int> nResultsIndexes = GetResultsIndexes(nVerse);
		if (nResultsIndexes.first != nResultsIndexes.second) {
			strToolTip += QString("%1").arg(bHeading ? "    " : "") +
						tr("Search Results %1-%2 of %3 phrase occurrences")
									.arg(nResultsIndexes.first)
									.arg(nResultsIndexes.second)
									.arg(GetResultsCount()) + "\n";
		} else {
			assert(nResultsIndexes.first != 0);		// This will assert if the row was beyond those defined in our list
			strToolTip += QString("%1").arg(bHeading ? "    " : "") +
						tr("Search Result %1 of %2 phrase occurrences")
									.arg(nResultsIndexes.first)
									.arg(GetResultsCount()) + "\n";
		}
		QPair<int, int> nVerseResult = GetVerseIndexAndCount(nVerse);
		strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Verse %1 of %2 in Search Scope").arg(nVerseResult.first).arg(nVerseResult.second) + "\n";
		QPair<int, int> nChapterResult = GetChapterIndexAndCount(nVerse);
		strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Chapter %1 of %2 in Search Scope").arg(nChapterResult.first).arg(nChapterResult.second) + "\n";
		QPair<int, int> nBookResult = GetBookIndexAndCount(nVerse);
		strToolTip += QString("%1    ").arg(bHeading ? "    " : "") + tr("Book %1 of %2 in Search Scope").arg(nBookResult.first).arg(nBookResult.second) + "\n";
		strToolTip += aVerse.getToolTip(m_lstParsedPhrases);
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "</pre></qt>";
		return strToolTip;
	}

	if (role == VERSE_ENTRY_ROLE) {
		return QVariant::fromValue(aVerse);
	}

	return QVariant();
}

bool CVerseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::SizeHintRole) {
		if (!index.isValid()) {
			// Special Case:  QModelIndex() is "invalidate all":
			m_mapSizeHints.clear();
			return false;				// But return false because we can't actually set a SizeHint for an invalid index
		}

		CRelIndex ndxRel(index.internalId());
		assert(ndxRel.isSet());
		if (!ndxRel.isSet()) return false;

		m_mapSizeHints[ndxRel.index()] = value.toSize();
		return true;
	}

/*
	if (index.row() < 0 || index.row() >= m_lstVerses.size()) return false;

	if ((role == Qt::EditRole) || (role == Qt::DisplayRole)) {
		switch (m_nDisplayMode) {
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

Qt::ItemFlags CVerseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());
	if (ndxRel.verse() == 0) return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable /* | Qt::ItemIsEditable */ | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool CVerseListModel::insertRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(row);
	Q_UNUSED(count);
	Q_UNUSED(parent);

	return false;
/*
	if (count < 1 || row < 0 || row > m_lstVerses.size())
		return false;
	if (parent.isValid()) return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.insert(row, CVerseListItem());

	endInsertRows();

	return true;
*/
}

bool CVerseListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(row);
	Q_UNUSED(count);
	Q_UNUSED(parent);

	return false;

/*
	if (count <= 0 || row < 0 || (row + count) > m_lstVerses.size())
		return false;
	if (parent.isValid()) return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.removeAt(row);

	endRemoveRows();

	return true;
*/
}

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

void CVerseListModel::sort(int /* column */, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();

	QList<QPair<CVerseListItem, int> > list;
	for (int i = 0; i < m_lstVerses.count(); ++i)
		list.append(QPair<CVerseListItem, int>(m_lstVerses.at(i), i));

	if (order == Qt::AscendingOrder)
		qSort(list.begin(), list.end(), ascendingLessThan);
	else
		qSort(list.begin(), list.end(), decendingLessThan);

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
}

Qt::DropActions CVerseListModel::supportedDropActions() const
{
	return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

QModelIndex CVerseListModel::locateIndex(const CRelIndex &ndxRel) const
{
	if (!ndxRel.isSet()) return QModelIndex();

	int ndxTarget;
	int ndxFirst;

	// See if this is a verse (search result) reference.  If so resolve:
	if (ndxRel.verse() != 0) {
		// Set ndxVerse to first verse m_lstVerses array for this parent node:
		if (m_nTreeMode == VTME_LIST) {
			ndxFirst = 0;			// For list mode, the list includes everything, so start with the first index
		} else {
			ndxFirst = GetVerse(0, ndxRel.book(), ((m_nTreeMode == VTME_TREE_CHAPTERS ) ? ndxRel.chapter() : 0));
		}
		ndxTarget = FindVerseIndex(ndxRel);
		if (ndxTarget == -1) return QModelIndex();
		return createIndex(ndxTarget-ndxFirst, 0, m_lstVerses.at(ndxTarget).getIndex().index());		// Use index from actual verse instead of ndxRel since word() isn't required to match
	}

	// If we are in list mode and caller only gave us a book/chapter reference,
	//		then we simply don't have an index to return.  That makes more
	//		sense than possibly returning the first one that might match:
	if (m_nTreeMode == VTME_LIST) return QModelIndex();

	if ((ndxRel.chapter() != 0) && (m_nTreeMode == VTME_TREE_CHAPTERS)) {
		// If this is a book/chapter reference, resolve it:
		ndxTarget = IndexByChapter(ndxRel.book(), ndxRel.chapter());
		if (ndxTarget == -1) return QModelIndex();
		return createIndex(ndxTarget, 0, CRelIndex(ndxRel.book(), ndxRel.chapter(), 0, 0).index());		// Create CRelIndex rather than using ndxRel, since we aren't requiring word() to match
	} else {
		// If this is a book-only reference, resolve it:
		ndxTarget = IndexByBook(ndxRel.book());
		if (ndxTarget == -1) return QModelIndex();
		return createIndex(ndxTarget, 0, CRelIndex(ndxRel.book(), 0, 0, 0).index());	// Create CRelIndex rather than using ndxRel, since we aren't requiring word() to match
	}

	return QModelIndex();
}

// ----------------------------------------------------------------------------

CVerseList CVerseListModel::verseList() const
{
	return m_lstVerses;
}

void CVerseListModel::setVerseList(const CVerseList &verses)
{
	emit beginResetModel();
	m_lstVerses = verses;
	m_mapVerses.clear();
	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		m_mapVerses[m_lstVerses.at(ndx).getIndex().index()] = ndx;
	}
	emit endResetModel();
}

TParsedPhrasesList CVerseListModel::parsedPhrases() const
{
	return m_lstParsedPhrases;
}

TPhraseTagList CVerseListModel::setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases)
{
	// Note: Basic setting of this list doesn't change the model, as the phrases
	//		themselves are used primarily for building of tooltips that are
	//		appropriate for the entire search scope.  However, once these are
	//		set, we'll call the buildVerseListFromParsedPhrases function that
	//		will build and set the VerseList, which will change the model.
	//		Therefore, the beginResetModel/endResetModel calls don't exist here,
	//		but down in setVerseList:
	m_lstParsedPhrases = phrases;
	m_SearchCriteria = aSearchCriteria;
	buildScopedResultsInParsedPhrases();
	return buildVerseListFromParsedPhrases();
}

void CVerseListModel::setDisplayMode(VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	m_mapSizeHints.clear();
	emit layoutAboutToBeChanged();
	m_nDisplayMode = nDisplayMode;
	emit layoutChanged();
}

void CVerseListModel::setTreeMode(VERSE_TREE_MODE_ENUM nTreeMode)
{
	m_mapSizeHints.clear();
	emit beginResetModel();
	m_nTreeMode = nTreeMode;
	emit endResetModel();
}

void CVerseListModel::setShowMissingLeafs(bool bShowMissing)
{
	if (m_nTreeMode != VTME_LIST) beginResetModel();
	m_bShowMissingLeafs = bShowMissing;
	if (m_nTreeMode != VTME_LIST) endResetModel();
}

// ----------------------------------------------------------------------------

int CVerseListModel::GetResultsCount(unsigned int nBk, unsigned int nChp) const
{
	int nResults = 0;

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if ((nBk != 0) && (m_lstVerses.at(ndx).getBook() != nBk)) continue;
		if ((nBk != 0) && (nChp != 0) && (m_lstVerses.at(ndx).getChapter() != nChp)) continue;
		nResults += m_lstVerses.at(ndx).phraseTags().size();
	}

	return nResults;
}

// ----------------------------------------------------------------------------

QPair<int, int> CVerseListModel::GetResultsIndexes(int nVerse) const
{
	QPair<int, int> nResultsIndexes;
	nResultsIndexes.first = 0;
	nResultsIndexes.second = 0;

	for (int ndx = 0; ((ndx < nVerse) && (ndx < m_lstVerses.size())); ++ndx) {
		nResultsIndexes.first += m_lstVerses.at(ndx).phraseTags().size();
	}
	nResultsIndexes.second = nResultsIndexes.first;
	if (nVerse < m_lstVerses.size()) {
		nResultsIndexes.first++;
		nResultsIndexes.second += m_lstVerses.at(nVerse).phraseTags().size();
	}

	return nResultsIndexes;		// Result first = first result index, second = last result index for specified row
}

QPair<int, int> CVerseListModel::GetBookIndexAndCount(int nVerse) const
{
	int ndxBook = 0;		// Index into Books
	int nBooks = 0;			// Results counts in Books

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		nBooks++;			// Count the book we are on and skip the ones that are on the same book:
		if (ndx <= nVerse) ndxBook++;
		if (ndx < (m_lstVerses.size()-1)) {
			bool bNextIsSameReference=false;
			uint32_t nCurrentBook = m_lstVerses.at(ndx).getBook();
			do {
				if (nCurrentBook == m_lstVerses.at(ndx+1).getBook()) {
					bNextIsSameReference=true;
					ndx++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndx<(m_lstVerses.size()-1)));
		}
	}

	return QPair<int, int>(ndxBook, nBooks);
}

QPair<int, int> CVerseListModel::GetChapterIndexAndCount(int nVerse) const
{
	int ndxChapter = 0;		// Index into Chapters
	int nChapters = 0;		// Results counts in Chapters

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same book/chapter:
		if (ndx <= nVerse) ndxChapter++;
		if (ndx < (m_lstVerses.size()-1)) {
			bool bNextIsSameReference=false;
			uint32_t nCurrentBook = m_lstVerses.at(ndx).getBook();
			uint32_t nCurrentChapter = m_lstVerses.at(ndx).getChapter();
			do {
				if ((nCurrentBook == m_lstVerses.at(ndx+1).getBook()) &&
					(nCurrentChapter == m_lstVerses.at(ndx+1).getChapter())) {
					bNextIsSameReference=true;
					ndx++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndx<(m_lstVerses.size()-1)));
		}
	}

	return QPair<int, int>(ndxChapter, nChapters);
}

QPair<int, int> CVerseListModel::GetVerseIndexAndCount(int nVerse) const
{
	return QPair<int, int>(nVerse+1, m_lstVerses.size());
}

// ----------------------------------------------------------------------------

int CVerseListModel::GetBookCount() const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bShowMissingLeafs) return m_pBibleDatabase->bibleEntry().m_nNumBk;

	QMap<uint32_t, int>::const_iterator itrVerseMapBookFirst = m_mapVerses.begin();
	QMap<uint32_t, int>::const_iterator itrVerseMapBookLast = m_mapVerses.end();

	int nCount = 0;
	while (itrVerseMapBookFirst != itrVerseMapBookLast) {
		CRelIndex relndxCurrent(itrVerseMapBookFirst.key());
		// Find next book (bypassing any chapters/verses in the current book):
		itrVerseMapBookFirst = m_mapVerses.lowerBound(CRelIndex(relndxCurrent.book() + 1, 0, 0, 0).index());
		++nCount;
	}
	return nCount;
}

int CVerseListModel::IndexByBook(unsigned int nBk) const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bShowMissingLeafs) {
		if ((nBk < 1) || (nBk > m_pBibleDatabase->bibleEntry().m_nNumBk)) return -1;
		return (nBk-1);
	}

	// Find the first entry with the correct Book number:
	QMap<uint32_t, int>::const_iterator itrVerseMapBookFirst = m_mapVerses.begin();
	QMap<uint32_t, int>::const_iterator itrVerseMapBookLast = m_mapVerses.lowerBound(CRelIndex(nBk, 0, 0, 0).index());

	// If we didn't find the book, return -1 (not found):
	if (itrVerseMapBookLast == m_mapVerses.end()) return -1;
	if (CRelIndex(itrVerseMapBookLast.key()).book() != nBk) return -1;

	int nIndex = 0;
	while (itrVerseMapBookFirst != itrVerseMapBookLast) {
		CRelIndex relndxCurrent(itrVerseMapBookFirst.key());
		// Find next book (bypassing any chapters/verses in the current book):
		itrVerseMapBookFirst = m_mapVerses.lowerBound(CRelIndex(relndxCurrent.book() + 1, 0, 0, 0).index());
		assert(itrVerseMapBookFirst != m_mapVerses.end());		// Shouldn't hit the end because we already know the correct book exists
		++nIndex;
	}
	return nIndex;
}

unsigned int CVerseListModel::BookByIndex(int ndxBook) const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bShowMissingLeafs) {
		if ((ndxBook < 0) || (static_cast<unsigned int>(ndxBook) >= m_pBibleDatabase->bibleEntry().m_nNumBk)) return 0;
		return (ndxBook+1);
	}

	QMap<uint32_t, int>::const_iterator itrVerseMapBookFirst = m_mapVerses.begin();
	QMap<uint32_t, int>::const_iterator itrVerseMapBookLast = m_mapVerses.end();

	int nIndex = 0;
	while (itrVerseMapBookFirst != itrVerseMapBookLast) {
		CRelIndex relndxCurrent(itrVerseMapBookFirst.key());
		if (nIndex == ndxBook) return relndxCurrent.book();			// If we've found the right index, return the book
		// Find next book (bypassing any chapters/verses in the current book):
		itrVerseMapBookFirst = m_mapVerses.lowerBound(CRelIndex(relndxCurrent.book() + 1, 0, 0, 0).index());
		++nIndex;
	}
	assert(false);
	return 0;				// Should have already returned a chapter above, but 0 if we're given an index beyond the list
}

int CVerseListModel::GetChapterCount(unsigned int nBk) const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (nBk == 0) return 0;
	if (m_bShowMissingLeafs) {
		if (nBk > m_pBibleDatabase->bibleEntry().m_nNumBk) return 0;
		return m_pBibleDatabase->bookEntry(nBk)->m_nNumChp;
	}

	// Find the first and last entries with the correct Book number:
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterFirst;
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterLast;
	itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, 0, 0, 0).index());			// This will be the first verse of the first chapter of this book
	itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0).index());			// This will be the first verse of the next book/chapter

	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return 0;
	if (CRelIndex(itrVerseMapBookChapterFirst.key()).book() != nBk) return 0;

	int nCount = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		CRelIndex relndxCurrent(itrVerseMapBookChapterFirst.key());
		// Find next chapter (bypassing any verses in the current chapter):
		itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, relndxCurrent.chapter() + 1, 0, 0).index());
		++nCount;
	}
	return nCount;
}

int CVerseListModel::IndexByChapter(unsigned int nBk, unsigned int nChp) const
{
	assert(m_pBibleDatabase.data() != NULL);

	if ((nBk == 0) || (nChp == 0)) return -1;
	if (m_bShowMissingLeafs) {
		if (nBk > m_pBibleDatabase->bibleEntry().m_nNumBk) return -1;
		if (nChp > m_pBibleDatabase->bookEntry(nBk)->m_nNumChp) return -1;
		return (nChp-1);
	}

	// Find the first entry with the correct Book number and with the correct Book/Chapter number:
	QMap<uint32_t, int>::const_iterator itrVerseMapBook = m_mapVerses.lowerBound(CRelIndex(nBk, 0, 0, 0).index());
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapter = m_mapVerses.lowerBound(CRelIndex(nBk, nChp, 0, 0).index());

	// If we didn't find the book and/or book/chapter, return -1 (not found):
	if ((itrVerseMapBook == m_mapVerses.end()) || (itrVerseMapBookChapter == m_mapVerses.end())) return -1;
	if (CRelIndex(itrVerseMapBook.key()).book() != nBk) return -1;
	CRelIndex relndxLast(itrVerseMapBookChapter.key());
	if ((relndxLast.book() != nBk) || (relndxLast.chapter() != nChp)) return -1;

	int nIndex = 0;
	while (itrVerseMapBook != itrVerseMapBookChapter) {
		CRelIndex relndxCurrent(itrVerseMapBook.key());
		// Find next chapter (bypassing any verses in the current chapter):
		itrVerseMapBook = m_mapVerses.lowerBound(CRelIndex(nBk, relndxCurrent.chapter() + 1, 0, 0).index());
		assert(itrVerseMapBook != m_mapVerses.end());		// Shouldn't hit the end because we already know the correct book/chapter exists
		++nIndex;
	}
	return nIndex;
}

unsigned int CVerseListModel::ChapterByIndex(int ndxBook, int ndxChapter) const
{
	assert(m_pBibleDatabase.data() != NULL);

	if ((ndxBook < 0) || (ndxChapter < 0)) return 0;
	if (m_bShowMissingLeafs) {
		if (static_cast<unsigned int>(ndxBook) >= m_pBibleDatabase->bibleEntry().m_nNumBk) return 0;
		if (static_cast<unsigned int>(ndxChapter) >= m_pBibleDatabase->bookEntry(ndxBook+1)->m_nNumChp) return 0;
		return (ndxChapter+1);
	}

	unsigned int nBk = BookByIndex(ndxBook);
	if (nBk == 0) return 0;

	// Find the first and last entries with the correct Book number:
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, 0, 0, 0).index());			// This will be the first verse of the first chapter of this book
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0).index());			// This will be the first verse of the next book/chapter

	// We should have found the book, because of the above BookByIndex() call and nBk check, but safe-guard:
	assert(itrVerseMapBookChapterFirst != m_mapVerses.end());
	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return 0;

	int nIndex = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		CRelIndex relndxCurrent(itrVerseMapBookChapterFirst.key());
		if (nIndex == ndxChapter) return relndxCurrent.chapter();			// If we've found the right index, return the chapter
		// Find next chapter (bypassing any verses in the current chapter):
		itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, relndxCurrent.chapter() + 1, 0, 0).index());
		++nIndex;
	}
	assert(false);
	return 0;				// Should have already returned a chapter above, but 0 if we're given an index beyond the list
}

int CVerseListModel::GetVerseCount(unsigned int nBk, unsigned int nChp) const
{
	// Note: This function has special cases for nBk == 0 and nChp == 0 (unlike the other count functions)

	if (nBk == 0) return m_lstVerses.size();		// Quick special-case

	// Find the first and last entries with the correct Book/Chapter number:
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterFirst;
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterLast;
	itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, nChp, 0, 0).index());			// This will be the first verse of this chapter of this book
	if (nChp != 0) {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk, nChp+1, 0, 0).index());		// This will be the first verse of the next book/chapter
	} else {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0).index());			// This will be the first verse of the next book
	}

	// If we didn't find the book and/or book/chapter, return none found:
	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return 0;
	CRelIndex relndxFirst(itrVerseMapBookChapterFirst.key());
	if ((relndxFirst.book() != nBk) || ((nChp != 0) && (relndxFirst.chapter() != nChp))) return 0;

	int nVerses = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		++itrVerseMapBookChapterFirst;
		++nVerses;
	}
	return nVerses;
}

int CVerseListModel::GetVerse(int ndxVerse, unsigned int nBk, unsigned int nChp) const
{
	// Note: This function has a special case for nChp == 0 (unlike the other index functions)

	if ((nBk == 0) || (ndxVerse < 0)) return -1;

	// Find the first and last entries with the correct Book/Chapter number:
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterFirst;
	QMap<uint32_t, int>::const_iterator itrVerseMapBookChapterLast;
	itrVerseMapBookChapterFirst = m_mapVerses.lowerBound(CRelIndex(nBk, nChp, 0, 0).index());			// This will be the first verse of this chapter of this book
	if (nChp != 0) {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk, nChp+1, 0, 0).index());			// This will be the first verse of the next book/chapter
	} else {
		itrVerseMapBookChapterLast = m_mapVerses.lowerBound(CRelIndex(nBk+1, 0, 0, 0).index());
	}

	// If we didn't find the book and/or book/chapter, return -1 (not found):
	if (itrVerseMapBookChapterFirst == m_mapVerses.end()) return -1;
	CRelIndex relndxFirst(itrVerseMapBookChapterFirst.key());
	if ((relndxFirst.book() != nBk) || ((nChp != 0) && (relndxFirst.chapter() != nChp))) return -1;

	int nVerses = 0;
	while (itrVerseMapBookChapterFirst != itrVerseMapBookChapterLast) {
		if (nVerses == ndxVerse) return itrVerseMapBookChapterFirst.value();
		++itrVerseMapBookChapterFirst;
		++nVerses;
	}
	assert(false);
	return -1;				// Should have already returned a verse above, but -1 if we're given an index beyond the list
}

int CVerseListModel::FindVerseIndex(const CRelIndex &ndxRel) const
{
	if (!ndxRel.isSet()) return -1;

	CRelIndex ndxSearch(ndxRel);
	ndxSearch.setWord(0);			// Make sure we don't consider the word
	return m_mapVerses.value(ndxSearch.index(), -1);
}

// ----------------------------------------------------------------------------

void CVerseListModel::buildScopedResultsInParsedPhrases()
{
	QList<int> lstNdxStart;
	QList<int> lstNdxEnd;
	QList<CRelIndex> lstScopedRefs;
	QList<bool> lstNeedScope;
	int nNumPhrases = m_lstParsedPhrases.size();

	// Fetch results from all phrases and build a list of lists, denormalizing entries, and
	//		setting the phrase size details:
	for (int ndx=0; ndx<nNumPhrases; ++ndx) {
		lstNdxStart.append(0);
		lstNdxEnd.append(0);
		lstScopedRefs.append(CRelIndex());
		lstNeedScope.append(true);
	}

	// Now, we'll go through our lists and compress the results to the scope specified
	//		for each phrase.  We'll then find the lowest valued one and see if the others
	//		match.  If they do, we'll push all of those results onto the output.  If not,
	//		we'll toss results for the lowest until we get a match.  When any list hits
	//		its end, we're done and can break out since we have no more matches

	bool bDone = (nNumPhrases == 0);		// We're done if we have no phrases (or phrases with results)
	while (!bDone) {
		uint32_t nMaxScope = 0;
		for (int ndx=0; ndx<nNumPhrases; ++ndx) {
			const CParsedPhrase *phrase = m_lstParsedPhrases.at(ndx);
			if (!lstNeedScope[ndx]) {
				nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
				continue;		// Only find next scope for a phrase if we need it
			}
			lstNdxStart[ndx] = lstNdxEnd[ndx];		// Begin at the last ending position
			if (lstNdxStart[ndx] >= phrase->GetPhraseTagSearchResults().size()) {
				bDone = true;
				break;
			}
			lstScopedRefs[ndx] = ScopeIndex(phrase->GetPhraseTagSearchResults().at(lstNdxStart[ndx]).first, m_SearchCriteria.searchScopeMode());
			for (lstNdxEnd[ndx] = lstNdxStart[ndx]+1; lstNdxEnd[ndx] < phrase->GetPhraseTagSearchResults().size(); ++lstNdxEnd[ndx]) {
				CRelIndex ndxScopedTemp = ScopeIndex(phrase->GetPhraseTagSearchResults().at(lstNdxEnd[ndx]).first, m_SearchCriteria.searchScopeMode());
				if (lstScopedRefs[ndx].index() != ndxScopedTemp.index()) break;
			}
			// Here lstNdxEnd will be one more than the number of matching, either the next index
			//		off the end of the array, or the first non-matching entry.  So the scoped
			//		area is from lstNdxStart to lstNdxEnd-1.
			nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
			lstNeedScope[ndx] = false;
		}
		if (bDone) continue;		// If we run out of phrase matches on any phrase, we're done
		// Now, check the scoped references.  If they match for all indexes, we'll push the
		//	results to our output and set flags to get all new scopes.  Otherwise, compare them
		//	all against our maximum scope value and tag any that's less than that as needing a
		//	new scope (they weren't matches).  Then loop back until we've either pushed all
		//	results or run out of matches.
		bool bMatch = true;
		for (int ndx=0; ndx<nNumPhrases; ++ndx) {
			if (lstScopedRefs[ndx].index() != nMaxScope) {
				lstNeedScope[ndx] = true;
				bMatch = false;
			}
		}
		if (bMatch) {
			// We got a match, so push results to output and flag for new scopes:
			for (int ndx=0; ndx<nNumPhrases; ++ndx) {
				const CParsedPhrase *phrase = m_lstParsedPhrases.at(ndx);
				phrase->SetContributingNumberOfMatches(phrase->GetContributingNumberOfMatches() + (lstNdxEnd[ndx]-lstNdxStart[ndx]));
				for ( ; lstNdxStart[ndx]<lstNdxEnd[ndx]; ++lstNdxStart[ndx]) {
					phrase->AddScopedPhraseTagSearchResult(phrase->GetPhraseTagSearchResults().at(lstNdxStart[ndx]));
				}
				lstNeedScope[ndx] = true;
			}
		}
	}
}

TPhraseTagList CVerseListModel::buildVerseListFromParsedPhrases()
{
	assert(m_pBibleDatabase.data() != NULL);

	CVerseList lstReferences;
	TPhraseTagList lstResults;

	for (int ndx=0; ndx<m_lstParsedPhrases.size(); ++ndx) {
		const CParsedPhrase *phrase = m_lstParsedPhrases.at(ndx);
		assert(phrase != NULL);
		lstResults.append(phrase->GetScopedPhraseTagSearchResults());
	}

	qSort(lstResults.begin(), lstResults.end(), TPhraseTagListSortPredicate::ascendingLessThan);

	for (int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
		if (!lstResults.at(ndxResults).first.isSet()) {
			assert(false);
			lstReferences.push_back(CVerseListItem(m_pBibleDatabase, 0, 0));
			continue;
		}
		lstReferences.push_back(CVerseListItem(m_pBibleDatabase, lstResults.at(ndxResults)));

		CVerseListItem &verseItem(lstReferences.last());

		if (ndxResults<(lstResults.size()-1)) {
			bool bNextIsSameReference=false;
			CRelIndex ndxRelative = lstResults.at(ndxResults).first;
			do {
				CRelIndex ndxNextRelative = lstResults.at(ndxResults+1).first;

				if ((ndxRelative.book() == ndxNextRelative.book()) &&
					(ndxRelative.chapter() == ndxNextRelative.chapter()) &&
					(ndxRelative.verse() == ndxNextRelative.verse())) {
					verseItem.addPhraseTag(lstResults.at(ndxResults+1));
					bNextIsSameReference=true;
					ndxResults++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
		}
	}

	setVerseList(lstReferences);

	return lstResults;
}

CRelIndex CVerseListModel::ScopeIndex(const CRelIndex &index, CSearchCriteria::SEARCH_SCOPE_MODE_ENUM nMode)
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex indexScoped;

	switch (nMode) {
		case (CSearchCriteria::SSME_WHOLE_BIBLE):
			// For Whole Bible, we'll set the Book to 1 so that anything in the Bible matches:
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
			break;
	}

	return indexScoped;
}

// ----------------------------------------------------------------------------

void CVerseListModel::setFont(const QFont& aFont)
{
	Q_UNUSED(aFont);
	emit layoutAboutToBeChanged();
	emit layoutChanged();
}

// ============================================================================

