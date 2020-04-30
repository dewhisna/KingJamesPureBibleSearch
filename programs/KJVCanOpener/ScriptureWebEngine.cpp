/****************************************************************************
**
** Copyright (C) 2020 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#include "ScriptureWebEngine.h"

#include <QPalette>
#include <QFrame>
#include <QToolTip>
#include <QStyle>

// ============================================================================

CScriptureWebEngineView::CScriptureWebEngineView(QWidget *pParent)
	:	QWebEngineView(pParent)
{
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::BypassGraphicsProxyWidget | Qt::WindowTitleHint);
	setWindowTitle(tr("King James Pure Bible Search", "MainMenu"));
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QPalette pal;
	pal.setBrush(QPalette::All, QPalette::Base, QToolTip::palette().toolTipBase());
	pal.setBrush(QPalette::All, QPalette::Text, QToolTip::palette().toolTipText());
	setPalette(pal);
	ensurePolished();
//	setFrameStyle(QFrame::Box | QFrame::Plain);
//	setAlignment(Qt::AlignLeft);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / qreal(255.0));

}



// ============================================================================
