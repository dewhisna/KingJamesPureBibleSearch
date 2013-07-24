//
// C++ Interface: qwwbuttonlineeditiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWBUTTONLINEEDITIFACE_H
#define QWWBUTTONLINEEDITIFACE_H

#include "wwinterfaces.h"

class QwwButtonLineEditIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwButtonLineEditIface(QObject *parent = 0);
    ~QwwButtonLineEditIface();
    QWidget *createWidget(QWidget *parent);
};

#endif
