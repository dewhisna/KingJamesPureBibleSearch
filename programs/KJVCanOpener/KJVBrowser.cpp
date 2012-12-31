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

#include "KJVBrowser.h"
#include "ui_KJVBrowser.h"

#include <assert.h>

#include <QComboBox>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>


// ============================================================================

CKJVBrowser::CKJVBrowser(QWidget *parent) :
	QWidget(parent),
	m_ndxCurrent(0),
	m_bDoingUpdate(false),
	ui(new Ui::CKJVBrowser)
{
	ui->setupUi(this);

	Initialize();

// UI Connections:
	connect(ui->textBrowserMainText, SIGNAL(gotoIndex(const TPhraseTag &)), this, SLOT(gotoIndex(const TPhraseTag &)));
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

void CKJVBrowser::Initialize(const TPhraseTag &nInitialIndex)
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

	if (nInitialIndex.first.isSet()) gotoIndex(nInitialIndex);
}

void CKJVBrowser::gotoIndex(const TPhraseTag &tag)
{
	TPhraseTag tagActual = tag;

	begin_update();

	// If branching to a "book only", goto chapter 1 of that book:
	if ((tagActual.first.book() != 0) &&
		(tagActual.first.chapter() == 0)) tagActual.first.setChapter(1);

	ui->textBrowserMainText->setSource(QString("#%1").arg(tagActual.first.asAnchor()));

	end_update();

	gotoIndex2(tagActual);
}

void CKJVBrowser::gotoIndex2(const TPhraseTag &tag)
{
	setBook(tag.first);
	setChapter(tag.first);
	setVerse(tag.first);
	setWord(tag);

	doHighlighting();

	emit IndexChanged(tag);
}

void CKJVBrowser::on_sourceChanged(const QUrl &src)
{
	if (m_bDoingUpdate) return;

	QString strURL = src.toString();		// Internal URLs are in the form of "#nnnnnnnn" as anchors
	int nPos = strURL.indexOf('#');
	if (nPos > -1) {
		CRelIndex ndxRel(strURL.mid(nPos+1));
		if (ndxRel.isSet()) gotoIndex2(TPhraseTag(ndxRel));
	}
}

void CKJVBrowser::focusBrowser()
{
	ui->textBrowserMainText->setFocus();
}

bool CKJVBrowser::hasFocusBrowser() const
{
	return ui->textBrowserMainText->hasFocus();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setHighlightTags(const TPhraseTagList &lstPhraseTags)
{
	doHighlighting(true);				// Remove existing highlighting
	m_Highlighter.setHighlightTags(lstPhraseTags);
	doHighlighting();					// Highlight using new tags
}

void CKJVBrowser::doHighlighting(bool bClear)
{
	ui->textBrowserMainText->navigator().doHighlighting(m_Highlighter, bClear, m_ndxCurrent);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::on_Bible_Beginning()
{
	gotoIndex(TPhraseTag(CRelIndex(1,1,1,1)));
}

void CKJVBrowser::on_Bible_Ending()
{
	CRelIndex ndx;
	ndx.setBook(g_lstTOC.size());
	ndx.setChapter(g_lstTOC.at(g_lstTOC.size()-1).m_nNumChp);
	ndx.setVerse(g_mapLayout[CRelIndex(ndx.book(), ndx.chapter(), 0, 0)].m_nNumVrs);
	ndx.setWord((g_lstBooks[ndx.book()-1])[CRelIndex(0, ndx.chapter(), ndx.verse(), 0)].m_nNumWrd);
	gotoIndex(TPhraseTag(ndx));
}

void CKJVBrowser::on_Book_Backward()
{
	if (m_ndxCurrent.book() < 2) return;

	gotoIndex(TPhraseTag(CRelIndex(m_ndxCurrent.book()-1, 1, 1, 1)));
}

void CKJVBrowser::on_Book_Forward()
{
	if (m_ndxCurrent.book() >= g_lstTOC.size()) return;

	gotoIndex(TPhraseTag(CRelIndex(m_ndxCurrent.book()+1, 1, 1, 1)));
}

void CKJVBrowser::on_ChapterBackward()
{
	CRelIndex ndx = CRefCountCalc::calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), true);
	if (ndx.isSet()) gotoIndex(TPhraseTag(ndx));
}

void CKJVBrowser::on_ChapterForward()
{
	CRelIndex ndx = CRefCountCalc::calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), false);
	if (ndx.isSet()) gotoIndex(TPhraseTag(ndx));
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

	ui->textBrowserMainText->navigator().setDocumentToChapter(ndx);
}

void CKJVBrowser::setVerse(const CRelIndex &ndx)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndx.verse(), 0);

}

void CKJVBrowser::setWord(const TPhraseTag &tag)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), tag.first.word());
	ui->textBrowserMainText->navigator().selectWords(tag);
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
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BkChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	ndxTarget.setBook(m_ndxCurrent.book());
	if (index != -1) {
		ndxTarget.setChapter(ui->comboBkChp->itemData(index).toUInt());
	}
	gotoIndex(TPhraseTag(ndxTarget));
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
	gotoIndex(TPhraseTag(ndxTarget));
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
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BibleBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui->comboBibleBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(TPhraseTag(ndxTarget));
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
	gotoIndex(TPhraseTag(ndxTarget));
}

// ----------------------------------------------------------------------------

