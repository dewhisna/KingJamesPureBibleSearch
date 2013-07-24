//
// C++ Interface: qwwcolorcomboboxiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWCOLORCOMBOBOXIFACE_H
#define QWWCOLORCOMBOBOXIFACE_H
#ifndef WW_NO_COLORCOMBOBOX

#include "wwinterfaces.h"
#include <QDesignerTaskMenuExtension>
#include "qwwcolorcombobox.h"

class QwwColorComboBoxIface : public wwWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwColorComboBoxIface(QObject *parent = 0);
    QIcon icon() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);
};

class QwwColorComboBoxTaskMenuExtension: public QObject,
            public QDesignerTaskMenuExtension {
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension);
public:
    QwwColorComboBoxTaskMenuExtension(QwwColorComboBox *widget, QObject *parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void mySlot();

private:
    QwwColorComboBox *widget;
    QAction *editAction;
};


#endif
#endif
