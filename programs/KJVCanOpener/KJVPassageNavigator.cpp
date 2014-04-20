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

#include "KJVPassageNavigator.h"

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "Highlighter.h"
#include "BusyCursor.h"
#include "Translator.h"

#include <QTextCursor>

#include <assert.h>

// ============================================================================

// Placeholder Constructor:

CKJVPassageNavigator::CKJVPassageNavigator(CBibleDatabasePtr pBibleDatabase, QWidget *parent, NavigatorRefTypeOptionFlags flagsRefTypes, NAVIGATOR_REF_TYPE_ENUM nRefType)
	:	QWidget(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_nTestament(0),
		m_nBook(0),
		m_nChapter(0),
		m_nVerse(0),
		m_nWord(0),
		m_flagsRefTypes(flagsRefTypes),
		m_nRefType(nRefType),
		m_bDoingUpdate(false)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui.setupUi(this);

	initialize();

	assert(m_pEditVersePreview != NULL);

	QAction *pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), m_pEditVersePreview, SLOT(showDetails()));

	connect(ui.comboTestament, SIGNAL(currentIndexChanged(int)), this, SLOT(en_TestamentComboIndexChanged(int)));
	connect(ui.spinWord, SIGNAL(valueChanged(int)), this, SLOT(en_WordChanged(int)));
	connect(ui.spinVerse, SIGNAL(valueChanged(int)), this, SLOT(en_VerseChanged(int)));
	connect(ui.spinChapter, SIGNAL(valueChanged(int)), this, SLOT(en_ChapterChanged(int)));
	connect(ui.spinBook, SIGNAL(valueChanged(int)), this, SLOT(en_BookChanged(int)));
	connect(ui.chkboxReverse, SIGNAL(clicked(bool)), this, SLOT(en_ReverseChanged(bool)));
	connect(ui.comboRefType, SIGNAL(currentIndexChanged(int)), this, SLOT(en_RefTypeChanged(int)));
	connect(ui.comboBookDirect, SIGNAL(currentIndexChanged(int)), this, SLOT(en_BookDirectChanged(int)));
	connect(ui.comboChapterDirect, SIGNAL(currentIndexChanged(int)), this, SLOT(en_ChapterDirectChanged(int)));
	connect(ui.comboVerseDirect, SIGNAL(currentIndexChanged(int)), this, SLOT(en_VerseDirectChanged(int)));
	connect(ui.comboWordDirect, SIGNAL(currentIndexChanged(int)), this, SLOT(en_WordDirectChanged(int)));
	connect(ui.widgetPassageReference, SIGNAL(passageReferenceChanged(const TPhraseTag &)), this, SLOT(en_PassageReferenceChanged(const TPhraseTag &)));
	connect(m_pEditVersePreview, SIGNAL(gotoIndex(const TPhraseTag &)), this, SIGNAL(gotoIndex(const TPhraseTag &)));
}

CKJVPassageNavigator::~CKJVPassageNavigator()
{

}

