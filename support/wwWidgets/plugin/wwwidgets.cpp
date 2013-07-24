//
// C++ Implementation: wwWidgets Designer plugin
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2007-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>

#include <QMainWindow>
#include <QMenuBar>

#include "wwwidgets.h"
#include "qwwtipwidgetiface.h"
#include "qwwlistwidgetiface.h"
#include "qwwlongspinboxiface.h"
#include "qwwtwocolorindicatoriface.h"
#include "qwwcolorcomboboxiface.h"
#include "qwwnavigationbariface.h"
#include "qwwnumpadiface.h"
#include "qwwcolorbuttoniface.h"
#include "qwwcolorcomboboxiface.h"
#include "qwwtaskpaneliface.h"
#include "qwwhuesatpickeriface.h"
#include "qwwhuesatradialpickeriface.h"
#include "qwwconfigwidgetiface.h"
#include "qwwtextspinboxiface.h"
#include "qwwrichtextbuttoniface.h"
#include "qwwfilechooseriface.h"
#include "qwwloginboxiface.h"
#include "qwwbreadcrumbiface.h"
#include "qwwbuttonlineeditiface.h"
#include "qwwclearlineeditiface.h"
#include "qwwresetlineeditiface.h"
#include "qwwrichtexteditiface.h"
#include "qwwlistnavigatoriface.h"
#include "qwwlediface.h"

#include <QMutex>
#include <QSettings>
QMutex cheatmutex;

#define WW_EXPOSE(x) widgets << new x(this)


/**
 * @mainpage wwWidgets
 *
 * @section Overview
 *
 * wwWidgets is a professional set of useful widgets for Qt 4. It consists of several different widgets that 
 * are either enhanced versions of widgets bundled with Qt or completely new ones that implement functionality 
 * not available in Qt including custom multipage container widgets that can hold other widgets.
 * 
 * The classes follow all guidelines for building new widgets. Thanks to that they can be used with different widget
 * styles (like Plastique or WindowsXP) and they are easily stylable using Qt style sheets allowing a perfect blend 
 * with the rest of your application.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */



wwWidgets::wwWidgets(QObject *parent)
        : QObject(parent) {
    WW_EXPOSE(QwwTipWidgetIface);
    WW_EXPOSE(QwwListWidgetIface);
#ifndef WW_NO_SPINBOX
    WW_EXPOSE(QwwLongSpinBoxIface);
#endif
    WW_EXPOSE(QwwTextSpinBoxIface);
#ifndef WW_NO_TWOCOLORINDICATOR
    WW_EXPOSE(QwwTwoColorIndicatorIface);
#endif
#ifndef WW_NO_COLORCOMBOBOX
    WW_EXPOSE(QwwColorComboBoxIface);
#endif
#ifndef WW_NO_NAVIGATIONBAR
    WW_EXPOSE(QwwNavigationBarIface);
#endif
    WW_EXPOSE(QwwNumPadIface);
    WW_EXPOSE(QwwColorButtonIface);
#ifndef WW_NO_TASKPANEL
    WW_EXPOSE(QwwTaskPanelIface);
#endif
    WW_EXPOSE(QwwHueSatPickerIface);
    WW_EXPOSE(QwwHueSatRadialPickerIface);
    WW_EXPOSE(QwwConfigWidgetIface);
    WW_EXPOSE(QwwRichTextButtonIface);
#ifndef WW_NO_FILECHOOSER
    WW_EXPOSE(QwwFileChooserIface);
#endif
#ifndef WW_NO_LOGINBOX
    WW_EXPOSE(QwwLoginBoxIface);
#endif
#ifndef WW_NO_BREADCRUMB
//    WW_EXPOSE(QwwBreadCrumbIface);
#endif
#ifndef WW_NO_BUTTONLINEEDIT
    WW_EXPOSE(QwwButtonLineEditIface);
    WW_EXPOSE(QwwClearLineEditIface);
    WW_EXPOSE(QwwResetLineEditIface);
#endif
    WW_EXPOSE(QwwRichTextEditIface);
    WW_EXPOSE(QwwListNavigatorIface);
    WW_EXPOSE(QwwLedIface);


    //QMetaObject::invokeMethod(this, "registerMenu");
    QTimer::singleShot(0, this, SLOT(registerMenu()));
}

QList< QDesignerCustomWidgetInterface * > wwWidgets::customWidgets( ) const {
    return widgets;
}

