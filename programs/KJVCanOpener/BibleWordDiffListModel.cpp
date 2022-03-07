/****************************************************************************
**
** Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#include "BibleWordDiffListModel.h"

#include "PersistentSettings.h"

// ============================================================================

CBibleWordDiffListModel::CBibleWordDiffListModel(CBibleDatabasePtr pBibleDatabase, QObject *parent)
	:	QAbstractListModel(parent)
{
	setBibleDatabase(pBibleDatabase);

	connect(TBibleDatabaseList::instance(), SIGNAL(endChangeBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &, const TBibleDatabaseSettings &, bool)), this, SLOT(en_endChangeBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &, const TBibleDatabaseSettings &, bool)));
}

CBibleWordDiffListModel::~CBibleWordDiffListModel()
{

}

void CBibleWordDiffListModel::setBibleDatabase(CBibleDatabasePtr pBibleDatabase)
{
	beginResetModel();

	m_lstWords.clear();
	m_pBibleDatabase = pBibleDatabase;
	if (!pBibleDatabase.isNull()) {
		const TConcordanceList &lstConcordance = pBibleDatabase->concordanceWordList();
		if (m_pBibleDatabase->searchSpaceIsCompleteConcordance()) {
			for (TConcordanceList::const_iterator itr = lstConcordance.constBegin(); itr != lstConcordance.constEnd(); ++itr) {
				if (itr->renderedWord().compare(itr->word()) != 0)
					m_lstWords.append(*itr);
			}
		} else {
			TWordListSet setSearchWords;
			for (TConcordanceList::const_iterator itr = lstConcordance.constBegin(); itr != lstConcordance.constEnd(); ++itr) {
				if (itr->renderedWord().compare(itr->word()) != 0) {
					if (setSearchWords.find(itr->searchWord()) == setSearchWords.cend()) {
						setSearchWords.insert(itr->searchWord());
						m_lstWords.append(*itr);
					}
				}
			}
		}
	}

	endResetModel();
}

int CBibleWordDiffListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstWords.size();
}

int CBibleWordDiffListModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return 2;
}

QVariant CBibleWordDiffListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	int ndxWord = index.row();

	if ((ndxWord < 0) || (ndxWord >= m_lstWords.size()))
		return QVariant();

	if (index.column() == 0) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole)) {
			return m_lstWords.at(ndxWord).word();

		}
	} else if (index.column() == 1) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole)) {
			return m_lstWords.at(ndxWord).renderedWord();
		}
	}

	return QVariant();

}

bool CBibleWordDiffListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);
	return false;
}

QVariant CBibleWordDiffListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			switch (section) {
				case 0:
					return tr("Original Word", "CBibleWordDiffListModel");
				case 1:
					return tr("Rendered Word", "CBibleWordDiffListModel");
				default:
					break;
			}
		}
	}

	return QVariant();
}

Qt::ItemFlags CBibleWordDiffListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	int ndxWord = index.row();
	Q_ASSERT((ndxWord >= 0) && (ndxWord < m_lstWords.size()));

	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable;
}

void CBibleWordDiffListModel::en_endChangeBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &oldSettings,
																const TBibleDatabaseSettings &newSettings, bool bForce)
{
	Q_UNUSED(oldSettings);
	Q_UNUSED(newSettings);
	Q_UNUSED(bForce);

	if (!m_pBibleDatabase.isNull()) {
		if (m_pBibleDatabase->compatibilityUUID().compare(strUUID, Qt::CaseInsensitive) == 0) {
			setBibleDatabase(m_pBibleDatabase);		// Reset the Bible Database again to update the list, since it will be changing
		}
	}
}

// ============================================================================
