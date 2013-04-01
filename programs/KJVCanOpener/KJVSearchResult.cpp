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

	m_pEditMenu = new QMenu("&Edit", this);
	m_pEditMenuLocal = new QMenu("&Edit", this);
	m_pEditMenu->setStatusTip("Search Results Edit Operations");
	// ----
	m_pActionCopyVerseText = m_pEditMenu->addAction("Copy &Verse Text", this, SLOT(on_copyVerseText()), QKeySequence(Qt::CTRL + Qt::Key_V));
	m_pActionCopyVerseText->setStatusTip("Copy Verse Text for the selected Search Results to the clipboard");
	m_pActionCopyVerseText->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVerseText);
	m_pActionCopyRaw = m_pEditMenu->addAction("Copy Raw Verse &Text (No headings)", this, SLOT(on_copyRaw()), QKeySequence(Qt::CTRL + Qt::Key_T));
	m_pActionCopyRaw->setStatusTip("Copy selected Search Results as raw phrase words to the clipboard");
	m_pActionCopyRaw->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyRaw);
	m_pActionCopyVeryRaw = m_pEditMenu->addAction("Copy Very Ra&w Verse Text (No punctuation)", this, SLOT(on_copyVeryRaw()), QKeySequence(Qt::CTRL + Qt::Key_W));
	m_pActionCopyVeryRaw->setStatusTip("Copy selected Search Results as very raw (no punctuation) phrase words to the clipboard");
	m_pActionCopyVeryRaw->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVeryRaw);
	// ----
	m_pEditMenu->addSeparator();
	m_pEditMenuLocal->addSeparator();
	m_pActionCopyVerseHeadings = m_pEditMenu->addAction("Copy &References", this, SLOT(on_copyVerseHeadings()), QKeySequence(Qt::CTRL + Qt::Key_R));
	m_pActionCopyVerseHeadings->setStatusTip("Copy Verse References for the selected Search Results to the clipboard");
	m_pActionCopyVerseHeadings->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyVerseHeadings);
	m_pActionCopyReferenceDetails = m_pEditMenu->addAction("Copy Reference Detai&ls (Word/Phrase Counts)", this, SLOT(on_copyReferenceDetails()), QKeySequence(Qt::CTRL + Qt::Key_L));
	m_pActionCopyReferenceDetails->setStatusTip("Copy the Word/Phrase Reference Details (Counts) for the selected Search Results to the clipboard");
	m_pActionCopyReferenceDetails->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyReferenceDetails);
	m_pActionCopyComplete = m_pEditMenu->addAction("Copy &Complete Verse Text and Reference Details", this, SLOT(on_copyComplete()), QKeySequence(Qt::CTRL + Qt::Key_C));
	m_pActionCopyComplete->setStatusTip("Copy Complete Verse Text and Reference Details (Counts) for the selected Search Results to the clipboard");
	m_pActionCopyComplete->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionCopyComplete);
	// ----
	m_pEditMenu->addSeparator();
	m_pEditMenuLocal->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction("Select &All", this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip("Select all Search Results");
	m_pActionSelectAll->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionSelectAll);
	m_pActionClearSelection = m_pEditMenu->addAction("C&lear Selection", this, SLOT(clearSelection()), QKeySequence(Qt::Key_Escape));
	m_pActionClearSelection->setStatusTip("Clear Search Results Selection");
	m_pActionClearSelection->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionClearSelection);
	// ----
	m_pEditMenuLocal->addSeparator();
	m_pActionNavigator = m_pEditMenuLocal->addAction("Passage &Navigator...");
	m_pActionNavigator->setEnabled(false);
	connect(m_pActionNavigator, SIGNAL(triggered()), this, SLOT(showPassageNavigator()));
	m_pActionNavigator->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	// ----

	m_pStatusAction = new QAction(this);
}

CSearchResultsTreeView::~CSearchResultsTreeView()
{
}

void CSearchResultsTreeView::on_copyVerseText()
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

