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

#include "PhraseEdit.h"
#include "ToolTipEdit.h"

#include <QStringListModel>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>
#include <QToolTip>

#include <QRegExp>

#include <algorithm>
#include <string>

#include <assert.h>

// ============================================================================

bool CPhraseEntry::operator==(const CParsedPhrase &src) const
{
	return ((m_bCaseSensitive == src.isCaseSensitive()) &&
			(m_strPhrase.compare(src.phrase(), Qt::CaseSensitive) == 0));
}

uint32_t CParsedPhrase::GetNumberOfMatches() const
{
	return m_lstMatchMapping.size();
}

const TIndexList &CParsedPhrase::GetNormalizedSearchResults() const
{
	if (m_cache_lstNormalizedSearchResults.size()) return m_cache_lstNormalizedSearchResults;

	m_cache_lstNormalizedSearchResults.resize(m_lstMatchMapping.size());
	for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
		m_cache_lstNormalizedSearchResults[ndxWord] = m_lstMatchMapping.at(ndxWord) - m_nLevel + 1;
	}
	sort(m_cache_lstNormalizedSearchResults.begin(), m_cache_lstNormalizedSearchResults.end());

	return m_cache_lstNormalizedSearchResults;
}

const TPhraseTagList &CParsedPhrase::GetPhraseTagSearchResults() const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_cache_lstPhraseTagResults.size()) return m_cache_lstPhraseTagResults;

	m_cache_lstPhraseTagResults.clear();		// This call really shouldn't be needed since we already know the size is zero (above), but it just feels better with it. :-)
	const TIndexList &lstPhraseResults(GetNormalizedSearchResults());
	for (unsigned int ndxResults=0; ndxResults<lstPhraseResults.size(); ++ndxResults) {
		m_cache_lstPhraseTagResults.append(TPhraseTag(CRelIndex(m_pBibleDatabase->DenormalizeIndex(lstPhraseResults.at(ndxResults))), phraseSize()));
	}

	return m_cache_lstPhraseTagResults;
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
	return phraseWords().join(" ");
}

QString CParsedPhrase::phraseRaw() const
{
	return phraseWordsRaw().join(" ");
}

unsigned int CParsedPhrase::phraseSize() const
{
	return phraseWords().size();
}

unsigned int CParsedPhrase::phraseRawSize() const
{
	return phraseWordsRaw().size();
}

QStringList CParsedPhrase::phraseWords() const
{
	if (m_cache_lstPhraseWords.size()) return m_cache_lstPhraseWords;

	m_cache_lstPhraseWords = m_lstWords;
	for (int ndx = (m_cache_lstPhraseWords.size()-1); ndx >= 0; --ndx) {
		if (m_cache_lstPhraseWords.at(ndx).isEmpty()) m_cache_lstPhraseWords.removeAt(ndx);
	}
	return m_cache_lstPhraseWords;
}

QStringList CParsedPhrase::phraseWordsRaw() const
{
	if (m_cache_lstPhraseWordsRaw.size()) return m_cache_lstPhraseWordsRaw;

	m_cache_lstPhraseWordsRaw = phraseWords();
	for (int ndx = (m_cache_lstPhraseWordsRaw.size()-1); ndx >= 0; --ndx) {
		QString strTemp = makeRawPhrase(m_cache_lstPhraseWordsRaw.at(ndx));
		if (strTemp.isEmpty()) {
			m_cache_lstPhraseWordsRaw.removeAt(ndx);
		} else {
			m_cache_lstPhraseWordsRaw[ndx] = strTemp;
		}
	}
	return m_cache_lstPhraseWordsRaw;
}

QString CParsedPhrase::makeRawPhrase(const QString &strPhrase)
{
	const QString strValidChars(" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-");
	QString strTemp = strPhrase;
	for (int i = (strTemp.size()-1); i >= 0; --i) {
		if (!strValidChars.contains(strTemp.at(i))) strTemp.remove(i, 1);
	}
	return strTemp;
}

void CParsedPhrase::clearCache() const
{
	m_cache_lstPhraseWords.clear();
	m_cache_lstPhraseWordsRaw.clear();
	m_cache_lstNormalizedSearchResults.clear();
	m_cache_lstPhraseTagResults.clear();
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
	CPhraseCursor myCursor(curInsert);
	myCursor.beginEditBlock();
	myCursor.clearSelection();
	myCursor.selectWordUnderCursor();							// Select word under the cursor
	myCursor.insertText(completion);							// Replace with completed word
	myCursor.endEditBlock();

	return myCursor;
}

