//
// C++ Interface: qwwconfigwidgetiface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWCONFIGWIDGETIFACE_H
#define QWWCONFIGWIDGETIFACE_H

#include "wwinterfaces.h"

#include <QVariant>
 #include <QExtensionManager>
#include "qwwconfigwidget.h"

/**
	@author Witold Wysota <wysota@wysota.eu.org>
*/
class QwwConfigWidgetIface : public QObject, public QDesignerCustomWidgetInterface
{
Q_OBJECT
Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwConfigWidgetIface(QObject *parent = 0);
    ~QwwConfigWidgetIface();
    bool isContainer() const {
        return true;
    }
    bool isInitialized() const;
    QIcon icon() const;
    QString includeFile() const;
    QString name() const {
        return "QwwConfigWidget";
    }
    QString toolTip() const;
    QString domXml() const;
    QString whatsThis() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);
    QString group() const { return "[ww] Containers"; }
private slots:
     void currentIndexChanged(int index);
private:
    bool initialized;
};

class QwwConfigWidgetContainerExtension: public QObject,
                                          public QDesignerContainerExtension
 {
     Q_OBJECT
     Q_INTERFACES(QDesignerContainerExtension);

 public:
     QwwConfigWidgetContainerExtension(QwwConfigWidget *widget, QObject *parent);

     void addWidget(QWidget *widget);
     int count() const;
     int currentIndex() const;
     void insertWidget(int index, QWidget *widget);
     void remove(int index);
     void setCurrentIndex(int index);
     QWidget *widget(int index) const;

 private:
     QwwConfigWidget *myWidget;
 };



class QwwConfigWidgetSheetExtension : public wwSheetExtension
{
     Q_OBJECT
     Q_INTERFACES(QDesignerPropertySheetExtension);

 public:
    QwwConfigWidgetSheetExtension(QExtensionManager *manager, QWidget *w, QObject *other, QObject *parent=0);
    ~QwwConfigWidgetSheetExtension();
    bool hasReset ( int index ) const;
    bool isChanged ( int index ) const;
    QVariant property ( int index ) const;
    QWidget *currentPage() const;
    bool reset ( int index );
    void setAttribute ( int index, bool attribute );
    void setChanged ( int index, bool changed );
    void setProperty ( int index, const QVariant & value );
    void setPropertyGroup ( int index, const QString & group );
    void setVisible ( int index, bool visible );
private:
  QwwConfigWidget *m_bar;
  void setPropertyChanged(QWidget *task, const QString &propName, const QVariant &value=QVariant());
 };

#endif
