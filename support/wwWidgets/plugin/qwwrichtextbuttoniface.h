//
// C++ Interface: qwwrichtextbuttoniface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWRICHTEXTBUTTONIFACE_H
#define QWWRICHTEXTBUTTONIFACE_H

#include <wwinterfaces.h>

/**
	@author Witold Wysota <wysota@wysota.eu.org>
*/
class QwwRichTextButtonIface : public wwWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwRichTextButtonIface(QObject *parent = 0);

    ~QwwRichTextButtonIface();
    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};

#endif
