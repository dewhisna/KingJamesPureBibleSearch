// dbstruct.cpp -- Defines structures used for database info
//

#include "dbstruct.h"

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

QString CRelIndex::SearchResultToolTip() const
{
//	CRefCountCalc Tst(CRefCountCalc::RTE_TESTAMENT, *this);
	CRefCountCalc Bk(CRefCountCalc::RTE_BOOK, *this);
	CRefCountCalc Chp(CRefCountCalc::RTE_CHAPTER, *this);
	CRefCountCalc Vrs(CRefCountCalc::RTE_VERSE, *this);
	CRefCountCalc Wrd(CRefCountCalc::RTE_WORD, *this);

//	return	QString("%1\n\n").arg(PassageReferenceText()) +
//			QString("Book \n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n").arg(Bk.ofBible()).arg(testamentName()).arg(Bk.ofTestament()) +
//			QString("Chapter \n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n"
//					"    of Book: %4\n").arg(Chp.ofBible()).arg(testamentName()).arg(Chp.ofTestament()).arg(Chp.ofBook()) +
//			QString("Verse \n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n"
//					"    of Book: %4\n"
//					"    of Chapter: %5\n").arg(Vrs.ofBible()).arg(testamentName()).arg(Vrs.ofTestament()).arg(Vrs.ofBook()).arg(Vrs.ofChapter()) +
//			QString("Word/Phrase\n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n"
//					"    of Book: %4\n"
//					"    of Chapter: %5\n"
//					"    of Verse: %6\n").arg(Wrd.ofBible()).arg(testamentName()).arg(Wrd.ofTestament()).arg(Wrd.ofBook()).arg(Wrd.ofChapter()).arg(Wrd.ofVerse());

	return	QString("%1\n\n").arg(PassageReferenceText()) +
			QString("Book \n"
					"    %1 of Bible\n"
					"    %2 of %3\n").arg(Bk.ofBible()).arg(Bk.ofTestament()).arg(testamentName()) +
			QString("Chapter \n"
					"    %1 of Bible\n"
					"    %2 of %3\n"
					"    %4 of %5\n").arg(Chp.ofBible()).arg(Chp.ofTestament()).arg(testamentName()).arg(Chp.ofBook()).arg(bookName()) +
			QString("Verse \n"
					"    %1 of Bible\n"
					"    %2 of %3\n"
					"    %4 of %5\n"
					"    %6 of Chapter\n").arg(Vrs.ofBible()).arg(Vrs.ofTestament()).arg(testamentName()).arg(Vrs.ofBook()).arg(bookName()).arg(Vrs.ofChapter()) +
			QString("Word/Phrase\n"
					"    %1 of Bible\n"
					"    %2 of %3\n"
					"    %4 of %5\n"
					"    %6 of Chapter\n"
					"    %7 of Verse\n").arg(Wrd.ofBible()).arg(Wrd.ofTestament()).arg(testamentName()).arg(Wrd.ofBook()).arg(bookName()).arg(Wrd.ofChapter()).arg(Wrd.ofVerse());

}

QString CRelIndex::PassageReferenceText() const
{
	if (!isSet()) return "<Invalid Reference>";
	return QString("%1 %2:%3 [%4]").arg(bookName()).arg(chapter()).arg(verse()).arg(word());
}

// ============================================================================

