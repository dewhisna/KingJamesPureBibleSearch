/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
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

// ============================================================================

// Giant array of all letters from the Bible text for speed
class CLetterMatrix : public QList<QChar>
{
public:
	explicit CLetterMatrix(CBibleDatabasePtr pBibleDatabase,
							bool bSkipColophons, bool bSkipSuperscriptions,
							bool bWordsOfJesusOnly, bool bIncludePrologues);

	uint32_t matrixIndexFromRelIndex(const CRelIndexEx nRelIndexEx) const;
	CRelIndexEx relIndexFromMatrixIndex(uint32_t nMatrixIndex) const;

	const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase.data(); }

	bool skipColophons() const { return m_bSkipColophons; }
	bool skipSuperscriptions() const { return m_bSkipSuperscriptions; }
	bool wordsOfJesusOnly() const { return m_bWordsOfJesusOnly; }
	bool includePrologues() const { return m_bIncludePrologues; }

private:
	CBibleDatabasePtr m_pBibleDatabase;
	bool m_bSkipColophons = false;
	bool m_bSkipSuperscriptions = false;
	bool m_bWordsOfJesusOnly = false;
	bool m_bIncludePrologues = false;

	// Matrix index to letter count shift for normalize/denormalize computations:
	//	When we are skipping colophons and/or superscriptions, the matrix index
	//	of what would be the colophon/superscription is set here with the number
	//	of letters to skip for it.  When transforming to/from matrix index and
	//	database normal/rel index, this map will be scanned for all values of
	//	matrix index less than or equal to the one being transformed and the
	//	corresponding letter count added or subtracted (depending on direction
	//	of the transformation):
	typedef QMap<uint32_t, uint32_t> TMapMatrixIndexToLetterShift;	// MatrixIndex -> LetterCount
	TMapMatrixIndexToLetterShift m_mapMatrixIndexToLetterShift;
};

// ============================================================================

#endif	// LETTER_MATRIX_H

