/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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
	:	QAbstractListModel(parent)
{
	updateBibleDatabaseList();
}

CBibleDatabaseListModel::~CBibleDatabaseListModel()
{

}

void CBibleDatabaseListModel::updateBibleDatabaseList()
{
	beginResetModel();
	m_lstAvailableDatabases = TBibleDatabaseList::instance()->availableBibleDatabases();
	m_mapAvailableToLoadedIndex.clear();
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		locateLoadedDatabase(ndx);
	}
	endResetModel();
}

void CBibleDatabaseListModel::locateLoadedDatabase(int nAvailableDBIndex)
{
	assert((nAvailableDBIndex >= 0) && (nAvailableDBIndex < m_lstAvailableDatabases.size()));
	const TBibleDescriptor &bblDesc = bibleDescriptor(m_lstAvailableDatabases.at(nAvailableDBIndex));

	bool bFound = false;
	for (int ndxLoaded = 0; ndxLoaded < TBibleDatabaseList::instance()->size(); ++ndxLoaded) {
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->at(ndxLoaded);
		assert(pBibleDatabase.data() != NULL);
		if (pBibleDatabase.data() == NULL) continue;
		if (pBibleDatabase->compatibilityUUID().compare(bblDesc.m_strUUID, Qt::CaseInsensitive) == 0) {
			m_mapAvailableToLoadedIndex[nAvailableDBIndex] = ndxLoaded;
			bFound = true;
		}
	}
	if (!bFound) m_mapAvailableToLoadedIndex[nAvailableDBIndex] = -1;
}

int CBibleDatabaseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstAvailableDatabases.size();
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

	if ((ndxDB < 0) || (ndxDB >= m_lstAvailableDatabases.size()))
		return QVariant();

	const TBibleDescriptor &bblDesc = bibleDescriptor(m_lstAvailableDatabases.at(ndxDB));
	bool bLoadOnStart = CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID).loadOnStart();

	if (index.column() == 0) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole))
			return bblDesc.m_strDBDesc;

		if (role == Qt::CheckStateRole) {
			bool bIsCurrentlyChecked = (bLoadOnStart || bblDesc.m_bAutoLoad);
			return (bIsCurrentlyChecked ? Qt::Checked : Qt::Unchecked);
		}
	} else if (index.column() == 1) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole)) {
			BIBLE_DESCRIPTOR_ENUM bdeMainDB = bibleDescriptorFromUUID(CPersistentSettings::instance()->mainBibleDatabaseUUID());
			if (bblDesc.m_bAutoLoad) {
				return QString("[%1]").arg(tr("Loaded - Cannot be unloaded"));
			} else if (bdeMainDB == m_lstAvailableDatabases.at(ndxDB)) {
				return QString("[%1]").arg(tr("Loaded - Selected as Initial Database"));
			} else if ((m_mapAvailableToLoadedIndex.value(ndxDB, -1) != -1) && (bLoadOnStart)) {
				return QString("[%1]").arg(tr("Loaded, Auto-Reloaded at startup"));
			} else if (m_mapAvailableToLoadedIndex.value(ndxDB, -1) != -1) {
				return QString("[%1]").arg(tr("Loaded, Will Not Auto-Reload at startup"));
			} else {
				return QString("[%1]").arg(tr("Not Loaded"));
			}
		}
	}

	if (role == BDDRE_BIBLE_DESCRIPTOR_ROLE)
		return QVariant::fromValue(m_lstAvailableDatabases.at(ndxDB));

	if (role == BDDRE_DATABASE_POINTER_ROLE) {
		int nBibleDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);
		if (nBibleDB != -1) {
			return QVariant::fromValue(TBibleDatabaseList::instance()->at(nBibleDB).data());
		} else {
			return QVariant::fromValue(static_cast<CBibleDatabase *>(NULL));
		}
	}

	if (role == BDDRE_UUID_ROLE) return bblDesc.m_strUUID;

	return QVariant();
}

QVariant CBibleDatabaseListModel::data(BIBLE_DESCRIPTOR_ENUM nBDE, int role) const
{
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		if (m_lstAvailableDatabases.at(ndx) == nBDE) {
			return data(createIndex(ndx, 0), role);
		}
	}

	return QVariant();
}

bool CBibleDatabaseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) return false;

	int ndxDB = index.row();

	if ((ndxDB >= 0) && (ndxDB < m_lstAvailableDatabases.size())) {
		if (role == Qt::CheckStateRole) {
			BIBLE_DESCRIPTOR_ENUM bdeMainDB = bibleDescriptorFromUUID(CPersistentSettings::instance()->mainBibleDatabaseUUID());
			const TBibleDescriptor &bblDesc = bibleDescriptor(m_lstAvailableDatabases.at(ndxDB));
			int nBibleDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);		// Get mapping if it's really loaded
			bool bIsCurrentlyChecked = (CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID).loadOnStart() || bblDesc.m_bAutoLoad);
			bool bIsDBLoaded = (nBibleDB != -1);
			bool bNewCheck = value.toBool();

			if (bNewCheck) {
				// If checked, make sure database is loaded and indexed:
				if (!bIsDBLoaded) emit loadBibleDatabase(m_lstAvailableDatabases.at(ndxDB));
				TBibleDatabaseSettings bblDBaseSettings = CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID);
				bblDBaseSettings.setLoadOnStart(true);
				CPersistentSettings::instance()->setBibleDatabaseSettings(bblDesc.m_strUUID, bblDBaseSettings);
				locateLoadedDatabase(ndxDB);
			} else {
				// "unload" it by unmapping it (unless it's a special autoLoad or our selected initial database):
				if ((!bblDesc.m_bAutoLoad) && (bdeMainDB != m_lstAvailableDatabases.at(ndxDB))) {
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
			//		then its status text is changes, so trigger it too if it's different:
			emit dataChanged(createIndex(index.row(), 0), createIndex(index.row(), 1));
			for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
				if ((bdeMainDB == m_lstAvailableDatabases.at(ndx)) &&
					(ndx != ndxDB)) {
					emit dataChanged(createIndex(ndx, 0), createIndex(ndx, 1));
				}
			}
			return true;
		}
	}

	return false;
}

bool CBibleDatabaseListModel::setData(BIBLE_DESCRIPTOR_ENUM nBDE, const QVariant &value, int role)
{
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		if (m_lstAvailableDatabases.at(ndx) == nBDE) {
			return setData(createIndex(ndx, 0), value, role);
		}
	}

	return false;
}

Qt::ItemFlags CBibleDatabaseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	int ndxDB = index.row();

	assert((ndxDB >= 0) && (ndxDB < m_lstAvailableDatabases.size()));

	BIBLE_DESCRIPTOR_ENUM bdeMainDB = bibleDescriptorFromUUID(CPersistentSettings::instance()->mainBibleDatabaseUUID());
	bool bCheckable = ((bdeMainDB != m_lstAvailableDatabases.at(ndxDB)) && (!bibleDescriptor(m_lstAvailableDatabases.at(ndxDB)).m_bAutoLoad));

	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | (bCheckable ? Qt::ItemIsUserCheckable : Qt::NoItemFlags) | Qt::ItemIsSelectable;
}

// ============================================================================
