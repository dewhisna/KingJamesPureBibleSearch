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
		m_pBibleDatabase(pBibleDatabase),
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
		m_pActionNavigator(NULL),
		m_pStatusAction(NULL)
{
	assert(m_pBibleDatabase.data() != NULL);

	setMouseTracking(true);
	setRootIsDecorated(false);

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_RESULTS_SPY
	CMyApplication::createSpy(this);
#endif
#endif

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
	m_pEditMenuLocal->addSeparator();
	m_pActionNavigator = m_pEditMenuLocal->addAction(tr("Passage &Navigator..."));
	m_pActionNavigator->setEnabled(false);
	connect(m_pActionNavigator, SIGNAL(triggered()), this, SLOT(showPassageNavigator()));
	m_pActionNavigator->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	// ----

	m_pStatusAction = new QAction(this);
}

CSearchResultsTreeView::~CSearchResultsTreeView()
{
}

void CSearchResultsTreeView::en_copyVerseText()
{
	assert(m_pBibleDatabase.data() != NULL);

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QTextDocument docList;
	QTextCursor cursorDocList(&docList);

	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();

	CVerseList lstVerses;
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = lstSelectedItems.at(ndx).internalId();
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
				const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
				lstVerses.append(item);
			}
		}
	}
	sortVerseList(lstVerses, Qt::AscendingOrder);

	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(lstVerses.at(ndx));
		QTextDocument docVerse;
		CPhraseNavigator navigator(m_pBibleDatabase, docVerse);
		CSearchResultHighlighter highlighter(item.phraseTags());

		// Note:  Qt bug with fragments causes leading <hr /> tags
		//		to get converted to <br /> tags.  Since this may
		//		change on us if/when they get it fixed, we'll pass
		//		false here and set our <hr /> or <br /> below as
		//		desired:
		navigator.setDocumentToVerse(item.getIndex(), false);
		navigator.doHighlighting(highlighter);
		navigator.removeAnchors();

		QTextDocumentFragment fragment(&docVerse);
		cursorDocList.insertFragment(fragment);
//		if (ndx != (lstVerses.size()-1)) cursorDocList.insertHtml("<hr />\n");
		if (ndx != (lstVerses.size()-1)) cursorDocList.insertHtml("<br />\n");
	}

	mime->setText(docList.toPlainText());
	mime->setHtml(docList.toHtml());
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyRaw()
{
	copyRawCommon(false);
}

void CSearchResultsTreeView::en_copyVeryRaw()
{
	copyRawCommon(true);
}

