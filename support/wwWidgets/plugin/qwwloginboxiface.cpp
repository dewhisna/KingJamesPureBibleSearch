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
#include "qwwloginboxiface.h"
#include "qwwloginbox.h"

QwwLoginBoxIface::QwwLoginBoxIface(QObject *parent)
        : wwWidgetInterface(parent){
            setGroup("[ww] Input Widgets");
            setDefaultGroup("Input Widgets");
        }


QwwLoginBoxIface::~QwwLoginBoxIface() {}

QWidget * QwwLoginBoxIface::createWidget(QWidget * parent) {
    return new QwwLoginBox(parent);
}
