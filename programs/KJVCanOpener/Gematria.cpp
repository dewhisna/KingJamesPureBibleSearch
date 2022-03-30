/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "Gematria.h"

#include "ParseSymbols.h"
#include "dbstruct.h"

#include <vector>
#include <map>

// ============================================================================

namespace {

	typedef std::map<QChar, uint32_t> TGematriaValueMap;									// Map of character to value map
	typedef std::map<GEMATRIA_BASE_TYPE_ENUM, TGematriaValueMap> TGematriaBaseValueMap;		// Map of Gematria Base Types to alphabet map
	typedef std::map<LANGUAGE_ID_ENUM, TGematriaBaseValueMap> TGematriaLanguageValueMap;	// Map of LangID to BaseValueMap

	const TGematriaBaseValueMap g_mapEnglishValues = {
		{ GBTE_STD_ABS_VAL, {
			{ QChar('A'), 1 },
			{ QChar('B'), 2 },
			{ QChar('C'), 3 },
			{ QChar('D'), 4 },
			{ QChar('E'), 5 },
			{ QChar('F'), 6 },
			{ QChar('G'), 7 },
			{ QChar('H'), 8 },
			{ QChar('I'), 9 },
			{ QChar('J'), 10 },
			{ QChar('K'), 20 },
			{ QChar('L'), 30 },
			{ QChar('M'), 40 },
			{ QChar('N'), 50 },
			{ QChar('O'), 60 },
			{ QChar('P'), 70 },
			{ QChar('Q'), 80 },
			{ QChar('R'), 90 },
			{ QChar('S'), 100 },
			{ QChar('T'), 200 },
			{ QChar('U'), 300 },
			{ QChar('V'), 400 },
			{ QChar('W'), 500 },
			{ QChar('X'), 600 },
			{ QChar('Y'), 700 },
			{ QChar('Z'), 800 },
			{ QChar('a'), 1 },
			{ QChar('b'), 2 },
			{ QChar('c'), 3 },
			{ QChar('d'), 4 },
			{ QChar('e'), 5 },
			{ QChar('f'), 6 },
			{ QChar('g'), 7 },
			{ QChar('h'), 8 },
			{ QChar('i'), 9 },
			{ QChar('j'), 10 },
			{ QChar('k'), 20 },
			{ QChar('l'), 30 },
			{ QChar('m'), 40 },
			{ QChar('n'), 50 },
			{ QChar('o'), 60 },
			{ QChar('p'), 70 },
			{ QChar('q'), 80 },
			{ QChar('r'), 90 },
			{ QChar('s'), 100 },
			{ QChar('t'), 200 },
			{ QChar('u'), 300 },
			{ QChar('v'), 400 },
			{ QChar('w'), 500 },
			{ QChar('x'), 600 },
			{ QChar('y'), 700 },
			{ QChar('z'), 800 },
		} },
		{ GBTE_ORD_VAL, {
			{ QChar('A'), 1 },
			{ QChar('B'), 2 },
			{ QChar('C'), 3 },
			{ QChar('D'), 4 },
			{ QChar('E'), 5 },
			{ QChar('F'), 6 },
			{ QChar('G'), 7 },
			{ QChar('H'), 8 },
			{ QChar('I'), 9 },
			{ QChar('J'), 10 },
			{ QChar('K'), 11 },
			{ QChar('L'), 12 },
			{ QChar('M'), 13 },
			{ QChar('N'), 14 },
			{ QChar('O'), 15 },
			{ QChar('P'), 16 },
			{ QChar('Q'), 17 },
			{ QChar('R'), 18 },
			{ QChar('S'), 19 },
			{ QChar('T'), 20 },
			{ QChar('U'), 21 },
			{ QChar('V'), 22 },
			{ QChar('W'), 23 },
			{ QChar('X'), 24 },
			{ QChar('Y'), 25 },
			{ QChar('Z'), 26 },
			{ QChar('a'), 1 },
			{ QChar('b'), 2 },
			{ QChar('c'), 3 },
			{ QChar('d'), 4 },
			{ QChar('e'), 5 },
			{ QChar('f'), 6 },
			{ QChar('g'), 7 },
			{ QChar('h'), 8 },
			{ QChar('i'), 9 },
			{ QChar('j'), 10 },
			{ QChar('k'), 11 },
			{ QChar('l'), 12 },
			{ QChar('m'), 13 },
			{ QChar('n'), 14 },
			{ QChar('o'), 15 },
			{ QChar('p'), 16 },
			{ QChar('q'), 17 },
			{ QChar('r'), 18 },
			{ QChar('s'), 19 },
			{ QChar('t'), 20 },
			{ QChar('u'), 21 },
			{ QChar('v'), 22 },
			{ QChar('w'), 23 },
			{ QChar('x'), 24 },
			{ QChar('y'), 25 },
			{ QChar('z'), 26 },
		} },
	};

