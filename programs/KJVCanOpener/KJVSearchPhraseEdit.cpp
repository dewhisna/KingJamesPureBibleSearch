#include "KJVSearchPhraseEdit.h"
#include "ui_KJVSearchPhraseEdit.h"

#include "dbstruct.h"

#include <QTextEdit>
#include <QModelIndex>
#include <QStringListModel>
#include <QTextCursor>
#include <QRegExp>

#include <assert.h>

// ============================================================================

CPhraseLineEdit::CPhraseLineEdit(QWidget *pParent)
	:	QTextEdit(pParent),
		m_pCompleter(NULL),
		m_nCompleterLevel(0)
{
	QStringListModel *pModel = new QStringListModel(g_lstConcordanceWords);
	m_pCompleter = new QCompleter(pModel, this);
	m_pCompleter->setWidget(this);
	m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
//	m_pCompleter->setCompletionMode(QCompleter::InlineCompletion);
	m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	connect(this, SIGNAL(textChanged()), this, SLOT(on_textChanged()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
	connect(m_pCompleter, SIGNAL(activated(const QString &)), this, SLOT(insertCompletion(const QString&)));
}

void CPhraseLineEdit::insertCompletion(const QString& completion)
{
	QTextCursor cursor = textCursor();
	int extra = completion.length() - m_pCompleter->completionPrefix().length();
	cursor.movePosition(QTextCursor::Left);
	cursor.movePosition(QTextCursor::EndOfWord);
	cursor.insertText(completion.right(extra));
	setTextCursor(cursor);
}

QString CPhraseLineEdit::textUnderCursor() const
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::WordUnderCursor);
	return cursor.selectedText();
}

void CPhraseLineEdit::on_textChanged()
{
	UpdateCompleter();
}

void CPhraseLineEdit::on_cursorPositionChanged()
{
	UpdateCompleter();
}

