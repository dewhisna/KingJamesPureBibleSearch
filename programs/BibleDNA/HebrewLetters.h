//
// Hebrew Letters : Type Definitions
//

#ifndef HEBREW_LETTERS_H_
#define HEBREW_LETTERS_H_

#include <QString>

// ============================================================================

class CHebrewLetters
{
public:
	enum HEB_LTR_NDX {
		HEBNDX_Unknown		= -1,
		HEBNDX_Alef			= 0,
		HEBNDX_Bet			= 1,
		HEBNDX_Gimel		= 2,
		HEBNDX_Dalet		= 3,
		HEBNDX_He			= 4,
		HEBNDX_Vav			= 5,
		HEBNDX_Zayin		= 6,
		HEBNDX_Chet			= 7,
		HEBNDX_Tet			= 8,
		HEBNDX_Yod			= 9,
		HEBNDX_Kaf			= 10,
		HEBNDX_Lamed		= 11,
		HEBNDX_Mem			= 12,
		HEBNDX_Nun			= 13,
		HEBNDX_Samekh		= 14,
		HEBNDX_Ayin			= 15,
		HEBNDX_Pe			= 16,
		HEBNDX_Tsade		= 17,
		HEBNDX_Qof			= 18,
		HEBNDX_Resh			= 19,
		HEBNDX_Shin			= 20,
		HEBNDX_Tav			= 21,
		HEBNDX_KafFinal		= 22,
		HEBNDX_MemFinal		= 23,
		HEBNDX_NunFinal		= 24,
		HEBNDX_PeFinal		= 25,
		HEBNDX_TsadeFinal	= 26,
	};

	HEB_LTR_NDX indexOfLetter(const QString &strLetter) const;
	QString letterForIndex(HEB_LTR_NDX nIndex) const;

};

// ============================================================================

#endif	// HEBREW_LETTERS_H_
