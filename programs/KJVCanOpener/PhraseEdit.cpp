/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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
#include "SearchCompleter.h"
#ifdef QT_WIDGETS_LIB
#include "ToolTipEdit.h"
#include <QToolTip>
#endif
#include "Translator.h"

#include <QStringListModel>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>
#include <QPair>
#include <QSet>

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#endif
#if QT_VERSION < 0x050F00
#include <QRegExp>
#endif

#include <algorithm>
#include <string>

#include <assert.h>

// ============================================================================

//#define DEBUG_CURSOR_SELECTION
//#define DEBUG_SELECTED_PHRASE

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

	return strRawPhrase.trimmed();
}

// ============================================================================

CSubPhrase::CSubPhrase()
	:	m_nLevel(0),
		m_nCursorLevel(0),
		m_nCursorWord(-1)
{

}

CSubPhrase::CSubPhrase(const CSubPhrase &aSrc)
	:	m_nLevel(aSrc.m_nLevel),
		m_lstMatchMapping(aSrc.m_lstMatchMapping),
		m_nCursorLevel(aSrc.m_nCursorLevel),
		m_lstNextWords(aSrc.m_lstNextWords),
		m_lstWords(aSrc.m_lstWords),
		m_nCursorWord(aSrc.m_nCursorWord),
		m_strCursorWord(aSrc.m_strCursorWord)
{

}

CSubPhrase &CSubPhrase::operator =(const CSubPhrase &aSrc)
{
	m_nLevel = aSrc.m_nLevel;
	m_lstMatchMapping = aSrc.m_lstMatchMapping;
	m_nCursorLevel = aSrc.m_nCursorLevel;
	m_lstNextWords = aSrc.m_lstNextWords;
	m_lstWords = aSrc.m_lstWords;
	m_nCursorWord = aSrc.m_nCursorWord;
	m_strCursorWord = aSrc.m_strCursorWord;
	return *this;
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

QString CSubPhrase::phraseToSpeak() const
{
	static const CVerseTextPlainRichifierTags tagsRichifier;
	QString strPhrase = phraseWords().join(" ");
	strPhrase.remove(tagsRichifier.transChangeAddedBegin());
	strPhrase.remove(tagsRichifier.transChangeAddedEnd());
	return strPhrase;
}

int CSubPhrase::phraseSize() const
{
	return phraseWords().size();
}

int CSubPhrase::phraseRawSize() const
{
	return phraseWordsRaw().size();
}

int CSubPhrase::phraseToSpeakSize() const
{
	return phraseWordsToSpeak().size();
}

QStringList CSubPhrase::phraseWords() const
{
	QStringList strPhraseWords;

// For some reason, this reserve() continually causes the Windows cross-build
//	to just hang, deep down in QList at this p.detach(alloc):
//		$$[QT_INSTALL_HEADERS]/QtCore/qlist.h
//
//		template <typename T>
//		Q_OUTOFLINE_TEMPLATE void QList<T>::detach_helper(int alloc)
//		{
//			Node *n = reinterpret_cast<Node *>(p.begin());
//			QListData::Data *x = p.detach(alloc);
//
// Since our list is relatively small, there really isn't much need
//	to call it anyway:
//
//	strPhraseWords.reserve(m_lstWords.size());

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

QStringList CSubPhrase::phraseWordsToSpeak() const
{
	static const CVerseTextPlainRichifierTags tagsRichifier;

	QStringList strPhraseWords;

	strPhraseWords.reserve(m_lstWords.size());

	for (int ndx = 0; ndx < m_lstWords.size(); ++ndx) {
		QString strWord = m_lstWords.at(ndx);
		strWord.remove(tagsRichifier.transChangeAddedBegin());
		strWord.remove(tagsRichifier.transChangeAddedEnd());
		if (!strWord.isEmpty()) strPhraseWords.append(strWord);
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

void CSubPhrase::ClearPhase()
{
	m_lstWords.clear();
	m_strCursorWord.clear();
	m_nCursorWord = -1;
	// Note: m_nLevel and m_nCursorLevel get cleared in FindWord()
}

void CSubPhrase::ParsePhrase(const QString &strPhrase)
{
#if QT_VERSION >= 0x050E00
	m_lstWords = strPhrase.normalized(QString::NormalizationForm_C).split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts);
#else
	m_lstWords = strPhrase.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), My_QString_SkipEmptyParts);
#endif
	m_strCursorWord.clear();
	m_nCursorWord = m_lstWords.size();
}

void CSubPhrase::ParsePhrase(const QStringList &lstPhrase)
{
	m_lstWords = lstPhrase;
	m_strCursorWord.clear();
	m_nCursorWord = m_lstWords.size();
}

void CSubPhrase::AppendPhrase(const QString &strPhrase)
{
#if QT_VERSION >= 0x050E00
	m_lstWords.append(strPhrase.normalized(QString::NormalizationForm_C).split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts));
#else
	m_lstWords.append(strPhrase.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), My_QString_SkipEmptyParts));
#endif
	m_strCursorWord.clear();
	m_nCursorWord = m_lstWords.size();
}

void CSubPhrase::AppendPhrase(const QStringList &lstPhrase)
{
	m_lstWords.append(lstPhrase);
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
		m_nActiveSubPhrase(-1),
		m_bHasChanged(false)
{
	m_pPrimarySubPhrase = attachSubPhrase(new CSubPhrase);
}

CParsedPhrase::CParsedPhrase(const CParsedPhrase &aSrc)
{
	m_pPrimarySubPhrase = attachSubPhrase(new CSubPhrase);
	*this = aSrc;
}

CParsedPhrase &CParsedPhrase::operator=(const CParsedPhrase &aSrc)
{
	m_pBibleDatabase = aSrc.m_pBibleDatabase;
	m_bIsDuplicate = aSrc.m_bIsDuplicate;
	m_bIsDisabled = aSrc.m_bIsDisabled;
	m_bCaseSensitive = aSrc.m_bCaseSensitive;
	m_bAccentSensitive = aSrc.m_bAccentSensitive;
	m_bExclude = aSrc.m_bExclude;
	m_nActiveSubPhrase = aSrc.m_nActiveSubPhrase;

	m_lstSubPhrases.clear();
	for (int ndx = 0; ndx < aSrc.m_lstSubPhrases.size(); ++ndx) {
		QSharedPointer<CSubPhrase> subPhrase;
		if (ndx == 0) {
			m_pPrimarySubPhrase->ClearPhase();
			subPhrase = attachSubPhrase(m_pPrimarySubPhrase);
		} else {
			subPhrase = attachSubPhrase(new CSubPhrase);
		}
		*subPhrase = *aSrc.m_lstSubPhrases.at(ndx).data();
	}
	assert(!m_lstSubPhrases.isEmpty());

	m_bHasChanged = false;

	clearCache();

	return *this;
}

CParsedPhrase::~CParsedPhrase()
{
	// Release all smart pointers pointing to us:
	for (int ndx=0; ndx<m_lstSmartPointers.size(); ++ndx) {
		m_lstSmartPointers.at(ndx)->clear();
	}
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

	if (!m_pBibleDatabase.isNull())
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
	assert(!m_pBibleDatabase.isNull());

	if (m_cache_lstPhraseTagResults.size()) return m_cache_lstPhraseTagResults;
	if (m_lstSubPhrases.isEmpty()) return m_cache_lstPhraseTagResults;		// This condition should never happen since we always have m_pPrimarySubPhrase in the list

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
			m_cache_lstPhraseTagResults.append(TPhraseTag(m_pBibleDatabase->DenormalizeIndex(ndxNormal), subPhrase->phraseSize()));
	}
	std::sort(m_cache_lstPhraseTagResults.begin(), m_cache_lstPhraseTagResults.end(), TPhraseTagListSortPredicate::ascendingLessThan);

	// Intersecting insert the other subphrases:
	for (int ndxSubPhrase = 0; ndxSubPhrase < m_lstSubPhrases.size(); ++ndxSubPhrase) {
		if (ndxSubPhrase == ndxMax) continue;
		subPhrase = m_lstSubPhrases.at(ndxSubPhrase).data();

		TPhraseTagList lstSubPhraseTags;
		lstSubPhraseTags.reserve(subPhrase->m_lstMatchMapping.size());
		for (unsigned int ndxWord=0; ndxWord<subPhrase->m_lstMatchMapping.size(); ++ndxWord) {
			uint32_t ndxNormal = (subPhrase->m_lstMatchMapping.at(ndxWord) - subPhrase->m_nLevel + 1);
			if (ndxNormal > 0) lstSubPhraseTags.append(TPhraseTag(m_pBibleDatabase->DenormalizeIndex(ndxNormal), subPhrase->phraseSize()));
		}
		std::sort(lstSubPhraseTags.begin(), lstSubPhraseTags.end(), TPhraseTagListSortPredicate::ascendingLessThan);

		m_cache_lstPhraseTagResults.intersectingInsert(m_pBibleDatabase.data(), lstSubPhraseTags);
	}

	// Note: Results of the intersectingInsert are already sorted

	return m_cache_lstPhraseTagResults;
}

static bool ascendingLessThanMatchingPhrases(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
	return (s1.first.compare(s2.first, Qt::CaseInsensitive) < 0);
}

QStringList CParsedPhrase::GetMatchingPhrases() const
{
	assert(!m_pBibleDatabase.isNull());

	const TPhraseTagList &lstTags = GetPhraseTagSearchResults();
	QList<QPair<QString, int> > lstMatchingPhrasesSort;
	lstMatchingPhrasesSort.reserve(lstTags.size());

	for (int ndx = 0; ndx < lstTags.size(); ++ndx) {
		if (!lstTags.at(ndx).isSet()) continue;
		uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(lstTags.at(ndx).relIndex());
		QStringList lstPhraseWordsDecomposed;
		lstPhraseWordsDecomposed.reserve(lstTags.at(ndx).count());
		for (unsigned int nWrd = 0; nWrd < lstTags.at(ndx).count(); ++nWrd) {
			lstPhraseWordsDecomposed.append(m_pBibleDatabase->decomposedWordAtIndex(ndxNormal));
			++ndxNormal;
		}
		lstMatchingPhrasesSort.append(QPair<QString, int>(lstPhraseWordsDecomposed.join(QChar(' ')), ndx));
	}
	std::sort(lstMatchingPhrasesSort.begin(), lstMatchingPhrasesSort.end(), ascendingLessThanMatchingPhrases);

	QStringList lstMatchingPhrases;
	lstMatchingPhrases.reserve(lstTags.size());
	for (int i = 0; i < lstMatchingPhrasesSort.size(); ++i) {
		int ndx = lstMatchingPhrasesSort.at(i).second;
		uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(lstTags.at(ndx).relIndex());
		QStringList lstPhraseWords;
		lstPhraseWords.reserve(lstTags.at(ndx).count());
		for (unsigned int nWrd = 0; nWrd < lstTags.at(ndx).count(); ++nWrd) {
			lstPhraseWords.append(m_pBibleDatabase->wordAtIndex(ndxNormal, true));
			++ndxNormal;
		}

		QString strPhrase = lstPhraseWords.join(QChar(' '));
		if (!lstMatchingPhrases.contains(strPhrase, Qt::CaseSensitive))
			lstMatchingPhrases.append(strPhrase);
	}
	return lstMatchingPhrases;
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

QString CParsedPhrase::phraseToSpeak() const
{
	QStringList lstPhrases;

	lstPhrases.reserve(m_lstSubPhrases.size());
	for (int ndx = 0; ndx < m_lstSubPhrases.size(); ++ndx) {
		lstPhrases.append(m_lstSubPhrases.at(ndx)->phraseToSpeak());
	}

	return lstPhrases.join(" ");
}

const QStringList CParsedPhrase::phraseWords() const
{
#if QT_VERSION >= 0x050E00
	return phrase().split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts);
#else
	return phrase().split(QRegExp("\\s+"), My_QString_SkipEmptyParts);
#endif
}

