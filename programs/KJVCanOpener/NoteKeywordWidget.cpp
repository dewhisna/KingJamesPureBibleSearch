/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "NoteKeywordWidget.h"
#include "ui_NoteKeywordWidget.h"

#include "SearchCompleter.h"

// ============================================================================

CNoteKeywordModel::CNoteKeywordModel(QObject *pParent)
	:	QAbstractListModel(pParent),
		m_pActionSelectAllKeywords(NULL),
		m_pActionClearKeywordSelection(NULL)

{
	m_pActionSelectAllKeywords = m_keywordContextMenu.addAction(tr("Select &All"), this, SLOT(en_selectAllKeywords()));
	m_pActionSelectAllKeywords->setStatusTip(tr("Select all keywords"));
	m_pActionSelectAllKeywords->setEnabled(false);
	m_pActionClearKeywordSelection = m_keywordContextMenu.addAction(tr("&Clear Selection"), this, SLOT(en_clearKeywordSelection()));
	m_pActionClearKeywordSelection->setStatusTip(tr("Clear keyword selection"));
	m_pActionClearKeywordSelection->setEnabled(false);
}

CNoteKeywordModel::~CNoteKeywordModel()
{

}

int CNoteKeywordModel::rowCount(const QModelIndex &zParent) const
{
	if (zParent.isValid())
		return 0;

	return m_lstKeywordData.size();
}

QVariant CNoteKeywordModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 || index.row() >= m_lstKeywordData.size()) return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
		return m_lstKeywordData.at(index.row()).m_strKeyword;

	if (role == Qt::CheckStateRole)
		return (m_lstKeywordData.at(index.row()).m_bChecked ? Qt::Checked : Qt::Unchecked);

	return QVariant();
}

bool CNoteKeywordModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) {
		// Special "do all" cases:
		if (role == Qt::CheckStateRole) {
			for (int n = 0; n < m_lstKeywordData.size(); ++n) {
				m_lstKeywordData[n].m_bChecked = value.toBool();
			}
			emit dataChanged(createIndex(0, 0, 0), createIndex(m_lstKeywordData.size()-1, 0, 0));
			emit changedNoteKeywords();
			updateContextMenu();
			return true;
		}
		return false;
	}

	if ((index.row() >= 0) && (index.row() < m_lstKeywordData.size())) {
		switch (role) {
			case Qt::EditRole:
			case Qt::DisplayRole:
			{
				QString strDecompNewKeyword = CSearchStringListModel::decompose(value.toString());
				QString strDecompOldKeyword = CSearchStringListModel::decompose(m_lstKeywordData.at(index.row()).m_strKeyword);
				if (strDecompOldKeyword.compare(strDecompNewKeyword, Qt::CaseInsensitive) != 0) {
					m_lstKeywordData[index.row()].m_strKeyword = value.toString();
					emit dataChanged(index, index);
					emit changedNoteKeywords();
				}
				return true;
			}
			case Qt::CheckStateRole:
				if (m_lstKeywordData.at(index.row()).m_bChecked != value.toBool()) {
					m_lstKeywordData[index.row()].m_bChecked = value.toBool();
					emit dataChanged(index, index);
					emit changedNoteKeywords();
					updateContextMenu();
				}
				return true;
			default:
				break;
		}
	}

	return false;
}

QModelIndex CNoteKeywordModel::findKeyword(const QString &strKeyword) const
{
	QString strDecomposedKeyword = CSearchStringListModel::decompose(strKeyword);
	int ndxFound = -1;
	for (int ndx = 0; ndx < m_lstKeywordData.size(); ++ndx) {
		if (m_lstKeywordData.at(ndx).m_strKeyword.compare(strDecomposedKeyword, Qt::CaseInsensitive) == 0) {
			ndxFound = ndx;
			break;
		}
	}

	if (ndxFound == -1) return QModelIndex();
	return index(ndxFound);
}

Qt::ItemFlags CNoteKeywordModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;	// | Qt::ItemIsSelectable;
}

bool CNoteKeywordModel::insertRows(int row, int count, const QModelIndex &zParent)
{
	if ((count < 1) || (row < 0) || (row > rowCount(zParent))) return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstKeywordData.insert(row, CNoteKeywordModelItemData());

	endInsertRows();

	updateContextMenu();

	return true;
}

bool CNoteKeywordModel::removeRows(int row, int count, const QModelIndex &zParent)
{
	if ((count <= 0) || (row < 0) || ((row + count) > rowCount(zParent))) return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstKeywordData.removeAt(row);

	endRemoveRows();

	updateContextMenu();

	return true;
}

