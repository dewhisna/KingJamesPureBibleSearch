//
// C++ Interface: qwwbreadcrumb
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWBREADCRUMB_H
#define QWWBREADCRUMB_H

#ifndef WW_NO_BREADCRUMB

#include <QWidget>
#include <wwglobal.h>
#include <QModelIndex>

class QAbstractItemModel;
class QAbstractItemDelegate;
class QwwBreadCrumbPrivate;
class Q_WW_EXPORT QwwBreadCrumb : public QWidget, public QwwPrivatable {
    Q_OBJECT
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
public:
    QwwBreadCrumb(QWidget *parent = 0);
    virtual void setModel(QAbstractItemModel *m);
    void setItemDelegate(QAbstractItemDelegate *deleg);
    virtual QModelIndex indexAt(const QPoint &pt);
    QAbstractItemDelegate *itemDelegate() const;
    QAbstractItemModel *model() const;

    QModelIndex rootIndex() const;

    QModelIndex currentIndex() const;
    QSize sizeHint() const;
    QSize iconSize() const;
    virtual QRect visualRect ( const QModelIndex & index ) const;
public slots:
    void setRootIndex(const QModelIndex &);
    void setCurrentIndex(const QModelIndex &);
    void setIconSize(const QSize &s);
protected:
    void paintEvent(QPaintEvent *event);
private:
    WW_DECLARE_PRIVATE(QwwBreadCrumb);
};

#endif
#endif
