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

#include <assert.h>

// ============================================================================

// Placeholder Constructor:

CKJVPassageNavigator::CKJVPassageNavigator(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QWidget(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_nTestament(0),
		m_nBook(0),
		m_nChapter(0),
		m_nVerse(0),
		m_nWord(0),
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
	connect(ui->chkboxReverse, SIGNAL(clicked(bool)), this, SLOT(on_ReverseChanged(bool)));
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

	startAbsoluteMode();
	reset();
}

void CKJVPassageNavigator::reset()
{
	assert(m_pBibleDatabase.data() != NULL);

	if (isAbsolute()) {
		setPassage(TPhraseTag(CRelIndex(1, 1, 1, 1), m_tagPassage.second));		// Default to Genesis 1:1 [1]
	} else {
		setPassage(TPhraseTag(CRelIndex(), m_tagPassage.second));
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

void CKJVPassageNavigator::on_ReverseChanged(bool bReverse)
{
	ui->spinBook->setPrefix(bReverse ? "-" : "");
	ui->spinChapter->setPrefix(bReverse ? "-" : "");
	ui->spinVerse->setPrefix(bReverse ? "-" : "");
	ui->spinWord->setPrefix(bReverse ? "-" : "");

	if (m_bDoingUpdate) return;

	CalcPassage();
}

void CKJVPassageNavigator::setPassage(const TPhraseTag &tag)
{
	begin_update();

	m_tagPassage = tag;

	ui->comboTestament->setCurrentIndex(ui->comboTestament->findData(0));
	m_nTestament = 0;
	ui->spinBook->setValue(tag.first.book());
	m_nBook = tag.first.book();
	ui->spinChapter->setValue(tag.first.chapter());
	m_nChapter = tag.first.chapter();
	ui->spinVerse->setValue(tag.first.verse());
	m_nVerse = tag.first.verse();
	ui->spinWord->setValue(tag.first.word());
	m_nWord = tag.first.word();
	CalcPassage();

	end_update();
}

void CKJVPassageNavigator::CalcPassage()
{
	assert(m_pBibleDatabase.data() != NULL);

	m_tagPassage.first = m_pBibleDatabase->calcRelIndex(m_nWord, m_nVerse, m_nChapter, m_nBook, (!m_tagStartRef.first.isSet() ? m_nTestament : 0), m_tagStartRef.first, (!m_tagStartRef.first.isSet() ? false : ui->chkboxReverse->isChecked()));
	ui->editResolved->setText(m_pBibleDatabase->PassageReferenceText(m_tagPassage.first));
	CPhraseEditNavigator navigator(m_pBibleDatabase, *m_pEditVersePreview);
	navigator.setDocumentToVerse(m_tagPassage.first);
	navigator.doHighlighting(CSearchResultHighlighter(m_tagPassage));
}

void CKJVPassageNavigator::startRelativeMode(TPhraseTag tagStart, TPhraseTag tagPassage)
{
	startRelativeMode(tagStart, isReversed(), tagPassage);
}

void CKJVPassageNavigator::startRelativeMode(TPhraseTag tagStart, bool bReverse, TPhraseTag tagPassage)
{
	assert(m_pBibleDatabase.data() != NULL);

	begin_update();

	if (tagStart.first.isSet()) {
		m_tagStartRef = tagStart;
	} else {
		m_tagStartRef = TPhraseTag(CRelIndex(1,1,1,1), 1);
	}

	ui->lblTestament->hide();
	ui->comboTestament->hide();

	ui->lblStartRef->show();
	ui->editStartRef->show();
	ui->chkboxReverse->show();

	ui->editStartRef->setText(m_pBibleDatabase->PassageReferenceText(m_tagStartRef.first));
	ui->chkboxReverse->setChecked(bReverse);

	ui->lblBook->setText(tr("&Books:"));
	ui->lblChapter->setText(tr("C&hapters:"));
	ui->lblVerse->setText(tr("&Verses:"));
	ui->lblWord->setText(tr("&Words:"));

	if (!tagPassage.first.isSet()) {
		// If we don't have an absolute starting passage, set the passage size (that we'll calculate from
		//		our zero-relative) to be the size of the starting reference passage:
		tagPassage.second = tagStart.second;
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

	if (tagPassage.first.isSet()) {
		setPassage(tagPassage);
		// setPassage will already call CalcPassage
	} else {
		CalcPassage();
	}

	// If the caller told us to not highlight any words, we will have not done
	//		so above on the setPassage painting, but we'll set it to one word
	//		here so that as the user starts selecting things, his word will
	//		highlighted appear:
	if (m_tagPassage.second == 0) m_tagPassage.second = 1;

	emit modeChanged(false);

	end_update();
}

bool CKJVPassageNavigator::isReversed() const
{
	return ui->chkboxReverse->isChecked();
}