	const TGematriaBaseValueMap g_mapHebrewValues = {
		{ GBTE_STD_ABS_VAL, {
			{ QChar(0x05D0), 1 },		// ("א") Alef
			{ QChar(0x05D1), 2 },		// ("ב") Bet
			{ QChar(0x05D2), 3 },		// ("ג") Gimel
			{ QChar(0x05D3), 4 },		// ("ד") Dalet
			{ QChar(0x05D4), 5 },		// ("ה") He
			{ QChar(0x05D5), 6 },		// ("ו") Vav
			{ QChar(0x05D6), 7 },		// ("ז") Zayin
			{ QChar(0x05D7), 8 },		// ("ח") Chet
			{ QChar(0x05D8), 9 },		// ("ט") Tet
			{ QChar(0x05D9), 10 },		// ("י") Yod
			{ QChar(0x05DB), 20 },		// ("כ‬") Kaf
			{ QChar(0x05DC), 30 },		// ("ל") Lamed
			{ QChar(0x05DE), 40 },		// ("מ") Mem
			{ QChar(0x05E0), 50 },		// ("נ") Nun
			{ QChar(0x05E1), 60 },		// ("ס") Samekh
			{ QChar(0x05E2), 70 },		// ("ע") Ayin
			{ QChar(0x05E4), 80 },		// ("פ") Pe
			{ QChar(0x05E6), 90 },		// ("צ") Tsadi
			{ QChar(0x05E7), 100 },		// ("ק") Qof
			{ QChar(0x05E8), 200 },		// ("ר‬") Resh
			{ QChar(0x05E9), 300 },		// ("ש") Shin
			{ QChar(0x05EA), 400 },		// ("ת") Tav
			{ QChar(0x05DA), 20 },		// ("ך") Kaf sofit
			{ QChar(0x05DD), 40 },		// ("ם") Mem sofit
			{ QChar(0x05DF), 50 },		// ("ן") Nun sofit
			{ QChar(0x05E3), 80 },		// ("ף") Pe sofit
			{ QChar(0x05E5), 90 },		// ("ץ") Tsadi sofit
		} },
		{ GBTE_STD_ABS_LRG_VAL, {
			{ QChar(0x05D0), 1 },		// ("א") Alef
			{ QChar(0x05D1), 2 },		// ("ב") Bet
			{ QChar(0x05D2), 3 },		// ("ג") Gimel
			{ QChar(0x05D3), 4 },		// ("ד") Dalet
			{ QChar(0x05D4), 5 },		// ("ה") He
			{ QChar(0x05D5), 6 },		// ("ו") Vav
			{ QChar(0x05D6), 7 },		// ("ז") Zayin
			{ QChar(0x05D7), 8 },		// ("ח") Chet
			{ QChar(0x05D8), 9 },		// ("ט") Tet
			{ QChar(0x05D9), 10 },		// ("י") Yod
			{ QChar(0x05DB), 20 },		// ("כ‬") Kaf
			{ QChar(0x05DC), 30 },		// ("ל") Lamed
			{ QChar(0x05DE), 40 },		// ("מ") Mem
			{ QChar(0x05E0), 50 },		// ("נ") Nun
			{ QChar(0x05E1), 60 },		// ("ס") Samekh
			{ QChar(0x05E2), 70 },		// ("ע") Ayin
			{ QChar(0x05E4), 80 },		// ("פ") Pe
			{ QChar(0x05E6), 90 },		// ("צ") Tsadi
			{ QChar(0x05E7), 100 },		// ("ק") Qof
			{ QChar(0x05E8), 200 },		// ("ר‬") Resh
			{ QChar(0x05E9), 300 },		// ("ש") Shin
			{ QChar(0x05EA), 400 },		// ("ת") Tav
			{ QChar(0x05DA), 500 },		// ("ך") Kaf sofit
			{ QChar(0x05DD), 600 },		// ("ם") Mem sofit
			{ QChar(0x05DF), 700 },		// ("ן") Nun sofit
			{ QChar(0x05E3), 800 },		// ("ף") Pe sofit
			{ QChar(0x05E5), 900 },		// ("ץ") Tsadi sofit
		} },
		{ GBTE_ORD_VAL, {
			{ QChar(0x05D0), 1 },		// ("א") Alef
			{ QChar(0x05D1), 2 },		// ("ב") Bet
			{ QChar(0x05D2), 3 },		// ("ג") Gimel
			{ QChar(0x05D3), 4 },		// ("ד") Dalet
			{ QChar(0x05D4), 5 },		// ("ה") He
			{ QChar(0x05D5), 6 },		// ("ו") Vav
			{ QChar(0x05D6), 7 },		// ("ז") Zayin
			{ QChar(0x05D7), 8 },		// ("ח") Chet
			{ QChar(0x05D8), 9 },		// ("ט") Tet
			{ QChar(0x05D9), 10 },		// ("י") Yod
			{ QChar(0x05DB), 11 },		// ("כ‬") Kaf
			{ QChar(0x05DC), 12 },		// ("ל") Lamed
			{ QChar(0x05DE), 13 },		// ("מ") Mem
			{ QChar(0x05E0), 14 },		// ("נ") Nun
			{ QChar(0x05E1), 15 },		// ("ס") Samekh
			{ QChar(0x05E2), 16 },		// ("ע") Ayin
			{ QChar(0x05E4), 17 },		// ("פ") Pe
			{ QChar(0x05E6), 18 },		// ("צ") Tsadi
			{ QChar(0x05E7), 19 },		// ("ק") Qof
			{ QChar(0x05E8), 20 },		// ("ר‬") Resh
			{ QChar(0x05E9), 21 },		// ("ש") Shin
			{ QChar(0x05EA), 22 },		// ("ת") Tav
			{ QChar(0x05DA), 11 },		// ("ך") Kaf sofit
			{ QChar(0x05DD), 13 },		// ("ם") Mem sofit
			{ QChar(0x05DF), 14 },		// ("ן") Nun sofit
			{ QChar(0x05E3), 17 },		// ("ף") Pe sofit
			{ QChar(0x05E5), 18 },		// ("ץ") Tsadi sofit
		} },
	};

