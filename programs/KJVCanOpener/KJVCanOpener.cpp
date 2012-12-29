#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include "dbstruct.h"
#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "KJVPassageNavigatorDlg.h"
#include "BuildDB.h"
#include "KJVAboutDlg.h"
#include "PhraseEdit.h"
#include "PhraseListModel.h"
#include "Highlighter.h"
#include "version.h"

#include <assert.h>

#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QStringList>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QItemSelection>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>

#define KJS_FILE_VERSION 1			// Current KJS File Version

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

CSearchResultsTreeView::CSearchResultsTreeView(QWidget *parent)
	:	QTreeView(parent),
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
	m_pActionCopyReferenceDetails = m_pEditMenu->addAction("Copy Reference &Details (Word/Phrase Counts)", this, SLOT(on_copyReferenceDetails()), QKeySequence(Qt::CTRL + Qt::Key_D));
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
	m_pEditMenuLocal->addAction(m_pActionSelectAll);
	m_pActionClearSelection = m_pEditMenu->addAction("C&lear Selection", this, SLOT(clearSelection()), QKeySequence(Qt::Key_Escape));
	m_pActionClearSelection->setStatusTip("Clear Search Results Selection");
	m_pActionClearSelection->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionClearSelection);
	// ----
	m_pEditMenuLocal->addSeparator();
	m_pActionNavigator = m_pEditMenuLocal->addAction("Passage &Navigator...");
	m_pActionNavigator->setEnabled(false);
	connect(m_pActionNavigator, SIGNAL(triggered()), this, SLOT(on_passageNavigator()));
	m_pActionNavigator->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	// ----

	m_pStatusAction = new QAction(this);
}

CSearchResultsTreeView::~CSearchResultsTreeView()
{
}

void CSearchResultsTreeView::on_copyVerseText()
{
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
		CPhraseNavigator navigator(docVerse);
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
		CPhraseNavigator navigator(docVerse);
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
		strRichText += pModel->dataForVerse(lstVerses.at(ndx), Qt::ToolTipRole).toString();
	}

	mime->setText(strPlainText);
	mime->setHtml(strRichText);
	clipboard->setMimeData(mime);
}

void CSearchResultsTreeView::on_copyComplete()
{
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
		CPhraseNavigator navigator(docVerse);
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

void CSearchResultsTreeView::on_passageNavigator()
{
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	if (lstSelectedItems.size() != 1) return;
	if (!lstSelectedItems.at(0).isValid()) return;

	CRelIndex ndxRel(lstSelectedItems.at(0).internalId());
	assert(ndxRel.isSet());
	if (!ndxRel.isSet()) return;

//	const CVerseListItem &item(lstSelectedItems.at(0).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
	CKJVPassageNavigatorDlg dlg(this);

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

	QTreeView::contextMenuEvent(event);
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
		strStatusText = "Too many search results to display in this mode!!  Try Switching to View References Only mode.";
	}
	setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();
}

