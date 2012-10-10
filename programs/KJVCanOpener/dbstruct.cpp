// dbstruct.cpp -- Defines structures used for database info
//

#include "dbstruct.h"

// Global Table of Contents:
TTOCList g_lstTOC;

// Global Layout:
TLayoutMap g_mapLayout;

// Global Books:
TBookList g_lstBooks;

// Our one and only master word list:
TWordListMap g_mapWordList;

// ============================================================================

uint32_t MakeIndex(uint32_t nN3, uint32_t nN2, uint32_t nN1, uint32_t nN0)
{
    return (((nN3 & 0xFF) << 24) | ((nN2 & 0xFF) << 16) | ((nN1 & 0xFF) << 8) | (nN0 & 0xFF));
}

// ============================================================================

