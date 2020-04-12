/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

#include "KJVSearchResult.h"

#include "ReportError.h"
#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "KJVPassageNavigatorDlg.h"
#include "Highlighter.h"
#include "KJVCanOpener.h"
#include "ReflowDelegate.h"
#include "PersistentSettings.h"
#include "NoteKeywordWidget.h"
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
#include "KJVNoteEditDlg.h"
#include "KJVCrossRefEditDlg.h"
#endif
#include "SearchCompleter.h"
#include "BusyCursor.h"
#include "PhraseEdit.h"

#include "myApplication.h"

#include <assert.h>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QMenu>
#include <QKeySequence>
#include <QLabel>
#include <QItemSelection>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QToolTip>
#include <ToolTipEdit.h>
#include <QDrag>
#include <QPainter>
#include <QStyle>
#include "Qt_QStyleOption_stub.h"
#include <QList>
#include <QPair>
#include <QMessageBox>
#include <QScrollBar>

#ifdef USING_QT_SPEECH
#include <QtSpeech>
#endif

#ifdef MODELTEST
#include <modeltest.h>
#endif

#ifdef TOUCH_GESTURE_PROCESSING
#include <QGestureEvent>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <QPanGesture>
#include <QSwipeGesture>

#include <QScroller>
#include <QScrollerProperties>
#endif

#ifdef WORKAROUND_QTBUG_33906
// Workaround for QTBUG-33906:
// Qt5 redefines scroll by pixel to be by single pixel.  This
//		defines how many lines we want to scroll the vertical
//		scrollbar on a per-step
#define LINES_PER_SCROLL_BLOCK 4
#endif

// ============================================================================

CSearchResultsTreeView::CSearchResultsTreeView(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent)
	:	QTreeView(parent),
#ifdef TOUCH_GESTURE_PROCESSING
		m_bDoubleTouchStarted(false),
		m_nAccumulatedScrollOffset(0),
#endif
		m_bInvertTextBrightness(false),
		m_nTextBrightness(100),
		m_bDoingPopup(false),
		m_pEditMenu(NULL),
		m_pEditMenuLocal(NULL),
		m_pActionCopyVerseEntry(NULL),
		m_pActionCopyVerseText(NULL),
		m_pActionCopyRaw(NULL),
		m_pActionCopyVeryRaw(NULL),
		m_pActionCopyVerseHeadings(NULL),
		m_pActionCopyReferenceDetails(NULL),
		m_pActionCopyComplete(NULL),
		m_pActionSelectAll(NULL),
		m_pActionClearSelection(NULL),
		m_pMenuInsertionPoint(NULL),
		m_pMenuUserNotesInsertionPoint(NULL),
		m_pActionNavigator(NULL),
		m_pStatusAction(NULL),
		m_pReflowDelegate(NULL),
		m_pParentCanOpener(NULL)
{
	assert(!pBibleDatabase.isNull());
	assert(!pUserNotesDatabase.isNull());

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_RESULTS_SPY
	CMyApplication::createSpy(this);
#endif
#endif

	setMouseTracking(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	// setRootIsDecorated(false);		// Set below based on vlmodel() type
	setExpandsOnDoubleClick(false);
	setProperty("isWrapping", QVariant(false));
	header()->setVisible(false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	en_changedScrollbarsEnabled(CPersistentSettings::instance()->scrollbarsEnabled());
	connect(CPersistentSettings::instance(), SIGNAL(changedScrollbarsEnabled(bool)), this, SLOT(en_changedScrollbarsEnabled(bool)));

#ifdef TOUCH_GESTURE_PROCESSING
	grabGesture(Qt::TapGesture);
	grabGesture(Qt::TapAndHoldGesture);
	grabGesture(Qt::PanGesture);
	grabGesture(Qt::SwipeGesture);

// The following is for QTouchEvent:
//	viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);

	m_dlyDoubleTouch.setMinimumDelay(QApplication::doubleClickInterval());
	connect(&m_dlyDoubleTouch, SIGNAL(triggered()), this, SLOT(en_doubleTouchTimeout()));

	QScroller *pScroller = QScroller::scroller(viewport());

	QScrollerProperties scrollerProps = pScroller->scrollerProperties();

//	qDebug("Scroller Properties for SearchResults:");
//	qDebug("MousePressEventDelay: %lf", scrollerProps.scrollMetric(QScrollerProperties::MousePressEventDelay).toDouble());
//	qDebug("DragStartDistance: %lf", scrollerProps.scrollMetric(QScrollerProperties::DragStartDistance).toDouble());
//	qDebug("DragVelocitySmoothingFactor: %lf", scrollerProps.scrollMetric(QScrollerProperties::DragVelocitySmoothingFactor).toDouble());
//	qDebug("AxisLockThreshold: %lf", scrollerProps.scrollMetric(QScrollerProperties::AxisLockThreshold).toDouble());
//	qDebug("ScrollingCurve: %lf", scrollerProps.scrollMetric(QScrollerProperties::ScrollingCurve).toDouble());
//	qDebug("DecelerationFactor: %lf", scrollerProps.scrollMetric(QScrollerProperties::DecelerationFactor).toDouble());
//	qDebug("MinimumVelocity: %lf", scrollerProps.scrollMetric(QScrollerProperties::MinimumVelocity).toDouble());
//	qDebug("MaximumVelocity: %lf", scrollerProps.scrollMetric(QScrollerProperties::MaximumVelocity).toDouble());
//	qDebug("MaximumClickThroughVelocity: %lf", scrollerProps.scrollMetric(QScrollerProperties::MaximumClickThroughVelocity).toDouble());
//	qDebug("AcceleratingFlickMaximumTime: %lf", scrollerProps.scrollMetric(QScrollerProperties::AcceleratingFlickMaximumTime).toDouble());
//	qDebug("AcceleratingFlickSpeedupFactor: %lf", scrollerProps.scrollMetric(QScrollerProperties::AcceleratingFlickSpeedupFactor).toDouble());
//	qDebug("SnapPositionRatio: %lf", scrollerProps.scrollMetric(QScrollerProperties::SnapPositionRatio).toDouble());
//	qDebug("SnapTime: %lf", scrollerProps.scrollMetric(QScrollerProperties::SnapTime).toDouble());
//	qDebug("OvershootDragResistanceFactor: %lf", scrollerProps.scrollMetric(QScrollerProperties::OvershootDragResistanceFactor).toDouble());
//	qDebug("OvershootDragDistanceFactor: %lf", scrollerProps.scrollMetric(QScrollerProperties::OvershootDragDistanceFactor).toDouble());
//	qDebug("OvershootScrollDistanceFactor: %lf", scrollerProps.scrollMetric(QScrollerProperties::OvershootScrollDistanceFactor).toDouble());
//	qDebug("OvershootScrollTime: %lf", scrollerProps.scrollMetric(QScrollerProperties::OvershootScrollTime).toDouble());
//	qDebug("HorizontalOvershootPolicy: %lf", scrollerProps.scrollMetric(QScrollerProperties::HorizontalOvershootPolicy).toDouble());
//	qDebug("VerticalOvershootPolicy: %lf", scrollerProps.scrollMetric(QScrollerProperties::VerticalOvershootPolicy).toDouble());
//	qDebug("FrameRate: %lf", scrollerProps.scrollMetric(QScrollerProperties::FrameRate).toDouble());
//	qDebug("ScrollMetricCount: %lf", scrollerProps.scrollMetric(QScrollerProperties::ScrollMetricCount).toDouble());
//
//	======================================
//	Default values from my Google Nexus 7:
//	======================================
//	MousePressEventDelay: 0.250000
//	DragStartDistance: 0.005000
//	DragVelocitySmoothingFactor: 0.800000
//	AxisLockThreshold: 0.000000
//	ScrollingCurve: 0.000000
//	DecelerationFactor: 0.125000
//	MinimumVelocity: 0.050000
//	MaximumVelocity: 0.500000
//	MaximumClickThroughVelocity: 0.066500
//	AcceleratingFlickMaximumTime: 1.250000
//	AcceleratingFlickSpeedupFactor: 3.000000
//	SnapPositionRatio: 0.500000
//	SnapTime: 0.300000
//	OvershootDragResistanceFactor: 0.500000
//	OvershootDragDistanceFactor: 1.000000
//	OvershootScrollDistanceFactor: 0.500000
//	OvershootScrollTime: 0.700000
//	HorizontalOvershootPolicy: 0.000000
//	VerticalOvershootPolicy: 0.000000
//	FrameRate: 0.000000
//	ScrollMetricCount: 0.000000

	scrollerProps.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.800);
	scrollerProps.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.500);
	scrollerProps.setScrollMetric(QScrollerProperties::AxisLockThreshold, 0.66);
	scrollerProps.setScrollMetric(QScrollerProperties::ScrollingCurve, QEasingCurve(QEasingCurve::OutQuad));
	scrollerProps.setScrollMetric(QScrollerProperties::DecelerationFactor, 0.05);
	scrollerProps.setScrollMetric(QScrollerProperties::MaximumVelocity, 1.0 /*0.635*/);
	scrollerProps.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.33);
	scrollerProps.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.33);
	scrollerProps.setScrollMetric(QScrollerProperties::SnapPositionRatio, 0.93);
	scrollerProps.setScrollMetric(QScrollerProperties::DragStartDistance, 0.001);

	pScroller->setScrollerProperties(scrollerProps);

