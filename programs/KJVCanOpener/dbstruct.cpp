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

// dbstruct.cpp -- Defines structures used for database info
//

#include "dbstruct.h"
#include "VerseRichifier.h"
#include "SearchCompleter.h"
#include "PhraseEdit.h"
#include "ScriptureDocument.h"

#include <QtAlgorithms>
#include <QSet>
#include <QObject>
#include <iterator>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>

#include <assert.h>

// ============================================================================

// Global Variables:

// Our Bible Databases:
CBibleDatabasePtr g_pMainBibleDatabase;		// Main Database (database currently active for main navigation)
TBibleDatabaseList g_lstBibleDatabases;

// User-defined phrases read from optional user database:
CPhraseList g_lstUserPhrases;
bool g_bUserPhrasesDirty = false;				// True if user has edited the phrase list

// ============================================================================

CPhraseEntry::CPhraseEntry(const QString &strEncodedText, const QVariant &varExtraInfo)
	:	m_bCaseSensitive(false),
		m_bAccentSensitive(false),
		m_bDisabled(false),
		m_nNumWrd(0),
		m_varExtraInfo(varExtraInfo)
{
	setTextEncoded(strEncodedText);
}

CPhraseEntry::~CPhraseEntry()
{

}

void CPhraseEntry::clear()
{
	m_strPhrase.clear();
	m_nNumWrd = 0;
	m_bCaseSensitive = false;
	m_bAccentSensitive = false;
	m_bDisabled = false;
	m_varExtraInfo.clear();
}

void CPhraseEntry::setFromPhrase(const CParsedPhrase *pPhrase)
{
	assert(pPhrase != NULL);
	if (pPhrase == NULL) return;

	clear();
	m_strPhrase = pPhrase->phrase();
	m_bCaseSensitive = pPhrase->isCaseSensitive();
	m_bAccentSensitive = pPhrase->isAccentSensitive();
	m_bDisabled = pPhrase->isDisabled();
}

QString CPhraseEntry::textEncoded() const
{
	QString strText;

	// The order here matters as we will always read/write the special flags in order
	//		so we don't need a complete parser to allow any arbitrary order:
	if (isDisabled()) strText += encCharDisabled();
	if (accentSensitive()) strText += encCharAccentSensitive();
	if (caseSensitive()) strText += encCharCaseSensitive();
	strText += m_strPhrase;

	return strText;
}

void CPhraseEntry::setText(const QString &strText)
{
	CParsedPhrase parsedPhrase(CBibleDatabasePtr(), caseSensitive(), accentSensitive());			// Note: the ParsePhrase() function doesn't need the datbase.  If that ever changes, this must change (TODO)
	parsedPhrase.ParsePhrase(strText);
	m_nNumWrd = parsedPhrase.phraseSize();
	m_strPhrase = strText;
}

void CPhraseEntry::setTextEncoded(const QString &strText)
{
	QString strTextToSet = strText;

	// The order here matters as we will always read/write the special flags in order
	//		so we don't need a complete parser to allow any arbitrary order:

	if (strTextToSet.startsWith(encCharDisabled())) {
		strTextToSet = strTextToSet.mid(1);				// Remove the special disable flag
		setDisabled(true);
	} else {
		setDisabled(false);
	}

	if (strTextToSet.startsWith(encCharAccentSensitive())) {
		strTextToSet = strTextToSet.mid(1);
		setAccentSensitive(true);
	} else {
		setAccentSensitive(false);
	}

	if (strTextToSet.startsWith(encCharCaseSensitive())) {
		strTextToSet = strTextToSet.mid(1);				// Remove the special case-sensitive flag
		setCaseSensitive(true);
	} else {
		setCaseSensitive(false);
	}

	setText(strTextToSet);
}

// ============================================================================

int CPhraseList::removeDuplicates()
{
	int n = size();
	int j = 0;
	QSet<CPhraseEntry> seen;
	seen.reserve(n);
	for (int i = 0; i < n; ++i) {
		const CPhraseEntry &s = at(i);
		if (seen.contains(s))
			continue;
		seen.insert(s);
		if (j != i)
			(*this)[j] = s;
		++j;
	}
	if (n != j)
		erase(begin() + j, end());
	return n - j;
}

// ============================================================================

QString CFootnoteEntry::htmlText(const CBibleDatabase *pBibleDatabase) const
{
	QTextDocument docFootote;
	docFootote.setHtml(text());

	CScriptureTextHtmlBuilder scriptureHTML;
	CScriptureTextDocumentDirector scriptureDirector(&scriptureHTML, pBibleDatabase);		// Only need BibleDatabase for embedded scripture and cross-refs, etc

	scriptureDirector.processDocument(&docFootote);

	return scriptureHTML.getResult();
}

QString CFootnoteEntry::plainText(const CBibleDatabase *pBibleDatabase) const
{
	QTextDocument docFootote;
	docFootote.setHtml(text());

	CScripturePlainTextBuilder scripturePlainText;
	CScriptureTextDocumentDirector scriptureDirector(&scripturePlainText, pBibleDatabase);		// Only need BibleDatabase for embedded scripture and cross-refs, etc

	scriptureDirector.processDocument(&docFootote);

	return scripturePlainText.getResult();
}

// ============================================================================

#ifdef OSIS_PARSER_BUILD

uint32_t CBibleDatabase::NormalizeIndexNoAccum(uint32_t nRelIndex) const
{
	uint32_t nNormalIndex = 0;
	unsigned int nBk = ((nRelIndex >> 24) & 0xFF);
	unsigned int nChp = ((nRelIndex >> 16) & 0xFF);
	unsigned int nVrs = ((nRelIndex >> 8) & 0xFF);
	unsigned int nWrd = (nRelIndex & 0xFF);

	if (nRelIndex == 0) return 0;

	// Add the number of words for all books prior to the target book:
	if (nBk < 1) return 0;
	if (nBk > m_lstBooks.size()) return 0;
	for (unsigned int ndxBk = 1; ndxBk < nBk; ++ndxBk) {
		nNormalIndex += m_lstBooks[ndxBk-1].m_nNumWrd;
	}
	// Add the number of words for all chapters in this book prior to the target chapter:
	if (nChp > m_lstBooks[nBk-1].m_nNumChp) return 0;
	for (unsigned int ndxChp = 1; ndxChp < nChp; ++ndxChp) {
		nNormalIndex += m_mapChapters.at(CRelIndex(nBk,ndxChp,0,0)).m_nNumWrd;
	}
	// Add the number of words for all verses in this book prior to the target verse:
	if (nVrs > m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) return 0;
	for (unsigned int ndxVrs = 1; ndxVrs < nVrs; ++ndxVrs) {
		nNormalIndex += (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,ndxVrs,0)).m_nNumWrd;
	}
	// Add the target word:
	if (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nNumWrd) return 0;
	nNormalIndex += nWrd;

	return nNormalIndex;
}

