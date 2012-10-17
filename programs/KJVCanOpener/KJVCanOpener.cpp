#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include "dbstruct.h"

#include <assert.h>

#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>

CKJVCanOpener::CKJVCanOpener(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CKJVCanOpener)
{
	ui->setupUi(this);
/*

	QSqlDatabase *pDatabase = new QSqlDatabase();
	*pDatabase = QSqlDatabase::addDatabase("QSQLITE");
	pDatabase->setDatabaseName("../KJVCanOpener/db/kjvtext.s3db");
//	pDatabase->setDatabaseName("C:/MyData/programs/KJVCanOpener/db/kjvtext.s3db");

//	QMessageBox::information(0,"Database",pDatabase->databaseName());

	if (!pDatabase->open()) {
		QMessageBox::warning(0,"Error","Couldn't open database file.");
	}

	QSqlTableModel *pAllModel = new QSqlTableModel(this, *pDatabase);
	pAllModel->setTable("TOC");
	pAllModel->select();
	ui->searchResultsView->setModel(pAllModel);
*/

	CPhraseEditListWidgetItem *pPhraseEdit = new CPhraseEditListWidgetItem();
	ui->listWidgetSearchPhrases->addItem(pPhraseEdit);
	ui->listWidgetSearchPhrases->setItemWidget(pPhraseEdit, pPhraseEdit->m_widgetPhraseEdit);
//	new CPhraseEditListWidgetItem(ui->listWidgetSearchPhrases);

ui->widgetPhraseEdit->pStatusBar = ui->statusBar;

}

CKJVCanOpener::~CKJVCanOpener()
{
	delete ui;
}

void CKJVCanOpener::Initialize(uint32_t nInitialIndex)
{
	ui->widgetKJVBrowser->Initialize(nInitialIndex);
}

