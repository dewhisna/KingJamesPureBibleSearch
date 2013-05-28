/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "ReflowDelegate.h"

#include <QTreeView>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QResizeEvent>
#include <QScrollBar>
#include <QPainter>
#include <QTime>
#include <QAbstractItemModel>

#include <algorithm>
#include <assert.h>

#define REFLOW_BATCH_SIZE 100			// Number of items to update per reflowTick
#define REFLOW_TIMESLICE 100			// Maximum reflowTick timeslice in milliseconds

// ============================================================================

CScrollPreserver::CScrollPreserver(QTreeView *pView)
	:	QObject(pView),
		m_nMode(SPME_Invalid),
		m_nOffset(0)
{
	assert(pView != NULL);

	if (pView->verticalScrollMode() == QAbstractItemView::ScrollPerPixel) {
		m_index = pView->selectionModel()->currentIndex();
		QRect rcVisual = pView->visualRect(index());
		if ((rcVisual.isValid()) && (pView->viewport()->rect().intersects(rcVisual))) {
			m_nMode = SPME_CurrentIndex;
		} else {
			m_nMode = SPME_TopOfView;
			if (pView->header()->isVisible()) {
				m_index = pView->indexAt(QPoint(-pView->header()->offset(), 0));
			} else {
				m_index = pView->indexAt(QPoint(0, 0));
			}
			rcVisual = pView->visualRect(index());
		}
		m_nOffset = rcVisual.bottom();
	}
}

CScrollPreserver::~CScrollPreserver()
{
	if ((parentView() != NULL) && (m_index.isValid())) {
		int dy = deltaY();
		if (dy != 0) {
			QScrollBar *pVBar = parentView()->verticalScrollBar();
			assert(pVBar != NULL);
			pVBar->setValue(pVBar->value() + dy);
		}
	}
}

int CScrollPreserver::deltaY() const
{
	if (parentView() == NULL)  return 0;

	const QRect rcVisual = parentView()->visualRect(index());
	int dy = rcVisual.bottom();

	if (m_nMode == SPME_TopOfView) {
		// If tracking the top item of the viewport, don't let anything appear above it
		dy -= std::min(m_nOffset, rcVisual.height());
	} else {
		// tracking an item within the viewport, so we don't care if other rows appear/disappear above it
		dy -= m_nOffset;
	}
	return dy;
}

// ============================================================================

namespace {
	template <typename T>
	class CScopedValue {
	public:
		CScopedValue(T& var) : var(var), value(var) { }
		CScopedValue(T& var, const T& tmp) : var(var), value(var) { var = tmp; }
		~CScopedValue() { std::swap(var,value); }
		CScopedValue &operator= (const T& value) { var = value; return *this; }
		T& operator*() { return var; }

	private:
		T& var;
		T value;
	};

	template <typename T> T clamp(T min, T value, T max) {
		if(value < min) return min;
		else if (value > max) return max;
		else return value;
	}
}

// ============================================================================

CReflowDelegate::CReflowDelegate(QTreeView *parent, bool bDoBlockingUpdate)
	:	QAbstractItemDelegate(parent),
		m_bOnlyLeaves(false),
		m_bDoBlockingUpdate(bDoBlockingUpdate),
		m_nSizeHintCacheMode(SHCME_CachedOnly)
{
	assert(parent != NULL);

	setItemDelegate(parent->itemDelegate());

	connect(parent->header(), SIGNAL(sectionResized(int,int,int)), SLOT(layoutColumn(int,int,int)));

	// Install Event Filter for QEvent::Resize and QEvent::Paint events:
	parent->viewport()->installEventFilter(this);

	connect(this, SIGNAL(sizeHintChanged(QModelIndex,QModelIndex)), parent, SLOT(dataChanged(QModelIndex,QModelIndex)));
	connect(parent->model(), SIGNAL(modelAboutToBeReset()), this, SLOT(reflowHalt()));

	m_timerReflow.setInterval(0);
	connect(&m_timerReflow, SIGNAL(timeout()), SLOT(reflowTick()));
}