CRefCountCalc::CRefCountCalc(REF_TYPE_ENUM nRefType, const CRelIndex &refIndex)
:	m_ndxRef(refIndex),
	m_nRefType(nRefType),
	m_nOfBible(0),
	m_nOfTst(0),
	m_nOfBk(0),
	m_nOfChp(0),
	m_nOfVrs(0)
{
	switch (nRefType) {
		case RTE_TESTAMENT:				// Calculate the Testament of the Bible
			m_nOfBible = m_ndxRef.testament();
			break;

		case RTE_BOOK:					// Calculate the Book of the Testament and Bible
			m_nOfBible = m_ndxRef.book();
			if (m_ndxRef.book() != 0) {
				const CTOCEntry &toc = g_lstTOC[m_ndxRef.book()-1];
				m_nOfTst = toc.m_nTstBkNdx;
			}
			break;

		case RTE_CHAPTER:				// Calculate the Chapter of the Book, Testament, and Bible
			m_nOfBk = m_ndxRef.chapter();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= g_lstTOC.size())) {
				// Number of Chapters in books prior to target:
				for (unsigned int ndxBk=0; ndxBk<(m_ndxRef.book()-1); ++ndxBk) {
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst += g_lstTOC[ndxBk].m_nNumChp;
					m_nOfBible += g_lstTOC[ndxBk].m_nNumChp;
				}
				// Number of Chapter in target:
				m_nOfTst += m_ndxRef.chapter();
				m_nOfBible += m_ndxRef.chapter();
			}
			break;

		case RTE_VERSE:					// Calculate the Verse of the Chapter, Book, Testament, and Bible
			m_nOfChp = m_ndxRef.verse();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= g_lstTOC.size()) &&
				(m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= g_lstTOC[m_ndxRef.book()-1].m_nNumChp)) {
				// Number of Verses in books prior to target:
				for (unsigned int ndxBk=0; ndxBk<(m_ndxRef.book()-1); ++ndxBk) {
					unsigned int nVerses = g_lstTOC[ndxBk].m_nNumVrs;
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst += nVerses;
					m_nOfBible += nVerses;
				}
				// Number of Verses in Chapters prior to target in target book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nVerses = g_mapLayout[CRelIndex(m_ndxRef.book(),ndxChp,0,0)].m_nNumVrs;
					m_nOfBk += nVerses;
					m_nOfTst += nVerses;
					m_nOfBible += nVerses;
				}
				// Number of Verses in target:
				m_nOfBk += m_ndxRef.verse();
				m_nOfTst += m_ndxRef.verse();
				m_nOfBible += m_ndxRef.verse();
			}
			break;

		case RTE_WORD:					// Calculate the Word of the Verse, Book, Testament, and Bible
			m_nOfVrs = m_ndxRef.word();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= g_lstTOC.size()) &&
				(m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= g_lstTOC[m_ndxRef.book()-1].m_nNumChp) &&
				(m_ndxRef.verse() > 0) && (m_ndxRef.verse() <= g_mapLayout[CRelIndex(m_ndxRef.book(),m_ndxRef.chapter(),0,0)].m_nNumVrs)) {
				// Number of Words in books prior to target:
				for (unsigned int ndxBk=0; ndxBk<(m_ndxRef.book()-1); ++ndxBk) {
					unsigned int nWords = g_lstTOC[ndxBk].m_nNumWrd;
					if (g_lstTOC[ndxBk].m_nTstNdx == m_ndxRef.testament())
						m_nOfTst += nWords;
					m_nOfBible += nWords;
				}
				// Number of Words in Chapters prior to target in target Book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nWords = g_mapLayout[CRelIndex(m_ndxRef.book(),ndxChp,0,0)].m_nNumWrd;
					m_nOfBk += nWords;
					m_nOfTst += nWords;
					m_nOfBible += nWords;
				}
				// Number of Words in Verses prior to target in target Chapter:
				for (unsigned int ndxVrs=1; ndxVrs<m_ndxRef.verse(); ++ndxVrs) {
					unsigned int nWords = (g_lstBooks[m_ndxRef.book()-1])[CRelIndex(0,m_ndxRef.chapter(),ndxVrs,0)].m_nNumWrd;
					m_nOfChp += nWords;
					m_nOfBk += nWords;
					m_nOfTst += nWords;
					m_nOfBible += nWords;
				}
				// Number of Words in target:
				m_nOfChp += m_ndxRef.word();
				m_nOfBk += m_ndxRef.word();
				m_nOfTst += m_ndxRef.word();
				m_nOfBible += m_ndxRef.word();
			}
			break;
	}
}

QString CRefCountCalc::SearchResultToolTip(const CRelIndex &refIndex)
{
	return refIndex.SearchResultToolTip();
}

