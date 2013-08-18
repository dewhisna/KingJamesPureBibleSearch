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
#include "ui_KJVPassageNavigator.h"

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "Highlighter.h"
#include "BusyCursor.h"

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
		m_bDoingUpdate(false),
		ui(new Ui::CKJVPassageNavigator)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui->setupUi(this);

	initialize();

	assert(m_pEditVersePreview != NULL);

	QAction *pAction = new QAction(this);
	pAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
	addAction(pAction);
	connect(pAction, SIGNAL(triggered()), m_pEditVersePreview, SLOT(showDetails()));

	connect(ui->comboTestament, SIGNAL(currentIndexChanged(int)), this, SLOT(TestamentComboIndexChanged(int)));
	connect(ui->spinWord, SIGNAL(valueChanged(int)), this, SLOT(WordChanged(int)));
	connect(ui->spinVerse, SIGNAL(valueChanged(int)), this, SLOT(VerseChanged(int)));
	connect(ui->spinChapter, SIGNAL(valueChanged(int)), this, SLOT(ChapterChanged(int)));
	connect(ui->spinBook, SIGNAL(valueChanged(int)), this, SLOT(BookChanged(int)));
	connect(ui->chkboxReverse, SIGNAL(clicked(bool)), this, SLOT(en_ReverseChanged(bool)));
	connect(ui->comboRefType, SIGNAL(currentIndexChanged(int)), this, SLOT(en_RefTypeChanged(int)));
	connect(m_pEditVersePreview, SIGNAL(gotoIndex(const TPhraseTag &)), this, SIGNAL(gotoIndex(const TPhraseTag &)));
}

CKJVPassageNavigator::~CKJVPassageNavigator()
{
	delete ui;
}

void CKJVPassageNavigator::initialize()
{
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

	delete ui->editVersePreview;
	ui->editVersePreview = NULL;
	ui->verticalLayout->addWidget(m_pEditVersePreview);

	// --------------------------------------------------------------

	m_tagStartRef = TPhraseTag(CRelIndex(), 1);		// Start with default word-size of one so we highlight at least one word when tracking
	m_tagPassage = TPhraseTag(CRelIndex(), 1);		// ""  (ditto)

	int nBooks = 0;
	int nChapters = 0;
	int nVerses = 0;
	int nWords = 0;

	ui->comboTestament->clear();
	for (unsigned int ndx=0; ndx<=m_pBibleDatabase->bibleEntry().m_nNumTst; ++ndx){
		if (ndx == 0) {
			ui->comboTestament->addItem(tr("Entire Bible"), ndx);
		} else {
			ui->comboTestament->addItem(m_pBibleDatabase->testamentEntry(ndx)->m_strTstName, ndx);
			nBooks += m_pBibleDatabase->testamentEntry(ndx)->m_nNumBk;
			nChapters += m_pBibleDatabase->testamentEntry(ndx)->m_nNumChp;
			nVerses += m_pBibleDatabase->testamentEntry(ndx)->m_nNumVrs;
			nWords += m_pBibleDatabase->testamentEntry(ndx)->m_nNumWrd;
		}
	}

	ui->spinBook->setRange(0, nBooks);
	ui->spinChapter->setRange(0, nChapters);
	ui->spinVerse->setRange(0, nVerses);
	ui->spinWord->setRange(0, nWords);

	bool bAllTypes = (m_flagsRefTypes == NRTO_Default);
	ui->comboRefType->clear();
	if ((m_flagsRefTypes & NRTO_Word) || (bAllTypes)) ui->comboRefType->addItem(tr("Word"), static_cast<int>(NRTE_WORD));
	if ((m_flagsRefTypes & NRTO_Verse) || (bAllTypes)) ui->comboRefType->addItem(tr("Verse"), static_cast<int>(NRTE_VERSE));
	if ((m_flagsRefTypes & NRTO_Chapter) || (bAllTypes)) ui->comboRefType->addItem(tr("Chapter"), static_cast<int>(NRTE_CHAPTER));
	if ((m_flagsRefTypes & NRTO_Book) || (bAllTypes)) ui->comboRefType->addItem(tr("Book"), static_cast<int>(NRTE_BOOK));
	int nTypeIndex = ui->comboRefType->findData(static_cast<int>(m_nRefType));
	assert(nTypeIndex != -1);
	ui->comboRefType->setCurrentIndex(nTypeIndex);

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
}

