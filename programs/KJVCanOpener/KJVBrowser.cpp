#include "KJVBrowser.h"
#include "ui_KJVBrowser.h"

#include "dbstruct.h"

#include "KJVSearchPhraseEdit.h"
#include "KJVPassageNavigatorDlg.h"

#include <assert.h>

#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>
#include <QToolTip>

// ============================================================================

CScriptureBrowser::CScriptureBrowser(QWidget *parent)
	:	QTextBrowser(parent)
{

}

CScriptureBrowser::~CScriptureBrowser()
{

}

int CScriptureBrowser::anchorPosition(const QString &strAnchorName) const
{
	if (strAnchorName.isEmpty()) return -1;

	for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
		QTextCharFormat format = block.charFormat();
		if (format.isAnchor()) {
			if (format.anchorNames().contains(strAnchorName)) {
				int nPos = block.position();
				QString strText = block.text();
				for (int nPosStr = 0; nPosStr < strText.length(); ++nPosStr) {
					if (strText[nPosStr].isSpace()) {
						nPos++;
					} else {
						break;
					}
				}
				return nPos;
			}
		}
		for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it) {
			QTextFragment fragment = it.fragment();
			format = fragment.charFormat();
			if (format.isAnchor()) {
				if (format.anchorNames().contains(strAnchorName)) {
					int nPos = fragment.position();
					QString strText = fragment.text();
					for (int nPosStr = 0; nPosStr < strText.length(); ++nPosStr) {
						if (strText[nPosStr].isSpace()) {
							nPos++;
						} else {
							break;
						}
					}
					return nPos;
				}
			}
		}
	}

	return -1;
}

CRelIndex CScriptureBrowser::ResolveCursorReference(CPhraseCursor &cursor)
{
	CRelIndex ndxReference = ResolveCursorReference2(cursor);

	if (ndxReference.book() != 0) {
		assert(ndxReference.book() <= g_lstTOC.size());
		if (ndxReference.book() <= g_lstTOC.size()) {
			if (ndxReference.chapter() != 0) {
				assert(ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp);
				if (ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp) {
					if (ndxReference.verse() != 0) {
						assert(ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs);
						if (ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs) {
							if (ndxReference.word() > (g_lstBooks[ndxReference.book()-1])[CRelIndex(0, ndxReference.chapter(), ndxReference.verse(), 0)].m_nNumWrd) {
								// Clip word index at max since it's possible to be on the space
								//		between words and have an index that is one larger than
								//		our largest word:
								ndxReference.setWord((g_lstBooks[ndxReference.book()-1])[CRelIndex(0, ndxReference.chapter(), ndxReference.verse(), 0)].m_nNumWrd);
							}
						}
					}
				}
			}
		}
	}

	return ndxReference;
}

CRelIndex CScriptureBrowser::ResolveCursorReference2(CPhraseCursor &cursor)
{

#define CheckForAnchor() {											\
	ndxReference = CRelIndex(cursor.charFormat().anchorName());		\
	if (ndxReference.isSet()) {										\
		if ((ndxReference.verse() != 0) &&							\
			(ndxReference.word() == 0)) {							\
			ndxReference.setWord(nWord);							\
		}															\
		return ndxReference;										\
	}																\
}


	CRelIndex ndxReference;
	unsigned int nWord = 0;

	CheckForAnchor();
	while (!cursor.charUnderCursor().isSpace()) {
		if (!cursor.moveCursorCharLeft(QTextCursor::MoveAnchor)) return ndxReference;
		CheckForAnchor();
	}

	do {
		nWord++;

		while (cursor.charUnderCursor().isSpace()) {
			if (!cursor.moveCursorCharLeft(QTextCursor::MoveAnchor)) return ndxReference;
			CheckForAnchor();
		}

		while (!cursor.charUnderCursor().isSpace()) {
			if (!cursor.moveCursorCharLeft(QTextCursor::MoveAnchor)) return ndxReference;
			CheckForAnchor();
		}
	} while (1);

	return ndxReference;
}

bool CScriptureBrowser::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip) {
		QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(e);
		CPhraseCursor cursor = cursorForPosition(pHelpEvent->pos());
		CRelIndex ndxReference = ResolveCursorReference(cursor);
		QString strToolTip;

		if (ndxReference.isSet()) {
			if (ndxReference.word() != 0) {
				uint32_t ndxNormal = NormalizeIndex(ndxReference);
				if ((ndxNormal != 0) && (ndxNormal <= g_lstConcordanceMapping.size())) {
					strToolTip += "Word: " + g_lstConcordanceWords.at(g_lstConcordanceMapping.at(ndxNormal)-1) + "\n";
				}
			}
			strToolTip += ndxReference.SearchResultToolTip();
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
		}

		if (!strToolTip.isEmpty()) {
			QToolTip::showText(pHelpEvent->globalPos(), strToolTip);
		} else {
			QToolTip::hideText();
		}
		return true;
	}

	return QTextBrowser::event(e);
}

