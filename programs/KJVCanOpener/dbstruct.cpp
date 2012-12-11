// dbstruct.cpp -- Defines structures used for database info
//

#include "dbstruct.h"

#include <QtAlgorithms>
#include <QSet>

#include <assert.h>

// Global Testament List:
TTestamentList g_lstTestaments;

// Global Table of Contents:
TTOCList g_lstTOC;

// Global Layout:
TLayoutMap g_mapLayout;

// Global Books:
TBookList g_lstBooks;

// Our one and only master word list:
TWordListMap g_mapWordList;

// List of all Unique Words in the order for the concordance with names of the TWordListMap key:
TConcordanceList g_lstConcordanceWords;

// List of WordNdx# (in ConcordanceMapping) for all 789629 words of the text:
TIndexList g_lstConcordanceMapping;

// Common phrases read from database:
CPhraseList g_lstCommonPhrases;

// User-defined phrases read from optional user database:
CPhraseList g_lstUserPhrases;
bool g_bUserPhrasesDirty = false;				// True if user has edited the phrase list

// ============================================================================

// User-editable options (TODO: move to options class)
bool g_bEnableNoLimits = false;

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

uint32_t NormalizeIndex(const CRelIndex &nRelIndex)
{
	return NormalizeIndex(nRelIndex.index());
}

uint32_t NormalizeIndex(uint32_t nRelIndex)
{
	uint32_t nNormalIndex = 0;
	unsigned int nBk = ((nRelIndex >> 24) & 0xFF);
	unsigned int nChp = ((nRelIndex >> 16) & 0xFF);
	unsigned int nVrs = ((nRelIndex >> 8) & 0xFF);
	unsigned int nWrd = (nRelIndex & 0xFF);

	if (nRelIndex == 0) return 0;

	// Add the number of words for all books prior to the target book:
	if (nBk < 1) return 0;
	if (nBk > g_lstTOC.size()) return 0;
	for (unsigned int ndxBk = 1; ndxBk < nBk; ++ndxBk) {
		nNormalIndex += g_lstTOC[ndxBk-1].m_nNumWrd;
	}
	// Add the number of words for all chapters in this book prior to the target chapter:
	if (nChp > g_lstTOC[nBk-1].m_nNumChp) return 0;
	for (unsigned int ndxChp = 1; ndxChp < nChp; ++ndxChp) {
		nNormalIndex += g_mapLayout[CRelIndex(nBk,ndxChp,0,0)].m_nNumWrd;
	}
	// Add the number of words for all verses in this book prior to the target verse:
	if (nVrs > g_mapLayout[CRelIndex(nBk,nChp,0,0)].m_nNumVrs) return 0;
	for (unsigned int ndxVrs = 1; ndxVrs < nVrs; ++ndxVrs) {
		nNormalIndex += (g_lstBooks[nBk-1])[CRelIndex(0,nChp,ndxVrs,0)].m_nNumWrd;
	}
	// Add the target word:
	if (nWrd > (g_lstBooks[nBk-1])[CRelIndex(0,nChp,nVrs,0)].m_nNumWrd) return 0;
	nNormalIndex += nWrd;

	return nNormalIndex;
}

uint32_t DenormalizeIndex(uint32_t nNormalIndex)
{
	unsigned int nBk = 0;
	unsigned int nChp = 1;
	unsigned int nVrs = 1;
	unsigned int nWrd = nNormalIndex;

	if (nNormalIndex == 0) return 0;

	while (nBk < g_lstTOC.size()) {
		if (g_lstTOC[nBk].m_nNumWrd >= nWrd) break;
		nWrd -= g_lstTOC[nBk].m_nNumWrd;
		nBk++;
	}
	if (nBk >= g_lstTOC.size()) return 0;
	nBk++;

	while (nChp <= g_lstTOC[nBk-1].m_nNumChp) {
		if (g_mapLayout[CRelIndex(nBk,nChp,0,0)].m_nNumWrd >= nWrd) break;
		nWrd -= g_mapLayout[CRelIndex(nBk,nChp,0,0)].m_nNumWrd;
		nChp++;
	}
	if (nChp > g_lstTOC[nBk-1].m_nNumChp) return 0;

	while (nVrs <= g_mapLayout[CRelIndex(nBk,nChp,0,0)].m_nNumVrs) {
		if ((g_lstBooks[nBk-1])[CRelIndex(0,nChp,nVrs,0)].m_nNumWrd >= nWrd) break;
		nWrd -= (g_lstBooks[nBk-1])[CRelIndex(0,nChp,nVrs,0)].m_nNumWrd;
		nVrs++;
	}
	if (nVrs > g_mapLayout[CRelIndex(nBk,nChp,0,0)].m_nNumVrs) return 0;

	if (nWrd > (g_lstBooks[nBk-1])[CRelIndex(0,nChp,nVrs,0)].m_nNumWrd) return 0;

	return CRelIndex(nBk, nChp, nVrs, nWrd).index();
}