void CSearchResultsTreeView::on_listChanged()
{
	handle_selectionChanged();
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

// ============================================================================

CKJVCanOpener::CKJVCanOpener(const QString &strUserDatabase, QWidget *parent) :
	QMainWindow(parent),
	m_strUserDatabase(strUserDatabase),
	m_bDoingUpdate(false),
	m_pActionPassageBrowserEditMenu(NULL),
	m_pActionSearchResultsEditMenu(NULL),
	m_pActionSearchPhraseEditMenu(NULL),
	m_pViewMenu(NULL),
	m_pActionShowVerseHeading(NULL),
	m_pActionShowVerseRichText(NULL),
	m_pActionShowAsList(NULL),
	m_pActionShowAsTreeBooks(NULL),
	m_pActionShowAsTreeChapters(NULL),
	m_pActionShowMissingLeafs(NULL),
	m_pActionBookBackward(NULL),
	m_pActionBookForward(NULL),
	m_pActionChapterBackward(NULL),
	m_pActionChapterForward(NULL),
	m_pActionNavBackward(NULL),
	m_pActionNavForward(NULL),
	m_pActionNavHome(NULL),
	m_pActionNavClear(NULL),
	m_pActionJump(NULL),
	m_pActionAbout(NULL),
	m_bBrowserActive(false),
	m_bSearchResultsActive(false),
	m_bPhraseEditorActive(false),
	m_pLayoutPhrases(NULL),
	m_nLastSearchOccurrences(0),
	m_nLastSearchVerses(0),
	m_nLastSearchChapters(0),
	m_nLastSearchBooks(0),
	m_bLastCalcSuccess(true),
	ui(new Ui::CKJVCanOpener)
{
	ui->setupUi(this);

// The following is supposed to be another workaround for QTBUG-13768
//	ui->splitter->setStyleSheet("QSplitterHandle:hover {}  QSplitter::handle:hover { background-color: palette(highlight); }");
	ui->splitter->handle(1)->setAttribute(Qt::WA_Hover);		// Work-Around QTBUG-13768
	setStyleSheet("QSplitter::handle:hover { background-color: palette(highlight); }");

	// TODO : Set preference for start mode!:
	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode = CVerseListModel::VDME_RICHTEXT;
	CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode = CVerseListModel::VTME_LIST;
	bool bShowMissingLeafs = false;

	// --------------------

	QAction *pAction;

	// --- File Menu
	QMenu *pFileMenu = ui->menuBar->addMenu("&File");

	pAction = pFileMenu->addAction(QIcon(":/res/file-new-icon2.png"), "&New Search", this, SLOT(on_NewSearch()), QKeySequence(Qt::CTRL + Qt::Key_N));
	pAction->setStatusTip("Clear All Search Phrases and Begin New Search");
	pAction->setToolTip("Clear All Search Phrases and Begin New Search");
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/open-file-icon3.png"), "&Open Search...", this, SLOT(on_OpenSearch()), QKeySequence(Qt::CTRL + Qt::Key_O));
	pAction->setStatusTip("Open a previously saved KJV Search File");
	pAction->setToolTip("Open a previously saved KJV Search File");
	ui->mainToolBar->addAction(pAction);

	pAction = pFileMenu->addAction(QIcon(":/res/save-file-icon3.png"), "&Save Search...", this, SLOT(on_SaveSearch()), QKeySequence(Qt::CTRL + Qt::Key_S));
	pAction->setStatusTip("Save current Search Phrases to a KJV Search File");
	pAction->setToolTip("Save current Search Phrases to a KJV Search File");
	ui->mainToolBar->addAction(pAction);

	pFileMenu->addSeparator();
	ui->mainToolBar->addSeparator();

	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), "E&xit", this, SLOT(close()), QKeySequence(Qt::CTRL + Qt::Key_Q));
	pAction->setStatusTip("Exit the King James Can Opener Application");
	pAction->setToolTip("Exit KJVCanOpener");

	// --- Edit Menu
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(activatedScriptureText()), this, SLOT(on_activatedBrowser()));
	connect(ui->treeViewSearchResults, SIGNAL(activatedSearchResults()), this, SLOT(on_activatedSearchResults()));

	// --- View Menu
	m_pViewMenu = ui->menuBar->addMenu("&View");

	QMenu *pViewToolbarsMenu = m_pViewMenu->addMenu("&Toolbars");
	pViewToolbarsMenu->addAction(ui->mainToolBar->toggleViewAction());
	ui->mainToolBar->toggleViewAction()->setStatusTip("Show/Hide Main Tool Bar");

	m_pViewMenu->addSeparator();
	ui->treeViewSearchResults->getLocalEditMenu()->addSeparator();

	m_pActionShowAsList = m_pViewMenu->addAction("View as &List", this, SLOT(on_viewAsList()));
	m_pActionShowAsList->setStatusTip("Show Search Results as a List");
	m_pActionShowAsList->setCheckable(true);
	m_pActionShowAsList->setChecked(nTreeMode == CVerseListModel::VTME_LIST);
	ui->treeViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowAsList);

	m_pActionShowAsTreeBooks = m_pViewMenu->addAction("View as Tree by &Book", this, SLOT(on_viewAsTreeBooks()));
	m_pActionShowAsTreeBooks->setStatusTip("Show Search Results in a Tree by Book");
	m_pActionShowAsTreeBooks->setCheckable(true);
	m_pActionShowAsTreeBooks->setChecked(nTreeMode == CVerseListModel::VTME_TREE_BOOKS);
	ui->treeViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowAsTreeBooks);

	m_pActionShowAsTreeChapters = m_pViewMenu->addAction("View as Tree by Book/&Chapter", this, SLOT(on_viewAsTreeChapters()));
	m_pActionShowAsTreeChapters->setStatusTip("Show Search Results in a Tree by Book and Chapter");
	m_pActionShowAsTreeChapters->setCheckable(true);
	m_pActionShowAsTreeChapters->setChecked(nTreeMode == CVerseListModel::VTME_TREE_CHAPTERS);
	ui->treeViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowAsTreeChapters);

	m_pViewMenu->addSeparator();
	ui->treeViewSearchResults->getLocalEditMenu()->addSeparator();

	m_pActionShowMissingLeafs = m_pViewMenu->addAction("View &Missing Books/Chapters", this, SLOT(on_viewShowMissingsLeafs()));
	m_pActionShowMissingLeafs->setStatusTip("Show Missing Books and/or Chapters in the Tree (ones that had no matching Search Results)");
	m_pActionShowMissingLeafs->setCheckable(true);
	m_pActionShowMissingLeafs->setChecked(bShowMissingLeafs);
	m_pActionShowMissingLeafs->setEnabled(nTreeMode != CVerseListModel::VTME_LIST);
	ui->treeViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowMissingLeafs);

	m_pViewMenu->addSeparator();
	ui->treeViewSearchResults->getLocalEditMenu()->addSeparator();

	m_pActionShowVerseHeading = m_pViewMenu->addAction("View &References Only", this, SLOT(on_viewVerseHeading()));
	m_pActionShowVerseHeading->setStatusTip("Show Search Results Verse References Only");
	m_pActionShowVerseHeading->setCheckable(true);
	m_pActionShowVerseHeading->setChecked(nDisplayMode == CVerseListModel::VDME_HEADING);
	ui->treeViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowVerseHeading);

	m_pActionShowVerseRichText = m_pViewMenu->addAction("View Verse &Preview", this, SLOT(on_viewVerseRichText()));
	m_pActionShowVerseRichText->setStatusTip("Show Search Results as Rich Text Verse Preview");
	m_pActionShowVerseRichText->setCheckable(true);
	m_pActionShowVerseRichText->setChecked(nDisplayMode == CVerseListModel::VDME_RICHTEXT);
	ui->treeViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowVerseRichText);

	// --- Navigate Menu
	QMenu *pNavMenu = ui->menuBar->addMenu("&Navigate");

	pAction = pNavMenu->addAction("Beginning of Bible", ui->widgetKJVBrowser, SLOT(on_Bible_Beginning()), QKeySequence(Qt::ALT + Qt::Key_Home));
	pAction->setStatusTip("Goto the very Beginning of the Bible");
	connect(pAction, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	pAction = pNavMenu->addAction("Ending of Bible", ui->widgetKJVBrowser, SLOT(on_Bible_Ending()), QKeySequence(Qt::ALT + Qt::Key_End));
	pAction->setStatusTip("Goto the very End of the Bible");
	connect(pAction, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionBookBackward = pNavMenu->addAction("Book Backward", ui->widgetKJVBrowser, SLOT(on_Book_Backward()), QKeySequence(Qt::CTRL + Qt::Key_PageUp));
	m_pActionBookBackward->setStatusTip("Move Backward one Book");
	connect(m_pActionBookBackward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionBookForward = pNavMenu->addAction("Book Forward", ui->widgetKJVBrowser, SLOT(on_Book_Forward()), QKeySequence(Qt::CTRL + Qt::Key_PageDown));
	m_pActionBookForward->setStatusTip("Move Forward one Book");
	connect(m_pActionBookForward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionChapterBackward = pNavMenu->addAction("Chapter Backward", ui->widgetKJVBrowser, SLOT(on_ChapterBackward()), QKeySequence(Qt::ALT + Qt::Key_PageUp));
	m_pActionChapterBackward->setStatusTip("Move Backward one Chapter");
	connect(m_pActionChapterBackward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionChapterForward = pNavMenu->addAction("Chapter Forward", ui->widgetKJVBrowser, SLOT(on_ChapterForward()), QKeySequence(Qt::ALT + Qt::Key_PageDown));
	m_pActionChapterForward->setStatusTip("Move Forward one Chapter");
	connect(m_pActionChapterForward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	connect(ui->widgetKJVBrowser, SIGNAL(IndexChanged(const TPhraseTag &)), this, SLOT(on_indexChanged(const TPhraseTag &)));

	pNavMenu->addSeparator();

	m_pActionNavBackward = new QAction(QIcon(":/res/Nav3_Arrow_Left.png"), "History &Backward", this);
	m_pActionNavBackward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	m_pActionNavBackward->setStatusTip("Go Backward in Navigation History");
	ui->mainToolBar->addAction(m_pActionNavBackward);
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(backwardAvailable(bool)), m_pActionNavBackward, SLOT(setEnabled(bool)));
	connect(m_pActionNavBackward, SIGNAL(triggered()), ui->widgetKJVBrowser->browser(), SLOT(backward()));
	connect(m_pActionNavBackward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionNavBackward->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable());
	pNavMenu->addAction(m_pActionNavBackward);

	m_pActionNavForward = new QAction(QIcon(":/res/Nav3_Arrow_Right.png"), "History &Forward", this);
	m_pActionNavForward->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));
	m_pActionNavForward->setStatusTip("Go Forward in Navigation History");
	ui->mainToolBar->addAction(m_pActionNavForward);
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(forwardAvailable(bool)), m_pActionNavForward, SLOT(setEnabled(bool)));
	connect(m_pActionNavForward, SIGNAL(triggered()), ui->widgetKJVBrowser->browser(), SLOT(forward()));
	connect(m_pActionNavForward, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
	m_pActionNavForward->setEnabled(ui->widgetKJVBrowser->browser()->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavForward);

	m_pActionNavHome = pNavMenu->addAction(QIcon(":/res/go_home.png"), "History &Home", ui->widgetKJVBrowser->browser(), SLOT(home()), QKeySequence(Qt::ALT + Qt::Key_Up));
	m_pActionNavHome->setStatusTip("Jump to History Home Passage");
	m_pActionNavHome->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
									ui->widgetKJVBrowser->browser()->isForwardAvailable());

	m_pActionNavClear = new QAction(QIcon(":/res/edit_clear.png"), "&Clear Navigation History", this);
	m_pActionNavClear->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));
	m_pActionNavClear->setStatusTip("Clear All Passage Navigation History");
	ui->mainToolBar->addAction(m_pActionNavClear);
	connect(m_pActionNavClear, SIGNAL(triggered()), this, SLOT(on_clearBrowserHistory()));
	m_pActionNavClear->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
									ui->widgetKJVBrowser->browser()->isForwardAvailable());
	pNavMenu->addAction(m_pActionNavClear);

	connect(ui->widgetKJVBrowser->browser(), SIGNAL(historyChanged()), this, SLOT(on_browserHistoryChanged()));

