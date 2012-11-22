#include "KJVPassageNavigator.h"
#include "ui_KJVPassageNavigator.h"

#include "dbstruct.h"
#include "PhraseEdit.h"

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

	setPassage(CRelIndex());

	startAbsoluteMode();

	connect(ui->comboTestament, SIGNAL(currentIndexChanged(int)), this, SLOT(TestamentComboIndexChanged(int)));
	connect(ui->editWord, SIGNAL(textEdited(const QString &)), this, SLOT(WordChanged(const QString &)));
	connect(ui->editVerse, SIGNAL(textEdited(const QString &)), this, SLOT(VerseChanged(const QString &)));
	connect(ui->editChapter, SIGNAL(textEdited(const QString &)), this, SLOT(ChapterChanged(const QString &)));
	connect(ui->editBook, SIGNAL(textEdited(const QString &)), this, SLOT(BookChanged(const QString &)));
	connect(ui->chkboxReverse, SIGNAL(clicked(bool)), this, SLOT(on_ReverseChanged(bool)));
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

void CKJVPassageNavigator::on_ReverseChanged(bool /* bReverse */)
{
	if (m_bDoingUpdate) return;

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
	CalcPassage();

	end_update();
}

void CKJVPassageNavigator::CalcPassage()
{
	m_ndxPassage = CRefCountCalc::calcRelIndex(m_nWord, m_nVerse, m_nChapter, m_nBook, (!m_ndxStartRef.isSet() ? m_nTestament : 0), m_ndxStartRef, (!m_ndxStartRef.isSet() ? false : ui->chkboxReverse->isChecked()));
	ui->editResolved->setText(m_ndxPassage.PassageReferenceText());
	CPhraseNavigator navigator(*ui->editVersePreview);
	TPhraseTagList tags;
	tags.push_back(TPhraseTag(m_ndxPassage, 1));
	navigator.fillEditorWithVerse(m_ndxPassage);
	navigator.doHighlighting(tags, QColor("blue"));			// TODO : Centralize Highlight color setting
}

void CKJVPassageNavigator::startRelativeMode(CRelIndex ndxStart, CRelIndex ndxPassage)
{
	startRelativeMode(ndxStart, isReversed(), ndxPassage);
}

void CKJVPassageNavigator::startRelativeMode(CRelIndex ndxStart, bool bReverse, CRelIndex ndxPassage)
{
	begin_update();

	if (ndxStart.isSet()) {
		m_ndxStartRef = ndxStart;
	} else {
		m_ndxStartRef = CRelIndex(1,1,1,1);
	}

	ui->lblTestament->hide();
	ui->comboTestament->hide();

	ui->lblStartRef->show();
	ui->editStartRef->show();
	ui->chkboxReverse->show();

	ui->editStartRef->setText(m_ndxStartRef.PassageReferenceText());
	ui->chkboxReverse->setChecked(bReverse);

	ui->lblBook->setText("&Books:");
	ui->lblChapter->setText("&Chapters:");
	ui->lblVerse->setText("&Verses:");
	ui->lblWord->setText("&Words:");

	if (ndxPassage.isSet()) setPassage(ndxPassage);

	emit modeChanged(true);

	CalcPassage();

	end_update();
}

void CKJVPassageNavigator::startAbsoluteMode(CRelIndex ndxPassage)
{
	begin_update();

	m_ndxStartRef = CRelIndex();		// Unset to indicate absolute mode

	ui->lblStartRef->hide();
	ui->editStartRef->hide();
	ui->chkboxReverse->hide();

	ui->lblTestament->show();
	ui->comboTestament->show();

	ui->lblBook->setText("&Book:");
	ui->lblChapter->setText("&Chapter:");
	ui->lblVerse->setText("&Verse:");
	ui->lblWord->setText("&Word:");

	if (ndxPassage.isSet()) setPassage(ndxPassage);

	emit modeChanged(false);

	CalcPassage();

	end_update();
}

bool CKJVPassageNavigator::isReversed() const
{
	return ui->chkboxReverse->isChecked();
}

