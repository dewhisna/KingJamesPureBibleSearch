#include "VerseListModel.h"

#include <QVector>
#include <QModelIndexList>

CVerseListModel::CVerseListModel(QObject *parent)
	:	QAbstractItemModel(parent),
		m_nSearchScopeMode(CKJVSearchCriteria::SSME_WHOLE_BIBLE),
		m_nDisplayMode(VDME_HEADING),
		m_nTreeMode(VTME_LIST),
		m_bShowMissingLeafs(false)
{
}

CVerseListModel::CVerseListModel(const CVerseList &verses, QObject *parent)
	:	QAbstractItemModel(parent),
		m_nSearchScopeMode(CKJVSearchCriteria::SSME_WHOLE_BIBLE),
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
			return (hasExceededDisplayLimit() ? 0 : m_lstVerses.count());
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
	if (!index.isValid()) return QVariant();

	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());
	if (!ndxRel.isSet()) return QVariant();

	if ((ndxRel.chapter() == 0) && (ndxRel.verse() == 0)) {
		if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
			QString strBookText = ndxRel.bookName();
			if (m_nDisplayMode != VDME_HEADING) return strBookText;		// For Rich Text, Let delegate add results so it can be formatted
			int nResults = GetResultsByBook(ndxRel.book());
			if (nResults) strBookText = QString("(%1) ").arg(nResults) + strBookText;
			return strBookText;
		}
		if ((role == Qt::ToolTipRole) ||
			(role == TOOLTIP_PLAINTEXT_ROLE) ||
			(role == TOOLTIP_NOHEADING_ROLE) ||
			(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
			return QString();
		}
		return QVariant();
	}

	if (ndxRel.verse() == 0) {
		if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
			QString strChapterText = ndxRel.bookName() + QString(" %1").arg(ndxRel.chapter());
			if (m_nDisplayMode != VDME_HEADING) return strChapterText;	// For Rich Text, Let delegate add results so it can be formatted
			int nResults = GetResultsByChapter(ndxRel.book(), ndxRel.chapter());
			if (nResults) strChapterText = QString("(%1) ").arg(nResults) + strChapterText;
			return strChapterText;
		}
		if ((role == Qt::ToolTipRole) ||
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

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		switch (m_nDisplayMode) {
			case VDME_HEADING:
				return m_lstVerses.at(nVerse).getHeading();
			case VDME_VERYPLAIN:
				return m_lstVerses.at(nVerse).getVerseVeryPlainText();
			case VDME_RICHTEXT:
				return m_lstVerses.at(nVerse).getVerseRichText();
			case VDME_COMPLETE:
				return m_lstVerses.at(nVerse).getVerseRichText();		// TODO : FINISH THIS ONE!!!
			default:
				return QString();
		}
	}

	if ((role == Qt::ToolTipRole) ||
		(role == TOOLTIP_PLAINTEXT_ROLE) ||
		(role == TOOLTIP_NOHEADING_ROLE) ||
		(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
		const CVerseListItem &refVerse = m_lstVerses[nVerse];
		bool bHeading = ((role != TOOLTIP_NOHEADING_ROLE) && (role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE));
		QString strToolTip;
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "<qt><pre>";
		if (bHeading) strToolTip += refVerse.getHeading() + "\n";
		QPair<int, int> nResultsIndexes = GetResultsIndexes(nVerse);
		if (nResultsIndexes.first != nResultsIndexes.second) {
			strToolTip += QString("%1Search Results %2-%3 of %4 phrase occurrences\n")
									.arg(bHeading ? "    " : "")
									.arg(nResultsIndexes.first)
									.arg(nResultsIndexes.second)
									.arg(GetTotalResultsCount());
		} else {
			assert(nResultsIndexes.first != 0);		// This will assert if the row was beyond those defined in our list
			strToolTip += QString("%1Search Result %2 of %3 phrase occurrences\n")
									.arg(bHeading ? "    " : "")
									.arg(nResultsIndexes.first)
									.arg(GetTotalResultsCount());
		}
		QPair<int, int> nVerseResult = GetVerseIndexAndCount(nVerse);
		strToolTip += QString("%1    Verse %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nVerseResult.first).arg(nVerseResult.second);
		QPair<int, int> nChapterResult = GetChapterIndexAndCount(nVerse);
		strToolTip += QString("%1    Chapter %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nChapterResult.first).arg(nChapterResult.second);
		QPair<int, int> nBookResult = GetBookIndexAndCount(nVerse);
		strToolTip += QString("%1    Book %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nBookResult.first).arg(nBookResult.second);
		strToolTip += refVerse.getToolTip(m_lstParsedPhrases);
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "</pre></qt>";
		return strToolTip;
	}

	if (role == VERSE_ENTRY_ROLE) {
		return QVariant::fromValue(m_lstVerses.at(nVerse));
	}

	return QVariant();
}

bool CVerseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);
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

	if ((role == Qt::ToolTipRole) ||
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

//	CRelIndex ndxRel(index.internalId());
//	assert(ndxRel.isSet());
//	if (ndxRel.verse() == 0) return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

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
	// Both normalized and denormalized are in order, but it's less expensive
	//	 to convert to normal when we already have relative
	return s1.first.getIndexDenormalized() < s2.first.getIndexDenormalized();
}

static bool decendingLessThan(const QPair<CVerseListItem, int> &s1, const QPair<CVerseListItem, int> &s2)
{
	// Both normalized and denormalized are in order, but it's less expensive
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

TPhraseTagList CVerseListModel::setParsedPhrases(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScopeMode, const TParsedPhrasesList &phrases)
{
	// Note: Basic setting of this list doesn't change the model, as the phrases
	//		themselves are used primarily for building of tooltips that are
	//		appropriate for the entire search scope.  However, once these are
	//		set, we'll call the buildVerseListFromParsedPhrases function that
	//		will build and set the VerseList, which will change the model.
	//		Therefore, the beginResetModel/endResetModel calls don't exist here,
	//		but down in setVerseList:
	m_lstParsedPhrases = phrases;
	m_nSearchScopeMode = nSearchScopeMode;
	buildScopedResultsInParsedPhrases();
	return buildVerseListFromParsedPhrases();
}

bool CVerseListModel::hasExceededDisplayLimit() const
{
	if (g_bEnableNoLimits) return false;
	return ((m_lstVerses.size() > g_nSearchLimit) && (m_nDisplayMode != VDME_HEADING));
}

void CVerseListModel::setDisplayMode(VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	if (!hasExceededDisplayLimit()) {
		emit layoutAboutToBeChanged();
		m_nDisplayMode = nDisplayMode;
		emit layoutChanged();
	} else {
		emit beginResetModel();
		m_nDisplayMode = nDisplayMode;
		emit endResetModel();
	}
}

void CVerseListModel::setTreeMode(VERSE_TREE_MODE_ENUM nTreeMode)
{
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

int CVerseListModel::GetTotalResultsCount() const
{
	int nResultsCount = 0;

	for (int ndx = 0; ndx < m_lstVerses.size(); ++ndx) {
		nResultsCount += m_lstVerses.at(ndx).phraseTags().size();
	}

	return nResultsCount;
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
	return (m_bShowMissingLeafs ? g_lstTOC.size() : GetBookIndexAndCount().second);
}

int CVerseListModel::IndexByBook(unsigned int nBk) const
{
	if (m_bShowMissingLeafs) {
		if ((nBk < 1) || (nBk > g_lstTOC.size())) return -1;
		return (nBk-1);
	}

	int nBooks = 0;			// Counts of Books

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (m_lstVerses.at(ndx).getBook() == nBk) return nBooks;
		nBooks++;			// Count the book we are on and skip the ones that are on the same book:
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

	return -1;
}

unsigned int CVerseListModel::BookByIndex(int ndxBook) const
{
	if (m_bShowMissingLeafs) {
		if ((ndxBook < 0) || (static_cast<unsigned int>(ndxBook) >= g_lstTOC.size())) return 0;
		return (ndxBook+1);
	}

	int nBooks = 0;			// Counts of Books

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (nBooks == ndxBook) return m_lstVerses.at(ndx).getBook();
		nBooks++;			// Count the book we are on and skip the ones that are on the same book:
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

	return 0;
}

int CVerseListModel::GetResultsByBook(unsigned int nBk) const
{
	int nResults = 0;
	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (m_lstVerses.at(ndx).getBook() == nBk) nResults++;
	}
	return nResults;
}

int CVerseListModel::GetChapterCount(unsigned int nBk) const
{
	if (nBk == 0) return 0;
	if (m_bShowMissingLeafs) {
		if (nBk > g_lstTOC.size()) return 0;
		return g_lstTOC[nBk-1].m_nNumChp;
	}

	int nChapters = 0;

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (m_lstVerses.at(ndx).getBook() != nBk) continue;
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same book/chapter:
		uint32_t nCurrentChapter = m_lstVerses.at(ndx).getChapter();
		if (ndx < (m_lstVerses.size()-1)) {
			bool bNextIsSameReference=false;
			do {
				if ((nBk == m_lstVerses.at(ndx+1).getBook()) &&
					(nCurrentChapter == m_lstVerses.at(ndx+1).getChapter())) {
					bNextIsSameReference=true;
					ndx++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndx<(m_lstVerses.size()-1)));
		}
	}

	return nChapters;
}

int CVerseListModel::IndexByChapter(unsigned int nBk, unsigned int nChp) const
{
	if ((nBk == 0) || (nChp == 0)) return -1;
	if (m_bShowMissingLeafs) {
		if (nBk > g_lstTOC.size()) return -1;
		if (nChp > g_lstTOC[nBk-1].m_nNumChp) return -1;
		return (nChp-1);
	}

	int nChapters = 0;

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (m_lstVerses.at(ndx).getBook() != nBk) continue;
		if (m_lstVerses.at(ndx).getChapter() == nChp) return nChapters;
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same book/chapter:
		uint32_t nCurrentChapter = m_lstVerses.at(ndx).getChapter();
		if (ndx < (m_lstVerses.size()-1)) {
			bool bNextIsSameReference=false;
			do {
				if ((nBk == m_lstVerses.at(ndx+1).getBook()) &&
					(nCurrentChapter == m_lstVerses.at(ndx+1).getChapter())) {
					bNextIsSameReference=true;
					ndx++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndx<(m_lstVerses.size()-1)));
		}
	}

	return -1;
}

unsigned int CVerseListModel::ChapterByIndex(int ndxBook, int ndxChapter) const
{
	if ((ndxBook < 0) || (ndxChapter < 0)) return 0;
	if (m_bShowMissingLeafs) {
		if (static_cast<unsigned int>(ndxBook) >= g_lstTOC.size()) return 0;
		if (static_cast<unsigned int>(ndxChapter) >= g_lstTOC[ndxBook].m_nNumChp) return 0;
		return (ndxChapter+1);
	}

	int nChapters = 0;
	unsigned int nBk = BookByIndex(ndxBook);

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (m_lstVerses.at(ndx).getBook() != nBk) continue;
		if (ndxChapter == nChapters) return m_lstVerses.at(ndx).getChapter();
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same book/chapter:
		uint32_t nCurrentChapter = m_lstVerses.at(ndx).getChapter();
		if (ndx < (m_lstVerses.size()-1)) {
			bool bNextIsSameReference=false;
			do {
				if ((nBk == m_lstVerses.at(ndx+1).getBook()) &&
					(nCurrentChapter == m_lstVerses.at(ndx+1).getChapter())) {
					bNextIsSameReference=true;
					ndx++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndx<(m_lstVerses.size()-1)));
		}
	}

	return 0;
}

