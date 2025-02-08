/****************************************************************************
**
** Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PHRASE_CURSOR_H
#define PHRASE_CURSOR_H

#include <QChar>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>

class CBibleDatabase;

// ============================================================================

class CPhraseCursor : public QTextCursor
{
public:
	CPhraseCursor(const QTextCursor &aCursor, CBibleDatabase *pBibleDatabase, bool bUseBibleBasedHyphenSeparator);
	CPhraseCursor(const CPhraseCursor &aCursor);
	CPhraseCursor(QTextDocument *pDocument, CBibleDatabase *pBibleDatabase, bool bUseBibleBasedHyphenSeparator);
	virtual ~CPhraseCursor();

	bool moveCursorCharLeft(MoveMode mode = MoveAnchor);
	bool moveCursorCharRight(MoveMode mode = MoveAnchor);
	inline QChar charUnderCursor()
	{
		return document()->characterAt(position());
	}
	bool charUnderCursorIsSeparator();				// True if charUnderCursor isSpace() or is a '|' character (as used for our 'OR' operator)

	bool moveCursorWordLeft(MoveMode mode = MoveAnchor);
	bool moveCursorWordRight(MoveMode mode = MoveAnchor);
	bool moveCursorWordStart(MoveMode mode = MoveAnchor);
	bool moveCursorWordEnd(MoveMode mode = MoveAnchor);
	QString wordUnderCursor();

	void selectWordUnderCursor();
	void selectCursorToLineStart();
	void selectCursorToLineEnd();

private:
	CBibleDatabase *m_pBibleDatabase;
	bool m_bUseBibleBasedHyphenSeparator;
};

// ============================================================================

#endif	// PHRASE_CURSOR_H