// ============================================================================

QString CRelIndex::testamentName() const
{
	uint32_t nTst = testament();
	if ((nTst < 1) || (nTst > g_lstTestaments.size())) return QString();
	return g_lstTestaments[nTst-1].m_strTstName;
}

uint32_t CRelIndex::testament() const
{
	uint32_t nBk = book();
	if ((nBk < 1) || (nBk > g_lstTOC.size())) return 0;
	const CTOCEntry &toc = g_lstTOC[nBk-1];
	return toc.m_nTstNdx;
}

QString CRelIndex::bookName() const
{
	uint32_t nBk = book();
	if ((nBk < 1) || (nBk > g_lstTOC.size())) return QString();
	const CTOCEntry &toc = g_lstTOC[nBk-1];
	return toc.m_strBkName;
}

QString CRelIndex::SearchResultToolTip(int nRIMask, unsigned int nSelectionSize) const
{
	CRefCountCalc Bk(CRefCountCalc::RTE_BOOK, *this);
	CRefCountCalc Chp(CRefCountCalc::RTE_CHAPTER, *this);
	CRefCountCalc Vrs(CRefCountCalc::RTE_VERSE, *this);
	CRefCountCalc Wrd(CRefCountCalc::RTE_WORD, *this);

	QString strTemp;

	if (nRIMask & RIMASK_HEADING) {
		if (nSelectionSize > 1) {
			strTemp += PassageReferenceText();
			strTemp += " - ";
			strTemp += CRelIndex(DenormalizeIndex(NormalizeIndex(index()) + nSelectionSize - 1)).PassageReferenceText();
			strTemp += QString(" (%1 Words)").arg(nSelectionSize);
			strTemp += "\n\n";
		} else {
			strTemp += PassageReferenceText();
			strTemp += "\n\n";
		}
	}

	if ((nRIMask & RIMASK_BOOK) &&
		((Bk.ofBible().first != 0) ||
		 (Bk.ofTestament().first != 0))) {
		strTemp += "Book:\n";
		if (Bk.ofBible().first != 0) {
			strTemp += QString("    %1 of %2 of Bible\n").arg(Bk.ofBible().first).arg(Bk.ofBible().second);
		}
		if (Bk.ofTestament().first != 0) {
			strTemp += QString("    %1 of %2 of %3\n").arg(Bk.ofTestament().first).arg(Bk.ofTestament().second).arg(testamentName());
		}
	}

	if ((nRIMask & RIMASK_CHAPTER) &&
		(chapter() != 0) &&
		((Chp.ofBible().first != 0) ||
		 (Chp.ofTestament().first != 0) ||
		 (Chp.ofBook().first != 0))) {
		strTemp += "Chapter:\n";
		if (Chp.ofBible().first != 0) {
			strTemp += QString("    %1 of %2 of Bible\n").arg(Chp.ofBible().first).arg(Chp.ofBible().second);
		}
		if (Chp.ofTestament().first != 0) {
			strTemp += QString("    %1 of %2 of %3\n").arg(Chp.ofTestament().first).arg(Chp.ofTestament().second).arg(testamentName());
		}
		if (Chp.ofBook().first != 0) {
			strTemp += QString("    %1 of %2 of %3\n").arg(Chp.ofBook().first).arg(Chp.ofBook().second).arg(bookName());
		}
	}

	if ((nRIMask & RIMASK_VERSE) &&
		(verse() != 0) &&
		((Vrs.ofBible().first != 0) ||
		 (Vrs.ofTestament().first != 0) ||
		 (Vrs.ofBook().first != 0) ||
		 (Vrs.ofChapter().first != 0))) {
		strTemp += "Verse:\n";
		if (Vrs.ofBible().first != 0) {
			strTemp += QString("    %1 of %2 of Bible\n").arg(Vrs.ofBible().first).arg(Vrs.ofBible().second);
		}
		if (Vrs.ofTestament().first != 0) {
			strTemp += QString("    %1 of %2 of %3\n").arg(Vrs.ofTestament().first).arg(Vrs.ofTestament().second).arg(testamentName());
		}
		if (Vrs.ofBook().first != 0) {
			strTemp += QString("    %1 of %2 of %3\n").arg(Vrs.ofBook().first).arg(Vrs.ofBook().second).arg(bookName());
		}
		if (Vrs.ofChapter().first != 0) {
			strTemp += QString("    %1 of %2 of Chapter\n").arg(Vrs.ofChapter().first).arg(Vrs.ofChapter().second);
		}
	}

	if ((nRIMask & RIMASK_WORD) &&
		(word() != 0) &&
		((Wrd.ofBible().first != 0) ||
		 (Wrd.ofTestament().first != 0) ||
		 (Wrd.ofBook().first != 0) ||
		 (Wrd.ofChapter().first != 0) ||
		 (Wrd.ofVerse().first != 0))) {
		strTemp += "Word/Phrase:\n";
		if (Wrd.ofBible().first != 0) {
			strTemp += QString("    %1 of %2 of Bible\n").arg(Wrd.ofBible().first).arg(Wrd.ofBible().second);
		}
		if (Wrd.ofTestament().first != 0) {
			strTemp += QString("    %1 of %2 of %3\n").arg(Wrd.ofTestament().first).arg(Wrd.ofTestament().second).arg(testamentName());
		}
		if (Wrd.ofBook().first != 0) {
			strTemp += QString("    %1 of %2 of %3\n").arg(Wrd.ofBook().first).arg(Wrd.ofBook().second).arg(bookName());
		}
		if (Wrd.ofChapter().first != 0) {
			strTemp += QString("    %1 of %2 of Chapter\n").arg(Wrd.ofChapter().first).arg(Wrd.ofChapter().second);
		}
		if (Wrd.ofVerse().first != 0) {
			strTemp += QString("    %1 of %2 of Verse\n").arg(Wrd.ofVerse().first).arg(Wrd.ofVerse().second);
		}
	}

	return strTemp;
}

