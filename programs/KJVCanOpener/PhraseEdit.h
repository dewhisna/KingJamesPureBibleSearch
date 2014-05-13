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

#ifndef PHRASEEDIT_H
#define PHRASEEDIT_H

#include "dbstruct.h"
#include "Highlighter.h"
#include "VerseRichifier.h"

#include <QFlags>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextCursor>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QHelpEvent>
#include <QList>
#include <QSharedPointer>

#include <assert.h>

// ============================================================================

// Forward declarations:
class CSearchCompleter;
#if !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)
class CKJVCanOpener;
#endif
class CParsedPhrase;

// ============================================================================

class CSubPhrase {
public:
	CSubPhrase();
	~CSubPhrase();

	int GetMatchLevel() const;
	int GetCursorMatchLevel() const;
	QString GetCursorWord() const;
	int GetCursorWordPos() const;
	QString phrase() const;						// Return reconstituted phrase
	QString phraseRaw() const;					// Return reconstituted phrase without punctuation or regexp symbols
	int phraseSize() const;						// Return number of words in reconstituted phrase
	int phraseRawSize() const;					// Return number of words in reconstituted raw phrase
	QStringList phraseWords() const;			// Return reconstituted phrase words
	QStringList phraseWordsRaw() const;			// Return reconstituted raw phrase words

	bool isCompleteMatch() const;
	unsigned int GetNumberOfMatches() const;

	void ParsePhrase(const QString &strPhrase);
	void ParsePhrase(const QStringList &lstPhrase);

private:
	friend class CParsedPhrase;

	int m_nLevel;				// Level of the search (Number of words matched).  This is the offset value for entries in m_lstMatchMapping (at 0 mapping is ALL words) (Set by FindWords())
	TNormalizedIndexList m_lstMatchMapping;	// Mapping for entire search -- This is the search result, but with each entry offset by the search level (Set by FindWords())
	int m_nCursorLevel;			// Matching level at cursor
	TConcordanceList m_lstNextWords;	// List of words mapping next for this phrase (Set by FindWords()) (Stored as decomposed-normalized strings to help sorting order in auto-completer)

	QStringList m_lstWords;		// Fully Parsed Word list.  Blank entries only at first or last entry to indicate an insertion point. (Filled by ParsePhrase())
	int m_nCursorWord;			// Index in m_lstWords where the cursor is at -- If insertion point is in the middle of two words, Cursor will be at the left word (Set by ParsePhrase())
	QString m_strCursorWord;	// Word at the cursor point between the left and right hand halves (Set by ParsePhrase())
};

typedef QList< QSharedPointer<CSubPhrase> > TSubPhraseList;

// ============================================================================

class CParsedPhrase
{
public:
	CParsedPhrase(CBibleDatabasePtr pBibleDatabase = CBibleDatabasePtr(), bool bCaseSensitive = false, bool bAccentSensitive = false, bool bExclude = false);
	virtual ~CParsedPhrase();

	// ------- Helpers functions for CSearchCompleter and CSearchStringListModel usage:
	const TConcordanceList &nextWordsList() const;
	bool atEndOfSubPhrase() const;						// True if the cursor is at the end of the active subPhrase

	// ------- Helpers functions for data maintained by controlling CKJVCanOpener class to
	//			use for maintaining statistics about this phrase in context with others and
	//			to build Search Results Verse Lists, do highlighting, etc.
	inline bool isDuplicate() const { return m_bIsDuplicate; }
	inline void setIsDuplicate(bool bIsDuplicate) const { m_bIsDuplicate = bIsDuplicate; }
	inline bool isDisabled() const { return m_bIsDisabled; }
	inline void setIsDisabled(bool bIsDisabled) const { m_bIsDisabled = bIsDisabled; }
	inline int GetContributingNumberOfMatches() const { return m_lstScopedPhraseTagResults.size(); }
	inline const TPhraseTagList &GetScopedPhraseTagSearchResults() const { return m_lstScopedPhraseTagResults; }			// Returned as reference so we don't have to keep copying
	inline TPhraseTagList &GetScopedPhraseTagSearchResultsNonConst() const { return m_lstScopedPhraseTagResults; }			// Non-const version used by VerseListModel for setting
	inline void ClearScopedPhraseTagSearchResults() const { m_lstScopedPhraseTagResults.clear(); }
	inline int GetNumberOfMatchesWithin() const { return m_lstWithinPhraseTagResults.size(); }
	inline const TPhraseTagList &GetWithinPhraseTagSearchResults() const { return m_lstWithinPhraseTagResults; }			// Returned as reference so we don't have to keep copying
	inline TPhraseTagList &GetWithinPhraseTagSearchResultsNonConst() const { return m_lstWithinPhraseTagResults; }			// Non-const version used by VerseListModel for setting
	inline void ClearWithinPhraseTagSearchResults() const { m_lstWithinPhraseTagResults.clear(); }
	// -------
	bool isCompleteMatch() const;
	unsigned int GetNumberOfMatches() const;

