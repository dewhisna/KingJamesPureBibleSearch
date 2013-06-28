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

#include "SearchCompleter.h"
#include "PhraseEdit.h"

#include <QString>
#include <QStringRef>
#include <QRegExp>

#include <assert.h>

// ============================================================================


CSearchStringListModel::CSearchStringListModel(const CParsedPhrase &parsedPhrase, QObject *parent)
	:	QAbstractListModel(parent),
		m_parsedPhrase(parsedPhrase)
{

}

CSearchStringListModel::~CSearchStringListModel()
{

}

int CSearchStringListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_parsedPhrase.nextWordsList().size();
}

QVariant CSearchStringListModel::data(const QModelIndex &index, int role) const
{
	if ((index.row() < 0) || (index.row() >= m_parsedPhrase.nextWordsList().size()))
		return QVariant();

	if (role == Qt::DisplayRole)
		return m_parsedPhrase.nextWordsList().at(index.row()).word();

	if (role == Qt::EditRole)
		return m_parsedPhrase.nextWordsList().at(index.row()).decomposedWord();

	if (role == SOUNDEX_ENTRY_ROLE)
		return CSoundExSearchCompleterFilter::soundEx(m_parsedPhrase.nextWordsList().at(index.row()).decomposedWord());

	return QVariant();
}

bool CSearchStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);

	return false;
}

void CSearchStringListModel::sort(int column, Qt::SortOrder order)
{
	Q_UNUSED(column);
	Q_UNUSED(order);
	assert(false);
}

void CSearchStringListModel::setWordsFromPhrase()
{
	emit beginResetModel();

//	m_ParsedPhrase.nextWordsList();

	emit endResetModel();
}

QString CSearchStringListModel::decompose(const QString &strWord)
{
	QString strDecomposed = strWord.normalized(QString::NormalizationForm_KD);

	strDecomposed.replace(QChar(0x00C6), "Ae");				// U+00C6	&#198;		AE character
	strDecomposed.replace(QChar(0x00E6), "ae");				// U+00E6	&#230;		ae character
	strDecomposed.replace(QChar(0x0132), "IJ");				// U+0132	&#306;		IJ character
	strDecomposed.replace(QChar(0x0133), "ij");				// U+0133	&#307;		ij character
	strDecomposed.replace(QChar(0x0152), "Oe");				// U+0152	&#338;		OE character
	strDecomposed.replace(QChar(0x0153), "oe");				// U+0153	&#339;		oe character

	// There are two possible ways to remove accent marks:
	//
	//		1) strDecomposed.remove(QRegExp("[^a-zA-Z\\s]"));
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

// ============================================================================

CSearchCompleter::CSearchCompleter(const CParsedPhrase &parsedPhrase, QWidget *parentWidget)
	:	QCompleter(parentWidget),
		m_nCompletionFilterMode(SCFME_SOUNDEX),
		m_pSearchStringListModel(NULL),
		m_pSoundExFilterModel(NULL)
{
	m_pSearchStringListModel = new CSearchStringListModel(parsedPhrase, this);
	m_pSoundExFilterModel = new CSoundExSearchCompleterFilter(this);
	setCompletionFilterMode(m_nCompletionFilterMode);
	m_pSoundExFilterModel->setSourceModel(m_pSearchStringListModel);
	setModel(m_pSoundExFilterModel);

	setWidget(parentWidget);
//	setCompletionMode(QCompleter::PopupCompletion /* UnfilteredPopupCompletion */ );
	setCaseSensitivity(Qt::CaseInsensitive);
	setModelSorting(QCompleter::CaseInsensitivelySortedModel);
}

CSearchCompleter::~CSearchCompleter()
{

}

void CSearchCompleter::setCompletionFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nCompletionFilterMode)
{
	switch (nCompletionFilterMode) {
		case SCFME_NORMAL:
			setCompletionMode(QCompleter::PopupCompletion);
			break;
		case SCFME_UNFILTERED:
			setCompletionMode(QCompleter::UnfilteredPopupCompletion);
			break;
		case SCFME_SOUNDEX:
			setCompletionMode(QCompleter::UnfilteredPopupCompletion);
			break;
	}

	m_nCompletionFilterMode = nCompletionFilterMode;
}

