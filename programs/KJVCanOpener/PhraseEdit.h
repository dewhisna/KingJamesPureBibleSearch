#ifndef PHRASEEDIT_H
#define PHRASEEDIT_H

#include "dbstruct.h"
#include "Highlighter.h"

#include <QTextEdit>
#include <QTextCursor>
#include <QCompleter>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QHelpEvent>

// ============================================================================

class CParsedPhrase
{
public:
	CParsedPhrase(bool bCaseSensitive = false)
		:	m_bCaseSensitive(bCaseSensitive),
			m_nLevel(0),
			m_nCursorLevel(0),
			m_nCursorWord(-1),
			m_nLastMatchWord(-1)
	{ }
	~CParsedPhrase()
	{ }

	uint32_t GetNumberOfMatches() const;
	TIndexList GetNormalizedSearchResults() const;
	uint32_t GetMatchLevel() const;
	uint32_t GetCursorMatchLevel() const;
	QString GetCursorWord() const;
	int GetCursorWordPos() const;
	QString phrase() const;						// Return reconstituted phrase
	unsigned int phraseSize() const;			// Return number of words in reconstituted phrase

	virtual void ParsePhrase(const QTextCursor &curInsert);		// Parses the phrase in the editor.  Sets m_lstWords and m_nCursorWord
	virtual void ParsePhrase(const QString &strPhrase);			// Parses a fixed phrase

	virtual bool isCaseSensitive() const { return m_bCaseSensitive; }
	virtual void setCaseSensitive(bool bCaseSensitive) { m_bCaseSensitive = bCaseSensitive; }

protected:
	void UpdateCompleter(const QTextCursor &curInsert, QCompleter &aCompleter);
	QTextCursor insertCompletion(const QTextCursor &curInsert, const QString& completion);

private:
	void FindWords();			// Uses m_lstWords and m_nCursorWord to populate m_lstNextWords, m_lstMapping, and m_nLevel

protected:
	bool m_bCaseSensitive;
	uint32_t m_nLevel;			// Level of the search (Number of words matched).  This is the offset value for entries in m_lstMatchMapping (at 0 mapping is ALL words) (Set by FindWords())
	TIndexList m_lstMatchMapping;	// Mapping for entire search -- This is the search result, but with each entry offset by the search level (Set by FindWords())
	uint32_t m_nCursorLevel;	// Matching level at cursor
	TIndexList m_lstMapping;	// Mapping for search through current cursor (Set by FindWords())
	QStringList m_lstNextWords;	// List of words mapping next for this phrase (Set by FindWords())

	QStringList m_lstWords;		// Fully Parsed Word list.  Blank entries only at first or last entry to indicate an insertion point. (Filled by ParsePhrase())
	int m_nCursorWord;			// Index in m_lstWords where the cursor is at -- If insertion point is in the middle of two words, Cursor will be at the left word (Set by ParsePhrase())
	int m_nLastMatchWord;		// Index in m_lstWords where the last match was found (Set by FindWords())

	QStringList m_lstLeftWords;		// Raw Left-hand Words list from extraction.  Punctionation appears clustered in separate entities (Set by ParsePhrase())
	QStringList m_lstRightWords;	// Raw Right-hand Words list from extraction.  Punctionation appears clustered in separate entities (Set by ParsePhrase())
	QString m_strCursorWord;	// Word at the cursor point between the left and right hand halves (Set by ParsePhrase())
};

// ============================================================================

class CPhraseCursor : public QTextCursor
{
public:
	CPhraseCursor(const QTextCursor &aCursor);
	virtual ~CPhraseCursor();

	bool moveCursorCharLeft(MoveMode mode = MoveAnchor);
	bool moveCursorCharRight(MoveMode mode = MoveAnchor);
	QChar charUnderCursor();

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
	CPhraseNavigator(QTextEdit &textEditor, QObject *parent = NULL)
		:	QObject(parent),
			m_TextEditor(textEditor)
	{ }

	// AnchorPosition returns the document postion for the specified anchor or -1 if none found:
	int anchorPosition(const QString &strAnchorName) const;

	// ResolveCursorReference interprets anchors at the currext textCursor and
	//		backtracks until it finds an anchor to determine the relative index.
	//		Used mainly with the KJVBrowser, but also useful for search results
	//		review and navigator dialog preview:
	CRelIndex ResolveCursorReference(CPhraseCursor cursor) const;		// Bounds limited for words
	CRelIndex ResolveCursorReference2(CPhraseCursor cursor) const;		// This helper loop finds the reference, but will extend one word off the end of the verse when cursor is between verses

	// Highlight the areas marked in the PhraseTags.  If bClear=True, removes
	//		the highlighting, which is used to swapout the current tag list
	//		for a new one without redrawing everything.  ndxCurrent is used
	//		as an optimization to skip areas not within current chapter.  Use
	//		empty index to ignore.  Highlighting is done in the specified
	//		color.
	void doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear = false, const CRelIndex &ndxCurrent = CRelIndex()) const;

	// Text Fill/Select Functions:
	void fillEditorWithChapter(const CRelIndex &ndx);
	void fillEditorWithVerse(const CRelIndex &ndx);
	void selectWords(const CRelIndex &ndx, unsigned int nWrdCount);
	bool handleToolTipEvent(const QHelpEvent *pHelpEvent, CBasicHighlighter &aHighlighter) const;
	QString getToolTip(const CRelIndex &ndxReference) const;

signals:
	void changedEditorText();

private:
	QTextEdit &m_TextEditor;
};

// ============================================================================

#endif // PHRASEEDIT_H
