#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include "dbstruct.h"
#include "VerseListModel.h"

#include <assert.h>

#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QListView>
#include <QStringList>
//#include <QStringListModel>

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

	CKJVSearchPhraseEdit *pPhraseEdit = new CKJVSearchPhraseEdit(ui->scrollAreaSearchPhrases);


ui->widgetPhraseEdit->pStatusBar = ui->statusBar;
pPhraseEdit->pStatusBar = ui->statusBar;


//	ui->listViewSearchResults->setModel(new QStringListModel());
	CVerseListModel *model = new CVerseListModel();
	model->setDisplayMode(CVerseListModel::VDME_HEADING);
	ui->listViewSearchResults->setModel(model);

	connect(ui->widgetPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
	connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
}

CKJVCanOpener::~CKJVCanOpener()
{
	delete ui;
}

void CKJVCanOpener::Initialize(uint32_t nInitialIndex)
{
	ui->widgetKJVBrowser->Initialize(nInitialIndex);
}

void CKJVCanOpener::on_phraseChanged(const CParsedPhrase &phrase)
{
	TIndexList lstResults = phrase.GetNormalizedSearchResults();
//	QStringList lstReferences;
	CVerseList lstReferences;

	if (lstResults.size() <= 5000) {		// This check keep the really heavy hitters like 'and' and 'the' from making us come to a complete stand-still
		for (unsigned int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
			int nCount = 1;
			uint32_t ndxDenormal = DenormalizeIndex(lstResults[ndxResults]);
			TRelIndex ndxRelative(ndxDenormal);

			if ((lstResults[ndxResults] == 0) || (ndxDenormal == 0)) {
//				lstReferences.push_back(QString("Invalid Index: @ %1: Norm: %2  Denorm: %3").arg(ndxResults).arg(lstResults[ndxResults]).arg(ndxDenormal));

				lstReferences.push_back(CVerseListItem(
						0,
						QString("Invalid Index: @ %1: Norm: %2  Denorm: %3").arg(ndxResults).arg(lstResults[ndxResults]).arg(ndxDenormal),
						QString("TODO : TOOLTIP")));
				continue;
			}

			if (ndxResults<(lstResults.size()-1)) {
				bool bNextIsSameReference=false;
				do {
					TRelIndex ndxNextRelative(DenormalizeIndex(lstResults[ndxResults+1]));
					if ((ndxRelative.m_nN3 == ndxNextRelative.m_nN3) &&
						(ndxRelative.m_nN2 == ndxNextRelative.m_nN2) &&
						(ndxRelative.m_nN1 == ndxNextRelative.m_nN1)) {
						bNextIsSameReference=true;
						nCount++;
						ndxResults++;
					} else {
						bNextIsSameReference=false;
					}
				} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
			}
			if ((lstResults[ndxResults] != 0) && (ndxDenormal != 0)) {
//				lstReferences.push_back(QString("%1 %2:%3 [%4] (%5)").arg(g_lstTOC[ndxRelative.m_nN3-1].m_strBkName).arg(ndxRelative.m_nN2).arg(ndxRelative.m_nN1).arg(ndxRelative.m_nN0).arg(nCount));
				lstReferences.push_back(CVerseListItem(
						ndxRelative,
						QString("%1 %2:%3 [%4] (%5)").arg(g_lstTOC[ndxRelative.m_nN3-1].m_strBkName).arg(ndxRelative.m_nN2).arg(ndxRelative.m_nN1).arg(ndxRelative.m_nN0).arg(nCount),
						QString("TODO : TOOLTIP")));
			}
		}
//		lstReferences.removeDuplicates();
	}

//	QStringListModel *pModel = static_cast<QStringListModel *>(ui->listViewSearchResults->model());
	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		if (lstReferences.size() <= 2000) {
//			pModel->setStringList(lstReferences);
			pModel->setVerseList(lstReferences);
		} else {
//			pModel->setStringList(QStringList());
			pModel->setVerseList(CVerseList());
		}
	}
}

