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
#include "ParseSymbols.h"
#include "VerseRichifier.h"
#include "SearchCompleter.h"
#include "UserNotesDatabase.h"
#include "ScriptureDocument.h"
#include "PersistentSettings.h"
#if !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)
#include "ToolTipEdit.h"
#include "myApplication.h"
#endif
#include "Translator.h"

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

static QString makeRawPhrase(const QString &strPhrase)
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

// ============================================================================

CSubPhrase::CSubPhrase()
	:	m_nLevel(0),
		m_nCursorLevel(0),
		m_nCursorWord(-1)
{

}

CSubPhrase::~CSubPhrase()
{

}

int CSubPhrase::GetMatchLevel() const
{
	return m_nLevel;
}

int CSubPhrase::GetCursorMatchLevel() const
{
	return m_nCursorLevel;
}

QString CSubPhrase::GetCursorWord() const
{
	return m_strCursorWord;
}

int CSubPhrase::GetCursorWordPos() const
{
	return m_nCursorWord;
}

QString CSubPhrase::phrase() const
{
	return phraseWords().join(" ");
}

QString CSubPhrase::phraseRaw() const
{
	return phraseWordsRaw().join(" ");
}

int CSubPhrase::phraseSize() const
{
	return phraseWords().size();
}

int CSubPhrase::phraseRawSize() const
{
	return phraseWordsRaw().size();
}

QStringList CSubPhrase::phraseWords() const
{
	QStringList strPhraseWords;
	strPhraseWords.reserve(m_lstWords.size());

	for (int ndx = 0; ndx < m_lstWords.size(); ++ndx) {
		if (!m_lstWords.at(ndx).isEmpty()) strPhraseWords.append(m_lstWords.at(ndx));
	}
	return strPhraseWords;
}

QStringList CSubPhrase::phraseWordsRaw() const
{
	QStringList strPhraseWords = phraseWords();
	for (int ndx = (strPhraseWords.size()-1); ndx >= 0; --ndx) {
		QString strTemp = makeRawPhrase(strPhraseWords.at(ndx));
		if (strTemp.isEmpty()) {
			strPhraseWords.removeAt(ndx);
		} else {
			strPhraseWords[ndx] = strTemp;
		}
	}
	return strPhraseWords;
}

bool CSubPhrase::isCompleteMatch() const
{
	return (GetMatchLevel() == phraseSize());
}

unsigned int CSubPhrase::GetNumberOfMatches() const
{
	return m_lstMatchMapping.size();
}

void CSubPhrase::ParsePhrase(const QString &strPhrase)
{
	m_lstWords = strPhrase.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), QString::SkipEmptyParts);
	m_strCursorWord.clear();
	m_nCursorWord = m_lstWords.size();
}

void CSubPhrase::ParsePhrase(const QStringList &lstPhrase)
{
	m_lstWords = lstPhrase;
	m_strCursorWord.clear();
	m_nCursorWord = m_lstWords.size();
}

// ============================================================================

CParsedPhrase::CParsedPhrase(CBibleDatabasePtr pBibleDatabase, bool bCaseSensitive, bool bAccentSensitive, bool bExclude)
	:	m_pBibleDatabase(pBibleDatabase),
		m_bIsDuplicate(false),
		m_bIsDisabled(false),
		m_bCaseSensitive(bCaseSensitive),
		m_bAccentSensitive(bAccentSensitive),
		m_bExclude(bExclude),
		m_nActiveSubPhrase(-1)
{

}

CParsedPhrase::~CParsedPhrase()
{

}

// ============================================================================

bool CPhraseEntry::operator==(const CParsedPhrase &src) const
{
	return ((m_bCaseSensitive == src.isCaseSensitive()) &&
			(m_bAccentSensitive == src.isAccentSensitive()) &&
			(m_bExclude == src.isExcluded()) &&
			(m_strPhrase.compare(src.phrase(), Qt::CaseSensitive) == 0));
}
bool CPhraseEntry::operator!=(const CParsedPhrase &src) const
{
	return (!(operator==(src)));
}

// ============================================================================

const TConcordanceList &CParsedPhrase::nextWordsList() const
{
	static const TConcordanceList lstEmptyConcordance;		// Fallback

	if ((m_nActiveSubPhrase >= 0) && (m_nActiveSubPhrase < m_lstSubPhrases.size())) {
		return m_lstSubPhrases.at(m_nActiveSubPhrase)->m_lstNextWords;
	}

	if (m_pBibleDatabase != NULL)
		return m_pBibleDatabase->concordanceWordList();

	return lstEmptyConcordance;
}

bool CParsedPhrase::atEndOfSubPhrase() const
{
	if (m_nActiveSubPhrase < 0) return true;

	assert((m_nActiveSubPhrase >=0) && (m_nActiveSubPhrase < m_lstSubPhrases.size()));
	return (m_lstSubPhrases.at(m_nActiveSubPhrase)->GetCursorWordPos() == m_lstSubPhrases.at(m_nActiveSubPhrase)->m_lstWords.size());
}

// ============================================================================

bool CParsedPhrase::isCompleteMatch() const
{
	bool bRetVal = false;
	for (int ndx = 0; ndx < m_lstSubPhrases.size(); ++ndx) {
		bRetVal = bRetVal || m_lstSubPhrases.at(ndx)->isCompleteMatch();
	}

	return bRetVal;
}

unsigned int CParsedPhrase::GetNumberOfMatches() const
{
	return GetPhraseTagSearchResults().size();
}

const TPhraseTagList &CParsedPhrase::GetPhraseTagSearchResults() const
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_cache_lstPhraseTagResults.size()) return m_cache_lstPhraseTagResults;
	if (m_lstSubPhrases.size() == 0) return m_cache_lstPhraseTagResults;

	// Find the overall largest reserve size to calculate and find the index of
	//		the subphrase with the largest number of results.  We'll start with
	//		the largest result and intersect insert the other results into it.
	//		(that should be fastest)
	int nMaxSize = -1;
	int ndxMax = 0;
	int nReserveSize = 0;
	for (int ndxSubPhrase = 0; ndxSubPhrase < m_lstSubPhrases.size(); ++ndxSubPhrase) {
		int nSize = m_lstSubPhrases.at(ndxSubPhrase)->m_lstMatchMapping.size();
		nReserveSize += nSize;
		if (nSize > nMaxSize) {
			nMaxSize = nSize;
			ndxMax = ndxSubPhrase;
		}
	}

	// Insert our Max list (or only list for single subphrases) first:
	m_cache_lstPhraseTagResults.reserve(nReserveSize);
	const CSubPhrase *subPhrase = m_lstSubPhrases.at(ndxMax).data();
	for (unsigned int ndxWord=0; ndxWord<subPhrase->m_lstMatchMapping.size(); ++ndxWord) {
		uint32_t ndxNormal = (subPhrase->m_lstMatchMapping.at(ndxWord) - subPhrase->m_nLevel + 1);
		if (ndxNormal > 0)
			m_cache_lstPhraseTagResults.append(TPhraseTag(CRelIndex(m_pBibleDatabase->DenormalizeIndex(ndxNormal)), subPhrase->phraseSize()));
	}
	qSort(m_cache_lstPhraseTagResults.begin(), m_cache_lstPhraseTagResults.end(), TPhraseTagListSortPredicate::ascendingLessThan);

	// Intersecting insert the other subphrases:
	for (int ndxSubPhrase = 0; ndxSubPhrase < m_lstSubPhrases.size(); ++ndxSubPhrase) {
		if (ndxSubPhrase == ndxMax) continue;
		subPhrase = m_lstSubPhrases.at(ndxSubPhrase).data();

		TPhraseTagList lstSubPhraseTags;
		lstSubPhraseTags.reserve(subPhrase->m_lstMatchMapping.size());
		for (unsigned int ndxWord=0; ndxWord<subPhrase->m_lstMatchMapping.size(); ++ndxWord) {
			uint32_t ndxNormal = (subPhrase->m_lstMatchMapping.at(ndxWord) - subPhrase->m_nLevel + 1);
			if (ndxNormal > 0) lstSubPhraseTags.append(TPhraseTag(CRelIndex(m_pBibleDatabase->DenormalizeIndex(ndxNormal)), subPhrase->phraseSize()));
		}
		qSort(lstSubPhraseTags.begin(), lstSubPhraseTags.end(), TPhraseTagListSortPredicate::ascendingLessThan);

		m_cache_lstPhraseTagResults.intersectingInsert(m_pBibleDatabase.data(), lstSubPhraseTags);
	}

	// Note: Results of the intersectingInsert are already sorted

	return m_cache_lstPhraseTagResults;
}

QString CParsedPhrase::GetCursorWord() const
{
	if (m_nActiveSubPhrase < 0) return QString();
	assert(m_nActiveSubPhrase < m_lstSubPhrases.size());

	return m_lstSubPhrases.at(m_nActiveSubPhrase)->GetCursorWord();
}

int CParsedPhrase::GetCursorWordPos() const
{
	if (m_nActiveSubPhrase < 0) return -1;
	assert(m_nActiveSubPhrase < m_lstSubPhrases.size());

	int posWord = 0;
	for (int ndxSubPhrase=0; ndxSubPhrase<m_nActiveSubPhrase; ++ndxSubPhrase) {
		posWord += m_lstSubPhrases.at(ndxSubPhrase)->m_lstWords.size() + 1;		// +1 for "word between phrases"
	}
	posWord += m_lstSubPhrases.at(m_nActiveSubPhrase)->GetCursorWordPos();

	return posWord;
}

QString CParsedPhrase::phrase() const
{
	QStringList lstPhrases;

	lstPhrases.reserve(m_lstSubPhrases.size());
	for (int ndx = 0; ndx < m_lstSubPhrases.size(); ++ndx) {
		lstPhrases.append(m_lstSubPhrases.at(ndx)->phrase());
	}

	return lstPhrases.join(" | ");
}

QString CParsedPhrase::phraseRaw() const
{
	QStringList lstPhrases;

	lstPhrases.reserve(m_lstSubPhrases.size());
	for (int ndx = 0; ndx < m_lstSubPhrases.size(); ++ndx) {
		lstPhrases.append(m_lstSubPhrases.at(ndx)->phraseRaw());
	}

	return lstPhrases.join(" | ");
}

const QStringList CParsedPhrase::phraseWords() const
{
	return phrase().split(QRegExp("\\s+"), QString::SkipEmptyParts);
}

const QStringList CParsedPhrase::phraseWordsRaw() const
{
	return phraseRaw().split(QRegExp("\\s+"), QString::SkipEmptyParts);
}

void CParsedPhrase::clearCache() const
{
	m_cache_lstPhraseTagResults = TPhraseTagList();
}

