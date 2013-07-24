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

#ifndef WW_NO_HUESATPICKER

#include "qwwhuesatpicker.h"
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QStyleOptionFrame>
#include "wwglobal_p.h"

const int wid = 360;
const int hei = 256;


class QwwHueSatPickerPrivate : public QwwPrivate {
public:
    QwwHueSatPickerPrivate(QwwHueSatPicker *q) : QwwPrivate(q) {
        m_minS = 0;
        m_maxS = 255;
        m_minH = 0;
        m_maxH = 359;
        m_v = 200;
        m_pt = QPoint(0, 0);
    }
    int hueFromX(int) const;
    int satFromY(int) const;
    int satToY(int) const;
    int hueToX(int) const;
    void buildPixmap();
    int m_minS;
    int m_maxS;
    int m_minH;
    int m_maxH;
    int m_v;
    QPoint m_pt;
    QPixmap px;
    QSize pxs;
    QColor m_color;
    WW_DECLARE_PUBLIC(QwwHueSatPicker);
};



/*!
 *  \class QwwHueSatPicker
 *  \brief The QwwHueSatPicker class provides a widget allowing color picking
 *         in a form of a hue-saturation rectangle.
 *
 *  \image qwwhuesatpicker.png QwwHueSatPicker
 *  \ingroup colorclasses
 */

/*!
 *  \property   QwwHueSatPicker::value
 *  \brief      This property holds the value component of the table visible
 *
 *  This property keeps the lightness component of the color table.
 *  The range of allowed values for this property is 0-255. The default value is 220.
 */

/*!
 *  \property   QwwHueSatPicker::color
 *  \brief      This property holds the currently chosen color.
 *
 *
 */

/*!
 *  \property   QwwHueSatPicker::minimumSat
 *  \brief      This property holds the minimum value of the saturation component shown by the widget.
 *
 *
 */

/*!
 *  \property   QwwHueSatPicker::maximumSat
 *  \brief      This property holds the maximum value of the saturation component shown by the widget.
 *
 *
 */

/*!
 *  \property   QwwHueSatPicker::minimumHue
 *  \brief      This property holds the minimum value of the hue component shown by the widget.
 *
 *
 */

/*!
 *  \property   QwwHueSatPicker::maximumHue
 *  \brief      This property holds the maximum value of the hue component shown by the widget.
 *
 *
 */
/*!
 *  \fn         QwwHueSatPicker::colorPicked(QColor color)
 *  \brief      This signal is emitted when \a color is picked.
 */
/*!
 *  \fn         QwwHueSatPicker::hueRangeChanged(int min, int max)
 *  \brief      This signal is emitted when the Hue range changes to (\a min, \a max)
 */

/*!
 * Constructs a hue sat picker with a given \a parent.
 */
QwwHueSatPicker::QwwHueSatPicker(QWidget *parent)
        : QFrame(parent), QwwPrivatable(new QwwHueSatPickerPrivate(this)) {
    Q_D(QwwHueSatPicker);
    d->buildPixmap();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}



/*!
 *  \brief Sets the current value
 */
void QwwHueSatPicker::setValue(int v) {
    Q_D(QwwHueSatPicker);
    if (d->m_v==v) return;
    if (v<0 || v>255) return;
    d->m_v = v;
    emit valueChanged(v);
    d->pxs=QSize();
    d->buildPixmap();
    update();

}

/*!
 * \internal
 */
QSize QwwHueSatPicker::sizeHint() const {
    Q_D(const QwwHueSatPicker);
    return QSize((d->m_maxH-d->m_minH)+2*frameWidth(), (d->m_maxS-d->m_minS)+2*frameWidth());
}

/*!
 * \internal
 */
QSize QwwHueSatPicker::minimumSizeHint() const {
    Q_D(const QwwHueSatPicker);
    return QSize(qMin((d->m_maxH-d->m_minH)/4, 40)+2*frameWidth(), qMin((d->m_maxS-d->m_minS)/4, 40)+2*frameWidth());
}

/*!
 * \internal
 */
