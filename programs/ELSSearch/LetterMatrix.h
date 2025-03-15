/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef LETTER_MATRIX_H
#define LETTER_MATRIX_H

#include "../KJVCanOpener/dbstruct.h"

#include <QChar>
#include <QList>
#include <QMap>
#include <QFlags>

// ============================================================================

// Warning: Don't change this list order or you'll break .els transcript files:
enum LetterMatrixTextModifierOptions {
	LMTMO_None = 0x0,						// No modifiers (use Bible Database as-is)
	// ----
	LMTMO_WordsOfJesusOnly = 0x0001,		// Use Words of Jesus Only
	LMTMO_RemoveColophons = 0x0002,			// Strip out colophons from epistles
	LMTMO_RemoveSuperscriptions = 0x0004,	// Strip out superscriptions from Psalms
	LMTMO_IncludeBookPrologues = 0x0008,	// Insert Book Title Prologes (KJV Bibles Only)
	LMTMO_IncludeChapterPrologues = 0x0010,	// Insert Chapter Heading Prologues (KJV Bibles Only)
	LMTMO_IncludeVersePrologues = 0x0020,	// Insert Verse Heading Prologues, like verse number (KJV Bibles Only)
	LMTMO_IncludePunctuation = 0x0040,		// Insert Punctuation from Verse Templates
	// ----
	LMTMO_FTextModeMask = 0x0040,			// Mask for Punctuation where the "full text" mode must be used -- Used as a placeholder for if/when additional options are added requiring Full Text Mode
	// ----
	LMTMO_ALL = 0x007F,						// Values with all flags set to use as a loop iterator over available flags
	// ----
	LMTMO_Default = LMTMO_None,				// Default -- No modifiers (use Bible Database as-is)
};
Q_DECLARE_FLAGS(LetterMatrixTextModifierOptionFlags, LetterMatrixTextModifierOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(LetterMatrixTextModifierOptionFlags)

// ============================================================================

// Warning: Don't change this list order or you'll break .els transcript files:
enum LMBookPrologueOptions {
	LMBPO_None = 0x0,						// No Modifier Options
	// ----
	LMBPO_Default = LMBPO_None,				// Default for new format .els files
};
Q_DECLARE_FLAGS(LMBookPrologueOptionFlags, LMBookPrologueOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(LMBookPrologueOptionFlags)

// ============================================================================

enum LMChapterPrologueOptions {
	LMCPO_None = 0x0,							// No Modifier Options
	// ----
	LMCPO_NumbersNone = 0x0,					// No numbers will be output
	LMCPO_NumbersRoman = 0x1,					// Use Roman Numerals
	LMCPO_NumbersArabic = 0x2,					// Use Arabic Numerals
	LMCPO_NumberOptionsMask = 0xF,				// Low Nybble is the Number Options
	// ----
	LMCPO_PsalmBookTags = 0x0010,				// Add the "BOOK" tags to Psalms 1, 42, 73, 90, and 107
	LMCPO_PsalmBookNumbersNone = 0x0000,		// No numbers will be output
	LMCPO_PsalmBookNumbersRoman = 0x0020,		// Use Roman Numerals
	LMCPO_PsalmBookNumbersArabic = 0x0040,		// Use Arabic Numerals
	LMCPO_PsalmBookNumberOptionsMask = 0x01E0,	// Low Nybble is the Number Options
	// ----
	LMCPO_Default = LMCPO_None,					// Default for new format .els files
};
Q_DECLARE_FLAGS(LMChapterPrologueOptionFlags, LMChapterPrologueOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(LMChapterPrologueOptionFlags)

// ============================================================================

enum LMVersePrologueOptions {
	LMVPO_None = 0x0,						// No Modifier Options
	// ----
	LMVPO_NumbersNone = 0x0,				// No numbers will be output
	LMVPO_NumbersRoman = 0x1,				// Use Roman Numerals
	LMVPO_NumbersArabic = 0x2,				// Use Arabic Numerals
	LMVPO_NumberOptionsMask = 0xF,			// Low Nybble is the Number Options
	// ----
	LMVPO_PS119_HebrewLetter = 0x0010,		// Add Hebrew letter on Ps119 Acrostics
	LMVPO_PS119_Transliteration = 0x0020,	// Add Hebrew->English Transliteration on Ps119 Acrostics
	LMVPO_PS119_Punctuation = 0x0040,		// Add period on Ps119 Acrostics Prologues
	// ----
	LMVPO_Default =							// Default for new format .els files
		(LMVPO_PS119_HebrewLetter | LMVPO_PS119_Transliteration | LMVPO_PS119_Punctuation),
};
Q_DECLARE_FLAGS(LMVersePrologueOptionFlags, LMVersePrologueOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(LMVersePrologueOptionFlags)

// ============================================================================

enum LMFullVerseTextOptions {
	LMFVTO_None = 0x0,						// No Modifier Options
	// ----
	LMFVTO_NoBracketsForTransChange = 0x1,	// Remove brackets for Translation Add/Change markup
	// ----
	LMFVTO_Default = LMFVTO_None,			// Default for new format .els files
};
Q_DECLARE_FLAGS(LMFullVerseTextOptionFlags, LMFullVerseTextOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(LMFullVerseTextOptionFlags)

// ============================================================================

// Giant array of all letters from the Bible text for speed
class CLetterMatrix : public QList<QChar>
{
public:
	explicit CLetterMatrix(CBibleDatabasePtr pBibleDatabase,
						   LetterMatrixTextModifierOptionFlags flagsLMTMO,
						   LMBookPrologueOptionFlags flagsLMBPO,
						   LMChapterPrologueOptionFlags flagsLMCPO,
						   LMVersePrologueOptionFlags flagsLMVPO,
						   LMFullVerseTextOptionFlags flagsLMFVTO);

	uint32_t matrixIndexFromRelIndex(const CRelIndexEx nRelIndexEx) const;
	CRelIndexEx relIndexFromMatrixIndex(uint32_t nMatrixIndex) const;

	CBibleDatabasePtr bibleDatabase() const { return m_pBibleDatabase; }

	LetterMatrixTextModifierOptionFlags textModifierOptions() const { return m_flagsLMTMO; }
	LMBookPrologueOptionFlags bookPrologueOptions() const { return m_flagsLMBPO; }
	LMChapterPrologueOptionFlags chapterPrologueOptions() const { return m_flagsLMCPO; }
	LMVersePrologueOptionFlags versePrologueOptions() const { return m_flagsLMVPO; }
	LMFullVerseTextOptionFlags fullVerseTextOptions() const { return m_flagsLMFVTO; }
	QString getOptionDescription(bool bSingleLine) const;
	bool isFTMode() const { return ((m_flagsLMTMO & LMTMO_FTextModeMask) != 0); }

	bool runMatrixIndexRoundtripTest() const;

	uint32_t letterCountForFullVerse(const CRelIndex nRelIndex) const;

private:
	CBibleDatabasePtr m_pBibleDatabase;
	LetterMatrixTextModifierOptionFlags m_flagsLMTMO = LMTMO_Default;
	LMBookPrologueOptionFlags m_flagsLMBPO = LMBPO_Default;
	LMChapterPrologueOptionFlags m_flagsLMCPO = LMCPO_Default;
	LMVersePrologueOptionFlags m_flagsLMVPO = LMVPO_Default;
	LMFullVerseTextOptionFlags m_flagsLMFVTO = LMFVTO_Default;

	// Matrix index to letter count shift for normalize/denormalize computations:
	//	When we are skipping colophons and/or superscriptions, the matrix index
	//	of what would be the colophon/superscription is set here with the number
	//	of letters to skip for it.  When transforming to/from matrix index and
	//	database normal/rel index, this map will be scanned for all values of
	//	matrix index less than or equal to the one being transformed and the
	//	corresponding letter count added or subtracted (depending on direction
	//	of the transformation):
	typedef QMap<uint32_t, uint32_t> TMapMatrixIndexToLetterShift;	// MatrixIndex -> LetterCount
	TMapMatrixIndexToLetterShift m_mapMatrixIndexToLetterShift;		// Note: This is things to remove

	// Matrix index to prologue insertion.  This is a map of the matrix
	//	insertion location of each prologue generated from various
	//	LetterMatrixTextModifierOptionFlags values and allows them to be
	//	independent from the specific Bible Database.
	//	Note: The CRelIndex entry must specify a different prologue type
	//	when there are adjacent prologue entries.  This is used in
	//	matrixIndexFromRelIndex() to decide when to continue processing
	//	vs. exiting when encountering adjacent entries.  Otherwise,
	//	adjacent entries would have to be combined into one.  This is a
	//	bit tacky, but there isn't a much better way to do it, as having
	//	them separate with the CRelIndex makes processing in the
	//	relIndexFromMatrixIndex() function so much easier to deal with.
	struct TPrologueEntry {
		QString m_strPrologue;
		CRelIndex m_ndxPrologue;		// RelIndex for the prologue, used to determine if this is a book, chapter, or verse prologue
	};
	typedef QMap<uint32_t, TPrologueEntry> TMapMatrixIndexToPrologue;	// MatrixIndex -> PrologeEntry
	typedef QMap<CRelIndex, uint32_t> TMapRelIndexToMatrixIndex;		// Map of CRelIndex to Matrix Index for Prologue entry lookup
	TMapMatrixIndexToPrologue m_mapMatrixIndexToPrologue;				// Note: This is things to add
	TMapRelIndexToMatrixIndex m_mapPrologueRelIndexToMatrixIndex;
	// TODO : Rework Prologues above to be done more like FullText logic below...

	// Full text map for use with "include punctuation" mode:
	struct TFullVerseEntry {
		QString m_strVerseText;
		uint32_t m_nMatrixIndex = 0;
	};
	typedef QMap<CRelIndex, TFullVerseEntry> TMapFullVerseText;			// Map of CRelIndex to Full Verse Text Line and Matrix Index
	typedef QMap<uint32_t, CRelIndex> TMapMatrixIndexToRelIndex;		// Map of Matrix Index to CRelIndex of Full Verse Text
	TMapFullVerseText m_mapFullVerseText;
	TMapMatrixIndexToRelIndex m_mapFullTextMatrixIndexToRelIndex;
};

// ============================================================================

#endif	// LETTER_MATRIX_H

