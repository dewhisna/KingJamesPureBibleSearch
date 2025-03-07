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

// Warning: Don't change this list or you'll break .els transcript files:
enum LetterMatrixTextModifierOptions {
	LMTMO_None = 0x0,						// Default -- No modifiers (use Bible Database as-is)
	LMTMO_WordsOfJesusOnly = 0x0001,		// Use Words of Jesus Only
	LMTMO_RemoveColophons = 0x0002,			// Strip out colophons from epistles
	LMTMO_RemoveSuperscriptions = 0x0004,	// Strip out superscriptions from Psalms
	LMTMO_IncludeBookPrologues = 0x0008,	// Insert Book Title Prologes (KJV Bibles Only)
	LMTMO_IncludeChapterPrologues = 0x0010,	// Insert Chapter Heading Prologues (KJV Bibles Only)
	LMTMO_IncludeVersePrologues=0x0020,		// Insert Verse Heading Prologues, like verse number (KJV Bibles Only)
	// ----
	LMTMO_ALL = 0x003F,						// Values with all flags set to use as a loop iterator over available flags
};
Q_DECLARE_FLAGS(LetterMatrixTextModifierOptionFlags, LetterMatrixTextModifierOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(LetterMatrixTextModifierOptionFlags)

// ============================================================================

// Giant array of all letters from the Bible text for speed
class CLetterMatrix : public QList<QChar>
{
public:
	explicit CLetterMatrix(CBibleDatabasePtr pBibleDatabase,
						   LetterMatrixTextModifierOptionFlags flagsLMTMO);

	uint32_t matrixIndexFromRelIndex(const CRelIndexEx nRelIndexEx) const;
	CRelIndexEx relIndexFromMatrixIndex(uint32_t nMatrixIndex) const;

	CBibleDatabasePtr bibleDatabase() const { return m_pBibleDatabase; }

	LetterMatrixTextModifierOptionFlags textModifierOptions() const { return m_flagsLMTMO; }

	bool runMatrixIndexRoundtripTest() const;

private:
	CBibleDatabasePtr m_pBibleDatabase;
	LetterMatrixTextModifierOptionFlags m_flagsLMTMO = LMTMO_None;

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
		CRelIndex m_ndxBible;			// RelIndex in the Bible, used to determine if this is a book, chapter, or verse prologue
	};
	typedef QMap<uint32_t, TPrologueEntry> TMapMatrixIndexToPrologue;	// MatrixIndex -> PrologeEntry
	TMapMatrixIndexToPrologue m_mapMatrixIndexToPrologue;				// Note: This is things to add
};

// ============================================================================

#endif	// LETTER_MATRIX_H