void CSearchCompleter::setCompletionPrefix(const QString &prefix)			// Note: Caller should be passing a decomposed prefix
{
	if (m_nCompletionFilterMode != SCFME_SOUNDEX) {
		QCompleter::setCompletionPrefix(prefix);
//		m_pSoundExFilterModel->setFilterRegExp(QString());
		m_pSoundExFilterModel->setFilterFixedString(QString());
	} else {
		QCompleter::setCompletionPrefix(QString());
//		m_pSoundExFilterModel->setFilterRegExp(CSoundExSearchCompleterFilter::soundEx(prefix));
		m_pSoundExFilterModel->setFilterFixedString(CSoundExSearchCompleterFilter::soundEx(prefix));
	}
}

// ============================================================================


// ============================================================================

CSoundExSearchCompleterFilter::CSoundExSearchCompleterFilter(QObject *parent)
	:	QSortFilterProxyModel(parent)
{
	setFilterRole(CSearchStringListModel::SOUNDEX_ENTRY_ROLE);
	setFilterCaseSensitivity(Qt::CaseSensitive);
	setSortRole(Qt::EditRole);
	setSortCaseSensitivity(Qt::CaseInsensitive);
//	setDynamicSortFilter(true);
}

CSoundExSearchCompleterFilter::~CSoundExSearchCompleterFilter()
{

}

CSoundExSearchCompleterFilter::SOUNDEX_LANGUAGES_ENUM CSoundExSearchCompleterFilter::languageValue(const QString &strLanguage)
{
	SOUNDEX_LANGUAGES_ENUM nLanguage = SELE_UNKNOWN;
	if (strLanguage.compare("en", Qt::CaseInsensitive) == 0) nLanguage = SELE_ENGLISH;
	if (strLanguage.compare("fr", Qt::CaseInsensitive) == 0) nLanguage = SELE_FRENCH;
	if (strLanguage.compare("es", Qt::CaseInsensitive) == 0) nLanguage = SELE_SPANISH;
	if (strLanguage.compare("de", Qt::CaseInsensitive) == 0) nLanguage = SELE_GERMAN;

	return nLanguage;
}

