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
#include <QFutureSynchronizer>
#include <QTextCursor>
#include <QElapsedTimer>

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
	// TODO : Switch to QFutureWatcher and add progress dialog

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

	CBusyCursor busy(this);

	QElapsedTimer elapsedTime;
	elapsedTime.start();

	insertSearchLogText("Searching for: " + lstSearchWords.join(','));

	QFutureSynchronizer<CELSResultList> synchronizer;
	synchronizer.setFuture(elsFinder.future(ui->spinMinSkip->value(), ui->spanMaxSkip->value(), &CFindELS::reduce));
	synchronizer.waitForFinished();
	CELSResultList lstResults = synchronizer.futures().at(0).result();
	m_pLetterMatrixTableModel->setSearchResults(lstResults);
	m_pELSResultListModel->setSearchResults(lstResults);

	insertSearchLogText(QString("Search Time: %1 secs").arg(elapsedTime.elapsed() / 1000.0));

	QString strBookRange;
	unsigned int nBookStart = elsFinder.bookStart();
	unsigned int nBookEnd = elsFinder.bookEnd();
	if ((nBookStart == 1) && (nBookEnd == m_letterMatrix.bibleDatabase()->bibleEntry().m_nNumBk)) {
		strBookRange = "Entire Bible";
	} else {
		strBookRange = QString("%1 through %2")
						   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookStart, 0, 0, 0)))
						   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookEnd, 0, 0, 0)));
	}
	insertSearchLogText(QString("Searching for ELS skips from %1 to %2 in %3")
									.arg(ui->spinMinSkip->value())
									.arg(ui->spanMaxSkip->value())
									.arg(strBookRange));

	insertSearchLogText(QString("Found %1 Results\n").arg(lstResults.size()));

	insertSearchLogText("----------------------------------------");

	ui->tvELSResults->resizeColumnsToContents();

	// Should we clear editWords here?
}

void CELSSearchMainWindow::clear()
{
	m_pLetterMatrixTableModel->clearSearchResults();
	m_pELSResultListModel->clearSearchResults();
	clearSearchLogText();
}

// ============================================================================
