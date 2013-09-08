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
#include "VerseListModel.h"
#include "PersistentSettings.h"
#include "UserNotesDatabase.h"

#include "BusyCursor.h"

#include <assert.h>

#include <QComboBox>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>
#include <QApplication>
#include <QScrollBar>
#include <QToolTip>
#include <QMouseEvent>
#include <QStyleOptionSlider>

// ============================================================================

CKJVBrowser::CKJVBrowser(CVerseListModel *pModel, CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QWidget(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_ndxCurrent(0),
	m_Highlighter(pModel),
	m_bDoingUpdate(false),
	m_bDoingPassageReference(false),
	m_pScriptureBrowser(NULL)
{
	assert(m_pBibleDatabase != NULL);
	assert(g_pUserNotesDatabase != NULL);

	ui.setupUi(this);

	initialize();

	assert(m_pScriptureBrowser != NULL);

	setNavigationActivationDelay(CPersistentSettings::instance()->navigationActivationDelay());
	setPassageReferenceActivationDelay(CPersistentSettings::instance()->passageReferenceActivationDelay());

	connect(CPersistentSettings::instance(), SIGNAL(changedNavigationActivationDelay(int)), this, SLOT(setNavigationActivationDelay(int)));
	connect(CPersistentSettings::instance(), SIGNAL(changedPassageReferenceActivationDelay(int)), this, SLOT(setPassageReferenceActivationDelay(int)));

// Data Connections:
	connect(pModel, SIGNAL(verseListAboutToChange()), this, SLOT(en_SearchResultsVerseListAboutToChange()));
	connect(pModel, SIGNAL(verseListChanged()), this, SLOT(en_SearchResultsVerseListChanged()));

// UI Connections:
	connect(m_pScriptureBrowser, SIGNAL(gotoIndex(const TPhraseTag &)), this, SLOT(gotoIndex(const TPhraseTag &)));
	connect(m_pScriptureBrowser, SIGNAL(sourceChanged(const QUrl &)), this, SLOT(en_sourceChanged(const QUrl &)));

	connect(ui.comboBk, SIGNAL(currentIndexChanged(int)), this, SLOT(delayBkComboIndexChanged(int)));
	connect(ui.comboBkChp, SIGNAL(currentIndexChanged(int)), this, SLOT(delayBkChpComboIndexChanged(int)));
	connect(ui.comboTstBk, SIGNAL(currentIndexChanged(int)), this, SLOT(delayTstBkComboIndexChanged(int)));
	connect(ui.comboTstChp, SIGNAL(currentIndexChanged(int)), this, SLOT(delayTstChpComboIndexChanged(int)));
	connect(ui.comboBibleBk, SIGNAL(currentIndexChanged(int)), this, SLOT(delayBibleBkComboIndexChanged(int)));
	connect(ui.comboBibleChp, SIGNAL(currentIndexChanged(int)), this, SLOT(delayBibleChpComboIndexChanged(int)));

	connect(ui.widgetPassageReference, SIGNAL(passageReferenceChanged(const TPhraseTag &)), this, SLOT(delayPassageReference(const TPhraseTag &)));
	connect(ui.widgetPassageReference, SIGNAL(enterPressed()), this, SLOT(PassageReferenceEnterPressed()));

	connect(ui.scrollbarChapter, SIGNAL(valueChanged(int)), this, SLOT(ChapterSliderMoved(int)));
	connect(ui.scrollbarChapter, SIGNAL(sliderMoved(int)), this, SLOT(ChapterSliderMoved(int)));
	connect(ui.scrollbarChapter, SIGNAL(sliderReleased()), this, SLOT(ChapterSliderValueChanged()));

	connect(&m_dlyBkCombo, SIGNAL(triggered(int)), this, SLOT(BkComboIndexChanged(int)));
	connect(&m_dlyBkChpCombo, SIGNAL(triggered(int)), this, SLOT(BkChpComboIndexChanged(int)));
	connect(&m_dlyTstBkCombo, SIGNAL(triggered(int)), this, SLOT(TstBkComboIndexChanged(int)));
	connect(&m_dlyTstChpCombo, SIGNAL(triggered(int)), this, SLOT(TstChpComboIndexChanged(int)));
	connect(&m_dlyBibleBkCombo, SIGNAL(triggered(int)), this, SLOT(BibleBkComboIndexChanged(int)));
	connect(&m_dlyBibleChpCombo, SIGNAL(triggered(int)), this, SLOT(BibleChpComboIndexChanged(int)));

	connect(&m_dlyPassageReference, SIGNAL(triggered(const TPhraseTag &)), this, SLOT(PassageReferenceChanged(const TPhraseTag &)));

	// Set Outgoing Pass-Through Signals:
	connect(m_pScriptureBrowser, SIGNAL(activatedScriptureText()), this, SIGNAL(activatedScriptureText()));
	connect(m_pScriptureBrowser, SIGNAL(backwardAvailable(bool)), this, SIGNAL(backwardAvailable(bool)));
	connect(m_pScriptureBrowser, SIGNAL(forwardAvailable(bool)), this, SIGNAL(forwardAvailable(bool)));
	connect(m_pScriptureBrowser, SIGNAL(historyChanged()), this, SIGNAL(historyChanged()));

	// Set Incoming Pass-Through Signals:
	connect(this, SIGNAL(backward()), m_pScriptureBrowser, SLOT(backward()));
	connect(this, SIGNAL(forward()), m_pScriptureBrowser, SLOT(forward()));
	connect(this, SIGNAL(home()), m_pScriptureBrowser, SLOT(home()));
	connect(this, SIGNAL(reload()), m_pScriptureBrowser, SLOT(reload()));

	// Highlighting colors changing:
	connect(CPersistentSettings::instance(), SIGNAL(changedColorSearchResults(const QColor &)), this, SLOT(en_SearchResultsColorChanged(const QColor &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedColorWordsOfJesus(const QColor &)), this, SLOT(en_WordsOfJesusColorChanged(const QColor &)));

	connect(g_pUserNotesDatabase.data(), SIGNAL(highlighterTagsAboutToChange(CBibleDatabasePtr, const QString &)), this, SLOT(en_highlighterTagsAboutToChange(CBibleDatabasePtr, const QString &)));
	connect(g_pUserNotesDatabase.data(), SIGNAL(highlighterTagsChanged(CBibleDatabasePtr, const QString &)), this, SLOT(en_highlighterTagsChanged(CBibleDatabasePtr, const QString &)));
	connect(g_pUserNotesDatabase.data(), SIGNAL(aboutToChangeHighlighters()), this, SLOT(en_highlightersAboutToChange()));
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedHighlighters()), this, SLOT(en_highlightersChanged()));

	// User Notes changing:
	connect(g_pUserNotesDatabase.data(), SIGNAL(addedUserNote(const CRelIndex &)), this, SLOT(en_userNoteEvent(const CRelIndex &)));
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedUserNote(const CRelIndex &)), this, SLOT(en_userNoteEvent(const CRelIndex &)));
	connect(g_pUserNotesDatabase.data(), SIGNAL(removedUserNote(const CRelIndex &)), this, SLOT(en_userNoteEvent(const CRelIndex &)));

	// Cross Refs changing:
	connect(g_pUserNotesDatabase.data(), SIGNAL(addedCrossRef(const CRelIndex &, const CRelIndex &)), this, SLOT(en_crossRefsEvent(const CRelIndex &, const CRelIndex &)));
	connect(g_pUserNotesDatabase.data(), SIGNAL(removedCrossRef(const CRelIndex &, const CRelIndex &)), this, SLOT(en_crossRefsEvent(const CRelIndex &, const CRelIndex &)));
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedAllCrossRefs()), this, SLOT(en_allCrossRefsChanged()));
}

