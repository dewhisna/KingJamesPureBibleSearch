//
// C++ Interface: qwwnumpad
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWNUMPAD_H
#define QWWNUMPAD_H

#ifndef WW_NO_NUMPAD

#include <QWidget>
#include <wwglobal.h>

class QwwNumPadPrivate;
class Q_WW_EXPORT QwwNumPad : public QWidget, public QwwPrivatable
{
Q_OBJECT
public:
    QwwNumPad(QWidget *parent = 0);

public slots:
    void animateClick(const QString &b, int msec = 100);
signals:
    void numberClicked(int);
    void keyClicked(QString);
    void hashClicked();
    void asteriskClicked();
private:
    WW_DECLARE_PRIVATE(QwwNumPad);
    Q_PRIVATE_SLOT(d_func(), void _q_clicked(int));
};

#endif
#endif
