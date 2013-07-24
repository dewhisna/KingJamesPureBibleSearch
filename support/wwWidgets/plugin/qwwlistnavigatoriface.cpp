//
// C++ Implementation: qwwlistnavigatoriface
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "qwwlistnavigatoriface.h"
#include "qwwlistnavigator.h"

QwwListNavigatorIface::QwwListNavigatorIface(QObject *parent) : wwWidgetInterface(parent){
    setGroup("[ww] Input Widgets");
    setDefaultGroup("Input Widgets");
}


QwwListNavigatorIface::~QwwListNavigatorIface() {}

QWidget * QwwListNavigatorIface::createWidget(QWidget * parent) {
    return new QwwListNavigator(parent);
}


QIcon QwwListNavigatorIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/hscrollbar.png");
}
