#ifndef KJVSEARCHPHRASEEDIT_H
#define KJVSEARCHPHRASEEDIT_H

#include "dbstruct.h"

#include <QWidget>
#include <QTextEdit>
#include <QTextCursor>
#include <QKeyEvent>
#include <QCompleter>
#include <QStringList>

#include <QStatusBar>
#include <utility>

class CPhraseLineEdit;		// Forward reference

class CParsedPhrase
{
public:
	CParsedPhrase()
	{ }
	~CParsedPhrase()
	{ }

	void UpdateCompleter(QCompleter &aCompleter);
	TIndexList GetNormalizedSearchResults() const;
	uint32_t GetMatchLevel() const;

	QTextCursor insertCompletion(const QTextCursor &curInsert, const QString& completion);

private:
	friend class CPhraseLineEdit;

	uint32_t m_nLevel;			// Level of the search (Number of words matched).  This is the offset value for entries in m_lstNextMapping (at 0 mapping is ALL words)
	TIndexList m_lstNextMapping;	// Next Mapping for search -- This is the search result, but with each entry offset by the search level
	QStringList m_lstNextWords;	// List of words mapping next for this phrase

	QStringList m_lstWords;		// Fully Parsed Word list.  Blank entries only at first or last entry to indicate an insertion point.
	int m_nCursorWord;			// Index in m_lstWords where the cursor is at -- If insertion point is in the middle of two words, Cursor will be at the left word

	QStringList m_lstLeftWords;		// Raw Left-hand Words list from extraction.  Punctionation appears clustered in separate entities
	QStringList m_lstRightWords;	// Raw Right-hand Words list from extraction.  Punctionation appears clustered in separate entities
	QString m_strCursorWord;	// Word at the cursor point between the left and right hand halves
};

class CPhraseLineEdit : public QTextEdit
{
	Q_OBJECT

public:
	CPhraseLineEdit(QWidget *pParent = 0);



private slots:
	void insertCompletion(const QString &completion);
	void on_textChanged();
	void on_cursorPositionChanged();

protected:
//	bool eventFilter(QObject *obj, QEvent *event);

	std::pair<QStringList, int> ParsePhrase();		// Parses the phrase in the editor.  Returns a word list and index of the word the cursor is on
	void UpdateCompleter();

private:
	void keyPressEvent(QKeyEvent* event);
	QString textUnderCursor() const;

// Data Private:
private:
	QCompleter *m_pCompleter;
	int m_nCompleterLevel;			// Level of completer: 0=Whole List, 1=One Word Complete, 2=Two Words, etc.
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
	~CKJVSearchPhraseEdit();

QStatusBar *pStatusBar;

/*
private slots:
	void on_textEdited(const QString &text);
	void insertCompletion(const QString &completion);

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private:
//	void keyPressEvent(QKeyEvent* event);

	QString textUnderCursor() const;

// Data Private:
private:
	QCompleter *m_pCompleter;
*/

// UI Private:
private:
	Ui::CKJVSearchPhraseEdit *ui;
};

#endif // KJVSEARCHPHRASEEDIT_H