	const TGematriaBaseValueMap g_mapGreekValues = {
		{ GBTE_STD_ABS_VAL, {
			{ QChar(0x0391), 1 },		// ("Α") CAPITAL ALPHA
			{ QChar(0x0392), 2 },		// ("Β") CAPITAL BETA
			{ QChar(0x0393), 3 },		// ("Γ") CAPITAL GAMMA
			{ QChar(0x0394), 4 },		// ("Δ") CAPITAL DELTA
			{ QChar(0x0395), 5 },		// ("Ε") CAPITAL EPSILON
			{ QChar(0x03DA), 6 },		// ("Ϛ") CAPITAL ARCHAIC STIGMA
			{ QChar(0x0396), 7 },		// ("Ζ") CAPITAL ZETA
			{ QChar(0x0397), 8 },		// ("Η") CAPITAL ETA
			{ QChar(0x0398), 9 },		// ("Θ") CAPITAL THETA
			{ QChar(0x0399), 10 },		// ("Ι") CAPITAL IOTA
			{ QChar(0x039A), 20 },		// ("Κ") CAPITAL KAPPA
			{ QChar(0x039B), 30 },		// ("Λ") CAPITAL LAMDA
			{ QChar(0x039C), 40 },		// ("Μ") CAPITAL MU
			{ QChar(0x039D), 50 },		// ("Ν") CAPITAL NU
			{ QChar(0x039E), 60 },		// ("Ξ") CAPITAL XI
			{ QChar(0x039F), 70 },		// ("Ο") CAPITAL OMICRON
			{ QChar(0x03A0), 80 },		// ("Π") CAPITAL PI
			{ QChar(0x03D8), 90 },		// ("Ϙ") CAPITAL ARCHAIC KOPPA
			{ QChar(0x03A1), 100 },		// ("Ρ") CAPITAL RHO
			{ QChar(0x03A3), 200 },		// ("Σ") CAPITAL SIGMA
			{ QChar(0x03A4), 300 },		// ("Τ") CAPITAL TAU
			{ QChar(0x03A5), 400 },		// ("Υ") CAPITAL UPSILON
			{ QChar(0x03A6), 500 },		// ("Φ") CAPITAL PHI
			{ QChar(0x03A7), 600 },		// ("Χ") CAPITAL CHI
			{ QChar(0x03A8), 700 },		// ("Ψ") CAPITAL PSI
			{ QChar(0x03A9), 800 },		// ("Ω") CAPITAL OMEGA
			{ QChar(0x03B1), 1 },		// ("α") SMALL ALPHA
			{ QChar(0x03B2), 2 },		// ("β") SMALL BETA
			{ QChar(0x03B3), 3 },		// ("γ") SMALL GAMMA
			{ QChar(0x03B4), 4 },		// ("δ") SMALL DELTA
			{ QChar(0x03B5), 5 },		// ("ε") SMALL EPSILON
			{ QChar(0x03DB), 6 },		// ("ϛ") SMALL ARCHAIC STIGMA
			{ QChar(0x03B6), 7 },		// ("ζ") SMALL ZETA
			{ QChar(0x03B7), 8 },		// ("η") SMALL ETA
			{ QChar(0x03B8), 9 },		// ("θ") SMALL THETA
			{ QChar(0x03B9), 10 },		// ("ι") SMALL IOTA
			{ QChar(0x03BA), 20 },		// ("κ") SMALL KAPPA
			{ QChar(0x03BB), 30 },		// ("λ") SMALL LAMDA
			{ QChar(0x03BC), 40 },		// ("μ") SMALL MU
			{ QChar(0x03BD), 50 },		// ("ν") SMALL NU
			{ QChar(0x03BE), 60 },		// ("ξ") SMALL XI
			{ QChar(0x03BF), 70 },		// ("ο") SMALL OMICRON
			{ QChar(0x03C0), 80 },		// ("π") SMALL PI
			{ QChar(0x03D9), 90 },		// ("ϙ") SMALL ARCHAIC KOPPA
			{ QChar(0x03C1), 100 },		// ("ρ") SMALL RHO
			{ QChar(0x03C2), 200 },		// ("ς") SMALL FINAL SIGMA
			{ QChar(0x03C3), 200 },		// ("σ") SMALL SIGMA
			{ QChar(0x03C4), 300 },		// ("τ") SMALL TAU
			{ QChar(0x03C5), 400 },		// ("υ") SMALL UPSILON
			{ QChar(0x03C6), 500 },		// ("φ") SMALL PHI
			{ QChar(0x03C7), 600 },		// ("χ") SMALL CHI
			{ QChar(0x03C8), 700 },		// ("ψ") SMALL PSI
			{ QChar(0x03C9), 800 },		// ("ω") SMALL OMEGA
		} },
		{ GBTE_STD_ABS_VAL, {
			{ QChar(0x0391), 1 },		// ("Α") CAPITAL ALPHA
			{ QChar(0x0392), 2 },		// ("Β") CAPITAL BETA
			{ QChar(0x0393), 3 },		// ("Γ") CAPITAL GAMMA
			{ QChar(0x0394), 4 },		// ("Δ") CAPITAL DELTA
			{ QChar(0x0395), 5 },		// ("Ε") CAPITAL EPSILON
			{ QChar(0x0396), 6 },		// ("Ζ") CAPITAL ZETA
			{ QChar(0x0397), 7 },		// ("Η") CAPITAL ETA
			{ QChar(0x0398), 8 },		// ("Θ") CAPITAL THETA
			{ QChar(0x0399), 9 },		// ("Ι") CAPITAL IOTA
			{ QChar(0x039A), 10 },		// ("Κ") CAPITAL KAPPA
			{ QChar(0x039B), 11 },		// ("Λ") CAPITAL LAMDA
			{ QChar(0x039C), 12 },		// ("Μ") CAPITAL MU
			{ QChar(0x039D), 13 },		// ("Ν") CAPITAL NU
			{ QChar(0x039E), 14 },		// ("Ξ") CAPITAL XI
			{ QChar(0x039F), 15 },		// ("Ο") CAPITAL OMICRON
			{ QChar(0x03A0), 16 },		// ("Π") CAPITAL PI
			{ QChar(0x03A1), 17 },		// ("Ρ") CAPITAL RHO
			{ QChar(0x03A3), 18 },		// ("Σ") CAPITAL SIGMA
			{ QChar(0x03A4), 19 },		// ("Τ") CAPITAL TAU
			{ QChar(0x03A5), 20 },		// ("Υ") CAPITAL UPSILON
			{ QChar(0x03A6), 21 },		// ("Φ") CAPITAL PHI
			{ QChar(0x03A7), 22 },		// ("Χ") CAPITAL CHI
			{ QChar(0x03A8), 23 },		// ("Ψ") CAPITAL PSI
			{ QChar(0x03A9), 24 },		// ("Ω") CAPITAL OMEGA
			{ QChar(0x03B1), 1 },		// ("α") SMALL ALPHA
			{ QChar(0x03B2), 2 },		// ("β") SMALL BETA
			{ QChar(0x03B3), 3 },		// ("γ") SMALL GAMMA
			{ QChar(0x03B4), 4 },		// ("δ") SMALL DELTA
			{ QChar(0x03B5), 5 },		// ("ε") SMALL EPSILON
			{ QChar(0x03B6), 6 },		// ("ζ") SMALL ZETA
			{ QChar(0x03B7), 7 },		// ("η") SMALL ETA
			{ QChar(0x03B8), 8 },		// ("θ") SMALL THETA
			{ QChar(0x03B9), 9 },		// ("ι") SMALL IOTA
			{ QChar(0x03BA), 10 },		// ("κ") SMALL KAPPA
			{ QChar(0x03BB), 11 },		// ("λ") SMALL LAMDA
			{ QChar(0x03BC), 12 },		// ("μ") SMALL MU
			{ QChar(0x03BD), 13 },		// ("ν") SMALL NU
			{ QChar(0x03BE), 14 },		// ("ξ") SMALL XI
			{ QChar(0x03BF), 15 },		// ("ο") SMALL OMICRON
			{ QChar(0x03C0), 16 },		// ("π") SMALL PI
			{ QChar(0x03C1), 17 },		// ("ρ") SMALL RHO
			{ QChar(0x03C2), 18 },		// ("ς") SMALL FINAL SIGMA
			{ QChar(0x03C3), 18 },		// ("σ") SMALL SIGMA
			{ QChar(0x03C4), 19 },		// ("τ") SMALL TAU
			{ QChar(0x03C5), 20 },		// ("υ") SMALL UPSILON
			{ QChar(0x03C6), 21 },		// ("φ") SMALL PHI
			{ QChar(0x03C7), 22 },		// ("χ") SMALL CHI
			{ QChar(0x03C8), 23 },		// ("ψ") SMALL PSI
			{ QChar(0x03C9), 24 },		// ("ω") SMALL OMEGA
		} },
	};

