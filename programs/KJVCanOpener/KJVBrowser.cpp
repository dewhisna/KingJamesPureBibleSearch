#include "KJVBrowser.h"
#include "ui_KJVBrowser.h"

#include "dbstruct.h"

#include "KJVSearchPhraseEdit.h"

#include <assert.h>

#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QToolTip>

// ============================================================================

CScriptureBrowser::CScriptureBrowser(QWidget *parent)
	:	QTextBrowser(parent)
{

}

CScriptureBrowser::~CScriptureBrowser()
{

}

bool CScriptureBrowser::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip) {
		QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(e);
		CPhraseCursor cursor = cursorForPosition(pHelpEvent->pos());

		QTextCharFormat fmt = cursor.charFormat();
		if (!fmt.anchorName().isEmpty()) {
			CRelIndex ndxReference(fmt.anchorName());
			QString strToolTip = ndxReference.SearchResultToolTip();
			if (ndxReference.book() != 0) {
				assert(ndxReference.book() <= g_lstTOC.size());
				if (ndxReference.book() <= g_lstTOC.size()) {
					strToolTip += "\n----------\n";
					strToolTip += QString("\n%1 contains:\n"
											"    %2 Chapters\n"
											"    %3 Verses\n"
											"    %4 Words\n")
											.arg(ndxReference.bookName())
											.arg(g_lstTOC[ndxReference.book()-1].m_nNumChp)
											.arg(g_lstTOC[ndxReference.book()-1].m_nNumVrs)
											.arg(g_lstTOC[ndxReference.book()-1].m_nNumWrd);
					if (ndxReference.chapter() != 0) {
						assert(ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp);
						if (ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp) {
							strToolTip += QString("\n%1 %2 contains:\n"
													"    %3 Verses\n"
													"    %4 Words\n")
													.arg(ndxReference.bookName()).arg(ndxReference.chapter())
													.arg(g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs)
													.arg(g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumWrd);
							if (ndxReference.verse() != 0) {
								assert(ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs);
								if (ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs) {
									strToolTip += QString("\n%1 %2:%3 contains:\n"
															"    %4 Words\n")
															.arg(ndxReference.bookName()).arg(ndxReference.chapter()).arg(ndxReference.verse())
															.arg((g_lstBooks[ndxReference.book()-1])[CRelIndex(0, ndxReference.chapter(), ndxReference.verse(), 0)].m_nNumWrd);
								}
							}
						}
					}
				}
			}
			QToolTip::showText(pHelpEvent->globalPos(), strToolTip);
		} else {
			QToolTip::hideText();
		}
		return true;
	}

	return QTextBrowser::event(e);
}

// ============================================================================

CKJVBrowser::CKJVBrowser(QWidget *parent) :
	QWidget(parent),
	m_ndxCurrent(0),
	m_bDoingUpdate(false),
	ui(new Ui::CKJVBrowser)
{
	ui->setupUi(this);

	ui->textBrowserMainText->setStyleSheet("font: 12pt \"Times New Roman\";");

	Initialize();

// UI Connections:
	connect(ui->comboBk, SIGNAL(currentIndexChanged(int)), this, SLOT(BkComboIndexChanged(int)));
	connect(ui->comboBkChp, SIGNAL(currentIndexChanged(int)), this, SLOT(BkChpComboIndexChanged(int)));
	connect(ui->comboTstBk, SIGNAL(currentIndexChanged(int)), this, SLOT(TstBkComboIndexChanged(int)));
	connect(ui->comboTstChp, SIGNAL(currentIndexChanged(int)), this, SLOT(TstChpComboIndexChanged(int)));
	connect(ui->comboBibleBk, SIGNAL(currentIndexChanged(int)), this, SLOT(BibleBkComboIndexChanged(int)));
	connect(ui->comboBibleChp, SIGNAL(currentIndexChanged(int)), this, SLOT(BibleChpComboIndexChanged(int)));
}

CKJVBrowser::~CKJVBrowser()
{
	delete ui;
}

void CKJVBrowser::Initialize(CRelIndex nInitialIndex)
{
	begin_update();

	unsigned int nBibleChp = 0;
	ui->comboBk->clear();
	ui->comboBibleBk->clear();
	for (unsigned int ndxBk=0; ndxBk<g_lstTOC.size(); ++ndxBk) {
		ui->comboBk->addItem(g_lstTOC[ndxBk].m_strBkName, ndxBk+1);
		ui->comboBibleBk->addItem(QString("%1").arg(ndxBk+1), ndxBk+1);
		nBibleChp += g_lstTOC[ndxBk].m_nNumChp;
	}
	ui->comboBibleChp->clear();
	for (unsigned int ndxBibleChp=0; ndxBibleChp<nBibleChp; ++ ndxBibleChp) {
		ui->comboBibleChp->addItem(QString("%1").arg(ndxBibleChp+1), ndxBibleChp+1);
	}

	end_update();

	if (nInitialIndex.isSet()) gotoIndex(nInitialIndex);
}

