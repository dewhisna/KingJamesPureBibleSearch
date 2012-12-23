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
		QString strToolTip;
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "<qt><pre>";
		if ((role != TOOLTIP_NOHEADING_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += refVerse.getHeading() + "\n\n";
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

void CVerseListModel::setParsedPhrases(const TParsedPhrasesList &phrases)
{
	// Note: This doesn't change the basic model list. It's only used to provide
	//		additional lookup data for the model objects for building of tooltips
	//		that are appropriate for the entire search scope
	m_lstParsedPhrases = phrases;
}

