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

#include "SearchPhraseListModel.h"

#include <QVector>
#include <QModelIndexList>

#include <algorithm>

CSearchPhraseListModel::CSearchPhraseListModel(QObject *parent) :
	QAbstractListModel(parent)
{
}
int CSearchPhraseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstPhraseEditors.count();
}

QVariant CSearchPhraseListModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_lstPhraseEditors.size())
		return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		const CKJVSearchPhraseEdit *editor = m_lstPhraseEditors[index.row()];
		if (editor) return editor->parsedPhrase()->phrase();
		return QString();
	}

	if (role == Qt::ToolTipRole) {
		const CKJVSearchPhraseEdit *editor = m_lstPhraseEditors[index.row()];
		if (editor) return editor->toolTip();
		return QString();
	}

	if (role == SEARCH_PHRASE_EDIT_WIDGET_ROLE) {
		return QVariant::fromValue(m_lstPhraseEditors.at(index.row()));
	}

	return QVariant();
}

bool CSearchPhraseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.row() < 0 || index.row() >= m_lstPhraseEditors.size()) return false;

	if ((role == Qt::EditRole) || (role == Qt::DisplayRole)) {
		return false;		// read-only
	}

	if (role == Qt::ToolTipRole) {
		return false;				// read-only
	}

	if (role == SEARCH_PHRASE_EDIT_WIDGET_ROLE) {
		m_lstPhraseEditors.replace(index.row(), value.value<CKJVSearchPhraseEdit*>());
		emit dataChanged(index, index);
		return true;
	}

	return false;
}

Qt::ItemFlags CSearchPhraseListModel::flags(const QModelIndex &index) const
{
	return QAbstractItemModel::flags(index);
//	if (!index.isValid())
//		return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
//
//	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool CSearchPhraseListModel::insertRows(int row, int count, const QModelIndex &parent)
{
	if (count < 1 || row < 0 || row > rowCount(parent))
		return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstPhraseEditors.insert(row, nullptr);

	endInsertRows();

	return true;
}

bool CSearchPhraseListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
		return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstPhraseEditors.removeAt(row);

	endRemoveRows();

	return true;
}

static bool ascendingLessThan(const QPair<CKJVSearchPhraseEdit*, int> &s1, const QPair<CKJVSearchPhraseEdit*, int> &s2)
{
//	return false;				// Decide how to compare phrase editors if at all
	return (s1.second < s2.second);
}

static bool decendingLessThan(const QPair<CKJVSearchPhraseEdit*, int> &s1, const QPair<CKJVSearchPhraseEdit*, int> &s2)
{
//	return false;				// Decide how to compare phrase editors if at all
	return (s2.second < s1.second);
}

void CSearchPhraseListModel::sort(int /* column */, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();

	QList<QPair<CKJVSearchPhraseEdit*, int> > list;
	for (int i = 0; i < m_lstPhraseEditors.count(); ++i)
		list.append(QPair<CKJVSearchPhraseEdit*, int>(m_lstPhraseEditors.at(i), i));

	if (order == Qt::AscendingOrder)
		std::sort(list.begin(), list.end(), ascendingLessThan);
	else
		std::sort(list.begin(), list.end(), decendingLessThan);

	m_lstPhraseEditors.clear();
	QVector<int> forwarding(list.count());
	for (int i = 0; i < list.count(); ++i) {
		m_lstPhraseEditors.append(list.at(i).first);
		forwarding[list.at(i).second] = i;
	}

	QModelIndexList oldList = persistentIndexList();
	QModelIndexList newList;
	for (int i = 0; i < oldList.count(); ++i)
		newList.append(index(forwarding.at(oldList.at(i).row()), 0));
	changePersistentIndexList(oldList, newList);

	emit layoutChanged();
}

Qt::DropActions CSearchPhraseListModel::supportedDropActions() const
{
	return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

CSearchPhraseEditList CSearchPhraseListModel::phraseEditorsList() const
{
	return m_lstPhraseEditors;
}

void CSearchPhraseListModel::setPhraseEditorsList(const CSearchPhraseEditList &lstPhraseEditors)
{
	emit beginResetModel();
	m_lstPhraseEditors = lstPhraseEditors;
	emit endResetModel();
}

