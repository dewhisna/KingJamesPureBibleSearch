//
// C++ Interface: qwwnavigationbar
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWNAVIGATIONBAR_H
#define QWWNAVIGATIONBAR_H

#ifndef WW_NO_NAVIGATIONBAR

#include <QWidget>
#include <QList>
#include <QIcon>
#include <wwglobal.h>

class QPushButton;
class QStackedWidget;
class QFrame;
class QSplitter;
class QVBoxLayout;
class QToolButton;


class QwwNavigationBarPrivate;
class Q_WW_EXPORT QwwNavigationBar : public QWidget, QwwPrivatable
{
Q_OBJECT
Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex STORED true)
Q_PROPERTY(bool topWidgetVisible READ topWidgetIsVisible WRITE setTopWidgetVisible)
public:
    QwwNavigationBar(QWidget *parent = 0);
    void addWidget(QWidget *child, const QString &label=QString::null);
    void addWidget(QWidget *child, const QIcon &, const QString &label=QString::null);
    void insertWidget(int index, QWidget *child, const QString &label=QString::null);
    void insertWidget(int index, QWidget *child, const QIcon &, const QString &label=QString::null);
    void removeWidget(int index);

    int widgetCount() const;
    QWidget *widget(int index) const;
    int indexOf(QWidget *) const;
    const QPushButton *button(int index) const;

    int currentIndex() const;


    void setCurrentWidget(QWidget *);

    void setWidgetLabel(int index, const QString &);
    QString widgetLabel(int index) const;
    void setWidgetIcon(int index, const QIcon &);
    QIcon widgetIcon(int index) const;
    bool topWidgetIsVisible() const;
public slots:

    void setCurrentIndex(int);
    void setTopWidgetVisible(bool);
protected:
    virtual void actionEvent(QActionEvent *ev);
    bool eventFilter(QObject *, QEvent *);
private:
    WW_DECLARE_PRIVATE(QwwNavigationBar);
    Q_DISABLE_COPY(QwwNavigationBar);
signals:
    void currentIndexChanged(int);
    void widgetLabelChanged(const QString &);
    void widgetIconChanged(const QIcon &);
    void topWidgetVisibleChanged(bool);
    void widgetLabelChanged(int index, const QString &);
    void widgetIconChanged(int index, const QIcon &);
private:
    Q_PRIVATE_SLOT(d_func(), void _q_buttonClicked());
};

#endif
#endif
