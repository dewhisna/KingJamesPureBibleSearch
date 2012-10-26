#ifndef KJVBROWSER_H
#define KJVBROWSER_H

#include <QWidget>

#include "dbstruct.h"

namespace Ui {
class CKJVBrowser;
}

class CKJVBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVBrowser(QWidget *parent = 0);
	~CKJVBrowser();

	void Initialize(CRelIndex nInitialIndex = CRelIndex(1,1,1,1));	// Default initial location is the first word of Genesis 1:1)

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
