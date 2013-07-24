//
// C++ Implementation: QwwTipWidgetIface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "qwwtipwidgetiface.h"
#include "qwwtipwidget.h"

QwwTipWidgetIface::QwwTipWidgetIface(QObject * parent) :wwWidgetInterface(parent) {
        setGroup("[ww] Display Widgets");
        setDefaultGroup("Display Widgets");
}

QIcon QwwTipWidgetIface::icon() const {
        return QPixmap(":/trolltech/formeditor/images/widgets/widget.png");
}

QWidget * QwwTipWidgetIface::createWidget(QWidget * parent) {
    QStringList slist;
    return new QwwTipWidget(slist, parent);
}