int CVerseListModel::GetResultsByChapter(unsigned int nBk, unsigned int nChp) const
{
	int nResults = 0;
	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if ((m_lstVerses.at(ndx).getBook() == nBk) &&
			(m_lstVerses.at(ndx).getChapter() == nChp)) nResults++;
	}
	return nResults;
}

int CVerseListModel::GetVerseCount(unsigned int nBk, unsigned int nChp) const
{
	if (nBk == 0) return 0;

	int nVerses = 0;

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (m_lstVerses.at(ndx).getBook() != nBk) continue;
		if ((nChp != 0) && (m_lstVerses.at(ndx).getChapter() != nChp)) continue;
		nVerses++;
	}

	return nVerses;
}

int CVerseListModel::GetVerse(int ndxVerse, unsigned int nBk, unsigned int nChp) const
{
	if ((nBk == 0) || (ndxVerse < 0)) return -1;

	int nVerses = 0;

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		if (m_lstVerses.at(ndx).getBook() != nBk) continue;
		if ((nChp != 0) && (m_lstVerses.at(ndx).getChapter() != nChp)) continue;
		if (ndxVerse == nVerses) return ndx;
		nVerses++;
	}

	return -1;
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
			lstScopedRefs[ndx] = ScopeIndex(phrase->GetPhraseTagSearchResults().at(lstNdxStart[ndx]).first, m_nSearchScopeMode);
			for (lstNdxEnd[ndx] = lstNdxStart[ndx]+1; lstNdxEnd[ndx] < phrase->GetPhraseTagSearchResults().size(); ++lstNdxEnd[ndx]) {
				CRelIndex ndxScopedTemp = ScopeIndex(phrase->GetPhraseTagSearchResults().at(lstNdxEnd[ndx]).first, m_nSearchScopeMode);
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
			lstReferences.push_back(CVerseListItem(0, 0));
			continue;
		}
		lstReferences.push_back(CVerseListItem(lstResults.at(ndxResults)));

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