const QStringList CParsedPhrase::phraseWordsRaw() const
{
#if QT_VERSION >= 0x050E00
	return phraseRaw().split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts);
#else
	return phraseRaw().split(QRegExp("\\s+"), My_QString_SkipEmptyParts);
#endif
}

const QStringList CParsedPhrase::phraseWordsToSpeak() const
{
#if QT_VERSION >= 0x050E00
	return phraseToSpeak().split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts);
#else
	return phraseToSpeak().split(QRegExp("\\s+"), My_QString_SkipEmptyParts);
#endif
}

void CParsedPhrase::clearCache() const
{
	m_cache_lstPhraseTagResults = TPhraseTagList();
}

void CParsedPhrase::UpdateCompleter(const QTextCursor &curInsert, CSearchCompleter &aCompleter)
{
	ParsePhrase(curInsert);
	FindWords();
#ifdef QT_WIDGETS_LIB
	aCompleter.setFilterMatchString();
	aCompleter.setWordsFromPhrase();
#else
	Q_UNUSED(aCompleter);
#endif
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

#if QT_VERSION >= 0x050500
	if (chrL.isNull() || bLIsSeparator) strRightText = strRightText.mid(strRightText.indexOf(QRegularExpression("\\S")));
#else
	if (chrL.isNull() || bLIsSeparator) strRightText = strRightText.mid(strRightText.indexOf(QRegExp("\\S")));
#endif

	QChar chrR = (strRightText.size() ? strRightText.at(0) : QChar());
	bool bRIsSpace = chrR.isSpace();
	bool bRIsOR = (chrR == QChar('|'));
	bool bRIsSeparator = (bRIsSpace || bRIsOR);

	if (!bRIsSeparator) {
#if QT_VERSION >= 0x050500
		int nPosRSpace = strRightText.indexOf(QRegularExpression("\\s+"));
#else
		int nPosRSpace = strRightText.indexOf(QRegExp("\\s+"));
#endif
		strLeftText += strRightText.left(nPosRSpace);
		if (nPosRSpace != -1) {
			strRightText = strRightText.mid(nPosRSpace);
		} else {
			strRightText.clear();
		}
	}

	ParsePhrase(strComplete);
	assert(!m_lstSubPhrases.isEmpty());

	strComplete.replace(QString("|"), QString(" | "));		// Make sure we have separation around the "OR" operators so we break them into individual elements below...
#if QT_VERSION >= 0x050E00
	QStringList lstCompleteWords = strComplete.normalized(QString::NormalizationForm_C).split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts);
#else
	QStringList lstCompleteWords = strComplete.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), My_QString_SkipEmptyParts);
#endif

	strLeftText.replace(QString("|"), QString(" | "));		// Make sure we have separation around the "OR" operators so we break them into individual elements below...
#if QT_VERSION >= 0x050E00
	QStringList lstLeftWords = strLeftText.normalized(QString::NormalizationForm_C).split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts);
#else
	QStringList lstLeftWords = strLeftText.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), My_QString_SkipEmptyParts);
#endif

	strRightText.replace(QString("|"), QString(" | "));		// Make sure we have separation around the "OR" operators so we break them into individual elements below...
#if QT_VERSION >= 0x050E00
	QStringList lstRightWords = strRightText.normalized(QString::NormalizationForm_C).split(QRegularExpression("\\s+"), My_QString_SkipEmptyParts);
#else
	QStringList lstRightWords = strRightText.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), My_QString_SkipEmptyParts);
#endif

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

void CParsedPhrase::ParsePhrase(const QString &strPhrase, bool bFindWords)
{
	clearCache();

	QStringList lstPhrases = strPhrase.split(QChar('|'));
	assert(lstPhrases.size() >= 1);

	m_lstSubPhrases.clear();
	m_lstSubPhrases.reserve(lstPhrases.size());
	for (int ndx=0; ndx<lstPhrases.size(); ++ndx) {
		QSharedPointer<CSubPhrase> subPhrase;
		if (ndx == 0) {
			m_pPrimarySubPhrase->ClearPhase();
			subPhrase = attachSubPhrase(m_pPrimarySubPhrase);
		} else {
			subPhrase = attachSubPhrase(new CSubPhrase);
		}
		subPhrase->ParsePhrase(lstPhrases.at(ndx));
	}
	assert(!m_lstSubPhrases.isEmpty());

	m_nActiveSubPhrase = m_lstSubPhrases.size()-1;
	if (bFindWords) FindWords();
}

void CParsedPhrase::ParsePhrase(const QStringList &lstPhrase, bool bFindWords)
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
			QSharedPointer<CSubPhrase> subPhrase;
			if (m_lstSubPhrases.isEmpty()) {
				m_pPrimarySubPhrase->ClearPhase();
				subPhrase = attachSubPhrase(m_pPrimarySubPhrase);
			} else {
				subPhrase = attachSubPhrase(new CSubPhrase);
			}
			subPhrase->ParsePhrase(lstSubPhrase);
		}
		if (ndxFrom != -1) ++ndxFrom;			// Skip the 'OR'
	}

	assert(!m_lstSubPhrases.isEmpty());			// Should have inserted a phrase above
	if (m_lstSubPhrases.isEmpty()) attachSubPhrase(m_pPrimarySubPhrase);	// Failsafe

	m_nActiveSubPhrase = m_lstSubPhrases.size()-1;
	if (bFindWords) FindWords();
}

void CParsedPhrase::FindWords()
{
	for (int ndxSubPhrase = 0; ndxSubPhrase < m_lstSubPhrases.size(); ++ndxSubPhrase)
		FindWords(*m_lstSubPhrases[ndxSubPhrase]);
}

void CParsedPhrase::ResumeFindWords()
{
	for (int ndxSubPhrase = 0; ndxSubPhrase < m_lstSubPhrases.size(); ++ndxSubPhrase)
		FindWords(*m_lstSubPhrases[ndxSubPhrase], true);
}

