//
// C++ Implementation: QwwLed
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2008-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "qwwled.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <QGradient>
#include <QApplication>
#include "wwglobal_p.h"

class QwwLedPrivate : public QwwPrivate {
public:
    QwwLedPrivate(QwwLed *pub) : QwwPrivate(pub) {
        color = Qt::red;
        checked = false;
        width = 1;
        period = 1000;
        timer = -1;
        shape = QwwLed::Circular;
    }
    QColor color;
    bool checked;
    int width;
    QwwLed::Shape shape;
    int timer;
    int period;
private:
    WW_DECLARE_PUBLIC(QwwLed);
};

/*!
 * \class QwwLed
 * \brief The QwwLed class provides a widget that displays a LED.
 * \mainclass
 *
 * \inmodule wwWidgets
 *
 * \image qwwled.png
 *
 */

/*!
 *
 * \property    QwwLed::checked
 * \brief  	Property keeping information whether the led is turned on
 */
/*!
 * \property    QwwLed::animated
 * \brief  	Property keeping information whether the diode is animating
 * 
 */
/*!
 * \enum QwwLed::Shape
 * \value Circular		round LED
 * \value RectangularSunken	rectangular LED with sunken appearance
 * \value RectangularRaised	rectangular LED with raised appearence
 * \value RectangularPlain	rectangular flat LED
 *
 */
/*!
 * \fn QwwLed::clicked()
 * \brief Signal emitted when the diode is clicked
 */
/*!
 * \fn QwwLed::toggled(bool checked)
 * \brief Signal emitted when the diode is toggled.
 *
 * \a checked carries the information about the current state of the diode.
 */
/*!
 * \fn QwwLed::colorChanged(const QColor &color)
 * \brief Signal emitted when the color of the diode is changed to \a color.
 */

/*!
 * \brief Constructs a led widget with a given \a parent.
 * 
 */
QwwLed::QwwLed(QWidget *parent)
        : QWidget(parent), QwwPrivatable(new QwwLedPrivate(this)) {
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
}

/*!
 * \internal
 */
QwwLed::~QwwLed() {
}

/*!
 * \internal
 */
QSize QwwLed::sizeHint() const {
    Q_D(const QwwLed);
    if (d->shape == Circular)
        return QSize(25,25).expandedTo(QApplication::globalStrut());
    else return QSize(25,15).expandedTo(QApplication::globalStrut());
}


/*!
 * \brief Constructs a led widget of colour \a col and shape \a shap with a given \a parent.
 * 
 */
QwwLed::QwwLed(const QColor & col, QwwLed::Shape shap, QWidget * parent) : QWidget(parent), QwwPrivatable(new QwwLedPrivate(this)) {
    Q_D(QwwLed);
    d->color = col;
    d->checked = true;
    d->shape = shap;
}

/*!
 * \brief Sets the colour for the checked diode
 * 
 */
void QwwLed::setColor(const QColor & c) {
    Q_D(QwwLed);
    if (c==d->color) return;
    d->color = c;
    emit colorChanged(c);
    update();
}

/*!
 * \property QwwLed::color
 * \brief This property holds the colour of the diode
 */
QColor QwwLed::color() const {
    Q_D(const QwwLed);
    return d->color;
}

/*!
 * \internal
 */