uint32_t CBibleDatabase::DenormalizeIndexNoAccum(uint32_t nNormalIndex) const
{
	unsigned int nBk = 0;
	unsigned int nChp = 1;
	unsigned int nVrs = 1;
	unsigned int nWrd = nNormalIndex;

	if (nNormalIndex == 0) return 0;

	while (nBk < m_lstBooks.size()) {
		if (m_lstBooks[nBk].m_nNumWrd >= nWrd) break;
		nWrd -= m_lstBooks[nBk].m_nNumWrd;
		nBk++;
	}
	if (nBk >= m_lstBooks.size()) return 0;
	nBk++;

	while (nChp <= m_lstBooks.at(nBk-1).m_nNumChp) {
		if (m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumWrd >= nWrd) break;
		nWrd -= m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumWrd;
		nChp++;
	}
	if (nChp > m_lstBooks[nBk-1].m_nNumChp) return 0;

	while (nVrs <= m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) {
		if ((m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nNumWrd >= nWrd) break;
		nWrd -= (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nNumWrd;
		nVrs++;
	}
	if (nVrs > m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) return 0;

	if (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nNumWrd) return 0;

	return CRelIndex(nBk, nChp, nVrs, nWrd).index();
}

#endif

uint32_t CBibleDatabase::NormalizeIndex(uint32_t nRelIndex) const
{
	unsigned int nBk = ((nRelIndex >> 24) & 0xFF);
	unsigned int nChp = ((nRelIndex >> 16) & 0xFF);
	unsigned int nVrs = ((nRelIndex >> 8) & 0xFF);
	unsigned int nWrd = (nRelIndex & 0xFF);

	if (nRelIndex == 0) return 0;

	if ((nBk < 1) ||
		(nChp < 1) ||
		(nVrs < 1) ||
		(nWrd < 1)) return 0;
	if (nBk > m_lstBooks.size()) return 0;
	if (nChp > m_lstBooks[nBk-1].m_nNumChp) return 0;
	if (nVrs > m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) return 0;
	if (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nNumWrd) return 0;

	return ((m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nWrdAccum + nWrd);
}

uint32_t CBibleDatabase::DenormalizeIndex(uint32_t nNormalIndex) const
{
	unsigned int nWrd = nNormalIndex;

	if (nWrd == 0) return 0;

	unsigned int nBk = m_lstBooks.size();
	while ((nBk > 0) && (nWrd <= m_lstBooks.at(nBk-1).m_nWrdAccum)) {
		nBk--;
	}
	if (nBk == 0) {
		assert(false);
		return 0;
	}

	unsigned int nChp = m_lstBooks.at(nBk-1).m_nNumChp;
	while ((nChp > 0) && (nWrd <= m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nWrdAccum)) {
		nChp--;
	}
	if (nChp == 0) {
		assert(false);
		return 0;
	}

	unsigned int nVrs = m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs;
	while ((nVrs > 0) && (nWrd <= (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nWrdAccum)) {
		nVrs--;
	}
	if (nVrs == 0) {
		assert(false);
		return 0;
	}

	nWrd -= (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nWrdAccum;
	if (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(0,nChp,nVrs,0)).m_nNumWrd) {
		// We can get here if the caller is addressing one word beyond the end-of-the-text, for example,
		//		and this has always been defined as "0" (out-of-bounds or not-set), just like the "0"
		//		at the beginning of the text.
		return 0;
	}

	return CRelIndex(nBk, nChp, nVrs, nWrd).index();
}

// ============================================================================

CConcordanceEntry::CConcordanceEntry(const QString &strWord, int nIndex)
	:	m_strWord(strWord),
		m_strDecomposedWord(CSearchStringListModel::decompose(strWord)),
		m_nIndex(nIndex)
{

}

// ============================================================================

QString CBibleDatabase::testamentName(const CRelIndex &nRelIndex) const
{
	uint32_t nTst = testament(nRelIndex);
	if ((nTst < 1) || (nTst > m_lstTestaments.size())) return QString();
	return m_lstTestaments.at(nTst-1).m_strTstName;
}

uint32_t CBibleDatabase::testament(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return 0;
	return m_lstBooks.at(nBk-1).m_nTstNdx;
}

QString CBibleDatabase::bookCategoryName(const CRelIndex &nRelIndex) const
{
	uint32_t nCat = bookCategory(nRelIndex);
	if ((nCat < 1) || (nCat > m_lstBookCategories.size())) return QString();
	return m_lstBookCategories.at(nCat-1).m_strCategoryName;
}

uint32_t CBibleDatabase::bookCategory(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return 0;
	return m_lstBooks.at(nBk-1).m_nCatNdx;
}

QString CBibleDatabase::bookName(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return QString();
	const CBookEntry &book = m_lstBooks[nBk-1];
	return book.m_strBkName;
}

QString CBibleDatabase::bookNameAbbr(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return QString();
	const CBookEntry &book = m_lstBooks[nBk-1];
	return book.m_strBkAbbr;
}

QString CBibleDatabase::SearchResultToolTip(const CRelIndex &nRelIndex, unsigned int nRIMask, unsigned int nSelectionSize) const
{
	CRefCountCalc Bk(this, CRefCountCalc::RTE_BOOK, nRelIndex);
	CRefCountCalc Chp(this, CRefCountCalc::RTE_CHAPTER, nRelIndex);
	CRefCountCalc Vrs(this, CRefCountCalc::RTE_VERSE, nRelIndex);
	CRefCountCalc Wrd(this, CRefCountCalc::RTE_WORD, nRelIndex);

	QString strTemp;

	if (nRIMask & RIMASK_HEADING) {
		if (nSelectionSize > 1) {
			strTemp += PassageReferenceText(nRelIndex);
			strTemp += " - ";
			strTemp += PassageReferenceText(CRelIndex(DenormalizeIndex(NormalizeIndex(nRelIndex) + nSelectionSize - 1)));
			strTemp += " " + QObject::tr("(%1 Words)").arg(nSelectionSize);
			strTemp += "\n\n";
		} else {
			strTemp += PassageReferenceText(nRelIndex);
			strTemp += "\n\n";
		}
	}

	if ((nRIMask & RIMASK_BOOK) &&
		((Bk.ofBible().first != 0) ||
		 (Bk.ofTestament().first != 0))) {
		strTemp += QObject::tr("Book:") + "\n";
		if (Bk.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible").arg(Bk.ofBible().first).arg(Bk.ofBible().second) + "\n";
		}
		if (Bk.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3").arg(Bk.ofTestament().first).arg(Bk.ofTestament().second).arg(testamentName(nRelIndex)) + "\n";
		}
	}

	if ((nRIMask & RIMASK_CHAPTER) &&
		(nRelIndex.chapter() != 0) &&
		((Chp.ofBible().first != 0) ||
		 (Chp.ofTestament().first != 0) ||
		 (Chp.ofBook().first != 0))) {
		strTemp += QObject::tr("Chapter:") + "\n";
		if (Chp.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible").arg(Chp.ofBible().first).arg(Chp.ofBible().second) + "\n";
		}
		if (Chp.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3").arg(Chp.ofTestament().first).arg(Chp.ofTestament().second).arg(testamentName(nRelIndex)) + "\n";
		}
		if (Chp.ofBook().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3").arg(Chp.ofBook().first).arg(Chp.ofBook().second).arg(bookName(nRelIndex)) + "\n";
		}
	}

	if ((nRIMask & RIMASK_VERSE) &&
		(nRelIndex.verse() != 0) &&
		((Vrs.ofBible().first != 0) ||
		 (Vrs.ofTestament().first != 0) ||
		 (Vrs.ofBook().first != 0) ||
		 (Vrs.ofChapter().first != 0))) {
		strTemp += QObject::tr("Verse:") + "\n";
		if (Vrs.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible").arg(Vrs.ofBible().first).arg(Vrs.ofBible().second) + "\n";
		}
		if (Vrs.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3").arg(Vrs.ofTestament().first).arg(Vrs.ofTestament().second).arg(testamentName(nRelIndex)) + "\n";
		}
		if (Vrs.ofBook().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3").arg(Vrs.ofBook().first).arg(Vrs.ofBook().second).arg(bookName(nRelIndex)) + "\n";
		}
		if (Vrs.ofChapter().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3 %4").arg(Vrs.ofChapter().first).arg(Vrs.ofChapter().second).arg(bookName(nRelIndex)).arg(nRelIndex.chapter()) + "\n";
		}
	}

	if ((nRIMask & RIMASK_WORD) &&
		(nRelIndex.word() != 0) &&
		((Wrd.ofBible().first != 0) ||
		 (Wrd.ofTestament().first != 0) ||
		 (Wrd.ofBook().first != 0) ||
		 (Wrd.ofChapter().first != 0) ||
		 (Wrd.ofVerse().first != 0))) {
		strTemp += QObject::tr("Word/Phrase:") + "\n";
		if (Wrd.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible").arg(Wrd.ofBible().first).arg(Wrd.ofBible().second) + "\n";
		}
		if (Wrd.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3").arg(Wrd.ofTestament().first).arg(Wrd.ofTestament().second).arg(testamentName(nRelIndex)) + "\n";
		}
		if (Wrd.ofBook().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3").arg(Wrd.ofBook().first).arg(Wrd.ofBook().second).arg(bookName(nRelIndex)) + "\n";
		}
		if (Wrd.ofChapter().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3 %4").arg(Wrd.ofChapter().first).arg(Wrd.ofChapter().second).arg(bookName(nRelIndex)).arg(nRelIndex.chapter()) + "\n";
		}
		if (Wrd.ofVerse().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3 %4:%5").arg(Wrd.ofVerse().first).arg(Wrd.ofVerse().second).arg(bookName(nRelIndex)).arg(nRelIndex.chapter()).arg(nRelIndex.verse()) + "\n";
		}
	}

	return strTemp;
}

QString CBibleDatabase::PassageReferenceText(const CRelIndex &nRelIndex) const
{
	if ((!nRelIndex.isSet()) || (nRelIndex.book() == 0)) return QObject::tr("<Invalid Reference>");
	if (nRelIndex.chapter() == 0) {
		return QString("%1").arg(bookName(nRelIndex));
	}
	if (nRelIndex.verse() == 0) {
		return QString("%1 %2").arg(bookName(nRelIndex)).arg(nRelIndex.chapter());
	}
	if (nRelIndex.word() == 0) {
		return QString("%1 %2:%3").arg(bookName(nRelIndex)).arg(nRelIndex.chapter()).arg(nRelIndex.verse());
	}
	return QString("%1 %2:%3 [%4]").arg(bookName(nRelIndex)).arg(nRelIndex.chapter()).arg(nRelIndex.verse()).arg(nRelIndex.word());
}

QString CBibleDatabase::PassageReferenceAbbrText(const CRelIndex &nRelIndex) const
{
	if ((!nRelIndex.isSet()) || (nRelIndex.book() == 0)) return QObject::tr("<Invalid Reference>");
	if (nRelIndex.chapter() == 0) {
		return QString("%1").arg(bookNameAbbr(nRelIndex));
	}
	if (nRelIndex.verse() == 0) {
		return QString("%1 %2").arg(bookNameAbbr(nRelIndex)).arg(nRelIndex.chapter());
	}
	if (nRelIndex.word() == 0) {
		return QString("%1 %2:%3").arg(bookNameAbbr(nRelIndex)).arg(nRelIndex.chapter()).arg(nRelIndex.verse());
	}
	return QString("%1 %2:%3 [%4]").arg(bookNameAbbr(nRelIndex)).arg(nRelIndex.chapter()).arg(nRelIndex.verse()).arg(nRelIndex.word());
}


// ============================================================================

CRefCountCalc::CRefCountCalc(const CBibleDatabase *pBibleDatabase, REF_TYPE_ENUM nRefType, const CRelIndex &refIndex)
:	m_ndxRef(refIndex),
	m_nRefType(nRefType),
	m_nOfBible(0,0),
	m_nOfTst(0,0),
	m_nOfBk(0,0),
	m_nOfChp(0,0),
	m_nOfVrs(0,0)
{
	assert(pBibleDatabase != NULL);
	switch (nRefType) {
		case RTE_TESTAMENT:				// Calculate the Testament of the Bible
			m_nOfBible.first = pBibleDatabase->testament(m_ndxRef);
			m_nOfBible.second = pBibleDatabase->bibleEntry().m_nNumTst;
			break;

		case RTE_BOOK:					// Calculate the Book of the Testament and Bible
			m_nOfBible.first = m_ndxRef.book();
			m_nOfBible.second = pBibleDatabase->bibleEntry().m_nNumBk;
			if (m_ndxRef.book() != 0) {
				const CBookEntry &book = *pBibleDatabase->bookEntry(m_ndxRef.book());
				m_nOfTst.first = book.m_nTstBkNdx;
				m_nOfTst.second = pBibleDatabase->testamentEntry(book.m_nTstNdx)->m_nNumBk;
			}
			break;

		case RTE_CHAPTER:				// Calculate the Chapter of the Book, Testament, and Bible
			m_nOfBk.first = m_ndxRef.chapter();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= pBibleDatabase->bibleEntry().m_nNumBk)) {
				m_nOfBk.second = pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumChp;
				// Number of Chapters in books prior to target:
				for (unsigned int ndxBk=1; ndxBk<m_ndxRef.book(); ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.first += pBook->m_nNumChp;
					m_nOfBible.first += pBook->m_nNumChp;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=m_ndxRef.book(); ndxBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.second += pBook->m_nNumChp;
					m_nOfBible.second += pBook->m_nNumChp;
				}
				// Number of Chapter in target:
				m_nOfTst.first += m_ndxRef.chapter();
				m_nOfBible.first += m_ndxRef.chapter();
			}
			break;

		case RTE_VERSE:					// Calculate the Verse of the Chapter, Book, Testament, and Bible
			m_nOfChp.first = m_ndxRef.verse();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= pBibleDatabase->bibleEntry().m_nNumBk) &&
				(m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumChp)) {
				m_nOfChp.second = pBibleDatabase->chapterEntry(m_ndxRef)->m_nNumVrs;
				m_nOfBk.second = pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumVrs;
				// Number of Verses in books prior to target:
				for (unsigned int ndxBk=1; ndxBk<m_ndxRef.book(); ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nVerses = pBook->m_nNumVrs;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.first += nVerses;
					m_nOfBible.first += nVerses;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=m_ndxRef.book(); ndxBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nVerses = pBook->m_nNumVrs;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.second += nVerses;
					m_nOfBible.second += nVerses;
				}
				// Number of Verses in Chapters prior to target in target book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nVerses = pBibleDatabase->chapterEntry(CRelIndex(m_ndxRef.book(),ndxChp,0,0))->m_nNumVrs;
					m_nOfBk.first += nVerses;
					m_nOfTst.first += nVerses;
					m_nOfBible.first += nVerses;
				}
				// Number of Verses in target:
				m_nOfBk.first += m_ndxRef.verse();
				m_nOfTst.first += m_ndxRef.verse();
				m_nOfBible.first += m_ndxRef.verse();
			}
			break;

		case RTE_WORD:					// Calculate the Word of the Verse, Book, Testament, and Bible
			m_nOfVrs.first = m_ndxRef.word();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= pBibleDatabase->bibleEntry().m_nNumBk) &&
				(m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumChp) &&
				(m_ndxRef.verse() > 0) && (m_ndxRef.verse() <= pBibleDatabase->chapterEntry(m_ndxRef)->m_nNumVrs)) {
				m_nOfVrs.second = pBibleDatabase->verseEntry(m_ndxRef)->m_nNumWrd;
				m_nOfChp.second = pBibleDatabase->chapterEntry(m_ndxRef)->m_nNumWrd;
				m_nOfBk.second = pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumWrd;
				// Number of Words in books prior to target:
				for (unsigned int ndxBk=1; ndxBk<m_ndxRef.book(); ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nWords = pBook->m_nNumWrd;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=m_ndxRef.book(); ndxBk<= pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nWords = pBook->m_nNumWrd;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.second += nWords;
					m_nOfBible.second += nWords;
				}
				// Number of Words in Chapters prior to target in target Book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nWords = pBibleDatabase->chapterEntry(CRelIndex(m_ndxRef.book(),ndxChp,0,0))->m_nNumWrd;
					m_nOfBk.first += nWords;
					m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				// Number of Words in Verses prior to target in target Chapter:
				for (unsigned int ndxVrs=1; ndxVrs<m_ndxRef.verse(); ++ndxVrs) {
					unsigned int nWords = pBibleDatabase->verseEntry(CRelIndex(m_ndxRef.book(),m_ndxRef.chapter(),ndxVrs,0))->m_nNumWrd;
					m_nOfChp.first += nWords;
					m_nOfBk.first += nWords;
					m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				// Number of Words in target:
				m_nOfChp.first += m_ndxRef.word();
				m_nOfBk.first += m_ndxRef.word();
				m_nOfTst.first += m_ndxRef.word();
				m_nOfBible.first += m_ndxRef.word();
			}
			break;
	}
}