void CParsedPhrase::FindWords(CSubPhrase &subPhrase, bool bResume)
{
	clearCache();			// Clear cache since it will no longer be valid

	assert(!m_pBibleDatabase.isNull());

	int nCursorWord = subPhrase.m_nCursorWord;
	assert((nCursorWord >= 0) && (nCursorWord <= subPhrase.m_lstWords.size()));

	bool bComputedNextWords = false;
	if (!bResume) {
		subPhrase.m_lstMatchMapping.clear();
		subPhrase.m_nLevel = 0;
		subPhrase.m_nCursorLevel = 0;
	}
	bool bInFirstWordStar = false;
	for (int ndx=subPhrase.m_nLevel; ndx<subPhrase.m_lstWords.size(); ++ndx) {
		if (subPhrase.m_lstWords.at(ndx).isEmpty()) continue;

		QString strCurWordDecomp = CSearchStringListModel::decompose(subPhrase.m_lstWords.at(ndx), true);
		QString strCurWord = (isAccentSensitive() ? CSearchStringListModel::deApostrHyphen(subPhrase.m_lstWords.at(ndx), !m_pBibleDatabase->settings().hyphenSensitive()) :
													CSearchStringListModel::decompose(subPhrase.m_lstWords.at(ndx), !m_pBibleDatabase->settings().hyphenSensitive()));

		QString strCurWordKey = strCurWordDecomp.toLower();
		QString strCurWordWildKey = strCurWordKey;			// Note: This becomes the "Word*" value later, so can't substitute strCurWordWild for all m_lstWords.at(ndx) (or strCurWord)
#if QT_VERSION >= 0x050500
		int nPreRegExp = strCurWordWildKey.indexOf(QRegularExpression("[\\[\\]\\*\\?]"));
#else
		int nPreRegExp = strCurWordWildKey.indexOf(QRegExp("[\\[\\]\\*\\?]"));
#endif

		if (nPreRegExp == -1) {
			if ((ndx == (subPhrase.m_lstWords.size()-1)) &&
				(m_pBibleDatabase->mapWordList().find(strCurWordKey) == m_pBibleDatabase->mapWordList().end())) {
				strCurWordWildKey += "*";			// If we're on the word currently being typed and it's not an exact match, simulate a "*" trailing wildcard to match all strings with this prefix
			}
		}

#if QT_VERSION >= 0x050F00
		QRegularExpression expCurWordWildKey(QRegularExpression::wildcardToRegularExpression(strCurWordWildKey), QRegularExpression::CaseInsensitiveOption);
		QRegularExpression expCurWordExactKey(QRegularExpression::wildcardToRegularExpression(strCurWordKey), QRegularExpression::CaseInsensitiveOption);
		QRegularExpression expCurWord(QRegularExpression::wildcardToRegularExpression(strCurWord), (isCaseSensitive() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption));
#else
		QRegExp expCurWordWildKey(strCurWordWildKey, Qt::CaseInsensitive, QRegExp::Wildcard);
		QRegExp expCurWordExactKey(strCurWordKey, Qt::CaseInsensitive, QRegExp::Wildcard);
		QRegExp expCurWord(strCurWord, (isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive), QRegExp::Wildcard);
#endif

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
#if QT_VERSION >= 0x050F00
					if (!expCurWordExactKey.match(m_pBibleDatabase->lstWordList().at(ndxWord)).hasMatch()) continue;
#else
					if (!expCurWordExactKey.exactMatch(m_pBibleDatabase->lstWordList().at(ndxWord))) continue;
#endif
					TWordListMap::const_iterator itrWordMap = m_pBibleDatabase->mapWordList().find(m_pBibleDatabase->lstWordList().at(ndxWord));
					assert(itrWordMap != m_pBibleDatabase->mapWordList().end());
					if (itrWordMap == m_pBibleDatabase->mapWordList().end()) continue;

					const CWordEntry &wordEntry = itrWordMap->second;		// Entry for current word

					if ((!isCaseSensitive()) && (!isAccentSensitive()) && (!m_pBibleDatabase->settings().hyphenSensitive())) {
						subPhrase.m_lstMatchMapping.insert(subPhrase.m_lstMatchMapping.end(), wordEntry.m_ndxNormalizedMapping.begin(), wordEntry.m_ndxNormalizedMapping.end());
					} else {
						unsigned int nCount = 0;
						for (int ndxAltWord = 0; ndxAltWord<wordEntry.m_lstAltWords.size(); ++ndxAltWord) {
							const QString &strAltWord = ((!isAccentSensitive()) ?
									 ((!m_pBibleDatabase->settings().hyphenSensitive()) ? wordEntry.m_lstDecomposedAltWords.at(ndxAltWord) : wordEntry.m_lstDecomposedHyphenAltWords.at(ndxAltWord)) :
									 ((!m_pBibleDatabase->settings().hyphenSensitive()) ? wordEntry.m_lstDeApostrAltWords.at(ndxAltWord) : wordEntry.m_lstDeApostrHyphenAltWords.at(ndxAltWord)));
#if QT_VERSION >= 0x050F00
							if (expCurWord.match(strAltWord).hasMatch()) {
#else
							if (expCurWord.exactMatch(strAltWord)) {
#endif
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
					const CConcordanceEntry *pNextWordEntry = m_pBibleDatabase->concordanceEntryForWordAtIndex(subPhrase.m_lstMatchMapping.at(ndxWord)+1);
					assert(pNextWordEntry != nullptr);
					const QString &strNextWord = ((!isAccentSensitive()) ?
							 ((!m_pBibleDatabase->settings().hyphenSensitive()) ? pNextWordEntry->decomposedWord() : pNextWordEntry->decomposedHyphenWord()) :
							 ((!m_pBibleDatabase->settings().hyphenSensitive()) ? pNextWordEntry->deApostrWord() : pNextWordEntry->deApostrHyphenWord()));
#if QT_VERSION >= 0x050F00
					if (expCurWord.match(strNextWord).hasMatch()) {
#else
					if (expCurWord.exactMatch(strNextWord)) {
#endif
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
					// Add our resulting Concordance Indexes to a QSet to remove duplicates and
					//		create a unique list that we can then turn to strings:
					subPhrase.m_lstNextWords.clear();
					QSet<int> setNextWords;
					setNextWords.reserve(subPhrase.m_lstMatchMapping.size());
					for (unsigned int ndxWord=0; ndxWord<subPhrase.m_lstMatchMapping.size(); ++ndxWord) {
						if ((subPhrase.m_lstMatchMapping.at(ndxWord)+1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) {
							int nConcordanceIndex = m_pBibleDatabase->concordanceIndexForWordAtIndex(subPhrase.m_lstMatchMapping.at(ndxWord)+1);
							assert(nConcordanceIndex != -1);
							setNextWords.insert(nConcordanceIndex);
						}
					}

					subPhrase.m_lstNextWords.reserve(setNextWords.size());
					for (QSet<int>::const_iterator itrNdxWord = setNextWords.constBegin(); itrNdxWord != setNextWords.constEnd(); ++itrNdxWord) {
						const CConcordanceEntry &nextWordEntry(m_pBibleDatabase->concordanceWordList().at(*itrNdxWord));
						subPhrase.m_lstNextWords.append(nextWordEntry);
					}

					std::sort(subPhrase.m_lstNextWords.begin(), subPhrase.m_lstNextWords.end(), TConcordanceListSortPredicate::ascendingLessThanWordCaseInsensitive);
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
	m_richifierTagsDisplay.setFromPersistentSettings(*CPersistentSettings::instance(), false);
	m_richifierTagsCopying.setFromPersistentSettings(*CPersistentSettings::instance(), true);

	connect(CPersistentSettings::instance(), SIGNAL(changedColorWordsOfJesus(const QColor &)), this, SLOT(en_WordsOfJesusColorChanged(const QColor &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedColorSearchResults(const QColor &)), this, SLOT(en_SearchResultsColorChanged(const QColor &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedShowPilcrowMarkers(bool)), this, SLOT(en_changedShowPilcrowMarkers(bool)));
	connect(CPersistentSettings::instance(), SIGNAL(changedCopyOptions()), this, SLOT(en_changedCopyOptions()));
}

void CPhraseNavigator::en_changedCopyOptions()
{
	m_richifierTagsCopying.setFromPersistentSettings(*CPersistentSettings::instance(), true);
}

int CPhraseNavigator::anchorPosition(const QString &strAnchorName) const
{
	assert(!m_pBibleDatabase.isNull());

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
	assert(!m_pBibleDatabase.isNull());

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
				ndxRel = m_pBibleDatabase->DenormalizeIndex(ndxNormalStart);
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
				assert(pCurrentWordVerseEntry != nullptr);
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
			nStartPos = anchorPosition(m_pBibleDatabase->DenormalizeIndex(ndxNormalStart).asAnchor());
		}
	}

	myCursor.endEditBlock();
}

TPhraseTagList CPhraseNavigator::currentChapterDisplayPhraseTagList(const CRelIndex &ndxCurrent) const
{
	TPhraseTagList tagsCurrentDisplay;

	if ((ndxCurrent.isSet()) && (ndxCurrent.book() != 0) && (ndxCurrent.book() <= m_pBibleDatabase->bibleEntry().m_nNumBk)) {
		// Note: If this is a colophon reference, find the corresponding last chapter:
		CRelIndex ndxDisplay = CRelIndex(ndxCurrent.book(), ((ndxCurrent.chapter() != 0) ? ndxCurrent.chapter() : m_pBibleDatabase->bookEntry(ndxCurrent.book())->m_nNumChp), 0, 1);
		// This can happen if the versification of the reference doesn't match the active database:
		if (m_pBibleDatabase->NormalizeIndex(ndxDisplay) == 0) return TPhraseTagList();

		// Main Chapter:
		const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndxDisplay);
		assert(pChapter != nullptr);
		tagsCurrentDisplay.append((TPhraseTag(ndxDisplay, pChapter->m_nNumWrd)));

		// Verse Before:
		CRelIndex ndxVerseBefore = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndxDisplay.book(), ndxDisplay.chapter(), 1, 1), true);	// Calculate one verse prior to the first verse of this book/chapter
		if (ndxVerseBefore.isSet()) {
			const CVerseEntry *pVerseBefore = m_pBibleDatabase->verseEntry(ndxVerseBefore);
			assert(pVerseBefore != nullptr);
			tagsCurrentDisplay.append(TPhraseTag(ndxVerseBefore, pVerseBefore->m_nNumWrd));
		}

		// Verse After:
		CRelIndex ndxVerseAfter = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndxDisplay.book(), ndxDisplay.chapter(), 1, 1), false);	// Calculate first verse of next chapter
		if (ndxVerseAfter.isSet()) {
			const CVerseEntry *pVerseAfter = m_pBibleDatabase->verseEntry(ndxVerseAfter);
			assert(pVerseAfter != nullptr);
			tagsCurrentDisplay.append(TPhraseTag(ndxVerseAfter, pVerseAfter->m_nNumWrd));
		}

		// If this book has a colophon and this is the last chapter of that book, we
		//	need to add it as well:
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxDisplay.book());
		assert(pBook != nullptr);
		if ((pBook->m_bHaveColophon) && (ndxDisplay.chapter() == pBook->m_nNumChp)) {
			const CVerseEntry *pBookColophon = m_pBibleDatabase->verseEntry(CRelIndex(ndxDisplay.book(), 0, 0, 0));
			assert(pBookColophon != nullptr);
			tagsCurrentDisplay.append(TPhraseTag(CRelIndex(ndxDisplay.book(), 0, 0, 1), pBookColophon->m_nNumWrd));
		}

		// If the ndxVerseBefore is in a different book, check that book to see if
		//	it has a colophon and if so, add it so that we will render highlighting
		//	and other markup for it:
		if ((ndxVerseBefore.isSet()) && (ndxVerseBefore.book() != ndxDisplay.book())) {
			const CBookEntry *pBookVerseBefore = m_pBibleDatabase->bookEntry(ndxVerseBefore.book());
			assert(pBookVerseBefore != nullptr);
			if (pBookVerseBefore->m_bHaveColophon) {
				const CVerseEntry *pPrevBookColophon = m_pBibleDatabase->verseEntry(CRelIndex(ndxVerseBefore.book(), 0, 0, 0));
				assert(pPrevBookColophon != nullptr);
				tagsCurrentDisplay.append(TPhraseTag(CRelIndex(ndxVerseBefore.book(), 0, 0, 1), pPrevBookColophon->m_nNumWrd));
			}
		}
	}

	return tagsCurrentDisplay;
}

QString CPhraseNavigator::setDocumentToBookInfo(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO)
{
	assert(!m_pBibleDatabase.isNull());
	assert(!g_pUserNotesDatabase.isNull());

	bool bTotalColophonAnchor = (!(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoColophonAnchors));

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		m_TextDocument.clear();
	}

	if (ndx.book() == 0) return QString();

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	// Search for "Category:".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strCategory = tr("Category:", "Scope");
	TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
	if (!pTranslator.isNull()) {
		QString strTemp = pTranslator->translatorApp().translate("CPhraseNavigator", "Category:", "Scope");
		if (!strTemp.isEmpty()) strCategory = strTemp;
	}

	CScriptureTextHtmlBuilder scriptureHTML;

	CPhraseNavigator::COPY_FONT_SELECTION_ENUM cfseToUse = CFSE_NONE;
	if (flagsTRO & TRO_SearchResults) cfseToUse = CFSE_SEARCH_RESULTS;
	if (flagsTRO & TRO_Copying) cfseToUse = (CPersistentSettings::instance()->copyFontSelection());
	QString strCopyFont= "font-size:medium;";
	switch (cfseToUse) {
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

	double nLineHeight = 1.0;
	if (flagsTRO & TRO_ScriptureBrowser) {
		nLineHeight = CPersistentSettings::instance()->scriptureBrowserLineHeight();
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n.subtitle { font-size:12pt; font-weight:normal; }\n.category { font-size:12pt; font-weight:normal; }\n</style></head><body>\n")
//											.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
//											.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
											"<title>%1</title><style type=\"text/css\">\n"
											"body, p, li, br, .bodyIndent { white-space: pre-line; line-height:%3; %2 }\n"
											".book { font-size:xx-large; font-weight:bold; }\n"
											".chapter { font-size:x-large; font-weight:bold; }\n"
											".verse { }\n"
											".word { }\n"
											".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".category { font-size:medium; font-weight:normal; }\n"
											".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
											"</style></head><body>\n")
											.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
											.arg(strCopyFont)																// Copy Font
											.arg(QString("%1").arg(nLineHeight*100, 0, 'f', 0) + "%"));						// Line-Height
	}

	CRelIndex ndxBookChap(ndx.book(), ndx.chapter(), 0, 0);
	CRelIndex ndxBook(ndx.book(), 0, 0, 0);

	// Print Heading for this Book:
	scriptureHTML.beginDiv("book");
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.beginAnchorID(ndxBook.asAnchor());
	scriptureHTML.appendLiteralText(book.m_strBkName);
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.endAnchor();
	// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
	//		at the end of the book name so people adding notes/cross-refs for the book
	//		aren't confused by it being at the beginning of the name.  But then switch
	//		back to a book reference so that book category/descriptions are properly
	//		labeled:
	if (!(flagsTRO & TRO_NoAnchors)) {
		if (!(flagsTRO & TRO_NoChapterAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChap.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
		if (!(flagsTRO & TRO_NoBookAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBook.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
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
		scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
	}
	// If we have a User Note for this book, print it too:
	if ((flagsTRO & TRO_UserNotes) &&
		(scriptureHTML.addNoteFor(ndxBook, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
		scriptureHTML.insertHorizontalRule();

	// Add colophon for the book if it exists and we are instructed to add it:
	if ((flagsTRO & TRO_Colophons) && (book.m_bHaveColophon)) {
		// Try pseudo-verse (searchable) style first:
		scriptureHTML.beginDiv("colophon");
		scriptureHTML.beginParagraph();
		scriptureHTML.appendRawText("<span class=\"verse\">");
		if (bTotalColophonAnchor) {
			CRelIndex ndxColophon(ndxBook);
			ndxColophon.setWord(1);
			scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxColophon.asAnchor()));
		}
		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxBook, ((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay), !(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)));
		if (bTotalColophonAnchor) {
			scriptureHTML.appendRawText("</a>");
		}
		scriptureHTML.appendRawText("</span>");
		scriptureHTML.endParagraph();
		scriptureHTML.endDiv();
	} else {
		// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
		scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
		if ((flagsTRO & TRO_Colophons) &&
			(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
			scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the colophon divison ahead of footnote
			scriptureHTML.beginDiv("colophon");
			scriptureHTML.flushBuffer();
			scriptureHTML.endDiv();
		}
		scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		scriptureHTML.appendRawText("</body></html>");
	}

	QString strRawHTML = scriptureHTML.getResult();
	if ((flagsTRO & TRO_InnerHTML) == 0) {
		m_TextDocument.setHtml(strRawHTML);
		emit changedDocumentText();
	}
	return strRawHTML;
}

QString CPhraseNavigator::setDocumentToChapter(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO, const CBasicHighlighter *pHighlighter)
{
	assert(!m_pBibleDatabase.isNull());
	assert(!g_pUserNotesDatabase.isNull());

	bool bTotalColophonAnchor = (!(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoColophonAnchors));
	bool bTotalSuperscriptionAnchor =  (!(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoSuperscriptAnchors));

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		m_TextDocument.clear();
	}

	if ((ndx.book() == 0) || (ndx.chapter() == 0)) return QString();

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		// Note: This condition can happen if we were given an invalid passage to
		//	render for our database, such as the made-up previews in the settings
		//	for certain Bible databases:
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndx.book());

	const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndx);
	if (pChapter == nullptr) {
		assert(false);
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	if (ndx.chapter() > book.m_nNumChp) {
		assert(false);
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	// Search for "Category:".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strCategory = tr("Category:", "Scope");
	TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
	if (!pTranslator.isNull()) {
		QString strTemp = pTranslator->translatorApp().translate("CPhraseNavigator", "Category:", "Scope");
		if (!strTemp.isEmpty()) strCategory = strTemp;
	}

	// Search for "Chapter".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strChapter = tr("Chapter", "Scope");
	//TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
	if (!pTranslator.isNull()) {
		QString strTemp = pTranslator->translatorApp().translate("CPhraseNavigator", "Chapter", "Scope");
		if (!strTemp.isEmpty()) strChapter = strTemp;
	}

	CScriptureTextHtmlBuilder scriptureHTML;

	CPhraseNavigator::COPY_FONT_SELECTION_ENUM cfseToUse = CFSE_NONE;
	if (flagsTRO & TRO_SearchResults) cfseToUse = CFSE_SEARCH_RESULTS;
	if (flagsTRO & TRO_Copying) cfseToUse = (CPersistentSettings::instance()->copyFontSelection());
	QString strCopyFont= "font-size:medium;";
	switch (cfseToUse) {
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

	double nLineHeight = 1.0;
	if (flagsTRO & TRO_ScriptureBrowser) {
		nLineHeight = CPersistentSettings::instance()->scriptureBrowserLineHeight();
	}

	VERSE_RENDERING_MODE_ENUM vrmeMode = ((flagsTRO & TRO_Copying) ?
											CPersistentSettings::instance()->verseRenderingModeCopying() :
											CPersistentSettings::instance()->verseRenderingMode());

	if ((flagsTRO & TRO_InnerHTML) == 0) {
//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n.subtitle { font-size:12pt; font-weight:normal; }\n.category { font-size:12pt; font-weight:normal; }\n</style></head><body>\n")
//											.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
//											.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
											"<title>%1</title><style type=\"text/css\">\n"
											"body, p, li, br, .bodyIndent { white-space: pre-line; line-height:%3; %2 }\n"
											".book { font-size:xx-large; font-weight:bold; }\n"
											".chapter { font-size:x-large; font-weight:bold; }\n"
											".verse { display: inline-block; }\n"
											".word { float: left; padding: 0em 0.5em %4em 0em; }\n"
											".ref { float: left; padding: 0em 0.5em %4em 0em; }\n"
											".stack { display: block; }\n"
											".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".category { font-size:medium; font-weight:normal; }\n"
											".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
											"</style></head><body>\n")
											.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
											.arg(strCopyFont)																// Copy Font
											.arg(QString("%1").arg((flagsTRO & TRO_UseLemmas) ? 125.0 : nLineHeight*100, 0, 'f', 0) + "%")		// Line-Height
											.arg(QString("%1").arg((flagsTRO & TRO_UseLemmas) ? (nLineHeight-0.5) : 0.25, 0, 'f', 0)));			// .word/.ref padding bottom in em's
	}

	CRelIndex relPrev = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndx.book(), ndx.chapter(), 1, 1), true);	// Calculate one verse prior to the first verse of this book/chapter
	CRelIndex relNext = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndx.book(), ndx.chapter(), 1, 1), false);	// Calculate first verse of next chapter

	// Print last verse of previous chapter if available:
	if ((!(flagsTRO & TRO_SuppressPrePostChapters)) && (relPrev.isSet())) {
		relPrev.setWord(0);
		const CBookEntry &bookPrev = *m_pBibleDatabase->bookEntry(relPrev.book());
		scriptureHTML.beginParagraph();
		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING)) {
			scriptureHTML.beginIndent(1, -m_TextDocument.indentWidth());
		}
		if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.beginIndent(0, m_TextDocument.indentWidth());
		}

		scriptureHTML.appendRawText("<span class=\"verse\">");

		scriptureHTML.appendRawText("<span class=\"ref\">");
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(relPrev.asAnchor());
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(QString("%1 ").arg(relPrev.verse()));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();
		scriptureHTML.appendRawText("</span>");	// Ref

		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(relPrev,
																	((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																	(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																	pHighlighter, (flagsTRO & TRO_UseLemmas), (flagsTRO & TRO_UseWordSpans)));
		scriptureHTML.appendRawText("</span>");	// Verse

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), relPrev, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
		}
		// And Notes:
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(relPrev, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true);

		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING) ||
			(vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.endIndent();
		}

		scriptureHTML.endParagraph();

		// If we have a footnote or user note for this book and this is the end of the last chapter,
		//		print it too:
		if (relPrev.chapter() == bookPrev.m_nNumChp) {
			if ((flagsTRO & TRO_Colophons) && (bookPrev.m_bHaveColophon)) {
				// Try pseudo-verse (searchable) style first:
				scriptureHTML.beginDiv("colophon");
				scriptureHTML.beginParagraph();
				scriptureHTML.appendRawText("<span class=\"verse\">");
				if (bTotalColophonAnchor) {
					CRelIndex ndxColophon(relPrev.book(), 0, 0, 1);
					scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxColophon.asAnchor()));
				}
				scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(CRelIndex(relPrev.book(), 0, 0, 0),
																			((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																			(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																			pHighlighter, (flagsTRO & TRO_UseLemmas), (flagsTRO & TRO_UseWordSpans)));
				if (bTotalColophonAnchor) {
					scriptureHTML.appendRawText("</a>");
				}
				scriptureHTML.appendRawText("</span>");
				scriptureHTML.endParagraph();
				scriptureHTML.endDiv();
			} else {
				// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
				scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
				if ((flagsTRO & TRO_Colophons) &&
					(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), CRelIndex(relPrev.book(),0,0,0), (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
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
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.beginAnchorID(ndxBook.asAnchor());
	scriptureHTML.appendLiteralText(book.m_strBkName);
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.endAnchor();
	// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
	//		at the end of the book name so people adding notes/cross-refs for the book
	//		aren't confused by it being at the beginning of the name.  But then switch
	//		back to a book reference so that book category/descriptions are properly
	//		labeled:
	if (!(flagsTRO & TRO_NoAnchors)) {
		if (!(flagsTRO & TRO_NoChapterAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChap.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
		if (!(flagsTRO & TRO_NoBookAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBook.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
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
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
		}
		// If we have a User Note for this book, print it too:
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(ndxBook, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
			scriptureHTML.insertHorizontalRule();
	}

	// Print Heading for this Chapter:
	scriptureHTML.beginDiv("chapter");
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.beginAnchorID(ndxBookChap.asAnchor());
	scriptureHTML.appendLiteralText(QString("%1 %2").arg(strChapter).arg(ndx.chapter()));
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.endAnchor();
	scriptureHTML.endDiv();
	// If we have a chapter Footnote for this chapter, print it too:
	if ((flagsTRO & TRO_Superscriptions) && (pChapter->m_bHaveSuperscription)) {
		// Try pseudo-verse (searchable) style first:
		scriptureHTML.beginDiv("superscription");
		scriptureHTML.beginParagraph();
		scriptureHTML.appendRawText("<span class=\"verse\">");
		if (bTotalSuperscriptionAnchor) {
			CRelIndex ndxSuperscription(ndxBookChap);
			ndxSuperscription.setWord(1);
			scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxSuperscription.asAnchor()));
		}
		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxBookChap,
																	((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																	(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																	pHighlighter, (flagsTRO & TRO_UseLemmas), (flagsTRO & TRO_UseWordSpans)));
		if (bTotalSuperscriptionAnchor) {
			scriptureHTML.appendRawText("</a>");
		}
		scriptureHTML.appendRawText("</span>");
		scriptureHTML.endParagraph();
		scriptureHTML.endDiv();
	} else {
		// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
		scriptureHTML.startBuffered();			// Start buffering so we can insert superscription division if there is a footnote
		if ((flagsTRO & TRO_Superscriptions) &&
			(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), ndxBookChap, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
			scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the superscription divison ahead of footnote
			scriptureHTML.beginDiv("superscription");
			scriptureHTML.flushBuffer();
			scriptureHTML.endDiv();
		}
		scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
	}

	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBookChap, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
	}

	// If we have a chapter User Note for this chapter, print it too:
	if ((flagsTRO & TRO_UserNotes) &&
		(scriptureHTML.addNoteFor(ndxBookChap, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
		scriptureHTML.insertHorizontalRule();

	// Print the Chapter Text:
	bool bParagraph = false;
	bool bInIndent = false;
	bool bNeedLeadSpace = false;
	CRelIndex ndxVerse;
	bool bVPLNeedsLineBreak = false;
	bool bStartedText = false;
	for (unsigned int ndxVrs=0; ndxVrs<pChapter->m_nNumVrs; ++ndxVrs) {
		if (((vrmeMode == VRME_VPL) || (vrmeMode == VRME_VPL_DS)) &&
			(bParagraph)) bVPLNeedsLineBreak = true;

		ndxVerse = CRelIndex(ndx.book(), ndx.chapter(), ndxVrs+1, 0);
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndxVerse);
		if (pVerse == nullptr) {
			assert(false);
			continue;
		}
		if ((!bStartedText) && (pVerse->m_nNumWrd == 0)) continue;			// Don't print verses that are empty if we haven't started printing anything for the chapter yet

		if (pVerse->m_nPilcrow != CVerseEntry::PTE_NONE) {
			if (bParagraph) {
				scriptureHTML.endParagraph();
				bParagraph=false;
			}
			bVPLNeedsLineBreak = false;
		}
		if (!bParagraph) {
			scriptureHTML.beginParagraph();
			bParagraph = true;
			bNeedLeadSpace = false;
		}

		if ((!bInIndent) && ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING))) {
			scriptureHTML.beginIndent(1, -m_TextDocument.indentWidth());
			bInIndent = true;
		}
		if ((!bInIndent) && ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT))) {
			scriptureHTML.beginIndent(0, m_TextDocument.indentWidth());
			bInIndent = true;
		}

		if (bVPLNeedsLineBreak) {
			scriptureHTML.addLineBreak();
			if (vrmeMode == VRME_VPL_DS) scriptureHTML.addLineBreak();
			bVPLNeedsLineBreak = false;
		} else if ((pVerse->m_nPilcrow != CVerseEntry::PTE_NONE) && (vrmeMode == VRME_VPL_DS)) {
			scriptureHTML.addLineBreak();
		}
//		if (((vrmeMode == VRME_VPL) || (vrmeMode == VRME_VPL_DS)) && (pVerse->m_nPilcrow != CVerseEntry::PTE_NONE)) {
//			scriptureHTML.addLineBreak();
//		}

		scriptureHTML.appendRawText("<span class=\"verse\">");

		scriptureHTML.appendRawText("<span class=\"ref\">");
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(ndxVerse.asAnchor());
		scriptureHTML.beginBold();
		if ((bNeedLeadSpace) && (vrmeMode == VRME_FF)) scriptureHTML.appendLiteralText(" ");
		scriptureHTML.appendLiteralText(QString("%1 ").arg(ndxVrs+1));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();
		scriptureHTML.appendRawText("</span>");	// Ref

		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxVerse,
																	((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																	(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																	pHighlighter, (flagsTRO & TRO_UseLemmas), (flagsTRO & TRO_UseWordSpans)));
		scriptureHTML.appendRawText("</span>");	// Verse

		bStartedText = true;
		bNeedLeadSpace = true;

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxVerse, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
		}

		// Output notes for this verse, but make use of the buffer in case we need to end the paragraph tag:
		scriptureHTML.startBuffered();
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(ndxVerse, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true))) {
			if (bInIndent) {
				scriptureHTML.endIndent();
				bInIndent = false;
			}
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

			if ((vrmeMode == VRME_VPL_DS) ||
				(vrmeMode == VRME_VPL_DS_HANGING) ||
				(vrmeMode == VRME_VPL_DS_INDENT)) scriptureHTML.addLineBreak();
			bNeedLeadSpace = false;
		}
		scriptureHTML.flushBuffer(true);		// Stop buffering and flush

		if (bInIndent) {
			if ((vrmeMode == VRME_VPL_DS_HANGING) || (vrmeMode == VRME_VPL_DS_INDENT)) scriptureHTML.addLineBreak();
			scriptureHTML.endIndent();
			bInIndent = false;
		}

		ndxVerse.setWord(pVerse->m_nNumWrd);		// At end of loop, ndxVerse will be index of last word we've output...
	}
	if (bInIndent) {
		scriptureHTML.endIndent();
		bInIndent = false;
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
			scriptureHTML.appendRawText("<span class=\"verse\">");
			if (bTotalColophonAnchor) {
				CRelIndex ndxColophon(ndxBook);
				ndxColophon.setWord(1);
				scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxColophon.asAnchor()));
			}
			scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxBook,
																		((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																		(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																		pHighlighter, (flagsTRO & TRO_UseLemmas), (flagsTRO & TRO_UseWordSpans)));
			if (bTotalColophonAnchor) {
				scriptureHTML.appendRawText("</a>");
			}
			scriptureHTML.appendRawText("</span>");
			scriptureHTML.endParagraph();
			scriptureHTML.endDiv();
		} else {
			// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
			scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
			if ((flagsTRO & TRO_Colophons) &&
				(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
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
		assert(pChapterNext != nullptr);

		// Print Heading for this Book:
		if (relNext.book() != ndx.book()) {
			// Print Heading for this Book:
			scriptureHTML.beginDiv("book");
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.beginAnchorID(ndxBookNext.asAnchor());
			scriptureHTML.appendLiteralText(bookNext.m_strBkName);
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.endAnchor();
			// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
			//		at the end of the book name so people adding notes/cross-refs for the book
			//		aren't confused by it being at the beginning of the name.  But then switch
			//		back to a book reference so that book category/descriptions are properly
			//		labeled:
			if (!(flagsTRO & TRO_NoAnchors)) {
				if (!(flagsTRO & TRO_NoChapterAnchors)) {
					scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChapNext.asAnchor()));
					scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
					scriptureHTML.endAnchor();
				}
				if (!(flagsTRO & TRO_NoBookAnchors)) {
					scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookNext.asAnchor()));
					scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
					scriptureHTML.endAnchor();
				}
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
				scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBookNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
			}
			// If we have a User Note for this book, print it too:
			if ((flagsTRO & TRO_UserNotes) &&
				(scriptureHTML.addNoteFor(ndxBookNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
				scriptureHTML.insertHorizontalRule();
		}
		// Print Heading for this Chapter:
		scriptureHTML.beginDiv("chapter");
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.beginAnchorID(ndxBookChapNext.asAnchor());
		scriptureHTML.appendLiteralText(QString("%1 %2").arg(strChapter).arg(relNext.chapter()));
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.endAnchor();
		scriptureHTML.endDiv();

		// If we have a chapter note for this chapter, print it too:
		if ((flagsTRO & TRO_Superscriptions) && (pChapterNext->m_bHaveSuperscription)) {
			// Try pseudo-verse (searchable) style first:
			scriptureHTML.beginDiv("superscription");
			scriptureHTML.beginParagraph();
			scriptureHTML.appendRawText("<span class=\"verse\">");
			if (bTotalSuperscriptionAnchor) {
				CRelIndex ndxSuperscription(ndxBookChapNext);
				ndxSuperscription.setWord(1);
				scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxSuperscription.asAnchor()));
			}
			scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxBookChapNext,
																		((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																		(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																		pHighlighter, (flagsTRO & TRO_UseLemmas), (flagsTRO & TRO_UseWordSpans)));
			if (bTotalSuperscriptionAnchor) {
				scriptureHTML.appendRawText("</a>");
			}
			scriptureHTML.appendRawText("</span>");
			scriptureHTML.endParagraph();
			scriptureHTML.endDiv();
		} else {
			// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
			scriptureHTML.startBuffered();			// Start buffering so we can insert superscription division if there is a footnote
			if ((flagsTRO & TRO_Superscriptions) &&
				(scriptureHTML.addFootnoteFor(m_pBibleDatabase.data(), ndxBookChapNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
				scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the superscription divison ahead of footnote
				scriptureHTML.beginDiv("superscription");
				scriptureHTML.flushBuffer();
				scriptureHTML.endDiv();
			}
			scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
		}

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxBookChapNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
		}

		// If we have a chapter User Note for this chapter, print it too:
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(ndxBookChapNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
			scriptureHTML.insertHorizontalRule();

		scriptureHTML.beginParagraph();
		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING)) {
			scriptureHTML.beginIndent(1, -m_TextDocument.indentWidth());
		}
		if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.beginIndent(0, m_TextDocument.indentWidth());
		}

		scriptureHTML.appendRawText("<span class=\"verse\">");

		scriptureHTML.appendRawText("<span class=\"ref\">");
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(relNext.asAnchor());
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(QString("%1 ").arg(relNext.verse()));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();
		scriptureHTML.appendRawText("</span>");	// Ref

		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(relNext,
																	((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																	(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																	pHighlighter, (flagsTRO & TRO_UseLemmas), (flagsTRO & TRO_UseWordSpans)));
		scriptureHTML.appendRawText("</span>");	// Verse

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), relNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
		}
		// And Notes:
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(relNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true);

		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING) ||
			(vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.endIndent();
		}

		scriptureHTML.endParagraph();
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		scriptureHTML.appendRawText("</body></html>");
	}
	QString strRawHTML = scriptureHTML.getResult();
	if ((flagsTRO & TRO_InnerHTML) == 0) {
		m_TextDocument.setHtml(strRawHTML);
		emit changedDocumentText();
	}
	return strRawHTML;
}