void wwWidgets::registerMenu(){
    if(QApplication::applicationName()=="Designer") {
        foreach(QWidget *win, qApp->topLevelWidgets()){
            QMainWindow *mw = qobject_cast<QMainWindow*>(win);
            if (!mw) continue;
            wwWidgetsMenu *menu = new wwWidgetsMenu("wwWidgets");
            mw->menuBar()->addMenu(menu);
        }
    }
}

Q_EXPORT_PLUGIN2(wwwidgets, wwWidgets)


void wwWidgetsMenu::about() {
    QDialog dlg;
    Ui::About ui;
    ui.setupUi(&dlg);
    dlg.setFixedSize(dlg.sizeHint());
    QSettings settings("wysota.eu.org", "wwWidgetsPlugin");
    int pol = settings.value("GroupPolicy", 0).toInt();
    ui.groupPolicy->setCurrentIndex(pol);
    dlg.exec();
    settings.setValue("GroupPolicy", ui.groupPolicy->currentIndex());
}

wwWidgetsMenu::wwWidgetsMenu(const QString & title, QWidget * parent) : QMenu(title, parent) {
    addAction("About", this, SLOT(about()));
}


QwwWidgetsExtensionFactory::QwwWidgetsExtensionFactory(QExtensionManager *manager ) {
    m_manager = manager;
}

#define WW_REGISTER_CONTAINEREXTENSION(cl, object, parent) \
    if(cl *widget = qobject_cast<cl*>(object)) \
        return new cl ## ContainerExtension(widget, parent);

QObject * QwwWidgetsExtensionFactory::createExtension(QObject * object, const QString & iid, QObject * parent) const {

    if (iid == Q_TYPEID(QDesignerContainerExtension)) {
#ifndef WW_NO_TASKPANEL
        WW_REGISTER_CONTAINEREXTENSION(QwwTaskPanel, object, parent);
#endif
#ifndef WW_NO_NAVIGATIONBAR
        WW_REGISTER_CONTAINEREXTENSION(QwwNavigationBar, object, parent);
#endif
#ifndef WW_NO_CONFIGWIDGET
        WW_REGISTER_CONTAINEREXTENSION(QwwConfigWidget, object, parent);
#endif
        return 0;
    } else if (iid == Q_TYPEID(QDesignerPropertySheetExtension)
    #if QT_VERSION >= 0x040300
        || iid == Q_TYPEID(QDesignerDynamicPropertySheetExtension)
    #endif
    ) {
        if (cheatmutex.tryLock()) {
#ifndef WW_NO_TASKPANEL
            if (QwwTaskPanel *widget = qobject_cast<QwwTaskPanel*>(object) ) {
                QObject *otherExtension = m_manager->extension(object, iid);
                cheatmutex.unlock();
                return new QwwTaskPanelSheetExtension(m_manager, widget, otherExtension, parent);
            }
#endif
#ifndef WW_NO_NAVIGATIONBAR
            if (QwwNavigationBar *widget = qobject_cast<QwwNavigationBar*>(object) ){
                QObject *otherExtension = m_manager->extension(object, iid);
                cheatmutex.unlock();
                return new QwwNavigationBarSheetExtension(m_manager, widget, otherExtension, parent);
            }
#endif
#ifndef WW_NO_CONFIGWIDGET
            if (QwwConfigWidget *widget = qobject_cast<QwwConfigWidget*>(object) ){
                QObject *otherExtension = m_manager->extension(object, iid);
                cheatmutex.unlock();
                return new QwwConfigWidgetSheetExtension(m_manager, widget, otherExtension, parent);
            }
#endif
            cheatmutex.unlock();
        }
    } else if(iid == Q_TYPEID(QDesignerTaskMenuExtension)){
            if(QwwColorButton *widget = qobject_cast<QwwColorButton*>(object)){
                return new QwwColorButtonTaskMenuExtension(widget, parent);
            }
#ifndef WW_NO_COLORCOMBOBOX
            if(QwwColorComboBox *widget = qobject_cast<QwwColorComboBox*>(object)){
                return new QwwColorComboBoxTaskMenuExtension(widget, parent);
            }
#endif
#ifndef WW_NO_TWOCOLORINDICATOR
            if(QwwTwoColorIndicator *widget = qobject_cast<QwwTwoColorIndicator*>(object)){
                return new QwwTwoColorIndicatorTaskMenuExtension(widget, parent);
            }
#endif
    }
    return 0;
}

