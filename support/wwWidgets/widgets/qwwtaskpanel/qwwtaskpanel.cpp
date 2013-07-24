//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2009-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "qwwtaskpanel.h"
#ifndef WW_NO_TASKPANEL
#include "qwwtaskpanel_p.h"


/*
 *
 *  This class needs a complete rewrite
 *
 *  It needs an own layout that will handle animating its items,
 *  just like QMainWindowLayout does the same for QMainWindow
 *
 */

TaskHeader::TaskHeader(QWidget *w, QWidget *parent) : QFrame(parent) {
    m_widget = w;
    setFrameShape(QFrame::StyledPanel);
    setFrameRect(rect().adjusted(0, 8, 0, 0));
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(1);
    m_spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    l->addItem(m_spacer);
    m_text = new QLabel;
    QFont f = m_text->font();
    f.setBold(true);
    m_text->setFont(f);
    m_button = new QToolButton;
    m_button->setObjectName("__qt__passive_button");
    m_button->setAutoRaise(true);
    m_button->setCheckable(true);
    m_button->setArrowType(Qt::DownArrow);
    l->addWidget(m_text);
    l->addWidget(m_button);
}

void TaskHeader::setToggleIcon(const QIcon &i) {
    m_button->setIcon(i);
    if (i.isNull()) {
        m_button->setArrowType(m_button->isChecked() ? Qt::UpArrow : Qt::DownArrow);
    } else {
        m_button->setArrowType(Qt::NoArrow);
    }
}

void TaskHeader::setTaskName(const QString &n) {
    m_text->setText(n);
    layout()->invalidate();
    update();
}

