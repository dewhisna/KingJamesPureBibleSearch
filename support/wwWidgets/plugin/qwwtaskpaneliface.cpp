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
#ifndef WW_NO_TASKPANEL

#include "qwwtaskpaneliface.h"
#include "qwwtaskpanel.h"
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

#include "qwwtaskpanel/qwwtaskpanel_p.h"
#include <QChildEvent>
#include <QMainWindow>
#include <QMenuBar>

#include "wwwidgets.h"
// #include "propertymatcher.h"

QwwTaskPanelIface::QwwTaskPanelIface(QObject *parent)
        : wwWidgetInterface(parent) {
            setGroup("[ww] Containers");
            setDefaultGroup("Containers");
}


QwwTaskPanelIface::~QwwTaskPanelIface() {}


QWidget * QwwTaskPanelIface::createWidget(QWidget * parent) {
    QwwTaskPanel *panel = new QwwTaskPanel(parent);
    connect(panel, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged(int)));
    QWidget *w = qFindChild<QWidget*>(panel, "ww_taskpanel_panel");
    if (w) {
        w->installEventFilter(this);
    }
    return panel;
}

QIcon QwwTaskPanelIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/widgetstack.png");
}

void QwwTaskPanelIface::initialize(QDesignerFormEditorInterface * core) {
    if (isInitialized())
        return;
    wwWidgetInterface::initialize(core);
    formEditor = core;
    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new QwwWidgetsExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));
    manager->registerExtensions(factory, Q_TYPEID(QDesignerPropertySheetExtension));
}

QwwTaskPanelContainerExtension::QwwTaskPanelContainerExtension(QwwTaskPanel * widget, QObject *parent ) : QObject(parent) {
    myWidget = widget;
}

QWidget * QwwTaskPanelContainerExtension::widget(int index) const {
    return myWidget->task(index);
}

void QwwTaskPanelContainerExtension::setCurrentIndex(int index) {
    myWidget->setCurrentIndex(index);
}

void QwwTaskPanelContainerExtension::remove(int index) {
    myWidget->removeTask(index);
}

void QwwTaskPanelContainerExtension::insertWidget(int index, QWidget * widget) {
    myWidget->insertTask(index, widget);
    widget->setMinimumHeight(100);
}

int QwwTaskPanelContainerExtension::currentIndex() const {
    return myWidget->currentIndex();
}

int QwwTaskPanelContainerExtension::count() const {
    return myWidget->taskCount();
}

void QwwTaskPanelContainerExtension::addWidget(QWidget * widget) {
    myWidget->addTask(widget);
    widget->setMinimumHeight(100);
}

QString QwwTaskPanelIface::domXml() const {
    QString xml;
#if QT_VERSION >= 0x040400
    xml+="<ui>\n";
#endif
    xml+="<widget class=\"QwwTaskPanel\" name=\"taskPanel\">\
         <widget class=\"QWidget\" name=\"task1\" >\
         <property name=\"windowTitle\">\
         <string>Task 1</string>\
         </property>\
         </widget>\
         <widget class=\"QWidget\" name=\"task2\" >\
         <property name=\"windowTitle\">\
         <string>Task 2</string>\
         </property>\
         </widget>\
         </widget>\n";
#if QT_VERSION >= 0x040400
    xml+="<customwidgets>\
         <customwidget>\
         <class>QwwTaskPanel</class>\
         <extends>QWidget</extends>\
         <addpagemethod>addTask</addpagemethod>\
         </customwidget>\
         </customwidgets>\
         </ui>";
#endif
    return xml;
}

void QwwTaskPanelIface::currentIndexChanged(int ) {
    QwwTaskPanel *widget = qobject_cast<QwwTaskPanel*>(sender());
    if (widget) {
        QDesignerFormWindowInterface *form;
        form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}

bool QwwTaskPanelIface::eventFilter(QObject * , QEvent * e) {
//     if(e->type() == QEvent::ChildAdded){
//         QChildEvent *chev = static_cast<QChildEvent*>(e);
//         Task *tsk = qobject_cast<Task*>(chev->child());
//         if(tsk){
//             tsk->body()->setMinimumHeight(200);
//         }
//     }
    return false;
}




bool QwwTaskPanelSheetExtension::isChanged(int index) const {
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


bool QwwTaskPanelSheetExtension::hasReset(int index) const {
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


QVariant QwwTaskPanelSheetExtension::property(int index) const {
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

void QwwTaskPanelSheetExtension::setAttribute(int index, bool attribute) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setAttribute(newIndex, attribute);
}

bool QwwTaskPanelSheetExtension::reset(int index) {
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

void QwwTaskPanelSheetExtension::setProperty(int index, const QVariant & value) {
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
            m_panel->setTaskName(m_panel->currentIndex(), task->objectName());
        }
        if (myIndex == 1) {
            taskSheet->setProperty(taskSheet->indexOf("windowIcon"), value);
            m_panel->setTaskIcon(m_panel->currentIndex(), task->windowIcon());
        }
        if (myIndex == 2) {
            taskSheet->setProperty(taskSheet->indexOf("windowTitle"), value);
            m_panel->setTaskTitle(m_panel->currentIndex(), task->windowTitle());
        }
        return;
    }
    wwSheetExtension::setProperty(index, value);
}

void QwwTaskPanelSheetExtension::setChanged(int index, bool changed) {
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

void QwwTaskPanelSheetExtension::setPropertyGroup(int index, const QString & group) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setPropertyGroup(newIndex, group);
}

void QwwTaskPanelSheetExtension::setVisible(int index, bool visible) {
    if (index >= beginCount() && index < beginCount() + names().count())
        return;
    if (!other())
        return;
    int newIndex = index < beginCount() ? index : index - names().count();
    other()->setVisible(newIndex, visible);
}



QwwTaskPanelSheetExtension::QwwTaskPanelSheetExtension(QExtensionManager *manager, QWidget *w, QObject *other, QObject *parent) : wwSheetExtension(manager, QStringList() << "currentTaskName" << "currentTaskIcon" << "currentTaskTitle",
                qobject_cast<QDesignerPropertySheetExtension*>(other),
#if QT_VERSION >= 0x040300
                qobject_cast<QDesignerDynamicPropertySheetExtension*>(other),
#endif
                parent) {
    m_panel = qobject_cast<QwwTaskPanel*>(w);

}

QwwTaskPanelSheetExtension::~ QwwTaskPanelSheetExtension() {

}

void QwwTaskPanelSheetExtension::setPropertyChanged(QWidget * task, const QString & propName, const QVariant &value) {
    QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(m_panel);
    if (form && task) {
        QDesignerPropertySheetExtension *sheet;
        sheet = qt_extension<QDesignerPropertySheetExtension*>(manager(), task);
        int propertyIndex = sheet->indexOf(propName);
        if (value.isValid())
            sheet->setProperty(propertyIndex, value);
        sheet->setChanged(propertyIndex, true);
    }
}

QWidget * QwwTaskPanelSheetExtension::currentPage() const {
    return m_panel ? m_panel->currentTask() : 0;
}
#endif
