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
#include "qwwtextspinboxiface.h"
#include "qwwtextspinbox.h"

QwwTextSpinBoxIface::QwwTextSpinBoxIface(QObject *parent)
        : wwWidgetInterface(parent) {
    setGroup("[ww] Input Widgets");
    setDefaultGroup("Input Widgets");
        }


QwwTextSpinBoxIface::~QwwTextSpinBoxIface() {}

QWidget * QwwTextSpinBoxIface::createWidget(QWidget * parent) {
    return new QwwTextSpinBox(parent);
}

QIcon QwwTextSpinBoxIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/spinbox.png");
}