CRelIndex CVerseListModel::ScopeIndex(const CRelIndex &index, CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nMode)
{
	CRelIndex indexScoped;

	switch (nMode) {
		case (CKJVSearchCriteria::SSME_WHOLE_BIBLE):
			// For Whole Bible, we'll set the Book to 1 so that anything in the Bible matches:
			if (index.isSet()) indexScoped = CRelIndex(1, 0, 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_TESTAMENT):
			// For Testament, set the Book to the 1st Book of the corresponding Testament:
			if (index.book()) {
				if (index.book() <= g_lstTOC.size()) {
					const CTOCEntry &toc = g_lstTOC[index.book()-1];
					unsigned int nTestament = toc.m_nTstNdx;
					unsigned int nBook = 1;
					for (unsigned int i=1; i<nTestament; ++i)
						nBook += g_lstTestaments[i-1].m_nNumBk;
					indexScoped = CRelIndex(nBook, 0, 0 ,0);
				}
			}
			break;
		case (CKJVSearchCriteria::SSME_BOOK):
			// For Book, mask off Chapter, Verse, and Word:
			indexScoped = CRelIndex(index.book(), 0, 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_CHAPTER):
			// For Chapter, mask off Verse and Word:
			indexScoped = CRelIndex(index.book(), index.chapter(), 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_VERSE):
			// For Verse, mask off word:
			indexScoped = CRelIndex(index.book(), index.chapter(), index.verse(), 0);
			break;
		default:
			break;
	}

	return indexScoped;
}


