/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#include "LetterMatrix.h"

#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/ParseSymbols.h"

#include <QStringList>
#include <QRegularExpression>

#include <iostream>
#include <limits>

#define TEST_MATRIX_INDEXING 0		// Set to '1' to enable matrix index roundtrip testing -- WARNING: This is VERY slow test!

// ============================================================================

namespace {
	const QList<QString> g_conlstBookProloguesKJV =
	{
		"THE FIRST BOOK OF MOSES, CALLED GENESIS.",
		"THE SECOND BOOK OF MOSES, CALLED EXODUS.",
		"THE THIRD BOOK OF MOSES, CALLED LEVITICUS.",
		"THE FOURTH BOOK OF MOSES, CALLED NUMBERS.",
		"THE FIFTH BOOK OF MOSES, CALLED DEUTERONOMY.",
		"THE BOOK OF JOSHUA",
		"THE BOOK OF JUDGES",
		"THE BOOK OF RUTH",
		"THE FIRST BOOK OF SAMUEL, OTHERWISE CALLED, THE FIRST BOOK OF THE KINGS.",
		"THE SECOND BOOK OF SAMUEL, OTHERWISE CALLED, THE SECOND BOOK OF THE KINGS.",
		"THE FIRST BOOK OF THE KINGS, COMMONLY CALLED, THE THIRD BOOK OF THE KINGS.",
		"THE SECOND BOOK OF THE KINGS, COMMONLY CALLED, THE FOURTH BOOK OF THE KINGS.",
		"THE FIRST BOOK OF THE CHRONICLES",
		"THE SECOND BOOK OF THE CHRONICLES",
		"EZRA",
		"THE BOOK OF NEHEMIAH",
		"THE BOOK OF ESTHER",
		"THE BOOK OF JOB",
		"THE BOOK OF PSALMS",
		"THE PROVERBS",
		"ECCLESIASTES OR, THE PREACHER",
		"THE SONG OF SOLOMON",
		"THE BOOK OF THE PROPHET ISAIAH",
		"THE BOOK OF THE PROPHET JEREMIAH",
		"THE LAMENTATIONS OF JEREMIAH",
		"THE BOOK OF THE PROPHET EZEKIEL",
		"THE BOOK OF DANIEL",
		"HOSEA",
		"JOEL",
		"AMOS",
		"OBADIAH",
		"JONAH",
		"MICAH",
		"NAHUM",
		"HABAKKUK",
		"ZEPHANIAH",
		"HAGGAI",
		"ZECHARIAH",
		"MALACHI",
		// ----
		"THE GOSPEL ACCORDING TO ST MATTHEW",
		"THE GOSPEL ACCORDING TO ST MARK",
		"THE GOSPEL ACCORDING TO ST LUKE",
		"THE GOSPEL ACCORDING TO ST JOHN",
		"THE ACTS OF THE APOSTLES",
		"THE EPISTLE OF PAUL THE APOSTLE TO THE ROMANS",
		"THE FIRST EPISTLE OF PAUL THE APOSTLE TO THE CORINTHIANS",
		"THE SECOND EPISTLE OF PAUL THE APOSTLE TO THE CORINTHIANS",
		"THE EPISTLE OF PAUL THE APOSTLE TO THE GALATIANS",
		"THE EPISTLE OF PAUL THE APOSTLE TO THE EPHESIANS",
		"THE EPISTLE OF PAUL THE APOSTLE TO THE PHILIPPIANS",
		"THE EPISTLE OF PAUL THE APOSTLE TO THE COLOSSIANS",
		"THE FIRST EPISTLE OF PAUL THE APOSTLE TO THE THESSALONIANS",
		"THE SECOND EPISTLE OF PAUL THE APOSTLE TO THE THESSALONIANS",
		"THE FIRST EPISTLE OF PAUL THE APOSTLE TO TIMOTHY",
		"THE SECOND EPISTLE OF PAUL THE APOSTLE TO TIMOTHY",
		"THE EPISTLE OF PAUL TO TITUS",
		"THE EPISTLE OF PAUL TO PHILEMON",
		"THE EPISTLE OF PAUL THE APOSTLE TO THE HEBREWS",
		"THE GENERAL EPISTLE OF JAMES",
		"THE FIRST EPISTLE GENERAL OF PETER",
		"THE SECOND EPISTLE GENERAL OF PETER",
		"THE FIRST EPISTLE GENERAL OF JOHN",
		"THE SECOND EPISTLE OF JOHN",
		"THE THIRD EPISTLE OF JOHN",
		"THE GENERAL EPISTLE OF JUDE",
		"THE REVELATION OF ST JOHN THE DIVINE",
		// ----
		"THE FIRST BOOK OF ESDRAS",
		"THE SECOND BOOK OF ESDRAS",
		"TOBIT",
		"JUDITH",
		"THE REST OF THE CHAPTERS OF THE BOOK OF ESTHER, WHICH ARE FOUND NEITHER IN THE HEBREW, NOR IN THE CHALDEE",
		"THE WISDOM OF SOLOMON",
		"THE WISDOM OF JESUS THE SON OF SIRACH, OR, ECCLESIASTICUS",
		"BARUCH",
		"THE SONG OF THE THREE HOLY CHILDREN",
		"THE HISTORY OF SUSANNA",
		"THE HISTORY OF THE DESTRUCTION OF BEL AND THE DRAGON",
		"THE PRAYER OF MANASSES KING OF JUDA WHEN HE WAS HOLDEN CAPTIVE IN BABYLON",
		"THE FIRST BOOK OF THE MACCABEES",
		"THE SECOND BOOK OF THE MACCABEES",
	};