void CParsedPhrase::ParsePhrase(const QTextCursor &curInsert)
{
	clearCache();

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
	clearCache();

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
	assert(m_pBibleDatabase.data() != NULL);

	assert(m_nCursorWord < m_lstWords.size());

	m_lstMatchMapping.clear();
	m_lstMapping.clear();
	m_lstNextWords = m_pBibleDatabase->concordanceWordList();
	m_nLevel = 0;
	m_nCursorLevel = 0;
	for (int ndx=0; ndx<m_lstWords.size(); ++ndx) {
		if (m_lstWords.at(ndx).isEmpty()) continue;

		TWordListMap::const_iterator itrWordMap;
		TWordListMap::const_iterator itrWordMapEnd = m_pBibleDatabase->mapWordList().end();

		QString strCurWord = m_lstWords.at(ndx);			// Note: This becomes the "Word*" value later, so can't substitute strCurWord for all m_lstWords.at(ndx)
		std::size_t nPreRegExp = strCurWord.toStdString().find_first_of("*?[]");
		if (nPreRegExp == std::string::npos) {
			if ((ndx == (m_lstWords.size()-1)) &&
				(m_pBibleDatabase->mapWordList().find(m_lstWords.at(ndx).toLower()) == m_pBibleDatabase->mapWordList().end())) {
				nPreRegExp = strCurWord.size();
				strCurWord += "*";			// If we're on the word currently being typed and it's not an exact match, simulate a "*" trailing wildcard to match all strings with this prefix
			}
		}
		if (nPreRegExp == std::string::npos) {
			itrWordMap = m_pBibleDatabase->mapWordList().find(m_lstWords.at(ndx).toLower());
			if (itrWordMap==m_pBibleDatabase->mapWordList().end()) {
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
				itrWordMap = m_pBibleDatabase->mapWordList().lower_bound(strPreRegExp);
				for (itrWordMapEnd = itrWordMap; itrWordMapEnd != m_pBibleDatabase->mapWordList().end(); ++itrWordMapEnd) {
					if (!itrWordMapEnd->first.startsWith(strPreRegExp)) break;
				}
			} else {
				itrWordMap = m_pBibleDatabase->mapWordList().begin();
				itrWordMapEnd = m_pBibleDatabase->mapWordList().end();
			}
		}
		bool bMatch = false;
		bool bFirstWordExactMatch = false;
		for (/* itrWordMap Set Above */; itrWordMap != itrWordMapEnd; ++itrWordMap) {
			QRegExp expNC(strCurWord, Qt::CaseInsensitive, QRegExp::Wildcard);
			if (expNC.exactMatch(itrWordMap->first)) {
				if (!isCaseSensitive()) {
					bMatch = true;
					if (ndx == 0) {
						m_lstMatchMapping.insert(m_lstMatchMapping.end(), itrWordMap->second.m_ndxNormalizedMapping.begin(), itrWordMap->second.m_ndxNormalizedMapping.end());
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
							if (ndx == 0) {
								m_lstMatchMapping.insert(m_lstMatchMapping.end(),
												&wordEntry.m_ndxNormalizedMapping[nCount],
												&wordEntry.m_ndxNormalizedMapping[nCount+wordEntry.m_lstAltWordCount.at(ndxAltWord)]);
							} else {
								break;		// If we aren't adding more indices, once we get a match, we are done...
							}
						}
						nCount += wordEntry.m_lstAltWordCount.at(ndxAltWord);
					}
					if (bMatch && (ndx != 0)) break;		// If we aren't adding more indices, stop once we get a match
				}
			}
			if (ndx == 0) {
				if (isCaseSensitive()) {
					QRegExp exp(m_lstWords[ndx], Qt::CaseSensitive, QRegExp::Wildcard);
					const CWordEntry &wordEntry = itrWordMap->second;		// Entry for current word
					for (int ndxAltWord = 0; ndxAltWord<wordEntry.m_lstAltWords.size(); ++ndxAltWord) {
						if (exp.exactMatch(wordEntry.m_lstAltWords.at(ndxAltWord))) {
							bFirstWordExactMatch = true;
							break;
						}
					}
				} else {
					QRegExp exp(m_lstWords[ndx], Qt::CaseInsensitive, QRegExp::Wildcard);
					if (exp.exactMatch(itrWordMap->first)) bFirstWordExactMatch = true;
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
				if (((m_lstMatchMapping[ndxWord]+1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) &&
					(exp.exactMatch(m_pBibleDatabase->wordAtIndex(m_lstMatchMapping[ndxWord]+1)))) {
					lstNextMapping.push_back(m_lstMatchMapping[ndxWord]+1);
				}
			}
			m_lstMatchMapping = lstNextMapping;
		}

		if (((m_lstMatchMapping.size() != 0) && (ndx > 0)) ||
			((bFirstWordExactMatch) && (ndx == 0))) m_nLevel++;

		if (ndx < m_nCursorWord) {
			m_lstMapping = m_lstMatchMapping;		// Mapping for the current word possibilities is calculated at the word right before it
			m_nCursorLevel = m_nLevel;

			if ((ndx+1) == m_nCursorWord) {			// Only build list of next words if we are at the last word before the cursor
				m_lstNextWords.clear();
				for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
					if ((m_lstMatchMapping[ndxWord]+1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) {
						m_lstNextWords.push_back(m_pBibleDatabase->wordAtIndex(m_lstMatchMapping[ndxWord]+1));
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

CPhraseCursor::CPhraseCursor(const CPhraseCursor &aCursor)
	:	QTextCursor(aCursor)
{
}

CPhraseCursor::CPhraseCursor(QTextDocument *pDocument)
	:	QTextCursor(pDocument)
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
	assert(m_pBibleDatabase.data() != NULL);

	if (strAnchorName.isEmpty()) return -1;

	for (QTextBlock block = m_TextDocument.begin(); block.isValid(); block = block.next()) {
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

CRelIndex CPhraseNavigator::ResolveCursorReference(CPhraseCursor cursor) const
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndxReference = ResolveCursorReference2(cursor);

	if (ndxReference.book() != 0) {
		assert(ndxReference.book() <= m_pBibleDatabase->bibleEntry().m_nNumBk);
		if (ndxReference.book() <= m_pBibleDatabase->bibleEntry().m_nNumBk) {
			if (ndxReference.chapter() != 0) {
				assert(ndxReference.chapter() <= m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp);
				if (ndxReference.chapter() <= m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) {
					if (ndxReference.verse() != 0) {
						assert(ndxReference.verse() <= m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs);
						if (ndxReference.verse() <= m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) {
							if (ndxReference.word() > m_pBibleDatabase->verseEntry(ndxReference)->m_nNumWrd) {
								// Clip word index at max since it's possible to be on the space
								//		between words and have an index that is one larger than
								//		our largest word:
								ndxReference.setWord(m_pBibleDatabase->verseEntry(ndxReference)->m_nNumWrd);
							}
						}
					}
				}
			}
		}
	}

	return ndxReference;
}

CRelIndex CPhraseNavigator::ResolveCursorReference2(CPhraseCursor cursor) const
{
	assert(m_pBibleDatabase.data() != NULL);

#define CheckForAnchor() {											\
	if (cursor.charFormat().anchorName().startsWith('B')) {			\
		++nWord;													\
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
		if ((!bInABanchor) && (!CParsedPhrase::makeRawPhrase(cursor.wordUnderCursor()).isEmpty())) {
			nWord++;
		}

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

void CPhraseNavigator::doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear, const CRelIndex &ndxCurrent) const
{
	assert(m_pBibleDatabase.data() != NULL);

	const TPhraseTagList &lstPhraseTags(aHighlighter.getHighlightTags());
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
		CPhraseCursor myCursor(&m_TextDocument);
		myCursor.beginEditBlock();
		myCursor.setPosition(nPos);
		int nSelEnd = nPos;
		while (ndxWord) {
			QTextCharFormat fmt = myCursor.charFormat();
			QString strAnchorName = fmt.anchorName();
			if ((!fmt.isAnchor()) || (strAnchorName.startsWith('B'))) {		// Either we shouldn't be in an anchor or the end of an A-B special section marker
				if (!CParsedPhrase::makeRawPhrase(myCursor.wordUnderCursor()).isEmpty())
					ndxWord--;
				nSelEnd = myCursor.position();
				if (!myCursor.moveCursorWordRight()) break;
			} else {
				// If we hit an anchor, see if it's either a special section A-B marker or if
				//		it's a chapter start anchor.  If it's an A-anchor, find the B-anchor.
				//		If it is a chapter start anchor, search for our special X-anchor so
				//		we'll be at the correct start of the next verse:
				if (strAnchorName.startsWith('A')) {
					int nEndAnchorPos = anchorPosition("B" + strAnchorName.mid(1));
					assert(nEndAnchorPos >= 0);
					if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
				} else {
					if (!strAnchorName.isEmpty()) {
						CRelIndex ndxAnchor(strAnchorName);
						assert(ndxAnchor.isSet());
						if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
							int nEndAnchorPos = anchorPosition("X" + strAnchorName);
							assert(nEndAnchorPos >= 0);
							if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
						}
					}
					nSelEnd = myCursor.position();
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
		myCursor.setPosition(nSelEnd);
		unsigned int nCount = lstPhraseTags.at(ndx).second;
		while (nCount) {
			QTextCharFormat fmt = myCursor.charFormat();
			QString strAnchorName = fmt.anchorName();
			if ((!fmt.isAnchor()) || (strAnchorName.startsWith('B'))) {		// Either we shouldn't be in an anchor or the end of an A-B special section marker
				myCursor.moveCursorWordStart();
				do {
					if (!myCursor.moveCursorCharRight(QTextCursor::KeepAnchor)) break;
					fmt = myCursor.charFormat();
					aHighlighter.doHighlighting(fmt, bClear);
					myCursor.setCharFormat(fmt);
					myCursor.clearSelection();
				} while (!myCursor.charUnderCursor().isSpace());
				if (!CParsedPhrase::makeRawPhrase(myCursor.wordUnderCursor()).isEmpty())
					nCount--;
				if (!myCursor.moveCursorWordRight()) break;
			} else {
				// If we hit an anchor, see if it's either a special section A-B marker or if
				//		it's a chapter start anchor.  If it's an A-anchor, find the B-anchor.
				//		If it is a chapter start anchor, search for our special X-anchor so
				//		we'll be at the correct start of the next verse:
				if (strAnchorName.startsWith('A')) {
					int nEndAnchorPos = anchorPosition("B" + strAnchorName.mid(1));
					assert(nEndAnchorPos >= 0);
					if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
				} else {
					if (!strAnchorName.isEmpty()) {
						CRelIndex ndxAnchor(strAnchorName);
						assert(ndxAnchor.isSet());
						if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
							int nEndAnchorPos = anchorPosition("X" + strAnchorName);
							assert(nEndAnchorPos >= 0);
							if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
						}
					}
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
		myCursor.endEditBlock();
	}
}

void CPhraseNavigator::setDocumentToChapter(const CRelIndex &ndx, bool bNoAnchors)
{
	assert(m_pBibleDatabase.data() != NULL);

	m_TextDocument.clear();

	if ((ndx.book() == 0) || (ndx.chapter() == 0)) return;

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndx);
	if (pChapter == NULL) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	if (ndx.chapter() > book.m_nNumChp) {
		assert(false);
		emit changedDocumentText();
		return;
	}

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n.subtitle { font-size:12pt; font-weight:normal; }\n.category { font-size:12pt; font-weight:normal; }\n</style></head><body>\n")
//						.arg(Qt::escape(ndx.PassageReferenceText()));		// Document Title

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
						.arg(Qt::escape(m_pBibleDatabase->PassageReferenceText(ndx)));		// Document Title

	uint32_t nFirstWordNormal = m_pBibleDatabase->NormalizeIndex(CRelIndex(ndx.book(), ndx.chapter(), 1, 1));		// Find normalized word number for the first verse, first word of this book/chapter
	uint32_t nNextChapterFirstWordNormal = nFirstWordNormal + pChapter->m_nNumWrd;		// Add the number of words in this chapter to get first word normal of next chapter
	uint32_t nRelPrevChapter = m_pBibleDatabase->DenormalizeIndex(nFirstWordNormal - 1);			// Find previous book/chapter/verse (and word)
	uint32_t nRelNextChapter = m_pBibleDatabase->DenormalizeIndex(nNextChapterFirstWordNormal);		// Find next book/chapter/verse (and word)

	// Print last verse of previous chapter if available:
	if (nRelPrevChapter != 0) {
		CRelIndex relPrev(nRelPrevChapter);
		const CBookEntry &bookPrev = *m_pBibleDatabase->bookEntry(relPrev.book());
		strHTML += "<p>";
		if (!bNoAnchors) {
			strHTML += QString("<a id=\"%1\"><b> %2 </b></a>").arg(CRelIndex(relPrev.book(), relPrev.chapter(), relPrev.verse(), 0).asAnchor()).arg(relPrev.verse());
		} else {
			strHTML += QString("<b> %1 </b>").arg(relPrev.verse());
		}
		strHTML += m_pBibleDatabase->verseEntry(relPrev)->text() + "\n";
		strHTML += "</p>";

		// If we have a footnote for this book and this is the end of the last chapter,
		//		print it too:
		if (relPrev.chapter() == bookPrev.m_nNumChp) {
			const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(relPrev.book(),0,0,0));
			if (pFootnote) {
				if (!bNoAnchors) {
					strHTML += QString("<p><a id=\"%1\">%2</a><a id=\"X%3\">%4</a></p>\n")
									.arg(CRelIndex(relPrev.book(),0,0,0).asAnchor())
									.arg(Qt::escape(pFootnote->text()))
									.arg(CRelIndex(relPrev.book(),0,0,0).asAnchor())
									.arg(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
				} else {
					strHTML += QString("<p>%1</p>\n")
									.arg(Qt::escape(pFootnote->text()));
				}
			}
		}
	}

	strHTML += "<hr />\n";

	// Print Heading for this Book/Chapter:
	if (!bNoAnchors) {
		CRelIndex ndxBookChap(ndx.book(), ndx.chapter(), 0, 0);
		strHTML += QString("<div class=book><a id=\"%1\">%2</a></div>\n")		// Note: No X anchor because it's coming with chapter heading below
						.arg(ndxBookChap.asAnchor())
						.arg(Qt::escape(book.m_strBkName));
		if ((!book.m_strDesc.isEmpty()) && (ndx.chapter() == 1))
			strHTML += QString("<div class=subtitle>(%1)</div>\n")
							.arg(Qt::escape(book.m_strDesc));
		if  ((!book.m_strCat.isEmpty()) && (ndx.chapter() == 1))
			strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
							.arg(Qt::escape(tr("Category:")))
							.arg(Qt::escape(book.m_strCat));
		// If we have a chapter note for this chapter, print it too:
		const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(),ndx.chapter(),0,0));
		if (pFootnote) {
			strHTML += QString("<div class=chapter>%1 %2</div>\n")
						.arg(Qt::escape(tr("Chapter")))
						.arg(ndx.chapter());
			strHTML += QString("<div class=subtitle>%1<a id=\"X%2\"> </a></div>\n")
						.arg(Qt::escape(pFootnote->text()))
						.arg(ndxBookChap.asAnchor());
		} else {
			strHTML += QString("<div class=chapter>%1 %2<a id=\"X%3\"> </a></div>\n")
						.arg(Qt::escape(tr("Chapter")))
						.arg(ndx.chapter())
						.arg(ndxBookChap.asAnchor());
		}
	} else {
		strHTML += QString("<div class=book>%1</div>\n")
						.arg(Qt::escape(book.m_strBkName));
		if ((!book.m_strDesc.isEmpty()) && (ndx.chapter() == 1))
			strHTML += QString("<div class=subtitle>(%1)</div>\n")
							.arg(Qt::escape(book.m_strDesc));
		if  ((!book.m_strCat.isEmpty()) && (ndx.chapter() == 1))
			strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
							.arg(Qt::escape(tr("Category:")))
							.arg(Qt::escape(book.m_strCat));
		strHTML += QString("<div class=chapter>%1 %2</div>\n")
						.arg(Qt::escape(tr("Chapter")))
						.arg(ndx.chapter());
		// If we have a chapter note for this chapter, print it too:
		const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(),ndx.chapter(),0,0));
		if (pFootnote) {
			strHTML += QString("<div class=subtitle>%1</div>\n")
						.arg(Qt::escape(pFootnote->text()));
		}
	}

	// Print this Chapter Text:
	bool bParagraph = false;
	CRelIndex ndxVerse;
	for (unsigned int ndxVrs=0; ndxVrs<pChapter->m_nNumVrs; ++ndxVrs) {
		ndxVerse = CRelIndex(ndx.book(), ndx.chapter(), ndxVrs+1, 0);
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(CRelIndex(ndx.book(), ndx.chapter(), ndxVrs+1,0));
		if (pVerse == NULL) {
			assert(false);
			continue;
		}
		if (pVerse->m_nPilcrow != CVerseEntry::PTE_NONE) {
			if (bParagraph) {
				strHTML += "</p>";
				bParagraph=false;
			}
		}
		if (!bParagraph) {
			strHTML += "<p>";
			bParagraph = true;
		}
		if (!bNoAnchors) {
			strHTML += QString("<a id=\"%1\"><b> %2 </b></a>")
						.arg(ndxVerse.asAnchor())
						.arg(ndxVrs+1);
		} else {
			strHTML += QString("<b> %1 </b>")
						.arg(ndxVrs+1);
		}
		strHTML += pVerse->text() + "\n";
		ndxVerse.setWord(pVerse->m_nNumWrd);		// At end of loop, ndxVerse will be index of last word we've output...
	}
	if (bParagraph) {
		strHTML += "</p>";
		bParagraph = false;
	}

	// If we have a footnote for this book and this is the end of the last chapter,
	//		print it too:
	if (ndx.chapter() == book.m_nNumChp) {
		const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(),0,0,0));
		if (pFootnote) {
			if (!bNoAnchors) {
				strHTML += QString("<p><a id=\"%1\">%2</a><a id=\"X%3\">%4</a></p>\n")
								.arg(CRelIndex(ndx.book(),0,0,0).asAnchor())
								.arg(Qt::escape(pFootnote->text()))
								.arg(CRelIndex(ndx.book(),0,0,0).asAnchor())
								.arg(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			} else {
				strHTML += QString("<p>%1</p>\n")
								.arg(Qt::escape(pFootnote->text()));
			}
		}
	}

	strHTML += "<hr />\n";

	// Print first verse of next chapter if available:
	if (nRelNextChapter != 0) {
		CRelIndex relNext(nRelNextChapter);
		CRelIndex ndxBookChap(relNext.book(), relNext.chapter(), 0, 0);
		const CBookEntry &bookNext = *m_pBibleDatabase->bookEntry(relNext.book());

		// Print Heading for this Book/Chapter:
		bool bNextChapterDifferentBook = false;
		if (relNext.book() != ndx.book()) {
			bNextChapterDifferentBook = true;
			if (!bNoAnchors) {
				strHTML += QString("<div class=book><a id=\"%1\">%2</a></div>\n")		// Note: No X anchor because it's coming with chapter heading below
								.arg(ndxBookChap.asAnchor())
								.arg(Qt::escape(bookNext.m_strBkName));
				if ((!bookNext.m_strDesc.isEmpty()) && (relNext.chapter() == 1))
					strHTML += QString("<div class=subtitle>(%1)</div>\n")
									.arg(Qt::escape(bookNext.m_strDesc));
				if  ((!bookNext.m_strCat.isEmpty()) && (relNext.chapter() == 1))
					strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
									.arg(Qt::escape(tr("Category:")))
									.arg(Qt::escape(bookNext.m_strCat));
			} else {
				strHTML += QString("<div class=book>%1</div>\n")
								.arg(Qt::escape(bookNext.m_strBkName));
				if ((!bookNext.m_strDesc.isEmpty()) && (relNext.chapter() == 1))
					strHTML += QString("<div class=subtitle>(%1)</div>\n")
									.arg(Qt::escape(bookNext.m_strDesc));
				if  ((!bookNext.m_strCat.isEmpty()) && (relNext.chapter() == 1))
					strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
									.arg(Qt::escape(tr("Category:")))
									.arg(Qt::escape(bookNext.m_strCat));
			}
		}
		if (!bNoAnchors) {
			if (bNextChapterDifferentBook) {
				// If we have a chapter note for this chapter, print it too:
				const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(relNext.book(),relNext.chapter(),0,0));
				if (pFootnote) {
					strHTML += QString("<div class=chapter>%1 %2</div>\n")
									.arg(Qt::escape(tr("Chapter")))
									.arg(relNext.chapter());
					strHTML += QString("<div class=subtitle>%1<a id=\"X%2\"> </a></div>\n")
									.arg(Qt::escape(pFootnote->text()))
									.arg(ndxBookChap.asAnchor());
				} else {
					strHTML += QString("<div class=chapter>%1 %2<a id=\"X%3\"> </a></div>\n")
									.arg(Qt::escape(tr("Chapter")))
									.arg(relNext.chapter())
									.arg(ndxBookChap.asAnchor());
				}
			} else {
				// If we have a chapter note for this chapter, print it too:
				const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(relNext.book(),relNext.chapter(),0,0));
				if (pFootnote) {
					strHTML += QString("<div class=chapter><a id=\"%1\">%2 %3</a></div>\n")
									.arg(ndxBookChap.asAnchor())
									.arg(Qt::escape(tr("Chapter")))
									.arg(relNext.chapter());
					strHTML += QString("<div class=subtitle>%1<a id=\"X%2\"> </a></div>\n")
									.arg(Qt::escape(pFootnote->text()))
									.arg(ndxBookChap.asAnchor());
				} else {
					strHTML += QString("<div class=chapter><a id=\"%1\">%2 %3</a><a id=\"X%4\"> </a></div>\n")
									.arg(ndxBookChap.asAnchor())
									.arg(Qt::escape(tr("Chapter")))
									.arg(relNext.chapter())
									.arg(ndxBookChap.asAnchor());
				}
			}
		} else {
			strHTML += QString("<div class=chapter>%1 %2</div>\n")
								.arg(Qt::escape(tr("Chapter")))
								.arg(relNext.chapter());
			// If we have a chapter note for this chapter, print it too:
			const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(relNext.book(),relNext.chapter(),0,0));
			if (pFootnote) {
				strHTML += QString("<div class=subtitle>%1</div>\n")
							.arg(Qt::escape(pFootnote->text()));
			}
		}

		strHTML += "<p>";
		if (!bNoAnchors) {
			strHTML += QString("<a id=\"%1\"><b> %2 </b></a>").arg(CRelIndex(relNext.book(), relNext.chapter(), relNext.verse(), 0).asAnchor()).arg(relNext.verse());
		} else {
			strHTML += QString("<b> %1 </b>").arg(relNext.verse());
		}
		strHTML += m_pBibleDatabase->verseEntry(relNext)->text() + "\n";
		strHTML += "</p>";
	}

	strHTML += "</body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

void CPhraseNavigator::setDocumentToVerse(const CRelIndex &ndx, bool bAddDividerLineBefore, bool bNoAnchors)
{
	assert(m_pBibleDatabase.data() != NULL);

	m_TextDocument.clear();

	if ((ndx.book() == 0) || (ndx.chapter() == 0) || (ndx.verse() == 0)) {
		emit changedDocumentText();
		return;
	}

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	if (ndx.chapter() > book.m_nNumChp) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndx);
	if (pChapter == NULL) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	if (ndx.verse() > pChapter->m_nNumVrs) {
		assert(false);
		emit changedDocumentText();
		return;
	}

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
//						.arg(Qt::escape(ndx.PassageReferenceText()));		// Document Title

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n")
	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n")
						.arg(Qt::escape(m_pBibleDatabase->PassageReferenceText(ndx)));		// Document Title

	if (bAddDividerLineBefore) strHTML += "<hr />";

	// Print Book/Chapter for this verse:
	if (!bNoAnchors) {
		//		Note: This little shenanigan is so we can have an ending "X" anchor within the name of the book
		//				itself.  This is because the chapter/verse reference anchor below must begin with
		//				a space so that we can find a dual unique anchor in our searching.  If we don't do
		//				this with the book name, we have to insert an extra space at the end for the "X" anchor
		//				and that extra space was just annoying me!!!
		QString strBook = book.m_strBkName;
		strBook = strBook.leftJustified(2, ' ', false);
		strHTML += QString("<p><a id=\"%1\"><b>%2</b></a><a id=\"X%3\"><b>%4</b></a>")
						.arg(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor())
						.arg(Qt::escape(strBook.left(strBook.size()-1)))
						.arg(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor())
						.arg(Qt::escape(strBook.right(1)));
	} else {
		strHTML += QString("<p><b>%1</b>")
						.arg(Qt::escape(book.m_strBkName));
	}

	// Print this Verse Text:
	const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndx);
	if (pVerse == NULL) {
		assert(false);
		emit changedDocumentText();
		return;
	}
	if (!bNoAnchors) {
		strHTML += QString("<a id=\"%1\"><b> %2:%3 </b></a>")
					.arg(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), 0).asAnchor())
					.arg(ndx.chapter())
					.arg(ndx.verse());
	} else {
		strHTML += QString("<b> %1:%2 </b>")
					.arg(ndx.chapter())
					.arg(ndx.verse());
	}
	strHTML += pVerse->text() + "\n";

	strHTML += "</p></body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

