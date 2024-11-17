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
#include <QCoreApplication>
#include <QApplication>
#include <QMessageBox>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QClipboard>
#include <QShortcut>
#include <QPoint>

#define ACCEL_KEY(k) (!QCoreApplication::testAttribute(Qt::AA_DontShowShortcutsInContextMenus) ?		\
					  u'\t' + QKeySequence(k).toString(QKeySequence::NativeText) : QString())


// ============================================================================

CELSSearchMainWindow::CELSSearchMainWindow(CBibleDatabasePtr pBibleDatabase,
										   bool bSkipColophons, bool bSkipSuperscriptions,
										   bool bWordsOfJesusOnly, bool bIncludeBookPrologues,
										   QWidget *parent)
	:	QMainWindow(parent),
		m_letterMatrix(pBibleDatabase, bSkipColophons, bSkipSuperscriptions, bWordsOfJesusOnly, bIncludeBookPrologues),
		ui(new Ui::CELSSearchMainWindow)
{
	Q_ASSERT(!pBibleDatabase.isNull());

	ui->setupUi(this);

	// --------------------------------

	int nMax = m_letterMatrix.size()/2;
	if (nMax < 1) nMax = 1;		// Safe-guard in case Matrix is empty (like Words of Jesus only mode on a database without them)
	ui->spinMinSkip->setMaximum(nMax);
	ui->spinMaxSkip->setMaximum(nMax);
	ui->spinWidth->setMaximum(nMax);

	// --------------------------------

	// Set BookNames in drop lists:
	for (unsigned int nBk = 1; nBk <= pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		ui->cmbBookStart->addItem(pBibleDatabase->bookName(CRelIndex(nBk, 0, 0, 0)), nBk);
		ui->cmbBookEnd->addItem(pBibleDatabase->bookName(CRelIndex(nBk, 0, 0, 0)), nBk);
	}
	ui->cmbBookStart->setCurrentIndex(0);
	ui->cmbBookEnd->setCurrentIndex(pBibleDatabase->bibleEntry().m_nNumBk-1);

	// --------------------------------

	QItemSelectionModel *pOldSelModel = ui->tvLetterMatrix->selectionModel();
	QAbstractItemModel *pOldModel = ui->tvLetterMatrix->model();
	m_pLetterMatrixTableModel = new CLetterMatrixTableModel(m_letterMatrix, ui->spinWidth->value(), ui->chkUppercase->isChecked(), this);
	ui->tvLetterMatrix->setModel(m_pLetterMatrixTableModel);
	if (pOldModel) delete pOldModel;
	if (pOldSelModel) delete pOldSelModel;

	QHeaderView *pTVHeader = ui->tvLetterMatrix->verticalHeader();
	pTVHeader->setDefaultSectionSize(m_pLetterMatrixTableModel->fontMetrics().height()+2);
	pTVHeader = ui->tvLetterMatrix->horizontalHeader();
	pTVHeader->setStretchLastSection(false);
	pTVHeader->setDefaultSectionSize(m_pLetterMatrixTableModel->fontMetrics().maxWidth()+2);
	pTVHeader->setSectionResizeMode(QHeaderView::Fixed);			// This avoids needing to resize columns

	// --------------------------------

	pOldSelModel = ui->tvELSResults->selectionModel();
	pOldModel = ui->tvELSResults->model();
	m_pELSResultListModel = new CELSResultListModel(m_letterMatrix, ui->chkUppercase->isChecked(), this);
	ui->tvELSResults->setModel(m_pELSResultListModel);
	if (pOldModel) delete pOldModel;
	if (pOldSelModel) delete pOldSelModel;

	ui->tvELSResults->resizeColumnsToContents();
	ui->tvELSResults->horizontalHeader()->setStretchLastSection(false);
	ui->tvELSResults->setDragEnabled(true);
	ui->tvELSResults->setSelectionBehavior(QAbstractItemView::SelectRows);

	// --------------------------------

	// Set sortOrder descriptions:
	for (int i = ESO_FIRST; i < ESO_COUNT; ++i) {
		ui->cmbSortOrder->addItem(elsresultSortOrderDescription(static_cast<ELSRESULT_SORT_ORDER_ENUM>(i)), i);
	}
	ui->cmbSortOrder->setCurrentIndex(m_pELSResultListModel->sortOrder());

	// --------------------------------

	m_pQuitAction = ui->toolBar->addAction(QIcon(":/res/exit.png"), tr("E&xit", "MainMenu"), QApplication::instance(), &QApplication::exit);
	m_pQuitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
	m_pQuitAction->setStatusTip(tr("Exit the KJPBS ELS Search Application", "MainMenu"));
	m_pQuitAction->setToolTip(tr("Exit Application", "MainMenu"));

	// --------------------------------

	connect(ui->spinWidth, SIGNAL(valueChanged(int)), m_pLetterMatrixTableModel, SLOT(setWidth(int)));
	connect(ui->chkUppercase, SIGNAL(toggled(bool)), m_pLetterMatrixTableModel, SLOT(setUppercase(bool)));
	connect(ui->chkUppercase, SIGNAL(toggled(bool)), m_pELSResultListModel, SLOT(setUppercase(bool)));

	connect(ui->btnSearch, &QToolButton::clicked, this, &CELSSearchMainWindow::search);
	connect(ui->btnClear, &QToolButton::clicked, this, &CELSSearchMainWindow::clear);
	connect(ui->editWords, &QLineEdit::returnPressed, this, &CELSSearchMainWindow::search);

	connect(ui->tvELSResults, &QTableView::doubleClicked, this, &CELSSearchMainWindow::en_searchResultClicked);
	connect(ui->tvELSResults, &QTableView::activated, this, &CELSSearchMainWindow::en_searchResultClicked);
	connect(ui->cmbSortOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSortOrder(int)));

	// --------------------------------

	QShortcut *pShortcut = new QShortcut(QKeySequence::ZoomIn, ui->tvLetterMatrix);
	connect(pShortcut, &QShortcut::activated, ui->spinWidth, &QSpinBox::stepUp);
	pShortcut = new QShortcut(QKeySequence::ZoomOut, ui->tvLetterMatrix);
	connect(pShortcut, &QShortcut::activated, ui->spinWidth, &QSpinBox::stepDown);

	// --------------------------------

	ui->tvELSResults->installEventFilter(this);
	ui->tvLetterMatrix->installEventFilter(this);
}

