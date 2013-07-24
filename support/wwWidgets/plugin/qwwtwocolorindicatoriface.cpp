//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/QDesignerFormWindowCursorInterface>

#include "qwwtwocolorindicatoriface.h"
#include "qwwtwocolorindicator.h"
#include "wwwidgets.h"
#include <QColorDialog>

QwwTwoColorIndicatorIface::QwwTwoColorIndicatorIface(QObject *parent)
        : wwWidgetInterface(parent) {
            setGroup("[ww] Display Widgets");
            setDefaultGroup("Display Widgets");
        }


QwwTwoColorIndicatorIface::~QwwTwoColorIndicatorIface() {}


QWidget * QwwTwoColorIndicatorIface::createWidget(QWidget * parent) {
    return new QwwTwoColorIndicator(parent);
}

QIcon QwwTwoColorIndicatorIface::icon() const {
    return QPixmap(":/twocolor.png");
}

void QwwTwoColorIndicatorIface::initialize(QDesignerFormEditorInterface * formEditor) {
    if (isInitialized())
        return;
    wwWidgetInterface::initialize(formEditor);
    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new QwwWidgetsExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerTaskMenuExtension));
}

QwwTwoColorIndicatorTaskMenuExtension::QwwTwoColorIndicatorTaskMenuExtension(QwwTwoColorIndicator * widget, QObject * parent) : QObject(parent) {
    this->widget = widget;
    fgAction = new QAction(tr("Edit foreground color..."), this);
    connect(fgAction, SIGNAL(triggered()), this, SLOT(editFg()));
    bgAction = new QAction(tr("Edit background color..."), this);
    connect(bgAction, SIGNAL(triggered()), this, SLOT(editBg()));
    swapAction = new QAction(tr("Swap colors"), this);
    connect(swapAction, SIGNAL(triggered()), this, SLOT(swap()));
}

QAction * QwwTwoColorIndicatorTaskMenuExtension::preferredEditAction() const {
    return fgAction;
}

void QwwTwoColorIndicatorTaskMenuExtension::editFg() {
    QColor c = QColorDialog::getColor(widget->fgColor(), widget->window());
    if (c.isValid()) {
        if (QDesignerFormWindowInterface *formWindow
                = QDesignerFormWindowInterface::findFormWindow(widget)) {
            formWindow->cursor()->setProperty("fgColor", c);
        }
    }
}

void QwwTwoColorIndicatorTaskMenuExtension::editBg() {
    QColor c = QColorDialog::getColor(widget->bgColor(), widget->window());
    if (c.isValid()) {
        if (QDesignerFormWindowInterface *formWindow
                = QDesignerFormWindowInterface::findFormWindow(widget)) {
            formWindow->cursor()->setProperty("bgColor", c);
        }
    }
}

void QwwTwoColorIndicatorTaskMenuExtension::swap() {
    widget->switchColors();
//     if (QDesignerFormWindowInterface *formWindow
//                 = QDesignerFormWindowInterface::findFormWindow(widget)) {
//         formWindow->setWindowModified(true);
//     }
}

QList< QAction * > QwwTwoColorIndicatorTaskMenuExtension::taskActions() const {
    QList<QAction *> a;
    a << fgAction << bgAction << swapAction;
    return a;
}