void CPhraseNavigator::setDocumentToFormattedVerses(const TPhraseTag &tag)
{
	assert(m_pBibleDatabase.data() != NULL);

	m_TextDocument.clear();

	if ((!tag.first.isSet()) || (tag.second == 0)) {
		emit changedDocumentText();
		return;
	}

	CRelIndex ndxFirst = CRelIndex(tag.first.book(), tag.first.chapter(), tag.first.verse(), 1);		// Start on first word of verse
	CRelIndex ndxLast = m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(tag.first) + tag.second - 1);		// Add number of words to arrive at last word, wherever that is
	ndxLast.setWord(1);			// Shift back to the first word of this verse
	CRelIndex ndxNext = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, ndxLast);	// Add a verse, so we ndxNext is on first word of next verse.
	ndxLast = m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(ndxNext) - 1);		// Move to next word so ndxLast is the last word of the last verse
	TPhraseTag tagAdjusted(ndxFirst, m_pBibleDatabase->NormalizeIndex(ndxNext) - m_pBibleDatabase->NormalizeIndex(ndxFirst));

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
//						.arg(Qt::escape(tagAdjusted.PassageReferenceRangeText()));		// Document Title

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n")
	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n")
						.arg(Qt::escape(tagAdjusted.PassageReferenceRangeText(m_pBibleDatabase)));		// Document Title

	QString strReference;

	if (ndxFirst.book() == ndxLast.book()) {
		if (ndxFirst.chapter() == ndxLast.chapter()) {
			if (ndxFirst.verse() == ndxLast.verse()) {
				strReference = QString("(%1 %2:%3)")
										.arg(Qt::escape(m_pBibleDatabase->bookName(ndxFirst)))
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse());
			} else {
				strReference = QString("(%1 %2:%3-%4)")
										.arg(Qt::escape(m_pBibleDatabase->bookName(ndxFirst)))
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse())
										.arg(ndxLast.verse());
			}
		} else {
			strReference = QString("(%1 %2:%3-%4:%5)")
									.arg(Qt::escape(m_pBibleDatabase->bookName(ndxFirst)))
									.arg(ndxFirst.chapter())
									.arg(ndxFirst.verse())
									.arg(ndxLast.chapter())
									.arg(ndxLast.verse());
		}
	} else {
		strReference = QString("(%1 %2:%3-%4 %5:%6)")
								.arg(Qt::escape(m_pBibleDatabase->bookName(ndxFirst)))
								.arg(ndxFirst.chapter())
								.arg(ndxFirst.verse())
								.arg(Qt::escape(m_pBibleDatabase->bookName(ndxLast)))
								.arg(ndxLast.chapter())
								.arg(ndxLast.verse());
	}

	strHTML += QString("<p><b>%1</b> &quot;").arg(Qt::escape(strReference));

	CRelIndex ndxPrev = ndxFirst;
	for (CRelIndex ndx = ndxFirst; ndx.index() < ndxLast.index(); ndx=m_pBibleDatabase->calcRelIndex(0,1,0,0,0,ndx)) {
		if (ndx.book() != ndxPrev.book()) {
			strHTML += QString("  <b>(%1 %2:%3)</b> ").arg(Qt::escape(m_pBibleDatabase->bookName(ndx))).arg(ndx.chapter()).arg(ndx.verse());
		} else if (ndx.chapter() != ndxPrev.chapter()) {
			strHTML += QString("  <b>{%1:%2}</b> ").arg(ndx.chapter()).arg(ndx.verse());
		} else if (ndx.verse() != ndxPrev.verse()) {
			strHTML += QString("  <b>{%1}</b> ").arg(ndx.verse());
		}

		if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
			assert(false);
			emit changedDocumentText();
			return;
		}

		const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

		if (ndx.chapter() > book.m_nNumChp) {
			assert(false);
			emit changedDocumentText();
			return;
		}

		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndx);
		if (pVerse == NULL) {
			assert(false);
			emit changedDocumentText();
			return;
		}

		strHTML += pVerse->text();

		ndxPrev = ndx;
	}

	strHTML += "&quot;</p></body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