void CScriptureBrowser::mouseDoubleClickEvent(QMouseEvent * e)
{
	CPhraseCursor cursor = cursorForPosition(e->pos());
	CRelIndex ndxReference = ResolveCursorReference(cursor);
	if (ndxReference.isSet()) {
		CKJVPassageNavigatorDlg dlg(parentWidget());
		dlg.navigator().startRelativeMode(ndxReference, false);
		if (dlg.exec() == QDialog::Accepted) {
			emit gotoIndex(dlg.passage(), 1);
		}
	}
}

// ============================================================================

CKJVBrowser::CKJVBrowser(QWidget *parent) :
	QWidget(parent),
	m_ndxCurrent(0),
	m_colorHighlight("blue"),
	m_bDoingUpdate(false),
	ui(new Ui::CKJVBrowser)
{
	ui->setupUi(this);

	ui->textBrowserMainText->setStyleSheet("font: 12pt \"Times New Roman\";");

	Initialize();

// UI Connections:
	connect(ui->textBrowserMainText, SIGNAL(gotoIndex(const CRelIndex &, unsigned int)), this, SLOT(gotoIndex(const CRelIndex &, unsigned int)));
	connect(ui->textBrowserMainText, SIGNAL(sourceChanged(const QUrl &)), this, SLOT(on_sourceChanged(const QUrl &)));

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

CScriptureBrowser *CKJVBrowser::browser()
{
	return ui->textBrowserMainText;
}

// ----------------------------------------------------------------------------

void CKJVBrowser::Initialize(const CRelIndex &nInitialIndex, const QColor &colorHighlight)
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

	m_colorHighlight = colorHighlight;

	if (nInitialIndex.isSet()) gotoIndex(nInitialIndex);
}

void CKJVBrowser::gotoIndex(const CRelIndex &ndx, unsigned int nWrdCount)
{
	begin_update();
	ui->textBrowserMainText->setSource(QString("#%1").arg(ndx.asAnchor()));
	end_update();

	gotoIndex2(ndx, nWrdCount);
}

void CKJVBrowser::gotoIndex2(const CRelIndex &ndx, unsigned int nWrdCount)
{
	setBook(ndx);
	setChapter(ndx);
	setVerse(ndx);
	setWord(ndx, nWrdCount);

	doHighlighting();

	emit IndexChanged(ndx);
}

void CKJVBrowser::on_sourceChanged(const QUrl &src)
{
	if (m_bDoingUpdate) return;

	QString strURL = src.toString();		// Internal URLs are in the form of "#nnnnnnnn" as anchors
	int nPos = strURL.indexOf('#');
	if (nPos > -1) {
		CRelIndex ndxRel(strURL.mid(nPos+1));
		if (ndxRel.isSet()) gotoIndex2(ndxRel);
	}
}

