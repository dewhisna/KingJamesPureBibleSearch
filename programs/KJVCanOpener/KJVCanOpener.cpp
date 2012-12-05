#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

#include "dbstruct.h"
#include "VerseListModel.h"
#include "VerseListDelegate.h"
#include "KJVPassageNavigatorDlg.h"
#include "BuildDB.h"
#include "KJVAboutDlg.h"
#include "PhraseEdit.h"
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
		m_pStatusAction(NULL)
{
	setMouseTracking(true);

	m_pEditMenu = new QMenu("&Edit", this);
	m_pEditMenuLocal = new QMenu("&Edit", this);
	m_pEditMenu->setStatusTip("Search Results Edit Operations");
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
	m_pEditMenu->addSeparator();
	m_pEditMenuLocal->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction("Select &All", this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip("Select all Search Results");
	m_pEditMenuLocal->addAction(m_pActionSelectAll);
	m_pActionClearSelection = m_pEditMenu->addAction("C&lear Selection", this, SLOT(clearSelection()), QKeySequence(Qt::Key_Escape));
	m_pActionClearSelection->setStatusTip("Clear Search Results Selection");
	m_pActionClearSelection->setEnabled(false);
	m_pEditMenuLocal->addAction(m_pActionClearSelection);

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

	QModelIndexList lstSelectedItems = selectedIndexes();
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
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
//		if (ndx != (lstSelectedItems.size()-1)) cursorDocList.insertHtml("<hr />\n");
		if (ndx != (lstSelectedItems.size()-1)) cursorDocList.insertHtml("<br />\n");
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

	QModelIndexList lstSelectedItems = selectedIndexes();
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
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

	QModelIndexList lstSelectedItems = selectedIndexes();
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
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

	QModelIndexList lstSelectedItems = selectedIndexes();
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (ndx) {
			strPlainText += "--------------------\n";
			strRichText += "<hr />\n";
		}
		strPlainText += lstSelectedItems.at(ndx).data(CVerseListModel::TOOLTIP_PLAINTEXT_ROLE).toString();
		strRichText += lstSelectedItems.at(ndx).data(Qt::ToolTipRole).toString();
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

	QModelIndexList lstSelectedItems = selectedIndexes();
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		const CVerseListItem &item(lstSelectedItems.at(ndx).data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());
		QTextDocument docVerse;
		CPhraseNavigator navigator(docVerse);
		CSearchResultHighlighter highlighter(item.phraseTags());

		navigator.setDocumentToVerse(item.getIndex(), (ndx != 0));		// Not quite sure why passing true here doesn't give us an <hr> in our results but a <br>.  Yet adding an <hr> manually later works.  Hmmm...
		navigator.doHighlighting(highlighter);

		QTextCursor cursorDocVerse(&docVerse);
		cursorDocVerse.select(QTextCursor::Document);
		QTextDocumentFragment fragment(cursorDocVerse);
		cursorDocList.insertFragment(fragment);

		cursorDocList.insertHtml("<br />\n<pre>" + item.getToolTip() + "</pre>\n");
		if (ndx != (lstSelectedItems.size()-1)) cursorDocList.insertHtml("\n<hr /><br />\n");
	}

	mime->setText(docList.toPlainText());
	mime->setHtml(docList.toHtml());
	clipboard->setMimeData(mime);
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
	if (selectedIndexes().size()) {
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

	QString strStatusText = QString("%1 Search Result(s) Selected").arg(selectedIndexes().size());
	setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();

	QListView::selectionChanged(selected, deselected);
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
	ui(new Ui::CKJVCanOpener)
{
	ui->setupUi(this);

	ui->splitter->handle(1)->setAttribute(Qt::WA_Hover);		// Work-Around QTBUG-13768
	setStyleSheet("QSplitter::handle:hover { background-color: palette(highlight); }");

// The following is supposed to be another workaround for QTBUG-13768
//	ui->splitter->setStyleSheet("QSplitterHandle:hover {}  QSplitter::handle:hover { background-color: palette(highlight); }");

	// TODO : Set preference for start mode!:
	CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode = CVerseListModel::VDME_RICHTEXT;

	QAction *pAction;

	QMenu *pFileMenu = ui->menuBar->addMenu("&File");
	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), "E&xit", this, SLOT(close()), QKeySequence(Qt::CTRL + Qt::Key_Q));
	pAction->setStatusTip("Exit the King James Can Opener Application");
	pFileMenu->addAction(pAction);

//	QMenu *pEditMenu = ui->menuBar->addMenu("&Edit");
//	QMenu *pBrowserEditMenu = ui->widgetKJVBrowser->browser()->getEditMenu();
//	ui->menuBar->addMenu(pBrowserEditMenu);
//	connect(ui->widgetKJVBrowser->browser(), SIGNAL(addEditMenu(bool)), this, SLOT(on_addPassageBrowserEditMenu(bool)));
	connect(ui->widgetKJVBrowser->browser(), SIGNAL(activatedScriptureText()), this, SLOT(on_activatedBrowser()));
	connect(ui->listViewSearchResults, SIGNAL(activatedSearchResults()), this, SLOT(on_activatedSearchResults()));

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

	ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	CKJVSearchPhraseEdit *pPhraseEdit = new CKJVSearchPhraseEdit(this);
	connect(pPhraseEdit, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
	connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
	m_lstSearchPhraseEditors.append(pPhraseEdit);

	QVBoxLayout *pLayoutPhrases = new QVBoxLayout(ui->scrollAreaWidgetContents);
	pLayoutPhrases->setSpacing(0);
	pLayoutPhrases->setContentsMargins(0, 0, 0, 0);
	pLayoutPhrases->addWidget(pPhraseEdit);

	ui->scrollAreaWidgetContents->setMinimumSize(/* pLayoutPhrases->sizeHint() */ pPhraseEdit->sizeHint() );

	ui->scrollAreaSearchPhrases->setMinimumSize(pLayoutPhrases->sizeHint().width() +
							ui->scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui->scrollAreaSearchPhrases->frameWidth() * 2,
							pLayoutPhrases->sizeHint().height() /* pPhraseEdit->sizeHint() */);

/*
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());
pLayoutPhrases->addWidget(new CKJVSearchPhraseEdit());

ui->scrollAreaWidgetContents->setMinimumSize(pPhraseEdit->sizeHint().width(), pPhraseEdit->sizeHint().height()*7);
*/



pPhraseEdit = new CKJVSearchPhraseEdit(this);
connect(pPhraseEdit, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
m_lstSearchPhraseEditors.append(pPhraseEdit);
pLayoutPhrases->addWidget(pPhraseEdit);

pPhraseEdit = new CKJVSearchPhraseEdit(this);
connect(pPhraseEdit, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
m_lstSearchPhraseEditors.append(pPhraseEdit);
pLayoutPhrases->addWidget(pPhraseEdit);

pPhraseEdit = new CKJVSearchPhraseEdit(this);
connect(pPhraseEdit, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
m_lstSearchPhraseEditors.append(pPhraseEdit);
pLayoutPhrases->addWidget(pPhraseEdit);

pPhraseEdit = new CKJVSearchPhraseEdit(this);
connect(pPhraseEdit, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
m_lstSearchPhraseEditors.append(pPhraseEdit);
pLayoutPhrases->addWidget(pPhraseEdit);

pPhraseEdit = new CKJVSearchPhraseEdit(this);
connect(pPhraseEdit, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
m_lstSearchPhraseEditors.append(pPhraseEdit);
pLayoutPhrases->addWidget(pPhraseEdit);

pPhraseEdit = new CKJVSearchPhraseEdit(this);
connect(pPhraseEdit, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit *)));
connect(pPhraseEdit, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
m_lstSearchPhraseEditors.append(pPhraseEdit);
pLayoutPhrases->addWidget(pPhraseEdit);

m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);


//ui->scrollAreaWidgetContents->setMinimumSize(pPhraseEdit->sizeHint().width(), pPhraseEdit->sizeHint().height()*1);
ui->scrollAreaWidgetContents->setMinimumSize(pPhraseEdit->sizeHint().width(), pPhraseEdit->sizeHint().height()*7);



	CVerseListModel *model = new CVerseListModel(ui->listViewSearchResults);
	model->setDisplayMode(nDisplayMode);
	ui->listViewSearchResults->setModel(model);

	CVerseListDelegate *delegate = new CVerseListDelegate(*model, ui->listViewSearchResults);
	ui->listViewSearchResults->setItemDelegate(delegate);

	connect(ui->listViewSearchResults, SIGNAL(activated(const QModelIndex &)), this, SLOT(on_SearchResultActivated(const QModelIndex &)));
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

void CKJVCanOpener::on_addPassageBrowserEditMenu(bool bAdd)
{
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

void CKJVCanOpener::on_phraseChanged(const CParsedPhrase &phrase)
{
	CVerseList lstReferences;
	TPhraseTagList lstTags;

	if (phrase.GetNumberOfMatches() <= 5000) {		// This check keeps the really heavy hitters like 'and' and 'the' from making us come to a complete stand-still
		TIndexList lstResults = phrase.GetNormalizedSearchResults();

		for (unsigned int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
			uint32_t ndxDenormal = DenormalizeIndex(lstResults.at(ndxResults));
			CRelIndex ndxRelative(ndxDenormal);
			unsigned int nPhraseSize = phrase.phraseSize();

			if ((lstResults[ndxResults] == 0) || (ndxDenormal == 0)) {
				assert(false);
				lstReferences.push_back(CVerseListItem(0, 0));
				continue;
			} else {
				lstReferences.push_back(CVerseListItem(ndxRelative, nPhraseSize));
			}

			CVerseListItem &verseItem(lstReferences.last());

			if (ndxResults<(lstResults.size()-1)) {
				bool bNextIsSameReference=false;
				do {
					CRelIndex ndxNextRelative(DenormalizeIndex(lstResults.at(ndxResults+1)));
					if ((ndxRelative.book() == ndxNextRelative.book()) &&
						(ndxRelative.chapter() == ndxNextRelative.chapter()) &&
						(ndxRelative.verse() == ndxNextRelative.verse())) {
						verseItem.addPhraseTag(ndxNextRelative, nPhraseSize);
						bNextIsSameReference=true;
						ndxResults++;
					} else {
						bNextIsSameReference=false;
					}
				} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
			}

			lstTags.append(verseItem.phraseTags());
		}
//		lstReferences.removeDuplicates();
	}

	CVerseListModel *pModel = static_cast<CVerseListModel *>(ui->listViewSearchResults->model());
	if (pModel) {
		if (lstReferences.size() <= 2000) {
			pModel->setVerseList(lstReferences);
		} else {
			pModel->setVerseList(CVerseList());
		}
	}

	if ((lstReferences.size() != 0) ||
		((lstReferences.size() == 0) && (phrase.GetNumberOfMatches() == 0))) {
		ui->lblSearchResultsCount->setText(QString("Found %1 occurrences in %2 verses").arg(phrase.GetNumberOfMatches()).arg(lstReferences.size()));
	} else {
		ui->lblSearchResultsCount->setText(QString("Found %1 occurrences (too many verses!)").arg(phrase.GetNumberOfMatches()));
	}

	ui->widgetKJVBrowser->setHighlightTags(lstTags);
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
	if (ui->widgetKJVBrowser->browser()->hasFocus()) {
		ui->widgetKJVBrowser->browser()->on_passageNavigator();
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

}

void CKJVCanOpener::on_HelpAbout()
{
	CKJVAboutDlg dlg(this);

	dlg.exec();
}

