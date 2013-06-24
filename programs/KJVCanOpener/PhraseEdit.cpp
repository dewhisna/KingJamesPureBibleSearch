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
#include "ParseSymbols.h"
#include "VerseRichifier.h"
#include "SearchCompleter.h"

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

CParsedPhrase::CParsedPhrase(CBibleDatabasePtr pBibleDatabase, bool bCaseSensitive, bool bAccentSensitive)
	:	m_pBibleDatabase(pBibleDatabase),
		m_bIsDuplicate(false),
		m_bIsDisabled(false),
		m_bCaseSensitive(bCaseSensitive),
		m_bAccentSensitive(bAccentSensitive),
		m_nLevel(0),
		m_nCursorLevel(0),
		m_nCursorWord(-1),
		m_nLastMatchWord(-1)
{
	if (pBibleDatabase)
		m_lstNextWords = pBibleDatabase->concordanceWordList();
}

CParsedPhrase::~CParsedPhrase()
{

}

// ============================================================================

bool CPhraseEntry::operator==(const CParsedPhrase &src) const
{
	return ((m_bCaseSensitive == src.isCaseSensitive()) &&
			(m_bAccentSensitive == src.isAccentSensitive()) &&
			(m_strPhrase.compare(src.phrase(), Qt::CaseSensitive) == 0));
}

uint32_t CParsedPhrase::GetNumberOfMatches() const
{
	return m_lstMatchMapping.size();
}

#ifdef NORMALIZED_SEARCH_PHRASE_RESULTS_CACHE
const TIndexList &CParsedPhrase::GetNormalizedSearchResults() const
{
	if (m_cache_lstNormalizedSearchResults.size()) return m_cache_lstNormalizedSearchResults;

	m_cache_lstNormalizedSearchResults.resize(m_lstMatchMapping.size());
	for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
		if ((m_lstMatchMapping.at(ndxWord) - m_nLevel + 1) > 0)
			m_cache_lstNormalizedSearchResults[ndxWord] = m_lstMatchMapping.at(ndxWord) - m_nLevel + 1;
	}
	sort(m_cache_lstNormalizedSearchResults.begin(), m_cache_lstNormalizedSearchResults.end());

	return m_cache_lstNormalizedSearchResults;
}
#endif

const TPhraseTagList &CParsedPhrase::GetPhraseTagSearchResults() const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_cache_lstPhraseTagResults.size()) return m_cache_lstPhraseTagResults;

#ifdef NORMALIZED_SEARCH_PHRASE_RESULTS_CACHE
	const TIndexList &lstPhraseResults(GetNormalizedSearchResults());

	unsigned int nNumResults = lstPhraseResults.size();
	m_cache_lstPhraseTagResults.reserve(nNumResults);
	for (unsigned int ndxResults=0; ndxResults<nNumResults; ++ndxResults) {
		m_cache_lstPhraseTagResults.append(TPhraseTag(CRelIndex(m_pBibleDatabase->DenormalizeIndex(lstPhraseResults.at(ndxResults))), phraseSize()));
	}
#else
	m_cache_lstPhraseTagResults.reserve(m_lstMatchMapping.size());
	for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
		if ((m_lstMatchMapping.at(ndxWord) - m_nLevel + 1) > 0)
			m_cache_lstPhraseTagResults.append(TPhraseTag(CRelIndex(m_pBibleDatabase->DenormalizeIndex(m_lstMatchMapping.at(ndxWord) - m_nLevel + 1)), phraseSize()));
	}
	qSort(m_cache_lstPhraseTagResults.begin(), m_cache_lstPhraseTagResults.end(), TPhraseTagListSortPredicate::ascendingLessThan);
#endif

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

const QStringList &CParsedPhrase::phraseWords() const
{
	if (m_cache_lstPhraseWords.size()) return m_cache_lstPhraseWords;

	m_cache_lstPhraseWords = m_lstWords;
	for (int ndx = (m_cache_lstPhraseWords.size()-1); ndx >= 0; --ndx) {
		if (m_cache_lstPhraseWords.at(ndx).isEmpty()) m_cache_lstPhraseWords.removeAt(ndx);
	}
	return m_cache_lstPhraseWords;
}

