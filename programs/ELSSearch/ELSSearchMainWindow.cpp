/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifdef USING_ELSSEARCH			// if using ELSSearch as KJPBS subcomponent:
#include "myApplication.h"
#include "KJVCanOpener.h"
#endif

#include "../KJVCanOpener/BusyCursor.h"

#include "LetterMatrixTableModel.h"
#include "FindELS.h"
#include "ELSResult.h"

#include <utility>			// for std::swap

#include "../KJVCanOpener/ReportError.h"

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
#include "../KJVCanOpener/SaveLoadFileDialog.h"
#include <QtIOCompressor>
#include "../KJVCanOpener/CSV.h"
#include <QTimer>
#endif

#include <QStringList>
#include <QRegularExpression>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QTextCursor>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QApplication>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QClipboard>
#include <QShortcut>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

#define ACCEL_KEY(k) (!QCoreApplication::testAttribute(Qt::AA_DontShowShortcutsInContextMenus) ?		\
					  u'\t' + QKeySequence(k).toString(QKeySequence::NativeText) : QString())


constexpr int ELS_FILE_VERSION = 5;		// Current ELS Transcript File Version

// ============================================================================

void CLetterMatrixLineWidget::paintEvent(QPaintEvent *event)
{
	// TODO : Fix this to be not so hard coded:
	static QPen penLine(QBrush(QColor("lightblue")), 3, Qt::DashLine);

	QWidget::paintEvent(event);

	QPainter linePainter(this);

	CLetterMatrixTableModel *pModel = qobject_cast<CLetterMatrixTableModel *>(m_pView->model());
	Q_ASSERT(pModel != nullptr);
	QSize szViewport = m_pView->viewport()->size();
	Q_ASSERT(size() == szViewport);

	int nRowFirst = m_pView->rowAt(0);
	int nRowLast = m_pView->rowAt(szViewport.height()-2);
	int nColFirst = (m_pView->layoutDirection() == Qt::LeftToRight) ? m_pView->columnAt(0) : m_pView->columnAt(szViewport.width()-2);
	int nColLast =  (m_pView->layoutDirection() == Qt::LeftToRight) ? m_pView->columnAt(szViewport.width()-2) : m_pView->columnAt(0);

	if (nRowLast == -1) nRowLast = nRowFirst + pModel->rowCount() -1;
	if (nColLast == -1) nColLast = nColFirst + pModel->width() - 1;

	QPainterPath clipWhole;
	clipWhole.addRect(0, 0, size().width(), size().height());
	for (int nRow = nRowFirst; nRow <= nRowLast; ++nRow) {
		for (int nCol = nColFirst; nCol <= nColLast; ++nCol) {
			QModelIndex index = pModel->modelIndexFromMatrixIndex(pModel->matrixIndexFromRowCol(nRow, nCol));
			if (index.isValid()) {
				const QVariant &varResultsSet = index.data(CLetterMatrixTableModel::UserRole_ResultsSet);
				if (varResultsSet.isValid() && varResultsSet.canConvert<CELSResultSet>()) {
					const CELSResultSet &setResults = varResultsSet.value<CELSResultSet>();

					for (CELSResultSet::const_iterator itrResults = setResults.cbegin(); itrResults != setResults.cend(); ++itrResults) {
						const CELSResult &result = itrResults.key();
						uint32_t ndxLetter = pModel->matrix().matrixIndexFromRelIndex(result.m_ndxStart);
						QRect rcCur = m_pView->visualRect(pModel->modelIndexFromMatrixIndex(ndxLetter));
						for (int i = 0; i < result.m_strWord.size()-1; ++i) {
							ndxLetter += CFindELS::nextOffset(result.m_nSkip, i, result.m_nSearchType);
							QRect rcNext = m_pView->visualRect(pModel->modelIndexFromMatrixIndex(ndxLetter));

							if (rcCur.isValid() && rcNext.isValid()) {
								linePainter.setPen(penLine);
								linePainter.drawEllipse(rcCur.center(), rcCur.width()/2, rcCur.height()/2);
								QPainterPath clipPath;
								clipPath.addEllipse(rcCur.center(), rcCur.width()/2, rcCur.height()/2);
								clipPath.addEllipse(rcNext.center(), rcNext.width()/2, rcNext.height()/2);
								linePainter.setClipPath(clipWhole.subtracted(clipPath));
								linePainter.drawLine(rcCur.center(), rcNext.center());
								linePainter.setClipPath(QPainterPath(), Qt::NoClip);
							}

							rcCur = rcNext;
						}
						linePainter.drawEllipse(rcCur.center(), rcCur.width()/2, rcCur.height()/2);		// Final letter
					}
				}
			}
		}
	}
}

// ============================================================================

CLetterMatrixTableView::CLetterMatrixTableView(QWidget *pParent)
	:	QTableView(pParent)
{
	// Create special widget to draw lines on the LetterMatrix for ELSResults:
	m_pLetterMatrixLineWidget = new CLetterMatrixLineWidget(this, viewport());
}

bool CLetterMatrixTableView::viewportEvent(QEvent *event)
{
	bool bRetVal = QTableView::viewportEvent(event);
	if (m_pLetterMatrixLineWidget &&					// Note: This function may be called when m_pLetterMatrixLineWidget is still nullptr during construction
		(event->type() == QEvent::Resize)) {
		QResizeEvent *pResizeEvent = static_cast<QResizeEvent *>(event);
		m_pLetterMatrixLineWidget->resize(pResizeEvent->size());
	}
	return bRetVal;
}

void CLetterMatrixTableView::scrollContentsBy(int dx, int dy)
{
	Q_ASSERT(m_pLetterMatrixLineWidget != nullptr);

	QTableView::scrollContentsBy(dx, dy);
	if (m_pLetterMatrixLineWidget) {
		m_pLetterMatrixLineWidget->move(0, 0);
	}
}

// ============================================================================

QString CELSSearchMainWindow::g_strLastELSFilePath;

// ----------------------------------------------------------------------------