void CParsedPhrase::UpdateCompleter(const QTextCursor &curInsert, CSearchCompleter &aCompleter)
{
	ParsePhrase(curInsert);
	FindWords();
	aCompleter.setFilterMatchString();
	aCompleter.setWordsFromPhrase();
}

QTextCursor CParsedPhrase::insertCompletion(const QTextCursor &curInsert, const QString& completion)
{
	CPhraseCursor myCursor(curInsert);
	myCursor.beginEditBlock();
	myCursor.clearSelection();
	if (!atEndOfSubPhrase()) {
		myCursor.selectWordUnderCursor();							// Select word under the cursor
		myCursor.insertText(completion);							// Replace with completed word
	} else {
		// If at the end of the phrase, just insert it and add
		//		a space if the subphrase isn't the last subphrase
		//		(i.e. we are just before the "|"):
		myCursor.insertText(completion + ((m_nActiveSubPhrase != (m_lstSubPhrases.size()-1)) ? QString(" ") : QString()));
	}
	myCursor.endEditBlock();

	return myCursor;
}

void CParsedPhrase::ParsePhrase(const QTextCursor &curInsert, bool bFindWords)
{
	// Note: clearCache() called in secondary ParsePhrase() call below
	//		once we've parsed the cursor into a string

	CPhraseCursor myCursor(curInsert);
	int nCursorPos = myCursor.position();

	myCursor.setPosition(0);
	myCursor.selectCursorToLineEnd();
	QString strComplete = myCursor.selectedText();

	assert(nCursorPos <= strComplete.size());

	QString strLeftText = strComplete.left(nCursorPos);
	QString strRightText = strComplete.mid(nCursorPos);

	QChar chrL = (strLeftText.size() ? strLeftText.at(strLeftText.size()-1) : QChar());
	bool bLIsSpace = chrL.isSpace();
	bool bLIsOR = (chrL == QChar('|'));
	bool bLIsSeparator = (bLIsSpace || bLIsOR);

	if (chrL.isNull() || bLIsSeparator) strRightText = strRightText.mid(strRightText.indexOf(QRegExp("\\S")));

	QChar chrR = (strRightText.size() ? strRightText.at(0) : QChar());
	bool bRIsSpace = chrR.isSpace();
	bool bRIsOR = (chrR == QChar('|'));
	bool bRIsSeparator = (bRIsSpace || bRIsOR);

	if (!bRIsSeparator) {
		int nPosRSpace = strRightText.indexOf(QRegExp("\\s+"));
		strLeftText += strRightText.left(nPosRSpace);
		if (nPosRSpace != -1) {
			strRightText = strRightText.mid(nPosRSpace);
		} else {
			strRightText.clear();
		}
	}

	ParsePhrase(strComplete);
	assert(m_lstSubPhrases.size() > 0);

	strComplete.replace(QString("|"), QString(" | "));		// Make sure we have separation around the "OR" operators so we break them into individual elements below...
	QStringList lstCompleteWords = strComplete.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), QString::SkipEmptyParts);

	strLeftText.replace(QString("|"), QString(" | "));		// Make sure we have separation around the "OR" operators so we break them into individual elements below...
	QStringList lstLeftWords = strLeftText.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), QString::SkipEmptyParts);

	strRightText.replace(QString("|"), QString(" | "));		// Make sure we have separation around the "OR" operators so we break them into individual elements below...
	QStringList lstRightWords = strRightText.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), QString::SkipEmptyParts);

	assert(lstCompleteWords.size() == (lstLeftWords.size() + lstRightWords.size()));

	int nCursorWord = (lstLeftWords.size() ? lstLeftWords.size()-1 : 0);
	QString strCursorWord = (lstLeftWords.size() ? lstLeftWords.last() : QString());
	if (((bLIsSpace) && ((bRIsSeparator) || chrR.isNull())) ||
		(strCursorWord.contains(QChar('|')))) {
		strCursorWord.clear();
		nCursorWord++;
	}

	m_nActiveSubPhrase = -1;
	for (int ndxSubPhrase = 0; ndxSubPhrase < m_lstSubPhrases.size(); ++ndxSubPhrase) {
		if (nCursorWord <= m_lstSubPhrases.at(ndxSubPhrase)->m_lstWords.size()) {
			m_lstSubPhrases[ndxSubPhrase]->m_nCursorWord = nCursorWord;
			if (nCursorWord < m_lstSubPhrases.at(ndxSubPhrase)->m_lstWords.size()) {
				m_lstSubPhrases[ndxSubPhrase]->m_strCursorWord = m_lstSubPhrases.at(ndxSubPhrase)->m_lstWords.at(nCursorWord);
			} else {
				m_lstSubPhrases[ndxSubPhrase]->m_strCursorWord = QString();
			}
			m_nActiveSubPhrase = ndxSubPhrase;
			break;
		} else {
			nCursorWord -= (m_lstSubPhrases.at(ndxSubPhrase)->m_lstWords.size() + 1);	// +1 for "word between subprases"
		}
	}
	// Note: It's possible to have not hit an ActiveSubPhrase above if the user had
	//		a single word or phrase, selects the whole phrase, and then presses space.
	//		That causes us to have a cursor character and go through this parsing logic,
	//		but we have only one subPhrase and no words.  It should only be true that
	//		nCursorWord is 0 and that there is only one phrase (though I'm not explicitly
	//		requiring that below) and that phrase should have no words in it.
	//		Also, we know we have at least one SubPhrase in our list due to the assert
	//		above after the call to ParsePhrase(strComplete), so there's no need to
	//		recheck that:
	if (m_nActiveSubPhrase == -1) {
		assert(nCursorWord == 0);		// If we aren't at a non-existent word at the end of an empty phrase, then something went wrong above
		m_nActiveSubPhrase = m_lstSubPhrases.size()-1;
		if (nCursorWord < m_lstSubPhrases[m_nActiveSubPhrase]->m_lstWords.size()) {
			// I don't think this case can ever happen, as we should have set the word in the loop above:
			assert(false);
			m_lstSubPhrases[m_nActiveSubPhrase]->m_strCursorWord = m_lstSubPhrases.at(m_nActiveSubPhrase)->m_lstWords.at(nCursorWord);
		} else {
			m_lstSubPhrases[m_nActiveSubPhrase]->m_strCursorWord = QString();
		}
		m_lstSubPhrases[m_nActiveSubPhrase]->m_nCursorWord = nCursorWord;
	}

	if (bFindWords) FindWords();
}

void CParsedPhrase::ParsePhrase(const QString &strPhrase)
{
	clearCache();

	QStringList lstPhrases = strPhrase.split(QChar('|'));
	assert(lstPhrases.size() >= 1);

	m_lstSubPhrases.clear();
	m_lstSubPhrases.reserve(lstPhrases.size());
	for (int ndx=0; ndx<lstPhrases.size(); ++ndx) {
		QSharedPointer<CSubPhrase> subPhrase(new CSubPhrase);
		if (m_pBibleDatabase != NULL) subPhrase->m_lstNextWords = m_pBibleDatabase->concordanceWordList();
		subPhrase->ParsePhrase(lstPhrases.at(ndx));
		m_lstSubPhrases.append(subPhrase);
	}

	m_nActiveSubPhrase = m_lstSubPhrases.size()-1;
}

void CParsedPhrase::ParsePhrase(const QStringList &lstPhrase)
{
	clearCache();

	m_lstSubPhrases.clear();

	int ndxFrom = 0;
	while (ndxFrom != -1) {
		int ndxFirst = ndxFrom;
		ndxFrom = lstPhrase.indexOf(QString("|"), ndxFrom);
		int ndxLast = ((ndxFrom != -1) ? ndxFrom : lstPhrase.size());
		QStringList lstSubPhrase;
		for (int ndxWord = ndxFirst; ndxWord < ndxLast; ++ndxWord) {
			lstSubPhrase.append(lstPhrase.at(ndxWord));
		}
		if (!lstSubPhrase.isEmpty()) {
			QSharedPointer<CSubPhrase> subPhrase(new CSubPhrase);
			if (m_pBibleDatabase != NULL) subPhrase->m_lstNextWords = m_pBibleDatabase->concordanceWordList();
			subPhrase->ParsePhrase(lstSubPhrase);
			m_lstSubPhrases.append(subPhrase);
		}
		if (ndxFrom != -1) ++ndxFrom;			// Skip the 'OR'
	}

	m_nActiveSubPhrase = m_lstSubPhrases.size()-1;
}

void CParsedPhrase::FindWords()
{
	for (int ndxSubPhrase = 0; ndxSubPhrase < m_lstSubPhrases.size(); ++ndxSubPhrase)
		FindWords(*m_lstSubPhrases[ndxSubPhrase]);
}

