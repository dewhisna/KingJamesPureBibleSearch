/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "PhraseNavigatorEdit.h"

#include "PhraseCursor.h"
#include "ToolTipEdit.h"
#include <QToolTip>

// ============================================================================

void CPhraseNavigatorEdit::selectWords(const TPhraseTag &tag)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	CRelIndex ndxScroll = tag.relIndex();
	if (!ndxScroll.isColophon()) {
		if (m_pBibleDatabase->NormalizeIndex(CRelIndex(ndxScroll.book(), ndxScroll.chapter(), 1, 1)) == m_pBibleDatabase->NormalizeIndex(ndxScroll)) {
			ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading
		}
		ndxScroll.setWord(0);
	} else {
		// For colophons, goto word 1 and skip the normalization check (as it will be incorrect):
		ndxScroll.setWord(1);
	}

	m_TextEditor.scrollToAnchor(ndxScroll.asAnchor());

	CRelIndex ndxRel = tag.relIndex();
	if (ndxRel.isSet()) {
		int nStartPos = anchorPosition(ndxRel.asAnchor());
		int nEndPos = anchorPosition(m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(ndxRel) + tag.count() - 1).asAnchor());

		if (nStartPos != -1) {
			CPhraseCursor myCursor(m_TextEditor.textCursor(), m_pBibleDatabase.data(), true);
			myCursor.beginEditBlock();
			myCursor.setPosition(nStartPos);
			if ((nEndPos != -1) && (tag.count() > 0)) {
				myCursor.setPosition(nEndPos, QTextCursor::KeepAnchor);
				myCursor.moveCursorWordEnd(QTextCursor::KeepAnchor);
			} else {
				// Special-case for Book/Chapter tag so that it works (specially looks correctly) with the new
				//		special U+0x200B anchor tags now used in Book-Info and Chapter rendered text in CPhraseNavigator:
				if ((ndxRel.verse() == 0) && (ndxRel.word() == 0)) myCursor.movePosition(QTextCursor::StartOfLine);
			}
			myCursor.endEditBlock();
			m_TextEditor.setTextCursor(myCursor);
		}
		m_TextEditor.ensureCursorVisible();				// Hmmm, for some strange reason, this doen't always work when user has used mousewheel to scroll off.  Qt bug?
	}
}

CSelectionPhraseTagList CPhraseNavigatorEdit::getSelection() const
{
	return getSelection(CPhraseCursor(m_TextEditor.textCursor(), m_pBibleDatabase.data(), true));
}

CSelectedPhraseList CPhraseNavigatorEdit::getSelectedPhrases() const
{
	return getSelectedPhrases(CPhraseCursor(m_TextEditor.textCursor(), m_pBibleDatabase.data(), true));
}

bool CPhraseNavigatorEdit::handleToolTipEvent(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener, const QHelpEvent *pHelpEvent, CCursorFollowHighlighter *pHighlighter, const CSelectionPhraseTagList &selection) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	Q_ASSERT(pHelpEvent != nullptr);
	CSelectionPhraseTagList lstRefSelection = getSelection(CPhraseCursor(m_TextEditor.cursorForPosition(pHelpEvent->pos()), m_pBibleDatabase.data(), true));
	TPhraseTag tagReference = TPhraseTag(lstRefSelection.primarySelection().relIndex(), 1);
	QString strToolTip = getToolTip(nTipType, tagReference, selection);

	if (!strToolTip.isEmpty()) {
		if (pHighlighter) {
			highlightCursorFollowTag(*pHighlighter, (selection.haveSelection() ? static_cast<TPhraseTagList>(selection) : TPhraseTagList(tagReference)));
		}
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(nTipType, pCanOpener, pHelpEvent->globalPos(), strToolTip, &m_TextEditor);
		} else {
			QToolTip::showText(pHelpEvent->globalPos(), strToolTip);
		}
	} else {
		if (pHighlighter) {
			highlightCursorFollowTag(*pHighlighter);
		}
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::hideText(nTipType, pCanOpener);
		} else {
			QToolTip::hideText();
		}
		return false;
	}

	return true;
}

bool CPhraseNavigatorEdit::handleToolTipEvent(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener, CCursorFollowHighlighter *pHighlighter, const TPhraseTag &tag, const CSelectionPhraseTagList &selection) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	QString strToolTip = getToolTip(nTipType, tag, selection);

	if (!strToolTip.isEmpty()) {
		if (pHighlighter) {
			highlightCursorFollowTag(*pHighlighter, (selection.haveSelection() ? static_cast<TPhraseTagList>(selection) : TPhraseTagList(TPhraseTag(tag.relIndex(), 1))));
		}
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(nTipType, pCanOpener, m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip, m_TextEditor.viewport(), m_TextEditor.rect());
		} else {
			QToolTip::showText(m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip);
		}
	} else {
		if (pHighlighter) {
			highlightCursorFollowTag(*pHighlighter);
		}
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::hideText(nTipType, pCanOpener);
		} else {
			QToolTip::hideText();
		}
		return false;
	}

	return true;
}

void CPhraseNavigatorEdit::highlightCursorFollowTag(CCursorFollowHighlighter &aHighlighter, const TPhraseTagList &tagList) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	doHighlighting(aHighlighter, true);
	TPhraseTagList tagsToHighlight;
	for (int ndx = 0; ndx < tagList.size(); ++ndx) {
		TPhraseTag tag = tagList.at(ndx);
		// Highlight the word only if we have a reference for an actual word (not just a chapter or book or something):
		if ((tag.relIndex().book() != 0) &&
			(tag.relIndex().word() != 0) &&
			(tag.count() != 0)) {
			tagsToHighlight.append(tag);
		}
	}
	if (!tagsToHighlight.isEmpty()) {
		aHighlighter.setPhraseTags(tagsToHighlight);
		doHighlighting(aHighlighter);
	} else {
		aHighlighter.clearPhraseTags();
	}
}

// ============================================================================

