//
// C++ Interface: qwwtextspinboxiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWTEXTSPINBOXIFACE_H
#define QWWTEXTSPINBOXIFACE_H

#include "wwinterfaces.h"

class QwwTextSpinBoxIface : public wwWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwTextSpinBoxIface(QObject *parent = 0);
    ~QwwTextSpinBoxIface();
    QWidget *createWidget(QWidget *parent=0);
    QIcon icon() const;
};

#endif