void CKJVPassageNavigator::TestamentComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	m_nTestament = ui->comboTestament->itemData(index).toUInt();
	CalcPassage();
}

void CKJVPassageNavigator::BookChanged(int nBook)
{
	if (m_bDoingUpdate) return;

	m_nBook = nBook;
	CalcPassage();
}

void CKJVPassageNavigator::ChapterChanged(int nChapter)
{
	if (m_bDoingUpdate) return;

	m_nChapter = nChapter;
	CalcPassage();
}

void CKJVPassageNavigator::VerseChanged(int nVerse)
{
	if (m_bDoingUpdate) return;

	m_nVerse = nVerse;
	CalcPassage();
}

void CKJVPassageNavigator::WordChanged(int nWord)
{
	if (m_bDoingUpdate) return;

	m_nWord = nWord;
	CalcPassage();
}

void CKJVPassageNavigator::en_ReverseChanged(bool bReverse)
{
	ui->spinBook->setPrefix(bReverse ? "-" : "");
	ui->spinChapter->setPrefix(bReverse ? "-" : "");
	ui->spinVerse->setPrefix(bReverse ? "-" : "");
	ui->spinWord->setPrefix(bReverse ? "-" : "");

	if (m_bDoingUpdate) return;

	CalcPassage();
}

void CKJVPassageNavigator::en_RefTypeChanged(int nType)
{
	if (m_bDoingUpdate) return;

	m_nRefType = static_cast<NAVIGATOR_REF_TYPE_ENUM>(ui->comboRefType->itemData(nType).toInt());
	CalcPassage();
}

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

	ui->comboTestament->setCurrentIndex(ui->comboTestament->findData(0));
	m_nTestament = 0;
	ui->spinBook->setValue(tag.relIndex().book());
	m_nBook = tag.relIndex().book();
	ui->spinChapter->setValue(tag.relIndex().chapter());
	m_nChapter = tag.relIndex().chapter();
	ui->spinVerse->setValue(tag.relIndex().verse());
	m_nVerse = tag.relIndex().verse();
	ui->spinWord->setValue(tag.relIndex().word());
	m_nWord = tag.relIndex().word();
	CalcPassage();

	end_update();
}

void CKJVPassageNavigator::CalcPassage()
{
	assert(m_pBibleDatabase.data() != NULL);

	m_tagPassage.relIndex() = m_pBibleDatabase->calcRelIndex(m_nWord, m_nVerse, m_nChapter, m_nBook, (!m_tagStartRef.relIndex().isSet() ? m_nTestament : 0), m_tagStartRef.relIndex(), (!m_tagStartRef.relIndex().isSet() ? false : ui->chkboxReverse->isChecked()));
	ui->editResolved->setText(m_pBibleDatabase->PassageReferenceText(m_tagPassage.relIndex()));
	CPhraseEditNavigator navigator(m_pBibleDatabase, *m_pEditVersePreview);

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

	int nTypeIndex = ui->comboRefType->findData(static_cast<int>(nRefType));
	assert(nTypeIndex != -1);
	if (nTypeIndex != -1) {
		m_nRefType = nRefType;
		ui->comboRefType->setCurrentIndex(nTypeIndex);
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

	ui->lblTestament->hide();
	ui->comboTestament->hide();

	ui->lblStartRef->show();
	ui->editStartRef->show();
	ui->chkboxReverse->show();

	ui->editStartRef->setText(m_pBibleDatabase->PassageReferenceText(m_tagStartRef.relIndex()));
	ui->chkboxReverse->setChecked(bReverse);

	ui->lblBook->setText(tr("&Books:"));
	ui->lblChapter->setText(tr("C&hapters:"));
	ui->lblVerse->setText(tr("&Verses:"));
	ui->lblWord->setText(tr("&Words:"));

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

	ui->lblStartRef->hide();
	ui->editStartRef->hide();
	ui->chkboxReverse->setChecked(false);
	ui->chkboxReverse->hide();

	ui->lblTestament->show();
	ui->comboTestament->show();

	ui->lblBook->setText(tr("&Book:"));
	ui->lblChapter->setText(tr("C&hapter:"));
	ui->lblVerse->setText(tr("&Verse:"));
	ui->lblWord->setText(tr("&Word:"));

	ui->spinBook->setPrefix("");
	ui->spinChapter->setPrefix("");
	ui->spinVerse->setPrefix("");
	ui->spinWord->setPrefix("");

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
	return ui->chkboxReverse->isChecked();
}

