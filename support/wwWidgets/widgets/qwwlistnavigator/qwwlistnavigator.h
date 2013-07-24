//
// C++ Interface: qwwlistnavigator
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLISTNAVIGATOR_H
#define QWWLISTNAVIGATOR_H

#if defined(QT_NO_TOOLBUTTON) || defined(QT_NO_SLIDER)
#define WW_NO_LISTNAVIGATOR
#endif

#if !defined(WW_NO_LISTNAVIGATOR)

#include <wwglobal.h>
#include <QAbstractSlider>
#include <QMap>

class QwwListNavigatorPrivate;
class QwwListWidget;


class Q_WW_EXPORT QwwListNavigator : public QAbstractSlider, public QwwPrivatable {
    Q_OBJECT
    Q_FLAGS(Buttons)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation);
    Q_PROPERTY(Buttons buttons READ buttons WRITE setButtons);
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise);
    Q_PROPERTY(Qt::ToolButtonStyle toolButtonStyle READ toolButtonStyle WRITE setToolButtonStyle);
public:
    enum Button {
        NoButtons = 0,
        FirstButton = 1,
        PrevButton = 2,
        Slider = 4,
        NextButton = 8,
        LastButton = 16
    };
    Q_DECLARE_FLAGS(Buttons, Button)
    QwwListNavigator(QWidget *parent = 0);
    void setButtons(Buttons);
    Buttons buttons() const;
    bool autoRaise() const;
    Qt::ToolButtonStyle toolButtonStyle() const;
    void setListWidget(QwwListWidget *lw);

public slots:
    void setOrientation(Qt::Orientation o);
    void setAutoRaise(bool v);
    void setToolButtonStyle(Qt::ToolButtonStyle);
    void toFirst();
    void toLast();
    void toPrevious();
    void toNext();
signals:
    void first();
    void previous();
    void next();
    void last();
protected:
    void sliderChange ( SliderChange change );
private:
    Q_PRIVATE_SLOT(d_func(), void _q_valueChanged(int));
    Q_PRIVATE_SLOT(d_func(), void _q_updateLWRange());
    Q_PRIVATE_SLOT(d_func(), void _q_disconnectListWidget());
private:
    WW_DECLARE_PRIVATE(QwwListNavigator);
};

#endif
#endif
