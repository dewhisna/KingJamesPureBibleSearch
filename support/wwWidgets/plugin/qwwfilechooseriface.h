//
// C++ Interface: qwwfilechooseriface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWFILECHOOSERIFACE_H
#define QWWFILECHOOSERIFACE_H

#include "wwinterfaces.h"


class QwwFileChooserIface : public wwWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwFileChooserIface(QObject *parent = 0);

    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};

#endif
