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

#include "SoundEx.h"

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#endif
#if QT_VERSION < 0x050F00
#include <QRegExp>
#endif

// ============================================================================

namespace SoundEx {
	SOUNDEX_LANGUAGES_ENUM languageValue(const QString &strQtLanguage)
	{
		SOUNDEX_LANGUAGES_ENUM nLanguage = SELE_UNKNOWN;
		if (strQtLanguage.compare("en", Qt::CaseInsensitive) == 0) nLanguage = SELE_ENGLISH;
		if (strQtLanguage.compare("fr", Qt::CaseInsensitive) == 0) nLanguage = SELE_FRENCH;
		if (strQtLanguage.compare("es", Qt::CaseInsensitive) == 0) nLanguage = SELE_SPANISH;
		if (strQtLanguage.compare("de", Qt::CaseInsensitive) == 0) nLanguage = SELE_GERMAN;

		return nLanguage;
	}

	SOUNDEX_LANGUAGES_ENUM languageValue(LANGUAGE_ID_ENUM nLangID)
	{
		switch (nLangID) {
			case LIDE_ENGLISH:
				return SELE_ENGLISH;
			case LIDE_FRENCH:
				return SELE_FRENCH;
			case LIDE_SPANISH:
				return SELE_SPANISH;
			case LIDE_GERMAN:
				return SELE_GERMAN;
			default:
				return SELE_UNKNOWN;
		}
	}

