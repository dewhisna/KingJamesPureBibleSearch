//
// C++ Interface: qwwlistwidgetiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLISTWIDGETIFACE_H
#define QWWLISTWIDGETIFACE_H

//
// C++ Interface: qwwlineeditiface
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

class QwwListWidgetIface : public wwWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwListWidgetIface(QObject *parent = 0);
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);
};


#endif
