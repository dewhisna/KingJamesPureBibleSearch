/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef REFLOW_DELEGATE_H
#define REFLOW_DELEGATE_H

#include <QAbstractItemDelegate>
#include <QPersistentModelIndex>
#include <QAbstractItemView>
#include <QPointer>
#include <QTimer>
#include "Qt_QStyleOption_stub.h"

#include "ModelRowForwardIterator.h"

// Forward Declarations:
class QTreeView;

// ============================================================================

class CScrollPreserver : public QObject {
public:
	CScrollPreserver(QTreeView *pView);
	~CScrollPreserver();

	QModelIndex index() const { return m_index; }
	QModelIndex row() const { return m_index.sibling(m_index.row(), 0); }
	int deltaY() const;
private:
	QAbstractItemView *parentView() const { return qobject_cast<QAbstractItemView*>(parent()); }

private:
	enum SCROLL_PRESERVER_MODE_ENUM {
		SPME_Invalid = 0,
		SPME_CurrentIndex = 1,
		SPME_TopOfView = 2
	};

	SCROLL_PRESERVER_MODE_ENUM m_nMode;
	QPersistentModelIndex m_index;			// TODO : replace this with CModelRowForwardIterator (if the index this is tracking is filtered out -- go to the next thing - not the beginning of the model)
	int m_nOffset;
};

// ============================================================================

class CReflowDelegate : public QAbstractItemDelegate {
	Q_OBJECT
public:
	CReflowDelegate(QTreeView *parent, bool bDoBlockingUpdate = true, bool bReflowDisabled = false);
	~CReflowDelegate();

	virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex & index) const override;
	virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

	virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	QAbstractItemDelegate *itemDelegate(const QModelIndex &index) const;
	QAbstractItemDelegate *itemDelegate() const { return m_mapItemDelegates.value(-1); }
	void setItemDelegate(QAbstractItemDelegate *pDelegate) { setItemDelegateForColumn(-1, pDelegate); }

	QAbstractItemDelegate *itemDelegateForColumn(int i) const;
	void setItemDelegateForColumn(int column, QAbstractItemDelegate *pDelegate);

	// if set to true, only leaf nodes (no children) get reflowed
	// and column headers are left alone (either as columns, or spanned, or whatever you've set separately)
	bool onlyLeaves() const { return m_bOnlyLeaves; }
	void setOnlyLeaves(bool bOnlyLeaves);

	inline bool isReflowDisabled() const { return m_bReflowDisabled; }

	void setFakeSizeHintRowCount(int nRowCount = 1) { Q_ASSERT(nRowCount >= 1); m_nFakeSizeHintRowCount = nRowCount; }
	inline int fakeSizeHintRowCount() const { return m_nFakeSizeHintRowCount; }

public slots:
	void setReflowDisabled(bool bDisable);
	void startReflow();

	virtual bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private slots:
	void layoutItem(const QModelIndex &index);
	void layoutColumn(int column, int oldSize, int newSize);
	void layoutViewport(QSize size);

	void reflowViewport();
	void reflowTick();
	void reflowHalt();

protected:
	void sizeHintChanged(const QModelIndex &index) const { emit sizeHintChanged(index,index); }
signals:
	void sizeHintChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) const;

private:
	enum SIZE_HINT_CACHE_MODE_ENUM {
		SHCME_CachedOnly = 0,
		SHCME_ComputeIfNeeded = 1
	};

	void layoutRow(const QStyleOptionViewItem &option, const QModelIndex &index, QSize *pSizeHint = nullptr, QVector<QSize> *pVecSZColumns = nullptr, SIZE_HINT_CACHE_MODE_ENUM nSizeHintMode = SHCME_ComputeIfNeeded) const;
	QStyleOptionViewItemV4_t viewOptions(const QModelIndex &index);

	QTreeView *parentView() const;
	virtual bool eventFilter(QObject *o, QEvent *e) override;

	// We are a proxy for Column 0.  For the others, continue to delegate
	//		to whatever delegate would have normally handled it:
	typedef QMap<int, QPointer<QAbstractItemDelegate> > TItemDelegatesMap;
	TItemDelegatesMap m_mapItemDelegates;
	int delegateRefCount(const QAbstractItemDelegate *pDelegate) const;

private:
	// Cache:
	mutable QSize m_szViewport;

	bool m_bOnlyLeaves;
	bool m_bDoBlockingUpdate;
	int m_nFakeSizeHintRowCount;

	SIZE_HINT_CACHE_MODE_ENUM m_nSizeHintCacheMode;

	QTimer m_timerReflow;
	CModelRowForwardIterator m_itrReflowIndex;
	bool m_bReflowDisabled;
};

#endif		// REFLOW_DELEGATE_H
