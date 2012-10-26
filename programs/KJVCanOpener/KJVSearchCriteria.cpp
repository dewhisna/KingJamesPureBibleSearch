#include "KJVSearchCriteria.h"
#include "ui_KJVSearchCriteria.h"

CKJVSearchCriteria::CKJVSearchCriteria(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CKJVSearchCriteria)
{
	ui->setupUi(this);
}

CKJVSearchCriteria::~CKJVSearchCriteria()
{
	delete ui;
}
