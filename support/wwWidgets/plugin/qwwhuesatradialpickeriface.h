//
// C++ Interface: qwwnumpadiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWHUESATRADIALPICKERIFACE_H
#define QWWHUESATRADIALPICKERIFACE_H

#include "wwinterfaces.h"


class QwwHueSatRadialPickerIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwHueSatRadialPickerIface(QObject *parent = 0);
    ~QwwHueSatRadialPickerIface();
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);
};

#endif