void QwwHueSatPickerPrivate::buildPixmap() {
    Q_Q(QwwHueSatPicker);
    int cy = q->contentsRect().height();
    int cx = q->contentsRect().width();
    QImage img(cx, cy, QImage::Format_RGB32);
    for (int y=0; y<cy;y++)
        for (int x=0; x<cx;x++) {
            QColor c;
            c.setHsv(hueFromX(x), satFromY(y), m_v);
            img.setPixel(x, y, c.rgb());
        }
    px = QPixmap::fromImage(img);
    pxs = px.size();
}

/*!
 * \internal
 */
void QwwHueSatPicker::paintEvent(QPaintEvent *) {
    Q_D(QwwHueSatPicker);
    QPainter p(this);
    drawFrame(&p);
    QRect rct = contentsRect();
    QStyleOptionFrame opt;
    opt.initFrom(this);
    if (opt.state & QStyle::State_Enabled) {
        p.drawPixmap(rct.topLeft(), d->px);
        drawCrosshair(&p, d->m_pt);
    } else {
        QIcon i(d->px);
        i.paint(&p, rct, 0, QIcon::Disabled);
    }
}

/*!
 * \internal
 */
int QwwHueSatPickerPrivate::hueFromX(int x) const {
    Q_Q(const QwwHueSatPicker);
    return m_maxH-x*(m_maxH-m_minH)/q->contentsRect().width();
}

/*!
 * \internal
 */
int QwwHueSatPickerPrivate::satFromY(int y) const {
    \
    Q_Q(const QwwHueSatPicker);
    return m_maxS-y*(m_maxS-m_minS)/q->contentsRect().height();
}

/*!
 * \internal
 */
int QwwHueSatPickerPrivate::satToY(int s) const {
    Q_Q(const QwwHueSatPicker);
    float ys = q->contentsRect().height()*1.0/(m_maxS-m_minS);
    float dist = s*ys;
    return (int)(q->contentsRect().height() - dist);
}


/*!
 * \internal
 */
int QwwHueSatPickerPrivate::hueToX(int h) const {
    Q_Q(const QwwHueSatPicker);
    float xs = q->contentsRect().width()*1.0/(m_maxH-m_minH); // pixels per hue
    float dist = h*xs;
    return (int)(q->contentsRect().width()-dist);
}


/*!
 * \internal
 */
void QwwHueSatPicker::mousePressEvent(QMouseEvent * me) {
    Q_D(QwwHueSatPicker);
    if (me->button() == Qt::LeftButton) {
        QPoint pt = me->pos();
        int h = d->hueFromX(pt.x()-contentsRect().x());
        int s = d->satFromY(pt.y()-contentsRect().y());
        if (h<0 || s<0 || h>359 || s>255) return;
        emit colorPicked(QColor::fromHsv(h, s, d->m_v));
        d->m_pt = pt;
        update();
    } else QFrame::mousePressEvent(me);
}

/*!
 * \internal
 */
void QwwHueSatPicker::mouseMoveEvent(QMouseEvent * me) {
    Q_D(QwwHueSatPicker);
    if (me->buttons() & Qt::LeftButton) {
        QPoint pt = me->pos();
        if (!contentsRect().contains(pt)) return;
        int h = d->hueFromX(pt.x()-contentsRect().x());
        int s = d->satFromY(pt.y()-contentsRect().y());
        if (h<0 || s<0 || h>359 || s>255) return;
        emit colorPicked(QColor::fromHsv(h, s, d->m_v));
        d->m_pt = pt;
        update();
    } else QFrame::mouseMoveEvent(me);
}

/*!
 * \brief       This method draws the crosshair pointing the currently chosen color at point \a pt on painter \a p.
 * \param p     painter to draw on
 * \param pt    point to draw the cross in
 */
void QwwHueSatPicker::drawCrosshair(QPainter * p, const QPoint & pt) {
    p->save();
    p->setPen(Qt::black);
    p->drawLine(pt-QPoint(0, -3), pt-QPoint(0, -1));
    p->drawLine(pt-QPoint(0, 1), pt-QPoint(0, 3));
    p->drawLine(pt-QPoint(-3, 0), pt-QPoint(-1, 0));
    p->drawLine(pt-QPoint(1, 0), pt-QPoint(3, 0));
    p->restore();
}

/*!
 * 
 * \param v
 */
