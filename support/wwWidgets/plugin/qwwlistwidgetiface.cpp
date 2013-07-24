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
#include "qwwlistwidgetiface.h"
#include "qwwlistwidget.h"

QwwListWidgetIface::QwwListWidgetIface(QObject * parent) : wwWidgetInterface(parent) {
    setGroup("[ww] Item Widgets (Item-Based)");
    setDefaultGroup("Item Widgets (Item-Based)");
}


QIcon QwwListWidgetIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/listbox.png");
}

QWidget * QwwListWidgetIface::createWidget(QWidget * parent) {
    return new QwwListWidget(parent);
}
