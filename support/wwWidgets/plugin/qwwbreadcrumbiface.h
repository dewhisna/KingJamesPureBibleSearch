//
// C++ Interface: qwwbreadcrumbiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWBREADCRUMBIFACE_H
#define QWWBREADCRUMBIFACE_H

#include "wwinterfaces.h"

class QwwBreadCrumbIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwBreadCrumbIface(QObject *parent = 0);
    ~QwwBreadCrumbIface();
    QWidget *createWidget(QWidget *parent);
};

#endif