//	m_pActionJump = new QAction(QIcon(":/res/go_jump2.png"), "Passage Navigator", this);
	m_pActionJump = new QAction(QIcon(":/res/green_arrow.png"), "Passage &Navigator", this);
	m_pActionJump->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	m_pActionJump->setStatusTip("Display the Passage Navigator Widget");
	ui->mainToolBar->addAction(m_pActionJump);
	connect(m_pActionJump, SIGNAL(triggered()), this, SLOT(on_PassageNavigatorTriggered()));

	pNavMenu->addSeparator();
	pNavMenu->addAction(m_pActionJump);

	// --- Help Menu
	QMenu *pHelpMenu = ui->menuBar->addMenu("&Help");
	pAction = pHelpMenu->addAction(QIcon(":/res/help_book.png"), "&Help", this, SLOT(on_HelpManual()), QKeySequence(Qt::SHIFT + Qt::Key_F1));
	pAction->setStatusTip("Display the Users Manual");

	m_pActionAbout = new QAction(QIcon(":/res/help_icon1.png"), "About...", this);
	m_pActionAbout->setShortcut(QKeySequence(Qt::Key_F1));
	m_pActionAbout->setStatusTip("About the King James Can Opener");
	m_pActionAbout->setToolTip("About the King James Can Opener...");
	m_pActionAbout->setMenuRole(QAction::AboutRole);
	connect(m_pActionAbout, SIGNAL(triggered()), this, SLOT(on_HelpAbout()));
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addAction(m_pActionAbout);
	pHelpMenu->addAction(m_pActionAbout);

	// -------------------- Search Phrase Widgets:

	m_pLayoutPhrases = new QVBoxLayout(ui->scrollAreaWidgetContents);
	m_pLayoutPhrases->setSpacing(0);
	m_pLayoutPhrases->setContentsMargins(0, 0, 0, 0);

	ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	CKJVSearchPhraseEdit *pFirstSearchPhraseEditor = addSearchPhrase();
	QTimer::singleShot(0, pFirstSearchPhraseEditor, SLOT(focusEditor()));

	ui->scrollAreaSearchPhrases->setMinimumSize(m_pLayoutPhrases->sizeHint().width() +
							ui->scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui->scrollAreaSearchPhrases->frameWidth() * 2,
							m_pLayoutPhrases->sizeHint().height() /* pFirstSearchPhraseEditor->sizeHint() */);


