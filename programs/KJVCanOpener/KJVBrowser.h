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

#ifndef KJVBROWSER_H
#define KJVBROWSER_H

#include "dbstruct.h"
#include "Highlighter.h"
#include "PhraseEdit.h"
#include "ScriptureEdit.h"
#include "DelayedExecutionTimer.h"

#include <QWidget>
#include <QTextBrowser>
#include <QColor>
#include <QTimer>
#include <QUrl>
#include <QMenu>

// ============================================================================

class CVerseListModel;			// Forward declaration

// ============================================================================

namespace Ui {
class CKJVBrowser;
}

class CKJVBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVBrowser(CVerseListModel *pModel, CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);
	~CKJVBrowser();

	inline void savePersistentSettings(const QString &strGroup) { m_pScriptureBrowser->savePersistentSettings(strGroup); }
	inline void restorePersistentSettings(const QString &strGroup) { m_pScriptureBrowser->restorePersistentSettings(strGroup); }

	bool hasFocusBrowser() const;

	inline QMenu *getEditMenu() { return m_pScriptureBrowser->getEditMenu(); }

	inline bool haveSelection() const { return m_pScriptureBrowser->haveSelection(); }
	inline TPhraseTag selection() const { return m_pScriptureBrowser->selection(); }

	inline bool haveDetails() const { return m_pScriptureBrowser->haveDetails(); }

	inline bool isBackwardAvailable() const { return m_pScriptureBrowser->isBackwardAvailable(); }
	inline bool isForwardAvailable() const { return m_pScriptureBrowser->isForwardAvailable(); }

	inline int forwardHistoryCount() const { return m_pScriptureBrowser->forwardHistoryCount(); }
	int backwardHistoryCount() const { return m_pScriptureBrowser->backwardHistoryCount(); }
	void clearHistory() { return m_pScriptureBrowser->clearHistory(); }
	inline QString historyTitle(int i) const { return m_pScriptureBrowser->historyTitle(i); }
	inline QUrl historyUrl(int i) const { return m_pScriptureBrowser->historyUrl(i); }

public slots:
	void setFontScriptureBrowser(const QFont& aFont);
	void setTextBrightness(bool bInvert, int nBrightness);
	void showDetails();
	void showPassageNavigator();

	void gotoIndex(const TPhraseTag &tag);
	void setFocusBrowser();
	void en_SearchResultsVerseListAboutToChange();
	void en_SearchResultsVerseListChanged();

	// Navigation Shortcut Processing:
	void en_Bible_Beginning();
	void en_Bible_Ending();
	void en_Book_Backward();
	void en_Book_Forward();
	void en_ChapterBackward();
	void en_ChapterForward();

signals:
	void en_gotoIndex(const TPhraseTag &tag);

signals:			// Outgoing Pass-Through:
	void activatedScriptureText();
	void backwardAvailable(bool available);
	void forwardAvailable(bool available);
	void historyChanged();

signals:			// Incoming Pass-Through:
	void backward();
	void forward();
	void home();
	void reload();

private:
	void initialize();

private slots:
	void en_sourceChanged(const QUrl &src);

	void BkComboIndexChanged(int index);
	void BkChpComboIndexChanged(int index);
	void TstBkComboIndexChanged(int index);
	void TstChpComboIndexChanged(int index);
	void BibleBkComboIndexChanged(int index);
	void BibleChpComboIndexChanged(int index);

	void delayBkComboIndexChanged(int index);
	void delayBkChpComboIndexChanged(int index);
	void delayTstBkComboIndexChanged(int index);
	void delayTstChpComboIndexChanged(int index);
	void delayBibleBkComboIndexChanged(int index);
	void delayBibleChpComboIndexChanged(int index);

private:
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
	CSearchResultHighlighter m_Highlighter;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()					\
	CBusyCursor iAmBusy(NULL);			\
	bool bUpdateSave = m_bDoingUpdate;	\
	m_bDoingUpdate = true;
#define end_update()					\
	m_bDoingUpdate = bUpdateSave;

	DelayedExecutionTimer m_dlyBkCombo;
	DelayedExecutionTimer m_dlyBkChpCombo;
	DelayedExecutionTimer m_dlyTstBkCombo;
	DelayedExecutionTimer m_dlyTstChpCombo;
	DelayedExecutionTimer m_dlyBibleBkCombo;
	DelayedExecutionTimer m_dlyBibleChpCombo;
	CScriptureBrowser *m_pScriptureBrowser;
	Ui::CKJVBrowser *ui;
};

#endif // KJVBROWSER_H
