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

#ifndef BROWSER_WIDGET_H
#define BROWSER_WIDGET_H

#include "dbstruct.h"
#include "Highlighter.h"
#include "ScriptureEdit.h"
#include "DelayedExecutionTimer.h"
#include "PersistentSettings.h"

#include <QWidget>
#include <QTextBrowser>
#include <QColor>
#include <QTimer>
#include <QUrl>
#include <QMenu>
#include <QPoint>

// For Qt4, we will use Plastique Style.  For Qt5+, we will use Fusion Style.
#if QT_VERSION < 0x050000
#include <QPlastiqueStyle>
#else
class QStyle;
#endif

// ============================================================================

class CVerseListModel;			// Forward declaration

// ============================================================================

#include "ui_BrowserWidget.h"

class CBrowserWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CBrowserWidget(CVerseListModel *pSearchResultsListModel, CBibleDatabasePtr pBibleDatabase, QWidget *parent = nullptr);
	virtual ~CBrowserWidget();

	inline void savePersistentSettings(const QString &strGroup)
	{
		m_pScriptureBrowser->savePersistentSettings(strGroup);
#ifdef USING_LITEHTML
		m_pScriptureLiteHtml->savePersistentSettings(strGroup + "/Lemmas");
#endif
	}
	inline void restorePersistentSettings(const QString &strGroup)
	{
		m_pScriptureBrowser->restorePersistentSettings(strGroup);
#ifdef USING_LITEHTML
		m_pScriptureLiteHtml->restorePersistentSettings(strGroup + "/Lemmas");
#endif
	}

	bool hasFocusBrowser() const;
	bool hasFocusPassageReferenceEditor() const;

	inline QMenu *getEditMenu(bool bPassageReferenceEditor) { return (bPassageReferenceEditor ? ui.widgetPassageReference->getEditMenu() : m_pCurrentScriptureTextBase->getEditMenu()); }

	inline bool haveSelection() const { return m_pCurrentScriptureTextBase->haveSelection(); }
	inline CSelectionPhraseTagList selection() const { return m_pCurrentScriptureTextBase->selection(); }

	inline bool haveDetails() const { return m_pCurrentScriptureTextBase->haveDetails(); }
	inline bool haveGematria() const { return m_pCurrentScriptureTextBase->haveGematria(); }

// TODO : Figure out how to special case these between CScriptureBrowser and CScriptureLiteHtml:

	inline bool isBackwardAvailable() const { return m_pScriptureBrowser->isBackwardAvailable(); }
	inline bool isForwardAvailable() const { return m_pScriptureBrowser->isForwardAvailable(); }

	inline int forwardHistoryCount() const { return m_pScriptureBrowser->forwardHistoryCount(); }
	int backwardHistoryCount() const { return m_pScriptureBrowser->backwardHistoryCount(); }
	void clearHistory() { return m_pScriptureBrowser->clearHistory(); }
	inline QString historyTitle(int i) const { return m_pScriptureBrowser->historyTitle(i); }
	inline QUrl historyUrl(int i) const { return m_pScriptureBrowser->historyUrl(i); }

	virtual bool eventFilter(QObject *obj, QEvent *ev) override;

	BROWSER_DISPLAY_MODE_ENUM browserDisplayMode() const { return m_nBrowserDisplayMode; }

public slots:
	void showDetails();
	void showGematria();
	void showPassageNavigator();

	void setBrowserDisplayMode(BROWSER_DISPLAY_MODE_ENUM nBrowserDisplayMode);

	void setNavigationActivationDelay(int nDelay);
	void setPassageReferenceActivationDelay(int nDelay);

	void gotoPassageReference(const QString &strPassageReference);
	void gotoIndex(const TPhraseTag &tag);
	void setFocusBrowser();
	void setFocusPassageReferenceEditor();
	void en_SearchResultsVerseListAboutToChange();
	void en_SearchResultsVerseListChanged();
	void en_highlighterTagsAboutToChange(const CBibleDatabase *pBibleDatabase, const QString &strUserDefinedHighlighterName);
	void en_highlighterTagsChanged(const CBibleDatabase *pBibleDatabase, const QString &strUserDefinedHighlighterName);
	void en_highlightersAboutToChange();
	void en_highlightersChanged();
	void en_userNoteEvent(BIBLE_VERSIFICATION_TYPE_ENUM nVersification, const CRelIndex &ndx);		// Triggered on User Note Added/Changed/Remove
	void en_allUserNotesChanged();						// Triggered if entire UserNotes changed
	void en_crossRefsEvent(BIBLE_VERSIFICATION_TYPE_ENUM nVersification, const CRelIndex &ndxFirst, const CRelIndex &ndxSecond);		// Triggered on Cross-Ref Added/Remove
	void en_allCrossRefsChanged();						// Triggered if entire Cross-Refs changed

	// Navigation Shortcut Processing:
	void en_Bible_Beginning();
	void en_Bible_Ending();
	void en_Book_Backward();
	void en_Book_Forward();
	void en_ChapterBackward();
	void en_ChapterForward();

