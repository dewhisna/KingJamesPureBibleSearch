#include "KJVSearchCriteria.h"
#include "ui_KJVSearchCriteria.h"

CKJVSearchCriteria::CKJVSearchCriteria(QWidget *parent) :
	QWidget(parent),
	m_nSearchScopeMode(SSME_WHOLE_BIBLE),
	m_bDoingUpdate(false),
	ui(new Ui::CKJVSearchCriteria)
{
	ui->setupUi(this);

	ui->buttonAdd->setToolTip("Add Phrase to Search Criteria");
	ui->buttonAdd->setStatusTip("Add another Phrase to the current Search Criteria");

	ui->comboSearchScope->addItem("Entire Bible", SSME_WHOLE_BIBLE);
	ui->comboSearchScope->addItem("Same Testament", SSME_TESTAMENT);
	ui->comboSearchScope->addItem("Same Book", SSME_BOOK);
	ui->comboSearchScope->addItem("Same Chapter", SSME_CHAPTER);
	ui->comboSearchScope->addItem("Same Verse", SSME_VERSE);
	ui->comboSearchScope->setToolTip("Select Search Scope");
	ui->comboSearchScope->setStatusTip("Set Search Scope Mode for phrase searches");

	// Set Initial Mode:
	ui->comboSearchScope->setCurrentIndex(ui->comboSearchScope->findData(m_nSearchScopeMode));

	connect(ui->comboSearchScope, SIGNAL(currentIndexChanged(int)), this, SLOT(on_changeSearchScopeMode(int)));
	connect(ui->buttonAdd, SIGNAL(clicked()), this, SIGNAL(addSearchPhraseClicked()));
	connect(ui->buttonCopySummary, SIGNAL(clicked()), this, SIGNAL(copySearchPhraseSummary()));
}

CKJVSearchCriteria::~CKJVSearchCriteria()
{
	delete ui;
}

void CKJVSearchCriteria::on_changeSearchScopeMode(int ndx)
{
	if (m_bDoingUpdate) return;

	begin_update();

	if (ndx == -1) return;
	m_nSearchScopeMode = static_cast<SEARCH_SCOPE_MODE_ENUM>(ui->comboSearchScope->itemData(ndx).toInt());
	emit changedSearchScopeMode(m_nSearchScopeMode);

	end_update();
}

void CKJVSearchCriteria::enableCopySearchPhraseSummary(bool bEnable)
{
	ui->buttonCopySummary->setEnabled(bEnable);
}

