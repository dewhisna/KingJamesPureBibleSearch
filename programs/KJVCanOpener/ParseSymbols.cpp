/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "ParseSymbols.h"

// ============================================================================
// ============================================================================

// For processing hyphenated words, the following symbols will be treated
//	as hyphens and rolled into the "-" symbol for processing.  Words with
//	only hyphen differences will be added to the base word as a special
//	alternate form, allowing users to search them with or without hyphen
//	sensitivity:
const QString g_strHyphens =	QString(QChar(0x002D)) +		// U+002D	&#45;		hyphen-minus 	the Ascii hyphen, with multiple usage, or “ambiguous semantic value”; the width should be “average”
//								QString(QChar(0x007E)) +		// U+007E	&#126;		tilde 	the Ascii tilde, with multiple usage; “swung dash”
								QString(QChar(0x00AD)) +		// U+00AD	&#173;		soft hyphen 	“discretionary hyphen”
								QString(QChar(0x058A)) +		// U+058A	&#1418; 	armenian hyphen 	as soft hyphen, but different in shape
								QString(QChar(0x05BE)) +		// U+05BE	&#1470; 	hebrew punctuation maqaf 	word hyphen in Hebrew
//								QString(QChar(0x1400)) +		// U+1400	&#5120; 	canadian syllabics hyphen 	used in Canadian Aboriginal Syllabics
//								QString(QChar(0x1806)) +		// U+1806	&#6150; 	mongolian todo soft hyphen 	as soft hyphen, but displayed at the beginning of the second line
								QString(QChar(0x2010)) +		// U+2010	&#8208; 	hyphen 	unambiguously a hyphen character, as in “left-to-right”; narrow width
								QString(QChar(0x2011)) +		// U+2011	&#8209; 	non-breaking hyphen 	as hyphen (U+2010), but not an allowed line break point
								QString(QChar(0x2012)) +		// U+2012	&#8210; 	figure dash 	as hyphen-minus, but has the same width as digits
								QString(QChar(0x2013)) +		// U+2013	&#8211; 	en dash 	used e.g. to indicate a range of values
// >>>>>>>>>>>>					QString(QChar(0x2014)) +		// U+2014	&#8212; 	em dash 	used e.g. to make a break in the flow of a sentence
// >>>>>>>>>>>>					QString(QChar(0x2015)) +		// U+2015	&#8213; 	horizontal bar 	used to introduce quoted text in some typographic styles; “quotation dash”; often (e.g., in the representative glyph in the Unicode standard) longer than em dash
//								QString(QChar(0x2053)) +		// U+2053	&#8275; 	swung dash 	like a large tilde
//								QString(QChar(0x207B)) +		// U+207B	&#8315; 	superscript minus 	a compatibility character which is equivalent to minus sign U+2212 in superscript style
//								QString(QChar(0x208B)) +		// U+208B	&#8331; 	subscript minus 	a compatibility character which is equivalent to minus sign U+2212 in subscript style
								QString(QChar(0x2212)) +		// U+2212	&#8722; 	minus sign 	an arithmetic operator; the glyph may look the same as the glyph for a hyphen-minus, or may be longer ;
//								QString(QChar(0x2E17)) +		// U+2E17	&#11799; 	double oblique hyphen 	used in ancient Near-Eastern linguistics; not in Fraktur, but the glyph of Ascii hyphen or hyphen is similar to this character in Fraktur fonts
// >>>>>>>>>>>>					QString(QChar(0x2E3A)) +		// U+2E3A	&#11834; 	two-em dash 	omission dash<(a>, 2 em units wide
// >>>>>>>>>>>>					QString(QChar(0x2E3B)) +		// U+2E3B	&#11835; 	three-em dash 	used in bibliographies, 3 em units wide
//								QString(QChar(0x301C)) +		// U+301C	&#12316; 	wave dash 	a Chinese/Japanese/Korean character
//								QString(QChar(0x3030)) +		// U+3030	&#12336; 	wavy dash 	a Chinese/Japanese/Korean character
//								QString(QChar(0x30A0)) +		// U+30A0	&#12448;	katakana-hiragana double hyphen	in Japasene kana writing
//								QString(QChar(0xFE31)) +		// U+FE31	&#65073;	presentation form for vertical em dash	vertical variant of em dash
//								QString(QChar(0xFE32)) +		// U+FE32	&#65074;	presentation form for vertical en dash	vertical variant of en dash
								QString(QChar(0xFE58)) +		// U+FE58	&#65112;	small em dash	small variant of em dash
								QString(QChar(0xFE63)) +		// U+FE63	&#65123;	small hyphen-minus	small variant of Ascii hyphen
								QString(QChar(0xFF0D));			// U+FF0D	&#65293;	fullwidth hyphen-minus

// For processing words with apostrophes, the following symbols will be treated
//	as apostrophes and rolled into the "'" symbol for processing.  Words with
//	only apostrophe differences will be added to the base word as a special
//	alternate form, allowing users to search them with or without the apostrophe:
const QString g_strApostrophes =	QString(QChar(0x0027)) +		// U+0027	&#39;		Ascii apostrophe (single quote)
									QString(QChar(0x2018)) +		// U+2018	&#8216;		Quote left
									QString(QChar(0x2019)) +		// U+2019	&#8217;		Quote right
									QString(QChar(0x201B));			// U+201B	&#8219;		Quote reversed


// Ascii Word characters -- these will be kept in words as-is and includes
//	alphanumerics.  Hyphen and apostrophe are kept too, but by the rules
//	above, not here.  Non-Ascii (high UTF8 values) are also kept, but have
//	rules of their own:
const QString g_strAsciiWordChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


// Non-Word Non-Ascii characters -- these are non-Ascii characters the do
//	do automatically apply as a word -- things like quotes, etc:
const QString g_strNonAsciiNonWordChars =	QString(QChar(0x201C)) +		// U+201C	&#8220;		Double-Quote Left
											QString(QChar(0x201D)) +		// U+201D	&#8221;		Double-Quote Right
											QString(QChar(0x201E)) +		// U+201E	&#8222;		Double-Quote Base
											QString(QChar(0x201A)) +		// U+201A	&#8218;		Single-Quote Base
											QString(QChar(0x2039)) +		// U+2039	&#8249;		Single-Guil Left
											QString(QChar(0x203A)) +		// U+203A	&#8250;		Single-Guil Right
											QString(QChar(0x203C)) +		// U+203C	&#8252;		Double Exclamation
											QString(QChar(0x00AB)) +		// U+00AB	&#164;		Double-guillemot Left
											QString(QChar(0x00BB)) +		// U+00BB	&#187;		Double-guillemot Right
											QString(QChar(0x00BF)) +		// U+00BF	&#191;		Upside down question mark
											QString(QChar(0x00A1)) +		// U+00A1	&#161;		Upside down exclamation mark
											QString(QChar(0x00B7));			// U+00B7	&#183;		Centered period

const QChar g_chrPilcrow = QChar(0x00B6);		// Pilcrow paragraph marker

// ============================================================================
// ============================================================================