CReflowDelegate::~CReflowDelegate()
{
	// Closing the widget interrupts reflow:
	if (m_timerReflow.isActive()) {
		QApplication::restoreOverrideCursor();
	}
}

QWidget *CReflowDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	Q_UNUSED(parent);
	Q_UNUSED(option);
	Q_UNUSED(index);
	return NULL;		// Editing not allowed (yet)
}

bool CReflowDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	return itemDelegate(index)->editorEvent(event, model, option, index);
}

void CReflowDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	Q_UNUSED(editor);
	Q_UNUSED(index);
	// Editing not allowed (yet)
}

void CReflowDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	Q_UNUSED(editor);
	Q_UNUSED(model);
	Q_UNUSED(index);
	// Editing not allowed (yet)
}

void CReflowDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(editor);
	Q_UNUSED(option);
	Q_UNUSED(index);
	// Editing not allowed (yet)
}

void CReflowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	layoutRow(option, index);
	itemDelegate(index)->paint(painter, option, index);
}

QSize CReflowDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize sizeHint;
	layoutRow(option, index, &sizeHint, NULL, m_nSizeHintCacheMode);

	return sizeHint;
}

QAbstractItemDelegate *CReflowDelegate::itemDelegate(const QModelIndex &index) const
{
	QAbstractItemDelegate *pDelegate = itemDelegateForColumn(index.column());
	if (!pDelegate) pDelegate = itemDelegate();
	return pDelegate;
}

QAbstractItemDelegate *CReflowDelegate::itemDelegateForColumn(int i) const
{
	return m_mapItemDelegates.value(i, 0);
}

