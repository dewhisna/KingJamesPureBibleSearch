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
	if (m_cache_lstPhraseTagResults.size()) return m_cache_lstPhraseTagResults;

	m_cache_lstPhraseTagResults.clear();		// This call really shouldn't be needed since we already know the size is zero (above), but it just feels better with it. :-)
	const TIndexList &lstPhraseResults(GetNormalizedSearchResults());
	for (unsigned int ndxResults=0; ndxResults<lstPhraseResults.size(); ++ndxResults) {
		m_cache_lstPhraseTagResults.append(TPhraseTag(CRelIndex(DenormalizeIndex(lstPhraseResults.at(ndxResults))), phraseSize()));
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
	CPhraseCursor cursor(curInsert);
	cursor.clearSelection();
	cursor.selectWordUnderCursor();							// Select word under the cursor
	cursor.insertText(completion);							// Replace with completed word

	return cursor;
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
				QRegExp exp(m_lstWords[ndx], (isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::Wildcard);
				if (exp.exactMatch(itrWordMap->first)) bFirstWordExactMatch = true;
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

		if (((m_lstMatchMapping.size() != 0) && (ndx > 0)) ||
			((bFirstWordExactMatch) && (ndx == 0))) m_nLevel++;

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

CRelIndex CPhraseNavigator::ResolveCursorReference2(CPhraseCursor cursor) const
{

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
					CRelIndex ndxAnchor(strAnchorName);
					assert(ndxAnchor.isSet());
					if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
						int nEndAnchorPos = anchorPosition("X" + fmt.anchorName());
						assert(nEndAnchorPos >= 0);
						if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
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
					CRelIndex ndxAnchor(strAnchorName);
					assert(ndxAnchor.isSet());
					if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
						int nEndAnchorPos = anchorPosition("X" + strAnchorName);
						assert(nEndAnchorPos >= 0);
						if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
					}
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
	}
}

