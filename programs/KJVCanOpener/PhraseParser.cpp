/****************************************************************************
**
** Copyright (C) 2012-2023 Donna Whisnant, a.k.a. Dewtronics.
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

#include "PhraseParser.h"
#include "PhraseCursor.h"
#include "ParseSymbols.h"
#include "VerseRichifier.h"

#include <QTextCursor>
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

			//			strRawPhrase += StringParse::deLigature(chrNext);
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
	return static_cast<unsigned int>(m_lstMatchMapping.size());
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
	Q_ASSERT(!m_lstSubPhrases.isEmpty());

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

	Q_ASSERT((m_nActiveSubPhrase >=0) && (m_nActiveSubPhrase < m_lstSubPhrases.size()));
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
	Q_ASSERT(!m_pBibleDatabase.isNull());

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
		int nSize = static_cast<int>(m_lstSubPhrases.at(ndxSubPhrase)->m_lstMatchMapping.size());
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
		lstSubPhraseTags.reserve(static_cast<decltype(lstSubPhraseTags)::size_type>(subPhrase->m_lstMatchMapping.size()));
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
	Q_ASSERT(!m_pBibleDatabase.isNull());

	const TPhraseTagList &lstTags = GetPhraseTagSearchResults();
	QList<QPair<QString, int> > lstMatchingPhrasesSort;
	lstMatchingPhrasesSort.reserve(lstTags.size());

	for (int ndx = 0; ndx < lstTags.size(); ++ndx) {
		if (!lstTags.at(ndx).isSet()) continue;
		uint32_t ndxNormal = m_pBibleDatabase->NormalizeIndex(lstTags.at(ndx).relIndex());
		QStringList lstPhraseWordsDecomposed;
		lstPhraseWordsDecomposed.reserve(lstTags.at(ndx).count());
		for (unsigned int nWrd = 0; nWrd < lstTags.at(ndx).count(); ++nWrd) {
			lstPhraseWordsDecomposed.append(m_pBibleDatabase->wordAtIndex(ndxNormal, WTE_DECOMPOSED));
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
			lstPhraseWords.append(m_pBibleDatabase->wordAtIndex(ndxNormal, WTE_SEARCH));
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
	Q_ASSERT(m_nActiveSubPhrase < m_lstSubPhrases.size());

	return m_lstSubPhrases.at(m_nActiveSubPhrase)->GetCursorWord();
}

int CParsedPhrase::GetCursorWordPos() const
{
	if (m_nActiveSubPhrase < 0) return -1;
	Q_ASSERT(m_nActiveSubPhrase < m_lstSubPhrases.size());

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

void CParsedPhrase::UpdateCompleter(const QTextCursor &curInsert)
{
	ParsePhrase(curInsert);
	FindWords();
}

QTextCursor CParsedPhrase::insertCompletion(const QTextCursor &curInsert, const QString& completion)
{
	CPhraseCursor myCursor(curInsert, m_pBibleDatabase.data(), false);		// Hyphens aren't considered as word separators for phrases
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

	CPhraseCursor myCursor(curInsert, m_pBibleDatabase.data(), false);		// Hyphens aren't considered as word separators for phrases
	int nCursorPos = myCursor.position();

	myCursor.setPosition(0);
	myCursor.selectCursorToLineEnd();
	QString strComplete = myCursor.selectedText();

	Q_ASSERT(nCursorPos <= strComplete.size());

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
	Q_ASSERT(!m_lstSubPhrases.isEmpty());

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

	Q_ASSERT(lstCompleteWords.size() == (lstLeftWords.size() + lstRightWords.size()));

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
		Q_ASSERT(nCursorWord == 0);		// If we aren't at a non-existent word at the end of an empty phrase, then something went wrong above
		m_nActiveSubPhrase = m_lstSubPhrases.size()-1;
		if (nCursorWord < m_lstSubPhrases[m_nActiveSubPhrase]->m_lstWords.size()) {
			// I don't think this case can ever happen, as we should have set the word in the loop above:
			Q_ASSERT(false);
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
	Q_ASSERT(lstPhrases.size() >= 1);

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
	Q_ASSERT(!m_lstSubPhrases.isEmpty());

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

	Q_ASSERT(!m_lstSubPhrases.isEmpty());			// Should have inserted a phrase above
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

	Q_ASSERT(!m_pBibleDatabase.isNull());

	int nCursorWord = subPhrase.m_nCursorWord;
	Q_ASSERT((nCursorWord >= 0) && (nCursorWord <= subPhrase.m_lstWords.size()));

	bool bComputedNextWords = false;
	if (!bResume) {
		subPhrase.m_lstMatchMapping.clear();
		subPhrase.m_nLevel = 0;
		subPhrase.m_nCursorLevel = 0;
	}
	bool bInFirstWordStar = false;
	for (int ndx=subPhrase.m_nLevel; ndx<subPhrase.m_lstWords.size(); ++ndx) {
		if (subPhrase.m_lstWords.at(ndx).isEmpty()) continue;

		QString strCurWordDecomp = StringParse::decompose(subPhrase.m_lstWords.at(ndx), true);
		QString strCurWord = (isAccentSensitive() ? StringParse::deApostrHyphen(subPhrase.m_lstWords.at(ndx), !m_pBibleDatabase->settings().hyphenSensitive()) :
								  StringParse::decompose(subPhrase.m_lstWords.at(ndx), !m_pBibleDatabase->settings().hyphenSensitive()));

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
			int nFirstWord = expCurWordWildKey.isValid() ? m_pBibleDatabase->lstWordList().indexOf(expCurWordWildKey) : -1;
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
				int nLastWord = expCurWordWildKey.isValid() ? m_pBibleDatabase->lstWordList().lastIndexOf(expCurWordWildKey) : -1;
				Q_ASSERT(nLastWord != -1);			// Should have at least one match since forward search matched above!

				for (int ndxWord = nFirstWord; ndxWord <= nLastWord; ++ndxWord) {
#if QT_VERSION >= 0x050F00
					if (!expCurWordExactKey.isValid() || !expCurWordExactKey.match(m_pBibleDatabase->lstWordList().at(ndxWord)).hasMatch()) continue;
#else
					if (!expCurWordExactKey.isValid() || !expCurWordExactKey.exactMatch(m_pBibleDatabase->lstWordList().at(ndxWord))) continue;
#endif
					TWordListMap::const_iterator itrWordMap = m_pBibleDatabase->mapWordList().find(m_pBibleDatabase->lstWordList().at(ndxWord));
					Q_ASSERT(itrWordMap != m_pBibleDatabase->mapWordList().end());
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
							if (expCurWord.isValid() && expCurWord.match(strAltWord).hasMatch()) {
#else
							if (expCurWord.isValid() && expCurWord.exactMatch(strAltWord)) {
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
					Q_ASSERT(pNextWordEntry != nullptr);
					const QString &strNextWord = ((!isAccentSensitive()) ?
													  ((!m_pBibleDatabase->settings().hyphenSensitive()) ? pNextWordEntry->decomposedWord() : pNextWordEntry->decomposedHyphenWord()) :
													  ((!m_pBibleDatabase->settings().hyphenSensitive()) ? pNextWordEntry->deApostrWord() : pNextWordEntry->deApostrHyphenWord()));
#if QT_VERSION >= 0x050F00
					if (expCurWord.isValid() && expCurWord.match(strNextWord).hasMatch()) {
#else
					if (expCurWord.isValid() && expCurWord.exactMatch(strNextWord)) {
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
					setNextWords.reserve(static_cast<decltype(setNextWords)::size_type>(subPhrase.m_lstMatchMapping.size()));
					for (unsigned int ndxWord=0; ndxWord<subPhrase.m_lstMatchMapping.size(); ++ndxWord) {
						if ((subPhrase.m_lstMatchMapping.at(ndxWord)+1) <= m_pBibleDatabase->bibleEntry().m_nNumWrd) {
							int nConcordanceIndex = m_pBibleDatabase->concordanceIndexForWordAtIndex(subPhrase.m_lstMatchMapping.at(ndxWord)+1);
							Q_ASSERT(nConcordanceIndex != -1);
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