CKJVBrowser::~CKJVBrowser()
{

}

// ----------------------------------------------------------------------------

bool CKJVBrowser::eventFilter(QObject *obj, QEvent *ev)
{
	if ((obj == ui.scrollbarChapter) &&
		(ev->type() == QEvent::MouseMove) &&
		(ui.scrollbarChapter->isSliderDown())) {
		QMouseEvent *pMouseEvent = static_cast<QMouseEvent*>(ev);
		m_ptChapterScrollerMousePos = pMouseEvent->globalPos();
	}

	return QWidget::eventFilter(obj, ev);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setNavigationActivationDelay(int nDelay)
{
	m_dlyBkCombo.setMinimumDelay(nDelay);
	m_dlyBkChpCombo.setMinimumDelay(nDelay);
	m_dlyTstBkCombo.setMinimumDelay(nDelay);
	m_dlyTstChpCombo.setMinimumDelay(nDelay);
	m_dlyBibleBkCombo.setMinimumDelay(nDelay);
	m_dlyBibleChpCombo.setMinimumDelay(nDelay);
}

void CKJVBrowser::setPassageReferenceActivationDelay(int nDelay)
{
	m_dlyPassageReference.setMinimumDelay(nDelay);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::initialize()
{
	// --------------------------------------------------------------

	ui.widgetPassageReference->initialize(m_pBibleDatabase);

	// --------------------------------------------------------------

	//	Swapout the widgetKJVPassageNavigator from the layout with
	//		one that we can set the database on:

	int ndx = ui.gridLayout->indexOf(ui.textBrowserMainText);
	assert(ndx != -1);
	if (ndx == -1) return;
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;
	ui.gridLayout->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pScriptureBrowser = new CScriptureBrowser(m_pBibleDatabase, this);
	m_pScriptureBrowser->setObjectName(QString::fromUtf8("textBrowserMainText"));
	m_pScriptureBrowser->setMouseTracking(true);
	m_pScriptureBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_pScriptureBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pScriptureBrowser->setTabChangesFocus(false);
	m_pScriptureBrowser->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	m_pScriptureBrowser->setOpenLinks(false);

	delete ui.textBrowserMainText;
	ui.textBrowserMainText = NULL;
	ui.gridLayout->addWidget(m_pScriptureBrowser, nRow, nCol, nRowSpan, nColSpan);

	// Reinsert it in the correct TabOrder:
	QWidget::setTabOrder(ui.comboBkChp, m_pScriptureBrowser);
	QWidget::setTabOrder(m_pScriptureBrowser, ui.comboTstBk);

	// --------------------------------------------------------------

	begin_update();

	unsigned int nBibleChp = 0;
	ui.comboBk->clear();
	ui.comboBibleBk->clear();
	for (unsigned int ndxBk=1; ndxBk<=m_pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxBk);
		assert(pBook != NULL);
		ui.comboBk->addItem(pBook->m_strBkName, ndxBk);
		ui.comboBibleBk->addItem(QString("%1").arg(ndxBk), ndxBk);
		nBibleChp += pBook->m_nNumChp;
	}
	ui.comboBibleChp->clear();
	for (unsigned int ndxBibleChp=1; ndxBibleChp<=nBibleChp; ++ndxBibleChp) {
		ui.comboBibleChp->addItem(QString("%1").arg(ndxBibleChp), ndxBibleChp);
	}

	// Setup the Chapter Scroller:
	ui.scrollbarChapter->setStyle(&m_PlastiqueStyle);
	ui.scrollbarChapter->setRange(1, m_pBibleDatabase->bibleEntry().m_nNumChp);
	ui.scrollbarChapter->setTracking(true);
	ui.scrollbarChapter->setMouseTracking(true);
	ui.scrollbarChapter->installEventFilter(this);
	ui.scrollbarChapter->setSingleStep(1);
	ui.scrollbarChapter->setPageStep(3);

	end_update();
}

void CKJVBrowser::gotoIndex(const TPhraseTag &tag)
{
	// Note: Special case !tag->relIndex().isSet() means reload current index
	TPhraseTag tagActual = (tag.relIndex().isSet() ? tag : TPhraseTag(m_ndxCurrent, tag.count()));

	begin_update();

	if (!m_bDoingPassageReference) ui.widgetPassageReference->clear();

	// If branching to a "book only", goto chapter 1 of that book:
	if ((tagActual.relIndex().book() != 0) &&
		(tagActual.relIndex().chapter() == 0)) tagActual.relIndex().setChapter(1);

	m_pScriptureBrowser->setSource(QString("#%1").arg(tagActual.relIndex().asAnchor()));

	end_update();

	gotoIndex2(tag.relIndex().isSet() ? tagActual : tag);		// Pass special-case on to gotoIndex2
}

void CKJVBrowser::gotoIndex2(const TPhraseTag &tag)
{
	// Note: Special case !tag->relIndex().isSet() means reload current index
	TPhraseTag tagActual = (tag.relIndex().isSet() ? tag : TPhraseTag(m_ndxCurrent, tag.count()));

	setBook(tagActual.relIndex());
	setChapter(tagActual.relIndex());
	setVerse(tagActual.relIndex());
	setWord(tagActual);

	doHighlighting();

	emit en_gotoIndex(tagActual);
}

void CKJVBrowser::en_sourceChanged(const QUrl &src)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	QString strURL = src.toString();		// Internal URLs are in the form of "#nnnnnnnn" as anchors
	int nPos = strURL.indexOf('#');
	if (nPos > -1) {
		CRelIndex ndxRel(strURL.mid(nPos+1));
		if (ndxRel.isSet()) gotoIndex2(TPhraseTag(ndxRel));
	}
}