void CPhraseNavigator::setDocumentToChapter(const CRelIndex &ndx, bool bNoAnchors)
{
	m_TextDocument.clear();

	if ((ndx.book() == 0) || (ndx.chapter() == 0)) return;

	if (ndx.book() > g_lstTOC.size()) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	const CTOCEntry &toc = g_lstTOC[ndx.book()-1];
	const TBookEntryMap &book = g_lstBooks[ndx.book()-1];

	TLayoutMap::const_iterator mapLookupLayout = g_mapLayout.find(CRelIndex(ndx.book(),ndx.chapter(),0,0));
	if (mapLookupLayout == g_mapLayout.end()) {
		assert(false);
		emit changedDocumentText();
		return;
	}
	const CLayoutEntry &layout(mapLookupLayout->second);

	if (ndx.chapter() > toc.m_nNumChp) {
		assert(false);
		emit changedDocumentText();
		return;
	}
	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
						.arg(Qt::escape(ndx.PassageReferenceText()));		// Document Title
//	QString strHTML = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><style type=\"text/css\"><!-- A { text-decoration:none } %s --></style></head><body><br />";

	uint32_t nFirstWordNormal = NormalizeIndex(CRelIndex(ndx.book(), ndx.chapter(), 1, 1));		// Find normalized word number for the first verse, first word of this book/chapter
	uint32_t nNextChapterFirstWordNormal = nFirstWordNormal + layout.m_nNumWrd;		// Add the number of words in this chapter to get first word normal of next chapter
	uint32_t nRelPrevChapter = DenormalizeIndex(nFirstWordNormal - 1);				// Find previous book/chapter/verse (and word)
	uint32_t nRelNextChapter = DenormalizeIndex(nNextChapterFirstWordNormal);		// Find next book/chapter/verse (and word)

	// Print last verse of previous chapter if available:
	if (nRelPrevChapter != 0) {
		CRelIndex relPrev(nRelPrevChapter);
		strHTML += "<p>";
		if (!bNoAnchors) {
			strHTML += QString("<a id=\"%1\"><b> %2 </b></a>").arg(CRelIndex(relPrev.book(), relPrev.chapter(), relPrev.verse(), 0).asAnchor()).arg(relPrev.verse());
		} else {
			strHTML += QString("<b> %1 </b>").arg(relPrev.verse());
		}
		strHTML += (g_lstBooks[relPrev.book()-1])[CRelIndex(0,relPrev.chapter(),relPrev.verse(),0)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<hr />\n";

	// Print Heading for this Book/Chapter:
	if (!bNoAnchors) {
		strHTML += QString("<div class=book><a id=\"%1\">%2</a></div>\n")
						.arg(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor())
						.arg(Qt::escape(toc.m_strBkName));
		strHTML += QString("<div class=chapter><a id=\"%1\">Chapter %2</a></div><a id=\"X%3\"> </a>\n")
						.arg(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor())
						.arg(ndx.chapter())
						.arg(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor());
	} else {
		strHTML += QString("<div class=book>%1</div>\n")
						.arg(Qt::escape(toc.m_strBkName));
		strHTML += QString("<div class=chapter>Chapter %1</div>\n")
						.arg(ndx.chapter());
	}

	// Print this Chapter Text:
	bool bParagraph = false;
	for (unsigned int ndxVrs=0; ndxVrs<layout.m_nNumVrs; ++ndxVrs) {
		TBookEntryMap::const_iterator mapLookupVerse = book.find(CRelIndex(0,ndx.chapter(),ndxVrs+1,0));
		if (mapLookupVerse == book.end()) {
			assert(false);
			continue;
		}
		const CBookEntry &verse(mapLookupVerse->second);
		if (verse.m_bPilcrow) {
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
						.arg(CRelIndex(ndx.book(), ndx.chapter(), ndxVrs+1, 0).asAnchor())
						.arg(ndxVrs+1);
		} else {
			strHTML += QString("<b> %1 </b>")
						.arg(ndxVrs+1);
		}
		strHTML += verse.GetRichText() + "\n";
	}
	if (bParagraph) {
		strHTML += "</p>";
		bParagraph = false;
	}

	strHTML += "<hr />\n";

	// Print first verse of next chapter if available:
	if (nRelNextChapter != 0) {
		CRelIndex relNext(nRelNextChapter);

		// Print Heading for this Book/Chapter:
		if (relNext.book() != ndx.book()) {
			if (!bNoAnchors) {
				strHTML += QString("<div class=book><a id=\"%1\">%2</a></div>\n")
								.arg(CRelIndex(relNext.book(), relNext.chapter(), 0 ,0).asAnchor())
								.arg(g_lstTOC[relNext.book()-1].m_strBkName);
			} else {
				strHTML += QString("<div class=book>%1</div>\n")
								.arg(g_lstTOC[relNext.book()-1].m_strBkName);
			}
		}
		if (!bNoAnchors) {
			strHTML += QString("<div class=chapter><a id=\"%1\">Chapter %2</a></div><a id=\"X%3\"> </a>\n")
								.arg(CRelIndex(relNext.book(), relNext.chapter(), 0, 0).asAnchor())
								.arg(relNext.chapter())
								.arg(CRelIndex(relNext.book(), relNext.chapter(), 0, 0).asAnchor());
		} else {
			strHTML += QString("<div class=chapter>Chapter %1</div>\n")
								.arg(relNext.chapter());
		}

		strHTML += "<p>";
		if (!bNoAnchors) {
			strHTML += QString("<a id=\"%1\"><b> %2 </b></a>").arg(CRelIndex(relNext.book(), relNext.chapter(), relNext.verse(), 0).asAnchor()).arg(relNext.verse());
		} else {
			strHTML += QString("<b> %1 </b>").arg(relNext.verse());
		}
		strHTML += (g_lstBooks[relNext.book()-1])[CRelIndex(0,relNext.chapter(),relNext.verse(),0)].GetRichText() + "\n";
		strHTML += "</p>";
	}

	strHTML += "<br /></body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

void CPhraseNavigator::setDocumentToVerse(const CRelIndex &ndx, bool bAddDividerLineBefore, bool bNoAnchors)
{
	m_TextDocument.clear();

	if ((ndx.book() == 0) || (ndx.chapter() == 0) || (ndx.verse() == 0)) {
		emit changedDocumentText();
		return;
	}

	if (ndx.book() > g_lstTOC.size()) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	const CTOCEntry &toc = g_lstTOC[ndx.book()-1];
	const TBookEntryMap &book = g_lstBooks[ndx.book()-1];

	if (ndx.chapter() > toc.m_nNumChp) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	TLayoutMap::const_iterator mapLookupLayout = g_mapLayout.find(CRelIndex(ndx.book(),ndx.chapter(),0,0));
	if (mapLookupLayout == g_mapLayout.end()) {
		assert(false);
		emit changedDocumentText();
		return;
	}
	const CLayoutEntry &layout(mapLookupLayout->second);

	if (ndx.verse() > layout.m_nNumVrs) {
		assert(false);
		emit changedDocumentText();
		return;
	}

	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
						.arg(Qt::escape(ndx.PassageReferenceText()));		// Document Title
//	QString strHTML = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><style type=\"text/css\"><!-- A { text-decoration:none } %s --></style></head><body><br />";

	if (bAddDividerLineBefore) strHTML += "<hr />";

	// Print Book/Chapter for this verse:
	if (!bNoAnchors) {
		//		Note: This little shenanigan is so we can have an ending "X" anchor within the name of the book
		//				itself.  This is because the chapter/verse reference anchor below must begin with
		//				a space so that we can find a dual unique anchor in our searching.  If we don't do
		//				this with the book name, we have to insert an extra space at the end for the "X" anchor
		//				and that extra space was just annoying me!!!
		QString strBook = toc.m_strBkName;
		strBook = strBook.leftJustified(2, ' ', false);
		strHTML += QString("<p><a id=\"%1\"><b>%2</b></a><a id=\"X%3\"><b>%4</b></a>")
						.arg(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor())
						.arg(Qt::escape(strBook.left(strBook.size()-1)))
						.arg(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor())
						.arg(Qt::escape(strBook.right(1)));
	} else {
		strHTML += QString("<p><b>%1</b>")
						.arg(Qt::escape(toc.m_strBkName));
	}

	// Print this Verse Text:
	TBookEntryMap::const_iterator mapLookupVerse = book.find(CRelIndex(0,ndx.chapter(),ndx.verse(),0));
	if (mapLookupVerse == book.end()) {
		assert(false);
		emit changedDocumentText();
		return;
	}
	const CBookEntry &verse(mapLookupVerse->second);
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
	strHTML += verse.GetRichText() + "\n";

	strHTML += "</p></body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

void CPhraseNavigator::setDocumentToFormattedVerses(const TPhraseTag &tag)
{
	m_TextDocument.clear();

	if ((!tag.first.isSet()) || (tag.second == 0)) {
		emit changedDocumentText();
		return;
	}

	CRelIndex ndxFirst = CRelIndex(tag.first.book(), tag.first.chapter(), tag.first.verse(), 1);		// Start on first word of verse
	CRelIndex ndxLast = DenormalizeIndex(NormalizeIndex(tag.first) + tag.second - 1);		// Add number of words to arrive at last word, wherever that is
	ndxLast.setWord(1);			// Shift back to the first word of this verse
	CRelIndex ndxNext = CRefCountCalc::calcRelIndex(0, 1, 0, 0, 0, ndxLast);	// Add a verse, so we ndxNext is on first word of next verse.
	ndxLast = DenormalizeIndex(NormalizeIndex(ndxNext) - 1);		// Move to next word so ndxLast is the last word of the last verse
	TPhraseTag tagAdjusted(ndxFirst, NormalizeIndex(ndxNext) - NormalizeIndex(ndxFirst));

	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
						.arg(Qt::escape(tagAdjusted.PassageReferenceRangeText()));		// Document Title

	QString strReference;

	if (ndxFirst.book() == ndxLast.book()) {
		if (ndxFirst.chapter() == ndxLast.chapter()) {
			if (ndxFirst.verse() == ndxLast.verse()) {
				strReference = QString("(%1 %2:%3)")
										.arg(ndxFirst.bookName())
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse());
			} else {
				strReference = QString("(%1 %2:%3-%4)")
										.arg(ndxFirst.bookName())
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse())
										.arg(ndxLast.verse());
			}
		} else {
			strReference = QString("(%1 %2:%3-%4:%5)")
									.arg(ndxFirst.bookName())
									.arg(ndxFirst.chapter())
									.arg(ndxFirst.verse())
									.arg(ndxLast.chapter())
									.arg(ndxLast.verse());
		}
	} else {
		strReference = QString("(%1 %2:%3-%4 %5:%6)")
								.arg(ndxFirst.bookName())
								.arg(ndxFirst.chapter())
								.arg(ndxFirst.verse())
								.arg(ndxLast.bookName())
								.arg(ndxLast.chapter())
								.arg(ndxLast.verse());
	}

	strHTML += QString("<p><b>%1</b> &quot;").arg(Qt::escape(strReference));

	CRelIndex ndxPrev = ndxFirst;
	for (CRelIndex ndx = ndxFirst; ndx.index() < ndxLast.index(); ndx=CRefCountCalc::calcRelIndex(0,1,0,0,0,ndx)) {
		if (ndx.book() != ndxPrev.book()) {
			strHTML += QString("  <b>%1 %2:%3</b> ").arg(Qt::escape(ndx.bookName())).arg(ndx.chapter()).arg(ndx.verse());
		} else if (ndx.chapter() != ndxPrev.chapter()) {
			strHTML += QString("  <b>%1:%2</b> ").arg(ndx.chapter()).arg(ndx.verse());
		} else if (ndx.verse() != ndxPrev.verse()) {
			strHTML += QString("  <b>%1</b> ").arg(ndx.verse());
		}

		if (ndx.book() > g_lstTOC.size()) {
			assert(false);
			emit changedDocumentText();
			return;
		}

		const CTOCEntry &toc = g_lstTOC[ndx.book()-1];

		if (ndx.chapter() > toc.m_nNumChp) {
			assert(false);
			emit changedDocumentText();
			return;
		}

		const TBookEntryMap &book = g_lstBooks[ndx.book()-1];
		TBookEntryMap::const_iterator mapLookupVerse = book.find(CRelIndex(0,ndx.chapter(),ndx.verse(),0));
		if (mapLookupVerse == book.end()) {
			assert(false);
			emit changedDocumentText();
			return;
		}
		const CBookEntry &verse(mapLookupVerse->second);

		strHTML += verse.GetRichText();

		ndxPrev = ndx;
	}

	strHTML += "&quot;</p></body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

QPair<CParsedPhrase, TPhraseTag> CPhraseNavigator::getSelectedPhrase(const CPhraseCursor &aCursor) const
{
	QPair<CParsedPhrase, TPhraseTag> retVal;

	CPhraseCursor myCursor(aCursor);
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
				if (!CParsedPhrase::makeRawPhrase(myCursor.wordUnderCursor()).isEmpty()) {
					nWords++;
					if (!strPhrase.isEmpty()) strPhrase += " ";
					strPhrase += myCursor.wordUnderCursor();
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
					CRelIndex ndxAnchor(strAnchorName);
					assert(ndxAnchor.isSet());
					if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
						int nEndAnchorPos = anchorPosition("X" + fmt.anchorName());
						assert(nEndAnchorPos >= 0);
						if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
					}
					if (!myCursor.moveCursorWordRight()) break;
				}
			}
		}
	}

	retVal.first.ParsePhrase(strPhrase);
	retVal.second.first = nIndex;
	retVal.second.second = retVal.first.phraseRawSize();
	assert(nWords == retVal.second.second);

	return retVal;
}

