//
// Hebrew Letters : Type Definitions
//

#include "HebrewLetters.h"

// ============================================================================

template <typename T, std::size_t N>
constexpr std::size_t __countof(T const (&)[N]) noexcept
{
	return N;
}

namespace {

	const QString g_HebrewLetters =
																	// Name			: Ndx: B3  : B6  : B9
		QString(QChar(0x005D0))+	// QString::fromUtf8("א"),		// Alef			:  0 : 000 : 00 : 00
		QString(QChar(0x005D1))+	// QString::fromUtf8("ב"),		// Bet			:  1 : 001 : 01 : 01
		QString(QChar(0x005D2))+	// QString::fromUtf8("ג"),		// Gimel		:  2 : 002 : 02 : 02
		QString(QChar(0x005D3))+	// QString::fromUtf8("ד"),		// Dalet		:  3 : 010 : 03 : 03
		QString(QChar(0x005D4))+	// QString::fromUtf8("ה"),		// He			:  4 : 011 : 04 : 04
		QString(QChar(0x005D5))+	// QString::fromUtf8("ו"),		// Vav			:  5 : 012 : 05 : 05
		QString(QChar(0x005D6))+	// QString::fromUtf8("ז"),		// Zayin		:  6 : 020 : 10 : 06
		QString(QChar(0x005D7))+	// QString::fromUtf8("ח"),		// Chet			:  7 : 021 : 11 : 07
		QString(QChar(0x005D8))+	// QString::fromUtf8("ט"),		// Tet			:  8 : 022 : 12 : 08
		QString(QChar(0x005D9))+	// QString::fromUtf8("י"),		// Yod			:  9 : 100 : 13 : 10
		QString(QChar(0x005DB))+	// QString::fromUtf8("כ‬"),		// Kaf			: 10 : 101 : 14 : 11
		QString(QChar(0x005DC))+	// QString::fromUtf8("ל"),		// Lamed		: 11 : 102 : 15 : 12
		QString(QChar(0x005DE))+	// QString::fromUtf8("מ"),		// Mem			: 12 : 110 : 20 : 13
		QString(QChar(0x005E0))+	// QString::fromUtf8("נ"),		// Nun			: 13 : 111 : 21 : 14
		QString(QChar(0x005E1))+	// QString::fromUtf8("ס"),		// Samekh		: 14 : 112 : 22 : 15
		QString(QChar(0x005E2))+	// QString::fromUtf8("ע"),		// Ayin			: 15 : 120 : 23 : 16
		QString(QChar(0x005E4))+	// QString::fromUtf8("פ"),		// Pe			: 16 : 121 : 24 : 17
		QString(QChar(0x005E6))+	// QString::fromUtf8("צ"),		// Tsadi		: 17 : 122 : 25 : 18
		QString(QChar(0x005E7))+	// QString::fromUtf8("ק"),		// Qof			: 18 : 200 : 30 : 20
		QString(QChar(0x005E8))+	// QString::fromUtf8("ר‬"),		// Resh			: 19 : 201 : 31 : 21
		QString(QChar(0x005E9))+	// QString::fromUtf8("ש"),		// Shin			: 20 : 202 : 32 : 22
		QString(QChar(0x005EA))+	// QString::fromUtf8("ת"),		// Tav			: 21 : 210 : 33 : 23
		QString(QChar(0x005DA))+	// QString::fromUtf8("ך"),		// Kaf sofit	: 22 : 211 : 34 : 24
		QString(QChar(0x005DD))+	// QString::fromUtf8("ם"),		// Mem sofit	: 23 : 212 : 35 : 25
		QString(QChar(0x005DF))+	// QString::fromUtf8("ן"),		// Nun sofit	: 24 : 220 : 40 : 26
		QString(QChar(0x005E3))+	// QString::fromUtf8("ף"),		// Pe sofit		: 25 : 221 : 41 : 27
		QString(QChar(0x005E5))		// QString::fromUtf8("ץ")		// Tsadi sofit	: 26 : 222 : 42 : 28
	;		// Note: The fromUtf8 versions create multi-QChar entries that don't work!

