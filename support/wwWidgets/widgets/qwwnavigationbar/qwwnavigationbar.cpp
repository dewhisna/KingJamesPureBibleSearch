//
// C++ Implementation: QwwNavigationBar
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2007-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WW_NO_NAVIGATIONBAR

#include "qwwnavigationbar.h"
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QLayout>
#include <QSplitter>
#include <QToolButton>
#include <QVariant>
#include <QMessageBox>
#include <QActionEvent>
#include <QButtonGroup>
#include "wwglobal_p.h"


class QwwNavigationBarPrivate : public QwwPrivate {
public:
    QwwNavigationBarPrivate(QwwNavigationBar *q) : QwwPrivate(q) {
        stack = 0;
        buttonLayout = 0;
        bottomFrame = 0;
        topButton = 0;
        splitter = 0;
        bgroup = 0;
    }
    QList<QPushButton *> buttons;
    QStackedWidget *stack;
    QVBoxLayout *buttonLayout;
    QFrame *bottomFrame;
    QPushButton *topButton;
    QSplitter *splitter;
    QButtonGroup *bgroup;
    void setTitle(QWidget *w, const QString &s){
        w->setWindowTitle(s);
    }
    void setIcon(QWidget *w, const QIcon &ic){
        w->setWindowIcon(ic);
    }
    void _q_buttonClicked();
    WW_DECLARE_PUBLIC(QwwNavigationBar);
};


/*!
 *  \class      QwwNavigationBar
 *  \brief      The QwwNavigationBar class provides a widget similar to MS Outlook navigation bar.
 *  \inmodule wwWidgets
 *
 *              \image qwwnavigationbar.png Navigation bar
 *
 *              \section4 Action management
 *              Actions can be added to a navigation bar which causes buttons representing them
 *              to appear in the bottom of the widget.
 *
 **/
/*!
 *  \property   QwwNavigationBar::topWidgetVisible
 *  \brief      This property holds whether the top widget is visible
 *
 *              If this property is true then a widget containing the current
 *              group is shown at the top of the widget. This property is true
 *              by default.
 *
 **/
/*!
 *  \property   QwwNavigationBar::currentIndex
 *  \brief      This property holds the index position of the current bar
 *
 */
/*!
 *  \fn     void QwwNavigationBar::widgetIconChanged(const QIcon &icon);
 *
 *  \brief  This signal is emitted when the icon of the current widget changes to \a icon.
 */
/*!
 *  \fn     void QwwNavigationBar::widgetLabelChanged(const QString &label);
 *
 *  \brief  This signal is emitted when the label of the current widget changes to \a label.
 */
/*!
 *  \fn     void QwwNavigationBar::topWidgetVisibleChanged(bool vis)
 *  \brief  This signal is emitted whenever visibility of the top widget changes to \a vis
 */
/*!
 *  \fn     void QwwNavigationBar::currentIndexChanged(int index)
 *  \brief  This signal is emitted whenever the current index of the widget changes to \a index
 */

/*!
 * \brief Constructs a navigation bar widget with a given \a parent
 */
QwwNavigationBar::QwwNavigationBar(QWidget *parent)
        : QWidget(parent), QwwPrivatable(new QwwNavigationBarPrivate(this)) {
    Q_D(QwwNavigationBar);
    d->stack = new QStackedWidget(this);
    while (d->stack->count()>0)
        d->stack->removeWidget(d->stack->widget(0));
    QWidget *bw = new QWidget;
    bw->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    d->buttonLayout = new QVBoxLayout(bw);
    d->buttonLayout->setSpacing(0);
    d->buttonLayout->setMargin(0);
    d->splitter = new QSplitter(Qt::Vertical);
//     splitter->setObjectName("__qt__passive_splitter");
    d->splitter->addWidget(d->stack);
    d->splitter->addWidget(bw);
    QVBoxLayout *l = new QVBoxLayout(this);
    d->topButton = new QPushButton;
    d->topButton->setObjectName("TopButton");
    l->addWidget(d->topButton);
    d->topButton->setFlat(true);
    l->addWidget(d->splitter);
    l->setSpacing(0);
    l->setMargin(2);
    d->bottomFrame = new QFrame;
    d->bottomFrame->setFrameShape(QFrame::NoFrame);
    d->bottomFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    l->addWidget(d->bottomFrame);
    QHBoxLayout *hl = new QHBoxLayout(d->bottomFrame);
    d->bgroup = new QButtonGroup(this);
    hl->setSpacing(2);
    hl->setMargin(1);
    hl->addStretch(1);
    connect(d->stack, SIGNAL(currentChanged(int)), this, SIGNAL(currentIndexChanged(int)));
}


/*!
 *      Returns true if the the top widget is visible; otherwise returns false.
 * \sa  setTopWidgetVisible and topWidgetVisible
 */
