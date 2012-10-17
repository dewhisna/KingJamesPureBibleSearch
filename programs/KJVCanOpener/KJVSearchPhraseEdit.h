#ifndef KJVSEARCHPHRASEEDIT_H
#define KJVSEARCHPHRASEEDIT_H

#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>
#include <QCompleter>
#include <QStringList>

#include <QStatusBar>
#include <utility>

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
