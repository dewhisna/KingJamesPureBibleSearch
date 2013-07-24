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
#ifndef WW_NO_COLORCOMBOBOX
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include "qwwcolorcomboboxiface.h"
#include "qwwcolorcombobox.h"
#include "wwwidgets.h"
#include "colorlisteditor.h"
#include "colormodel.h"

QwwColorComboBoxIface::QwwColorComboBoxIface(QObject *parent)
        : wwWidgetInterface(parent) {
            setGroup("[ww] Input Widgets");
            setDefaultGroup("Input Widgets");
        }

QIcon QwwColorComboBoxIface::icon() const {
    return QPixmap(":/trolltech/formeditor/images/widgets/combobox.png");
}

QWidget * QwwColorComboBoxIface::createWidget(QWidget * parent) {
    return new QwwColorComboBox(parent);
}

void QwwColorComboBoxIface::initialize(QDesignerFormEditorInterface * formEditor) {
    if (isInitialized())
        return;
    wwWidgetInterface::initialize(formEditor);
    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new QwwWidgetsExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerTaskMenuExtension));
}



QwwColorComboBoxTaskMenuExtension::QwwColorComboBoxTaskMenuExtension(QwwColorComboBox * widget, QObject * parent) : QObject(parent) {
    this->widget = widget;
    editAction = new QAction(tr("Edit colors..."), this);
    connect(editAction, SIGNAL(triggered()), this, SLOT(mySlot()));
}

QAction * QwwColorComboBoxTaskMenuExtension::preferredEditAction() const {
    return editAction;
}

void QwwColorComboBoxTaskMenuExtension::mySlot() {
    ColorListEditor editor(widget->window());
    QStringList collist = widget->colors();
    ColorModel origModel;
    foreach(QString col, collist) {
        QStringList tmp = col.split(",");
        QColor c;
        c.setNamedColor(tmp[0]);
        origModel.addColor(c, tmp[1]);
    }

    editor.setColorModel(&origModel);
    if (editor.exec()) {
        if (QDesignerFormWindowInterface *formWindow
                = QDesignerFormWindowInterface::findFormWindow(widget)) {
//             ColorModel *model = editor.colorModel();
            QStringList slist;
            for(int i=0;i<origModel.rowCount();i++){
                QModelIndex ind = origModel.index(i,0);
                QColor c = qvariant_cast<QColor>(ind.data(Qt::DecorationRole));
                slist << QString("%1,%2").arg(c.name(), ind.data(Qt::DisplayRole).toString());
            }
            formWindow->cursor()->setProperty("colors", slist);
        }
    }
}

QList< QAction * > QwwColorComboBoxTaskMenuExtension::taskActions() const {
    QList<QAction *> a;
    a << editAction;
    return a;
}

#endif
