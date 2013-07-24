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
#include "qwwrichtextbuttoniface.h"
#include "qwwrichtextbutton.h"

QwwRichTextButtonIface::QwwRichTextButtonIface(QObject *parent)
        : wwWidgetInterface(parent) {
    setGroup("[ww] Buttons");
    setDefaultGroup("Buttons");
        }


QwwRichTextButtonIface::~QwwRichTextButtonIface() {}




QWidget * QwwRichTextButtonIface::createWidget(QWidget * parent) {
    return new QwwRichTextButton(parent);
}

QIcon QwwRichTextButtonIface::icon() const {
return QPixmap(":/trolltech/formeditor/images/widgets/pushbutton.png");
}