	const TGematriaLanguageValueMap g_mapGematriaValues = {
		{ LIDE_ENGLISH, g_mapEnglishValues },
		{ LIDE_FRENCH, g_mapEnglishValues },		// French, Spanish, and German will use same
		{ LIDE_SPANISH, g_mapEnglishValues },		//	values as English after accent decomposition
		{ LIDE_GERMAN, g_mapEnglishValues },
		{ LIDE_HEBREW, g_mapHebrewValues },
		{ LIDE_GREEK, g_mapGreekValues },
		// TODO : LIDE_RUSSIAN
	};

	// ------------------------------------------------------------------------

	typedef std::map<QChar, QChar> TGematriaLetterMap;					// Letter transformation mapping
	typedef std::map<GEMATRIA_LETTER_TRANSFORM_ENUM, TGematriaLetterMap> TGematriaLetterXformMap;	// Transform to letter mapping map

	const TGematriaLetterXformMap g_mapGematriaLetterXforms = {
		{ GLTE_Atbash, {
			{ QChar('A'), QChar('z') },
			{ QChar('B'), QChar('y') },
			{ QChar('C'), QChar('x') },
			{ QChar('D'), QChar('w') },
			{ QChar('E'), QChar('v') },
			{ QChar('F'), QChar('u') },
			{ QChar('G'), QChar('t') },
			{ QChar('H'), QChar('s') },
			{ QChar('I'), QChar('r') },
			{ QChar('J'), QChar('q') },
			{ QChar('K'), QChar('p') },
			{ QChar('L'), QChar('o') },
			{ QChar('M'), QChar('n') },
			{ QChar('N'), QChar('m') },
			{ QChar('O'), QChar('l') },
			{ QChar('P'), QChar('k') },
			{ QChar('Q'), QChar('j') },
			{ QChar('R'), QChar('i') },
			{ QChar('S'), QChar('h') },
			{ QChar('T'), QChar('g') },
			{ QChar('U'), QChar('f') },
			{ QChar('V'), QChar('e') },
			{ QChar('W'), QChar('d') },
			{ QChar('X'), QChar('c') },
			{ QChar('Y'), QChar('b') },
			{ QChar('Z'), QChar('a') },
			{ QChar('a'), QChar('Z') },
			{ QChar('b'), QChar('Y') },
			{ QChar('c'), QChar('X') },
			{ QChar('d'), QChar('W') },
			{ QChar('e'), QChar('V') },
			{ QChar('f'), QChar('U') },
			{ QChar('g'), QChar('T') },
			{ QChar('h'), QChar('S') },
			{ QChar('i'), QChar('R') },
			{ QChar('j'), QChar('Q') },
			{ QChar('k'), QChar('P') },
			{ QChar('l'), QChar('O') },
			{ QChar('m'), QChar('N') },
			{ QChar('n'), QChar('M') },
			{ QChar('o'), QChar('L') },
			{ QChar('p'), QChar('K') },
			{ QChar('q'), QChar('J') },
			{ QChar('r'), QChar('I') },
			{ QChar('s'), QChar('H') },
			{ QChar('t'), QChar('G') },
			{ QChar('u'), QChar('F') },
			{ QChar('v'), QChar('E') },
			{ QChar('w'), QChar('D') },
			{ QChar('x'), QChar('C') },
			{ QChar('y'), QChar('B') },
			{ QChar('z'), QChar('A') },
			// ----
			{ QChar(0x05D0), QChar(0x05EA) },		// ("א") Alef -> ("ת") Tav
			{ QChar(0x05D1), QChar(0x05E9) },		// ("ב") Bet -> ("ש") Shin
			{ QChar(0x05D2), QChar(0x05E8) },		// ("ג") Gimel -> ("ר‬") Resh
			{ QChar(0x05D3), QChar(0x05E7) },		// ("ד") Dalet -> ("ק") Qof
			{ QChar(0x05D4), QChar(0x05E6) },		// ("ה") He -> ("צ") Tsadi
			{ QChar(0x05D5), QChar(0x05E4) },		// ("ו") Vav -> ("פ") Pe
			{ QChar(0x05D6), QChar(0x05E2) },		// ("ז") Zayin -> ("ע") Ayin
			{ QChar(0x05D7), QChar(0x05E1) },		// ("ח") Chet -> ("ס") Samekh
			{ QChar(0x05D8), QChar(0x05E0) },		// ("ט") Tet -> ("נ") Nun
			{ QChar(0x05D9), QChar(0x05DE) },		// ("י") Yod -> ("מ") Mem
			{ QChar(0x05DB), QChar(0x05DC) },		// ("כ‬") Kaf -> ("ל") Lamed
			{ QChar(0x05DC), QChar(0x05DB) },		// ("ל") Lamed -> ("כ‬") Kaf
			{ QChar(0x05DE), QChar(0x05D9) },		// ("מ") Mem -> ("י") Yod
			{ QChar(0x05E0), QChar(0x05D8) },		// ("נ") Nun -> ("ט") Tet
			{ QChar(0x05E1), QChar(0x05D7) },		// ("ס") Samekh -> ("ח") Chet
			{ QChar(0x05E2), QChar(0x05D6) },		// ("ע") Ayin -> ("ז") Zayin
			{ QChar(0x05E4), QChar(0x05D5) },		// ("פ") Pe -> ("ו") Vav
			{ QChar(0x05E6), QChar(0x05D4) },		// ("צ") Tsadi -> ("ה") He
			{ QChar(0x05E7), QChar(0x05D3) },		// ("ק") Qof -> ("ד") Dalet
			{ QChar(0x05E8), QChar(0x05D2) },		// ("ר‬") Resh -> ("ג") Gimel
			{ QChar(0x05E9), QChar(0x05D1) },		// ("ש") Shin -> ("ב") Bet
			{ QChar(0x05EA), QChar(0x05D0) },		// ("ת") Tav -> ("א") Alef
			{ QChar(0x05DA), QChar(0x05DC) },		// ("ך") Kaf sofit -> ("ל") Lamed
			{ QChar(0x05DD), QChar(0x05D9) },		// ("ם") Mem sofit -> ("י") Yod
			{ QChar(0x05DF), QChar(0x05D8) },		// ("ן") Nun sofit -> ("ט") Tet
			{ QChar(0x05E3), QChar(0x05D5) },		// ("ף") Pe sofit -> ("ו") Vav
			{ QChar(0x05E5), QChar(0x05D4) },		// ("ץ") Tsadi sofit -> ("ה") He
			// ----
			{ QChar(0x0391), QChar(0x03C9) },		// ("Α") CAPITAL ALPHA -> ("ω") SMALL OMEGA
			{ QChar(0x0392), QChar(0x03C8) },		// ("Β") CAPITAL BETA -> ("ψ") SMALL PSI
			{ QChar(0x0393), QChar(0x03C7) },		// ("Γ") CAPITAL GAMMA -> ("χ") SMALL CHI
			{ QChar(0x0394), QChar(0x03C6) },		// ("Δ") CAPITAL DELTA -> ("φ") SMALL PHI
			{ QChar(0x0395), QChar(0x03C5) },		// ("Ε") CAPITAL EPSILON -> ("υ") SMALL UPSILON
			{ QChar(0x0396), QChar(0x03C4) },		// ("Ζ") CAPITAL ZETA -> ("τ") SMALL TAU
			{ QChar(0x0397), QChar(0x03C3) },		// ("Η") CAPITAL ETA -> ("σ") SMALL SIGMA
			{ QChar(0x0398), QChar(0x03C1) },		// ("Θ") CAPITAL THETA -> ("ρ") SMALL RHO
			{ QChar(0x0399), QChar(0x03C0) },		// ("Ι") CAPITAL IOTA -> ("π") SMALL PI
			{ QChar(0x039A), QChar(0x03BF) },		// ("Κ") CAPITAL KAPPA -> ("ο") SMALL OMICRON
			{ QChar(0x039B), QChar(0x03BE) },		// ("Λ") CAPITAL LAMDA -> ("ξ") SMALL XI
			{ QChar(0x039C), QChar(0x03BD) },		// ("Μ") CAPITAL MU -> ("ν") SMALL NU
			{ QChar(0x039D), QChar(0x03BC) },		// ("Ν") CAPITAL NU -> ("μ") SMALL MU
			{ QChar(0x039E), QChar(0x03BB) },		// ("Ξ") CAPITAL XI -> ("λ") SMALL LAMDA
			{ QChar(0x039F), QChar(0x03BA) },		// ("Ο") CAPITAL OMICRON -> ("κ") SMALL KAPPA
			{ QChar(0x03A0), QChar(0x03B9) },		// ("Π") CAPITAL PI -> ("ι") SMALL IOTA
			{ QChar(0x03A1), QChar(0x03B8) },		// ("Ρ") CAPITAL RHO -> ("θ") SMALL THETA
			{ QChar(0x03A3), QChar(0x03B7) },		// ("Σ") CAPITAL SIGMA -> ("η") SMALL ETA
			{ QChar(0x03A4), QChar(0x03B6) },		// ("Τ") CAPITAL TAU -> ("ζ") SMALL ZETA
			{ QChar(0x03A5), QChar(0x03B5) },		// ("Υ") CAPITAL UPSILON -> ("ε") SMALL EPSILON
			{ QChar(0x03A6), QChar(0x03B4) },		// ("Φ") CAPITAL PHI -> ("δ") SMALL DELTA
			{ QChar(0x03A7), QChar(0x03B3) },		// ("Χ") CAPITAL CHI -> ("γ") SMALL GAMMA
			{ QChar(0x03A8), QChar(0x03B2) },		// ("Ψ") CAPITAL PSI -> ("β") SMALL BETA
			{ QChar(0x03A9), QChar(0x03B1) },		// ("Ω") CAPITAL OMEGA -> ("α") SMALL ALPHA
			{ QChar(0x03B1), QChar(0x03A9) },		// ("α") SMALL ALPHA -> ("Ω") CAPITAL OMEGA
			{ QChar(0x03B2), QChar(0x03A8) },		// ("β") SMALL BETA -> ("Ψ") CAPITAL PSI
			{ QChar(0x03B3), QChar(0x03A7) },		// ("γ") SMALL GAMMA -> ("Χ") CAPITAL CHI
			{ QChar(0x03B4), QChar(0x03A6) },		// ("δ") SMALL DELTA -> ("Φ") CAPITAL PHI
			{ QChar(0x03B5), QChar(0x03A5) },		// ("ε") SMALL EPSILON -> ("Υ") CAPITAL UPSILON
			{ QChar(0x03B6), QChar(0x03A4) },		// ("ζ") SMALL ZETA -> ("Τ") CAPITAL TAU
			{ QChar(0x03B7), QChar(0x03A3) },		// ("η") SMALL ETA -> ("Σ") CAPITAL SIGMA
			{ QChar(0x03B8), QChar(0x03A1) },		// ("θ") SMALL THETA -> ("Ρ") CAPITAL RHO
			{ QChar(0x03B9), QChar(0x03A0) },		// ("ι") SMALL IOTA -> ("Π") CAPITAL PI
			{ QChar(0x03BA), QChar(0x039F) },		// ("κ") SMALL KAPPA -> ("Ο") CAPITAL OMICRON
			{ QChar(0x03BB), QChar(0x039E) },		// ("λ") SMALL LAMDA -> ("Ξ") CAPITAL XI
			{ QChar(0x03BC), QChar(0x039D) },		// ("μ") SMALL MU -> ("Ν") CAPITAL NU
			{ QChar(0x03BD), QChar(0x039C) },		// ("ν") SMALL NU -> ("Μ") CAPITAL MU
			{ QChar(0x03BE), QChar(0x039B) },		// ("ξ") SMALL XI -> ("Λ") CAPITAL LAMDA
			{ QChar(0x03BF), QChar(0x039A) },		// ("ο") SMALL OMICRON -> ("Κ") CAPITAL KAPPA
			{ QChar(0x03C0), QChar(0x0399) },		// ("π") SMALL PI -> ("Ι") CAPITAL IOTA
			{ QChar(0x03C1), QChar(0x0398) },		// ("ρ") SMALL RHO -> ("Θ") CAPITAL THETA
			{ QChar(0x03C2), QChar(0x0397) },		// ("ς") SMALL FINAL SIGMA -> ("Η") CAPITAL ETA
			{ QChar(0x03C3), QChar(0x0397) },		// ("σ") SMALL SIGMA -> ("Η") CAPITAL ETA
			{ QChar(0x03C4), QChar(0x0396) },		// ("τ") SMALL TAU -> ("Ζ") CAPITAL ZETA
			{ QChar(0x03C5), QChar(0x0395) },		// ("υ") SMALL UPSILON -> ("Ε") CAPITAL EPSILON
			{ QChar(0x03C6), QChar(0x0394) },		// ("φ") SMALL PHI -> ("Δ") CAPITAL DELTA
			{ QChar(0x03C7), QChar(0x0393) },		// ("χ") SMALL CHI -> ("Γ") CAPITAL GAMMA
			{ QChar(0x03C8), QChar(0x0392) },		// ("ψ") SMALL PSI -> ("Β") CAPITAL BETA
			{ QChar(0x03C9), QChar(0x0391) },		// ("ω") SMALL OMEGA -> ("Α") CAPITAL ALPHA
		} },
	};