void CKJVBrowser::setFocusBrowser()
{
	m_pScriptureBrowser->setFocus();
}

bool CKJVBrowser::hasFocusBrowser() const
{
	return m_pScriptureBrowser->hasFocus();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::en_SearchResultsVerseListAboutToChange()
{
	doHighlighting(true);				// Remove existing highlighting
}

void CKJVBrowser::en_SearchResultsVerseListChanged()
{
	doHighlighting();					// Highlight using new tags
}

void CKJVBrowser::en_highlighterTagsAboutToChange(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName)
{
	Q_UNUSED(pBibleDatabase);
	Q_UNUSED(strUserDefinedHighlighterName);
	doHighlighting(true);				// Remove existing highlighting
}

void CKJVBrowser::en_highlighterTagsChanged(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName)
{
	Q_UNUSED(pBibleDatabase);
	Q_UNUSED(strUserDefinedHighlighterName);
	doHighlighting();					// Highlight using new tags
}

void CKJVBrowser::en_highlightersAboutToChange()
{
	doHighlighting(true);				// Remove existing highlighting
}

void CKJVBrowser::en_highlightersChanged()
{
	doHighlighting();					// Highlight using new tags
}

void CKJVBrowser::doHighlighting(bool bClear)
{
	m_pScriptureBrowser->navigator().doHighlighting(m_Highlighter, bClear, m_ndxCurrent);

	assert(g_pUserNotesDatabase != NULL);
	const THighlighterTagMap *pmapHighlighterTags = g_pUserNotesDatabase->highlighterTagsFor(m_pBibleDatabase);
	if (pmapHighlighterTags) {
		// Note: These are painted in sorted order so they overlay each other with alphabetical precedence:
		//			(the map is already sorted)
		for (THighlighterTagMap::const_iterator itrHighlighters = pmapHighlighterTags->begin(); itrHighlighters != pmapHighlighterTags->end(); ++itrHighlighters) {
			CUserDefinedHighlighter highlighter(itrHighlighters->first, itrHighlighters->second);
			m_pScriptureBrowser->navigator().doHighlighting(highlighter, bClear, m_ndxCurrent);
		}
	}

#if QT_VERSION >= 0x050000					// I HATE this work-around when using it with the highlighters!  Annoying jumps!!
	// Work around Qt5 bug.  Without this, rendering goes Minnie Mouse and
	//		the scroll jumps back a half-line on some lines after doing the
	//		highlighting -- usually noticeable just after a gotoIndex call:
	TPhraseTag tagSelection = m_pScriptureBrowser->navigator().getSelection();
	m_pScriptureBrowser->navigator().selectWords(tagSelection);
#endif
}

void CKJVBrowser::en_WordsOfJesusColorChanged(const QColor &color)
{
	// Only way we can change the Words of Jesus color is by forcing a chapter re-render,
	//		after we change the richifier tags (which is done by the navigator's
	//		signal/slot connection to the persistent settings:
	Q_UNUSED(color);
	if (m_ndxCurrent.isSet()) gotoIndex(TPhraseTag(m_ndxCurrent));
}

void CKJVBrowser::en_SearchResultsColorChanged(const QColor &color)
{
	// Simply redo the highlighting again to change the highlight color:
	Q_UNUSED(color);
	doHighlighting();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::en_userNoteEvent(const CRelIndex &ndx)
{
	if (!selection().isSet()) return;
	CRelIndex ndxNote = ndx;
	ndxNote.setWord(1);			// All incoming note references will be by verse instead of word and normalize only deals with words
	TPhraseTag tagCurrentDisplay = m_pScriptureBrowser->navigator().currentChapterDisplayPhraseTag(m_ndxCurrent);
	if ((!ndx.isSet()) ||
		(!tagCurrentDisplay.isSet()) ||
		(tagCurrentDisplay.intersects(m_pBibleDatabase, TPhraseTag(ndxNote)))) {
		gotoIndex(selection());			// Re-render text (note: The Note may be deleted as well as changed)
	}
}

void CKJVBrowser::en_crossRefsEvent(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond)
{
	if (!selection().isSet()) return;
	CRelIndex ndxCrossRefFirst = ndxFirst;
	CRelIndex ndxCrossRefSecond = ndxSecond;
	ndxCrossRefFirst.setWord(1);			// All incoming cross-ref references will be by verse instead of word and normalize only deals with words
	ndxCrossRefSecond.setWord(1);
	TPhraseTag tagCurrentDisplay = m_pScriptureBrowser->navigator().currentChapterDisplayPhraseTag(m_ndxCurrent);
	if ((!ndxFirst.isSet()) ||
		(!ndxSecond.isSet()) ||
		(!tagCurrentDisplay.isSet()) ||
		(tagCurrentDisplay.intersects(m_pBibleDatabase, TPhraseTag(ndxCrossRefFirst))) ||
		(tagCurrentDisplay.intersects(m_pBibleDatabase, TPhraseTag(ndxCrossRefSecond)))) {
		gotoIndex(selection());			// Re-render text (note: The Note may be deleted as well as changed)
	}

}

void CKJVBrowser::en_allCrossRefsChanged()
{
	if (!selection().isSet()) return;
	gotoIndex(selection());			// Re-render text (note: The Note may be deleted as well as changed)
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setFontScriptureBrowser(const QFont& aFont)
{
	m_pScriptureBrowser->setFont(aFont);
}

void CKJVBrowser::setTextBrightness(bool bInvert, int nBrightness)
{
	m_pScriptureBrowser->setTextBrightness(bInvert, nBrightness);
}

void CKJVBrowser::showDetails()
{
	m_pScriptureBrowser->showDetails();
}

void CKJVBrowser::showPassageNavigator()
{
	m_pScriptureBrowser->showPassageNavigator();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::en_Bible_Beginning()
{
	assert(m_pBibleDatabase.data() != NULL);

	gotoIndex(TPhraseTag(CRelIndex(1,1,1,1)));
}

void CKJVBrowser::en_Bible_Ending()
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndx;
	ndx.setBook(m_pBibleDatabase->bibleEntry().m_nNumBk);
	ndx.setChapter(m_pBibleDatabase->bookEntry(ndx.book())->m_nNumChp);
	ndx.setVerse(m_pBibleDatabase->chapterEntry(ndx)->m_nNumVrs);
	ndx.setWord(m_pBibleDatabase->verseEntry(ndx)->m_nNumWrd);
	gotoIndex(TPhraseTag(ndx));
}

void CKJVBrowser::en_Book_Backward()
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_ndxCurrent.book() < 2) return;

	gotoIndex(TPhraseTag(CRelIndex(m_ndxCurrent.book()-1, 1, 1, 1)));
}

void CKJVBrowser::en_Book_Forward()
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_ndxCurrent.book() >= m_pBibleDatabase->bibleEntry().m_nNumBk) return;

	gotoIndex(TPhraseTag(CRelIndex(m_ndxCurrent.book()+1, 1, 1, 1)));
}

