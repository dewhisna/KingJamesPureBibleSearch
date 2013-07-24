//
// C++ Interface: qwwhuesatpicker
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWHUESATPICKER_H
#define QWWHUESATPICKER_H

#ifndef WW_NO_HUESATPICKER

#include <QFrame>
#include <QPixmap>
#include <wwglobal.h>

class QwwHueSatPickerPrivate;
class Q_WW_EXPORT QwwHueSatPicker : public QFrame, public QwwPrivatable {
    Q_OBJECT
    Q_PROPERTY(int minimumHue READ minimumHue WRITE setMinimumHue)
    Q_PROPERTY(int maximumHue READ maximumHue WRITE setMaximumHue)
    Q_PROPERTY(int minimumSat READ minimumSat WRITE setMinimumSat)
    Q_PROPERTY(int maximumSat READ maximumSat WRITE setMaximumSat)
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(QColor color READ color WRITE setColor)
public:
    QwwHueSatPicker(QWidget *parent = 0);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    int value() const;
    QColor color() const;
    int minimumSat() const;
    int maximumSat() const;
    int minimumHue() const;
    int maximumHue() const;
    void setHueRange(int mi, int ma);
    void setSatRange(int mi, int ma);
public slots:
    void setValue(int v);
    void setMinimumSat(int v);
    void setMaximumSat(int v);
    void setMinimumHue(int h);
    void setMaximumHue(int h);
    void setColor(const QColor &c);
signals:
    void valueChanged(int);
    void minimumHueChanged(int);
    void maximumHueChanged(int);
    void minimumSatChanged(int);
    void maximumSatChanged(int);
    void satRangeChanged(int, int);
    void hueRangeChanged(int, int);
    void colorPicked(QColor);
protected:
    virtual void drawCrosshair(QPainter *p, const QPoint &pt);
    void paintEvent(QPaintEvent *pe);
    void mousePressEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);
    void resizeEvent(QResizeEvent *);
private:
    WW_DECLARE_PRIVATE(QwwHueSatPicker);
    Q_DISABLE_COPY(QwwHueSatPicker);
};

#endif

#endif
