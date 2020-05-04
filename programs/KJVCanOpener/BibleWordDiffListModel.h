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

#ifndef BIBLE_WORD_DIFF_LIST_MODEL_H
#define BIBLE_WORD_DIFF_LIST_MODEL_H

#include "dbstruct.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>

// ============================================================================

class CBibleWordDiffListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CBibleWordDiffListModel(CBibleDatabasePtr pBibleDatabase = CBibleDatabasePtr(), QObject *parent = NULL);
	virtual ~CBibleWordDiffListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	void setBibleDatabase(CBibleDatabasePtr pBibleDatabase);

private slots:
	void en_changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings);

private:
	CBibleDatabasePtr m_pBibleDatabase;
	TConcordanceList m_lstWords;							// List of words that have diffs

	Q_DISABLE_COPY(CBibleWordDiffListModel)
};

// ============================================================================

#endif	// BIBLE_WORD_DIFF_LIST_MODEL_H