// ============================================================================

CRelIndex CBibleDatabase::calcRelIndex(
					unsigned int nWord, unsigned int nVerse, unsigned int nChapter,
					unsigned int nBook, unsigned int nTestament,
					const CRelIndex &ndxStart,
					bool bReverse) const
{
	uint32_t ndxWord = 0;			// We will calculate target via word, which we can then call Denormalize on
	CRelIndex ndxResult;

	if (!bReverse) {
		// FORWARD:

		// Start with our relative location:
		nWord += ndxStart.word();
		nVerse += ndxStart.verse();
		nChapter += ndxStart.chapter();
		nBook += ndxStart.book();

		// Assume 1st word/verse/chapter/book of target:
		if (nWord == 0) nWord = 1;
		if (nVerse == 0) nVerse = 1;
		if (nChapter == 0) nChapter = 1;
		if (nBook == 0) nBook = 1;

		// ===================
		// Testament of Bible:
		if (nTestament) {
			if (nTestament > m_lstTestaments.size()) return CRelIndex();		// Testament too large, past end of Bible
			for (unsigned int ndx=1; ndx<nTestament; ++ndx) {
				// Ripple down to children:
				nBook += m_lstTestaments[ndx-1].m_nNumBk;
			}
		}	// At this point, top specified index will be relative to the Bible, nTestament isn't needed beyond this point

		// ===================
		// Book of Bible/Testament:
		if (nBook > m_lstBooks.size()) return CRelIndex();
		for (unsigned int ndx=1; ndx<nBook; ++ndx)
			ndxWord += m_lstBooks[ndx-1].m_nNumWrd;					// Add words for Books prior to target

		// ===================
		// Chapter of Bible/Testament:
		while (nChapter > m_lstBooks[nBook-1].m_nNumChp) {		// Resolve nBook
			ndxWord += m_lstBooks[nBook-1].m_nNumWrd;			// Add words for books prior to target book
			nChapter -= m_lstBooks[nBook-1].m_nNumChp;
			nBook++;
			if (nBook > m_lstBooks.size()) return CRelIndex();	// Chapter too large (past end of last Book of Bible/Testament)
		}
		// Chapter of Book:
		//	Note:  Here we'll push the verses of the chapter down to nVerse and
		//			relocate Chapter back to 1.  We do that so that we will be
		//			relative to the start of the book for the word search
		//			so that it can properly push to a following chapter or
		//			book if that's necessary.  Otherwise, we can run off the
		//			end of this book looking for more chapters.  We could,
		//			of course, update verse, chapter and/or book in the verse in
		//			book resolution loop, but that really complicates that,
		//			especially since we have to do that anyway.  We won't
		//			update ndxWord here since that will get done in the Verse
		//			loop below once we push this down to that:
		if (nChapter>m_lstBooks[nBook-1].m_nNumChp) return CRelIndex();		// Chapter too large (past end of book)
		for (unsigned int ndx=1; ndx<nChapter; ++ndx) {
			nVerse += m_mapChapters.at(CRelIndex(nBook, ndx, 0, 0)).m_nNumVrs;	// Push all chapters prior to target down to nVerse level
		}
		nChapter = 1;	// Reset to beginning of book so nVerse can count from there

		// ===================
		// Verse of Bible/Testament:
		while (nVerse > m_lstBooks[nBook-1].m_nNumVrs) {	// Resolve nBook
			ndxWord += m_lstBooks[nBook-1].m_nNumWrd;		// Add words for books prior to target book
			nVerse -= m_lstBooks[nBook-1].m_nNumVrs;
			nBook++;
			if (nBook > m_lstBooks.size()) return CRelIndex();	// Verse too large (past end of last Book of Bible/Testament)
		}
		// Verse of Book:
		if (nVerse>m_lstBooks[nBook-1].m_nNumVrs) return CRelIndex();		// Verse too large (past end of book)
		while (nVerse > m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs) {		// Resolve nChapter
			nVerse -= m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs;
			nChapter++;
			if (nChapter > m_lstBooks[nBook-1].m_nNumChp) return CRelIndex();	// Verse too large (past end of last Chapter of Book)
		}
		// Verse of Chapter:
		//	Note:  Here we'll push the words of the verses down to nWord and
		//			relocate Verse back to 1.  We do that so that we will be
		//			relative to the start of the chapter for the word search
		//			so that it can properly push to a following chapter or
		//			book if that's necessary.  Otherwise, we can run off the
		//			end of this chapter looking for more verses.  We could,
		//			of course, update chapter and/or book in the word in
		//			chapter resolution loop, but that really complicates that,
		//			especially since we have to do that anyway.  We won't
		//			update ndxWord here since that will get done in the Word
		//			loop below once we push this down to that:
		if (nVerse>m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs) return CRelIndex();		// Verse too large (past end of Chapter of Book)
		for (unsigned int ndx=1; ndx<nVerse; ++ndx) {
			nWord += m_lstBookVerses[nBook-1].at(CRelIndex(0, nChapter, ndx, 0)).m_nNumWrd;		// Push all verses prior to target down to nWord level
		}
		nVerse = 1;		// Reset to beginning of chapter so nWord can count from there

		for (unsigned int ndx=1; ndx<nChapter; ++ndx) {
			nWord += m_mapChapters.at(CRelIndex(nBook, ndx, 0, 0)).m_nNumWrd;	// Push all chapters prior to target down to nWord level
		}
		nChapter = 1;	// Reset to beginning of book so nWord can count from there

		// ===================
		// Word of Bible/Testament:
		while (nWord > m_lstBooks[nBook-1].m_nNumWrd) {		// Resolve nBook
			ndxWord += m_lstBooks[nBook-1].m_nNumWrd;
			nWord -= m_lstBooks[nBook-1].m_nNumWrd;
			nBook++;
			if (nBook > m_lstBooks.size()) return CRelIndex();		// Word too large (past end of last Book of Bible/Testament)
		}
		// Word of Book:
		if (nWord>m_lstBooks[nBook-1].m_nNumWrd) return CRelIndex();	// Word too large (past end of Book/Chapter)
		while (nWord > m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumWrd) {	// Resolve nChapter
			ndxWord += m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumWrd;
			nWord -= m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumWrd;
			nChapter++;
			if (nChapter > m_lstBooks[nBook-1].m_nNumChp) return CRelIndex();		// Word too large (past end of last Verse of last Book/Chapter)
		}
		// Word of Chapter:
		if (nWord>m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumWrd) return CRelIndex();		// Word too large (past end of Book/Chapter)
		const TVerseEntryMap &bookVerses = m_lstBookVerses[nBook-1];
		while (nWord > bookVerses.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd) {	// Resolve nVerse
			ndxWord += bookVerses.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd;
			nWord -= bookVerses.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd;
			nVerse++;
			if (nVerse > m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs) return CRelIndex();	// Word too large (past end of last Verse of last Book/Chapter)
		}
		// Word of Verse:
		if (nWord>m_lstBookVerses[nBook-1].at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd) return CRelIndex();		// Word too large (past end of Verse of Chapter of Book)
		ndxWord += nWord;		// Add up to include target word

		// ===================
		// At this point, either nBook/nChapter/nVerse/nWord is completely resolved.
		//		As a cross-check, ndxWord should be the Normalized index:
		ndxResult = CRelIndex(nBook, nChapter, nVerse, nWord);
		uint32_t ndxNormal = NormalizeIndex(ndxResult);
		assert(ndxWord == ndxNormal);
	} else {
		// REVERSE:

		// Start with starting location or last word of Bible:
		ndxWord = NormalizeIndex(ndxStart.index());
		if (ndxWord == 0) {
			// Set ndxWord to the total number of words in Bible:
			for (unsigned int ndx = 0; ndx<m_lstTestaments.size(); ++ndx) {
				ndxWord += m_lstTestaments[ndx].m_nNumWrd;
			}
		}
		// ndxWord is now pointing to the last word of the last verse of
		//	the last chapter of the last book... or is the word of the
		//	specified starting point...
		assert(ndxWord != 0);

		CRelIndex ndxTarget(DenormalizeIndex(ndxWord));
		assert(ndxTarget.index() != 0);		// Must be either the starting location or the last entry in the Bible

		// In Reverse mode, we ignore the nTestament entry

		// Word back:
		if (ndxWord <= nWord) return CRelIndex();
		ndxWord -= nWord;
		ndxTarget = CRelIndex(DenormalizeIndex(ndxWord));
		nWord = ndxTarget.word();					// nWord = Offset of word into verse so we can traverse from start of verse to start of verse
		ndxTarget.setWord(1);						// Move to first word of this verse
		ndxWord = NormalizeIndex(ndxTarget.index());

		// Verse back:
		while (nVerse) {
			ndxWord--;				// This will move us to previous verse since we are at word 1 of current verse (see above and below)
			if (ndxWord == 0) return CRelIndex();
			ndxTarget = CRelIndex(DenormalizeIndex(ndxWord));
			ndxTarget.setWord(1);	// Move to first word of this verse
			ndxWord = NormalizeIndex(ndxTarget.index());
			nVerse--;
		}
		nVerse = ndxTarget.verse();					// nVerse = Offset of verse into chapter so we can traverse from start of chapter to start of chapter
		ndxTarget.setVerse(1);						// Move to first verse of this chapter
		ndxWord = NormalizeIndex(ndxTarget.index());

		// Chapter back:
		while (nChapter) {
			ndxWord--;				// This will move us to previous chapter since we are at word 1 of verse 1 (see above and below)
			if (ndxWord == 0) return CRelIndex();
			ndxTarget = CRelIndex(DenormalizeIndex(ndxWord));
			ndxTarget.setVerse(1);	// Move to first word of first verse of this chapter
			ndxTarget.setWord(1);
			ndxWord = NormalizeIndex(ndxTarget.index());
			nChapter--;
		}
		nChapter = ndxTarget.chapter();				// nChapter = Offset of chapter into book so we can traverse from start of book to start of book
		ndxTarget.setChapter(1);					// Move to first chapter of this book
		ndxWord = NormalizeIndex(ndxTarget.index());

		// Book back:
		while (nBook) {
			ndxWord--;				// This will move us to previous book since we are at word 1 of verse 1 of chapter 1 (see above and below)
			if (ndxWord == 0) return CRelIndex();
			ndxTarget = CRelIndex(DenormalizeIndex(ndxWord));
			ndxTarget.setChapter(1);	// Move to first word of first verse of first chapter of this book
			ndxTarget.setVerse(1);
			ndxTarget.setWord(1);
			ndxWord = NormalizeIndex(ndxTarget.index());
			nBook--;
		}
		nBook = ndxTarget.book();					// nBook = Offset of book into Bible for final location
		ndxTarget.setBook(1);						// Move to first book of the Bible
		ndxWord = NormalizeIndex(ndxTarget.index());
		assert(ndxWord == 1);			// We should be at the beginning of the Bible now

		// Call ourselves to calculate index from beginning of Bible:
		ndxResult = calcRelIndex(nWord, nVerse, nChapter, nBook, 0);
	}

	return ndxResult;
}