const QStringList &CParsedPhrase::phraseWordsRaw() const
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
	QString strRawPhrase;
	QString strTemp = strPhrase;
	QChar chrNext;
	bool bNeedSpace = false;

	while (!strTemp.isEmpty()) {
		chrNext = strTemp.at(0);
		bool bIsHyphen = g_strHyphens.contains(chrNext);
		bool bIsApostrophe = g_strApostrophes.contains(chrNext);

		if ((chrNext.unicode() < 128) ||
			(g_strNonAsciiNonWordChars.contains(chrNext)) ||
			(bIsHyphen) ||
			(bIsApostrophe)) {
			if ((g_strAsciiWordChars.contains(chrNext)) ||
				(bIsHyphen) ||
				(bIsApostrophe) ||
				(chrNext == ' ')) {
				if ((bNeedSpace) && (chrNext != ' ')) strRawPhrase += ' ';
				bNeedSpace = false;
				if (bIsHyphen) {
					strRawPhrase += '-';
				} else if (bIsApostrophe) {
					strRawPhrase += '\'';
				} else strRawPhrase += chrNext;
			} else {
				// Ignore NonAsciiNonWordChars and codes <128 that aren't in AsciiWordChars.
				//		But, make sure we have a space before a word character, or else our
				//		word counts won't be correct:
				bNeedSpace = true;
			}
		} else {
			if ((bNeedSpace) && (chrNext != ' ')) strRawPhrase += ' ';
			bNeedSpace = false;

//			if (chrNext == QChar(0x00C6)) {				// U+00C6	&#198;		AE character
//				strRawPhrase += "Ae";
//			} else if (chrNext == QChar(0x00E6)) {		// U+00E6	&#230;		ae character
//				strRawPhrase += "ae";
//			} else if (chrNext == QChar(0x0132)) {		// U+0132	&#306;		IJ character
//				strRawPhrase += "IJ";
//			} else if (chrNext == QChar(0x0133)) {		// U+0133	&#307;		ij character
//				strRawPhrase += "ij";
//			} else if (chrNext == QChar(0x0152)) {		// U+0152	&#338;		OE character
//				strRawPhrase += "Oe";
//			} else if (chrNext == QChar(0x0153)) {		// U+0153	&#339;		oe character
//				strRawPhrase += "oe";
//			}											// All other UTF-8 leave untranslated
			strRawPhrase += chrNext;
		}

		strTemp = strTemp.right(strTemp.size()-1);
	}

	return strRawPhrase;
}

void CParsedPhrase::clearCache() const
{
	m_cache_lstPhraseWords = QStringList();
	m_cache_lstPhraseWordsRaw = QStringList();
#ifdef NORMALIZED_SEARCH_PHRASE_RESULTS_CACHE
	m_cache_lstNormalizedSearchResults = TIndexList();
#endif
	m_cache_lstPhraseTagResults = TPhraseTagList();
}

void CParsedPhrase::UpdateCompleter(const QTextCursor &curInsert, CSearchCompleter &aCompleter)
{
	CSearchStringListModel *pModel = (CSearchStringListModel *)(aCompleter.model());

	ParsePhrase(curInsert);
	FindWords(m_nCursorWord);

	pModel->setWordsFromPhrase();
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
		m_lstLeftWords.push_front(curLeft.wordUnderCursor().normalized(QString::NormalizationForm_C));
	}

	CPhraseCursor curRight(curInsert);
	m_strCursorWord = curRight.wordUnderCursor().normalized(QString::NormalizationForm_C);
	while (curRight.moveCursorWordRight()) {
		m_lstRightWords.push_back(curRight.wordUnderCursor().normalized(QString::NormalizationForm_C));
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

	m_lstWords = strPhrase.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), QString::SkipEmptyParts);
	m_lstLeftWords = m_lstWords;
	m_strCursorWord.clear();
	m_lstRightWords.clear();
	m_nCursorWord = m_lstWords.size();
}

void CParsedPhrase::ParsePhrase(const QStringList &lstPhrase)
{
	clearCache();

	m_lstWords = lstPhrase;
	m_lstLeftWords = m_lstWords;
	m_strCursorWord.clear();
	m_lstRightWords.clear();
	m_nCursorWord = m_lstWords.size();
}

