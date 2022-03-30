/****************************************************************************
**
** Copyright (C) 2013-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "ModelRowForwardIterator.h"

#include <QStack>

void CModelRowForwardIterator::connect(const QAbstractItemModel *pModel)
{
	// if our current row is due to disappear, find the next sibling of the item that's going to still be around.
	//	Or if a row is inserted, restore who our parent is as it might have changed:
	connect(pModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
	connect(pModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(restore()));
	connect(pModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(save()));
	connect(pModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(restore()));

	// layoutChanged means the model reordered itself extensively.
	// Since we cannot practically determine which row is the earliest one that contains
	//	indexes we haven't visited, just start over at the beginning:
	connect(pModel, SIGNAL(layoutChanged()), this, SLOT(reset()));
	connect(pModel, SIGNAL(modelReset()), this, SLOT(reset()));
}

void CModelRowForwardIterator::disconnect(const QAbstractItemModel *pModel)
{
	disconnect(pModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
	disconnect(pModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(restore()));
	disconnect(pModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(save()));
	disconnect(pModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(restore()));

	disconnect(pModel, SIGNAL(layoutChanged()), this, SLOT(reset()));
	disconnect(pModel, SIGNAL(modelReset()), this, SLOT(reset()));
}

// ----------------------------------------------------------------------------

void CModelRowForwardIterator::rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
	if (contains(parent, start, end, m_indexCurrent)) {
		// Move to the end of the to-be-removed range:
		m_indexParent = parent;
		m_indexCurrent = m_indexCurrent.model()->index(end, 0, parent);
		// and then move to whatever follows it, which isn't going to be removed:
		nextSibling();
	}
	save();
}

void CModelRowForwardIterator::save()
{
	m_indexPersist = m_indexCurrent;
}

void CModelRowForwardIterator::restore()
{
	if (m_indexPersist.isValid()) {
		m_indexCurrent = m_indexPersist;
		m_indexParent = m_indexCurrent.parent();
	} else {
		reset();
	}
	m_indexPersist = QPersistentModelIndex();
}

void CModelRowForwardIterator::reset()
{
	if (!m_indexCurrent.isValid()) return;

	const QAbstractItemModel *pModel = m_indexCurrent.model();
	m_indexParent = QModelIndex();
	if ((pModel->rowCount(m_indexParent) > 0) && (pModel->columnCount(m_indexParent) > 0)) {
		m_indexCurrent = pModel->index(0, 0, m_indexParent);
	} else {
		disconnect(pModel);
		m_indexCurrent = QModelIndex();
	}
}

// ----------------------------------------------------------------------------

QModelIndex CModelRowForwardIterator::findCommonParent(const QModelIndex &ndx1, const QModelIndex &ndx2, int &nRow1, int &nRow2)
{
	QStack<QModelIndex> stkParents1;
	QStack<QModelIndex> stkParents2;

	// Convert both indexes to a stack of their ancestors, stopping at (but including) QModelIndex():
	QModelIndex ndxParent1 = ndx1;
	while (ndxParent1.isValid()) {
		stkParents1 += ndxParent1;
		ndxParent1 = ndxParent1.parent();
	}
	stkParents1 += ndxParent1;			// Include top QModelIndex()

	QModelIndex ndxParent2 = ndx2;
	while (ndxParent2.isValid()) {
		stkParents2 += ndxParent2;
		ndxParent2 = ndxParent2.parent();
	}
	stkParents2 += ndxParent2;			// Include top QModelIndex()

	QModelIndex ndxCommonParent = QModelIndex();
	do {
		// This works because our first pop is guaranteed to pop off the QModelIndex()
		//		that is at the top of both stacks and we iterate here as long as the
		//		topmost parents are the same:
		ndxCommonParent = stkParents1.pop();
		stkParents2.pop();
		nRow1 = (stkParents1.isEmpty() ? -1 : stkParents1.top().row());
		nRow2 = (stkParents2.isEmpty() ? -1 : stkParents2.top().row());
	} while (!stkParents1.isEmpty() && !stkParents2.isEmpty() && (stkParents1.top() == stkParents2.top()));

	return ndxCommonParent;
}

int CModelRowForwardIterator::compare(const QModelIndex &ndx1, int nRow1, const QModelIndex &ndx2, int nRow2)
{
	int nCommonRow1;
	int nCommonRow2;

	findCommonParent(ndx1, ndx2, nCommonRow1, nCommonRow2);

	// If either parent ended up containing the other, use the provided row;
	//	otherwise use the higher level row found from the model:
	if (nCommonRow1 == -1) nCommonRow1 = nRow1;
	if (nCommonRow2 == -1) nCommonRow2 = nRow2;

	if (nCommonRow1 < nCommonRow2) {
		return -1;
	} else if (nCommonRow1 > nCommonRow2) {
		return 1;
	}

	return 0;
}