void CKJVPassageNavigator::initialize()
{
	// --------------------------------------------------------------

	ui.widgetPassageReference->initialize(m_pBibleDatabase);

	// --------------------------------------------------------------

	//	Swapout the editVersePreview from the layout with
	//		one that we can set the database on:

	m_pEditVersePreview = new CScriptureEdit(m_pBibleDatabase, this);
	m_pEditVersePreview->setObjectName(QString::fromUtf8("editVersePreview"));
	m_pEditVersePreview->setMinimumSize(QSize(200, 150));
	m_pEditVersePreview->setMouseTracking(true);
	m_pEditVersePreview->setAcceptDrops(false);
	m_pEditVersePreview->setTabChangesFocus(true);
	m_pEditVersePreview->setUndoRedoEnabled(false);
	m_pEditVersePreview->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

	delete ui.editVersePreview;
	ui.editVersePreview = NULL;
	ui.verticalLayoutMain->addWidget(m_pEditVersePreview);

	// Updated Tab Ordering:
	QWidget::setTabOrder(ui.widgetPassageReference, ui.spinWord);
	QWidget::setTabOrder(ui.spinWord, ui.spinVerse);
	QWidget::setTabOrder(ui.spinVerse, ui.spinChapter);
	QWidget::setTabOrder(ui.spinChapter, ui.spinBook);
	QWidget::setTabOrder(ui.spinBook, ui.comboTestament);
	QWidget::setTabOrder(ui.comboTestament, ui.editStartRef);
	QWidget::setTabOrder(ui.editStartRef, ui.editResolved);
	QWidget::setTabOrder(ui.editResolved, ui.editVersePreview);
	QWidget::setTabOrder(ui.editVersePreview, ui.comboBookDirect);
	QWidget::setTabOrder(ui.comboBookDirect, ui.comboChapterDirect);
	QWidget::setTabOrder(ui.comboChapterDirect, ui.comboVerseDirect);
	QWidget::setTabOrder(ui.comboVerseDirect, ui.comboWordDirect);
	QWidget::setTabOrder(ui.comboWordDirect, ui.comboRefType);
	QWidget::setTabOrder(ui.comboRefType, ui.chkboxReverse);

	// Add the ScriptureEdit's editMenu to this widget's actions so that the
	//		keyboard shortcuts work correctly inside this widget:
	addAction(m_pEditVersePreview->getEditMenu()->menuAction());

	// --------------------------------------------------------------

	begin_update();

	m_tagStartRef = TPhraseTag(CRelIndex(), 1);		// Start with default word-size of one so we highlight at least one word when tracking
	m_tagPassage = TPhraseTag(CRelIndex(), 1);		// ""  (ditto)

	int nBooks = 0;
	int nChapters = 0;
	int nVerses = 0;
	int nWords = 0;

	ui.comboTestament->clear();
	for (unsigned int ndx=0; ndx<=m_pBibleDatabase->bibleEntry().m_nNumTst; ++ndx){
		if (ndx == 0) {
			// Search for "Entire Bible".  First try and see if we can translate it in the language of the selected Bible,
			//		but if not, try in the current language setting
			QString strEntireBible = tr("Entire Bible", "Scope");
			TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
			if (pTranslator.data() != NULL) {
				QString strTemp = pTranslator->translator().translate("CKJVPassageNavigator", "Entire Bible", "Scope");
				if (!strTemp.isEmpty()) strEntireBible = strTemp;
			}
			ui.comboTestament->addItem(strEntireBible, ndx);
		} else {
			ui.comboTestament->addItem(m_pBibleDatabase->testamentEntry(ndx)->m_strTstName, ndx);
			nBooks += m_pBibleDatabase->testamentEntry(ndx)->m_nNumBk;
			nChapters += m_pBibleDatabase->testamentEntry(ndx)->m_nNumChp;
			nVerses += m_pBibleDatabase->testamentEntry(ndx)->m_nNumVrs;
			nWords += m_pBibleDatabase->testamentEntry(ndx)->m_nNumWrd;
		}
	}

	ui.spinBook->setRange(0, nBooks);
	ui.spinChapter->setRange(0, nChapters);
	ui.spinVerse->setRange(0, nVerses);
	ui.spinWord->setRange(0, nWords);

	bool bAllTypes = (m_flagsRefTypes == NRTO_Default);
	ui.comboRefType->clear();
	if ((m_flagsRefTypes & NRTO_Word) || (bAllTypes)) ui.comboRefType->addItem(tr("Word", "Scope"), static_cast<int>(NRTE_WORD));
	if ((m_flagsRefTypes & NRTO_Verse) || (bAllTypes)) ui.comboRefType->addItem(tr("Verse", "Scope"), static_cast<int>(NRTE_VERSE));
	if ((m_flagsRefTypes & NRTO_Chapter) || (bAllTypes)) ui.comboRefType->addItem(tr("Chapter", "Scope"), static_cast<int>(NRTE_CHAPTER));
	if ((m_flagsRefTypes & NRTO_Book) || (bAllTypes)) ui.comboRefType->addItem(tr("Book", "Scope"), static_cast<int>(NRTE_BOOK));
	int nTypeIndex = ui.comboRefType->findData(static_cast<int>(m_nRefType));
	assert(nTypeIndex != -1);
	ui.comboRefType->setCurrentIndex(nTypeIndex);

	ui.comboBookDirect->clear();
	for (unsigned int ndxBk=1; ndxBk<=m_pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxBk);
		assert(pBook != NULL);
		ui.comboBookDirect->addItem(pBook->m_strBkName, ndxBk);
	}

	end_update();

	startAbsoluteMode();
	reset();
}

