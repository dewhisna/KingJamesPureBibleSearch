/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SEARCH_PHRASE_EDIT_H
#define SEARCH_PHRASE_EDIT_H

#include "dbstruct.h"
#include "PhraseParser.h"
#include "DelayedExecutionTimer.h"
#include "SubControls.h"
#include "PersistentSettings.h"

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

// Forward Declarations
class CMatchingPhrasesListModel;

// ============================================================================

struct TPhraseSettings {
	TPhraseSettings()
		:	m_bCaseSensitive(false),
			m_bAccentSensitive(false),
			m_bExclude(false),
			m_bDisabled(false)
	{
	}

	bool m_bCaseSensitive;
	bool m_bAccentSensitive;
	bool m_bExclude;
	QString m_strPhrase;
	bool m_bDisabled;
};

class CPhraseLineEdit : public CSingleLineTextEdit, public CParsedPhrase
{
	Q_OBJECT

public:
	explicit CPhraseLineEdit(QWidget *pParent = nullptr);
	CPhraseLineEdit(CBibleDatabasePtr pBibleDatabase, QWidget *pParent = nullptr);
	virtual ~CPhraseLineEdit();

	void setupPhrase(const TPhraseSettings &aPhrase);

	QMenu *getEditMenu() const { return m_pEditMenu; }
	QWidget *getDropListButton() const { return m_pButtonDroplist; }

	virtual bool isCaseSensitive() const override { return CParsedPhrase::isCaseSensitive(); }
	virtual void setCaseSensitive(bool bCaseSensitive) override;

	virtual bool isAccentSensitive() const override { return CParsedPhrase::isAccentSensitive(); }
	virtual void setAccentSensitive(bool bAccentSensitive) override;

	virtual bool isExcluded() const override { return CParsedPhrase::isExcluded(); }
	virtual void setExclude(bool bExclude) override;

	inline bool isDisabled() const { Q_ASSERT(false); return false; }									// Call on either CSearchPhraseEdit or CParsedPhrase
	inline void setIsDisabled(bool bIsDisabled) const { Q_UNUSED(bIsDisabled); Q_ASSERT(false); }		// Call on either CSearchPhraseEdit or CParsedPhrase

	virtual void setFromPhraseEntry(const CPhraseEntry &aPhraseEntry, bool bFindWords) override;

	void processPendingUpdateCompleter();

	int completerPopupDelay() const { return m_dlyPopupCompleter.minimumDelay(); }

public slots:
	virtual void en_textChanged() override;
	void setCompleterPopupDelay(int nDelay) { m_dlyPopupCompleter.setMinimumDelay(nDelay); }

private slots:
	void insertCompletion(const QString &completion);
	void insertCompletion(const QModelIndex &index);
	void insertCommonPhraseCompletion(const QString &completion);
	void en_dropCommonPhrasesClicked();
	void en_changedSearchPhraseCompleterFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nMode);
	void delayed_UpdatedCompleter();
	void popCompleter(bool bForce = false);
	void en_phraseChanged();

signals:
	void phraseChanged();
	void changeCaseSensitive(bool bCaseSensitive);
	void changeAccentSensitive(bool bAccentSensitive);
	void changeExclude(bool bExclude);
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);

	// Pass-through:
//	virtual void enterTriggered();						// Signaled when the user presses enter/return

protected:
	virtual void insertFromMimeData(const QMimeData * source) override;
	virtual bool canInsertFromMimeData(const QMimeData *source) const override;

protected:
	virtual void focusInEvent(QFocusEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void contextMenuEvent(QContextMenuEvent *event) override;
	virtual void setupCompleter(const QString &strText, bool bForce = false) override;
	virtual void UpdateCompleter() override;

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
	DelayedExecutionTimer m_dlyPopupCompleter;		// Delay time until completer is repopped (to help slow Windows)
};

// ============================================================================

#include "ui_SearchPhraseEdit.h"

class CSearchPhraseEdit : public QWidget
{
	Q_OBJECT

public:
	explicit CSearchPhraseEdit(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase = true, QWidget *parent = nullptr);
	virtual ~CSearchPhraseEdit();

	virtual bool eventFilter(QObject *pObject, QEvent *pEvent) override;

	void setupPhrase(const TPhraseSettings &aPhrase);

	const CParsedPhrase *parsedPhrase() const;
	CPhraseLineEdit *phraseEditor() const;
	const CPhraseEntry &phraseEntry() const {return m_phraseEntry; }

	int searchActivationDelay() const { return m_dlyTextChanged.minimumDelay(); }

signals:
	void closingSearchPhrase(CSearchPhraseEdit *pSearchPhrase);
	void phraseChanged(CSearchPhraseEdit *pSearchPhrase);
	void resizing(CSearchPhraseEdit *pSearchPhrase);
	void changingShowMatchingPhrases(CSearchPhraseEdit *pSearchPhrase);
	void activatedPhraseEditor(const CPhraseLineEdit *pEditor);
	void enterTriggered();						// Signaled when the user presses enter/return

public slots:
	void processPendingTextChanges();
	void showSeperatorLine(bool bShow = true);
	void enableCloseButton(bool bEnable = true);
	void focusEditor() const;
	void phraseStatisticsChanged() const;
	void closeSearchPhrase();
	void clearSearchPhrase();
	void setDisabled(bool bDisabled);
	void setSearchActivationDelay(int nDelay) { m_dlyTextChanged.setMinimumDelay(nDelay); }

protected slots:
	void en_matchingPhraseActivated(const QModelIndex &index);

	void en_phraseChanged();
	void en_CaseSensitiveChanged(bool bCaseSensitive);
	void en_AccentSensitiveChanged(bool bAccentSensitive);
	void en_ExcludeChanged(bool bExclude);
	void en_phraseAdd();
	void en_phraseDel();
	void en_phraseClear();
	void setPhraseButtonEnables(const QString &strUUID = QString());

	virtual void resizeEvent(QResizeEvent *event) override;
	void en_showMatchingPhrases(bool bShow);
	void setShowMatchingPhrases(bool bShow, bool bClearMatchingPhraseList);
	void en_changedHideMatchingPhrasesLists(bool bHideMatchingPhrasesLists);

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	bool m_bHaveUserDatabase;			// True if there is a user database defined (used for enabling add/remove icons)
	CPhraseEntry m_phraseEntry;			// Last phrase entry (updated on phrase changed signal)
	bool m_bLastPhraseChangeHadResults;	// True if the last en_phraseChanged() notification from the phrase editor had resulting matches, used to optimize change notifications
	bool m_bUpdateInProgress;			// case-sensitivity update in progress (to guard against re-entrance)

// UI Private:
private:
	CMatchingPhrasesListModel *m_pMatchingPhrasesModel;
	bool m_bMatchingPhrasesModelCurrent;			// True when we have filled in the matching phrases model, set back to false when the en_phraseChanged happens, so if user minimizes list it doesn't have to get recomputed on expanding
	DelayedExecutionTimer m_dlyTextChanged;
	Ui::CSearchPhraseEdit ui;
};

// ============================================================================

#endif // SEARCH_PHRASE_EDIT_H
