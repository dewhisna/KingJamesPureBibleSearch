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

#ifndef TEXT_NAVIGATOR_H
#define TEXT_NAVIGATOR_H

#include "dbstruct.h"
#include "TextRenderer.h"
#include "PhraseParser.h"

#include <QFlags>
#include <QTextDocument>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QList>
#include <QSharedPointer>

// ============================================================================

// Forward declarations:
class CBasicHighlighter;

// ============================================================================

class CTextNavigator : public QObject
{
	Q_OBJECT
public:
	CTextNavigator(CBibleDatabasePtr pBibleDatabase, QTextDocument &textDocument, QObject *parent = nullptr)
		:	QObject(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_TextDocument(textDocument)
	{ }

	// AnchorPosition returns the document postion for the specified anchor or -1 if none found:
	int anchorPosition(const QString &strAnchorName) const;

	// Highlight the areas marked in the PhraseTags.  If bClear=True, removes
	//		the highlighting, which is used to swapout the current tag list
	//		for a new one without redrawing everything.  ndxCurrent is used
	//		as an optimization to skip areas not within current chapter.  Use
	//		empty index to ignore.  Highlighting is done in the specified
	//		color.
	void doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear, const CRelIndex &ndxCurrent) const;
	void doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear = false, const TPhraseTagList &tagCurrent = TPhraseTagList()) const;

	// Calculate a phrase tag (reference and word count) that represents the current
	//		display of our browser text having been set via setDocumentToChapter.  Used
	//		to calculate intersections with other tags for optimzing highlighting, etc:
	TPhraseTagList currentChapterDisplayPhraseTagList(const CRelIndex &ndxCurrent) const;

	// Text Fill Functions:
	// Returns unaltered raw-HTML text (as opposed to the QTextEdit changes to the HTML):
	QString setDocumentToBookInfo(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateBookInfoTextFlags));
	QString setDocumentToChapter(const CRelIndex &ndx,
								 TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateChapterTextFlags),
								 const CBasicHighlighter *pSRHighlighter = nullptr,
								 const CBasicHighlighter *pSRExclHighlighter = nullptr);
	QString setDocumentToVerse(const CRelIndex &ndx, const TPhraseTagList &tagsToInclude,
							   TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateVerseTextFlags),
							   const CBasicHighlighter *pSRHighlighter = nullptr,
							   const CBasicHighlighter *pSRExclHighlighter = nullptr);
	QString setDocumentToFormattedVerses(const TPhraseTagList &lstPhraseTags, TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateFormattedVersesTextFlags));		// Note: By definition, this one doesn't include anchors
	QString setDocumentToFormattedVerses(const TPassageTagList &lstPassageTags, TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateFormattedVersesTextFlags));	// Note: By definition, this one doesn't include anchors

	void clearDocument();

	CSelectionPhraseTagList getSelection(const CPhraseCursor &aCursor, bool bRecursion = false) const;		// Returns the tag for the cursor's currently selected text (less expensive than getSelectPhrase since we don't have to generate the CParsedPhrase object)
	CSelectedPhraseList getSelectedPhrases(const CPhraseCursor &aCursor) const;		// Returns the parsed phrase and tag for the cursor's currently selected text

	void removeAnchors();

	QString getToolTip(TIP_EDIT_TYPE_ENUM nTipType, const TPhraseTag &tag, const CSelectionPhraseTagList &selection, TOOLTIP_TYPE_ENUM nToolTipType = TTE_COMPLETE, bool bPlainText = false) const
	{
		return CTextRenderer::getToolTip(nTipType, m_pBibleDatabase.data(), tag, selection, nToolTipType, bPlainText);
	}

	QString getFootnote(const CRelIndex &ndx, bool bPlainText = false) const
	{
		return CTextRenderer::getFootnote(m_pBibleDatabase.data(), ndx, bPlainText);
	}

signals:
	void changedDocumentText();

protected:
	CBibleDatabasePtr m_pBibleDatabase;

private:
	QTextDocument &m_TextDocument;
};

// ============================================================================

#endif // TEXT_NAVIGATOR_H
