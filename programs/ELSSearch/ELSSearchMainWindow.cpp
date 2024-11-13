/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#include "ELSSearchMainWindow.h"
#include "ui_ELSSearchMainWindow.h"

#include "../KJVCanOpener/BusyCursor.h"

#include "LetterMatrixTableModel.h"
#include "FindELS.h"
#include "ELSResult.h"

#include <QStringList>
#include <QRegularExpression>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QTextCursor>
#include <QElapsedTimer>
#include <QApplication>

// ============================================================================

CELSSearchMainWindow::CELSSearchMainWindow(CBibleDatabasePtr pBibleDatabase,
										   bool bSkipColophons, bool bSkipSuperscriptions,
										   QWidget *parent)
	:	QMainWindow(parent),
		m_letterMatrix(pBibleDatabase, bSkipColophons, bSkipSuperscriptions),
		ui(new Ui::CELSSearchMainWindow)
{
	ui->setupUi(this);

	// --------------------------------

	QItemSelectionModel *pOldSelModel = ui->tvLetterMatrix->selectionModel();
	QAbstractItemModel *pOldModel = ui->tvLetterMatrix->model();
	m_pLetterMatrixTableModel = new CLetterMatrixTableModel(m_letterMatrix, ui->spinWidth->value(), this);
	ui->tvLetterMatrix->setModel(m_pLetterMatrixTableModel);
	if (pOldModel) delete pOldModel;
	if (pOldSelModel) delete pOldSelModel;

	ui->tvLetterMatrix->resizeColumnsToContents();

	// --------------------------------

	pOldSelModel = ui->tvELSResults->selectionModel();
	pOldModel = ui->tvELSResults->model();
	m_pELSResultListModel = new CELSResultListModel(pBibleDatabase, this);
	ui->tvELSResults->setModel(m_pELSResultListModel);
	if (pOldModel) delete pOldModel;
	if (pOldSelModel) delete pOldSelModel;

	ui->tvELSResults->resizeColumnsToContents();
	ui->tvELSResults->horizontalHeader()->setStretchLastSection(false);

	// --------------------------------

	m_pQuitAction = ui->toolBar->addAction(QIcon(":/res/exit.png"), tr("E&xit", "MainMenu"), QApplication::instance(), &QApplication::exit);
	m_pQuitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
	m_pQuitAction->setStatusTip(tr("Exit the KJPBS ELS Search Application", "MainMenu"));
	m_pQuitAction->setToolTip(tr("Exit Application", "MainMenu"));

	// --------------------------------

	connect(ui->spinWidth, SIGNAL(valueChanged(int)), m_pLetterMatrixTableModel, SLOT(setWidth(int)));
	connect(m_pLetterMatrixTableModel, &CLetterMatrixTableModel::layoutChanged, this, &CELSSearchMainWindow::en_letterMatrixLayoutChanged);

	connect(ui->btnSearch, &QToolButton::clicked, this, &CELSSearchMainWindow::search);
	connect(ui->btnClear, &QToolButton::clicked, this, &CELSSearchMainWindow::clear);

	connect(ui->tvELSResults, &QTableView::doubleClicked, this, &CELSSearchMainWindow::en_searchResultClicked);
}

CELSSearchMainWindow::~CELSSearchMainWindow()
{
	delete ui;
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::en_letterMatrixLayoutChanged()
{
	ui->tvLetterMatrix->resizeColumnsToContents();
}

void CELSSearchMainWindow::en_searchResultClicked(const QModelIndex &index)
{
	CRelIndexEx ndx =  m_pELSResultListModel->data(index, Qt::UserRole).value<CRelIndexEx>();
	uint32_t matrixIndex = m_letterMatrix.matrixIndexFromRelIndex(ndx);
	if (matrixIndex) ui->tvLetterMatrix->scrollTo(m_pLetterMatrixTableModel->modelIndexFromMatrixIndex(matrixIndex), QAbstractItemView::PositionAtTop);
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::insertSearchLogText(const QString &strText)
{
	ui->editSearchLog->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	ui->editSearchLog->insertPlainText(strText + "\n");
}

void CELSSearchMainWindow::clearSearchLogText()
{
	ui->editSearchLog->clear();
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::search()
{
	// TODO : Add book search range

	QStringList lstSearchWords = ui->editWords->text().split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);

	CFindELS elsFinder(m_pLetterMatrixTableModel->matrix(), lstSearchWords);
//	if (!elsFinder.setBookEnds(nBookStart, nBookEnd)) {
//		std::cerr << QString("\n*** ERROR: Invalid Book Begin/End specified.  Database has books 1 through %1!\n")
//						 .arg(pBibleDatabase->bibleEntry().m_nNumBk).toUtf8().data();
//		return -4;
//	}
//	nBookStart = elsFinder.bookStart();
//	nBookEnd = elsFinder.bookEnd();

	QProgressDialog dlgProgress;
	dlgProgress.setLabelText(tr("Searching for: ") + lstSearchWords.join(','));

	insertSearchLogText(tr("Searching for: ") + lstSearchWords.join(','));

	QElapsedTimer elapsedTime;
	elapsedTime.start();

	QFutureWatcher<CELSResultList> watcher;
	connect(&watcher, &QFutureWatcher<CELSResultList>::finished, &dlgProgress, &QProgressDialog::reset);
	connect(&dlgProgress, &QProgressDialog::canceled, &watcher, &QFutureWatcher<CELSResultList>::cancel);
	connect(&watcher, &QFutureWatcher<CELSResultList>::progressRangeChanged, &dlgProgress, &QProgressDialog::setRange);
	connect(&watcher, &QFutureWatcher<CELSResultList>::progressValueChanged, &dlgProgress, &QProgressDialog::setValue);

	watcher.setFuture(elsFinder.future(ui->spinMinSkip->value(), ui->spanMaxSkip->value(), &CFindELS::reduce));
	dlgProgress.exec();
	watcher.waitForFinished();

	if (!watcher.future().isCanceled()) {
		CELSResultList lstResults = watcher.result();
		m_pLetterMatrixTableModel->setSearchResults(lstResults);
		m_pELSResultListModel->setSearchResults(lstResults);

		insertSearchLogText(tr("Search Time: %1 secs").arg(elapsedTime.elapsed() / 1000.0));

		QString strBookRange;
		unsigned int nBookStart = elsFinder.bookStart();
		unsigned int nBookEnd = elsFinder.bookEnd();
		if ((nBookStart == 1) && (nBookEnd == m_letterMatrix.bibleDatabase()->bibleEntry().m_nNumBk)) {
			strBookRange = "Entire Bible";
		} else {
			strBookRange = tr("%1 through %2")
							   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookStart, 0, 0, 0)))
							   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookEnd, 0, 0, 0)));
		}
		insertSearchLogText(tr("Searching for ELS skips from %1 to %2 in %3")
										.arg(ui->spinMinSkip->value())
										.arg(ui->spanMaxSkip->value())
										.arg(strBookRange));

		insertSearchLogText(tr("Found %1 Results\n").arg(lstResults.size()));

		ui->tvELSResults->resizeColumnsToContents();

		// Should we clear editWords here?
	} else {
		insertSearchLogText(tr("Search was cancelled by user\n"));
	}

	insertSearchLogText("----------------------------------------");
}

void CELSSearchMainWindow::clear()
{
	m_pLetterMatrixTableModel->clearSearchResults();
	m_pELSResultListModel->clearSearchResults();
	clearSearchLogText();
}

// ============================================================================
