#include "KJVBrowser.h"
#include "ui_KJVBrowser.h"

#include "dbstruct.h"

#include "KJVPassageNavigatorDlg.h"

#include <assert.h>

#include <QHelpEvent>
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
	:	QTextBrowser(parent),
		m_navigator(*this)
{

}

CScriptureBrowser::~CScriptureBrowser()
{

}

bool CScriptureBrowser::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip) {
		QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(e);
		CRelIndex ndxReference = m_navigator.ResolveCursorReference(cursorForPosition(pHelpEvent->pos()));
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
	CRelIndex ndxReference = m_navigator.ResolveCursorReference(cursorForPosition(e->pos()));
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
	CPhraseNavigator(*ui->textBrowserMainText).doHighlighting(m_lstPhraseTags, m_colorHighlight, bClear, m_ndxCurrent);
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

	if ((m_ndxCurrent.book() == 0) || (m_ndxCurrent.chapter() == 0)) {
		ui->textBrowserMainText->clear();
		end_update();
		return;
	}

	if (m_ndxCurrent.book() > g_lstTOC.size()) {
		assert(false);
		end_update();
		return;
	}

	const CTOCEntry &toc = g_lstTOC[m_ndxCurrent.book()-1];
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

	CPhraseNavigator(*ui->textBrowserMainText).fillEditorWithChapter(ndx);
}

void CKJVBrowser::setVerse(const CRelIndex &ndx)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndx.verse(), 0);
}

void CKJVBrowser::setWord(const CRelIndex &ndx, unsigned int nWrdCount)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), ndx.word());
	CPhraseNavigator(*ui->textBrowserMainText).selectWords(ndx, nWrdCount);
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