void CParsedPhrase::FindWords(CSubPhrase &subPhrase)
{
	assert(m_pBibleDatabase.data() != NULL);

	int nCursorWord = subPhrase.m_nCursorWord;
	assert((nCursorWord >= 0) && (nCursorWord <= subPhrase.m_lstWords.size()));

	subPhrase.m_lstMatchMapping.clear();
	bool bComputedNextWords = false;
	subPhrase.m_nLevel = 0;
	subPhrase.m_nCursorLevel = 0;
	bool bInFirstWordStar = false;
	for (int ndx=0; ndx<subPhrase.m_lstWords.size(); ++ndx) {
		if (subPhrase.m_lstWords.at(ndx).isEmpty()) continue;

		QString strCurWordDecomp = CSearchStringListModel::decompose(subPhrase.m_lstWords.at(ndx), true);
		QString strCurWord = (isAccentSensitive() ? CSearchStringListModel::deApostrHyphen(subPhrase.m_lstWords.at(ndx), !m_pBibleDatabase->settings().hyphenSensitive()) :
													CSearchStringListModel::decompose(subPhrase.m_lstWords.at(ndx), !m_pBibleDatabase->settings().hyphenSensitive()));

		QString strCurWordKey = strCurWordDecomp.toLower();
		QString strCurWordWildKey = strCurWordKey;			// Note: This becomes the "Word*" value later, so can't substitute strCurWordWild for all m_lstWords.at(ndx) (or strCurWord)
		int nPreRegExp = strCurWordWildKey.indexOf(QRegExp("[\\[\\]\\*\\?]"));

		if (nPreRegExp == -1) {
			if ((ndx == (subPhrase.m_lstWords.size()-1)) &&
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
					subPhrase.m_lstNextWords.clear();
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

					if ((!isCaseSensitive()) && (!isAccentSensitive()) && (!m_pBibleDatabase->settings().hyphenSensitive())) {
						subPhrase.m_lstMatchMapping.insert(subPhrase.m_lstMatchMapping.end(), wordEntry.m_ndxNormalizedMapping.begin(), wordEntry.m_ndxNormalizedMapping.end());
					} else {
						unsigned int nCount = 0;
						for (int ndxAltWord = 0; ndxAltWord<wordEntry.m_lstAltWords.size(); ++ndxAltWord) {
							QString strAltWord = wordEntry.m_lstAltWords.at(ndxAltWord);
							if (!isAccentSensitive()) {
								strAltWord = CSearchStringListModel::decompose(strAltWord, !m_pBibleDatabase->settings().hyphenSensitive());
							} else{
								strAltWord = CSearchStringListModel::deApostrHyphen(strAltWord, !m_pBibleDatabase->settings().hyphenSensitive());
							}
							if (expCurWord.exactMatch(strAltWord)) {
								subPhrase.m_lstMatchMapping.insert(subPhrase.m_lstMatchMapping.end(),
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
				TNormalizedIndexList lstNextMapping;
				for (unsigned int ndxWord=0; ndxWord<subPhrase.m_lstMatchMapping.size(); ++ndxWord) {
					if ((subPhrase.m_lstMatchMapping.at(ndxWord)+1) > m_pBibleDatabase->bibleEntry().m_nNumWrd) continue;
					QString strNextWord;
					if ((!isAccentSensitive()) && (!m_pBibleDatabase->settings().hyphenSensitive())) {
						strNextWord = m_pBibleDatabase->decomposedWordAtIndex(subPhrase.m_lstMatchMapping.at(ndxWord)+1);
					} else if (!isAccentSensitive()) {
						strNextWord = CSearchStringListModel::decompose(m_pBibleDatabase->wordAtIndex(subPhrase.m_lstMatchMapping.at(ndxWord)+1, false), !m_pBibleDatabase->settings().hyphenSensitive());
					} else {
						strNextWord = CSearchStringListModel::deApostrHyphen(m_pBibleDatabase->wordAtIndex(subPhrase.m_lstMatchMapping.at(ndxWord)+1, false), !m_pBibleDatabase->settings().hyphenSensitive());
					}
					if (expCurWord.exactMatch(strNextWord)) {
						lstNextMapping.push_back(subPhrase.m_lstMatchMapping.at(ndxWord)+1);
					}
				}
				subPhrase.m_lstMatchMapping = lstNextMapping;
			} else {
				// An "*" matches everything from the word before it, except for the "next index":
				for (TNormalizedIndexList::iterator itrWord = subPhrase.m_lstMatchMapping.begin(); itrWord != subPhrase.m_lstMatchMapping.end(); /* increment is inside loop */) {
					if (((*itrWord) + 1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) {
						++(*itrWord);
						++itrWord;
					} else {
						itrWord = subPhrase.m_lstMatchMapping.erase(itrWord);
					}
				}
			}
		}

		if ((subPhrase.m_lstMatchMapping.size() != 0) || (bInFirstWordStar)) subPhrase.m_nLevel++;

		if (ndx < nCursorWord) {
			subPhrase.m_nCursorLevel = subPhrase.m_nLevel;

			if ((ndx+1) == nCursorWord) {			// Only build list of next words if we are at the last word before the cursor
				if (!bInFirstWordStar) {
					// Note: For some reason, adding to a QStringList and removing duplicates is
					//		faster than using !TConcordanceList.contains() to just not add them
					//		with initially and directly into m_lstNextWords.  Strange...
					//		This will use a little more memory, but...
					subPhrase.m_lstNextWords.clear();
					QStringList lstNextWords;
					for (unsigned int ndxWord=0; ndxWord<subPhrase.m_lstMatchMapping.size(); ++ndxWord) {
						if ((subPhrase.m_lstMatchMapping.at(ndxWord)+1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) {
							int nConcordanceIndex = m_pBibleDatabase->concordanceIndexForWordAtIndex(subPhrase.m_lstMatchMapping.at(ndxWord)+1);
							assert(nConcordanceIndex != -1);
							lstNextWords.append(QString("%1").arg(nConcordanceIndex));
						}
					}
					lstNextWords.removeDuplicates();

					subPhrase.m_lstNextWords.reserve(lstNextWords.size());
					for (int ndxWord = 0; ndxWord < lstNextWords.size(); ++ndxWord) {
						const CConcordanceEntry &nextWordEntry(m_pBibleDatabase->concordanceWordList().at(lstNextWords.at(ndxWord).toInt()));
						subPhrase.m_lstNextWords.append(nextWordEntry);
					}

					qSort(subPhrase.m_lstNextWords.begin(), subPhrase.m_lstNextWords.end(), TConcordanceListSortPredicate::ascendingLessThanWordCaseInsensitive);
					bComputedNextWords = true;
				} else {
					subPhrase.m_lstNextWords = m_pBibleDatabase->concordanceWordList();
					bComputedNextWords = true;
				}
			}
		}

		if ((subPhrase.m_lstMatchMapping.size() == 0) && (!bInFirstWordStar)) break;
	}

	// Copy our complete word list, but only if we didn't compute a wordList above:
	if (!bComputedNextWords)
		subPhrase.m_lstNextWords = m_pBibleDatabase->concordanceWordList();
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

inline bool CPhraseCursor::charUnderCursorIsSeparator()
{
	QChar chrValue = charUnderCursor();
	return (chrValue.isSpace() || (chrValue == QChar('|')));
}

bool CPhraseCursor::moveCursorWordLeft(MoveMode mode)
{
	// Try going left one in case we are at the end of the current word
	moveCursorCharLeft(mode);
	// If we are inside the "current word under the cursor", move left past it:
	while (!charUnderCursorIsSeparator()) {
		if (!moveCursorCharLeft(mode)) return false;
	}
	// We should now be between words, move left until we hit previous word:
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharLeft(mode)) return false;
	}
	// While in previous word, keep moving:
	while (!charUnderCursorIsSeparator()) {
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
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	// If we are inside the "current word under the cursor", move right past it:
	while (!charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;
	}
	// We should now be between word, move right until we hit next word:
	while (charUnderCursorIsSeparator()) {
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
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;	// Yes, move to right as current word is the one on the righthand side
	}
	// We should now be inside the current word, move left until we find the left side:
	while (!charUnderCursorIsSeparator()) {
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
	while (charUnderCursorIsSeparator()) {
		if (!moveCursorCharRight(mode)) return false;	// Move right here (not opposite of WordStart above), as current word is the one on the righthand side
	}
	// We're now inside the current word, move right until we hit the end:
	while (!charUnderCursorIsSeparator()) {
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

CPhraseNavigator::CPhraseNavigator(CBibleDatabasePtr pBibleDatabase, QTextDocument &textDocument, QObject *parent)
	:	QObject(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_TextDocument(textDocument)
{
	m_richifierTags.setWordsOfJesusTagsByColor(CPersistentSettings::instance()->colorWordsOfJesus());
	connect(CPersistentSettings::instance(), SIGNAL(changedColorWordsOfJesus(const QColor &)), this, SLOT(en_WordsOfJesusColorChanged(const QColor &)));

	m_richifierTags.setShowPilcrowMarkers(CPersistentSettings::instance()->showPilcrowMarkers());
	connect(CPersistentSettings::instance(), SIGNAL(changedShowPilcrowMarkers(bool)), this, SLOT(en_changedShowPilcrowMarkers(bool)));
}

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
	// Save some time if the tag isn't anything close to what we are displaying.
	//		We'll find a verse before and a verse after the main chapter being
	//		displayed (i.e. the actual scripture browser display window).  We
	//		will precalculate our current index before the main loop:
	TPhraseTagList tagCurrentDisplay = currentChapterDisplayPhraseTagList(ndxCurrent);
	doHighlighting(aHighlighter, bClear, tagCurrentDisplay);
}

void CPhraseNavigator::doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear, const TPhraseTagList &tagsCurrent) const
{
	assert(m_pBibleDatabase.data() != NULL);

	CPhraseCursor myCursor(&m_TextDocument);

	myCursor.beginEditBlock();

	CHighlighterPhraseTagFwdItr itrHighlighter = aHighlighter.getForwardIterator();
	while (!itrHighlighter.isEnd()) {
		TPhraseTag tag = itrHighlighter.nextTag();
		CRelIndex ndxRel = tag.relIndex();
		if (!ndxRel.isSet()) continue;

		// Save some time if the tag isn't anything close to what we are displaying.
		//		Check for intersection of the highlight tag with our display:
		if ((tagsCurrent.isSet()) && (!tagsCurrent.intersects(m_pBibleDatabase.data(), tag))) continue;

		unsigned int nTagCount = tag.count();
		if (nTagCount) --nTagCount;					// Make nTagCount the number of positions to move, not number words

		uint32_t ndxNormalStart = m_pBibleDatabase->NormalizeIndex(ndxRel);
		uint32_t ndxNormalEnd = ndxNormalStart + nTagCount;
		int nStartPos = -1;
		while ((nStartPos == -1) && (ndxNormalStart <= ndxNormalEnd)) {
			nStartPos = anchorPosition(ndxRel.asAnchor());
			if (nStartPos == -1) {
				ndxNormalStart++;
				ndxRel = CRelIndex(m_pBibleDatabase->DenormalizeIndex(ndxNormalStart));
				// Safeguard incase we run off the end.  This is needed, for example, if
				//		on the last word of the Bible (Rev 22:21 [12]) with the cursor
				//		tracker visible and the user hits Alt-PgUp to go to the previous
				//		chapter.  It will run off the end looking for that word and
				//		not finding it because it's already scrolled out of view.  And
				//		then will run off the end of the text:
				if (!ndxRel.isSet()) ndxNormalStart = ndxNormalEnd+1;
			}
		}
		if (nStartPos == -1) continue;				// Note: Some highlight lists have tags not in this browser document

		while ((nStartPos != -1) && (ndxNormalStart <= ndxNormalEnd)) {
			myCursor.setPosition(nStartPos);
			int nWordEndPos = nStartPos + m_pBibleDatabase->wordAtIndex(ndxNormalStart).size();

			// If this is a continuous highlighter, instead of stopping at the end of the word,
			//		we'll find the end of the last word of this verse that we'll be highlighting
			//		so that we will highlight everything in between and also not have to search
			//		for start/end of words within the verse.  We can't do more than one verse
			//		because of notes and cross-references:
			if ((aHighlighter.isContinuous()) & (ndxNormalStart != ndxNormalEnd)) {
				CRelIndex ndxCurrentWord(m_pBibleDatabase->DenormalizeIndex(ndxNormalStart));
				const CVerseEntry *pCurrentWordVerseEntry = m_pBibleDatabase->verseEntry(ndxCurrentWord);
				assert(pCurrentWordVerseEntry != NULL);
				if (pCurrentWordVerseEntry) {
					unsigned int nVrsWordCount = pCurrentWordVerseEntry->m_nNumWrd;
					uint32_t ndxNormalLastVerseWord = ndxNormalStart + nVrsWordCount - ndxCurrentWord.word();
					if (ndxNormalLastVerseWord > ndxNormalEnd) ndxNormalLastVerseWord = ndxNormalEnd;
					CRelIndex ndxLastVerseWord(m_pBibleDatabase->DenormalizeIndex(ndxNormalLastVerseWord));
					int nNextLastVerseWordPos = anchorPosition(ndxLastVerseWord.asAnchor());
					if (nNextLastVerseWordPos != -1) {
						nNextLastVerseWordPos += m_pBibleDatabase->wordAtIndex(ndxNormalLastVerseWord).size();
					} else {
						assert(false);
					}
					assert(nWordEndPos <= nNextLastVerseWordPos);
					nWordEndPos = nNextLastVerseWordPos;
					ndxNormalStart = ndxNormalLastVerseWord;
				}
			}

			if (nStartPos < nWordEndPos) {
				if (myCursor.moveCursorCharRight(QTextCursor::KeepAnchor)) {
					QTextCharFormat fmtNew = aHighlighter.doHighlighting(myCursor.charFormat(), bClear);
					myCursor.setPosition(nWordEndPos, QTextCursor::KeepAnchor);
					myCursor.mergeCharFormat(fmtNew);
					myCursor.clearSelection();
				} else {
					assert(false);
				}
			}

			++ndxNormalStart;
			nStartPos = anchorPosition(CRelIndex(m_pBibleDatabase->DenormalizeIndex(ndxNormalStart)).asAnchor());
		}
	}

	myCursor.endEditBlock();
}

TPhraseTagList CPhraseNavigator::currentChapterDisplayPhraseTagList(const CRelIndex &ndxCurrent) const
{
	TPhraseTagList tagsCurrentDisplay;

	if ((ndxCurrent.isSet()) && (ndxCurrent.book() != 0) && (ndxCurrent.chapter() != 0)) {
		CRelIndex ndxDisplay = CRelIndex(ndxCurrent.book(), ndxCurrent.chapter(), 0, 1);
		uint32_t ndxNormalCurrent = m_pBibleDatabase->NormalizeIndex(ndxDisplay);
		// This can happen if the versification of the reference doesn't match the active database:
		if (ndxNormalCurrent == 0) return TPhraseTagList();
		const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndxDisplay);
		assert(pChapter != NULL);
		unsigned int nNumWordsDisplayed = pChapter->m_nNumWrd;
		CRelIndex ndxVerseBefore = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndxCurrent.book(), ndxCurrent.chapter(), 1, 1), true);	// Calculate one verse prior to the first verse of this book/chapter
		if (ndxVerseBefore.isSet()) {
			const CVerseEntry *pVerseBefore = m_pBibleDatabase->verseEntry(ndxVerseBefore);
			assert(pVerseBefore != NULL);
			nNumWordsDisplayed += m_pBibleDatabase->NormalizeIndex(ndxDisplay) - m_pBibleDatabase->NormalizeIndex(CRelIndex(ndxVerseBefore.book(), ndxVerseBefore.chapter(), ndxVerseBefore.verse(), 1));
			ndxDisplay = CRelIndex(ndxVerseBefore.book(), ndxVerseBefore.chapter(), ndxVerseBefore.verse(), 1);
		}
		CRelIndex ndxVerseAfter = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndxCurrent.book(), ndxCurrent.chapter(), 1, 1), false);	// Calculate first verse of next chapter
		if (ndxVerseAfter.isSet()) {
			const CVerseEntry *pVerseAfter = m_pBibleDatabase->verseEntry(ndxVerseAfter);
			assert(pVerseAfter != NULL);
			ndxVerseAfter.setWord(pVerseAfter->m_nNumWrd);
			nNumWordsDisplayed = m_pBibleDatabase->NormalizeIndex(ndxVerseAfter) - m_pBibleDatabase->NormalizeIndex(ndxDisplay);
		}
		tagsCurrentDisplay.append(TPhraseTag(ndxDisplay, nNumWordsDisplayed));

		// If this book has a colophon and this is the last chapter of that book, we
		//	need to add it as well:
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxCurrent.book());
		assert(pBook != NULL);
		if ((pBook->m_bHaveColophon) && (ndxCurrent.chapter() == pBook->m_nNumChp)) {
			const CVerseEntry *pBookColophon = m_pBibleDatabase->verseEntry(CRelIndex(ndxCurrent.book(), 0, 0, 0));
			assert(pBookColophon != NULL);
			tagsCurrentDisplay.append(TPhraseTag(CRelIndex(ndxCurrent.book(), 0, 0, 1), pBookColophon->m_nNumWrd));
		}

		// If the ndxVerseBefore is in a different book, check that book to see if
		//	it has a colophon and if so, add it so that we will render highlighting
		//	and other markup for it:
		if (ndxVerseBefore.book() != ndxCurrent.book()) {
			const CBookEntry *pBookVerseBefore = m_pBibleDatabase->bookEntry(ndxVerseBefore.book());
			assert(pBookVerseBefore != NULL);
			if (pBookVerseBefore->m_bHaveColophon) {
				const CVerseEntry *pPrevBookColophon = m_pBibleDatabase->verseEntry(CRelIndex(ndxVerseBefore.book(), 0, 0, 0));
				assert(pPrevBookColophon != NULL);
				tagsCurrentDisplay.append(TPhraseTag(CRelIndex(ndxVerseBefore.book(), 0, 0, 1), pPrevBookColophon->m_nNumWrd));
			}
		}
	}

	return tagsCurrentDisplay;
}

QString CPhraseNavigator::setDocumentToBookInfo(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO)
{
	assert(m_pBibleDatabase.data() != NULL);
	assert(g_pUserNotesDatabase.data() != NULL);

	m_TextDocument.clear();

	if (ndx.book() == 0) return QString();

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	// Search for "Category:".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strCategory = tr("Category:", "Scope");
	TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
	if (pTranslator.data() != NULL) {
		QString strTemp = pTranslator->translator().translate("CPhraseNavigator", "Category:", "Scope");
		if (!strTemp.isEmpty()) strCategory = strTemp;
	}

	CScriptureTextHtmlBuilder scriptureHTML;

	QString strCopyFont= "font-size:medium;";
	if (flagsTRO & TRO_Copying) {
		switch (CPersistentSettings::instance()->copyFontSelection()) {
			case CFSE_NONE:
				break;
			case CFSE_COPY_FONT:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
				break;
			case CFSE_SCRIPTURE_BROWSER:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
				break;
			case CFSE_SEARCH_RESULTS:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
				break;
			default:
				assert(false);
				break;
		}
	}

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n.subtitle { font-size:12pt; font-weight:normal; }\n.category { font-size:12pt; font-weight:normal; }\n</style></head><body>\n")
//										.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
//										.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
										"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
										"<title>%1</title><style type=\"text/css\">\n"
										"body, p, li { white-space: pre-line; %2 }\n"
										".book { font-size:xx-large; font-weight:bold; }\n"
										".chapter { font-size:x-large; font-weight:bold; }\n"
										".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".category { font-size:medium; font-weight:normal; }\n"
										".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
										"</style></head><body>\n")
										.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
										.arg(strCopyFont));																// Copy Font

	CRelIndex ndxBookChap(ndx.book(), ndx.chapter(), 0, 0);
	CRelIndex ndxBook(ndx.book(), 0, 0, 0);

	// Print Heading for this Book:
	scriptureHTML.beginDiv("book");
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(ndxBook.asAnchor());
	scriptureHTML.appendLiteralText(book.m_strBkName);
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();
	// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
	//		at the end of the book name so people adding notes/cross-refs for the book
	//		aren't confused by it being at the beginning of the name.  But then switch
	//		back to a book reference so that book category/descriptions are properly
	//		labeled:
	if (!(flagsTRO & TRO_NoAnchors)) {
		scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChap.asAnchor()));
		scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
		scriptureHTML.endAnchor();
		scriptureHTML.beginAnchorID(QString("%1").arg(ndxBook.asAnchor()));
		scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
		scriptureHTML.endAnchor();
	}
	scriptureHTML.endDiv();

	// Print Book Descriptions:
	if ((flagsTRO & TRO_Subtitles) && (!book.m_strDesc.isEmpty())) {
		scriptureHTML.beginDiv("subtitle");
		scriptureHTML.appendRawText(QString("(%1)").arg(book.m_strDesc));
		scriptureHTML.endDiv();
	}
	// Print Book Category:
	if  ((flagsTRO & TRO_Category) && (!m_pBibleDatabase->bookCategoryName(ndxBook).isEmpty())) {
		scriptureHTML.beginDiv("category");
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strCategory);
		scriptureHTML.endBold();
		scriptureHTML.appendRawText(QString(" %1").arg(m_pBibleDatabase->bookCategoryName(ndxBook)));
		scriptureHTML.endDiv();
	}
	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBook, !(flagsTRO & TRO_NoAnchors));
	}
	// If we have a User Note for this book, print it too:
	if ((flagsTRO & TRO_UserNotes) &&
		(scriptureHTML.addNoteFor(ndxBook, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
		scriptureHTML.insertHorizontalRule();

	scriptureHTML.appendRawText("</body></html>");
	QString strRawHTML = scriptureHTML.getResult();
	m_TextDocument.setHtml(strRawHTML);
	emit changedDocumentText();
	return strRawHTML;
}

QString CPhraseNavigator::setDocumentToChapter(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO)
{
	assert(m_pBibleDatabase.data() != NULL);
	assert(g_pUserNotesDatabase.data() != NULL);

	m_TextDocument.clear();

	if ((ndx.book() == 0) || (ndx.chapter() == 0)) return QString();

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndx);
	if (pChapter == NULL) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}

	if (ndx.chapter() > book.m_nNumChp) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}

	// Search for "Category:".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strCategory = tr("Category:", "Scope");
	TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
	if (pTranslator.data() != NULL) {
		QString strTemp = pTranslator->translator().translate("CPhraseNavigator", "Category:", "Scope");
		if (!strTemp.isEmpty()) strCategory = strTemp;
	}

	// Search for "Chapter".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strChapter = tr("Chapter", "Scope");
	//TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
	if (pTranslator.data() != NULL) {
		QString strTemp = pTranslator->translator().translate("CPhraseNavigator", "Chapter", "Scope");
		if (!strTemp.isEmpty()) strChapter = strTemp;
	}

	CScriptureTextHtmlBuilder scriptureHTML;

	QString strCopyFont= "font-size:medium;";
	if (flagsTRO & TRO_Copying) {
		switch (CPersistentSettings::instance()->copyFontSelection()) {
			case CFSE_NONE:
				break;
			case CFSE_COPY_FONT:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
				break;
			case CFSE_SCRIPTURE_BROWSER:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
				break;
			case CFSE_SEARCH_RESULTS:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
				break;
			default:
				assert(false);
				break;
		}
	}

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n.subtitle { font-size:12pt; font-weight:normal; }\n.category { font-size:12pt; font-weight:normal; }\n</style></head><body>\n")
//										.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
//										.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
										"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
										"<title>%1</title><style type=\"text/css\">\n"
										"body, p, li { white-space: pre-line; %2 }\n"
										".book { font-size:xx-large; font-weight:bold; }\n"
										".chapter { font-size:x-large; font-weight:bold; }\n"
										".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".category { font-size:medium; font-weight:normal; }\n"
										".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
										"</style></head><body>\n")
										.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
										.arg(strCopyFont));																// Copy Font

	CRelIndex relPrev = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndx.book(), ndx.chapter(), 1, 1), true);	// Calculate one verse prior to the first verse of this book/chapter
	CRelIndex relNext = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndx.book(), ndx.chapter(), 1, 1), false);	// Calculate first verse of next chapter

	// Print last verse of previous chapter if available:
	if ((!(flagsTRO & TRO_SuppressPrePostChapters)) && (relPrev.isSet())) {
		relPrev.setWord(0);
		const CBookEntry &bookPrev = *m_pBibleDatabase->bookEntry(relPrev.book());
		scriptureHTML.beginParagraph();
		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(relPrev.asAnchor());
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(QString("%1 ").arg(relPrev.verse()));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();

		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(relPrev, m_richifierTags, !(flagsTRO & TRO_NoAnchors)));

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), relPrev, !(flagsTRO & TRO_NoAnchors), true);
		}
		// And Notes:
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(relPrev, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true);

		scriptureHTML.endParagraph();

		// If we have a footnote or user note for this book and this is the end of the last chapter,
		//		print it too:
		if (relPrev.chapter() == bookPrev.m_nNumChp) {
			if ((flagsTRO & TRO_Colophons) && (bookPrev.m_bHaveColophon)) {
				// Try pseudo-verse (searchable) style first:
				scriptureHTML.beginDiv("colophon");
				scriptureHTML.beginParagraph();
				scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(CRelIndex(relPrev.book(), 0, 0, 0), m_richifierTags, !(flagsTRO & TRO_NoAnchors)));
				scriptureHTML.endParagraph();
				scriptureHTML.endDiv();
			} else {
				// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
				scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
				if ((flagsTRO & TRO_Colophons) &&
					(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), CRelIndex(relPrev.book(),0,0,0), !(flagsTRO & TRO_NoAnchors)))) {
					scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the colophon divison ahead of footnote
					scriptureHTML.beginDiv("colophon");
					scriptureHTML.flushBuffer();
					scriptureHTML.endDiv();
				}
				scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
			}

			if (flagsTRO & TRO_UserNotes)
				scriptureHTML.addNoteFor(CRelIndex(relPrev.book(),0,0,0), (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible));
				// No extra <hr> as we have one below for the whole chapter anyway
		}
	}

	if (!(flagsTRO & TRO_SuppressPrePostChapters)) scriptureHTML.insertHorizontalRule();

	CRelIndex ndxBookChap(ndx.book(), ndx.chapter(), 0, 0);
	CRelIndex ndxBook(ndx.book(), 0, 0, 0);
	CRelIndex ndxBookChapter1(ndx.book(), 1, 0, 0);

	// Print Heading for this Book:
	scriptureHTML.beginDiv("book");
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(ndxBook.asAnchor());
	scriptureHTML.appendLiteralText(book.m_strBkName);
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();
	// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
	//		at the end of the book name so people adding notes/cross-refs for the book
	//		aren't confused by it being at the beginning of the name.  But then switch
	//		back to a book reference so that book category/descriptions are properly
	//		labeled:
	if (!(flagsTRO & TRO_NoAnchors)) {
		scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChap.asAnchor()));
		scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
		scriptureHTML.endAnchor();
		scriptureHTML.beginAnchorID(QString("%1").arg(ndxBook.asAnchor()));
		scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
		scriptureHTML.endAnchor();
	}
	scriptureHTML.endDiv();
	// If this is the first chapter of the book:
	if (m_pBibleDatabase->NormalizeIndex(ndxBookChapter1) == m_pBibleDatabase->NormalizeIndex(ndxBookChap)) {
		// Print Book Descriptions:
		if ((flagsTRO & TRO_Subtitles) && (!book.m_strDesc.isEmpty())) {
			scriptureHTML.beginDiv("subtitle");
			scriptureHTML.appendRawText(QString("(%1)").arg(book.m_strDesc));
			scriptureHTML.endDiv();
		}
		// Print Book Category:
		if  ((flagsTRO & TRO_Category) && (!m_pBibleDatabase->bookCategoryName(ndxBook).isEmpty())) {
			scriptureHTML.beginDiv("category");
			scriptureHTML.beginBold();
			scriptureHTML.appendLiteralText(strCategory);
			scriptureHTML.endBold();
			scriptureHTML.appendRawText(QString(" %1").arg(m_pBibleDatabase->bookCategoryName(ndxBook)));
			scriptureHTML.endDiv();
		}
		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBook, !(flagsTRO & TRO_NoAnchors));
		}
		// If we have a User Note for this book, print it too:
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(ndxBook, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
			scriptureHTML.insertHorizontalRule();
	}

	// Print Heading for this Chapter:
	scriptureHTML.beginDiv("chapter");
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(ndxBookChap.asAnchor());
	scriptureHTML.appendLiteralText(QString("%1 %2").arg(strChapter).arg(ndx.chapter()));
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();
	scriptureHTML.endDiv();
	// If we have a chapter Footnote for this chapter, print it too:
	if ((flagsTRO & TRO_Superscriptions) && (pChapter->m_bHaveSuperscription)) {
		// Try pseudo-verse (searchable) style first:
		scriptureHTML.beginDiv("superscription");
		scriptureHTML.beginParagraph();
		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxBookChap, m_richifierTags, !(flagsTRO & TRO_NoAnchors)));
		scriptureHTML.endParagraph();
		scriptureHTML.endDiv();
	} else {
		// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
		scriptureHTML.startBuffered();			// Start buffering so we can insert superscription division if there is a footnote
		if ((flagsTRO & TRO_Superscriptions) &&
			(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), ndxBookChap, !(flagsTRO & TRO_NoAnchors)))) {
			scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the superscription divison ahead of footnote
			scriptureHTML.beginDiv("superscription");
			scriptureHTML.flushBuffer();
			scriptureHTML.endDiv();
		}
		scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
	}

	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBookChap, !(flagsTRO & TRO_NoAnchors));
	}

	// If we have a chapter User Note for this chapter, print it too:
	if ((flagsTRO & TRO_UserNotes) &&
		(scriptureHTML.addNoteFor(ndxBookChap, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
		scriptureHTML.insertHorizontalRule();

	// Print the Chapter Text:
	bool bParagraph = false;
	CRelIndex ndxVerse;
	bool bVPLNeedsLineBreak = false;
	bool bStartedText = false;
	for (unsigned int ndxVrs=0; ndxVrs<pChapter->m_nNumVrs; ++ndxVrs) {
		if ((CPersistentSettings::instance()->verseRenderingMode() == VRME_VPL) &&
			(bParagraph)) bVPLNeedsLineBreak = true;

		ndxVerse = CRelIndex(ndx.book(), ndx.chapter(), ndxVrs+1, 0);
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndxVerse);
		if (pVerse == NULL) {
			assert(false);
			continue;
		}
		if ((!bStartedText) && (pVerse->m_nNumWrd == 0)) continue;			// Don't print verses that are empty if we haven't started printing anything for the chapter yet

		if ((pVerse->m_nPilcrow != CVerseEntry::PTE_NONE) &&
			(CPersistentSettings::instance()->verseRenderingMode() != VRME_VPL)) {
			if (bParagraph) {
				scriptureHTML.endParagraph();
				bParagraph=false;
			}
		}
		if (!bParagraph) {
			scriptureHTML.beginParagraph();
			bParagraph = true;
		}

		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(ndxVerse.asAnchor());

		if (bVPLNeedsLineBreak) {
			scriptureHTML.addLineBreak();
			bVPLNeedsLineBreak = false;
		}

		scriptureHTML.beginBold();
		if ((bStartedText) && (CPersistentSettings::instance()->verseRenderingMode() != VRME_VPL)) scriptureHTML.appendLiteralText(" ");
		scriptureHTML.appendLiteralText(QString("%1 ").arg(ndxVrs+1));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();

		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxVerse, m_richifierTags, !(flagsTRO & TRO_NoAnchors)));

		bStartedText = true;

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxVerse, !(flagsTRO & TRO_NoAnchors), true);
		}

		// Output notes for this verse, but make use of the buffer in case we need to end the paragraph tag:
		scriptureHTML.startBuffered();
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(ndxVerse, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true))) {
			if (bParagraph) {
				scriptureHTML.stopBuffered();	// Switch to direct output to end the paragraph ahead of the note
				scriptureHTML.endParagraph();
				bParagraph = false;
			}
			// Do an extra horizontal break if not at the end of the chapter:
			if (ndxVrs != (pChapter->m_nNumVrs - 1)) {
				scriptureHTML.flushBuffer(true);		// Flush our note, stop buffering (call below is redundant in this one case)
				scriptureHTML.insertHorizontalRule();	//	but is needed so we can output this <hr>
			}
		}
		scriptureHTML.flushBuffer(true);		// Stop buffering and flush

		ndxVerse.setWord(pVerse->m_nNumWrd);		// At end of loop, ndxVerse will be index of last word we've output...
	}
	if (bParagraph) {
		scriptureHTML.endParagraph();
		bParagraph = false;
	}

	// If we have a footnote or user note for this book and this is the end of the last chapter,
	//		print it too:
	if (ndx.chapter() == book.m_nNumChp) {
		if ((flagsTRO & TRO_Colophons) && (book.m_bHaveColophon)) {
			// Try pseudo-verse (searchable) style first:
			scriptureHTML.beginDiv("colophon");
			scriptureHTML.beginParagraph();
			scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxBook, m_richifierTags, !(flagsTRO & TRO_NoAnchors)));
			scriptureHTML.endParagraph();
			scriptureHTML.endDiv();
		} else {
			// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
			scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
			if ((flagsTRO & TRO_Colophons) &&
				(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), ndxBook, !(flagsTRO & TRO_NoAnchors)))) {
				scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the colophon divison ahead of footnote
				scriptureHTML.beginDiv("colophon");
				scriptureHTML.flushBuffer();
				scriptureHTML.endDiv();
			}
			scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
		}
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(CRelIndex(ndx.book(),0,0,0), (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible));
			// No extra <hr> as we have one below for the whole chapter anyway
	}

	if (!(flagsTRO & TRO_SuppressPrePostChapters)) scriptureHTML.insertHorizontalRule();

	// Print first verse of next chapter if available:
	if ((!(flagsTRO & TRO_SuppressPrePostChapters)) && (relNext.isSet())) {
		relNext.setWord(0);
		CRelIndex ndxBookChapNext(relNext.book(), relNext.chapter(), 0, 0);
		CRelIndex ndxBookNext(relNext.book(), 0, 0, 0);
		const CBookEntry &bookNext = *m_pBibleDatabase->bookEntry(relNext.book());
		const CChapterEntry *pChapterNext = m_pBibleDatabase->chapterEntry(ndxBookChapNext);
		assert(pChapterNext != NULL);

		// Print Heading for this Book:
		if (relNext.book() != ndx.book()) {
			// Print Heading for this Book:
			scriptureHTML.beginDiv("book");
			if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(ndxBookNext.asAnchor());
			scriptureHTML.appendLiteralText(bookNext.m_strBkName);
			if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();
			// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
			//		at the end of the book name so people adding notes/cross-refs for the book
			//		aren't confused by it being at the beginning of the name.  But then switch
			//		back to a book reference so that book category/descriptions are properly
			//		labeled:
			if (!(flagsTRO & TRO_NoAnchors)) {
				scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChapNext.asAnchor()));
				scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
				scriptureHTML.endAnchor();
				scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookNext.asAnchor()));
				scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
				scriptureHTML.endAnchor();
			}
			scriptureHTML.endDiv();
			// Print Book Descriptions for first chapter of book:
			if ((flagsTRO & TRO_Subtitles) && (!bookNext.m_strDesc.isEmpty()) && (relNext.chapter() == 1)) {
				scriptureHTML.beginDiv("subtitle");
				scriptureHTML.appendRawText(QString("(%1)").arg(bookNext.m_strDesc));
				scriptureHTML.endDiv();
			}
			// Print Book Category for first chapter of book:
			if ((flagsTRO & TRO_Category) && (!m_pBibleDatabase->bookCategoryName(ndxBookNext).isEmpty()) && (relNext.chapter() == 1)) {
				scriptureHTML.beginDiv("category");
				scriptureHTML.beginBold();
				scriptureHTML.appendLiteralText(strCategory);
				scriptureHTML.endBold();
				scriptureHTML.appendRawText(QString(" %1").arg(m_pBibleDatabase->bookCategoryName(ndxBookNext)));
				scriptureHTML.endDiv();
			}
			// Add CrossRefs:
			if (flagsTRO & TRO_CrossRefs) {
				scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBookNext, !(flagsTRO & TRO_NoAnchors));
			}
			// If we have a User Note for this book, print it too:
			if ((flagsTRO & TRO_UserNotes) &&
				(scriptureHTML.addNoteFor(ndxBookNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
				scriptureHTML.insertHorizontalRule();
		}
		// Print Heading for this Chapter:
		scriptureHTML.beginDiv("chapter");
		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(ndxBookChapNext.asAnchor());
		scriptureHTML.appendLiteralText(QString("%1 %2").arg(strChapter).arg(relNext.chapter()));
		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();
		scriptureHTML.endDiv();

		// If we have a chapter note for this chapter, print it too:
		if ((flagsTRO & TRO_Superscriptions) && (pChapterNext->m_bHaveSuperscription)) {
			// Try pseudo-verse (searchable) style first:
			scriptureHTML.beginDiv("superscription");
			scriptureHTML.beginParagraph();
			scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxBookChapNext, m_richifierTags, !(flagsTRO & TRO_NoAnchors)));
			scriptureHTML.endParagraph();
			scriptureHTML.endDiv();
		} else {
			// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
			scriptureHTML.startBuffered();			// Start buffering so we can insert superscription division if there is a footnote
			if ((flagsTRO & TRO_Superscriptions) &&
				(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), ndxBookChapNext, !(flagsTRO & TRO_NoAnchors)))) {
				scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the superscription divison ahead of footnote
				scriptureHTML.beginDiv("superscription");
				scriptureHTML.flushBuffer();
				scriptureHTML.endDiv();
			}
			scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
		}

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBookChapNext, !(flagsTRO & TRO_NoAnchors));
		}

		// If we have a chapter User Note for this chapter, print it too:
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(ndxBookChapNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
			scriptureHTML.insertHorizontalRule();

		scriptureHTML.beginParagraph();
		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(relNext.asAnchor());
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(QString("%1 ").arg(relNext.verse()));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();

		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(relNext, m_richifierTags, !(flagsTRO & TRO_NoAnchors)));

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), relNext, !(flagsTRO & TRO_NoAnchors), true);
		}
		// And Notes:
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(relNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true);

		scriptureHTML.endParagraph();
	}

	scriptureHTML.appendRawText("</body></html>");
	QString strRawHTML = scriptureHTML.getResult();
	m_TextDocument.setHtml(strRawHTML);
	emit changedDocumentText();
	return strRawHTML;
}