void CKJVBrowser::gotoIndex(CRelIndex ndx)
{
	setBook(ndx.book());
	setChapter(ndx.chapter());
	setVerse(ndx.verse());

	emit IndexChanged(ndx);
}

void CKJVBrowser::setBook(uint32_t nBk)
{
	if (nBk == 0) return;

	begin_update();

	m_ndxCurrent.setIndex(nBk, 0, 0, 0);

	if (m_ndxCurrent.book() > g_lstTOC.size()) {
		assert(false);
		end_update();
		return;
	}

	const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];

	ui->comboBk->setCurrentIndex(ui->comboBk->findData(m_ndxCurrent.book()));
	ui->comboBibleBk->setCurrentIndex(ui->comboBibleBk->findData(m_ndxCurrent.book()));

	unsigned int nTst = toc.m_nTstNdx;
	ui->lblTestament->setText(g_lstTestaments[nTst-1].m_strTstName + ":");
	ui->comboTstBk->clear();
	for (unsigned int ndxTstBk=0; ndxTstBk<g_lstTestaments[nTst-1].m_nNumBk; ++ndxTstBk) {
		ui->comboTstBk->addItem(QString("%1").arg(ndxTstBk+1), ndxTstBk+1);
	}
	ui->comboTstBk->setCurrentIndex(ui->comboTstBk->findData(toc.m_nTstBkNdx));

	ui->comboBkChp->clear();
	for (unsigned int ndxBkChp=0; ndxBkChp<toc.m_nNumChp; ++ndxBkChp) {
		ui->comboBkChp->addItem(QString("%1").arg(ndxBkChp+1), ndxBkChp+1);
	}
	ui->comboTstChp->clear();
	for (unsigned int ndxTstChp=0; ndxTstChp<g_lstTestaments[nTst-1].m_nNumChp; ++ndxTstChp) {
		ui->comboTstChp->addItem(QString("%1").arg(ndxTstChp+1), ndxTstChp+1);
	}

	end_update();
}

void CKJVBrowser::setChapter(uint32_t nChp)
{
	begin_update();

	m_ndxCurrent.setIndex(m_ndxCurrent.book(), nChp, 0, 0);

	ui->comboBkChp->setCurrentIndex(ui->comboBkChp->findData(nChp));

	ui->textBrowserMainText->clear();

	if ((m_ndxCurrent.book() == 0) || (m_ndxCurrent.chapter() == 0)) {
		end_update();
		return;
	}

	if (m_ndxCurrent.book() > g_lstTOC.size()) {
		assert(false);
		end_update();
		return;
	}

	const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
	const TBookEntryMap &book = g_lstBooks[m_ndxCurrent.book()-1];

	unsigned int nTstChp = 0;
	unsigned int nBibleChp = 0;
	for (unsigned int ndxBk=0; ndxBk<(m_ndxCurrent.book()-1); ++ndxBk) {
		if (g_lstTOC[ndxBk].m_nTstNdx == toc.m_nTstNdx)
			nTstChp += g_lstTOC[ndxBk].m_nNumChp;
		nBibleChp += g_lstTOC[ndxBk].m_nNumChp;
	}
	nTstChp += m_ndxCurrent.chapter();
	nBibleChp += m_ndxCurrent.chapter();

	ui->comboTstChp->setCurrentIndex(ui->comboTstChp->findData(nTstChp));
	ui->comboBibleChp->setCurrentIndex(ui->comboBibleChp->findData(nBibleChp));

	end_update();


	TLayoutMap::const_iterator mapLookupLayout = g_mapLayout.find(CRelIndex(m_ndxCurrent.book(),m_ndxCurrent.chapter(),0,0));
	if (mapLookupLayout == g_mapLayout.end()) {
		assert(false);
		return;
	}
	const CLayoutEntry &layout(mapLookupLayout->second);

	if (m_ndxCurrent.chapter() > toc.m_nNumChp) {
		assert(false);
		return;
	}

//	QString strHTML = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n<br/>";
	QString strHTML = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; }\n</style></head><body style=\" font-family:'Times New Roman'; font-size:12pt; font-weight:400; font-style:normal;\">\n";
