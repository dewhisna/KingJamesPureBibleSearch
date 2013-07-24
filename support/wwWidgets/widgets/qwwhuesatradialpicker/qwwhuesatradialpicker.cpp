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
#ifndef WW_NO_HUESATRADIALPICKER

#include "qwwhuesatradialpicker.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include "wwglobal_p.h"

#ifndef M_PI
#define M_PI            3.14159265358979323846  /* pi */
#endif

/**
 *  @internal
 */
class QwwHueSatRadialPickerPrivate : public QwwPrivate {
public:
    QwwHueSatRadialPickerPrivate(QwwHueSatRadialPicker *parent) : QwwPrivate(parent) {
        m_value = 255;
        for (int i=0;i<360;i++) {
            conical.setColorAt(i*1.0/360, QColor::fromHsv(i, 255, m_value));
        }
        conical.setColorAt(1, QColor::fromHsv(359, 255, m_value));
    }
    void buildPixmap();
    QPixmap px;
    QConicalGradient conical;
    int m_value;
    QPoint m_pt;
    QColor m_color;
    int radius(const QPoint &pt) const;
    int hue(const QPoint &pt) const;
    WW_DECLARE_PUBLIC(QwwHueSatRadialPicker);
};

void QwwHueSatRadialPickerPrivate::buildPixmap() {
    // build huecircle
    Q_Q(QwwHueSatRadialPicker);
    conical.setCenter(q->rect().center());
    QImage huecircle(q->size(), QImage::Format_ARGB32);
    huecircle.fill(Qt::transparent);
    QPainter phcircle(&huecircle);
    phcircle.setRenderHint(QPainter::Antialiasing, true);
    phcircle.setBrush(QBrush(conical));
    phcircle.setPen(q->palette().color(QPalette::Shadow));
    phcircle.drawEllipse(q->rect().adjusted(1, 1, -1, -1));
    px = QPixmap::fromImage(huecircle);

    // alpha gradient
    QRadialGradient rg(q->rect().center(), q->width()/2-2, q->rect().center());

    for (float i=0;i<1;i+=0.1) {
        rg.setColorAt(i, QColor::fromHsv(0, 0, (int)(256*i)));
    }
    rg.setColorAt(1, Qt::white);

    // alpha channel
    QImage ac(q->size(), QImage::Format_RGB32);
    ac.fill(Qt::transparent);
    QPainter acpainter(&ac);
    acpainter.setPen(Qt::NoPen);
    acpainter.setBrush(QBrush(rg));
    acpainter.drawEllipse(q->rect().adjusted(1, 1, -1, -1));
    px.setAlphaChannel(QPixmap::fromImage(ac));

    // destination image
    QImage dst(q->size(), QImage::Format_ARGB32);
    dst.fill(Qt::transparent);
    QPainter dstp(&dst);
    dstp.setBrush(QColor::fromHsv(0, 0, m_value));
    dstp.setRenderHint(QPainter::Antialiasing, true);
    dstp.setPen(q->palette().color(QPalette::Shadow));
    dstp.drawEllipse(q->rect().adjusted(1, 1, -1, -1));
    dstp.setCompositionMode(QPainter::CompositionMode_SourceOver);
    dstp.drawPixmap(0, 0, px);
    /*dstp.setCompositionMode(QPainter::CompositionMode_Source);
    dstp.setBrush(Qt::NoBrush);
    dstp.setPen(Qt::black);
    dstp.drawEllipse(rect()); */
    px = QPixmap::fromImage(dst);
}

int QwwHueSatRadialPickerPrivate::radius(const QPoint & pt) const {
    Q_Q(const QwwHueSatRadialPicker);
    QPoint c = q->rect().center();
    int x = pt.x() - c.x();
    int y = pt.y() - c.y();
    int r = qRound(sqrt((double)(x*x + y*y)));

    return r;
}

int QwwHueSatRadialPickerPrivate::hue(const QPoint & pt) const {
    Q_Q(const QwwHueSatRadialPicker);
    QPoint c = q->rect().center();
    int x = c.x()-pt.x();
    int y = pt.y()-c.y();
    double a = qAbs(y)*1.0 / qAbs(x);   // tangent
    double rd = atan(a);
    double deg = rd * 180 / M_PI;
    int h = qRound(deg);
    if (x>=0 && y>=0) h=180+h;
    else if (x<0 && y>=0) h=360-h;
    else if (x<0 && y<0) h=h;
    else h=180-h;
    return h % 360;
}

/*!
 *  \class QwwHueSatRadialPicker
 *  \brief The QwwHueSatRadialPicker class provides a widget allowing color picking
 *         in a form of hue-saturation circle.
 *
 *  \image qwwhuesatradialpicker.png QwwHueSatRadialPicker
 *
 *  \ingroup colorclasses
 */
/*!
 *  \fn     QwwHueSatRadialPicker::valueChanged(int value)
 *  \brief  This signal is emitted whenever the V component of the color changes to \a value.
 */
/*!
 *  \fn     QwwHueSatRadialPicker::colorPicked(QColor color)
 *  \brief  This signal is emitted whenever \a color is picked.
 */

/*!
 * Constructs a radial picker widget with a given \a parent.
 *
 */