	QString soundEx(const QString &strWordIn, SOUNDEX_LANGUAGES_ENUM nLanguage, int nLength, SOUNDEX_OPTION_MODE_ENUM nOption)
	{
	// strWordIn should already be decomposed:
	//	QString strSoundEx = StringParse::decompose(strWordIn, true).toUpper();
		QString strSoundEx = strWordIn.toUpper();
		int nSoundExLen = 0;

		if (nLanguage == SELE_UNKNOWN) nLanguage = SELE_ENGLISH;		// Use default functionality for English if the language isn't defined

		if (nLanguage != SELE_ENGLISH) nOption = SEOME_CLASSIC;			// Currently only support enhanced and census modes in English

		if ((nOption == SEOME_CENSUS_NORMAL) || (nOption == SEOME_CENSUS_SPECIAL)) nLength = 4;
		if (nLength) nSoundExLen = nLength;
		if (nSoundExLen > 10) nSoundExLen = 10;
		if (nSoundExLen < 4) nSoundExLen = 4;

		for (int i = 0; i < strSoundEx.size(); ++i) {
			if (!strSoundEx.at(i).isLetter()) strSoundEx[i] = QChar(' ');
		}
		strSoundEx = strSoundEx.trimmed();

		if (strSoundEx.isEmpty()) return QString();

		// Enhanced Non-Census Mode:
		if (nOption == SEOME_ENHANCED) {
#if QT_VERSION >= 0x050000
			strSoundEx.remove(QRegularExpression("^P(?=[SF])"));			// Replace PS at start of word with S and PF at start of word with F
			strSoundEx.replace(QRegularExpression("^[AI](?=[AEIO])"), "E");	// Replace A or I with E at start of word when followed by [AEIO]

			strSoundEx.replace(QRegularExpression("DG"), "G");				// Replace DG with G
			strSoundEx.replace(QRegularExpression("GH"), "H");				// Replace GH with H
			strSoundEx.replace(QRegularExpression("[KG]N"), "N");			// Replace KN and GN (not "ng") with N
			strSoundEx.replace(QRegularExpression("MB"), "M");				// Replace MB with M
			strSoundEx.replace(QRegularExpression("PH"), "F");				// Replace PH wtih F
			strSoundEx.replace(QRegularExpression("TCH"), "CH");			// Replace TCH with CH
			strSoundEx.replace(QRegularExpression("MP(?=[STZ])"), "M");		// Replace MP with M when followed by S, Z, or T
#else
			strSoundEx.remove(QRegExp("^P(?=[SF])"));				// Replace PS at start of word with S and PF at start of word with F
			strSoundEx.replace(QRegExp("^[AI](?=[AEIO])"), "E");	// Replace A or I with E at start of word when followed by [AEIO]

			strSoundEx.replace(QRegExp("DG"), "G");					// Replace DG with G
			strSoundEx.replace(QRegExp("GH"), "H");					// Replace GH with H
			strSoundEx.replace(QRegExp("[KG]N"), "N");				// Replace KN and GN (not "ng") with N
			strSoundEx.replace(QRegExp("MB"), "M");					// Replace MB with M
			strSoundEx.replace(QRegExp("PH"), "F");					// Replace PH wtih F
			strSoundEx.replace(QRegExp("TCH"), "CH");				// Replace TCH with CH
			strSoundEx.replace(QRegExp("MP(?=[STZ])"), "M");		// Replace MP with M when followed by S, Z, or T
#endif
		}

		Q_ASSERT(strSoundEx.size());

		// The following must be done AFTER the multi-letter
		//		replacements above since these could change
		//		the first character:
		QChar chrFirstLetter = strSoundEx[0];						// Note: We already know we have at least one character

		strSoundEx = strSoundEx.mid(1);

		// For "Normal" Census SoundEx, remove H and W
		//		before performing the test for adjacent
		//		digits:
		if ((nOption == SEOME_CLASSIC) || (nOption == SEOME_ENHANCED)) {
#if QT_VERSION >= 0x050000
			strSoundEx.remove(QRegularExpression("[HW]"));			// Note, strSoundEx[0] won't get removed here because of above preserving it
#else
			strSoundEx.remove(QRegExp("[HW]"));						// Note, strSoundEx[0] won't get removed here because of above preserving it
#endif
		}

		// Perform classic SoundEx replacements:
		switch (nLanguage) {
			case SELE_ENGLISH:
	//			https://en.wikipedia.org/wiki/Soundex
	//			https://creativyst.com/Doc/Articles/SoundEx1/SoundEx1.htm
	//
	//			The correct value can be found as follows:
	//
	//			1. Retain the first letter of the name and drop all other occurrences of a, e, i, o, u, y, h, w.
	//			2. Replace consonants with digits as follows (after the first letter):
	//					b, f, p, v => 1
	//					c, g, j, k, q, s, x, z => 2
	//					d, t => 3
	//					l => 4
	//					m, n => 5
	//					r => 6
	//			3. If two or more letters with the same number are adjacent in the original name (before step 1),
	//				only retain the first letter; also two letters with the same number separated by 'h' or 'w' are
	//				coded as a single number, whereas such letters separated by a vowel are coded twice. This rule
	//				also applies to the first letter.
	//			4. Iterate the previous step until you have one letter and three numbers. If you have too few letters
	//				in your word that you can't assign three numbers, append with zeros until there are three numbers.
	//				If you have more than 3 letters, just retain the first 3 numbers.

#if QT_VERSION >= 0x050000
				strSoundEx.replace(QRegularExpression("[AEIOUYHW]"), "0");	// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegularExpression("[BPFV]"), "1");		// [BPFV] => 1
				strSoundEx.replace(QRegularExpression("[CSGJKQXZ]"), "2");	// [CSGJKQXZ] => 2
				strSoundEx.replace(QRegularExpression("[DT]"), "3");		// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegularExpression("[MN]"), "5");		// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
#else
				strSoundEx.replace(QRegExp("[AEIOUYHW]"), "0");				// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegExp("[BPFV]"), "1");					// [BPFV] => 1
				strSoundEx.replace(QRegExp("[CSGJKQXZ]"), "2");				// [CSGJKQXZ] => 2
				strSoundEx.replace(QRegExp("[DT]"), "3");					// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegExp("[MN]"), "5");					// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
#endif
				break;

			case SELE_FRENCH:
	//			https://fr.wikipedia.org/wiki/Soundex
	//
	//			L'algorithme exact procède comme suit :
	//
	//			    1. Supprimer les éventuels 'espace' initiaux
	//			    2. Mettre le mot en majuscule
	//			    3. Garder la première lettre
	//			    4. Conserver la première lettre de la chaîne
	//			    5. Supprimer toutes les occurrences des lettres : a, e, h, i, o, u, w, y (à moins
	//					que ce ne soit la première lettre du nom)
	//			    6. Attribuer une valeur numérique aux lettres restantes de la manière suivante :
	//			        Version pour le français :
	//			            1 = B, P
	//			            2 = C, K, Q
	//			            3 = D, T
	//			            4 = L
	//			            5 = M, N
	//			            6 = R
	//			            7 = G, J
	//			            8 = X, Z, S
	//			            9 = F, V
	//			    7. Si deux lettres (ou plus) avec le même nombre sont adjacentes dans le nom
	//					d'origine, ou s'il n'y a qu'un h ou un w entre elles, alors on ne retient
	//					que la première de ces lettres.
	//			    8. Renvoyer les quatre premiers octets complétés par des zéros.

#if QT_VERSION >= 0x050000
				strSoundEx.replace(QRegularExpression("[AEIOUYHW]"), "0");	// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegularExpression("[BP]"), "1");		// [BP] => 1
				strSoundEx.replace(QRegularExpression("[CKQ]"), "2");		// [CKQ] => 2
				strSoundEx.replace(QRegularExpression("[DT]"), "3");		// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegularExpression("[MN]"), "5");		// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
				strSoundEx.replace(QRegularExpression("[GJ]"), "7");		// [GJ] => 7
				strSoundEx.replace(QRegularExpression("[XZS]"), "8");		// [XZS] => 8
				strSoundEx.replace(QRegularExpression("[FV]"), "9");		// [FV] => 9
#else
				strSoundEx.replace(QRegExp("[AEIOUYHW]"), "0");				// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegExp("[BP]"), "1");					// [BP] => 1
				strSoundEx.replace(QRegExp("[CKQ]"), "2");					// [CKQ] => 2
				strSoundEx.replace(QRegExp("[DT]"), "3");					// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegExp("[MN]"), "5");					// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
				strSoundEx.replace(QRegExp("[GJ]"), "7");					// [GJ] => 7
				strSoundEx.replace(QRegExp("[XZS]"), "8");					// [XZS] => 8
				strSoundEx.replace(QRegExp("[FV]"), "9");					// [FV] => 9
#endif
				break;

			case SELE_SPANISH:
	//			http://oraclenotepad.blogspot.com/2008/03/soundex-en-espaol.html
	//
	//			Los pasos básicos son:
	//
	//			    1. Retener la primera letra de la cadena. Tener en cuenta las letras dobles como CH y LL.
	//			    2. Remover todas las ocurrencias de las letras siguientes a partir de la segunda posición: a, e, i, o, u, h, w, y (cuando suena como vocal i )
	//			    3. Asignar números a las siguientes letras (luego de la primera):
	//			        b, f, p, v = 1
	//			        c, g, j, k, q, s, x, z = 2
	//			        d, t = 3
	//			        l = 4
	//			        m, n = 5
	//			        r = 6
	//			        ll, y, ch = 7
	//			    4. Si hay números consecutivos, dejar solamente uno en la serie.
	//			    5. Retornar los cuatro primeros caracteres, si son menos de cuatro completar con ceros.
	//
	//			SOUNDESP es un proyecto abierto y es bienvenido cualquier comentario para mejorar su implementación.
	//
	//			Y Rules:
	//			--------
	//			(Keep) : Y -> when alone, or after a vowel, or followed by a consonant, or at the end of a word,
	//					is a vowel, and sounds as e or ee in English: Hoy y mañana (today and tomorrow), o’-e ee mah-nyah’-nah
	//
	//			(remove) : Y -> before a vowel in the same syllable, or between two vowels in the same word, is a consonant,
	//					and sounds like the English y in the words yard, yell, you

#if QT_VERSION >= 0x050000
				strSoundEx.replace("CH", "7");								// Proceso letras dobles primero.
				strSoundEx.replace("LL", "7");
				strSoundEx.replace(QRegularExpression("Y$"), "7");			// Y al final de la palabra.
				strSoundEx.replace(QRegularExpression("Y(?=[bcdfghjklmnpqrstvwxz])"), "7");	// Y antes de una consonante.
				strSoundEx.replace(QRegularExpression("[AEIOU]Y"), "07");	// Y después de una vocal : Note: QRegExp doesn't support look-behind, so combining this with "0" substitution for vowels that follows

				strSoundEx.replace(QRegularExpression("[AEIOUYHW]"), "0");	// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegularExpression("[BPFV]"), "1");		// [BPFV] => 1
				strSoundEx.replace(QRegularExpression("[CSGJKQXZ]"), "2");	// [CSGJKQXZ] => 2
				strSoundEx.replace(QRegularExpression("[DT]"), "3");		// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegularExpression("[MN]"), "5");					// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
#else
				strSoundEx.replace("CH", "7");								// Proceso letras dobles primero.
				strSoundEx.replace("LL", "7");
				strSoundEx.replace(QRegExp("Y$"), "7");						// Y al final de la palabra.
				strSoundEx.replace(QRegExp("Y(?=[bcdfghjklmnpqrstvwxz])"), "7");	// Y antes de una consonante.
				strSoundEx.replace(QRegExp("[AEIOU]Y"), "07");				// Y después de una vocal : Note: QRegExp doesn't support look-behind, so combining this with "0" substitution for vowels that follows

				strSoundEx.replace(QRegExp("[AEIOUYHW]"), "0");				// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegExp("[BPFV]"), "1");					// [BPFV] => 1
				strSoundEx.replace(QRegExp("[CSGJKQXZ]"), "2");				// [CSGJKQXZ] => 2
				strSoundEx.replace(QRegExp("[DT]"), "3");					// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegExp("[MN]"), "5");					// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
#endif
				break;

			case SELE_GERMAN:
	//			http://www.sound-ex.de/soundex_verfahren.htm
	//			https://de.wikipedia.org/wiki/Soundex
	//
	//			Grundregeln
	//
	//			Jeder Soundex-Code besteht aus einem Buchstaben gefolgt von drei Ziffern, z.B.
	//			W-213 für Wikipedia. Hat das zu codierende Wort soviele Buchstaben, daß man mehr
	//			Ziffern erzeugen könnte, bricht man nach der dritten Ziffer ab. Hat das Wort zu
	//			wenige Buchstaben, füllt man die letzten Ziffern mit 0-en auf. Der asiatische Name
	//			Lee wird also als L-000 codiert.
	//			Ziffer => Repräsentatierte Buchstaben
	//			1 => B, F, P, V
	//			2 => C, G, J, K, Q, S, X, Z
	//			3 => D, T
	//			4 => L
	//			5 => M, N
	//			6 => R
	//
	//			Die Vokale A, E, I, O und U, als auch die Konsonanten H, W und Y sind zu ignorieren,
	//			allerdings nicht an erster Stelle als führender Buchstabe. Erweiternd für die deutsche
	//			Sprache definiert man: Die Umlaute Ä, Ö und Ü sind zu ignorieren, das "scharfe S" ß wird
	//			wie das einfache S als 2 codiert.
	//
	//			Doppelte Buchstaben
	//
	//			Doppelte Buchstaben, wie in Kallbach sind wie ein einzelner Buchstabe zu betrachten.
	//
	//			• Kallbach wird daher zu K-412 (K -> K, A wird verworfen, L -> 4, 2. L wird verworfen,
	//				B -> 1, 2. A wird verworfen, C -> 2, Abbruch weil wir bereits 3 Ziffern haben).
	//
	//			Aufeinanderfolgende Buchstaben mit gleichem Soundex-Code
	//
	//			Werden wie gleiche Buchstaben behandelt.
	//
	//			• Hackelmeier wird daher zu H-245 (H -> H, A wird verworfen, C -> 2, K wird verworfen weil
	//				auch = 2, E wird verworfen, L -> 4, M -> 5, Abbruch weil wir bereits 3 Ziffern haben.
	//
	//			Namenszusätze
	//
	//			Namenszusätze können ignoriert werden, oder normal mitkodiert werden. Bei der Suche ist
	//			dies entsprechend zu berücksichtigen, d.h. es sind ggf. zwei Suchen durchzuführen.
	//
	//			• von Neumann wird einmal zu V-555 oder zu N-550 (beachte auch die folgende Regel)
	//
	//			Konsonantentrennung
	//
	//			Werden zwei Konsonanten mit dem gleichen Soundex-Code durch ein Vokal (oder Y) getrennt,
	//			so wird der rechte Konsonant NICHT verworfen. Ist allerdings ein H oder ein W das Trennzeichen,
	//			so wird der rechte Konsonant wie bei der Aufeinanderfolgende Buchstaben-Regel verworfen.

#if QT_VERSION >= 0x050000
				strSoundEx.replace(QRegularExpression("[AEIOUYHW]"), "0");	// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegularExpression("[BPFV]"), "1");		// [BPFV] => 1
				strSoundEx.replace(QRegularExpression("[CSGJKQXZ]"), "2");	// [CSGJKQXZ] => 2
				strSoundEx.replace(QRegularExpression("[DT]"), "3");		// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegularExpression("[MN]"), "5");		// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
#else
				strSoundEx.replace(QRegExp("[AEIOUYHW]"), "0");				// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
				strSoundEx.replace(QRegExp("[BPFV]"), "1");					// [BPFV] => 1
				strSoundEx.replace(QRegExp("[CSGJKQXZ]"), "2");				// [CSGJKQXZ] => 2
				strSoundEx.replace(QRegExp("[DT]"), "3");					// [DT] => 3
				strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
				strSoundEx.replace(QRegExp("[MN]"), "5");					// [MN] => 5
				strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
#endif

				// TODO : Complete German Rules
				break;

			default:
				break;
		}

		// Remove extra equal adjacent digits:
		for (int i = 0; i < (strSoundEx.size()-1); /* Increment inside loop */) {
			if (strSoundEx[i] == strSoundEx[i+1]) {
				strSoundEx.remove(i+1, 1);
			} else {
				++i;
			}
		}

		// Remove spaces and 0's
		strSoundEx.remove(QChar(' '));
		strSoundEx.remove(QChar('0'));

		// Replace first Letter and Right-pad with zeros:
		QString strZeros;
		strZeros.fill(QChar('0'), nSoundExLen);
		strSoundEx = chrFirstLetter + strSoundEx + strZeros;
		strSoundEx = strSoundEx.left(nSoundExLen);

		return strSoundEx;
	}

};	// namespace SoundEx

// ============================================================================