void QwwHueSatPicker::setMinimumSat(int v) {
    Q_D(QwwHueSatPicker);
    setSatRange(v, qMax(d->m_maxS, v));
}

/*!
 *
 * \param h
 */
void QwwHueSatPicker::setMinimumHue(int h) {
    Q_D(QwwHueSatPicker);
    setHueRange(h, qMax(d->m_maxH, h));
}

/*!
 *
 * \param h
 */
void QwwHueSatPicker::setMaximumHue(int h) {
    Q_D(QwwHueSatPicker);
    setHueRange(qMin(d->m_minH, h), h);
}

/*!
 *
 * \param mi
 * \param ma
 */
void QwwHueSatPicker::setHueRange(int mi, int ma) {
    Q_D(QwwHueSatPicker);
    if (mi<0 || ma < 0 || mi > 359 || ma>359 || mi >ma) return;
    bool maC = d->m_maxH!=ma;
    bool miC = d->m_minH!=mi;
    d->m_maxH = ma;
    d->m_minH = mi;
    if (maC) emit maximumHueChanged(ma);
    if (miC) emit minimumHueChanged(mi);
    if (miC || maC) emit hueRangeChanged(mi, ma);
    else return;

    updateGeometry();
    d->buildPixmap();
    update();
}

/*!
 *
 * \param v
 */
void QwwHueSatPicker::setMaximumSat(int v) {
    Q_D(QwwHueSatPicker);
    setSatRange(qMin(d->m_minS, v), v);
}

/*!
 *
 * \param mi
 * \param ma
 */
void QwwHueSatPicker::setSatRange(int mi, int ma) {
    Q_D(QwwHueSatPicker);
    if (mi<0 || ma < 0 || mi > 255 || ma>255 || mi >ma) return;
    bool maC = d->m_maxS!=ma;
    bool miC = d->m_minS!=mi;
    d->m_maxS = ma;
    d->m_minS = mi;
    if (maC) emit maximumSatChanged(ma);
    if (miC) emit minimumSatChanged(mi);
    if (miC || maC) emit satRangeChanged(mi, ma);
    else return;
//     m_maxS = ma;
//     m_minS = mi;
    updateGeometry();
    d->buildPixmap();
    update();
}

/*!
 *
 * \param c
 */
void QwwHueSatPicker::setColor(const QColor & c) {
    Q_D(QwwHueSatPicker);
    int h, s, v;
    if (d->m_color==c) return;
    c.getHsv(&h, &s, &v);
    if (d->m_v!=v) setValue(v);
    int Hspan = d->m_maxH - d->m_minH;
    int Sspan = d->m_maxS - d->m_minS;
    if (s<(d->m_minS) || s>(d->m_maxS)) {
        int ss = qMax(0, s-Sspan/2);
        setSatRange(ss, ss+Sspan);
    }
    if (h<(d->m_minH) || h>(d->m_maxH)) {
        int hh = qMax(0, h-Hspan/2);
        setHueRange(hh, hh+Hspan);
    }
    d->m_pt = QPoint(d->hueToX(h), d->satToY(s));
    d->m_color = c;
    emit colorPicked(c);
    update();
}

/*!
 * \internal
 */
void QwwHueSatPicker::resizeEvent(QResizeEvent *re) {
    Q_D(QwwHueSatPicker);
    QFrame::resizeEvent(re);
    d->buildPixmap();
    update();
}

QColor QwwHueSatPicker::color() const {
    Q_D(const QwwHueSatPicker);
    return d->m_color;
}


int QwwHueSatPicker::minimumSat() const {
    Q_D(const QwwHueSatPicker);
    return d->m_minS;
}

int QwwHueSatPicker::maximumSat() const {
    Q_D(const QwwHueSatPicker);
    return d->m_maxS;
}


int QwwHueSatPicker::maximumHue() const {
    Q_D(const QwwHueSatPicker);
    return d->m_maxH;
}


int QwwHueSatPicker::minimumHue() const {
    Q_D(const QwwHueSatPicker);
    return d->m_minH;
}


int QwwHueSatPicker::value() const {
    Q_D(const QwwHueSatPicker);
    return d->m_v;
}

#endif