void CPhraseLineEdit::UpdateCompleter()
{
	QStringListModel *pModel = (QStringListModel *)(m_pCompleter->model());

	std::pair<QStringList, int> parseWords = ParsePhrase();
	assert(parseWords.second < parseWords.first.size());

	TIndexList lstMapping;			// Mapping to word targets
	QStringList lstNextWords = g_lstConcordanceWords;
	int nLevel = 0;
	for (int ndx=0; ndx<parseWords.second; ++ndx) {
		if (parseWords.first.at(ndx).isEmpty()) continue;

		TWordListMap::const_iterator itrWordMap;
		itrWordMap = g_mapWordList.find(parseWords.first.at(ndx));
		if (itrWordMap==g_mapWordList.end()) itrWordMap = g_mapWordList.find(parseWords.first.at(ndx).toLower());
		if (itrWordMap==g_mapWordList.end()) break;		// If we can't find this word, break out at this level and stop searching

		const CWordEntry &wordEntry = itrWordMap->second;		// Entry for current word
		if (nLevel == 0) {
			// If this is our first word, set its mapping to all possible next words:
			lstMapping = wordEntry.m_ndxNormalized;
		} else {
			// Otherwise, match this word from our list from the last mapping and populate
			//		a list of remaining mappings:
			TIndexList lstNextMapping;
			for (unsigned int ndxWord=0; ndxWord<lstMapping.size(); ++ndxWord) {
				if (((lstMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) &&
					(parseWords.first[ndx].compare(g_lstConcordanceWords[g_lstConcordanceMapping[lstMapping[ndxWord]+1]-1], Qt::CaseInsensitive) == 0)) {
					lstNextMapping.push_back(lstMapping[ndxWord]+1);
				}
			}
			lstMapping = lstNextMapping;
		}

		lstNextWords.clear();
		for (unsigned int ndxWord=0; ndxWord<lstMapping.size(); ++ndxWord) {
			if ((lstMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) {
				lstNextWords.push_back(g_lstConcordanceWords[g_lstConcordanceMapping[lstMapping[ndxWord]+1]-1]);
			}
		}
		lstNextWords.removeDuplicates();
		lstNextWords.sort();

		nLevel++;
	}

	m_nCompleterLevel = nLevel;
	pModel->setStringList(lstNextWords);


/*
	for (unsigned int ndx=1; ndx<((strCursorText.isEmpty() ? lstWords.size() : (lstWords.size()-1))) ; ++ndx) {
		if (strCursorText.compare(lstWords[ndx], Qt::CaseInsensitive) == 0) break;
		TIndexList lstNextMapping;
		for (unsigned int ndxWord=0; ndxWord<lstMapping.size(); ++ndxWord) {
			if (((lstMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) &&
				(lstWords[ndx].compare(g_lstConcordanceWords[g_lstConcordanceMapping[lstMapping[ndxWord]+1]-1], Qt::CaseInsensitive) == 0)) {
				lstNextMapping.push_back(lstMapping[ndxWord]+1);
			}
		}
		lstMapping = lstNextMapping;
	}

	for (unsigned int ndxWord=0; ndxWord<lstMapping.size(); ++ndxWord) {
		if ((lstMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) {
			lstNextWords.push_back(g_lstConcordanceWords[g_lstConcordanceMapping[lstMapping[ndxWord]+1]-1]);
		}
	}
	lstNextWords.removeDuplicates();

	pModel->setStringList(lstNextWords);
*/


}

std::pair<QStringList, int> CPhraseLineEdit::ParsePhrase()
{
	QStringList lstLeftWords;
	QStringList lstRightWords;
	QString strWord;

	QTextCursor curCursor = textCursor();
	bool bCurAtEnd = curCursor.atEnd();
	curCursor.select(QTextCursor::WordUnderCursor);
	strWord = curCursor.selectedText();					// Save current word
	curCursor.clearSelection();
	QTextCursor cursor(curCursor);

	while (!cursor.atStart()) {
		if (cursor.movePosition(QTextCursor::WordLeft)) {
			QTextCursor selCursor(cursor);
			selCursor.select(QTextCursor::WordUnderCursor);
			lstLeftWords.push_front(selCursor.selectedText());
		}
	}
	cursor = curCursor;
	if (!bCurAtEnd) cursor.movePosition(QTextCursor::WordLeft);
	while (!cursor.atEnd()) {
			QTextCursor selCursor(cursor);
			selCursor.select(QTextCursor::WordUnderCursor);
			lstRightWords.push_back(selCursor.selectedText());
		cursor.movePosition(QTextCursor::WordRight);
	}

	// FINALLY!! The above works!
	// Here:
	//	if (lstRightWords.size()!=0) then the first entry of lstRightWords is the current word! (Will only be true when !strWord.isEmpty() as well)
	//	if ((lstLeftWords.size()!=0) AND (!strWord.isEmpty())) then the last entry of lstLeftWords is the current word!

	if (!strWord.isEmpty()) {
		if (lstRightWords.size() != 0) lstRightWords.removeFirst();
		if (lstLeftWords.size() != 0) lstLeftWords.removeLast();
	}

	// The QTextCursor parses symbols, like "'" and "-" as individual words.  We
	//		want to treat them as part of the word, so find them and combine
	//		them with their word pairs.  In our text, a word could end with a "'"
	//		or a "'s", but that's the only uses of "'".
	//	So, there's two ways we could do it.  We could see if we have a "'" entry
	//		and see if the next entry is a "s" and if so combine all three.  Or
	//		else just combine the two.  But, this is very very hacky if their next
	//		word happens to start with an "s" and will cause problems.
	//	Another way to do it is to get the plaintext string, filter it for our
	//		character set and split it on whitespace.  Then, compare that against
	//		the entries from the cursor searching to find which word we are really
	//		on.  Wow, how hacky!
	//	There has to be a way to substitute the boundary finding logic of QTextCursor
	//		but I can't figure out what it is!!  The closest thing is to create our
	//		on locale with different flags per character denoting what constitutes
	//		a word character or not.  But that's a lot of work and not sure I could
	//		get it to work correctly.  <ugh!>

	QStringList lstCombinedWords;
	lstCombinedWords.append(lstLeftWords);
	int ndxCursor = lstCombinedWords.size();
	lstCombinedWords.append(strWord);
	lstCombinedWords.append(lstRightWords);

/*
	QString strLine = toPlainText();
	QStringList lstAllWords = strLine.split(QRegExp("[^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-]+"), QString::SkipEmptyParts);

	assert(lstCombinedWords.size() >= lstAllWords.size());
*/

	for (int i=1; i<lstCombinedWords.size(); ++i) {
		if (lstCombinedWords.at(i).compare("-", Qt::CaseInsensitive) == 0) {
			lstCombinedWords[i-1] = lstCombinedWords.at(i-1).trimmed() + lstCombinedWords.at(i).trimmed();
			if (i<(lstCombinedWords.size()-1)) {
				lstCombinedWords[i-1] += lstCombinedWords.at(i+1).trimmed();
				lstCombinedWords.removeAt(i+1);
				if (i<=ndxCursor) ndxCursor--;
			}
			lstCombinedWords.removeAt(i);
			i--;
			if (i<ndxCursor) ndxCursor--;
			continue;
		}
		if (lstCombinedWords.at(i).compare("'", Qt::CaseInsensitive) == 0) {
			lstCombinedWords[i-1] = lstCombinedWords.at(i-1).trimmed() + lstCombinedWords.at(i).trimmed();
			if ((i<(lstCombinedWords.size()-1)) && (lstCombinedWords.at(i+1).trimmed().compare("s", Qt::CaseInsensitive) == 0)) {
				lstCombinedWords[i-1] += lstCombinedWords.at(i+1).trimmed();
				lstCombinedWords.removeAt(i+1);
				if (i<=ndxCursor) ndxCursor--;
			}
			lstCombinedWords.removeAt(i);
			i--;
			if (i<ndxCursor) ndxCursor--;
			continue;
		}
	}

	// Make sure our cursor is within the index range of the list.  If we're adding
	//	things to the end of the list, we're at an empty string:
	if (ndxCursor == lstCombinedWords.size()) lstCombinedWords.push_back(QString());


QStatusBar *pStatusBar = ((CKJVSearchPhraseEdit *)parent())->pStatusBar;

QString strTemp;
for (int n=0; n<lstCombinedWords.size(); ++n) {
	if (n==ndxCursor) strTemp += "(";
	strTemp += lstCombinedWords[n];
	if (n==ndxCursor) strTemp += ")";
	strTemp += " ";
}
pStatusBar->showMessage(strTemp);




	return std::pair<QStringList, int>(lstCombinedWords, ndxCursor);

}


void CPhraseLineEdit::keyPressEvent(QKeyEvent* event)
{
//	if (m_pCompleter->popup()->isVisible())
//	{
		switch (event->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
				event->ignore();
				return;
		}
//	}

	QTextEdit::keyPressEvent(event);

//	const QString completionPrefix = textUnderCursor();
	std::pair<QStringList, int> parseWords = ParsePhrase();
	assert(parseWords.second < parseWords.first.size());
	const QString completionPrefix = parseWords.first.at(parseWords.second);

	if (completionPrefix != m_pCompleter->completionPrefix()) {
		m_pCompleter->setCompletionPrefix(completionPrefix);
		UpdateCompleter();
		m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index( 0, 0 ));
	}

//	if (!event->text().isEmpty() /* && completionPrefix.length() > 2 */)
	if ((!event->text().isEmpty()) && ((completionPrefix.length() > 0) || (textCursor().atEnd())))
		m_pCompleter->complete();
}



/*
	bool MyTextEdit::event(QEvent* event)
	{
	if (event->type() == QEvent::ToolTip)
	{
	QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
	QTextCursor cursor = cursorForPosition(helpEvent->pos());
	cursor.select(QTextCursor::WordUnderCursor);
	if (!cursor.selectedText().isEmpty())
	QToolTip::showText(helpEvent->globalPos(), cursor.selectedText());
	else
	QToolTip::hideText();
	return true;
	}
	return QTextEdit::event(event);
	}
*/




// ============================================================================

CKJVSearchPhraseEdit::CKJVSearchPhraseEdit(QWidget *parent) :
	QWidget(parent),
pStatusBar(NULL),
	ui(new Ui::CKJVSearchPhraseEdit)
{
	ui->setupUi(this);

/*
	QStringListModel *pModel = new QStringListModel(g_lstConcordanceWords);
	m_pCompleter = new QCompleter(pModel, this);
	m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
//	m_pCompleter->setCompletionMode(QCompleter::InlineCompletion);
	m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_pCompleter->setWidget(ui->editPhrase);

//	ui->editPhrase->setCompleter(m_pCompleter);
	ui->editPhrase->installEventFilter(this);
	connect(ui->editPhrase, SIGNAL(textEdited(const QString &)), this, SLOT(on_textEdited(const QString&)));
	connect(m_pCompleter, SIGNAL(activated(const QString &)), this, SLOT(insertCompletion(const QString&)));
*/

}

CKJVSearchPhraseEdit::~CKJVSearchPhraseEdit()
{
	delete ui;
}

/*

void CKJVSearchPhraseEdit::insertCompletion(const QString& completion)
{
	QTextCursor cursor = ui->editPhrase->textCursor();
	int extra = completion.length() - m_pCompleter->completionPrefix().length();
	cursor.movePosition(QTextCursor::Left);
	cursor.movePosition(QTextCursor::EndOfWord);
	cursor.insertText(completion.right(extra));
	ui->editPhrase->setTextCursor(cursor);
}

QString CKJVSearchPhraseEdit::textUnderCursor() const
{
	QTextCursor cursor = ui->editPhrase->textCursor();
	cursor.select(QTextCursor::WordUnderCursor);
	return cursor.selectedText();
}

void CKJVSearchPhraseEdit::on_textEdited(const QString &text)
{
	QStringListModel *pModel = (QStringListModel *)(m_pCompleter->model());
	pModel->setStringList(g_lstConcordanceWords);
//	pModel->setStringList();
}

//void CKJVSearchPhraseEdit::keyPressEvent(QKeyEvent* event)
bool CKJVSearchPhraseEdit::eventFilter(QObject *obj, QEvent *event)
{
	bool bRetVal = true;

	if ((obj == ui->editPhrase) && (event->type() == QEvent::KeyPress)) {
		QKeyEvent *keyevent = (QKeyEvent *)(event);

		if (m_pCompleter->popup()->isVisible())
		{
			switch (keyevent->key()) {
				case Qt::Key_Enter:
				case Qt::Key_Return:
				case Qt::Key_Escape:
				case Qt::Key_Tab:
					keyevent->ignore();
					return true;
			}
		}

		bRetVal = QWidget::eventFilter(obj, event);

		const QString completionPrefix = textUnderCursor();

		if (completionPrefix != m_pCompleter->completionPrefix()) {
			m_pCompleter->setCompletionPrefix(completionPrefix);
			m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index( 0, 0 ));
		}

		if (!keyevent->text().isEmpty() && completionPrefix.length() > 2)
			m_pCompleter->complete();
	} else {
		bRetVal = QWidget::eventFilter(obj, event);
	}

	return bRetVal;
}


*/