void CParsedPhrase::FindWords(int nCursorWord)
{
	assert(m_pBibleDatabase.data() != NULL);

	assert((nCursorWord >= 0) && (nCursorWord <= m_lstWords.size()));

	m_lstMatchMapping.clear();
	bool bComputedNextWords = false;
	m_nLevel = 0;
	m_nCursorLevel = 0;
	bool bInFirstWordStar = false;
	for (int ndx=0; ndx<m_lstWords.size(); ++ndx) {
		if (m_lstWords.at(ndx).isEmpty()) continue;

		QString strCurWordDecomp = CSearchStringListModel::decompose(m_lstWords.at(ndx));
		QString strCurWord = (isAccentSensitive() ? m_lstWords.at(ndx) : strCurWordDecomp);
		QString strCurWordKey = strCurWordDecomp.toLower();
		QString strCurWordWildKey = strCurWordKey;			// Note: This becomes the "Word*" value later, so can't substitute strCurWordWild for all m_lstWords.at(ndx) (or strCurWord)
		int nPreRegExp = strCurWordWildKey.indexOf(QRegExp("[\\[\\]\\*\\?]"));

		if (nPreRegExp == -1) {
			if ((ndx == (m_lstWords.size()-1)) &&
				(m_pBibleDatabase->mapWordList().find(strCurWordKey) == m_pBibleDatabase->mapWordList().end())) {
				strCurWordWildKey += "*";			// If we're on the word currently being typed and it's not an exact match, simulate a "*" trailing wildcard to match all strings with this prefix
			}
		}

		QRegExp expCurWordWildKey(strCurWordWildKey, Qt::CaseInsensitive, QRegExp::Wildcard);
		QRegExp expCurWordExactKey(strCurWordKey, Qt::CaseInsensitive, QRegExp::Wildcard);
		QRegExp expCurWord(strCurWord, (isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::Wildcard);

		if ((ndx == 0) || (bInFirstWordStar)) {				// If we're matching the first word, build complete index to start off the compare:
			int nFirstWord = m_pBibleDatabase->lstWordList().indexOf(expCurWordWildKey);
			if (nFirstWord == -1) {
				if (nCursorWord > ndx) {
					// If we've stopped matching before the cursor, we have no "next words":
					m_lstNextWords.clear();
					bComputedNextWords = true;
				}
				break;			// If we've stopped matching, we're done
			}

			if (strCurWordWildKey.compare("*") == 0) {
				bInFirstWordStar = true;			// Treat "*" special, as if we have a match with no results, but yet we do
			} else {
				bInFirstWordStar = false;
				int nLastWord = m_pBibleDatabase->lstWordList().lastIndexOf(expCurWordWildKey);
				assert(nLastWord != -1);			// Should have at least one match since forward search matched above!

				for (int ndxWord = nFirstWord; ndxWord <= nLastWord; ++ndxWord) {
					if (!expCurWordExactKey.exactMatch(m_pBibleDatabase->lstWordList().at(ndxWord))) continue;
					TWordListMap::const_iterator itrWordMap = m_pBibleDatabase->mapWordList().find(m_pBibleDatabase->lstWordList().at(ndxWord));
					assert(itrWordMap != m_pBibleDatabase->mapWordList().end());
					if (itrWordMap == m_pBibleDatabase->mapWordList().end()) continue;

					const CWordEntry &wordEntry = itrWordMap->second;		// Entry for current word

					if ((!isCaseSensitive()) && (!isAccentSensitive())) {
						m_lstMatchMapping.insert(m_lstMatchMapping.end(), wordEntry.m_ndxNormalizedMapping.begin(), wordEntry.m_ndxNormalizedMapping.end());
					} else {
						unsigned int nCount = 0;
						for (int ndxAltWord = 0; ndxAltWord<wordEntry.m_lstAltWords.size(); ++ndxAltWord) {
							QString strAltWord = wordEntry.m_lstAltWords.at(ndxAltWord);
							if (!isAccentSensitive()) strAltWord = CSearchStringListModel::decompose(strAltWord);
							if (expCurWord.exactMatch(strAltWord)) {
								m_lstMatchMapping.insert(m_lstMatchMapping.end(),
															&wordEntry.m_ndxNormalizedMapping[nCount],
															&wordEntry.m_ndxNormalizedMapping[nCount+wordEntry.m_lstAltWordCount.at(ndxAltWord)]);
							}
							nCount += wordEntry.m_lstAltWordCount.at(ndxAltWord);
						}
					}
				}
			}
		} else {
			// Otherwise, match this word from our list from the last mapping and populate
			//		a list of remaining mappings:
			if (strCurWordWildKey.compare("*") != 0) {
				TIndexList lstNextMapping;
				for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
					if ((m_lstMatchMapping.at(ndxWord)+1) > m_pBibleDatabase->bibleEntry().m_nNumWrd) continue;
					QString strNextWord = (!isAccentSensitive() ? m_pBibleDatabase->decomposedWordAtIndex(m_lstMatchMapping.at(ndxWord)+1)
																: m_pBibleDatabase->wordAtIndex(m_lstMatchMapping.at(ndxWord)+1));
					if (expCurWord.exactMatch(strNextWord)) {
						lstNextMapping.push_back(m_lstMatchMapping.at(ndxWord)+1);
					}
				}
				m_lstMatchMapping = lstNextMapping;
			} else {
				// An "*" matches everything from the word before it, except for the "next index":
				for (TIndexList::iterator itrWord = m_lstMatchMapping.begin(); itrWord != m_lstMatchMapping.end(); /* increment is inside loop */) {
					if (((*itrWord) + 1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) {
						++(*itrWord);
						++itrWord;
					} else {
						itrWord = m_lstMatchMapping.erase(itrWord);
					}
				}
			}
		}

		if ((m_lstMatchMapping.size() != 0) || (bInFirstWordStar)) m_nLevel++;

		if (ndx < nCursorWord) {
			m_nCursorLevel = m_nLevel;

			if ((ndx+1) == nCursorWord) {			// Only build list of next words if we are at the last word before the cursor
				if (!bInFirstWordStar) {
					// Note: For some reason, adding to a QStringList and removing duplicates is
					//		faster than using !TConcordanceList.contains() to just not add them
					//		with initially and directly into m_lstNextWords.  Strange...
					//		This will use a little more memory, but...
					m_lstNextWords.clear();
					QStringList lstNextWords;
					for (unsigned int ndxWord=0; ndxWord<m_lstMatchMapping.size(); ++ndxWord) {
						if ((m_lstMatchMapping.at(ndxWord)+1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) {
							lstNextWords.append(m_pBibleDatabase->wordAtIndex(m_lstMatchMapping.at(ndxWord)+1));
						}
					}
					lstNextWords.removeDuplicates();

					m_lstNextWords.reserve(lstNextWords.size());
					for (int ndxWord = 0; ndxWord < lstNextWords.size(); ++ndxWord) {
						CConcordanceEntry nextWordEntry(lstNextWords.at(ndxWord));
						m_lstNextWords.append(nextWordEntry);
					}

					qSort(m_lstNextWords.begin(), m_lstNextWords.end(), TConcordanceListSortPredicate::ascendingLessThanWordCaseInsensitive);
					bComputedNextWords = true;
				} else {
					m_lstNextWords = m_pBibleDatabase->concordanceWordList();
					bComputedNextWords = true;
				}
			}
		}

		if ((m_lstMatchMapping.size() == 0) && (!bInFirstWordStar)) break;
	}

	// Copy our complete word list, but only if we didn't compute a wordList above:
	if (!bComputedNextWords)
		m_lstNextWords = m_pBibleDatabase->concordanceWordList();
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

inline QChar CPhraseCursor::charUnderCursor()
{
	return document()->characterAt(position());
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

void CPhraseNavigator::doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear, const CRelIndex &ndxCurrent) const
{
	assert(m_pBibleDatabase.data() != NULL);

	QTextCharFormat fmt;
	CPhraseCursor myCursor(&m_TextDocument);

	myCursor.beginEditBlock();

	CHighlighterPhraseTagFwdItr itrHighlighter = aHighlighter.getForwardIterator();
	while (!itrHighlighter.isEnd()) {
		TPhraseTag tag = itrHighlighter.nextTag();
		CRelIndex ndxRel = tag.relIndex();
		if (!ndxRel.isSet()) continue;
		// Save some time if the tag isn't anything close to what we are displaying.
		//		We'll use one before/one after since we might be displaying part of
		//		the proceding passage:
		if  ((ndxCurrent.isSet()) &&
				((ndxRel.book() < (ndxCurrent.book()-1)) ||
				 (ndxRel.book() > (ndxCurrent.book()+1)) ||
				 (ndxRel.chapter() < (ndxCurrent.chapter()-1)) ||
				 (ndxRel.chapter() > (ndxCurrent.chapter()+1)))) continue;

		int nStartPos = anchorPosition(ndxRel.asAnchor());
		if (nStartPos == -1) continue;				// Note: Some highlight lists have tags not in this browser document
		uint32_t ndxNormalStart = m_pBibleDatabase->NormalizeIndex(ndxRel);
		uint32_t ndxNormalEnd = ndxNormalStart;
		if (tag.count()) ndxNormalEnd = ndxNormalStart + tag.count() - 1;

		while ((nStartPos != -1) && (ndxNormalStart <= ndxNormalEnd)) {
			myCursor.setPosition(nStartPos);
			int nWordEndPos = nStartPos + m_pBibleDatabase->wordAtIndex(ndxNormalStart).size();

			while (nStartPos < nWordEndPos) {
				myCursor.moveCursorCharRight(QTextCursor::KeepAnchor);
				fmt = myCursor.charFormat();
				aHighlighter.doHighlighting(fmt, bClear);
				myCursor.setCharFormat(fmt);
				myCursor.clearSelection();
				++nStartPos;
			}

			++ndxNormalStart;
			nStartPos = anchorPosition(CRelIndex(m_pBibleDatabase->DenormalizeIndex(ndxNormalStart)).asAnchor());
		}
	}

	myCursor.endEditBlock();
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
		strHTML += m_pBibleDatabase->richVerseText(relPrev, m_richifierTags, !bNoAnchors) + "\n";
		strHTML += "</p>";

		// If we have a footnote for this book and this is the end of the last chapter,
		//		print it too:
		if (relPrev.chapter() == bookPrev.m_nNumChp) {
			const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(relPrev.book(),0,0,0));
			if (pFootnote) {
				if (!bNoAnchors) {
					strHTML += QString("<p><a id=\"%1\">%2</a><a id=\"X%3\">%4</a></p>\n")
									.arg(CRelIndex(relPrev.book(),0,0,0).asAnchor())
									.arg(pFootnote->text())
									.arg(CRelIndex(relPrev.book(),0,0,0).asAnchor())
									.arg(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
				} else {
					strHTML += QString("<p>%1</p>\n")
									.arg(pFootnote->text());
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
							.arg(book.m_strDesc);
		if  ((!book.m_strCat.isEmpty()) && (ndx.chapter() == 1))
			strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
							.arg(Qt::escape(tr("Category:")))
							.arg(book.m_strCat);
		// If we have a chapter note for this chapter, print it too:
		const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(),ndx.chapter(),0,0));
		if (pFootnote) {
			strHTML += QString("<div class=chapter>%1 %2</div>\n")
						.arg(Qt::escape(tr("Chapter")))
						.arg(ndx.chapter());
			strHTML += QString("<div class=subtitle>%1<a id=\"X%2\"> </a></div>\n")
						.arg(pFootnote->text())
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
							.arg(book.m_strDesc);
		if  ((!book.m_strCat.isEmpty()) && (ndx.chapter() == 1))
			strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
							.arg(Qt::escape(tr("Category:")))
							.arg(book.m_strCat);
		strHTML += QString("<div class=chapter>%1 %2</div>\n")
						.arg(Qt::escape(tr("Chapter")))
						.arg(ndx.chapter());
		// If we have a chapter note for this chapter, print it too:
		const CFootnoteEntry *pFootnote = m_pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(),ndx.chapter(),0,0));
		if (pFootnote) {
			strHTML += QString("<div class=subtitle>%1</div>\n")
						.arg(pFootnote->text());
		}
	}

	// Print this Chapter Text:
	bool bParagraph = false;
	CRelIndex ndxVerse;
	for (unsigned int ndxVrs=0; ndxVrs<pChapter->m_nNumVrs; ++ndxVrs) {
		ndxVerse = CRelIndex(ndx.book(), ndx.chapter(), ndxVrs+1, 0);
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndxVerse);
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
		strHTML += m_pBibleDatabase->richVerseText(ndxVerse, m_richifierTags, !bNoAnchors) + "\n";
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
								.arg(pFootnote->text())
								.arg(CRelIndex(ndx.book(),0,0,0).asAnchor())
								.arg(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			} else {
				strHTML += QString("<p>%1</p>\n")
								.arg(pFootnote->text());
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
									.arg(bookNext.m_strDesc);
				if  ((!bookNext.m_strCat.isEmpty()) && (relNext.chapter() == 1))
					strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
									.arg(Qt::escape(tr("Category:")))
									.arg(bookNext.m_strCat);
			} else {
				strHTML += QString("<div class=book>%1</div>\n")
								.arg(Qt::escape(bookNext.m_strBkName));
				if ((!bookNext.m_strDesc.isEmpty()) && (relNext.chapter() == 1))
					strHTML += QString("<div class=subtitle>(%1)</div>\n")
									.arg(bookNext.m_strDesc);
				if  ((!bookNext.m_strCat.isEmpty()) && (relNext.chapter() == 1))
					strHTML += QString("<div class=category><b>%1</b> %2</div>\n")
									.arg(Qt::escape(tr("Category:")))
									.arg(bookNext.m_strCat);
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
									.arg(pFootnote->text())
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
									.arg(pFootnote->text())
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
							.arg(pFootnote->text());
			}
		}

		strHTML += "<p>";
		if (!bNoAnchors) {
			strHTML += QString("<a id=\"%1\"><b> %2 </b></a>").arg(CRelIndex(relNext.book(), relNext.chapter(), relNext.verse(), 0).asAnchor()).arg(relNext.verse());
		} else {
			strHTML += QString("<b> %1 </b>").arg(relNext.verse());
		}
		strHTML += m_pBibleDatabase->richVerseText(relNext, m_richifierTags, !bNoAnchors) + "\n";
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
	strHTML += m_pBibleDatabase->richVerseText(ndx, m_richifierTags, !bNoAnchors) + "\n";

	strHTML += "</p></body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

void CPhraseNavigator::setDocumentToFormattedVerses(const TPhraseTag &tag)
{
	assert(m_pBibleDatabase.data() != NULL);

	m_TextDocument.clear();

	if ((!tag.relIndex().isSet()) || (tag.count() == 0)) {
		emit changedDocumentText();
		return;
	}

	CRelIndex ndxFirst = CRelIndex(tag.relIndex().book(), tag.relIndex().chapter(), tag.relIndex().verse(), 1);		// Start on first word of verse
	CRelIndex ndxLast = m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(tag.relIndex()) + tag.count() - 1);		// Add number of words to arrive at last word, wherever that is
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
										.arg(m_pBibleDatabase->bookName(ndxFirst))
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse());
			} else {
				strReference = QString("(%1 %2:%3-%4)")
										.arg(m_pBibleDatabase->bookName(ndxFirst))
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse())
										.arg(ndxLast.verse());
			}
		} else {
			strReference = QString("(%1 %2:%3-%4:%5)")
									.arg(m_pBibleDatabase->bookName(ndxFirst))
									.arg(ndxFirst.chapter())
									.arg(ndxFirst.verse())
									.arg(ndxLast.chapter())
									.arg(ndxLast.verse());
		}
	} else {
		strReference = QString("(%1 %2:%3-%4 %5:%6)")
								.arg(m_pBibleDatabase->bookName(ndxFirst))
								.arg(ndxFirst.chapter())
								.arg(ndxFirst.verse())
								.arg(m_pBibleDatabase->bookName(ndxLast))
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

		strHTML += m_pBibleDatabase->richVerseText(ndx, m_richifierTags, false);

		ndxPrev = ndx;
	}

	strHTML += "&quot;</p></body></html>";
	m_TextDocument.setHtml(strHTML);
	emit changedDocumentText();
}

