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
#include "qwwconfigwidgetiface.h"
#include "qwwconfigwidget.h"

#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

#include "wwwidgets.h"

const int cw_propcnt = 3;

QwwConfigWidgetIface::QwwConfigWidgetIface(QObject *parent)
        : QObject(parent) {
    initialized = false;
}


QwwConfigWidgetIface::~QwwConfigWidgetIface() {}

bool QwwConfigWidgetIface::isInitialized() const {
    return initialized;
}

void QwwConfigWidgetIface::initialize(QDesignerFormEditorInterface * formEditor) {
    if (initialized)
        return;

    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new QwwWidgetsExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));
    manager->registerExtensions(factory, Q_TYPEID(QDesignerPropertySheetExtension));
    initialized = true;


}

QWidget * QwwConfigWidgetIface::createWidget(QWidget * parent) {
    QwwConfigWidget *bar = new QwwConfigWidget(parent);
    connect(bar, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged(int)));
    return bar;
}

QString QwwConfigWidgetIface::whatsThis() const {
    return "";
}

QString QwwConfigWidgetIface::domXml() const {
    QString xml;
#if QT_VERSION >= 0x040400
    xml += "<ui>\n";
#endif
    xml += "<widget class=\"QwwConfigWidget\" name=\"configWidget\">\
           <widget class=\"QWidget\" name=\"module1\">\
           <property name=\"windowTitle\">\
           <string>Module 1</string>\
           </property>\
           </widget>\
           <widget class=\"QWidget\" name=\"module2\">\
           <property name=\"windowTitle\">\
           <string>Module 2</string>\
           </property>\
           </widget>\
           </widget>\n";
#if QT_VERSION >= 0x040400
    xml +="<customwidgets>\
          <customwidget>\
          <class>QwwConfigWidget</class>\
          <extends>QWidget</extends>\
          <addpagemethod>addGroup</addpagemethod>\
          </customwidget>\
          </customwidgets>\
          </ui>";
#endif
    return xml;
}

QString QwwConfigWidgetIface::toolTip() const {
    return "";
}

QString QwwConfigWidgetIface::includeFile() const {
    return "qwwconfigwidget.h";
}

QIcon QwwConfigWidgetIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/groupbox.png");
}



QwwConfigWidgetContainerExtension::QwwConfigWidgetContainerExtension(QwwConfigWidget *widget,
        QObject *parent)
        :QObject(parent) {
    myWidget = widget;
}

void QwwConfigWidgetContainerExtension::addWidget(QWidget *widget) {
    myWidget->addGroup(widget);
}

int QwwConfigWidgetContainerExtension::count() const {
    return myWidget->count();
}

int QwwConfigWidgetContainerExtension::currentIndex() const {
    return myWidget->currentIndex();
}

void QwwConfigWidgetContainerExtension::insertWidget(int index, QWidget *widget) {
    myWidget->insertGroup(index, widget);
}

void QwwConfigWidgetContainerExtension::remove(int index) {
    myWidget->removeGroup(index);
}

void QwwConfigWidgetContainerExtension::setCurrentIndex(int index) {
    myWidget->setCurrentIndex(index);
}

QWidget* QwwConfigWidgetContainerExtension::widget(int index) const {
    return myWidget->group(index);

}


void QwwConfigWidgetIface::currentIndexChanged(int index) {
    QwwConfigWidget *widget = qobject_cast<QwwConfigWidget*>(sender());
    if (widget) {
        QDesignerFormWindowInterface *form;
        form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}

QWidget * QwwConfigWidgetSheetExtension::currentPage() const {
    QWidget *page = m_bar ? m_bar->currentGroup() : 0;
    return page;
}


bool QwwConfigWidgetSheetExtension::isChanged(int index) const {
    if (index >= beginCount() && index < beginCount() + names().count()) {
        int myIndex = index - beginCount();
        QWidget *task = currentPage();
        if (!task)
            return false;
        QDesignerPropertySheetExtension *taskSheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        if (!taskSheet)
            return false;
        if (myIndex == 0)
            return taskSheet->isChanged(taskSheet->indexOf("objectName"));
        if (myIndex == 1)
            return taskSheet->isChanged(taskSheet->indexOf("windowIcon"));
        if (myIndex == 2)
            return taskSheet->isChanged(taskSheet->indexOf("windowTitle"));
        return false;
    }
    return wwSheetExtension::isChanged(index);
}

bool QwwConfigWidgetSheetExtension::hasReset(int index) const {
    if (index >= beginCount() && index < beginCount() + names().count()) {
        int myIndex = index - beginCount();
        QWidget *task = currentPage();
        if (!task)
            return false;
        QDesignerPropertySheetExtension *taskSheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        if (!taskSheet)
            return false;
        if (myIndex == 0)
            return taskSheet->hasReset(taskSheet->indexOf("objectName"));
        if (myIndex == 1)
            return taskSheet->hasReset(taskSheet->indexOf("windowIcon"));
        if (myIndex == 2)
            return taskSheet->hasReset(taskSheet->indexOf("windowTitle"));
        return false;
    }
    return wwSheetExtension::hasReset(index);
}

QVariant QwwConfigWidgetSheetExtension::property(int index) const {
    if (index >= beginCount() && index < beginCount() + names().count()) {
        int myIndex = index - beginCount();
        QWidget *task = currentPage();
        if (!task)
            return QVariant();
        QDesignerPropertySheetExtension *taskSheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        if (!taskSheet)
            return QVariant();
        if (myIndex == 0)
            return taskSheet->property(taskSheet->indexOf("objectName"));
        if (myIndex == 1)
            return taskSheet->property(taskSheet->indexOf("windowIcon"));
        if (myIndex == 2)
            return taskSheet->property(taskSheet->indexOf("windowTitle"));
        return QVariant();
    }
    return wwSheetExtension::property(index);
}

void QwwConfigWidgetSheetExtension::setAttribute(int index, bool attribute) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setAttribute(newIndex, attribute);
}

