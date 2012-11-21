#include "PhraseEdit.h"

#include <QStringListModel>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>

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

	CPhraseCursor curLeft(curInsert);
	while (curLeft.moveCursorWordLeft()) {
		m_lstLeftWords.push_front(curLeft.wordUnderCursor());
	}

	CPhraseCursor curRight(curInsert);
	m_strCursorWord = curRight.wordUnderCursor();
	while (curRight.moveCursorWordRight()) {
		m_lstRightWords.push_back(curRight.wordUnderCursor());
	}

	m_lstWords.clear();
	m_lstWords.append(m_lstLeftWords);
	m_nCursorWord = m_lstWords.size();
	m_lstWords.append(m_strCursorWord);
	m_lstWords.append(m_lstRightWords);

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

		if (m_nLevel > 0) {
			// Otherwise, match this word from our list from the last mapping and populate
			//		a list of remaining mappings:
			TIndexList lstNextMapping;
			QRegExp exp(m_lstWords[ndx], (isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::Wildcard);
			for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
				if (((m_lstMatchMapping[ndxWord]+1) < g_lstConcordanceMapping.size()) &&
					(exp.exactMatch(g_lstConcordanceWords[g_lstConcordanceMapping[m_lstMatchMapping[ndxWord]+1]-1]))) {
					lstNextMapping.push_back(m_lstMatchMapping[ndxWord]+1);
				}
			}
			m_lstMatchMapping = lstNextMapping;
		}

		if (m_lstMatchMapping.size()) m_nLevel++;

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

int CPhraseNavigator::anchorPosition(const QString &strAnchorName) const
{
	if (strAnchorName.isEmpty()) return -1;

	for (QTextBlock block = m_TextEditor.document()->begin(); block.isValid(); block = block.next()) {
		QTextCharFormat format = block.charFormat();
		if (format.isAnchor()) {
			if (format.anchorNames().contains(strAnchorName)) {
				int nPos = block.position();
				QString strText = block.text();
				for (int nPosStr = 0; nPosStr < strText.length(); ++nPosStr) {
					if (strText[nPosStr].isSpace()) {
						nPos++;
					} else {
						break;
					}
				}
				return nPos;
			}
		}
		for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it) {
			QTextFragment fragment = it.fragment();
			format = fragment.charFormat();
			if (format.isAnchor()) {
				if (format.anchorNames().contains(strAnchorName)) {
					int nPos = fragment.position();
					QString strText = fragment.text();
					for (int nPosStr = 0; nPosStr < strText.length(); ++nPosStr) {
						if (strText[nPosStr].isSpace()) {
							nPos++;
						} else {
							break;
						}
					}
					return nPos;
				}
			}
		}
	}

	return -1;
}

CRelIndex CPhraseNavigator::ResolveCursorReference(CPhraseCursor cursor)
{
	CRelIndex ndxReference = ResolveCursorReference2(cursor);

	if (ndxReference.book() != 0) {
		assert(ndxReference.book() <= g_lstTOC.size());
		if (ndxReference.book() <= g_lstTOC.size()) {
			if (ndxReference.chapter() != 0) {
				assert(ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp);
				if (ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp) {
					if (ndxReference.verse() != 0) {
						assert(ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs);
						if (ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs) {
							if (ndxReference.word() > (g_lstBooks[ndxReference.book()-1])[CRelIndex(0, ndxReference.chapter(), ndxReference.verse(), 0)].m_nNumWrd) {
								// Clip word index at max since it's possible to be on the space
								//		between words and have an index that is one larger than
								//		our largest word:
								ndxReference.setWord((g_lstBooks[ndxReference.book()-1])[CRelIndex(0, ndxReference.chapter(), ndxReference.verse(), 0)].m_nNumWrd);
							}
						}
					}
				}
			}
		}
	}

	return ndxReference;
}