void CKJVPassageNavigator::reset()
{
	assert(m_pBibleDatabase.data() != NULL);

	if (isAbsolute()) {
		setPassage(TPhraseTag(CRelIndex(1, 1, 1, 1), m_tagPassage.count()));		// Default to Genesis 1:1 [1]
	} else {
		setPassage(TPhraseTag(CRelIndex(), m_tagPassage.count()));
	}

	ui.widgetPassageReference->clear();
}

// ============================================================================

void CKJVPassageNavigator::en_TestamentComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nTestament = ui.comboTestament->itemData(index).toUInt();
	CalcPassage();
}

void CKJVPassageNavigator::en_BookChanged(int nBook)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nBook = nBook;
	CalcPassage();
}

void CKJVPassageNavigator::en_ChapterChanged(int nChapter)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nChapter = nChapter;
	CalcPassage();
}

void CKJVPassageNavigator::en_VerseChanged(int nVerse)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nVerse = nVerse;
	CalcPassage();
}

void CKJVPassageNavigator::en_WordChanged(int nWord)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nWord = nWord;
	CalcPassage();
}

void CKJVPassageNavigator::en_ReverseChanged(bool bReverse)
{
	ui.spinBook->setPrefix(bReverse ? "-" : "");
	ui.spinChapter->setPrefix(bReverse ? "-" : "");
	ui.spinVerse->setPrefix(bReverse ? "-" : "");
	ui.spinWord->setPrefix(bReverse ? "-" : "");

	if (m_bDoingUpdate) return;

	CalcPassage();
}

void CKJVPassageNavigator::en_RefTypeChanged(int nType)
{
	if (m_bDoingUpdate) return;

	m_nRefType = static_cast<NAVIGATOR_REF_TYPE_ENUM>(ui.comboRefType->itemData(nType).toInt());
	CalcPassage();
}

// ============================================================================

void CKJVPassageNavigator::en_BookDirectChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	if (index != -1) {
		CRelIndex ndxTarget;
		ndxTarget.setBook(ui.comboBookDirect->itemData(index).toUInt());
		ndxTarget.setChapter(1);
		ndxTarget.setVerse(1);
		ndxTarget.setWord(1);
		setPassage(TPhraseTag(ndxTarget));
	}
}

void CKJVPassageNavigator::en_ChapterDirectChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	if ((ui.comboChapterDirect->currentIndex() != -1) &&
		(index != -1)) {
		CRelIndex ndxTarget;
		ndxTarget.setBook(ui.comboBookDirect->itemData(ui.comboBookDirect->currentIndex()).toUInt());
		ndxTarget.setChapter(ui.comboChapterDirect->itemData(index).toUInt());
		ndxTarget.setVerse(1);
		ndxTarget.setWord(1);
		setPassage(TPhraseTag(ndxTarget));
	}
}

void CKJVPassageNavigator::en_VerseDirectChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	if ((ui.comboBookDirect->currentIndex() != -1) &&
		(ui.comboChapterDirect->currentIndex() != -1) &&
		(index != -1)) {
		CRelIndex ndxTarget;
		ndxTarget.setBook(ui.comboBookDirect->itemData(ui.comboBookDirect->currentIndex()).toUInt());
		ndxTarget.setChapter(ui.comboChapterDirect->itemData(ui.comboChapterDirect->currentIndex()).toUInt());
		ndxTarget.setVerse(ui.comboVerseDirect->itemData(index).toUInt());
		ndxTarget.setWord(1);
		setPassage(TPhraseTag(ndxTarget));
	}
}