QString CPhraseNavigator::setDocumentToVerse(const CRelIndex &ndx, const TPhraseTagList &tagsToInclude, TextRenderOptionFlags flagsTRO, const CBasicHighlighter *pHighlighter)
{
	assert(!m_pBibleDatabase.isNull());

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		m_TextDocument.clear();
	}

	if (ndx.book() == 0) {
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	if (ndx.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	if (ndx.chapter() > m_pBibleDatabase->bookEntry(ndx.book())->m_nNumChp) {
		assert(false);
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndx);		// Note: Will be null on colophon

	if (((pChapter != nullptr) && (ndx.verse() > pChapter->m_nNumVrs)) ||
		((pChapter == nullptr) && (ndx.verse() != 0))) {
		assert(false);
		if ((flagsTRO & TRO_InnerHTML) == 0) {
			emit changedDocumentText();
		}
		return QString();
	}

	CRelIndex ndxVerse = ndx;
	ndxVerse.setWord(0);			// Create special index to make sure we use a verse only reference

	CRelIndex ndxSuperColo = ndx;
	ndxSuperColo.setWord(1);

	CScriptureTextHtmlBuilder scriptureHTML;

	CPhraseNavigator::COPY_FONT_SELECTION_ENUM cfseToUse = CFSE_NONE;
	if (flagsTRO & TRO_SearchResults) cfseToUse = CFSE_SEARCH_RESULTS;
	if (flagsTRO & TRO_Copying) cfseToUse = (CPersistentSettings::instance()->copyFontSelection());
	QString strCopyFont= "font-size:medium;";
	switch (cfseToUse) {
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

	double nLineHeight = 1.0;
	if (flagsTRO & TRO_ScriptureBrowser) {
		nLineHeight = CPersistentSettings::instance()->scriptureBrowserLineHeight();
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
//							.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n")
//							.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
											"<title>%1</title><style type=\"text/css\">\n"
											"body, p, li, br, .bodyIndent { white-space: pre-wrap; line-height:%3; %2 }\n"
											".book { font-size:xx-large; font-weight:bold; }\n"
											".chapter { font-size:x-large; font-weight:bold; }\n"
											".verse { }\n"
											".word { }\n"
											".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".category { font-size:medium; font-weight:normal; }\n"
											".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
											"</style></head><body>\n")
											.arg(scriptureHTML.escape(m_pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
											.arg(strCopyFont)																// Copy Font
											.arg(QString("%1").arg(nLineHeight*100, 0, 'f', 0) + "%"));						// Line-Height
	}

	if (flagsTRO & TRO_AddDividerLineBefore) scriptureHTML.insertHorizontalRule();

	scriptureHTML.beginParagraph();

	bool bExtended = false;			// True if result extends to multiple verses

	do {
		const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndxVerse);
		if (pVerse == nullptr) {
			assert(false);
			if ((flagsTRO & TRO_InnerHTML) == 0) {
				emit changedDocumentText();
			}
			return QString();
		}

		// Print Book/Chapter for this verse:

		if (bExtended) scriptureHTML.appendRawText(QString("  "));

		if (!bExtended || (ndxVerse.book() != ndx.book())) {
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.beginAnchorID(CRelIndex(ndxVerse.book(), ndxVerse.chapter(), 0, 0).asAnchor());
			scriptureHTML.beginBold();
			if (bExtended) scriptureHTML.appendLiteralText(QString("("));
			scriptureHTML.appendLiteralText(m_pBibleDatabase->bookEntry(ndxVerse.book())->m_strBkName);
			scriptureHTML.endBold();
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.endAnchor();
		}

		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(ndxVerse.asAnchor());
		scriptureHTML.beginBold();
		if (bExtended) {
			if (ndxVerse.book() == ndx.book()) {
				scriptureHTML.appendLiteralText(QString("("));
			} else {
				scriptureHTML.appendLiteralText(QString(" "));
			}
		}

		if (bExtended && (ndxVerse.book() == ndx.book()) && (ndxVerse.chapter() == ndx.chapter())) {
			if (ndxVerse.verse() != 0) {
				scriptureHTML.appendLiteralText(QString("%1").arg(ndxVerse.verse()));
			} else {
				if (ndxVerse.chapter() == 0) {
					scriptureHTML.appendLiteralText(m_pBibleDatabase->translatedColophonString());
				} else {
					scriptureHTML.appendLiteralText(m_pBibleDatabase->translatedSuperscriptionString());
				}
			}
		} else {
			if (!bExtended) scriptureHTML.appendLiteralText(QString(" "));
			if (ndxVerse.verse() != 0) {
				scriptureHTML.appendLiteralText(QString("%1:%2").arg(ndxVerse.chapter()).arg(ndxVerse.verse()));
			} else {
				if (ndxVerse.chapter() == 0) {
					scriptureHTML.appendLiteralText(m_pBibleDatabase->translatedColophonString());
				} else {
					scriptureHTML.appendLiteralText(QString("%1 %2").arg(ndxVerse.chapter()).arg(m_pBibleDatabase->translatedSuperscriptionString()));
				}
			}
		}
		if (bExtended) scriptureHTML.appendLiteralText(QString(")"));
		scriptureHTML.appendLiteralText(QString(" "));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();

		// Print this Verse Text:

		bool bTotalColophonAnchor = (ndxSuperColo.isColophon() && !(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoColophonAnchors));
		bool bTotalSuperscriptionAnchor =  (ndxSuperColo.isSuperscription() && !(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoSuperscriptAnchors));

//
// Note: The problem with applying a special colophon/superscription style with
//		a <div> causes it to be separated as its own paragraph rather than
//		inline like normal verses.  Can we do this with a span or something?
//		More importantly, do we even want to do it?  As we do lose our
//		transChange markup in all of the italics...
//
//		if (ndxVerse.verse() == 0) {
//			if (ndxVerse.chapter() == 0) {
//				scriptureHTML.beginDiv("colophon");
//			} else {
//				scriptureHTML.beginDiv("superscription");
//			}
//		}
		scriptureHTML.appendRawText("<span class=\"verse\">");
		if (bTotalColophonAnchor || bTotalSuperscriptionAnchor) {
			scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxSuperColo.asAnchor()));
		}
		scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndxVerse,
																	((flagsTRO & TRO_Copying) ? m_richifierTagsCopying : m_richifierTagsDisplay),
																	(!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)),
																	pHighlighter));
		if (bTotalColophonAnchor || bTotalSuperscriptionAnchor) {
			scriptureHTML.appendRawText("</a>");
		}
		scriptureHTML.appendRawText("</span>");
