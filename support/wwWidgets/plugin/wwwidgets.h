//
// C++ Interface: wwwidgets
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WWWIDGETS_H
#define WWWIDGETS_H

#include <QObject>
#include <QDesignerCustomWidgetCollectionInterface>
#include <QtDesigner/QExtensionFactory>
#include "ui_about.h"
#include <QDialog>
#include <QMenu>


/**
    @author
*/
class wwWidgets : public QObject, public QDesignerCustomWidgetCollectionInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
    wwWidgets(QObject *parent = 0);
    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;
private slots:
    void registerMenu();
private:
    QList<QDesignerCustomWidgetInterface*> widgets;
};

class wwWidgetsMenu : public QMenu {
    Q_OBJECT
public:
    wwWidgetsMenu(const QString &title, QWidget *parent=0);
public slots:
    void about();
};

class QwwWidgetsExtensionFactory: public QExtensionFactory {
    Q_OBJECT
public:
    QwwWidgetsExtensionFactory(QExtensionManager *parent = 0);
protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;

    QExtensionManager *m_manager;
};


#endif
