#include "KJVSearchPhraseEdit.h"
#include "ui_KJVSearchPhraseEdit.h"

#include "PhraseListModel.h"

#include <QStringListModel>
#include <QTextCharFormat>

#include <QTextDocumentFragment>

#include <algorithm>
#include <string>

#include <assert.h>


// ============================================================================

CPhraseLineEdit::CPhraseLineEdit(QWidget *pParent)
	:	QTextEdit(pParent),
		m_pCompleter(NULL),
		m_pCommonPhrasesCompleter(NULL),
		m_nLastCursorWord(-1),
		m_bUpdateInProgress(false),
		m_icoDroplist(":/res/droplist.png"),
		m_pButtonDroplist(NULL),
		m_pEditMenu(NULL),
		m_pStatusAction(NULL)
{
	setAcceptRichText(false);

	QAction *pAction;
	m_pEditMenu = new QMenu("&Edit");
	m_pEditMenu->setStatusTip("Search Phrase Editor Operations");
	pAction = m_pEditMenu->addAction("&Undo", this, SLOT(undo()), QKeySequence(Qt::CTRL + Qt::Key_Z));
	pAction->setStatusTip("Undo last operation to the Serach Phrase Editor");
	pAction->setEnabled(false);
	connect(this, SIGNAL(undoAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction("&Redo", this, SLOT(redo()), QKeySequence(Qt::CTRL + Qt::Key_Y));
	pAction->setStatusTip("Redo last operation on the Search Phrase Editor");
	pAction->setEnabled(false);
	connect(this, SIGNAL(redoAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
	pAction = m_pEditMenu->addAction("Cu&t", this, SLOT(cut()), QKeySequence(Qt::CTRL + Qt::Key_X));
	pAction->setStatusTip("Cut selected text from the Search Phrase Editor to the clipboard");
	pAction->setEnabled(false);
	connect(this, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction("&Copy", this, SLOT(copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	pAction->setStatusTip("Copy selected text from the Search Phrase Editor to the clipboard");
	pAction->setEnabled(false);
	connect(this, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction("&Paste", this, SLOT(paste()), QKeySequence(Qt::CTRL + Qt::Key_V));
	pAction->setStatusTip("Paste text on clipboard into the Search Phrase Editor");
	pAction->setEnabled(true);
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	pAction = m_pEditMenu->addAction("&Delete", this, SLOT(clear()), QKeySequence(Qt::Key_Delete));
	pAction->setStatusTip("Delete selected text from the Search Phrase Editor");
	pAction->setEnabled(false);
	connect(this, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));
	m_pEditMenu->addSeparator();
	pAction = m_pEditMenu->addAction("Select &All", this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	pAction->setStatusTip("Select All Text in the Search Phrase Editor");
	pAction->setEnabled(true);
	connect(pAction, SIGNAL(triggered()), this, SLOT(setFocus()));

	QStringListModel *pModel = new QStringListModel(g_lstConcordanceWords);
	m_pCompleter = new QCompleter(pModel, this);
	m_pCompleter->setWidget(this);
	m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);

	m_pButtonDroplist = new QPushButton(m_icoDroplist, QString(), this);
	m_pButtonDroplist->setFlat(true);
	m_pButtonDroplist->setToolTip("Show Phrase List");
	m_pButtonDroplist->setStatusTip("Show List of Common Phrases and User Phrases from Database");
	m_pButtonDroplist->setGeometry(sizeHint().width()-m_pButtonDroplist->sizeHint().width(),0,
								m_pButtonDroplist->sizeHint().width(),m_pButtonDroplist->sizeHint().height());

	CPhraseList phrases = g_lstCommonPhrases;
	phrases.append(g_lstUserPhrases);
	phrases.removeDuplicates();
	CPhraseListModel *pCommonPhrasesModel = new CPhraseListModel(phrases, this);
	pCommonPhrasesModel->sort(0, Qt::AscendingOrder);
	m_pCommonPhrasesCompleter = new QCompleter(pCommonPhrasesModel, this);
	m_pCommonPhrasesCompleter->setWidget(this);
	m_pCommonPhrasesCompleter->setCompletionMode(QCompleter::PopupCompletion);
	m_pCommonPhrasesCompleter->setCaseSensitivity(Qt::CaseSensitive);

	connect(this, SIGNAL(textChanged()), this, SLOT(on_textChanged()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
	connect(m_pCompleter, SIGNAL(activated(const QString &)), this, SLOT(insertCompletion(const QString&)));
	connect(m_pButtonDroplist, SIGNAL(clicked()), this, SLOT(on_dropCommonPhrasesClicked()));
	connect(m_pCommonPhrasesCompleter, SIGNAL(activated(const QString &)), this, SLOT(insertCommonPhraseCompletion(const QString&)));

	m_pStatusAction = new QAction(this);
}

void CPhraseLineEdit::setCaseSensitive(bool bCaseSensitive)
{
	CParsedPhrase::setCaseSensitive(bCaseSensitive);
	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);

	if (!m_bUpdateInProgress) {
		UpdateCompleter();
		emit phraseChanged(*this);
		emit changeCaseSensitive(bCaseSensitive);
	}
}

void CPhraseLineEdit::on_phraseListChanged()
{
	assert(m_pCommonPhrasesCompleter != NULL);
	if (m_pCommonPhrasesCompleter == NULL) return;

	CPhraseListModel *pModel = (CPhraseListModel *)(m_pCommonPhrasesCompleter->model());
	assert(pModel != NULL);
	if (pModel == NULL) return;

	CPhraseList phrases = g_lstCommonPhrases;
	phrases.append(g_lstUserPhrases);
	phrases.removeDuplicates();
	pModel->setPhraseList(phrases);
	pModel->sort(0, Qt::AscendingOrder);
}

void CPhraseLineEdit::insertCompletion(const QString& completion)
{
	CParsedPhrase::insertCompletion(textCursor(), completion);
}

void CPhraseLineEdit::insertCommonPhraseCompletion(const QString &completion)
{
	CPhraseCursor cursor(textCursor());
	cursor.clearSelection();
	cursor.select(QTextCursor::LineUnderCursor);
	bool bUpdateSave = m_bUpdateInProgress;
	m_bUpdateInProgress = true;									// Hold-over to update everything at once
	if (completion.startsWith('§')) {
		cursor.insertText(completion.mid(1));					// Replace with complete word minus special flag
		setCaseSensitive(true);
	} else {
		cursor.insertText(completion);							// Replace with completed word
		setCaseSensitive(false);
	}
	m_bUpdateInProgress = bUpdateSave;
	if (!m_bUpdateInProgress) {
		UpdateCompleter();
		emit phraseChanged(*this);
		emit changeCaseSensitive(isCaseSensitive());
	}
}

QString CPhraseLineEdit::textUnderCursor() const
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::WordUnderCursor);
	return cursor.selectedText();
}

void CPhraseLineEdit::on_textChanged()
{
	if (!m_bUpdateInProgress) {
		UpdateCompleter();

		emit phraseChanged(*this);
	}
}

void CPhraseLineEdit::on_cursorPositionChanged()
{
	if (!m_bUpdateInProgress) UpdateCompleter();
}

void CPhraseLineEdit::UpdateCompleter()
{
	CParsedPhrase::UpdateCompleter(textCursor(), *m_pCompleter);

	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;

	QTextCursor saveCursor = textCursor();
	saveCursor.clearSelection();

	CPhraseCursor cursor(textCursor());
	QTextCharFormat fmt = cursor.charFormat();
	fmt.setFontStrikeOut(false);
	fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.setCharFormat(fmt);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	int nWord = 0;
	do {
		cursor.selectWordUnderCursor();
		if (/* (GetCursorWordPos() != nWord) && */
			(static_cast<int>(GetMatchLevel()) <= nWord) &&
			(static_cast<int>(GetCursorMatchLevel()) <= nWord) &&
			((nWord != GetCursorWordPos()) ||
			 ((!GetCursorWord().isEmpty()) && (nWord == GetCursorWordPos()))
			 )
			) {
			fmt.setFontStrikeOut(true);
			fmt.setUnderlineColor(QColor(255,0,0));
			fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
			cursor.setCharFormat(fmt);
		}

		nWord++;
	} while (cursor.moveCursorWordRight(QTextCursor::MoveAnchor));

	m_bUpdateInProgress = false;
}

void CPhraseLineEdit::ParsePhrase(const QTextCursor &curInsert)
{
	// TODO : Remove this function after done debugging!

	CParsedPhrase::ParsePhrase(curInsert);

	if (m_pStatusAction) {
		QString strTemp;
		for (int n=0; n<m_lstWords.size(); ++n) {
			if (n==m_nCursorWord) strTemp += "(";
			strTemp += m_lstWords[n];
			if (n==m_nCursorWord) strTemp += ")";
			strTemp += " ";
		}
		strTemp += QString("  Cursor: %1  CursorLevel: %2  Level: %3  Words: %4").arg(m_nCursorWord).arg(m_nCursorLevel).arg(m_nLevel).arg(m_lstWords.size());
		setStatusTip(strTemp);
		m_pStatusAction->setStatusTip(strTemp);
		m_pStatusAction->showStatusText();
	}
}


void CPhraseLineEdit::insertFromMimeData(const QMimeData * source)
{
	if (!(textInteractionFlags() & Qt::TextEditable) || !source) return;

	bool bHasData = false;
	QTextDocumentFragment fragment;

	// Change all newlines to spaces, since we are simulating a single-line editor:

// Uncomment to re-enable rich text (don't forget to change acceptRichText setting in constructor)
//	if (source->hasFormat(QLatin1String("application/x-qrichtext")) && acceptRichText()) {
//		// x-qrichtext is always UTF-8 (taken from Qt3 since we don't use it anymore).
//		QString richtext = QString::fromUtf8(source->data(QLatin1String("application/x-qrichtext")));
//		richtext.prepend(QLatin1String("<meta name=\"qrichtext\" content=\"1\" />"));
//		fragment = QTextDocumentFragment::fromHtml(richtext, document());
//		bHasData = true;
//	} else if (source->hasHtml() && acceptRichText()) {
//		fragment = QTextDocumentFragment::fromHtml(source->html(), document());
//		bHasData = true;
//	} else {
		QString text = source->text();
		if (!text.isNull()) {
			text.replace("\r","");
			text.replace("\n"," ");
			if (!text.isEmpty()) {
				fragment = QTextDocumentFragment::fromPlainText(text);
				bHasData = true;
			}
		}
//	}

	if (bHasData) textCursor().insertFragment(fragment);
	ensureCursorVisible();
}

void CPhraseLineEdit::focusInEvent(QFocusEvent *event)
{
	emit activatedPhraseEdit(this);
	QTextEdit::focusInEvent(event);
}

void CPhraseLineEdit::keyPressEvent(QKeyEvent* event)
{
	bool bForceCompleter = false;

//	if (m_pCompleter->popup()->isVisible())
//	{
		switch (event->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
				event->ignore();
				return;

			case Qt::Key_Down:
				bForceCompleter = true;
				break;
		}
//	}

	QTextEdit::keyPressEvent(event);

	ParsePhrase(textCursor());

	QString strPrefix = m_strCursorWord;
	std::size_t nPreRegExp = strPrefix.toStdString().find_first_of("*?[]");
	if (nPreRegExp != std::string::npos) {
		strPrefix = strPrefix.left(nPreRegExp);
	}

	if (strPrefix != m_pCompleter->completionPrefix()) {
		m_pCompleter->setCompletionPrefix(strPrefix);
		UpdateCompleter();
		if (m_nLastCursorWord != m_nCursorWord) {
			m_pCompleter->popup()->close();
			m_nLastCursorWord = m_nCursorWord;
		}
		m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index(0, 0));
	}

	if (bForceCompleter || (!event->text().isEmpty() && ((m_strCursorWord.length() > 0) || (textCursor().atEnd()))))
		m_pCompleter->complete();

}

void CPhraseLineEdit::resizeEvent(QResizeEvent * /* event */)
{
	m_pButtonDroplist->move(width()-m_pButtonDroplist->width(),0);
}

void CPhraseLineEdit::on_dropCommonPhrasesClicked()
{
	m_pCommonPhrasesCompleter->complete();
}


// ============================================================================

CKJVSearchPhraseEdit::CKJVSearchPhraseEdit(QWidget *parent) :
	QWidget(parent),
	m_bUpdateInProgress(false),
	ui(new Ui::CKJVSearchPhraseEdit)
{
	ui->setupUi(this);

	ui->chkCaseSensitive->setChecked(ui->editPhrase->isCaseSensitive());
	ui->buttonAddPhrase->setEnabled(false);
	ui->buttonAddPhrase->setToolTip("Add Phrase to User Database");
	ui->buttonAddPhrase->setStatusTip("Add this Phrase to the User Database");
	ui->buttonDelPhrase->setEnabled(false);
	ui->buttonDelPhrase->setToolTip("Delete Phrase from User Database");
	ui->buttonDelPhrase->setStatusTip("Delete this Phrase from the User Database");
	ui->buttonClear->setEnabled(false);
	ui->buttonClear->setToolTip("Clear Phrase Text");
	ui->buttonClear->setStatusTip("Clear this Phrase Text");

	ui->buttonRemove->setToolTip("Remove Phrase from Search Criteria");
	ui->buttonRemove->setStatusTip("Remove this Phrase from the current Search Criteria");

	connect(ui->editPhrase, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
	connect(ui->chkCaseSensitive, SIGNAL(clicked(bool)), this, SLOT(on_CaseSensitiveChanged(bool)));
	connect(ui->editPhrase, SIGNAL(changeCaseSensitive(bool)), this, SLOT(on_CaseSensitiveChanged(bool)));
	connect(ui->buttonAddPhrase, SIGNAL(clicked()), this, SLOT(on_phraseAdd()));
	connect(ui->buttonDelPhrase, SIGNAL(clicked()), this, SLOT(on_phraseDel()));
	connect(ui->buttonClear, SIGNAL(clicked()), this, SLOT(on_phraseClear()));
	connect(this, SIGNAL(phraseListChanged()), ui->editPhrase, SLOT(on_phraseListChanged()));
	connect(ui->editPhrase, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)), this, SIGNAL(activatedPhraseEdit(const CPhraseLineEdit *)));
}

CKJVSearchPhraseEdit::~CKJVSearchPhraseEdit()
{
	delete ui;
}

const CParsedPhrase *CKJVSearchPhraseEdit::parsedPhrase() const
{
	return ui->editPhrase;
}

void CKJVSearchPhraseEdit::on_phraseChanged(const CParsedPhrase &phrase)
{
	ui->lblOccurrenceCount->setText(QString("Number of Occurrences: %1").arg(phrase.GetNumberOfMatches()));

	m_phraseEntry.m_strPhrase=phrase.phrase();		// Use reconstituted phrase for save/restore
	m_phraseEntry.m_bCaseSensitive=phrase.isCaseSensitive();
	m_phraseEntry.m_nNumWrd=phrase.phraseSize();

	bool bCommonFound = g_lstCommonPhrases.contains(m_phraseEntry);
	bool bUserFound = g_lstUserPhrases.contains(m_phraseEntry);
	bool bHaveText = (!ui->editPhrase->toPlainText().isEmpty());
	ui->buttonAddPhrase->setEnabled(bHaveText && !bUserFound && !bCommonFound);
	ui->buttonDelPhrase->setEnabled(bHaveText && bUserFound);
	ui->buttonClear->setEnabled(bHaveText);

	emit phraseChanged(phrase);
}

void CKJVSearchPhraseEdit::on_CaseSensitiveChanged(bool bCaseSensitive)
{
	if (m_bUpdateInProgress) return;
	m_bUpdateInProgress = true;
	ui->chkCaseSensitive->setChecked(bCaseSensitive);		// Set the checkbox in case the phrase editor is setting us
	ui->editPhrase->setCaseSensitive(bCaseSensitive);		// Set the phrase editor in case the checkbox is setting us
	m_bUpdateInProgress = false;
}

void CKJVSearchPhraseEdit::on_phraseAdd()
{
	g_lstUserPhrases.push_back(m_phraseEntry);
	g_bUserPhrasesDirty = true;
	ui->buttonAddPhrase->setEnabled(false);
	ui->buttonDelPhrase->setEnabled(!ui->editPhrase->toPlainText().isEmpty());
	emit phraseListChanged();
}

void CKJVSearchPhraseEdit::on_phraseDel()
{
	int ndx = g_lstUserPhrases.indexOf(m_phraseEntry);
	assert(ndx != -1);		// Shouldn't be in this handler if it didn't exist!!  What happened?
	if (ndx >= 0) {
		g_lstUserPhrases.removeAt(ndx);
		g_bUserPhrasesDirty = true;
	}
	ui->buttonAddPhrase->setEnabled(!ui->editPhrase->toPlainText().isEmpty());
	ui->buttonDelPhrase->setEnabled(false);
	emit phraseListChanged();
}

void CKJVSearchPhraseEdit::on_phraseClear()
{
	ui->editPhrase->clear();
}