void CKJVPassageNavigator::en_WordDirectChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	if ((ui.comboBookDirect->currentIndex() != -1) &&
		(ui.comboChapterDirect->currentIndex() != -1) &&
		(ui.comboVerseDirect->currentIndex() != -1) &&
		(index != -1)) {
		CRelIndex ndxTarget;
		ndxTarget.setBook(ui.comboBookDirect->itemData(ui.comboBookDirect->currentIndex()).toUInt());
		ndxTarget.setChapter(ui.comboChapterDirect->itemData(ui.comboChapterDirect->currentIndex()).toUInt());
		ndxTarget.setVerse(ui.comboVerseDirect->itemData(ui.comboVerseDirect->currentIndex()).toUInt());
		ndxTarget.setWord(ui.comboWordDirect->itemData(index).toUInt());
		setPassage(TPhraseTag(ndxTarget));
	}
}

void CKJVPassageNavigator::setDirectReference(const CRelIndex &ndx)
{
	assert(m_pBibleDatabase.data() != NULL);

	// Special "not set" case:
	if (!ndx.isSet()) {
		begin_update();
		ui.comboBookDirect->setCurrentIndex(-1);
		ui.comboChapterDirect->clear();
		ui.comboVerseDirect->clear();
		ui.comboWordDirect->clear();
		end_update();
		return;
	}

	// It's OK for the whole reference to not be set (above), but not one specific piece only:
	if ((ndx.book() == 0) || (ndx.chapter() == 0) || (ndx.verse() == 0) || (ndx.word() == 0)) {
		assert(false);
		return;
	}

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		return;
	}

	begin_update();

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	ui.comboBookDirect->setCurrentIndex(ui.comboBookDirect->findData(ndx.book()));

	ui.comboChapterDirect->clear();
	for (unsigned int ndxChp=1; ndxChp<=book.m_nNumChp; ++ndxChp) {
		const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(CRelIndex(ndx.book(), ndxChp, 0, 0));
		if (pChapter == NULL) continue;
		if (pChapter->m_nNumVrs == 0) continue;			// Skip chapters that are empty (like additions of Esther in Apocrypha)
		ui.comboChapterDirect->addItem(QString("%1").arg(ndxChp), ndxChp);
	}
	ui.comboChapterDirect->setCurrentIndex(ui.comboChapterDirect->findData(ndx.chapter()));

	if (ndx.chapter() > book.m_nNumChp) {
		assert(false);
		end_update();
		return;
	}

	const CChapterEntry &chapter = *m_pBibleDatabase->chapterEntry(ndx);

	ui.comboVerseDirect->clear();
	for (unsigned int ndxVrs=1; ndxVrs<=chapter.m_nNumVrs; ++ndxVrs) {
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(CRelIndex(ndx.book(), ndx.chapter(), ndxVrs, 0));
		if (pVerse == NULL) continue;
		if (pVerse->m_nNumWrd == 0) continue;		// Skip verses that are empty (like additions of Esther in Apocrypha)
		ui.comboVerseDirect->addItem(QString("%1").arg(ndxVrs), ndxVrs);
	}
	ui.comboVerseDirect->setCurrentIndex(ui.comboVerseDirect->findData(ndx.verse()));

	if (ndx.verse() > chapter.m_nNumVrs) {
		assert(false);
		end_update();
		return;
	}

	const CVerseEntry &verse = *m_pBibleDatabase->verseEntry(ndx);

	ui.comboWordDirect->clear();
	for (unsigned int ndxWrd=1; ndxWrd<=verse.m_nNumWrd; ++ndxWrd) {
		ui.comboWordDirect->addItem(QString("%1").arg(ndxWrd), ndxWrd);
	}
	ui.comboWordDirect->setCurrentIndex(ui.comboWordDirect->findData(ndx.word()));

	end_update();
}

// ============================================================================

void CKJVPassageNavigator::en_PassageReferenceChanged(const TPhraseTag &tagPhrase)
{
	if (tagPhrase.isSet()) setPassage(tagPhrase);
}

// ============================================================================

TPhraseTag CKJVPassageNavigator::passage() const
{
	TPhraseTag tagPassage;

	switch (m_nRefType) {
		case NRTE_WORD:
			tagPassage = m_tagPassage;
			break;
		case NRTE_VERSE:
			tagPassage = TPhraseTag(CRelIndex(m_tagPassage.relIndex().book(), m_tagPassage.relIndex().chapter(), m_tagPassage.relIndex().verse(), 0), m_tagPassage.count());
			break;
		case NRTE_CHAPTER:
			tagPassage = TPhraseTag(CRelIndex(m_tagPassage.relIndex().book(), m_tagPassage.relIndex().chapter(), 0, 0), m_tagPassage.count());
			break;
		case NRTE_BOOK:
			tagPassage = TPhraseTag(CRelIndex(m_tagPassage.relIndex().book(), 0, 0, 0), m_tagPassage.count());
			break;
		default:
			assert(false);
			break;
	}

	return tagPassage;
}

