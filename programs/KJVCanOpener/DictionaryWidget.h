/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef DICTIONARY_WIDGET_H
#define DICTIONARY_WIDGET_H

#include "dbstruct.h"

#include "DelayedExecutionTimer.h"
#include "SubControls.h"

#include <QWidget>
#include <QModelIndex>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QPointer>

// ============================================================================

class CDictionaryLineEdit : public CSingleLineTextEdit
{
	Q_OBJECT

public:
	explicit CDictionaryLineEdit(QWidget *pParent = 0);
	virtual ~CDictionaryLineEdit();

	void initialize(CDictionaryDatabasePtr pDictionary);
	void setDictionary(CDictionaryDatabasePtr pDictionary);

	void processPendingUpdateCompleter();

public slots:
	void insertCompletion(const QString &strWord);

protected slots:
	void insertCompletion(const QModelIndex &index);

	virtual void en_cursorPositionChanged();

private slots:
	void en_changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode);
	void delayed_UpdatedCompleter();

protected:
	virtual void setupCompleter(const QString &strText, bool bForce = false);
	virtual void UpdateCompleter();

// Data Private:
private:
	CDictionaryDatabasePtr m_pDictionaryDatabase;
	SearchCompleter_t *m_pCompleter;			// Word completer
	bool m_bUpdateInProgress;
	DelayedExecutionTimer m_dlyUpdateCompleter;		// Activation delay for completer update to avoid extra updates
};

// ============================================================================

#include "ui_DictionaryWidget.h"

class CDictionaryWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit CDictionaryWidget(CDictionaryDatabasePtr pDictionary, QWidget *parent = 0);
	~CDictionaryWidget();

	CDictionaryDatabasePtr dictionaryDatabase() const { return m_pDictionaryDatabase; }

	inline QMenu *getEditMenu(bool bWordEditor) { return (bWordEditor ? m_pEditMenuDictWord : m_pEditMenuDictionary); }

	int dictionaryActivationDelay() const { return m_dlyTextChanged.minimumDelay(); }

	virtual bool eventFilter(QObject *pObject, QEvent *pEvent);

signals:
	void activatedDictionary(bool bWordEditor);
	void gotoPassageReference(const QString &strPassageReference);

public slots:
	void setWord(const QString &strWord, bool bIsTracking = true);		// Tracking defaults to true so that a connection to wordUnderCursorChanged() signal can trigger a tracking change

	void setFont(const QFont& aFont);
	void setTextBrightness(bool bInvert, int nBrightness);

	void setDictionaryActivationDelay(int nDelay) { m_dlyTextChanged.setMinimumDelay(nDelay); }

protected slots:
	void en_wordChanged();
	void en_sourceChanged(const QUrl &src);
	void en_anchorClicked(const QUrl &link);

	void en_definitionBrowserContextMenuRequested(const QPoint &pos);
	void en_editDictionaryWordContextMenuRequested(const QPoint &pos);

	void en_updateDictionaryDatabasesList();
	void en_selectDictionary(QAction *pAction = NULL);

// Data Private:
private:
	CDictionaryDatabasePtr m_pDictionaryDatabase;

// UI Private:
private:
	QString m_strLanguage;			// Language to use for dictionary lists.  If empty, all languages are used
	bool m_bDoingPopup;				// True if popping up a menu or dialog (useful for things like not disabling highlight, etc)
	QMenu *m_pEditMenuDictionary;	// Edit menu for main screen when the dictionary is active
	QMenu *m_pEditMenuDictWord;		// Edit menu for main screen when the dictionary word editor is active
	// ----
	QAction *m_pActionDictDatabasesList;	// Action for Loaded Dictionary Databases list from which the user can open/select
	QPointer<QActionGroup> m_pActionGroupDictDatabasesList;	// Actual Dictionary Databases List items for the selection list
	// ----
	bool m_bDoingUpdate;
	DelayedExecutionTimer m_dlyTextChanged;
	bool m_bIgnoreNextWordChange;	// Used to ignore the automatic word update when navigating to a passage via an embedded link, as that is annoying
	QString m_strLastWordDictionaryUUID;	// UUID of the dictionary used to display the last word whose definition is displayed.  Used to determine if an update is needed on database change
	bool m_bHaveURLLastWord;		// Set to true if we have a valid word URL for our last word -- cleared when we encounter an unfound word.  This is needed so that entering an invalid word and then entering the same previous valid word will actually navigate to it.
	Ui::CDictionaryWidget ui;
};

// ============================================================================

#endif // DICTIONARY_WIDGET_H