CELSSearchMainWindow::CELSSearchMainWindow(CBibleDatabasePtr pBibleDatabase,
										   LetterMatrixTextModifierOptionFlags flagsLMTMO,
										   LMBookPrologueOptionFlags flagsLMBPO,
										   LMChapterPrologueOptionFlags flagsLMCPO,
										   LMVersePrologueOptionFlags flagsLMVPO,
										   LMFullVerseTextOptionFlags flagsLMFVTO,
										   QWidget *parent)
	:	QMainWindow(parent),
		m_letterMatrix(pBibleDatabase, flagsLMTMO, flagsLMBPO, flagsLMCPO, flagsLMVPO, flagsLMFVTO),
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
	ui->spinOffset->setMaximum(ui->spinWidth->value()-1);

	// --------------------------------

	// Set BookNames in drop lists:
	for (unsigned int nBk = 1; nBk <= pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		ui->cmbBookStart->addItem(pBibleDatabase->bookName(CRelIndex(nBk, 0, 0, 0)), nBk);
		ui->cmbBookEnd->addItem(pBibleDatabase->bookName(CRelIndex(nBk, 0, 0, 0)), nBk);
	}
	ui->cmbBookStart->setCurrentIndex(0);
	ui->cmbBookEnd->setCurrentIndex(pBibleDatabase->bibleEntry().m_nNumBk-1);

	// --------------------------------

	// Set LetterCase drop list:
	ui->cmbLetterCase->addItem(letterCaseDescription(LCE_LOWER), LCE_LOWER);
	ui->cmbLetterCase->addItem(letterCaseDescription(LCE_UPPER), LCE_UPPER);
	ui->cmbLetterCase->addItem(letterCaseDescription(LCE_ORIGINAL), LCE_ORIGINAL);
	ui->cmbLetterCase->setCurrentIndex(LCE_LOWER);

	// --------------------------------

	QItemSelectionModel *pOldSelModel = ui->tvLetterMatrix->selectionModel();
	QAbstractItemModel *pOldModel = ui->tvLetterMatrix->model();
	m_pLetterMatrixTableModel = new CLetterMatrixTableModel(m_letterMatrix,
															ui->spinWidth->value(),
															ui->spinOffset->value(),
															ui->cmbLetterCase->currentData().value<LETTER_CASE_ENUM>(),
															this);
	ui->tvLetterMatrix->setModel(m_pLetterMatrixTableModel);
	if (pOldModel) delete pOldModel;
	if (pOldSelModel) delete pOldSelModel;

	QHeaderView *pTVHeader = ui->tvLetterMatrix->verticalHeader();
	pTVHeader->setDefaultSectionSize(m_pLetterMatrixTableModel->fontMetrics().height()+2);
	pTVHeader = ui->tvLetterMatrix->horizontalHeader();
	pTVHeader->setStretchLastSection(false);
	pTVHeader->setDefaultSectionSize(m_pLetterMatrixTableModel->fontMetrics().maxWidth()+2);
	pTVHeader->setSectionResizeMode(QHeaderView::Fixed);			// This avoids needing to resize columns

	ui->tvLetterMatrix->setLayoutDirection(pBibleDatabase->direction());
	ui->editWords->setLayoutDirection(pBibleDatabase->direction());

	// --------------------------------

	pOldSelModel = ui->tvELSResults->selectionModel();
	pOldModel = ui->tvELSResults->model();
	m_pELSResultListModel = new CELSResultListModel(m_letterMatrix, ui->cmbLetterCase->currentData().value<LETTER_CASE_ENUM>(), this);
	ui->tvELSResults->setModel(m_pELSResultListModel);
	if (pOldModel) delete pOldModel;
	if (pOldSelModel) delete pOldSelModel;

	ui->tvELSResults->resizeColumnsToContents();
	ui->tvELSResults->horizontalHeader()->setStretchLastSection(false);
	ui->tvELSResults->setDragEnabled(true);
	ui->tvELSResults->setSelectionBehavior(QAbstractItemView::SelectRows);
	for (int i = 0; i < m_pELSResultListModel->columnCount(); ++i) {
		ui->tvELSResults->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
	}

	// --------------------------------

	// Set sortOrder descriptions:
	for (int i = ESO_FIRST; i < ESO_COUNT; ++i) {
		ui->cmbSortOrder->addItem(elsresultSortOrderDescription(static_cast<ELSRESULT_SORT_ORDER_ENUM>(i)), i);
	}
	ui->cmbSortOrder->setCurrentIndex(m_pELSResultListModel->sortOrder());

	// --------------------------------

	// Set searchType descriptions:
	for (int i = ESTE_FIRST; i < ESTE_COUNT; ++i) {
		ui->cmbSearchType->addItem(elsSearchTypeDescription(static_cast<ELS_SEARCH_TYPE_ENUM>(i)), i);
	}
	ui->cmbSearchType->setCurrentIndex(ESTE_ELS);

	// --------------------------------

	ui->splitterELSResults->setStyleSheet("QSplitter::handle:hover { background-color: palette(highlight); }");
	ui->splitterSearchLog->setStyleSheet("QSplitter::handle:hover { background-color: palette(highlight); }");

	// --------------------------------

	// --- File Menu
	QMenu *pFileMenu = ui->menuBar->addMenu(tr("&File", "MainMenu"));
	QAction *pAction = nullptr;

#ifdef USING_ELSSEARCH
	pAction = pFileMenu->addAction(QIcon(":/res/gnome_window_new.png"), tr("&New ELS Search Window...", "MainMenu"), this, SLOT(newELSSearchWindow()));
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
	pAction->setStatusTip(tr("Create New ELS Search Window", "MainMenu"));
	pAction->setToolTip(tr("Create New ELS Search Window", "MainMenu"));

	pFileMenu->addSeparator();
#endif

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	m_pLoadTranscriptionAction = pFileMenu->addAction(QIcon(":/res/open-file-icon3.png"), tr("Playback Search Rec&ording File...", "MainMenu"), this, SLOT(en_openSearchTranscript()));
	m_pLoadTranscriptionAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
	m_pLoadTranscriptionAction->setStatusTip(tr("Load and Playback ELS Search Recording Transcript File", "MainMenu"));
	m_pLoadTranscriptionAction->setToolTip(tr("Load and Playback ELS Search Recording Transcript File", "MainMenu"));

	m_pCreateTranscriptionAction = pFileMenu->addAction(QIcon(":/res/save-file-icon3.png"), tr("&Start Search Recording File...", "MainMenu"), this, SLOT(en_createSearchTranscript()));
	m_pCreateTranscriptionAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
	m_pCreateTranscriptionAction->setStatusTip(tr("Start Recording Searches to ELS Search Transcript File", "MainMenu"));
	m_pCreateTranscriptionAction->setToolTip(tr("Start Recording Searches to ELS Search Transcript File", "MainMenu"));

	m_pCloseTranscriptionAction = pFileMenu->addAction(tr("Stop S&earch Recording", "MainMenu"), this, SLOT(closeSearchTranscript()));
	m_pCloseTranscriptionAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
	m_pCloseTranscriptionAction->setStatusTip(tr("Stop ELS Search Transcript Recording", "MainMenu"));
	m_pCloseTranscriptionAction->setToolTip(tr("Stop ELS Search Transcript Recording", "MainMenu"));
	m_pCloseTranscriptionAction->setEnabled(false);

	pFileMenu->addSeparator();
#endif

	// -------------