bool QwwConfigWidgetSheetExtension::reset(int index) {
    if (index >= beginCount() && index < beginCount() + names().count()) {
        int myIndex = index - beginCount();
        QWidget *task = currentPage();
        if (!task)
            return false;
        QDesignerPropertySheetExtension *taskSheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        if (!taskSheet)
            return false;
        if (myIndex == 0)
            return taskSheet->reset(taskSheet->indexOf("objectName"));
        if (myIndex == 1)
            return taskSheet->reset(taskSheet->indexOf("windowIcon"));
        if (myIndex == 2)
            return taskSheet->reset(taskSheet->indexOf("windowTitle"));
        return false;
    }
    return wwSheetExtension::reset(index);

}

void QwwConfigWidgetSheetExtension::setProperty(int index, const QVariant & value) {
    if (index >= beginCount() && index < beginCount() + names().count()) {
        int myIndex = index - beginCount();
        QWidget *task = currentPage();
        if (!task)
            return;
        QDesignerPropertySheetExtension *taskSheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        if (!taskSheet)
            return;
        if (myIndex == 0) {
            taskSheet->setProperty(taskSheet->indexOf("objectName"), value);
        }
        if (myIndex == 1) {
            taskSheet->setProperty(taskSheet->indexOf("windowIcon"), value);
            m_bar->setGroupIcon(m_bar->currentIndex(), task->windowIcon());
        }
        if (myIndex == 2) {
            taskSheet->setProperty(taskSheet->indexOf("windowTitle"), value);
            m_bar->setGroupLabel(m_bar->currentIndex(), task->windowTitle());
        }
        return;
    }
    wwSheetExtension::setProperty(index, value);
}

void QwwConfigWidgetSheetExtension::setChanged(int index, bool changed) {
    if (index >= beginCount() && index < beginCount() + names().count()) {
        int myIndex = index - beginCount();
        QWidget *task = currentPage();
        if (!task)
            return;
        QDesignerPropertySheetExtension *taskSheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        if (!taskSheet)
            return;
        if (myIndex == 0)
            taskSheet->setChanged(taskSheet->indexOf("objectName"), changed);
        if (myIndex == 1)
            taskSheet->setChanged(taskSheet->indexOf("windowIcon"), changed);
        if (myIndex == 2)
            taskSheet->setChanged(taskSheet->indexOf("windowTitle"), changed);
        return;
    }
    wwSheetExtension::setChanged(index, changed);
}

void QwwConfigWidgetSheetExtension::setPropertyGroup(int index, const QString & group) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setPropertyGroup(newIndex, group);
}

void QwwConfigWidgetSheetExtension::setVisible(int index, bool visible) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setVisible(newIndex, visible);
}

QwwConfigWidgetSheetExtension::QwwConfigWidgetSheetExtension(QExtensionManager *manager, QWidget *w,
        QObject *oth, QObject *parent)
        : wwSheetExtension(manager, QStringList() << "currentWidgetName"
                           << "currentWidgetIcon"
                           << "currentWidgetTitle",
                           qobject_cast<QDesignerPropertySheetExtension*>(oth),
#if QT_VERSION >= 0x040300
                           qobject_cast<QDesignerDynamicPropertySheetExtension*>(oth),
#endif
                           parent) {
    m_bar = qobject_cast<QwwConfigWidget*>(w);
}

QwwConfigWidgetSheetExtension::~QwwConfigWidgetSheetExtension() {}

void QwwConfigWidgetSheetExtension::setPropertyChanged(QWidget * task, const QString & propName, const QVariant &value) {
    QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(m_bar);
    if (form && task) {
        QDesignerPropertySheetExtension *sheet;
        sheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        int propertyIndex = sheet->indexOf(propName);
        if (value.isValid())
            sheet->setProperty(propertyIndex, value);
        sheet->setChanged(propertyIndex, true);
    }
}
