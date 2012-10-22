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

private:
	void FillBookList(CRelIndex nRelIndex = CRelIndex(1,1,1,1));	// Fill book list and goto RelIndex, which by default is the first word of Genesis 1:1

public slots:
	void gotoIndex(CRelIndex ndx);
	void setBook(uint32_t nBk);
	void setChapter(uint32_t nChp);
	void setVerse(uint32_t nVrs);

signals:
	void BookSelectionChanged(uint32_t nBk);
	void ChapterSelectionChanged(uint32_t nChp);
	void VerseSelectionChanged(uint32_t nVrs);

private slots:
	void BookComboIndexChanged(int index);
	void ChapterComboIndexChanged(int index);

// Data Private:
private:
	CRelIndex m_ndxCurrent;

// UI Private:
private:
	Ui::CKJVBrowser *ui;
};

#endif // KJVBROWSER_H
