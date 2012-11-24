#ifndef VERSELISTDELEGATE_H
#define VERSELISTDELEGATE_H

#include "VerseListModel.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QWidget>
#include <QAbstractItemModel>
#include <QAbstractItemView>

class CVerseListDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	CVerseListDelegate(CVerseListModel &model, QObject *parent = NULL);

	virtual void paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
//	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
//	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:


protected:
	QAbstractItemView* parentView() const { return static_cast<QAbstractItemView*>(QObject::parent()); }

	inline void setVerseListPalette(QPalette *palette) const {
		// This sets a palette useful for the black text of the verses:
		palette->setColor(QPalette::All, QPalette::HighlightedText, palette->color(QPalette::Active, QPalette::Text));
		// Note that setting a saturated color here results in ugly XOR colors in the focus rect
		palette->setColor(QPalette::All, QPalette::Highlight, palette->base().color().darker(108));
	}

private:
	CVerseListModel &m_model;
};

#endif // VERSELISTDELEGATE_H
