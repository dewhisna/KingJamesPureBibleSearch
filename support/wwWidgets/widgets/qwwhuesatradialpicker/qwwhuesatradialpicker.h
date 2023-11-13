//
// C++ Interface: qwwhuesatradialpicker
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWHUESATRADIALPICKER_H
#define QWWHUESATRADIALPICKER_H

#ifndef WW_NO_HUESATRADIALPICKER

#include <QWidget>
#include <QConicalGradient>
#include <wwglobal.h>

class QwwHueSatRadialPickerPrivate;
class Q_WW_EXPORT QwwHueSatRadialPicker : public QWidget, public QwwPrivatable {
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue);
    Q_PROPERTY(QColor color READ color WRITE setColor);
public:
    QwwHueSatRadialPicker(QWidget *parent = nullptr);
    int value() const;
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    virtual int heightForWidth(int w) const override;
    const QColor color() const;
public slots:
    void setValue(int v);
    void setColor(const QColor &c);
signals:
    void valueChanged(int);
    void colorPicked(QColor);
protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void mousePressEvent(QMouseEvent *me) override;
    virtual void mouseMoveEvent(QMouseEvent *me) override;
    virtual void changeEvent(QEvent *ce) override;
    virtual void drawCrosshair(QPainter *p, const QPoint &pt);
private:

    WW_DECLARE_PRIVATE(QwwHueSatRadialPicker);
    Q_DISABLE_COPY(QwwHueSatRadialPicker);
};

#endif
#endif
