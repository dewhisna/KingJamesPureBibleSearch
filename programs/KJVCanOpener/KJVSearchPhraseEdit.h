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

#ifndef KJVSEARCHPHRASEEDIT_H
#define KJVSEARCHPHRASEEDIT_H

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "DelayedExecutionTimer.h"

#include <QWidget>
#include <QIcon>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QPushButton>
#include <QMimeData>

#include <QTextEdit>
#include <QTextCursor>
#include <QString>
#include <QCompleter>

#include <QMenu>
#include <QContextMenuEvent>

// ============================================================================

// CComposingCompleter -- Needed to fix a bug in Qt 4.8.x QCompleter whereby
//		inputMethod events get redirected to the popup, but don't come back
//		to the editor because inputContext()->setFocusWidget() never gets
//		called again for the editor:
class CComposingCompleter : public QCompleter
{
	Q_OBJECT

public:
	CComposingCompleter(QObject *parent = 0)
		:	QCompleter(parent)
	{

	}

	CComposingCompleter(QAbstractItemModel *model, QObject *parent = 0)
		:	QCompleter(model, parent)
	{

	}

	CComposingCompleter(const QStringList &list, QObject *parent = 0)
		:	QCompleter(list, parent)
	{

	}

	~CComposingCompleter()
	{

	}

	virtual bool eventFilter(QObject *obj, QEvent *ev);
};

// ============================================================================

class CPhraseLineEdit : public QTextEdit, public CParsedPhrase
{
	Q_OBJECT

public:
	explicit CPhraseLineEdit(QWidget *pParent = 0);
	CPhraseLineEdit(CBibleDatabasePtr pBibleDatabase, QWidget *pParent = 0);
	virtual ~CPhraseLineEdit();

	QMenu *getEditMenu() const { return m_pEditMenu; }
	QWidget *getDropListButton() const { return m_pButtonDroplist; }

	virtual bool isCaseSensitive() const { return CParsedPhrase::isCaseSensitive(); }
	virtual void setCaseSensitive(bool bCaseSensitive);

	virtual QSize sizeHint();

public slots:
	void on_phraseListChanged();

private slots:
	void insertCompletion(const QString &completion);
	void insertCommonPhraseCompletion(const QString &completion);
	void on_textChanged();
	void on_cursorPositionChanged();
	void on_dropCommonPhrasesClicked();

signals:
	void phraseChanged();
	void changeCaseSensitive(bool bCaseSensitive);
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);

protected:
	virtual void insertFromMimeData(const QMimeData * source);
	virtual bool canInsertFromMimeData(const QMimeData *source) const;

	void UpdateCompleter();

	// TODO : Remove this and set parent to non-virtual after done debugging!
	virtual void ParsePhrase(const QTextCursor &curInsert);

protected:
	virtual void wheelEvent(QWheelEvent *event);
	virtual void focusInEvent(QFocusEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void inputMethodEvent(QInputMethodEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	QString textUnderCursor() const;
	void setupCompleter(const QString &strText, bool bForce = false);

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	CComposingCompleter *m_pCompleter;			// Word completer
	QCompleter *m_pCommonPhrasesCompleter;		// Common phrases completer
	int m_nLastCursorWord;		// Used to dismiss and redisplay the popup for resizing
	bool m_bUpdateInProgress;	// Completer/Case-Sensivitity update in progress (to guard against re-entrance)

// UI Private:
private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog (useful for things like not disabling highlight, etc)
	QIcon m_icoDroplist;
	QPushButton *m_pButtonDroplist;		// Phrase Suggestions Droplist
	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QAction *m_pActionSelectAll;	// Edit menu select all function (needed to enable/disable based on text available)
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
	explicit CKJVSearchPhraseEdit(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase = true, QWidget *parent = 0);
	virtual ~CKJVSearchPhraseEdit();

	const CParsedPhrase *parsedPhrase() const;
	CPhraseLineEdit *phraseEditor() const;

signals:
	void closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
	void phraseListChanged();
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);

public slots:
	void showSeperatorLine(bool bShow = true);
	void enableCloseButton(bool bEnable = true);
	void focusEditor() const;
	void phraseStatisticsChanged() const;
	void closeSearchPhrase();

protected slots:
	void on_phraseChanged();
	void on_CaseSensitiveChanged(bool bCaseSensitive);
	void on_phraseAdd();
	void on_phraseDel();
	void on_phraseClear();

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	bool m_bHaveUserDatabase;			// True if there is a user database defined (used for enabling add/remove icons)
	CPhraseEntry m_phraseEntry;			// Last phrase entry (updated on phrase changed signal)
	bool m_bLastPhraseChangeHadResults;	// True if the last on_phraseChanged() notification from the phrase editor had resulting matches, used to optimize change notifications
	bool m_bUpdateInProgress;			// case-sensitivity update in progress (to guard against re-entrance)

// UI Private:
private:
	DelayedExecutionTimer m_dlyTextChanged;
	Ui::CKJVSearchPhraseEdit *ui;
};

#endif // KJVSEARCHPHRASEEDIT_H
