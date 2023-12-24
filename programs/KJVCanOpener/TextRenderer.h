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

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <QFlags>

// ============================================================================

enum TOOLTIP_TYPE_ENUM {
	TTE_COMPLETE = 0,
	TTE_REFERENCE_ONLY = 1,
	TTE_STATISTICS_ONLY = 2
};

enum TextRenderOptions {
	TRO_None = 0x0,								// Default for no options
	TRO_NoAnchors = 0x1,						// Suppresses internal navigation anchors
	TRO_AddDividerLineBefore = 0x2,				// Add <hr> line before (Verse output only)
	TRO_Subtitles = 0x4,						// Add book subtitles (descriptions)
	TRO_Colophons = 0x8,						// Add book colophons (at end)
	TRO_Superscriptions = 0x10,					// Add chapter superscriptions
	TRO_UserNotes = 0x20,						// Displays active/visible user notes
	TRO_UserNotesForceVisible = 0x40,			// Force show user notes (Force only flag)
	TRO_AllUserNotesVisible = 0x60,				// Force show all user notes (Combines with UserNotes for setting both)
	TRO_UserNoteExpandAnchors = 0x80,			// Add navigation anchors to expand/collapse User Notes
	TRO_CrossRefs = 0x100,						// Add navigation anchors/text for cross-references
	TRO_Category = 0x200,						// Add book category
	TRO_SuppressPrePostChapters = 0x400,		// Suppress adding pre/post chapter displays
	TRO_Copying = 0x800,						// Text Copying mode (i.e. add selected font from copy option, etc)
	TRO_ScriptureBrowser = 0x1000,				// Rendering Scripture Browser Text
	TRO_SearchResults = 0x2000,					// Rendering Search Results Text
	TRO_InnerHTML = 0x4000,						// Generate Inner-HTML Only (i.e. no header and body tags)
	TRO_NoWordAnchors = 0x8000,					// Disables per-word anchors and uses verse, chapter, book anchors only (superceded if TRO_NoAnchors is set)
	TRO_NoVerseAnchors = 0x10000,				// Disables per-verse anchors and uses word, chapter, book anchors only (superceded if TRO_NoAnchors is set)
	TRO_NoChapterAnchors = 0x20000,				// Disables per-chapter anchors and uses word, verse, book anchors only (superceded if TRO_NoAnchors is set)
	TRO_NoBookAnchors = 0x40000,				// Disables per-book anchors and uses word, verse, chapter anchors only (superceded if TRO_NoAnchors is set)
	TRO_NoCrossRefAnchors = 0x80000,			// Disables navigation anchors for CrossRefs (superceded by TRO_NoAnchors)
	TRO_NoFootnoteAnchors = 0x100000,			// Disables outputting anchors for Footnotes (superceded by TRO_NoAnchors)
	TRO_NoColophonAnchors = 0x200000,			// If TRO_NoWordAnchors is used, the entire Colophon will be anchored, unless this flag is set (superceded if TRO_NoAnchors is set)
	TRO_NoSuperscriptAnchors = 0x400000,		// If TRO_NoWordAnchors is used, the entire Superscription will be anchored, unless this flag is set (superceded if TRO_NoAnchors is set)
	TRO_UseLemmas = 0x800000,					// Render Lemmas interlinearly with verses
	TRO_UseWordSpans = 0x1000000,				// Output Word-Spans in HTML (this is implied when TRO_UseLemmas is specified)
	TRO_InlineFootnotes = 0x2000000,			// Render inline footnotes in verses
	TRO_NoQTextDocument = 0x4000000,			// Disable functionality requiring QTextDocument (i.e. no clear() or setHtml() calls, no indent/hanging-indent VRME modes)
	TRO_NoIndent = 0x8000000,					// Disable Indent and Hanging-Indent VRME modes
};
Q_DECLARE_FLAGS(TextRenderOptionFlags, TextRenderOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(TextRenderOptionFlags)

enum REFERENCE_DELIMITER_MODE_ENUM {
	RDME_NO_NUMBER = -1,						// No Numbers (Verse Number Delimiter Type only)
	RDME_NO_DELIMITER = 0,						// No Delimiter
	RDME_SQUARE_BRACKETS = 1,					// Reference and/or Verse in Square Brackets:  [Genesis 1:1], [2]
	RDME_CURLY_BRACES = 2,						// Reference and/or Verse in Curly Braces: {Genesis 1:1}, {2}
	RDME_PARENTHESES = 3,						// Reference and/or Verse in Parentheses: (Genesis 1:1), (2)
	RDME_SUPERSCRIPT = 4,						// Verse in Superscript (Verse Number Delimiter Type only)
	RDME_COMPLETE_REFERENCE = 5					// Verse rendered as full reference (Verse Number Delimiter Type only)
};

enum TRANS_CHANGE_ADD_WORD_MODE_ENUM {
	TCAWME_NO_MARKING = 0,						// Remove delimiters from translation add/change word
	TCAWME_ITALICS = 1,							// Put translation add/change words in italics
	TCAWME_BRACKETS = 2							// Put brackets around translation add/change words
};

enum VERSE_RENDERING_MODE_ENUM {
	VRME_FF = 0,								// Display as Free-Flow/Paragraph mode
	VRME_VPL = 1,								// Verse-Per-Line mode
	VRME_VPL_DS = 2,							// Verse Per-Line mode Double-Spaced
	VRME_VPL_INDENT = 3,						// Verse Per-Line mode with Indent
	VRME_VPL_HANGING = 4,						// Verse Per-Line mode with Hanging Indent
	VRME_VPL_DS_INDENT = 5,						// Verse Per-Line mode Double-Spaced with Indent
	VRME_VPL_DS_HANGING = 6						// Verse Per-Line mode Double-Spaced with Hanging Indent
};

enum COPY_FONT_SELECTION_ENUM {
	CFSE_NONE = 0,								// Do copying without any font hints
	CFSE_COPY_FONT = 1,							// Copy Font defined in settings
	CFSE_SCRIPTURE_BROWSER = 2,					// Use Scripture Browser's Current font setting
	CFSE_SEARCH_RESULTS = 3						// Use Search Results' Current font setting
};

enum FootnoteRenderingModes {
	FRME_NONE = 0x0,							// Don't render Bible footnotes
	FRME_INLINE = 0x1,							// Render footnotes inline with verse text
	FRME_STATUS_BAR = 0x2,						// Render footnotes on the status bar
	// Additional types to consider : popup and/or tooltip rendering
};
Q_DECLARE_FLAGS(FootnoteRenderingModeFlags, FootnoteRenderingModes)


// Text Generation Function Defaults:
#define defaultGenerateBookInfoTextFlags		(TRO_Subtitles | \
												 TRO_Category | \
												 TRO_Colophons)