//	pScroller->grabGesture(this, QScroller::TouchGesture);
	pScroller->grabGesture(this, QScroller::LeftMouseButtonGesture);
#endif

	// Setup our Context Menu:
	m_pEditMenu = new QMenu(tr("&Edit", "MainMenu"), this);
	m_pEditMenuLocal = new QMenu(tr("&Edit", "MainMenu"), this);
	m_pEditMenu->setStatusTip(tr("Search Results Edit Operations", "MainMenu"));
	// ----
	m_pActionCopyVerseEntry = m_pEditMenu->addAction(tr("Copy &Entries", "MainMenu"), this, SLOT(en_copyVerseEntry()), QKeySequence(Qt::CTRL + Qt::Key_T));
	m_pActionCopyVerseEntry->setStatusTip(tr("Copy Entries for the selected Search Results to the clipboard", "MainMenu"));
	m_pActionCopyVerseEntry->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVerseEntry);
	m_pActionCopyVerseText = m_pEditMenu->addAction(tr("Copy &Verse Text", "MainMenu"), this, SLOT(en_copyVerseText()), QKeySequence(Qt::CTRL + Qt::Key_V));
	m_pActionCopyVerseText->setStatusTip(tr("Copy Verse Text for the selected Search Results to the clipboard", "MainMenu"));
	m_pActionCopyVerseText->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVerseText);
	m_pActionCopyRaw = m_pEditMenu->addAction(tr("Copy Raw Verse &Text (No headings)", "MainMenu"), this, SLOT(en_copyRaw()));
	m_pActionCopyRaw->setStatusTip(tr("Copy selected Search Results as raw phrase words to the clipboard", "MainMenu"));
	m_pActionCopyRaw->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyRaw);
	m_pActionCopyVeryRaw = m_pEditMenu->addAction(tr("Copy Very Ra&w Verse Text (No punctuation)", "MainMenu"), this, SLOT(en_copyVeryRaw()));
	m_pActionCopyVeryRaw->setStatusTip(tr("Copy selected Search Results as very raw (no punctuation) phrase words to the clipboard", "MainMenu"));
	m_pActionCopyVeryRaw->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVeryRaw);
	// ----
	m_pEditMenu->addSeparator();
	m_pEditMenuLocal->addSeparator();
	m_pActionCopyVerseHeadings = m_pEditMenu->addAction(tr("Copy &References", "MainMenu"), this, SLOT(en_copyVerseHeadings()), QKeySequence(Qt::CTRL + Qt::Key_C));
	m_pActionCopyVerseHeadings->setStatusTip(tr("Copy Verse References for the selected Search Results to the clipboard", "MainMenu"));
	m_pActionCopyVerseHeadings->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVerseHeadings);
	m_pActionCopyReferenceDetails = m_pEditMenu->addAction(tr("Copy Reference Detai&ls (Word/Phrase Counts)", "MainMenu"), this, SLOT(en_copyReferenceDetails()));
	m_pActionCopyReferenceDetails->setStatusTip(tr("Copy the Word/Phrase Reference Details (Counts) for the selected Search Results to the clipboard", "MainMenu"));
	m_pActionCopyReferenceDetails->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyReferenceDetails);
	m_pActionCopyComplete = m_pEditMenu->addAction(tr("Copy &Complete Verse Text and Reference Details", "MainMenu"), this, SLOT(en_copyComplete()), QKeySequence(Qt::CTRL + Qt::Key_B));
	m_pActionCopyComplete->setStatusTip(tr("Copy Complete Verse Text and Reference Details (Counts) for the selected Search Results to the clipboard", "MainMenu"));
	m_pActionCopyComplete->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyComplete);
	// ----
	m_pEditMenu->addSeparator();
	m_pEditMenuLocal->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction(tr("Select &All", "MainMenu"), this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip(tr("Select all Search Results", "MainMenu"));
	m_pActionSelectAll->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionSelectAll);
	m_pActionClearSelection = m_pEditMenu->addAction(tr("C&lear Selection", "MainMenu"), this, SLOT(clearSelection()), QKeySequence(Qt::Key_Escape));
	m_pActionClearSelection->setStatusTip(tr("Clear Search Results Selection", "MainMenu"));
	m_pActionClearSelection->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionClearSelection);
	// ----
	m_pEditMenu->addSeparator();
	m_pMenuInsertionPoint = m_pEditMenuLocal->addSeparator();
	// ----
	// << User Notes menu stuff is inserted in the en_findParentCanOpener() delayed:
	QTimer::singleShot(1, this, SLOT(en_findParentCanOpener()));
	// ----
	m_pMenuUserNotesInsertionPoint = m_pEditMenuLocal->addSeparator();
	m_pActionNavigator = m_pEditMenuLocal->addAction(QIcon(":/res/green_arrow.png"), tr("Passage &Navigator...", "MainMenu"));
	m_pActionNavigator->setEnabled(false);
	connect(m_pActionNavigator, SIGNAL(triggered()), this, SLOT(showPassageNavigator()));
	m_pActionNavigator->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	// ----

	m_pStatusAction = new QAction(this);

	//
	// Setup the CVerseListModel:
	//	Note: This has to be done AFTER creating the menu details above
	//	because on some platforms (e.g. Windows with Qt 5.6.3), the setModel()
	//	call below can trigger a call to selectionChanged(), and since our
	//	selectionChanged() handler expects the above menu pointers to be
	//	set, we will crash if we setup the model before setting those
	//	pointers!!
	//
	CVerseListModel *pModel = new CVerseListModel(pBibleDatabase, pUserNotesDatabase, this);
#ifdef MODELTEST
	new ModelTest(pModel, this);
#endif
	QAbstractItemModel *pOldModel = model();
	setModel(pModel);
	assert(pModel == vlmodel());
	if (pOldModel) delete pOldModel;
	bool bDecorateRoot = (vlmodel()->treeMode() != CVerseListModel::VTME_LIST) ||
						((vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS) && (vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED));
	if ((vlmodel()->viewMode() == CVerseListModel::VVME_CROSSREFS) && (vlmodel()->singleCrossRefSourceIndex().isSet())) bDecorateRoot = false;
	setRootIsDecorated(bDecorateRoot);
	setDragDropMode(QAbstractItemView::DragDrop);

#ifndef IS_MOBILE_APP
	m_pReflowDelegate = new CReflowDelegate(this, true, true);
#else
	// The reflow delegate doesn't get along well with QScroller on mobile devices (probably a bug
	//		on the mobile ports of Qt).  It seems to be assuming some things about item height that
	//		isn't true.  So, when we are processing touch gestures, we need to disable the reflow delegate:
	m_pReflowDelegate = new CReflowDelegate(this, true, !CPersistentSettings::instance()->touchGesturesEnabled());
#endif
	CVerseListDelegate *pDelegate = new CVerseListDelegate(*vlmodel(), this);
	m_pReflowDelegate->setItemDelegate(pDelegate);
	QAbstractItemDelegate *pOldDelegate = itemDelegate();
	setItemDelegate(m_pReflowDelegate);
	if (pOldDelegate) delete pOldDelegate;
	m_pReflowDelegate->setFakeSizeHintRowCount((vlmodel()->displayMode() != CVerseListModel::VDME_HEADING) ? 4 : 1);

	// Setup Default Font and TextBrightness:
	//	Note: This has to be done AFTER setting up the vlmodel above or
	//	else we will assert or segfault for not having a model object
	//	to set the font on:
	setFontSearchResults(CPersistentSettings::instance()->fontSearchResults());
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedSearchResults(const QFont &)), this, SLOT(setFontSearchResults(const QFont &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));

#ifdef USING_QT_SPEECH
	assert(!g_pMyApplication.isNull());
	QtSpeech *pSpeech = g_pMyApplication->speechSynth();

	if (pSpeech != NULL) {
		connect(pSpeech, SIGNAL(beginning()), this, SLOT(setSpeechActionEnables()));
		connect(pSpeech, SIGNAL(finished(bool)), this, SLOT(setSpeechActionEnables()));
	}
#endif	// USING_QT_SPEECH

	// Setup our change notifications:
	connect(vlmodel(), SIGNAL(modelReset()), this, SLOT(en_listChanged()));
	connect(vlmodel(), SIGNAL(layoutChanged()), this, SLOT(en_listChanged()));

	connect(this, SIGNAL(displayContextMenu(const QPoint &)), this, SLOT(en_displayContextMenu(const QPoint &)), Qt::QueuedConnection);

	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(handle_searchResultActivated(const QModelIndex &)));
}

CSearchResultsTreeView::~CSearchResultsTreeView()
{
}

// ----------------------------------------------------------------------------