QwwHueSatRadialPicker::QwwHueSatRadialPicker(QWidget *parent)
        : QWidget(parent), QwwPrivatable(new QwwHueSatRadialPickerPrivate(this)) {
    QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
}


/*!
 *  \internal
 */
void QwwHueSatRadialPicker::paintEvent(QPaintEvent *) {
    Q_D(QwwHueSatRadialPicker);
    if (d->px.isNull()) d->buildPixmap();
    QPainter p(this);
    p.drawPixmap(0, 0, d->px);
    drawCrosshair(&p, d->m_pt);
}


void QwwHueSatRadialPicker::setValue(int v) {
    Q_D(QwwHueSatRadialPicker);
    if (v<0 || v>255 || v==d->m_value) return;
    d->m_value = v;
    emit valueChanged(v);
    d->conical.setStops(QGradientStops());
    for (int i=0;i<360;i++) {
        d->conical.setColorAt(i*1.0/360, QColor::fromHsv(i, 255, d->m_value));
    }
    d->conical.setColorAt(1, QColor::fromHsv(359, 255, d->m_value));
    d->buildPixmap();
    update();
}

/*!
 * \property QwwHueSatRadialPicker::value
 * \brief  Property holds the Value component of colors available in the picker.
 *
 */
int QwwHueSatRadialPicker::value() const {
    Q_D(const QwwHueSatRadialPicker);
    return d->m_value;
}

/*!
 *  \internal
 */
QSize QwwHueSatRadialPicker::sizeHint() const {
    return QSize(202, 202);
}

/*!
 *  \internal
 */
QSize QwwHueSatRadialPicker::minimumSizeHint() const {
    return QSize(202, 202);
}

/*!
 *  \internal
 */
int QwwHueSatRadialPicker::heightForWidth(int w) const {
    return w;
}

/*!
 *  \internal
 */
void QwwHueSatRadialPicker::resizeEvent(QResizeEvent *) {
    Q_D(QwwHueSatRadialPicker);
    if (width()!=height()) {
        int s = qMin(width(), height());
        resize(s,s);
        return;
    }
    d->buildPixmap();
    update();
}


/*!
 *  \internal
 */
void QwwHueSatRadialPicker::mousePressEvent(QMouseEvent * me) {
    Q_D(QwwHueSatRadialPicker);
    if (me->button()==Qt::LeftButton) {
        QPoint pt = me->pos();
        int r = d->radius(pt);
        if (r>=width()/2) return;
        d->m_pt = pt;
        int h = d->hue(pt);
        int s = (int)(r*255.0/(width()/2));
        emit colorPicked(QColor::fromHsv(h, s, d->m_value));
        update();
    } else QWidget::mousePressEvent(me);
}

/*!
 *  \internal
 */
void QwwHueSatRadialPicker::mouseMoveEvent(QMouseEvent * me) {
    Q_D(QwwHueSatRadialPicker);
    if (me->buttons() & Qt::LeftButton) {
        QPoint pt = me->pos();
        int r = d->radius(pt);
        if (r>=width()/2) {
            r = width()/2;
            pt = d->m_pt;
        }
        d->m_pt = pt;
        int h = d->hue(pt);
        int s = (int)(r*255.0/(width()/2));
        emit colorPicked(QColor::fromHsv(h, s, d->m_value));
        update();
    } else QWidget::mouseMoveEvent(me);
}

/*!
 *  \brief  Draws the crosshair on painter \a p at point \a pt.
 *
 *
 */
void QwwHueSatRadialPicker::drawCrosshair(QPainter * p, const QPoint & pt) {
    if (pt.isNull()) return;
    p->save();
    p->setPen(Qt::black);
    p->drawLine(pt-QPoint(0, -3), pt-QPoint(0, -1));
    p->drawLine(pt-QPoint(0, 1), pt-QPoint(0, 3));
    p->drawLine(pt-QPoint(-3, 0), pt-QPoint(-1, 0));
    p->drawLine(pt-QPoint(1, 0), pt-QPoint(3, 0));
    p->restore();
}

void QwwHueSatRadialPicker::setColor(const QColor & c) {
    Q_D(QwwHueSatRadialPicker);
    if (c==d->m_color) return;
    int h, s, v;
    c.getHsv(&h, &s, &v);
    if (v!=d->m_value) setValue(v);
    int r = qRound(s*1.0*(width()/2-2)/255.0);
    int x = qRound(r*cos(h*M_PI/180.0));
    int y = qRound(r*sin(h*M_PI/180.0));
    QPoint ctr = rect().center();
    d->m_pt = QPoint(x+ctr.x(), -y+ctr.y());
    d->m_color = c;
    emit colorPicked(c);
    update();
}

/*!
 * \property QwwHueSatRadialPicker::color
 * \brief  Property holds the currently picked color.
 *
 */
const QColor QwwHueSatRadialPicker::color() const {
    Q_D(const QwwHueSatRadialPicker);
    return d->m_color;
}

/*!
 *  \internal
 *
 */
void QwwHueSatRadialPicker::changeEvent(QEvent * ce) {
    Q_D(QwwHueSatRadialPicker);
    if (ce->type()==QEvent::PaletteChange||ce->type()==QEvent::ApplicationPaletteChange) {
        d->buildPixmap();
        update();
    }
    QWidget::changeEvent(ce);
}

#endif
