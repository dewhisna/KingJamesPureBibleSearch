// dbstruct.cpp -- Defines structures used for database info
//

#include "dbstruct.h"

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
	CRefCountCalc Tst(RTE_TESTAMENT, refIndex);
	CRefCountCalc Bk(RTE_BOOK, refIndex);
	CRefCountCalc Chp(RTE_CHAPTER, refIndex);
	CRefCountCalc Vrs(RTE_VERSE, refIndex);
	CRefCountCalc Wrd(RTE_WORD, refIndex);

//	return	QString("%1\n\n").arg(PassageReferenceText(refIndex)) +
//			QString("Book \n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n").arg(Bk.ofBible()).arg(refIndex.testamentName()).arg(Bk.ofTestament()) +
//			QString("Chapter \n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n"
//					"    of Book: %4\n").arg(Chp.ofBible()).arg(refIndex.testamentName()).arg(Chp.ofTestament()).arg(Chp.ofBook()) +
//			QString("Verse \n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n"
//					"    of Book: %4\n"
//					"    of Chapter: %5\n").arg(Vrs.ofBible()).arg(refIndex.testamentName()).arg(Vrs.ofTestament()).arg(Vrs.ofBook()).arg(Vrs.ofChapter()) +
//			QString("Word/Phrase\n"
//					"    of Bible: %1\n"
//					"    of %2: %3\n"
//					"    of Book: %4\n"
//					"    of Chapter: %5\n"
//					"    of Verse: %6\n").arg(Wrd.ofBible()).arg(refIndex.testamentName()).arg(Wrd.ofTestament()).arg(Wrd.ofBook()).arg(Wrd.ofChapter()).arg(Wrd.ofVerse());

	return	QString("%1\n\n").arg(PassageReferenceText(refIndex)) +
			QString("Book \n"
					"    %1 of Bible\n"
					"    %2 of %3\n").arg(Bk.ofBible()).arg(Bk.ofTestament()).arg(refIndex.testamentName()) +
			QString("Chapter \n"
					"    %1 of Bible\n"
					"    %2 of %3\n"
					"    %4 of %5\n").arg(Chp.ofBible()).arg(Chp.ofTestament()).arg(refIndex.testamentName()).arg(Chp.ofBook()).arg(refIndex.bookName()) +
			QString("Verse \n"
					"    %1 of Bible\n"
					"    %2 of %3\n"
					"    %4 of %5\n"
					"    %6 of Chapter\n").arg(Vrs.ofBible()).arg(Vrs.ofTestament()).arg(refIndex.testamentName()).arg(Vrs.ofBook()).arg(refIndex.bookName()).arg(Vrs.ofChapter()) +
			QString("Word/Phrase\n"
					"    %1 of Bible\n"
					"    %2 of %3\n"
					"    %4 of %5\n"
					"    %6 of Chapter\n"
					"    %7 of Verse\n").arg(Wrd.ofBible()).arg(Wrd.ofTestament()).arg(refIndex.testamentName()).arg(Wrd.ofBook()).arg(refIndex.bookName()).arg(Wrd.ofChapter()).arg(Wrd.ofVerse());
}

QString CRefCountCalc::PassageReferenceText(const CRelIndex &refIndex)
{
	return QString("%1 %2:%3 [%4]").arg(refIndex.bookName()).arg(refIndex.chapter()).arg(refIndex.verse()).arg(refIndex.word());
}

// ============================================================================