void CReflowDelegate::setItemDelegateForColumn(int column, QAbstractItemDelegate *pDelegate)
{
	QAbstractItemDelegate *pColumnDelegate = m_mapItemDelegates.value(column, 0);
	if (pColumnDelegate) {
		if (delegateRefCount(pColumnDelegate) == 1) {
			disconnect(pColumnDelegate, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
			disconnect(pColumnDelegate, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
			disconnect(pColumnDelegate, SIGNAL(sizeHintChanged(QModelIndex)), this, SLOT(layoutItem(QModelIndex)));
		}
		m_mapItemDelegates.remove(column);
	}

	if (pDelegate) {
		if (delegateRefCount(pDelegate) == 0) {
			connect(pDelegate, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
			connect(pDelegate, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
			connect(pDelegate, SIGNAL(sizeHintChanged(QModelIndex)), this, SLOT(layoutItem(QModelIndex)));
		}
		m_mapItemDelegates.insert(column, pDelegate);
	}
}

void CReflowDelegate::setOnlyLeaves(bool bOnlyLeaves)
{
	if (bOnlyLeaves != m_bOnlyLeaves) {
		bool bWasOnlyLeaves = m_bOnlyLeaves;
		m_bOnlyLeaves = bOnlyLeaves;

		QTreeView *pView = parentView();
		assert(pView != NULL);
		for (CModelRowForwardIterator itr(pView->model()); itr; ++itr) {
			if (itr.hasChildren()) {
				if (!bWasOnlyLeaves) {
					// Clear spanning on any headers that presently were spanned and refresh it:
					if (pView->isFirstColumnSpanned(itr.row(), itr.parent())) {
						pView->setFirstColumnSpanned(itr.row(), itr.parent(), false);
						emit sizeHintChanged(*itr);
					}
				}
				if (!m_bOnlyLeaves) {
					// Re-examine headers:
					emit sizeHintChanged(*itr);
				}
			}
		}
	}
}

void CReflowDelegate::layoutRow(const QStyleOptionViewItem &option, const QModelIndex &index, QSize *pSizeHint, QVector<QSize> *pVecSZColumns, SIZE_HINT_CACHE_MODE_ENUM nSizeHintMode) const
{
	// If this one that isn't considered for reflowing, bailout:
	if (!index.isValid() || (m_bOnlyLeaves && index.model()->hasChildren(index))) return;

	QTreeView *pView = parentView();
	assert(pView != NULL);
	QModelIndex ndxParent(index.parent());
	int nColumnCount = index.model()->columnCount(ndxParent);

	QVector<QSize> vecSizeHints(nColumnCount);
	for (int nColumn = 0; nColumn < nColumnCount; ++nColumn) {
		QModelIndex ndxColumn = index.model()->index(index.row(), nColumn, ndxParent);
		QSize sizeHint = ndxColumn.data(Qt::SizeHintRole).toSize();

		if (!sizeHint.isValid()) {
			if ((nSizeHintMode == SHCME_CachedOnly) && (pView->model()->setData(ndxColumn, QSize(), Qt::SizeHintRole))) {
				// If we do not have a cached SizeHint, but could store one (setData() returns true),
				// give a placeholder instead of doing the calculation right now,
				// so that revealing large numbers of rows does not make the app seem to hang.

				// Instead, just use a bogus placeholder for the row with enough height to make scrolling behave reasonably:
				if (pSizeHint) *pSizeHint = QSize(0, option.fontMetrics.height());

				// We're giving bogus answers, so start a background reflow to compute an accurate answer later (reflowTick will use ComputeIfNeeded)
				const_cast<CReflowDelegate *>(this)->startReflow();
				return;
			}

			// If requested, or if we cannot cache cannot cache it (in which case there's nowhere
			//	to keep a value computed in the background) we have to compute it right now.  Then
			//	compute and cache the sizeHint:
			sizeHint = itemDelegate(ndxColumn)->sizeHint(option, ndxColumn);
			pView->model()->setData(ndxColumn, sizeHint, Qt::SizeHintRole);
		}
		vecSizeHints[nColumn] = sizeHint;
	}

	if (m_szViewport.isNull()) m_szViewport = pView->viewport()->size();
	if (pVecSZColumns) *pVecSZColumns = vecSizeHints;
	if(pSizeHint) *pSizeHint = vecSizeHints.value(index.column());
}

void CReflowDelegate::layoutItem(const QModelIndex &index)
{
	// Invalidate any cached sizeHint for this item:
	parentView()->model()->setData(index, QSize(), Qt::SizeHintRole);
	if (index.isValid()) {
		emit sizeHintChanged(index);
	} else {
		// Special-case : sizeHintChanged(QModelIndex()) is "invalidate all"
		startReflow();
		parentView()->viewport()->update();
	}
}

void CReflowDelegate::layoutColumn(int column, int oldSize, int newSize)
{
	Q_UNUSED(column);
	Q_UNUSED(oldSize);
	Q_UNUSED(newSize);

	// Reflow the viewport right now for user feedback:
	reflowViewport();
	// and start background reflowing of the rest:
	startReflow();
}

void CReflowDelegate::layoutViewport(QSize size)
{
	int nOldWidth = m_szViewport.width();
	m_szViewport = size;		// Update our cached viewport details

	if (nOldWidth == size.width()) return;		// If the width didn't change, we are done, as the row layouts won't change

	layoutItem(QModelIndex());			// Special case to invalidate all SizeHint caches

	// Reflow the viewport right now for user feedback:
	reflowViewport();
	// and start background reflowing of the rest:
	startReflow();
}

void CReflowDelegate::startReflow()
{
	if (m_timerReflow.isActive()) {
		// If we are already running reflow, restart back tot he beginning of the model:
		m_itrReflowIndex = QModelIndex();
	} else {
		// If we aren't current running reflow, start it:
		QApplication::setOverrideCursor(Qt::BusyCursor);
		m_timerReflow.start();
	}
}

void CReflowDelegate::reflowViewport()
{
	QTreeView *pView = parentView();
	assert(pView != NULL);
	QRect rcVisible = pView->viewport()->rect();

	QModelIndex ndxTop = pView->indexAt(rcVisible.topLeft());
	for (CModelRowForwardIterator itr(ndxTop); itr; ) {
		//NOTE: this is a nearly complete option, but we can't guarantee that it
		// perfectly matches what the view will use when it really makes delegate calls.
		// (in particular, it's still missing selections, hover highlights, and other transient-state things).
		// but it's only used for other delegate's sizeHints, and those *should* already all be cached from
		// real view calls to our sizeHint() and paint() (which did get a complete option).
		// So it ought to be close enough:
		QStyleOptionViewItemV4 option = viewOptions(*itr);

		// If we are now past the bottom of the viewport, quit since the rest aren't visible yet:
		if (option.rect.top() > rcVisible.bottom()) break;

		QSize sizeHint;
		QVector<QSize> vecSizeColumns;
		layoutRow(option, *itr, &sizeHint, &vecSizeColumns, SHCME_ComputeIfNeeded);

		// isFirstColumnSpanning() is rather slow (checks the QPersisentModelIndex list),
		// and the right edge tells us anyway. Note that width() does *not* work, because of indentation
		bool bIsSpanning = ((option.rect.right() + pView->header()->offset()) > pView->header()->sectionSize(0));
		bool bWantSpanning = false;
		if  (bIsSpanning != bWantSpanning) {
			pView->setFirstColumnSpanned(itr->row(), itr->parent(), bWantSpanning);
			emit sizeHintChanged(*itr);
		} else {
			if (!bWantSpanning) {
				// layoutRow only gives an overall sizeHint when spanning will be used (i.e., style > Columns)
				// otherwise, we need to figure out the maximum of all cells
				for (int ndxSize = 0; ndxSize < vecSizeColumns.size(); ++ndxSize) {
					sizeHint.setHeight(std::max(sizeHint.height(), vecSizeColumns.at(ndxSize).height()));
				}
			}

			if (option.rect.height() != sizeHint.height()) emit sizeHintChanged(*itr);
		}

		// We're about to paint anyway, so no need to do an update();

		if (pView->isExpanded(*itr)) {
			++itr;
		} else {
			// if its children aren't being shown, skip past them:
			itr.nextSibling();
		}
	}
}

void CReflowDelegate::reflowTick()
{
	if (QApplication::mouseButtons() == 0) {		// Only do reflow if user doesn't have a mouse button pressed:
		QTreeView *pView = parentView();
		assert(pView != NULL);

		// QModelIndex() marks the *end* of iteration.  But if we'd really finished, the timer wouldn't still
		//	be ticking so thus must be that the current position was invalidated, and we need to start over:
		if (!m_itrReflowIndex) m_itrReflowIndex = pView->model();

		QTime timeSlice;
		timeSlice.start();

		// prevent lazy sizeHint updates; this *is* the routine that comes back through to fix them
		CScopedValue<SIZE_HINT_CACHE_MODE_ENUM> sizeHintMode(m_nSizeHintCacheMode, SHCME_ComputeIfNeeded);

		// Save our view's scroll position (and restore it on exit):
		CScrollPreserver verticalOffset(pView);

		if (m_bDoBlockingUpdate) {
			while (m_itrReflowIndex) {
				QModelIndex ndxParent = m_itrReflowIndex.parent();
				QModelIndex ndxTopLeft = *m_itrReflowIndex;
				QModelIndex ndxBottomRight;
				// Doing updates one at a time causes the view to try and do updates one at a time,
				//	which means it spends a *lot* of time computing the visualRect of each item.  Therefore,
				//	we'll do them in batches, as defined by REFLOW_BATCH_SIZE:
				int nBlockCount = 0;
				do {
					ndxBottomRight = *m_itrReflowIndex;

					if (pView->isExpanded(*m_itrReflowIndex)) {
						++m_itrReflowIndex;

						// Optimization for Trees, as only trees will be expanded:
						// If we already have a size hint for it, keep going because we
						//	have an entry that will be quick anyway:
						if (m_itrReflowIndex->data(Qt::SizeHintRole).isValid()) continue;
					} else {
						// if its children aren't being shown, skip past them:
						m_itrReflowIndex.nextSibling();
					}

					++nBlockCount;
				} while((nBlockCount < REFLOW_BATCH_SIZE) && (m_itrReflowIndex) && (m_itrReflowIndex.parent() == ndxParent));		// Note: parents should be the same because dataChanged is defined only when the parents are the same
				emit sizeHintChanged(ndxTopLeft, ndxBottomRight);

				if (timeSlice.elapsed() > REFLOW_TIMESLICE) break;			// If it's taking too long, stop to let the UI run and do more on next tick...
			}
		} else {
			// For models with no hierarchy, it's OK to do them all at once:
			QAbstractItemModel *pModel = pView->model();
			assert(pModel != NULL);
			sizeHintChanged(pModel->index(0,0), pModel->index(pModel->rowCount()-1, 0));
		}

		// If reflow is complete, so ticking:
		if (!m_itrReflowIndex) {
			m_timerReflow.stop();
			QApplication::restoreOverrideCursor();
		}
	}
}

void CReflowDelegate::reflowHalt()
{
	if (m_timerReflow.isActive()) {
		m_timerReflow.stop();
		QApplication::restoreOverrideCursor();
	}
}

bool CReflowDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	// Call the other delegates help.  This is invoke because helpEvent is not virtual:
	bool retval = false;
	QMetaObject::invokeMethod(itemDelegate(index), "helpEvent",
							  Q_RETURN_ARG(bool, retval),
							  Q_ARG(QHelpEvent *, event),
							  Q_ARG(QAbstractItemView *, view),
							  Q_ARG(QStyleOptionViewItem, option),
							  Q_ARG(QModelIndex, index));
	return retval;
}

QStyleOptionViewItemV4 CReflowDelegate::viewOptions(const QModelIndex &index)
{
	class CSneakyItemView : public QTreeView {
	public:
		QStyleOptionViewItem viewOptions() const { return QTreeView::viewOptions(); }
		QStyleOptionViewItemV4 viewOptionsV4() const {
			// based on QAbstractItemViewPrivate::viewOptionsV4():
			QStyleOptionViewItemV4 option = viewOptions();
			if (this->wordWrap()) option.features = QStyleOptionViewItemV2::WrapText;
			option.locale = locale();
			option.locale.setNumberOptions(QLocale::OmitGroupSeparator);
			option.widget = this;
			return QAbstractItemView::viewOptions();
		}
	};

	QTreeView *pView = parentView();
	assert(pView != NULL);
	QStyleOptionViewItemV4 option = static_cast<CSneakyItemView *>(pView)->viewOptionsV4();
	option.rect = pView->visualRect(index);
	return option;
}

QTreeView *CReflowDelegate::parentView() const
{
	return static_cast<QTreeView *>(parent());
}

bool CReflowDelegate::eventFilter(QObject *o, QEvent *e)
{
	if (o == parentView()->viewport()) {
		switch(e->type()) {
			case QEvent::Paint:
				// Ensure our sizeHints are right for the viewport before it paints:
				reflowViewport();
				break;
			case QEvent::Resize:
				layoutViewport(static_cast<QResizeEvent *>(e)->size());
				break;
			default:
				break;
		}
	}

	return QAbstractItemDelegate::eventFilter(o,e);
}

int CReflowDelegate::delegateRefCount(const QAbstractItemDelegate *pDelegate) const
{
	int nRef = 0;

	for (TItemDelegatesMap::const_iterator itr = m_mapItemDelegates.begin(); itr != m_mapItemDelegates.end(); ++itr) {
		if (itr->data() == pDelegate) ++nRef;
	}

	return nRef;
}