void CPhraseNavigator::removeAnchors()
{
	// Note: I discovered in this that just moving the cursor one character
	//		to the right at a time and looking for anchors wasn't sufficient.
	//		Not totally sure why, but it seems like some are kept on the
	//		block level and not in a corresponding text fragment.  So, I
	//		cobbled this up, pattered after our anchorPosition function, which
	//		was patterned after the Qt code for doing this:

	CPhraseCursor cursor(&m_TextDocument);

	for (QTextBlock block = m_TextDocument.begin(); block.isValid(); block = block.next()) {
		QTextCharFormat format = block.charFormat();
		if (format.isAnchor()) {
			format.setAnchorNames(QStringList());
			format.setAnchor(false);
			cursor.setPosition(block.position());
			cursor.setPosition(block.position()+1, QTextCursor::KeepAnchor);
			cursor.setCharFormat(format);
			// This one is a linked list instead of an iterator, so no need to reset
			//	any iterators here
		}
		for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); /* increment inside loop */ ) {
			QTextFragment fragment = it.fragment();
			format = fragment.charFormat();
			if (format.isAnchor()) {
				format.setAnchorNames(QStringList());
				format.setAnchor(false);
				cursor.setPosition(fragment.position());
				cursor.setPosition(fragment.position()+1, QTextCursor::KeepAnchor);
				cursor.setCharFormat(format);
				// Note: The above affects the fragment iteration list and
				//	if we don't reset our loop here, we'll segfault with an
				//	invalid iterator:
				it = block.begin();
			} else {
				++it;
			}
		}
	}
}

