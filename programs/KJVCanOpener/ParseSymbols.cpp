/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#endif
#if QT_VERSION < 0x050F00
#include <QRegExp>
#endif

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
								QString(QChar(0x05BE)) +		// U+05BE	&#1470; 	hebrew punctuation maqaf (or maqqef) 	word hyphen in Hebrew
//								QString(QChar(0x1400)) +		// U+1400	&#5120; 	canadian syllabics hyphen 	used in Canadian Aboriginal Syllabics
//								QString(QChar(0x1806)) +		// U+1806	&#6150; 	mongolian todo soft hyphen 	as soft hyphen, but displayed at the beginning of the second line
								QString(QChar(0x2010)) +		// U+2010	&#8208; 	hyphen 	unambiguously a hyphen character, as in “left-to-right”; narrow width
								QString(QChar(0x2011)) +		// U+2011	&#8209; 	non-breaking hyphen 	as hyphen (U+2010), but not an allowed line break point
								QString(QChar(0x2012)) +		// U+2012	&#8210; 	figure dash 	as hyphen-minus, but has the same width as digits
								QString(QChar(0x2013)) +		// U+2013	&#8211; 	en dash 	used e.g. to indicate a range of values
								QString(QChar(0x2014)) +		// U+2014	&#8212; 	em dash 	used e.g. to make a break in the flow of a sentence
								QString(QChar(0x2015)) +		// U+2015	&#8213; 	horizontal bar 	used to introduce quoted text in some typographic styles; “quotation dash”; often (e.g., in the representative glyph in the Unicode standard) longer than em dash
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
									QString(QChar(0x201B)) +		// U+201B	&#8219;		Quote reversed
									QString(QChar(0x05F3));			// U+05F3	&#1523;		HEBREW PUNCTUATION GERESH ("׳")


// Ascii Word characters -- these will be kept in words as-is and includes
//	alphanumerics.  Hyphen and apostrophe are kept too, but by the rules
//	above, not here.  Non-Ascii (high UTF8 values) are also kept, but have
//	rules of their own:
const QString g_strAsciiWordChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


// Non-Word Non-Ascii characters -- these are non-Ascii characters that do
//	not automatically apply as a word -- things like quotes, etc:
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
											QString(QChar(0x00B7)) +		// U+00B7	&#183;		Centered period
											QString(QChar(0x00A0)) +		// U+00A0	&#160;		No-Break Space
											QString(QChar(0x05C3)) +		// U+05C3	&#1475;		HEBREW PUNCTUATION SOF PASUQ ("׃")
											QString(QChar(0x05C0)) +		// U+05C0	&#1472;		HEBREW PUNCTUATION PASEQ ("׀")
// Note: We don't include the Gershayim here
//	because it acts as a word modifier to denote
//	acronyms, multi-digit Hebrew numbers, and
//	to denote literal letters vs homographs, etc.
//	Therefore, we want to treat it as part of the
//	word and not exclude it:
//											QString(QChar(0x05F4)) +		// U+05F4	&#1524;		HEBREW PUNCTUATION GERSHAYIM ("״")
											QString(QChar(0x05C6));			// U+05C6	&#1478;		HEBREW PUNCTUATION NUN HAFUKHA ("׆")

const QChar g_chrPilcrow = QChar(0x00B6);		// Pilcrow paragraph marker

// ============================================================================
// ============================================================================

namespace StringParse {

	QString decompose(const QString &strWord, bool bRemoveHyphens)
	{
		QString strDecomposed = deLigature(deApostrHyphen(strWord, bRemoveHyphens).normalized(QString::NormalizationForm_KD));

		// There are two possible ways to remove accent marks:
		//
		//		1) strDecomposed.remove(QRegExp("[^a-zA-Z\\s]"));
		//				Note: This one only works for English and similar
		//				languages (not Greek or Hebrew or Cyrillic, etc).
		//
		//		2) Remove characters of class "Mark" (QChar::Mark_NonSpacing,
		//				QChar::Mark_SpacingCombining, QChar::Mark_Enclosing),
		//				which can be done by checking isMark()
		//

		for (int nPos = strDecomposed.size()-1; nPos >= 0; --nPos) {
			if (strDecomposed.at(nPos).isMark()) strDecomposed.remove(nPos, 1);
		}

		return strDecomposed;
	}

