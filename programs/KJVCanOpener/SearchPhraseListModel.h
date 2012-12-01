#ifndef SEARCHPHRASELISTMODEL_H
#define SEARCHPHRASELISTMODEL_H

#include "dbstruct.h"
#include "KJVSearchPhraseEdit.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QList>
#include <QStringList>
#include <QVariant>

#include <assert.h>

// ============================================================================

Q_DECLARE_METATYPE(CKJVSearchPhraseEdit*)

typedef QList<CKJVSearchPhraseEdit *> CSearchPhraseEditList;

// ============================================================================

class CSearchPhraseListModel : public QAbstractListModel
{
	Q_OBJECT
public:

	enum SEARCH_PHRASE_DATA_ROLES_ENUM {
		SEARCH_PHRASE_EDIT_WIDGET_ROLE = Qt::UserRole + 0
	};

	explicit CSearchPhraseListModel(QObject *parent = 0);

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	virtual Qt::DropActions supportedDropActions() const;

	CSearchPhraseEditList phraseEditorsList() const;
	void setPhraseEditorsList(const CSearchPhraseEditList &lstPhraseEditors);

signals:

public slots:

private:
	Q_DISABLE_COPY(CSearchPhraseListModel)
	CSearchPhraseEditList m_lstPhraseEditors;
};

// ============================================================================

#endif // SEARCHPHRASELISTMODEL_H
