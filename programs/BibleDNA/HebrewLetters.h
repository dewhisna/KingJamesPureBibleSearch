//
// Hebrew Letters : Type Definitions
//

#ifndef HEBREW_LETTERS_H_
#define HEBREW_LETTERS_H_

#include <QString>

#include <utility>		// std::pair

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
		HEBNDX_Tsadi		= 17,
		HEBNDX_Qof			= 18,
		HEBNDX_Resh			= 19,
		HEBNDX_Shin			= 20,
		HEBNDX_Tav			= 21,
		HEBNDX_KafFinal		= 22,
		HEBNDX_MemFinal		= 23,
		HEBNDX_NunFinal		= 24,
		HEBNDX_PeFinal		= 25,
		HEBNDX_TsadiFinal	= 26,
		HEBNDX_COUNT		= 27
	};

	struct THebrewLetterBase3Pairing {		// Pairing structure for each Hebrew Letter
		HEB_LTR_NDX m_nPairedLetter;		// Letter it's paired with, for example Bet<->Yod
		bool m_bSelf;						// 'True' if it's a letter only paired with itself, like Alef
		int m_nIndex;						// Unique 0-originated Index of the pairing, used for counting pairing statistics.  Each unique pair has a different index, and is repeated for the unpaired letters
	};

	static constexpr int numberOfLetterPairs = 9;			// 18 of the letters form 9 pairs
	static constexpr int numberOfUnpairedLetters = 9;		// 9 letters are only self-paired

	static HEB_LTR_NDX indexOfLetter(const QChar &chrLetter);
	static QChar letterForIndex(HEB_LTR_NDX nIndex);
	static const THebrewLetterBase3Pairing &base3pairing(HEB_LTR_NDX nIndex);
	typedef std::pair<HEB_LTR_NDX, HEB_LTR_NDX> TLetterPair;
	static TLetterPair base3LetterPairFromIndex(int nIndex);
	static HEB_LTR_NDX base3LetterUnpairedFromIndex(int nIndex);
};

// ============================================================================

#endif	// HEBREW_LETTERS_H_
