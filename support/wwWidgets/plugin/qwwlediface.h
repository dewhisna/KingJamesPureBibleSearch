//
// C++ Interface: qwwlediface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLEDIFACE_H
#define QWWLEDIFACE_H

#include "wwinterfaces.h"

class QwwLedIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwLedIface(QObject *parent = 0);
    ~QwwLedIface();
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);

};


#endif