QPair<CParsedPhrase, TPhraseTag> CPhraseNavigator::getSelectedPhrase(const CPhraseCursor &aCursor) const
{
	assert(m_pBibleDatabase.data() != NULL);

	QPair<CParsedPhrase, TPhraseTag> retVal;

	CPhraseCursor myCursor(aCursor);
	myCursor.beginEditBlock();
	int nPosFirst = qMin(myCursor.anchor(), myCursor.position());
	int nPosLast = qMax(myCursor.anchor(), myCursor.position());
	QString strPhrase;
	unsigned int nWords = 0;
	CRelIndex nIndex;
	bool bFoundAnchorA = false;			// Set to true once we've found the first A anchor so if we start in the middle of an A-B pair and hit a B, we know if we've seen the A or not.

	if (nPosFirst < nPosLast) {
		myCursor.setPosition(nPosFirst);
		// See if our first character is a space.  If so, don't include it because it's
		//		most likely the single space between words.  If so, we would end up
		//		starting the selection at the preceding word and while that's technically
		//		correct, it's confusing to the user who probably didn't mean for that
		//		to happen:
		if (myCursor.charUnderCursor().isSpace()) myCursor.moveCursorCharRight();
		myCursor.moveCursorWordStart();
		while (myCursor.position() < nPosLast) {
			QTextCharFormat fmt = myCursor.charFormat();
			QString strAnchorName = fmt.anchorName();
			if ((!fmt.isAnchor()) || (strAnchorName.startsWith('B'))) {		// Either we shouldn't be in an anchor or the end of an A-B special section marker
				if ((fmt.isAnchor()) && (!bFoundAnchorA)) {
					// If we hit a B anchor and haven't found our first A anchor yet,
					//		it means the user started the selection in the middle
					//		of the A-B pair and we want to clear everything we've
					//		found so far, because it belongs to the void between
					//		the two and not real text.  However, we want to continue
					//		and set our location info and the word we are on top of
					//		is actually the first word we want to keep.
					nIndex = CRelIndex();
					nWords = 0;
					strPhrase.clear();
				}
				if (!nIndex.isSet()) {
					CPhraseCursor tempCursor(myCursor);		// Need temp cursor as the following call destroys it:
					nIndex = ResolveCursorReference(tempCursor);
				}
				// If we haven't hit an anchor for an actual word within a verse, we can't be selecting
				//		text from a verse.  We must be in a special tag section of heading:
				if ((nIndex.verse() == 0) || (nIndex.word() == 0)) {
					nIndex = CRelIndex();
					nWords = 0;
					strPhrase.clear();
				} else {
					if (!CParsedPhrase::makeRawPhrase(myCursor.wordUnderCursor()).isEmpty()) {
						nWords++;
						if (!strPhrase.isEmpty()) strPhrase += " ";
						strPhrase += myCursor.wordUnderCursor();
					}
				}
				if (!myCursor.moveCursorWordRight()) break;
			} else {
				// If we hit an anchor, see if it's either a special section A-B marker or if
				//		it's a chapter start anchor.  If it's an A-anchor, find the B-anchor.
				//		If it is a chapter start anchor, search for our special X-anchor so
				//		we'll be at the correct start of the next verse:
				if (strAnchorName.startsWith('A')) {
					bFoundAnchorA = true;
					int nEndAnchorPos = anchorPosition("B" + strAnchorName.mid(1));
					assert(nEndAnchorPos >= 0);
					if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
				} else {
					if ((!strAnchorName.isEmpty()) && (!strAnchorName.startsWith('X'))) {
						CRelIndex ndxAnchor(strAnchorName);
						assert(ndxAnchor.isSet());
						if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
							int nEndAnchorPos = anchorPosition("X" + strAnchorName);
							assert(nEndAnchorPos >= 0);
							if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
						}
					}
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
	}
	myCursor.endEditBlock();

	retVal.first.ParsePhrase(strPhrase);
	retVal.second.first = nIndex;
	retVal.second.second = retVal.first.phraseRawSize();
	assert(nWords == retVal.second.second);

	return retVal;
}

void CPhraseNavigator::removeAnchors()
{
	assert(m_pBibleDatabase.data() != NULL);

	// Note: I discovered in this that just moving the cursor one character
	//		to the right at a time and looking for anchors wasn't sufficient.
	//		Not totally sure why, but it seems like some are kept on the
	//		block level and not in a corresponding text fragment.  So, I
	//		cobbled this up, pattered after our anchorPosition function, which
	//		was patterned after the Qt code for doing this:

	CPhraseCursor myCursor(&m_TextDocument);
	myCursor.beginEditBlock();

	for (QTextBlock block = m_TextDocument.begin(); block.isValid(); block = block.next()) {
		QTextCharFormat format = block.charFormat();
		if (format.isAnchor()) {
			format.setAnchorNames(QStringList());
			format.setAnchor(false);
			myCursor.setPosition(block.position());
			myCursor.setPosition(block.position()+1, QTextCursor::KeepAnchor);
			myCursor.setCharFormat(format);
			// This one is a linked list instead of an iterator, so no need to reset
			//	any iterators here
		}
		for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); /* increment inside loop */ ) {
			QTextFragment fragment = it.fragment();
			format = fragment.charFormat();
			if (format.isAnchor()) {
				format.setAnchorNames(QStringList());
				format.setAnchor(false);
				myCursor.setPosition(fragment.position());
				myCursor.setPosition(fragment.position()+1, QTextCursor::KeepAnchor);
				myCursor.setCharFormat(format);
				// Note: The above affects the fragment iteration list and
				//	if we don't reset our loop here, we'll segfault with an
				//	invalid iterator:
				it = block.begin();
			} else {
				++it;
			}
		}
	}
	myCursor.endEditBlock();
}