// ============================================================================


CBibleDatabase::CBibleDatabase(const QString &strName, const QString &strDescription, const QString &strCompatUUID)
	:	m_strName(strName),
		m_strDescription(strDescription),
		m_strCompatibilityUUID(strCompatUUID),
		m_pKJPBSWordScriptureObject(new CKJPBSWordScriptureObject(this))
{

}

CBibleDatabase::~CBibleDatabase()
{
	if (m_pKJPBSWordScriptureObject) {
		delete m_pKJPBSWordScriptureObject;
		m_pKJPBSWordScriptureObject = NULL;
	}
}

void CBibleDatabase::registerTextLayoutHandlers(QAbstractTextDocumentLayout *pDocLayout)
{
	assert(m_pKJPBSWordScriptureObject != NULL);
	m_pKJPBSWordScriptureObject->registerTextLayoutHandlers(pDocLayout);
}

const CTestamentEntry *CBibleDatabase::testamentEntry(uint32_t nTst) const
{
	assert((nTst >= 1) && (nTst <= m_lstTestaments.size()));
	if ((nTst < 1) || (nTst > m_lstTestaments.size())) return NULL;
	return &m_lstTestaments.at(nTst-1);
}

const CBookCategoryEntry *CBibleDatabase::bookCategoryEntry(uint32_t nCat) const
{
	assert((nCat >= 1) && (nCat <= m_lstBookCategories.size()));
	if ((nCat < 1) || (nCat > m_lstBookCategories.size())) return NULL;
	return &m_lstBookCategories.at(nCat-1);
}

