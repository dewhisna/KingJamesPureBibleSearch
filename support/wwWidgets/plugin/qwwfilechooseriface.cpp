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
#include "qwwfilechooseriface.h"
#include "qwwfilechooser.h"

QwwFileChooserIface::QwwFileChooserIface(QObject *parent)
 : wwWidgetInterface(parent)
{
    setGroup("[ww] Input Widgets");
    setDefaultGroup("Input Widgets");
}


QWidget * QwwFileChooserIface::createWidget(QWidget * parent)
{
return new QwwFileChooser(parent);
}

QIcon QwwFileChooserIface::icon() const
{
    return QPixmap(":/fileopenicon");
}


