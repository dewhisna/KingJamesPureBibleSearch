//
// C++ Interface: qwwresetlineeditiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWRESETLINEEDITIFACE_H
#define QWWRESETLINEEDITIFACE_H

#include "wwinterfaces.h"

class QwwResetLineEditIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwResetLineEditIface(QObject *parent = 0);
    ~QwwResetLineEditIface();
    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};

#endif
