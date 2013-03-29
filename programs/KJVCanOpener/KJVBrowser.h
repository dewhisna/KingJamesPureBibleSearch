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

#include <QWidget>
#include <QTextBrowser>
#include <QColor>
#include <QTimer>
#include <QUrl>
#include <QMenu>

// ============================================================================

namespace Ui {
class CKJVBrowser;
}

class CKJVBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVBrowser(CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);
	~CKJVBrowser();

	inline void savePersistentSettings(const QString &strGroup) { browser()->savePersistentSettings(strGroup); }
	inline void restorePersistentSettings(const QString &strGroup) { browser()->restorePersistentSettings(strGroup); }

	bool hasFocusBrowser() const;

	inline QMenu *getEditMenu() { return browser()->getEditMenu(); }

	inline bool haveSelection() const { return browser()->haveSelection(); }
	inline TPhraseTag selection() const { return browser()->selection(); }

	inline bool haveDetails() const { return browser()->haveDetails(); }

	inline bool isBackwardAvailable() const { return browser()->isBackwardAvailable(); }
	inline bool isForwardAvailable() const { return browser()->isForwardAvailable(); }

	inline int forwardHistoryCount() const { return browser()->forwardHistoryCount(); }
	int backwardHistoryCount() const { return browser()->backwardHistoryCount(); }
	void clearHistory() { return browser()->clearHistory(); }
	inline QString historyTitle(int i) const { return browser()->historyTitle(i); }
	inline QUrl historyUrl(int i) const { return browser()->historyUrl(i); }

public slots:
	void setFontScriptureBrowser(const QFont& aFont);
	void showDetails();
	void showPassageNavigator();

	void gotoIndex(const TPhraseTag &tag);
	void setFocusBrowser();
	void setHighlightTags(const TPhraseTagList &lstPhraseTags);

	// Navigation Shortcut Processing:
	void on_Bible_Beginning();
	void on_Bible_Ending();
	void on_Book_Backward();
	void on_Book_Forward();
	void on_ChapterBackward();
	void on_ChapterForward();

signals:
	void on_gotoIndex(const TPhraseTag &tag);

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

protected:
	void initialize();

private slots:
	void on_sourceChanged(const QUrl &src);

	void BkComboIndexChanged(int index);
	void BkChpComboIndexChanged(int index);
	void TstBkComboIndexChanged(int index);
	void TstChpComboIndexChanged(int index);
	void BibleBkComboIndexChanged(int index);
	void BibleChpComboIndexChanged(int index);

private:
	void gotoIndex2(const TPhraseTag &tag);
	void doHighlighting(bool bClear = false);		// Highlight the areas marked in the PhraseTags.  If bClear=True, removes the highlighting, which is used to swapout the current tag list for a new one without redrawing everything

	// These should be used in order:
	void setBook(const CRelIndex &ndx);		// Updates BkChp list, sets lblTestament, updates TstBk and TstChp lists
	void setChapter(const CRelIndex &ndx);	// Fills in the main browser text for the desired chapter
	void setVerse(const CRelIndex &ndx);	// Scrolls browser to the specified verse for the current Bk/Tst/Chp, etc.
	void setWord(const TPhraseTag &tag);	// Scrolls browser to the specified word for the current Bk/Tst/Chp/Vrs, etc.  And selects the number of words specified

protected:
	CScriptureBrowser *browser() const;

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	CRelIndex m_ndxCurrent;
	CSearchResultHighlighter m_Highlighter;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()					\
	bool bUpdateSave = m_bDoingUpdate;	\
	m_bDoingUpdate = true;
#define end_update()					\
	m_bDoingUpdate = bUpdateSave;


	Ui::CKJVBrowser *ui;
};

#endif // KJVBROWSER_H