	// ------------------------------------------------------------------------

	struct TGematriaResult {
		uint64_t m_nValue = 0;
		bool m_bValid = true;
	};

	typedef TGematriaResult (*FMathXlate)(TGematriaResult nCurrentValue, uint32_t nCharValue);
	typedef std::map<GEMATRIA_MATH_TRANSFORM_ENUM, FMathXlate> TGematriaMathXformMap;

	const TGematriaMathXformMap g_mapGematriaMathXforms = {
		{ GMTE_None, [](TGematriaResult nCurrentValue, uint32_t nCharValue)->TGematriaResult
			{
				if (!nCurrentValue.m_bValid) return nCurrentValue;		// Once invalid, quick exit
				nCurrentValue.m_nValue += nCharValue;
				return nCurrentValue;
			}
		},

		{ GMTE_Square, [](TGematriaResult nCurrentValue, uint32_t nCharValue)->TGematriaResult
			{
				if (!nCurrentValue.m_bValid) return nCurrentValue;		// Once invalid, quick exit
				nCurrentValue.m_nValue += nCharValue*nCharValue;
				return nCurrentValue;
			}
		},

		{ GMTE_Cubed, [](TGematriaResult nCurrentValue, uint32_t nCharValue)->TGematriaResult
			{
				if (!nCurrentValue.m_bValid) return nCurrentValue;		// Once invalid, quick exit
				nCurrentValue.m_nValue += nCharValue*nCharValue*nCharValue;
				return nCurrentValue;
			}
		},

		{ GMTE_Revua_Square, [](TGematriaResult nCurrentValue, uint32_t nCharValue)->TGematriaResult
			{
				if (!nCurrentValue.m_bValid) return nCurrentValue;		// Once invalid, quick exit
				nCurrentValue.m_nValue += nCurrentValue.m_nValue + nCharValue;
				return nCurrentValue;
			}
		},
	};

