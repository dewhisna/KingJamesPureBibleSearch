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
#ifndef QWWHUESATPICKERIFACE_H
#define QWWHUESATPICKERIFACE_H

#include "wwinterfaces.h"


class QwwHueSatPickerIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwHueSatPickerIface(QObject *parent = 0);
    ~QwwHueSatPickerIface();
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);
};

#endif
