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
#include "qwwhuesatpickeriface.h"
#include "qwwhuesatpicker.h"
#include <QSettings>

QwwHueSatPickerIface::QwwHueSatPickerIface(QObject *parent)
: wwWidgetInterface(parent){
    setGroup("[ww] Input Widgets");
    setDefaultGroup("Input Widgets");
}

QwwHueSatPickerIface::~QwwHueSatPickerIface() {}

QIcon QwwHueSatPickerIface::icon() const {
    return QIcon(QPixmap(":/hspicker.png"));
}


QWidget * QwwHueSatPickerIface::createWidget(QWidget * parent) {
    return new QwwHueSatPicker(parent);
}