//		if (ndxVerse.verse() == 0) {
//			scriptureHTML.endDiv();
//		}

		// Calculate the next verse index so we can see if it intersects our
		//	results of this verse entry (i.e. spills to next verse)
		ndxVerse = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, ndxVerse, false);
		ndxVerse.setWord(0);
		ndxSuperColo = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, ndxSuperColo, false);
		bExtended = true;
	} while (tagsToInclude.intersects(m_pBibleDatabase.data(), TPhraseTag(ndxVerse)));

	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(m_pBibleDatabase.data(), ndxVerse, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
	}

	scriptureHTML.endParagraph();

	if (flagsTRO & TRO_UserNotes)
		scriptureHTML.addNoteFor(ndxVerse, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible));

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		scriptureHTML.appendRawText("</body></html>");
	}
	QString strRawHTML = scriptureHTML.getResult();
	if ((flagsTRO & TRO_InnerHTML) == 0) {
		m_TextDocument.setHtml(strRawHTML);
		emit changedDocumentText();
	}
	return strRawHTML;
}

QString CPhraseNavigator::setDocumentToFormattedVerses(const TPhraseTagList &lstPhraseTags)
{
	return setDocumentToFormattedVerses(TPassageTagList(m_pBibleDatabase.data(), lstPhraseTags));
}

