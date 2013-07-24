//
// C++ Interface: qwwbuttonlineedit_p
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWBUTTONLINEEDITPRIVATE_H
#define QWWBUTTONLINEEDITPRIVATE_H
#include "qwwbuttonlineedit.h"
#include "wwglobal_p.h"

class QwwButtonLineEditPrivate : public QwwPrivate {
public:
    QwwButtonLineEditPrivate(QwwButtonLineEdit *pub);
    void updateButtonPosition(int w);
    QToolButton *button;
    QwwButtonLineEdit::Position position;
    bool visible;
#if QT_VERSION < 0x040700
    QString placeholderText;
#endif
protected:
    WW_DECLARE_PUBLIC(QwwButtonLineEdit);
};

#endif