//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);


	ui->widgetSearchCriteria->enableCopySearchPhraseSummary(false);

	connect(ui->widgetSearchCriteria, SIGNAL(addSearchPhraseClicked()), this, SLOT(addSearchPhrase()));
	connect(ui->widgetSearchCriteria, SIGNAL(changedSearchScopeMode(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM)), this, SLOT(on_changedSearchCriteria()));
	connect(ui->widgetSearchCriteria, SIGNAL(copySearchPhraseSummary()), this, SLOT(on_copySearchPhraseSummary()));

	// -------------------- Search Results List View:

	CVerseListModel *model = new CVerseListModel(ui->treeViewSearchResults);
	model->setDisplayMode(nDisplayMode);
	model->setTreeMode(nTreeMode);
	model->setShowMissingLeafs(bShowMissingLeafs);
	ui->treeViewSearchResults->setModel(model);
	ui->treeViewSearchResults->setRootIsDecorated(nTreeMode != CVerseListModel::VTME_LIST);

	CVerseListDelegate *delegate = new CVerseListDelegate(*model, ui->treeViewSearchResults);
	ui->treeViewSearchResults->setItemDelegate(delegate);

	connect(ui->treeViewSearchResults, SIGNAL(activated(const QModelIndex &)), this, SLOT(on_SearchResultActivated(const QModelIndex &)));
	connect(ui->treeViewSearchResults, SIGNAL(gotoIndex(const TPhraseTag &)), ui->widgetKJVBrowser, SLOT(gotoIndex(TPhraseTag)));
	connect(this, SIGNAL(changedSearchResults()), ui->treeViewSearchResults, SLOT(on_listChanged()));
	connect(model, SIGNAL(modelReset()), ui->treeViewSearchResults, SLOT(on_listChanged()));
	connect(model, SIGNAL(layoutChanged()), ui->treeViewSearchResults, SLOT(on_listChanged()));
}

CKJVCanOpener::~CKJVCanOpener()
{
	delete ui;
}

void CKJVCanOpener::Initialize(const TPhraseTag &nInitialIndex)
{
	ui->widgetKJVBrowser->gotoIndex(nInitialIndex);
}

void CKJVCanOpener::closeEvent(QCloseEvent *event)
{
	if ((g_bUserPhrasesDirty) && haveUserDatabase()) {
		int nResult = QMessageBox::warning(this, windowTitle(), "Do you wish to save the search phrase list changes you've made to the user database?",
											QString("Yes"), QString("No"), QString("Cancel"), 0, 2);
		if (nResult == 2) {
			event->ignore();
			return;
		}
		if (nResult == 0) {
			CBuildDatabase bdb(this);
			if (!bdb.BuildUserDatabase(m_strUserDatabase)) {
				QMessageBox::warning(this, windowTitle(), "Failed to save KJV User Database!\nCheck installation and settings!");
				event->ignore();
				return;
			}
		}
	}

	return QMainWindow::closeEvent(event);
}

// ------------------------------------------------------------------

void CKJVCanOpener::on_NewSearch()
{
	closeAllSearchPhrases();
	addSearchPhrase();
	ui->widgetSearchCriteria->setSearchScopeMode(CKJVSearchCriteria::SSME_WHOLE_BIBLE);
}

void CKJVCanOpener::on_OpenSearch()
{
	QString strFilePathName = QFileDialog::getOpenFileName(this, "Open KJV Search File", QString(), "KJV Search Files (*.kjs)", NULL, QFileDialog::ReadOnly);
	if (!strFilePathName.isEmpty())
		if (!openKJVSearchFile(strFilePathName))
			QMessageBox::warning(this, "KJV Search File Open Failed", "Failed to open and read the specified KJV Search File!");
}

void CKJVCanOpener::on_SaveSearch()
{
	QString strFilePathName = QFileDialog::getSaveFileName(this, "Save KJV Search File", QString(), "KJV Search Files (*.kjs)", NULL, 0);
	if (!strFilePathName.isEmpty())
		if (!saveKJVSearchFile(strFilePathName))
			QMessageBox::warning(this, "KJV Search File Save Failed", "Failed to save the specified KJV Search File!");
}

void CKJVCanOpener::closeAllSearchPhrases()
{
	for (int ndx = m_lstSearchPhraseEditors.size()-1; ndx>=0; --ndx) {
		m_lstSearchPhraseEditors.at(ndx)->closeSearchPhrase();
	}
}

bool CKJVCanOpener::openKJVSearchFile(const QString &strFilePathName)
{
	QSettings kjsFile(strFilePathName, QSettings::IniFormat);
	if (kjsFile.status() != QSettings::NoError) return false;

	unsigned int nFileVersion = 0;
	CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScope = CKJVSearchCriteria::SSME_WHOLE_BIBLE;

	kjsFile.beginGroup("KJVCanOpener");
	nFileVersion = kjsFile.value("KJSFileVersion").toUInt();
	kjsFile.endGroup();

	if (nFileVersion < KJS_FILE_VERSION) {
		QMessageBox::warning(this, "Opening KJV Search File", "Warning: The file you are opening was saved on "
									"an older version of KJVCanOpener.  Some manual editing may be necessary "
									"to configure any new search options added since that older version.");
	} else if (nFileVersion > KJS_FILE_VERSION) {
		QMessageBox::warning(this, "Opening KJV Search File", "Warning: The file you are opening was created on "
									"a newer version of KJVCanOpener.  It may contain settings for options not "
									"available on this version of KJVCanOpener.  If so, those options will be "
									"ignored.");
	}

	closeAllSearchPhrases();

	kjsFile.beginGroup("SearchCriteria");
	nSearchScope = static_cast<CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(kjsFile.value("SearchScope", CKJVSearchCriteria::SSME_WHOLE_BIBLE).toInt());
	if ((nSearchScope < CKJVSearchCriteria::SSME_WHOLE_BIBLE) ||
		(nSearchScope > CKJVSearchCriteria::SSME_VERSE))
		nSearchScope = CKJVSearchCriteria::SSME_WHOLE_BIBLE;
	kjsFile.endGroup();

	ui->widgetSearchCriteria->setSearchScopeMode(nSearchScope);

	int nPhrases = kjsFile.beginReadArray("SearchPhrases");
	for (int ndx = 0; ndx < nPhrases; ++ndx) {
		CKJVSearchPhraseEdit *pPhraseEditor = addSearchPhrase();
		assert(pPhraseEditor != NULL);
		kjsFile.setArrayIndex(ndx);
		pPhraseEditor->phraseEditor()->setCaseSensitive(kjsFile.value("CaseSensitive", false).toBool());
		pPhraseEditor->phraseEditor()->setText(kjsFile.value("Phrase").toString());
	}
	kjsFile.endArray();

	return (kjsFile.status() == QSettings::NoError);
}

