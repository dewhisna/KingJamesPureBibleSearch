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

#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "KJVPassageNavigatorDlg.h"
#include "Highlighter.h"
#include "KJVCanOpener.h"
#include "ReflowDelegate.h"
#include "PersistentSettings.h"
#include "NoteKeywordWidget.h"
#include "KJVNoteEditDlg.h"
#include "KJVCrossRefEditDlg.h"
#include "SearchCompleter.h"

#ifdef SIGNAL_SPY_DEBUG
#include "main.h"
#endif

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
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QToolTip>
#include <ToolTipEdit.h>

// ============================================================================

CSearchResultsTreeView::CSearchResultsTreeView(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QTreeView(parent),
		m_bInvertTextBrightness(false),
		m_nTextBrightness(100),
		m_bDoingPopup(false),
		m_pEditMenu(NULL),
		m_pEditMenuLocal(NULL),
		m_pActionCopyVerseText(NULL),
		m_pActionCopyRaw(NULL),
		m_pActionCopyVeryRaw(NULL),
		m_pActionCopyVerseHeadings(NULL),
		m_pActionCopyReferenceDetails(NULL),
		m_pActionCopyComplete(NULL),
		m_pActionSelectAll(NULL),
		m_pActionClearSelection(NULL),
		m_pMenuInsertionPoint(NULL),
		m_pActionNavigator(NULL),
		m_pStatusAction(NULL),
		m_pReflowDelegate(NULL)
{
	assert(pBibleDatabase.data() != NULL);

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_RESULTS_SPY
	CMyApplication::createSpy(this);
#endif
#endif

	setMouseTracking(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setProperty("showDropIndicator", QVariant(false));
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	// setRootIsDecorated(false);		// Set below based on vlmodel() type
	setExpandsOnDoubleClick(false);
	setProperty("isWrapping", QVariant(false));
	header()->setVisible(false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	CVerseListModel *pModel = new CVerseListModel(pBibleDatabase, this);
	QAbstractItemModel *pOldModel = model();
	setModel(pModel);
	assert(pModel == vlmodel());
	if (pOldModel) delete pOldModel;
	setRootIsDecorated((vlmodel()->treeMode() != CVerseListModel::VTME_LIST) || (vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS));

	m_pReflowDelegate = new CReflowDelegate(this, true, true);
	CVerseListDelegate *pDelegate = new CVerseListDelegate(*vlmodel(), this);
	m_pReflowDelegate->setItemDelegate(pDelegate);
	QAbstractItemDelegate *pOldDelegate = itemDelegate();
	setItemDelegate(m_pReflowDelegate);
	if (pOldDelegate) delete pOldDelegate;
	m_pReflowDelegate->setFakeSizeHintRowCount((vlmodel()->displayMode() != CVerseListModel::VDME_HEADING) ? 4 : 1);

	// Setup Default Font and TextBrightness:
	setFontSearchResults(CPersistentSettings::instance()->fontSearchResults());
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedSearchResults(const QFont &)), this, SLOT(setFontSearchResults(const QFont &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));

	// Setup our Context Menu:
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pEditMenuLocal = new QMenu(tr("&Edit"), this);
	m_pEditMenu->setStatusTip(tr("Search Results Edit Operations"));
	// ----
	m_pActionCopyVerseText = m_pEditMenu->addAction(tr("Copy &Verse Text"), this, SLOT(en_copyVerseText()), QKeySequence(Qt::CTRL + Qt::Key_V));
	m_pActionCopyVerseText->setStatusTip(tr("Copy Verse Text for the selected Search Results to the clipboard"));
	m_pActionCopyVerseText->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVerseText);
	m_pActionCopyRaw = m_pEditMenu->addAction(tr("Copy Raw Verse &Text (No headings)"), this, SLOT(en_copyRaw()), QKeySequence(Qt::CTRL + Qt::Key_T));
	m_pActionCopyRaw->setStatusTip(tr("Copy selected Search Results as raw phrase words to the clipboard"));
	m_pActionCopyRaw->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyRaw);
	m_pActionCopyVeryRaw = m_pEditMenu->addAction(tr("Copy Very Ra&w Verse Text (No punctuation)"), this, SLOT(en_copyVeryRaw()), QKeySequence(Qt::CTRL + Qt::Key_W));
	m_pActionCopyVeryRaw->setStatusTip(tr("Copy selected Search Results as very raw (no punctuation) phrase words to the clipboard"));
	m_pActionCopyVeryRaw->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVeryRaw);
	// ----
	m_pEditMenu->addSeparator();
	m_pEditMenuLocal->addSeparator();
	m_pActionCopyVerseHeadings = m_pEditMenu->addAction(tr("Copy &References"), this, SLOT(en_copyVerseHeadings()), QKeySequence(Qt::CTRL + Qt::Key_R));
	m_pActionCopyVerseHeadings->setStatusTip(tr("Copy Verse References for the selected Search Results to the clipboard"));
	m_pActionCopyVerseHeadings->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVerseHeadings);
	m_pActionCopyReferenceDetails = m_pEditMenu->addAction(tr("Copy Reference Detai&ls (Word/Phrase Counts)"), this, SLOT(en_copyReferenceDetails()), QKeySequence(Qt::CTRL + Qt::Key_L));
	m_pActionCopyReferenceDetails->setStatusTip(tr("Copy the Word/Phrase Reference Details (Counts) for the selected Search Results to the clipboard"));
	m_pActionCopyReferenceDetails->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyReferenceDetails);
	m_pActionCopyComplete = m_pEditMenu->addAction(tr("Copy &Complete Verse Text and Reference Details"), this, SLOT(en_copyComplete()), QKeySequence(Qt::CTRL + Qt::Key_C));
	m_pActionCopyComplete->setStatusTip(tr("Copy Complete Verse Text and Reference Details (Counts) for the selected Search Results to the clipboard"));
	m_pActionCopyComplete->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyComplete);
	// ----
	m_pEditMenu->addSeparator();
	m_pEditMenuLocal->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction(tr("Select &All"), this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip(tr("Select all Search Results"));
	m_pActionSelectAll->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionSelectAll);
	m_pActionClearSelection = m_pEditMenu->addAction(tr("C&lear Selection"), this, SLOT(clearSelection()), QKeySequence(Qt::Key_Escape));
	m_pActionClearSelection->setStatusTip(tr("Clear Search Results Selection"));
	m_pActionClearSelection->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionClearSelection);
	// ----
	m_pMenuInsertionPoint = m_pEditMenuLocal->addSeparator();
	m_pEditMenuLocal->addAction(CKJVNoteEditDlg::actionUserNoteEditor());
	// ----
	m_pEditMenuLocal->addSeparator();
	m_pEditMenuLocal->addAction(CKJVCrossRefEditDlg::actionCrossRefsEditor());
	// ----
	m_pEditMenuLocal->addSeparator();
	m_pActionNavigator = m_pEditMenuLocal->addAction(QIcon(":/res/green_arrow.png"), tr("Passage &Navigator..."));
	m_pActionNavigator->setEnabled(false);
	connect(m_pActionNavigator, SIGNAL(triggered()), this, SLOT(showPassageNavigator()));
	m_pActionNavigator->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	// ----

	m_pStatusAction = new QAction(this);

	// Setup our change notifications:
	connect(vlmodel(), SIGNAL(modelReset()), this, SLOT(en_listChanged()));
	connect(vlmodel(), SIGNAL(layoutChanged()), this, SLOT(en_listChanged()));
}

