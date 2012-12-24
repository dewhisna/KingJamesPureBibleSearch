#include "VerseListModel.h"

#include <QVector>
#include <QModelIndexList>

CVerseListModel::CVerseListModel(QObject *parent)
	:	QAbstractListModel(parent),
		m_nDisplayMode(VDME_HEADING)
{
}

CVerseListModel::CVerseListModel(const CVerseList &verses, VERSE_DISPLAY_MODE_ENUM nDisplayMode, QObject *parent)
	:	QAbstractListModel(parent),
		m_lstVerses(verses),
		m_nDisplayMode(nDisplayMode)
{
}

int CVerseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstVerses.count();
}

QVariant CVerseListModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_lstVerses.size())
		return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		switch (m_nDisplayMode) {
			case VDME_HEADING:
				return m_lstVerses.at(index.row()).getHeading();
			case VDME_VERYPLAIN:
				return m_lstVerses.at(index.row()).getVerseVeryPlainText();
			case VDME_RICHTEXT:
				return m_lstVerses.at(index.row()).getVerseRichText();
			case VDME_COMPLETE:
				return m_lstVerses.at(index.row()).getVerseRichText();		// TODO : FINISH THIS ONE!!!
			default:
				return QString();
		}
	}

	if ((role == Qt::ToolTipRole) ||
		(role == TOOLTIP_PLAINTEXT_ROLE) ||
		(role == TOOLTIP_NOHEADING_ROLE) ||
		(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
		const CVerseListItem &refVerse = m_lstVerses[index.row()];
		bool bHeading = ((role != TOOLTIP_NOHEADING_ROLE) && (role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE));
		QString strToolTip;
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "<qt><pre>";
		if (bHeading) strToolTip += refVerse.getHeading() + "\n";
		QPair<int, int> nResultsIndexes = GetResultsIndexes(index.row());
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
		QPair<int, int> nVerseResult = GetVerseIndexAndCount(index.row());
		strToolTip += QString("%1    Verse %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nVerseResult.first).arg(nVerseResult.second);
		QPair<int, int> nChapterResult = GetChapterIndexAndCount(index.row());
		strToolTip += QString("%1    Chapter %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nChapterResult.first).arg(nChapterResult.second);
		QPair<int, int> nBookResult = GetBookIndexAndCount(index.row());
		strToolTip += QString("%1    Book %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nBookResult.first).arg(nBookResult.second);
		strToolTip += "\n";
		strToolTip += refVerse.getToolTip(m_lstParsedPhrases);
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "</pre></qt>";
		return strToolTip;
	}

	if (role == VERSE_ENTRY_ROLE) {
		return QVariant::fromValue(m_lstVerses.at(index.row()));
	}

	return QVariant();
}

bool CVerseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
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

	return false;
}

Qt::ItemFlags CVerseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool CVerseListModel::insertRows(int row, int count, const QModelIndex &parent)
{
	if (count < 1 || row < 0 || row > rowCount(parent))
		return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.insert(row, CVerseListItem());

	endInsertRows();

	return true;
}

bool CVerseListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
		return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.removeAt(row);

	endRemoveRows();

	return true;
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

CVerseList CVerseListModel::verseList() const
{
	return m_lstVerses;
}

void CVerseListModel::setVerseList(const CVerseList &verses)
{
	emit beginResetModel();
	m_lstVerses = verses;
	emit endResetModel();
}

TParsedPhrasesList CVerseListModel::parsedPhrases() const
{
	return m_lstParsedPhrases;
}

TPhraseTagList CVerseListModel::setParsedPhrases(const TParsedPhrasesList &phrases)
{
	// Note: Basic setting of this list doesn't change the model, as the phrases
	//		themselves are used primarily for building of tooltips that are
	//		appropriate for the entire search scope.  However, once these are
	//		set, we'll call the buildVerseListFromParsedPhrases function that
	//		will build and set the VerseList, which will change the model.
	//		Therefore, the beginResetModel/endResetModel calls don't exist here,
	//		but down in setVerseList:
	m_lstParsedPhrases = phrases;
	return buildVerseListFromParsedPhrases();
}

QPair<int, int> CVerseListModel::GetResultsIndexes(int nRow) const
{
	QPair<int, int> nResultsIndexes;
	nResultsIndexes.first = 0;
	nResultsIndexes.second = 0;

	for (int ndx = 0; ((ndx < nRow) && (ndx < m_lstVerses.size())); ++ndx) {
		nResultsIndexes.first += m_lstVerses.at(ndx).phraseTags().size();
	}
	nResultsIndexes.second = nResultsIndexes.first;
	if (nRow < m_lstVerses.size()) {
		nResultsIndexes.first++;
		nResultsIndexes.second += m_lstVerses.at(nRow).phraseTags().size();
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

QPair<int, int> CVerseListModel::GetBookIndexAndCount(int nRow) const
{
	int ndxBook = 0;		// Index into Books
	int nBooks = 0;			// Results counts in Books

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		nBooks++;			// Count the book we are on and skip the ones that are on the same book:
		if (ndx <= nRow) ndxBook++;
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

QPair<int, int> CVerseListModel::GetChapterIndexAndCount(int nRow) const
{
	int ndxChapter = 0;		// Index into Chapters
	int nChapters = 0;		// Results counts in Chapters

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same book/chapter:
		if (ndx <= nRow) ndxChapter++;
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

QPair<int, int> CVerseListModel::GetVerseIndexAndCount(int nRow) const
{
	return QPair<int, int>(nRow+1, m_lstVerses.size());
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