	const CHebrewLetters::THebrewLetterBase3Pairing g_Base3Mirror[] =
	{
		{ CHebrewLetters::HEBNDX_Alef, true, 0 },		// 000->000 (self 0)
		{ CHebrewLetters::HEBNDX_Yod, false, 0 },		// 001->100			(0) Bet      <-> Yod
		{ CHebrewLetters::HEBNDX_Qof, false, 1 },		// 002->200			(1) Gimel    <-> Qof
		{ CHebrewLetters::HEBNDX_Dalet, true, 1 },		// 010->010 (self 1)
		{ CHebrewLetters::HEBNDX_Mem, false, 2 },		// 011->110			(2) He       <-> Mem
		{ CHebrewLetters::HEBNDX_Tav, false, 3 },		// 012->210			(3) Vav      <-> Tav
		{ CHebrewLetters::HEBNDX_Zayin, true, 2 },		// 020->020 (self 2)
		{ CHebrewLetters::HEBNDX_Ayin, false, 4 },		// 021->120			(4) Chet     <-> Ayin
		{ CHebrewLetters::HEBNDX_NunFinal, false, 5 },	// 022->220			(5) Tet      <-> NunFinal
		{ CHebrewLetters::HEBNDX_Bet, false, 0 },		// 100->001			(0) Yod      <-> Bet
		{ CHebrewLetters::HEBNDX_Kaf, true, 3 },		// 101->101 (self 3)
		{ CHebrewLetters::HEBNDX_Resh, false, 6 },		// 102->201         (6) Lamed    <-> Resh
		{ CHebrewLetters::HEBNDX_He, false, 2 },		// 110->011			(2) Mem      <-> He
		{ CHebrewLetters::HEBNDX_Nun, true, 4 },		// 111->111 (self 4)
		{ CHebrewLetters::HEBNDX_KafFinal, false, 7 },	// 112->211			(7) Samekh   <-> KafFinal
		{ CHebrewLetters::HEBNDX_Chet, false, 4 },		// 120->021			(4) Ayin     <-> Chet
		{ CHebrewLetters::HEBNDX_Pe, true, 5 },			// 121->121 (self 5)
		{ CHebrewLetters::HEBNDX_PeFinal, false, 8 },	// 122->221			(8) Tsadi    <-> PeFinal
		{ CHebrewLetters::HEBNDX_Gimel, false, 1 },		// 200->002			(1) Qof      <-> Gimel
		{ CHebrewLetters::HEBNDX_Lamed, false, 6 },		// 201->102			(6) Resh     <-> Lamed
		{ CHebrewLetters::HEBNDX_Shin, true, 6 },		// 202->202 (self 6)
		{ CHebrewLetters::HEBNDX_Vav, false, 3 },		// 210->012			(3) Tav      <-> Vav
		{ CHebrewLetters::HEBNDX_Samekh, false, 7 },	// 211->112			(7) KafFinal <-> Samekh
		{ CHebrewLetters::HEBNDX_MemFinal, true, 7 },	// 212->212 (self 7)
		{ CHebrewLetters::HEBNDX_Tet, false, 5 },		// 220->022			(5) NunFinal <-> Tet
		{ CHebrewLetters::HEBNDX_Tsadi, false, 8 },		// 221->122			(8) PeFinal  <-> Tsadi
		{ CHebrewLetters::HEBNDX_TsadiFinal, true, 8 },	// 222->222 (self 8)
	};

	static_assert(__countof(g_Base3Mirror) == CHebrewLetters::HEBNDX_COUNT, "Base-3 Array Size is wrong");
}

// ============================================================================

CHebrewLetters::HEB_LTR_NDX CHebrewLetters::indexOfLetter(const QChar &chrLetter)
{
	Q_ASSERT(g_HebrewLetters.size() == HEBNDX_COUNT);
	return static_cast<HEB_LTR_NDX>(g_HebrewLetters.indexOf(chrLetter));
}

QChar CHebrewLetters::letterForIndex(CHebrewLetters::HEB_LTR_NDX nIndex)
{
	Q_ASSERT(g_HebrewLetters.size() == HEBNDX_COUNT);
	int niIndex = static_cast<int>(nIndex);
	if ((niIndex < 0) || (niIndex >= g_HebrewLetters.size())) return QChar();
	return g_HebrewLetters.at(niIndex);
}

const CHebrewLetters::THebrewLetterBase3Pairing &CHebrewLetters::base3pairing(CHebrewLetters::HEB_LTR_NDX nIndex)
{
	static const THebrewLetterBase3Pairing unknownPairing = { HEBNDX_Unknown, false, 0 };
	unsigned int niIndex = static_cast<unsigned int>(nIndex);
	if (niIndex >= __countof(g_Base3Mirror)) return unknownPairing;
	return g_Base3Mirror[niIndex];
}

CHebrewLetters::TLetterPair CHebrewLetters::base3LetterPairFromIndex(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= numberOfLetterPairs)) return TLetterPair(HEBNDX_Unknown, HEBNDX_Unknown);
	for (int ndx = 0; ndx < HEBNDX_COUNT; ++ndx) {
		if (!g_Base3Mirror[ndx].m_bSelf && (g_Base3Mirror[ndx].m_nIndex == nIndex)) {
			return TLetterPair(static_cast<HEB_LTR_NDX>(ndx), g_Base3Mirror[ndx].m_nPairedLetter);
		}
	}
	return TLetterPair(HEBNDX_Unknown, HEBNDX_Unknown);
}

CHebrewLetters::HEB_LTR_NDX CHebrewLetters::base3LetterUnpairedFromIndex(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= numberOfUnpairedLetters)) return HEBNDX_Unknown;
	for (int ndx = 0; ndx < HEBNDX_COUNT; ++ndx) {
		if (g_Base3Mirror[ndx].m_bSelf && (g_Base3Mirror[ndx].m_nIndex == nIndex)) {
			return g_Base3Mirror[ndx].m_nPairedLetter;
		}
	}
	return HEBNDX_Unknown;
}