bool CKJVCanOpener::saveKJVSearchFile(const QString &strFilePathName) const
{
	QSettings kjsFile(strFilePathName, QSettings::IniFormat);
	if (kjsFile.status() != QSettings::NoError) return false;

	kjsFile.clear();

	kjsFile.beginGroup("KJVCanOpener");
	kjsFile.setValue("AppVersion", VER_QT);
	kjsFile.setValue("KJSFileVersion", KJS_FILE_VERSION);
	kjsFile.endGroup();

	kjsFile.beginGroup("SearchCriteria");
	kjsFile.setValue("SearchScope", ui->widgetSearchCriteria->searchScopeMode());
	kjsFile.endGroup();

	int ndxCurrent = 0;
	kjsFile.beginWriteArray("SearchPhrases");
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		if (m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase().isEmpty()) continue;
		kjsFile.setArrayIndex(ndxCurrent);
		kjsFile.setValue("Phrase", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase());
		kjsFile.setValue("CaseSensitive", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isCaseSensitive());
		ndxCurrent++;
	}
	kjsFile.endArray();

	kjsFile.sync();

	return (kjsFile.status() == QSettings::NoError);
}

// ------------------------------------------------------------------

CKJVSearchPhraseEdit *CKJVCanOpener::addSearchPhrase()
{
	CKJVSearchPhraseEdit *pPhraseWidget = new CKJVSearchPhraseEdit(this);
	connect(pPhraseWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SLOT(on_closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(on_phraseChanged(CKJVSearchPhraseEdit *)));
	m_lstSearchPhraseEditors.append(pPhraseWidget);
	pPhraseWidget->showSeperatorLine(m_lstSearchPhraseEditors.size() > 1);
	m_pLayoutPhrases->addWidget(pPhraseWidget);
	// Calculate height, since it varies depending on whether or not the widget is showing a separator:
	int nHeight = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
	}
	ui->scrollAreaWidgetContents->setMinimumSize(pPhraseWidget->sizeHint().width(), nHeight);
	ui->scrollAreaSearchPhrases->ensureVisible((pPhraseWidget->sizeHint().width()/2),
												nHeight - (pPhraseWidget->sizeHint().height()/2));
	pPhraseWidget->focusEditor();

//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);

	return pPhraseWidget;
}

void CKJVCanOpener::on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase)
{
	assert(pSearchPhrase != NULL);

	bool bPhraseChanged = ((!pSearchPhrase->parsedPhrase()->IsDuplicate()) &&
							(pSearchPhrase->parsedPhrase()->GetNumberOfMatches() != 0) &&
							(pSearchPhrase->parsedPhrase()->isCompleteMatch()));

	int ndx = m_lstSearchPhraseEditors.indexOf(pSearchPhrase);
	assert(ndx != -1);
	if (ndx != -1) {
		m_lstSearchPhraseEditors.removeAt(ndx);
	}
	if ((ndx == 0) && (m_lstSearchPhraseEditors.size() != 0))
		m_lstSearchPhraseEditors.at(0)->showSeperatorLine(false);

	int nHeight = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
	}
	ui->scrollAreaWidgetContents->setMinimumSize(ui->scrollAreaWidgetContents->minimumSize().width(), nHeight);
	if (bPhraseChanged) on_phraseChanged(NULL);
}

void CKJVCanOpener::on_changedSearchCriteria()
{
	on_phraseChanged(NULL);
}

typedef struct {
	unsigned int m_nNumMatches;
	unsigned int m_nNumContributingMatches;
} TPhraseOccurrenceInfo;
Q_DECLARE_METATYPE(TPhraseOccurrenceInfo)