// ============================================================================

void CPhraseEditNavigator::selectWords(const TPhraseTag &tag)
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndxScroll = tag.first;
	if (ndxScroll.verse() == 1) ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading
	ndxScroll.setWord(0);

	m_TextEditor.scrollToAnchor(ndxScroll.asAnchor());

	CRelIndex ndxRel = tag.first;
	uint32_t ndxWord = ndxRel.word();
	ndxRel.setWord(0);
	int nPos = anchorPosition(ndxRel.asAnchor());
	if (nPos != -1) {
		CPhraseCursor myCursor(m_TextEditor.textCursor());
		myCursor.beginEditBlock();
		myCursor.setPosition(nPos);
		int nSelEnd = nPos;
		while (ndxWord) {
			QTextCharFormat fmt = myCursor.charFormat();
			QString strAnchorName = fmt.anchorName();
			if ((!fmt.isAnchor()) || (strAnchorName.startsWith('B'))) {		// Either we shouldn't be in an anchor or the end of an A-B special section marker
				if (!CParsedPhrase::makeRawPhrase(myCursor.wordUnderCursor()).isEmpty())
					ndxWord--;
				nSelEnd = myCursor.position();
				if (!myCursor.moveCursorWordRight()) break;
			} else {
				// If we hit an anchor, see if it's either a special section A-B marker or if
				//		it's a chapter start anchor.  If it's an A-anchor, find the B-anchor.
				//		If it is a chapter start anchor, search for our special X-anchor so
				//		we'll be at the correct start of the next verse:
				if (strAnchorName.startsWith('A')) {
					int nEndAnchorPos = anchorPosition("B" + strAnchorName.mid(1));
					assert(nEndAnchorPos >= 0);
					if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
				} else {
					if (!strAnchorName.isEmpty()) {
						CRelIndex ndxAnchor(strAnchorName);
						assert(ndxAnchor.isSet());
						if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
							int nEndAnchorPos = anchorPosition("X" + strAnchorName);
							assert(nEndAnchorPos >= 0);
							if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
						}
					}
					nSelEnd = myCursor.position();
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
		myCursor.setPosition(nSelEnd);
		unsigned int nCount = tag.second;
		while (nCount) {
			QTextCharFormat fmt = myCursor.charFormat();
			QString strAnchorName = fmt.anchorName();
			if ((!fmt.isAnchor()) || (strAnchorName.startsWith('B'))) {		// Either we shouldn't be in an anchor or the end of an A-B special section marker
				myCursor.moveCursorWordStart(QTextCursor::KeepAnchor);
				myCursor.moveCursorWordEnd(QTextCursor::KeepAnchor);
				fmt = myCursor.charFormat();
				if ((!fmt.isAnchor()) && (!CParsedPhrase::makeRawPhrase(myCursor.wordUnderCursor()).isEmpty()))
					nCount--;
				nSelEnd = myCursor.position();
				if (!myCursor.moveCursorWordRight(QTextCursor::KeepAnchor)) break;
			} else {
				// If we hit an anchor, see if it's either a special section A-B marker or if
				//		it's a chapter start anchor.  If it's an A-anchor, find the B-anchor.
				//		If it is a chapter start anchor, search for our special X-anchor so
				//		we'll be at the correct start of the next verse:
				if (strAnchorName.startsWith('A')) {
					int nEndAnchorPos = anchorPosition("B" + strAnchorName.mid(1));
					assert(nEndAnchorPos >= 0);
					if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos, QTextCursor::KeepAnchor);
				} else {
					if (!strAnchorName.isEmpty()) {
						CRelIndex ndxAnchor(strAnchorName);
						assert(ndxAnchor.isSet());
						if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
							int nEndAnchorPos = anchorPosition("X" + strAnchorName);
							assert(nEndAnchorPos >= 0);
							if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos, QTextCursor::KeepAnchor);
						}
					}
					nSelEnd = myCursor.position();
					if (!myCursor.moveCursorWordRight(QTextCursor::KeepAnchor)) break;
				}
			}
		}
		myCursor.setPosition(nSelEnd, QTextCursor::KeepAnchor);
		myCursor.endEditBlock();
		m_TextEditor.setTextCursor(myCursor);
		m_TextEditor.ensureCursorVisible();				// Hmmm, for some strange reason, this doen't always work when user has used mousewheel to scroll off.  Qt bug?
	}
}

