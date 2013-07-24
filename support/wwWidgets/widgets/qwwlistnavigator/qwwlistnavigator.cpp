//
// C++ Implementation: QwwListNavigator
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2008-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "qwwlistnavigator.h"
#if !defined(WW_NO_LISTNAVIGATOR)
#include <QLayout>
#include <QToolButton>
#include <QSlider>
#include <QtDebug>
#include <QMessageBox>
#include <QScrollBar>

#include "wwglobal_p.h"
#include "qwwlistwidget.h"


class QwwListNavigatorPrivate : public QwwPrivate {
public:
    QwwListNavigatorPrivate(QwwListNavigator *pub) : QwwPrivate(pub) {listWidget = 0;}

    QwwListNavigator::Buttons buttons;
    QMap<QwwListNavigator::Button, QWidget*> widgets;
    QwwListWidget *listWidget;
    void _q_valueChanged(int);
    void _q_first();
    void _q_next();
    void _q_previous();
    void _q_last();
    void _q_updateLWRange();
    void _q_disconnectListWidget();
    void retranslateUi();
private:
    WW_DECLARE_PUBLIC(QwwListNavigator);
};


void QwwListNavigatorPrivate::_q_valueChanged(int v) {
    Q_Q(QwwListNavigator);
//     if (((QSlider*)widgets[QwwListNavigator::Slider])->value()==v) return;
    ((QSlider*)widgets[QwwListNavigator::Slider])->setValue(v);
    widgets[QwwListNavigator::LastButton]->setEnabled(q->value()!=q->maximum() && q->minimum()!=q->maximum());
    widgets[QwwListNavigator::FirstButton]->setEnabled(q->value()!=q->minimum() && q->minimum()!=q->maximum());
    widgets[QwwListNavigator::NextButton]->setEnabled(q->value()!=q->maximum() && q->minimum()!=q->maximum());
    widgets[QwwListNavigator::PrevButton]->setEnabled(q->value()!=q->minimum() && q->minimum()!=q->maximum());
}

void QwwListNavigatorPrivate::_q_updateLWRange() {
    Q_Q(QwwListNavigator);
    if (!listWidget){ q->setRange(0,0); return; }
    q->setRange(0, listWidget->count()-1);
    q->setValue(listWidget->currentRow());
}

void QwwListNavigatorPrivate::_q_disconnectListWidget() {
    listWidget = 0;
}

void QwwListNavigatorPrivate::retranslateUi() {
    Q_Q(QwwListNavigator);
    qobject_cast<QAbstractButton*>(widgets[QwwListNavigator::FirstButton])->setText(q->tr("First"));
    widgets[QwwListNavigator::FirstButton]->setToolTip(q->tr("Go to first"));
    qobject_cast<QAbstractButton*>(widgets[QwwListNavigator::PrevButton])->setText(q->tr("Previous"));
    widgets[QwwListNavigator::PrevButton]->setToolTip(q->tr("Go to previous"));
    qobject_cast<QAbstractButton*>(widgets[QwwListNavigator::NextButton])->setText(q->tr("Next"));
    widgets[QwwListNavigator::NextButton]->setToolTip(q->tr("Go to next"));
    qobject_cast<QAbstractButton*>(widgets[QwwListNavigator::LastButton])->setText(q->tr("Last"));
    widgets[QwwListNavigator::LastButton]->setToolTip(q->tr("Go to last"));
}

/*!
 * \class QwwListNavigator
 * \brief The QwwListNavigator class provides a widget letting the user navigate through items in a list.
 * \inmodule wwWidgets
 *
 * \image qwwlistnavigator.png QwwListNavigator
 * 
 */
/*!
 * \property    QwwListNavigator::orientation
 * \brief 	This property holds the orientation of the slider.
 * 
 *        The orientation must be Qt::Vertical (the default) or Qt::Horizontal.
 *
 *
 */
