/****************************************************************************
**
** Copyright (C) 2012-2023 Donna Whisnant, a.k.a. Dewtronics.
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

#include "PhraseCursor.h"
#include "ParseSymbols.h"
#include "dbstruct.h"

// ============================================================================

CPhraseCursor::CPhraseCursor(const QTextCursor &aCursor, CBibleDatabase *pBibleDatabase, bool bUseBibleBasedHyphenSeparator)
	:	QTextCursor(aCursor),
	m_pBibleDatabase(pBibleDatabase),
	m_bUseBibleBasedHyphenSeparator(bUseBibleBasedHyphenSeparator)
{
}

CPhraseCursor::CPhraseCursor(const CPhraseCursor &aCursor)
	:	QTextCursor(aCursor),
	m_pBibleDatabase(aCursor.m_pBibleDatabase),
	m_bUseBibleBasedHyphenSeparator(aCursor.m_bUseBibleBasedHyphenSeparator)
{
}

CPhraseCursor::CPhraseCursor(QTextDocument *pDocument, CBibleDatabase *pBibleDatabase, bool bUseBibleBasedHyphenSeparator)
	:	QTextCursor(pDocument),
	m_pBibleDatabase(pBibleDatabase),
	m_bUseBibleBasedHyphenSeparator(bUseBibleBasedHyphenSeparator)
{
}

CPhraseCursor::~CPhraseCursor()
{
}

bool CPhraseCursor::moveCursorCharLeft(MoveMode mode)
{
	return movePosition(PreviousCharacter, mode);
}

bool CPhraseCursor::moveCursorCharRight(MoveMode mode)
{
	return movePosition(NextCharacter, mode);
}

bool CPhraseCursor::charUnderCursorIsSeparator()
{
	bool bHyphenSeparators = false;		// True if Hyphens are a separator character

	// If the current word has hyphens in it, then hyphens are
	//	NOT a word separator character.  However, if not, then
	//	they should be considered separators to work with databases
	//	like OSHB that have hyphenated words where the separate
	//	halves are tagged as different lemma words:
	if (m_bUseBibleBasedHyphenSeparator && m_pBibleDatabase) {
		bHyphenSeparators = true;		// Flip the logic
		QString strAnchorName = charFormat().anchorNames().value(0);
		CRelIndex ndxAnchor(strAnchorName);
		if (ndxAnchor.isSet() && (ndxAnchor.word() != 0)) {
			QString strCursorWord = m_pBibleDatabase->wordAtIndex(ndxAnchor, WTE_RENDERED);
			if (!strCursorWord.isEmpty()) {
				bool bFound = false;
				for (int ndx = 0; ((ndx < strCursorWord.size()) && !bFound); ++ndx) {
					if (g_strHyphens.contains(strCursorWord.at(ndx))) bFound = true;
				}
				if (bFound) bHyphenSeparators = false;
			}
		}
	}

	QChar chrValue = charUnderCursor();
	if (bHyphenSeparators && g_strHyphens.contains(chrValue)) return true;
	return (chrValue.isSpace() || (chrValue == QChar('|')));
}

bool CPhraseCursor::moveCursorWordLeft(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word
	moveCursorCharLeft(mode);
	// If we are inside the "current word under the cursor", move left past it:
	while (!charUnderCursorIsSeparator()) {
		if (!moveCursorCharLeft(mode)) return false;
	}
	// We should now be between words, move left until we hit previous word:
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharLeft(mode)) return false;
	}
	// While in previous word, keep moving:
	while (!charUnderCursorIsSeparator()) {
		if (!moveCursorCharLeft(mode)) return true;		// If we hit the left edge, we have the final left word
	}
	// Here, we went one character too far.  So move back one:
	moveCursorCharRight(mode);
	return true;
}

bool CPhraseCursor::moveCursorWordRight(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word
	moveCursorCharLeft(mode);
	// If we are in the space between words, move right past it:
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	// If we are inside the "current word under the cursor", move right past it:
	while (!charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	// We should now be between word, move right until we hit next word:
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	return true;
}

bool CPhraseCursor::moveCursorWordStart(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word
	moveCursorCharLeft(mode);
	// If we're between words, move right until we get to the start of the word.
	//		Otherwise we're already somewhere inside the current word:
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;	// Yes, move to right as current word is the one on the righthand side
	}
	// We should now be inside the current word, move left until we find the left side:
	while (!charUnderCursorIsSeparator()) {
		if (!moveCursorCharLeft(mode)) return true;		// If we hit the left edge, we are at start of word already
	}
	// Here, we went one character too far.  So move back one:
	moveCursorCharRight(mode);
	return true;
}

bool CPhraseCursor::moveCursorWordEnd(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word already
	moveCursorCharLeft(mode);
	// If we're between words, the current word is to the right.  So move through
	//	the space until we find the word.  Otherwise, we should already be in
	//	the current word:
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;	// Move right here (not opposite of WordStart above), as current word is the one on the righthand side
	}
	// We're now inside the current word, move right until we hit the end:
	while (!charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return true;	// If we hit the right edge, we are at the end of the word
	}
	return true;
}

QString CPhraseCursor::wordUnderCursor()
{
	QString strRetVal;
	int nSelStart = anchor();
	int nSelEnd = position();
	clearSelection();

	// Find and return word we're on or just to our right:
	if (moveCursorWordStart(MoveAnchor)) {
		if (moveCursorWordEnd(KeepAnchor)) {
			strRetVal = selectedText();
		}
	}
	setPosition(nSelStart, MoveAnchor);
	setPosition(nSelEnd, KeepAnchor);
	return strRetVal;
}

void CPhraseCursor::selectWordUnderCursor()
{
	moveCursorWordStart(MoveAnchor);
	moveCursorWordEnd(KeepAnchor);
}

void CPhraseCursor::selectCursorToLineStart()
{
	clearSelection();
	movePosition(StartOfLine, KeepAnchor);
}

void CPhraseCursor::selectCursorToLineEnd()
{
	clearSelection();
	movePosition(EndOfLine, KeepAnchor);
}

// ============================================================================
