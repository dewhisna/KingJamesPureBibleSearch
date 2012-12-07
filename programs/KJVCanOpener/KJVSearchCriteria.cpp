#include "KJVSearchCriteria.h"
#include "ui_KJVSearchCriteria.h"

CKJVSearchCriteria::CKJVSearchCriteria(QWidget *parent) :
	QWidget(parent),
	m_nSearchScope(SSME_WHOLE_BIBLE),
	m_nOperator(OME_OR),
	m_bDoingUpdate(false),
	ui(new Ui::CKJVSearchCriteria)
{
	ui->setupUi(this);

	ui->radioButtonANDSearch->setToolTip("Require ALL Words/Phrases to Match");
	ui->radioButtonANDSearch->setStatusTip("Require that ALL of the listed Words/Phrases in the Search Criteria Match");
	ui->radioButtonORSearch->setToolTip("Require ANY Word/Phrases to Match");
	ui->radioButtonORSearch->setStatusTip("Require that ANY of the listed Words/Phrases in the Search Criteria Match");

	ui->buttonAdd->setToolTip("Add Phrase to Search Criteria");
	ui->buttonAdd->setStatusTip("Add another Phrase to the current Search Criteria");

	ui->comboSearchScope->addItem("Entire Bible", SSME_WHOLE_BIBLE);
	ui->comboSearchScope->addItem("Same Testament", SSME_TESTAMENT);
	ui->comboSearchScope->addItem("Same Book", SSME_BOOK);
	ui->comboSearchScope->addItem("Same Chapter", SSME_CHAPTER);
	ui->comboSearchScope->addItem("Same Verse", SSME_VERSE);
	ui->comboSearchScope->setToolTip("Select Search Scope");
	ui->comboSearchScope->setStatusTip("Set Search Scope for AND-mode phrase searches");

	connect(ui->radioButtonANDSearch, SIGNAL(toggled(bool)), this, SLOT(on_changeOperator()));
	connect(ui->radioButtonORSearch, SIGNAL(toggled(bool)), this, SLOT(on_changeOperator()));
	connect(ui->comboSearchScope, SIGNAL(currentIndexChanged(int)), this, SLOT(on_changeSearchScope(int)));

	// Set Initial Mode:
	ui->comboSearchScope->setCurrentIndex(ui->comboSearchScope->findData(m_nSearchScope));
	switch (m_nOperator) {
		case OME_OR:
			ui->radioButtonORSearch->setChecked(true);
			break;
		case OME_AND:
			ui->radioButtonANDSearch->setChecked(true);
			break;
	}
}

CKJVSearchCriteria::~CKJVSearchCriteria()
{
	delete ui;
}


void CKJVSearchCriteria::on_changeSearchScope(int ndx)
{
	if (m_bDoingUpdate) return;

	begin_update();

	if (ndx == -1) return;
	m_nSearchScope = static_cast<SEARCH_SCOPE_MODE_ENUM>(ui->comboSearchScope->itemData(ndx).toInt());
	emit changedSearchScope(m_nSearchScope);

	end_update();
}

void CKJVSearchCriteria::on_changeOperator()
{
	if (ui->radioButtonORSearch->isChecked()) {
		m_nOperator = OME_OR;
		emit changedOperator(m_nOperator);
		ui->comboSearchScope->setCurrentIndex(ui->comboSearchScope->findData(SSME_WHOLE_BIBLE));
		ui->comboSearchScope->setEnabled(false);
	} else if (ui->radioButtonANDSearch->isChecked()) {
		m_nOperator = OME_AND;
		emit changedOperator(m_nOperator);
		ui->comboSearchScope->setEnabled(true);
	}
}

