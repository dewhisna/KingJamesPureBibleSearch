//
// C++ Interface: qwwrichtexteditiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWRICHTEXTEDITIFACE_H
#define QWWRICHTEXTEDITIFACE_H

#include "wwinterfaces.h"

class QwwRichTextEditIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwRichTextEditIface(QObject *parent = 0);
    ~QwwRichTextEditIface();
    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};

#endif