TPhraseTag CPhraseNavigator::getSelection(const CPhraseCursor &aCursor) const
{
	assert(m_pBibleDatabase.data() != NULL);

	TPhraseTag tag;

	CPhraseCursor myCursor(aCursor);
	myCursor.beginEditBlock();
	int nPosFirst = qMin(myCursor.anchor(), myCursor.position());
	int nPosLast = qMax(myCursor.anchor(), myCursor.position());
	int nPosFirstWordStart = nPosFirst;
	CRelIndex nIndexFirst;				// First Word anchor tag
	CRelIndex nIndexLast;				// Last Word anchor tag
	QString strAnchorName;

	// Find first word anchor:
	myCursor.setPosition(nPosFirst);
	// See if our first character is a space.  If so, don't include it because it's
	//		most likely the single space between words.  If so, we would end up
	//		starting the selection at the preceding word and while that's technically
	//		correct, it's confusing to the user who probably didn't mean for that
	//		to happen:
	myCursor.moveCursorWordStart();
	nPosFirstWordStart = myCursor.position();
	while ((myCursor.position() <= (nPosLast+1)) && (!nIndexFirst.isSet())) {
		strAnchorName = myCursor.charFormat().anchorName();

		nIndexFirst = CRelIndex(strAnchorName);
		// If we haven't hit an anchor for an actual word within a verse, we can't be selecting
		//		text from a verse.  We must be in a special tag section of heading:
		if ((nIndexFirst.verse() == 0) || (nIndexFirst.word() == 0)) {
			nIndexFirst = CRelIndex();
		}
		if (!myCursor.moveCursorCharRight()) break;
	}

	// Find last word anchor:
	myCursor.setPosition(nPosLast);
	while ((myCursor.moveCursorCharLeft()) && (myCursor.charUnderCursor().isSpace()));	// Note: Always move left at least one character so we don't pickup the start of the next word (short-circuit order!)
	myCursor.moveCursorWordEnd();
	while ((myCursor.position() >= nPosFirstWordStart) && (!nIndexLast.isSet())) {
		strAnchorName = myCursor.charFormat().anchorName();

		nIndexLast = CRelIndex(strAnchorName);
		// If we haven't hit an anchor for an actual word within a verse, we can't be selecting
		//		text from a verse.  We must be in a special tag section of heading:
		if ((nIndexLast.verse() == 0) || (nIndexLast.word() == 0)) {
			nIndexLast = CRelIndex();
		}
		if (!myCursor.moveCursorCharLeft()) break;
	}

	// Handle single-word selection:
	if (!nIndexLast.isSet()) nIndexLast = nIndexFirst;

	// If the cursor is floating in "no man's land" in a special tag area or footnote text or
	//		something, then find the closest matching tag to the left.  This is the same as
	//		the current position tracking:
	if (!nIndexFirst.isSet()) {
		myCursor.setPosition(nPosFirst);
		while (!nIndexFirst.isSet()) {
			nIndexFirst = CRelIndex(myCursor.charFormat().anchorName());
			if (!myCursor.moveCursorCharLeft()) break;
		}
	}

	myCursor.endEditBlock();

	uint32_t ndxNormFirst = m_pBibleDatabase->NormalizeIndex(nIndexFirst);
	uint32_t ndxNormLast = m_pBibleDatabase->NormalizeIndex(nIndexLast);
	unsigned int nWordCount = 0;

	if ((ndxNormFirst != 0) && (ndxNormLast != 0)) {
		nWordCount = (ndxNormLast - ndxNormFirst + 1);
	}

	tag.relIndex() = nIndexFirst;
	tag.count() = ((nPosFirst != nPosLast) ? nWordCount : 0);

	return tag;
}

