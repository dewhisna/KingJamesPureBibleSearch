#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include <QtSql>
#include <QMessageBox>

CKJVCanOpener::CKJVCanOpener(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CKJVCanOpener)
{
	ui->setupUi(this);
/*

	QSqlDatabase *pDatabase = new QSqlDatabase();
	*pDatabase = QSqlDatabase::addDatabase("QSQLITE");
	pDatabase->setDatabaseName("../KJVCanOpener/db/kjvtext.s3db");
//    pDatabase->setDatabaseName("C:/MyData/programs/KJVCanOpener/db/kjvtext.s3db");

//    QMessageBox::information(0,"Database",pDatabase->databaseName());

	if (!pDatabase->open()) {
		QMessageBox::warning(0,"Error","Couldn't open database file.");
	}

	QSqlTableModel *pAllModel = new QSqlTableModel(this, *pDatabase);
	pAllModel->setTable("TOC");
	pAllModel->select();
	ui->searchResultsView->setModel(pAllModel);
*/

}

CKJVCanOpener::~CKJVCanOpener()
{
	delete ui;
}
