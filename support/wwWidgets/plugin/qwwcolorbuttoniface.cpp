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

#include "qwwcolorbuttoniface.h"
#include "qwwcolorbutton.h"
#include "colorlisteditor.h"
#include "wwwidgets.h"
#include "colormodel.h"

QwwColorButtonIface::QwwColorButtonIface(QObject *parent)
        : wwWidgetInterface(parent) {
            setGroup("[ww] Buttons");
            setDefaultGroup("Buttons");
        }



QIcon QwwColorButtonIface::icon() const {
    return QIcon(QPixmap(":/colorbutton.png"));
}

QWidget * QwwColorButtonIface::createWidget(QWidget * parent) {
    return new QwwColorButton(parent);
}

void QwwColorButtonIface::initialize(QDesignerFormEditorInterface * formEditor) {
    if (isInitialized())
        return;
    wwWidgetInterface::initialize(formEditor);
    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new QwwWidgetsExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerTaskMenuExtension));
}

QwwColorButtonTaskMenuExtension::QwwColorButtonTaskMenuExtension(QwwColorButton * widget, QObject * parent) : QObject(parent) {
    this->widget = widget;
    editAction = new QAction(tr("Edit colors..."), this);
    connect(editAction, SIGNAL(triggered()), this, SLOT(mySlot()));
}

QAction * QwwColorButtonTaskMenuExtension::preferredEditAction() const {
    return editAction;
}

void QwwColorButtonTaskMenuExtension::mySlot() {
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

QList< QAction * > QwwColorButtonTaskMenuExtension::taskActions() const {
    QList<QAction *> a;
    a << editAction;
    return a;
}
