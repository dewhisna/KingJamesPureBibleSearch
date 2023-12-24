/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "PassageNavigator.h"

#include "dbstruct.h"
#include "PhraseNavigatorEdit.h"
#include "Highlighter.h"
#include "BusyCursor.h"
#include "Translator.h"

#include <QTextCursor>

// ============================================================================

// Placeholder Constructor:

CPassageNavigator::CPassageNavigator(CBibleDatabasePtr pBibleDatabase, QWidget *parent, NavigatorRefTypeOptionFlags flagsRefTypes, NAVIGATOR_REF_TYPE_ENUM nRefType)
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
	Q_ASSERT(!m_pBibleDatabase.isNull());

	ui.setupUi(this);

	initialize();

	Q_ASSERT(m_pEditVersePreview != nullptr);

	QAction *pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
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
	connect(ui.widgetPassageReference, SIGNAL(passageReferenceChanged(TPhraseTag)), this, SLOT(en_PassageReferenceChanged(TPhraseTag)));
	connect(m_pEditVersePreview, SIGNAL(gotoIndex(TPhraseTag)), this, SIGNAL(gotoIndex(TPhraseTag)));
}

CPassageNavigator::~CPassageNavigator()
{

}

void CPassageNavigator::initialize()
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
	ui.editVersePreview = nullptr;
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
			TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(toQtLanguageName(m_pBibleDatabase->langID()));
			if (!pTranslator.isNull()) {
				QString strTemp = pTranslator->translatorApp().translate("CPassageNavigator", "Entire Bible", "Scope");
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
	Q_ASSERT(nTypeIndex != -1);
	ui.comboRefType->setCurrentIndex(nTypeIndex);

	ui.comboBookDirect->clear();
	for (unsigned int ndxBk=1; ndxBk<=m_pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxBk);
		Q_ASSERT(pBook != nullptr);
		if (pBook->m_nNumWrd == 0) continue;		// Skip books that are empty (partial database support)
		ui.comboBookDirect->addItem(pBook->m_strBkName, ndxBk);
	}

	end_update();

	startAbsoluteMode();
	reset();
}

void CPassageNavigator::reset()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	if (isAbsolute()) {
		setPassage(TPhraseTag(CRelIndex(1, 1, 1, 1), m_tagPassage.count()));		// Default to Genesis 1:1 [1]
	} else {
		setPassage(TPhraseTag(CRelIndex(), m_tagPassage.count()));
	}

	ui.widgetPassageReference->clear();
}

// ============================================================================

void CPassageNavigator::en_TestamentComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nTestament = ui.comboTestament->itemData(index).toUInt();
	CalcPassage();
}

void CPassageNavigator::en_BookChanged(int nBook)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nBook = nBook;
	CalcPassage();
}

void CPassageNavigator::en_ChapterChanged(int nChapter)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nChapter = nChapter;
	CalcPassage();
}

void CPassageNavigator::en_VerseChanged(int nVerse)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nVerse = nVerse;
	CalcPassage();
}

void CPassageNavigator::en_WordChanged(int nWord)
{
	if (m_bDoingUpdate) return;

	ui.widgetPassageReference->clear();

	m_nWord = nWord;
	CalcPassage();
}

void CPassageNavigator::en_ReverseChanged(bool bReverse)
{
	ui.spinBook->setPrefix(bReverse ? "-" : "");
	ui.spinChapter->setPrefix(bReverse ? "-" : "");
	ui.spinVerse->setPrefix(bReverse ? "-" : "");
	ui.spinWord->setPrefix(bReverse ? "-" : "");

	if (m_bDoingUpdate) return;

	CalcPassage();
}

void CPassageNavigator::en_RefTypeChanged(int nType)
{
	if (m_bDoingUpdate) return;

	m_nRefType = static_cast<NAVIGATOR_REF_TYPE_ENUM>(ui.comboRefType->itemData(nType).toInt());
	CalcPassage();
}

// ============================================================================

void CPassageNavigator::en_BookDirectChanged(int index)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

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

void CPassageNavigator::en_ChapterDirectChanged(int index)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

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

void CPassageNavigator::en_VerseDirectChanged(int index)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

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

void CPassageNavigator::en_WordDirectChanged(int index)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

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

void CPassageNavigator::setDirectReference(const CRelIndex &ndx)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

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
		Q_ASSERT(false);
		return;
	}

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		Q_ASSERT(false);
		return;
	}

	begin_update();

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	ui.comboBookDirect->setCurrentIndex(ui.comboBookDirect->findData(ndx.book()));

	ui.comboChapterDirect->clear();
	for (unsigned int ndxChp=1; ndxChp<=book.m_nNumChp; ++ndxChp) {
		const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(CRelIndex(ndx.book(), ndxChp, 0, 0));
		if (pChapter == nullptr) continue;
		if (pChapter->m_nNumWrd == 0) continue;			// Skip chapters that are empty (partial database support)
		ui.comboChapterDirect->addItem(QString("%1").arg(ndxChp), ndxChp);
	}
	ui.comboChapterDirect->setCurrentIndex(ui.comboChapterDirect->findData(ndx.chapter()));

	if (ndx.chapter() > book.m_nNumChp) {
		Q_ASSERT(false);
		end_update();
		return;
	}

	const CChapterEntry &chapter = *m_pBibleDatabase->chapterEntry(ndx);

	ui.comboVerseDirect->clear();
	for (unsigned int ndxVrs=1; ndxVrs<=chapter.m_nNumVrs; ++ndxVrs) {
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(CRelIndex(ndx.book(), ndx.chapter(), ndxVrs, 0));
		if (pVerse == nullptr) continue;
		if (pVerse->m_nNumWrd == 0) continue;		// Skip verses that are empty (partial database support)
		ui.comboVerseDirect->addItem(QString("%1").arg(ndxVrs), ndxVrs);
	}
	ui.comboVerseDirect->setCurrentIndex(ui.comboVerseDirect->findData(ndx.verse()));

	if (ndx.verse() > chapter.m_nNumVrs) {
		Q_ASSERT(false);
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

void CPassageNavigator::en_PassageReferenceChanged(const TPhraseTag &tagPhrase)
{
	if (tagPhrase.isSet()) setPassage(tagPhrase);
}

// ============================================================================

TPhraseTag CPassageNavigator::passage() const
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
			Q_ASSERT(false);
			break;
	}

	return tagPassage;
}