typedef QPair<CRelIndex, CRelIndex> TRelIndexPair;
typedef QList<TRelIndexPair> TRelIndexPairList;

QString CPhraseNavigator::setDocumentToFormattedVerses(const TPassageTagList &lstPassageTags)
{
	assert(!m_pBibleDatabase.isNull());

	m_TextDocument.clear();

	if ((lstPassageTags.isEmpty()) || (lstPassageTags.verseCount() == 0)) {
		emit changedDocumentText();
		return QString();
	}

	QString strPassageReferenceRange;
	TRelIndexPairList lstFirstLastIndexes;

	CRelIndex ndxFirst;
	CRelIndex ndxLast;

	// Build list of overall first/last indexes and establish
	//	our outer-most first and last for the whole list:
	for (int ndx = 0; ndx < lstPassageTags.size(); ++ndx) {
		TPassageTag tagPassage = lstPassageTags.at(ndx);
		if (!tagPassage.isSet()) continue;
		CRelIndex ndxLocalFirst = m_pBibleDatabase->calcRelIndex(tagPassage.relIndex(), CBibleDatabase::RIME_Absolute);
		if (!ndxLocalFirst.isSet()) continue;	// Above absolute calculation can deem the reference invalid
		if (ndxLocalFirst != tagPassage.relIndex()) continue;		// If for some reason the above absolute calculation (normalization) changed our reference (i.e. incomplete text database), toss it as it isn't in this database anyway
		assert(ndxLocalFirst.word() == 1);		// Passages should always begin with the first word of a verse.  Plus this must point to first word so normalize will work correctly
		CRelIndex ndxLocalLast;
		if ((ndxLocalFirst.isColophon()) || (ndxLocalFirst.isSuperscription())) {
			ndxLocalLast = ndxLocalFirst;
		} else {
			ndxLocalLast = m_pBibleDatabase->calcRelIndex(0, tagPassage.verseCount()-1, 0, 0, 0, ndxLocalFirst);		// Add number of verses to find last verse to output
		}
		if (!ndxLocalLast.isSet()) continue;	// Note: If the passage tag we were given is totally outside of the text of the Bible Database, the calculate ndxLocalLast won't be set, so toss this entry
		assert(ndxLocalLast.word() == 1);		// Note: When we calculate next verse, we'll automatically resolve to the first word.  Leave it at 1st word so our loop compare will work

		if ((ndxLocalFirst.isColophon()) && (!CPersistentSettings::instance()->copyColophons())) continue;
		if ((ndxLocalFirst.isSuperscription()) && (!CPersistentSettings::instance()->copySuperscriptions())) continue;

		if (!strPassageReferenceRange.isEmpty()) strPassageReferenceRange += "; ";
			strPassageReferenceRange += tagPassage.PassageReferenceRangeText(m_pBibleDatabase.data());

		lstFirstLastIndexes.append(TRelIndexPair(ndxLocalFirst, ndxLocalLast));

		if (!ndxFirst.isSet()) ndxFirst = ndxLocalFirst;
		ndxLast = ndxLocalLast;
	}
	if (!ndxFirst.isSet() || !ndxLast.isSet()) {
		emit changedDocumentText();
		return QString();		// If passage totally outside Bible Database, we have nothing to render
	}

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

	VERSE_RENDERING_MODE_ENUM vrmeMode = CPersistentSettings::instance()->verseRenderingModeCopying();

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
//								"<html><head><title>%1</title><style type=\"text/css\">\n"
//								"body, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n"
//								".book { font-size:24pt; font-weight:bold; }\n"
//								".chapter { font-size:18pt; font-weight:bold; }\n"
//								".verse { }\n"
//								".word { }\n"
//								"</style></head><body>\n")
//						.arg(scriptureHTML.escape(strPassageReferenceRange));										// Document Title

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
//								"<html><head><title>%1</title><style type=\"text/css\">\n"
//								"body, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n"
//								".book { font-size:xx-large; font-weight:bold; }\n"
//								".chapter { font-size:x-large; font-weight:bold; }\n"
//								".verse { }\n"
//								".word { }\n"
//								"</style></head><body>\n")
//						.arg(scriptureHTML.escape(strPassageReferenceRange));										// Document Title

	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
								"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
								"<title>%1</title><style type=\"text/css\">\n"
								"body, p, li, br, .bodyIndent { white-space: pre-wrap; %2 }\n"
								".book { font-size:xx-large; font-weight:bold; }\n"
								".chapter { font-size:x-large; font-weight:bold; }\n"
								".verse { }\n"
								".word { }\n"
								"</style></head><body>\n")
								.arg(scriptureHTML.escape(strPassageReferenceRange))								// Document Title
								.arg(strCopyFont));																	// Copy Font

	QString strReference;

	QString strBookNameFirst = (CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ? m_pBibleDatabase->bookNameAbbr(ndxFirst) : m_pBibleDatabase->bookName(ndxFirst));
	strReference += referenceStartingDelimiter();
	if ((ndxFirst.book() == ndxLast.book()) && (!ndxFirst.isColophon()) && (!ndxFirst.isSuperscription()) && (!ndxLast.isColophon()) && (!ndxLast.isSuperscription())) {
		if (ndxFirst.chapter() == ndxLast.chapter()) {
			if (ndxFirst.verse() == ndxLast.verse()) {
				strReference += (CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ?
									 m_pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true) :
									 m_pBibleDatabase->PassageReferenceText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true));
			} else {
				if (ndxFirst.chapter() != 0) {
					strReference += QString("%1 %2:%3-%4")
											.arg(strBookNameFirst)
											.arg(ndxFirst.chapter())
											.arg((!ndxFirst.isSuperscription()) ? QString("%1").arg(ndxFirst.verse()) : m_pBibleDatabase->translatedSuperscriptionString())
											.arg((!ndxLast.isSuperscription()) ? QString("%1").arg(ndxLast.verse()) : m_pBibleDatabase->translatedSuperscriptionString());
				} else {
					assert(false);		// Colophons (chapter==0) can't have superscriptions or verses
				}
			}
		} else {
			strReference += QString("%1 %2-%3")
									.arg(strBookNameFirst)
									.arg(m_pBibleDatabase->PassageReferenceText(CRelIndex(0, ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true))
									.arg(m_pBibleDatabase->PassageReferenceText(CRelIndex(0, ndxLast.chapter(), ndxLast.verse(), (((ndxLast.isColophon()) || (ndxLast.isSuperscription())) ? 1 : 0)), true));
		}
	} else {
		strReference += QString("%1-%2")
								.arg((CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ?
										  m_pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true) :
										  m_pBibleDatabase->PassageReferenceText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true)))
								.arg((CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ?
										  m_pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndxLast.book(), ndxLast.chapter(), ndxLast.verse(), (((ndxLast.isColophon()) || (ndxLast.isSuperscription())) ? 1 : 0)), true) :
										  m_pBibleDatabase->PassageReferenceText(CRelIndex(ndxLast.book(), ndxLast.chapter(), ndxLast.verse(), (((ndxLast.isColophon()) || (ndxLast.isSuperscription())) ? 1 : 0)), true)));
	}
	strReference += referenceEndingDelimiter();

	bool bInIndent = false;

	scriptureHTML.beginParagraph();

	if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING)) {
		scriptureHTML.beginIndent(1, -m_TextDocument.indentWidth());
		bInIndent = true;
	}
	if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
		scriptureHTML.beginIndent(0, m_TextDocument.indentWidth());
		bInIndent = true;
	}

	if (!CPersistentSettings::instance()->referencesAtEnd()) {
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strReference);
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.endBold();
		if (CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE) {
			scriptureHTML.addLineBreak();
		} else {
			scriptureHTML.appendLiteralText(" ");
		}
	}

	if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_COMPLETE_REFERENCE) ||
		(vrmeMode == VRME_FF)) {
		scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
	}

	CRelIndex ndxPrev = ndxFirst;
	if (CPersistentSettings::instance()->referencesAtEnd()) {
		// If printing the reference at the end, for printing of the initial verse number:
		ndxPrev.setVerse(-1);
		ndxPrev.setWord(-1);
	}
	for (int nIndexPair = 0; nIndexPair < lstFirstLastIndexes.size(); ++nIndexPair) {
		for (CRelIndex ndx = lstFirstLastIndexes.at(nIndexPair).first; ((ndx <= lstFirstLastIndexes.at(nIndexPair).second) && (ndx.isSet())); /* Increment inside */) {
			if (((vrmeMode == VRME_VPL) ||
				 (vrmeMode == VRME_VPL_DS) ||
				 (vrmeMode == VRME_VPL_DS_HANGING) ||
				 (vrmeMode == VRME_VPL_DS_INDENT)) &&
				(ndx != ndxFirst)) {
				scriptureHTML.addLineBreak();
				if ((vrmeMode == VRME_VPL_DS) ||
					(vrmeMode == VRME_VPL_DS_HANGING) ||
					(vrmeMode == VRME_VPL_DS_INDENT)) scriptureHTML.addLineBreak();
			}

			if ((bInIndent) && (ndx != ndxFirst)) {
				scriptureHTML.endIndent();
				bInIndent = false;
			}

			if ((!bInIndent) && ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING))) {
				scriptureHTML.beginIndent(1, -m_TextDocument.indentWidth());
				bInIndent = true;
			}
			if ((!bInIndent) && ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT))) {
				scriptureHTML.beginIndent(0, m_TextDocument.indentWidth());
				bInIndent = true;
			}

			if ((ndx.book() != ndxPrev.book()) &&
				(CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_COMPLETE_REFERENCE)) {
				if (vrmeMode == VRME_FF) {
					scriptureHTML.appendLiteralText("  ");
				}
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.beginBold();
				scriptureHTML.appendLiteralText(QString("%1%2%3")
												.arg(referenceStartingDelimiter())
												.arg(CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames() ?
																					 m_pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true) :
																					 m_pBibleDatabase->PassageReferenceText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true))
												.arg(referenceEndingDelimiter()));
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.endBold();
				scriptureHTML.appendLiteralText(" ");
			} else if ((ndx.chapter() != ndxPrev.chapter()) || (ndx.verse() != ndxPrev.verse()) ||
					   (CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE)) {
				if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_NO_NUMBER) &&
					(vrmeMode != VRME_VPL) && (vrmeMode != VRME_VPL_DS) &&
					(ndx != ndxFirst)) {
					scriptureHTML.appendLiteralText("  ");
				}
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.beginBold();

				QString strBookChapterVerse = QString("%1%2%3")
												.arg(referenceStartingDelimiter())
												.arg(CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames() ?
																					 m_pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true) :
																					 m_pBibleDatabase->PassageReferenceText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true))
												.arg(referenceEndingDelimiter());
				QString strChapterVerse = m_pBibleDatabase->PassageReferenceText(CRelIndex((((ndx.isColophon()) || (ndx.isSuperscription())) ? ndx.book() : 0), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true);
				QString strVerse = ((!ndx.isSuperscription()) ? QString("%1").arg(ndx.verse()) : m_pBibleDatabase->translatedSuperscriptionString());
				switch (CPersistentSettings::instance()->verseNumberDelimiterMode()) {
					case RDME_NO_NUMBER:
						break;
					case RDME_NO_DELIMITER:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("%1").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("%1").arg(strVerse));
						}
						break;
					case RDME_SQUARE_BRACKETS:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("[%1]").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("[%1]").arg(strVerse));
						}
						break;
					case RDME_CURLY_BRACES:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("{%1}").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("{%1}").arg(strVerse));
						}
						break;
					case RDME_PARENTHESES:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("(%1)").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("(%1)").arg(strVerse));
						}
						break;
					case RDME_SUPERSCRIPT:
						scriptureHTML.beginSuperscript();
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("%1").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("%1").arg(strVerse));
						}
						scriptureHTML.endSuperscript();
						break;
					case RDME_COMPLETE_REFERENCE:
						scriptureHTML.appendLiteralText(QString("%1").arg(strBookChapterVerse));
						break;
					default:
						assert(false);
						break;
				}
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.endBold();

				if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_NO_NUMBER) ||
					(vrmeMode == VRME_FF)) {
					scriptureHTML.appendLiteralText(" ");
				}
			}

			if ((CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE) &&
				(vrmeMode != VRME_FF)) {
				scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
			}

			scriptureHTML.appendRawText("<span class=\"verse\">");
			scriptureHTML.appendRawText(m_pBibleDatabase->richVerseText(ndx, m_richifierTagsCopying, false));
			scriptureHTML.appendRawText("</span>");

			if ((CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE) &&
				(vrmeMode != VRME_FF)) {
				scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
			}

			ndxPrev = ndx;

			if ((!ndx.isColophon()) && (!ndx.isSuperscription())) {
				ndx = m_pBibleDatabase->calcRelIndex(0,1,0,0,0,ndx);
			} else {
				ndx.clear();
			}
		}
	}

	if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_COMPLETE_REFERENCE) ||
		(vrmeMode == VRME_FF)) {
		scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
	}

	if (CPersistentSettings::instance()->referencesAtEnd()) {
		if ((vrmeMode == VRME_VPL) || (vrmeMode == VRME_VPL_DS)) {
			scriptureHTML.addLineBreak();
		} else {
			scriptureHTML.appendLiteralText(" ");
		}
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strReference);
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.endBold();
	}

	if ((bInIndent)) {
		scriptureHTML.endIndent();
		bInIndent = false;
	}

	scriptureHTML.endParagraph();
	scriptureHTML.appendRawText("</body></html>");

	QString strRawHTML = scriptureHTML.getResult();
	m_TextDocument.setHtml(strRawHTML);
	emit changedDocumentText();
	return strRawHTML;
}

