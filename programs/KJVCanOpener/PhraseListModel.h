/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PHRASELISTMODEL_H
#define PHRASELISTMODEL_H

#include "dbstruct.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QMimeData>

// ============================================================================

class CPhraseListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	enum PHRASE_DATA_ROLES_ENUM {
		PHRASE_ENTRY_ROLE = Qt::UserRole + 0
	};

	explicit CPhraseListModel(QObject *parent = nullptr);
	CPhraseListModel(const CPhraseList &phrases, QObject *parent = nullptr);

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	virtual Qt::DropActions supportedDropActions() const override;

	CPhraseList phraseList() const;
	void setPhraseList(const CPhraseList &phrases);

signals:

public slots:

private:
	Q_DISABLE_COPY(CPhraseListModel)
	CPhraseList m_lstPhrases;
};

// ============================================================================

class CMatchingPhrasesListModel : public QStringListModel
{
	Q_OBJECT

public:
	explicit CMatchingPhrasesListModel(QObject *pParent = nullptr)
		:	QStringListModel(pParent)
	{
#if QT_VERSION < 0x050000
		setSupportedDragActions(mySupportedDragActions());
#endif
	}

	CMatchingPhrasesListModel(const QStringList &lstPhrases, QObject *pParent = nullptr)
		:	QStringListModel(lstPhrases, pParent)
	{
#if QT_VERSION < 0x050000
		setSupportedDragActions(mySupportedDragActions());
#endif
	}

#if QT_VERSION >= 0x050000
	virtual Qt::DropActions supportedDragActions() const override
	{
		return mySupportedDragActions();
	}
#endif
	virtual Qt::DropActions supportedDropActions() const override;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

private:
	Qt::DropActions mySupportedDragActions() const;
};

// ============================================================================

#endif // PHRASELISTMODEL_H