	QString reduce(const QString &strWord, bool bRemoveHyphens)
	{
		QString strReduced = deLigature(deApostrHyphen(strWord, bRemoveHyphens).normalized(QString::NormalizationForm_KC));

		for (int nPos = strReduced.size()-1; nPos >= 0; --nPos) {
			if (strReduced.at(nPos).isMark()) strReduced.remove(nPos, 1);
		}

		return strReduced;
	}

	QString deLigature(const QString &strWord)
	{
		QString strDecomposed = strWord;

		static const QString constrDoubleVav = QString(QChar(0x05D5)) + QString(QChar(0x05D5));
		static const QString constrVavYod = QString(QChar(0x05D5)) + QString(QChar(0x05D9));
		static const QString constrDoubleYod = QString(QChar(0x05D9)) + QString(QChar(0x05D9));

		strDecomposed.replace(QChar(0x00C6), "Ae");				// U+00C6	&#198;		AE character
		strDecomposed.replace(QChar(0x00E6), "ae");				// U+00E6	&#230;		ae character
		strDecomposed.replace(QChar(0x0132), "IJ");				// U+0132	&#306;		IJ character
		strDecomposed.replace(QChar(0x0133), "ij");				// U+0133	&#307;		ij character
		strDecomposed.replace(QChar(0x0152), "Oe");				// U+0152	&#338;		OE character
		strDecomposed.replace(QChar(0x0153), "oe");				// U+0153	&#339;		oe character

		strDecomposed.replace(QChar(0x05F0), constrDoubleVav);	// U+05F0	&#1520;		Hebrew Yiddish Double Vav ("װ")
		strDecomposed.replace(QChar(0x05F1), constrVavYod);		// U+05F1	&#1521;		Hebrew Yiddish Vav Yod ("ױ")
		strDecomposed.replace(QChar(0x05F2), constrDoubleYod);	// U+05F2	&#1522;		Hebrew Yiddish Double Yod ("ײ")

		return strDecomposed;
	}

	QString deApostrHyphen(const QString &strWord, bool bRemoveHyphens)
	{
		return deHyphen(deApostrophe(strWord, false), bRemoveHyphens);
	}

	QString deApostrophe(const QString &strWord, bool bRemove)
	{
	#if QT_VERSION >= 0x050000
		static const QString strApostropheRegExp = QChar('[') + QRegularExpression::escape(g_strApostrophes) + QChar(']');
		static const QRegularExpression expApostrophe(strApostropheRegExp);
	#else
		static const QString strApostropheRegExp = QChar('[') + QRegExp::escape(g_strApostrophes) + QChar(']');
		static const QRegExp expApostrophe(strApostropheRegExp);
	#endif

		QString strDecomposed = strWord;

		if (!bRemove) {
			strDecomposed.replace(expApostrophe, "'");
		} else {
			strDecomposed.remove(expApostrophe);
		}

		return strDecomposed;
	}

	QString deHyphen(const QString &strWord, bool bRemove)
	{
	#if QT_VERSION >= 0x050000
		static const QString strHyphenRegExp = QChar('[') + QRegularExpression::escape(g_strHyphens) + QChar(']');
		static const QRegularExpression expHyphen(strHyphenRegExp);
	#else
		static const QString strHyphenRegExp = QChar('[') + QRegExp::escape(g_strHyphens) + QChar(']');
		static const QRegExp expHyphen(strHyphenRegExp);
	#endif

		QString strDecomposed;

		if (!bRemove) {
			strDecomposed = strWord;
			strDecomposed.replace(expHyphen, "-");
		} else {
			// Remove hyphens, but leave embedded regexp charsets intact:
			int nPos = 0;
			QString strDecomposed2 = strWord;
			while (!strDecomposed2.isEmpty()) {
				nPos = strDecomposed2.indexOf(QChar('['));
				if (nPos != -1) {
					strDecomposed += strDecomposed2.mid(0, nPos).remove(expHyphen);
					strDecomposed2 = strDecomposed2.mid(nPos);
					nPos = strDecomposed2.indexOf(QChar(']'));
					if (nPos != -1) {
						strDecomposed += strDecomposed2.mid(0, nPos+1);
						strDecomposed2 = strDecomposed2.mid(nPos+1);
					} else {
						strDecomposed += strDecomposed2;
						strDecomposed2.clear();
					}
				} else {
					strDecomposed += strDecomposed2.remove(expHyphen);
					strDecomposed2.clear();
				}
			}
		}

		return strDecomposed;
	}