	const TPhraseTagList &GetPhraseTagSearchResults() const;		// Returned as reference so we don't have to keep copying

	QString GetCursorWord() const;				// CursorWord for entire composite of all subPhrases
	int GetCursorWordPos() const;				// CursorWordPos for entire composite of all subPhrases -- includes tail insertion point for each subPhrase at each '|'
	QString phrase() const;						// Return reconstituted phrase
	QString phraseRaw() const;					// Return reconstituted phrase without punctuation or regexp symbols
	const QStringList phraseWords() const;		// Return reconstituted phrase words
	const QStringList phraseWordsRaw() const;	// Return reconstituted raw phrase words

	int subPhraseCount() const { return m_lstSubPhrases.size(); }
	int currentSubPhrase() const { return m_nActiveSubPhrase; }
	const CSubPhrase *subPhrase(int nIndex) const
	{
		assert((nIndex >= 0) && (nIndex < m_lstSubPhrases.size()));
		return m_lstSubPhrases.at(nIndex).data();
	}

	virtual void ParsePhrase(const QTextCursor &curInsert, bool bFindWords = true);		// Parses the phrase in the editor.  Sets m_lstWords and m_nCursorWord (Clears word cache and bFindWords determines if we FindWords() for our BibleDatabase)
	virtual void ParsePhrase(const QString &strPhrase);			// Parses a fixed phrase (Clears word cache and does NOT call FindWords())
	virtual void ParsePhrase(const QStringList &lstPhrase);		// Parses a fixed phrase already divided into words (like getSelectedPhrase from CPhraseNavigator) (Clears word cache and does NOT call FindWords())

	virtual bool isCaseSensitive() const { return m_bCaseSensitive; }
	virtual void setCaseSensitive(bool bCaseSensitive) { m_bCaseSensitive = bCaseSensitive; }

	virtual bool isAccentSensitive() const { return m_bAccentSensitive; }
	virtual void setAccentSensitive(bool bAccentSensitive) { m_bAccentSensitive = bAccentSensitive; }

	virtual bool isExcluded() const { return m_bExclude; }
	virtual void setExclude(bool bExclude) { m_bExclude = bExclude; }

	bool operator==(const CParsedPhrase &src) const
	{
		return ((m_bCaseSensitive == src.m_bCaseSensitive) &&
				(m_bAccentSensitive == src.m_bAccentSensitive) &&
				(m_bExclude == src.m_bExclude) &&
				(phrase().compare(src.phrase(), Qt::CaseSensitive) == 0));
	}
	bool operator!=(const CParsedPhrase &src) const
	{
		return (!(operator==(src)));
	}

	bool operator==(const CPhraseEntry &src) const
	{
		return ((m_bCaseSensitive == src.caseSensitive()) &&
				(m_bAccentSensitive == src.accentSensitive()) &&
				(m_bExclude == src.isExcluded()) &&
				(phrase().compare(src.text(), Qt::CaseSensitive) == 0));
	}
	bool operator!=(const CPhraseEntry &src) const
	{
		return (!(operator==(src)));
	}

	void UpdateCompleter(const QTextCursor &curInsert, CSearchCompleter &aCompleter);
	QTextCursor insertCompletion(const QTextCursor &curInsert, const QString& completion);
	void clearCache() const;