void CKJVCanOpener::on_copySearchPhraseSummary()
{
	int nNumPhrases = 0;
	bool bCaseSensitive = false;

	CPhraseList phrases;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		if ((pPhrase->GetNumberOfMatches()) &&
			(!pPhrase->IsDuplicate())) {
			nNumPhrases++;
			CPhraseEntry entry;
			entry.m_bCaseSensitive = pPhrase->isCaseSensitive();
			entry.m_strPhrase = pPhrase->phrase();
			entry.m_nNumWrd = pPhrase->phraseSize();
			TPhraseOccurrenceInfo poiUsage;
			poiUsage.m_nNumMatches = pPhrase->GetNumberOfMatches();
			poiUsage.m_nNumContributingMatches = pPhrase->GetContributingNumberOfMatches();
			entry.m_varExtraInfo = QVariant::fromValue(poiUsage);
			phrases.append(entry);
			if (entry.m_bCaseSensitive) bCaseSensitive = true;
		}
	}

	CPhraseListModel mdlPhrases(phrases);
	mdlPhrases.sort(0);

	QString strScope;
	CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nScope = ui->widgetSearchCriteria->searchScopeMode();
	switch (nScope) {
		case (CKJVSearchCriteria::SSME_WHOLE_BIBLE):
			strScope = "in the Entire Bible";
			break;
		case (CKJVSearchCriteria::SSME_TESTAMENT):
			strScope = "in the same Testament";
			break;
		case (CKJVSearchCriteria::SSME_BOOK):
			strScope = "in the same Book";
			break;
		case (CKJVSearchCriteria::SSME_CHAPTER):
			strScope = "in the same Chapter";
			break;
		case (CKJVSearchCriteria::SSME_VERSE):
			strScope = "in the same Verse";
			break;
		default:
			break;
	}

	QString strSummary;
	if (nNumPhrases != 1) {
		strSummary += QString("Search of %1 Phrases %2:\n")
								.arg(nNumPhrases)
								.arg(strScope);
	} else {
		strSummary += QString("Search of: ");
	}
	if (nNumPhrases > 1) strSummary += "\n";
	for (int ndx=0; ndx<mdlPhrases.rowCount(); ++ndx) {
		const CPhraseEntry &aPhrase = mdlPhrases.index(ndx).data(CPhraseListModel::PHRASE_ENTRY_ROLE).value<CPhraseEntry>();
		if (nNumPhrases > 1) {
			if (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE) {
				strSummary += QString("    \"%1\" (Found %2 Time%3 in the Entire Bible, %4 in Scope)\n")
									.arg(mdlPhrases.index(ndx).data().toString())
									.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches)
									.arg((aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches != 1) ? "s" : "")
									.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumContributingMatches);
			} else {
				strSummary += QString("    \"%1\" (Found %2 Time%3 in the Entire Bible)\n")
									.arg(mdlPhrases.index(ndx).data().toString())
									.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches)
									.arg((aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches != 1) ? "s" : "");
				assert(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches == aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumContributingMatches);
			}
		} else {
			strSummary += QString("\"%1\"\n").arg(mdlPhrases.index(ndx).data().toString());
		}
	}
	if (bCaseSensitive) {
		if (nNumPhrases > 1) strSummary += "\n";
		strSummary += QString("    (%1 = Case Sensitive)\n").arg(QChar(0xA7));
	}
	if (nNumPhrases) strSummary += "\n";
	if (m_bLastCalcSuccess) {
		strSummary += QString("Found %1 %2Occurrence%3\n").arg(m_nLastSearchOccurrences).arg((nNumPhrases > 1) ? "Combined " : "").arg((m_nLastSearchOccurrences != 1) ? "s" : "");
		strSummary += QString("    in %1 Verse%2\n").arg(m_nLastSearchVerses).arg((m_nLastSearchVerses != 1) ? "s" : "");
		strSummary += QString("    in %1 Chapter%2\n").arg(m_nLastSearchChapters).arg((m_nLastSearchChapters != 1) ? "s" : "");
		strSummary += QString("    in %1 Book%2\n").arg(m_nLastSearchBooks).arg((m_nLastSearchBooks != 1) ? "s" : "");
		strSummary += "\n";
		strSummary += QString("Not found%1 at all in %2 Verse%3 of the Bible\n").arg(((nNumPhrases > 1) && (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(g_EntireBible.m_nNumVrs - m_nLastSearchVerses).arg(((g_EntireBible.m_nNumVrs - m_nLastSearchVerses) != 1) ? "s" : "");
		strSummary += QString("Not found%1 at all in %2 Chapter%3 of the Bible\n").arg(((nNumPhrases > 1) && (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(g_EntireBible.m_nNumChp - m_nLastSearchChapters).arg(((g_EntireBible.m_nNumChp - m_nLastSearchChapters) != 1) ? "s" : "");
		strSummary += QString("Not found%1 at all in %2 Book%3 of the Bible\n").arg(((nNumPhrases > 1) && (nScope != CKJVSearchCriteria::SSME_WHOLE_BIBLE)) ? " together" : "").arg(g_EntireBible.m_nNumBk - m_nLastSearchBooks).arg(((g_EntireBible.m_nNumBk - m_nLastSearchBooks) != 1) ? "s" : "");
	} else {
		strSummary += QString("Search was incomplete -- too many possible matches\n");
	}

	QMimeData *mime = new QMimeData();

	mime->setText(strSummary);
	mime->setHtml("<qt><pre>\n" + strSummary + "</pre></qt>\n");
	QApplication::clipboard()->setMimeData(mime);
}

void CKJVCanOpener::on_addPassageBrowserEditMenu(bool bAdd)
{
	m_bBrowserActive = bAdd;

	if (bAdd) {
		if (m_pActionPassageBrowserEditMenu == NULL) {
			m_pActionPassageBrowserEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), ui->widgetKJVBrowser->browser()->getEditMenu());
			connect(m_pActionPassageBrowserEditMenu, SIGNAL(triggered()), ui->widgetKJVBrowser, SLOT(focusBrowser()));
		}
	} else {
		if (m_pActionPassageBrowserEditMenu) {
			ui->menuBar->removeAction(m_pActionPassageBrowserEditMenu);
			m_pActionPassageBrowserEditMenu = NULL;
		}
	}
}

void CKJVCanOpener::on_addSearchResultsEditMenu(bool bAdd)
{
	m_bSearchResultsActive = bAdd;

	if (bAdd) {
		if (m_pActionSearchResultsEditMenu == NULL) {
			m_pActionSearchResultsEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), ui->treeViewSearchResults->getEditMenu());
		}
	} else {
		if (m_pActionSearchResultsEditMenu) {
			ui->menuBar->removeAction(m_pActionSearchResultsEditMenu);
			m_pActionSearchResultsEditMenu = NULL;
		}
	}
}

void CKJVCanOpener::on_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor)
{
	m_bPhraseEditorActive = bAdd;

	if (m_pActionSearchPhraseEditMenu) {
		ui->menuBar->removeAction(m_pActionSearchPhraseEditMenu);
		m_pActionSearchPhraseEditMenu = NULL;
	}
	if ((bAdd) && (pEditor != NULL)) {
		m_pActionSearchPhraseEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), pEditor->getEditMenu());
	}
}

void CKJVCanOpener::on_activatedBrowser()
{
	on_addPassageBrowserEditMenu(true);
	on_addSearchResultsEditMenu(false);
	on_addSearchPhraseEditMenu(false);
}

void CKJVCanOpener::on_activatedSearchResults()
{
	on_addPassageBrowserEditMenu(false);
	on_addSearchResultsEditMenu(true);
	on_addSearchPhraseEditMenu(false);
}

void CKJVCanOpener::on_activatedPhraseEditor(const CPhraseLineEdit *pEditor)
{
	on_addPassageBrowserEditMenu(false);
	on_addSearchResultsEditMenu(false);
	on_addSearchPhraseEditMenu(true, pEditor);
}