CRelIndex CPhraseNavigator::ResolveCursorReference2(CPhraseCursor cursor)
{

#define CheckForAnchor() {											\
	if (cursor.charFormat().anchorName().startsWith('B')) {			\
		bInABanchor = true;											\
		bBanchorFound = true;										\
	} else if (cursor.charFormat().anchorName().startsWith('A')) {	\
		if (!bBanchorFound) nWord = 0;								\
		bInABanchor = false;										\
		bBanchorFound = false;										\
	} else {														\
		ndxReference = CRelIndex(cursor.charFormat().anchorName());	\
		if (ndxReference.isSet()) {									\
			if ((ndxReference.verse() != 0) &&						\
				(ndxReference.word() == 0)) {						\
				ndxReference.setWord(nWord);						\
			}														\
			return ndxReference;									\
		}															\
	}																\
}

	CRelIndex ndxReference;
	unsigned int nWord = 0;
	bool bInABanchor = false;		// Set to true if we are inside an A-B anchor set where we should ignore word counts
	bool bBanchorFound = false;		// Set to true if we encounter a B anchor, used when we get to A anchor to know if we should clear nWord

	CheckForAnchor();
	while (!cursor.charUnderCursor().isSpace()) {
		if (!cursor.moveCursorCharLeft(QTextCursor::MoveAnchor)) return ndxReference;
		CheckForAnchor();
	}

	do {
		if (!bInABanchor) nWord++;

		while (cursor.charUnderCursor().isSpace()) {
			if (!cursor.moveCursorCharLeft(QTextCursor::MoveAnchor)) return ndxReference;
			CheckForAnchor();
		}

		while (!cursor.charUnderCursor().isSpace()) {
			if (!cursor.moveCursorCharLeft(QTextCursor::MoveAnchor)) return ndxReference;
			CheckForAnchor();
		}
	} while (1);

	return ndxReference;
}

void CPhraseNavigator::doHighlighting(const TPhraseTagList &lstPhraseTags, const QColor &colorHighlight, bool bClear, const CRelIndex &ndxCurrent)
{
	for (int ndx=0; ndx<lstPhraseTags.size(); ++ndx) {
		CRelIndex ndxRel = lstPhraseTags.at(ndx).first;
		if (!ndxRel.isSet()) continue;
		// Save some time if the tag isn't anything close to what we are displaying.
		//		We'll use one before/one after since we might be displaying part of
		//		the proceding passage:
		if  ((ndxCurrent.isSet()) &&
				((ndxRel.book() < (ndxCurrent.book()-1)) ||
				 (ndxRel.book() > (ndxCurrent.book()+1)) ||
				 (ndxRel.chapter() < (ndxCurrent.chapter()-1)) ||
				 (ndxRel.chapter() > (ndxCurrent.chapter()+1)))) continue;
		uint32_t ndxWord = ndxRel.word();
		ndxRel.setWord(0);
		int nPos = anchorPosition(ndxRel.asAnchor());
		if (nPos == -1) continue;
		CPhraseCursor myCursor(m_TextEditor.textCursor());
		myCursor.setPosition(nPos);
		while (ndxWord) {
			myCursor.selectWordUnderCursor();
			myCursor.moveCursorWordRight();
			ndxWord--;
		}
		unsigned int nCount = lstPhraseTags.at(ndx).second;
		while (nCount) {
			QTextCharFormat fmt = myCursor.charFormat();
			QString strAnchorName = fmt.anchorName();
			if ((!fmt.isAnchor()) || (strAnchorName.startsWith('B'))) {		// Either we shouldn't be in an anchor or the end of an A-B special section marker
				myCursor.selectWordUnderCursor();
				fmt = myCursor.charFormat();
				if (!bClear) {
//					fmt.setUnderlineColor(m_colorHighlight);
//					fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
					// Save current brush in UserProperty so we can restore it later in undoHighlighting:
					fmt.setProperty(QTextFormat::UserProperty, QVariant(fmt.foreground()));
					fmt.setForeground(QBrush(colorHighlight));
				} else {
//					fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);
					// Restore preserved brush to restore text:
					fmt.setForeground(fmt.property(QTextFormat::UserProperty).value<QBrush>());
				}
				myCursor.setCharFormat(fmt);
				nCount--;
				if (!myCursor.moveCursorWordRight()) break;
			} else {
				// If we hit an anchor, see if it's either a special section A-B marker or if
				//		it's a chapter start anchor.  If it's an A-anchor, find the B-anchor.
				//		If it is a chapter start anchor, search for our special X-anchor so
				//		we'll be at the correct start of the next verse:
				if (strAnchorName.startsWith('A')) {
					int nEndAnchorPos = anchorPosition("B" + strAnchorName.mid(1));
					if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
				} else {
					CRelIndex ndxAnchor(strAnchorName);
					assert(ndxAnchor.isSet());
					if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
						int nEndAnchorPos = anchorPosition("X" + fmt.anchorName());
						if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
					}
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
	}
}

// ============================================================================