CSearchResultsTreeView::~CSearchResultsTreeView()
{
}

// ----------------------------------------------------------------------------

QModelIndexList CSearchResultsTreeView::getSelectedVerses() const
{
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();

	for (int ndx = 0; ndx < lstSelectedItems.size(); /* Increment inside loop */) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = vlmodel()->navigationIndexForModelIndex(lstSelectedItems.at(ndx));
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
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

// ----------------------------------------------------------------------------

void CSearchResultsTreeView::en_copyVerseText() const
{
	assert(vlmodel()->bibleDatabase().data() != NULL);

	QModelIndexList lstVerses = getSelectedVerses();

	QTextDocument docList;
	QTextCursor cursorDocList(&docList);
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(vlmodel()->dataForVerse(lstVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		QTextDocument docVerse;
		CPhraseNavigator navigator(vlmodel()->bibleDatabase(), docVerse);

		// Note:  Qt bug with fragments causes leading <hr /> tags
		//		to get converted to <br /> tags.  Since this may
		//		change on us if/when they get it fixed, we'll pass
		//		TRO_None here and set our <hr /> or <br /> below as
		//		desired:
		navigator.setDocumentToVerse(item.getIndex());
		if (viewMode() == CVerseListModel::VVME_SEARCH_RESULTS) {
			CSearchResultHighlighter highlighter(item.phraseTags());
			navigator.doHighlighting(highlighter);
		} else if (viewMode() == CVerseListModel::VVME_HIGHLIGHTERS) {
			CUserDefinedHighlighter highlighter(vlmodel()->results(*item.verseIndex()).resultsName(), item.phraseTags());
			navigator.doHighlighting(highlighter);
		}
		navigator.removeAnchors();

		QTextDocumentFragment fragment(&docVerse);
		cursorDocList.insertFragment(fragment);
//		if (ndx != (lstVerses.size()-1)) cursorDocList.insertHtml("<hr />\n");
		if (ndx != (lstVerses.size()-1)) cursorDocList.insertHtml("<br />\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(docList.toPlainText());
	mime->setHtml(docList.toHtml());
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyRaw() const
{
	copyRawCommon(false);
}

void CSearchResultsTreeView::en_copyVeryRaw() const
{
	copyRawCommon(true);
}

void CSearchResultsTreeView::copyRawCommon(bool bVeryRaw) const
{
	assert(vlmodel()->bibleDatabase().data() != NULL);

	QModelIndexList lstVerses = getSelectedVerses();

	QString strText;
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(vlmodel()->dataForVerse(lstVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		QTextDocument docVerse;
		CPhraseNavigator navigator(vlmodel()->bibleDatabase(), docVerse);
		navigator.setDocumentToVerse(item.getIndex());

		QTextCursor cursorDocVerse(&docVerse);
		cursorDocVerse.select(QTextCursor::Document);
		CSelectedPhrase phrase = navigator.getSelectedPhrase(cursorDocVerse);

		if (!bVeryRaw) {
			strText += phrase.phrase().phrase() + "\n";
		} else {
			strText += phrase.phrase().phraseRaw() + "\n";
		}
	}

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(strText);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyVerseHeadings() const
{
	QModelIndexList lstVerses = getSelectedVerses();

	QString strVerseHeadings;
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(vlmodel()->dataForVerse(lstVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		strVerseHeadings += item.getHeading() + "\n";
	}

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(strVerseHeadings);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyReferenceDetails() const
{
	QModelIndexList lstVerses = getSelectedVerses();

	QString strPlainText;
	QString strRichText;
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		if (ndx > 0) {
			strPlainText += "--------------------\n";
			strRichText += "<hr />\n";
		}
		strPlainText += vlmodel()->dataForVerse(lstVerses.at(ndx), CVerseListModel::TOOLTIP_PLAINTEXT_ROLE).toString();
		strRichText += vlmodel()->dataForVerse(lstVerses.at(ndx), CVerseListModel::TOOLTIP_ROLE).toString();
	}

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(strPlainText);
	mime->setHtml(strRichText);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyComplete() const
{
	assert(vlmodel()->bibleDatabase().data() != NULL);

	QModelIndexList lstVerses = getSelectedVerses();

	QTextDocument docList;
	QTextCursor cursorDocList(&docList);
	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(vlmodel()->dataForVerse(lstVerses.at(ndx), CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		QTextDocument docVerse;
		CPhraseNavigator navigator(vlmodel()->bibleDatabase(), docVerse);

		// Note:  Qt bug with fragments causes leading <hr /> tags
		//		to get converted to <br /> tags.  Since this may
		//		change on us if/when they get it fixed, we'll pass
		//		TRO_None here and set our <hr /> or <br /> below as
		//		desired:
		navigator.setDocumentToVerse(item.getIndex());
		if (viewMode() == CVerseListModel::VVME_SEARCH_RESULTS) {
			CSearchResultHighlighter highlighter(item.phraseTags());
			navigator.doHighlighting(highlighter);
		} else if (viewMode() == CVerseListModel::VVME_HIGHLIGHTERS) {
			CUserDefinedHighlighter highlighter(vlmodel()->results(*item.verseIndex()).resultsName(), item.phraseTags());
			navigator.doHighlighting(highlighter);
		}
		navigator.removeAnchors();

		QTextDocumentFragment fragment(&docVerse);
		cursorDocList.insertFragment(fragment);

		if (viewMode() == CVerseListModel::VVME_SEARCH_RESULTS) {
			cursorDocList.insertHtml("<br />\n<pre>" + vlmodel()->dataForVerse(lstVerses.at(ndx), CVerseListModel::TOOLTIP_NOHEADING_PLAINTEXT_ROLE).toString() + "</pre>\n");
			if (ndx != (lstVerses.size()-1)) cursorDocList.insertHtml("\n<hr /><br />\n");
		} else {
			cursorDocList.insertHtml("<br />\n");
		}
	}

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	mime->setText(docList.toPlainText());
	mime->setHtml(docList.toHtml());
	clipboard->setMimeData(mime);
}

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

bool CSearchResultsTreeView::canShowPassageNavigator() const
{
	return ((selectionModel()->selectedRows().count() == 1) || (vlmodel()->navigationIndexForModelIndex(currentIndex()).isSet()));
}

void CSearchResultsTreeView::setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode)
{
	// Set root decoration before switching mode so en_listChanged emits canExpandAll/canCollapseAll correctly
	setRootIsDecorated((vlmodel()->treeMode() != CVerseListModel::VTME_LIST) || (nViewMode != CVerseListModel::VVME_SEARCH_RESULTS));
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
	setRootIsDecorated((nTreeMode != CVerseListModel::VTME_LIST) || (vlmodel()->viewMode() != CVerseListModel::VVME_SEARCH_RESULTS));
	vlmodel()->setTreeMode(nTreeMode);
}

void CSearchResultsTreeView::setShowMissingLeafs(bool bShowMissing)
{
	vlmodel()->setShowMissingLeafs(bShowMissing);
}

void CSearchResultsTreeView::setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases)
{
	vlmodel()->setParsedPhrases(aSearchCriteria, phrases);
}

void CSearchResultsTreeView::showPassageNavigator()
{
	assert(vlmodel()->bibleDatabase().data() != NULL);

	CRelIndex ndxRel;

	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	if (lstSelectedItems.size() == 1) {
		if (!lstSelectedItems.at(0).isValid()) return;
		ndxRel = vlmodel()->navigationIndexForModelIndex(lstSelectedItems.at(0));
		assert(ndxRel.isSet());
		if (!ndxRel.isSet()) return;
	} else {
		ndxRel = vlmodel()->navigationIndexForModelIndex(currentIndex());
		assert(ndxRel.isSet());			// Should have had one or the other because of canShowPassageNavigator()
		if (!ndxRel.isSet()) return;
	}

//	const CVerseListItem &item(lstSelectedItems.at(0).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
	CKJVPassageNavigatorDlg dlg(vlmodel()->bibleDatabase(), this);

//	dlg.navigator().startAbsoluteMode(TPhraseTag(item.getIndex(), 0));

	dlg.navigator().startAbsoluteMode(TPhraseTag(ndxRel, 0));
	if (dlg.exec() == QDialog::Accepted) {
		emit gotoIndex(dlg.passage());
	}
}

void CSearchResultsTreeView::focusInEvent(QFocusEvent *event)
{
	emit activatedSearchResults();
	QTreeView::focusInEvent(event);
}

void CSearchResultsTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	m_bDoingPopup = true;
	m_pEditMenuLocal->exec(event->globalPos());
	m_bDoingPopup = false;
}

void CSearchResultsTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QTreeView::currentChanged(current, previous);
	emit currentItemChanged();
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
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = vlmodel()->navigationIndexForModelIndex(lstSelectedItems.at(ndx));
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
				nNumResultsSelected++;
			}
		}
	}

	if (nNumResultsSelected) {
		m_pActionCopyVerseText->setEnabled(true);
		m_pActionCopyRaw->setEnabled(true);
		m_pActionCopyVeryRaw->setEnabled(true);
		m_pActionCopyVerseHeadings->setEnabled(true);
		m_pActionCopyReferenceDetails->setEnabled(true);
		m_pActionCopyComplete->setEnabled(true);
		m_pActionClearSelection->setEnabled(true);
	} else {
		m_pActionCopyVerseText->setEnabled(false);
		m_pActionCopyRaw->setEnabled(false);
		m_pActionCopyVeryRaw->setEnabled(false);
		m_pActionCopyVerseHeadings->setEnabled(false);
		m_pActionCopyReferenceDetails->setEnabled(false);
		m_pActionCopyComplete->setEnabled(false);
		m_pActionClearSelection->setEnabled(false);
	}
	m_pActionNavigator->setEnabled(selectionModel()->selectedRows().size() == 1);		// Only allow navigation on a node (verse or otherwise)

	QString strStatusText = ((viewMode() == CVerseListModel::VVME_SEARCH_RESULTS) ?
									tr("%n Search Result(s) Selected", NULL, nNumResultsSelected) :
									tr("%n Highlighted Verse(s) Selected", NULL, nNumResultsSelected));
	setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();
}

void CSearchResultsTreeView::en_listChanged()
{
	int nResultsCount = vlmodel()->GetResultsCount();

	m_pActionSelectAll->setEnabled(nResultsCount != 0);
	emit canExpandAll((rootIsDecorated()) && (vlmodel()->hasChildren()));
	emit canCollapseAll((rootIsDecorated()) && (vlmodel()->hasChildren()));

	handle_selectionChanged();
}

void CSearchResultsTreeView::showDetails()
{
	QVariant varTooltip = vlmodel()->data(currentIndex(), CVerseListModel::TOOLTIP_ROLE);
	if (varTooltip.canConvert<QString>()) {
		scrollTo(currentIndex(), QAbstractItemView::EnsureVisible);

//		QToolTip::showText(mapToGlobal(visualRect(QTreeView::currentIndex()).topRight()), varTooltip.toString(), this);
		QToolTip::hideText();
		CToolTipEdit::showText(mapToGlobal(visualRect(currentIndex()).topRight()), varTooltip.toString(), this, rect());
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
	extern QMainWindow *g_pMainWindow;
	CKJVCanOpener *pMainWindow = qobject_cast<CKJVCanOpener *>(g_pMainWindow);
	assert(pMainWindow != NULL);
	return ((hasFocus()) || ((pMainWindow != NULL) && (pMainWindow->isSearchResultsActive())));
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

QStyleOptionViewItem CSearchResultsTreeView::viewOptions () const
{
	QStyleOptionViewItemV4 optionV4 = QTreeView::viewOptions();

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

// ============================================================================


// ============================================================================

CKJVSearchResult::CKJVSearchResult(CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QWidget(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_nLastSearchOccurrences(0),
	m_nLastSearchVerses(0),
	m_nLastSearchChapters(0),
	m_nLastSearchBooks(0),
	m_bLastCalcSuccess(true),
	m_nLastSearchNumPhrases(0),
	m_bDoingUpdate(false),
	m_pSearchResultsType(NULL),
	m_pSearchResultsCount(NULL),
	m_pNoteKeywordWidget(NULL),
	m_pSearchResultsTreeView(NULL)
{
	assert(m_pBibleDatabase != NULL);
	assert(g_pUserNotesDatabase != NULL);

	QVBoxLayout *pLayout = new QVBoxLayout(this);
	pLayout->setSpacing(4);
	pLayout->setContentsMargins(11, 11, 11, 11);
	pLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	pLayout->setContentsMargins(0, 0, 0, 0);
	m_pSearchResultsType = new QLabel(this);
	m_pSearchResultsType->setObjectName(QString::fromUtf8("SearchResultsType"));
	m_pSearchResultsType->setWordWrap(true);
	m_pSearchResultsType->setAlignment(Qt::AlignHCenter);
	m_pSearchResultsType->setTextFormat(Qt::RichText);
	pLayout->addWidget(m_pSearchResultsType);
	m_pSearchResultsCount = new QLabel(this);
	m_pSearchResultsCount->setObjectName(QString::fromUtf8("SearchResultsCount"));
	m_pSearchResultsCount->setWordWrap(true);
	m_pSearchResultsCount->setText(tr("Found 0 Occurrences") + "\n"
									  "    " + tr("in 0 Verses in 0 Chapters in 0 Books"));
	pLayout->addWidget(m_pSearchResultsCount);

	// --------------------------------

	m_pNoteKeywordWidget = new CNoteKeywordWidget(this);
	m_pNoteKeywordWidget->setObjectName("keywordWidget");
	m_pNoteKeywordWidget->setMode(CNoteKeywordWidget::KWME_SELECTOR);

	pLayout->addWidget(m_pNoteKeywordWidget);

	// --------------------------------

	m_pSearchResultsTreeView = new CSearchResultsTreeView(m_pBibleDatabase, this);
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

	// Setup our keyword model -- note we have to do this after setting up our SearchResultsTreeView
	//		since it will call it to set the keyword list for filtering:
	m_pNoteKeywordWidget->setKeywordList(g_pUserNotesDatabase->compositeKeywordList(), g_pUserNotesDatabase->compositeKeywordList());

	connect(m_pNoteKeywordWidget, SIGNAL(keywordListChanged()), this, SLOT(en_modelKeywordListChanged()));
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedUserNotesKeywords()), this, SLOT(keywordListChanged()));

	// -------------------- Search Results List View:

	connect(this, SIGNAL(changedSearchResults()), m_pSearchResultsTreeView, SLOT(en_listChanged()));

	// Set Outgoing Pass-Through Signals:
	connect(m_pSearchResultsTreeView, SIGNAL(activated(const QModelIndex &)), this, SIGNAL(activated(const QModelIndex &)));
	connect(m_pSearchResultsTreeView, SIGNAL(gotoIndex(const TPhraseTag &)), this, SIGNAL(gotoIndex(const TPhraseTag &)));
	connect(m_pSearchResultsTreeView, SIGNAL(currentItemChanged()), this, SIGNAL(setDetailsEnable()));

	connect(m_pSearchResultsTreeView, SIGNAL(activatedSearchResults()), this, SIGNAL(activatedSearchResults()));
	connect(m_pSearchResultsTreeView, SIGNAL(canExpandAll(bool)), this, SIGNAL(canExpandAll(bool)));
	connect(m_pSearchResultsTreeView, SIGNAL(canCollapseAll(bool)), this, SIGNAL(canCollapseAll(bool)));
	connect(m_pSearchResultsTreeView, SIGNAL(currentItemChanged()), this, SIGNAL(currentItemChanged()));

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

bool CKJVSearchResult::canShowPassageNavigator() const
{
	return m_pSearchResultsTreeView->canShowPassageNavigator();
}

void CKJVSearchResult::setViewMode(CVerseListModel::VERSE_VIEW_MODE_ENUM nViewMode)
{
	m_pSearchResultsCount->setVisible(nViewMode == CVerseListModel::VVME_SEARCH_RESULTS);
	m_pNoteKeywordWidget->setVisible(nViewMode == CVerseListModel::VVME_USERNOTES);
	m_pSearchResultsTreeView->setViewMode(nViewMode);
	setSearchResultsType();
}

void CKJVSearchResult::setSearchResultsType()
{
	QString strResultsType;

	switch (m_pSearchResultsTreeView->viewMode()) {
		case CVerseListModel::VVME_SEARCH_RESULTS:
			strResultsType = tr("Search Results");
			break;
		case CVerseListModel::VVME_HIGHLIGHTERS:
			strResultsType = tr("Highlighters");
			break;
		case CVerseListModel::VVME_USERNOTES:
			strResultsType = tr("Notes");
			break;
		case CVerseListModel::VVME_CROSSREFS:
			strResultsType = tr("Cross References");
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

void CKJVSearchResult::setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases)
{
	m_LastSearchCriteria = aSearchCriteria;
	m_pSearchResultsTreeView->setParsedPhrases(aSearchCriteria, phrases);
	m_nLastSearchNumPhrases = phrases.size();

	int nVerses = 0;		// Results counts in Verses
	int nChapters = 0;		// Results counts in Chapters
	int nBooks = 0;			// Results counts in Books
	int nResults = 0;		// Total number of Results in Scope

	nVerses = vlmodel()->searchResults().GetVerseIndexAndCount().second;
	nChapters = vlmodel()->searchResults().GetChapterIndexAndCount().second;
	nBooks = vlmodel()->searchResults().GetBookIndexAndCount().second;
	nResults = vlmodel()->searchResults().GetResultsCount();

	QString strResults;

	strResults += tr("Found %n Occurrence(s)", NULL, nResults) + "\n";
	strResults += "    " + tr("in %n Verse(s)", NULL, nVerses) +
					" " + tr("in %n Chapter(s)", NULL, nChapters) +
					" " + tr("in %n Book(s)", NULL, nBooks);
	if (aSearchCriteria.withinIsEntireBible(m_pBibleDatabase)) {
		if (nResults > 0) {
			strResults += "\n";
			strResults += "    " + tr("Not found at all in %n Verse(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumVrs - nVerses) + "\n";
			strResults += "    " + tr("Not found at all in %n Chapter(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumChp - nChapters) + "\n";
			strResults += "    " + tr("Not found at all in %n Book(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumBk - nBooks);
		}
	} else {
		QString strSearchWithinDescription = aSearchCriteria.searchWithinDescription(m_pBibleDatabase);
		if (!strSearchWithinDescription.isEmpty()) {
			strResults += " " + tr("within") + " " + strSearchWithinDescription;
		}
	}

	m_pSearchResultsCount->setText(strResults);

	m_bLastCalcSuccess = true;
	m_nLastSearchOccurrences = nResults;
	m_nLastSearchVerses = nVerses;
	m_nLastSearchChapters = nChapters;
	m_nLastSearchBooks = nBooks;
}

QString CKJVSearchResult::searchResultsSummaryText() const
{
	QString strSummary;

	if (m_bLastCalcSuccess) {
		strSummary += tr("Found %n %1Occurrence(s)", NULL, m_nLastSearchOccurrences).arg((m_nLastSearchNumPhrases > 1) ? (tr("Combined") + " ") : "") + "\n";
		strSummary += "    " + tr("in %n Verse(s)", NULL, m_nLastSearchVerses) + "\n";
		strSummary += "    " + tr("in %n Chapter(s)", NULL, m_nLastSearchChapters) + "\n";
		strSummary += "    " + tr("in %n Book(s)", NULL, m_nLastSearchBooks) + "\n";
		if (m_LastSearchCriteria.withinIsEntireBible(m_pBibleDatabase)) {
			strSummary += "\n";
			strSummary += tr("Not found%1 at all in %n Verse(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumVrs - m_nLastSearchVerses).arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? (" " + tr("together")) : "") + "\n";
			strSummary += tr("Not found%1 at all in %n Chapter(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumChp - m_nLastSearchChapters).arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? (" " + tr("together")) : "") + "\n";
			strSummary += tr("Not found%1 at all in %n Book(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumBk - m_nLastSearchBooks).arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? (" " + tr("together")) : "") + "\n";
		} else {
			QString strSearchWithinDescription = m_LastSearchCriteria.searchWithinDescription(m_pBibleDatabase);
			if (!strSearchWithinDescription.isEmpty()) {
				strSummary += "    " + tr("within") + " " + strSearchWithinDescription;
			}
		}
	} else {
		strSummary += tr("Search was incomplete -- too many possible matches") + "\n";
	}

	return strSummary;
}

// ----------------------------------------------------------------------------

void CKJVSearchResult::keywordListChanged(bool bInitialLoad)
{
	assert(g_pUserNotesDatabase != NULL);

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
				lstKeywordFilter.replace(ndx, CSearchStringListModel::decompose(lstKeywordFilter.at(ndx)));
			}
		}
	}

	vlmodel()->setUserNoteKeywordFilter(lstKeywordFilter);

	m_bDoingUpdate = false;
}

// ============================================================================