void CKJVCanOpener::on_viewVerseHeading()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nMode = CVerseListModel::VDME_HEADING;

	if (m_pActionShowVerseHeading->isChecked()) {
		m_pActionShowVerseRichText->setChecked(false);
		nMode = CVerseListModel::VDME_HEADING;
	} else {
		m_pActionShowVerseRichText->setChecked(true);
		nMode = CVerseListModel::VDME_RICHTEXT;
	}

	CRelIndex ndxCurrent(ui->treeViewSearchResults->currentIndex().internalId());

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);
	pModel->setDisplayMode(nMode);

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewVerseRichText()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nMode = CVerseListModel::VDME_RICHTEXT;

	if (m_pActionShowVerseRichText->isChecked()) {
		m_pActionShowVerseHeading->setChecked(false);
		nMode = CVerseListModel::VDME_RICHTEXT;
	} else {
		m_pActionShowVerseHeading->setChecked(true);
		nMode = CVerseListModel::VDME_HEADING;
	}

	CRelIndex ndxCurrent(ui->treeViewSearchResults->currentIndex().internalId());

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);
	pModel->setDisplayMode(nMode);

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}


void CKJVCanOpener::on_viewAsList()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(ui->treeViewSearchResults->currentIndex().internalId());

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);

	if (m_pActionShowAsList->isChecked()) {
		m_pActionShowAsTreeBooks->setChecked(false);
		m_pActionShowAsTreeChapters->setChecked(false);
		pModel->setTreeMode(CVerseListModel::VTME_LIST);
		ui->treeViewSearchResults->setRootIsDecorated(false);
		m_pActionShowMissingLeafs->setEnabled(false);
	} else {
		if (pModel->treeMode() == CVerseListModel::VTME_LIST) {
			m_pActionShowAsList->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsTreeBooks()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(ui->treeViewSearchResults->currentIndex().internalId());

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);

	if (m_pActionShowAsTreeBooks->isChecked()) {
		m_pActionShowAsList->setChecked(false);
		m_pActionShowAsTreeChapters->setChecked(false);
		pModel->setTreeMode(CVerseListModel::VTME_TREE_BOOKS);
		ui->treeViewSearchResults->setRootIsDecorated(true);
		m_pActionShowMissingLeafs->setEnabled(true);
	} else {
		if (pModel->treeMode() == CVerseListModel::VTME_TREE_BOOKS) {
			m_pActionShowAsTreeBooks->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewAsTreeChapters()
{
	assert(m_pActionShowAsList != NULL);
	assert(m_pActionShowAsTreeBooks != NULL);
	assert(m_pActionShowAsTreeChapters != NULL);
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(ui->treeViewSearchResults->currentIndex().internalId());

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);

	if (m_pActionShowAsTreeChapters->isChecked()) {
		m_pActionShowAsList->setChecked(false);
		m_pActionShowAsTreeBooks->setChecked(false);
		pModel->setTreeMode(CVerseListModel::VTME_TREE_CHAPTERS);
		ui->treeViewSearchResults->setRootIsDecorated(true);
		m_pActionShowMissingLeafs->setEnabled(true);
	} else {
		if (pModel->treeMode() == CVerseListModel::VTME_TREE_CHAPTERS) {
			m_pActionShowAsTreeChapters->setChecked(true);
		}
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::on_viewShowMissingsLeafs()
{
	assert(m_pActionShowMissingLeafs != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CRelIndex ndxCurrent(ui->treeViewSearchResults->currentIndex().internalId());

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);

	if (pModel->treeMode() == CVerseListModel::VTME_LIST) {
		if (pModel->showMissingLeafs()) pModel->setShowMissingLeafs(false);
	} else {
		pModel->setShowMissingLeafs(m_pActionShowMissingLeafs->isChecked());
	}

	m_bDoingUpdate = false;

	setCurrentIndex(ndxCurrent);
}

void CKJVCanOpener::setCurrentIndex(const CRelIndex &ndxCurrent)
{
	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);

	QModelIndex mndxCurrent = pModel->locateIndex(ndxCurrent);
	ui->treeViewSearchResults->setCurrentIndex(mndxCurrent);
	ui->treeViewSearchResults->scrollTo(mndxCurrent, QAbstractItemView::EnsureVisible);
	ui->treeViewSearchResults->setFocus();
}

void CKJVCanOpener::on_indexChanged(const TPhraseTag &tag)
{
	assert(m_pActionBookBackward != NULL);
	assert(m_pActionBookForward != NULL);
	assert(m_pActionChapterBackward != NULL);
	assert(m_pActionChapterForward != NULL);
	if ((m_pActionBookBackward == NULL) ||
		(m_pActionBookForward == NULL) ||
		(m_pActionChapterBackward == NULL) ||
		(m_pActionChapterForward == NULL)) return;

	m_pActionBookBackward->setEnabled(tag.first.book() >= 2);
	m_pActionBookForward->setEnabled(tag.first.book() < g_lstTOC.size());
	m_pActionChapterBackward->setEnabled((tag.first.book() >= 2) ||
										((tag.first.book() == 1) && (tag.first.chapter() >= 2)));
	m_pActionChapterForward->setEnabled((tag.first.book() < g_lstTOC.size()) ||
										((tag.first.book() == g_lstTOC.size()) && (tag.first.chapter() < g_lstTOC.at(tag.first.book()-1).m_nNumChp)));
}

void CKJVCanOpener::on_browserHistoryChanged()
{
	if (m_pActionNavBackward) {
		m_pActionNavBackward->setToolTip(ui->widgetKJVBrowser->browser()->historyTitle(-1));
		if (ui->widgetKJVBrowser->browser()->isBackwardAvailable()) {
			m_pActionNavBackward->setStatusTip("Go to: " + ui->widgetKJVBrowser->browser()->historyTitle(-1));
		} else {
			m_pActionNavBackward->setStatusTip("Go Backward in Navigation History");
		}
	}
	if (m_pActionNavForward) {
		m_pActionNavForward->setToolTip(ui->widgetKJVBrowser->browser()->historyTitle(+1));
		if (ui->widgetKJVBrowser->browser()->isForwardAvailable()) {
			m_pActionNavForward->setStatusTip("Go to: " + ui->widgetKJVBrowser->browser()->historyTitle(+1));
		} else {
			m_pActionNavForward->setStatusTip("Go Forward in Navigation History");
		}
	}
	if (m_pActionNavClear) m_pActionNavClear->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
														ui->widgetKJVBrowser->browser()->isForwardAvailable());
	if (m_pActionNavHome) m_pActionNavHome->setEnabled(ui->widgetKJVBrowser->browser()->isBackwardAvailable() ||
														ui->widgetKJVBrowser->browser()->isForwardAvailable());
}

void CKJVCanOpener::on_clearBrowserHistory()
{
	ui->widgetKJVBrowser->browser()->clearHistory();
}

void CKJVCanOpener::on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase)
{
	Q_UNUSED(pSearchPhrase);

	TParsedPhrasesList lstPhrases;
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		pPhrase->SetContributingNumberOfMatches(0);
		pPhrase->SetIsDuplicate(false);
		pPhrase->ClearScopedPhraseTagSearchResults();
		if ((!pPhrase->isCompleteMatch()) || (pPhrase->GetNumberOfMatches() == 0)) {
			continue;		// Don't include phrases that had no matches of themselves
		}
		// Check for phrases with the same text and ignore them:
		bool bDuplicate = false;
		for (int ndx2 = 0; ndx2 < ndx; ++ndx2) {
			if ((*pPhrase) == (*m_lstSearchPhraseEditors.at(ndx2)->parsedPhrase())) {
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate) {
			pPhrase->SetIsDuplicate(true);
			continue;
		}

		// Do sorted insertion so the results with the greatest matches comes last.  That
		//		way it will have more things we'll discard rather than things we'll uselessly
		//		look at:
		int ndxInsert = 0;
		for ( ; ndxInsert < lstPhrases.size(); ++ndxInsert) {
			if (pPhrase->GetNumberOfMatches() < lstPhrases.at(ndxInsert)->GetNumberOfMatches()) break;
		}
		lstPhrases.insert(ndxInsert, pPhrase);
	}

	// ----------------------------

	int nVerses = 0;		// Results counts in Verses
	int nChapters = 0;		// Results counts in Chapters
	int nBooks = 0;			// Results counts in Books
	int nResults = 0;		// Total number of Results in Scope
	TPhraseTagList lstResults;

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->treeViewSearchResults->model());
	assert(pModel != NULL);

	lstResults = pModel->setParsedPhrases(ui->widgetSearchCriteria->searchScopeMode(), lstPhrases);		// Setting the phrases will build all of the results and set the verse list on the model

	nVerses = pModel->GetVerseIndexAndCount().second;
	nChapters = pModel->GetChapterIndexAndCount().second;
	nBooks = pModel->GetBookIndexAndCount().second;
	nResults = pModel->GetResultsCount();

	// ----------------------------

	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		m_lstSearchPhraseEditors.at(ndx)->phraseStatisticsChanged();
	}

	// ----------------------------

	QString strResults;

	strResults += QString("Found %1 Occurrence%2\n").arg(nResults).arg((nResults != 1) ? "s" : "");
	strResults += QString("    in %1 Verse%2 in %3 Chapter%4 in %5 Book%6")
							.arg(nVerses).arg((nVerses != 1) ? "s" : "")
							.arg(nChapters).arg((nChapters != 1) ? "s" : "")
							.arg(nBooks).arg((nBooks != 1) ? "s" : "");
	if (nResults > 0) {
		strResults += "\n";
		strResults += QString("    Not found at all in %1 Verse%2 of the Bible\n")
								.arg(g_EntireBible.m_nNumVrs - nVerses).arg(((g_EntireBible.m_nNumVrs - nVerses) != 1) ? "s" : "");
		strResults += QString("    Not found at all in %1 Chapter%2 of the Bible\n")
								.arg(g_EntireBible.m_nNumChp - nChapters).arg(((g_EntireBible.m_nNumChp - nChapters) != 1) ? "s" : "");
		strResults += QString("    Not found at all in %1 Book%2 of the Bible")
								.arg(g_EntireBible.m_nNumBk - nBooks).arg(((g_EntireBible.m_nNumBk - nBooks) != 1) ? "s" : "");
	}

	ui->lblSearchResultsCount->setText(strResults);

	m_bLastCalcSuccess = true;
	m_nLastSearchOccurrences = nResults;
	m_nLastSearchVerses = nVerses;
	m_nLastSearchChapters = nChapters;
	m_nLastSearchBooks = nBooks;
	ui->widgetSearchCriteria->enableCopySearchPhraseSummary(nResults > 0);

	ui->widgetKJVBrowser->setHighlightTags(lstResults);

	emit changedSearchResults();
}

