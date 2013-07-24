//
// C++ Interface: qwwlistwidget
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLISTWIDGET_H
#define QWWLISTWIDGET_H

#if defined(QT_NO_LISTWIDGET)
#define WW_NO_LISTWIDGET
#endif

#if !defined(WW_NO_LISTWIDGET)

#include <QListWidget>
#include <wwglobal.h>

class QwwListWidgetPrivate;
class Q_WW_EXPORT QwwListWidget : public QListWidget, public QwwPrivatable
{
    Q_OBJECT
public:
    QwwListWidget(QWidget *parent = 0);
signals:
    void currentAvailable(bool);
    void moveDownAvailable(bool);
    void moveUpAvailable(bool);
public slots:
    void moveCurrentDown();
    void moveCurrentUp();
    void removeCurrent();
    void setCurrentRow(int);
private:
    WW_DECLARE_PRIVATE(QwwListWidget);
    Q_PRIVATE_SLOT(d_func(), void _q_curCh(QListWidgetItem*));
};

#endif
#endif
