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

#ifndef BIBLE_DATABASE_LIST_MODEL_H
#define BIBLE_DATABASE_LIST_MODEL_H

#include "dbDescriptors.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>

// ============================================================================

class CBibleDatabaseListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum BIBLE_DATABASE_DATA_ROLES_ENUM {
		BDDRE_DATABASE_POINTER_ROLE = Qt::UserRole + 0,		// CBibleDatabase* Role
		BDDRE_UUID_ROLE = Qt::UserRole + 1					// Bible Database UUID
	};

	CBibleDatabaseListModel(QObject *parent = nullptr);
	virtual ~CBibleDatabaseListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QVariant data(const QString &strUUID, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual bool setData(const QString &strUUID, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	void updateBibleDatabaseList();

signals:
	void loadBibleDatabase(const QString &strUUID);
	void changedAutoLoadStatus(const QString &strUUID, bool bAutoLoad);

private:
	void locateLoadedDatabase(int nAvailableDBIndex);

private:
	const QList<TBibleDescriptor> &m_lstAvailableDatabaseDescriptors;	// List of descriptors for available Bible databases reference to TBibleDatabaseList::availableBibleDatabases()
	QMap<int, int> m_mapAvailableToLoadedIndex;					// Mapping of indexes in Available Database list (above) to the global Bible Database list index (-1 = database isn't loaded)

	Q_DISABLE_COPY(CBibleDatabaseListModel)
};

// ============================================================================

#endif	// BIBLE_DATABASE_LIST_MODEL_H