const CBookEntry *CBibleDatabase::bookEntry(uint32_t nBk) const
{
	assert((nBk >= 1) && (nBk <= m_lstBooks.size()));
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return NULL;
	return &m_lstBooks.at(nBk-1);
}

#ifdef OSIS_PARSER_BUILD
const CChapterEntry *CBibleDatabase::chapterEntry(const CRelIndex &ndx, bool bForceCreate) const
{
	if (bForceCreate) (const_cast<TChapterMap &>(m_mapChapters))[CRelIndex(ndx.book(),ndx.chapter(),0,0)];		// Force the creation of this entry
	TChapterMap::const_iterator chapter = m_mapChapters.find(CRelIndex(ndx.book(),ndx.chapter(),0,0));
	if (chapter == m_mapChapters.end()) return NULL;
	return &(chapter->second);
}

const CVerseEntry *CBibleDatabase::verseEntry(const CRelIndex &ndx, bool bForceCreate) const
{
	if ((ndx.book() < 1) || (ndx.book() > m_lstBookVerses.size())) return NULL;
	const TVerseEntryMap &book = m_lstBookVerses[ndx.book()-1];
	if (bForceCreate) (const_cast<TVerseEntryMap &>(book))[CRelIndex(0, ndx.chapter(), ndx.verse(), 0)];			// Force the creation of this entry
	const TVerseEntryMap::const_iterator mapVerse = book.find(CRelIndex(0, ndx.chapter(), ndx.verse(), 0));
	if (mapVerse == book.end()) return NULL;
	return &(mapVerse->second);
}
#else
const CChapterEntry *CBibleDatabase::chapterEntry(const CRelIndex &ndx) const
{
	TChapterMap::const_iterator chapter = m_mapChapters.find(CRelIndex(ndx.book(),ndx.chapter(),0,0));
	if (chapter == m_mapChapters.end()) return NULL;
	return &(chapter->second);
}