	const QList<QString> g_conlstBookPrologues1611 =
	{
		"THE FIRST BOOKE OF MOSES, called GENESIS.",
		"THE SECOND BOOKE OF Moses, called Exodus.",
		QString(QChar(0x00B6)) + " THE THIRD BOOKE of Moses, called Leuiticus.",
		QString(QChar(0x00B6)) + " THE FOVRTH BOOKE of Moses, called Numbers.",
		QString(QChar(0x00B6)) + " THE FIFTH BOOKE OF Moses, called Deuteronomie.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Ioshua.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Iudges.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Ruth.",
		QString(QChar(0x00B6)) + " THE FIRST BOOKE of Samuel, otherwise called, The first Booke of the Kings.",
		QString(QChar(0x00B6)) + " THE SECOND BOOKE of Samuel, otherwise called, The second Booke of the Kings.",
		QString(QChar(0x00B6)) + " THE FIRST BOOKE OF the Kings, commonly called The third Booke of the Kings.",
		QString(QChar(0x00B6)) + " THE SECOND BOOKE of the Kings, commonly called, The fourth Booke of the Kings.",
		QString(QChar(0x00B6)) + " THE FIRST BOOKE of the Chronicles.",
		QString(QChar(0x00B6)) + " THE SECOND BOOKE of the Chronicles.",
		QString(QChar(0x00B6)) + " EZRA.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Nehemiah.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Esther.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Iob.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Psalmes.",
		QString(QChar(0x00B6)) + " THE PROVERBES.",
		QString(QChar(0x00B6)) + " ECCLESIASTES, or the Preacher.",
		QString(QChar(0x00B6)) + " The Song of Solomon.",
		QString(QChar(0x00B6)) + " THE BOOKE OF THE Prophet Isaiah.",
		QString(QChar(0x00B6)) + " THE BOOKE OF THE Prophet Ieremiah.",
		QString(QChar(0x00B6)) + " The Lamentations of Ieremiah.",
		QString(QChar(0x00B6)) + " THE BOOKE OF THE Prophet Ezekiel.",
		QString(QChar(0x00B6)) + " THE BOOKE OF Daniel.",
		QString(QChar(0x00B6)) + " HOSEA.",
		QString(QChar(0x00B6)) + " IOEL.",
		QString(QChar(0x00B6)) + " AMOS.",
		QString(QChar(0x00B6)) + " OBADIAH.",
		QString(QChar(0x00B6)) + " IONAH.",
		QString(QChar(0x00B6)) + " MICAH.",
		QString(QChar(0x00B6)) + " NAHVM.",
		QString(QChar(0x00B6)) + " HABAKKVK.",
		QString(QChar(0x00B6)) + " ZEPHANIAH.",
		QString(QChar(0x00B6)) + " HAGGAI.",
		QString(QChar(0x00B6)) + " ZECHARIAH.",
		QString(QChar(0x00B6)) + " MALACHI.",
		// ----
		"THE GOSPEL ACCORDING to S.Matthew.",
		QString(QChar(0x00B6)) + " The Gospel according to S.Marke.",
		QString(QChar(0x00B6)) + " The Gospel according to S.Luke.",
		QString(QChar(0x00B6)) + " The Gospel according to S. Iohn.",
		QString(QChar(0x00B6)) + " THE ACTES OF the Apostles.",
		"THE EPISTLE OF PAVL THE Apostle to the Romanes.",
		QString(QChar(0x00B6)) + " THE FIRST EPISTLE of Paul the Apostle to the Corinthians.",
		QString(QChar(0x00B6)) + " THE SECOND EPISTLE of Paul the Apostle to the Corinthians.",
		QString(QChar(0x00B6)) + " THE EPISTLE OF Paul to the Galatians.",
		QString(QChar(0x00B6)) + " THE EPISTLE OF PAVL the Apostle to the Ephesians.",
		QString(QChar(0x00B6)) + " THE EPISTLE OF PAVL the Apostle to the Philippians.",
		QString(QChar(0x00B6)) + " THE EPISTLE OF PAVL the Apostle to the Colossians.",
		QString(QChar(0x00B6)) + " THE FIRST EPISTLE OF Paul the Apostle to the Thessalonians.",
		QString(QChar(0x00B6)) + " THE SECOND EPISTLE of Paul the Apostle to the Thessalonians.",
		QString(QChar(0x00B6)) + " THE FIRST EPISTLE of Paul the Apostle to Timothie.",
		QString(QChar(0x00B6)) + " THE SECOND EPISTLE of Paul the Apostle to Timothie.",
		QString(QChar(0x00B6)) + " THE EPISTLE OF Paul to Titus.",
		QString(QChar(0x00B6)) + " THE EPISTLE OF Paul to Philemon.",
		QString(QChar(0x00B6)) + " THE EPISTLE OF PAVL the Apostle to the Hebrewes.",
		QString(QChar(0x00B6)) + " THE GENERALL Epistle of Iames.",
		QString(QChar(0x00B6)) + " THE FIRST EPISTLE generall of Peter.",
		QString(QChar(0x00B6)) + " THE SECOND EPISTLE generall of Peter.",
		QString(QChar(0x00B6)) + " THE FIRST EPISTLE generall of Iohn.",
		QString(QChar(0x00B6)) + " The second Epistle of Iohn.",
		QString(QChar(0x00B6)) + " The third Epistle of Iohn.",
		QString(QChar(0x00B6)) + " THE GENERALL Epistle of Iude.",
		QString(QChar(0x00B6)) + " THE REVELATION of S.Iohn the Divine.",
		// ----
		QString(QChar(0x00B6)) + " I. ESDRAS.",
		QString(QChar(0x00B6)) + " II. ESDRAS.",
		QString(QChar(0x00B6)) + " TOBIT.",
		QString(QChar(0x00B6)) + " IVDETH.",
		QString(QChar(0x00B6)) + " The rest of the Chapters of the Booke of Esther, which are found neither in the Hebrew, nor in the Calde.",
		QString(QChar(0x00B6)) + " The Wisedome of Solomon.",
		QString(QChar(0x00B6)) + " THE WISDOME OF Iesus the sonne of Sirach, Or Ecclesiasticus.",
		QString(QChar(0x00B6)) + " BARVCH.",
		QString(QChar(0x00B6)) + " The Song of the three holy children, which followeth in the third Chapter of Daniel after this place, [And they walked in the midst of the fire, praising God, and blessing the Lord.] That which followeth is not in the Hebrew; to wit [Then Azarias stood vp] vnto these wordes, [And Nabuchodonosor.]",
		QString(QChar(0x00B6)) + " The historie of Susanna, set apart from the beginning of Daniel, because it is not in Hebrew, as neither the narration of Bel and the Dragon.",
		QString(QChar(0x00B6)) + " The history of the destruction of Bel and the Dragon, cut off from the end of Daniel.",
		QString(QChar(0x00B6)) + " The Prayer of Manasses King of Iuda, when he was holden captiue in Babylon.",
		QString(QChar(0x00B6)) + " The first booke of the Maccabees.",
		QString(QChar(0x00B6)) + " The second booke of the Maccabees.",
	};

	// ------------------------------------------------------------------------

	const QString g_constrNoNumerals = QObject::tr("No Numerals", "CLetterMatrix");
	const QString g_constrNoNums = QObject::tr("NoNums", "CLetterMatrix");

	const QString g_constrRomanNumerals = QObject::tr("Roman Numerals", "CLetterMatrix");
	const QString g_constrRomanNums = QObject::tr("Roman", "CLetterMatrix");

	const QString g_constrArabicNumerals = QObject::tr("Arabic Numerals", "CLetterMatrix");
	const QString g_constrArabicNums = QObject::tr("Arabic", "CLetterMatrix");

}		// Namespace

// ============================================================================

