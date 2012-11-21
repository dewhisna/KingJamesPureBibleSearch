#include "KJVSearchPhraseEdit.h"
#include "ui_KJVSearchPhraseEdit.h"

#include "dbstruct.h"
#include "PhraseListModel.h"

#include <QTextEdit>
#include <QModelIndex>
#include <QStringListModel>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QRegExp>

#include <algorithm>
#include <string>

#include <assert.h>

// ============================================================================

uint32_t CParsedPhrase::GetNumberOfMatches() const
{
	return m_lstMatchMapping.size();
}

TIndexList CParsedPhrase::GetNormalizedSearchResults() const
{
	TIndexList lstResults;

	lstResults.resize(m_lstMatchMapping.size());
	for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
		lstResults[ndxWord] = m_lstMatchMapping.at(ndxWord) - m_nLevel + 1;
	}
	sort(lstResults.begin(), lstResults.end());

	return lstResults;
}

uint32_t CParsedPhrase::GetMatchLevel() const
{
	return m_nLevel;
}

uint32_t CParsedPhrase::GetCursorMatchLevel() const
{
	return m_nCursorLevel;
}

QString CParsedPhrase::GetCursorWord() const
{
	return m_strCursorWord;
}

int CParsedPhrase::GetCursorWordPos() const
{
	return m_nCursorWord;
}

QString CParsedPhrase::phrase() const
{
	QString strPhrase;
	for (int ndx = 0; ndx < m_lstWords.size(); ++ndx) {
		if (m_lstWords.at(ndx).isEmpty()) continue;
		if (ndx) strPhrase += " ";
		strPhrase += m_lstWords.at(ndx);
	}
	return strPhrase;
}

unsigned int CParsedPhrase::phraseSize() const
{
	unsigned int nSize = 0;
	for (int ndx = 0; ndx < m_lstWords.size(); ++ndx) {
		if (m_lstWords.at(ndx).isEmpty()) continue;
		nSize++;
	}
	return nSize;
}

void CParsedPhrase::UpdateCompleter(const QTextCursor &curInsert, QCompleter &aCompleter)
{
	QStringListModel *pModel = (QStringListModel *)(aCompleter.model());

	ParsePhrase(curInsert);
	FindWords();

	pModel->setStringList(m_lstNextWords);
}

QTextCursor CParsedPhrase::insertCompletion(const QTextCursor &curInsert, const QString& completion)
{

//	QTextCursor cursor = curInsert;
////	int extra = completion.length() - m_pCompleter->completionPrefix().length();
////	cursor.movePosition(QTextCursor::Left);
////	cursor.movePosition(QTextCursor::EndOfWord);
////	cursor.insertText(completion.right(extra));
////	setTextCursor(cursor);

//	cursor.clearSelection();
///*
//	if (((m_strCursorWord.compare("-", Qt::CaseInsensitive) == 0) && (completion.contains('-', Qt::CaseInsensitive))) ||
//		((m_strCursorWord.compare("'", Qt::CaseInsensitive) == 0) && (completion.contains('\'', Qt::CaseInsensitive)))) {
//		cursor.movePosition(QTextCursor::WordLeft);
//		cursor.movePosition(QTextCursor::WordLeft);
//		cursor.selectionStart();
//		cursor.movePosition(QTextCursor::WordRight);
//		cursor.movePosition(QTextCursor::WordRight);
//	}
//*/
////	cursor.movePosition(QTextCursor::WordLeft);			// TODO : Find proper movement based on word arrays!

//	cursor.select(QTextCursor::WordUnderCursor);
//	cursor.insertText(completion);


	CPhraseCursor cursor(curInsert);
	cursor.clearSelection();
	cursor.selectWordUnderCursor();							// Select word under the cursor
	cursor.insertText(completion);							// Replace with completed word

	return cursor;
}