void CPassageNavigator::setPassage(const TPhraseTag &tag)
{
	begin_update();

	TPhraseTag tagUpdated = tag;
	// If we are passed a tag that has a reference to a word of a colophon or superscription,
	//		remove the word selection so that we are referencing just the book or chapter itself:
	if ((tag.relIndex().chapter() == 0) || (tag.relIndex().word() == 0)) {
		tagUpdated = TPhraseTag(CRelIndex(tag.relIndex().book(), tag.relIndex().chapter(), tag.relIndex().verse(), 0), tag.count());
	}

	m_tagPassage = tagUpdated;

	ui.comboTestament->setCurrentIndex(ui.comboTestament->findData(0));
	m_nTestament = 0;
	ui.spinBook->setValue(tagUpdated.relIndex().book());
	m_nBook = tagUpdated.relIndex().book();
	ui.spinChapter->setValue(tagUpdated.relIndex().chapter());
	m_nChapter = tagUpdated.relIndex().chapter();
	ui.spinVerse->setValue(tagUpdated.relIndex().verse());
	m_nVerse = tagUpdated.relIndex().verse();
	ui.spinWord->setValue(tagUpdated.relIndex().word());
	m_nWord = tagUpdated.relIndex().word();
	CalcPassage();

	end_update();
}

void CPassageNavigator::CalcPassage()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	m_tagPassage.relIndex() = m_pBibleDatabase->calcRelIndex(m_nWord, m_nVerse, m_nChapter, m_nBook, (!m_tagStartRef.relIndex().isSet() ? m_nTestament : 0), m_tagStartRef.relIndex(), (!m_tagStartRef.relIndex().isSet() ? false : ui.chkboxReverse->isChecked()));
	ui.editResolved->setText(m_pBibleDatabase->PassageReferenceText(passage().relIndex()));
	CPhraseNavigatorEdit navigator(m_pBibleDatabase, *m_pEditVersePreview);

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
			navigator.setDocumentToVerse(ndxWord, TPhraseTagList(), defaultGenerateVerseTextFlags | TRO_ScriptureBrowser);
			navigator.doHighlighting(CSearchResultHighlighter(m_tagPassage));
			break;
		case NRTE_VERSE:
			navigator.setDocumentToVerse(ndxVerse, TPhraseTagList(), defaultGenerateVerseTextFlags | TRO_ScriptureBrowser);
			break;
		case NRTE_CHAPTER:
			navigator.setDocumentToChapter(ndxChapter, TRO_Colophons | TRO_Superscriptions | TRO_Subtitles | TRO_Category | TRO_SuppressPrePostChapters | TRO_ScriptureBrowser);
			txtCursor = m_pEditVersePreview->textCursor();
			txtCursor.movePosition(QTextCursor::Start);
			m_pEditVersePreview->setTextCursor(txtCursor);
			break;
		case NRTE_BOOK:
			navigator.setDocumentToBookInfo(ndxBook, defaultGenerateBookInfoTextFlags | TRO_ScriptureBrowser);
			break;
	}

}

void CPassageNavigator::setRefType(NAVIGATOR_REF_TYPE_ENUM nRefType)
{
	begin_update();

	int nTypeIndex = ui.comboRefType->findData(static_cast<int>(nRefType));
	Q_ASSERT(nTypeIndex != -1);
	if (nTypeIndex != -1) {
		m_nRefType = nRefType;
		ui.comboRefType->setCurrentIndex(nTypeIndex);
		CalcPassage();
	}

	end_update();
}

void CPassageNavigator::startRelativeMode(TPhraseTag tagStart, TPhraseTag tagPassage)
{
	startRelativeMode(tagStart, isReversed(), tagPassage);
}

void CPassageNavigator::startRelativeMode(TPhraseTag tagStart, bool bReverse, TPhraseTag tagPassage)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

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

	ui.lblBook->setText(tr("&Books:", "CPassageNavigator"));
	ui.lblChapter->setText(tr("C&hapters:", "CPassageNavigator"));
	ui.lblVerse->setText(tr("&Verses:", "CPassageNavigator"));
	ui.lblWord->setText(tr("&Words:", "CPassageNavigator"));

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

void CPassageNavigator::startAbsoluteMode(TPhraseTag tagPassage)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	begin_update();

	m_tagStartRef = TPhraseTag(CRelIndex(), 1);		// Unset (but one word) to indicate absolute mode

	ui.lblStartRef->hide();
	ui.editStartRef->hide();
	ui.chkboxReverse->setChecked(false);
	ui.chkboxReverse->hide();

	ui.lblTestament->show();
	ui.comboTestament->show();

	ui.lblBook->setText(tr("&Book:", "CPassageNavigator"));
	ui.lblChapter->setText(tr("C&hapter:", "CPassageNavigator"));
	ui.lblVerse->setText(tr("&Verse:", "CPassageNavigator"));
	ui.lblWord->setText(tr("&Word:", "CPassageNavigator"));

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

bool CPassageNavigator::isReversed() const
{
	return ui.chkboxReverse->isChecked();
}

// ============================================================================