void CSearchResultsTreeView::on_copyRaw()
{
	copyRawCommon(false);
}

void CSearchResultsTreeView::on_copyVeryRaw()
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
		QPair<CParsedPhrase, TPhraseTag> phrase = navigator.getSelectedPhrase(cursorDocVerse);

		if (!bVeryRaw) {
			strText += phrase.first.phrase() + "\n";
		} else {
			strText += phrase.first.phraseRaw() + "\n";
		}
	}

	mime->setText(strText);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::on_copyVerseHeadings()
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

void CSearchResultsTreeView::on_copyReferenceDetails()
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

void CSearchResultsTreeView::on_copyComplete()
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

	QString strStatusText;
	if (!pModel->hasExceededDisplayLimit()) {
		strStatusText = QString("%1 Search Result(s) Selected").arg(nNumResultsSelected);
	} else {
		strStatusText = "Too many search results to display in this mode!!  Try Switching to either View References Only mode or to Tree Mode.";
	}
	setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();
}

void CSearchResultsTreeView::on_listChanged()
{
	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);

	int nResultsCount = pModel->GetResultsCount();

	m_pActionSelectAll->setEnabled(nResultsCount != 0);
	emit canExpandAll((pModel->treeMode() != CVerseListModel::VTME_LIST) &&
						(pModel->hasChildren()) &&
						(!((nResultsCount > g_nSearchLimit) && (!g_bEnableNoLimits))));
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
	extern CKJVCanOpener *g_pMainWindow;
	assert(g_pMainWindow != NULL);
	return ((hasFocus()) || ((g_pMainWindow != NULL) && (g_pMainWindow->isSearchResultsActive())));
}