// intToRoman:
//	Convert integer to Roman Numeral.
//	Note: b1611Style drops the 'IV' for '4'.  For some reason
//	they wrote the number '4' as 'IIII', like 'XXIIII', etc.
static QString intToRoman(int num, bool b1611Style)
{
	if ((num <= 0) || (num > 3999)) QString();

	QString result;

	if (!b1611Style) {
		const int values[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
		const QString numerals[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
		Q_ASSERT(_countof(values) == _countof(numerals));

		for (unsigned int i = 0; i < _countof(values); ++i) {
			while (num >= values[i]) {
				result += numerals[i];
				num -= values[i];
			}
		}
	} else {
		const int values1611[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 1};
		const QString numerals1611[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "I"};
		Q_ASSERT(_countof(values1611) == _countof(numerals1611));

		for (unsigned int i = 0; i < _countof(values1611); ++i) {
			while (num >= values1611[i]) {
				result += numerals1611[i];
				num -= values1611[i];
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

static QString ps119Prologue(uint32_t nVerse, LMVersePrologueOptionFlags nFlags, bool b1611)
{
	if (((nVerse-1) % 8) != 0) return QString();

	QString strHebrewPrefix;

	if (nFlags.testFlag(LMVPO_PS119_HebrewLetter)) {
		switch ((nVerse-1)/8) {
			case 0:
				// ALEPH
				strHebrewPrefix += QChar(0x005D0);
				break;
			case 1:
				// BETH
				strHebrewPrefix += QChar(0x005D1);
				break;
			case 2:
				// GIMEL
				strHebrewPrefix += QChar(0x005D2);
				break;
			case 3:
				// DALETH
				strHebrewPrefix += QChar(0x005D3);
				break;
			case 4:
				// HE
				strHebrewPrefix += QChar(0x005D4);
				break;
			case 5:
				// VAU
				strHebrewPrefix += QChar(0x005D5);
				break;
			case 6:
				// ZAIN
				strHebrewPrefix += QChar(0x005D6);
				break;
			case 7:
				// CHETH
				strHebrewPrefix += QChar(0x005D7);
				break;
			case 8:
				// TETH
				strHebrewPrefix += QChar(0x005D8);
				break;
			case 9:
				// JOD
				strHebrewPrefix += QChar(0x005D9);
				break;
			case 10:
				// CAPH
				strHebrewPrefix += QChar(0x005DB);		// Using nonfinal-CAPH
				break;
			case 11:
				// LAMED
				strHebrewPrefix += QChar(0x005DC);
				break;
			case 12:
				// MEM
				strHebrewPrefix += QChar(0x005DE);		// Using nonfinal-Mem
				break;
			case 13:
				// NUN
				strHebrewPrefix += QChar(0x005E0);		// Using nonfinal-Nun
				break;
			case 14:
				// SAMECH
				strHebrewPrefix += QChar(0x005E1);
				break;
			case 15:
				// AIN
				strHebrewPrefix += QChar(0x005E2);
				break;
			case 16:
				// PE
				strHebrewPrefix += QChar(0x005E4);		// Using nonfinal-Pe
				break;
			case 17:
				// TZADDI
				strHebrewPrefix += QChar(0x005E6);		// Using nonfinal-Tzaddi
				break;
			case 18:
				// KOPH
				strHebrewPrefix += QChar(0x005E7);
				break;
			case 19:
				// RESH
				strHebrewPrefix += QChar(0x005E8);
				break;
			case 20:
				// SCHIN
				strHebrewPrefix += QChar(0x005E9);
				break;
			case 21:
				// TAU
				strHebrewPrefix += QChar(0x005EA);
				break;
		}
		strHebrewPrefix += " ";
	}

	if (nFlags.testFlag(LMVPO_PS119_Transliteration)) {
		switch ((nVerse-1)/8) {
			case 0:
				strHebrewPrefix += "ALEPH";
				break;
			case 1:
				strHebrewPrefix += "BETH";
				break;
			case 2:
				strHebrewPrefix += "GIMEL";
				break;
			case 3:
				strHebrewPrefix += "DALETH";
				break;
			case 4:
				strHebrewPrefix += "HE";
				break;
			case 5:
				strHebrewPrefix += (!b1611 ? "VAU" : "VAV");
				break;
			case 6:
				strHebrewPrefix += "ZAIN";
				break;
			case 7:
				strHebrewPrefix += "CHETH";
				break;
			case 8:
				strHebrewPrefix += "TETH";
				break;
			case 9:
				strHebrewPrefix += (!b1611 ? "JOD" : "IOD");
				break;
			case 10:
				strHebrewPrefix += "CAPH";
				break;
			case 11:
				strHebrewPrefix += "LAMED";
				break;
			case 12:
				strHebrewPrefix += "MEM";
				break;
			case 13:
				strHebrewPrefix += (!b1611 ? "NUN" : "NVN");
				break;
			case 14:
				strHebrewPrefix += "SAMECH";
				break;
			case 15:
				strHebrewPrefix += "AIN";
				break;
			case 16:
				strHebrewPrefix += "PE";
				break;
			case 17:
				strHebrewPrefix += (!b1611 ? "TZADDI" : "TSADDI");
				break;
			case 18:
				strHebrewPrefix += "KOPH";
				break;
			case 19:
				strHebrewPrefix += "RESH";
				break;
			case 20:
				strHebrewPrefix += "SCHIN";
				break;
			case 21:
				strHebrewPrefix += (!b1611 ? "TAU" : "TAV");
				break;
		}
	}

	if (nFlags.testFlag(LMVPO_PS119_Punctuation)) strHebrewPrefix += ".";
	strHebrewPrefix += " ";

	return strHebrewPrefix;
}

// ----------------------------------------------------------------------------

QString chapterNumber(const QString &strPrefix, LMChapterPrologueOptionFlags flagsCPO, uint32_t nNumber, bool b1611)
{
	QString strChapNumber;
	switch (flagsCPO & LMCPO_NumberOptionsMask) {
		case LMCPO_NumbersRoman:
			strChapNumber = intToRoman(nNumber, b1611);
			break;

		case LMCPO_NumbersArabic:
			strChapNumber = QString::number(nNumber);
			break;

		case LMCPO_NumbersNone:
		default:
			break;
	}

	return strPrefix + ((!strPrefix.isEmpty() && !strChapNumber.isEmpty()) ? " " : "") + strChapNumber;
}

QString chapterPsalmBookNumber(const QString &strPrefix, LMChapterPrologueOptionFlags flagsCPO, uint32_t nNumber, bool b1611)
{
	QString strChapNumber;
	switch (flagsCPO & LMCPO_PsalmBookNumberOptionsMask) {
		case LMCPO_PsalmBookNumbersRoman:
			strChapNumber = intToRoman(nNumber, b1611);
			break;

		case LMCPO_PsalmBookNumbersArabic:
			strChapNumber = QString::number(nNumber);
			break;

		case LMCPO_PsalmBookNumbersNone:
		default:
			break;
	}

	return strPrefix + ((!strPrefix.isEmpty() && !strChapNumber.isEmpty()) ? " " : "") + strChapNumber;
}

QString verseNumber(const QString &strPrefix, LMVersePrologueOptionFlags flagsVPO, uint32_t nNumber, bool b1611)
{
	QString strVrsNumber;
	switch (flagsVPO & LMVPO_NumberOptionsMask) {
		case LMVPO_NumbersRoman:
			strVrsNumber = intToRoman(nNumber, b1611);
			break;

		case LMVPO_NumbersArabic:
			strVrsNumber = QString::number(nNumber);
			break;

		case LMVPO_NumbersNone:
		default:
			break;
	}

	return strPrefix + ((!strPrefix.isEmpty() && !strVrsNumber.isEmpty()) ? " " : "") + strVrsNumber;
}

// ============================================================================

// Words of Jesus extractor variant of Verse Text Richifier Tags:
class CWordsOfJesusExtractor : public CVerseTextPlainRichifierTags
{
public:
	CWordsOfJesusExtractor(QStringList &lstWordsOfJesus)
		:	m_lstWordsOfJesus(lstWordsOfJesus)
	{
		setTransChangeAddedTags(QString(), QString());
	}
protected:
	virtual void wordCallback(const QString &strWord, VerseWordTypeFlags nWordTypes) const override
	{
		if (nWordTypes & VWT_WordsOfJesus) {
			m_lstWordsOfJesus.append(strWord);
		} else {
			m_lstWordsOfJesus.append(QString());
		}
	}
private:
	QStringList &m_lstWordsOfJesus;
};

// ----------------------------------------------------------------------------

CLetterMatrix::CLetterMatrix(CBibleDatabasePtr pBibleDatabase,
							 LetterMatrixTextModifierOptionFlags flagsLMTMO,
							 LMBookPrologueOptionFlags flagsLMBPO,
							 LMChapterPrologueOptionFlags flagsLMCPO,
							 LMVersePrologueOptionFlags flagsLMVPO,
							 LMFullVerseTextOptionFlags flagsLMFVTO)
	:	m_pBibleDatabase(pBibleDatabase),
		m_flagsLMTMO(flagsLMTMO),
		m_flagsLMBPO(flagsLMBPO),
		m_flagsLMCPO(flagsLMCPO),
		m_flagsLMVPO(flagsLMVPO),
		m_flagsLMFVTO(flagsLMFVTO)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	bool bIsKJV = false;			// Used for prologues
	bool bIs1611 = false;			// Used for weird 1611 prologue variants

	if ((pBibleDatabase->descriptor().m_strUUID.compare(bibleDescriptor(BDE_KJV).m_strUUID, Qt::CaseInsensitive) == 0) ||
		(pBibleDatabase->descriptor().m_strUUID.compare(bibleDescriptor(BDE_KJVPCE).m_strUUID, Qt::CaseInsensitive) == 0) ||
		(pBibleDatabase->descriptor().m_strUUID.compare(bibleDescriptor(BDE_KJVA).m_strUUID, Qt::CaseInsensitive) == 0)) {
		bIsKJV = true;
		Q_ASSERT(g_conlstBookProloguesKJV.size() == 80);		// 39 + 27 + 14
	} else if ((pBibleDatabase->descriptor().m_strUUID.compare(bibleDescriptor(BDE_KJV1611).m_strUUID, Qt::CaseInsensitive) == 0) ||
			   (pBibleDatabase->descriptor().m_strUUID.compare(bibleDescriptor(BDE_KJV1611A).m_strUUID, Qt::CaseInsensitive) == 0)) {
		bIs1611 = true;
		Q_ASSERT(g_conlstBookPrologues1611.size() == 80);		// 39 + 27 + 14
	}

	// Create giant array of all letters from the Bible text for speed:
	//	NOTE: This is with the entire Bible content (sans colophons/
	//	superscriptions when they are skipped) and not the search span
	//	so that we don't have to convert the matrix index based on the
	//	search span.
	reserve(pBibleDatabase->bibleEntry().m_nNumLtr + 1);		// +1 since we reserve the 0 entry (Note this fails to preallocate space for prologues since we don't know how big they will be yet)
	append(QChar());
	CRelIndex ndxMatrixCurrent = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_Start);
	CRelIndex ndxMatrixEnd = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_End);
	uint32_t normalMatrixEnd = pBibleDatabase->NormalizeIndex(ndxMatrixEnd);
	QList<QChar> lstColophon;
	CRelIndex ndxMatrixLastColophon;
	QList<QChar> lstSuperscription;
	CRelIndex ndxMatrixLastSuperscription;
	uint32_t nWordsOfJesusLetterSkip = 0;		// Number of letters to skip prior to words of Jesus
	QStringList lstWordsOfJesus;
	CWordsOfJesusExtractor vtrtWordsOfJesus(lstWordsOfJesus);
	CVerseTextPlainRichifierTags vtrtFullText;
	if (m_flagsLMFVTO.testFlag(LMFVTO_NoBracketsForTransChange)) vtrtFullText.setTransChangeAddedTags(QString(), QString());
	vtrtFullText.setShowPilcrowMarkers(m_flagsLMFVTO.testFlag(LMFVTO_IncludePilcrowMarkers));
	CRelIndex ndxMatrixLastPrologue;
	for (uint32_t normalMatrixCurrent = pBibleDatabase->NormalizeIndex(ndxMatrixCurrent);
		 normalMatrixCurrent <= normalMatrixEnd; ++normalMatrixCurrent) {
		ndxMatrixCurrent = pBibleDatabase->DenormalizeIndex(normalMatrixCurrent);

		// Transfer any pending colophon if book changes:
		if (!lstColophon.isEmpty() && (ndxMatrixLastColophon.book() != ndxMatrixCurrent.book())) {
			if (!m_flagsLMTMO.testFlag(LMTMO_RemoveColophons)) {
				if (isFTMode()) {
					m_mapFullVerseText[ndxMatrixLastColophon].m_nMatrixIndex = size();
					m_mapFullTextMatrixIndexToRelIndex[size()] = ndxMatrixLastColophon;
				}
				append(lstColophon);
			} else {
				if (!isFTMode()) {
					m_mapMatrixIndexToLetterShift[size()] += lstColophon.size();
				}
			}
			lstColophon.clear();
			ndxMatrixLastColophon.clear();
		}

		// Check for skipped superscription to map:
		if (!lstSuperscription.isEmpty() && (ndxMatrixLastSuperscription.verse() != ndxMatrixCurrent.verse())) {
			Q_ASSERT(m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions));		// Should only be here if actually skipping the superscriptions
			m_mapMatrixIndexToLetterShift[size()] += lstSuperscription.size();
			lstSuperscription.clear();
			ndxMatrixLastSuperscription.clear();
		}

		// Add prologues first:
		bool bAddedPrologue = false;
		if (ndxMatrixCurrent.book() != ndxMatrixLastPrologue.book()) {		// Check entering new book
			TPrologueEntry entryPrologue;
			entryPrologue.m_ndxPrologue = CRelIndex(ndxMatrixCurrent.book(), 0, 0, 0);

			if (bIsKJV && (ndxMatrixCurrent.book() <= static_cast<unsigned int>(g_conlstBookProloguesKJV.size()))) {
				entryPrologue.m_strPrologue = g_conlstBookProloguesKJV.at(ndxMatrixCurrent.book()-1);
			} else if (bIs1611 && (ndxMatrixCurrent.book()<= static_cast<unsigned int>(g_conlstBookPrologues1611.size()))) {
				entryPrologue.m_strPrologue = g_conlstBookPrologues1611.at(ndxMatrixCurrent.book()-1);
			}
			entryPrologue.m_strPrologue.remove(
				QRegularExpression(m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation) ? "\\s" : "[^a-zA-Z]"));

			if (!entryPrologue.m_strPrologue.isEmpty()) {
				if (m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues) && !m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
					m_mapPrologueRelIndexToMatrixIndex[entryPrologue.m_ndxPrologue] = size();
					m_mapMatrixIndexToPrologue[size()] = entryPrologue;								// Add Book Prologue to the prologue map
					for (auto const &chrLetter : entryPrologue.m_strPrologue) append(chrLetter);	// Add Book Prologue to the matrix
				}
			}
			bAddedPrologue = true;
			ndxMatrixLastPrologue.setChapter(0);		// Force enter new chapter if this is a new book -- without this, we miss chapters of books with only 1 chapter
			ndxMatrixLastPrologue.setVerse(0);			// Force enter new verse if this is a new book
		}
		if ((ndxMatrixCurrent.chapter() != ndxMatrixLastPrologue.chapter()) &&
			(ndxMatrixCurrent.chapter() != 0)) {							// Check entering new chapter
			TPrologueEntry entryPrologue;
			entryPrologue.m_ndxPrologue = CRelIndex(ndxMatrixCurrent.book(), ndxMatrixCurrent.chapter(), 0, 0);

			if (bIsKJV || bIs1611) {
				if (ndxMatrixCurrent.book() == PSALMS_BOOK_NUM) {
					if (m_flagsLMCPO.testFlag(LMCPO_PsalmBookTags)) {
						if (ndxMatrixCurrent.chapter() == 1) {
							entryPrologue.m_strPrologue += chapterPsalmBookNumber("BOOK", m_flagsLMCPO, 1, bIs1611);
						} else if (ndxMatrixCurrent.chapter() == 42) {
							entryPrologue.m_strPrologue += chapterPsalmBookNumber("BOOK", m_flagsLMCPO, 2, bIs1611);
						} else if (ndxMatrixCurrent.chapter() == 73) {
							entryPrologue.m_strPrologue += chapterPsalmBookNumber("BOOK", m_flagsLMCPO, 3, bIs1611);
						} else if (ndxMatrixCurrent.chapter() == 90) {
							entryPrologue.m_strPrologue += chapterPsalmBookNumber("BOOK", m_flagsLMCPO, 4, bIs1611);
						} else if (ndxMatrixCurrent.chapter() == 107) {
							entryPrologue.m_strPrologue += chapterPsalmBookNumber("BOOK", m_flagsLMCPO, 5, bIs1611);
						}
					}
				}
			}

			if (bIsKJV) {
				if (ndxMatrixCurrent.book() == PSALMS_BOOK_NUM) {
					entryPrologue.m_strPrologue += chapterNumber("PSALM", m_flagsLMCPO, ndxMatrixCurrent.chapter(), bIs1611);
				} else {
					entryPrologue.m_strPrologue += chapterNumber("CHAPTER", m_flagsLMCPO, ndxMatrixCurrent.chapter(), bIs1611);
				}
			} else if (bIs1611) {
				if (ndxMatrixCurrent.book() == PSALMS_BOOK_NUM) {
					if (ndxMatrixCurrent.chapter() == 1) {
						entryPrologue.m_strPrologue += chapterNumber("PSALME", m_flagsLMCPO, ndxMatrixCurrent.chapter(), bIs1611);
					} else {
						entryPrologue.m_strPrologue += chapterNumber("PSAL.", m_flagsLMCPO, ndxMatrixCurrent.chapter(), bIs1611);
					}
				} else {
					entryPrologue.m_strPrologue += chapterNumber("CHAP.", m_flagsLMCPO, ndxMatrixCurrent.chapter(), bIs1611) + ".";
				}
			}
			entryPrologue.m_strPrologue.remove(
				QRegularExpression(m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation) ? "\\s" : "[^a-zA-Z0-9]"));

			if (!entryPrologue.m_strPrologue.isEmpty()) {
				if (m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues) && !m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
					m_mapPrologueRelIndexToMatrixIndex[entryPrologue.m_ndxPrologue] = size();
					m_mapMatrixIndexToPrologue[size()] = entryPrologue;								// Add Chapter Prologue to the prologue map
					for (auto const &chrLetter : entryPrologue.m_strPrologue) append(chrLetter);	// Add Chapter Prologue to the matrix
				}
			}
			bAddedPrologue = true;
			ndxMatrixLastPrologue.setVerse(0);			// Force enter new verse if this is a new chapter
		}
		if ((ndxMatrixCurrent.verse() != ndxMatrixLastPrologue.verse()) &&
			(ndxMatrixCurrent.verse() != 0)) {							// Check entering new verse
			TPrologueEntry entryPrologue;
			entryPrologue.m_ndxPrologue = CRelIndex(ndxMatrixCurrent.book(), ndxMatrixCurrent.chapter(), ndxMatrixCurrent.verse(), 0);

			if (bIsKJV || bIs1611) {
				if ((ndxMatrixCurrent.book() == PSALMS_BOOK_NUM) && (ndxMatrixCurrent.chapter() == 119)) {
					entryPrologue.m_strPrologue += ps119Prologue(ndxMatrixCurrent.verse(), m_flagsLMVPO, bIs1611);
				}
				entryPrologue.m_strPrologue += verseNumber(QString(), m_flagsLMVPO, ndxMatrixCurrent.verse(), bIs1611);
			}
			entryPrologue.m_strPrologue.remove(
				QRegularExpression(m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation) ? QString("\\s") :
									   (QString("[^a-zA-Z0-9") + QChar(0x005D0) + "-" + QChar(0x005EA) + "]")));

			if (!entryPrologue.m_strPrologue.isEmpty()) {
				if (m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues) && !m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
					m_mapPrologueRelIndexToMatrixIndex[entryPrologue.m_ndxPrologue] = size();
					m_mapMatrixIndexToPrologue[size()] = entryPrologue;								// Add Verse Prologue to the prologue map
					for (auto const &chrLetter : entryPrologue.m_strPrologue) append(chrLetter);	// Add Verse Prologue to the matrix
				}
			}
			bAddedPrologue = true;
		}
		if (bAddedPrologue) ndxMatrixLastPrologue = ndxMatrixCurrent;

		if (!isFTMode() || m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
			const CConcordanceEntry *pWordEntry = pBibleDatabase->concordanceEntryForWordAtIndex(ndxMatrixCurrent);
			Q_ASSERT(pWordEntry != nullptr);
			if (pWordEntry) {
				QString strWord = pWordEntry->rawWord();
				if (m_flagsLMTMO.testFlag(LMTMO_DecomposeLetters)) strWord = StringParse::decompose(strWord, false);

				if (!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
					// Since the Words of Jesus can never be in Colophons or Superscriptions,
					//	we only need to shift the colophons to their correct position when
					//	searching everything:
					if (!ndxMatrixCurrent.isColophon()) {
						if (!ndxMatrixCurrent.isSuperscription() || !m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions)) {
							// Output the book as-is without shuffling:
							for (auto const &chrLetter : strWord) append(chrLetter);
						} else {
							// If skipping superscriptions, put them on a local buffer like
							//	we did with colophons so that when we hit the next "verse"
							//	(i.e. exit the superscription), we can insert a MatrixIndex
							//	to LetterShift mapping.  Note that unlike colophons, since
							//	we aren't moving the superscription itself, this buffer is
							//	never transferred to the matrix:
							for (auto const &chrLetter : strWord) lstSuperscription.append(chrLetter);
							ndxMatrixLastSuperscription = ndxMatrixCurrent;
						}
					} else {
						// Output colophon to temp buff:
						for (auto const &chrLetter : strWord) lstColophon.append(chrLetter);
						ndxMatrixLastColophon = ndxMatrixCurrent;
					}
				} else {
					// Trigger per-verse rendering on first word of verse:
					if (ndxMatrixCurrent.word() == 1) {
						const CVerseEntry *pVerse = pBibleDatabase->verseEntry(ndxMatrixCurrent);
						Q_ASSERT(pVerse != nullptr);
						if (pVerse == nullptr) continue;

						lstWordsOfJesus.clear();
						pBibleDatabase->richVerseText(ndxMatrixCurrent, vtrtWordsOfJesus);
						Q_ASSERT(pVerse->m_nNumWrd == static_cast<unsigned int>(lstWordsOfJesus.size()));
					}
					Q_ASSERT(static_cast<unsigned int>(lstWordsOfJesus.size()) >= ndxMatrixCurrent.word());
					Q_ASSERT(ndxMatrixCurrent.word() >= 1);

					if (!lstWordsOfJesus.at(ndxMatrixCurrent.word()-1).isEmpty()) {
						// If in Words of Jesus, output letter skip for main text and append words to matrix:
						if (nWordsOfJesusLetterSkip) {
							m_mapMatrixIndexToLetterShift[size()] += nWordsOfJesusLetterSkip;
							nWordsOfJesusLetterSkip = 0;
						}
						for (auto const &chrLetter : strWord) append(chrLetter);
					} else {
						// Otherwise, plan to skip the letters that aren't:
						nWordsOfJesusLetterSkip += strWord.size();
					}
				}
			}
		} else {
			QString strFullVerseText = pBibleDatabase->richVerseText(ndxMatrixCurrent, vtrtFullText);
			strFullVerseText.remove(QRegularExpression("\\s"));		// Remove spaces in verse rendering
			strFullVerseText = StringParse::deCantillate(strFullVerseText);				// Decantillate first in case that's not the current Bible database setting, so we match the "search word" part of the logic
			if (m_flagsLMTMO.testFlag(LMTMO_DecomposeLetters)) {
				strFullVerseText = StringParse::decompose(strFullVerseText, false);		// Decompose things like ligatures and remove accents, but leave apostrophes and hyphens
			} else {
				strFullVerseText = StringParse::reduce(strFullVerseText, false);		// Remove free-standing marks (like Hebrew Sheva)
			}

			if (!ndxMatrixCurrent.isColophon()) {
				if (!ndxMatrixCurrent.isSuperscription() || !m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions)) {
					// Output the book as-is without shuffling:
					Q_ASSERT(ndxMatrixCurrent.word() == 1);
					m_mapFullVerseText[ndxMatrixCurrent].m_strVerseText = strFullVerseText;
					m_mapFullVerseText[ndxMatrixCurrent].m_nMatrixIndex = size();
					m_mapFullTextMatrixIndexToRelIndex[size()] = ndxMatrixCurrent;
					for (auto const &chrLetter : strFullVerseText) append(chrLetter);
				}	// Else case would be "superscriptions" that we are completely skipping
			} else {
				// Output colophon to temp buff for shuffling:
				for (auto const &chrLetter : strFullVerseText) lstColophon.append(chrLetter);
				ndxMatrixLastColophon = ndxMatrixCurrent;
				m_mapFullVerseText[ndxMatrixCurrent].m_strVerseText = strFullVerseText;
				// Note: We'll populate m_mapFullTextMatrixIndexToRelIndex when we transfer this buffer to the matrix above
			}

			const CVerseEntry *pVerseEntry = pBibleDatabase->verseEntry(ndxMatrixCurrent);
			Q_ASSERT(pVerseEntry != nullptr);
			if (pVerseEntry) normalMatrixCurrent += pVerseEntry->m_nNumWrd-1;	// -1 to index us to the last word of this verse.  The loop will increment us to the next verse.
		}
	}
	if (m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && (nWordsOfJesusLetterSkip != 0)) {
		m_mapMatrixIndexToLetterShift[size()] += nWordsOfJesusLetterSkip;
		nWordsOfJesusLetterSkip = 0;
	}

	// Verify the matrix size as a sanity check for code bugs:
	if (!isFTMode() || m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
		CRelIndexEx ndxLast = pBibleDatabase->calcRelIndex(CRelIndex(), CBibleDatabase::RIME_End);
		const CConcordanceEntry *pceLastWord = pBibleDatabase->concordanceEntryForWordAtIndex(ndxLast);
		if (pceLastWord) ndxLast.setLetter(pceLastWord->letterCount());
		uint32_t matrixIndexLast = matrixIndexFromRelIndex(ndxLast);
		Q_ASSERT((matrixIndexLast+1) == static_cast<uint32_t>(size()));
	} else {
		// TODO : Figure out how to do a sanity check for full-text mode
	}

	// Additional Matrix Index Roundtrip Test for debugging:
#if TEST_MATRIX_INDEXING
	runMatrixIndexRoundtripTest();
#endif
}

bool CLetterMatrix::runMatrixIndexRoundtripTest() const
{
	for (uint32_t ndx = 1; ndx < static_cast<uint32_t>(size()); ++ndx) {
#if !TEST_MATRIX_INDEXING
		if ((ndx % 10000) == 0) {
			std::cerr << ".";
			std::cerr.flush();
		}
#endif
		CRelIndexEx relIndex = relIndexFromMatrixIndex(ndx);
		uint32_t ndxTest = matrixIndexFromRelIndex(relIndex);		// Separate variable for debug viewing
		if (ndx != ndxTest) {
			QString strTemp;
			for (uint32_t i = 0; ((i < 32) && (i < static_cast<uint32_t>(size()))); ++i) strTemp += at(ndx+i);
#if TEST_MATRIX_INDEXING
			qDebug("Real Index:     %d : %s", ndx, strTemp.toUtf8().data());
			qDebug("relIndex: 0x%s +0x%s", QString("%1").arg(relIndex.index(), 8, 16, QChar('0')).toUpper().toUtf8().data(),
					QString("%1").arg(relIndex.letter(), 2, 16, QChar('0')).toUpper().toUtf8().data());
#else
			std::cerr << "\n";
			std::cerr << "Real Index:     " << (unsigned int)(ndx) << " : " << strTemp.toUtf8().data() << std::endl;
			std::cerr << "relIndex: 0x" << QString("%1").arg(relIndex.index(), 8, 16, QChar('0')).toUpper().toUtf8().data()
					  << " +0x" << QString("%1").arg(relIndex.letter(), 2, 16, QChar('0')).toUpper().toUtf8().data() << std::endl;
#endif
			strTemp.clear();
			for (uint32_t i = 0; ((i < 32) && (i < static_cast<uint32_t>(size()))); ++i) strTemp += at(ndxTest+i);
#if TEST_MATRIX_INDEXING
			qDebug("Resolved Index: %d : %s", ndxTest, strTemp.toUtf8().data());
#else
			std::cerr << "Resolved Index: " << (unsigned int)(ndxTest) << " : " << strTemp.toUtf8().data() << std::endl;
#endif

			// Perform again as a convenient place to attach a breakpoint and watch what happens:
			CRelIndexEx relRedo = relIndexFromMatrixIndex(ndx);
			uint32_t ndxTest2 = matrixIndexFromRelIndex(relRedo);
			Q_UNUSED(ndxTest2);
#if TEST_MATRIX_INDEXING
			Q_ASSERT(false);
#endif
			return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------------

uint32_t CLetterMatrix::letterCountForFullVerse(const CRelIndex nRelIndex) const
{
	Q_ASSERT(isFTMode());
	TMapFullVerseText::const_iterator itrFullVerse = m_mapFullVerseText.find(nRelIndex);
	if (itrFullVerse == m_mapFullVerseText.cend()) return 0;
	return itrFullVerse->m_strVerseText.size();
}

uint32_t CLetterMatrix::matrixIndexFromRelIndex(const CRelIndexEx nRelIndexEx) const
{
	CRelIndex relIndex{nRelIndexEx.index()};

	// Note: MatrixIndex must be signed here in case the part being eliminated from
	//	the text is larger than the text before the part being kept (like in the
	//	Words of Jesus only mode):
	int32_t nMatrixIndex = 0;
	if (!isFTMode() || m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(relIndex);
		if (pBook == nullptr) return 0;

		nMatrixIndex = m_pBibleDatabase->NormalizeIndexEx(CRelIndexEx(nRelIndexEx, nRelIndexEx.isPrologue() ? 0 : nRelIndexEx.letter()));
		if (!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
			if (nRelIndexEx.isColophon()) {
				Q_ASSERT(pBook->m_bHaveColophon);
				const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(relIndex);
				Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
				uint32_t nColophonShift = (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);
				nMatrixIndex += nColophonShift;		// Shift colophon to end of book
			} else {
				if (pBook->m_bHaveColophon && !nRelIndexEx.isBookPrologue()) {		// If this book has a colophon, but this index isn't it, shift this index ahead of colophon
					const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
					Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
					nMatrixIndex -= pColophonVerse->m_nNumLtr;
				}
			}
		}

		// Add in Matrix Cells for all prologues inserted ahead of this point.
		//	And do the letter shift shuffles concurrently.  They have to be
		//	concurrent or the indexes being compared won't be correct:
		// Note: These must be subtracted as we go as these shifts must be cumulative
		//	(unlike the other direction):
		TMapMatrixIndexToPrologue::const_iterator itrPrologues = m_mapMatrixIndexToPrologue.cbegin();
		TMapMatrixIndexToLetterShift::const_iterator itrXformMatrix = m_mapMatrixIndexToLetterShift.cbegin();
		bool bDoneShuffle = false;
		struct TFoundPrologue {
			int32_t m_nMatrixIndex = 0;
			TMapMatrixIndexToPrologue::const_iterator m_itr;
		};
		QList<TFoundPrologue> lstFoundPrologues;
		lstFoundPrologues.reserve(3);				// Room for book, chapter, and verse prologues

		while (!bDoneShuffle) {
			bool bPrologue = (itrPrologues != m_mapMatrixIndexToPrologue.cend());
			bool bXform = (itrXformMatrix != m_mapMatrixIndexToLetterShift.cend());
			uint32_t nNextKey = std::min(bPrologue ? itrPrologues.key() : std::numeric_limits<uint32_t>::max(),
										 bXform ? itrXformMatrix.key() : std::numeric_limits<uint32_t>::max());

			if (bXform && (nNextKey == itrXformMatrix.key())) {					// Must handle text removals before insertions
				if (static_cast<int32_t>(nNextKey) <= nMatrixIndex) {
					nMatrixIndex -= itrXformMatrix.value();
					++itrXformMatrix;
				} else {
					itrXformMatrix = m_mapMatrixIndexToLetterShift.cend();		// break
				}
			} else if (bPrologue && (nNextKey == itrPrologues.key())) {
				if (static_cast<int32_t>(nNextKey) <= nMatrixIndex) {
					if ((static_cast<int32_t>(nNextKey) == nMatrixIndex) &&
						(nMatrixIndex < static_cast<int32_t>(nNextKey + itrPrologues.value().m_strPrologue.size())) &&
						nRelIndexEx.isPrologue()) {
						// If we are currently inside a prologue, add it to the list
						//	so we can return the best prologue that fits the request:
						TFoundPrologue fp;
						fp.m_nMatrixIndex = nMatrixIndex;
						fp.m_itr = itrPrologues;
						lstFoundPrologues.append(fp);
					}
					nMatrixIndex += itrPrologues.value().m_strPrologue.size();
					++itrPrologues;
				} else {
					itrPrologues = m_mapMatrixIndexToPrologue.cend();			// break;
				}
			} else {
				bDoneShuffle = true;
			}
		}

		// If this was a prologue, find the best fit prologue part for
		//	that requested and index by its letter into the matrix:
		if (!lstFoundPrologues.isEmpty()) {
			Q_ASSERT(nRelIndexEx.isPrologue());
			// First, try to find exact match:
			int32_t nFoundMatrixIndex = 0;
			for (auto const & prologue : lstFoundPrologues) {
				CRelIndexEx ndxPrologue(prologue.m_itr.value().m_ndxPrologue, 1);
				if ((ndxPrologue.isBookPrologue() && nRelIndexEx.isBookPrologue()) ||
					(ndxPrologue.isChapterPrologue() && nRelIndexEx.isChapterPrologue()) ||
					(ndxPrologue.isVersePrologue() && nRelIndexEx.isVersePrologue())) {
					nFoundMatrixIndex = prologue.m_nMatrixIndex;
					break;
				}
			}
			// If no exact match, find closest match:
			if (nFoundMatrixIndex == 0) {
				if (nRelIndexEx.isBookPrologue()) {
					nFoundMatrixIndex = lstFoundPrologues.first().m_nMatrixIndex;	// Book -> Chapter -> Verse
				} else if (nRelIndexEx.isVersePrologue()) {
					nFoundMatrixIndex = lstFoundPrologues.last().m_nMatrixIndex;	// Verse -> Chapter -> Book
				} else {
					nFoundMatrixIndex = lstFoundPrologues.first().m_nMatrixIndex;	// Chapter -> Book -> Verse
				}
			}
			nMatrixIndex = nFoundMatrixIndex + nRelIndexEx.letter()-1;
		}
	} else {
		relIndex.setWord(1);
		if (relIndex.isColophon() && m_flagsLMTMO.testFlag(LMTMO_RemoveColophons)) {
			relIndex.setBook(relIndex.book()+1);
			relIndex.setChapter(1);
			relIndex.setVerse(1);
		} else if (relIndex.isSuperscription() && m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions)) {
			relIndex.setVerse(1);
		}

		// First see if we have a prologue for it:
		bool bFound = false;
		CRelIndexEx ndxSearch = nRelIndexEx;
		if (ndxSearch.isPrologue()) {
			TMapRelIndexToMatrixIndex::const_iterator itrPrologue = m_mapPrologueRelIndexToMatrixIndex.find(ndxSearch);
			if (itrPrologue != m_mapPrologueRelIndexToMatrixIndex.cend()) {
				nMatrixIndex = itrPrologue.value();
				nMatrixIndex += ndxSearch.letter()-1;		// Index into the prologue
				bFound = true;
			}
			if (!bFound && ndxSearch.isBookPrologue()) {
				ndxSearch.setChapter(1);					// Convert from Book Prologue to Chapter Prologue and try that
				itrPrologue = m_mapPrologueRelIndexToMatrixIndex.find(ndxSearch);
				if (itrPrologue != m_mapPrologueRelIndexToMatrixIndex.cend()) {
					nMatrixIndex = itrPrologue.value();
					nMatrixIndex += ndxSearch.letter()-1;		// Index into the prologue
					bFound = true;
				}
			}
			if (!bFound && ndxSearch.isChapterPrologue()) {
				ndxSearch.setVerse(1);						// Convert from Chapter Prologue to Verse Prologue and try that
				itrPrologue = m_mapPrologueRelIndexToMatrixIndex.find(ndxSearch);
				if (itrPrologue != m_mapPrologueRelIndexToMatrixIndex.cend()) {
					nMatrixIndex = itrPrologue.value();
					nMatrixIndex += ndxSearch.letter()-1;		// Index into the prologue
					bFound = true;
				}
			}
		}
		if (!bFound) {
			// Normalize and Denormalize to find first book/chapter/verse with text:
			TMapFullVerseText::const_iterator itrFullVerse = m_mapFullVerseText.find(
										m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(relIndex)));
			if (itrFullVerse != m_mapFullVerseText.cend()) {
				nMatrixIndex = itrFullVerse->m_nMatrixIndex;
				nMatrixIndex += nRelIndexEx.letter()-1;		// Index into the verse full text
			}
		}
	}

	if (nMatrixIndex < 0) nMatrixIndex = 1;		// Return first active position if real position is before anything in the matrix
	return nMatrixIndex;
}

CRelIndexEx CLetterMatrix::relIndexFromMatrixIndex(uint32_t nMatrixIndex) const
{
	if (!isFTMode() || m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
		// Since this is a matrix index that's becoming a Bible Normal
		//	index, we should first adjust for any prologues added prior
		//	to this matrix index.  And if we are on a prologue, save it:
		uint32_t nMatrixShift = 0;
		for (TMapMatrixIndexToPrologue::const_iterator itrPrologues = m_mapMatrixIndexToPrologue.cbegin();
			 itrPrologues != m_mapMatrixIndexToPrologue.cend(); ++itrPrologues) {
			if (itrPrologues.key() <= nMatrixIndex) {
				if (nMatrixIndex < (itrPrologues.key() + itrPrologues.value().m_strPrologue.size())) {
					// If we are currently inside a prologue, return with its index:
					return CRelIndexEx(itrPrologues.value().m_ndxPrologue, (nMatrixIndex - itrPrologues.key())+1);
				}
				nMatrixShift += itrPrologues.value().m_strPrologue.size();
			} else {
				break;
			}
		}

		// Note: Since the colophon transform mapping is inserted at the
		//	point where the colophon would be (after moving) in the matrix, then
		//	we must do this index transform prior to other colophon transforms.
		//	This is also needed for the logic below where we do a denormalization
		//	of MatrixIndex and it needs to have the correct number of letters prior
		//	to this point:
		// Note: These must be added after we've accumulated the total as these
		//	shouldn't be cumulative (unlike the other direction):
		uint32_t nLetterShift = 0;
		for (TMapMatrixIndexToLetterShift::const_iterator itrXformMatrix = m_mapMatrixIndexToLetterShift.cbegin();
			 itrXformMatrix != m_mapMatrixIndexToLetterShift.cend(); ++itrXformMatrix) {
			if (itrXformMatrix.key() <= nMatrixIndex) {
				nLetterShift += itrXformMatrix.value();
				} else {
				break;
			}
		}

		nMatrixIndex -= nMatrixShift;			// Remove letters from all of the prologues inserted ahead of current index
		nMatrixIndex += nLetterShift;			// Add letters for all text skipped ahead of current index

		// Since we are only shifting the colophons from the beginning of each book
		//	to the end of each book, the number of letters in each book should be
		//	the same.  Therefore, the standard Denormalize call should give the same
		//	book:
		CRelIndex relIndex{m_pBibleDatabase->DenormalizeIndexEx(nMatrixIndex)};
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(relIndex);
		Q_ASSERT(pBook != nullptr);  if (pBook == nullptr) return CRelIndexEx();

		if (!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly)) {
			if (pBook->m_bHaveColophon) {		// Must do this even when skipping colophons to shift other indexes after colophon until we catchup with the transform above
				const CVerseEntry *pColophonVerse = m_pBibleDatabase->verseEntry(CRelIndex(relIndex.book(), 0, 0, relIndex.word()));
				Q_ASSERT(pColophonVerse != nullptr);  if (pColophonVerse == nullptr) return 0;
				uint32_t nMatrixOldColophonNdx = pColophonVerse->m_nLtrAccum + 1;
				uint32_t nColophonShift = (pBook->m_nNumLtr - pColophonVerse->m_nNumLtr);
				uint32_t nMatrixNewColophonNdx = nMatrixOldColophonNdx + nColophonShift;
				if (nMatrixIndex >= nMatrixNewColophonNdx) {
					if (!m_flagsLMTMO.testFlag(LMTMO_RemoveColophons)) {	// Don't adjust for the colophon here if we skipped it
						nMatrixIndex -= nColophonShift;						// Shift colophon back to start of book
					}
				} else {
					if (nMatrixIndex >= nMatrixOldColophonNdx) {
						nMatrixIndex += pColophonVerse->m_nNumLtr;		// Shift other indexes after colophon (as long as the index isn't part of the prologue)
					}
				}
			}
		}
		return m_pBibleDatabase->DenormalizeIndexEx(nMatrixIndex);
	} else {
		// Find the start of this verse entry or prologue:
		uint32_t nLetterCount = 1;
		TMapMatrixIndexToRelIndex::const_iterator itrRelIndex = m_mapFullTextMatrixIndexToRelIndex.find(nMatrixIndex);
		TMapMatrixIndexToPrologue::const_iterator itrPrologue = m_mapMatrixIndexToPrologue.find(nMatrixIndex);
		bool bFound = (itrRelIndex != m_mapFullTextMatrixIndexToRelIndex.cend()) ||
					  (itrPrologue != m_mapMatrixIndexToPrologue.cend());
		while (!bFound && (nMatrixIndex > 0)) {
			++nLetterCount;
			--nMatrixIndex;
			itrRelIndex = m_mapFullTextMatrixIndexToRelIndex.find(nMatrixIndex);
			itrPrologue = m_mapMatrixIndexToPrologue.find(nMatrixIndex);
			bFound = (itrRelIndex != m_mapFullTextMatrixIndexToRelIndex.cend()) ||
					 (itrPrologue != m_mapMatrixIndexToPrologue.cend());
		}
		if (itrRelIndex != m_mapFullTextMatrixIndexToRelIndex.cend()) {
			// This was a verse or superscription or colophon:
			return CRelIndexEx(itrRelIndex.value(), nLetterCount);
		} else {
			Q_ASSERT(itrPrologue != m_mapMatrixIndexToPrologue.cend());
			return CRelIndexEx(itrPrologue->m_ndxPrologue, nLetterCount);
		}
	}
}

