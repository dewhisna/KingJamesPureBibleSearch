#include "KJVPassageNavigator.h"
#include "ui_KJVPassageNavigator.h"

#include "dbstruct.h"

CKJVPassageNavigator::CKJVPassageNavigator(QWidget *parent)
	:	QWidget(parent),
		m_nTestament(0),
		m_nBook(0),
		m_nChapter(0),
		m_nVerse(0),
		m_nWord(0),
		m_bDoingUpdate(false),
		ui(new Ui::CKJVPassageNavigator)
{
	ui->setupUi(this);

	ui->comboTestament->clear();
	for (unsigned int ndx=0; ndx<=g_lstTestaments.size(); ++ndx){
		if (ndx == 0) {
			ui->comboTestament->addItem("Entire Bible", ndx);
		} else {
			ui->comboTestament->addItem(g_lstTestaments[ndx-1].m_strTstName, ndx);
		}
	}

//	setPassage(CRelIndex(1, 1, 1, 1));
	setPassage(CRelIndex());

	connect(ui->editWord, SIGNAL(textEdited(const QString &)), this, SLOT(WordChanged(const QString &)));
	connect(ui->editVerse, SIGNAL(textEdited(const QString &)), this, SLOT(VerseChanged(const QString &)));
	connect(ui->editChapter, SIGNAL(textEdited(const QString &)), this, SLOT(ChapterChanged(const QString &)));
	connect(ui->editBook, SIGNAL(textEdited(const QString &)), this, SLOT(BookChanged(const QString &)));
}

CKJVPassageNavigator::~CKJVPassageNavigator()
{
	delete ui;
}

void CKJVPassageNavigator::TestamentComboIndexChanged(int index)
{
	if (m_bDoingUpdate) return;

	m_nTestament = ui->comboTestament->itemData(index).toUInt();
	CalcPassage();
}

void CKJVPassageNavigator::BookChanged(const QString &strBook)
{
	if (m_bDoingUpdate) return;

	m_nBook = strBook.toUInt();
	CalcPassage();
}

void CKJVPassageNavigator::ChapterChanged(const QString &strChapter)
{
	if (m_bDoingUpdate) return;

	m_nChapter = strChapter.toUInt();
	CalcPassage();
}

void CKJVPassageNavigator::VerseChanged(const QString &strVerse)
{
	if (m_bDoingUpdate) return;

	m_nVerse = strVerse.toUInt();
	CalcPassage();
}

void CKJVPassageNavigator::WordChanged(const QString &strWord)
{
	if (m_bDoingUpdate) return;

	m_nWord = strWord.toUInt();
	CalcPassage();
}

void CKJVPassageNavigator::setPassage(const CRelIndex &ndx)
{
	begin_update();

	m_ndxPassage = ndx;

	ui->comboTestament->setCurrentIndex(ui->comboTestament->findData(0));
	m_nTestament = 0;
	ui->editBook->setText(QString("%1").arg(ndx.book()));
	m_nBook = ndx.book();
	ui->editChapter->setText(QString("%1").arg(ndx.chapter()));
	m_nChapter = ndx.chapter();
	ui->editVerse->setText(QString("%1").arg(ndx.verse()));
	m_nVerse = ndx.verse();
	ui->editWord->setText(QString("%1").arg(ndx.word()));
	m_nWord = ndx.word();
	ui->editResolved->setText(ndx.PassageReferenceText());

	end_update();
}

void CKJVPassageNavigator::CalcPassage()
{
	m_ndxPassage = CRefCountCalc::calcRelIndex(m_nWord, m_nVerse, m_nChapter, m_nBook, m_nTestament);
	ui->editResolved->setText(m_ndxPassage.PassageReferenceText());
}