const CVerseEntry *CBibleDatabase::verseEntry(const CRelIndex &ndx) const
{
	if ((ndx.book() < 1) || (ndx.book() > m_lstBookVerses.size())) return NULL;
	const TVerseEntryMap &book = m_lstBookVerses[ndx.book()-1];
	const TVerseEntryMap::const_iterator mapVerse = book.find(CRelIndex(0, ndx.chapter(), ndx.verse(), 0));
	if (mapVerse == book.end()) return NULL;
	return &(mapVerse->second);
}
#endif

const CWordEntry *CBibleDatabase::wordlistEntry(const QString &strWord) const
{
	TWordListMap::const_iterator word = m_mapWordList.find(strWord);
	if (word == m_mapWordList.end()) return NULL;
	return &(word->second);
}

QString CBibleDatabase::wordAtIndex(uint32_t ndxNormal) const
{
	if ((ndxNormal < 1) || (ndxNormal > m_lstConcordanceMapping.size()))
		return QString();

	return m_lstConcordanceWords.at(m_lstConcordanceMapping.at(ndxNormal)).word();
}

QString CBibleDatabase::wordAtIndex(const CRelIndex &relIndex) const
{
	if (!relIndex.isSet()) return QString();
	return wordAtIndex(NormalizeIndex(relIndex));
}

QString CBibleDatabase::decomposedWordAtIndex(uint32_t ndxNormal) const
{
	assert((ndxNormal >= 1) && (ndxNormal <= m_lstConcordanceMapping.size()));
	if ((ndxNormal < 1) || (ndxNormal > m_lstConcordanceMapping.size()))
		return QString();

	return m_lstConcordanceWords.at(m_lstConcordanceMapping.at(ndxNormal)).decomposedWord();
}

const CFootnoteEntry *CBibleDatabase::footnoteEntry(const CRelIndex &ndx) const
{
	TFootnoteEntryMap::const_iterator footnote = m_mapFootnotes.find(ndx);
	if (footnote == m_mapFootnotes.end()) return NULL;
	return &(footnote->second);
}

QString CBibleDatabase::richVerseText(const CRelIndex &ndxRel, const CVerseTextRichifierTags &tags, bool bAddAnchors) const
{
	CRelIndex ndx = ndxRel;
	ndx.setWord(0);							// We always return the whole verse, not specific words
	const CVerseEntry *pVerse = verseEntry(ndx);
	assert(pVerse != NULL);

#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
	TVerseCacheMap &cache = (bAddAnchors ? m_mapVerseCacheWithAnchors[tags.hash()] : m_mapVerseCacheNoAnchors[tags.hash()]);
	TVerseCacheMap::iterator itr = cache.find(ndx);
	if (itr != cache.end()) return (itr->second);
	cache[ndx] = CVerseTextRichifier::parse(ndx, this, pVerse, tags, bAddAnchors);
	return cache[ndx];
#else
	return CVerseTextRichifier::parse(ndx, this, pVerse, tags, bAddAnchors);
#endif
}

