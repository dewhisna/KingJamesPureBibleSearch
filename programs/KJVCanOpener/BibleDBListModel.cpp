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

// ============================================================================

CBibleDatabaseListModel::CBibleDatabaseListModel(QObject *parent)
	:	QAbstractListModel(parent)
{
	for (unsigned int dbNdx = 0; dbNdx < bibleDescriptorCount(); ++dbNdx) {
		const TBibleDescriptor &bblDesc = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx));
		CReadDatabase rdbMain(g_strBibleDatabasePath, g_strDictionaryDatabasePath);
		if (!rdbMain.haveBibleDatabaseFiles(bblDesc)) continue;
		int ndxCurrent = m_lstAvailableDatabases.size();
		m_lstAvailableDatabases.append(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx));
		bool bFound = false;
		for (int ndxLoaded = 0; ndxLoaded < g_lstBibleDatabases.size(); ++ndxLoaded) {
			CBibleDatabasePtr pBibleDatabase = g_lstBibleDatabases.at(ndxLoaded);
			assert(pBibleDatabase.data() != NULL);
			if (pBibleDatabase.data() == NULL) continue;
			if (pBibleDatabase->compatibilityUUID().compare(bblDesc.m_strUUID) == 0) {
				m_mapAvailableToLoadedIndex[ndxCurrent] = ndxLoaded;
				bFound = true;
			}
		}
		if (!bFound) m_mapAvailableToLoadedIndex[ndxCurrent] = -1;
	}
}

CBibleDatabaseListModel::~CBibleDatabaseListModel()
{

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

	if ((role == Qt::DisplayRole) ||
		(role == Qt::EditRole))
		return bibleDescriptor(m_lstAvailableDatabases.at(ndxDB)).m_strDBDesc;

	if (role == BDDRE_BIBLE_DESCRIPTOR_ROLE)
		return QVariant::fromValue(m_lstAvailableDatabases.at(ndxDB));

	if (role == BDDRE_DATABASE_POINTER_ROLE) {
		int nBibleDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);
		if (nBibleDB != -1) {
			return QVariant::fromValue(g_lstBibleDatabases.at(nBibleDB).data());
		} else return QVariant::fromValue(static_cast<CBibleDatabase *>(NULL));
	}

	if (role == Qt::CheckStateRole) {
		int nBibleDB = m_mapAvailableToLoadedIndex.value(ndxDB, -1);
		return ((nBibleDB != -1) ? Qt::Checked : Qt::Unchecked);
	}

	return QVariant();
}

bool CBibleDatabaseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);

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