void CKJVBrowser::focusBrowser()
{
	ui->textBrowserMainText->setFocus();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setHighlight(const TPhraseTagList &lstPhraseTags)
{
	doHighlighting(true);				// Remove existing highlighting
	m_lstPhraseTags = lstPhraseTags;	// Set new set of tags
	doHighlighting();					// Highlight using new tags
}

void CKJVBrowser::doHighlighting(bool bClear)
{
	for (int ndx=0; ndx<m_lstPhraseTags.size(); ++ndx) {
		CRelIndex ndxRel = m_lstPhraseTags.at(ndx).first;
		if (!ndxRel.isSet()) continue;
		// Save some time if the tag isn't anything close to what we are displaying.
		//		We'll use one before/one after since we might be displaying part of
		//		the proceding passage:
		if ((ndxRel.book() < (m_ndxCurrent.book()-1)) ||
			(ndxRel.book() > (m_ndxCurrent.book()+1)) ||
			(ndxRel.chapter() < (m_ndxCurrent.chapter()-1)) ||
			(ndxRel.chapter() > (m_ndxCurrent.chapter()+1))) continue;
		uint32_t ndxWord = ndxRel.word();
		ndxRel.setWord(0);
		int nPos = ui->textBrowserMainText->anchorPosition(ndxRel.asAnchor());
		if (nPos == -1) continue;
		CPhraseCursor myCursor(ui->textBrowserMainText->textCursor());
		int nSelStart = myCursor.anchor();
		int nSelEnd = myCursor.position();
		myCursor.setPosition(nPos);
		while (ndxWord) {
			myCursor.selectWordUnderCursor();
			myCursor.moveCursorWordRight();
			ndxWord--;
		}
		unsigned int nCount = m_lstPhraseTags.at(ndx).second;
		while (nCount) {
			QTextCharFormat fmt = myCursor.charFormat();
			QString strAnchorName = fmt.anchorName();
			if ((!fmt.isAnchor()) || (strAnchorName.startsWith('B'))) {		// Either we shouldn't be in an anchor or the end of an A-B special section marker
				myCursor.selectWordUnderCursor();
				fmt = myCursor.charFormat();
				if (!bClear) {
//					fmt.setUnderlineColor(m_colorHighlight);
//					fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
					// Save current brush in UserProperty so we can restore it later in undoHighlighting:
					fmt.setProperty(QTextFormat::UserProperty, QVariant(fmt.foreground()));
					fmt.setForeground(QBrush(m_colorHighlight));
				} else {
//					fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);
					// Restore preserved brush to restore text:
					fmt.setForeground(fmt.property(QTextFormat::UserProperty).value<QBrush>());
				}
				myCursor.setCharFormat(fmt);
				nCount--;
				if (!myCursor.moveCursorWordRight()) break;
			} else {
				// If we hit an anchor, see if it's either a special section A-B marker or if
				//		it's a chapter start anchor.  If it's an A-anchor, find the B-anchor.
				//		If it is a chapter start anchor, search for our special X-anchor so
				//		we'll be at the correct start of the next verse:
				if (strAnchorName.startsWith('A')) {
					int nEndAnchorPos = ui->textBrowserMainText->anchorPosition("B" + strAnchorName.mid(1));
					if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
				} else {
					CRelIndex ndxAnchor(strAnchorName);
					assert(ndxAnchor.isSet());
					if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
						int nEndAnchorPos = ui->textBrowserMainText->anchorPosition("X" + fmt.anchorName());
						if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
					}
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
		myCursor.setPosition(nSelStart, QTextCursor::MoveAnchor);
		myCursor.setPosition(nSelEnd, QTextCursor::KeepAnchor);
		ui->textBrowserMainText->setTextCursor(myCursor);
	}
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setBook(const CRelIndex &ndx)
{
	if (ndx.book() == 0) return;
	if (ndx.book() == m_ndxCurrent.book()) return;

	begin_update();

	m_ndxCurrent.setIndex(ndx.book(), 0, 0, 0);

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

void CKJVBrowser::setChapter(const CRelIndex &ndx)
{
	begin_update();

	m_ndxCurrent.setIndex(m_ndxCurrent.book(), ndx.chapter(), 0, 0);

	ui->comboBkChp->setCurrentIndex(ui->comboBkChp->findData(ndx.chapter()));

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
	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; }\n</style></head><body style=\" font-family:'Times New Roman'; font-size:12pt; font-weight:400; font-style:normal;\">\n")
						.arg(ndx.PassageReferenceText());		// Document Title
//	QString strHTML = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><style type=\"text/css\"><!-- A { text-decoration:none } %s --></style></head><body><br/>";

	uint32_t nFirstWordNormal = NormalizeIndex(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1));		// Find normalized word number for the first verse, first word of this book/chapter
	uint32_t nNextChapterFirstWordNormal = nFirstWordNormal + layout.m_nNumWrd;		// Add the number of words in this chapter to get first word normal of next chapter
	uint32_t nRelPrevChapter = DenormalizeIndex(nFirstWordNormal - 1);				// Find previous book/chapter/verse (and word)
	uint32_t nRelNextChapter = DenormalizeIndex(nNextChapterFirstWordNormal);		// Find next book/chapter/verse (and word)

	// Print last verse of previous chapter if available:
	if (nRelPrevChapter != 0) {
		CRelIndex relPrev(nRelPrevChapter);
		strHTML += "<p>";
		strHTML += QString("<a id=\"%1\"><bold> %2 </bold></a>").arg(CRelIndex(relPrev.book(), relPrev.chapter(), relPrev.verse(), 0).asAnchor()).arg(relPrev.verse());
		strHTML += (g_lstBooks[relPrev.book()-1])[CRelIndex(0,relPrev.chapter(),relPrev.verse(),0)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<hr/>\n";

	// Print Heading for this Book/Chapter:
	strHTML += QString("<h1><a id=\"%1\">%2</a></h1>\n")
					.arg(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0).asAnchor())
					.arg(toc.m_strBkName);
	strHTML += QString("<h2><a id=\"%1\">Chapter %2</a></h2><a id=\"X%3\"> </a>\n")
					.arg(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0).asAnchor())
					.arg(m_ndxCurrent.chapter())
					.arg(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0).asAnchor());

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
		}
		if (!bParagraph) {
			strHTML += "<p>";
			bParagraph = true;
		}
		strHTML += QString("<a id=\"%1\"><bold> %2 </bold></a>")
					.arg(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndxVrs+1, 0).asAnchor())
					.arg(ndxVrs+1);
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
			strHTML += QString("<h1><a id=\"%1\">%2</a></h1>\n")
							.arg(CRelIndex(relNext.book(), relNext.chapter(), 0 ,0).asAnchor())
							.arg(g_lstTOC[relNext.book()-1].m_strBkName);
		strHTML += QString("<h2><a id=\"%1\">Chapter %2</a></h2><a id=\"X%3\"> </a>\n")
							.arg(CRelIndex(relNext.book(), relNext.chapter(), 0, 0).asAnchor())
							.arg(relNext.chapter())
							.arg(CRelIndex(relNext.book(), relNext.chapter(), 0, 0).asAnchor());

		strHTML += "<p>";
		strHTML += QString("<a id=\"%1\"><bold> %2 </bold></a>").arg(CRelIndex(relNext.book(), relNext.chapter(), relNext.verse(), 0).asAnchor()).arg(relNext.verse());
		strHTML += (g_lstBooks[relNext.book()-1])[CRelIndex(0,relNext.chapter(),relNext.verse(),0)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<br/></body></html>";
	ui->textBrowserMainText->setHtml(strHTML);
}