CELSSearchMainWindow::~CELSSearchMainWindow()
{
	delete ui;
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::en_searchResultClicked(const QModelIndex &index)
{
	CRelIndexEx ndx =  m_pELSResultListModel->data(index, Qt::UserRole).value<CRelIndexEx>();
	uint32_t matrixIndex = m_letterMatrix.matrixIndexFromRelIndex(ndx);
	if (matrixIndex) ui->tvLetterMatrix->scrollTo(m_pLetterMatrixTableModel->modelIndexFromMatrixIndex(matrixIndex), QAbstractItemView::PositionAtTop);
}

void CELSSearchMainWindow::en_changedSortOrder(int nIndex)
{
	m_pELSResultListModel->setSortOrder(static_cast<ELSRESULT_SORT_ORDER_ENUM>(ui->cmbSortOrder->itemData(nIndex).toInt()));
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
	QStringList lstSearchWords = ui->editWords->text().split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);

	CFindELS elsFinder(m_pLetterMatrixTableModel->matrix(), lstSearchWords);
	if (!elsFinder.setBookEnds(ui->cmbBookStart->currentData().toUInt(), ui->cmbBookEnd->currentData().toUInt())) {
		Q_ASSERT(false);
		QMessageBox::critical(this, QApplication::applicationName(), tr("Failed to set Book Range!"));
		return;
	}
	unsigned int nBookStart = elsFinder.bookStart();
	unsigned int nBookEnd = elsFinder.bookEnd();

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

	watcher.setFuture(elsFinder.future(ui->spinMinSkip->value(), ui->spinMaxSkip->value(), &CFindELS::reduce));
	dlgProgress.exec();
	CBusyCursor iAmBusy(this);
	watcher.waitForFinished();

	if (!watcher.future().isCanceled()) {
		CELSResultList lstResults = watcher.result();
		m_pLetterMatrixTableModel->setSearchResults(lstResults);
		m_pELSResultListModel->setSearchResults(lstResults);
		// TODO : Add sort order support in model and tie to GUI

		insertSearchLogText(tr("Search Time: %1 secs").arg(elapsedTime.elapsed() / 1000.0));

		QString strBookRange;
		if ((nBookStart == 1) && (nBookEnd == m_letterMatrix.bibleDatabase()->bibleEntry().m_nNumBk)) {
			strBookRange = "Entire Bible";
		} else {
			strBookRange = tr("%1 through %2")
							   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookStart, 0, 0, 0)))
							   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookEnd, 0, 0, 0)));
		}
		if (m_letterMatrix.wordsOfJesusOnly()) {
			strBookRange += " (" + tr("Words of Jesus Only") + ")";
		} else {
			// There's no Words of Jesus in Colophons or Superscriptions
			if (m_letterMatrix.skipColophons() || m_letterMatrix.skipSuperscriptions()) {
				strBookRange += " " + tr("Without") + " ";
				if (m_letterMatrix.skipColophons()) {
					strBookRange += tr("Colophons");
					if (m_letterMatrix.skipSuperscriptions()) {
						strBookRange += " " + tr("or Superscriptions");
					}
				} else {
					strBookRange += tr("Superscriptions");
				}
			}
		}

		insertSearchLogText(tr("Searching for ELS skips from %1 to %2 in %3")
										.arg(ui->spinMinSkip->value())
										.arg(ui->spinMaxSkip->value())
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

// ----------------------------------------------------------------------------

bool CELSSearchMainWindow::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == ui->tvELSResults) {									// SearchResults:
		if (ev->type() == QEvent::ContextMenu) {					//		ContextMenu
			QContextMenuEvent *pCMEvent = static_cast<QContextMenuEvent *>(ev);
			QMenu *pMenu = createELSResultsContextMenu();
			pMenu->setAttribute(Qt::WA_DeleteOnClose);
			pMenu->popup(pCMEvent->globalPos());
			pCMEvent->setAccepted(true);
			return true;
		} else if (ev->type() == QEvent::KeyPress) {				//		Keypress
			QKeyEvent *pKEvent = static_cast<QKeyEvent *>(ev);
			if (pKEvent->matches(QKeySequence::Copy)) {				//			Copy
				en_copySearchResults();
				pKEvent->accept();
				return true;
			}
		}
	} else if (obj == ui->tvLetterMatrix) {							// LetterMatrix:
		if (ev->type() == QEvent::ContextMenu) {					//		ContextMenu
			QContextMenuEvent *pCMEvent = static_cast<QContextMenuEvent *>(ev);
			QMenu *pMenu = createLetterMatrixContextMenu();
			pMenu->setAttribute(Qt::WA_DeleteOnClose);
			pMenu->popup(pCMEvent->globalPos());
			pCMEvent->setAccepted(true);
			return true;
		} else if (ev->type() == QEvent::KeyPress) {				//		Keypress
			QKeyEvent *pKEvent = static_cast<QKeyEvent *>(ev);
			if (pKEvent->matches(QKeySequence::Copy)) {				//			Copy
				en_copyLetterMatrix();
				pKEvent->accept();
				return true;
			}
		} else if (ev->type() == QEvent::Wheel) {					//		Wheel
			QWheelEvent *pWEvent = static_cast<QWheelEvent *>(ev);
			QPoint ptDelta = pWEvent->angleDelta();

			if ((pWEvent->modifiers() & Qt::ControlModifier) && (pWEvent->buttons() == Qt::NoButton))  {
				if (!ptDelta.isNull()) {
					if (ptDelta.y() > 8) {
						ui->spinWidth->stepUp();
					} else if (ptDelta.y() < -8) {
						ui->spinWidth->stepDown();
					}
				}
				pWEvent->setAccepted(true);
				return true;
			}
		}
	}

	return false;
}