/*!
 * \enum	QwwListNavigator::Button
 * \value NoButtons	No buttons are visible
 * \value FirstButton   Button for moving onto the first record in the set
 * \value PrevButton	Button for moving onto previous record in the set
 * \value Slider	Slider for moving between records
 * \value NextButton	Button for moving onto next record in the set
 * \value LastButton	Button for moving onto the last record in the set
 *
 */
/*!
 * \fn 		QwwListNavigator::first()
 * \brief 	The signal is emited when the navigator is set on the first record in the set.
 * \sa          toFirst()
 */
/*!
 * \fn 		QwwListNavigator::next()
 * \brief 	The signal is emited when the navigator is set on the next record in the set.
 * \sa          toNext()
 */
/*!
 * \fn 		QwwListNavigator::previous()
 * \brief 	The signal is emited when the navigator is set on the previous record in the set.
 * \sa          toPrevious()
 */
/*!
 * \fn 		QwwListNavigator::last()
 * \brief 	The signal is emited when the navigator is set on the last record in the set.
 * \sa          toLast()
 */



/*!
 * \brief Constructs a horizontal list navigator with a given \a parent.
 * 
 */
QwwListNavigator::QwwListNavigator(QWidget * parent) : QAbstractSlider(parent), QwwPrivatable(new QwwListNavigatorPrivate(this)) {
    Q_D(QwwListNavigator);
    setOrientation(Qt::Horizontal);
    d->buttons = (Buttons)(FirstButton|PrevButton|Slider|NextButton|LastButton);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(0);
    QToolButton *fir = new QToolButton;
    fir->setIcon(wwWidgets::icon("go-first", QPixmap(":/go-first.png")));
    fir->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    l->addWidget(fir);
    QToolButton *pre = new QToolButton;
    pre->setIcon(wwWidgets::icon("go-previous", QPixmap(":/go-previous.png")));
    pre->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    pre->setAutoRepeat(true);
    l->addWidget(pre);
    //l->addStretch();
    QSlider *sli = new QSlider(Qt::Horizontal);
    sli->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connect(sli, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    connect(this, SIGNAL(valueChanged(int)), sli, SLOT(setValue(int)));
    l->addWidget(sli);
    QToolButton *nex = new QToolButton;
    nex->setIcon(wwWidgets::icon("go-next", QPixmap(":/go-next.png")));
    nex->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    nex->setAutoRepeat(true);
    l->addWidget(nex);
    QToolButton *las = new QToolButton;
    las->setIcon(wwWidgets::icon("go-last", QPixmap(":/go-last.png")));
    las->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    l->addWidget(las);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->widgets[Slider] = sli;
    d->widgets[FirstButton] = fir;
    d->widgets[PrevButton] = pre;
    d->widgets[NextButton] = nex;
    d->widgets[LastButton] = las;
    connect(fir, SIGNAL(clicked()), this, SLOT(toFirst()));
    connect(pre, SIGNAL(clicked()), this, SLOT(toPrevious()));
    connect(nex, SIGNAL(clicked()), this, SLOT(toNext()));
    connect(las, SIGNAL(clicked()), this, SLOT(toLast()));
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(_q_valueChanged(int)));
    d->retranslateUi();
    d->_q_valueChanged(value());
}


/*!
 * \property QwwListNavigator::buttons
 * \brief This property holds visible buttons of the navigator.
 * 
 */
QwwListNavigator::Buttons QwwListNavigator::buttons() const {
    Q_D(const QwwListNavigator);
    return d->buttons;
}

/*!
 * \brief Sets visible buttons for the navigator.
 * \param b
 */
void QwwListNavigator::setButtons(Buttons b) {
    Q_D(QwwListNavigator);
    if (d->buttons == b) return;
    d->buttons = b;
    foreach(Button bu, d->widgets.keys()) {
        d->widgets[bu]->setVisible(d->buttons & bu);
    }
}

/*!
 * \brief This method performs a change of slider value specified by \a change.
 *
 * 
 * 
 */