bool QwwNavigationBar::topWidgetIsVisible() const {
    Q_D(const QwwNavigationBar);
    return d->topButton->isVisible();
}

/*!
 *      Sets whether top widget is to be visible
 */
void QwwNavigationBar::setTopWidgetVisible(bool v) {
    Q_D(QwwNavigationBar);
    d->topButton->setVisible(v);
//     emit topWidgetVisibleChanged(v);
}


/*!
 * \brief   Adds \a child with \a icon and \a label to the navigation bar at the last position.
 */
void QwwNavigationBar::addWidget(QWidget * child, const QIcon &icon, const QString &label) {
    Q_D(QwwNavigationBar);
    insertWidget(d->stack->count(), child, icon, label);
}

/*!
 *  \overload
 */
void QwwNavigationBar::addWidget(QWidget * child, const QString & label) {
    addWidget(child, QIcon(), label);
}



/*!
 * \brief   Inserts widget specified by \a child with \a icon and \a label
 *          to the navigation bar under position \a index
 */
void QwwNavigationBar::insertWidget(int index, QWidget * child, const QIcon &icon, const QString &label) {
    Q_D(QwwNavigationBar);
    QPushButton *b = new QPushButton;
    b->setCheckable(true);
    connect(b, SIGNAL(clicked()), SLOT(_q_buttonClicked()));
    d->stack->insertWidget(index, child);
    d->buttonLayout->insertWidget(index, b);
    d->buttons.insert(index, b);
    d->bgroup->addButton(b);
    b->setObjectName("__qt__passive_button");
    b->setIcon(child->windowIcon());
    child->installEventFilter(this);
    b->updateGeometry();
    if (label.isNull()) {
        b->setText(child->windowTitle());
    } else {
        child->setWindowTitle(label);
    }
    if (icon.isNull()) {
        b->setIcon(child->windowIcon());
    } else {
        child->setWindowIcon(icon);
    }
    int h = d->buttonLayout->parentWidget()->sizeHint().height();
    d->buttonLayout->parentWidget()->setMaximumSize(16777215, h);
    if (d->stack->count()==1) {
        setCurrentIndex(0);
    }
}

/*!
 *  \overload
 */
void QwwNavigationBar::insertWidget(int index, QWidget * child, const QString & label) {
    insertWidget(index, child, QIcon(), label);
}

bool QwwNavigationBar::eventFilter(QObject *o, QEvent *e) {
    Q_D(const QwwNavigationBar);
    if(e->type()==QEvent::WindowTitleChange) {
        QWidget *w = qobject_cast<QWidget*>(o);
        int ind = d->stack->indexOf(w);
        if(ind!=-1) {
            QPushButton *b = d->buttons.at(ind);
            QString title = w->windowTitle();
            b->setText(title);
            if(ind==currentIndex()) {
                d->topButton->setText(title);
                emit widgetLabelChanged(title);
            }
            emit widgetLabelChanged(ind, title);
        }

    }
    if(e->type()==QEvent::WindowIconChange) {
        QWidget *w = qobject_cast<QWidget*>(o);
        int ind = d->stack->indexOf(w);
        if(ind!=-1) {
            QPushButton *b = d->buttons.at(ind);
            QIcon icon = w->windowIcon();
            b->setIcon(icon);
            if(ind==currentIndex())
                d->topButton->setIcon(icon);
            emit widgetIconChanged(ind, icon);
        }
    }
    return false;
}

/*!
 *  \internal
 */
void QwwNavigationBarPrivate::_q_buttonClicked() {
    Q_Q(QwwNavigationBar);
    QPushButton *b = static_cast<QPushButton*>(q->sender());
    int ind = buttons.indexOf(b);
    topButton->setText(b->text());
    topButton->setIcon(b->icon());
    q->setCurrentIndex(ind);
}

/*!
 *  \brief Removes widget specified by \a index from the navigation bar
 */
void QwwNavigationBar::removeWidget(int index) {
    Q_D(QwwNavigationBar);
    delete d->buttons.at(index);
    d->buttons.removeAt(index);
    QWidget *child = d->stack->widget(index);
    child->removeEventFilter(this);
    d->stack->removeWidget(child);
    int ind = d->stack->currentIndex();
    if (ind>-1) {
        QPushButton *b = const_cast<QPushButton*>(button(ind));
        d->topButton->setText(b->text());
        d->topButton->setIcon(b->icon());
    } else {
        d->topButton->setText(QString::null);
        d->topButton->setIcon(QIcon());
    }
}

/*!
 *  \brief Returns an index of \a widget in the navigation bar
 */
int QwwNavigationBar::indexOf(QWidget *widget) const {
    Q_D(const QwwNavigationBar);
    return d->stack->indexOf(widget);
}