void CKJVPassageNavigator::setPassage(const TPhraseTag &tag)
{
	begin_update();

	m_tagPassage = tag;

	ui.comboTestament->setCurrentIndex(ui.comboTestament->findData(0));
	m_nTestament = 0;
	ui.spinBook->setValue(tag.relIndex().book());
	m_nBook = tag.relIndex().book();
	ui.spinChapter->setValue(tag.relIndex().chapter());
	m_nChapter = tag.relIndex().chapter();
	ui.spinVerse->setValue(tag.relIndex().verse());
	m_nVerse = tag.relIndex().verse();
	ui.spinWord->setValue(tag.relIndex().word());
	m_nWord = tag.relIndex().word();
	CalcPassage();

	end_update();
}

void CKJVPassageNavigator::CalcPassage()
{
	assert(m_pBibleDatabase.data() != NULL);

	m_tagPassage.relIndex() = m_pBibleDatabase->calcRelIndex(m_nWord, m_nVerse, m_nChapter, m_nBook, (!m_tagStartRef.relIndex().isSet() ? m_nTestament : 0), m_tagStartRef.relIndex(), (!m_tagStartRef.relIndex().isSet() ? false : ui.chkboxReverse->isChecked()));
	ui.editResolved->setText(m_pBibleDatabase->PassageReferenceText(passage().relIndex()));
	CPhraseEditNavigator navigator(m_pBibleDatabase, *m_pEditVersePreview);

	setDirectReference(m_tagPassage.relIndex());

	CRelIndex ndxWord(m_tagPassage.relIndex());
	CRelIndex ndxVerse(ndxWord);
	ndxVerse.setWord(0);
	CRelIndex ndxChapter(ndxVerse);
	ndxChapter.setVerse(0);
	CRelIndex ndxBook(ndxChapter);
	ndxBook.setChapter(0);

	QTextCursor txtCursor;

	switch (m_nRefType) {
		case NRTE_WORD:
			navigator.setDocumentToVerse(ndxWord, defaultDocumentToVerseFlags);
			navigator.doHighlighting(CSearchResultHighlighter(m_tagPassage));
			break;
		case NRTE_VERSE:
			navigator.setDocumentToVerse(ndxVerse, defaultDocumentToVerseFlags);
			break;
		case NRTE_CHAPTER:
			navigator.setDocumentToChapter(ndxChapter, CPhraseNavigator::TRO_Colophons | CPhraseNavigator::TRO_Subtitles | CPhraseNavigator::TRO_Category | CPhraseNavigator::TRO_SuppressPrePostChapters);
			txtCursor = m_pEditVersePreview->textCursor();
			txtCursor.movePosition(QTextCursor::Start);
			m_pEditVersePreview->setTextCursor(txtCursor);
			break;
		case NRTE_BOOK:
			navigator.setDocumentToBookInfo(ndxBook, defaultDocumentToBookInfoFlags);
			break;
	}

}

void CKJVPassageNavigator::setRefType(NAVIGATOR_REF_TYPE_ENUM nRefType)
{
	begin_update();

	int nTypeIndex = ui.comboRefType->findData(static_cast<int>(nRefType));
	assert(nTypeIndex != -1);
	if (nTypeIndex != -1) {
		m_nRefType = nRefType;
		ui.comboRefType->setCurrentIndex(nTypeIndex);
		CalcPassage();
	}

	end_update();
}

void CKJVPassageNavigator::startRelativeMode(TPhraseTag tagStart, TPhraseTag tagPassage)
{
	startRelativeMode(tagStart, isReversed(), tagPassage);
}