	QString deCantillate(const QString &strWord)
	{
		QString strDecomposed = strWord.normalized(QString::NormalizationForm_KD);

		for (int nPos = strDecomposed.size()-1; nPos >= 0; --nPos) {
			if ((strDecomposed.at(nPos).unicode() >= 0x0591) &&
				(strDecomposed.at(nPos).unicode() <= 0x05AF)) {
				strDecomposed.remove(nPos, 1);
			}
		}

		return strDecomposed.normalized(QString::NormalizationForm_C);
	}

	TFirstCharSize firstCharSize(const QString &strWord)
	{
		// Get the number of QChars from the string that represents the first
		//	real character of the word.  This includes the first base-character
		//	(non-mark) and any "mark" that comes after it to mark it up.  Any of
		//	these can, of course, be dual-QChar surrogate pairs.  So, if the
		//	first QChar read is a surrogate, it and its lowSurrogate pair count
		//	in the final results.  We then want to read and count any marks after
		//	that first base character.  If what follows is a mark, then it's
		//	counted as-is.  But, if what follows is a surrogate, we have to look
		//	at the next character combined to see if it's a surrogate mark.  If
		//	so, we count it too, but if not, we have to uncount the first half of
		//	the pair because it means we are looking at the next complete character
		//	of the word instead...
		//
		//	NSC = Non-Surrogate base-character
		//	HSC = High Surrogate for base-character
		//	LSC = Low Surrogate for base-character
		//	NSM = Non-Surrogate mark
		//	HSM = High Surrogate for mark
		//	LSM = Low Surrogate for mark
		//
		//		NSC NSM NSM NSC ... count = 3, break at ndx==3
		//		HSC LSC NSM NSC ... count = 3, break at ndx==3
		//		HSC LSC HSM LSM NSC ... count = 4, break at ndx==4 with no decrement
		//		HSC LSC HSC LSC ... count = 2, break at ndx==3 with decrement
		//		NSC HSM LSM NSM HSC LSC ... count = 4, break at ndx==5 with decrement
		//
		TFirstCharSize fcs;

		if (!strWord.isEmpty()) {
			QChar chrPrevious = strWord.at(0);
			++fcs.m_nSize;
			for (int ndx = 1; ndx < strWord.size(); ++ndx) {
				// Note: High surrogates always come first when there's a surrogate pair
#if QT_VERSION >= 0x050000
				if (!strWord.at(ndx).isMark() && (!strWord.at(ndx).isSurrogate())) break;
#else
				if (!strWord.at(ndx).isMark() && (strWord.at(ndx).category() != QChar::Other_Surrogate)) break;
#endif
				if (strWord.at(ndx).isMark()) fcs.m_bHasMarks = true;
#if QT_VERSION >= 0x050000
				if (strWord.at(ndx).isLowSurrogate() &&
					(fcs.m_nSize > 2) &&
					!QChar::isMark(QChar::surrogateToUcs4(chrPrevious, strWord.at(ndx)))) {
					--fcs.m_nSize;		// If the pair isn't a mark, "unget" the upper-byte from the count as this is the next base-character
					break;
				}
				if (strWord.at(ndx).isLowSurrogate() &&
					QChar::isMark(QChar::surrogateToUcs4(chrPrevious, strWord.at(ndx)))) {
					fcs.m_bHasMarks = true;
				}
#else
				if (strWord.at(ndx).isHighSurrogate()) {
					// Note: On Qt4, just don't support surrogate marks, only base characters.
					//	If this is a new surrogate starting, bail out.  Note that we use
					//	isHighSurrogate() here for a new surrogate because we could still have
					//	the lowSurrogate for the base-character to process.  It's only marks we drop:
					break;
				}
#endif
				chrPrevious = strWord.at(ndx);
				++fcs.m_nSize;
			}
		}

		return fcs;
	}

};	// Namespace StringParse

// ============================================================================