void CSearchResultsTreeView::en_changedScrollbarsEnabled(bool bEnabled)
{
	if (bEnabled) {
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	} else {
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
}

// ----------------------------------------------------------------------------

CKJVCanOpener *CSearchResultsTreeView::parentCanOpener() const
{
	if (m_pParentCanOpener == NULL) {
		assert(!g_pMyApplication.isNull());
		m_pParentCanOpener = g_pMyApplication->findCanOpenerFromChild<CSearchResultsTreeView>(this);
		// Note: It's possible for the parentCanOpener to be NULL if this function is called during
		//		the construction process before the parent actually exists.  In that case, we'll
		//		return NULL (callers will have to deal with that) and lock in our parent in a future
		//		call when it becomes available...
	}
	return m_pParentCanOpener;
}

void CSearchResultsTreeView::en_findParentCanOpener()
{
	CKJVCanOpener *pCanOpener = parentCanOpener();
	assert(pCanOpener != NULL);

	if (pCanOpener != NULL) {
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
		m_pEditMenu->addActions(pCanOpener->highlighterButtons()->actions());
		m_pEditMenuLocal->insertActions(m_pMenuUserNotesInsertionPoint, pCanOpener->highlighterButtons()->actions());
		connect(pCanOpener->highlighterButtons(), SIGNAL(highlighterToolTriggered(int, bool)), this, SLOT(en_highlightSearchResults(int, bool)));
		// ----
		m_pEditMenu->addSeparator();
		m_pEditMenuLocal->insertSeparator(m_pMenuUserNotesInsertionPoint);
		// ----
		m_pEditMenu->addAction(pCanOpener->actionUserNoteEditor());
		m_pEditMenuLocal->insertAction(m_pMenuUserNotesInsertionPoint, pCanOpener->actionUserNoteEditor());
		// ----
		m_pEditMenu->addSeparator();
		m_pEditMenuLocal->insertSeparator(m_pMenuUserNotesInsertionPoint);
		// ----
		m_pEditMenu->addAction(pCanOpener->actionCrossRefsEditor());
		m_pEditMenuLocal->insertAction(m_pMenuUserNotesInsertionPoint, pCanOpener->actionCrossRefsEditor());
#endif
#ifdef USING_QT_SPEECH
		if (pCanOpener->actionSpeakSelection()) addAction(pCanOpener->actionSpeakSelection());
#endif
	}
}

// ----------------------------------------------------------------------------

#ifdef USING_QT_SPEECH

void CSearchResultsTreeView::en_speechPlay()
{
	assert(!g_pMyApplication.isNull());
	QtSpeech *pSpeech = g_pMyApplication->speechSynth();
	if (pSpeech == NULL) return;

	if (!speakableNodeSelected()) return;

	QModelIndexList lstSelectedVerses = getSelectedVerses();
	vlmodel()->sortModelIndexList(lstSelectedVerses, false);

	// The speech buffer has a limited size, so break into individual sentences at a period.
	//		This will combine questions and exclamations, joining them with adjacent statements,
	//		but there isn't likely to be a ton of them run together, which will achieve the
	//		goal of not overflowing the buffer:
	static const QRegExp regexpSentence("[;.:]");			// Note: Don't include '?' or it will get trimmed -- causing TTS to not do proper inflection (similar for '!')
	QStringList lstSentences;

	for (int ndx = 0; ndx < lstSelectedVerses.size(); ++ndx) {
		const CVerseListItem &item(vlmodel()->data(lstSelectedVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		if (item.verseIndex().isNull()) continue;
		CRelIndex ndxVerse = item.getIndex();
		ndxVerse.setWord(0);
		lstSentences.append(vlmodel()->bibleDatabase()->PassageReferenceText(ndxVerse, true).split(regexpSentence, QString::SkipEmptyParts));
		CParsedPhrase phrase;
		phrase.ParsePhrase(item.getVersePlainText(true));
		lstSentences.append(phrase.phraseToSpeak().split(regexpSentence, QString::SkipEmptyParts));
	}

	for (int ndx = 0; ndx < lstSentences.size(); ++ndx) {
		// Remove Apostrophes and Hyphens and reconstitute normalized composition, as
		//		some special characters (like specialized apostrophes) mess up the
		//		speech synthesis:
		pSpeech->tell(CSearchStringListModel::deApostrophe(CSearchStringListModel::decompose(lstSentences.at(ndx).trimmed(), true), true).normalized(QString::NormalizationForm_KC));
	}
}

void CSearchResultsTreeView::setSpeechActionEnables()
{
	assert(!g_pMyApplication.isNull());
	QtSpeech *pSpeech = g_pMyApplication->speechSynth();

	if ((pSpeech != NULL) && (hasFocus())) {
		if (parentCanOpener()->actionSpeechPlay() != NULL) {
			parentCanOpener()->actionSpeechPlay()->setEnabled(pSpeech->canSpeak() && !pSpeech->isTalking() && speakableNodeSelected());
		}
	}
}

#endif	// USING_QT_SPEECH

// ----------------------------------------------------------------------------

QModelIndexList CSearchResultsTreeView::getSelectedVerses() const
{
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();

	for (int ndx = 0; ndx < lstSelectedItems.size(); /* Increment inside loop */) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = vlmodel()->logicalIndexForModelIndex(lstSelectedItems.at(ndx));
			if ((ndxRel.isSet()) && ((ndxRel.verse() != 0) || ((ndxRel.verse() == 0) && (ndxRel.word() != 0)))) {
				++ndx;
			} else {
				lstSelectedItems.removeAt(ndx);
			}
		} else {
			lstSelectedItems.removeAt(ndx);
		}
	}

	return lstSelectedItems;
}

QModelIndexList CSearchResultsTreeView::getSelectedEntries() const
{
	QModelIndexList lstVerses;

	lstVerses = selectionModel()->selectedRows();
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		if (!lstVerses.at(ndx).isValid()) {
			lstVerses.removeAt(ndx);
			--ndx;
		}
	}

	return lstVerses;
}

// ----------------------------------------------------------------------------

void CSearchResultsTreeView::en_copyVerseEntry() const
{
	QModelIndexList lstVerses = (((vlmodel()->viewMode() == CVerseListModel::VVME_CROSSREFS) ||
								 (vlmodel()->viewMode() == CVerseListModel::VVME_USERNOTES)) ? getSelectedEntries() : getSelectedVerses());

	QMimeData *mime = vlmodel()->mimeDataFromVerseText(lstVerses, false);
	assert(mime != NULL);
	if (mime == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mime);
	displayCopyCompleteToolTip();
}

void CSearchResultsTreeView::en_copyVerseText() const
{
	QMimeData *mime = vlmodel()->mimeDataFromVerseText(getSelectedVerses(), true);
	assert(mime != NULL);
	if (mime == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mime);
	displayCopyCompleteToolTip();
}

void CSearchResultsTreeView::en_copyRaw() const
{
	QMimeData *mime = vlmodel()->mimeDataFromRawVerseText(getSelectedVerses(), false);
	assert(mime != NULL);
	if (mime == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mime);
	displayCopyCompleteToolTip();
}

void CSearchResultsTreeView::en_copyVeryRaw() const
{
	QMimeData *mime = vlmodel()->mimeDataFromRawVerseText(getSelectedVerses(), true);
	assert(mime != NULL);
	if (mime == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mime);
	displayCopyCompleteToolTip();
}

void CSearchResultsTreeView::en_copyVerseHeadings() const
{
	QModelIndexList lstVerses = (((vlmodel()->viewMode() == CVerseListModel::VVME_CROSSREFS) ||
								 (vlmodel()->viewMode() == CVerseListModel::VVME_USERNOTES)) ? getSelectedEntries() : getSelectedVerses());

	QMimeData *mime = vlmodel()->mimeDataFromVerseHeadings(lstVerses, true);
	assert(mime != NULL);
	if (mime == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mime);
	displayCopyCompleteToolTip();
}

void CSearchResultsTreeView::en_copyReferenceDetails() const
{
	QMimeData *mime = vlmodel()->mimeDataFromReferenceDetails(getSelectedVerses());
	assert(mime != NULL);
	if (mime == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mime);
	displayCopyCompleteToolTip();
}

void CSearchResultsTreeView::en_copyComplete() const
{
	QMimeData *mime = vlmodel()->mimeDataFromCompleteVerseDetails(getSelectedVerses());
	assert(mime != NULL);
	if (mime == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mime);
	displayCopyCompleteToolTip();
}

void CSearchResultsTreeView::displayCopyCompleteToolTip() const
{
	new CNotificationToolTip(1500, m_ptLastTrackPosition, tr("Text Copied to Clipboard", "MainMenu"), viewport());
}