void CParsedPhrase::ParsePhrase(const QTextCursor &curInsert)
{
	m_lstLeftWords.clear();
	m_lstRightWords.clear();
	m_strCursorWord.clear();

/*
	QTextCursor curCursor = curInsert;
	bool bCurAtEnd = curCursor.atEnd();
	curCursor.select(QTextCursor::WordUnderCursor);
	m_strCursorWord = curCursor.selectedText();					// Save current word
	curCursor.clearSelection();
	QTextCursor cursor(curCursor);

	while (!cursor.atStart()) {
		if (cursor.movePosition(QTextCursor::WordLeft)) {
			QTextCursor selCursor(cursor);
			selCursor.select(QTextCursor::WordUnderCursor);
			m_lstLeftWords.push_front(selCursor.selectedText());
		}
	}
	cursor = curCursor;
	if (!bCurAtEnd) cursor.movePosition(QTextCursor::WordLeft);
	while (!cursor.atEnd()) {
			QTextCursor selCursor(cursor);
			selCursor.select(QTextCursor::WordUnderCursor);
			m_lstRightWords.push_back(selCursor.selectedText());
		cursor.movePosition(QTextCursor::WordRight);
	}

	// FINALLY!! The above works!
	// Here:
	//	if (m_lstRightWords.size()!=0) then the first entry of m_lstRightWords is the current word! (Will only be true when !m_strCursorWord.isEmpty() as well)
	//	if ((m_lstLeftWords.size()!=0) AND (!m_strCursorWord.isEmpty())) then the last entry of m_lstLeftWords is the current word!

	if (!m_strCursorWord.isEmpty()) {
		if (m_lstRightWords.size() != 0) m_lstRightWords.removeFirst();
		if (m_lstLeftWords.size() != 0) m_lstLeftWords.removeLast();
	}
*/

	CPhraseCursor curLeft(curInsert);
	while (curLeft.moveCursorWordLeft()) {
		m_lstLeftWords.push_front(curLeft.wordUnderCursor());
	}

	CPhraseCursor curRight(curInsert);
	m_strCursorWord = curRight.wordUnderCursor();
	while (curRight.moveCursorWordRight()) {
		m_lstRightWords.push_back(curRight.wordUnderCursor());
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

	m_lstWords.clear();
	m_lstWords.append(m_lstLeftWords);
	m_nCursorWord = m_lstWords.size();
	m_lstWords.append(m_strCursorWord);
	m_lstWords.append(m_lstRightWords);

/*
	QString strLine = toPlainText();
	QStringList lstAllWords = strLine.split(QRegExp("[^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-]+"), QString::SkipEmptyParts);

	assert(m_lstWords.size() >= lstAllWords.size());
*/

/*

	for (int i=1; i<m_lstWords.size(); ++i) {
		if (m_lstWords.at(i).compare("-", Qt::CaseInsensitive) == 0) {
			m_lstWords[i-1] = m_lstWords.at(i-1).trimmed() + m_lstWords.at(i).trimmed();
			if (i<(m_lstWords.size()-1)) {
				m_lstWords[i-1] += m_lstWords.at(i+1).trimmed();
				m_lstWords.removeAt(i+1);
				if (i<=m_nCursorWord) m_nCursorWord--;
			}
			m_lstWords.removeAt(i);
			i--;
			if (i<m_nCursorWord) m_nCursorWord--;
			continue;
		}
		if (m_lstWords.at(i).compare("'", Qt::CaseInsensitive) == 0) {
			m_lstWords[i-1] = m_lstWords.at(i-1).trimmed() + m_lstWords.at(i).trimmed();
			if ((i<(m_lstWords.size()-1)) && (m_lstWords.at(i+1).trimmed().compare("s", Qt::CaseInsensitive) == 0)) {
				m_lstWords[i-1] += m_lstWords.at(i+1).trimmed();
				m_lstWords.removeAt(i+1);
				if (i<=m_nCursorWord) m_nCursorWord--;
			}
			m_lstWords.removeAt(i);
			i--;
			if (i<m_nCursorWord) m_nCursorWord--;
			continue;
		}
	}


*/


	// Make sure our cursor is within the index range of the list.  If we're adding
	//	things to the end of the list, we're at an empty string:
	if (m_nCursorWord == m_lstWords.size()) m_lstWords.push_back(QString());


}

void CParsedPhrase::ParsePhrase(const QString &strPhrase)
{
	m_lstLeftWords.clear();
	m_lstRightWords.clear();
	m_strCursorWord.clear();
	m_lstWords.clear();

	m_lstLeftWords = strPhrase.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	m_lstWords.append(m_lstLeftWords);
	m_nCursorWord = m_lstWords.size();
	m_lstWords.append(m_strCursorWord);
	m_lstWords.append(m_lstRightWords);
}

void CParsedPhrase::FindWords()
{
	assert(m_nCursorWord < m_lstWords.size());

	m_lstMatchMapping.clear();
	m_lstMapping.clear();
	m_lstNextWords = g_lstConcordanceWords;
	m_nLevel = 0;
	m_nCursorLevel = 0;
	for (int ndx=0; ndx<m_lstWords.size(); ++ndx) {
		if (m_lstWords.at(ndx).isEmpty()) continue;
//		if (m_lstWords.at(ndx).isEmpty()) {
//			if (ndx != 0) {
//				m_nLevel++;
//				if (ndx < m_nCursorWord) m_nCursorLevel = m_nLevel;
//			}
//			continue;
//		}

		TWordListMap::const_iterator itrWordMap;
		TWordListMap::const_iterator itrWordMapEnd = g_mapWordList.end();

		QString strCurWord = m_lstWords.at(ndx);
		std::size_t nPreRegExp = strCurWord.toStdString().find_first_of("*?[]");
		if (nPreRegExp == std::string::npos) {
			if ((ndx == (m_lstWords.size()-1)) &&
				(g_mapWordList.find(m_lstWords.at(ndx).toLower()) == g_mapWordList.end())) {
				nPreRegExp = strCurWord.size();
				strCurWord += "*";			// If we're on the word currently being typed and it's not an exact match, simulate a "*" trailing wildcard to match all strings with this prefix
			}
		}
		if (nPreRegExp == std::string::npos) {
			itrWordMap = g_mapWordList.find(m_lstWords.at(ndx).toLower());
			if (itrWordMap==g_mapWordList.end()) {
				if (m_nCursorWord > ndx) {
					// If we've stopped matching before the cursor, we're done:
					m_lstMatchMapping.clear();
					m_lstMapping.clear();
					m_lstNextWords.clear();
					break;			// If we've stopped matching before the cursor, we're done
				}
			} else {
				itrWordMapEnd = itrWordMap;
				itrWordMapEnd++;
			}
		} else {
			QString strPreRegExp;
			strPreRegExp = strCurWord.toLower().left(nPreRegExp);
			if (!strPreRegExp.isEmpty()) {
				itrWordMap = g_mapWordList.lower_bound(strPreRegExp);
				for (itrWordMapEnd = itrWordMap; itrWordMapEnd != g_mapWordList.end(); ++itrWordMapEnd) {
					if (!itrWordMapEnd->first.startsWith(strPreRegExp)) break;
				}
			} else {
				itrWordMap = g_mapWordList.begin();
				itrWordMapEnd = g_mapWordList.end();
			}
		}
		bool bMatch = false;
		for (/* itrWordMap Set Above */; itrWordMap != itrWordMapEnd; ++itrWordMap) {
			QRegExp expNC(strCurWord, Qt::CaseInsensitive, QRegExp::Wildcard);
			if (expNC.exactMatch(itrWordMap->first)) {
				if (!isCaseSensitive()) {
					bMatch = true;
					if (m_nLevel == 0) {
						m_lstMatchMapping.insert(m_lstMatchMapping.end(), itrWordMap->second.m_ndxNormalized.begin(), itrWordMap->second.m_ndxNormalized.end());
					} else {
						break;		// If we aren't adding more indices, once we get a match, we are done...
					}
				} else {
					QRegExp expCase(strCurWord, Qt::CaseSensitive, QRegExp::Wildcard);
					const CWordEntry &wordEntry = itrWordMap->second;		// Entry for current word
					unsigned int nCount = 0;
					for (int ndxAltWord = 0; ndxAltWord<wordEntry.m_lstAltWords.size(); ++ndxAltWord) {
						if (expCase.exactMatch(wordEntry.m_lstAltWords.at(ndxAltWord))) {
							bMatch = true;
							if (m_nLevel == 0) {
								m_lstMatchMapping.insert(m_lstMatchMapping.end(),
												&wordEntry.m_ndxNormalized[nCount],
												&wordEntry.m_ndxNormalized[nCount+wordEntry.m_lstAltWordCount.at(ndxAltWord)]);
							} else {
								break;		// If we aren't adding more indices, once we get a match, we are done...
							}
						}
						nCount += wordEntry.m_lstAltWordCount.at(ndxAltWord);
					}
					if (bMatch && (m_nLevel != 0)) break;		// If we aren't adding more indices, stop once we get a match
				}
			}
		}
		if (!bMatch) m_lstMatchMapping.clear();



//		itrWordMap = g_mapWordList.find(m_lstWords.at(ndx));
//		if (itrWordMap==g_mapWordList.end()) itrWordMap = g_mapWordList.find(m_lstWords.at(ndx).toLower());
//		if (itrWordMap==g_mapWordList.end()) {
//			if (m_nCursorWord > ndx) {
//				// If we've stopped matching before the cursor, we're done:
//				m_lstMatchMapping.clear();
//				m_lstMapping.clear();
//				m_lstNextWords.clear();
//			} else if (m_nCursorWord == ndx) {
//				// At the cursor, see if the word at the cursor starts with something in
//				//	our list.  If so, we'll bump our CursorLevel, before we bailout.  That
//				//	will signify that we're matching through the cursor, but haven't matched
//				//	full words yet to this point.  In any case, we still need the word list
//				//	to return so that we can show completions for this word and beyond:
//				QRegExp exp(m_lstWords[ndx]+"*", (isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::Wildcard);
//				for (int ndxWord=0; ndxWord<m_lstNextWords.size(); ++ndxWord) {
//					if (exp.exactMatch(m_lstNextWords.at(ndxWord))) {
//						m_nCursorLevel++;
//						break;
//					}
//				}
//			}
//			break;		// If we can't find this word, break out at this level and stop searching
//		}

//		const CWordEntry &wordEntry = itrWordMap->second;		// Entry for current word
//		if (m_nLevel == 0) {
//			// If this is our first word, set its mapping to all possible next words:
//			m_lstMatchMapping = wordEntry.m_ndxNormalized;
//		} else {
		if (m_nLevel > 0) {
			// Otherwise, match this word from our list from the last mapping and populate
			//		a list of remaining mappings:
			TIndexList lstNextMapping;
			QRegExp exp(m_lstWords[ndx], (isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::Wildcard);
			for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
//				if (((m_lstMatchMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) &&
//					(m_lstWords[ndx].compare(g_lstConcordanceWords[g_lstConcordanceMapping[m_lstMatchMapping[ndxWord]+1]-1], (isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive)) == 0)) {
				if (((m_lstMatchMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) &&
					(exp.exactMatch(g_lstConcordanceWords[g_lstConcordanceMapping[m_lstMatchMapping[ndxWord]+1]-1]))) {
					lstNextMapping.push_back(m_lstMatchMapping[ndxWord]+1);
				}
			}
			m_lstMatchMapping = lstNextMapping;
		}

		if (m_lstMatchMapping.size()) m_nLevel++;
//		if (bMatch) m_nLevel++;

		if (ndx < m_nCursorWord) {
			m_lstMapping = m_lstMatchMapping;		// Mapping for the current word possibilities is calculated at the word right before it
			m_nCursorLevel = m_nLevel;

			if ((ndx+1) == m_nCursorWord) {			// Only build list of next words if we are at the last word before the cursor
				m_lstNextWords.clear();
				for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
					if ((m_lstMatchMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) {
						m_lstNextWords.push_back(g_lstConcordanceWords[g_lstConcordanceMapping[m_lstMatchMapping[ndxWord]+1]-1]);
					}
				}
				m_lstNextWords.removeDuplicates();
				m_lstNextWords.sort();
			}
		}

		if (m_lstMatchMapping.size() == 0) break;
	}
}

// ============================================================================

CPhraseCursor::CPhraseCursor(const QTextCursor &aCursor)
	:	QTextCursor(aCursor)
{
}

CPhraseCursor::~CPhraseCursor()
{
}

bool CPhraseCursor::moveCursorCharLeft(MoveMode mode)
{
	return movePosition(PreviousCharacter, mode);
}

bool CPhraseCursor::moveCursorCharRight(MoveMode mode)
{
	return movePosition(NextCharacter, mode);
}

QChar CPhraseCursor::charUnderCursor()
{
	int nSelStart = anchor();
	int nSelEnd = position();
	clearSelection();
	movePosition(NextCharacter, KeepAnchor);
	QString strTemp = selectedText();
	setPosition(nSelStart, MoveAnchor);
	setPosition(nSelEnd, KeepAnchor);
	return ((strTemp.size()>0) ? strTemp[0] : QChar());
}

bool CPhraseCursor::moveCursorWordLeft(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word
	moveCursorCharLeft(mode);
	// If we are inside the "current word under the cursor", move left past it:
	while (!charUnderCursor().isSpace()) {
		if (!moveCursorCharLeft(mode)) return false;
	}
	// We should now be between words, move left until we hit previous word:
	while (charUnderCursor().isSpace()) {
		if (!moveCursorCharLeft(mode)) return false;
	}
	// While in previous word, keep moving:
	while (!charUnderCursor().isSpace()) {
		if (!moveCursorCharLeft(mode)) return true;		// If we hit the left edge, we have the final left word
	}
	// Here, we went one character too far.  So move back one:
	moveCursorCharRight(mode);
	return true;
}

bool CPhraseCursor::moveCursorWordRight(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word
	moveCursorCharLeft(mode);
	// If we are in the space between words, move right past it:
	while (charUnderCursor().isSpace()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	// If we are inside the "current word under the cursor", move right past it:
	while (!charUnderCursor().isSpace()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	// We should now be between word, move right until we hit next word:
	while (charUnderCursor().isSpace()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	return true;
}

bool CPhraseCursor::moveCursorWordStart(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word
	moveCursorCharLeft(mode);
	// If we're between words, move right until we get to the start of the word.
	//		Otherwise we're already somewhere inside the current word:
	while (charUnderCursor().isSpace()) {
		if (!moveCursorCharRight(mode)) return false;	// Yes, move to right as current word is the one on the righthand side
	}
	// We should now be inside the current word, move left until we find the left side:
	while (!charUnderCursor().isSpace()) {
		if (!moveCursorCharLeft(mode)) return true;		// If we hit the left edge, we are at start of word already
	}
	// Here, we went one character too far.  So move back one:
	moveCursorCharRight(mode);
	return true;
}

bool CPhraseCursor::moveCursorWordEnd(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word already
	moveCursorCharLeft(mode);
	// If we're between words, the current word is to the right.  So move through
	//	the space until we find the word.  Otherwise, we should already be in
	//	the current word:
	while (charUnderCursor().isSpace()) {
		if (!moveCursorCharRight(mode)) return false;	// Move right here (not opposite of WordStart above), as current word is the one on the righthand side
	}
	// We're now inside the current word, move right until we hit the end:
	while (!charUnderCursor().isSpace()) {
		if (!moveCursorCharRight(mode)) return true;	// If we hit the right edge, we are at the end of the word
	}
	return true;
}

QString CPhraseCursor::wordUnderCursor()
{
	QString strRetVal;
	int nSelStart = anchor();
	int nSelEnd = position();
	clearSelection();

	// Return empty string if between words:
//	if (charUnderCursor().isSpace()) {
//		moveCursorCharLeft(MoveAnchor);
//		if (charUnderCursor().isSpace()) {
//			setPosition(nSelStart, MoveAnchor);
//			setPosition(nSelEnd, KeepAnchor);
//			return QString();
//		}
//		moveCursorCharRight(MoveAnchor);
//	}

	// Find and return word we're on or just to our right:
	if (moveCursorWordStart(MoveAnchor)) {
		if (moveCursorWordEnd(KeepAnchor)) {
			strRetVal = selectedText();
		}
	}
	setPosition(nSelStart, MoveAnchor);
	setPosition(nSelEnd, KeepAnchor);
	return strRetVal;
}

void CPhraseCursor::selectWordUnderCursor()
{
	moveCursorWordStart(MoveAnchor);
	moveCursorWordEnd(KeepAnchor);
}

void CPhraseCursor::selectCursorToLineStart()
{
	clearSelection();
	movePosition(StartOfLine, KeepAnchor);
}

void CPhraseCursor::selectCursorToLineEnd()
{
	clearSelection();
	movePosition(EndOfLine, KeepAnchor);
}

// ============================================================================

CPhraseLineEdit::CPhraseLineEdit(QWidget *pParent)
	:	QTextEdit(pParent),
		m_pCompleter(NULL),
		m_pCommonPhrasesCompleter(NULL),
		m_nLastCursorWord(-1),
		m_bUpdateInProgress(false),
		m_icoDroplist(":/res/droplist.png"),
		m_pButtonDroplist(NULL)
{
	setAcceptRichText(false);

	QStringListModel *pModel = new QStringListModel(g_lstConcordanceWords);
	m_pCompleter = new QCompleter(pModel, this);
	m_pCompleter->setWidget(this);
	m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
//	m_pCompleter->setCompletionMode(QCompleter::InlineCompletion);
	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);

	m_pButtonDroplist = new QPushButton(m_icoDroplist, QString(), this);
	m_pButtonDroplist->setFlat(true);
	m_pButtonDroplist->setGeometry(sizeHint().width()-m_pButtonDroplist->sizeHint().width(),0,
								m_pButtonDroplist->sizeHint().width(),m_pButtonDroplist->sizeHint().height());

	CPhraseList phrases = g_lstCommonPhrases;
	phrases.append(g_lstUserPhrases);
	phrases.removeDuplicates();
	CPhraseListModel *pCommonPhrasesModel = new CPhraseListModel(phrases);
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
/*
	QTextCursor cursor = textCursor();
	int extra = completion.length() - m_pCompleter->completionPrefix().length();
	cursor.movePosition(QTextCursor::Left);
	cursor.movePosition(QTextCursor::EndOfWord);
	cursor.insertText(completion.right(extra));
	setTextCursor(cursor);
*/

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

	int nSelStart = cursor.anchor();
	int nSelEnd = cursor.position();
	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.setCharFormat(fmt);
	cursor.setPosition(nSelStart, QTextCursor::MoveAnchor);
	cursor.setPosition(nSelEnd, QTextCursor::KeepAnchor);
	setTextCursor(cursor);

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

	cursor.setPosition(nSelStart, QTextCursor::MoveAnchor);
	cursor.setPosition(nSelEnd, QTextCursor::KeepAnchor);
	setTextCursor(cursor);

	m_bUpdateInProgress = false;
}

void CPhraseLineEdit::ParsePhrase(const QTextCursor &curInsert)
{
	// TODO : Remove this function after done debugging!

	CParsedPhrase::ParsePhrase(curInsert);

	QStatusBar *pStatusBar = ((CKJVSearchPhraseEdit *)parent())->pStatusBar;

	QString strTemp;
	for (int n=0; n<m_lstWords.size(); ++n) {
		if (n==m_nCursorWord) strTemp += "(";
		strTemp += m_lstWords[n];
		if (n==m_nCursorWord) strTemp += ")";
		strTemp += " ";
	}
	strTemp += QString("  Cursor: %1  CursorLevel: %2  Level: %3  Words: %4").arg(m_nCursorWord).arg(m_nCursorLevel).arg(m_nLevel).arg(m_lstWords.size());
	pStatusBar->showMessage(strTemp);
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


////	const QString completionPrefix = textUnderCursor();
//	std::pair<QStringList, int> parseWords = ParsePhrase();
//	assert(parseWords.second < parseWords.first.size());
//	const QString completionPrefix = parseWords.first.at(parseWords.second);

//	if (completionPrefix != m_pCompleter->completionPrefix()) {
//		m_pCompleter->setCompletionPrefix(completionPrefix);
//		UpdateCompleter();
//		m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index( 0, 0 ));
//	}

////	if (!event->text().isEmpty() /* && completionPrefix.length() > 2 */)
//	if ((!event->text().isEmpty()) && ((completionPrefix.length() > 0) || (textCursor().atEnd())))
//		m_pCompleter->complete();

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


void CPhraseLineEdit::resizeEvent(QResizeEvent *event)
{
	m_pButtonDroplist->move(width()-m_pButtonDroplist->width(),0);
}

void CPhraseLineEdit::on_dropCommonPhrasesClicked()
{
//	CPhraseCursor cursor(textCursor());
//
//	cursor.clearSelection();
//	cursor.select(QTextCursor::LineUnderCursor);
//
//	QString strPhrase = cursor.selectedText();
//
//	m_pCommonPhrasesCompleter->setCompletionPrefix(strPhrase);

	m_pCommonPhrasesCompleter->complete();
}


// ============================================================================

CKJVSearchPhraseEdit::CKJVSearchPhraseEdit(QWidget *parent) :
	QWidget(parent),
pStatusBar(NULL),
	m_bUpdateInProgress(false),
	ui(new Ui::CKJVSearchPhraseEdit)
{
	ui->setupUi(this);

	ui->chkCaseSensitive->setChecked(ui->editPhrase->isCaseSensitive());
	ui->buttonAddPhrase->setEnabled(false);
	ui->buttonDelPhrase->setEnabled(false);
	ui->buttonClear->setEnabled(false);

	connect(ui->editPhrase, SIGNAL(phraseChanged(const CParsedPhrase &)), this, SLOT(on_phraseChanged(const CParsedPhrase &)));
	connect(ui->chkCaseSensitive, SIGNAL(clicked(bool)), this, SLOT(on_CaseSensitiveChanged(bool)));
	connect(ui->editPhrase, SIGNAL(changeCaseSensitive(bool)), this, SLOT(on_CaseSensitiveChanged(bool)));
	connect(ui->buttonAddPhrase, SIGNAL(clicked()), this, SLOT(on_phraseAdd()));
	connect(ui->buttonDelPhrase, SIGNAL(clicked()), this, SLOT(on_phraseDel()));
	connect(ui->buttonClear, SIGNAL(clicked()), this, SLOT(on_phraseClear()));
	connect(this, SIGNAL(phraseListChanged()), ui->editPhrase, SLOT(on_phraseListChanged()));
}

CKJVSearchPhraseEdit::~CKJVSearchPhraseEdit()
{
	delete ui;
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