QString CPhraseNavigator::setDocumentToVerse(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO)
{
	assert(m_pBibleDatabase.data() != NULL);

	m_TextDocument.clear();

	if (ndx.book() == 0) {
		emit changedDocumentText();
		return QString();
	}

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	if (ndx.chapter() > book.m_nNumChp) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}

	const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndx);		// Note: Will be null on colophon

	if (((pChapter != NULL) && (ndx.verse() > pChapter->m_nNumVrs)) ||
		((pChapter == NULL) && (ndx.verse() != 0))) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}

	CRelIndex ndxVerse = ndx;
	ndxVerse.setWord(0);			// Create special index to make sure we use a verse only reference

	CScriptureTextHtmlBuilder scriptureHTML;

	QString strCopyFont= "font-size:medium;";
	if (flagsTRO & TRO_Copying) {
		switch (CPersistentSettings::instance()->copyFontSelection()) {
			case CFSE_NONE:
				break;
			case CFSE_COPY_FONT:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
				break;
			case CFSE_SCRIPTURE_BROWSER:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
				break;
			case CFSE_SEARCH_RESULTS:
				strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
				break;
			default:
				assert(false);
				break;
		}
	}

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
//						.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n")
//						.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
										"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
										"<title>%1</title><style type=\"text/css\">\n"
										"body, p, li { white-space: pre-wrap; %2 }\n"
										".book { font-size:xx-large; font-weight:bold; }\n"
										".chapter { font-size:x-large; font-weight:bold; }\n"
										".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".category { font-size:medium; font-weight:normal; }\n"
										".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
										"</style></head><body>\n")
										.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
										.arg(strCopyFont));																// Copy Font

	if (flagsTRO & TRO_AddDividerLineBefore) scriptureHTML.insertHorizontalRule();

	// Print Book/Chapter for this verse:
	scriptureHTML.beginParagraph();

	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(CRelIndex(ndx.book(), ndx.chapter(), 0, 0).asAnchor());
	scriptureHTML.beginBold();
	scriptureHTML.appendLiteralText(book.m_strBkName);
	scriptureHTML.endBold();
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();

	// Print this Verse Text:
	const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndx);
	if (pVerse == NULL) {
		assert(false);
		emit changedDocumentText();
		return QString();
	}
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.beginAnchorID(ndxVerse.asAnchor());
	scriptureHTML.beginBold();
	if (ndx.verse() != 0) {
		scriptureHTML.appendLiteralText(QString(" %1:%2 ").arg(ndx.chapter()).arg(ndx.verse()));
	} else {
		if (ndx.chapter() == 0) {
			scriptureHTML.appendLiteralText(" " + tr("Colophon", "Statistics") + " ");
		} else {
			scriptureHTML.appendLiteralText(QString(" %1 %2 ").arg(ndx.chapter()).arg(tr("Superscription", "Statistics")));
		}
	}
	scriptureHTML.endBold();
	if (!(flagsTRO & TRO_NoAnchors)) scriptureHTML.endAnchor();