void CSearchResultsTreeView::copyRawCommon(bool bVeryRaw) const
{
	assert(m_pBibleDatabase.data() != NULL);

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QString strText;

	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();

	CVerseList lstVerses;
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = lstSelectedItems.at(ndx).internalId();
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
				const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
				lstVerses.append(item);
			}
		}
	}
	sortVerseList(lstVerses, Qt::AscendingOrder);

	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(lstVerses.at(ndx));
		QTextDocument docVerse;
		CPhraseNavigator navigator(m_pBibleDatabase, docVerse);
		navigator.setDocumentToVerse(item.getIndex(), false);

		QTextCursor cursorDocVerse(&docVerse);
		cursorDocVerse.select(QTextCursor::Document);
		CSelectedPhrase phrase = navigator.getSelectedPhrase(cursorDocVerse);

		if (!bVeryRaw) {
			strText += phrase.phrase().phrase() + "\n";
		} else {
			strText += phrase.phrase().phraseRaw() + "\n";
		}
	}

	mime->setText(strText);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyVerseHeadings()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QString strVerseHeadings;

	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();

	CVerseList lstVerses;
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = lstSelectedItems.at(ndx).internalId();
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
				const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
				lstVerses.append(item);
			}
		}
	}
	sortVerseList(lstVerses, Qt::AscendingOrder);

	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(lstVerses.at(ndx));
		strVerseHeadings += item.getHeading() + "\n";
	}

	mime->setText(strVerseHeadings);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyReferenceDetails()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QString strPlainText;
	QString strRichText;

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();

	CVerseList lstVerses;
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = lstSelectedItems.at(ndx).internalId();
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
				const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
				lstVerses.append(item);
			}
		}
	}
	sortVerseList(lstVerses, Qt::AscendingOrder);

	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		if (ndx > 0) {
			strPlainText += "--------------------\n";
			strRichText += "<hr />\n";
		}
		strPlainText += pModel->dataForVerse(lstVerses.at(ndx), CVerseListModel::TOOLTIP_PLAINTEXT_ROLE).toString();
		strRichText += pModel->dataForVerse(lstVerses.at(ndx), CVerseListModel::TOOLTIP_ROLE).toString();
	}

	mime->setText(strPlainText);
	mime->setHtml(strRichText);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::en_copyComplete()
{
	assert(m_pBibleDatabase.data() != NULL);

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QTextDocument docList;
	QTextCursor cursorDocList(&docList);

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();

	CVerseList lstVerses;
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = lstSelectedItems.at(ndx).internalId();
			if ((ndxRel.isSet()) && (ndxRel.verse() != 0)) {
				const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
				lstVerses.append(item);
			}
		}
	}
	sortVerseList(lstVerses, Qt::AscendingOrder);

	for (int ndx = 0; ndx < lstVerses.size(); ++ndx) {
		const CVerseListItem &item(lstVerses.at(ndx));
		QTextDocument docVerse;
		CPhraseNavigator navigator(m_pBibleDatabase, docVerse);
		CSearchResultHighlighter highlighter(item.phraseTags());

		// Note:  Qt bug with fragments causes leading <hr /> tags
		//		to get converted to <br /> tags.  Since this may
		//		change on us if/when they get it fixed, we'll pass
		//		false here and set our <hr /> or <br /> below as
		//		desired:
		navigator.setDocumentToVerse(item.getIndex(), false);
		navigator.doHighlighting(highlighter);
		navigator.removeAnchors();

		QTextDocumentFragment fragment(&docVerse);
		cursorDocList.insertFragment(fragment);

		cursorDocList.insertHtml("<br />\n<pre>" + pModel->dataForVerse(item, CVerseListModel::TOOLTIP_NOHEADING_PLAINTEXT_ROLE).toString() + "</pre>\n");
		if (ndx != (lstVerses.size()-1)) cursorDocList.insertHtml("\n<hr /><br />\n");
	}

	mime->setText(docList.toPlainText());
	mime->setHtml(docList.toHtml());
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::showPassageNavigator()
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndxRel;

	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	if (lstSelectedItems.size() == 1) {
		if (!lstSelectedItems.at(0).isValid()) return;
		ndxRel = lstSelectedItems.at(0).internalId();
		assert(ndxRel.isSet());
		if (!ndxRel.isSet()) return;
	} else {
		ndxRel = currentIndex().internalId();
		assert(ndxRel.isSet());			// Show have had one or the othe because of CKJVSearchResult::canShowPassageNavigator()
		if (!ndxRel.isSet()) return;
	}

//	const CVerseListItem &item(lstSelectedItems.at(0).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
	CKJVPassageNavigatorDlg dlg(m_pBibleDatabase, this);

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
			CRelIndex ndxRel = lstSelectedItems.at(ndx).internalId();
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

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);

	QString strStatusText = tr("%n Search Result(s) Selected", NULL, nNumResultsSelected);
	setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();
}

void CSearchResultsTreeView::en_listChanged()
{
	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);

	int nResultsCount = pModel->GetResultsCount();

	m_pActionSelectAll->setEnabled(nResultsCount != 0);
	emit canExpandAll((pModel->treeMode() != CVerseListModel::VTME_LIST) && (pModel->hasChildren()));
	emit canCollapseAll((pModel->treeMode() != CVerseListModel::VTME_LIST) && (pModel->hasChildren()));

	handle_selectionChanged();
}

void CSearchResultsTreeView::showDetails()
{
	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);

	QVariant varTooltip = pModel->data(currentIndex(), CVerseListModel::TOOLTIP_ROLE);
	if (varTooltip.canConvert<QString>()) {
		scrollTo(currentIndex(), QAbstractItemView::EnsureVisible);

//		QToolTip::showText(mapToGlobal(visualRect(currentIndex()).topRight()), varTooltip.toString(), this);
		QToolTip::hideText();
		CToolTipEdit::showText(mapToGlobal(visualRect(currentIndex()).topRight()), varTooltip.toString(), this, rect());
	}
}

