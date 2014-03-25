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
	m_lstAvailableDatabases.clear();
	m_mapAvailableToLoadedIndex.clear();
	for (unsigned int dbNdx = 0; dbNdx < bibleDescriptorCount(); ++dbNdx) {
		const TBibleDescriptor &bblDesc = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx));
		CReadDatabase rdbMain(g_strBibleDatabasePath, g_strDictionaryDatabasePath);
		if (!rdbMain.haveBibleDatabaseFiles(bblDesc)) continue;
		int ndxCurrent = m_lstAvailableDatabases.size();
		m_lstAvailableDatabases.append(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx));
		locateLoadedDatabase(ndxCurrent);
	}
	endResetModel();
}

void CBibleDatabaseListModel::locateLoadedDatabase(int nAvailableDBIndex)
{
	assert((nAvailableDBIndex >= 0) && (nAvailableDBIndex < m_lstAvailableDatabases.size()));
	const TBibleDescriptor &bblDesc = bibleDescriptor(m_lstAvailableDatabases.at(nAvailableDBIndex));

	bool bFound = false;
	for (int ndxLoaded = 0; ndxLoaded < g_lstBibleDatabases.size(); ++ndxLoaded) {
		CBibleDatabasePtr pBibleDatabase = g_lstBibleDatabases.at(ndxLoaded);
		assert(pBibleDatabase.data() != NULL);
		if (pBibleDatabase.data() == NULL) continue;
		if (pBibleDatabase->compatibilityUUID().compare(bblDesc.m_strUUID, Qt::CaseInsensitive) == 0) {
			m_mapAvailableToLoadedIndex[nAvailableDBIndex] = ndxLoaded;
			bFound = true;
		}
	}
	if (!bFound) m_mapAvailableToLoadedIndex[nAvailableDBIndex] = -1;
}


QStringList CBibleDatabaseListModel::availableBibleDatabasesUUIDs() const
{
	QStringList lstUUIDs;

	lstUUIDs.reserve(m_lstAvailableDatabases.size());
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		lstUUIDs.append(bibleDescriptor(m_lstAvailableDatabases.at(ndx)).m_strUUID);
	}

	return lstUUIDs;
}

int CBibleDatabaseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstAvailableDatabases.size();
}

QVariant CBibleDatabaseListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	int ndxDB = index.row();

	if ((ndxDB < 0) || (ndxDB >= m_lstAvailableDatabases.size()))
		return QVariant();

	const TBibleDescriptor &bblDesc = bibleDescriptor(m_lstAvailableDatabases.at(ndxDB));

	if (role == Qt::DisplayRole)
		return QString("%1%2").arg(bblDesc.m_strDBDesc).arg(bblDesc.m_bAutoLoad ? QString("  [%1]").arg(tr("Cannot be unloaded")) : QString());

	if (role == Qt::EditRole)
		return bblDesc.m_strDBDesc;


	if (role == BDDRE_BIBLE_DESCRIPTOR_ROLE)
		return QVariant::fromValue(m_lstAvailableDatabases.at(ndxDB));

	if (role == BDDRE_DATABASE_POINTER_ROLE) {
		int nBibleDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);
		if (nBibleDB != -1) {
			return QVariant::fromValue(g_lstBibleDatabases.at(nBibleDB).data());
		} else {
			return QVariant::fromValue(static_cast<CBibleDatabase *>(NULL));
		}
	}

	if (role == BDDRE_UUID_ROLE) return bblDesc.m_strUUID;

	if (role == Qt::CheckStateRole) {
		bool bIsCurrentlyChecked = (CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID).loadOnStart() || bblDesc.m_bAutoLoad);
		return (bIsCurrentlyChecked ? Qt::Checked : Qt::Unchecked);
	}

	return QVariant();
}

bool CBibleDatabaseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) return false;

	int ndxDB = index.row();

	if ((ndxDB >= 0) && (ndxDB < m_lstAvailableDatabases.size())) {
		if (role == Qt::CheckStateRole) {
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
				// "unload" it by unmapping it (unless it's a special autoLoad:
				if (!bblDesc.m_bAutoLoad) {
					TBibleDatabaseSettings bblDBaseSettings = CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID);
					bblDBaseSettings.setLoadOnStart(false);
					CPersistentSettings::instance()->setBibleDatabaseSettings(bblDesc.m_strUUID, bblDBaseSettings);
				} else {
					bNewCheck = true;
				}
			}
			if (bNewCheck != bIsCurrentlyChecked) emit changedAutoLoadStatus(bblDesc.m_strUUID, bNewCheck);

			if (bIsCurrentlyChecked != bNewCheck) {
				emit dataChanged(index, index);
			}
			return true;
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

	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | (bibleDescriptor(m_lstAvailableDatabases.at(ndxDB)).m_bAutoLoad ? Qt::NoItemFlags : Qt::ItemIsUserCheckable) | Qt::ItemIsSelectable;
}

// ============================================================================
