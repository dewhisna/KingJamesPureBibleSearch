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

#include "DictDBListModel.h"

#include "dbstruct.h"
#include "ReadDB.h"
#include "PersistentSettings.h"

// ============================================================================

CDictDatabaseListModel::CDictDatabaseListModel(QObject *parent)
	:	QAbstractListModel(parent)
{
	updateDictDatabaseList();
}

CDictDatabaseListModel::~CDictDatabaseListModel()
{

}

void CDictDatabaseListModel::updateDictDatabaseList()
{
	beginResetModel();
	m_lstAvailableDatabases =  TDictionaryDatabaseList::instance()->availableDictionaryDatabases();
	m_mapAvailableToLoadedIndex.clear();
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		locateLoadedDatabase(ndx);
	}
	endResetModel();
}

void CDictDatabaseListModel::locateLoadedDatabase(int nAvailableDBIndex)
{
	assert((nAvailableDBIndex >= 0) && (nAvailableDBIndex < m_lstAvailableDatabases.size()));
	const TDictionaryDescriptor &dctDesc = dictionaryDescriptor(m_lstAvailableDatabases.at(nAvailableDBIndex));

	bool bFound = false;
	for (int ndxLoaded = 0; ndxLoaded < TDictionaryDatabaseList::instance()->size(); ++ndxLoaded) {
		CDictionaryDatabasePtr pDictDatabase = TDictionaryDatabaseList::instance()->at(ndxLoaded);
		assert(!pDictDatabase.isNull());
		if (pDictDatabase.isNull()) continue;
		if (pDictDatabase->compatibilityUUID().compare(dctDesc.m_strUUID, Qt::CaseInsensitive) == 0) {
			m_mapAvailableToLoadedIndex[nAvailableDBIndex] = ndxLoaded;
			bFound = true;
		}
	}
	if (!bFound) m_mapAvailableToLoadedIndex[nAvailableDBIndex] = -1;
}

int CDictDatabaseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstAvailableDatabases.size();
}

int CDictDatabaseListModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return 2;
}

QVariant CDictDatabaseListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	int ndxDB = index.row();

	if ((ndxDB < 0) || (ndxDB >= m_lstAvailableDatabases.size()))
		return QVariant();

	const TDictionaryDescriptor &dctDesc = dictionaryDescriptor(m_lstAvailableDatabases.at(ndxDB));
	bool bLoadOnStart = CPersistentSettings::instance()->dictionaryDatabaseSettings(dctDesc.m_strUUID).loadOnStart();

	if (index.column() == 0) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole))
			return dctDesc.m_strDBDesc;

		if (role == Qt::CheckStateRole) {
			bool bIsCurrentlyChecked = (bLoadOnStart || (dctDesc.m_dtoFlags & DTO_AutoLoad));
//			bool bIsCurrentlyChecked = (bLoadOnStart || (dctDesc.m_dtoFlags & DTO_AutoLoad) ||
//										((TDictionaryDatabaseList::instance()->haveMainDictionaryDatabase()) &&
//										  (TDictionaryDatabaseList::instance()->mainDictionaryDatabase()->compatibilityUUID().compare(dctDesc.m_strUUID, Qt::CaseInsensitive) == 0)));
			return (bIsCurrentlyChecked ? Qt::Checked : Qt::Unchecked);
		}
	} else if (index.column() == 1) {
		if ((role == Qt::DisplayRole) ||
			(role == Qt::EditRole)) {
			DICTIONARY_DESCRIPTOR_ENUM ddeMainDB = dictionaryDescriptorFromUUID(CPersistentSettings::instance()->mainDictDatabaseUUID());
			if (dctDesc.m_dtoFlags & DTO_AutoLoad) {
				return QString("[%1]").arg(tr("Loaded - Cannot be unloaded", "DictDBStatus"));
			} else if (ddeMainDB == m_lstAvailableDatabases.at(ndxDB)) {
				return QString("[%1]").arg(tr("Loaded - Selected as Initial Database", "DictDBStatus"));
			} else if ((m_mapAvailableToLoadedIndex.value(ndxDB, -1) != -1) && (bLoadOnStart)) {
				return QString("[%1]").arg(tr("Loaded, Auto-Reloaded at startup", "DictDBStatus"));
			} else if (m_mapAvailableToLoadedIndex.value(ndxDB, -1) != -1) {
				return QString("[%1]").arg(tr("Loaded, Will Not Auto-Reload at startup", "DictDBStatus"));
			} else {
				return QString("[%1]").arg(tr("Not Loaded", "DictDBStatus"));
			}
		}
	}

	if (role == DDDRE_DICTIONARY_DESCRIPTOR_ROLE)
		return QVariant::fromValue(m_lstAvailableDatabases.at(ndxDB));

	if (role == DDDRE_DATABASE_POINTER_ROLE) {
		int nDictDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);
		if (nDictDB != -1) {
			return QVariant::fromValue(TDictionaryDatabaseList::instance()->at(nDictDB).data());
		} else {
			return QVariant::fromValue(static_cast<CDictionaryDatabase *>(nullptr));
		}
	}

	if (role == DDDRE_UUID_ROLE) return dctDesc.m_strUUID;

	return QVariant();
}

