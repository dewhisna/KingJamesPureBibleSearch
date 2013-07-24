//
// C++ Interface: qwwtwocolorindicatoriface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWTWOCOLORINDICATORIFACE_H
#define QWWTWOCOLORINDICATORIFACE_H

#include "wwinterfaces.h"
#include "qwwtwocolorindicator.h"
#include <QDesignerTaskMenuExtension>

class QwwTwoColorIndicatorIface : public wwWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwTwoColorIndicatorIface(QObject *parent = 0);
    ~QwwTwoColorIndicatorIface();
    void initialize(QDesignerFormEditorInterface *core);
    QWidget *createWidget(QWidget *parent);
    QIcon icon() const;
};

class QwwTwoColorIndicatorTaskMenuExtension: public QObject,
            public QDesignerTaskMenuExtension {
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension);
public:
    QwwTwoColorIndicatorTaskMenuExtension(QwwTwoColorIndicator *widget, QObject *parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void editBg();
    void editFg();
    void swap();

private:
    QwwTwoColorIndicator *widget;
    QAction *fgAction, *bgAction, *swapAction;
};

#endif