// ============================================================================

void CPhraseEditNavigator::selectWords(const TPhraseTag &tag)
{
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
					CRelIndex ndxAnchor(strAnchorName);
					assert(ndxAnchor.isSet());
					if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
						int nEndAnchorPos = anchorPosition("X" + fmt.anchorName());
						assert(nEndAnchorPos >= 0);
						if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos);
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
					CRelIndex ndxAnchor(strAnchorName);
					assert(ndxAnchor.isSet());
					if ((ndxAnchor.isSet()) && (ndxAnchor.verse() == 0) && (ndxAnchor.word() == 0)) {
						int nEndAnchorPos = anchorPosition("X" + fmt.anchorName());
						assert(nEndAnchorPos >= 0);
						if (nEndAnchorPos >= 0) myCursor.setPosition(nEndAnchorPos, QTextCursor::KeepAnchor);
					}
					nSelEnd = myCursor.position();
					if (!myCursor.moveCursorWordRight(QTextCursor::KeepAnchor)) break;
				}
			}
		}
		myCursor.setPosition(nSelEnd, QTextCursor::KeepAnchor);
		m_TextEditor.setTextCursor(myCursor);
		m_TextEditor.ensureCursorVisible();				// Hmmm, for some strange reason, this doen't always work when user has used mousewheel to scroll off.  Qt bug?
	}
}