QPair<CParsedPhrase, TPhraseTag> CPhraseEditNavigator::getSelectedPhrase() const
{
	assert(m_pBibleDatabase.data() != NULL);

	return CPhraseNavigator::getSelectedPhrase(m_TextEditor.textCursor());
}

bool CPhraseEditNavigator::handleToolTipEvent(const QHelpEvent *pHelpEvent, CBasicHighlighter &aHighlighter, const TPhraseTag &selection) const
{
	assert(m_pBibleDatabase.data() != NULL);

	assert(pHelpEvent != NULL);
	CRelIndex ndxReference = ResolveCursorReference(m_TextEditor.cursorForPosition(pHelpEvent->pos()));
	QString strToolTip = getToolTip(TPhraseTag(ndxReference, 1), selection);

	if (!strToolTip.isEmpty()) {
		highlightTag(aHighlighter, (selection.haveSelection() ? selection : TPhraseTag(ndxReference, 1)));
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(pHelpEvent->globalPos(), strToolTip, &m_TextEditor);
		} else {
			QToolTip::showText(pHelpEvent->globalPos(), strToolTip);
		}
	} else {
		highlightTag(aHighlighter);
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::hideText();
		} else {
			QToolTip::hideText();
		}
		return false;
	}

	return true;
}

bool CPhraseEditNavigator::handleToolTipEvent(CBasicHighlighter &aHighlighter, const TPhraseTag &tag, const TPhraseTag &selection) const
{
	assert(m_pBibleDatabase.data() != NULL);

	QString strToolTip = getToolTip(tag, selection);

	if (!strToolTip.isEmpty()) {
		highlightTag(aHighlighter, (selection.haveSelection() ? selection : TPhraseTag(tag.first, 1)));
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip, m_TextEditor.viewport(), m_TextEditor.rect());
		} else {
			QToolTip::showText(m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip);
		}
	} else {
		highlightTag(aHighlighter);
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::hideText();
		} else {
			QToolTip::hideText();
		}
		return false;
	}

	return true;
}

