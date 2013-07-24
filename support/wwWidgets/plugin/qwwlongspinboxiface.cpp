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
#include "qwwlongspinboxiface.h"
#include "qwwlongspinbox.h"

#ifndef WW_NO_SPINBOX

QwwLongSpinBoxIface::QwwLongSpinBoxIface(QObject *parent)
        : wwWidgetInterface(parent) {
            setGroup("[ww] Input Widgets");
            setDefaultGroup("Input Widgets");
        }

QIcon QwwLongSpinBoxIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/spinbox.png");
}

QWidget * QwwLongSpinBoxIface::createWidget(QWidget * parent) {
    return new QwwLongSpinBox(parent);
}

#endif // WW_NO_SPINBOX