QString CRefCountCalc::PassageReferenceText(const CRelIndex &refIndex)
{
	return refIndex.PassageReferenceText();
}

CRelIndex CRefCountCalc::calcRelIndex(	unsigned int nWord, unsigned int nVerse,
									unsigned int nChapter, unsigned int nBook, unsigned int nTestament)
{
	uint32_t ndxWord = 0;			// We will calculate target via word, which we can then call Denormalize on

	if (nWord == 0) nWord = 1;			// Assume 1st word of target

	// Testament of Bible:
	if (nTestament) {
		if (nTestament > g_lstTestaments.size()) return CRelIndex();		// Testament too large, past end of Bible
		for (unsigned int ndx=1; ndx<nTestament; ++ndx) {
			// Ripple down to children:
			if (nBook) {
				nBook += g_lstTestaments[ndx-1].m_nNumBk;
			} else if (nChapter) {
				nChapter += g_lstTestaments[ndx-1].m_nNumChp;
			} else if (nVerse) {
				nVerse += g_lstTestaments[ndx-1].m_nNumVrs;
			} else {
				nWord += g_lstTestaments[ndx-1].m_nNumWrd;
			}
		}
	}	// At this point, top specified index will be relative to the Bible, nTestament isn't needed beyond this point

	if (nBook > g_lstTOC.size()) return CRelIndex();
	if (nBook) {
		// Book of Bible/Testament:
		for (unsigned int ndx=1; ndx<nBook; ++ndx)
			ndxWord += g_lstTOC[ndx-1].m_nNumWrd;					// Add words for Books prior to target
	}

	if (nChapter) {
		if (nBook == 0) {
			// Chapter of Bible/Testament:
			while (nChapter > g_lstTOC[nBook].m_nNumChp) {		// Resolve nBook -- NOTE: nBook starts at 0 here!! (no -1!!)
				ndxWord += g_lstTOC[nBook].m_nNumWrd;			// Add words for books prior to target book
				nChapter -= g_lstTOC[nBook].m_nNumChp;
				nBook++;
				if (nBook >= g_lstTOC.size()) return CRelIndex();	// Chapter too large (past end of last Book of Bible/Testament)
			}
			nBook++;
		}
		// Chapter of Book:
		if (nChapter>g_lstTOC[nBook-1].m_nNumChp) return CRelIndex();		// Chapter too large (past end of book)
		for (unsigned int ndx=1; ndx<nChapter; ++ndx) {
			ndxWord += g_mapLayout[CRelIndex(nBook, ndx, 0, 0)].m_nNumWrd;	// Add words for chapters prior to target chapter in target book
		}
	}
	// At this point, if we have nChapter, we will have a definite nBook set, and nChapter will be relative to nBook
	//			However, if nBook is set, we don't necessarily have nChapter, as we could be referencing a verse or word of a book instead of a chapter

	if (nVerse) {
		if (nChapter == 0) {
			// Verse of Bible/Testament/Book:
			if (nBook == 0) {
				// Verse of Bible/Testament:
				while (nVerse > g_lstTOC[nBook].m_nNumVrs) {	// Resolve nBook -- NOTE: nBook starts at 0 here!! (no -1!!)
					ndxWord += g_lstTOC[nBook].m_nNumWrd;		// Add words for books prior to target book
					nVerse -= g_lstTOC[nBook].m_nNumVrs;
					nBook++;
					if (nBook >= g_lstTOC.size()) return CRelIndex();	// Verse too large (past end of last Book of Bible/Testament)
				}
				nBook++;
			}
			// Verse of Book:
			if (nVerse>g_lstTOC[nBook-1].m_nNumVrs) return CRelIndex();		// Verse too large (past end of book)
			nChapter++;
			while (nVerse > g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumVrs) {		// Resolve nChapter -- NOTE: nChapter starts at 1 here!! (no +1!!)
				ndxWord += g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd;			// Add words for chapters prior to target chapter in target book
				nVerse -= g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumVrs;
				nChapter++;
				if (nChapter > g_lstTOC[nBook-1].m_nNumChp) return CRelIndex();	// Verse too large (past end of last Chapter of Book)
			}
		}
		// Verse of Chapter (we already have the book, calculated above):
		assert(nBook > 0);		// We should have nBook set above!
		const CLayoutEntry &bkLayout = g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)];
		if (nVerse>bkLayout.m_nNumVrs) return CRelIndex();		// Verse too large (past end of Chapter of Book)
		for (unsigned int ndx=1; ndx<nVerse; ++ndx) {
			ndxWord += (g_lstBooks[nBook-1])[CRelIndex(0, nChapter, ndx, 0)].m_nNumWrd;		// Add words for verses prior to target verse in target chapter of target book
		}
	}
	// At this point, if we have nVerse, we will have a definite nBook and nChapter set, and nVerse will be relative to nBook/nChapter
	//			However, if either nBook or nChapter is set, we don't necessarily have nVerse, as we could be referencing a word of a book or chapter instead of a verse

	if (nVerse == 0) {
		// Word of Bible/Testament/Book/Chapter:
		if (nChapter == 0) {
			// Verse of Bible/Testament/Book:
			if (nBook == 0) {
				// Word of Bible/Testament:
				while (nWord > g_lstTOC[nBook].m_nNumWrd) {		// Resolve nBook  -- NOTE: nBook starts at 0 here!! (no -1!!)
					ndxWord += g_lstTOC[nBook].m_nNumWrd;
					nWord -= g_lstTOC[nBook].m_nNumWrd;
					nBook++;
					if (nBook >= g_lstTOC.size()) return CRelIndex();		// Word too large (past end of last Book of Bible/Testament)
				}
				nBook++;
			}
			// Word of Book
			if (nWord>g_lstTOC[nBook-1].m_nNumWrd) return CRelIndex();	// Word too large (past end of Book/Chapter)
			nChapter++;
			while (nWord > g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd) {	// Resolve nChapter -- NOTE: nChapter starts at 1 here!! (no +1!!)
				ndxWord += g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd;
				nWord -= g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd;
				nChapter++;
				if (nChapter > g_lstTOC[nBook-1].m_nNumChp) return CRelIndex();		// Word too large (past end of last Verse of last Book/Chapter)
			}
		}
		// Word of Chapter:
		if (nWord>g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumWrd) return CRelIndex();		// Word too large (past end of Book/Chapter)
		const TBookEntryMap &book = g_lstBooks[nBook-1];
		nVerse++;
		while (nWord > book.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd) {	// Resolve nVerse -- NOTE: nVerse starts at 1 here!! (no +1!!)
			ndxWord += book.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd;
			nWord -= book.at(CRelIndex(0, nChapter, nVerse, 0)).m_nNumWrd;
			nVerse++;
			if (nVerse > g_mapLayout[CRelIndex(nBook, nChapter, 0, 0)].m_nNumVrs) return CRelIndex();	// Word too large (past end of last Verse of last Book/Chapter)
		}
	}
	// Word of Verse (we already have the book and chapter calculated above):
	assert(nBook > 0);		// We should have nBook set above!
	assert(nChapter > 0);	// We should have nChapter set above!
	assert(nVerse > 0);		// We should have nVerse set above!
	assert(nWord > 0);		// We should have nWord set above (or defaulted to 1)!
	const CBookEntry &verse = (g_lstBooks[nBook-1])[CRelIndex(0, nChapter, nVerse, 0)];
	if (nWord>verse.m_nNumWrd) return CRelIndex();		// Word too large (past end of Verse of Chapter of Book)
	ndxWord += nWord;		// Add up to include target word

	// At this point, either nBook/nChapter/nVerse/nWord is completely resolved.
	//		As a cross-check, ndxWord should be the Normalized index:
	CRelIndex ndxResult(nBook, nChapter, nVerse, nWord);
	assert(ndxWord == NormalizeIndex((ndxResult)));

	return ndxResult;
}

// ============================================================================