int QwwNavigationBar::currentIndex() const {
    Q_D(const QwwNavigationBar);
    return d->stack->currentIndex();
}

/*!
 *  sets position index of the current child
 */
void QwwNavigationBar::setCurrentIndex(int index) {
    Q_D(QwwNavigationBar);
    if (index>=d->stack->count() || index<0) return;
    d->stack->setCurrentIndex(index);
    QWidget *w = widget(index);
    if (!w) return;
    QPushButton *b = const_cast<QPushButton*>(button(index));
    b->setText(w->windowTitle());
    d->topButton->setText(w->windowTitle());
    d->topButton->setIcon(w->windowIcon());
    b->setChecked(true);
}

/*!
 * \brief Sets \a widget as the currently visible widget.
 */
void QwwNavigationBar::setCurrentWidget(QWidget *widget){
    setCurrentIndex(indexOf(widget));
}

/*!
 *
 * \return pointer to a button associated with a child under \a index
 */
const QPushButton * QwwNavigationBar::button(int index) const {
    Q_D(const QwwNavigationBar);
    if (index>=d->buttons.count() || index<0) return 0;
    return d->buttons.at(index);
}

/*!
 *
 * \return icon of the widget under \a index
 */
QIcon QwwNavigationBar::widgetIcon(int index) const {
    const QPushButton *b = button(index);
    if (!b) return QIcon();
    return b->icon();
}

/*!
 *  Associates \a icon with a widget under \a index
 */
void QwwNavigationBar::setWidgetIcon(int index, const QIcon &icon) {
    Q_D(QwwNavigationBar);
    QPushButton *b = const_cast<QPushButton*>(button(index));
    if (!b)
        return;
    b->setIcon(icon);
    QWidget *w = widget(index);
    if (!w) return;
    d->setIcon(w, icon);
    if (index == currentIndex()) {
        d->topButton->setIcon(icon);
    }
}

/*!
 * \return label of the widget under \a index
 */
QString QwwNavigationBar::widgetLabel(int index) const {
    QWidget *w = widget(index);
    if (!w) return QString();
    return w->windowTitle();
}

/*!
 *  Sets \a label as the title for the widget under \a index
 */
void QwwNavigationBar::setWidgetLabel(int index, const QString &label) {
    Q_D(QwwNavigationBar);
    QPushButton *b = const_cast<QPushButton*>(button(index));
    if (!b)
        return;
    b->setText(label);
    QWidget *w = widget(index);
    w->setWindowTitle(label);
}


/*!
 *  \brief  Returns the number of widgets in the navigation bar.
 * 
 */
int QwwNavigationBar::widgetCount() const {
    Q_D(const QwwNavigationBar);
    return d->stack->count();
}

/*!
 * \brief   Returns a pointer to the widget at position \a index
 */
QWidget * QwwNavigationBar::widget(int index) const {
    Q_D(const QwwNavigationBar);
    return d->stack->widget(index);
}


/*!
 * \internal
 */
void QwwNavigationBar::actionEvent(QActionEvent * ev) {
    Q_D(QwwNavigationBar);
    switch (ev->type()) {
    case QEvent::ActionAdded: {
        QHBoxLayout *hl = static_cast<QHBoxLayout*>(d->bottomFrame->layout());
        if (ev->before()==0) {
            QToolButton *tb = new QToolButton;
            tb->setDefaultAction(ev->action());
            tb->setAutoRaise(true);
            hl->addWidget(tb);
        } else
            for (int i=0;i<hl->count();i++) {
                QLayoutItem *lit = hl->itemAt(i);
                if (!lit) break;
                QWidget *w = lit->widget();
                if (!w) continue;
                QToolButton *t = dynamic_cast<QToolButton*>(w);
                if (!t) continue;
                if (ev->before()==t->defaultAction()) {
                    QToolButton *tb = new QToolButton;
                    tb->setDefaultAction(ev->action());
                    tb->setAutoRaise(true);
                    hl->insertWidget(i, tb);
                    return;
                }
            }
    }
    break;
    case QEvent::ActionRemoved: {
        QHBoxLayout *hl = static_cast<QHBoxLayout*>(d->bottomFrame->layout());
        for (int i=0;i<hl->count();i++) {
            QLayoutItem *lit = hl->itemAt(i);
            if (!lit) break;
            QWidget *w = lit->widget();
            if (!w) continue;
            QToolButton *t = dynamic_cast<QToolButton*>(w);
            if (!t) continue;
            if (t->defaultAction()==ev->action()) {
                delete t;
                return;
            }
        }
    }
    break;
    default:
        break;
    }

}

#include "moc_qwwnavigationbar.cpp"
#endif