// ----------------------------------------------------------------------------

QString CLetterMatrix::getOptionDescription(bool bSingleLine) const
{
	auto &&fnCPONumDesc = [=](LMChapterPrologueOptionFlags nOpt)->QString {
		QString strDesc;
		switch (nOpt & LMCPO_NumberOptionsMask) {
			case LMCPO_NumbersNone:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrNoNums : g_constrNoNumerals);
				break;
			case LMCPO_NumbersRoman:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrRomanNums : g_constrRomanNumerals);
				break;
			case LMCPO_NumbersArabic:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrArabicNums : g_constrArabicNumerals);
				break;
			default:
				break;
		};
		return strDesc;
	};
	auto &&fnCPOPsalmBookNumDesc = [=](LMChapterPrologueOptionFlags nOpt)->QString {
		QString strDesc;
		switch (nOpt & LMCPO_PsalmBookNumberOptionsMask) {
			case LMCPO_PsalmBookNumbersNone:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrNoNums : g_constrNoNumerals);
				break;
			case LMCPO_PsalmBookNumbersRoman:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrRomanNums : g_constrRomanNumerals);
				break;
			case LMCPO_PsalmBookNumbersArabic:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrArabicNums : g_constrArabicNumerals);
				break;
			default:
				break;
		};
		return strDesc;
	};
	auto &&fnVPONumDesc = [=](LMVersePrologueOptionFlags nOpt)->QString {
		QString strDesc;
		switch (nOpt & LMVPO_NumberOptionsMask) {
			case LMVPO_NumbersNone:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrNoNums : g_constrNoNumerals);
				break;
			case LMVPO_NumbersRoman:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrRomanNums : g_constrRomanNumerals);
				break;
			case LMVPO_NumbersArabic:
				strDesc = QString(" (%1)").arg(bSingleLine ? g_constrArabicNums : g_constrArabicNumerals);
				break;
			default:
				break;
		};
		return strDesc;
	};

	QString strDescription;
	if (textModifierOptions().testFlag(LMTMO_WordsOfJesusOnly)) {
		if (bSingleLine) strDescription += " ";
		strDescription += QObject::tr("Words of Jesus Only", "CLetterMatrix");
		if (!bSingleLine) strDescription += "\n";
	} else {
		bool bPrologues = false;
		QString strWithPrefix = QObject::tr("With", "CLetterMatrix") + " ";

		// There's no Words of Jesus in Colophons or Superscriptions or Book/Chapter Prologues
		if (textModifierOptions().testFlag(LMTMO_IncludeBookPrologues)) {
			if (bSingleLine) strDescription += (bPrologues ? ", " : " ");
			strDescription += ((!bPrologues || !bSingleLine) ? strWithPrefix : "");
			strDescription += QObject::tr("Book Prologues", "CLetterMatrix");
			if (!bSingleLine) strDescription += "\n";
			bPrologues = true;
		}
		if (textModifierOptions().testFlag(LMTMO_IncludeChapterPrologues)) {
			if (bSingleLine) strDescription += (bPrologues ? ", " : " ");
			strDescription += ((!bPrologues || !bSingleLine) ? strWithPrefix : "");
			strDescription += QObject::tr("Chapter Prologues", "CLetterMatrix");
			strDescription += fnCPONumDesc(chapterPrologueOptions());
			if (chapterPrologueOptions().testFlag(LMCPO_PsalmBookTags)) {
				strDescription += " " + QObject::tr("w/PsalmBOOKs", "CLetterMatrix") + fnCPOPsalmBookNumDesc(chapterPrologueOptions());
			}
			if (!bSingleLine) strDescription += "\n";
			bPrologues = true;
		}
		if (textModifierOptions().testFlag(LMTMO_IncludeVersePrologues)) {
			if (bSingleLine) strDescription += (bPrologues ? ", " : " ");
			strDescription += ((!bPrologues || !bSingleLine) ? strWithPrefix : "");
			strDescription += QObject::tr("Verse Prologues", "CLetterMatrix");
			strDescription += fnVPONumDesc(versePrologueOptions());
			if ((versePrologueOptions() & LMVPO_PS119_Mask) != 0) {
				strDescription += " (w/";
				if (versePrologueOptions().testFlag(LMVPO_PS119_HebrewLetter)) strDescription += QObject::tr("Heb", "CLetterMatrix");
				if (versePrologueOptions().testFlag(LMVPO_PS119_Transliteration)) strDescription += QObject::tr("Trn", "CLetterMatrix");
				if (versePrologueOptions().testFlag(LMVPO_PS119_Punctuation) &&
					textModifierOptions().testFlag(LMTMO_IncludePunctuation)) strDescription += QObject::tr("Pun", "CLetterMatrix");
				strDescription += QObject::tr("Acrostics", "CLetterMatrix");
				strDescription += ")";
			}
			if (!bSingleLine) strDescription += "\n";
			bPrologues = true;
		}
		if (textModifierOptions().testFlag(LMTMO_IncludePunctuation)) {
			if (bSingleLine) strDescription += (bPrologues ? ", " : " ");
			strDescription += ((!bPrologues || !bSingleLine) ? strWithPrefix : "");
			strDescription += QObject::tr("Punctuation", "CLetterMatrix");
			if (fullVerseTextOptions() != LMFVTO_None) strDescription += " ";
			if (fullVerseTextOptions().testFlag(LMFVTO_NoBracketsForTransChange)) {
				strDescription += "(" + QObject::tr("NoTCA", "CLetterMatrix") + ")";
			}
			if (fullVerseTextOptions().testFlag(LMFVTO_IncludePilcrowMarkers)) {
				strDescription += "(" + QString(g_chrPilcrow) + ")";
			}
			if (!bSingleLine) strDescription += "\n";
			bPrologues = true;
		}
		if (textModifierOptions().testFlag(LMTMO_DecomposeLetters)) {
			if (bSingleLine) {
				strDescription += (bPrologues ? ", " : " ");
				strDescription += QObject::tr("Decomp", "CLetterMatrix");
			} else {
				strDescription += QObject::tr("Decomposed Letters", "CLetterMatrix");
			}
			if (!bSingleLine) strDescription += "\n";
			bPrologues = true;
		}

		if (textModifierOptions() & (LMTMO_RemoveColophons | LMTMO_RemoveSuperscriptions)) {
			if (bSingleLine) {
				strDescription += (bPrologues ? ", " : " ");
				strDescription += QObject::tr("W/O", "CLetterMatrix") + " ";
			} else {
				strDescription += QObject::tr("Without", "CLetterMatrix") + " ";
			}
			if (textModifierOptions().testFlag(LMTMO_RemoveColophons)) {
				strDescription += QObject::tr("Colophons", "CLetterMatrix");
				if (textModifierOptions().testFlag(LMTMO_RemoveSuperscriptions)) {
					strDescription += " " + QObject::tr("or Superscriptions", "CLetterMatrix");
				}
			} else {
				strDescription += QObject::tr("Superscriptions", "CLetterMatrix");
			}
			if (!bSingleLine) strDescription += "\n";
		}
	}

	return strDescription;
}

// ----------------------------------------------------------------------------