#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
void CBibleDatabase::dumpRichVerseTextCache(uint nTextRichifierTagHash)
{
	if (nTextRichifierTagHash == 0) {
		m_mapVerseCacheWithAnchors.clear();
		m_mapVerseCacheNoAnchors.clear();
		return;
	}

	TSpecVerseCacheMap::iterator itr;

	itr = m_mapVerseCacheWithAnchors.find(nTextRichifierTagHash);
	if (itr != m_mapVerseCacheWithAnchors.end())
		(itr->second).clear();

	itr = m_mapVerseCacheNoAnchors.find(nTextRichifierTagHash);
	if (itr != m_mapVerseCacheNoAnchors.end())
		(itr->second).clear();
}
#endif

QString CBibleDatabase::soundEx(const QString &strDecomposedConcordanceWord, bool bCache) const
{
	if (bCache) {
		QString &strSoundEx = m_mapSoundEx[strDecomposedConcordanceWord];
		// TODO : Assert if language not set
		if (strSoundEx.isEmpty()) strSoundEx = CSoundExSearchCompleterFilter::soundEx(strDecomposedConcordanceWord);			// TODO : Pass Language ID to soundEx
		return strSoundEx;
	}

	TSoundExMap::const_iterator itrSoundEx = m_mapSoundEx.find(strDecomposedConcordanceWord);
	if (itrSoundEx != m_mapSoundEx.end()) return (itrSoundEx->second);
	return CSoundExSearchCompleterFilter::soundEx(strDecomposedConcordanceWord);			// TODO : Pass Language ID to soundEx
}

// ============================================================================

void TPhraseTag::setFromPassageTag(CBibleDatabasePtr pBibleDatabase, const TPassageTag &tagPassage)
{
	if (!tagPassage.isSet()) {
		m_RelIndex = CRelIndex();
		m_nCount = 0;
	} else {
		assert(pBibleDatabase != NULL);
		m_RelIndex = tagPassage.relIndex();
		CRelIndex ndxStart = tagPassage.relIndex();
		ndxStart.setWord(1);
		CRelIndex ndxTarget = tagPassage.relIndex();
		if (tagPassage.verseCount() > 1) {
			// If more than one verse is specified, find the beginning of the last verse:
			ndxTarget = pBibleDatabase->calcRelIndex(0, tagPassage.verseCount()-1, 0, 0, 0, ndxStart);
		}
		ndxTarget.setWord(pBibleDatabase->verseEntry(ndxTarget)->m_nNumWrd);		// Select all words of last verse
		m_nCount = pBibleDatabase->NormalizeIndex(ndxTarget) - pBibleDatabase->NormalizeIndex(ndxStart);
	}
}

bool TPhraseTag::completelyContains(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag) const
{
	if ((!relIndex().isSet()) || (!aTag.relIndex().isSet())) return false;

	assert(pBibleDatabase != NULL);
	uint32_t nNormalRefLo = pBibleDatabase->NormalizeIndex(relIndex());
	uint32_t nNormalRefHi = nNormalRefLo + count() - ((count() != 0) ? 1 : 0);
	uint32_t nNormalSrcLo = pBibleDatabase->NormalizeIndex(aTag.relIndex());
	uint32_t nNormalSrcHi = nNormalSrcLo + aTag.count() - ((aTag.count() != 0) ? 1 : 0);

	if ((nNormalSrcLo >= nNormalRefLo) && (nNormalSrcHi <= nNormalRefHi)) return true;
	return false;
}

bool TPhraseTag::intersects(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag) const
{
	if ((!relIndex().isSet()) || (!aTag.relIndex().isSet())) return false;

	assert(pBibleDatabase != NULL);
	uint32_t nNormalRefLo = pBibleDatabase->NormalizeIndex(relIndex());
	uint32_t nNormalRefHi = nNormalRefLo + count() - ((count() != 0) ? 1 : 0);
	uint32_t nNormalSrcLo = pBibleDatabase->NormalizeIndex(aTag.relIndex());
	uint32_t nNormalSrcHi = nNormalSrcLo + aTag.count() - ((aTag.count() != 0) ? 1 : 0);

	if (((nNormalSrcLo >= nNormalRefLo) && (nNormalSrcLo <= nNormalRefHi)) ||
		((nNormalSrcHi >= nNormalRefLo) && (nNormalSrcHi <= nNormalRefHi)) ||
		((nNormalRefLo >= nNormalSrcLo) && (nNormalRefLo <= nNormalSrcHi)) ||
		((nNormalRefHi >= nNormalSrcLo) && (nNormalRefHi <= nNormalSrcHi))) return true;
	return false;
}

bool TPhraseTag::intersectingInsert(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag)
{
	if ((!relIndex().isSet()) || (!aTag.relIndex().isSet())) return false;

	assert(pBibleDatabase != NULL);
	uint32_t nNormalRefLo = pBibleDatabase->NormalizeIndex(relIndex());
	uint32_t nNormalRefHi = nNormalRefLo + count() - ((count() != 0) ? 1 : 0);
	uint32_t nNormalSrcLo = pBibleDatabase->NormalizeIndex(aTag.relIndex());
	uint32_t nNormalSrcHi = nNormalSrcLo + aTag.count() - ((aTag.count() != 0) ? 1 : 0);
	bool bHadCount = ((count() != 0) || (aTag.count() != 0));			// If we end up with a single word, this will tell us if we have that word selected or not

	if (((nNormalSrcLo >= nNormalRefLo) && (nNormalSrcLo <= nNormalRefHi)) ||
		((nNormalSrcHi >= nNormalRefLo) && (nNormalSrcHi <= nNormalRefHi)) ||
		((nNormalRefLo >= nNormalSrcLo) && (nNormalRefLo <= nNormalSrcHi)) ||
		((nNormalRefHi >= nNormalSrcLo) && (nNormalRefHi <= nNormalSrcHi))) {
		uint32_t nMinNormal = qMin(nNormalRefLo, nNormalSrcLo);
		uint32_t nMaxNormal = qMax(nNormalRefHi, nNormalSrcHi);
		m_RelIndex.setIndex(pBibleDatabase->DenormalizeIndex(nMinNormal));
		m_nCount = (nMaxNormal - nMinNormal + 1);
		if ((m_nCount == 1) && (!bHadCount)) m_nCount = 0;
		return true;
	}

	return false;
}

// ============================================================================

TPhraseTagList::TPhraseTagList()
	:	QList<TPhraseTag>()
{

}

TPhraseTagList::TPhraseTagList(const TPhraseTagList &src)
	:	QList<TPhraseTag>(src)
{

}