CSelectedPhrase CPhraseNavigator::getSelectedPhrase(const CPhraseCursor &aCursor) const
{
	assert(m_pBibleDatabase.data() != NULL);

	CSelectedPhrase retVal(m_pBibleDatabase);

	retVal.tag() = getSelection(aCursor);

	uint32_t ndxNormFirst = m_pBibleDatabase->NormalizeIndex(retVal.tag().relIndex());
	uint32_t ndxNormLast = ndxNormFirst;
	if (retVal.tag().count()) {
		ndxNormLast = m_pBibleDatabase->NormalizeIndex(ndxNormFirst + retVal.tag().count() - 1);
	}

	QString strPhrase;
	QStringList lstPhrase;
	if ((ndxNormFirst != 0) && (ndxNormLast != 0)) {
		lstPhrase.reserve(ndxNormLast - ndxNormFirst + 1);
		do {
			lstPhrase.append(m_pBibleDatabase->wordAtIndex(ndxNormFirst));
			ndxNormFirst++;
		} while (ndxNormFirst <= ndxNormLast);
		strPhrase = lstPhrase.join(" ");
	}

	retVal.phrase().ParsePhrase(lstPhrase);

//	qDebug("%s", strPhrase.toUtf8().data());
//	qDebug("%s %d", m_pBibleDatabase->PassageReferenceText(nIndexFirst).toUtf8().data(), retVal.tag().count());

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

	CRelIndex ndxScroll = tag.relIndex();
	if (ndxScroll.verse() == 1) ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading
	ndxScroll.setWord(0);

	m_TextEditor.scrollToAnchor(ndxScroll.asAnchor());

	CRelIndex ndxRel = tag.relIndex();
	if (ndxRel.isSet()) {
		int nStartPos = anchorPosition(ndxRel.asAnchor());
		int nEndPos = anchorPosition(CRelIndex(m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(ndxRel) + tag.count() - 1)).asAnchor());

		if (nStartPos != -1) {
			CPhraseCursor myCursor(m_TextEditor.textCursor());
			myCursor.beginEditBlock();
			myCursor.setPosition(nStartPos);
			if ((nEndPos != -1) && (tag.count() > 0)) {
				myCursor.setPosition(nEndPos, QTextCursor::KeepAnchor);
				myCursor.moveCursorWordEnd(QTextCursor::KeepAnchor);
			}
			myCursor.endEditBlock();
			m_TextEditor.setTextCursor(myCursor);
		}
		m_TextEditor.ensureCursorVisible();				// Hmmm, for some strange reason, this doen't always work when user has used mousewheel to scroll off.  Qt bug?
	}
}

