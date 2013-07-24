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
#ifndef QWWNUMPADIFACE_H
#define QWWNUMPADIFACE_H

#include "wwinterfaces.h"

class QwwNumPadIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwNumPadIface(QObject *parent = 0);
    ~QwwNumPadIface();
    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};

#endif
