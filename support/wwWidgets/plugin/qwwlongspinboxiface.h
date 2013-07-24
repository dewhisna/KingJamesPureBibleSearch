//
// C++ Interface: qwwlongspinboxiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLONGSPINBOXIFACE_H
#define QWWLONGSPINBOXIFACE_H

#ifndef WW_NO_SPINBOX
#include "wwinterfaces.h"

class QwwLongSpinBoxIface : public wwWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwLongSpinBoxIface(QObject *parent = 0);
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);
};

#endif // WW_NO_SPINBOX

#endif
