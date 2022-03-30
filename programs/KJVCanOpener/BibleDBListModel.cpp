/****************************************************************************
**
** Copyright (C) 2014-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "BibleDBListModel.h"

#include "dbstruct.h"
#include "ReadDB.h"
#include "PersistentSettings.h"

// ============================================================================

CBibleDatabaseListModel::CBibleDatabaseListModel(QObject *parent)
	:	QAbstractListModel(parent),
		m_lstAvailableDatabaseDescriptors(TBibleDatabaseList::availableBibleDatabases())
{
	updateBibleDatabaseList();
}

CBibleDatabaseListModel::~CBibleDatabaseListModel()
{

}

void CBibleDatabaseListModel::updateBibleDatabaseList()
{
	beginResetModel();
	m_mapAvailableToLoadedIndex.clear();
	for (int ndx = 0; ndx < m_lstAvailableDatabaseDescriptors.size(); ++ndx) {
		locateLoadedDatabase(ndx);
	}
	endResetModel();
}

void CBibleDatabaseListModel::locateLoadedDatabase(int nAvailableDBIndex)
{
	Q_ASSERT((nAvailableDBIndex >= 0) && (nAvailableDBIndex < m_lstAvailableDatabaseDescriptors.size()));

	bool bFound = false;
	for (int ndxLoaded = 0; ndxLoaded < TBibleDatabaseList::instance()->size(); ++ndxLoaded) {
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->at(ndxLoaded);
		Q_ASSERT(!pBibleDatabase.isNull());
		if (pBibleDatabase.isNull()) continue;
		if (pBibleDatabase->compatibilityUUID().compare(m_lstAvailableDatabaseDescriptors.at(nAvailableDBIndex).m_strUUID, Qt::CaseInsensitive) == 0) {
			m_mapAvailableToLoadedIndex[nAvailableDBIndex] = ndxLoaded;
			bFound = true;
		}
	}
	if (!bFound) m_mapAvailableToLoadedIndex[nAvailableDBIndex] = -1;
}

int CBibleDatabaseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstAvailableDatabaseDescriptors.size();
}

int CBibleDatabaseListModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return 2;
}

QVariant CBibleDatabaseListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	int ndxDB = index.row();

	if ((ndxDB < 0) || (ndxDB >= m_lstAvailableDatabaseDescriptors.size()))
		return QVariant();

	const TBibleDescriptor &bblDesc = m_lstAvailableDatabaseDescriptors.at(ndxDB);
	bool bLoadOnStart = CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID).loadOnStart();

	if (index.column() == 0) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole))
			return bblDesc.m_strDBDesc;

		if (role == Qt::CheckStateRole) {
			bool bIsCurrentlyChecked = (bLoadOnStart || (bblDesc.m_btoFlags & BTO_AutoLoad));
			return (bIsCurrentlyChecked ? Qt::Checked : Qt::Unchecked);
		}
	} else if (index.column() == 1) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole)) {
			QString strMainDBUUID = CPersistentSettings::instance()->mainBibleDatabaseUUID();
			QString strText;
			if (bblDesc.m_btoFlags & BTO_AutoLoad) {
				strText = tr("Loaded - Cannot be unloaded", "BibleDBStatus");
			} else if (strMainDBUUID.compare(bblDesc.m_strUUID, Qt::CaseInsensitive) == 0) {
				strText = tr("Loaded - Selected as Initial Database", "BibleDBStatus");
			} else if ((m_mapAvailableToLoadedIndex.value(ndxDB, -1) != -1) && (bLoadOnStart)) {
				strText = tr("Loaded, Auto-Reloaded at startup", "BibleDBStatus");
			} else if (m_mapAvailableToLoadedIndex.value(ndxDB, -1) != -1) {
				strText = tr("Loaded, Will Not Auto-Reload at startup", "BibleDBStatus");
			} else {
				strText = tr("Not Loaded", "BibleDBStatus");
			}
			if (bblDesc.m_btoFlags & BTO_Discovered) strText += QString(", %1").arg(tr("Auto-Discovered", "BibleDBStatus"));
			return QString("[%1]").arg(strText);
		}
	}

	if (role == BDDRE_DATABASE_POINTER_ROLE) {
		int nBibleDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);
		if (nBibleDB != -1) {
			return QVariant::fromValue(TBibleDatabaseList::instance()->at(nBibleDB).data());
		} else {
			return QVariant::fromValue(static_cast<CBibleDatabase *>(nullptr));
		}
	}

	if (role == BDDRE_UUID_ROLE) return bblDesc.m_strUUID;

	return QVariant();
}

QVariant CBibleDatabaseListModel::data(const QString &strUUID, int role) const
{
	for (int ndx = 0; ndx < m_lstAvailableDatabaseDescriptors.size(); ++ndx) {
		if (strUUID.compare(m_lstAvailableDatabaseDescriptors.at(ndx).m_strUUID, Qt::CaseInsensitive) == 0) {
			return data(createIndex(ndx, 0), role);
		}
	}

	return QVariant();
}

bool CBibleDatabaseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) return false;

	int ndxDB = index.row();

	if ((ndxDB >= 0) && (ndxDB < m_lstAvailableDatabaseDescriptors.size())) {
		if (role == Qt::CheckStateRole) {
			QString strMainDBUUID = CPersistentSettings::instance()->mainBibleDatabaseUUID();
			const TBibleDescriptor &bblDesc = m_lstAvailableDatabaseDescriptors.at(ndxDB);
			int nBibleDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);		// Get mapping if it's really loaded
			bool bIsCurrentlyChecked = (CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID).loadOnStart() || (bblDesc.m_btoFlags & BTO_AutoLoad));
			bool bIsDBLoaded = (nBibleDB != -1);
			bool bNewCheck = value.toBool();

			if (bNewCheck) {
				// If checked, make sure database is loaded and indexed:
				if (!bIsDBLoaded) emit loadBibleDatabase(bblDesc.m_strUUID);
				TBibleDatabaseSettings bblDBaseSettings = CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID);
				bblDBaseSettings.setLoadOnStart(true);
				CPersistentSettings::instance()->setBibleDatabaseSettings(bblDesc.m_strUUID, bblDBaseSettings);
				locateLoadedDatabase(ndxDB);
			} else {
				// "unload" it by unmapping it (unless it's a special autoLoad or our selected initial database):
				if ((!(bblDesc.m_btoFlags & BTO_AutoLoad)) && (strMainDBUUID.compare(bblDesc.m_strUUID, Qt::CaseInsensitive) != 0)) {
					TBibleDatabaseSettings bblDBaseSettings = CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID);
					bblDBaseSettings.setLoadOnStart(false);
					CPersistentSettings::instance()->setBibleDatabaseSettings(bblDesc.m_strUUID, bblDBaseSettings);
				} else {
					bNewCheck = true;
				}
			}
			if (bNewCheck != bIsCurrentlyChecked) emit changedAutoLoadStatus(bblDesc.m_strUUID, bNewCheck);

			// Always trigger a dataChange as most likely the status text in the second column is
			//		changing even when the checkbox isn't.  And if our main database is also changing,
			//		then its status text is changing, so trigger it too if it's different:
			emit dataChanged(createIndex(index.row(), 0), createIndex(index.row(), 1));
			for (int ndx = 0; ndx < m_lstAvailableDatabaseDescriptors.size(); ++ndx) {
				if ((strMainDBUUID.compare(m_lstAvailableDatabaseDescriptors.at(ndx).m_strUUID, Qt::CaseInsensitive) == 0) &&
					(ndx != ndxDB)) {
					emit dataChanged(createIndex(ndx, 0), createIndex(ndx, 1));
				}
			}
			return true;
		}
	}

	return false;
}

bool CBibleDatabaseListModel::setData(const QString &strUUID, const QVariant &value, int role)
{
	for (int ndx = 0; ndx < m_lstAvailableDatabaseDescriptors.size(); ++ndx) {
		if (strUUID.compare(m_lstAvailableDatabaseDescriptors.at(ndx).m_strUUID, Qt::CaseInsensitive) == 0) {
			return setData(createIndex(ndx, 0), value, role);
		}
	}

	return false;
}

Qt::ItemFlags CBibleDatabaseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	int ndxDB = index.row();

	Q_ASSERT((ndxDB >= 0) && (ndxDB < m_lstAvailableDatabaseDescriptors.size()));

	QString strMainDBUUID = CPersistentSettings::instance()->mainBibleDatabaseUUID();
	bool bCheckable = ((strMainDBUUID.compare(m_lstAvailableDatabaseDescriptors.at(ndxDB).m_strUUID, Qt::CaseInsensitive) != 0) &&
					   (!(m_lstAvailableDatabaseDescriptors.at(ndxDB).m_btoFlags & BTO_AutoLoad)));

	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | (bCheckable ? Qt::ItemIsUserCheckable : Qt::NoItemFlags) | Qt::ItemIsSelectable;
}

// ============================================================================
