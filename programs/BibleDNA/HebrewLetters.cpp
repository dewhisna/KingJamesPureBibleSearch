//
// Hebrew Letters : Type Definitions
//

#include <QStringList>

#include "HebrewLetters.h"

// ============================================================================

namespace {

	const QStringList g_HebrewLetters =
	{
					// Name			: Ndx: B3  : B6  : B9
		QString::fromUtf8("א"),		// Alef			:  0 : 000 : 00 : 00
		QString::fromUtf8("ב"),		// Bet			:  1 : 001 : 01 : 01
		QString::fromUtf8("ג"),		// Gimel		:  2 : 002 : 02 : 02
		QString::fromUtf8("ד"),		// Dalet		:  3 : 010 : 03 : 03
		QString::fromUtf8("ה"),		// He			:  4 : 011 : 04 : 04
		QString::fromUtf8("ו"),		// Vav			:  5 : 012 : 05 : 05
		QString::fromUtf8("ז"),		// Zayin		:  6 : 020 : 10 : 06
		QString::fromUtf8("ח"),		// Chet			:  7 : 021 : 11 : 07
		QString::fromUtf8("ט"),		// Tet			:  8 : 022 : 12 : 08
		QString::fromUtf8("י"),		// Yod			:  9 : 100 : 13 : 10
		QString::fromUtf8("כ‬"),		// Kaf			: 10 : 101 : 14 : 11
		QString::fromUtf8("ל"),		// Lamed		: 11 : 102 : 15 : 12
		QString::fromUtf8("מ"),		// Mem			: 12 : 110 : 20 : 13
		QString::fromUtf8("נ"),		// Nun			: 13 : 111 : 21 : 14
		QString::fromUtf8("ס"),		// Samekh		: 14 : 112 : 22 : 15
		QString::fromUtf8("ע"),		// Ayin			: 15 : 120 : 23 : 16
		QString::fromUtf8("פ"),		// Pe			: 16 : 121 : 24 : 17
		QString::fromUtf8("צ"),		// Tsade		: 17 : 122 : 25 : 18
		QString::fromUtf8("ק"),		// Qof			: 18 : 200 : 30 : 20
		QString::fromUtf8("ר‬"),		// Resh			: 19 : 201 : 31 : 21
		QString::fromUtf8("ש"),		// Shin			: 20 : 202 : 32 : 22
		QString::fromUtf8("ת"),		// Tav			: 21 : 210 : 33 : 23
		QString::fromUtf8("ך"),		// Kaf sofit	: 22 : 211 : 34 : 24
		QString::fromUtf8("ם"),		// Mem sofit	: 23 : 212 : 35 : 25
		QString::fromUtf8("ן"),		// Nun sofit	: 24 : 220 : 40 : 26
		QString::fromUtf8("ף"),		// Pe sofit		: 25 : 221 : 41 : 27
		QString::fromUtf8("ץ"),		// Tsade sofit	: 26 : 222 : 42 : 28
	};
}

// ============================================================================

CHebrewLetters::HEB_LTR_NDX CHebrewLetters::indexOfLetter(const QString &strLetter) const
{
	return static_cast<HEB_LTR_NDX>(g_HebrewLetters.indexOf(strLetter));
}

QString CHebrewLetters::letterForIndex(CHebrewLetters::HEB_LTR_NDX nIndex) const
{
	int niIndex = static_cast<int>(nIndex);
	if ((niIndex < 0) || (niIndex >= g_HebrewLetters.size())) return QString();
	return g_HebrewLetters.at(niIndex);
}