// ----------------------------------------------------------------------------

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
void CSearchResultsTreeView::en_highlightSearchResults(int ndxHighlighterTool, bool bSecondaryActive)
{
	if (!hasFocus()) return;
	assert(parentCanOpener() != NULL);			// We should have a parentCanOpener or else we shouldn't have connected this slot yet
	assert(vlmodel()->userNotesDatabase() != NULL);

	QString strHighlighterName = parentCanOpener()->highlighterButtons()->highlighter(ndxHighlighterTool);
	if (strHighlighterName.isEmpty()) return;
	const TPhraseTagList *plstHighlighterTags = vlmodel()->userNotesDatabase()->highlighterTagsFor(vlmodel()->bibleDatabase(), strHighlighterName);

	QModelIndexList lstVerses = getSelectedVerses();
	if (lstVerses.isEmpty()) return;

	bool bAllAlreadyHighlighted = (plstHighlighterTags != NULL);
	if (plstHighlighterTags != NULL) {
		for (int ndxVerse = 0; ndxVerse < lstVerses.size(); ++ndxVerse) {
			const CVerseListItem &item(vlmodel()->data(lstVerses.at(ndxVerse), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
			if (!plstHighlighterTags->completelyContains(vlmodel()->bibleDatabase().data(), (bSecondaryActive ? item.phraseTags() : item.getWholeVersePhraseTag()))) {
				bAllAlreadyHighlighted = false;
				break;
			}
		}
	}

	TPhraseTagList lstVerseTags;
	lstVerseTags.reserve(lstVerses.size());
	for (int ndxVerse = 0; ndxVerse < lstVerses.size(); ++ndxVerse) {
		const CVerseListItem &item(vlmodel()->data(lstVerses.at(ndxVerse), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		lstVerseTags.append(bSecondaryActive ? item.phraseTags() : item.getWholeVersePhraseTag());
	}

	if (bAllAlreadyHighlighted) {
		int nResult = displayInformation(this, windowTitle(), tr("All of the verses you have selected are already highlighted with that highlighter!\n\n"
																	   "Do you wish to unhighlight all of them instead??", "Errors"),
																	(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
		if (nResult != QMessageBox::Yes) return;
		CBusyCursor iAmBusy(NULL);
		vlmodel()->userNotesDatabase()->removeHighlighterTagsFor(vlmodel()->bibleDatabase(), strHighlighterName, lstVerseTags);
		return;
	}

	CBusyCursor iAmBusy(NULL);
	vlmodel()->userNotesDatabase()->appendHighlighterTagsFor(vlmodel()->bibleDatabase(), strHighlighterName, lstVerseTags);
}
#endif

// ----------------------------------------------------------------------------

TVerseIndex CSearchResultsTreeView::currentVerseIndex() const
{
	return (*CVerseListModel::toVerseIndex(currentIndex()));
}

bool CSearchResultsTreeView::setCurrentIndex(const TVerseIndex &ndx, bool bFocusTreeView)
{
	QModelIndex ndxModel = vlmodel()->locateIndex(ndx);
	QTreeView::setCurrentIndex(ndxModel);
	if (ndxModel.isValid()) expand(ndxModel);
	scrollTo(ndxModel, QAbstractItemView::EnsureVisible);
	if (bFocusTreeView) setFocus();
	return ndxModel.isValid();
}

// ----------------------------------------------------------------------------

bool CSearchResultsTreeView::editableNodeSelected() const
{
	return ((selectionModel()->selectedRows().size() <= 1) &&
			(currentIndex().isValid()) &&
			(CVerseListModel::toVerseIndex(currentIndex())->relIndex().isSet()));
}

bool CSearchResultsTreeView::speakableNodeSelected() const
{
	return !getSelectedVerses().isEmpty();
}

void CSearchResultsTreeView::setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode)
{
	// Set root decoration before switching mode so en_listChanged emits canExpandAll/canCollapseAll correctly
	bool bDecorateRoot = (vlmodel()->treeMode() != CVerseListModel::VTME_LIST) ||
						((nViewMode != CVerseListModel::VVME_SEARCH_RESULTS) && (nViewMode != CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED));
	if ((nViewMode == CVerseListModel::VVME_CROSSREFS) && (vlmodel()->singleCrossRefSourceIndex().isSet())) bDecorateRoot = false;
	setRootIsDecorated(bDecorateRoot);
	vlmodel()->setViewMode(nViewMode);
}

void CSearchResultsTreeView::setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	vlmodel()->setDisplayMode(nDisplayMode);
	m_pReflowDelegate->setFakeSizeHintRowCount((vlmodel()->displayMode() != CVerseListModel::VDME_HEADING) ? 4 : 1);
}

void CSearchResultsTreeView::setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode)
{
	// Set root decoration before switching mode so en_listChanged emits canExpandAll/canCollapseAll correctly
	bool bDecorateRoot = (nTreeMode != CVerseListModel::VTME_LIST) ||
						((vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS) && (vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED));
	if ((vlmodel()->viewMode() == CVerseListModel::VVME_CROSSREFS) && (vlmodel()->singleCrossRefSourceIndex().isSet())) bDecorateRoot = false;
	setRootIsDecorated(bDecorateRoot);
	vlmodel()->setTreeMode(nTreeMode);
}

void CSearchResultsTreeView::setShowMissingLeafs(bool bShowMissing)
{
	vlmodel()->setShowMissingLeafs(bShowMissing);
}

void CSearchResultsTreeView::setShowHighlightersInSearchResults(bool bShowHighlightersInSearchResults)
{
	vlmodel()->setShowHighlightersInSearchResults(bShowHighlightersInSearchResults);
}

void CSearchResultsTreeView::setSingleCrossRefSourceIndex(const CRelIndex &ndx)
{
	// Set root decoration before switching mode so en_listChanged emits canExpandAll/canCollapseAll correctly
	bool bDecorateRoot = (vlmodel()->treeMode() != CVerseListModel::VTME_LIST) ||
						((vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS) && (vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED));
	if ((vlmodel()->viewMode() == CVerseListModel::VVME_CROSSREFS) && (ndx.isSet())) bDecorateRoot = false;
	setRootIsDecorated(bDecorateRoot);
	vlmodel()->setSingleCrossRefSourceIndex(ndx);
}

void CSearchResultsTreeView::setParsedPhrases(const CSearchResultsData &searchResultsData)
{
	vlmodel()->setParsedPhrases(searchResultsData);
}

void CSearchResultsTreeView::showPassageNavigator()
{
	assert(!vlmodel()->bibleDatabase().isNull());

	CRelIndex ndxRel;

	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	if (lstSelectedItems.size() == 1) {
		if (!lstSelectedItems.at(0).isValid()) return;
		ndxRel = vlmodel()->logicalIndexForModelIndex(lstSelectedItems.at(0));
		assert(ndxRel.isSet());
		if (!ndxRel.isSet()) return;
	} else {
		ndxRel = vlmodel()->logicalIndexForModelIndex(currentIndex());
		assert(ndxRel.isSet());			// Should have had one or the other because of editableNodeSelected()
		if (!ndxRel.isSet()) return;
	}

#ifndef USE_ASYNC_DIALOGS
//	const CVerseListItem &item(lstSelectedItems.at(0).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
	CKJVCanOpener::CKJVCanOpenerCloseGuard closeGuard(parentCanOpener());
	CKJVPassageNavigatorDlgPtr pDlg(vlmodel()->bibleDatabase(), this);

//	pDlg->navigator().startAbsoluteMode(TPhraseTag(item.getIndex(), 0));

	pDlg->navigator().startAbsoluteMode(TPhraseTag(ndxRel, 0));
	if (pDlg->exec() == QDialog::Accepted) {
		if (pDlg != NULL) emit gotoIndex(pDlg->passage());		// Could get deleted during execution
	}
#else
	CKJVPassageNavigatorDlg *pDlg = new CKJVPassageNavigatorDlg(vlmodel()->bibleDatabase(), this);
	connect(pDlg, SIGNAL(gotoIndex(const TPhraseTag &)), this, SIGNAL(gotoIndex(const TPhraseTag &)));
	pDlg->navigator().startAbsoluteMode(TPhraseTag(ndxRel, 0));
	pDlg->show();
#endif
}

// ----------------------------------------------------------------------------

bool CSearchResultsTreeView::event(QEvent *event)
{
	assert(event != NULL);
#ifdef TOUCH_GESTURE_PROCESSING
	if (event->type() == QEvent::Gesture) {
		QGestureEvent *pGestureEvent = static_cast<QGestureEvent *>(event);
		QTapGesture *pTap = static_cast<QTapGesture *>(pGestureEvent->gesture(Qt::TapGesture));
		QTapAndHoldGesture *pTapAndHold = static_cast<QTapAndHoldGesture *>(pGestureEvent->gesture(Qt::TapAndHoldGesture));
		QPanGesture *pPan = static_cast<QPanGesture *>(pGestureEvent->gesture(Qt::PanGesture));
		QSwipeGesture *pSwipe = static_cast<QSwipeGesture *>(pGestureEvent->gesture(Qt::SwipeGesture));
		// Observe in order of priority, as some gestures can trigger multiple recognitions:
		if (pSwipe != NULL) {
			return handleSwipeGesture(pSwipe);
		} else if (pPan != NULL) {
			return handlePanGesture(pPan);
		} else if (pTapAndHold != NULL) {
			return handleTapAndHoldGesture(pTapAndHold);
		} else if (pTap) {
			return handleTapGesture(pTap);
		}
	}
#endif

	return QTreeView::event(event);
}

void CSearchResultsTreeView::keyPressEvent(QKeyEvent *event)
{
	assert(event != NULL);
	switch (event->key()) {
		case Qt::Key_Select:
			// Also do Key_Enter action.
			if (currentIndex().isValid()) {
				if (state() != EditingState) handle_searchResultActivated(currentIndex());
			} else {
				event->ignore();
			}
			break;

		case Qt::Key_Enter:
		case Qt::Key_Return:
			// ### we can't open the editor on enter, becuse
			// some widgets will forward the enter event back
			// to the viewport, starting an endless loop
			if (state() != EditingState || hasFocus()) {
				if (currentIndex().isValid()) handle_searchResultActivated(currentIndex());
				event->ignore();
			}
			break;
		default:
			QTreeView::keyPressEvent(event);
			break;
	}
}

void CSearchResultsTreeView::handle_searchResultActivated(const QModelIndex &index)
{
	emit searchResultActivated(index);
}

// ----------------------------------------------------------------------------

#ifdef TOUCH_GESTURE_PROCESSING
QString CSearchResultsTreeView::debugGestureState(QGesture *pGesture) const
{
	switch (pGesture->state()) {
		case Qt::GestureStarted:
			return QString("GestureStarted");
		case Qt::GestureUpdated:
			return QString("GestureUpdated");
		case Qt::GestureFinished:
			return QString("GestureFinished");
		case Qt::GestureCanceled:
			return QString("GestureCanceled");
		default:
			return QString("Unknown Gesture State");
	}

	return QString();
}

bool CSearchResultsTreeView::handleTapGesture(QTapGesture *pTapGesture)
{
qDebug("%s", QString("Handle Tap Gesture -- %1").arg(debugGestureState(pTapGesture)).toUtf8().data());
	assert(pTapGesture != NULL);

	if (pTapGesture->state() == Qt::GestureFinished) {
		if (m_bDoubleTouchStarted) {
qDebug("Finish double-touch");
			// Second touch triggers a double-touch gesture:
			m_dlyDoubleTouch.untrigger();
			m_bDoubleTouchStarted = false;
			QPersistentModelIndex index = indexAt(pTapGesture->position().toPoint());
			if ((index.isValid()) &&
				(index == m_ndxDoubleTouch)) handle_searchResultActivated(index);
			m_ndxDoubleTouch = QModelIndex();
		} else {
qDebug("Start double-touch");
			// First touch starts a double-touch gesture:
			m_bDoubleTouchStarted = true;
			m_dlyDoubleTouch.trigger();
			m_ndxDoubleTouch = indexAt(pTapGesture->position().toPoint());
		}
	} else if (pTapGesture->state() == Qt::GestureCanceled) {
if (m_bDoubleTouchStarted) qDebug("Cancel double-touch");
		// Canceling a touch-gesture cancels any pending double-touch gesture too:
		m_bDoubleTouchStarted = false;
		m_dlyDoubleTouch.untrigger();
		m_ndxDoubleTouch = QModelIndex();
	}

	return true;
}

void CSearchResultsTreeView::en_doubleTouchTimeout()
{
qDebug("Double-touch timed out");
	m_bDoubleTouchStarted = false;
	m_ndxDoubleTouch = QModelIndex();
}

bool CSearchResultsTreeView::handleTapAndHoldGesture(QTapAndHoldGesture *pTapAndHoldGesture)
{
qDebug("%s", QString("Handle TapAndHold Gesture -- %1").arg(debugGestureState(pTapAndHoldGesture)).toUtf8().data());
	assert(pTapAndHoldGesture != NULL);

	if ((pTapAndHoldGesture->state() == Qt::GestureFinished) &&
		(!m_bDoingPopup)) {
		en_displayContextMenu(pTapAndHoldGesture->position().toPoint());
//		emit displayContextMenu(pTapAndHoldGesture->position().toPoint());
	} else if (pTapAndHoldGesture->state() == Qt::GestureStarted) {
		// Starting a tap-and-hold cancels a pending double-touch:
		m_bDoubleTouchStarted = false;
		m_dlyDoubleTouch.untrigger();
		m_ndxDoubleTouch = QModelIndex();
	}

	return true;
}

bool CSearchResultsTreeView::handlePanGesture(QPanGesture *pPanGesture)
{
qDebug("%s", QString("Handle Pan Gesture -- %1").arg(debugGestureState(pPanGesture)).toUtf8().data());
	assert(pPanGesture != NULL);

	QScrollBar *pVertSB = verticalScrollBar();
	int nStepsToScroll = 0;
	qreal nOffset = (pPanGesture->delta().ry() / 120) * pVertSB->singleStep();

	// Check if the pan changed direction since last event:
	if ((m_nAccumulatedScrollOffset != 0) && ((nOffset / m_nAccumulatedScrollOffset) < 0))
		m_nAccumulatedScrollOffset = 0;

	m_nAccumulatedScrollOffset += nOffset;
	nStepsToScroll = int(m_nAccumulatedScrollOffset);
	m_nAccumulatedScrollOffset -= int(m_nAccumulatedScrollOffset);
	if (nStepsToScroll == 0) return false;

	if (pVertSB->invertedControls()) nStepsToScroll = -nStepsToScroll;

	int nPrevValue = pVertSB->value();

	int nNewValue = pVertSB->value() + nStepsToScroll;
	if ((nStepsToScroll > 0) && (nNewValue < pVertSB->value())) {
		nNewValue = pVertSB->maximum();
	} else if ((nStepsToScroll < 0) && (nNewValue > pVertSB->value())) {
		nNewValue = pVertSB->minimum();
	}
	pVertSB->setValue(nNewValue);

// TODO :CLEAN
//    position = overflowSafeAdd(stepsToScroll); // value will be updated by triggerAction()
//    q->triggerAction(QAbstractSlider::SliderMove);

	if (nPrevValue == nNewValue) {
		m_nAccumulatedScrollOffset = 0;
		return false;
	}

	return true;
}

bool CSearchResultsTreeView::handleSwipeGesture(QSwipeGesture *pSwipeGesture)
{
qDebug("%s", QString("Handle Swipe Gesture -- %1").arg(debugGestureState(pSwipeGesture)).toUtf8().data());
	assert(pSwipeGesture != NULL);

	// TODO : Something here for swipe gesture...

	return true;
}
#endif

void CSearchResultsTreeView::mouseMoveEvent(QMouseEvent *ev)
{
	m_ptLastTrackPosition = ev->globalPos();
	QTreeView::mouseMoveEvent(ev);
}

void CSearchResultsTreeView::focusInEvent(QFocusEvent *event)
{
	emit activatedSearchResults();
	QTreeView::focusInEvent(event);
	if (parentCanOpener() != NULL) {
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
		parentCanOpener()->highlighterButtons()->setHighlighterTips(true);
#endif
#ifdef USING_QT_SPEECH
		if (parentCanOpener()->actionSpeechPlay())
			connect(parentCanOpener()->actionSpeechPlay(), SIGNAL(triggered()), this, SLOT(en_speechPlay()), Qt::UniqueConnection);
		if (parentCanOpener()->actionSpeakSelection())
			connect(parentCanOpener()->actionSpeakSelection(), SIGNAL(triggered()), this, SLOT(en_speechPlay()), Qt::UniqueConnection);
		setSpeechActionEnables();
#endif
	}

	handle_selectionChanged();
}

void CSearchResultsTreeView::focusOutEvent(QFocusEvent *event)
{
	QTreeView::focusOutEvent(event);

	if ((parentCanOpener() != NULL) &&
		(event->reason() != Qt::MenuBarFocusReason) &&
		(event->reason() != Qt::PopupFocusReason)) {
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
		parentCanOpener()->highlighterButtons()->setHighlighterTips(false);
		parentCanOpener()->actionUserNoteEditor()->setEnabled(false);
		parentCanOpener()->actionCrossRefsEditor()->setEnabled(false);
		const QList<QAction *> lstHighlightActions = parentCanOpener()->highlighterButtons()->actions();
		for (int ndxHighlight = 0; ndxHighlight < lstHighlightActions.size(); ++ndxHighlight) {
			lstHighlightActions.at(ndxHighlight)->setEnabled(false);
		}
#endif
#ifdef USING_QT_SPEECH
		if (parentCanOpener()->actionSpeechPlay())
			disconnect(parentCanOpener()->actionSpeechPlay(), SIGNAL(triggered()), this, SLOT(en_speechPlay()));
		if (parentCanOpener()->actionSpeakSelection())
			disconnect(parentCanOpener()->actionSpeakSelection(), SIGNAL(triggered()), this, SLOT(en_speechPlay()));
		setSpeechActionEnables();
#endif
	}
}

void CSearchResultsTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	if (event->reason() == QContextMenuEvent::Mouse) {
		// Prevent the mouse right-click from being right on top of the
		//		menu, which tends to cause me to accidentally click
		//		something on the menu that I wasn't intending to:
		QPoint pos = event->globalPos();
		pos.rx() += (viewport()->width()/10);
		en_displayContextMenu(pos);
	} else {
		en_displayContextMenu(event->globalPos());
	}
}

void CSearchResultsTreeView::en_displayContextMenu(const QPoint &ptGlobalPos)
{
	m_bDoingPopup = true;
#ifndef USE_ASYNC_DIALOGS
	m_pEditMenuLocal->exec(ptGlobalPos);
#else
	m_pEditMenuLocal->popup(ptGlobalPos);
#endif
	m_bDoingPopup = false;
	m_ptLastTrackPosition = ptGlobalPos;
}

void CSearchResultsTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QTreeView::currentChanged(current, previous);
	emit currentItemChanged();
#if QT_VERSION >= 0x050400		// Functor calls was introduced in Qt 5.4
	// If the user pressed 'end', then the Viewport size hint won't have been
	//	calculated initially and will end up scrolling to the wrong location.
	//	To fix it, we must queue a scrollTo() event to get it there.  Most of
	//	the time, this event does nothing since the reflowViewport() of the
	//	ReflowDelegate handles the current page and the page up/down and next
	//	logic OK.  This is really only for the "user presses end" case and
	//	the size hints of the last page haven't been calculated yet.  This
	//	one-liner is a lot easier than creating a reverseIterator and making
	//	reflowViewport always calculate size-hints for the last page too:
	QTimer::singleShot(1, [this]()->void { scrollTo(currentIndex(), QAbstractItemView::EnsureVisible); });
#endif
}

void CSearchResultsTreeView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	handle_selectionChanged();
	QTreeView::selectionChanged(selected, deselected);
}