static bool ascendingLessThan(const QPair<CNoteKeywordModelItemData, int> &s1, const QPair<CNoteKeywordModelItemData, int> &s2)
{
	return (CSearchStringListModel::decompose(s1.first.m_strKeyword).compare(CSearchStringListModel::decompose(s2.first.m_strKeyword), Qt::CaseInsensitive) < 0);
}

static bool decendingLessThan(const QPair<CNoteKeywordModelItemData, int> &s1, const QPair<CNoteKeywordModelItemData, int> &s2)
{
	return (CSearchStringListModel::decompose(s2.first.m_strKeyword).compare(CSearchStringListModel::decompose(s1.first.m_strKeyword), Qt::CaseInsensitive) < 0);
}

void CNoteKeywordModel::sort(int /* column */, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();

	QList<QPair<CNoteKeywordModelItemData, int> > list;
	list.reserve(m_lstKeywordData.size());
	for (int i = 0; i < m_lstKeywordData.size(); ++i)
		list.append(QPair<CNoteKeywordModelItemData, int>(m_lstKeywordData.at(i), i));

	if (order == Qt::AscendingOrder)
		qSort(list.begin(), list.end(), ascendingLessThan);
	else
		qSort(list.begin(), list.end(), decendingLessThan);

	m_lstKeywordData.clear();
	m_lstKeywordData.reserve(list.size());
	QVector<int> forwarding(list.size());
	for (int i = 0; i < list.size(); ++i) {
		m_lstKeywordData.append(list.at(i).first);
		forwarding[list.at(i).second] = i;
	}

	QModelIndexList oldList = persistentIndexList();
	QModelIndexList newList;
	newList.reserve(oldList.size());
	for (int i = 0; i < oldList.size(); ++i)
		newList.append(index(forwarding.at(oldList.at(i).row()), 0));
	changePersistentIndexList(oldList, newList);

	emit layoutChanged();
}

const CNoteKeywordModelItemDataList &CNoteKeywordModel::itemList() const
{
	return m_lstKeywordData;
}

void CNoteKeywordModel::setItemList(const CNoteKeywordModelItemDataList &aList)
{
	emit beginResetModel();
	m_lstKeywordData = aList;
	emit endResetModel();
	updateContextMenu();
}

QStringList CNoteKeywordModel::selectedKeywordList() const
{
	QStringList lstKeywords;
	lstKeywords.reserve(m_lstKeywordData.size());

	for (int ndx = 0; ndx < m_lstKeywordData.size(); ++ndx) {
		if (m_lstKeywordData.at(ndx).m_bChecked)
			lstKeywords.append(m_lstKeywordData.at(ndx).m_strKeyword);
	}

	return lstKeywords;
}

void CNoteKeywordModel::setKeywordList(const QStringList &lstSelectedKeywords, const QStringList &lstCompositeKeywords)
{
	emit beginResetModel();

	m_lstKeywordData.clear();
	m_lstKeywordData.reserve(lstCompositeKeywords.size());
	for (int ndx = 0; ndx < lstCompositeKeywords.size(); ++ndx) {
		m_lstKeywordData.append(CNoteKeywordModelItemData(lstCompositeKeywords.at(ndx), lstSelectedKeywords.contains(lstCompositeKeywords.at(ndx), Qt::CaseInsensitive)));
	}

	emit endResetModel();
	updateContextMenu();
}

/*
Qt::DropActions CNoteKeywordModel::supportedDropActions() const
{

}
*/

// ----------------------------------------------------------------------------

void CNoteKeywordModel::en_selectAllKeywords()
{
	setData(QModelIndex(), Qt::Checked, Qt::CheckStateRole);		// Special-case index for check-all
}

void CNoteKeywordModel::en_clearKeywordSelection()
{
	setData(QModelIndex(), Qt::Unchecked, Qt::CheckStateRole);		// Special-case index for uncheck-all
}

void CNoteKeywordModel::updateContextMenu()
{
	bool bAllSelected = true;
	bool bNoneSelected = true;
	for (int ndx = 0; ndx < m_lstKeywordData.size(); ++ndx) {
		if (!m_lstKeywordData.at(ndx).m_bChecked) {
			bAllSelected = false;
		} else {
			bNoneSelected = false;
		}
	}

	m_pActionSelectAllKeywords->setEnabled(!bAllSelected);
	m_pActionClearKeywordSelection->setEnabled(!bNoneSelected);
}

// ============================================================================

