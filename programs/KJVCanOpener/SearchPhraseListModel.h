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

#ifndef SEARCH_PHRASE_LIST_MODEL_H
#define SEARCH_PHRASE_LIST_MODEL_H

#include "dbstruct.h"
#include "SearchPhraseEdit.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <QStringList>
#include <QVariant>

// ============================================================================

Q_DECLARE_METATYPE(CSearchPhraseEdit*)

typedef QList<CSearchPhraseEdit *> CSearchPhraseEditList;

// ============================================================================

class CSearchPhraseListModel : public QAbstractListModel
{
	Q_OBJECT
public:

	enum SEARCH_PHRASE_DATA_ROLES_ENUM {
		SEARCH_PHRASE_EDIT_WIDGET_ROLE = Qt::UserRole + 0
	};

	explicit CSearchPhraseListModel(QObject *parent = nullptr);

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	virtual Qt::DropActions supportedDropActions() const override;

	CSearchPhraseEditList phraseEditorsList() const;
	void setPhraseEditorsList(const CSearchPhraseEditList &lstPhraseEditors);

signals:

public slots:

private:
	Q_DISABLE_COPY(CSearchPhraseListModel)
	CSearchPhraseEditList m_lstPhraseEditors;
};

// ============================================================================

#endif // SEARCH_PHRASE_LIST_MODEL_H
