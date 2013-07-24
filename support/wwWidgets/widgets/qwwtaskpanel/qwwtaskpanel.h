//
// C++ Interface: qwwtaskpanel
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWTASKPANEL_H
#define QWWTASKPANEL_H

#if defined(QT_NO_TOOLBUTTON) || defined(QT_NO_SCROLLAREA)
#define WW_NO_TASKPANEL
#endif

#ifndef WW_NO_TASKPANEL
#include <QScrollArea>
#include <QList>
#include <QIcon>
#include <wwglobal.h>

class Q_WW_EXPORT QwwTaskPanel : public QWidget/*QScrollArea*/ {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QIcon toggleIcon READ toggleIcon WRITE setToggleIcon)
    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated)
public:
    QwwTaskPanel(QWidget *parent = 0);
    ~QwwTaskPanel();
    void addTask(QWidget *task, const QString &label = QString());
    void addTask(QWidget *task, const QIcon &icon, const QString &label = QString());

    void insertTask(int index, QWidget *task, const QString &label = QString());
    void insertTask(int index, QWidget *task, const QIcon &icon, const QString &label = QString());
    void removeTask(int index);
    int taskCount() const;
    QWidget *task(int index) const;
    QWidget *currentTask() const;
    int indexOf(QWidget *task) const;
    const QIcon &toggleIcon() const { return m_toggleIcon; }
    void setToggleIcon(const QIcon &icon);
    void setTaskIcon(int index, const QIcon &icon);
    void setTaskTitle(int index, const QString &title);
    void setTaskName(int index, const QString &name);
    int currentIndex() const { return m_current; }
    bool isAnimated() const { return m_animated; }
    //void setCurrentTask(QWidget *task);
public slots:
    void setCurrentIndex(int index);
    void setAnimated(bool a){ m_animated = a; }
signals:
    void currentIndexChanged(int);
private:
    QList<QWidget*> m_tasks;
    QWidget *m_panel;
    QIcon m_toggleIcon;
    int m_current;
    bool m_animated;
};


#endif
#endif