void CSearchResultsTreeView::handle_selectionChanged()
{
	int nNumResultsSelected = 0;

	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
/*
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = vlmodel()->logicalIndexForModelIndex(lstSelectedItems.at(ndx));
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
				nNumResultsSelected++;
			}
		}
	}
*/

	nNumResultsSelected = lstSelectedItems.size();
	bool bHaveVerses = (getSelectedVerses().size() > 0);
	bool bInSearchResultsMode = ((vlmodel()->viewMode() == CVerseListModel::VVME_SEARCH_RESULTS) ||
								 (vlmodel()->viewMode() == CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED));

	if (nNumResultsSelected) {
		m_pActionCopyVerseEntry->setEnabled(true);
		m_pActionCopyVerseText->setEnabled(bHaveVerses);
		m_pActionCopyRaw->setEnabled(bHaveVerses);
		m_pActionCopyVeryRaw->setEnabled(bHaveVerses);
		m_pActionCopyVerseHeadings->setEnabled(true);
		m_pActionCopyReferenceDetails->setEnabled(bInSearchResultsMode);
		m_pActionCopyComplete->setEnabled(bInSearchResultsMode);
		m_pActionClearSelection->setEnabled(true);
	} else {
		m_pActionCopyVerseEntry->setEnabled(false);
		m_pActionCopyVerseText->setEnabled(false);
		m_pActionCopyRaw->setEnabled(false);
		m_pActionCopyVeryRaw->setEnabled(false);
		m_pActionCopyVerseHeadings->setEnabled(false);
		m_pActionCopyReferenceDetails->setEnabled(false);
		m_pActionCopyComplete->setEnabled(false);
		m_pActionClearSelection->setEnabled(false);
	}
	// Only allow navigation, note adding, cross-refs, etc on a node (verse or otherwise)
	bool bEditableNode = editableNodeSelected();

	m_pActionNavigator->setEnabled(bEditableNode);

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	if (hasFocus()) {
		if (parentCanOpener() != NULL) {
			parentCanOpener()->actionUserNoteEditor()->setEnabled(bEditableNode);
			parentCanOpener()->actionCrossRefsEditor()->setEnabled(bEditableNode);
			const QList<QAction *> lstHighlightActions = parentCanOpener()->highlighterButtons()->actions();
			for (int ndxHighlight = 0; ndxHighlight < lstHighlightActions.size(); ++ndxHighlight) {
				lstHighlightActions.at(ndxHighlight)->setEnabled(bInSearchResultsMode && vlmodel()->showHighlightersInSearchResults() && bHaveVerses);
			}
		}
	}
#endif

	QString strStatusText;

	switch (viewMode()) {
		case CVerseListModel::VVME_SEARCH_RESULTS:
			strStatusText = tr("%n Search Result(s) Selected", "Statistics", nNumResultsSelected);
			break;
		case CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED:
			strStatusText = tr("%n Excluded Search Result(s) Selected", "Statistics", nNumResultsSelected);
			break;
		case CVerseListModel::VVME_HIGHLIGHTERS:
			strStatusText = tr("%n Highlighted Verse(s) Selected", "Statistics", nNumResultsSelected);
			break;
		case CVerseListModel::VVME_USERNOTES:
			strStatusText = tr("%n Note(s) Selected", "Statistics", nNumResultsSelected);
			break;
		case CVerseListModel::VVME_CROSSREFS:
			strStatusText = tr("%n Cross-Reference(s) Selected", "Statistics", nNumResultsSelected);
			break;
		default:
			assert(false);
			break;
	}

	setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();

	if (CTipEdit::tipEditIsPinned(parentCanOpener())) showDetails();

	emit selectionListChanged();
}

