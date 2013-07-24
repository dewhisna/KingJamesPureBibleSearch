//
// C++ Interface: qwwcolorbuttoniface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWCOLORBUTTONIFACE_H
#define QWWCOLORBUTTONIFACE_H

#include "wwinterfaces.h"
#include "qwwcolorbutton.h"
#include <QDesignerTaskMenuExtension>

class QwwColorButtonIface : public wwWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwColorButtonIface(QObject *parent = 0);
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);
};

class QwwColorButtonTaskMenuExtension: public QObject,
            public QDesignerTaskMenuExtension {
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension);
public:
    QwwColorButtonTaskMenuExtension(QwwColorButton *widget, QObject *parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void mySlot();

private:
    QwwColorButton *widget;
    QAction *editAction;
};

#endif
