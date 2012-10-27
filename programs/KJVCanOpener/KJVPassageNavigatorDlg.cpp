#include "KJVPassageNavigatorDlg.h"
#include "ui_KJVPassageNavigatorDlg.h"

CKJVPassageNavigatorDlg::CKJVPassageNavigatorDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CKJVPassageNavigatorDlg)
{
	ui->setupUi(this);
}

CKJVPassageNavigatorDlg::~CKJVPassageNavigatorDlg()
{
	delete ui;
}

CRelIndex CKJVPassageNavigatorDlg::passage() const
{
	return ui->widgetKJVPassageNavigator->passage();
}

void CKJVPassageNavigatorDlg::setPassage(const CRelIndex &ndx)
{
	ui->widgetKJVPassageNavigator->setPassage(ndx);
}