void CPhraseEditNavigator::highlightTag(CBasicHighlighter &aHighlighter, const TPhraseTag &tag) const
{
	assert(m_pBibleDatabase.data() != NULL);

	doHighlighting(aHighlighter, true);
	TPhraseTagList tags;
	// Highlight the word only if we have a reference for an actual word (not just a chapter or book or something):
	if ((tag.first.book() != 0) &&
		(tag.first.chapter() != 0) &&
		(tag.first.verse() != 0) &&
		(tag.first.word() != 0) &&
		(tag.second != 0)) {
		tags.append(tag);
		aHighlighter.setHighlightTags(tags);
		doHighlighting(aHighlighter);
	} else {
		aHighlighter.clearPhraseTags();
	}
}

QString CPhraseEditNavigator::getToolTip(const TPhraseTag &tag, const TPhraseTag &selection, TOOLTIP_TYPE_ENUM nToolTipType, bool bPlainText) const
{
	assert(m_pBibleDatabase.data() != NULL);

	bool bHaveSelection = selection.haveSelection();
	const CRelIndex &ndxReference(bHaveSelection ? selection.first : tag.first);

	QString strToolTip;

	if (ndxReference.isSet()) {
		if (!bPlainText) strToolTip += "<html><body><pre>";
		if ((nToolTipType == TTE_COMPLETE) ||
			(nToolTipType == TTE_REFERENCE_ONLY)) {
			if (!bHaveSelection) {
				if (ndxReference.word() != 0) {
					uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(ndxReference);
					if ((ndxNormal != 0) && (ndxNormal <= m_pBibleDatabase->bibleEntry().m_nNumWrd)) {
						strToolTip += tr("Word:") + " \"" + m_pBibleDatabase->wordAtIndex(ndxNormal) + "\"\n";
					}
				}
				strToolTip += m_pBibleDatabase->SearchResultToolTip(ndxReference);
			} else {
				strToolTip += tr("Phrase:") + " \"";
				uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(ndxReference);
				if (ndxNormal != 0) {
					unsigned int ndx;
					for (ndx = 0; ((ndx < qMin(7u, selection.second)) && ((ndxNormal + ndx) <= m_pBibleDatabase->bibleEntry().m_nNumWrd)); ++ndx) {
						if (ndx) strToolTip += " ";
						strToolTip += m_pBibleDatabase->wordAtIndex(ndxNormal + ndx);
					}
					if ((ndx == 7u) && (selection.second > 7u)) strToolTip += " ...";
				} else {
					assert(false);
					strToolTip += "???";
				}
				strToolTip += "\"\n";
				strToolTip += m_pBibleDatabase->SearchResultToolTip(ndxReference, RIMASK_ALL, selection.second);
			}
		}
		if ((nToolTipType == TTE_COMPLETE) ||
			(nToolTipType == TTE_STATISTICS_ONLY)) {
			if (ndxReference.book() != 0) {
				assert(ndxReference.book() <= m_pBibleDatabase->bibleEntry().m_nNumBk);
				if (ndxReference.book() <= m_pBibleDatabase->bibleEntry().m_nNumBk) {
					if (nToolTipType == TTE_COMPLETE) {
						if (!bPlainText) {
							strToolTip += "</pre><hr /><pre>";
						} else {
							strToolTip += "--------------------\n";
						}
					}
					strToolTip += QString("\n%1 ").arg(m_pBibleDatabase->bookName(ndxReference)) + tr("contains:") + "\n"
											"    " + tr("%n Chapter(s)", NULL, m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) + "\n"
											"    " + tr("%n Verse(s)", NULL, m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumVrs) + "\n"
											"    " + tr("%n Word(s)", NULL, m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumWrd) + "\n";
					if (ndxReference.chapter() != 0) {
						assert(ndxReference.chapter() <= m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp);
						if (ndxReference.chapter() <= m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) {
							strToolTip += QString("\n%1 %2 ").arg(m_pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()) + tr("contains:") + "\n"
											"    " + tr("%n Verse(s)", NULL, m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) + "\n"
											"    " + tr("%n Word(s)", NULL, m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumWrd) + "\n";
							if ((!bHaveSelection) && (ndxReference.verse() != 0)) {
								assert(ndxReference.verse() <= m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs);
								if (ndxReference.verse() <= m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) {
									strToolTip += QString("\n%1 %2:%3 ").arg(m_pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()).arg(ndxReference.verse()) + tr("contains:") + "\n"
											"    " + tr("%n Word(s)", NULL, m_pBibleDatabase->verseEntry(ndxReference)->m_nNumWrd) + "\n";
								}
							}
						}
					}
				}
			}
			if (bHaveSelection) {
				strToolTip += "\n" + tr("%n Word(s) Selected", NULL, selection.second) + "\n";
			}
		}
		if (!bPlainText) strToolTip += "</pre></body></html>";
	}

	return strToolTip;
}

// ============================================================================

