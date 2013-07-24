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
#include "qwwhuesatradialpickeriface.h"
#include "qwwhuesatradialpicker.h"

QwwHueSatRadialPickerIface::QwwHueSatRadialPickerIface(QObject *parent)
: wwWidgetInterface(parent){
    setGroup("[ww] Input Widgets");
    setDefaultGroup("Input Widgets");
}


QwwHueSatRadialPickerIface::~QwwHueSatRadialPickerIface() {}


QIcon QwwHueSatRadialPickerIface::icon() const {
    return QIcon(QPixmap(":/hsrpicker.png"));
}

QWidget * QwwHueSatRadialPickerIface::createWidget(QWidget * parent) {
    return new QwwHueSatRadialPicker(parent);
}
