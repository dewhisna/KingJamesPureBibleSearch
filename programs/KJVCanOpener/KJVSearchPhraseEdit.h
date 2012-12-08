#ifndef KJVSEARCHPHRASEEDIT_H
#define KJVSEARCHPHRASEEDIT_H

#include "dbstruct.h"
#include "PhraseEdit.h"

#include <QWidget>
#include <QIcon>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPushButton>
#include <QMimeData>

#include <QTextEdit>
#include <QTextCursor>
#include <QString>
#include <QCompleter>

#include <QMenu>
#include <QContextMenuEvent>


// ============================================================================

class CPhraseLineEdit : public QTextEdit, public CParsedPhrase
{
	Q_OBJECT

public:
	CPhraseLineEdit(QWidget *pParent = 0);

	QMenu *getEditMenu() const { return m_pEditMenu; }

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
	void activatedPhraseEdit(const CPhraseLineEdit *pEditor);

protected:
//	bool eventFilter(QObject *obj, QEvent *event);

	virtual void insertFromMimeData(const QMimeData * source);
	virtual bool canInsertFromMimeData(const QMimeData *source) const;

	void UpdateCompleter();

	// TODO : Remove this and set parent to non-virtual after done debugging!
	virtual void ParsePhrase(const QTextCursor &curInsert);

protected:
	virtual void focusInEvent(QFocusEvent *event);
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
	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QAction *m_pStatusAction;		// Used to update the status bar without an enter/leave sequence
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

	const CParsedPhrase *parsedPhrase() const;

signals:
	void phraseChanged(const CParsedPhrase &phrase);
	void phraseListChanged();
	void activatedPhraseEdit(const CPhraseLineEdit *pEditor);

public slots:
	void showSeperatorLine(bool bShow = true);
	void enableCloseButton(bool bEnable = true);

protected slots:
	void on_closeSearchPhraseClicked();

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
