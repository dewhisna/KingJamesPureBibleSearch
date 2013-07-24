//
// C++ Interface: qwwloginboxiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLOGINBOXIFACE_H
#define QWWLOGINBOXIFACE_H

#include "wwinterfaces.h"

class QwwLoginBoxIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwLoginBoxIface(QObject *parent = 0);
    ~QwwLoginBoxIface();
    QWidget *createWidget(QWidget *parent);
};

#endif