bool CSearchResultsTreeView::haveDetails() const
{
	if (!currentIndex().isValid()) return false;

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);

	QVariant varTooltip = pModel->data(currentIndex(), CVerseListModel::TOOLTIP_ROLE);
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
	m_pSearchResultsTreeView(NULL),
	m_pReflowDelegate(NULL)
{
	assert(m_pBibleDatabase.data() != NULL);

	QVBoxLayout *pLayout = new QVBoxLayout(this);
	pLayout->setSpacing(6);
	pLayout->setContentsMargins(11, 11, 11, 11);
	pLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	pLayout->setContentsMargins(0, 0, 0, 0);
	m_pSearchResultsCount = new QLabel(this);
	m_pSearchResultsCount->setObjectName(QString::fromUtf8("SearchResultsCount"));
	m_pSearchResultsCount->setWordWrap(true);
	m_pSearchResultsCount->setText(tr("Found 0 Occurrences") + "\n"
									  "    " + tr("in 0 Verses in 0 Chapters in 0 Books"));
	pLayout->addWidget(m_pSearchResultsCount);

	m_pSearchResultsTreeView = new CSearchResultsTreeView(m_pBibleDatabase, this);
	m_pSearchResultsTreeView->setObjectName(QString::fromUtf8("SearchResultsTreeView"));
	QSizePolicy aSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	aSizePolicy.setHorizontalStretch(10);
	aSizePolicy.setVerticalStretch(0);
	aSizePolicy.setHeightForWidth(m_pSearchResultsTreeView->sizePolicy().hasHeightForWidth());
	m_pSearchResultsTreeView->setSizePolicy(aSizePolicy);
	m_pSearchResultsTreeView->setMouseTracking(true);
	m_pSearchResultsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pSearchResultsTreeView->setProperty("showDropIndicator", QVariant(false));
	m_pSearchResultsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pSearchResultsTreeView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_pSearchResultsTreeView->setRootIsDecorated(false);
	m_pSearchResultsTreeView->setExpandsOnDoubleClick(false);
	m_pSearchResultsTreeView->setProperty("isWrapping", QVariant(false));
	m_pSearchResultsTreeView->header()->setVisible(false);
	m_pSearchResultsTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pSearchResultsTreeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	pLayout->addWidget(m_pSearchResultsTreeView);

#ifdef SIGNAL_SPY_DEBUG
#ifdef SEARCH_RESULTS_SPY
	CMyApplication::createSpy(this);
#endif
#endif

	// -------------------- Search Results List View:

	CVerseListModel *pModel = new CVerseListModel(m_pBibleDatabase, m_pSearchResultsTreeView);
	QAbstractItemModel *pOldModel = m_pSearchResultsTreeView->model();
	m_pSearchResultsTreeView->setModel(pModel);
	if (pOldModel) delete pOldModel;
	m_pSearchResultsTreeView->setRootIsDecorated(pModel->treeMode() != CVerseListModel::VTME_LIST);

	m_pReflowDelegate = new CReflowDelegate(m_pSearchResultsTreeView, true, true);
	CVerseListDelegate *pDelegate = new CVerseListDelegate(*pModel, m_pSearchResultsTreeView);
	m_pReflowDelegate->setItemDelegate(pDelegate);
	QAbstractItemDelegate *pOldDelegate = m_pSearchResultsTreeView->itemDelegate();
	m_pSearchResultsTreeView->setItemDelegate(m_pReflowDelegate);
	if (pOldDelegate) delete pOldDelegate;
	m_pReflowDelegate->setFakeSizeHintRowCount((pModel->displayMode() != CVerseListModel::VDME_HEADING) ? 4 : 1);

	connect(this, SIGNAL(changedSearchResults()), m_pSearchResultsTreeView, SLOT(en_listChanged()));
	connect(model(), SIGNAL(modelReset()), m_pSearchResultsTreeView, SLOT(en_listChanged()));
	connect(model(), SIGNAL(layoutChanged()), m_pSearchResultsTreeView, SLOT(en_listChanged()));

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
}

