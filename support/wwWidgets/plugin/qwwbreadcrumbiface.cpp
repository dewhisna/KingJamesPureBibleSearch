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
#include "qwwbreadcrumbiface.h"
#include "qwwbreadcrumb.h"

QwwBreadCrumbIface::QwwBreadCrumbIface(QObject *parent)
        : wwWidgetInterface(parent){}


QwwBreadCrumbIface::~QwwBreadCrumbIface() {}

QWidget * QwwBreadCrumbIface::createWidget(QWidget * parent) {
    return new QwwBreadCrumb(parent);
}
