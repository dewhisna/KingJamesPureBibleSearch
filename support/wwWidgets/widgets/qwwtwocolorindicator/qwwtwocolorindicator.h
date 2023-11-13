//
// C++ Interface: qwwtwocolorindicator
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWTWOCOLORINDICATOR_H
#define QWWTWOCOLORINDICATOR_H

#ifndef WW_NO_TWOCOLORINDICATOR

#include <QWidget>
#include <wwglobal.h>

class QwwTwoColorIndicatorPrivate;
class Q_WW_EXPORT QwwTwoColorIndicator : public QWidget, QwwPrivatable
{
Q_OBJECT
Q_PROPERTY(QColor fgColor READ fgColor WRITE setFgColor)
Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
Q_PROPERTY(bool active READ isActive WRITE setActive)
Q_PROPERTY(bool dragEnabled READ dragEnabled WRITE setDragEnabled)
public:
    QwwTwoColorIndicator(QWidget *parent = nullptr);

    const QColor &fgColor() const;
    const QColor &bgColor() const;
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    bool isActive() const;
    bool dragEnabled() const;
    void setDragEnabled(bool);
public slots:
    void setFgColor(const QColor &);
    void setBgColor(const QColor &);
    void switchColors();
    void setActive(bool);
signals:
    void fgChanged(const QColor &);
    void bgChanged(const QColor &);
    void fgClicked();
    void bgClicked();
    void fgPressed();
    void bgPressed();
protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void dragEnterEvent(QDragEnterEvent*) override;
    virtual void dragMoveEvent(QDragMoveEvent*) override;
    virtual void dropEvent(QDropEvent*) override;
    virtual void paintSection(QPainter *p, const QRect &rect, const QColor &color);
private:
    WW_DECLARE_PRIVATE(QwwTwoColorIndicator);

};

#endif // WW_NO_TWOCOLORINDICATOR

#endif // QWWTWOCOLORINDICATOR_H