	void FindWords();								// Calls FindWords(subPhrase) with the ActiveSubPhrase
	void FindWords(CSubPhrase &subPhrase);			// Uses m_lstWords and m_nCursorWord to populate m_lstNextWords, m_lstMatchMapping, and m_nLevel

	inline const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase.data(); }

protected:
	CBibleDatabasePtr m_pBibleDatabase;
	mutable TPhraseTagList m_cache_lstPhraseTagResults;		// Cached Denormalized Search Results converted to phrase tags (Set on call to GetPhraseTagSearchResults, cleared on ClearCache)
	// -------
	mutable bool m_bIsDuplicate;							// Indicates this phrase is exact duplicate of another phrase.  Set/Cleared by parent phraseChanged logic.
	mutable bool m_bIsDisabled;								// Indicates this phrase is disabled.  Set/Cleared by parent phraseChanged logic
	mutable TPhraseTagList m_lstScopedPhraseTagResults;		// List of Denormalized Search Results from Scope.  Set/Cleared by parent phraseChanged logic and buildScopedResultsInParsedPhrases on VerseListModel.  The size of this list is the GetContributingNumberOfMatches
	mutable TPhraseTagList m_lstWithinPhraseTagResults;		// List of Denormalized Search Results from within Selected Search Documents (but not scoped with other phrases).  Set/Cleared by parent phraseChanged logic and buildScopedResultsInParsedPhrases on VerseListModel.  The size of this list is the GetNumberOfMatchesWithin
	// -------
	bool m_bCaseSensitive;
	bool m_bAccentSensitive;
	bool m_bExclude;

	int m_nActiveSubPhrase;

	TSubPhraseList m_lstSubPhrases;

};

typedef QList <const CParsedPhrase *> TParsedPhrasesList;

class CSelectedPhrase
{
public:
	CSelectedPhrase(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag, const QString &strPhrase, bool bCaseSensitive = false, bool bAccentSensitive = false, bool bExclude = false)
		:	m_ParsedPhrase(pBibleDatabase, bCaseSensitive, bAccentSensitive, bExclude),
			m_Tag(aTag)
	{
		m_ParsedPhrase.ParsePhrase(strPhrase);
	}

	inline const CParsedPhrase &phrase() const { return m_ParsedPhrase; }
	inline const TPhraseTag &tag() const { return m_Tag; }

	bool operator==(const CSelectedPhrase &src) const
	{
		return (m_Tag == src.m_Tag);
	}
	bool operator!=(const CSelectedPhrase &src) const
	{
		return (m_Tag != src.m_Tag);
	}

private:
	CParsedPhrase m_ParsedPhrase;
	TPhraseTag m_Tag;
};

class CSelectionPhraseTagList : public TPhraseTagList
{
public:
	bool haveSelection() const {
		if (size() == 0) return false;
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (at(ndx).haveSelection()) return true;
		}
		return false;
	}
	TPhraseTag primarySelection() const
	{
		if (haveSelection()) {
			for (int ndx = 0; ndx < size(); ++ndx) {
				if (at(ndx).haveSelection()) return at(ndx);
			}
		}
		// If we don't have a real selection (i.e. something with words), pick
		//		the first entry, which will contain a relIndex for a location:
		if (size() > 0) return at(0);
		return TPhraseTag();
	}
};

class  CSelectedPhraseList : public QList<CSelectedPhrase>
{
public:
	bool haveSelection() const {
		if (size() == 0) return false;
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (at(ndx).tag().haveSelection()) return true;
		}
		return false;
	}
	const CSelectedPhrase &primarySelectionPhrase() const
	{
		assert(size() != 0);
		assert(haveSelection());
		for (int ndx = 0; ndx < size(); ++ndx) {
			if (at(ndx).tag().haveSelection()) return at(ndx);
		}
		return at(0);
	}
	CSelectionPhraseTagList selection() const
	{
		CSelectionPhraseTagList lstSelection;
		for (int ndx = 0; ndx < size(); ++ndx) {
			lstSelection.append(at(ndx).tag());
		}
		return lstSelection;
	}
};