void CKJVPassageNavigator::startRelativeMode(TPhraseTag tagStart, bool bReverse, TPhraseTag tagPassage)
{
	assert(m_pBibleDatabase.data() != NULL);

	begin_update();

	if (tagStart.relIndex().isSet()) {
		m_tagStartRef = tagStart;
	} else {
		m_tagStartRef = TPhraseTag(CRelIndex(1,1,1,1), 1);
	}

	ui.lblTestament->hide();
	ui.comboTestament->hide();

	ui.lblStartRef->show();
	ui.editStartRef->show();
	ui.chkboxReverse->show();

	ui.editStartRef->setText(m_pBibleDatabase->PassageReferenceText(m_tagStartRef.relIndex()));
	ui.chkboxReverse->setChecked(bReverse);

	ui.lblBook->setText(tr("&Books:", "CKJVPassageNavigator"));
	ui.lblChapter->setText(tr("C&hapters:", "CKJVPassageNavigator"));
	ui.lblVerse->setText(tr("&Verses:", "CKJVPassageNavigator"));
	ui.lblWord->setText(tr("&Words:", "CKJVPassageNavigator"));

	ui.lblBookDirect->setVisible(false);
	ui.comboBookDirect->setVisible(false);
	ui.lblChapterDirect->setVisible(false);
	ui.comboChapterDirect->setVisible(false);
	ui.lblVerseDirect->setVisible(false);
	ui.comboVerseDirect->setVisible(false);
	ui.lblWordDirect->setVisible(false);
	ui.comboWordDirect->setVisible(false);
	ui.widgetPassageReference->setVisible(false);
	ui.lineDirectReference->setVisible(false);
	ui.linePassageReference->setVisible(false);
	ui.widgetPassageReference->clear();

	if (!tagPassage.relIndex().isSet()) {
		// If we don't have an absolute starting passage, set the passage size (that we'll calculate from
		//		our zero-relative) to be the size of the starting reference passage:
		tagPassage.count() = tagStart.count();
	}
	setPassage(tagPassage);			// Note: setPassage will already call CalcPassage

	emit modeChanged(true);

	end_update();
}

void CKJVPassageNavigator::startAbsoluteMode(TPhraseTag tagPassage)
{
	assert(m_pBibleDatabase.data() != NULL);

	begin_update();

	m_tagStartRef = TPhraseTag(CRelIndex(), 1);		// Unset (but one word) to indicate absolute mode

	ui.lblStartRef->hide();
	ui.editStartRef->hide();
	ui.chkboxReverse->setChecked(false);
	ui.chkboxReverse->hide();

	ui.lblTestament->show();
	ui.comboTestament->show();

	ui.lblBook->setText(tr("&Book:", "CKJVPassageNavigator"));
	ui.lblChapter->setText(tr("C&hapter:", "CKJVPassageNavigator"));
	ui.lblVerse->setText(tr("&Verse:", "CKJVPassageNavigator"));
	ui.lblWord->setText(tr("&Word:", "CKJVPassageNavigator"));

	ui.spinBook->setPrefix("");
	ui.spinChapter->setPrefix("");
	ui.spinVerse->setPrefix("");
	ui.spinWord->setPrefix("");

	ui.lblBookDirect->setVisible(true);
	ui.comboBookDirect->setVisible(true);
	ui.lblChapterDirect->setVisible(true);
	ui.comboChapterDirect->setVisible(true);
	ui.lblVerseDirect->setVisible(true);
	ui.comboVerseDirect->setVisible(true);
	ui.lblWordDirect->setVisible(true);
	ui.comboWordDirect->setVisible(true);
	ui.widgetPassageReference->setVisible(true);
	ui.lineDirectReference->setVisible(true);
	ui.linePassageReference->setVisible(true);
	ui.widgetPassageReference->clear();
	ui.widgetPassageReference->setFocus();

	if (tagPassage.relIndex().isSet()) {
		setPassage(tagPassage);
		// setPassage will already call CalcPassage
	} else {
		CalcPassage();
	}

	// If the caller told us to not highlight any words, we will have not done
	//		so above on the setPassage painting, but we'll set it to one word
	//		here so that as the user starts selecting things, his word will
	//		highlighted appear:
	if (m_tagPassage.count() == 0) m_tagPassage.count() = 1;

	emit modeChanged(false);

	end_update();
}

bool CKJVPassageNavigator::isReversed() const
{
	return ui.chkboxReverse->isChecked();
}

// ============================================================================