//	QString strHTML = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><style type=\"text/css\"><!-- A { text-decoration:none } %s --></style></head><body><br/>";

	uint32_t nFirstWordNormal = NormalizeIndex(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1));		// Find normalized word number for the first verse, first word of this book/chapter
	uint32_t nNextChapterFirstWordNormal = nFirstWordNormal + layout.m_nNumWrd;		// Add the number of words in this chapter to get first word normal of next chapter
	uint32_t nRelPrevChapter = DenormalizeIndex(nFirstWordNormal - 1);				// Find previous book/chapter/verse (and word)
	uint32_t nRelNextChapter = DenormalizeIndex(nNextChapterFirstWordNormal);		// Find next book/chapter/verse (and word)

	// Print last verse of previous chapter if available:
	if (nRelPrevChapter != 0) {
		CRelIndex relPrev(nRelPrevChapter);
		strHTML += "<p>";
		strHTML += QString("<a id=\"%1\"><bold> %2 </bold></a>").arg(relPrev.asAnchor()).arg(relPrev.verse());
		strHTML += (g_lstBooks[relPrev.book()-1])[CRelIndex(0,relPrev.chapter(),relPrev.verse(),0)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<hr/>\n";

	// Print Heading for this Book/Chapter:
	strHTML += QString("<h1><a id=\"%1\">%2</a></h1>\n").arg(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0).asAnchor()).arg(toc.m_strBkName);
	strHTML += QString("<h2><a id=\"%1\">Chapter %2</a></h2>\n").arg(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0).asAnchor()).arg(m_ndxCurrent.chapter());

	// Print this Chapter Text:
	bool bParagraph = false;
	for (unsigned int ndxVrs=0; ndxVrs<layout.m_nNumVrs; ++ndxVrs) {
		TBookEntryMap::const_iterator mapLookupVerse = book.find(CRelIndex(0,m_ndxCurrent.chapter(),ndxVrs+1,0));
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
		strHTML += QString("<a id=\"%1\"><bold> %2 </bold></a>").arg(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndxVrs+1, 0).asAnchor()).arg(ndxVrs+1);
		strHTML += verse.GetRichText() + "\n";
	}
	if (bParagraph) {
		strHTML += "</p>";
		bParagraph = false;
	}

	strHTML += "<hr/>\n";

	// Print first verse of next chapter if available:
	if (nRelNextChapter != 0) {
		CRelIndex relNext(nRelNextChapter);

		// Print Heading for this Book/Chapter:
		if (relNext.book() != m_ndxCurrent.book())
			strHTML += QString("<h1><a id=\"%1\">%2</a></h1>\n").arg(CRelIndex(relNext.book(), relNext.chapter(), 0 ,0).asAnchor()).arg(g_lstTOC[relNext.book()-1].m_strBkName);
		strHTML += QString("<h2><a id=\"%1\">Chapter %2</a></h2>\n").arg(CRelIndex(relNext.book(), relNext.chapter(), 0, 0).asAnchor()).arg(relNext.chapter());

		strHTML += "<p>";
		strHTML += QString("<a id=\"%1\"><bold> %2 </bold></a>").arg(relNext.asAnchor()).arg(relNext.verse());
		strHTML += (g_lstBooks[relNext.book()-1])[CRelIndex(0,relNext.chapter(),relNext.verse(),0)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<br/></body></html>";
	ui->textBrowserMainText->setHtml(strHTML);
}

void CKJVBrowser::setVerse(uint32_t nVrs)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), nVrs, 0);

	CRelIndex ndxScroll = m_ndxCurrent;
	if (ndxScroll.verse() == 1) ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading

	ui->textBrowserMainText->scrollToAnchor(ndxScroll.asAnchor());
}

void CKJVBrowser::BkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	if (index != -1) {
		setBook(ui->comboBk->itemData(index).toUInt());
		setChapter(1);
		setVerse(1);
	} else {
		setBook(0);
		setChapter(0);
		setVerse(0);
	}
}

void CKJVBrowser::BkChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	if (index != -1) {
		setChapter(ui->comboBkChp->itemData(index).toUInt());
		setVerse(1);
	} else {
		setChapter(0);
		setVerse(0);
	}
}

void CKJVBrowser::TstBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get TOC for current book so we know what testament we're currently in:
		const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
		CRelIndex ndxTarget = CRefCountCalc::calcRelIndex(0, 0, 0, ui->comboTstBk->itemData(index).toUInt(), toc.m_nTstNdx);
		ndxTarget.setVerse(1);
		ndxTarget.setWord(1);
		gotoIndex(ndxTarget);
	} else {
		setBook(0);
		setChapter(0);
		setVerse(0);
	}
}

void CKJVBrowser::TstChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get TOC for current book so we know what testament we're currently in:
		const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
		CRelIndex ndxTarget = CRefCountCalc::calcRelIndex(0, 0, ui->comboTstChp->itemData(index).toUInt(), 0, toc.m_nTstNdx);
		ndxTarget.setVerse(1);
		ndxTarget.setWord(1);
		gotoIndex(ndxTarget);
	} else {
		setBook(0);
		setChapter(0);
		setVerse(0);
	}
}

void CKJVBrowser::BibleBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	if (index != -1) {
		setBook(ui->comboBibleBk->itemData(index).toUInt());
		setChapter(1);
		setVerse(1);
	} else {
		setBook(0);
		setChapter(0);
		setVerse(0);
	}
}

void CKJVBrowser::BibleChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	if (index != -1) {
		CRelIndex ndxTarget = CRefCountCalc::calcRelIndex(0, 0, ui->comboBibleChp->itemData(index).toUInt(), 0, 0);
		ndxTarget.setVerse(1);
		ndxTarget.setWord(1);
		gotoIndex(ndxTarget);
	} else {
		setBook(0);
		setChapter(0);
		setVerse(0);
	}
}