void CKJVBrowser::en_ChapterBackward()
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndx = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), true);
	if (ndx.isSet()) gotoIndex(TPhraseTag(ndx));
}

void CKJVBrowser::en_ChapterForward()
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndx = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), false);
	if (ndx.isSet()) gotoIndex(TPhraseTag(ndx));
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setBook(const CRelIndex &ndx)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (ndx.book() == 0) return;
	if (ndx.book() == m_ndxCurrent.book()) return;

	begin_update();

	m_ndxCurrent.setIndex(ndx.book(), 0, 0, 0);

	if (m_ndxCurrent.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		end_update();
		return;
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());

	ui.comboBk->setCurrentIndex(ui.comboBk->findData(m_ndxCurrent.book()));
	ui.comboBibleBk->setCurrentIndex(ui.comboBibleBk->findData(m_ndxCurrent.book()));

	unsigned int nTst = book.m_nTstNdx;
	ui.lblTestament->setText(m_pBibleDatabase->testamentEntry(nTst)->m_strTstName + ":");
	ui.comboTstBk->clear();
	for (unsigned int ndxTstBk=1; ndxTstBk<=m_pBibleDatabase->testamentEntry(nTst)->m_nNumBk; ++ndxTstBk) {
		ui.comboTstBk->addItem(QString("%1").arg(ndxTstBk), ndxTstBk);
	}
	ui.comboTstBk->setCurrentIndex(ui.comboTstBk->findData(book.m_nTstBkNdx));

	ui.comboBkChp->clear();
	for (unsigned int ndxBkChp=1; ndxBkChp<=book.m_nNumChp; ++ndxBkChp) {
		ui.comboBkChp->addItem(QString("%1").arg(ndxBkChp), ndxBkChp);
	}
	ui.comboTstChp->clear();
	for (unsigned int ndxTstChp=1; ndxTstChp<=m_pBibleDatabase->testamentEntry(nTst)->m_nNumChp; ++ndxTstChp) {
		ui.comboTstChp->addItem(QString("%1").arg(ndxTstChp), ndxTstChp);
	}

	end_update();
}