//
// Note: The problem with applying a special colophon/superscription style with
//		a <div> causes it to be separated as its own paragraph rather than
//		inline like normal verses.  Can we do this with a span or something?
//		More importantly, do we even want to do it?  As we do lose our
//		transChange markup in all of the italics...
//
//	if (ndx.verse() == 0) {
//		if (ndx.chapter() == 0) {
//			scriptureHTML.beginDiv("colophon");
//		} else {
//			scriptureHTML.beginDiv("superscription");
//		}
//	}
	scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndx, m_richifierTags, !(flagsTRO & TRO_NoAnchors)));
//	if (ndx.verse() == 0) {
//		scriptureHTML.endDiv();
//	}

	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxVerse, !(flagsTRO & TRO_NoAnchors), true);
	}

	scriptureHTML.endParagraph();

	if (flagsTRO & TRO_UserNotes)
		scriptureHTML.addNoteFor(ndxVerse, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible));

	scriptureHTML.appendRawText("</body></html>");
	QString strRawHTML = scriptureHTML.getResult();
	m_TextDocument.setHtml(strRawHTML);
	emit changedDocumentText();
	return strRawHTML;
}

QString CPhraseNavigator::setDocumentToFormattedVerses(const TPhraseTag &tagPhrase)
{
	return setDocumentToFormattedVerses(TPassageTag::fromPhraseTag(m_pBibleDatabase.data(), tagPhrase));
}