void CSearchResultsTreeView::en_listChanged()
{
	int nResultsCount = vlmodel()->GetResultsCount();

	m_pActionSelectAll->setEnabled(nResultsCount != 0);
	emit canExpandAll((rootIsDecorated()) && (vlmodel()->hasChildren()));
	emit canCollapseAll((rootIsDecorated()) && (vlmodel()->hasChildren()));

	if ((CPersistentSettings::instance()->autoExpandSearchResultsTree()) &&
		((vlmodel()->viewMode() == CVerseListModel::VVME_SEARCH_RESULTS) ||
		 (vlmodel()->viewMode() == CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED)))
		expandAll();

	handle_selectionChanged();
}

void CSearchResultsTreeView::showDetails()
{
	QVariant varTooltip = vlmodel()->data(currentIndex(), CVerseListModel::TOOLTIP_ROLE);
	if (varTooltip.canConvert<QString>()) {
		scrollTo(currentIndex(), QAbstractItemView::EnsureVisible);

//		QToolTip::showText(mapToGlobal(visualRect(QTreeView::currentIndex()).topRight()), varTooltip.toString(), this);
		QToolTip::hideText();
		CToolTipEdit::showText(parentCanOpener(), mapToGlobal(visualRect(currentIndex()).topRight()), varTooltip.toString(), this, rect());
	} else {
		if (CTipEdit::tipEditIsPinned(parentCanOpener())) CToolTipEdit::hideText(parentCanOpener());
	}
}

bool CSearchResultsTreeView::haveDetails() const
{
	if (!currentIndex().isValid()) return false;

	QVariant varTooltip = vlmodel()->data(currentIndex(), CVerseListModel::TOOLTIP_ROLE);
	if ((varTooltip.canConvert<QString>()) &&
		(!varTooltip.toString().isEmpty())) return true;

	return false;
}

bool CSearchResultsTreeView::isActive() const
{
	return ((hasFocus()) || ((parentCanOpener() != NULL) && (parentCanOpener()->isSearchResultsActive())));
}

void CSearchResultsTreeView::resizeEvent(QResizeEvent *event)
{
	assert(event != NULL);

// This isn't needed when using the ReflowDelegate because the delegate is handling
//	the resizeEvent as well and will invalidate our sizeHints appropriately.  Leaving
//	this code here for reference in case we ever remove the ReflowDelegate:
//
//	// Unlike the QListView, the QTreeView doesn't have a ResizeMode for Adjust.  So
//	//		we need to handle this event to do a new layout when the
//	//		view size changes.
//
//	QSize szDelta = event->size() - event->oldSize();
//
//	if (!szDelta.isNull()) {
//		bool bFlowDimensionChanged = (szDelta.width() != 0);
//
//		if ((state() == NoState) && (bFlowDimensionChanged)) {
//			scheduleDelayedItemsLayout();
//		}
//	}

	// Save our scroll position when resizing the whole tree:
	CScrollPreserver verticalOffset(this);

	QTreeView::resizeEvent(event);
}

void CSearchResultsTreeView::setFontSearchResults(const QFont& aFont)
{
	vlmodel()->setFont(aFont);

#ifdef WORKAROUND_QTBUG_33906
	verticalScrollBar()->setSingleStep(qMax(fontMetrics().height() * LINES_PER_SCROLL_BLOCK, 2));
#endif
}

void CSearchResultsTreeView::setTextBrightness(bool bInvert, int nBrightness)
{
	m_bInvertTextBrightness = bInvert;
	m_nTextBrightness = nBrightness;

	// Note: This will automatically cause a repaint:
	setStyleSheet(QString("CSearchResultsTreeView { background-color:%1; color:%2; } ")
									.arg(CPersistentSettings::textBackgroundColor(bInvert, nBrightness).name())
									.arg(CPersistentSettings::textForegroundColor(bInvert, nBrightness).name()));
}

