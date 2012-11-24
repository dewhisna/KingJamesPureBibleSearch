#include "KJVSearchCriteria.h"
#include "ui_KJVSearchCriteria.h"

CKJVSearchCriteria::CKJVSearchCriteria(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CKJVSearchCriteria)
{
	ui->setupUi(this);

	ui->radioButtonANDSearch->setToolTip("Require ALL Words/Phrases to Match");
	ui->radioButtonANDSearch->setStatusTip("Require that ALL of the listed Words/Phrases in the Search Criteria Match");
	ui->radioButtonORSearch->setToolTip("Require ANY Word/Phrases to Match");
	ui->radioButtonORSearch->setStatusTip("Require that ANY of the listed Words/Phrases in the Search Criteria Match");

	ui->buttonAdd->setToolTip("Add Phrase to Search Criteria");
	ui->buttonAdd->setStatusTip("Add another Phrase to the current Search Criteria");
}

CKJVSearchCriteria::~CKJVSearchCriteria()
{
	delete ui;
}
