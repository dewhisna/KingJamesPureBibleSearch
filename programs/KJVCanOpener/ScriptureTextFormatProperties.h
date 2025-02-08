/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SCRIPTURE_TEXT_FORMAT_PROPERTIES_H
#define SCRIPTURE_TEXT_FORMAT_PROPERTIES_H

// ============================================================================

//
// UserProperties:
// ---------------
//
// Highlighter.cpp:
#define USERPROP_FOREGROUND_BRUSH		(QTextFormat::UserProperty + 0)			//		0		Foreground Brush
#define USERPROP_BACKGROUND_BRUSH		(QTextFormat::UserProperty + 1)			//		1		Background Brush
#define USERPROP_UNDERLINE_COLOR		(QTextFormat::UserProperty + 2)			//		2		Underline Color
#define USERPROP_UNDERLINE_STYLE		(QTextFormat::UserProperty + 3)			//		3		Underline Style
#define USERPROP_FONT_STRIKE_OUT		(QTextFormat::UserProperty + 4)			//		4		Font Strike Out
//
// ScriptureDocument.cpp:
#define USERPROP_RELINDEX				(QTextFormat::UserProperty + 16)		//		16		RelIndex

//
// UserObjects:
// ------------
//
// ScriptureDocument.cpp:
#define USEROBJ_KJPBS_WORD				(QTextFormat::UserObject + 0)			//		0		KJPBS Word Object


// ============================================================================

#endif		// SCRIPTURE_TEXT_FORMAT_PROPERTIES_H