	// ------------------------------------------------------------------------

	static TGematriaResult calculateNextGematria(const TGematriaBaseValueMap::const_iterator &itrBaseValue,
												const TGematriaMathXformMap::const_iterator &itrMathXform,
												GEMATRIA_LETTER_TRANSFORM_ENUM nLtrXform,
												TGematriaResult nCurrentValue,
												QChar chrNext)
	{
		if (!nCurrentValue.m_bValid) return nCurrentValue;		// Once invalid, quick exit

		// Transform the letter:
		TGematriaLetterXformMap::const_iterator itrLetterXform = g_mapGematriaLetterXforms.find(nLtrXform);
		if (itrLetterXform != g_mapGematriaLetterXforms.cend()) {
			TGematriaLetterMap::const_iterator itrChar = itrLetterXform->second.find(chrNext);
			if (itrChar != itrLetterXform->second.cend()) chrNext = itrChar->second;
		}	// Note: Letter transform is completely optional... ignore if none found...

		// Find the letter value:
		TGematriaValueMap::const_iterator itrChrValue = itrBaseValue->second.find(chrNext);
		if (itrChrValue != itrBaseValue->second.cend()) {
			// We have a character value, do math transform with it:
			nCurrentValue = itrMathXform->second(nCurrentValue, itrChrValue->second);
		}	// Note: If the character isn't found in the list, fall through and return current value as valid
		return nCurrentValue;
	}

};

