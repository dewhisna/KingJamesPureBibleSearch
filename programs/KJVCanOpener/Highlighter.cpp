/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

#include "Highlighter.h"

#include <QVariant>
#include <QBrush>
#include <QTextFormat>

//
// UserProperties:
//
#define USERPROP_FOREGROUND_BRUSH		0			//		+0		Foreground Brush
#define USERPROP_BACKGROUND_BRUSH		1			//		+1		Background Brush
#define USERPROP_UNDERLINE_COLOR		2			//		+2		Underline Color
#define USERPROP_UNDERLINE_STYLE		3			//		+3		Underline Style

// ============================================================================

const TPhraseTagList &CBasicHighlighter::getHighlightTags() const
{
	return m_lstPhraseTags;
}

void CBasicHighlighter::setHighlightTags(const TPhraseTagList &lstPhraseTags)
{
	m_lstPhraseTags = lstPhraseTags;
}

void CBasicHighlighter::clearPhraseTags()
{
	m_lstPhraseTags.clear();
}

// ============================================================================

void CSearchResultHighlighter::doHighlighting(QTextCharFormat &aFormat, bool bClear) const
{
	if ((!bClear) && (enabled())) {
		aFormat.setProperty(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH, QVariant(aFormat.foreground()));
		aFormat.setForeground(QBrush(QColor("blue")));				// TODO : Get properties from global settings!
	} else {
		if (aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH))
			aFormat.setForeground(aFormat.property(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH).value<QBrush>());
	}
}

// ============================================================================

Q_DECLARE_METATYPE(QTextCharFormat::UnderlineStyle)

void CCursorFollowHighlighter::doHighlighting(QTextCharFormat &aFormat, bool bClear) const
{
	if ((!bClear) && (enabled())) {
		aFormat.setProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR, QVariant(aFormat.underlineColor()));
		aFormat.setProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE, QVariant(aFormat.underlineStyle()));
		aFormat.setUnderlineColor(QColor("blue"));							// TODO : Get properties from global settings!
		aFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);		// TODO : Get properties from global settings!
	} else {
		if (aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR))
			aFormat.setUnderlineColor(aFormat.property(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR).value<QColor>());
		if (aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE))
			aFormat.setUnderlineStyle(aFormat.property(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE).value<QTextCharFormat::UnderlineStyle>());
	}
}

// ============================================================================


