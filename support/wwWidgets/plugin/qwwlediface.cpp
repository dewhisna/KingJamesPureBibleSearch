//
// C++ Implementation: qwwlediface
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "qwwlediface.h"
#include "qwwled.h"

QwwLedIface::QwwLedIface(QObject *parent) : wwWidgetInterface(parent){
    setGroup("[ww] Display Widgets");
    setDefaultGroup("Display Widgets");
}


QwwLedIface::~QwwLedIface() {}

QWidget * QwwLedIface::createWidget(QWidget * parent) {
    return new QwwLed(parent);
}

QIcon QwwLedIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/radiobutton.png");
}




