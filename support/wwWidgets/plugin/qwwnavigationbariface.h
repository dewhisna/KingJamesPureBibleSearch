//
// C++ Interface: qwwnavigationbariface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWNAVIGATIONBARIFACE_H
#define QWWNAVIGATIONBARIFACE_H

#include "wwinterfaces.h"

#include <QVariant>
 #include <QExtensionManager>
#include "qwwnavigationbar.h"


class QwwNavigationBarIface : public wwWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwNavigationBarIface(QObject *parent = 0);
    ~QwwNavigationBarIface();
    bool isContainer() const {
        return true;
    }
    QIcon icon() const;
    QString domXml() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);
private slots:
     void currentIndexChanged(int index);
};



class QwwNavigationBarContainerExtension: public QObject,
                                          public QDesignerContainerExtension
 {
     Q_OBJECT
     Q_INTERFACES(QDesignerContainerExtension);

 public:
     QwwNavigationBarContainerExtension(QwwNavigationBar *widget, QObject *parent);

     void addWidget(QWidget *widget);
     int count() const;
     int currentIndex() const;
     void insertWidget(int index, QWidget *widget);
     void remove(int index);
     void setCurrentIndex(int index);
     QWidget *widget(int index) const;

 private:
     QwwNavigationBar *myWidget;
 };


class QwwNavigationBarSheetExtension : public wwSheetExtension
 {
     Q_OBJECT
     Q_INTERFACES(QDesignerPropertySheetExtension);

 public:
    QwwNavigationBarSheetExtension(QExtensionManager *manager, QWidget *w, QObject *other, QObject *parent=0);
    ~QwwNavigationBarSheetExtension();
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
  QwwNavigationBar *m_bar;
  void setPropertyChanged(QWidget *task, const QString &propName, const QVariant &value=QVariant());
 };

#endif
