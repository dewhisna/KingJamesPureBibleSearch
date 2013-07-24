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
#include "qwwclearlineeditiface.h"
#include "qwwclearlineedit.h"

QwwClearLineEditIface::QwwClearLineEditIface(QObject *parent)
        : wwWidgetInterface(parent){
            setGroup("[ww] Input Widgets");
            setDefaultGroup("Input Widgets");
        }


QwwClearLineEditIface::~QwwClearLineEditIface() {}

QWidget * QwwClearLineEditIface::createWidget(QWidget * parent) {
    return new QwwClearLineEdit(parent);
}

QIcon QwwClearLineEditIface::icon() const
{
    return QPixmap(":/closetab.png");
}
