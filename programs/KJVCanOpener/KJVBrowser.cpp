#include "KJVBrowser.h"
#include "ui_KJVBrowser.h"

#include "dbstruct.h"

#include <assert.h>

#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>


CKJVBrowser::CKJVBrowser(QWidget *parent) :
	QWidget(parent),
	m_nCurrentBook(0),
	m_nCurrentChapter(0),
	m_nCurrentVerse(0),
	m_nCurrentWord(0),
	ui(new Ui::CKJVBrowser)
{
	ui->setupUi(this);


	ui->textBrowserMainText->setStyleSheet("font: 12pt \"Times New Roman\";");


// UI Connections:
	connect(ui->comboBook, SIGNAL(currentIndexChanged(int)), this, SLOT(BookComboIndexChanged(int)));
	connect(ui->comboChapter, SIGNAL(currentIndexChanged(int)), this, SLOT(ChapterComboIndexChanged(int)));

// Text Navigation Connections:
//	connect(this, SIGNAL(BookSelectionChanged(uint32_t)), this, SLOT(setBook(uint32_t)));
//	connect(this, SIGNAL(ChapterSelectionChanged(uint32_t)), this, SLOT(setChapter(uint32_t)));
//	connect(this, SIGNAL(VerseSelectionChanged(uint32_t)), this, SLOT(setVerse(uint32_t)));

}

CKJVBrowser::~CKJVBrowser()
{
	delete ui;
}


void CKJVBrowser::Initialize(uint32_t nInitialIndex)
{
	FillBookList(nInitialIndex);
}

void CKJVBrowser::FillBookList(uint32_t nRelIndex)
{
	TRelIndex ndx = DecomposeIndex(nRelIndex);

	ui->comboBook->clear();

	for (unsigned int ndxBk=0; ndxBk<g_lstTOC.size(); ++ndxBk) {
		ui->comboBook->addItem(g_lstTOC[ndxBk].m_strBkName, ndxBk+1);
	}
	ui->comboBook->setCurrentIndex(ui->comboBook->findData(ndx.m_nN3));
	ui->comboChapter->setCurrentIndex(ui->comboChapter->findData(ndx.m_nN2));
	// TODO : Add verse and word
}

void CKJVBrowser::setBook(uint32_t nBk)
{
	m_nCurrentBook = nBk;
	m_nCurrentChapter = 0;
	m_nCurrentVerse = 0;
	m_nCurrentWord = 0;

	emit BookSelectionChanged(nBk);

	ui->comboChapter->clear();

	if (nBk > 0) {
		for (unsigned int ndxChp=0; ndxChp<g_lstTOC[nBk-1].m_nNumChp; ++ndxChp) {
			ui->comboChapter->addItem(QString("%1").arg(ndxChp+1), ndxChp+1);
		}
	}
//	setChapter(0);
}

void CKJVBrowser::setChapter(uint32_t nChp)
{
	m_nCurrentChapter = nChp;
	m_nCurrentVerse = 0;
	m_nCurrentWord = 0;

	emit ChapterSelectionChanged(nChp);

	ui->textBrowserMainText->clear();

	if ((m_nCurrentBook == 0) || (m_nCurrentChapter == 0)) return;

	if (m_nCurrentBook > g_lstTOC.size()) {
		assert(false);
		return;
	}

	const CTOCEntry &toc = g_lstTOC[m_nCurrentBook-1];
	const TBookEntryMap &book = g_lstBooks[m_nCurrentBook-1];

	TLayoutMap::const_iterator mapLookupLayout = g_mapLayout.find(MakeIndex(0,0,m_nCurrentBook,m_nCurrentChapter));
	if (mapLookupLayout == g_mapLayout.end()) {
		assert(false);
		return;
	}
	const CLayoutEntry &layout(mapLookupLayout->second);

	if (m_nCurrentChapter > toc.m_nNumChp) {
		assert(false);
		return;
	}

//	QString strHTML = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n<br/>";
	QString strHTML = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; }\n</style></head><body style=\" font-family:'Times New Roman'; font-size:12pt; font-weight:400; font-style:normal;\">\n";