bool TPhraseTagList::completelyContains(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag) const
{
	assert(pBibleDatabase != NULL);

	if (!aTag.relIndex().isSet()) return false;

	// To avoid having to flatten our entire list of tags and check for complete intersection, it
	//	would be easier to check every word of the specified tag and see if we have an intersection.
	//	Since we are limiting it to one-word tags, if we have an intersection for all of them, we
	//	know it's completely contained:
	uint32_t nNormalSrcLo = pBibleDatabase->NormalizeIndex(aTag.relIndex());
	uint32_t nNormalSrcHi = nNormalSrcLo + aTag.count() - ((aTag.count() != 0) ? 1 : 0);
	bool bContained = true;

	while ((bContained) && (nNormalSrcLo <= nNormalSrcHi)) {
		bool bFound = false;
		for (const_iterator itrTags = constBegin(); ((!bFound) && (itrTags != constEnd())); ++itrTags) {
			uint32_t nNormalRefLo = pBibleDatabase->NormalizeIndex(itrTags->relIndex());
			uint32_t nNormalRefHi = nNormalRefLo + itrTags->count() - ((itrTags->count() != 0) ? 1 : 0);
			if ((nNormalSrcLo >= nNormalRefLo) && (nNormalSrcLo <= nNormalRefHi)) bFound = true;
		}
		if (!bFound) bContained = false;
		++nNormalSrcLo;
	}

	return bContained;
}

void TPhraseTagList::intersectingInsert(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag)
{
	assert(pBibleDatabase != NULL);

	if (!aTag.relIndex().isSet()) return;

	iterator itrLast = end();			// Tag we are comparing once we find an intersection, just initialize it to something (I don't like uninitialized vars!)
	bool bFoundFirst = false;			// True when we find our first intersection

	//	When we find an intersection, combine them, move

	for (iterator itrTags = begin(); itrTags != end(); /* iterator inside loop */) {
		if (!bFoundFirst) {
			if (itrTags->intersectingInsert(pBibleDatabase, aTag)) {
				// If we find an intersection, update it and set our iterator to it.  But keep
				//		processing in case we find others:
				itrLast = itrTags;
				bFoundFirst = true;
			}
		} else {
			// If we've already found one, compare the next one with the previous.
			if (itrLast->intersectingInsert(pBibleDatabase, *itrTags)) {
				// If we were able to insert it back into the other, calc or iterator distances
				//		so we can nuke the list with a remove, and then continue:
				int nDistLast = std::distance(begin(), itrLast);
				itrTags = erase(itrTags);
				itrLast = begin() + nDistLast;		// Fix our iterator to our combining tag
				continue;			// No need to increment our iterator, since we just did in the erase
			}
		}

		++itrTags;
	}
	if (!bFoundFirst) append(aTag);			// If we didn't find it anywhere, add the new one on the end
}

bool TPhraseTagList::removeIntersection(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &aTag)
{
	assert(pBibleDatabase != NULL);

	bool bRemovedIntersection = false;

	if (!aTag.relIndex().isSet()) return false;

	for (iterator itrTags = begin(); itrTags != end(); /* increment in loop */) {
		if (!aTag.haveSelection()) {
			// If there is no selection and the tag in the list contains the specified
			//		reference, remove it:
			if (itrTags->completelyContains(pBibleDatabase, aTag)) {
				itrTags = erase(itrTags);
				bRemovedIntersection = true;
				continue;
			}
		} else if (aTag.completelyContains(pBibleDatabase, *itrTags)) {
			// If the passed tag completely contains the one in the list, remove the one in the list:
			itrTags = erase(itrTags);
			bRemovedIntersection = true;
			continue;
		} else if (itrTags->intersects(pBibleDatabase, aTag)) {
			// This is the difficult one...  There's an intersection between the two,
			//		but we don't know if it's the first part, last part, or some part in the middle.
			//		If it's some part in the middle, we have to split it in half (yuck):

			uint32_t nNormalRefLo = pBibleDatabase->NormalizeIndex(itrTags->relIndex());
			uint32_t nNormalRefHi = nNormalRefLo + itrTags->count() - ((itrTags->count() != 0) ? 1 : 0);
			uint32_t nNormalSrcLo = pBibleDatabase->NormalizeIndex(aTag.relIndex());
			uint32_t nNormalSrcHi = nNormalSrcLo + aTag.count() - ((aTag.count() != 0) ? 1 : 0);		// Note: count() shouldn't be zero due to above 'if'
			bool bHadCount = ((itrTags->count() != 0) || (aTag.count() != 0));			// If we end up with a single word, this will tell us if we have that word selected or not

			uint32_t nNormalNewLo = nNormalRefLo;
			uint32_t nNormalNewHi = nNormalRefHi;
			bool bSingle = false;

			if ((nNormalSrcLo > nNormalRefLo) && (nNormalSrcHi >= nNormalRefHi)) {
				// Trim on the bottom part of the range case:
				nNormalNewHi = nNormalSrcLo - 1;
				bSingle = true;
			} else if ((nNormalSrcLo <= nNormalRefLo) && (nNormalSrcHi < nNormalRefHi)) {
				// Trim on the top part of the range case:
				nNormalNewLo = nNormalSrcHi + 1;
				bSingle = true;
			}

			// if bSingle, trim to nNormalNew, else nNormalSrc is in the middle of nNormalRef and needs a split:
			if (bSingle) {
				itrTags->m_RelIndex.setIndex(pBibleDatabase->DenormalizeIndex(nNormalNewLo));
				itrTags->m_nCount = (nNormalNewHi - nNormalNewLo + 1);
				if ((itrTags->m_nCount == 1) && (!bHadCount)) itrTags->m_nCount = 0;
			} else {
				// For the split, make the current be the low half:
				itrTags->m_RelIndex.setIndex(pBibleDatabase->DenormalizeIndex(nNormalRefLo));
				itrTags->m_nCount = (nNormalSrcLo - nNormalRefLo);		// Note, don't include first of cut section
				int nDistTags = std::distance(begin(), itrTags);		// Save where our iterator is at, since we're about to nuke it
				// And insert the upper half:
				append(TPhraseTag(CRelIndex(pBibleDatabase->DenormalizeIndex(nNormalSrcHi + 1)), nNormalRefHi - nNormalSrcHi));
				// Fix the iterator we just nuked:
				itrTags = begin() + nDistTags;
			}
		}
		++itrTags;
	}

	return bRemovedIntersection;
}

// ============================================================================

void TPassageTag::setFromPhraseTag(CBibleDatabasePtr pBibleDatabase, const TPhraseTag &tagPhrase)
{
	if (!tagPhrase.isSet()) {
		m_RelIndex = CRelIndex();
		m_nVerseCount = 0;
	} else {
		assert(pBibleDatabase != NULL);
		m_RelIndex = tagPhrase.relIndex();
		m_RelIndex.setWord(1);
		CRelIndex ndxTarget = tagPhrase.relIndex();
		if (tagPhrase.count() > 1) {
			ndxTarget = pBibleDatabase->calcRelIndex(tagPhrase.count()-1, 0, 0, 0, 0, tagPhrase.relIndex());
		}
		m_nVerseCount = (CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_VERSE, ndxTarget).ofBible().first -
						CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_VERSE, tagPhrase.relIndex()).ofBible().first) + 1;
	}
}

// ============================================================================

bool HighlighterNameSortPredicate::operator() (const QString &v1, const QString &v2) const
{
	return (CSearchStringListModel::decompose(v1).compare(CSearchStringListModel::decompose(v2), Qt::CaseInsensitive) < 0);
}

// ============================================================================