CKJVSearchResult::~CKJVSearchResult()
{
}

CRelIndex CKJVSearchResult::currentIndex() const
{
	return CRelIndex(m_pSearchResultsTreeView->currentIndex().internalId());
}

bool CKJVSearchResult::setCurrentIndex(const CRelIndex &ndx, bool bFocusTreeView)
{
	CVerseListModel *pModel = m_pSearchResultsTreeView->vlmodel();
	assert(pModel != NULL);

	QModelIndex ndxModel = pModel->locateIndex(ndx);
	m_pSearchResultsTreeView->setCurrentIndex(ndxModel);
	m_pSearchResultsTreeView->scrollTo(ndxModel, QAbstractItemView::EnsureVisible);
	if (bFocusTreeView) m_pSearchResultsTreeView->setFocus();
	return ndxModel.isValid();
}

void CKJVSearchResult::setFocusSearchResult()
{
	m_pSearchResultsTreeView->setFocus();
}

bool CKJVSearchResult::hasFocusSearchResult() const
{
	return m_pSearchResultsTreeView->hasFocus();
}

bool CKJVSearchResult::canShowPassageNavigator() const
{
	return ((m_pSearchResultsTreeView->selectionModel()->selectedRows().count() == 1) || (currentIndex().isSet()));
}

void CKJVSearchResult::setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	model()->setDisplayMode(nDisplayMode);
	m_pReflowDelegate->setFakeSizeHintRowCount((model()->displayMode() != CVerseListModel::VDME_HEADING) ? 4 : 1);
}

void CKJVSearchResult::setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode)
{
	model()->setTreeMode(nTreeMode);
	m_pSearchResultsTreeView->setRootIsDecorated(nTreeMode != CVerseListModel::VTME_LIST);
}

void CKJVSearchResult::setShowMissingLeafs(bool bShowMissing)
{
	model()->setShowMissingLeafs(bShowMissing);
}

void CKJVSearchResult::setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases)
{
	m_LastSearchCriteria = aSearchCriteria;
	model()->setParsedPhrases(aSearchCriteria, phrases);
	m_nLastSearchNumPhrases = phrases.size();

	int nVerses = 0;		// Results counts in Verses
	int nChapters = 0;		// Results counts in Chapters
	int nBooks = 0;			// Results counts in Books
	int nResults = 0;		// Total number of Results in Scope

	nVerses = model()->GetVerseIndexAndCount().second;
	nChapters = model()->GetChapterIndexAndCount().second;
	nBooks = model()->GetBookIndexAndCount().second;
	nResults = model()->GetResultsCount();

	QString strResults;

	strResults += tr("Found %n Occurrence(s)", NULL, nResults) + "\n";
	strResults += "    " + tr("in %n Verse(s)", NULL, nVerses) +
					" " + tr("in %n Chapter(s)", NULL, nChapters) +
					" " + tr("in %n Book(s)", NULL, nBooks);
	if (nResults > 0) {
		strResults += "\n";
		strResults += "    " + tr("Not found at all in %n Verse(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumVrs - nVerses) + "\n";
		strResults += "    " + tr("Not found at all in %n Chapter(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumChp - nChapters) + "\n";
		strResults += "    " + tr("Not found at all in %n Book(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumBk - nBooks);
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
		strSummary += "\n";
		strSummary += tr("Not found%1 at all in %n Verse(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumVrs - m_nLastSearchVerses).arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? (" " + tr("together")) : "") + "\n";
		strSummary += tr("Not found%1 at all in %n Chapter(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumChp - m_nLastSearchChapters).arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? (" " + tr("together")) : "") + "\n";
		strSummary += tr("Not found%1 at all in %n Book(s) of the Bible", NULL, m_pBibleDatabase->bibleEntry().m_nNumBk - m_nLastSearchBooks).arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? (" " + tr("together")) : "") + "\n";
	} else {
		strSummary += tr("Search was incomplete -- too many possible matches") + "\n";
	}

	return strSummary;
}

void CKJVSearchResult::showPassageNavigator()
{
	m_pSearchResultsTreeView->showPassageNavigator();
}

void CKJVSearchResult::showDetails()
{
	m_pSearchResultsTreeView->showDetails();
}