void CPhraseNavigator::clearDocument()
{
	m_TextDocument.clear();
	emit changedDocumentText();
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

CSelectionPhraseTagList CPhraseNavigator::getSelection(const CPhraseCursor &aCursor, bool bRecursion) const
{
	assert(!m_pBibleDatabase.isNull());

	TPhraseTag tag;

	CPhraseCursor myCursor(aCursor);
//	myCursor.beginEditBlock();
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
		strAnchorName = myCursor.charFormat().anchorNames().value(0);

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
	int nPosOfIndexLast = nPosLast;
	CRelIndex nIndexLastDetected;
	myCursor.setPosition(nPosLast);
	while ((myCursor.moveCursorCharLeft()) && (myCursor.charUnderCursorIsSeparator())) { }	// Note: Always move left at least one character so we don't pickup the start of the next word (short-circuit order!)
	myCursor.moveCursorWordEnd();
	bool bFoundHit = false;
	uint32_t nNormPrev = 0;
	CRelIndex ndxPrev;
	while (myCursor.position() >= nPosFirstWordStart) {
		strAnchorName = myCursor.charFormat().anchorNames().value(0);
		CRelIndex ndxCurrent = CRelIndex(strAnchorName);

		uint32_t nNormCurrent = m_pBibleDatabase->NormalizeIndex(ndxCurrent);

		if (nNormCurrent != 0) {
			// Make sure words selected are consecutive words (i.e. that we don't have a "hidden" colophon between them or something):
			//		Note: nNormPrev will be equal to nNormCurrent when we are on the first word of the line with how our
			//		paragraphs/blocks work...
			if ((ndxCurrent.word() != 0) && (nNormPrev != (nNormCurrent+1)) && (nNormPrev != nNormCurrent)) {
				bFoundHit = false;
			}

			// If we are moving into or out of a colophon or superscription, treat it as a
			//		discontinuity and break the selection into multiple parts:
			if ((ndxCurrent.word() != 0) && (ndxPrev.isSet()) &&
				(((ndxCurrent.chapter() != 0) && (ndxPrev.chapter() == 0)) ||
				 ((ndxCurrent.chapter() == 0) && (ndxPrev.chapter() != 0)) ||
				 ((ndxCurrent.verse() != 0) && (ndxPrev.verse() == 0)) ||
				 ((ndxCurrent.verse() == 0) && (ndxPrev.verse() != 0)))) {
				bFoundHit = false;
			}

			// If we haven't hit an anchor for an actual word within a verse, we can't be selecting
			//		text from a verse.  We must be in a special tag section of heading:
			if ((ndxCurrent.word() == 0) || (ndxCurrent < nIndexFirst)) {
				ndxCurrent = CRelIndex();
			} else {
				nNormPrev = nNormCurrent;
				ndxPrev = ndxCurrent;
			}

			if (!bFoundHit) {
				nIndexLast = ndxCurrent;
				bFoundHit = nIndexLast.isSet();
				if (bFoundHit) {
					nPosOfIndexLast = myCursor.position();
					nIndexLastDetected = nIndexLast;
				}
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

	if (nIndexLastDetected.isSet()) {
		myCursor.setPosition(nPosOfIndexLast);
		myCursor.moveCursorWordEnd();
		nPosOfIndexLast = myCursor.position()+1;
	} else {
		nPosOfIndexLast = nPosLast+1;
	}

	// Handle single-word selection:
	if (!nIndexLast.isSet()) nIndexLast = nIndexFirst;

	// If the cursor is floating in "no man's land" in a special tag area or footnote text or
	//		something, then find the closest matching tag to the left.  This is the same as
	//		the current position tracking:
	if (!nIndexFirst.isSet()) {
		myCursor.setPosition(nPosFirst);
		while (!nIndexFirst.isSet()) {
			nIndexFirst = CRelIndex(myCursor.charFormat().anchorNames().value(0));
			if (!myCursor.moveCursorCharLeft()) break;
		}
	}

//	myCursor.endEditBlock();

	uint32_t ndxNormFirst = m_pBibleDatabase->NormalizeIndex(nIndexFirst);
	uint32_t ndxNormLast = m_pBibleDatabase->NormalizeIndex(nIndexLast);
	unsigned int nWordCount = 0;

	if ((ndxNormFirst != 0) && (ndxNormLast != 0)) {
		assert(ndxNormLast >= ndxNormFirst);
		nWordCount = (ndxNormLast - ndxNormFirst + 1);
	}

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

	CSelectionPhraseTagList lstSelectTags;
	if ((!bRecursion) || (bRecursion && tag.haveSelection() && (tag.relIndex().word() != 0) && (nIndexLastDetected.isSet()))) lstSelectTags.append(tag);

	if ((nPosOfIndexLast < nPosLast) && (nPosFirst != nPosLast) && (nIndexLastDetected.isSet())) {
		myCursor.setPosition(nPosOfIndexLast);
		myCursor.setPosition(nPosLast, QTextCursor::KeepAnchor);
		lstSelectTags.append(getSelection(myCursor, true));
	}

	return lstSelectTags;
}

CSelectedPhraseList CPhraseNavigator::getSelectedPhrases(const CPhraseCursor &aCursor) const
{
	assert(!m_pBibleDatabase.isNull());

	CSelectedPhraseList lstSelectedPhrases;

	CSelectionPhraseTagList lstSelections = getSelection(aCursor);
	if (lstSelections.size() == 0) lstSelections.append(TPhraseTag());

#ifdef DEBUG_SELECTED_PHRASE
	qDebug("%d Phrases Selected", lstSelections.size());
#endif
	for (int ndxSel = 0; ndxSel < lstSelections.size(); ++ndxSel) {
		QString strPhrase;
		TPhraseTag tagSel = lstSelections.at(ndxSel);

		int nCount = tagSel.count();
		CRelIndex ndxRel = tagSel.relIndex();
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
		static const CVerseTextPlainRichifierTags tagsRichifier;
		while ((nCount > 0) && (ndxRel.isSet())) {
			const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndxRel);
			assert(pVerse != nullptr);
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

		lstSelectedPhrases.append(CSelectedPhrase(m_pBibleDatabase, tagSel, strPhrase));

#ifdef DEBUG_SELECTED_PHRASE
		qDebug("    \"%s\"", strPhrase.toUtf8().data());
		qDebug("    %s %d", m_pBibleDatabase->PassageReferenceText(tagSel.relIndex()).toUtf8().data(), tagSel.count());
#endif
	}

	return lstSelectedPhrases;
}

void CPhraseNavigator::removeAnchors()
{
	assert(!m_pBibleDatabase.isNull());

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

#ifdef QT_WIDGETS_LIB

void CPhraseEditNavigator::selectWords(const TPhraseTag &tag)
{
	assert(!m_pBibleDatabase.isNull());

	CRelIndex ndxScroll = tag.relIndex();
	if (!ndxScroll.isColophon()) {
		if (m_pBibleDatabase->NormalizeIndex(CRelIndex(ndxScroll.book(), ndxScroll.chapter(), 1, 1)) == m_pBibleDatabase->NormalizeIndex(ndxScroll)) {
			ndxScroll.setVerse(0);		// Use 0 anchor if we are going to the first word of the chapter so we'll scroll to top of heading
		}
		ndxScroll.setWord(0);
	} else {
		// For colophons, goto word 1 and skip the normalization check (as it will be incorrect):
		ndxScroll.setWord(1);
	}

	m_TextEditor.scrollToAnchor(ndxScroll.asAnchor());

	CRelIndex ndxRel = tag.relIndex();
	if (ndxRel.isSet()) {
		int nStartPos = anchorPosition(ndxRel.asAnchor());
		int nEndPos = anchorPosition(m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(ndxRel) + tag.count() - 1).asAnchor());

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

CSelectionPhraseTagList CPhraseEditNavigator::getSelection() const
{
	return getSelection(m_TextEditor.textCursor());
}

CSelectedPhraseList CPhraseEditNavigator::getSelectedPhrases() const
{
	return getSelectedPhrases(m_TextEditor.textCursor());
}

#if !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)

bool CPhraseEditNavigator::handleToolTipEvent(CKJVCanOpener *pCanOpener, const QHelpEvent *pHelpEvent, CCursorFollowHighlighter &aHighlighter, const CSelectionPhraseTagList &selection) const
{
	assert(!m_pBibleDatabase.isNull());

	assert(pHelpEvent != nullptr);
	CSelectionPhraseTagList lstRefSelection = getSelection(m_TextEditor.cursorForPosition(pHelpEvent->pos()));
	TPhraseTag tagReference = TPhraseTag(lstRefSelection.primarySelection().relIndex(), 1);
	QString strToolTip = getToolTip(tagReference, selection);

	if (!strToolTip.isEmpty()) {
		highlightCursorFollowTag(aHighlighter, (selection.haveSelection() ? static_cast<TPhraseTagList>(selection) : TPhraseTagList(tagReference)));
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

bool CPhraseEditNavigator::handleToolTipEvent(CKJVCanOpener *pCanOpener, CCursorFollowHighlighter &aHighlighter, const TPhraseTag &tag, const CSelectionPhraseTagList &selection) const
{
	assert(!m_pBibleDatabase.isNull());

	QString strToolTip = getToolTip(tag, selection);

	if (!strToolTip.isEmpty()) {
		highlightCursorFollowTag(aHighlighter, (selection.haveSelection() ? static_cast<TPhraseTagList>(selection) : TPhraseTagList(TPhraseTag(tag.relIndex(), 1))));
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

#endif	// !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)

void CPhraseEditNavigator::highlightCursorFollowTag(CCursorFollowHighlighter &aHighlighter, const TPhraseTagList &tagList) const
{
	assert(!m_pBibleDatabase.isNull());

	doHighlighting(aHighlighter, true);
	TPhraseTagList tagsToHighlight;
	for (int ndx = 0; ndx < tagList.size(); ++ndx) {
		TPhraseTag tag = tagList.at(ndx);
		// Highlight the word only if we have a reference for an actual word (not just a chapter or book or something):
		if ((tag.relIndex().book() != 0) &&
			(tag.relIndex().word() != 0) &&
			(tag.count() != 0)) {
			tagsToHighlight.append(tag);
		}
	}
	if (!tagsToHighlight.isEmpty()) {
		aHighlighter.setPhraseTags(tagsToHighlight);
		doHighlighting(aHighlighter);
	} else {
		aHighlighter.clearPhraseTags();
	}
}

QString CPhraseNavigator::getToolTip(const CBibleDatabasePtr &pBibleDatabase, const TPhraseTag &tag, const CSelectionPhraseTagList &selection, TOOLTIP_TYPE_ENUM nToolTipType, bool bPlainText)
{
	assert(!pBibleDatabase.isNull());

	QString strToolTip;

	for (int ndxSel = 0; ((ndxSel < selection.size()) || (ndxSel == 0)); ++ndxSel) {
		bool bUseTag = !selection.haveSelection();

		bool bHaveSelection = (bUseTag ? false : selection.at(ndxSel).haveSelection());
		const CRelIndex &ndxReference(bUseTag ? tag.relIndex() : selection.at(ndxSel).relIndex());
		unsigned int nCount = (bUseTag ? tag.count() : selection.at(ndxSel).count());

		if (ndxReference.isSet()) {
			if (!strToolTip.isEmpty()) {
				if (!bPlainText) {
					strToolTip += "</pre><hr /><hr /><pre>";
				} else {
					strToolTip += "\n------------------------------------------------------------\n";
				}
			}
			if ((strToolTip.isEmpty()) && (!bPlainText)) strToolTip += "<html><body><pre>";
			if ((nToolTipType == TTE_COMPLETE) ||
				(nToolTipType == TTE_REFERENCE_ONLY)) {
				if (!bHaveSelection) {
					if (ndxReference.word() != 0) {
						uint32_t ndxNormal = pBibleDatabase->NormalizeIndex(ndxReference);
						if ((ndxNormal != 0) && (ndxNormal <= pBibleDatabase->bibleEntry().m_nNumWrd)) {
							strToolTip += tr("Word:", "Statistics") + " \"" + pBibleDatabase->wordAtIndex(ndxNormal) + "\"\n";
						}
					}
					strToolTip += pBibleDatabase->SearchResultToolTip(ndxReference);
				} else {
					strToolTip += tr("Phrase:", "Statistics") + " \"";
					uint32_t ndxNormal = pBibleDatabase->NormalizeIndex(ndxReference);
					if (ndxNormal != 0) {
						unsigned int ndx;
						for (ndx = 0; ((ndx < qMin(7u, nCount)) && ((ndxNormal + ndx) <= pBibleDatabase->bibleEntry().m_nNumWrd)); ++ndx) {
							if (ndx) strToolTip += " ";
							strToolTip += pBibleDatabase->wordAtIndex(ndxNormal + ndx);
						}
						if ((ndx == 7u) && (nCount > 7u)) strToolTip += " ...";
					} else {
						assert(false);
						strToolTip += "???";
					}
					strToolTip += "\"\n";
					strToolTip += pBibleDatabase->SearchResultToolTip(ndxReference, RIMASK_ALL, nCount);
				}
			}
			if ((nToolTipType == TTE_COMPLETE) ||
				(nToolTipType == TTE_STATISTICS_ONLY)) {
				if (ndxReference.book() != 0) {
					assert(ndxReference.book() <= pBibleDatabase->bibleEntry().m_nNumBk);
					if (ndxReference.book() <= pBibleDatabase->bibleEntry().m_nNumBk) {
						if (nToolTipType == TTE_COMPLETE) {
							if (!bPlainText) {
								strToolTip += "</pre><hr /><pre>";
							} else {
								strToolTip += "--------------------\n";
							}
						}
						strToolTip += QString("\n%1 ").arg(pBibleDatabase->bookName(ndxReference)) + tr("contains:", "Statistics") + "\n"
												"    " + tr("%n Chapter(s)", "Statistics", pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) + "\n"
												"    " + tr("%n Verse(s)", "Statistics", pBibleDatabase->bookEntry(ndxReference.book())->m_nNumVrs) + "\n"
												"    " + tr("%n Word(s)", "Statistics", pBibleDatabase->bookEntry(ndxReference.book())->m_nNumWrd) + "\n";
						if (ndxReference.chapter() != 0) {
							assert(ndxReference.chapter() <= pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp);
							if (ndxReference.chapter() <= pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) {
								strToolTip += QString("\n%1 %2 ").arg(pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()) + tr("contains:", "Statistics") + "\n"
												"    " + tr("%n Verse(s)", "Statistics", pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) + "\n"
												"    " + tr("%n Word(s)", "Statistics", pBibleDatabase->chapterEntry(ndxReference)->m_nNumWrd) + "\n";
								if ((!bHaveSelection) && (ndxReference.verse() != 0)) {
									assert(ndxReference.verse() <= pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs);
									if (ndxReference.verse() <= pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) {
										strToolTip += QString("\n%1 %2:%3 ").arg(pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()).arg(ndxReference.verse()) + tr("contains:", "Statistics") + "\n"
												"    " + tr("%n Word(s)", "Statistics", pBibleDatabase->verseEntry(ndxReference)->m_nNumWrd) + "\n";
									}
								}
							}
						}
					}
				}
				if (bHaveSelection) {
					strToolTip += "\n" + tr("%n Word(s) Selected", "Statistics", nCount) + "\n";
				}
			}
		}
	}
	if ((!strToolTip.isEmpty()) && (!bPlainText)) strToolTip += "</pre></body></html>";

	return strToolTip;
}

#endif

// ============================================================================

