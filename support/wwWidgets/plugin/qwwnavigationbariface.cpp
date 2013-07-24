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
#include "qwwnavigationbariface.h"
#include "qwwnavigationbar.h"

#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

#include "wwwidgets.h"


QwwNavigationBarIface::QwwNavigationBarIface(QObject *parent)
        : wwWidgetInterface(parent) {
    setGroup("[ww] Containers");
    setDefaultGroup("Containers");
        }


QwwNavigationBarIface::~QwwNavigationBarIface() {}

QIcon QwwNavigationBarIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/toolbox.png");
}

void QwwNavigationBarIface::initialize(QDesignerFormEditorInterface * formEditor) {
    if (isInitialized())
        return;
    wwWidgetInterface::initialize(formEditor);
    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new QwwWidgetsExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));
    manager->registerExtensions(factory, Q_TYPEID(QDesignerPropertySheetExtension));
    manager->registerExtensions(factory, Q_TYPEID(QDesignerDynamicPropertySheetExtension));
}

QWidget * QwwNavigationBarIface::createWidget(QWidget * parent) {
    QwwNavigationBar *bar = new QwwNavigationBar(parent);
    connect(bar, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged(int)));
    return bar;
}


QwwNavigationBarContainerExtension::QwwNavigationBarContainerExtension(QwwNavigationBar *widget,
        QObject *parent)
        :QObject(parent) {
    myWidget = widget;
}

void QwwNavigationBarContainerExtension::addWidget(QWidget *widget) {
    myWidget->addWidget(widget);
}

int QwwNavigationBarContainerExtension::count() const {
    return myWidget->widgetCount();
}

int QwwNavigationBarContainerExtension::currentIndex() const {
    return myWidget->currentIndex();
}

void QwwNavigationBarContainerExtension::insertWidget(int index, QWidget *widget) {
    myWidget->insertWidget(index, widget);
}

void QwwNavigationBarContainerExtension::remove(int index) {
    myWidget->removeWidget(index);
}

void QwwNavigationBarContainerExtension::setCurrentIndex(int index) {
    myWidget->setCurrentIndex(index);
}

QWidget* QwwNavigationBarContainerExtension::widget(int index) const {
    return myWidget->widget(index);

}

QString QwwNavigationBarIface::domXml() const {
    QString xml;
#if QT_VERSION >= 0x040400
    xml+="<ui>\n";
#endif
    xml+="<widget class=\"QwwNavigationBar\" name=\"navigationBar\">\
         <property name=\"geometry\" >\
         <rect>\
         <x>0</x>\
         <y>0</y>\
         <width>150</width>\
         <height>200</height>\
         </rect>\
         </property>\
         <widget class=\"QWidget\" name=\"page1\" >\
         <property name=\"windowTitle\">\
         <string>Page 1</string>\
         </property>\
         </widget>\
         <widget class=\"QWidget\" name=\"page2\" >\
         <property name=\"windowTitle\">\
         <string>Page 2</string>\
         </property>\
         </widget>\
         </widget>\n";
#if QT_VERSION >= 0x040400
    xml+="<customwidgets>\
         <customwidget>\
         <class>QwwNavigationBar</class>\
         <extends>QWidget</extends>\
         <addpagemethod>addWidget</addpagemethod>\
         </customwidget>\
         </customwidgets>\
         </ui>";
#endif
    return xml;
}

QWidget * QwwNavigationBarSheetExtension::currentPage() const {
    QWidget *page = m_bar ? m_bar->widget(m_bar->currentIndex()) : 0;
    return page;
}


void QwwNavigationBarIface::currentIndexChanged(int index) {
    QwwNavigationBar *widget = qobject_cast<QwwNavigationBar*>(sender());
    if (widget) {
        QDesignerFormWindowInterface *form;
        form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}

bool QwwNavigationBarSheetExtension::isChanged(int index) const {
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

bool QwwNavigationBarSheetExtension::hasReset(int index) const {
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

QVariant QwwNavigationBarSheetExtension::property(int index) const {
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

void QwwNavigationBarSheetExtension::setAttribute(int index, bool attribute) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setAttribute(newIndex, attribute);
}

bool QwwNavigationBarSheetExtension::reset(int index) {
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

void QwwNavigationBarSheetExtension::setProperty(int index, const QVariant & value) {
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
            m_bar->setWidgetIcon(m_bar->currentIndex(), task->windowIcon());
        }
        if (myIndex == 2) {
            taskSheet->setProperty(taskSheet->indexOf("windowTitle"), value);
            m_bar->setWidgetLabel(m_bar->currentIndex(), task->windowTitle());
        }
        return;
    }
    wwSheetExtension::setProperty(index, value);
}

void QwwNavigationBarSheetExtension::setChanged(int index, bool changed) {
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

void QwwNavigationBarSheetExtension::setPropertyGroup(int index, const QString & group) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setPropertyGroup(newIndex, group);
}

void QwwNavigationBarSheetExtension::setVisible(int index, bool visible) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setVisible(newIndex, visible);
}

QwwNavigationBarSheetExtension::QwwNavigationBarSheetExtension(QExtensionManager *manager, QWidget *w,
        QObject *oth, QObject *parent)
        : wwSheetExtension(manager, QStringList() << "currentWidgetName"
                           << "currentWidgetIcon"
                           << "currentWidgetTitle",
                           qobject_cast<QDesignerPropertySheetExtension*>(oth),
#if QT_VERSION >= 0x040300
                           qobject_cast<QDesignerDynamicPropertySheetExtension*>(oth),
#endif
                           parent) {
    m_bar = qobject_cast<QwwNavigationBar*>(w);
}

QwwNavigationBarSheetExtension::~QwwNavigationBarSheetExtension() {}

void QwwNavigationBarSheetExtension::setPropertyChanged(QWidget * task, const QString & propName, const QVariant &value) {
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