//	QString strHTML = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><style type=\"text/css\"><!-- A { text-decoration:none } %s --></style></head><body><br/>";

	uint32_t nFirstWordNormal = NormalizeIndex(MakeIndex(m_nCurrentBook, m_nCurrentChapter, 1, 1));		// Find normalized word number for the first verse, first word of this book/chapter
	uint32_t nNextChapterFirstWordNormal = nFirstWordNormal + layout.m_nNumWrd;		// Add the number of words in this chapter to get first word normal of next chapter
	uint32_t nRelPrevChapter = DenormalizeIndex(nFirstWordNormal - 1);				// Find previous book/chapter/verse (and word)
	uint32_t nRelNextChapter = DenormalizeIndex(nNextChapterFirstWordNormal);		// Find next book/chapter/verse (and word)

	// Print last verse of previous chapter if available:
	if (nRelPrevChapter != 0) {
		TRelIndex relPrev = DecomposeIndex(nRelPrevChapter);
		strHTML += "<p>";
		strHTML += QString("<bold> %1 </bold>").arg(relPrev.m_nN1);
		strHTML += (g_lstBooks[relPrev.m_nN3-1])[MakeIndex(0,0,relPrev.m_nN2,relPrev.m_nN1)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<hr/>\n";

	// Print Heading for this Book/Chapter:
	strHTML += QString("<h1>%1</h1>\n").arg(toc.m_strBkName);
	strHTML += QString("<h2>Chapter %1</h2>\n").arg(m_nCurrentChapter);

	// Print this Chapter Text:
	bool bParagraph = false;
	for (unsigned int ndxVrs=0; ndxVrs<layout.m_nNumVrs; ++ndxVrs) {
		TBookEntryMap::const_iterator mapLookupVerse = book.find(MakeIndex(0,0,m_nCurrentChapter,ndxVrs+1));
		if (mapLookupVerse == book.end()) {
			assert(false);
			continue;
		}
		const CBookEntry &verse(mapLookupVerse->second);
		if (verse.m_bPilcrow) {
			if (bParagraph) {
				strHTML += "</p>";
				bParagraph=false;
			}
//			strHTML += "<br/>\n";
		}
		if (!bParagraph) {
			strHTML += "<p>";
			bParagraph = true;
		}
		strHTML += QString("<bold> %1 </bold>").arg(ndxVrs+1);
		strHTML += verse.GetRichText() + "\n";
	}
	if (bParagraph) {
		strHTML += "</p>";
		bParagraph = false;
	}

	strHTML += "<hr/>\n";

	// Print first verse of next chapter if available:
	if (nRelNextChapter != 0) {
		TRelIndex relNext = DecomposeIndex(nRelNextChapter);

		// Print Heading for this Book/Chapter:
		if (relNext.m_nN3 != m_nCurrentBook)
			strHTML += QString("<h1>%1</h1>\n").arg(g_lstTOC[relNext.m_nN3-1].m_strBkName);
		strHTML += QString("<h2>Chapter %1</h2>\n").arg(relNext.m_nN2);

		strHTML += "<p>";
		strHTML += QString("<bold> %1 </bold>").arg(relNext.m_nN1);
		strHTML += (g_lstBooks[relNext.m_nN3-1])[MakeIndex(0,0,relNext.m_nN2,relNext.m_nN1)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<br/></body></html>";
	ui->textBrowserMainText->setHtml(strHTML);
}

void CKJVBrowser::setVerse(uint32_t nVrs)
{
	m_nCurrentVerse = nVrs;

	emit VerseSelectionChanged(nVrs);

}

void CKJVBrowser::BookComboIndexChanged(int index)
{
	if (ui->comboBook->currentIndex() != -1) {
		setBook(ui->comboBook->itemData(ui->comboBook->currentIndex()).toUInt());
	} else {
		setBook(0);
	}
}

void CKJVBrowser::ChapterComboIndexChanged(int index)
{
	if (ui->comboChapter->currentIndex() != -1) {
		setChapter(ui->comboChapter->itemData(ui->comboChapter->currentIndex()).toUInt());
	} else {
		setChapter(0);
	}
}

