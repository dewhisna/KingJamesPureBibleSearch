#ifndef KJVBROWSER_H
#define KJVBROWSER_H

#include <QWidget>
#include <QTextBrowser>

#include "dbstruct.h"
#include "KJVSearchPhraseEdit.h"


class CScriptureBrowser : public QTextBrowser
{
	Q_OBJECT

public:
	explicit CScriptureBrowser(QWidget *parent = 0);
	virtual ~CScriptureBrowser();

protected:
	virtual bool event(QEvent *e);

private:
	CRelIndex ResolveCursorReference(CPhraseCursor &cursor);		// Bounds limited for words
	CRelIndex ResolveCursorReference2(CPhraseCursor &cursor);		// This helper loop finds the reference, but will extend one word off the end of the verse when cursor is between verses

};

// ============================================================================

namespace Ui {
class CKJVBrowser;
}

class CKJVBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVBrowser(QWidget *parent = 0);
	virtual ~CKJVBrowser();

	void Initialize(CRelIndex nInitialIndex = CRelIndex(1,1,0,0));		// Default initial location is Genesis 1

public slots:
	void gotoIndex(CRelIndex ndx);

signals:
	void IndexChanged(const CRelIndex &index);

private slots:
	void BkComboIndexChanged(int index);
	void BkChpComboIndexChanged(int index);
	void TstBkComboIndexChanged(int index);
	void TstChpComboIndexChanged(int index);
	void BibleBkComboIndexChanged(int index);
	void BibleChpComboIndexChanged(int index);

private:
	// These should be used in order:
	void setBook(uint32_t nBk);				// Updates BkChp list, sets lblTestament, updates TstBk and TstChp lists
	void setChapter(uint32_t nChp);			// Fills in the main browser text for the desired chapter
	void setVerse(uint32_t nVrs);			// Scrolls browser to the specified verse for the curret Bk/Tst/Chp, etc.


// Data Private:
private:
	CRelIndex m_ndxCurrent;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;


	Ui::CKJVBrowser *ui;
};

#endif // KJVBROWSER_H