// ============================================================================

class CPhraseCursor : public QTextCursor
{
public:
	CPhraseCursor(const QTextCursor &aCursor);
	CPhraseCursor(const CPhraseCursor &aCursor);
	CPhraseCursor(QTextDocument *pDocument);
	virtual ~CPhraseCursor();

	bool moveCursorCharLeft(MoveMode mode = MoveAnchor);
	bool moveCursorCharRight(MoveMode mode = MoveAnchor);
	inline QChar charUnderCursor();
	inline bool charUnderCursorIsSeparator();				// True if charUnderCursor isSpace() or is a '|' character (as used for our 'OR' operator)

	bool moveCursorWordLeft(MoveMode mode = MoveAnchor);
	bool moveCursorWordRight(MoveMode mode = MoveAnchor);
	bool moveCursorWordStart(MoveMode mode = MoveAnchor);
	bool moveCursorWordEnd(MoveMode mode = MoveAnchor);
	QString wordUnderCursor();

	void selectWordUnderCursor();
	void selectCursorToLineStart();
	void selectCursorToLineEnd();
};

// ============================================================================

class CPhraseNavigator : public QObject
{
	Q_OBJECT
public:
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
		TRO_Copying = 0x800							// Text Copying mode (i.e. add selected font from copy option, etc)
	};
	Q_DECLARE_FLAGS(TextRenderOptionFlags, TextRenderOptions)

	enum REFERENCE_DELIMITER_MODE_ENUM {
		RDME_NO_NUMBER = -1,						// No Numbers (Verse Number Delimiter Type only)
		RDME_NO_DELIMITER = 0,						// No Delimiter
		RDME_SQUARE_BRACKETS = 1,					// Reference and/or Verse in Square Brackets:  [Genesis 1:1], [2]
		RDME_CURLY_BRACES = 2,						// Reference and/or Verse in Curly Braces: {Genesis 1:1}, {2}
		RDME_PARENTHESES = 3,						// Reference and/or Verse in Parentheses: (Genesis 1:1), (2)
		RDME_SUPERSCRIPT = 4						// Verse in Superscript (Verse Number Delimiter Type only)
	};

	enum TRANS_CHANGE_ADD_WORD_MODE_ENUM {
		TCAWME_NO_MARKING = 0,						// Remove delimiters from translation add/change word
		TCAWME_ITALICS = 1,							// Put translation add/change words in italics
		TCAWME_BRACKETS = 2							// Put brackets around translation add/change words
	};

	enum VERSE_RENDERING_MODE_ENUM {
		VRME_VPL = 0,								// Verse-Per-Line mode
		VRME_FF = 1,								// Display as Free-Flow/Paragraph mode
		VRME_VPL_HANGING = 2						// Verse Per-Line mode with Hanging Indent
	};

	enum COPY_FONT_SELECTION_ENUM {
		CFSE_NONE = 0,								// Do copying without any font hints
		CFSE_COPY_FONT = 1,							// Copy Font defined in settings
		CFSE_SCRIPTURE_BROWSER = 2,					// Use Scripture Browser's Current font setting
		CFSE_SEARCH_RESULTS = 3						// Use Search Results' Current font setting
	};

	CPhraseNavigator(CBibleDatabasePtr pBibleDatabase, QTextDocument &textDocument, QObject *parent = NULL);

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
#define defaultDocumentToBookInfoFlags	(CPhraseNavigator::TRO_Subtitles | \
										 CPhraseNavigator::TRO_Category)
#define defaultDocumentToChapterFlags	(CPhraseNavigator::TRO_UserNotes | \
										 CPhraseNavigator::TRO_UserNoteExpandAnchors | \
										 CPhraseNavigator::TRO_CrossRefs | \
										 CPhraseNavigator::TRO_Subtitles | \
										 CPhraseNavigator::TRO_Colophons | \
										 CPhraseNavigator::TRO_Superscriptions | \
										 CPhraseNavigator::TRO_Category)