QPair<CParsedPhrase, TPhraseTag> CPhraseEditNavigator::getSelectedPhrase() const
{
	return CPhraseNavigator::getSelectedPhrase(m_TextEditor.textCursor());
}

bool CPhraseEditNavigator::handleToolTipEvent(const QHelpEvent *pHelpEvent, CBasicHighlighter &aHighlighter, const TPhraseTag &selection) const
{
	assert(pHelpEvent != NULL);
	CRelIndex ndxReference = ResolveCursorReference(m_TextEditor.cursorForPosition(pHelpEvent->pos()));
	QString strToolTip = getToolTip(TPhraseTag(ndxReference, 1), selection);

	if (!strToolTip.isEmpty()) {
		highlightTag(aHighlighter, TPhraseTag(ndxReference, 1));
		if (m_bUseToolTipEdit) {
			CToolTipEdit::showText(pHelpEvent->globalPos(), strToolTip);
		} else {
			QToolTip::showText(pHelpEvent->globalPos(), strToolTip);
		}
	} else {
		highlightTag(aHighlighter);
		if (m_bUseToolTipEdit) {
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
	bool bHaveSelection = selection.haveSelection();
	const CRelIndex &ndxReference(bHaveSelection ? selection.first : tag.first);

	QString strToolTip;

	if (ndxReference.isSet()) {
		if (!bPlainText) strToolTip += "<html><body><pre>";
		if ((nToolTipType == TTE_COMPLETE) ||
			(nToolTipType == TTE_REFERENCE_ONLY)) {
			if (!bHaveSelection) {
				if (ndxReference.word() != 0) {
					uint32_t ndxNormal = NormalizeIndex(ndxReference);
					if ((ndxNormal != 0) && (ndxNormal <= g_lstConcordanceMapping.size())) {
						strToolTip += "Word: \"" + g_lstConcordanceWords.at(g_lstConcordanceMapping.at(ndxNormal)-1) + "\"\n";
					}
				}
				strToolTip += ndxReference.SearchResultToolTip();
			} else {
				strToolTip += "Phrase: \"";
				uint32_t ndxNormal = NormalizeIndex(ndxReference);
				if (ndxNormal != 0) {
					unsigned int ndx;
					for (ndx = 0; ((ndx < qMin(7u, selection.second)) && ((ndxNormal + ndx) <= g_lstConcordanceMapping.size())); ++ndx) {
						if (ndx) strToolTip += " ";
						strToolTip += g_lstConcordanceWords.at(g_lstConcordanceMapping.at(ndxNormal + ndx)-1);
					}
					if ((ndx == 7u) && (selection.second > 7u)) strToolTip += " ...";
				} else {
					assert(false);
					strToolTip += "???";
				}
				strToolTip += "\"\n";
				strToolTip += ndxReference.SearchResultToolTip(RIMASK_ALL, selection.second);
			}
		}
		if ((nToolTipType == TTE_COMPLETE) ||
			(nToolTipType == TTE_STATISTICS_ONLY)) {
			if (ndxReference.book() != 0) {
				assert(ndxReference.book() <= g_lstTOC.size());
				if (ndxReference.book() <= g_lstTOC.size()) {
					if (nToolTipType == TTE_COMPLETE) {
						if (!bPlainText) {
							strToolTip += "</pre><hr /><pre>";
						} else {
							strToolTip += "--------------------\n";
						}
					}
					strToolTip += QString("\n%1 contains:\n"
											"    %2 Chapters\n"
											"    %3 Verses\n"
											"    %4 Words\n")
											.arg(ndxReference.bookName())
											.arg(g_lstTOC[ndxReference.book()-1].m_nNumChp)
											.arg(g_lstTOC[ndxReference.book()-1].m_nNumVrs)
											.arg(g_lstTOC[ndxReference.book()-1].m_nNumWrd);
					if (ndxReference.chapter() != 0) {
						assert(ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp);
						if (ndxReference.chapter() <= g_lstTOC[ndxReference.book()-1].m_nNumChp) {
							strToolTip += QString("\n%1 %2 contains:\n"
													"    %3 Verses\n"
													"    %4 Words\n")
													.arg(ndxReference.bookName()).arg(ndxReference.chapter())
													.arg(g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs)
													.arg(g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumWrd);
							if ((!bHaveSelection) && (ndxReference.verse() != 0)) {
								assert(ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs);
								if (ndxReference.verse() <= g_mapLayout[CRelIndex(ndxReference.book(), ndxReference.chapter(), 0, 0)].m_nNumVrs) {
									strToolTip += QString("\n%1 %2:%3 contains:\n"
															"    %4 Words\n")
															.arg(ndxReference.bookName()).arg(ndxReference.chapter()).arg(ndxReference.verse())
															.arg((g_lstBooks[ndxReference.book()-1])[CRelIndex(0, ndxReference.chapter(), ndxReference.verse(), 0)].m_nNumWrd);
								}
							}
						}
					}
				}
			}
			if (bHaveSelection) {
				strToolTip += QString("\n%1 Word(s) Selected\n").arg(selection.second);
			}
		}
		if (!bPlainText) strToolTip += "</pre></body></html>";
	}

	return strToolTip;
}

// ============================================================================