QStyleOptionViewItem CSearchResultsTreeView::viewOptions() const
{
	QStyleOptionViewItemV4_t optionV4 = QTreeView::viewOptions();

	QColor clrForeground = CPersistentSettings::textForegroundColor(m_bInvertTextBrightness, m_nTextBrightness);
	QColor clrBackground = CPersistentSettings::textBackgroundColor(m_bInvertTextBrightness, m_nTextBrightness);
//	clrBackground.setAlpha(150);

	optionV4.palette.setColor(QPalette::All, QPalette::Base, clrBackground);
	optionV4.palette.setColor(QPalette::All, QPalette::AlternateBase, clrBackground);
	optionV4.palette.setColor(QPalette::All, QPalette::Background, clrForeground);			// This one is used by the PE_IndicatorBranch (+) boxes to expand children (yes, it's weird...)
	optionV4.palette.setColor(QPalette::All, QPalette::Button, clrBackground);

	optionV4.palette.setColor(QPalette::All, QPalette::Text, clrForeground);
	optionV4.palette.setColor(QPalette::All, QPalette::Foreground, clrForeground);
	optionV4.palette.setColor(QPalette::All, QPalette::ButtonText, clrForeground);
//	optionV4.palette.setColor(QPalette::All, QPalette::BrightText, clrForeground);

	QColor clrHighlight = CPersistentSettings::instance()->textBackgroundColor(false, m_nTextBrightness);
	clrHighlight.setAlpha(150);
	optionV4.palette.setColor(QPalette::All, QPalette::Highlight, clrHighlight.darker(m_bInvertTextBrightness ? 150 : 116));
	optionV4.palette.setColor(QPalette::Active, QPalette::Highlight, clrHighlight.darker(m_bInvertTextBrightness ? 184 : 150));
	optionV4.palette.setColor(QPalette::All, QPalette::HighlightedText, clrForeground);

	return optionV4;
}

// ----------------------------------------------------------------------------

void CSearchResultsTreeView::startDrag(Qt::DropActions supportedActions)
{
	QModelIndexList lstIndexes = selectedIndexes();
	for (int ndx = lstIndexes.count() - 1 ; ndx >= 0; --ndx) {
		if (!(model()->flags(lstIndexes.at(ndx)) & Qt::ItemIsDragEnabled))
			lstIndexes.removeAt(ndx);
	}

	if (lstIndexes.count() > 0) {
		QMimeData *pMimeData = model()->mimeData(lstIndexes);
		if (!pMimeData) return;
		QRect rc;
		QPixmap pixmap = renderToPixmap(lstIndexes, &rc);
		rc.adjust(horizontalOffset(), verticalOffset(), 0, 0);
		QDrag *pDrag = new QDrag(this);
		pDrag->setPixmap(pixmap);
		pDrag->setMimeData(pMimeData);
//		pDrag->setHotSpot(d->pressedPosition - rc.topLeft());
		QRect rcCurrentVisual = visualRect(currentIndex());
		QScrollBar *pVertScrollBar = verticalScrollBar();
		assert(pVertScrollBar != NULL);
		pDrag->setHotSpot(QPoint(rcCurrentVisual.left(), rcCurrentVisual.top() + pVertScrollBar->value() + rcCurrentVisual.height()/2) - rc.topLeft());
		Qt::DropAction aDefaultDropAction = Qt::IgnoreAction;
//		if (d->defaultDropAction != Qt::IgnoreAction && (supportedActions & d->defaultDropAction))
		if ((defaultDropAction() != Qt::IgnoreAction) && (model()->supportedDropActions() & defaultDropAction()))
			aDefaultDropAction = defaultDropAction();
//		else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
		else if ((model()->supportedDropActions() & Qt::CopyAction) && (dragDropMode() != QAbstractItemView::InternalMove))
			aDefaultDropAction = Qt::CopyAction;
		if (pDrag->exec(supportedActions, aDefaultDropAction) == Qt::MoveAction) {
//			d->clearOrRemove();
		}
	}
}

CSearchResultsTreeView::CItemViewPaintPairs CSearchResultsTreeView::draggablePaintPairs(const QModelIndexList &lstIndexes, QRect *pRC) const
{
	assert(pRC != NULL);
	QRect &rc = *pRC;
	const QRect viewportRC = viewport()->rect();
	CItemViewPaintPairs lstRet;
	for (int i = 0; i < lstIndexes.count(); ++i) {
		const QModelIndex &index = lstIndexes.at(i);
		const QRect currentRC = visualRect(index);
		if (currentRC.intersects(viewportRC)) {
			lstRet += qMakePair(currentRC, index);
			rc |= currentRC;
		}
	}
	rc &= viewportRC;
	return lstRet;
}

QPixmap CSearchResultsTreeView::renderToPixmap(const QModelIndexList &lstIndexes, QRect *pRC) const
{
	assert(pRC != NULL);

	CItemViewPaintPairs lstPaintPairs = draggablePaintPairs(lstIndexes, pRC);
	if (lstPaintPairs.isEmpty()) return QPixmap();
	QPixmap pixmap(pRC->size());
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setOpacity(0.5);

	QStyleOptionViewItemV4_t option = viewOptions();
//    if (wrapItemText)
//        option.features = QStyleOptionViewItemV2::WrapText;
	option.locale = locale();
	option.locale.setNumberOptions(QLocale::OmitGroupSeparator);
	option.widget = this;
	option.state |= QStyle::State_Selected;
	for (int j = 0; j < lstPaintPairs.count(); ++j) {
		option.rect = lstPaintPairs.at(j).first.translated(-pRC->topLeft());
		const QModelIndex &current = lstPaintPairs.at(j).second;
//		adjustViewOptionsForIndex(&option, current);
//		delegateForIndex(current)->paint(&painter, option, current);
		itemDelegate(current)->paint(&painter, option, current);
	}

	painter.end();

	return pixmap;
}

// ============================================================================


// ============================================================================

CKJVSearchResult::CKJVSearchResult(CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QWidget(parent),
	m_pBibleDatabase(pBibleDatabase),
	// ----
	m_bDoingUpdate(false),
	m_pSearchResultsType(NULL),
	m_pSearchResultsCount(NULL),
	m_pExcludedSearchResultsCount(NULL),
	m_pShowHighlightersInSearchResults(NULL),
	m_pNoteKeywordWidget(NULL),
	m_pSearchResultsTreeView(NULL)
{
	assert(!m_pBibleDatabase.isNull());
	assert(!g_pUserNotesDatabase.isNull());

	QVBoxLayout *pLayout = new QVBoxLayout(this);
	pLayout->setSpacing(4);
	pLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	pLayout->setContentsMargins(0, 0, 0, 0);

	m_pSearchResultsType = new QLabel(this);
	m_pSearchResultsType->setObjectName(QString::fromUtf8("SearchResultsType"));
	m_pSearchResultsType->setWordWrap(true);
	m_pSearchResultsType->setAlignment(Qt::AlignHCenter);
	m_pSearchResultsType->setTextFormat(Qt::RichText);
	pLayout->addWidget(m_pSearchResultsType);

	// --------------------------------

	m_pSearchResultsCount = new QLabel(this);
	m_pSearchResultsCount->setObjectName(QString::fromUtf8("SearchResultsCount"));
	m_pSearchResultsCount->setWordWrap(true);
	m_pSearchResultsCount->setText(tr("Found 0 Occurrences", "Statistics") + "\n"
									  "    " + tr("in 0 Verses in 0 Chapters in 0 Books", "Statistics"));
	pLayout->addWidget(m_pSearchResultsCount);

	m_pExcludedSearchResultsCount = new QLabel(this);
	m_pExcludedSearchResultsCount->setObjectName(QString::fromUtf8("ExcludedSearchResultsCount"));
	m_pExcludedSearchResultsCount->setWordWrap(true);
	m_pExcludedSearchResultsCount->setText(tr("Excluded 0 Occurrences", "Statistics") + "\n"
											  "    " + tr("in 0 Verses in 0 Chapters in 0 Books", "Statistics"));
	pLayout->addWidget(m_pExcludedSearchResultsCount);

	// --------------------------------

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER) && !defined(IS_CONSOLE_APP)
	m_pShowHighlightersInSearchResults = new QCheckBox(this);
	m_pShowHighlightersInSearchResults->setObjectName(QString::fromUtf8("checkBoxShowHighlightersInSearchResults"));
	m_pShowHighlightersInSearchResults->setText(tr("Show &Highlighting in Search Results", "MainMenu"));
	pLayout->addWidget(m_pShowHighlightersInSearchResults);
#endif

	// --------------------------------

	m_pNoteKeywordWidget = new CNoteKeywordWidget(this);
	m_pNoteKeywordWidget->setObjectName("keywordWidget");
	m_pNoteKeywordWidget->setMode(KWME_SELECTOR);
	pLayout->addWidget(m_pNoteKeywordWidget);

	// --------------------------------

	m_pSearchResultsTreeView = new CSearchResultsTreeView(m_pBibleDatabase, g_pUserNotesDatabase, this);
	m_pSearchResultsTreeView->setObjectName(QString::fromUtf8("SearchResultsTreeView"));
	QSizePolicy aSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	aSizePolicy.setHorizontalStretch(10);
	aSizePolicy.setVerticalStretch(0);
	aSizePolicy.setHeightForWidth(m_pSearchResultsTreeView->sizePolicy().hasHeightForWidth());
	m_pSearchResultsTreeView->setSizePolicy(aSizePolicy);
	pLayout->addWidget(m_pSearchResultsTreeView);

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_RESULTS_SPY
	CMyApplication::createSpy(this);
#endif
#endif

	// --------------------------------

	connect(CPersistentSettings::instance(), SIGNAL(changedHideNotFoundInStatistics(bool)), this, SLOT(setSearchResultsSummaryText()));

	// --------------------------------

	if (m_pShowHighlightersInSearchResults != NULL) {
		m_pShowHighlightersInSearchResults->setChecked(m_pSearchResultsTreeView->vlmodel()->showHighlightersInSearchResults());
		connect(m_pShowHighlightersInSearchResults, SIGNAL(clicked(bool)), m_pSearchResultsTreeView->vlmodel(), SLOT(setShowHighlightersInSearchResults(bool)));
	}

	// --------------------------------

	// Setup our keyword model -- note we have to do this after setting up our SearchResultsTreeView
	//		since it will call it to set the keyword list for filtering:
	m_pNoteKeywordWidget->setKeywordList(g_pUserNotesDatabase->compositeKeywordList(), g_pUserNotesDatabase->compositeKeywordList());
	keywordListChanged(true);

	connect(m_pNoteKeywordWidget, SIGNAL(keywordListChanged()), this, SLOT(en_modelKeywordListChanged()));
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedUserNotesKeywords()), this, SLOT(keywordListChanged()));

	// -------------------- Search Results List View:

	setViewMode(viewMode());		// This call will setup our UI so don't have redundant code

	connect(this, SIGNAL(changedSearchResults()), m_pSearchResultsTreeView, SLOT(en_listChanged()));

	// Set Outgoing Pass-Through Signals:
	connect(m_pSearchResultsTreeView, SIGNAL(searchResultActivated(const QModelIndex &)), this, SIGNAL(searchResultActivated(const QModelIndex &)));
	connect(m_pSearchResultsTreeView, SIGNAL(gotoIndex(const TPhraseTag &)), this, SIGNAL(gotoIndex(const TPhraseTag &)));
	connect(m_pSearchResultsTreeView, SIGNAL(currentItemChanged()), this, SIGNAL(setDetailsEnable()));

	connect(m_pSearchResultsTreeView, SIGNAL(activatedSearchResults()), this, SIGNAL(activatedSearchResults()));
	connect(m_pSearchResultsTreeView, SIGNAL(canExpandAll(bool)), this, SIGNAL(canExpandAll(bool)));
	connect(m_pSearchResultsTreeView, SIGNAL(canCollapseAll(bool)), this, SIGNAL(canCollapseAll(bool)));
	connect(m_pSearchResultsTreeView, SIGNAL(currentItemChanged()), this, SIGNAL(currentItemChanged()));

	connect(vlmodel(), SIGNAL(searchResultsReady()), this, SLOT(en_searchResultsReady()));
	connect(vlmodel(), SIGNAL(searchResultsReady()), this, SIGNAL(searchResultsReady()));

	// Set Incoming Pass-Through Signals:
	connect(this, SIGNAL(expandAll()), m_pSearchResultsTreeView, SLOT(expandAll()));
	connect(this, SIGNAL(collapseAll()), m_pSearchResultsTreeView, SLOT(collapseAll()));
	connect(this, SIGNAL(setFontSearchResults(const QFont &)), m_pSearchResultsTreeView, SLOT(setFontSearchResults(const QFont &)));
	connect(this, SIGNAL(setTextBrightness(bool, int)), m_pSearchResultsTreeView, SLOT(setTextBrightness(bool, int)));

	setSearchResultsType();
}