QString CRelIndex::PassageReferenceText() const
{
	if ((!isSet()) || (book() == 0)) return "<Invalid Reference>";
	if (chapter() == 0) {
		return QString("%1").arg(bookName());
	}
	if (verse() == 0) {
		return QString("%1 %2").arg(bookName()).arg(chapter());
	}
	if (word() == 0) {
		return QString("%1 %2:%3").arg(bookName()).arg(chapter()).arg(verse());
	}
	return QString("%1 %2:%3 [%4]").arg(bookName()).arg(chapter()).arg(verse()).arg(word());
}

// ============================================================================

CRefCountCalc::CRefCountCalc(REF_TYPE_ENUM nRefType, const CRelIndex &refIndex)
:	m_ndxRef(refIndex),
	m_nRefType(nRefType),
	m_nOfBible(0,0),
	m_nOfTst(0,0),
	m_nOfBk(0,0),
	m_nOfChp(0,0),
	m_nOfVrs(0,0)
{
	switch (nRefType) {
		case RTE_TESTAMENT:				// Calculate the Testament of the Bible
			m_nOfBible.first = m_ndxRef.testament();
			m_nOfBible.second = g_lstTestaments.size();
			break;

		case RTE_BOOK:					// Calculate the Book of the Testament and Bible
			m_nOfBible.first = m_ndxRef.book();
			m_nOfBible.second = g_lstTOC.size();
			if (m_ndxRef.book() != 0) {
				const CTOCEntry &toc = g_lstTOC[m_ndxRef.book()-1];
				m_nOfTst.first = toc.m_nTstBkNdx;
				m_nOfTst.second = g_lstTestaments[toc.m_nTstNdx-1].m_nNumBk;
			}
			break;

		case RTE_CHAPTER:				// Calculate the Chapter of the Book, Testament, and Bible
			m_nOfBk.first = m_ndxRef.chapter();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= g_lstTOC.size())) {
				m_nOfBk.second = g_lstTOC[m_ndxRef.book()-1].m_nNumChp;
				// Number of Chapters in books prior to target:
				for (unsigned int ndxBk=0; ndxBk<(m_ndxRef.book()-1); ++ndxBk) {
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst.first += g_lstTOC[ndxBk].m_nNumChp;
					m_nOfBible.first += g_lstTOC[ndxBk].m_nNumChp;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=(m_ndxRef.book()-1); ndxBk<g_lstTOC.size(); ++ndxBk) {
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst.second += g_lstTOC[ndxBk].m_nNumChp;
					m_nOfBible.second += g_lstTOC[ndxBk].m_nNumChp;
				}
				// Number of Chapter in target:
				m_nOfTst.first += m_ndxRef.chapter();
				m_nOfBible.first += m_ndxRef.chapter();
			}
			break;

		case RTE_VERSE:					// Calculate the Verse of the Chapter, Book, Testament, and Bible
			m_nOfChp.first = m_ndxRef.verse();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= g_lstTOC.size()) &&
				(m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= g_lstTOC[m_ndxRef.book()-1].m_nNumChp)) {
				m_nOfChp.second = g_mapLayout[CRelIndex(m_ndxRef.book(),m_ndxRef.chapter(),0,0)].m_nNumVrs;
				m_nOfBk.second = g_lstTOC[m_ndxRef.book()-1].m_nNumVrs;
				// Number of Verses in books prior to target:
				for (unsigned int ndxBk=0; ndxBk<(m_ndxRef.book()-1); ++ndxBk) {
					unsigned int nVerses = g_lstTOC[ndxBk].m_nNumVrs;
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst.first += nVerses;
					m_nOfBible.first += nVerses;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=(m_ndxRef.book()-1); ndxBk<g_lstTOC.size(); ++ndxBk) {
					unsigned int nVerses = g_lstTOC[ndxBk].m_nNumVrs;
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst.second += nVerses;
					m_nOfBible.second += nVerses;
				}
				// Number of Verses in Chapters prior to target in target book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nVerses = g_mapLayout[CRelIndex(m_ndxRef.book(),ndxChp,0,0)].m_nNumVrs;
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
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= g_lstTOC.size()) &&
				(m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= g_lstTOC[m_ndxRef.book()-1].m_nNumChp) &&
				(m_ndxRef.verse() > 0) && (m_ndxRef.verse() <= g_mapLayout[CRelIndex(m_ndxRef.book(),m_ndxRef.chapter(),0,0)].m_nNumVrs)) {
				m_nOfVrs.second = (g_lstBooks[m_ndxRef.book()-1])[CRelIndex(0,m_ndxRef.chapter(),m_ndxRef.verse(),0)].m_nNumWrd;
				m_nOfChp.second = g_mapLayout[CRelIndex(m_ndxRef.book(),m_ndxRef.chapter(),0,0)].m_nNumWrd;
				m_nOfBk.second = g_lstTOC[m_ndxRef.book()-1].m_nNumWrd;
				// Number of Words in books prior to target:
				for (unsigned int ndxBk=0; ndxBk<(m_ndxRef.book()-1); ++ndxBk) {
					unsigned int nWords = g_lstTOC[ndxBk].m_nNumWrd;
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=(m_ndxRef.book()-1); ndxBk<g_lstTOC.size(); ++ndxBk) {
					unsigned int nWords = g_lstTOC[ndxBk].m_nNumWrd;
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst.second += nWords;
					m_nOfBible.second += nWords;
				}
				// Number of Words in Chapters prior to target in target Book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nWords = g_mapLayout[CRelIndex(m_ndxRef.book(),ndxChp,0,0)].m_nNumWrd;
					m_nOfBk.first += nWords;
					m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				// Number of Words in Verses prior to target in target Chapter:
				for (unsigned int ndxVrs=1; ndxVrs<m_ndxRef.verse(); ++ndxVrs) {
					unsigned int nWords = (g_lstBooks[m_ndxRef.book()-1])[CRelIndex(0,m_ndxRef.chapter(),ndxVrs,0)].m_nNumWrd;
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

QString CRefCountCalc::SearchResultToolTip(const CRelIndex &refIndex, int nRIMask, unsigned int nSelectionSize)
{
	return refIndex.SearchResultToolTip(nRIMask, nSelectionSize);
}

QString CRefCountCalc::PassageReferenceText(const CRelIndex &refIndex)
{
	return refIndex.PassageReferenceText();
}

CRelIndex CRefCountCalc::calcRelIndex(
					unsigned int nWord, unsigned int nVerse, unsigned int nChapter,
					unsigned int nBook, unsigned int nTestament,
					CRelIndex ndxStart,
					bool bReverse)
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
			if (nTestament > g_lstTestaments.size()) return CRelIndex();		// Testament too large, past end of Bible
			for (unsigned int ndx=1; ndx<nTestament; ++ndx) {
				// Ripple down to children:
				nBook += g_lstTestaments[ndx-1].m_nNumBk;
			}
		}	// At this point, top specified index will be relative to the Bible, nTestament isn't needed beyond this point

		// ===================
		// Book of Bible/Testament:
		if (nBook > g_lstTOC.size()) return CRelIndex();
		for (unsigned int ndx=1; ndx<nBook; ++ndx)
			ndxWord += g_lstTOC[ndx-1].m_nNumWrd;					// Add words for Books prior to target

		// ===================
		// Chapter of Bible/Testament:
		while (nChapter > g_lstTOC[nBook-1].m_nNumChp) {	// Resolve nBook
			ndxWord += g_lstTOC[nBook-1].m_nNumWrd;			// Add words for books prior to target book
			nChapter -= g_lstTOC[nBook-1].m_nNumChp;
			nBook++;
			if (nBook > g_lstTOC.size()) return CRelIndex();	// Chapter too large (past end of last Book of Bible/Testament)
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
		if (nChapter>g_lstTOC[nBook-1].m_nNumChp) return CRelIndex();		// Chapter too large (past end of book)
		for (unsigned int ndx=1; ndx<nChapter; ++ndx) {
			nVerse += g_mapLayout[CRelIndex(nBook, ndx, 0, 0)].m_nNumVrs;	// Push all chapters prior to target down to nVerse level
		}
		nChapter = 1;	// Reset to beginning of book so nVerse can count from there

		// ===================
		// Verse of Bible/Testament:
		while (nVerse > g_lstTOC[nBook-1].m_nNumVrs) {	// Resolve nBook
			ndxWord += g_lstTOC[nBook-1].m_nNumWrd;		// Add words for books prior to target book
			nVerse -= g_lstTOC[nBook-1].m_nNumVrs;
			nBook++;
			if (nBook > g_lstTOC.size()) return CRelIndex();	// Verse too large (past end of last Book of Bible/Testament)
		}
		// Verse of Book:
		if (nVerse>g_lstTOC[nBook-1].m_nNumVrs) return CRelIndex();		// Verse too large (past end of book)
		while (nVerse > g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumVrs) {		// Resolve nChapter
			nVerse -= g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumVrs;
			nChapter++;
			if (nChapter > g_lstTOC[nBook-1].m_nNumChp) return CRelIndex();	// Verse too large (past end of last Chapter of Book)
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
		if (nVerse>g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumVrs) return CRelIndex();		// Verse too large (past end of Chapter of Book)
		for (unsigned int ndx=1; ndx<nVerse; ++ndx) {
			nWord += (g_lstBooks[nBook-1])[CRelIndex(0, nChapter, ndx, 0)].m_nNumWrd;		// Push all verses prior to target down to nWord level
		}
		nVerse = 1;		// Reset to beginning of chapter so nWord can count from there

		for (unsigned int ndx=1; ndx<nChapter; ++ndx) {
			nWord += g_mapLayout[CRelIndex(nBook, ndx, 0, 0)].m_nNumWrd;	// Push all chapters prior to target down to nWord level
		}
		nChapter = 1;	// Reset to beginning of book so nWord can count from there

		// ===================
		// Word of Bible/Testament:
		while (nWord > g_lstTOC[nBook-1].m_nNumWrd) {		// Resolve nBook
			ndxWord += g_lstTOC[nBook-1].m_nNumWrd;
			nWord -= g_lstTOC[nBook-1].m_nNumWrd;
			nBook++;
			if (nBook > g_lstTOC.size()) return CRelIndex();		// Word too large (past end of last Book of Bible/Testament)
		}
		// Word of Book:
		if (nWord>g_lstTOC[nBook-1].m_nNumWrd) return CRelIndex();	// Word too large (past end of Book/Chapter)
		while (nWord > g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd) {	// Resolve nChapter
			ndxWord += g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd;
			nWord -= g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd;
			nChapter++;
			if (nChapter > g_lstTOC[nBook-1].m_nNumChp) return CRelIndex();		// Word too large (past end of last Verse of last Book/Chapter)
		}
		// Word of Chapter:
		if (nWord>g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd) return CRelIndex();		// Word too large (past end of Book/Chapter)
		const TBookEntryMap &book = g_lstBooks[nBook-1];
		while (nWord > book.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd) {	// Resolve nVerse
			ndxWord += book.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd;
			nWord -= book.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd;
			nVerse++;
			if (nVerse > g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumVrs) return CRelIndex();	// Word too large (past end of last Verse of last Book/Chapter)
		}
		// Word of Verse:
		if (nWord>(g_lstBooks[nBook-1])[CRelIndex(0, nChapter, nVerse, 0)].m_nNumWrd) return CRelIndex();		// Word too large (past end of Verse of Chapter of Book)
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
			for (unsigned int ndx = 0; ndx<g_lstTestaments.size(); ++ndx) {
				ndxWord += g_lstTestaments[ndx].m_nNumWrd;
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