void CSearchResultsTreeView::resizeEvent(QResizeEvent *event)
{
	assert(event != NULL);

	// Unlike the QListView, the QTreeView doesn't have a ResizeMode for Adjust.  So
	//		we need to handle this event to do a new layout when the
	//		view size changes.

	QSize szDelta = event->size() - event->oldSize();

	if (!szDelta.isNull()) {
		bool bFlowDimensionChanged = (szDelta.width() != 0);

		if ((state() == NoState) && (bFlowDimensionChanged)) {
			scheduleDelayedItemsLayout();
		}
	}

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
	m_nLastSearchNumPhrases(0)
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
	m_pSearchResultsCount->setText("Found 0 Occurrences\n"
								   "    in 0 Verses in 0 Chapters in 0 Books");
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

	pLayout->addWidget(m_pSearchResultsTreeView);


	// -------------------- Search Results List View:

	CVerseListModel *pModel = new CVerseListModel(m_pBibleDatabase, m_pSearchResultsTreeView);
	QAbstractItemModel *pOldModel = m_pSearchResultsTreeView->model();
	m_pSearchResultsTreeView->setModel(pModel);
	if (pOldModel) delete pOldModel;
	m_pSearchResultsTreeView->setRootIsDecorated(pModel->treeMode() != CVerseListModel::VTME_LIST);

	CVerseListDelegate *pDelegate = new CVerseListDelegate(*pModel, m_pSearchResultsTreeView);
	QAbstractItemDelegate *pOldDelegate = m_pSearchResultsTreeView->itemDelegate();
	m_pSearchResultsTreeView->setItemDelegate(pDelegate);
	if (pOldDelegate) delete pOldDelegate;

	connect(this, SIGNAL(changedSearchResults()), m_pSearchResultsTreeView, SLOT(on_listChanged()));
	connect(model(), SIGNAL(modelReset()), m_pSearchResultsTreeView, SLOT(on_listChanged()));
	connect(model(), SIGNAL(layoutChanged()), m_pSearchResultsTreeView, SLOT(on_listChanged()));

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

const TPhraseTagList &CKJVSearchResult::setParsedPhrases(const CSearchCriteria &aSearchCriteria, const TParsedPhrasesList &phrases)
{
	m_LastSearchCriteria = aSearchCriteria;
	m_lstSearchResultsTags = model()->setParsedPhrases(aSearchCriteria, phrases);
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

	strResults += QString("Found %1 Occurrence%2\n").arg(nResults).arg((nResults != 1) ? "s" : "");
	strResults += QString("    in %1 Verse%2 in %3 Chapter%4 in %5 Book%6")
							.arg(nVerses).arg((nVerses != 1) ? "s" : "")
							.arg(nChapters).arg((nChapters != 1) ? "s" : "")
							.arg(nBooks).arg((nBooks != 1) ? "s" : "");
	if (nResults > 0) {
		strResults += "\n";
		strResults += QString("    Not found at all in %1 Verse%2 of the Bible\n")
								.arg(m_pBibleDatabase->bibleEntry().m_nNumVrs - nVerses).arg(((m_pBibleDatabase->bibleEntry().m_nNumVrs - nVerses) != 1) ? "s" : "");
		strResults += QString("    Not found at all in %1 Chapter%2 of the Bible\n")
								.arg(m_pBibleDatabase->bibleEntry().m_nNumChp - nChapters).arg(((m_pBibleDatabase->bibleEntry().m_nNumChp - nChapters) != 1) ? "s" : "");
		strResults += QString("    Not found at all in %1 Book%2 of the Bible")
								.arg(m_pBibleDatabase->bibleEntry().m_nNumBk - nBooks).arg(((m_pBibleDatabase->bibleEntry().m_nNumBk - nBooks) != 1) ? "s" : "");
	}

	m_pSearchResultsCount->setText(strResults);

	m_bLastCalcSuccess = true;
	m_nLastSearchOccurrences = nResults;
	m_nLastSearchVerses = nVerses;
	m_nLastSearchChapters = nChapters;
	m_nLastSearchBooks = nBooks;

	return m_lstSearchResultsTags;
}

QString CKJVSearchResult::searchResultsSummaryText() const
{
	QString strSummary;

	if (m_bLastCalcSuccess) {
		strSummary += QString("Found %1 %2Occurrence%3\n").arg(m_nLastSearchOccurrences).arg((m_nLastSearchNumPhrases > 1) ? "Combined " : "").arg((m_nLastSearchOccurrences != 1) ? "s" : "");
		strSummary += QString("    in %1 Verse%2\n").arg(m_nLastSearchVerses).arg((m_nLastSearchVerses != 1) ? "s" : "");
		strSummary += QString("    in %1 Chapter%2\n").arg(m_nLastSearchChapters).arg((m_nLastSearchChapters != 1) ? "s" : "");
		strSummary += QString("    in %1 Book%2\n").arg(m_nLastSearchBooks).arg((m_nLastSearchBooks != 1) ? "s" : "");
		strSummary += "\n";
		strSummary += QString("Not found%1 at all in %2 Verse%3 of the Bible\n").arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(m_pBibleDatabase->bibleEntry().m_nNumVrs - m_nLastSearchVerses).arg(((m_pBibleDatabase->bibleEntry().m_nNumVrs - m_nLastSearchVerses) != 1) ? "s" : "");
		strSummary += QString("Not found%1 at all in %2 Chapter%3 of the Bible\n").arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(m_pBibleDatabase->bibleEntry().m_nNumChp - m_nLastSearchChapters).arg(((m_pBibleDatabase->bibleEntry().m_nNumChp - m_nLastSearchChapters) != 1) ? "s" : "");
		strSummary += QString("Not found%1 at all in %2 Book%3 of the Bible\n").arg(((m_nLastSearchNumPhrases > 1) && (m_LastSearchCriteria.searchScopeMode() != CSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(m_pBibleDatabase->bibleEntry().m_nNumBk - m_nLastSearchBooks).arg(((m_pBibleDatabase->bibleEntry().m_nNumBk - m_nLastSearchBooks) != 1) ? "s" : "");
	} else {
		strSummary += QString("Search was incomplete -- too many possible matches\n");
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

