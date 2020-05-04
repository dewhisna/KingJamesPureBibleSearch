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

#ifndef ModelRowForwardIterator_h
#define ModelRowForwardIterator_h

#include <QObject>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>

#include <assert.h>

class CModelRowForwardIterator : public QObject
{
	Q_OBJECT
public:
	CModelRowForwardIterator() {}
	CModelRowForwardIterator(const QAbstractItemModel *pModel) { *this = pModel; }
	CModelRowForwardIterator(QModelIndex &index) { *this = index; }

	CModelRowForwardIterator & operator=(const QAbstractItemModel *pModel)
	{
		assert(pModel != NULL);
		return *this = (pModel->hasChildren() ? pModel->index(0,0) : QModelIndex());
	}

	CModelRowForwardIterator & operator=(const QModelIndex &index)
	{
		if (m_indexCurrent.isValid()) disconnect(m_indexCurrent.model());
		m_indexCurrent = index.sibling(index.row(), 0);
		m_indexParent = m_indexCurrent.parent();
		if (index.isValid()) connect(index.model());
		return *this;
	}

	const QModelIndex &operator*() const { return m_indexCurrent; }
	const QModelIndex *operator->() const { return &m_indexCurrent; }
	bool operator ==(const QModelIndex &index) const { return m_indexCurrent == index; }
	bool operator !=(const QModelIndex &index) const { return m_indexCurrent != index; }
	operator bool() const { return m_indexCurrent.isValid(); }

	CModelRowForwardIterator &operator++()
	{
		const QAbstractItemModel *pModel = m_indexCurrent.model();
		if (pModel->hasChildren(m_indexCurrent)) {
			// If the current index has children, move to the first child:
			m_indexParent = m_indexCurrent;
			m_indexCurrent = pModel->index(0, 0, m_indexCurrent);
			return *this;
		} else {
			return nextSibling();
		}
	}

	CModelRowForwardIterator &nextSibling()
	{
		const QAbstractItemModel *pModel = m_indexCurrent.model();
		// Find the next-sibling, either of the current index, or of its ancestors.
		//		When we reach the root index, we have exhausted the model:
		while (m_indexCurrent.isValid()) {
			if ((m_indexCurrent.row()+1) < pModel->rowCount(m_indexParent)) {
				// If the current has more siblings, move to those:
				m_indexCurrent = m_indexCurrent.sibling(m_indexCurrent.row()+1, 0);
				break;
			} else if (!m_indexParent.isValid()) {
				// reached the end of the model...
				disconnect(m_indexCurrent.model());
			}
			m_indexCurrent = m_indexParent;
			m_indexParent = m_indexCurrent.parent();
		}
		return *this;
	}

	const QModelIndex &parent() const { return m_indexParent; }
	bool hasChildren() const { return m_indexCurrent.model()->hasChildren(m_indexCurrent); }
	int row() const { return m_indexCurrent.row(); }
	QModelIndex column(int nCol) const { return m_indexCurrent.model()->index(m_indexCurrent.row(), nCol, m_indexParent); }
	const QAbstractItemModel *model() const { return m_indexCurrent.model(); }

	using QObject::connect;
	using QObject::disconnect;
private slots:
	// void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);		Need no extra behaviour other than what's in QPersistantModelIndex
	void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
	// no need to implement rowsAboutToBeMoved or rowsMoved, as beginMoveRows also emits
	//		layoutAboutToBeChanged, and endMoveRows emits layoutChanged, so we end up just
	//		resetting completely anyway

	void save();
	void restore();
	void reset();

private:
	void connect(const QAbstractItemModel *pModel);
	void disconnect(const QAbstractItemModel *pModel);

private:
	// Find the deepest common parent for the two provided indexes, and their respective row numbers.
	//	If one index is a child of the other, then -1 is returned for the row number of the element
	//	that is the parent.  If they are the same element, then the commonParent is returned and -1 is
	//	returned for the row numbers:
	static QModelIndex findCommonParent(const QModelIndex &ndx1, const QModelIndex &ndx2, int &nRow1, int &nRow2);

	// Compare the position of two indexes relative to this iterator:
	//	-1 = ndx1 < ndx2 (ndx1 is a preceding-sibling, a child of a preceding-sibling, or an ancestor of ndx2)
	//	 0 = ndx1 and ndx2 reference the same row (but not necessarily the same column)
	//	 1 = ndx1 > ndx2 (ndx1 is a child, following-sibling, or following-sibling of an ancestor of ndx2)
	static int compare(const QModelIndex &ndx1, int nRow1, const QModelIndex &ndx2, int nRow2);
	static int compare(const QModelIndex &ndx1, const QModelIndex &ndx2)
	{
		return compare(ndx1, -1, ndx2, -1);
	}

	static bool contains(const QModelIndex &ndxParent, int nStart, int nEnd, QModelIndex ndxQuery)
	{
		while (ndxQuery.isValid()) {
			int nRow = ndxQuery.row();
			ndxQuery = ndxQuery.parent();
			if (ndxQuery == ndxParent) {
				return ((nStart <= nRow) && (nEnd >= nRow));
			}
		}
		return false;
	}

private:
	QModelIndex m_indexParent;
	QModelIndex m_indexCurrent;
	QPersistentModelIndex m_indexPersist;
};

#endif		// ModelRowForwardIterator_h