void CKJVCanOpener::on_SearchResultActivated(const QModelIndex &index)
{
	if (!index.isValid()) return;

	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());
	if (!ndxRel.isSet()) return;

	ui->widgetKJVBrowser->gotoIndex(TPhraseTag(ndxRel));
	ui->widgetKJVBrowser->focusBrowser();
}

void CKJVCanOpener::on_PassageNavigatorTriggered()
{
	if ((ui->widgetKJVBrowser->browser()->hasFocus()) ||
		(m_bBrowserActive)) {
		ui->widgetKJVBrowser->browser()->on_passageNavigator();
	} else if (((ui->treeViewSearchResults->hasFocus()) || (m_bSearchResultsActive)) &&
				(ui->treeViewSearchResults->selectionModel()->selectedRows().count()== 1)) {
		ui->treeViewSearchResults->on_passageNavigator();
	} else {
		CKJVPassageNavigatorDlg dlg(this);

		if (dlg.exec() == QDialog::Accepted) {
			ui->widgetKJVBrowser->gotoIndex(dlg.passage());
			ui->widgetKJVBrowser->focusBrowser();
		}
	}
}

void CKJVCanOpener::on_HelpManual()
{
	QMessageBox::information(this, windowTitle(), "An online help manual is coming soon for the KJV Can Opener.\n\nKeep your eyes open for future updates.");
}

void CKJVCanOpener::on_HelpAbout()
{
	CKJVAboutDlg dlg(this);
	dlg.exec();
}