QMenu *CELSSearchMainWindow::createELSResultsContextMenu()
{
	QMenu *pMenu = new QMenu(tr("&Edit", "tvELSResults"), ui->tvELSResults);
	pMenu->setStatusTip(tr("ELS Search Results Edit Operations", "MainMenu"));

	QAction *pAction = pMenu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy", "tvELSResults") + ACCEL_KEY(QKeySequence::Copy), this, SLOT(en_copySearchResults()));
	pAction->setObjectName("edit-copy");
	pAction->setEnabled(ui->tvELSResults->selectionModel()->hasSelection());

	pMenu->addSeparator();

	pAction = pMenu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select All", "tvELSResults") + ACCEL_KEY(QKeySequence::SelectAll), ui->tvELSResults, SLOT(selectAll()));
	pAction->setObjectName("select-all");
	pAction->setEnabled(m_pELSResultListModel->rowCount() != 0);

	return pMenu;
}

void CELSSearchMainWindow::en_copySearchResults()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(m_pELSResultListModel->mimeData(ui->tvELSResults->selectionModel()->selectedIndexes()));
}

QMenu *CELSSearchMainWindow::createLetterMatrixContextMenu()
{
	QMenu *pMenu = new QMenu(tr("&Edit", "tvLetterMatrix"), ui->tvLetterMatrix);
	pMenu->setStatusTip(tr("ELS Search Letter Matrix Edit Operations", "MainMenu"));

	QAction *pAction = pMenu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy", "tvLetterMatrix") + ACCEL_KEY(QKeySequence::Copy), this, SLOT(en_copyLetterMatrix()));
	pAction->setObjectName("edit-copy");
	pAction->setEnabled(ui->tvLetterMatrix->selectionModel()->hasSelection());

	pMenu->addSeparator();

	pAction = pMenu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select All", "tvLetterMatrix") + ACCEL_KEY(QKeySequence::SelectAll), ui->tvLetterMatrix, SLOT(selectAll()));
	pAction->setObjectName("select-all");
	pAction->setEnabled(!m_letterMatrix.isEmpty());

	return pMenu;
}

void CELSSearchMainWindow::en_copyLetterMatrix()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(m_pLetterMatrixTableModel->mimeData(ui->tvLetterMatrix->selectionModel()->selectedIndexes()));
}

// ============================================================================
