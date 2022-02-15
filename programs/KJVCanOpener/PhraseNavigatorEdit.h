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

#ifndef PHRASE_NAVIGATOR_EDIT_H
#define PHRASE_NAVIGATOR_EDIT_H

#include "dbstruct.h"
#include "Highlighter.h"

#include "PhraseNavigator.h"

#include <QTextEdit>
#include <QHelpEvent>

// Forward declarations:
class CKJVCanOpener;

// ============================================================================

class CPhraseNavigatorEdit : public CPhraseNavigator
{
	Q_OBJECT
public:
	CPhraseNavigatorEdit(CBibleDatabasePtr pBibleDatabase, QTextEdit &textEditor, bool bUseToolTipEdit = true, QObject *parent = nullptr)
		:	CPhraseNavigator(pBibleDatabase, *textEditor.document(), parent),
			m_TextEditor(textEditor),
			m_bUseToolTipEdit(bUseToolTipEdit)
	{
		Q_ASSERT(!m_pBibleDatabase.isNull());
	}

	// Text Selection/ToolTip Functions:
	void selectWords(const TPhraseTag &tag);
	using CPhraseNavigator::getSelection;
	using CPhraseNavigator::getSelectedPhrases;
	CSelectionPhraseTagList getSelection() const;		// Returns the tag for the cursor's currently selected text (less expensive than getSelectPhrase since we don't have to generate the CParsedPhrase object)
	CSelectedPhraseList getSelectedPhrases() const;		// Returns the parsed phrase and tag for the cursor's currently selected text
	bool handleToolTipEvent(CKJVCanOpener *pCanOpener, const QHelpEvent *pHelpEvent, CCursorFollowHighlighter &aHighlighter, const CSelectionPhraseTagList &selection) const;
	bool handleToolTipEvent(CKJVCanOpener *pCanOpener, CCursorFollowHighlighter &aHighlighter, const TPhraseTag &tag, const CSelectionPhraseTagList &selection) const;
	void highlightCursorFollowTag(CCursorFollowHighlighter &aHighlighter, const TPhraseTagList &tagList = TPhraseTagList()) const;

private:
	QTextEdit &m_TextEditor;
	bool m_bUseToolTipEdit;			// True = Use CToolTipEdit instead of QToolTip
};

// ============================================================================

#endif	// PHRASE_NAVIGATOR_EDIT_H