QString CPhraseNavigator::setDocumentToFormattedVerses(const TPassageTag &tagPassage)
{
	assert(m_pBibleDatabase.data() != NULL);

	m_TextDocument.clear();

	if ((!tagPassage.relIndex().isSet()) || (tagPassage.verseCount() == 0)) {
		emit changedDocumentText();
		return QString();
	}

	CRelIndex ndxFirst = tagPassage.relIndex();
	ndxFirst.setWord(1);		// We aren't using words, only whole verses, but need to point to first word so normalize will work correctly
	CRelIndex ndxLast = m_pBibleDatabase->calcRelIndex(0, tagPassage.verseCount()-1, 0, 0, 0, ndxFirst);		// Add number of verses to find last verse to output
	assert(ndxLast.isSet());
	assert(ndxLast.word() == 1);		// Note: When we calculate next verse, we'll automatically resolve to the first word.  Leave it at 1st word so our loop compare will work

	CScriptureTextHtmlBuilder scriptureHTML;

	QString strCopyFont= "font-size:medium;";
	switch (CPersistentSettings::instance()->copyFontSelection()) {
		case CFSE_NONE:
			break;
		case CFSE_COPY_FONT:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
			break;
		case CFSE_SCRIPTURE_BROWSER:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
			break;
		case CFSE_SEARCH_RESULTS:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
			break;
		default:
			assert(false);
			break;
	}

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
//								"<html><head><title>%1</title><style type=\"text/css\">\n"
//								"body, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n"
//								".book { font-size:24pt; font-weight:bold; }\n"
//								".chapter { font-size:18pt; font-weight:bold; }\n"
//								"</style></head><body>\n")
//						.arg(scriptureHTML.escape(tagPassage.PassageReferenceRangeText(m_pBibleDatabase))));		// Document Title

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
//								"<html><head><title>%1</title><style type=\"text/css\">\n"
//								"body, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n"
//								".book { font-size:xx-large; font-weight:bold; }\n"
//								".chapter { font-size:x-large; font-weight:bold; }\n"
//								"</style></head><body>\n")
//						.arg(scriptureHTML.escape(tagPassage.PassageReferenceRangeText(m_pBibleDatabase))));		// Document Title

	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
								"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
								"<title>%1</title><style type=\"text/css\">\n"
								"body, p, li { white-space: pre-wrap; %2 }\n"
								".book { font-size:xx-large; font-weight:bold; }\n"
								".chapter { font-size:x-large; font-weight:bold; }\n"
								"</style></head><body>\n")
								.arg(scriptureHTML.escape(tagPassage.PassageReferenceRangeText(m_pBibleDatabase.data())))	// Document Title
								.arg(strCopyFont));																			// Copy Font

	QString strReference;

	QString strBookNameFirst = (CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ? m_pBibleDatabase->bookNameAbbr(ndxFirst) : m_pBibleDatabase->bookName(ndxFirst));
	QString strBookNameLast = (CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ? m_pBibleDatabase->bookNameAbbr(ndxLast) : m_pBibleDatabase->bookName(ndxLast));
	strReference += referenceStartingDelimiter();
	if (ndxFirst.book() == ndxLast.book()) {
		if (ndxFirst.chapter() == ndxLast.chapter()) {
			if (ndxFirst.verse() == ndxLast.verse()) {
				strReference += QString("%1 %2:%3")
										.arg(strBookNameFirst)
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse());
			} else {
				strReference += QString("%1 %2:%3-%4")
										.arg(strBookNameFirst)
										.arg(ndxFirst.chapter())
										.arg(ndxFirst.verse())
										.arg(ndxLast.verse());
			}
		} else {
			strReference += QString("%1 %2:%3-%4:%5")
									.arg(strBookNameFirst)
									.arg(ndxFirst.chapter())
									.arg(ndxFirst.verse())
									.arg(ndxLast.chapter())
									.arg(ndxLast.verse());
		}
	} else {
		strReference += QString("%1 %2:%3-%4 %5:%6")
								.arg(strBookNameFirst)
								.arg(ndxFirst.chapter())
								.arg(ndxFirst.verse())
								.arg(strBookNameLast)
								.arg(ndxLast.chapter())
								.arg(ndxLast.verse());
	}
	strReference += referenceEndingDelimiter();

	scriptureHTML.beginParagraph();

	if (!CPersistentSettings::instance()->referencesAtEnd()) {
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strReference);
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.endBold();
		scriptureHTML.appendLiteralText(" ");
	}

	scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));

	CVerseTextRichifierTags richifierTags = m_richifierTags;
	switch (CPersistentSettings::instance()->transChangeAddWordMode()) {
		case TCAWME_NO_MARKING:
			richifierTags.setTransChangeAddedTags(QString(), QString());
			break;
		case TCAWME_ITALICS:
			richifierTags.setTransChangeAddedTags(QString("<i>"), QString("</i>"));
			break;
		case TCAWME_BRACKETS:
			richifierTags.setTransChangeAddedTags(QString("["), QString("]"));
			break;
		default:
			assert(false);
			break;
	}
	richifierTags.setShowPilcrowMarkers(CPersistentSettings::instance()->copyPilcrowMarkers());

	CRelIndex ndxPrev = ndxFirst;
	if (CPersistentSettings::instance()->referencesAtEnd()) {
		// If printing the reference at the end, for printing of the initial verse number:
		ndxPrev.setVerse(0);
		ndxPrev.setWord(0);
	}
	for (CRelIndex ndx = ndxFirst; ((ndx.index() <= ndxLast.index()) && (ndx.isSet())); ndx=m_pBibleDatabase->calcRelIndex(0,1,0,0,0,ndx)) {
		if ((CPersistentSettings::instance()->verseRenderingModeCopying() == VRME_VPL) &&
			(ndx != ndxFirst)) scriptureHTML.addLineBreak();

		if (ndx.book() != ndxPrev.book()) {
			scriptureHTML.appendLiteralText("  ");
			if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.beginBold();
			scriptureHTML.appendLiteralText(QString("%1%2 %3:%4%5")
											.arg(referenceStartingDelimiter())
											.arg(CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames() ? m_pBibleDatabase->bookNameAbbr(ndx) : m_pBibleDatabase->bookName(ndx))
											.arg(ndx.chapter())
											.arg(ndx.verse())
											.arg(referenceEndingDelimiter()));
			if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.endBold();
			scriptureHTML.appendLiteralText(" ");
		} else if ((ndx.chapter() != ndxPrev.chapter()) || (ndx.verse() != ndxPrev.verse())) {
			if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_NO_NUMBER) &&
				(CPersistentSettings::instance()->verseRenderingModeCopying() != VRME_VPL) &&
				(ndx != ndxFirst)) {
				scriptureHTML.appendLiteralText("  ");
			}
			if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.beginBold();
			switch (CPersistentSettings::instance()->verseNumberDelimiterMode()) {
				case RDME_NO_NUMBER:
					break;
				case RDME_NO_DELIMITER:
					if (ndx.chapter() != ndxPrev.chapter()) {
						scriptureHTML.appendLiteralText(QString("%1:%2").arg(ndx.chapter()).arg(ndx.verse()));
					} else {
						scriptureHTML.appendLiteralText(QString("%1").arg(ndx.verse()));
					}
					break;
				case RDME_SQUARE_BRACKETS:
					if (ndx.chapter() != ndxPrev.chapter()) {
						scriptureHTML.appendLiteralText(QString("[%1:%2]").arg(ndx.chapter()).arg(ndx.verse()));
					} else {
						scriptureHTML.appendLiteralText(QString("[%1]").arg(ndx.verse()));
					}
					break;
				case RDME_CURLY_BRACES:
					if (ndx.chapter() != ndxPrev.chapter()) {
						scriptureHTML.appendLiteralText(QString("{%1:%2}").arg(ndx.chapter()).arg(ndx.verse()));
					} else {
						scriptureHTML.appendLiteralText(QString("{%1}").arg(ndx.verse()));
					}
					break;
				case RDME_PARENTHESES:
					if (ndx.chapter() != ndxPrev.chapter()) {
						scriptureHTML.appendLiteralText(QString("(%1:%2)").arg(ndx.chapter()).arg(ndx.verse()));
					} else {
						scriptureHTML.appendLiteralText(QString("(%1)").arg(ndx.verse()));
					}
					break;
				case RDME_SUPERSCRIPT:
					scriptureHTML.beginSuperscript();
					if (ndx.chapter() != ndxPrev.chapter()) {
						scriptureHTML.appendLiteralText(QString("%1:%2").arg(ndx.chapter()).arg(ndx.verse()));
					} else {
						scriptureHTML.appendLiteralText(QString("%1").arg(ndx.verse()));
					}
					scriptureHTML.endSuperscript();
					break;
				default:
					assert(false);
					break;
			}
			if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.endBold();
			scriptureHTML.appendLiteralText(" ");
		}

		if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
			assert(false);
			emit changedDocumentText();
			return QString();
		}

		const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

		if (ndx.chapter() > book.m_nNumChp) {
			assert(false);
			emit changedDocumentText();
			return QString();
		}

		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndx, richifierTags, false));

		ndxPrev = ndx;
	}

	scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));

	if (CPersistentSettings::instance()->referencesAtEnd()) {
		if (CPersistentSettings::instance()->verseRenderingModeCopying() == VRME_VPL) {
			scriptureHTML.addLineBreak();
		} else {
			scriptureHTML.appendLiteralText(" ");
		}
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strReference);
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.endBold();
	}

	scriptureHTML.endParagraph();
	scriptureHTML.appendRawText("</body></html>");

	QString strRawHTML = scriptureHTML.getResult();
	m_TextDocument.setHtml(strRawHTML);
	emit changedDocumentText();
	return strRawHTML;
}

