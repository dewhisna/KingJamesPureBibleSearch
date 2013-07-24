//
// C++ Interface: qwwled
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLED_H
#define QWWLED_H

#include <QWidget>
#include <wwglobal.h>
class QwwLedPrivate;


class Q_WW_EXPORT QwwLed : public QWidget, public QwwPrivatable
{
Q_OBJECT
Q_ENUMS(Shape)
Q_PROPERTY(QColor color READ color WRITE setColor)
Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
Q_PROPERTY(Shape shape READ shape WRITE setShape)
Q_PROPERTY(int frameWidth READ frameWidth WRITE setFrameWidth)
Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated)
Q_PROPERTY(int period READ period WRITE setPeriod)
public:
    enum Shape { Circular, RectangularSunken, RectangularRaised, RectangularPlain};
    QwwLed(QWidget *parent = 0);
    QwwLed(const QColor &col, Shape shap = Circular, QWidget *parent = 0);
    ~QwwLed();
    QSize sizeHint() const;
    QColor color() const;
    bool isChecked() const;
    int heightForWidth ( int w ) const;
    Shape shape() const;
    int frameWidth() const;
    int period() const;
    bool isAnimated() const;
public slots:
    void setColor(const QColor &c);
    void setChecked(bool);
    void setShape(Shape);
    void toggle();
    void setFrameWidth(int);
    void setAnimated(bool);
    void setPeriod(int);
    void blink();
signals:
    void colorChanged(const QColor &);
    void toggled(bool);
    void clicked();
protected:
    void paintEvent(QPaintEvent *pe);
    void mouseReleaseEvent(QMouseEvent *me);
    void timerEvent(QTimerEvent *te);
private:
    WW_DECLARE_PRIVATE(QwwLed);
};

#endif