void CKJVBrowser::setChapter(const CRelIndex &ndx)
{
	assert(m_pBibleDatabase.data() != NULL);

// Note: The following works great to keep from having to reload the chapter
//	text when navigating to the same chapter.  However, the History log doesn't
//	work correctly if we don't actually call setHtml()...  so...  just leave
//	it out and regenerate it:
//	(Also, repainting colors won't work right if we enable this optimization,
//	as changing things like the Words of Jesus needs to force a repaint.
//	if (ndx.chapter() == m_ndxCurrent.chapter()) return;

	begin_update();

	m_ndxCurrent.setIndex(m_ndxCurrent.book(), ndx.chapter(), 0, 0);

	ui.comboBkChp->setCurrentIndex(ui.comboBkChp->findData(ndx.chapter()));

	if ((m_ndxCurrent.book() == 0) || (m_ndxCurrent.chapter() == 0)) {
		m_pScriptureBrowser->clear();
		end_update();
		return;
	}

	if (m_ndxCurrent.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		end_update();
		return;
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());
	unsigned int nTstChp = 0;
	unsigned int nBibleChp = 0;
	for (unsigned int ndxBk=1; ndxBk<m_ndxCurrent.book(); ++ndxBk) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxBk);
		if (pBook->m_nTstNdx == book.m_nTstNdx)
			nTstChp += pBook->m_nNumChp;
		nBibleChp += pBook->m_nNumChp;
	}
	nTstChp += m_ndxCurrent.chapter();
	nBibleChp += m_ndxCurrent.chapter();

	ui.comboTstChp->setCurrentIndex(ui.comboTstChp->findData(nTstChp));
	ui.comboBibleChp->setCurrentIndex(ui.comboBibleChp->findData(nBibleChp));

	// Set the chapter scroller to the chapter of the Bible:
	ui.scrollbarChapter->setValue(CRefCountCalc(m_pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndx).ofBible().first);