CNoteKeywordWidget::CNoteKeywordWidget(QWidget *parent)
	:	QWidget(parent),
		m_pKeywordModel(NULL),
		ui(new Ui::CNoteKeywordWidget),
		m_bDoingUpdate(false)
{
	ui->setupUi(this);

	CNoteKeywordModelListView *pKeywordView = new CNoteKeywordModelListView(ui->comboKeywords);
	pKeywordView->setSelectionMode(QAbstractItemView::SingleSelection);
	pKeywordView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->comboKeywords->setView(pKeywordView);
	connect(pKeywordView, SIGNAL(currentKeywordChanged(const QString &)), this, SLOT(en_keywordCurrentIndexChanged(const QString &)));
//	connect(ui->comboKeywords, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(en_keywordCurrentIndexChanged(const QString &)));
	connect(ui->comboKeywords, SIGNAL(enterPressed()), this, SLOT(en_keywordEntered()));

	// Setup Keywords:
	m_pKeywordModel = new CNoteKeywordModel(ui->comboKeywords);		// Parent it to the comboBox so that it will get auto-deleted when we set a new model
	ui->comboKeywords->setInsertPolicy(QComboBox::NoInsert);		// No auto-insert as we need to parse and decide how to insert it
	ui->comboKeywords->setModel(m_pKeywordModel);
	ui->comboKeywords->clearEditText();
	ui->comboKeywords->setContextMenuPolicy(Qt::CustomContextMenu);
	setKeywordListPreview();

	connect(m_pKeywordModel, SIGNAL(changedNoteKeywords()), this, SLOT(en_keywordListChanged()));
	connect(ui->comboKeywords, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_customContextMenuRequested(const QPoint &)));
	connect(pKeywordView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_customContextMenuRequestedView(const QPoint &)));
}

CNoteKeywordWidget::~CNoteKeywordWidget()
{
	delete ui;
}

// ----------------------------------------------------------------------------

QStringList CNoteKeywordWidget::selectedKeywordList() const
{
	return m_pKeywordModel->selectedKeywordList();
}

void CNoteKeywordWidget::setKeywordList(const QStringList &lstSelectedKeywords, const QStringList &lstCompositeKeywords)
{
	assert(!m_bDoingUpdate);
	m_bDoingUpdate = true;
	m_pKeywordModel->setKeywordList(lstSelectedKeywords, lstCompositeKeywords);
	m_bDoingUpdate = false;
	setKeywordListPreview();
	emit keywordListChanged();
}

// ----------------------------------------------------------------------------

void CNoteKeywordWidget::en_keywordEntered()
{
	if (m_bDoingUpdate) return;

	m_bDoingUpdate = true;

	bool bDataChanged = false;
	QStringList lstNewKeywords = ui->comboKeywords->currentText().split(QChar(','), QString::SkipEmptyParts);
	for (int ndx = 0; ndx < lstNewKeywords.size(); ++ndx) {
		QString strNewKeyword = lstNewKeywords.at(ndx).trimmed();

		if (!strNewKeyword.isEmpty()) {
			QModelIndex index = m_pKeywordModel->findKeyword(strNewKeyword);
			if (!index.isValid()) {
				int ndx = m_pKeywordModel->rowCount();
				bool bSuccess;
				bSuccess = m_pKeywordModel->insertRow(ndx);
				assert(bSuccess);
				QModelIndex index = m_pKeywordModel->index(ndx);
				bSuccess = m_pKeywordModel->setData(index, strNewKeyword, Qt::EditRole);
				assert(bSuccess);
				bSuccess = m_pKeywordModel->setData(index, Qt::Checked, Qt::CheckStateRole);
				assert(bSuccess);
				bDataChanged = true;
			} else {
				if (!index.data(Qt::CheckStateRole).toBool()) {
					m_pKeywordModel->setData(index, Qt::Checked, Qt::CheckStateRole);
					bDataChanged = true;
				}
			}
		}
	}
	if (bDataChanged) {
		m_pKeywordModel->sort(0);
		setKeywordListPreview();
		emit keywordListChanged();
	}

	ui->comboKeywords->setEditText(QString());

	m_bDoingUpdate = false;
}

void CNoteKeywordWidget::en_keywordListChanged()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	setKeywordListPreview();
	emit keywordListChanged();

	m_bDoingUpdate = false;
}

void CNoteKeywordWidget::setKeywordListPreview()
{
	ui->lblKeywordsPreview->setText(m_pKeywordModel->selectedKeywordList().join(", "));
}

void CNoteKeywordWidget::en_keywordCurrentIndexChanged(const QString &text)
{
	ui->comboKeywords->setEditText(text);
	ui->comboKeywords->hidePopup();
}

// ----------------------------------------------------------------------------

void CNoteKeywordWidget::en_customContextMenuRequested(const QPoint &pos)
{
	m_pKeywordModel->contextMenu()->exec(ui->comboKeywords->mapToGlobal(pos));
}

void CNoteKeywordWidget::en_customContextMenuRequestedView(const QPoint &pos)
{
	m_pKeywordModel->contextMenu()->exec(ui->comboKeywords->view()->mapToGlobal(pos));
}

// ============================================================================