QString CPhraseNavigator::referenceStartingDelimiter()
{
	switch (CPersistentSettings::instance()->referenceDelimiterMode()) {
		case RDME_NO_DELIMITER:
			return QString();
		case RDME_SQUARE_BRACKETS:
			return QString("[");
		case RDME_CURLY_BRACES:
			return QString("{");
		case RDME_PARENTHESES:
			return QString("(");
		default:
			assert(false);
			break;
	}
	return QString();
}

QString CPhraseNavigator::referenceEndingDelimiter()
{
	switch (CPersistentSettings::instance()->referenceDelimiterMode()) {
		case RDME_NO_DELIMITER:
			return QString();
		case RDME_SQUARE_BRACKETS:
			return QString("]");
		case RDME_CURLY_BRACES:
			return QString("}");
		case RDME_PARENTHESES:
			return QString(")");
		default:
			assert(false);
			break;
	}
	return QString();
}

//#define DEBUG_CURSOR_SELECTION
TPhraseTag CPhraseNavigator::getSelection(const CPhraseCursor &aCursor,
											uint32_t *pNdxNormalFirst, uint32_t *pNdxNormalLast) const
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

#ifdef DEBUG_CURSOR_SELECTION
	int nPosCursorStart = -1;
	int nPosCursorEnd = -1;
#endif

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
		if (nIndexFirst.word() == 0) {
			nIndexFirst = CRelIndex();
		}
		if (!myCursor.moveCursorCharRight()) break;
	}
#ifdef DEBUG_CURSOR_SELECTION
	if (nIndexFirst.isSet()) nPosCursorStart = myCursor.position() - 2;		// -2 -> One for the extra moveCursorCharRight and one for the anchor character position
