//
// C++ Interface: qwwtipwidgetiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wwinterfaces.h"

class QwwTipWidgetIface : public wwWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwTipWidgetIface(QObject *parent = 0);
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);
};
