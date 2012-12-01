#include "KJVPassageNavigator.h"
#include "ui_KJVPassageNavigator.h"

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "Highlighter.h"

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

	setPassage(TPhraseTag());

	startAbsoluteMode();

	connect(ui->comboTestament, SIGNAL(currentIndexChanged(int)), this, SLOT(TestamentComboIndexChanged(int)));
	connect(ui->editWord, SIGNAL(textEdited(const QString &)), this, SLOT(WordChanged(const QString &)));
	connect(ui->editVerse, SIGNAL(textEdited(const QString &)), this, SLOT(VerseChanged(const QString &)));
	connect(ui->editChapter, SIGNAL(textEdited(const QString &)), this, SLOT(ChapterChanged(const QString &)));
	connect(ui->editBook, SIGNAL(textEdited(const QString &)), this, SLOT(BookChanged(const QString &)));
	connect(ui->chkboxReverse, SIGNAL(clicked(bool)), this, SLOT(on_ReverseChanged(bool)));
	connect(ui->editVersePreview, SIGNAL(gotoIndex(const TPhraseTag &)), this, SIGNAL(gotoIndex(const TPhraseTag &)));
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

void CKJVPassageNavigator::setPassage(const TPhraseTag &tag)
{
	begin_update();

	m_tagPassage = tag;

	ui->comboTestament->setCurrentIndex(ui->comboTestament->findData(0));
	m_nTestament = 0;
	ui->editBook->setText(QString("%1").arg(tag.first.book()));
	m_nBook = tag.first.book();
	ui->editChapter->setText(QString("%1").arg(tag.first.chapter()));
	m_nChapter = tag.first.chapter();
	ui->editVerse->setText(QString("%1").arg(tag.first.verse()));
	m_nVerse = tag.first.verse();
	ui->editWord->setText(QString("%1").arg(tag.first.word()));
	m_nWord = tag.first.word();
	CalcPassage();

	end_update();
}

void CKJVPassageNavigator::CalcPassage()
{
	m_tagPassage.first = CRefCountCalc::calcRelIndex(m_nWord, m_nVerse, m_nChapter, m_nBook, (!m_tagStartRef.first.isSet() ? m_nTestament : 0), m_tagStartRef.first, (!m_tagStartRef.first.isSet() ? false : ui->chkboxReverse->isChecked()));
	ui->editResolved->setText(m_tagPassage.first.PassageReferenceText());
	CPhraseEditNavigator navigator(*ui->editVersePreview);
//	TPhraseTagList tags;
//	tags.push_back(TPhraseTag(m_ndxPassage, 1));
	navigator.setDocumentToVerse(m_tagPassage.first);
	navigator.doHighlighting(CSearchResultHighlighter(m_tagPassage));
}

void CKJVPassageNavigator::startRelativeMode(TPhraseTag tagStart, TPhraseTag tagPassage)
{
	startRelativeMode(tagStart, isReversed(), tagPassage);
}

void CKJVPassageNavigator::startRelativeMode(TPhraseTag tagStart, bool bReverse, TPhraseTag tagPassage)
{
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

	ui->editStartRef->setText(m_tagStartRef.first.PassageReferenceText());
	ui->chkboxReverse->setChecked(bReverse);

	ui->lblBook->setText("&Books:");
	ui->lblChapter->setText("&Chapters:");
	ui->lblVerse->setText("&Verses:");
	ui->lblWord->setText("&Words:");

	if (tagPassage.first.isSet()) setPassage(tagPassage);

	emit modeChanged(true);

	CalcPassage();

	end_update();
}

void CKJVPassageNavigator::startAbsoluteMode(TPhraseTag tagPassage)
{
	begin_update();

	m_tagStartRef = TPhraseTag();		// Unset to indicate absolute mode

	ui->lblStartRef->hide();
	ui->editStartRef->hide();
	ui->chkboxReverse->hide();

	ui->lblTestament->show();
	ui->comboTestament->show();

	ui->lblBook->setText("&Book:");
	ui->lblChapter->setText("&Chapter:");
	ui->lblVerse->setText("&Verse:");
	ui->lblWord->setText("&Word:");

	if (tagPassage.first.isSet()) setPassage(tagPassage);

	emit modeChanged(false);

	CalcPassage();

	end_update();
}

bool CKJVPassageNavigator::isReversed() const
{
	return ui->chkboxReverse->isChecked();
}

