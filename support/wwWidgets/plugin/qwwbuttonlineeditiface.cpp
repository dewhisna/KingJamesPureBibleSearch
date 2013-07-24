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
#include "qwwbuttonlineeditiface.h"
#include "qwwbuttonlineedit.h"

QwwButtonLineEditIface::QwwButtonLineEditIface(QObject *parent)
        : wwWidgetInterface(parent){
                setGroup("[ww] Input Widgets");
                setDefaultGroup("Input Widgets");
        }


QwwButtonLineEditIface::~QwwButtonLineEditIface() {}

QWidget * QwwButtonLineEditIface::createWidget(QWidget * parent) {
    return new QwwButtonLineEdit(parent);
}