void QwwLed::paintEvent(QPaintEvent * ) {
    Q_D(QwwLed);
    QPainter p(this);
    QColor bgColor = palette().background().color();
    if (d->shape==Circular) {
        int sidesize = qMin(width(), height());
        p.setRenderHint(QPainter::Antialiasing);
        int rad = sidesize*0.45;
        QRect r = rect().adjusted((width()-sidesize)/2, (height()-sidesize)/2, -(width()-sidesize)/2, -(height()-sidesize)/2);
        QRadialGradient grad(rect().center(), rad, rect().center()-QPoint(sidesize*0.1, sidesize*0.1) );
        grad.setColorAt(0.0, palette().color(QPalette::Light));
        grad.setColorAt(0.75, isChecked() ? d->color : bgColor);
        p.setBrush(grad);
        QPen pe = p.pen();
        pe.setWidth(d->width);
        pe.setColor(palette().color(QPalette::Foreground));
        p.setPen(pe);

        p.drawEllipse(r.adjusted(d->width,d->width,-d->width,-d->width));
    } else { /*if (d->shape == Rectangular)*/
        QStyleOptionFrame opt;
        opt.initFrom(this);
        opt.lineWidth = d->width;
        opt.midLineWidth = d->width;
        if (d->shape==RectangularRaised)
            opt.state |= QStyle::State_Raised;
        else if (d->shape==RectangularSunken)
            opt.state |= QStyle::State_Sunken;
        QBrush br(isChecked() ? d->color : bgColor);
        if (d->shape==RectangularPlain)
            qDrawPlainRect(&p, opt.rect, opt.palette.foreground().color(), d->width, &br);
        else
            qDrawShadePanel(&p, opt.rect, opt.palette, d->shape==RectangularSunken, d->width, &br);

    }
}

/*!
 * \internal
 */
int QwwLed::heightForWidth(int w) const {
    Q_D(const QwwLed);
    if (d->shape!=Circular) return -1;
    return w;
}


bool QwwLed::isChecked() const {
    Q_D(const QwwLed);
    return d->checked;
}

/*!
 * \brief Sets the state of the diode.
 */
void QwwLed::setChecked(bool v) {
    Q_D(QwwLed);
    if (d->checked == v) return;
    d->checked = v;
    emit toggled(v);
    update();
}

/*!
 * \property QwwLed::shape
 * \brief This property holds the shape of the diode.
 */
QwwLed::Shape QwwLed::shape() const {
    Q_D(const QwwLed);
    return d->shape;
}

/*!
 * Sets the shape of the diode.
 * 
 */
void QwwLed::setShape(Shape s) {
    Q_D(QwwLed);
    if (s==d->shape) return;
    d->shape = s;
    updateGeometry();
    update();
}

/*!
 * \brief Toggles the diode on or off.
 */
void QwwLed::toggle() {
    setChecked(!isChecked());
}

/*!
 * \internal
 */
void QwwLed::mouseReleaseEvent(QMouseEvent * me) {
    if (me->button()==Qt::LeftButton) {
        emit clicked();
    }
}

/*!
 * \property QwwLed::frameWidth
 * \brief The width of led's frame
 */
int QwwLed::frameWidth() const {
    Q_D(const QwwLed);
    return d->width;
}

/*!
 * \brief Sets the width of led's frame
 * 
 */
void QwwLed::setFrameWidth(int w) {
    Q_D(QwwLed);
    if (w==d->width) return;
    d->width = w;
    update();
}


bool QwwLed::isAnimated() const {
    Q_D(const QwwLed);
    return d->timer!=-1;
}

/*!
 * \property QwwLed::period
 * \brief Blink period of the diode.
 */
int QwwLed::period() const {
    Q_D(const QwwLed);
    return d->period;
}

void QwwLed::setAnimated(bool v) {
    Q_D(QwwLed);
    if (v && d->timer==-1) {
        d->timer = startTimer(d->period);
        update();
    } else if (!v && d->timer!=-1) {
        killTimer(d->timer);
        d->timer = -1;
        update();
    }
}

/*!
 * \internal
 */
void QwwLed::timerEvent(QTimerEvent * te) {
    Q_D(QwwLed);
    if (te->timerId()!=d->timer)
        return;
    toggle();

}

void QwwLed::setPeriod(int p) {
    Q_D(QwwLed);
    if (p==d->period) return;
    d->period = p;
    if (isAnimated()) {
        killTimer(d->timer);
        d->timer = startTimer(d->period);
    }
}


/*!
 * \brief Blinks the diode once.
 */
void QwwLed::blink() {
    if (isAnimated()) return;
    setChecked(true);
    QTimer::singleShot(100, this, SLOT(toggle()));
}


