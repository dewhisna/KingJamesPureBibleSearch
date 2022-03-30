/****************************************************************************
**
** Copyright (C) 2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef GEMATRIA_H
#define GEMATRIA_H

#include "dbDescriptors.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <stdint.h>
#include <algorithm>		// min/max

// Forward Declarations:
class CBibleDatabase;
class CRelIndex;
class TPhraseTag;

// ============================================================================

enum GEMATRIA_BASE_TYPE_ENUM {
	GBTE_STD_ABS_VAL = 0,				// Mispar Hechrachi - 1-9, 10-90, 100-400
	GBTE_STD_ABS_LRG_VAL = 1,			// Mispar Gadol - 1-9, 10-90, 100-900 (final or sofit letter forms assigned 500-900)
	GBTE_ORD_VAL = 2,					// Mispar Siduri - 1-22
	GBTE_Preceding_Sum = 3,				// Sum of preceding values, example: Aleph=1, Bet=1+2=3, Gimel=1+2+3=6
	// ----
	GBTE_COUNT
};

enum GEMATRIA_MATH_TRANSFORM_ENUM {
	GMTE_None = 0,
	GMTE_Square = 1,					// Aleph=1x1, Bet=2x2=4, Gimel=3x3=9, etc.
	GMTE_Cubed = 2,						// Aleph=1x1x1, Bet=2x2x2=8, Gimel=3x3x3=27, etc.
	GMTE_Revua_Square = 3,				// Each letter adds the previous sum plus next letter, example: achad (אחד) 1+(1+8)+(1+8+4)
	// ----
	GMTE_COUNT
};

enum GEMATRIA_LETTER_TRANSFORM_ENUM {
	GLTE_None = 0,
	GLTE_Atbash = 1,					// Aleph<->Tav, Bet<->Shin, etc.
	// ----
	GLTE_COUNT
};

// ----------------------------------------------------------------------------

class CGematriaIndex
{
public:
	CGematriaIndex(const CGematriaIndex &ndx) = default;
	CGematriaIndex(uint32_t ndx = 0)
		:	m_ndx(ndx)
	{ }
	CGematriaIndex(uint32_t nBaseType, uint32_t nMathXform, uint32_t nLtrXform)
	{
		setIndex(nBaseType, nMathXform, nLtrXform);
	}

	inline uint32_t index() const { return m_ndx; }
	inline void setIndex(uint32_t ndx) {
		m_ndx = ndx;
	}
	inline void setIndex(uint32_t nBaseType, uint32_t nMathXform, uint32_t nLtrXform) {
		m_ndx = (((nBaseType & 0xFF) << 24) | ((nMathXform & 0xFF) << 16) | ((nLtrXform & 0xFF) << 8));
	}

	static uint32_t maxBaseTypeCount() { return std::min(static_cast<uint32_t>(0xFF), static_cast<uint32_t>(GBTE_COUNT)); }
	inline GEMATRIA_BASE_TYPE_ENUM baseType() const { return static_cast<GEMATRIA_BASE_TYPE_ENUM>((m_ndx >> 24) & 0xFF); }
	inline void setBaseType(uint32_t nBaseType) {
		m_ndx = ((m_ndx & 0x00FFFFFF) | ((nBaseType & 0xFF) << 24));
	}
	static uint32_t maxMathXformCount() { return std::min(static_cast<uint32_t>(0xFF), static_cast<uint32_t>(GMTE_COUNT)); }
	inline GEMATRIA_MATH_TRANSFORM_ENUM mathXform() const { return static_cast<GEMATRIA_MATH_TRANSFORM_ENUM>((m_ndx >> 16) & 0xFF); }
	inline void setMathXform(uint32_t nMathXform) {
		m_ndx = ((m_ndx & 0xFF00FFFF) | ((nMathXform & 0xFF) << 16));
	}
	static uint32_t maxLetterXformCount() { return std::min(static_cast<uint32_t>(0xFF), static_cast<uint32_t>(GLTE_COUNT)); }
	inline GEMATRIA_LETTER_TRANSFORM_ENUM letterXform() const { return static_cast<GEMATRIA_LETTER_TRANSFORM_ENUM>((m_ndx >> 8) & 0xFF); }
	inline void setLetterXform(uint32_t nLtrXform) {
		m_ndx = ((m_ndx & 0xFFFF00FF) | ((nLtrXform & 0xFF) << 8));
	}

private:
	uint32_t m_ndx;
};

// ----------------------------------------------------------------------------

class CGematriaNames : public QObject
{
	Q_OBJECT
	typedef QStringList TGematriaNameList;

public:
	static TGematriaNameList::size_type count()
	{
		Q_ASSERT(g_arrGematriaBaseTypeNames.size() == GBTE_COUNT);
		Q_ASSERT(g_arrGematriaMathXformNames.size() == GMTE_COUNT);
		Q_ASSERT(g_arrGematriaLetterXformNames.size() == GLTE_COUNT);
		return (GBTE_COUNT * GMTE_COUNT * GLTE_COUNT);
	}
	static QString name(const CGematriaIndex &ndx)
	{
		Q_ASSERT(ndx.baseType() < g_arrGematriaBaseTypeNames.size());
		Q_ASSERT(ndx.mathXform() < g_arrGematriaMathXformNames.size());
		Q_ASSERT(ndx.letterXform() < g_arrGematriaLetterXformNames.size());
		QString strMath = g_arrGematriaMathXformNames.at(ndx.mathXform());
		QString strLtr = g_arrGematriaLetterXformNames.at(ndx.letterXform());
		return QString("%1%2%3")
				.arg(g_arrGematriaBaseTypeNames.at(ndx.baseType()),
					!strMath.isEmpty() ? (" " + strMath) : "",
					!strLtr.isEmpty() ? (QString(" (%1)").arg(strLtr)) : "");
	}

private:
	static const TGematriaNameList g_arrGematriaBaseTypeNames;
	static const TGematriaNameList g_arrGematriaMathXformNames;
	static const TGematriaNameList g_arrGematriaLetterXformNames;
};

// ----------------------------------------------------------------------------

class CGematriaCalc
{
public:
	CGematriaCalc(const CGematriaCalc &src) = default;
	CGematriaCalc(LANGUAGE_ID_ENUM nLangID, const QString &strWord);

	uint32_t value(const CGematriaIndex &ndx) const
	{
		Q_ASSERT(ndx.baseType() < GBTE_COUNT);
		Q_ASSERT(ndx.mathXform() < GMTE_COUNT);
		Q_ASSERT(ndx.letterXform() < GLTE_COUNT);
		return m_arrnValues[ndx.baseType()][ndx.mathXform()][ndx.letterXform()];
	}
	bool skip(const CGematriaIndex &ndx) const				// Returns 'true' if this entry is redundant and should be skipped (such as English GBTE_STD_ABS_LRG_VAL, which is redundant with GBTE_STD_ABS_VAL)
	{
		Q_ASSERT(ndx.baseType() < GBTE_COUNT);
		Q_ASSERT(ndx.mathXform() < GMTE_COUNT);
		Q_ASSERT(ndx.letterXform() < GLTE_COUNT);
		return m_arrbSkip[ndx.baseType()][ndx.mathXform()][ndx.letterXform()];
	}

	static QString tooltip(const CBibleDatabase *pBibleDatabase, const TPhraseTag &tagReference, bool bPlainText);

private:
	uint32_t m_arrnValues[GBTE_COUNT][GMTE_COUNT][GLTE_COUNT] = {};		// Sums
	bool m_arrbSkip[GBTE_COUNT][GMTE_COUNT][GLTE_COUNT] = {};			// Values to skip
};


// ============================================================================

#endif	// GEMATRIA_H