#define defaultGenerateChapterTextFlags			(TRO_UserNotes | \
												 TRO_UserNoteExpandAnchors | \
												 TRO_CrossRefs | \
												 TRO_Subtitles | \
												 TRO_Colophons | \
												 TRO_Superscriptions | \
												 TRO_Category)
#define defaultGenerateVerseTextFlags			(TRO_None)
#define defaultGenerateFormattedVersesTextFlags	(TRO_NoAnchors | \
												 TRO_Copying)

// ============================================================================

// Forward declarations
class CBibleDatabase;
class CRelIndex;
class CBasicHighlighter;
class TPhraseTagList;
class TPassageTagList;

// ============================================================================

class CTextRenderer
{
public:
	static QString generateTextForBookInfo(CBibleDatabase *pBibleDatabase,
										   const CRelIndex &ndx,
										   TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateBookInfoTextFlags));
	static QString generateTextForChapter(CBibleDatabase *pBibleDatabase,
										  qreal nIndentWidth,
										  const CRelIndex &ndx,
										  TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateChapterTextFlags),
										  const CBasicHighlighter *pHighlighter = nullptr);
	static QString generateTextForVerse(CBibleDatabase *pBibleDatabase,
										const CRelIndex &ndx,
										const TPhraseTagList &tagsToInclude,
										TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateVerseTextFlags),
										const CBasicHighlighter *pHighlighter = nullptr);
	static QString generateTextForFormattedVerses(CBibleDatabase *pBibleDatabase,
												  qreal nIndentWidth,
												  const TPhraseTagList &lstPhraseTags,
												  TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateFormattedVersesTextFlags));	// Note: By definition, this one doesn't include anchors
	static QString generateTextForFormattedVerses(CBibleDatabase *pBibleDatabase,
												  qreal nIndentWidth,
												  const TPassageTagList &lstPassageTags,
												  TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultGenerateFormattedVersesTextFlags));	// Note: By definition, this one doesn't include anchors
};

// ============================================================================

#endif	// TEXT_RENDERER_H