//	ui.scrollbarChapter->setToolTip(m_pBibleDatabase->PassageReferenceText(CRelIndex(ndx.book(), ndx.chapter(), 0, 0)));

	end_update();

	m_pScriptureBrowser->navigator().setDocumentToChapter(ndx);
}

void CKJVBrowser::setVerse(const CRelIndex &ndx)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndx.verse(), 0);
}

void CKJVBrowser::setWord(const TPhraseTag &tag)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), tag.relIndex().word());
	m_pScriptureBrowser->navigator().selectWords(tag);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::BkComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui.comboBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BkChpComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	ndxTarget.setBook(m_ndxCurrent.book());
	if (index != -1) {
		ndxTarget.setChapter(ui.comboBkChp->itemData(index).toUInt());
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::TstBkComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get BookEntry for current book so we know what testament we're currently in:
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());
		ndxTarget = m_pBibleDatabase->calcRelIndex(0, 0, 0, ui.comboTstBk->itemData(index).toUInt(), book.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::TstChpComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get BookEntry for current book so we know what testament we're currently in:
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());
		ndxTarget = m_pBibleDatabase->calcRelIndex(0, 0, ui.comboTstChp->itemData(index).toUInt(), 0, book.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BibleBkComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui.comboBibleBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BibleChpComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget = m_pBibleDatabase->calcRelIndex(0, 0, ui.comboBibleChp->itemData(index).toUInt(), 0, 0);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::PassageReferenceChanged(const TPhraseTag &tag)
{
	if (m_bDoingUpdate) return;
	m_bDoingPassageReference = true;
	gotoIndex(tag);
	m_bDoingPassageReference = false;
}

void CKJVBrowser::PassageReferenceEnterPressed()
{
	if (m_bDoingUpdate) return;
	m_dlyPassageReference.untrigger();
	gotoIndex(ui.widgetPassageReference->phraseTag());
	setFocusBrowser();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::ChapterSliderMoved(int index)
{
	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget(m_pBibleDatabase->calcRelIndex(0, 0, index, 0, 0));
	ndxTarget.setVerse(0);
	ndxTarget.setWord(0);
	ui.scrollbarChapter->setToolTip(m_pBibleDatabase->PassageReferenceText(ndxTarget));
	if (!m_ptChapterScrollerMousePos.isNull()) {
		QToolTip::showText(m_ptChapterScrollerMousePos, ui.scrollbarChapter->toolTip());
	} else {
//		QToolTip::showText(ui.scrollbarChapter->mapToGlobal(QPoint( 0, 0 )), ui.scrollbarChapter->toolTip());
		QStyleOptionSlider opt;
		opt.initFrom(ui.scrollbarChapter);
		QRect rcSlider = ui.scrollbarChapter->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, ui.scrollbarChapter);
		QToolTip::showText(ui.scrollbarChapter->mapToGlobal(rcSlider.bottomLeft()), ui.scrollbarChapter->toolTip());
	}

	if (ui.scrollbarChapter->isSliderDown()) return;		// Just set ToolTip and exit
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::ChapterSliderValueChanged()
{
	ChapterSliderMoved(ui.scrollbarChapter->value());
	m_ptChapterScrollerMousePos = QPoint();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::delayBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;
	m_dlyBkCombo.trigger(index);
}

void CKJVBrowser::delayBkChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;
	m_dlyBkChpCombo.trigger(index);
}

void CKJVBrowser::delayTstBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;
	m_dlyTstBkCombo.trigger(index);
}

void CKJVBrowser::delayTstChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;
	m_dlyTstChpCombo.trigger(index);
}

void CKJVBrowser::delayBibleBkComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;
	m_dlyBibleBkCombo.trigger(index);
}

void CKJVBrowser::delayBibleChpComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;
	m_dlyBibleChpCombo.trigger(index);
}

void CKJVBrowser::delayPassageReference(const TPhraseTag &tag)
{
	if (m_bDoingUpdate) return;
	if (tag.isSet()) m_dlyPassageReference.trigger(tag);
}

// ----------------------------------------------------------------------------

