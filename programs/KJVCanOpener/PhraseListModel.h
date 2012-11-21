#ifndef PHRASELISTMODEL_H
#define PHRASELISTMODEL_H

#include "dbstruct.h"

#include <QAbstractListModel>
#include <QModelIndex>

class CPhraseListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	enum PHRASE_DATA_ROLES_ENUM {
		PHRASE_ENTRY_ROLE = Qt::UserRole + 0
	};

	explicit CPhraseListModel(QObject *parent = 0);
	CPhraseListModel(const CPhraseList &phrases, QObject *parent = 0);

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	virtual Qt::DropActions supportedDropActions() const;

	CPhraseList phraseList() const;
	void setPhraseList(const CPhraseList &phrases);

signals:

public slots:

private:
	Q_DISABLE_COPY(CPhraseListModel)
	CPhraseList m_lstPhrases;
};

#endif // PHRASELISTMODEL_H