void QwwListNavigator::sliderChange(SliderChange change) {
    Q_D(QwwListNavigator);
    d->_q_valueChanged(value());
    if (change==SliderRangeChange) {
        ((QSlider*)d->widgets[Slider])->setRange(minimum(), maximum());
    }
    if (change==SliderStepsChange) {
        ((QSlider*)d->widgets[Slider])->setSingleStep(singleStep());
        ((QSlider*)d->widgets[Slider])->setPageStep(pageStep());
    }
    if (change==SliderOrientationChange) {
        QBoxLayout *l = 0;
        QLayout *old = layout();
        old->removeWidget(d->widgets[FirstButton]);
        old->removeWidget(d->widgets[PrevButton]);
        old->removeWidget(d->widgets[Slider]);
        old->removeWidget(d->widgets[NextButton]);
        old->removeWidget(d->widgets[LastButton]);
        delete layout();
        switch (orientation()) {
        case Qt::Horizontal:
            l = new QHBoxLayout(this);
            ((QSlider*)d->widgets[Slider])->setInvertedAppearance(false);
            ((QSlider*)d->widgets[Slider])->setInvertedControls(false);
            ((QToolButton*)d->widgets[FirstButton])->setIcon(wwWidgets::icon("go-first", QPixmap(":/go-first.png")));
            ((QToolButton*)d->widgets[PrevButton])->setIcon(wwWidgets::icon("go-previous", QPixmap(":/go-previous.png")));
            ((QToolButton*)d->widgets[NextButton])->setIcon(wwWidgets::icon("go-next", QPixmap(":/go-next.png")));
            ((QToolButton*)d->widgets[LastButton])->setIcon(wwWidgets::icon("go-last", QPixmap(":/go-last.png")));
            break;
        case Qt::Vertical:
            l = new QVBoxLayout(this);
            ((QSlider*)d->widgets[Slider])->setInvertedAppearance(true);
            ((QSlider*)d->widgets[Slider])->setInvertedControls(true);
            ((QToolButton*)d->widgets[FirstButton])->setIcon(wwWidgets::icon("go-top", QPixmap(":/go-top.png")));
            ((QToolButton*)d->widgets[PrevButton])->setIcon(wwWidgets::icon("go-up", QPixmap(":/go-up.png")));
            ((QToolButton*)d->widgets[NextButton])->setIcon(wwWidgets::icon("go-down", QPixmap(":/go-down.png")));
            ((QToolButton*)d->widgets[LastButton])->setIcon(wwWidgets::icon("go-bottom", QPixmap(":/go-bottom.png")));
            break;
        }
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);
        l->setMargin(0);
        l->addWidget(d->widgets[FirstButton]);
        l->addWidget(d->widgets[PrevButton]);
        l->addWidget(d->widgets[Slider]);
        l->addWidget(d->widgets[NextButton]);
        l->addWidget(d->widgets[LastButton]);
        ((QSlider*)d->widgets[Slider])->setOrientation(orientation());
        resize(sizeHint());
    }
    QAbstractSlider::sliderChange(change);
}

/*!
 * \brief Sets the orientation of the widget.
 * 
 */
void QwwListNavigator::setOrientation(Qt::Orientation o) {
    if (orientation()==o) return;
    QAbstractSlider::setOrientation(o);
    sliderChange(SliderOrientationChange);
}

/*!
 * \property QwwListNavigator::autoRaise
 * \brief This property holds whether buttons of the navigator are auto raised.
 */
bool QwwListNavigator::autoRaise() const {
    Q_D(const QwwListNavigator);
    if (!d->widgets.contains(FirstButton)) return false;
    return ((QToolButton*)d->widgets[FirstButton])->autoRaise();
}

void QwwListNavigator::setAutoRaise(bool v) {
    Q_D(QwwListNavigator);
    if (autoRaise()==v) return;
    ((QToolButton*)d->widgets[FirstButton])->setAutoRaise(v);
    ((QToolButton*)d->widgets[PrevButton])->setAutoRaise(v);
    ((QToolButton*)d->widgets[NextButton])->setAutoRaise(v);
    ((QToolButton*)d->widgets[LastButton])->setAutoRaise(v);
}