TPhraseTag CPhraseEditNavigator::getSelection() const
{
	return getSelection(m_TextEditor.textCursor());
}

CSelectedPhrase CPhraseEditNavigator::getSelectedPhrase() const
{
	return getSelectedPhrase(m_TextEditor.textCursor());
}

bool CPhraseEditNavigator::handleToolTipEvent(const QHelpEvent *pHelpEvent, CCursorFollowHighlighter &aHighlighter, const TPhraseTag &selection) const
{
	assert(m_pBibleDatabase.data() != NULL);

	assert(pHelpEvent != NULL);
	CRelIndex ndxReference = getSelection(m_TextEditor.cursorForPosition(pHelpEvent->pos())).relIndex();
	QString strToolTip = getToolTip(TPhraseTag(ndxReference, 1), selection);

	if (!strToolTip.isEmpty()) {
		highlightCursorFollowTag(aHighlighter, (selection.haveSelection() ? selection : TPhraseTag(ndxReference, 1)));
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(pHelpEvent->globalPos(), strToolTip, &m_TextEditor);
		} else {
			QToolTip::showText(pHelpEvent->globalPos(), strToolTip);
		}
	} else {
		highlightCursorFollowTag(aHighlighter);
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

bool CPhraseEditNavigator::handleToolTipEvent(CCursorFollowHighlighter &aHighlighter, const TPhraseTag &tag, const TPhraseTag &selection) const
{
	assert(m_pBibleDatabase.data() != NULL);

	QString strToolTip = getToolTip(tag, selection);

	if (!strToolTip.isEmpty()) {
		highlightCursorFollowTag(aHighlighter, (selection.haveSelection() ? selection : TPhraseTag(tag.relIndex(), 1)));
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip, m_TextEditor.viewport(), m_TextEditor.rect());
		} else {
			QToolTip::showText(m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip);
		}
	} else {
		highlightCursorFollowTag(aHighlighter);
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

void CPhraseEditNavigator::highlightCursorFollowTag(CCursorFollowHighlighter &aHighlighter, const TPhraseTag &tag) const
{
	assert(m_pBibleDatabase.data() != NULL);

	doHighlighting(aHighlighter, true);
	TPhraseTagList tags;
	// Highlight the word only if we have a reference for an actual word (not just a chapter or book or something):
	if ((tag.relIndex().book() != 0) &&
		(tag.relIndex().chapter() != 0) &&
		(tag.relIndex().verse() != 0) &&
		(tag.relIndex().word() != 0) &&
		(tag.count() != 0)) {
		tags.append(tag);
		aHighlighter.setPhraseTags(tags);
		doHighlighting(aHighlighter);
	} else {
		aHighlighter.clearPhraseTags();
	}
}

QString CPhraseEditNavigator::getToolTip(const TPhraseTag &tag, const TPhraseTag &selection, TOOLTIP_TYPE_ENUM nToolTipType, bool bPlainText) const
{
	assert(m_pBibleDatabase.data() != NULL);

	bool bHaveSelection = selection.haveSelection();
	const CRelIndex &ndxReference(bHaveSelection ? selection.relIndex() : tag.relIndex());

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
					for (ndx = 0; ((ndx < qMin(7u, selection.count())) && ((ndxNormal + ndx) <= m_pBibleDatabase->bibleEntry().m_nNumWrd)); ++ndx) {
						if (ndx) strToolTip += " ";
						strToolTip += m_pBibleDatabase->wordAtIndex(ndxNormal + ndx);
					}
					if ((ndx == 7u) && (selection.count() > 7u)) strToolTip += " ...";
				} else {
					assert(false);
					strToolTip += "???";
				}
				strToolTip += "\"\n";
				strToolTip += m_pBibleDatabase->SearchResultToolTip(ndxReference, RIMASK_ALL, selection.count());
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
				strToolTip += "\n" + tr("%n Word(s) Selected", NULL, selection.count()) + "\n";
			}
		}
		if (!bPlainText) strToolTip += "</pre></body></html>";
	}

	return strToolTip;
}

// ============================================================================

