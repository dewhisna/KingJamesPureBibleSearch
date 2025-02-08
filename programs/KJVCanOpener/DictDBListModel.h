/****************************************************************************
**
** Copyright (C) 2014-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef DICT_DATABASE_LIST_MODEL_H
#define DICT_DATABASE_LIST_MODEL_H

#include "dbDescriptors.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>

// ============================================================================

class CDictDatabaseListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum DICT_DATABASE_DATA_ROLES_ENUM {
		DDDRE_DATABASE_POINTER_ROLE = Qt::UserRole + 0,			// CDictionaryDatabase* Role
		DDDRE_UUID_ROLE = Qt::UserRole + 1						// Dictionary Database UUID
	};

	CDictDatabaseListModel(QObject *parent = nullptr);
	virtual ~CDictDatabaseListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual QVariant data(const QString &strUUID, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	virtual bool setData(const QString &strUUID, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	void updateDictDatabaseList();

signals:
	void loadDictDatabase(const QString &strUUID);
	void changedAutoLoadStatus(const QString &strUUID, bool bAutoLoad);

private:
	void locateLoadedDatabase(int nAvailableDBIndex);

private:
	const QList<TDictionaryDescriptor> &m_lstAvailableDatabaseDescriptors;	// List of descriptors for available Dictionary databases reference to TDictionaryDatabaseList::availableDictionaryDatabases()
	QMap<int, int> m_mapAvailableToLoadedIndex;					// Mapping of indexes in Available Database list (above) to the global Dictionary Database list index (-1 = database isn't loaded)

	Q_DISABLE_COPY(CDictDatabaseListModel)
};

// ============================================================================

#endif	// DICT_DATABASE_LIST_MODEL_H
