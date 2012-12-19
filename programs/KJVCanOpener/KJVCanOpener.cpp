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

#include <assert.h>

#include <QMenu>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QListView>
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

CSearchResultsListView::CSearchResultsListView(QWidget *parent)
	:	QListView(parent),
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

CSearchResultsListView::~CSearchResultsListView()
{
}

void CSearchResultsListView::on_copyVerseText()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QTextDocument docList;
	QTextCursor cursorDocList(&docList);

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	int nCount = 0;
	for (int ndx = 0; ndx < pModel->rowCount(); ++ndx) {
		if (!lstSelectedItems.contains(pModel->index(ndx))) continue;
		nCount++;
		const CVerseListItem &item(pModel->index(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
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

		QTextDocumentFragment fragment(&docVerse);
		cursorDocList.insertFragment(fragment);
//		if (nCount != lstSelectedItems.size()) cursorDocList.insertHtml("<hr />\n");
		if (nCount != lstSelectedItems.size()) cursorDocList.insertHtml("<br />\n");
	}

	mime->setText(docList.toPlainText());
	mime->setHtml(docList.toHtml());
	clipboard->setMimeData(mime);
}

void CSearchResultsListView::on_copyRaw()
{
	copyRawCommon(false);
}

void CSearchResultsListView::on_copyVeryRaw()
{
	copyRawCommon(true);
}

void CSearchResultsListView::copyRawCommon(bool bVeryRaw) const
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QString strText;

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	for (int ndx = 0; ndx < pModel->rowCount(); ++ndx) {
		if (!lstSelectedItems.contains(pModel->index(ndx))) continue;
		const CVerseListItem &item(pModel->index(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
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

void CSearchResultsListView::on_copyVerseHeadings()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QString strVerseHeadings;

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	for (int ndx = 0; ndx < pModel->rowCount(); ++ndx) {
		if (!lstSelectedItems.contains(pModel->index(ndx))) continue;
		const CVerseListItem &item(pModel->index(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		strVerseHeadings += item.getHeading() + "\n";
	}

	mime->setText(strVerseHeadings);
	clipboard->setMimeData(mime);
}

void CSearchResultsListView::on_copyReferenceDetails()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QString strPlainText;
	QString strRichText;

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	int nCount = 0;
	for (int ndx = 0; ndx < pModel->rowCount(); ++ndx) {
		if (!lstSelectedItems.contains(pModel->index(ndx))) continue;
		nCount++;
		if (nCount > 1) {
			strPlainText += "--------------------\n";
			strRichText += "<hr />\n";
		}
		strPlainText += pModel->index(ndx).data(CVerseListModel::TOOLTIP_PLAINTEXT_ROLE).toString();
		strRichText += pModel->index(ndx).data(Qt::ToolTipRole).toString();
	}

	mime->setText(strPlainText);
	mime->setHtml(strRichText);
	clipboard->setMimeData(mime);
}

void CSearchResultsListView::on_copyComplete()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();
	QTextDocument docList;
	QTextCursor cursorDocList(&docList);

	CVerseListModel *pModel = static_cast<CVerseListModel *>(model());
	assert(pModel != NULL);
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	int nCount = 0;
	for (int ndx = 0; ndx < pModel->rowCount(); ++ndx) {
		if (!lstSelectedItems.contains(pModel->index(ndx))) continue;
		nCount++;
		const CVerseListItem &item(pModel->index(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
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

		QTextDocumentFragment fragment(&docVerse);
		cursorDocList.insertFragment(fragment);

		cursorDocList.insertHtml("<br />\n<pre>" + item.getToolTip() + "</pre>\n");
		if (nCount != lstSelectedItems.size()) cursorDocList.insertHtml("\n<hr /><br />\n");
	}

	mime->setText(docList.toPlainText());
	mime->setHtml(docList.toHtml());
	clipboard->setMimeData(mime);
}

void CSearchResultsListView::on_passageNavigator()
{
	QModelIndexList lstSelectedItems = selectionModel()->selectedRows();
	if (lstSelectedItems.size() != 1) return;

	const CVerseListItem &item(lstSelectedItems.at(0).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
	CKJVPassageNavigatorDlg dlg(this);

	dlg.navigator().startAbsoluteMode(TPhraseTag(item.getIndex(), 0));
	if (dlg.exec() == QDialog::Accepted) {
		emit gotoIndex(dlg.passage());
	}
}

void CSearchResultsListView::focusInEvent(QFocusEvent *event)
{
	emit activatedSearchResults();
	QListView::focusInEvent(event);
}

void CSearchResultsListView::contextMenuEvent(QContextMenuEvent *event)
{
	m_bDoingPopup = true;
	m_pEditMenuLocal->exec(event->globalPos());
	m_bDoingPopup = false;

	QListView::contextMenuEvent(event);
}

void CSearchResultsListView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	handle_selectionChanged();
	QListView::selectionChanged(selected, deselected);
}

void CSearchResultsListView::handle_selectionChanged()
{
	int nNumResultsSelected = selectionModel()->selectedRows().size();

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
	m_pActionNavigator->setEnabled(nNumResultsSelected == 1);		// Only allow navigation on a single entry

	QString strStatusText = QString("%1 Search Result(s) Selected").arg(nNumResultsSelected);
	setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();
}

void CSearchResultsListView::on_listChanged()
{
	handle_selectionChanged();
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
	m_pMainSearchPhraseEditor(NULL),
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

	// --------------------

	QAction *pAction;

	// --- File Menu
	QMenu *pFileMenu = ui->menuBar->addMenu("&File");
	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), "E&xit", this, SLOT(close()), QKeySequence(Qt::CTRL + Qt::Key_Q));
	pAction->setStatusTip("Exit the King James Can Opener Application");
	pFileMenu->addAction(pAction);

	// --- Edit Menu
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(activatedScriptureText()), this, SLOT(on_activatedBrowser()));
	connect(ui->listViewSearchResults, SIGNAL(activatedSearchResults()), this, SLOT(on_activatedSearchResults()));

	// --- View Menu
	m_pViewMenu = ui->menuBar->addMenu("&View");

	QMenu *pViewToolbarsMenu = m_pViewMenu->addMenu("&Toolbars");
	pViewToolbarsMenu->addAction(ui->mainToolBar->toggleViewAction());
	ui->mainToolBar->toggleViewAction()->setStatusTip("Show/Hide Main Tool Bar");

	m_pViewMenu->addSeparator();
	ui->listViewSearchResults->getLocalEditMenu()->addSeparator();

	m_pActionShowVerseHeading = m_pViewMenu->addAction(QIcon(), "View &References Only", this, SLOT(on_viewVerseHeading()));
	m_pActionShowVerseHeading->setStatusTip("Show Search Results Verse References Only");
	m_pActionShowVerseHeading->setCheckable(true);
	m_pActionShowVerseHeading->setChecked(nDisplayMode == CVerseListModel::VDME_HEADING);
	ui->listViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowVerseHeading);

	m_pActionShowVerseRichText = m_pViewMenu->addAction(QIcon(), "View Verse &Preview", this, SLOT(on_viewVerseRichText()));
	m_pActionShowVerseRichText->setStatusTip("Show Search Results as Rich Text Verse Preview");
	m_pActionShowVerseRichText->setCheckable(true);
	m_pActionShowVerseRichText->setChecked(nDisplayMode == CVerseListModel::VDME_RICHTEXT);
	ui->listViewSearchResults->getLocalEditMenu()->addAction(m_pActionShowVerseRichText);

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

	ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_pMainSearchPhraseEditor = new CKJVSearchPhraseEdit(this);
	connect(m_pMainSearchPhraseEditor, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
	connect(m_pMainSearchPhraseEditor, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(on_phraseChanged(CKJVSearchPhraseEdit *)));
	m_lstSearchPhraseEditors.append(m_pMainSearchPhraseEditor);
	m_pMainSearchPhraseEditor->showSeperatorLine(false);
	m_pMainSearchPhraseEditor->enableCloseButton(false);
	QTimer::singleShot(0, m_pMainSearchPhraseEditor, SLOT(focusEditor()));

	m_pLayoutPhrases = new QVBoxLayout(ui->scrollAreaWidgetContents);
	m_pLayoutPhrases->setSpacing(0);
	m_pLayoutPhrases->setContentsMargins(0, 0, 0, 0);
	m_pLayoutPhrases->addWidget(m_pMainSearchPhraseEditor);

	ui->scrollAreaWidgetContents->setMinimumSize(/* pLayoutPhrases->sizeHint() */ m_pMainSearchPhraseEditor->sizeHint() );

	ui->scrollAreaSearchPhrases->setMinimumSize(m_pLayoutPhrases->sizeHint().width() +
							ui->scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui->scrollAreaSearchPhrases->frameWidth() * 2,
							m_pLayoutPhrases->sizeHint().height() /* pPhraseEdit->sizeHint() */);


//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);


	ui->widgetSearchCriteria->enableCopySearchPhraseSummary(false);

	connect(ui->widgetSearchCriteria, SIGNAL(addSearchPhraseClicked()), this, SLOT(on_addSearchPhraseClicked()));
	connect(ui->widgetSearchCriteria, SIGNAL(changedSearchScopeMode(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM)), this, SLOT(on_changedSearchCriteria()));
	connect(ui->widgetSearchCriteria, SIGNAL(copySearchPhraseSummary()), this, SLOT(on_copySearchPhraseSummary()));

	// -------------------- Search Results List View:

	CVerseListModel *model = new CVerseListModel(ui->listViewSearchResults);
	model->setDisplayMode(nDisplayMode);
	ui->listViewSearchResults->setModel(model);

	CVerseListDelegate *delegate = new CVerseListDelegate(*model, ui->listViewSearchResults);
	ui->listViewSearchResults->setItemDelegate(delegate);

	connect(ui->listViewSearchResults, SIGNAL(activated(const QModelIndex &)), this, SLOT(on_SearchResultActivated(const QModelIndex &)));
	connect(ui->listViewSearchResults, SIGNAL(gotoIndex(const TPhraseTag &)), ui->widgetKJVBrowser, SLOT(gotoIndex(TPhraseTag)));
	connect(this, SIGNAL(changedSearchResults()), ui->listViewSearchResults, SLOT(on_listChanged()));
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

void CKJVCanOpener::on_addSearchPhraseClicked()
{
	CKJVSearchPhraseEdit *pPhraseWidget = new CKJVSearchPhraseEdit(this);
	connect(pPhraseWidget, SIGNAL(destroyed(QObject*)), this, SLOT(on_closingSearchPhrase(QObject*)));
	connect(pPhraseWidget, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(on_phraseChanged(CKJVSearchPhraseEdit *)));
	m_lstSearchPhraseEditors.append(pPhraseWidget);
	m_pLayoutPhrases->addWidget(pPhraseWidget);
	ui->scrollAreaWidgetContents->setMinimumSize(m_pMainSearchPhraseEditor->sizeHint().width(), m_pMainSearchPhraseEditor->sizeHint().height() + pPhraseWidget->sizeHint().height()*(m_lstSearchPhraseEditors.size()-1));
	pPhraseWidget->focusEditor();

//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);
}

void CKJVCanOpener::on_closingSearchPhrase(QObject *pWidget)
{
	CKJVSearchPhraseEdit *pSearchPhraseWidget = static_cast<CKJVSearchPhraseEdit *>(pWidget);
	assert(pSearchPhraseWidget != NULL);

	int ndx = m_lstSearchPhraseEditors.indexOf(pSearchPhraseWidget);
	assert(ndx != -1);
	if (ndx != -1) {
		m_lstSearchPhraseEditors.removeAt(ndx);
	}
	if (m_lstSearchPhraseEditors.size() > 1) {
		ui->scrollAreaWidgetContents->setMinimumSize(m_pMainSearchPhraseEditor->sizeHint().width(), m_pMainSearchPhraseEditor->sizeHint().height() + m_lstSearchPhraseEditors.at(1)->sizeHint().height()*(m_lstSearchPhraseEditors.size()-1));
	} else {
		ui->scrollAreaWidgetContents->setMinimumSize(m_pMainSearchPhraseEditor->sizeHint().width(), m_pMainSearchPhraseEditor->sizeHint().height());
	}
	on_phraseChanged(NULL);
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
	switch (ui->widgetSearchCriteria->searchScopeMode()) {
		case (CKJVSearchCriteria::SSME_WHOLE_BIBLE):
			strScope = "in the Whole Bible";
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
	strSummary += QString("Search of %1 Phrase(s) %2:\n")
							.arg(nNumPhrases)
							.arg(strScope);
	if (nNumPhrases) strSummary += "\n";
	for (int ndx=0; ndx<mdlPhrases.rowCount(); ++ndx) {
		const CPhraseEntry &aPhrase = mdlPhrases.index(ndx).data(CPhraseListModel::PHRASE_ENTRY_ROLE).value<CPhraseEntry>();
		strSummary += QString("    \"%1\" (Found %2 Times, %3 in Scope)\n")
							.arg(mdlPhrases.index(ndx).data().toString())
							.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches)
							.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumContributingMatches);
	}
	if (bCaseSensitive) strSummary += QString("\n    (%1 = Case Sensitive)\n").arg(QChar(0xA7));
	if (nNumPhrases) strSummary += "\n";
	if (m_bLastCalcSuccess) {
		strSummary += QString("Found %1 Occurrences\n").arg(m_nLastSearchOccurrences);
		strSummary += QString("    in %1 Verses\n").arg(m_nLastSearchVerses);
		strSummary += QString("    in %1 Chapters\n").arg(m_nLastSearchChapters);
		strSummary += QString("    in %1 Books\n").arg(m_nLastSearchBooks);
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
			m_pActionSearchResultsEditMenu = ui->menuBar->insertMenu(m_pViewMenu->menuAction(), ui->listViewSearchResults->getEditMenu());
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

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		pModel->setDisplayMode(nMode);
	}

	m_bDoingUpdate = false;

	ui->listViewSearchResults->scrollTo(ui->listViewSearchResults->currentIndex(), QAbstractItemView::EnsureVisible);
}

void CKJVCanOpener::on_viewVerseRichText()
{
	assert(m_pActionShowVerseHeading != NULL);
	assert(m_pActionShowVerseRichText != NULL);

	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nMode = CVerseListModel::VDME_HEADING;

	if (m_pActionShowVerseRichText->isChecked()) {
		m_pActionShowVerseHeading->setChecked(false);
		nMode = CVerseListModel::VDME_RICHTEXT;
	} else {
		m_pActionShowVerseHeading->setChecked(true);
		nMode = CVerseListModel::VDME_HEADING;
	}

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		pModel->setDisplayMode(nMode);
	}

	m_bDoingUpdate = false;

	ui->listViewSearchResults->scrollTo(ui->listViewSearchResults->currentIndex(), QAbstractItemView::EnsureVisible);
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
	bool bCalcFlag = false;

	unsigned int nTotalMatches = 0;
	unsigned int nMaxTotalMatches = 0;
	TParsedPhrasesList lstPhrases;
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		pPhrase->SetContributingNumberOfMatches(0);
		if (pPhrase->GetNumberOfMatches() == 0) {
			if (m_lstSearchPhraseEditors.at(ndx) != pSearchPhrase)		// Don't notify the one that notified us, as it will be updating itself already
				m_lstSearchPhraseEditors.at(ndx)->phraseStatisticsChanged();
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
		pPhrase->SetIsDuplicate(bDuplicate);
		if (bDuplicate) {
			if (m_lstSearchPhraseEditors.at(ndx) != pSearchPhrase)		// Don't notify the one that notified us, as it will be updating itself already
				m_lstSearchPhraseEditors.at(ndx)->phraseStatisticsChanged();
			continue;
		}
		lstPhrases.append(pPhrase);
		nTotalMatches += pPhrase->GetNumberOfMatches();
		nMaxTotalMatches = qMax(nMaxTotalMatches, pPhrase->GetNumberOfMatches());
	}

	CVerseList lstReferences;
	TPhraseTagList lstResults;

	if ((g_bEnableNoLimits) || (nMaxTotalMatches <= 5000)) {		// This check keeps the really heavy hitters like 'and' and 'the' from making us come to a complete stand-still
		bCalcFlag = true;

		TPhraseTagListList lstlstResults;
		QList<int> lstNdxStart;
		QList<int> lstNdxEnd;
		QList<CRelIndex> lstScopedRefs;
		QList<bool> lstNeedScope;
		int nNumPhrases = 0;
		CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScopeMode = ui->widgetSearchCriteria->searchScopeMode();

		// Fetch results from all phrases and build a list of lists, denormalizing entries, and
		//		setting the phrase size details:
		for (int ndx=0; ndx<lstPhrases.size(); ++ndx) {
			const CParsedPhrase *phrase = lstPhrases.at(ndx);
			TIndexList lstPhraseResults = phrase->GetNormalizedSearchResults();
			assert(lstPhraseResults.size() != 0);		// Should have already eliminated phrases with no results in loop above
			nNumPhrases++;
			TPhraseTagList aLstOfResults;
			for (unsigned int ndxResults=0; ndxResults<lstPhraseResults.size(); ++ndxResults) {
				aLstOfResults.append(TPhraseTag(CRelIndex(DenormalizeIndex(lstPhraseResults.at(ndxResults))), phrase->phraseSize()));
			}
			lstlstResults.append(aLstOfResults);
			lstNdxStart.append(0);
			lstNdxEnd.append(0);
			lstScopedRefs.append(CRelIndex());
			lstNeedScope.append(true);
		}

		// Now, we'll go through our lists and compress the results to the scope specified
		//		for each phrase.  We'll then find the lowest valued one and see if the others
		//		match.  If they do, we'll push all of those results onto the output.  If not,
		//		we'll toss results for the lowest until we get a match.  When any list hits
		//		its end, we're done and can break out since we have no more matches

		bool bDone = (nNumPhrases == 0);		// We're done if we have no phrases (or phrases with results)
		while (!bDone) {
			uint32_t nMaxScope = 0;
			for (int ndx=0; ndx<nNumPhrases; ++ndx) {
				if (!lstNeedScope[ndx]) {
					nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
					continue;		// Only find next scope for a phrase if we need it
				}
				lstNdxStart[ndx] = lstNdxEnd[ndx];		// Begin at the last ending position
				if (lstNdxStart[ndx] >= lstlstResults[ndx].size()) {
					bDone = true;
					break;
				}
				lstScopedRefs[ndx] = lstlstResults[ndx].at(lstNdxStart[ndx]).first;
				ScopeIndex(lstScopedRefs[ndx], nSearchScopeMode);
				for (lstNdxEnd[ndx] = lstNdxStart[ndx]+1; lstNdxEnd[ndx] < lstlstResults[ndx].size(); ++lstNdxEnd[ndx]) {
					CRelIndex ndxScopedTemp = lstlstResults[ndx].at(lstNdxEnd[ndx]).first;
					ScopeIndex(ndxScopedTemp, nSearchScopeMode);
					if (lstScopedRefs[ndx].index() != ndxScopedTemp.index()) break;
				}
				// Here lstNdxEnd will be one more than the number of matching, either the next index
				//		off the end of the array, or the first non-matching entry.  So the scoped
				//		area is from lstNdxStart to lstNdxEnd-1.
				nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
				lstNeedScope[ndx] = false;
			}
			if (bDone) continue;		// If we run out of phrase matches on any phrase, we're done
			// Now, check the scoped references.  If they match for all indexes, we'll push the
			//	results to our output and set flags to get all new scopes.  Otherwise, compare them
			//	all against our maximum scope value and tag any that's less than that as needing a
			//	new scope (they weren't matches).  Then loop back until we've either pushed all
			//	results or run out of matches.
			bool bMatch = true;
			for (int ndx=0; ndx<nNumPhrases; ++ndx) {
				if (lstScopedRefs[ndx].index() != nMaxScope) {
					lstNeedScope[ndx] = true;
					bMatch = false;
				}
			}
			if (bMatch) {
				// We got a match, so push results to output and flag for new scopes:
				for (int ndx=0; ndx<nNumPhrases; ++ndx) {
					lstPhrases.at(ndx)->SetContributingNumberOfMatches(lstPhrases.at(ndx)->GetContributingNumberOfMatches() + (lstNdxEnd[ndx]-lstNdxStart[ndx]));
					for ( ; lstNdxStart[ndx]<lstNdxEnd[ndx]; ++lstNdxStart[ndx]) {
						lstResults.append(lstlstResults[ndx].at(lstNdxStart[ndx]));
					}
					lstNeedScope[ndx] = true;
				}
			}
		}
	}

	qSort(lstResults.begin(), lstResults.end(), TPhraseTagListSortPredicate::ascendingLessThan);


	for (int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
		if (!lstResults.at(ndxResults).first.isSet()) {
			assert(false);
			lstReferences.push_back(CVerseListItem(0, 0));
			continue;
		}
		lstReferences.push_back(CVerseListItem(lstResults.at(ndxResults)));

		CVerseListItem &verseItem(lstReferences.last());

		if (ndxResults<(lstResults.size()-1)) {
			bool bNextIsSameReference=false;
			CRelIndex ndxRelative = lstResults.at(ndxResults).first;
			do {
				CRelIndex ndxNextRelative = lstResults.at(ndxResults+1).first;

				if ((ndxRelative.book() == ndxNextRelative.book()) &&
					(ndxRelative.chapter() == ndxNextRelative.chapter()) &&
					(ndxRelative.verse() == ndxNextRelative.verse())) {
					verseItem.addPhraseTag(lstResults.at(ndxResults+1));
					bNextIsSameReference=true;
					ndxResults++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
		}
	}
	int nChapters = 0;		// Results counts in Chapters
	for (int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same chapter:
		if (ndxResults<(lstResults.size()-1)) {
			bool bNextIsSameReference=false;
			CRelIndex ndxRelative = lstResults.at(ndxResults).first;
			do {
				CRelIndex ndxNextRelative = lstResults.at(ndxResults+1).first;

				if ((ndxRelative.book() == ndxNextRelative.book()) &&
					(ndxRelative.chapter() == ndxNextRelative.chapter())) {
					bNextIsSameReference=true;
					ndxResults++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
		}
	}
	int nBooks = 0;			// Results counts in Books
	for (int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
		nBooks++;			// Count the book we are on and skip the ones that are on the same book:
		if (ndxResults<(lstResults.size()-1)) {
			bool bNextIsSameReference=false;
			CRelIndex ndxRelative = lstResults.at(ndxResults).first;
			do {
				CRelIndex ndxNextRelative = lstResults.at(ndxResults+1).first;

				if (ndxRelative.book() == ndxNextRelative.book()) {
					bNextIsSameReference=true;
					ndxResults++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
		}
	}

	// ----------------------------

	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		if (m_lstSearchPhraseEditors.at(ndx) != pSearchPhrase)		// Don't notify the one that notified us, as it will be updating itself already
			m_lstSearchPhraseEditors.at(ndx)->phraseStatisticsChanged();
	}

	// ----------------------------

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
//		if (lstReferences.size() <= 2000) {
			pModel->setVerseList(lstReferences);
//		} else {
//			pModel->setVerseList(CVerseList());
//		}
	}

	if (bCalcFlag) {
		ui->lblSearchResultsCount->setText(QString("Found %1 Occurrences\n    in %2 Verses in %3 Chapters in %4 Books")
						.arg(lstResults.size())
						.arg(lstReferences.size())
						.arg(nChapters)
						.arg(nBooks));
		m_bLastCalcSuccess = true;
		m_nLastSearchOccurrences = lstResults.size();
		m_nLastSearchVerses = lstReferences.size();
		m_nLastSearchChapters = nChapters;
		m_nLastSearchBooks = nBooks;
		ui->widgetSearchCriteria->enableCopySearchPhraseSummary(lstResults.size() > 0);
	} else {
		ui->lblSearchResultsCount->setText(QString("Found %1 possible matches\n(too many to process)").arg(nTotalMatches));
		m_bLastCalcSuccess = false;
		m_nLastSearchOccurrences = 0;
		m_nLastSearchVerses = 0;
		m_nLastSearchChapters = 0;
		m_nLastSearchBooks = 0;
		ui->widgetSearchCriteria->enableCopySearchPhraseSummary(true);
	}

	ui->widgetKJVBrowser->setHighlightTags(lstResults);

	emit changedSearchResults();
}

void CKJVCanOpener::ScopeIndex(CRelIndex &index, CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nMode)
{
	switch (nMode) {
		case (CKJVSearchCriteria::SSME_WHOLE_BIBLE):
			// For Whole Bible, we'll set the Book to 1 so that anything in the Bible matches:
			if (index.isSet()) index = CRelIndex(1, 0, 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_TESTAMENT):
			// For Testament, set the Book to the 1st Book of the corresponding Testament:
			if (index.book()) {
				if (index.book() <= g_lstTOC.size()) {
					const CTOCEntry &toc = g_lstTOC[index.book()-1];
					unsigned int nTestament = toc.m_nTstNdx;
					unsigned int nBook = 1;
					for (unsigned int i=1; i<nTestament; ++i)
						nBook += g_lstTestaments[i-1].m_nNumBk;
					index = CRelIndex();
					index.setBook(nBook);
				}
			}
			break;
		case (CKJVSearchCriteria::SSME_BOOK):
			// For Book, mask off Chapter, Verse, and Word:
			index = CRelIndex(index.book(), 0, 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_CHAPTER):
			// For Chapter, mask off Verse and Word:
			index = CRelIndex(index.book(), index.chapter(), 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_VERSE):
			// For Verse, mask off word:
			index = CRelIndex(index.book(), index.chapter(), index.verse(), 0);
			break;
		default:
			break;
	}
}

void CKJVCanOpener::on_SearchResultActivated(const QModelIndex &index)
{
	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	CVerseListItem verse = pModel->data(index, CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>();

//	unsigned int nWordCount = 1;
//	if (verse.phraseTags().size() == 1) {
//		nWordCount = verse.phraseTags().at(0).second;
//		ui->widgetKJVBrowser->gotoIndex(verse.phraseTags().at(0).first, nWordCount);
//	} else {
//		if (verse.phraseTags().size() > 1) {
//			ui->widgetKJVBrowser->gotoIndex(verse.phraseTags().at(0).first, 0);
//		} else {
//			ui->widgetKJVBrowser->gotoIndex(verse.getIndex());
//		}
//	}

	if (verse.phraseTags().size() != 0) {
		ui->widgetKJVBrowser->gotoIndex(TPhraseTag(verse.phraseTags().at(0).first));
	} else {
		ui->widgetKJVBrowser->gotoIndex(TPhraseTag(verse.getIndex()));
	}

	ui->widgetKJVBrowser->focusBrowser();
}

void CKJVCanOpener::on_PassageNavigatorTriggered()
{
	if ((ui->widgetKJVBrowser->browser()->hasFocus()) ||
		(m_bBrowserActive)) {
		ui->widgetKJVBrowser->browser()->on_passageNavigator();
	} else if (((ui->listViewSearchResults->hasFocus()) || (m_bSearchResultsActive)) &&
				(ui->listViewSearchResults->selectionModel()->selectedRows().count()== 1)) {
		ui->listViewSearchResults->on_passageNavigator();
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

