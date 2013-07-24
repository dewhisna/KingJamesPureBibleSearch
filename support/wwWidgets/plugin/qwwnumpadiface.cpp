//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "qwwnumpadiface.h"
#include "qwwnumpad.h"

QwwNumPadIface::QwwNumPadIface(QObject *parent)
        : wwWidgetInterface(parent){
            setGroup("[ww] Buttons");
            setDefaultGroup("Buttons");
        }


QwwNumPadIface::~QwwNumPadIface() {}

QWidget * QwwNumPadIface::createWidget(QWidget * parent) {
    return new QwwNumPad(parent);
}


QIcon QwwNumPadIface::icon() const
{
    return QPixmap(":/numpad.png");
}