#ifdef USING_ELSSEARCH						// Add "close window" option only for KJPBS embedded ELSSearch, since there are multiple windows
	pAction = pFileMenu->addAction(QIcon(":/res/window_app_list_close.png"), tr("&Close this ELS Search Window", "MainMenu"), this, SLOT(close()));
	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));

	pFileMenu->addSeparator();
#endif

	// -------------

#ifndef USING_ELSSEARCH						// Here if "using" ELSSearch standalone:
	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit", "MainMenu"), QApplication::instance(), &QApplication::exit);
#else										// Here if "using" ELSSearch as a subcomponent of KJPBS:
	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit", "MainMenu"), g_pMyApplication.data(), SLOT(closeAllCanOpeners()));
#endif

	pAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
	pAction->setStatusTip(tr("Exit Application", "MainMenu"));
	pAction->setToolTip(tr("Exit Application", "MainMenu"));
	pAction->setMenuRole(QAction::QuitRole);
#ifdef USING_ELSSEARCH						// Here if "using" ELSSearch as a subcomponent of KJPBS:
	pAction->setEnabled(g_pMyApplication->canQuit());
	connect(g_pMyApplication.data(), SIGNAL(canQuitChanged(bool)), pAction, SLOT(setEnabled(bool)));
#endif

	// --------------------------------

	connect(ui->spinWidth, SIGNAL(valueChanged(int)), this, SLOT(en_widthSpinValueChanged(int)));
	connect(ui->spinOffset, SIGNAL(valueChanged(int)), m_pLetterMatrixTableModel, SLOT(setOffset(int)));
	connect(m_pLetterMatrixTableModel, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)), this, SLOT(en_letterMatrixLayoutAboutToChange()));
	connect(m_pLetterMatrixTableModel, SIGNAL(widthChanged(int)), this, SLOT(en_widthChanged(int)));
	connect(m_pLetterMatrixTableModel, SIGNAL(offsetChanged(int)), this, SLOT(en_offsetChanged(int)));
	connect(ui->cmbLetterCase, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedLetterCase(int)));

	connect(ui->tvLetterMatrix->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(en_letterMatrixCurrentChanged(QModelIndex,QModelIndex)));
	ui->tvLetterMatrix->setMouseTracking(true);		// Enable mouse movement changing the status message
	m_pStatusAction = new QAction(this);

	connect(ui->tvLetterMatrix, &QTableView::doubleClicked, this, &CELSSearchMainWindow::en_letterMatrixCellClicked);

	connect(ui->btnSearch, &QToolButton::clicked, this, &CELSSearchMainWindow::search);
	connect(ui->btnClear, &QToolButton::clicked, this, &CELSSearchMainWindow::clear);
	connect(ui->editWords, &QLineEdit::returnPressed, this, &CELSSearchMainWindow::search);

	connect(ui->tvELSResults, &QTableView::doubleClicked, this, &CELSSearchMainWindow::en_searchResultClicked);
	connect(ui->cmbSortOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSortOrder(int)));

	connect(ui->cmbSearchType, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSearchType(int)));

	// --------------------------------

	QShortcut *pShortcut = new QShortcut(QKeySequence::ZoomIn, ui->tvLetterMatrix);
	connect(pShortcut, &QShortcut::activated, ui->spinWidth, &QSpinBox::stepUp);
	pShortcut = new QShortcut(QKeySequence::ZoomOut, ui->tvLetterMatrix);
	connect(pShortcut, &QShortcut::activated, ui->spinWidth, &QSpinBox::stepDown);

	// --------------------------------

	ui->editWords->setFocus();

	// --------------------------------

	ui->tvELSResults->installEventFilter(this);
	ui->tvLetterMatrix->installEventFilter(this);

	// --------------------------------

	setWindowTitle(windowTitle() + " - " + pBibleDatabase->description());

	// --------------------------------

	QString strStatus;
	strStatus += m_letterMatrix.getOptionDescription(true);
	if (!strStatus.isEmpty()) {
		strStatus = tr("Matrix") + ":" + strStatus;
		QLabel *pStatusLabel = new QLabel(strStatus, this);
		ui->statusBar->addPermanentWidget(pStatusLabel);
	}
}

CELSSearchMainWindow::~CELSSearchMainWindow()
{
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	closeSearchTranscript();
#endif

	delete ui;
}

// ----------------------------------------------------------------------------

// ===========================================
// .els Search Transcript File Format Details:
// ===========================================
//
// Basic file is a zlib-stream deflated CSV file
//
// ELSFileVersion,<version>
//	File Format/Version Header
//		<version> = ELS File version
//			Example: ELSFileVersion,4
//
// Bible,<uuid>,<LMTMO>[,<LMBPO>,<LMCPO>,<LMVPO>]
//	Bible and Letter Matrix settings from CELSBibleDatabaseSelectDlg
//		<uuid> = Bible Database UUID unique identifier
//		<LMTMO> = LetterMatrixTextModifierOptions as decimal integer
//		<LMBPO> = LMBookPrologueOptions as decimal integer
//		<LMCPO> = LMChapterPrologueOptions as decimal integer
//		<LMVPO> = LMVersePrologueOptions as decimal integer
//		<LMFVTO> = LMFullVerseTextOptions as decimal integer
//			<LMBPO>,<LMCPO>,<LMVPO> are optional and was added in ELS Version 4
//			<LMFVTO> is optional and was added in ELS Version 5
//			Example: Bible,85D8A6B0-E670-11E2-A28F-0800200C9A66,0,0,1,0,1
//
// Search,<words>,<SearchType>,<MinSkip>,<MaxSkip>,<StartBook>,<EndBook>[,<CaseSensitive>]
//	Word Search Entry
//		<words> = Comma separated list of words to search (quoted in single field)
//		<SearchType> = ELS_SEARCH_TYPE_ENUM value as elsSearchTypeToID() ID string
//		<MinSkip> = Minimum skip for search as decimal integer
//		<MaxSkip> = Maximum skip for search as decimal integer
//		<StartBook> = 1-originated Bible Book Number to start the search as decimal integer
//		<EndBook> = 1-originated Bible Book Number to end the search as decimal integer
//		<CaseSensitive> = Boolean (true/false) flag to search case-sensitive WRT original text
//			<CaseSensitive> is optional and was added in ELS Version 4
//			Example: Search,roswell,ELS,3,3,1,66,false
//
// Delete,<count>,<ELSResult>
//	ELSResult Deletion Entry
//		<count> = Number of ELSResult values in array
//		<ELSResult> = New-Line separated of ELSResult entries, with each entry being
//					the unique data portion of CELSResult as a comma-separated entry as:
//					<word>,<skip>,<SearchType>,<CRelIndexExStart>,<Dir>
//						<word> = Search word for the result
//						<skip> = Skip distance word was found at
//						<SearchType> = ELS_SEARCH_TYPE_ENUM value as elsSearchTypeToID() ID string
//						<CRelIndexExStart> = Start CRelIndexEx location as 64-bit integer
//						<Dir> = Direction result was found as "Fwd" or "Rev" for LeftToRight or RightToLeft, respectively
//			Example: Delete,3,"ufo,11,ELS,80538144402833416,Fwd
//			         ","ufo,11,ELS,79699208440905732,Fwd
//			         ","ufo,11,ELS,73759685282365442,Rev
//			         "
//
// Width,<width>
//	Letter Matrix display width value
//		<width> = Matrix Width as decimal integer
//			Example: Width,4
//
// Offset,<offset>
//	Letter Matrix display offset value
//		<offset> = Matrix Offset as decimal integer
//			Example: Offset,0
//
// SortOrder,<SortOrder>
//	ELSResult output sort order
//		<SortOrder> = ELSRESULT_SORT_ORDER_ENUM value as elsresultSortOrderToLetters() ID string
//			Example: SortOrder,rws
//
// UpperCase,<UpperCase>
//	Display Letter Matrix and ELSResult words in uppercase instead of lowercase [DEPRECATED in ELS Version 4 -- USE LetterCase instead]
//		<UpperCase> = Boolean (true/false) flag with true being uppercase and false being lowercase
//			Example: Uppercase,false
//
// LetterCase,<LetterCase>
//	Display Case for Letter Matrix and ELSResult words [Replaces UpperCase, introduced in ELS Version 4]
//		<LetterCase> = LETTER_CASE_ENUM value as letterCaseToID() ID string
//			Example: LetterCase,lower
//
// MatrixTopLeftRowCol,<Row>,<Col>
//	Row and Column for the Letter Matrix display "scroll-to" position
//		<Row> = 0-originated row of the Letter Matrix for the Top-Left cell as decimal integer
//		<Col> = 0-originated column of the Letter Matrix for the Top-Left cell as decimal integer
//			Example: MatrixTopLeftRowCol,21925,0
//
//