QString CSoundExSearchCompleterFilter::soundEx(const QString &strWordIn, SOUNDEX_LANGUAGES_ENUM nLanguage, int nLength, SOUNDEX_OPTION_MODE_ENUM nOption)
{
// strWordIn should already be decomposed:
//	QString strSoundEx = CSearchStringListModel::decompose(strWordIn).toUpper();
	QString strSoundEx = strWordIn.toUpper();
	int nSoundExLen = 0;

	if (nLanguage == SELE_UNKNOWN) return QString();

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
	//		Underscore placeholders(_) will be removed below...
	if (nOption == SEOME_ENHANCED) {
		if (strSoundEx.startsWith("PS") ||						// Replace PS with S at start of word
			strSoundEx.startsWith("PF"))						// Replace PF with F at start of word
			strSoundEx[0] = QChar('_');

		if (strSoundEx.startsWith("AA") ||						// Replace A or I with E at start of word when followed by [AEIO]
			strSoundEx.startsWith("AE") ||
			strSoundEx.startsWith("AI") ||
			strSoundEx.startsWith("AO") ||
			strSoundEx.startsWith("IA") ||
			strSoundEx.startsWith("IE") ||
			strSoundEx.startsWith("II") ||
			strSoundEx.startsWith("IO"))
			strSoundEx[0] = QChar('E');

		for (int i = 0; i < strSoundEx.size(); ++i) {
			if (i > (strSoundEx.size() - 2)) continue;			// Need at least 2 characters for the following compares
			QStringRef strNextTwo(&strSoundEx, i, 2);

			if ((strNextTwo.compare("DG") == 0) ||				// Replace DG with G
				(strNextTwo.compare("GH") == 0) ||				// Replace GH with H
				(strNextTwo.compare("KN") == 0) ||				// Replace KN with N
				(strNextTwo.compare("GN") == 0)) {				// Replace GN with N (not "ng")
				strSoundEx[i] = QChar('_');
				++i;
				continue;
			}

			if (strNextTwo.compare("MB") == 0) {				// Replace MB with M
				strSoundEx[i+1] = QChar('_');
				++i;
				continue;
			}

			if (strNextTwo.compare("PH") == 0) {				// Replace PH wtih F
				strSoundEx[i] = QChar('F');
				strSoundEx[i+1] = QChar('_');
				++i;
				continue;
			}

			if (i > (strSoundEx.size() - 3)) continue;			// Need at least 3 characters for the following compares
			QStringRef strNextThree(&strSoundEx, i, 3);

			if (strNextThree.compare("TCH") == 0) {				// Replace TCH with CH
				strSoundEx[i] = QChar('_');
				i+=2;
				continue;
			}

			if ((strNextThree.compare("MPS") == 0) ||
				(strNextThree.compare("MPT") == 0) ||
				(strNextThree.compare("MPZ") == 0)) {			// Replace MP with M when followed by S, Z, or T
				strSoundEx[i+1] = QChar('_');
				++i;
				continue;
			}
		}

		strSoundEx.remove(QChar('_'));							// Remove our temporary "_" characters
	}

	assert(strSoundEx.size());

	// The following must be done AFTER the multi-letter
	//		replacements above since these could change
	//		the first character:
	QChar chrFirstLetter = strSoundEx[0];						// Note: We already know we have at least one character

	strSoundEx = strSoundEx.mid(1);

	// For "Normal" Census SoundEx, remove H and W
	//		before performing the test for adjacent
	//		digits:
	if ((nOption == SEOME_CLASSIC) || (nOption == SEOME_ENHANCED)) {
		strSoundEx.remove(QRegExp("[HW]"));						// Note, strSoundEx[0] won't get removed here because of above preserving it
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

			strSoundEx.replace(QRegExp("[AEIOUYHW]"), "0");				// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
			strSoundEx.replace(QRegExp("[BPFV]"), "1");					// [BPFV] => 1
			strSoundEx.replace(QRegExp("[CSGJKQXZ]"), "2");				// [CSGJKQXZ] => 2
			strSoundEx.replace(QRegExp("[DT]"), "3");					// [DT] => 3
			strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
			strSoundEx.replace(QRegExp("[MN]"), "5");					// [MN] => 5
			strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
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

			strSoundEx.replace("CH", "7");								// Proceso letras dobles primero.
			strSoundEx.replace("LL", "7");
			strSoundEx.replace(QRegExp("Y$"), "7");						// Y al final de la palabra.
			strSoundEx.replace(QRegExp("Y(?=[bcdfghjklmnpqrstvwxz])"), "7");	// Y antes de una consonante.
			strSoundEx.replace(QRegExp("AY"), "A7");					// Y después de una vocal
			strSoundEx.replace(QRegExp("EY"), "E7");					//		Note: QRegExp doesn't support look-behind, splitting into individual AEIOU expressions
			strSoundEx.replace(QRegExp("IY"), "I7");
			strSoundEx.replace(QRegExp("OY"), "O7");
			strSoundEx.replace(QRegExp("UY"), "U7");

			strSoundEx.replace(QRegExp("[AEIOUYHW]"), "0");				// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
			strSoundEx.replace(QRegExp("[BPFV]"), "1");					// [BPFV] => 1
			strSoundEx.replace(QRegExp("[CSGJKQXZ]"), "2");				// [CSGJKQXZ] => 2
			strSoundEx.replace(QRegExp("[DT]"), "3");					// [DT] => 3
			strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
			strSoundEx.replace(QRegExp("[MN]"), "5");					// [MN] => 5
			strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6
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

			strSoundEx.replace(QRegExp("[AEIOUYHW]"), "0");				// [AEIOUYHW] => 0 (Special case for doing separation, except H or W as found above)
			strSoundEx.replace(QRegExp("[BPFV]"), "1");					// [BPFV] => 1
			strSoundEx.replace(QRegExp("[CSGJKQXZ]"), "2");				// [CSGJKQXZ] => 2
			strSoundEx.replace(QRegExp("[DT]"), "3");					// [DT] => 3
			strSoundEx.replace(QChar('L'), QChar('4'));					// L => 4
			strSoundEx.replace(QRegExp("[MN]"), "5");					// [MN] => 5
			strSoundEx.replace(QChar('R'), QChar('6'));					// R => 6

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