void CKJVBrowser::setVerse(const CRelIndex &ndx)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndx.verse(), 0);

	CRelIndex ndxScroll = m_ndxCurrent;
	if (ndxScroll.verse() == 1) ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading
}

void CKJVBrowser::setWord(const CRelIndex &ndx, unsigned int nWrdCount)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), ndx.word());

	CRelIndex ndxScroll = m_ndxCurrent;
	if (ndxScroll.verse() == 1) ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading
	ndxScroll.setWord(0);

	begin_update();

	ui->textBrowserMainText->scrollToAnchor(ndxScroll.asAnchor());

	end_update();

	CRelIndex ndxRel = m_ndxCurrent;
	uint32_t ndxWord = ndxRel.word();
	ndxRel.setWord(0);
	int nPos = ui->textBrowserMainText->anchorPosition(ndxRel.asAnchor());
	if (nPos != -1) {
		CPhraseCursor myCursor(ui->textBrowserMainText->textCursor());
		myCursor.setPosition(nPos);
		while (ndxWord) {
			myCursor.moveCursorWordRight();
			ndxWord--;
		}
		int nSelEnd = myCursor.position();
		unsigned int nCount = nWrdCount;
		while (nCount) {
			QTextCharFormat fmt = myCursor.charFormat();
			if (!fmt.isAnchor()) {
				myCursor.moveCursorWordStart(QTextCursor::KeepAnchor);
				myCursor.moveCursorWordEnd(QTextCursor::KeepAnchor);
				fmt = myCursor.charFormat();
				if (!fmt.isAnchor()) nCount--;
			}
			nSelEnd = myCursor.position();
			if (!myCursor.moveCursorWordRight(QTextCursor::KeepAnchor)) break;
		}
		myCursor.setPosition(nSelEnd, QTextCursor::KeepAnchor);
		ui->textBrowserMainText->setTextCursor(myCursor);
	}
}

// ----------------------------------------------------------------------------

void CKJVBrowser::BkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui->comboBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::BkChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	ndxTarget.setBook(m_ndxCurrent.book());
	if (index != -1) {
		ndxTarget.setChapter(ui->comboBkChp->itemData(index).toUInt());
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::TstBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get TOC for current book so we know what testament we're currently in:
		const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
		ndxTarget = CRefCountCalc::calcRelIndex(0, 0, 0, ui->comboTstBk->itemData(index).toUInt(), toc.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::TstChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get TOC for current book so we know what testament we're currently in:
		const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
		ndxTarget = CRefCountCalc::calcRelIndex(0, 0, ui->comboTstChp->itemData(index).toUInt(), 0, toc.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::BibleBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui->comboBibleBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(ndxTarget);
}

void CKJVBrowser::BibleChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget = CRefCountCalc::calcRelIndex(0, 0, ui->comboBibleChp->itemData(index).toUInt(), 0, 0);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(ndxTarget);
}

// ----------------------------------------------------------------------------

