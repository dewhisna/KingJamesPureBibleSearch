//
// C++ Interface: qwwtaskpaneliface
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWTASKPANELIFACE_H
#define QWWTASKPANELIFACE_H
#ifndef WW_NO_TASKPANEL
#include "wwinterfaces.h"
#include <QTimer>
#include <QVariant>

#include "qwwtaskpanel.h"


class QwwTaskPanelIface : public wwWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface);
public:
    QwwTaskPanelIface(QObject *parent = 0);
    ~QwwTaskPanelIface();
    bool isContainer() const {
        return true;
    }
    QIcon icon() const;
    QString domXml() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);
private:
    bool eventFilter(QObject *obj, QEvent *e);
    QDesignerFormEditorInterface *formEditor;
private slots:
    void currentIndexChanged(int);
};


class QwwTaskPanelContainerExtension: public QObject,
            public QDesignerContainerExtension {
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension);

public:
    QwwTaskPanelContainerExtension(QwwTaskPanel *widget, QObject *parent);

    void addWidget(QWidget *widget);
    int count() const;
    int currentIndex() const;
    void insertWidget(int index, QWidget *widget);
    void remove(int index);
    void setCurrentIndex(int index);
    QWidget *widget(int index) const;

private:
    QwwTaskPanel *myWidget;
};

//class PropertyMatcher;

class QwwTaskPanelSheetExtension : public wwSheetExtension
 {
     Q_OBJECT
     Q_INTERFACES(QDesignerPropertySheetExtension);

 public:
    QwwTaskPanelSheetExtension(QExtensionManager *manager, QWidget *w, QObject *other, QObject *parent=0);
    ~QwwTaskPanelSheetExtension();
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
  QwwTaskPanel *m_panel;
  void setPropertyChanged(QWidget *task, const QString &propName, const QVariant &value=QVariant());
 };

#endif
#endif
