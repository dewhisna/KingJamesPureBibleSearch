//
// C++ Interface: qwwlistnavigatoriface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLISTNAVIGATORIFACE_H
#define QWWLISTNAVIGATORIFACE_H

#include "wwinterfaces.h"

class QwwListNavigatorIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwListNavigatorIface(QObject *parent = 0);
    ~QwwListNavigatorIface();

    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};


#endif