signals:
	void en_gotoIndex(const TPhraseTag &tag);
	void activatedBrowser(bool bPassageReferenceEditor);		// bPassageReferenceEditor = true if the passage reference activated vs. the scriptureText (for menu selection)

	void wordUnderCursorChanged(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &tag);

signals:			// Outgoing Pass-Through:
	void backwardAvailable(bool available);
	void forwardAvailable(bool available);
	void historyChanged();

signals:			// Incoming Pass-Through:
	void backward();
	void forward();
	void home();
	void reload();
	void rerender();

private:
	void initialize();

private slots:
	void en_changedScrollbarsEnabled(bool bEnabled);
	void en_changedChapterScrollbarMode();

	void en_clickedHideNavigationPane();
	void setBrowserNavigationPaneMode(BROWSER_NAVIGATION_PANE_MODE_ENUM nBrowserNavigationPaneMode);

	void en_clickedSetBrowserDisplayMode();

	void en_selectionChanged();

	void en_sourceChanged(const QUrl &src);

	void BkComboIndexChanged(int index);
	void BkChpComboIndexChanged(int index);
	void TstBkComboIndexChanged(int index);
	void TstChpComboIndexChanged(int index);
	void BibleBkComboIndexChanged(int index);
	void BibleChpComboIndexChanged(int index);

	void PassageReferenceChanged(const TPhraseTag &tag);
	void PassageReferenceEnterPressed();

	void en_activatedPassageReference();
	void en_activatedScriptureText();

	void ChapterSliderMoved(int index);
	void ChapterSliderValueChanged();

	void delayBkComboIndexChanged(int index);
	void delayBkChpComboIndexChanged(int index);
	void delayTstBkComboIndexChanged(int index);
	void delayTstChpComboIndexChanged(int index);
	void delayBibleBkComboIndexChanged(int index);
	void delayBibleChpComboIndexChanged(int index);

	void delayPassageReference(const TPhraseTag &tag);

	void en_WordsOfJesusColorChanged(const QColor &color);
	void en_SearchResultsColorChanged(const QColor &color);
	void en_ShowExcludedSearchResultsChanged(bool bShowExcludedSearchResults);

	void en_beginChangeBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &oldSettings,
												const TBibleDatabaseSettings &newSettings, bool bForce);

private:
	void setupChapterScrollbar();

	void gotoIndex2(const TPhraseTag &tag);
	void doHighlighting(bool bClear = false);		// Highlight the areas marked in the PhraseTags.  If bClear=True, removes the highlighting, which is used to swapout the current tag list for a new one without redrawing everything

	// These should be used in order:
	void setBook(const CRelIndex &ndx);		// Updates BkChp list, sets lblTestament, updates TstBk and TstChp lists
	void setChapter(const CRelIndex &ndx);	// Fills in the main browser text for the desired chapter
	void setVerse(const CRelIndex &ndx);	// Scrolls browser to the specified verse for the current Bk/Tst/Chp, etc.
	void setWord(const TPhraseTag &tag);	// Scrolls browser to the specified word for the current Bk/Tst/Chp/Vrs, etc.  And selects the number of words specified

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	CRelIndex m_ndxCurrent;
	CVerseListModel *m_pSearchResultsListModel;
	CSearchResultHighlighter m_SearchResultsHighlighter;
	CSearchResultHighlighter m_ExcludedSearchResultsHighlighter;
	bool m_bShowExcludedSearchResults;

// UI Private:
private:
	bool m_bDoingUpdate;					// True if combo boxes, etc, are being updated and change notifications should be ignored
	QPoint m_ptChapterScrollerMousePos;		// Last mouse position tracked for chapter scroller for rolling tooltips
#if QT_VERSION < 0x050000
	QPlastiqueStyle m_ChapterScrollerStyle;	// Used to define specific style for our chapter scroller so that it will have extra scroller buttons, etc, even on the limited Mac
#else
	QStyle *m_pChapterScrollerStyle;
#endif

#define begin_update()					\
	CBusyCursor iAmBusy(nullptr);		\
	bool bUpdateSave = m_bDoingUpdate;	\
	m_bDoingUpdate = true;
#define end_update()					\
	m_bDoingUpdate = bUpdateSave;

	BROWSER_DISPLAY_MODE_ENUM m_nBrowserDisplayMode;

	bool m_bDoingPassageReference;
	DelayedExecutionTimer m_dlyBkCombo;
	DelayedExecutionTimer m_dlyBkChpCombo;
	DelayedExecutionTimer m_dlyTstBkCombo;
	DelayedExecutionTimer m_dlyTstChpCombo;
	DelayedExecutionTimer m_dlyBibleBkCombo;
	DelayedExecutionTimer m_dlyBibleChpCombo;
	DelayedExecutionTimer m_dlyPassageReference;
	DelayedExecutionTimer m_dlyGotoIndex;
	CScriptureBrowser *m_pScriptureBrowser;
#ifdef USING_LITEHTML
	CScriptureLiteHtml *m_pScriptureLiteHtml;
#endif
	CScriptureTextBase *m_pCurrentScriptureTextBase;	// Pointer to current browser type (CScriptureBrowser or CScriptureLiteHtml)
	Ui::CBrowserWidget ui;
};

#endif // BROWSER_WIDGET_H
