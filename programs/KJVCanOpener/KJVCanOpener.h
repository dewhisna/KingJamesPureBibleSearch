#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include "dbstruct.h"
#include <QMainWindow>

namespace Ui {
class CKJVCanOpener;
}

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(QWidget *parent = 0);
	~CKJVCanOpener();

	void Initialize(uint32_t nInitialIndex = MakeIndex(1,1,1,1));	// Default initial location is the first word of Genesis 1:1)

private:
	void FillBookList(uint32_t nRelIndex = MakeIndex(1,1,1,1));		// Fill book list and goto RelIndex, which by default is the first word of Genesis 1:1

public slots:
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
	uint32_t m_nCurrentBook;
	uint32_t m_nCurrentChapter;
	uint32_t m_nCurrentVerse;
	uint32_t m_nCurrentWord;

// UI Private:
private:
	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