void TaskHeader::setIcon(const QIcon &i) {
    m_icon = i;
    if (i.isNull()) {
        m_spacer->changeSize(0,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    } else {
        m_spacer->changeSize(50,16, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    m_spacer->invalidate();
    layout()->invalidate();
    update();
}

void TaskHeader::paintEvent(QPaintEvent *ev) {
    QFrame::paintEvent(ev);
    QPainter p(this);
    m_icon.paint(&p, QRect(2, 1, 32, 32), Qt::AlignCenter,
                 isEnabled() ? QIcon::Normal : QIcon::Disabled,
                 toggleButton()->isChecked() ? QIcon::On : QIcon::Off);
}

QToolButton* TaskHeader::toggleButton() const {
    return m_button;
}




Task::Task(QWidget *body, QWidget *parent) : QWidget(parent) {
    m_body = body;
    m_animBody = 0;
    m_animator.setDuration(1200);
    m_animator.setUpdateInterval(20);
    m_animator.setCurveShape(QTimeLine::EaseInOutCurve);
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setMargin(0);
    m_header = new TaskHeader(body);

    l->addWidget(m_header);
    l->addWidget(m_body);

    m_body->setVisible(false);
    m_body->installEventFilter(this);
    connect(m_header->toggleButton(), SIGNAL(toggled(bool)), this, SLOT(setOpen(bool)));
    connect(&m_animator, SIGNAL(frameChanged(int)), SLOT(animate(int)));
    connect(&m_animator, SIGNAL(finished()), SLOT(animFinished()));
}

void Task::setOpen(bool o) {
    QToolButton *b = m_header->toggleButton();
    if (b->isChecked() == o) {
        b->setChecked(o);
        if (b->arrowType()!=Qt::NoArrow) {
            if (o) {
                b->setArrowType(Qt::UpArrow);
            } else {
                b->setArrowType(Qt::DownArrow);
            }
        }
        QwwTaskPanel *tp = parent() ? qobject_cast<QwwTaskPanel*>(parent()->parent()->parent()->parent()) : 0;
        if(tp && tp->isAnimated()){
        if (m_animator.state()!=QTimeLine::NotRunning) {
            m_animator.setDirection(m_animator.direction()==QTimeLine::Forward ? QTimeLine::Backward : QTimeLine::Forward);
        } else {
            m_animBody = new QWidget;
            m_animBody->installEventFilter(this);
#ifndef Q_WS_WIN
            m_animBody->setEnabled(false);
#endif
            m_animBody->setAttribute(Qt::WA_NoSystemBackground, true);
            body()->ensurePolished();
            QSize s = QLayout::closestAcceptableSize(body(), body()->sizeHint()).expandedTo(QSize(width(), 0));
            body()->resize(s);

            body()->setAttribute(Qt::WA_WState_ExplicitShowHide, true);
            body()->setAttribute(Qt::WA_WState_Hidden, false);
            m_animpix = QPixmap::grabWidget(body());
            body()->setAttribute(Qt::WA_WState_Hidden, true);
            if (o) {
                m_animator.setDirection(QTimeLine::Forward);
                m_animator.setFrameRange(0, s.height());
            } else {
                m_animator.setDirection(QTimeLine::Backward);
                m_animator.setFrameRange(0, m_body->height());
            }
            m_body->hide();
            QVBoxLayout *l = (QVBoxLayout*)layout();
            l->addWidget(m_animBody);
            m_animBody->show();
            m_animator.start();
        }
        } else {
            if(o)
                m_body->show();
            else
                m_body->hide();
        }
    }
}

bool Task::eventFilter(QObject *o, QEvent *e) {
    if (o==m_animBody && e->type()==QEvent::Paint) {
        QPainter p(m_animBody);
        p.drawPixmap(m_animBody->rect(), m_animpix, m_animpix.rect().adjusted(0, m_animpix.height()-m_animBody->height(), 0, 0));
        return true;
    }
//     if (o==m_body && e->type()==QEvent::WindowTitleChange) {
//         setName(m_body->windowTitle());
//     }
//     if (o==m_body && e->type()==QEvent::WindowIconChange) {
//         setIcon(m_body->windowIcon());
//     }
    return false;
}

void Task::animate(int frame ) {
    m_animBody->setFixedSize(m_animBody->width(), frame);
    m_animBody->updateGeometry();
//     m_animBody->update();

}

void Task::animFinished() {
    if (m_animator.currentFrame()!=0) {
        m_body->show();
    }
    m_animBody->lower();
    m_animBody->hide();
    m_animBody->deleteLater();
    m_animBody = 0;
    m_header->update();
    updateGeometry();
}


/*!
 *  \class QwwTaskPanel
 *  \brief The QwwTaskPanel widget provides a task panel similar to the one from WindowsXP.(unstable)
 *  \inmodule wwWidgets
 *
 *
 *
 */

QwwTaskPanel::QwwTaskPanel(QWidget *parent) : QWidget(parent)
        /*: QScrollArea(parent)*/ {
    QScrollArea *sa = new QScrollArea(this);
    QVBoxLayout *la = new QVBoxLayout(this);
    la->setMargin(0);
    la->addWidget(sa);
    m_animated = false;
    m_panel = new QWidget(sa->viewport());
    m_panel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);
    m_panel->setObjectName("ww_taskpanel_panel");
    m_current = -1;
    QVBoxLayout *l = new QVBoxLayout(m_panel);
    l->addStretch();
    sa->setWidget(m_panel);
    sa->setWidgetResizable(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
}


QwwTaskPanel::~QwwTaskPanel() {}

void QwwTaskPanel::addTask(QWidget * task, const QString & label) {
    insertTask(taskCount(), task, label);
}

void QwwTaskPanel::addTask(QWidget * task, const QIcon & icon, const QString & label) {
    insertTask(taskCount(), task, icon, label);
}

void QwwTaskPanel::insertTask(int index, QWidget * task, const QString & label) {
    insertTask(index, task, QIcon(), label);
}

void QwwTaskPanel::insertTask(int index, QWidget * task, const QIcon & icon, const QString & label) {
    if (!task) return;
    Task *tsk = new Task(task);
    tsk->setToggleIcon(m_toggleIcon);
    if (label.isNull())
        tsk->setName(task->windowTitle());
    else {
        tsk->setName(label);
        task->setWindowTitle(label);
    }
    if (icon.isNull()) {
        tsk->setIcon(task->windowIcon());
    } else {
        tsk->setIcon(icon);
        task->setWindowIcon(icon);
    }
    static_cast<QBoxLayout*>(m_panel->layout())->insertWidget(index, tsk);
    m_tasks.insert(index, tsk);

    if (m_tasks.count()==1) {
        setCurrentIndex(0);
    }
     tsk->show();
}

int QwwTaskPanel::taskCount() const {
    return m_tasks.count();
}

void QwwTaskPanel::removeTask(int index) {
    if (index < 0 || index>=m_tasks.count()) return;
    Task *tsk = static_cast<Task*>(m_tasks.at(index));
    m_tasks.removeAt(index);
    if (m_tasks.count()<=index) {
        setCurrentIndex(m_tasks.count()-1);
    }
    QWidget *body = tsk->body();
    body->setParent(this);
    delete tsk;
}

QWidget * QwwTaskPanel::task(int index) const {
    if (index < 0 || index>=m_tasks.count()) return 0;
    Task *tsk = static_cast<Task*>(m_tasks.at(index));
    return tsk ? tsk->body() : 0;
}

int QwwTaskPanel::indexOf(QWidget * task) const {
    for (int i=0;i<m_tasks.count();i++) {
        Task *tsk = static_cast<Task*>(m_tasks.at(i));
        if (task == tsk) return i;
    }
    return -1;
}

void QwwTaskPanel::setToggleIcon(const QIcon & icon) {
    m_toggleIcon = icon;
    foreach(QWidget *tsk, m_tasks) {
        static_cast<Task*>(tsk)->setToggleIcon(icon);
    }
}

void QwwTaskPanel::setCurrentIndex(int index) {
    if (index<0 || index>=m_tasks.count() || index==m_current)
        return;
    if (m_current!=-1) {
        Task *tsk = static_cast<Task*>(m_tasks.at(m_current));
        tsk->setOpen(false);
    }
    m_current = index;
    Task *tsk = static_cast<Task*>(m_tasks.at(index));
    tsk->setOpen(true);
    emit currentIndexChanged(index);
}

QWidget * QwwTaskPanel::currentTask() const {
    return task(currentIndex());
}

void QwwTaskPanel::setTaskIcon(int index, const QIcon & icon) {
    if (index < 0 || index>=m_tasks.count()) return;
    Task *tsk = qobject_cast<Task*>(m_tasks.at(index));
    if (!tsk) return;
    tsk->setIcon(icon);

}

void QwwTaskPanel::setTaskTitle(int index, const QString & title) {
    if (index < 0 || index>=m_tasks.count()) return;
    Task *tsk = qobject_cast<Task*>(m_tasks.at(index));
    if (!tsk) return;
    tsk->setName(title);
}

void QwwTaskPanel::setTaskName(int index, const QString & name) {
    if (index < 0 || index>=m_tasks.count()) return;
    Task *tsk = qobject_cast<Task*>(m_tasks.at(index));
    if (!tsk) return;
    tsk->body()->setObjectName(name);
}
#endif