// ============================================================================

const CGematriaNames::TGematriaNameList CGematriaNames::g_arrGematriaBaseTypeNames =
{
	CGematriaNames::tr("Standard Abs Val"),
	CGematriaNames::tr("Standard Abs Lrg Val"),
	CGematriaNames::tr("Ordinal Val"),
	CGematriaNames::tr("Preceding Sum"),
};

const CGematriaNames::TGematriaNameList CGematriaNames::g_arrGematriaMathXformNames =
{
	QString(),
	CGematriaNames::tr("Squared"),
	CGematriaNames::tr("Cubed"),
	CGematriaNames::tr("Revua Square"),
};

const CGematriaNames::TGematriaNameList CGematriaNames::g_arrGematriaLetterXformNames =
{
	QString(),
	CGematriaNames::tr("Atbash"),
};

// ============================================================================

CGematriaCalc::CGematriaCalc(LANGUAGE_ID_ENUM nLangID, const QString &strWord)
{
	TGematriaLanguageValueMap::const_iterator itrLang = g_mapGematriaValues.find(nLangID);
	if (itrLang != g_mapGematriaValues.cend()) {
		for (uint32_t nBaseType = 0; nBaseType < GBTE_COUNT; ++nBaseType) {
			TGematriaBaseValueMap::const_iterator itrBaseValue = itrLang->second.find(static_cast<GEMATRIA_BASE_TYPE_ENUM>(nBaseType));
			if (itrBaseValue != itrLang->second.cend()) {
				for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
					TGematriaMathXformMap::const_iterator itrMathXform = g_mapGematriaMathXforms.find(static_cast<GEMATRIA_MATH_TRANSFORM_ENUM>(nMathXform));
					if (itrMathXform != g_mapGematriaMathXforms.cend()) {
						for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
							TGematriaResult result;

							QString strDecomposedWord = StringParse::decompose(strWord, false);
							for (QString::size_type ndxChar = 0; ndxChar < strDecomposedWord.size(); ++ndxChar) {
								result = calculateNextGematria(itrBaseValue,
																itrMathXform,
																static_cast<GEMATRIA_LETTER_TRANSFORM_ENUM>(nLtrXform),
																result,
																strDecomposedWord.at(ndxChar));
							}

							if (!result.m_bValid) result.m_nValue = 0;
							m_arrnValues[nBaseType][nMathXform][nLtrXform] = result.m_nValue;
							m_arrbSkip[nBaseType][nMathXform][nLtrXform] = !result.m_bValid;
						}
					} else {
						// If we don't have a math transform for it, we can't compute it:
						for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
							m_arrnValues[nBaseType][nMathXform][nLtrXform] = 0;
							m_arrbSkip[nBaseType][nMathXform][nLtrXform] = true;
						}
					}
				}
			} else {
				// If we don't have a gematria values, we can't compute it:
				for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
					for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
						m_arrnValues[nBaseType][nMathXform][nLtrXform] = 0;
						m_arrbSkip[nBaseType][nMathXform][nLtrXform] = true;
					}
				}
			}
		}
	} else {
		// If we don't have a map for the language, we have no gematria for it:
		for (uint32_t nBaseType = 0; nBaseType < GBTE_COUNT; ++nBaseType) {
			for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
				for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
					m_arrnValues[nBaseType][nMathXform][nLtrXform] = 0;
					m_arrbSkip[nBaseType][nMathXform][nLtrXform] = true;
				}
			}
		}
	}
}

