#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include "dbstruct.h"
#include "VerseListModel.h"
#include "KJVPassageNavigatorDlg.h"

#include <assert.h>

#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QListView>
#include <QStringList>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>

// ============================================================================

QSize CSearchPhraseScrollArea::minimumSizeHint() const
{
	return QScrollArea::minimumSizeHint();
}

QSize CSearchPhraseScrollArea::sizeHint() const
{
	return QScrollArea::sizeHint();
}

// ============================================================================

CKJVCanOpener::CKJVCanOpener(QWidget *parent) :
	QMainWindow(parent),
	m_pActionJump(NULL),
	ui(new Ui::CKJVCanOpener)
{
	ui->setupUi(this);

	m_pActionJump = new QAction(QIcon(":/res/go_jump2.png"), "Passage Navigator", this);
	ui->mainToolBar->addAction(m_pActionJump);
	connect(m_pActionJump, SIGNAL(triggered()), this, SLOT(on_PassageNavigatorTriggered()));

	ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	CKJVSearchPhraseEdit *pPhraseEdit = new CKJVSearchPhraseEdit();

	QVBoxLayout *pLayoutPhrases = new QVBoxLayout(ui->scrollAreaWidgetContents);
	pLayoutPhrases->setSpacing(0);
	pLayoutPhrases->setContentsMargins(0, 0, 0, 0);
	pLayoutPhrases->addWidget(pPhraseEdit);

	ui->scrollAreaWidgetContents->setMinimumSize(/* pLayoutPhrases->sizeHint() */ pPhraseEdit->sizeHint() );

	ui->scrollAreaSearchPhrases->setMinimumSize(pLayoutPhrases->sizeHint().width() +
							ui->scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui->scrollAreaSearchPhrases->frameWidth() * 2,
							pLayoutPhrases->sizeHint().height() /* pPhraseEdit->sizeHint() */);

/*
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());

ui->scrollAreaWidgetContents->setMinimumSize(pPhraseEdit->sizeHint().width(), pPhraseEdit->sizeHint().height()*7);
*/

ui->scrollAreaWidgetContents->setMinimumSize(pPhraseEdit->sizeHint().width(), pPhraseEdit->sizeHint().height()*1);


//ui->widgetPhraseEdit->pStatusBar = ui->statusBar;
pPhraseEdit->pStatusBar = ui->statusBar;


	CVerseListModel *model = new CVerseListModel();
	model->setDisplayMode(CVerseListModel::VDME_HEADING);
	ui->listViewSearchResults->setModel(model);

//connect(ui->widgetPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
	connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));

	connect(ui->listViewSearchResults, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(on_SearchResultDoubleClick(const QModelIndex &)));
}

CKJVCanOpener::~CKJVCanOpener()
{
	delete ui;
}

void CKJVCanOpener::Initialize(CRelIndex nInitialIndex)
{
	ui->widgetKJVBrowser->gotoIndex(nInitialIndex);
}

void CKJVCanOpener::on_phraseChanged(const CParsedPhrase &phrase)
{
	TIndexList lstResults = phrase.GetNormalizedSearchResults();
	CVerseList lstReferences;

	if (lstResults.size() <= 5000) {		// This check keep the really heavy hitters like 'and' and 'the' from making us come to a complete stand-still
		for (unsigned int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
			int nCount = 1;
			uint32_t ndxDenormal = DenormalizeIndex(lstResults[ndxResults]);
			CRelIndex ndxRelative(ndxDenormal);

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
					CRelIndex ndxNextRelative(DenormalizeIndex(lstResults[ndxResults+1]));
					if ((ndxRelative.book() == ndxNextRelative.book()) &&
						(ndxRelative.chapter() == ndxNextRelative.chapter()) &&
						(ndxRelative.verse() == ndxNextRelative.verse())) {
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
						QString("%1 (%2)").arg(CRefCountCalc::PassageReferenceText(ndxRelative)).arg(nCount),
						QString("TODO : TOOLTIP")));
			}
		}
//		lstReferences.removeDuplicates();
	}

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		if (lstReferences.size() <= 2000) {
			pModel->setVerseList(lstReferences);
		} else {
			pModel->setVerseList(CVerseList());
		}
	}

	if ((lstReferences.size() != 0) ||
		((lstReferences.size() == 0) && (lstResults.size() == 0))) {
		ui->lblSearchResultsCount->setText(QString("Found %1 occurrences in %2 verses").arg(lstResults.size()).arg(lstReferences.size()));
	} else {
		ui->lblSearchResultsCount->setText(QString("Found %1 occurrences (too many verses!)").arg(lstResults.size()));
	}
}

void CKJVCanOpener::on_SearchResultDoubleClick(const QModelIndex &index)
{
	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	CVerseListItem verse = pModel->data(index, CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>();

	ui->widgetKJVBrowser->gotoIndex(verse.getIndex());
}

void CKJVCanOpener::on_PassageNavigatorTriggered()
{
	CKJVPassageNavigatorDlg dlg(this);

	if (dlg.exec() == QDialog::Accepted) {
		ui->widgetKJVBrowser->gotoIndex(dlg.passage());
	}
}