#endif

	// Find last word anchor:
	myCursor.setPosition(nPosLast);
	while ((myCursor.moveCursorCharLeft()) && (myCursor.charUnderCursorIsSeparator())) { }	// Note: Always move left at least one character so we don't pickup the start of the next word (short-circuit order!)
	myCursor.moveCursorWordEnd();
	bool bFoundHit = false;
	uint32_t nNormPrev = 0;
	while (myCursor.position() >= nPosFirstWordStart) {
		strAnchorName = myCursor.charFormat().anchorName();
		CRelIndex ndxCurrent = CRelIndex(strAnchorName);

		uint32_t nNormCurrent = m_pBibleDatabase->NormalizeIndex(ndxCurrent);

		if (nNormCurrent != 0) {
			// Make sure words selected are consecutive words (i.e. that we don't have a "hidden" colophon between them or something):
			//		Note: nNormPrev will be equal to nNormCurrent when we are on the first word of the line with how our
			//		paragraphs/blocks work...
			if ((bFoundHit) && (ndxCurrent.word() != 0) && (nNormPrev != (nNormCurrent+1)) && (nNormPrev != nNormCurrent)) {
				bFoundHit = false;
			}

			// If we haven't hit an anchor for an actual word within a verse, we can't be selecting
			//		text from a verse.  We must be in a special tag section of heading:
			if ((ndxCurrent.word() == 0) || (ndxCurrent < nIndexFirst)) {
				ndxCurrent = CRelIndex();
			} else {
				nNormPrev = nNormCurrent;
			}

			if (!bFoundHit) {
				nIndexLast = ndxCurrent;
				bFoundHit = nIndexLast.isSet();
			}
		}

		if (!myCursor.moveCursorCharLeft()) break;
	}
#ifdef DEBUG_CURSOR_SELECTION
	if (nIndexLast.isSet()) {
		myCursor.moveCursorWordRight();
		while ((myCursor.moveCursorCharLeft()) && (myCursor.charUnderCursorIsSeparator())) { }	// Note: Always move left at least one character so we don't pickup the start of the next word (short-circuit order!)
		nPosCursorEnd = myCursor.position() + 1;			// +1 -> One for the extra moveCursorCharLeft
	}
#endif

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
		assert(ndxNormLast >= ndxNormFirst);
		nWordCount = (ndxNormLast - ndxNormFirst + 1);
	}

	if (pNdxNormalFirst != NULL) *pNdxNormalFirst = ndxNormFirst;
	if (pNdxNormalLast != NULL) *pNdxNormalLast = ndxNormLast;

	tag.relIndex() = nIndexFirst;
	tag.count() = ((nPosFirst != nPosLast) ? nWordCount : 0);

#ifdef DEBUG_CURSOR_SELECTION
	QString strPhrase;
	if ((nPosCursorStart != -1) && (nPosCursorEnd != -1)) {
		CPhraseCursor myCursor(aCursor);
		myCursor.beginEditBlock();
		myCursor.setPosition(nPosCursorStart);
		myCursor.setPosition(nPosCursorEnd, QTextCursor::KeepAnchor);
		strPhrase = myCursor.selectedText();
		myCursor.endEditBlock();
	}

	qDebug("\"%s\"", strPhrase.toUtf8().data());
	qDebug("%s %d", m_pBibleDatabase->PassageReferenceText(tag.relIndex()).toUtf8().data(), tag.count());
#endif

	return tag;
}

//#define DEBUG_SELECTED_PHRASE
CSelectedPhrase CPhraseNavigator::getSelectedPhrase(const CPhraseCursor &aCursor) const
{
	assert(m_pBibleDatabase.data() != NULL);

	CSelectedPhrase retVal(m_pBibleDatabase);

	uint32_t ndxNormFirst = 0;
	uint32_t ndxNormLast = 0;
	retVal.tag() = getSelection(aCursor, &ndxNormFirst, &ndxNormLast);

	QString strPhrase;

	int nCount = retVal.tag().count();
	CRelIndex ndxRel = retVal.tag().relIndex();
	// If there is no selection (i.e. count is 0), and we are only in a book or chapter
	//		marker, don't return any selected phrase text:
	if ((nCount == 0) && (((ndxRel.chapter() == 0) || (ndxRel.verse() == 0)) && (ndxRel.word() == 0))) ndxRel.clear();
	if (ndxRel.isSet()) {
		// So the we'll start at the beginning of the "next verse", if we are only
		//		at the begging of the book or chapter, move to the start of the verse.
		//		In theory this won't happen because of how getSelection() currently
		//		works, but in case we ever change it for some reason:
		if ((ndxRel.chapter() == 0) && (ndxRel.word() == 0)) ndxRel.setChapter(1);
		if ((ndxRel.verse() == 0) && (ndxRel.word() == 0)) ndxRel.setVerse(1);
	}
	if (nCount == 0) nCount = 1;
	CVerseTextPlainRichifierTags tagsRichifier;
	while ((nCount > 0) && (ndxRel.isSet())) {
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndxRel);
		assert(pVerse != NULL);
		strPhrase += CVerseTextRichifier::parse(ndxRel, m_pBibleDatabase.data(), pVerse, tagsRichifier, false, &nCount);
		if (nCount) {
			// Goto the first word of this verse and add the number of words in this verse to get to start of next verse:
			ndxRel.setWord(1);
			ndxRel = m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(ndxRel) + m_pBibleDatabase->verseEntry(ndxRel)->m_nNumWrd);
			ndxRel.setWord(0);				// Process entire verse on next pass
			strPhrase += "  ";
		}
	}

	// Clean stray transChangeAddedBegin() marks:
	strPhrase = strPhrase.trimmed();
	if (strPhrase.endsWith(tagsRichifier.transChangeAddedBegin())) strPhrase = strPhrase.left(strPhrase.size() - tagsRichifier.transChangeAddedBegin().size());
	strPhrase = strPhrase.trimmed();

	retVal.phrase().ParsePhrase(strPhrase);

#ifdef DEBUG_SELECTED_PHRASE
	qDebug("\"%s\"", strPhrase.toUtf8().data());
	qDebug("%s %d", m_pBibleDatabase->PassageReferenceText(retVal.tag().relIndex()).toUtf8().data(), retVal.tag().count());
#endif

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
	if (m_pBibleDatabase->NormalizeIndex(CRelIndex(ndxScroll.book(), ndxScroll.chapter(), 1, 1)) == m_pBibleDatabase->NormalizeIndex(ndxScroll)) {
		ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading
	}
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
			} else {
				// Special-case for Book/Chapter tag so that it works (specially looks correctly) with the new
				//		special U+0x200B anchor tags now used in Book-Info and Chapter rendered text in CPhraseNavigator:
				if ((ndxRel.verse() == 0) && (ndxRel.word() == 0)) myCursor.movePosition(QTextCursor::StartOfLine);
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

#if !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)

bool CPhraseEditNavigator::handleToolTipEvent(CKJVCanOpener *pCanOpener, const QHelpEvent *pHelpEvent, CCursorFollowHighlighter &aHighlighter, const TPhraseTag &selection) const
{
	assert(m_pBibleDatabase.data() != NULL);

	assert(pHelpEvent != NULL);
	CRelIndex ndxReference = getSelection(m_TextEditor.cursorForPosition(pHelpEvent->pos())).relIndex();
	QString strToolTip = getToolTip(TPhraseTag(ndxReference, 1), selection);

	if (!strToolTip.isEmpty()) {
		highlightCursorFollowTag(aHighlighter, (selection.haveSelection() ? selection : TPhraseTag(ndxReference, 1)));
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(pCanOpener, pHelpEvent->globalPos(), strToolTip, &m_TextEditor);
		} else {
			QToolTip::showText(pHelpEvent->globalPos(), strToolTip);
		}
	} else {
		highlightCursorFollowTag(aHighlighter);
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::hideText(pCanOpener);
		} else {
			QToolTip::hideText();
		}
		return false;
	}

	return true;
}

bool CPhraseEditNavigator::handleToolTipEvent(CKJVCanOpener *pCanOpener, CCursorFollowHighlighter &aHighlighter, const TPhraseTag &tag, const TPhraseTag &selection) const
{
	assert(m_pBibleDatabase.data() != NULL);

	QString strToolTip = getToolTip(tag, selection);

	if (!strToolTip.isEmpty()) {
		highlightCursorFollowTag(aHighlighter, (selection.haveSelection() ? selection : TPhraseTag(tag.relIndex(), 1)));
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::showText(pCanOpener, m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip, m_TextEditor.viewport(), m_TextEditor.rect());
		} else {
			QToolTip::showText(m_TextEditor.mapToGlobal(m_TextEditor.cursorRect().topRight()), strToolTip);
		}
	} else {
		highlightCursorFollowTag(aHighlighter);
		if (m_bUseToolTipEdit) {
			QToolTip::hideText();
			CToolTipEdit::hideText(pCanOpener);
		} else {
			QToolTip::hideText();
		}
		return false;
	}

	return true;
}

#endif

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
						strToolTip += tr("Word:", "Statistics") + " \"" + m_pBibleDatabase->wordAtIndex(ndxNormal) + "\"\n";
					}
				}
				strToolTip += m_pBibleDatabase->SearchResultToolTip(ndxReference);
			} else {
				strToolTip += tr("Phrase:", "Statistics") + " \"";
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
					strToolTip += QString("\n%1 ").arg(m_pBibleDatabase->bookName(ndxReference)) + tr("contains:", "Statistics") + "\n"
											"    " + tr("%n Chapter(s)", "Statistics", m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) + "\n"
											"    " + tr("%n Verse(s)", "Statistics", m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumVrs) + "\n"
											"    " + tr("%n Word(s)", "Statistics", m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumWrd) + "\n";
					if (ndxReference.chapter() != 0) {
						assert(ndxReference.chapter() <= m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp);
						if (ndxReference.chapter() <= m_pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) {
							strToolTip += QString("\n%1 %2 ").arg(m_pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()) + tr("contains:", "Statistics") + "\n"
											"    " + tr("%n Verse(s)", "Statistics", m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) + "\n"
											"    " + tr("%n Word(s)", "Statistics", m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumWrd) + "\n";
							if ((!bHaveSelection) && (ndxReference.verse() != 0)) {
								assert(ndxReference.verse() <= m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs);
								if (ndxReference.verse() <= m_pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) {
									strToolTip += QString("\n%1 %2:%3 ").arg(m_pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()).arg(ndxReference.verse()) + tr("contains:", "Statistics") + "\n"
											"    " + tr("%n Word(s)", "Statistics", m_pBibleDatabase->verseEntry(ndxReference)->m_nNumWrd) + "\n";
								}
							}
						}
					}
				}
			}
			if (bHaveSelection) {
				strToolTip += "\n" + tr("%n Word(s) Selected", "Statistics", selection.count()) + "\n";
			}
		}
		if (!bPlainText) strToolTip += "</pre></body></html>";
	}

	return strToolTip;
}

// ============================================================================

