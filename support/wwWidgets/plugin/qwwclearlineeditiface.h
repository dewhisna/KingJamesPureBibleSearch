//
// C++ Interface: qwwclearlineeditiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWCLEARLINEEDITIFACE_H
#define QWWCLEARLINEEDITIFACE_H

#include "wwinterfaces.h"

class QwwClearLineEditIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwClearLineEditIface(QObject *parent = 0);
    ~QwwClearLineEditIface();
    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};

#endif
