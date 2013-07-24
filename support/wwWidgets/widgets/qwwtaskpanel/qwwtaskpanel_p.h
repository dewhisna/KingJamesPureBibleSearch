//
// C++ Interface: qwwtaskpanel_p
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QResizeEvent>
#include <QLayout>
#include <QPainter>
#include <QLabel>
#include <QIcon>
#include <QToolButton>
#include <QTimeLine>
#include <wwglobal.h>


class TaskHeader : public QFrame {
    Q_OBJECT
public:
    TaskHeader(QWidget *w, QWidget *parent = 0);
    void setToggleIcon(const QIcon &i);
    void setTaskName(const QString &n);
    void setIcon(const QIcon &i);
    inline QToolButton *toggleButton() const;
protected:
    void paintEvent(QPaintEvent *ev);
    QWidget *m_widget;
    QLabel *m_text;
    QIcon m_icon;
    QSpacerItem *m_spacer;
    QToolButton *m_button;
};

class Q_WW_EXPORT Task : public QWidget {
    Q_OBJECT
public:
    Task(QWidget *body, QWidget *parent = 0);
    void setName(const QString &n) {
        m_header->setTaskName(n);
        m_body->setWindowTitle(n);
    }
    void setIcon(const QIcon &i) {
        m_header->setIcon(i);
        m_body->setWindowIcon(i);
    }
    void setToggleIcon(const QIcon &i) {
        m_header->setToggleIcon(i);
    }
    QWidget *body() const {
        return m_body;
    }
signals:
    void opened();
    void closed();
public slots:
    void setOpen(bool o);

private slots:
    void animate(int);
    void animFinished();
protected:
    bool eventFilter(QObject *o, QEvent *e);
    TaskHeader *m_header;
    QWidget *m_body;
    QWidget *m_animBody;
    QTimeLine m_animator;
    QPixmap m_animpix;
};



