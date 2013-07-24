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
#include "qwwrichtexteditiface.h"
#include "qwwrichtextedit.h"

QwwRichTextEditIface::QwwRichTextEditIface(QObject *parent)
        : wwWidgetInterface(parent){
            setGroup("[ww] Input Widgets");
            setDefaultGroup("Input Widgets");
        }

QwwRichTextEditIface::~QwwRichTextEditIface() {}

QWidget * QwwRichTextEditIface::createWidget(QWidget * parent) {
    return new QwwRichTextEdit(parent);
}

QIcon QwwRichTextEditIface::icon() const
{
    return QPixmap(":/trolltech/formeditor/images/widgets/textedit.png");
}