// ----------------------------------------------------------------------------

#ifdef USING_ELSSEARCH
void CELSSearchMainWindow::newELSSearchWindow()
{
	const QList<CKJVCanOpener *> &lstCanOpeners = g_pMyApplication->canOpeners();
	Q_ASSERT(!lstCanOpeners.isEmpty());
	CKJVCanOpener *pCanOpener = lstCanOpeners.at(0);		// Shouldn't matter which one we launch from, so pick the first
	if (pCanOpener) {
		pCanOpener->launchELSSearch(QString(), LMTMO_Default, LMBPO_Default, LMCPO_Default, LMVPO_Default, LMFVTO_Default, this);
	}
}
#endif

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)

void CELSSearchMainWindow::en_openSearchTranscript(const QString &strFilePath)
{
	bool bSuccess = false;

	QString strFilePathName = strFilePath;
	if (strFilePath.isEmpty()) {
		strFilePathName = CSaveLoadFileDialog::getOpenFileName(this,
									tr("Open ELS Search Transcript File", "FileFilters"),
									g_strLastELSFilePath,
									tr("ELS Transcript Files (*.els)", "FileFilters"),
									nullptr, QFileDialog::ReadOnly);
	}
	if (!strFilePathName.isEmpty()) {
		g_strLastELSFilePath = strFilePathName;
		m_fileSearchTranscript.setFileName(strFilePathName);
		if (!m_fileSearchTranscript.open(QIODevice::ReadOnly)) {
			displayWarning(this, QApplication::applicationName(), tr("Failed to open ELS Search Transcript File!", "Errors"));
		} else {
			m_pSearchTranscriptCompressor.reset(new QtIOCompressor(&m_fileSearchTranscript));
			m_pSearchTranscriptCompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			if (!m_pSearchTranscriptCompressor->open(QIODevice::ReadOnly)) {
				displayWarning(this, QApplication::applicationName(), tr("Failed to create ELS Search Transcript Decompressor!", "Errors"));
			} else {
				m_pSearchTranscriptCSVStream.reset(
							new CCSVStream(static_cast<QIODevice *>(m_pSearchTranscriptCompressor.data())));
				bSuccess = true;
			}
		}
	}

	if (bSuccess) {
		clear();						// Clear old data before we start
		ui->spinOffset->setValue(0);	// Set Offset to zero for reading ELS that didn't have offset saved

		bool bBadELSFile = false;
		int nELSVersion = 0;

		while (!m_pSearchTranscriptCSVStream->atEnd()) {
			QStringList lstEntry;
			(*m_pSearchTranscriptCSVStream) >> lstEntry;
			if (lstEntry.isEmpty()) continue;

			QString strCommand = lstEntry.at(0);

			if (strCommand.compare("ELSFileVersion", Qt::CaseInsensitive) == 0) {
				if (lstEntry.size() != 2) {
					bBadELSFile = true;
					break;
				}
				nELSVersion = lstEntry.at(1).toInt();
				if (nELSVersion > ELS_FILE_VERSION) {
					int nResult = displayWarning(this, QApplication::applicationName(),
									tr( "This ELS Transcript File was created with a newer version of this search tool.\n\n"
										"Playback may not replicate the original search!  Continue?"), (QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
					if (nResult != QMessageBox::Yes) break;
				}
			} else if (strCommand.compare("Bible", Qt::CaseInsensitive) == 0) {
				if ((lstEntry.size() != 3) && (lstEntry.size() != 6) && (lstEntry.size() != 7)) {
					bBadELSFile = true;
					break;
				}
				QString strUUID = lstEntry.at(1);
				LetterMatrixTextModifierOptionFlags tmo = static_cast<LetterMatrixTextModifierOptionFlags>(lstEntry.at(2).toInt());		// This would use fromInt(), but that needs Qt 6.2+
				LMBookPrologueOptionFlags bpo = (lstEntry.size() >= 6) ? static_cast<LMBookPrologueOptionFlags>(lstEntry.at(3).toInt()) : LMBPO_Default;		// This would use fromInt(), but that needs Qt 6.2+
				LMChapterPrologueOptionFlags cpo = (lstEntry.size() >= 6) ? static_cast<LMChapterPrologueOptionFlags>(lstEntry.at(4).toInt()) : LMCPO_NumbersRoman;	// This would use fromInt(), but that needs Qt 6.2+ (Roman for backward compat with old file format)
				LMVersePrologueOptionFlags vpo = (lstEntry.size() >= 6) ? static_cast<LMVersePrologueOptionFlags>(lstEntry.at(5).toInt()) : LMVPO_Default;		// This would use fromInt(), but that needs Qt 6.2+
				LMFullVerseTextOptionFlags fvto = (lstEntry.size() >= 7) ? static_cast<LMFullVerseTextOptionFlags>(lstEntry.at(6).toInt()) : LMFVTO_Default;	// This would use fromInt(), but that needs Qt 6.2+
				if ((bibleDatabase()->compatibilityUUID().compare(strUUID, Qt::CaseInsensitive) != 0) ||
					(tmo != m_letterMatrix.textModifierOptions()) ||
					(bpo != m_letterMatrix.bookPrologueOptions()) ||
					(cpo != m_letterMatrix.chapterPrologueOptions()) ||
					(vpo != m_letterMatrix.versePrologueOptions()) ||
					(fvto != m_letterMatrix.fullVerseTextOptions())) {
#ifdef USING_ELSSEARCH
					const QList<CKJVCanOpener *> &lstCanOpeners = g_pMyApplication->canOpeners();
					Q_ASSERT(!lstCanOpeners.isEmpty());
					CKJVCanOpener *pCanOpener = lstCanOpeners.at(0);		// Shouldn't matter which one we launch from, so pick the first
					if (pCanOpener) {
						int nResult = displayWarning(this, QApplication::applicationName(),
										tr(	"This ELS Transcript File was created using a Different Bible Database and/or Text Modifier Options.\n\n"
											"Do you want to launch a new search window with those settings??"), (QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
						if (nResult == QMessageBox::Yes) {
							CELSSearchMainWindow *pNewELSSearch = pCanOpener->launchELSSearch(strUUID, tmo, bpo, cpo, vpo, fvto, this);
							if (!pNewELSSearch) break;
							// Launch the search in the new window after we've closed and exited here:
							QTimer::singleShot(1, pNewELSSearch, [pNewELSSearch, strFilePathName]()->void {
								pNewELSSearch->en_openSearchTranscript(strFilePathName);
							});
							break;
						}
						nResult = displayWarning(this, QApplication::applicationName(),
										tr( "Continue using this search window??\n\n"
											"The playback results will not be same as the original search!"), (QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
						if (nResult != QMessageBox::Yes) break;
					} else {
						int nResult = displayWarning(this, QApplication::applicationName(),
										tr(	"This ELS Transcript File was created using a Different Bible Database and/or Text Modifier Options.\n\n"
											"The playback results will not be same as the original search!  Continue?"), (QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
						if (nResult != QMessageBox::Yes) break;
					}
#else
					int nResult = displayWarning(this, QApplication::applicationName(),
									tr(	"This ELS Transcript File was created using a Different Bible Database and/or Text Modifier Options.\n\n"
										"The playback results will not be same as the original search!  Continue?"), (QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
					if (nResult != QMessageBox::Yes) break;
#endif
				}
			} else if (strCommand.compare("Width", Qt::CaseInsensitive) == 0) {
				if (lstEntry.size() != 2) {
					bBadELSFile = true;
					break;
				}
				ui->spinWidth->setValue(lstEntry.at(1).toInt());
			} else if (strCommand.compare("Offset", Qt::CaseInsensitive) == 0) {
				if (lstEntry.size() != 2) {
					bBadELSFile = true;
					break;
				}
				ui->spinOffset->setValue(lstEntry.at(1).toInt());
			} else if (strCommand.compare("SortOrder", Qt::CaseInsensitive) == 0) {
				if (lstEntry.size() != 2) {
					bBadELSFile = true;
					break;
				}
				ELSRESULT_SORT_ORDER_ENUM nSortOrder = elsresultSortOrderFromLetters(lstEntry.at(1));
				if (nSortOrder < ESO_COUNT) {
					int nIndex = ui->cmbSortOrder->findData(nSortOrder);
					if (nIndex >= 0) ui->cmbSortOrder->setCurrentIndex(nIndex);
				} else {
					bBadELSFile = true;
					break;
				}
			} else if (strCommand.compare("Uppercase", Qt::CaseInsensitive) == 0) {			// Deprecated -- Replaced by LetterCase
				if (lstEntry.size() != 2) {
					bBadELSFile = true;
					break;
				}
				int nIndex = ui->cmbLetterCase->findData(QVariant(lstEntry.at(1)).toBool() ? LCE_UPPER : LCE_LOWER);
				if (nIndex >= 0) ui->cmbLetterCase->setCurrentIndex(nIndex);
			} else if (strCommand.compare("LetterCase", Qt::CaseInsensitive) == 0) {
				if (lstEntry.size() != 2) {
					bBadELSFile = true;
					break;
				}
				LETTER_CASE_ENUM nLetterCase = letterCaseFromID(lstEntry.at(1));
				int nIndex = ui->cmbLetterCase->findData(nLetterCase);
				if (nIndex >= 0) {
					ui->cmbLetterCase->setCurrentIndex(nIndex);
				} else {
					bBadELSFile = true;
					break;
				}
			} else if (strCommand.compare("MatrixTopLeftRowCol", Qt::CaseInsensitive) == 0) {
				if (lstEntry.size() != 3) {
					bBadELSFile = true;
					break;
				}
				int nRow = lstEntry.at(1).toInt();
				int nCol = lstEntry.at(2).toInt();
				uint32_t nMatrixIndex = m_pLetterMatrixTableModel->matrixIndexFromRowCol(nRow, nCol);
				if (nMatrixIndex) {
					QModelIndex index = m_pLetterMatrixTableModel->modelIndexFromMatrixIndex(nMatrixIndex);
					ui->tvLetterMatrix->scrollTo(index, QAbstractItemView::PositionAtTop);
				}
			} else if (strCommand.compare("Search", Qt::CaseInsensitive) == 0) {
				if ((lstEntry.size() != 7) && (lstEntry.size() != 8)) {
					bBadELSFile = true;
					break;
				}
				ELS_SEARCH_TYPE_ENUM nSearchType = elsSearchTypeFromID(lstEntry.at(2));
				if (nSearchType < ESTE_COUNT) {
					int nIndexSearchType = ui->cmbSearchType->findData(nSearchType);
					int nIndexBookStart = ui->cmbBookStart->findData(lstEntry.at(5).toUInt());
					int nIndexBookEnd = ui->cmbBookEnd->findData(lstEntry.at(6).toUInt());
					bool bCaseSensitive = false;
					if (lstEntry.size() >= 8) bCaseSensitive = QVariant(lstEntry.at(7)).toBool();
					ui->chkCaseSensitive->setChecked(bCaseSensitive);
					if ((nIndexSearchType >= 0) &&
						(nIndexBookStart >= 0) &&
						(nIndexBookEnd >= 0)) {
						ui->editWords->setText(lstEntry.at(1));
						ui->cmbSearchType->setCurrentIndex(nIndexSearchType);
						ui->spinMinSkip->setValue(lstEntry.at(3).toInt());
						ui->spinMaxSkip->setValue(lstEntry.at(4).toInt());
						ui->cmbBookStart->setCurrentIndex(nIndexBookStart);
						ui->cmbBookEnd->setCurrentIndex(nIndexBookEnd);
						if (!search()) break;				// Perform search and break if user cancelled
					} else {
						if (nIndexSearchType < 0) {
							displayWarning(this, QApplication::applicationName(),
								tr("Unable to find the Search Type \"%1\".  Skipping search for \"%2\".")
								.arg(lstEntry.at(2)).arg(lstEntry.at(1)));
						} else {
							displayWarning(this, QApplication::applicationName(),
								tr("Unable to find the Book Start (%1) and/or Book End (%2) indexes.  Skipping search for \"%3\".")
								.arg(lstEntry.at(5)).arg(lstEntry.at(6)).arg(lstEntry.at(1)));
						}
					}
				}
			} else if (strCommand.compare("Delete", Qt::CaseInsensitive) == 0) {
				if (lstEntry.size() < 2) {
					bBadELSFile = true;
					break;
				}
				qsizetype nNumDeletions = lstEntry.at(1).toULongLong();
				if (lstEntry.size() != (nNumDeletions + 2)) {
					bBadELSFile = true;
					break;
				}
				CELSResultList lstResultsToRemove;
				for (qsizetype ndx = 0; ndx < nNumDeletions; ++ndx) {
					QString strResult = lstEntry.at(ndx + 2);
					CCSVStream csvResult(&strResult, QIODevice::ReadOnly);
					QStringList lstResult;
					csvResult >> lstResult;
					if (lstResult.size() != 5) {
						bBadELSFile = true;
						break;
					}
					CELSResult result;
					result.m_strWord = lstResult.at(0);
					result.m_nSkip = lstResult.at(1).toInt();
					result.m_nSearchType = elsSearchTypeFromID(lstResult.at(2));
					result.m_ndxStart = CRelIndexEx(lstResult.at(3).toULongLong());
					if (lstResult.at(4).compare("Fwd", Qt::CaseInsensitive) == 0) {
						result.m_nDirection = Qt::LeftToRight;
					} else if (lstResult.at(4).compare("Rev", Qt::CaseInsensitive) == 0) {
						result.m_nDirection = Qt::RightToLeft;
					} else {
						bBadELSFile = true;
						break;
					}
					lstResultsToRemove.append(result);
				}
				if (bBadELSFile) break;
				m_pELSResultListModel->deleteSearchResults(lstResultsToRemove);
			} else {
				if (nELSVersion <= ELS_FILE_VERSION) {		// Bad/Unknown command for this version of ELS, including backward compat
					bBadELSFile = true;
					break;
				}
			}
		}

		if (bBadELSFile) {
			displayWarning(this, QApplication::applicationName(),
						   tr("This ELS Transcript File is bad or contains corrupt data and can't be processed."));
		}
	}

	closeSearchTranscript();
}

void CELSSearchMainWindow::en_createSearchTranscript()
{
	bool bSuccess = false;

	QString strFilePathName = CSaveLoadFileDialog::getSaveFileName(this,
										tr("Save ELS Search Transcript File", "FileFilters"),
										g_strLastELSFilePath,
										tr("ELS Transcript Files (*.els)", "FileFilters"),
										"els", nullptr, QFileDialog::Options());
	if (!strFilePathName.isEmpty()) {
		g_strLastELSFilePath = strFilePathName;
		m_fileSearchTranscript.setFileName(strFilePathName);
		if (!m_fileSearchTranscript.open(QIODevice::WriteOnly)) {
			displayWarning(this, QApplication::applicationName(), tr("Failed to create ELS Search Transcript File!", "Errors"));
		} else {
			m_pSearchTranscriptCompressor.reset(new QtIOCompressor(&m_fileSearchTranscript));
			m_pSearchTranscriptCompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
			if (!m_pSearchTranscriptCompressor->open(QIODevice::WriteOnly)) {
				displayWarning(this, QApplication::applicationName(), tr("Failed to create ELS Search Transcript Compressor!", "Errors"));
			} else {
				m_pSearchTranscriptCSVStream.reset(
					new CCSVStream(static_cast<QIODevice *>(m_pSearchTranscriptCompressor.data())));
				bSuccess = true;
			}
		}
	}

	if (bSuccess) {
		clear();						// Clear old data before we start

		m_bRecordingTranscript = true;

		// Write ELS File Version:
		(*m_pSearchTranscriptCSVStream) << QStringList{ "ELSFileVersion", QString::number(ELS_FILE_VERSION) };

		// Write Bible Source Text Identifier:
		(*m_pSearchTranscriptCSVStream) << QStringList{ "Bible", bibleDatabase()->compatibilityUUID(),
														QString::number(m_letterMatrix.textModifierOptions()),
														QString::number(m_letterMatrix.bookPrologueOptions()),
														QString::number(m_letterMatrix.chapterPrologueOptions()),
														QString::number(m_letterMatrix.versePrologueOptions()),
														QString::number(m_letterMatrix.fullVerseTextOptions()), };

		// Disable Load/Create and Change "Clear" to "Close":
		m_pLoadTranscriptionAction->setEnabled(false);
		m_pCreateTranscriptionAction->setEnabled(false);
		m_pCloseTranscriptionAction->setEnabled(true);
		ui->btnClear->setText(tr("Stop Re&cording"));
	} else {
		closeSearchTranscript();
	}
}

void CELSSearchMainWindow::closeSearchTranscript()
{
	if (m_bRecordingTranscript) {
		Q_ASSERT(!m_pSearchTranscriptCSVStream.isNull());

		// Write GUI Setting information so layout can be reconstructed:
		(*m_pSearchTranscriptCSVStream) << QStringList{ "Width", QString::number(ui->spinWidth->value()) };
		(*m_pSearchTranscriptCSVStream) << QStringList{ "Offset", QString::number(ui->spinOffset->value()) };
		(*m_pSearchTranscriptCSVStream) << QStringList{ "SortOrder", elsresultSortOrderToLetters(
									 static_cast<ELSRESULT_SORT_ORDER_ENUM>(ui->cmbSortOrder->currentData().toInt())) };
		(*m_pSearchTranscriptCSVStream) << QStringList{ "LetterCase", letterCaseToID(ui->cmbLetterCase->currentData().value<LETTER_CASE_ENUM>()) };
		(*m_pSearchTranscriptCSVStream) << QStringList{ "MatrixTopLeftRowCol",
													 QString::number(ui->tvLetterMatrix->rowAt(0)),
													 QString::number(ui->tvLetterMatrix->columnAt(0)) };
	}

	// Close File objects:
	m_pSearchTranscriptCSVStream.reset();
	m_pSearchTranscriptCompressor.reset();
	if (m_fileSearchTranscript.isOpen()) m_fileSearchTranscript.close();

	// Enable Load/Create and Change "Close" to "Clear":
	m_pLoadTranscriptionAction->setEnabled(true);
	m_pCreateTranscriptionAction->setEnabled(true);
	m_pCloseTranscriptionAction->setEnabled(false);
	ui->btnClear->setText(tr("&Clear"));

	m_bRecordingTranscript = false;
}

#endif	// !defined(EMSCRIPTEN) && !defined(VNCSERVER)

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::en_searchResultClicked(const QModelIndex &index)
{
	CRelIndexEx ndx =  m_pELSResultListModel->data(index, CELSResultListModel::UserRole_Reference).value<CRelIndexEx>();
	uint32_t matrixIndex = m_letterMatrix.matrixIndexFromRelIndex(ndx);
	if (matrixIndex) ui->tvLetterMatrix->scrollTo(m_pLetterMatrixTableModel->modelIndexFromMatrixIndex(matrixIndex), QAbstractItemView::PositionAtCenter);
}

void CELSSearchMainWindow::en_changedSortOrder(int nIndex)
{
	m_pELSResultListModel->setSortOrder(static_cast<ELSRESULT_SORT_ORDER_ENUM>(ui->cmbSortOrder->itemData(nIndex).toInt()));
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::en_letterMatrixLayoutAboutToChange()
{
	QRect rcTableView = ui->tvLetterMatrix->rect();
	int nRow = ui->tvLetterMatrix->rowAt(rcTableView.height()/2);
	int nCol = ui->tvLetterMatrix->columnAt(0) + m_pLetterMatrixTableModel->columnCount()/2;
	m_nMatrixIndexToCenter = m_pLetterMatrixTableModel->matrixIndexFromRowCol(nRow, nCol);
}

void CELSSearchMainWindow::en_widthChanged(int nWidth)
{
	Q_UNUSED(nWidth);
	if (m_nMatrixIndexToCenter) ui->tvLetterMatrix->scrollTo(m_pLetterMatrixTableModel->modelIndexFromMatrixIndex(m_nMatrixIndexToCenter), QAbstractItemView::PositionAtCenter);
}

void CELSSearchMainWindow::en_offsetChanged(int nOffset)
{
	Q_UNUSED(nOffset);
}

void CELSSearchMainWindow::en_widthSpinValueChanged(int nWidth)
{
	ui->spinOffset->setMaximum(nWidth-1);
	m_pLetterMatrixTableModel->setWidth(nWidth);		// This will automatically cause a en_widthChanged() event
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::en_letterMatrixCellClicked(const QModelIndex &index)
{
	const CELSResultSet &setResults = m_pLetterMatrixTableModel->resultsSet(index);

	if (!setResults.isEmpty()) {
		QItemSelectionModel *pSelModel = ui->tvELSResults->selectionModel();
		Q_ASSERT(pSelModel != nullptr);

		QModelIndexList lstIndexes = m_pELSResultListModel->getResultIndexes(setResults);
		Q_ASSERT(lstIndexes.size() == setResults.size());
		pSelModel->clearSelection();
		for (auto const & ndx : lstIndexes) {
			pSelModel->select(ndx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			ui->tvELSResults->scrollTo(ndx);
		}
	}
}

void CELSSearchMainWindow::en_letterMatrixCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous);

	if (current.isValid()) {
		QString strStatusTip = current.data(Qt::StatusTipRole).toString();
		ui->tvLetterMatrix->setStatusTip(strStatusTip);
		m_pStatusAction->setStatusTip(strStatusTip);
		m_pStatusAction->showStatusText();
	}
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::en_changedLetterCase(int nIndex)
{
	m_pLetterMatrixTableModel->setLetterCase(ui->cmbLetterCase->itemData(nIndex).value<LETTER_CASE_ENUM>());
	m_pELSResultListModel->setLetterCase(ui->cmbLetterCase->itemData(nIndex).value<LETTER_CASE_ENUM>());
}

// ----------------------------------------------------------------------------

void CELSSearchMainWindow::en_changedSearchType(int nIndex)
{
	if ((ui->cmbSearchType->itemData(nIndex).toInt() == ESTE_ELS) ||
		(ui->cmbSearchType->itemData(nIndex).toInt() == ESTE_FLS)) {
		ui->spinMinSkip->setEnabled(true);
		ui->spinMaxSkip->setEnabled(true);
		ui->lblMinSkip->setVisible(true);
		ui->spinMinSkip->setVisible(true);
		ui->lblMaxSkip->setVisible(true);
		ui->spinMaxSkip->setVisible(true);
		if (ui->cmbSearchType->itemData(nIndex).toInt() == ESTE_ELS) {
			ui->lblMinSkip->setText(tr("Mi&nSkip:", "CELSSearchMainWindow"));
			ui->lblMaxSkip->setText(tr("Ma&xSkip:", "CELSSearchMainWindow"));

			ui->spinMinSkip->setMinimum(0);
			ui->spinMaxSkip->setMinimum(0);
		} else {
			ui->lblMinSkip->setText(tr("Mi&nMult:", "CELSSearchMainWindow"));
			ui->lblMaxSkip->setText(tr("Ma&xMult:", "CELSSearchMainWindow"));

			ui->spinMinSkip->setMinimum(1);
			ui->spinMaxSkip->setMinimum(1);
		}
	} else {
		ui->spinMinSkip->setEnabled(false);
		ui->spinMaxSkip->setEnabled(false);
		ui->lblMinSkip->setVisible(false);
		ui->spinMinSkip->setVisible(false);
		ui->lblMaxSkip->setVisible(false);
		ui->spinMaxSkip->setVisible(false);
	}
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

bool CELSSearchMainWindow::search()
{
	static const QRegularExpression regExWordSplit = QRegularExpression("[\\s,]+");
	QStringList lstSearchWords = ui->editWords->text().split(regExWordSplit, Qt::SkipEmptyParts);

	// Remove any words that aren't at least 2 letters.  This is needed
	//	for the skip logic, plus searching for single letters is ridiculous:
	for (int ndx = lstSearchWords.size()-1; ndx >= 0; --ndx) {
		if (lstSearchWords.at(ndx).size() < 2) lstSearchWords.removeAt(ndx);
	}
	if (lstSearchWords.isEmpty()) return false;

	ELS_SEARCH_TYPE_ENUM nSearchType = ui->cmbSearchType->currentData().value<ELS_SEARCH_TYPE_ENUM>();
	CFindELS elsFinder(m_pLetterMatrixTableModel->matrix(), lstSearchWords, nSearchType, ui->chkCaseSensitive->isChecked());
	if (!elsFinder.setBookEnds(ui->cmbBookStart->currentData().toUInt(), ui->cmbBookEnd->currentData().toUInt())) {
		Q_ASSERT(false);
		displayWarning(this, QApplication::applicationName(), tr("Failed to set Book Range!"));
		return false;
	}
	unsigned int nBookStart = elsFinder.bookStart();
	unsigned int nBookEnd = elsFinder.bookEnd();

	int nMinSkip = ui->spinMinSkip->value();
	int nMaxSkip = ui->spinMaxSkip->value();
	if (nSearchType != ESTE_ELS) {
		if (nMinSkip < 1) nMinSkip = 1;
		if (nMaxSkip < 1) nMaxSkip = 1;
	} else {
		if (nMinSkip < 0) nMinSkip = 0;
		if (nMaxSkip < 0) nMaxSkip = 0;
	}
	if (nMaxSkip < nMinSkip) std::swap(nMinSkip, nMaxSkip);

	QProgressDialog dlgProgress;
	dlgProgress.setLabelText(tr("Searching for") + ": " + lstSearchWords.join(','));

	insertSearchLogText(tr("Searching for") + ": " + lstSearchWords.join(',') +
						" (" + (ui->chkCaseSensitive->isChecked() ? tr("Case-Sensitive") : tr("Case-Insensitive")) + ")");
	insertSearchLogText(tr("Search Type") + ": " + elsSearchTypeDescription(nSearchType));

	QElapsedTimer elapsedTime;
	elapsedTime.start();

	QFutureWatcher<CELSResultList> watcher;
	connect(&watcher, &QFutureWatcher<CELSResultList>::finished, &dlgProgress, &QProgressDialog::reset);
	connect(&dlgProgress, &QProgressDialog::canceled, &watcher, &QFutureWatcher<CELSResultList>::cancel);
	connect(&watcher, &QFutureWatcher<CELSResultList>::progressRangeChanged, &dlgProgress, &QProgressDialog::setRange);
	connect(&watcher, &QFutureWatcher<CELSResultList>::progressValueChanged, &dlgProgress, &QProgressDialog::setValue);

	watcher.setFuture(elsFinder.future(nMinSkip, nMaxSkip, &CFindELS::reduce));
	dlgProgress.exec();
	CBusyCursor iAmBusy(this);
	watcher.waitForFinished();

	bool bWasCanceled = watcher.future().isCanceled();
	if (!bWasCanceled) {
		CELSResultList lstResults = watcher.result();
		m_pLetterMatrixTableModel->setSearchResults(lstResults);
		m_pELSResultListModel->setSearchResults(lstResults);

		insertSearchLogText(tr("Search Time: %1 secs").arg(elapsedTime.elapsed() / 1000.0));

		QString strBookRange;
		if ((nBookStart == 1) && (nBookEnd == m_letterMatrix.bibleDatabase()->bibleEntry().m_nNumBk)) {
			strBookRange = "Entire Bible";
		} else {
			strBookRange = tr("%1 through %2")
							   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookStart, 0, 0, 0)))
							   .arg(m_letterMatrix.bibleDatabase()->bookName(CRelIndex(nBookEnd, 0, 0, 0)));
		}
		strBookRange += m_letterMatrix.getOptionDescription(true);

		if (nSearchType == ESTE_ELS) {
			insertSearchLogText(tr("Searching for ELS skips from %1 to %2 in %3")
											.arg(nMinSkip)
											.arg(nMaxSkip)
											.arg(strBookRange));
		} else if (nSearchType == ESTE_FLS) {
			insertSearchLogText(tr("Searching with FLS multipliers of %1 to %2 in %3")
									.arg(nMinSkip)
									.arg(nMaxSkip)
									.arg(strBookRange));
		} else {
			insertSearchLogText(tr("Searching in %3").arg(strBookRange));
		}

		insertSearchLogText(tr("Found %1 Results\n").arg(lstResults.size()));

		ui->tvELSResults->resizeColumnsToContents();
		ui->tvELSResults->adjustSize();

		// Should we clear editWords here?

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		// Record transcript only if search was completed:
		if (m_bRecordingTranscript) {
			Q_ASSERT(!m_pSearchTranscriptCSVStream.isNull());
			(*m_pSearchTranscriptCSVStream) << QStringList{
				"Search",
				lstSearchWords.join(","),
				elsSearchTypeToID(nSearchType),
				QString::number(nMinSkip),
				QString::number(nMaxSkip),
				QString::number(nBookStart),
				QString::number(nBookEnd),
				QVariant(ui->chkCaseSensitive->isChecked()).toString(),
			};
		}
#endif

	} else {
		insertSearchLogText(tr("Search was cancelled by user\n"));
	}

	insertSearchLogText("----------------------------------------");

	return !bWasCanceled;
}

void CELSSearchMainWindow::clear()
{
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	if (m_bRecordingTranscript) {
		closeSearchTranscript();
		return;
	}
#endif

	m_pLetterMatrixTableModel->clearSearchResults();
	m_pELSResultListModel->clearSearchResults();
	clearSearchLogText();
	ui->editWords->clear();
	ui->editWords->setFocus();
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
			} else if (pKEvent->matches(QKeySequence::Delete)) {	//			Delete
				en_deleteSearchResults();
				pKEvent->accept();
				return true;
			} else if ((pKEvent->key() == Qt::Key_Return) || (pKEvent->key() == Qt::Key_Enter)) {
				en_searchResultClicked(ui->tvELSResults->currentIndex());
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

	pAction = pMenu->addAction(QIcon::fromTheme("edit-delete"), tr("&Delete", "tvELSResults") + ACCEL_KEY(QKeySequence::Delete), this, SLOT(en_deleteSearchResults()));
	pAction->setObjectName("edit-delete");
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

void CELSSearchMainWindow::en_deleteSearchResults()
{
	// Delete results selected in the ELSResult view:
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	CELSResultList lstResultsRemoved =
#endif
		m_pELSResultListModel->deleteSearchResults(ui->tvELSResults->selectionModel()->selectedIndexes());

	// Blow away the matrix results and recompute them, since it
	//	keeps running totals for overlaps:
	m_pLetterMatrixTableModel->clearSearchResults();
	m_pLetterMatrixTableModel->setSearchResults(m_pELSResultListModel->results());

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	// Record deletion if recording is active:
	if (m_bRecordingTranscript) {
		Q_ASSERT(!m_pSearchTranscriptCSVStream.isNull());
		QStringList lstDeletion = QStringList{
			"Delete",
			QString::number(lstResultsRemoved.size())
		};
		for (auto const & result : lstResultsRemoved) {
			QString strResult;
			CCSVStream csvResult(&strResult, QIODevice::WriteOnly);

			csvResult << QStringList {
				result.m_strWord,
				QString::number(result.m_nSkip),
				elsSearchTypeToID(result.m_nSearchType),
				QString::number(result.m_ndxStart.indexEx()),
				(result.m_nDirection == Qt::LeftToRight) ? QString("Fwd") : QString("Rev")
			};

			lstDeletion << strResult;
		}
		(*m_pSearchTranscriptCSVStream) << lstDeletion;
	}
#endif


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
