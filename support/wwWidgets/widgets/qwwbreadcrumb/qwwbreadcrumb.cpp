//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WW_NO_BREADCRUMB
#include "qwwbreadcrumb.h"
#include "wwglobal_p.h"

#include <QPainter>
#include <QtDebug>
#include <QDirModel>


#if QT_VERSION >=0x040400
#   include <QStyledItemDelegate>
#else
#   include <QItemDelegate>
#endif
#include <QPersistentModelIndex>


class QwwBreadCrumbPrivate: public QwwPrivate {
public:
    QwwBreadCrumbPrivate(QwwBreadCrumb *q) : QwwPrivate(q) {
        model = 0;
#if QT_VERSION >=0x040400
        delegate = new QStyledItemDelegate(q);
#else
        delegate = new QItemDelegate(q);
#endif

    }
    QAbstractItemModel *model;
    QAbstractItemDelegate *delegate;
    QPersistentModelIndex rootIndex;
    QPersistentModelIndex currentIndex;
    QSize iconSize;
    QList<QModelIndex> pathList() const {
        QList<QModelIndex> path;
        QModelIndex iterator = currentIndex;
        QModelIndex root = rootIndex;
        while(iterator!=root && iterator.isValid()){
            path.prepend(iterator);
            iterator = iterator.parent();
        }
        return path;
    }
private:
    WW_DECLARE_PUBLIC(QwwBreadCrumb);
};


/**
 *  @class QwwBreadCrumb
 *  @brief A bar displaying path (trail).
 *
 *          This widget provides an interactive bar that shows a trail
 *          within some structure.
 *
 *          It can be used for showing a path in a filesystem or
 *          section and subsequent subsections of a structured application
 *          (like an e-commerce app)
 *
 *
 */

QwwBreadCrumb::QwwBreadCrumb(QWidget * parent) : QWidget(parent), QwwPrivatable(new QwwBreadCrumbPrivate(this)) {
    Q_D(QwwBreadCrumb);
    QStyleOption opt;
    d->model = 0;
    opt.initFrom(this);
    int isize = style()->pixelMetric(QStyle::PM_SmallIconSize,&opt,this);
    d->iconSize = QSize(isize,isize);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->model = new QDirModel(this);
    d->currentIndex = ((QDirModel*)d->model)->index("/usr/share/icons");

}

/**
 *
 * @param m
 */
void QwwBreadCrumb::setModel(QAbstractItemModel * m) {
    Q_D(QwwBreadCrumb);
    if (m==d->model)
        return;
    if (d->model) {
        if (d->model->parent()==this)
            delete d->model;
        // disconnect signals
    }
    d->model = m;
    setRootIndex(QModelIndex());
    setCurrentIndex(QModelIndex());
    // connect signals
    // update
    update();
}

/**
 * @brief Returns the view's model
 *
 */
QAbstractItemModel * QwwBreadCrumb::model() const {
    Q_D(const QwwBreadCrumb);
    return d->model;
}

/**
 * @brief Sets the delegate for the view
 *
 */
void QwwBreadCrumb::setItemDelegate(QAbstractItemDelegate * deleg) {
    Q_D(QwwBreadCrumb);
    if (d->delegate->parent()==this)
        delete d->delegate;
    d->delegate = deleg;
    update();
}

/**
 * @brief Returns the current delegate
 *
 */
QAbstractItemDelegate * QwwBreadCrumb::itemDelegate() const {
    Q_D(const QwwBreadCrumb);
    return d->delegate;
}

void QwwBreadCrumb::paintEvent(QPaintEvent * event) {
    Q_D(const QwwBreadCrumb);
    QPainter painter(this);
    QList<QModelIndex> toBePainted = d->pathList();
    QModelIndex iterator = currentIndex();
    QRect r = contentsRect();
    QRect bounding;
    for(int i=0;i<toBePainted.count();i++){
        const QModelIndex &drawing = toBePainted[i];
        QString text = drawing.data(Qt::DisplayRole).toString();

#if QT_VERSION >= 0x040400
        QStyleOptionViewItemV2 option;
#else
        QStyleOptionViewItemV2 option;
#endif
        option.initFrom(this);

        option.decorationAlignment = Qt::AlignLeft|Qt::AlignVCenter;
        option.displayAlignment = Qt::AlignLeft|Qt::AlignVCenter;
        option.decorationSize = iconSize();
        QSize hint = itemDelegate()->sizeHint(option, drawing);
#if QT_VERSION >= 0x040400
        //option.widget = this;
//         option.features|=QStyleOptionViewItemV2::HasDecoration;
#endif
        if(i>0){
          QStyleOption opt;
          opt.initFrom(this);
          opt.rect = QRect(r.left(), r.top(), 16, hint.height());
          style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, &painter, this);
          r.setLeft(r.left()+20);
        }
        option.rect = QRect(r.left(), r.top(), hint.width(), hint.height());
        itemDelegate()->paint( &painter, option, drawing );
        r.setLeft(r.left()+hint.width()+4);
    }
}

/**
 *
 *
 */
QModelIndex QwwBreadCrumb::rootIndex() const {
    Q_D(const QwwBreadCrumb);
    return d->rootIndex;
}

/**
 *
 * @param i
 */
void QwwBreadCrumb::setRootIndex(const QModelIndex &i) {
    Q_D(QwwBreadCrumb);
    if (d->rootIndex==i)
        return;
    d->rootIndex = i;
    update();
}

/**
 *
 * @param i
 */
void QwwBreadCrumb::setCurrentIndex(const QModelIndex &i) {
    Q_D(QwwBreadCrumb);
    if (d->currentIndex==i)
        return;
    d->currentIndex = i;
    update();
}

/**
 *
 * @return
 */
QModelIndex QwwBreadCrumb::currentIndex() const {
    Q_D(const QwwBreadCrumb);
    return d->currentIndex;
}

QSize QwwBreadCrumb::sizeHint() const
{
    int l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    return QSize(l+r+400, t+b+qMax(fontMetrics().height(), iconSize().height()));
}

QModelIndex QwwBreadCrumb::indexAt(const QPoint & pt)
{
return QModelIndex();
}

/**
 *
 * @param s
 */
void QwwBreadCrumb::setIconSize(const QSize & s)
{
    Q_D(QwwBreadCrumb);
    if(d->iconSize==s)
        return;
    d->iconSize = s;
    updateGeometry();
    update();
}

/**
 *
 * @return
 */
QSize QwwBreadCrumb::iconSize() const
{
    Q_D(const QwwBreadCrumb);
    return d->iconSize;
}

QRect QwwBreadCrumb::visualRect(const QModelIndex & index) const
{
return QRect();
}

#endif