CKJVSearchResult::~CKJVSearchResult()
{

}

QModelIndex CKJVSearchResult::currentIndex() const
{
	return m_pSearchResultsTreeView->currentIndex();
}

TVerseIndex CKJVSearchResult::currentVerseIndex() const
{
	return m_pSearchResultsTreeView->currentVerseIndex();
}

bool CKJVSearchResult::setCurrentIndex(const TVerseIndex &ndx, bool bFocusTreeView)
{
	return m_pSearchResultsTreeView->setCurrentIndex(ndx, bFocusTreeView);
}

void CKJVSearchResult::setFocusSearchResult()
{
	m_pSearchResultsTreeView->setFocus();
}

bool CKJVSearchResult::hasFocusSearchResult() const
{
	return m_pSearchResultsTreeView->hasFocus();
}

void CKJVSearchResult::showPassageNavigator()
{
	m_pSearchResultsTreeView->showPassageNavigator();
}

void CKJVSearchResult::showDetails()
{
	m_pSearchResultsTreeView->showDetails();
}

bool CKJVSearchResult::editableNodeSelected() const
{
	return m_pSearchResultsTreeView->editableNodeSelected();
}

void CKJVSearchResult::setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode)
{
	m_pSearchResultsCount->setVisible(nViewMode == CVerseListModel::VVME_SEARCH_RESULTS);
	m_pExcludedSearchResultsCount->setVisible(nViewMode == CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED);
	if (m_pShowHighlightersInSearchResults != NULL) {
		m_pShowHighlightersInSearchResults->setVisible((nViewMode == CVerseListModel::VVME_SEARCH_RESULTS) ||
													   (nViewMode == CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED));
	}
	m_pNoteKeywordWidget->setVisible(nViewMode == CVerseListModel::VVME_USERNOTES);
	m_pSearchResultsTreeView->setViewMode(nViewMode);
	setSearchResultsType();
}

void CKJVSearchResult::setSearchResultsType()
{
	QString strResultsType;

	switch (m_pSearchResultsTreeView->viewMode()) {
		case CVerseListModel::VVME_SEARCH_RESULTS:
			strResultsType = tr("Search Results", "MainMenu");
			break;
		case CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED:
			strResultsType = tr("Excluded Search Results", "MainMenu");
			break;
		case CVerseListModel::VVME_HIGHLIGHTERS:
			strResultsType = tr("Highlighters", "MainMenu");
			break;
		case CVerseListModel::VVME_USERNOTES:
			strResultsType = tr("Notes", "MainMenu");
			break;
		case CVerseListModel::VVME_CROSSREFS:
			strResultsType = tr("Cross References", "MainMenu");
			break;
		default:
			assert(false);
			break;
	}

	m_pSearchResultsType->setText(QString("<h2><b>%1</b></h2>").arg(strResultsType));
}

void CKJVSearchResult::setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	m_pSearchResultsTreeView->setDisplayMode(nDisplayMode);
}

void CKJVSearchResult::setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode)
{
	m_pSearchResultsTreeView->setTreeMode(nTreeMode);
}

void CKJVSearchResult::setShowMissingLeafs(bool bShowMissing)
{
	m_pSearchResultsTreeView->setShowMissingLeafs(bShowMissing);
}

void CKJVSearchResult::setShowHighlightersInSearchResults(bool bShowHighlightersInSearchResults)
{
	m_pSearchResultsTreeView->setShowHighlightersInSearchResults(bShowHighlightersInSearchResults);
	if (m_pShowHighlightersInSearchResults != NULL) {
		m_pShowHighlightersInSearchResults->setChecked(bShowHighlightersInSearchResults);
	}
}

void CKJVSearchResult::setSingleCrossRefSourceIndex(const CRelIndex &ndx)
{
	m_pSearchResultsTreeView->setSingleCrossRefSourceIndex(ndx);
}

void CKJVSearchResult::setParsedPhrases(const CSearchResultsData &searchResultsData)
{
	m_pSearchResultsTreeView->setParsedPhrases(searchResultsData);
}

void CKJVSearchResult::en_searchResultsReady()
{
	m_SearchResultsSummary.setFromVerseListModel(*vlmodel());
	setSearchResultsSummaryText();
}

void CKJVSearchResult::setSearchResultsSummaryText()
{
	m_pSearchResultsCount->setText(m_SearchResultsSummary.summaryDisplayText(m_pBibleDatabase, false));
	m_pExcludedSearchResultsCount->setText(m_SearchResultsSummary.summaryDisplayText(m_pBibleDatabase, true));
}

QString CKJVSearchResult::searchResultsSummaryText() const
{
	return m_SearchResultsSummary.summaryCopyText(m_pBibleDatabase);
}

// ----------------------------------------------------------------------------

void CKJVSearchResult::keywordListChanged(bool bInitialLoad)
{
	assert(!g_pUserNotesDatabase.isNull());

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	QStringList lstCompositeKeywords;
	lstCompositeKeywords.append(QString());			// Special entry for notes without keywords
	lstCompositeKeywords.append(g_pUserNotesDatabase->compositeKeywordList());

	if (bInitialLoad) {
		m_pNoteKeywordWidget->setKeywordList(lstCompositeKeywords, lstCompositeKeywords);
	} else {
		m_pNoteKeywordWidget->setKeywordList(m_pNoteKeywordWidget->selectedKeywordList(), lstCompositeKeywords);
	}

	m_bDoingUpdate = false;

	en_modelKeywordListChanged();
}

void CKJVSearchResult::en_modelKeywordListChanged()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	QStringList lstKeywordFilter = m_pNoteKeywordWidget->selectedKeywordList();

	if (m_pNoteKeywordWidget->isAllKeywordsSelected()) {
		lstKeywordFilter.clear();			// Special shortcut for select all so VerseListModel won't have to even check for intersection
	} else {
		// Decompose our keyword matches for filtering:
		for (int ndx = 0; ndx < lstKeywordFilter.size(); ++ndx) {
			if (!lstKeywordFilter.at(ndx).isEmpty()) {
				lstKeywordFilter.replace(ndx, CSearchStringListModel::decompose(lstKeywordFilter.at(ndx), false));
			}
		}
	}

	vlmodel()->setUserNoteKeywordFilter(lstKeywordFilter);

	m_bDoingUpdate = false;
}

// ============================================================================
