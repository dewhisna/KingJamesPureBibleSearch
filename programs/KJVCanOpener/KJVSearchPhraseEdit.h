#ifndef KJVSEARCHPHRASEEDIT_H
#define KJVSEARCHPHRASEEDIT_H

#include "dbstruct.h"

#include <QWidget>
#include <QTextEdit>
#include <QTextCursor>
#include <QKeyEvent>
#include <QCompleter>
#include <QStringList>
#include <QIcon>
#include <QPushButton>
#include <QMimeData>
#include <QColor>

#include <QStatusBar>
#include <utility>

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

class CPhraseNavigator
{
public:
	CPhraseNavigator(QTextEdit &textEditor)
		: m_TextEditor(textEditor)
	{ }

	// AnchorPosition returns the document postion for the specified anchor or -1 if none found:
	int anchorPosition(const QString &strAnchorName) const;

	// ResolveCursorReference interprets anchors at the currext textCursor and
	//		backtracks until it finds an anchor to determine the relative index.
	//		Used mainly with the KJVBrowser, but also useful for search results
	//		review and navigator dialog preview:
	CRelIndex ResolveCursorReference(CPhraseCursor cursor);			// Bounds limited for words
	CRelIndex ResolveCursorReference2(CPhraseCursor cursor);		// This helper loop finds the reference, but will extend one word off the end of the verse when cursor is between verses

	// Highlight the areas marked in the PhraseTags.  If bClear=True, removes
	//		the highlighting, which is used to swapout the current tag list
	//		for a new one without redrawing everything.  ndxCurrent is used
	//		as an optimization to skip areas not within current chapter.  Use
	//		empty index to ignore.  Highlighting is done in the specified
	//		color.
	void doHighlighting(const TPhraseTagList &lstPhraseTags, const QColor &colorHighlight, bool bClear = false, const CRelIndex &ndxCurrent = CRelIndex());

private:
	QTextEdit &m_TextEditor;
};

// ============================================================================

class CPhraseLineEdit : public QTextEdit, CParsedPhrase
{
	Q_OBJECT

public:
	CPhraseLineEdit(QWidget *pParent = 0);

	virtual bool isCaseSensitive() const { return CParsedPhrase::isCaseSensitive(); }
	virtual void setCaseSensitive(bool bCaseSensitive);

public slots:
	void on_phraseListChanged();

private slots:
	void insertCompletion(const QString &completion);
	void insertCommonPhraseCompletion(const QString &completion);
	void on_textChanged();
	void on_cursorPositionChanged();
	void on_dropCommonPhrasesClicked();

signals:
	void phraseChanged(const CParsedPhrase &phrase);
	void changeCaseSensitive(bool bCaseSensitive);

protected:
//	bool eventFilter(QObject *obj, QEvent *event);

	virtual void insertFromMimeData(const QMimeData * source);

	void UpdateCompleter();

	// TODO : Remove this and set parent to non-virtual after done debugging!
	virtual void ParsePhrase(const QTextCursor &curInsert);

protected:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	QString textUnderCursor() const;

// Data Private:
private:
	QCompleter *m_pCompleter;					// Word completer
	QCompleter *m_pCommonPhrasesCompleter;		// Common phrases completer
	int m_nLastCursorWord;		// Used to dismiss and redisplay the popup for resizing
	bool m_bUpdateInProgress;	// Completer/Case-Sensivitity update in progress (to guard against re-entrance)

// UI Private:
private:
	QIcon m_icoDroplist;
	QPushButton *m_pButtonDroplist;		// Phrase Suggestions Droplist
};

// ============================================================================

namespace Ui {
class CKJVSearchPhraseEdit;
}

class CKJVSearchPhraseEdit : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVSearchPhraseEdit(QWidget *parent = 0);
	virtual ~CKJVSearchPhraseEdit();

QStatusBar *pStatusBar;


signals:
	void phraseChanged(const CParsedPhrase &phrase);
	void phraseListChanged();

protected slots:
	void on_phraseChanged(const CParsedPhrase &phrase);
	void on_CaseSensitiveChanged(bool bCaseSensitive);
	void on_phraseAdd();
	void on_phraseDel();
	void on_phraseClear();

// Data Private:
private:
	CPhraseEntry m_phraseEntry;			// Last phrase entry (updated on phrase changed signal)
	bool m_bUpdateInProgress;			// case-sensitivity update in progress (to guard against re-entrance)

// UI Private:
private:
	Ui::CKJVSearchPhraseEdit *ui;
};

#endif // KJVSEARCHPHRASEEDIT_H