/*!
 * \brief Sets the style of buttons in the navigator.
 * \param v
 */
void QwwListNavigator::setToolButtonStyle(Qt::ToolButtonStyle v) {
    Q_D(QwwListNavigator);
    if (toolButtonStyle()==v) return;
    ((QToolButton*)d->widgets[FirstButton])->setToolButtonStyle(v);
    ((QToolButton*)d->widgets[PrevButton])->setToolButtonStyle(v);
    ((QToolButton*)d->widgets[NextButton])->setToolButtonStyle(v);
    ((QToolButton*)d->widgets[LastButton])->setToolButtonStyle(v);
}

/*!
 * \property QwwListNavigator::toolButtonStyle
 * \brief This property holds the style of buttons in the navigator.
 */
Qt::ToolButtonStyle QwwListNavigator::toolButtonStyle() const {
    Q_D(const QwwListNavigator);
    if (!d->widgets.contains(FirstButton)) return Qt::ToolButtonIconOnly;
    return ((QToolButton*)d->widgets[FirstButton])->toolButtonStyle();
}

/*!
 * \brief Moves navigator to the first record.
 *
 * 	  Does nothing if the navigator is already at the first record.
 * \sa    first()
 */
void QwwListNavigator::toFirst() {
    if (value()==minimum()) return;
    setValue(minimum());
    emit first();
}

/*!
 *  \brief Moves navigator to the previous record.
 *
 *         Does nothing if the navigator is already at the first record.
 * \sa     previous()
 */
void QwwListNavigator::toPrevious() {
    if (value()==minimum()) return;
    setValue(value()-1);
    emit previous();
}

/*!
 * \brief Moves navigator to the last record.
 *
 *        Does nothing if the navigator is already at the last record.
 * \sa    last()
 */
void QwwListNavigator::toLast() {
    if (value()>=maximum()) return;
    setValue(maximum());
    emit last();
}

/*!
 * \brief Moves navigator to the next record.
 *
 *        Does nothing if the navigator is already at the last record.
 * \sa    next()
 */
void QwwListNavigator::toNext() {
    if (value()>=maximum()) return;
    setValue(value()+1);
    emit next();
}

/*!
 * \brief Associates a QwwListWidget instance with the navigator.
 * 
 *        The method observes the list widget \a lw allowing to navigate through
 *        its items using the navigator.
 */
void QwwListNavigator::setListWidget(QwwListWidget * lw) {
    Q_D(QwwListNavigator);
    if (d->listWidget) {
        disconnect(d->listWidget->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(_q_updateLWRange()));
        disconnect(d->listWidget->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(_q_updateLWRange()));
        disconnect(d->listWidget->model(), SIGNAL(modelReset()), this, SLOT(_q_updateLWRange()));
        disconnect(d->listWidget, SIGNAL(destroyed()), this, SLOT(_q_disconnectListWidget()));
        disconnect(this, SIGNAL(valueChanged(int)), d->listWidget, SLOT(setCurrentRow(int)));
        disconnect(d->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(setValue(int)));
    }
    d->listWidget = lw;
    if (d->listWidget) {
        connect(d->listWidget->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(_q_updateLWRange()));
        connect(d->listWidget->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(_q_updateLWRange()));
        connect(d->listWidget->model(), SIGNAL(modelReset()), this, SLOT(_q_updateLWRange()));
        connect(d->listWidget, SIGNAL(destroyed()), this, SLOT(_q_disconnectListWidget()));
        connect(this, SIGNAL(valueChanged(int)), d->listWidget, SLOT(setCurrentRow(int)));
        connect(d->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(setValue(int)));
    }
    d->_q_updateLWRange();
}


#include "moc_qwwlistnavigator.cpp"

#endif
