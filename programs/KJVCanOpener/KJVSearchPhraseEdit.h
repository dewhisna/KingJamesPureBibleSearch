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
#include "SubControls.h"

#include <QWidget>
#include <QIcon>
#include <QFocusEvent>
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

#include <assert.h>

// ============================================================================

class CPhraseLineEdit : public CSingleLineTextEdit, public CParsedPhrase
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

	virtual bool isAccentSensitive() const { return CParsedPhrase::isAccentSensitive(); }
	virtual void setAccentSensitive(bool bAccentSensitive);

	virtual bool isExcluded() const { return CParsedPhrase::isExcluded(); }
	virtual void setExclude(bool bExclude);

	inline bool isDisabled() const { assert(false); return false; }									// Call on either CKJVSearchPhraseEdit or CParsedPhrase
	inline void setIsDisabled(bool bIsDisabled) const { Q_UNUSED(bIsDisabled); assert(false); }		// Call on either CKJVSearchPhraseEdit or CParsedPhrase

	void processPendingUpdateCompleter();

public slots:
	virtual void en_textChanged();

private slots:
	void insertCompletion(const QString &completion);
	void insertCompletion(const QModelIndex &index);
	void insertCommonPhraseCompletion(const QString &completion);
	void en_dropCommonPhrasesClicked();
	void en_changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode);
	void delayed_UpdatedCompleter();

signals:
	void phraseChanged();
	void changeCaseSensitive(bool bCaseSensitive);
	void changeAccentSensitive(bool bAccentSensitive);
	void changeExclude(bool bExclude);
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);

protected:
	virtual void insertFromMimeData(const QMimeData * source);
	virtual bool canInsertFromMimeData(const QMimeData *source) const;

protected:
	virtual void focusInEvent(QFocusEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void setupCompleter(const QString &strText, bool bForce = false);
	virtual void UpdateCompleter();

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	SearchCompleter_t *m_pCompleter;			// Word completer
	QCompleter *m_pCommonPhrasesCompleter;		// Common phrases completer
	int m_nLastCursorWord;		// Used to dismiss and redisplay the popup for resizing

// UI Private:
private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog (useful for things like not disabling highlight, etc)
	QIcon m_icoDroplist;
	QPushButton *m_pButtonDroplist;		// Phrase Suggestions Droplist
	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QAction *m_pActionSelectAll;	// Edit menu select all function (needed to enable/disable based on text available)
	QAction *m_pStatusAction;		// Used to update the status bar without an enter/leave sequence
	DelayedExecutionTimer m_dlyUpdateCompleter;		// Activation delay for completer update to avoid extra updates
};

// ============================================================================

#include "ui_KJVSearchPhraseEdit.h"

class CKJVSearchPhraseEdit : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVSearchPhraseEdit(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase = true, QWidget *parent = 0);
	virtual ~CKJVSearchPhraseEdit();

	const CParsedPhrase *parsedPhrase() const;
	CPhraseLineEdit *phraseEditor() const;
	const CPhraseEntry &phraseEntry() const {return m_phraseEntry; }

	int searchActivationDelay() const { return m_dlyTextChanged.minimumDelay(); }

signals:
	void closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
	void resizing(CKJVSearchPhraseEdit *pSearchPhrase);
	void changingShowMatchingPhrases(CKJVSearchPhraseEdit *pSearchPhrase);
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);

public slots:
	void showSeperatorLine(bool bShow = true);
	void enableCloseButton(bool bEnable = true);
	void focusEditor() const;
	void phraseStatisticsChanged() const;
	void closeSearchPhrase();
	void clearSearchPhrase();
	void setDisabled(bool bDisabled);
	void setSearchActivationDelay(int nDelay) { m_dlyTextChanged.setMinimumDelay(nDelay); }

protected slots:
	void en_phraseChanged();
	void en_CaseSensitiveChanged(bool bCaseSensitive);
	void en_AccentSensitiveChanged(bool bAccentSensitive);
	void en_ExcludeChanged(bool bExclude);
	void en_phraseAdd();
	void en_phraseDel();
	void en_phraseClear();
	void setPhraseButtonEnables(const QString &strUUID = QString());

	virtual void resizeEvent(QResizeEvent *event);
	void en_showMatchingPhrases(bool bShow);

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	bool m_bHaveUserDatabase;			// True if there is a user database defined (used for enabling add/remove icons)
	CPhraseEntry m_phraseEntry;			// Last phrase entry (updated on phrase changed signal)
	bool m_bLastPhraseChangeHadResults;	// True if the last en_phraseChanged() notification from the phrase editor had resulting matches, used to optimize change notifications
	bool m_bUpdateInProgress;			// case-sensitivity update in progress (to guard against re-entrance)

// UI Private:
private:
	DelayedExecutionTimer m_dlyTextChanged;
	Ui::CKJVSearchPhraseEdit ui;
};

// ============================================================================

#endif // KJVSEARCHPHRASEEDIT_H
