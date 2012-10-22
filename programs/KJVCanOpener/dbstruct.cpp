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

/*
TODO : CLEAN

uint32_t MakeIndex(uint32_t nN3, uint32_t nN2, uint32_t nN1, uint32_t nN0)
{
	return (((nN3 & 0xFF) << 24) | ((nN2 & 0xFF) << 16) | ((nN1 & 0xFF) << 8) | (nN0 & 0xFF));
}

uint32_t MakeIndex(const TRelIndex &relIndex)
{
	return MakeIndex(relIndex.m_nN3, relIndex.m_nN2, relIndex.m_nN1, relIndex.m_nN0);
}

TRelIndex DecomposeIndex(uint32_t nIndex)
{
	TRelIndex relIndex;

	relIndex.m_nN3 = ((nIndex >> 24) & 0xFF);
	relIndex.m_nN2 = ((nIndex >> 16) & 0xFF);
	relIndex.m_nN1 = ((nIndex >> 8) & 0xFF);
	relIndex.m_nN0 = (nIndex & 0xFF);

	return relIndex;
}

*/

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

