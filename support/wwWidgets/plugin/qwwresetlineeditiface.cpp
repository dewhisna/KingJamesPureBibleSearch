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
#include "qwwresetlineeditiface.h"
#include "qwwresetlineedit.h"

QwwResetLineEditIface::QwwResetLineEditIface(QObject *parent)
        : wwWidgetInterface(parent){
    setGroup("[ww] Input Widgets");
    setDefaultGroup("Input Widgets");
        }

QwwResetLineEditIface::~QwwResetLineEditIface() {}

QWidget * QwwResetLineEditIface::createWidget(QWidget * parent) {
    return new QwwResetLineEdit(parent);
}

QIcon QwwResetLineEditIface::icon() const
{
    return QPixmap(":/wrap.png");
}