#define defaultDocumentToVerseFlags		(CPhraseNavigator::TRO_None)
	// Returns unaltered raw-HTML text (as opposed to the QTextEdit changes to the HTML):
	QString setDocumentToBookInfo(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultDocumentToBookInfoFlags));
	QString setDocumentToChapter(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultDocumentToChapterFlags));
	QString setDocumentToVerse(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO = TextRenderOptionFlags(defaultDocumentToVerseFlags));
	QString setDocumentToFormattedVerses(const TPhraseTag &tagPhrase);		// Note: By definition, this one doesn't include anchors
	QString setDocumentToFormattedVerses(const TPassageTag &tagPassage);	// Note: By definition, this one doesn't include anchors

	static QString referenceStartingDelimiter();
	static QString referenceEndingDelimiter();

	CSelectionPhraseTagList getSelection(const CPhraseCursor &aCursor) const;		// Returns the tag for the cursor's currently selected text (less expensive than getSelectPhrase since we don't have to generate the CParsedPhrase object)
	CSelectedPhraseList getSelectedPhrases(const CPhraseCursor &aCursor) const;		// Returns the parsed phrase and tag for the cursor's currently selected text

	void removeAnchors();

signals:
	void changedDocumentText();

protected slots:
	void en_WordsOfJesusColorChanged(const QColor &color)
	{
		m_richifierTags.setWordsOfJesusTagsByColor(color);
	}
	void en_changedShowPilcrowMarkers(bool bShowPilcrowMarkers)
	{
		m_richifierTags.setShowPilcrowMarkers(bShowPilcrowMarkers);
	}

protected:
	CBibleDatabasePtr m_pBibleDatabase;
	CVerseTextRichifierTags m_richifierTags;	// Richifier tags used to render the text in this browser document

private:
	QTextDocument &m_TextDocument;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CPhraseNavigator::TextRenderOptionFlags)

// ============================================================================

class CPhraseEditNavigator : public CPhraseNavigator
{
	Q_OBJECT
public:
	CPhraseEditNavigator(CBibleDatabasePtr pBibleDatabase, QTextEdit &textEditor, bool bUseToolTipEdit = true, QObject *parent = NULL)
		:	CPhraseNavigator(pBibleDatabase, *textEditor.document(), parent),
			m_TextEditor(textEditor),
			m_bUseToolTipEdit(bUseToolTipEdit)
	{
		assert(m_pBibleDatabase.data() != NULL);
	}

	enum TOOLTIP_TYPE_ENUM {
		TTE_COMPLETE = 0,
		TTE_REFERENCE_ONLY = 1,
		TTE_STATISTICS_ONLY = 2
	};

	// Text Selection/ToolTip Functions:
	void selectWords(const TPhraseTag &tag);
	using CPhraseNavigator::getSelection;
	using CPhraseNavigator::getSelectedPhrases;
	CSelectionPhraseTagList getSelection() const;		// Returns the tag for the cursor's currently selected text (less expensive than getSelectPhrase since we don't have to generate the CParsedPhrase object)
	CSelectedPhraseList getSelectedPhrases() const;		// Returns the parsed phrase and tag for the cursor's currently selected text
#if !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)
	bool handleToolTipEvent(CKJVCanOpener *pCanOpener, const QHelpEvent *pHelpEvent, CCursorFollowHighlighter &aHighlighter, const CSelectionPhraseTagList &selection) const;
	bool handleToolTipEvent(CKJVCanOpener *pCanOpener, CCursorFollowHighlighter &aHighlighter, const TPhraseTag &tag, const CSelectionPhraseTagList &selection) const;
#endif
	void highlightCursorFollowTag(CCursorFollowHighlighter &aHighlighter, const TPhraseTag &tag = TPhraseTag()) const;
	QString getToolTip(const TPhraseTag &tag, const CSelectionPhraseTagList &selection, TOOLTIP_TYPE_ENUM nToolTipType = TTE_COMPLETE, bool bPlainText = false) const;

private:
	QTextEdit &m_TextEditor;
	bool m_bUseToolTipEdit;			// True = Use CToolTipEdit instead of QToolTip
};

// ============================================================================

#endif // PHRASEEDIT_H