// ============================================================================

QString CGematriaCalc::tooltip(const CBibleDatabase *pBibleDatabase, const TPhraseTag &tagReference, bool bPlainText)
{
	Q_ASSERT(pBibleDatabase != nullptr);

	if (!tagReference.isSet()) return QString();

	QString strToolTip;
	CRelIndex ndxStart = tagReference.relIndex();
	unsigned int nCount = tagReference.count();
	if (nCount == 0) {
		CRefCountCalc refCalc(pBibleDatabase, CRefCountCalc::RTE_WORD, ndxStart);

		if (ndxStart.word()) {
			nCount = 1;			// Single word
		} else if (ndxStart.verse()) {
			nCount = refCalc.ofVerse().second;
		} else if (ndxStart.chapter()) {
			ndxStart = pBibleDatabase->calcRelIndex(ndxStart, CBibleDatabase::RIME_StartOfChapter);
			Q_ASSERT(pBibleDatabase->chapterEntry(ndxStart) != nullptr);
			if (pBibleDatabase->chapterEntry(ndxStart)->m_bHaveSuperscription) ndxStart.setVerse(0);
			refCalc = CRefCountCalc(pBibleDatabase, CRefCountCalc::RTE_WORD, ndxStart);
			nCount = refCalc.ofChapter().second;
		} else if (ndxStart.book()) {
			ndxStart = pBibleDatabase->calcRelIndex(ndxStart, CBibleDatabase::RIME_StartOfBook);
			Q_ASSERT(pBibleDatabase->bookEntry(ndxStart) != nullptr);
			if (pBibleDatabase->bookEntry(ndxStart)->m_bHaveColophon) {
				ndxStart.setChapter(0);
				ndxStart.setVerse(0);
			}
			refCalc = CRefCountCalc(pBibleDatabase, CRefCountCalc::RTE_WORD, ndxStart);
			nCount = refCalc.ofBook().second;
		}
	}

	uint32_t ndxNormal = pBibleDatabase->NormalizeIndex(ndxStart);
	uint64_t arrnValues[GBTE_COUNT][GMTE_COUNT][GLTE_COUNT] = {};
	bool arrbSkip[GBTE_COUNT][GMTE_COUNT][GLTE_COUNT] = {};
	while (nCount--) {
		for (uint32_t nBaseType = 0; nBaseType < GBTE_COUNT; ++nBaseType) {
			for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
				for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
					const CConcordanceEntry *pConcordanceEntry = pBibleDatabase->concordanceEntryForWordAtIndex(ndxNormal);
					Q_ASSERT(pConcordanceEntry != nullptr);
					if (pConcordanceEntry == nullptr) {
						arrbSkip[nBaseType][nMathXform][nLtrXform] = true;
					} else {
						const CGematriaCalc &nWordGematria = pConcordanceEntry->gematria();
						arrnValues[nBaseType][nMathXform][nLtrXform] += nWordGematria.m_arrnValues[nBaseType][nMathXform][nLtrXform];
						arrbSkip[nBaseType][nMathXform][nLtrXform] = arrbSkip[nBaseType][nMathXform][nLtrXform] ||
								nWordGematria.m_arrbSkip[nBaseType][nMathXform][nLtrXform];
					}
				}
			}
		}

		++ndxNormal;
	}

	for (uint32_t nBaseType = 0; nBaseType < GBTE_COUNT; ++nBaseType) {
		QString strThisToolTip;
		for (uint32_t nMathXform = 0; nMathXform < GMTE_COUNT; ++nMathXform) {
			for (uint32_t nLtrXform = 0; nLtrXform < GLTE_COUNT; ++nLtrXform) {
				if (arrbSkip[nBaseType][nMathXform][nLtrXform]) continue;
				if (arrnValues[nBaseType][nMathXform][nLtrXform] == 0) continue;
				strThisToolTip += QString("%1: %2\n").arg(CGematriaNames::name(CGematriaIndex(nBaseType, nMathXform, nLtrXform)))
												.arg(arrnValues[nBaseType][nMathXform][nLtrXform]);
			}
		}
		if (!strToolTip.isEmpty() && !strThisToolTip.isEmpty()) {
			if (!bPlainText) {
				strToolTip += "</pre><hr /><pre>";
			} else {
				strToolTip += "--------------------\n";
			}
		}
		strToolTip += strThisToolTip;
	}

	return strToolTip;
}

// ============================================================================