QVariant CDictDatabaseListModel::data(DICTIONARY_DESCRIPTOR_ENUM nDDE, int role) const
{
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		if (m_lstAvailableDatabases.at(ndx) == nDDE) {
			return data(createIndex(ndx, 0), role);
		}
	}

	return QVariant();
}

bool CDictDatabaseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) return false;

	int ndxDB = index.row();

	if ((ndxDB >= 0) && (ndxDB < m_lstAvailableDatabases.size())) {
		if (role == Qt::CheckStateRole) {
			DICTIONARY_DESCRIPTOR_ENUM ddeMainDB = dictionaryDescriptorFromUUID(CPersistentSettings::instance()->mainDictDatabaseUUID());
			const TDictionaryDescriptor &dctDesc = dictionaryDescriptor(m_lstAvailableDatabases.at(ndxDB));
			int nDictDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);		// Get mapping if it's really loaded
			bool bIsCurrentlyChecked = (CPersistentSettings::instance()->dictionaryDatabaseSettings(dctDesc.m_strUUID).loadOnStart() || (dctDesc.m_dtoFlags & DTO_AutoLoad));
			bool bIsDBLoaded = (nDictDB != -1);
			bool bNewCheck = value.toBool();

			if (bNewCheck) {
				// If checked, make sure database is loaded and indexed:
				if (!bIsDBLoaded) emit loadDictDatabase(m_lstAvailableDatabases.at(ndxDB));
				TDictionaryDatabaseSettings dctDBaseSettings = CPersistentSettings::instance()->dictionaryDatabaseSettings(dctDesc.m_strUUID);
				dctDBaseSettings.setLoadOnStart(true);
				CPersistentSettings::instance()->setDictionaryDatabaseSettings(dctDesc.m_strUUID, dctDBaseSettings);
			} else {
				// "unload" it by unmapping it (unless it's a special autoLoad or our selected initial database):
				if (!(dctDesc.m_dtoFlags & DTO_AutoLoad)) {
					TDictionaryDatabaseSettings dctDBaseSettings = CPersistentSettings::instance()->dictionaryDatabaseSettings(dctDesc.m_strUUID);
					dctDBaseSettings.setLoadOnStart(false);
					CPersistentSettings::instance()->setDictionaryDatabaseSettings(dctDesc.m_strUUID, dctDBaseSettings);
				} else {
					bNewCheck = true;
				}
			}
			locateLoadedDatabase(ndxDB);
			if (bNewCheck != bIsCurrentlyChecked) emit changedAutoLoadStatus(dctDesc.m_strUUID, bNewCheck);

			// Always trigger a dataChange as most likely the status text in the second column is
			//		changing even when the checkbox isn't.  And if our main database is also changing,
			//		then its status text is changes, so trigger it too if it's different:
			emit dataChanged(createIndex(index.row(), 0), createIndex(index.row(), 1));
			for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
				if ((ddeMainDB == m_lstAvailableDatabases.at(ndx)) &&
					(ndx != ndxDB)) {
					emit dataChanged(createIndex(ndx, 0), createIndex(ndx, 1));
				}
			}
			return true;
		}
	}

	return false;
}

bool CDictDatabaseListModel::setData(DICTIONARY_DESCRIPTOR_ENUM nDDE, const QVariant &value, int role)
{
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		if (m_lstAvailableDatabases.at(ndx) == nDDE) {
			return setData(createIndex(ndx, 0), value, role);
		}
	}

	return false;
}

Qt::ItemFlags CDictDatabaseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	int ndxDB = index.row();

	assert((ndxDB >= 0) && (ndxDB < m_lstAvailableDatabases.size()));

	bool bCheckable = (!(dictionaryDescriptor(m_lstAvailableDatabases.at(ndxDB)).m_dtoFlags & DTO_AutoLoad));
	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | (bCheckable ? Qt::ItemIsUserCheckable : Qt::NoItemFlags) | Qt::ItemIsSelectable;
}

// ============================================================================
