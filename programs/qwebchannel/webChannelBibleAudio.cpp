/****************************************************************************
**
** Copyright (C) 2015-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#include "webChannelBibleAudio.h"
#include "dbDescriptors.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <QUrl>

// ============================================================================

namespace {

#define NUM_BK 66
#define NUM_BK_OT 39
#define NUM_BK_NT 27

#define NUMBERS_BOOK_NUM 04
#define PSALMS_BOOK_NUM 19
#define OBADIAH_BOOK_NUM 31
#define ZEPHANIAH_BOOK_NUM 36
#define PHILEMON_BOOK_NUM 57
#define _2PETER_BOOK_NUM 61
#define _2JOHN_BOOK_NUM 63
#define _3JOHN_BOOK_NUM 64
#define JUDE_BOOK_NUM 65

	// Book names as they appear on the FCBH recording files:
	const QString g_arrconstrFCBHBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1Samuel",
		"2Samuel",
		"1Kings",
		"2Kings",
		"1Chronicles",
		"2Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalms",
		"Proverbs",
		"Ecclesiastes",
		"SongofSongs",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1Corinthians",
		"2Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1Thess",
		"2Thess",
		"1Timothy",
		"2Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1Peter",
		"2Peter",
		"1John",
		"2John",
		"3John",
		"Jude",
		"Revelation"
	};

	const QString g_constrFCBHNonDramaURL = "http://audios.dewtronics.com/KingJamesBible/ON_NonDrama/%1";
	const QString g_constrFCBHDramaURL = "http://audios.dewtronics.com/KingJamesBible/ON_Drama/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Scourby NonDrama recording files:
	const QString g_arrconstrScourbyNDBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1 Samuel",
		"2 Samuel",
		"1 Kings",
		"2 Kings",
		"1 Chronicles",
		"2 Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalm",
		"Proverbs",
		"Ecclesiastes",
		"Song of Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1 Corinthians",
		"2 Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1 Thessalonians",
		"2 Thessalonians",
		"1 Timothy",
		"2 Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1 Peter",
		"2 Peter",
		"1 John",
		"2 John",
		"3 John",
		"Jude",
		"Revelations",
	};

	const QString g_constrScourbyNDURL = "http://audios.dewtronics.com/KingJamesBible/Scourby/NonDrama/%1";

	// Book names as they appear on the Scourby NonDrama recording #2 files:
	const QString g_arrconstrScourbyND2Books[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"I Samuel",
		"II Samuel",
		"I Kings",
		"II Kings",
		"I Chronicles",
		"II Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalm",
		"Proverbs",
		"Ecclesiastes",
		"Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"I Corinthians",
		"II Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"I Thessalonians",
		"II Thessalonians",
		"I Timothy",
		"II Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"I Peter",
		"II Peter",
		"I John",
		"II John",
		"III John",
		"Jude",
		"Revelation"
	};

	const QString g_constrScourbyND2URL = "http://audios.dewtronics.com/KingJamesBible/Scourby/NonDrama2/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Scourby Drama recording files:
	const QString g_arrconstrScourbyWDBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1 Samuel",
		"2 Samuel",
		"1 Kings",
		"2 Kings",
		"1 Chronicles",
		"2 Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalms",
		"Proverbs",
		"Ecclesiastes",
		"Song Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1 Corinthians",
		"2 Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1 Thessalonians",
		"2 Thessalonians",
		"1 Timothy",
		"2 Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1 Peter",
		"2 Peter",
		"1 John",
		"2 John",
		"3 John",
		"Jude",
		"Revelation"
	};

	// Book name prefixes as they appear on the Scourby Drama recording files:
	const QString g_arrconstrScourbyWDBooksPre[NUM_BK] = {
		"Gen",
		"Ex",
		"Lev",
		"Num",
		"Deut",
		"Josh",
		"Judg",
		"Ruth",
		"1 Sam ",
		"2 Sam ",
		"1 Kings ",
		"2 Kings ",
		"1 Chron",
		"2 Chron",
		"Ezra",
		"Neh",
		"Esth",
		"Job",
		"Psa",
		"Prov",
		"Eccl",
		"Song",
		"Isa",
		"Jer",
		"Lam",
		"Eze",
		"Dan",
		"Hos",
		"Joel",
		"Amos",
		"Oba",
		"Jon",
		"Mic",
		"Nah",
		"Hab",
		"Zep",
		"Hag",
		"Zec",
		"Mal",
		"Mat",
		"Mar",
		"Luk",
		"John",
		"Acts",
		"Rom",
		"1 Cor",
		"2 Cor",
		"Gal",
		"Eph",
		"Phil",
		"Col",
		"1 Thes",
		"2 Thes",
		"1 Tim",
		"2 Tim",
		"Tit",
		"Phil",
		"Heb",
		"Jam",
		"1 Pet",
		"2 Pet",
		"1 John",
		"2 John",
		"3 John",
		"Jude",
		"Rev"
	};

	const QString g_constrScourbyWDURL = "http://audios.dewtronics.com/KingJamesBible/Scourby/Drama/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the DanWagner recording files:
	const QString g_arrconstrDWBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1Samuel",
		"2Samuel",
		"1Kings",
		"2Kings",
		"1Chronicles",
		"2Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalms",
		"Proverbs",
		"Ecclesiastes",
		"Song_of_Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1Corinthians",
		"2Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1Thessalonians",
		"2Thessalonians",
		"1Timothy",
		"2Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1Peter",
		"2Peter",
		"1John",
		"2John",
		"3John",
		"Jude",
		"Revelation"
	};

	const QString g_constrDWURL = "http://audios.dewtronics.com/KingJamesBible/DanWagner/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the StephenJohnston recording files:
	const QString g_arrconstrSJBooks[NUM_BK] = {
		"Gen",
		"Exo",
		"Lev",
		"Num",
		"Deu",
		"Jos",
		"Jdg",
		"Rth",
		"1Sa",
		"2Sa",
		"1Ki",
		"2Ki",
		"1Ch",
		"2Ch",
		"Ezr",
		"Neh",
		"Est",
		"Job",
		"Psa",
		"Pro",
		"Ecc",
		"Son",
		"Isa",
		"Jer",
		"Lam",
		"Eze",
		"Dan",
		"Hos",
		"Joe",
		"Amo",
		"Oba",
		"Jon",
		"Mic",
		"Nah",
		"Hab",
		"Zep",
		"Hag",
		"Zec",
		"Mal",
		"Mat",
		"Mar",
		"Luk",
		"Joh",
		"Act",
		"Rom",
		"1Co",
		"2Co",
		"Gal",
		"Eph",
		"Php",
		"Col",
		"1Th",
		"2Th",
		"1Ti",
		"2Ti",
		"Tts",
		"Phm",
		"Heb",
		"Jam",
		"1Pe",
		"2Pe",
		"1Jo",
		"2Jo",
		"3Jo",
		"Jde",
		"Rev"
	};

	const QString g_constrSJURL = "http://audios.dewtronics.com/KingJamesBible/StephenJohnston/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the ChristopherGlyn recording files:
	const QString g_arrconstrCGBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1-Samuel",
		"2-Samuel",
		"1-Kings",
		"2-Kings",
		"1-Chronicles",
		"2-Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalm",
		"Proverbs",
		"Ecclesiastes",
		"Song_of_Solomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggai",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1Corinthians",
		"2Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1Thessalonians",
		"2Thessalonians",
		"1Timothy",
		"2Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1Peter",
		"2Peter",
		"1John",
		"2John",
		"3John",
		"Jude",
		"Revelation"
	};

	const QString g_constrCGURL = "http://audios.dewtronics.com/KingJamesBible/ChristopherGlyn/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Willard Waggoner recording files:
	const QString g_arrconstrWWBooks[NUM_BK] = {
		"GEN",
		"EXO",
		"LEV",
		"NUM",
		"DEU",
		"JOS",
		"JUD",
		"RUT",
		"1SA",
		"2SA",
		"1KI",
		"2KI",
		"1CH",
		"2CH",
		"EXR",
		"NEH",
		"EST",
		"JOB",
		"PSA",
		"PRO",
		"ECC",
		"SON",
		"ISA",
		"JER",
		"LAM",
		"EZE",
		"DAN",
		"HOS",
		"JOE",
		"AMO",
		"OBA",
		"JON",
		"MIC",
		"NAH",
		"HAB",
		"ZEP",
		"HAG",
		"ZEC",
		"MAL",
		"MAT",
		"MAR",
		"LUK",
		"JOH",
		"ACT",
		"ROM",
		"1CO",
		"2CO",
		"GAL",
		"EPH",
		"PHP",
		"COL",
		"1TH",
		"2TH",
		"1TI",
		"2TI",
		"TIT",
		"PHM",
		"HEB",
		"JAM",
		"1PE",
		"2PE",
		"1JO",
		"2JO",
		"3JO",
		"JDE",
		"REV"
	};

	const QString g_constrWWURL = "http://audios.dewtronics.com/KingJamesBible/WillardWaggoner/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Sherberg/Jones recording files:
	const QString g_arrconstrSherbergJonesBooks[NUM_BK] = {
		"Genesis",
		"Exodus",
		"Leviticus",
		"Numbers",
		"Deuteronomy",
		"Joshua",
		"Judges",
		"Ruth",
		"1Samuel",
		"2Samuel",
		"1Kings",
		"2Kings",
		"1Chronicles",
		"2Chronicles",
		"Ezra",
		"Nehemiah",
		"Esther",
		"Job",
		"Psalms",
		"Proverbs",
		"Ecclesiastes",
		"SongOfSolomon",
		"Isaiah",
		"Jeremiah",
		"Lamentations",
		"Ezekiel",
		"Daniel",
		"Hosea",
		"Joel",
		"Amos",
		"Obadiah",
		"Jonah",
		"Micah",
		"Nahum",
		"Habakkuk",
		"Zephaniah",
		"Haggi",
		"Zechariah",
		"Malachi",
		"Matthew",
		"Mark",
		"Luke",
		"John",
		"Acts",
		"Romans",
		"1Corinthians",
		"2Corinthians",
		"Galatians",
		"Ephesians",
		"Philippians",
		"Colossians",
		"1Thessalonians",
		"2Thessalonians",
		"1Timothy",
		"2Timothy",
		"Titus",
		"Philemon",
		"Hebrews",
		"James",
		"1Peter",
		"2Peter",
		"1John",
		"2John",
		"3John",
		"Jude",
		"Revelation"
	};

	const QString g_constrSherbergJonesURL = "http://audios.dewtronics.com/KingJamesBible/SherbergJones/%1";

	// ------------------------------------------------------------------------

	// Reina-Valera 1865:
	//	Filename format: nnn.BOOKNAME_x.mp3
	//		nnn = 3-digit Testament Chapter number, 0 filled
	//		BOOKNAME = Exact bookname as listed below
	//		x = Book Chapter

	const QString g_arrconstrRV1865Books[NUM_BK_NT] = {
		"Mateo",
		"Marcos",
		"Lucas",
		"Juan",
		"Actos",
		"Romanos",
		"Primera_Corintios",
		"Segunda_Corintios",
		"Gálatas",
		"Efesios",
		"Filipenses",
		"Colosenses",
		"Primera_Tesalonicenses",
		"Segunda_Tesalonicenses",
		"Primera_Timoteo",
		"Segunda_Timoteo",
		"Tito",
		"Filemón",
		"Hebreos",
		"Santiago",
		"Primera_de_Pedro",
		"Segunda_de_Pedro",
		"Primera_Epistola_de_San_Juan",
		"Segunda_Epistola_de_San_Juan",
		"Tercera_de_Juan",
		"San_Judas",
		"Revelación",
	};

	const QString g_constrRV1865URL = "http://audios.dewtronics.com/ReinaValera1865/%1";

	// ------------------------------------------------------------------------

}	// Namespace

// ============================================================================


CWebChannelBibleAudio::CWebChannelBibleAudio()
{

}

CWebChannelBibleAudio::~CWebChannelBibleAudio()
{

}

CWebChannelBibleAudio *CWebChannelBibleAudio::instance()
{
	static CWebChannelBibleAudio g_wcba;
	return &g_wcba;
}

QString CWebChannelBibleAudio::urlsForChapterAudio(const CBibleDatabasePtr pBibleDatabase, const CRelIndex &ndxRel, BibleAudioSourcesFlags flagsBAS)
{
	assert(!pBibleDatabase.isNull());

	CRelIndex ndxDecolophonated(ndxRel);
	if (ndxDecolophonated.isColophon()) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxDecolophonated);
		if (pBook) {
			ndxDecolophonated.setChapter(pBook->m_nNumChp);
		}
	}

	QJsonArray arrBibleAudioList;

	if (ndxDecolophonated.isSet()) {
		int nTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_TESTAMENT, ndxDecolophonated).ofBible().first;
		int nBkTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_BOOK, ndxDecolophonated).ofTestament().first;
		int nBk = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_BOOK, ndxDecolophonated).ofBible().first;
		int nChp = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofBook().first;
		int nChpTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofTestament().first;
		int nChpBible = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofBible().first;

		bool bKJVValid = (((nTst > 0) && (nTst <= 2)) && (nBkTst != 0) && ((nBk > 0) && (nBk <= NUM_BK)));
		if ((pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVPCE).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVA).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611A).m_strUUID, Qt::CaseInsensitive) != 0))
			bKJVValid = false;

		bool bRV1865Valid = ((nTst == 2) && ((nBkTst > 0) && (nBkTst <= NUM_BK_NT)));
		if (pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_RV1865mv20180504).m_strUUID, Qt::CaseInsensitive) != 0)
			bRV1865Valid = false;

		if (bKJVValid) {
			// Faith Comes By Hearing:
			QString strFCBHBookName = QString("%1").arg(g_arrconstrFCBHBooks[nBk-1], -12, QChar('_'));
			QString strFCBHBkTst = QString("%1%2").arg((nTst == 1) ? "A" : "B").arg(nBkTst, 2, 10, QChar('0'));
			QString strFCBHChp = ((nBk == PSALMS_BOOK_NUM) ?
								QString("%1").arg(nChp, 3, 10, QChar('0')) :
								QString("_%1").arg(nChp, 2, 10, QChar('0')));
			QString strFCBHNonDrama = QString("%1__%2_%3ENGKJVC1DA.mp3").arg(strFCBHBkTst).arg(strFCBHChp).arg(strFCBHBookName);
			QString strFCBHDrama = QString("%1__%2_%3ENGKJVC2DA.mp3").arg(strFCBHBkTst).arg(strFCBHChp).arg(strFCBHBookName);
			if (flagsBAS & BAS_FCBH_NONDRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "FCBH Non-Drama";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrFCBHNonDramaURL).arg(strFCBHNonDrama)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}
			if (flagsBAS & BAS_FCBH_DRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "FCBH Drama";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrFCBHDramaURL).arg(strFCBHDrama)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// Scourby NonDrama #1:
//			QString strScourbyNDBkChp = g_arrconstrScourbyNDBooks[nBk-1];
//			switch (nBk) {
//				case OBADIAH_BOOK_NUM:				// No digits
//				case PHILEMON_BOOK_NUM:
//				case _2JOHN_BOOK_NUM:
//				case _3JOHN_BOOK_NUM:
//				case JUDE_BOOK_NUM:
//					break;
//				case ZEPHANIAH_BOOK_NUM:			// 1 Digit
//				case _2PETER_BOOK_NUM:
//					strScourbyNDBkChp += QString(" %1").arg(nChp, 1, 10, QChar('0'));
//					break;
//				case PSALMS_BOOK_NUM:				// 3 Digits
//					strScourbyNDBkChp += QString(" %1").arg(nChp, 3, 10, QChar('0'));
//					break;
//				default:							// 2 Digits
//					strScourbyNDBkChp += QString(" %1").arg(nChp, 2, 10, QChar('0'));
//					break;
//			}
//			strScourbyNDBkChp += ".mp3";
//			if (flagsBAS & BAS_SCOURBY_NONDRAMA) {
//				QJsonObject objBibleAudio;
//				objBibleAudio["name"] = "Scourby Non-Drama";
//				objBibleAudio["url"] = QString(QUrl(QString(g_constrScourbyNDURL).arg(strScourbyNDBkChp)).toEncoded());
//				arrBibleAudioList.append(objBibleAudio);
//			}

			//Scourby NonDrama #2:
			QString strScourbyND2 = QString("%1 %2 %3.mp3").arg(nBk, 2, 10, QChar('0'))
															.arg(g_arrconstrScourbyND2Books[nBk-1])
															.arg(nChp, 3, 10, QChar('0'));
			if (flagsBAS & BAS_SCOURBY_NONDRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Scourby Non-Drama";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrScourbyND2URL).arg(strScourbyND2)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// Scourby Drama:
			QString strScourbyWDBook = QString("%1 %2").arg(nBk, 2, 10, QChar('0')).arg(g_arrconstrScourbyWDBooks[nBk-1]);
			QString strScourbyWDBkChp = QString("%1 ch%2").arg(g_arrconstrScourbyWDBooksPre[nBk-1]).arg(nChp, 3, 10, QChar('0'));
			QString strScourbyWD = QString("%1/%2.mp3").arg(strScourbyWDBook).arg(strScourbyWDBkChp);
			if (flagsBAS & BAS_SCOURBY_DRAMA) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Scourby Drama";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrScourbyWDURL).arg(strScourbyWD)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// Dan Wagner:
			QString strDWBookFolder = QString("%1_%2").arg(nBk, 2, 10, QChar('0')).arg(g_arrconstrDWBooks[nBk-1]);
			QString strDWBkChp;
			if (nBk == NUMBERS_BOOK_NUM) {
				strDWBkChp = QString("%1%2").arg(g_arrconstrDWBooks[nBk-1]).arg(nChp, 2, 10, QChar('0'));
			} else if ((nBk == PHILEMON_BOOK_NUM) ||
					   (nBk == _2JOHN_BOOK_NUM) ||
					   (nBk == _3JOHN_BOOK_NUM) ||
					   (nBk == JUDE_BOOK_NUM)) {
				strDWBkChp = g_arrconstrDWBooks[nBk-1];
			} else {
				strDWBkChp = QString("%1%2").arg(g_arrconstrDWBooks[nBk-1]).arg(nChp, 3, 10, QChar('0'));
			}
			QString strDanWagner = QString("%1/%2.mp3").arg(strDWBookFolder).arg(strDWBkChp);
			if (flagsBAS & BAS_DAN_WAGNER) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Dan Wagner";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrDWURL).arg(strDanWagner)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// Stephen Johnston:
			QString strSJBkChp = QString("%1%2%3.mp3").arg(nBk, 2, 10, QChar('0')).arg(g_arrconstrSJBooks[nBk-1]).arg(nChp, 3, 10, QChar('0'));
			if (flagsBAS & BAS_STEPHEN_JOHNSTON) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Stephen Johnston";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrSJURL).arg(strSJBkChp)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// Christophen Glyn:
			QString strCGTstFolder = QString("%1").arg((nTst == 1) ? "ot" : "nt");
			QString strCGBkChp = QString("%1_%2_%3").arg(nBkTst, 2, 10, QChar('0'))
													.arg(g_arrconstrCGBooks[nBk-1])
													.arg(nChp, (nBk == PSALMS_BOOK_NUM) ? 3 : 2, 10, QChar('0'));
			QString strChristopherGlyn = QString("%1/%2.mp3").arg(strCGTstFolder).arg(strCGBkChp);
			if (flagsBAS & BAS_CHRISTOPHER_GLYN) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Christopher Glyn";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrCGURL).arg(strChristopherGlyn)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// Willard Waggoner:
			QString strWWTstFolder = QString("%1").arg((nTst == 1) ? "Old Testament" : "New Testament");
			QString strWWBkFolder = QString("%1_%2").arg(nBk, 2, 10, QChar('0')).arg(g_arrconstrWWBooks[nBk-1]);
			QString strWWBkChp = QString("%1_%2_%3").arg(nBk, 2, 10, QChar('0'))
													.arg(g_arrconstrWWBooks[nBk-1])
													.arg(nChp, (nBk == PSALMS_BOOK_NUM) ? 3 : 2, 10, QChar('0'));
			QString strWillardWaggoner = QString("%1/%2/%3.mp3").arg(strWWTstFolder).arg(strWWBkFolder).arg(strWWBkChp);
			if (flagsBAS & BAS_WILLARD_WAGGONER) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Willard Waggoner";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrWWURL).arg(strWillardWaggoner)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// Jon Sherberg/James Earl Jones:
			QString strSherbergJones = QString("%1-%2_%3")
											.arg(nChpBible, 4, 10, QChar('0'))
											.arg(g_arrconstrSherbergJonesBooks[nBk-1])
											.arg(nChp);
			if ((nChpBible == 37) ||
				(nChpBible == 70) ||
				(nChpBible == 86) ||
				(nChpBible == 103) ||
				(nChpBible == 116) ||
				(nChpBible == 131) ||
				(nChpBible == 162) ||
				(nChpBible == 181) ||
				(nChpBible == 215) ||
				(nChpBible == 250) ||
				(nChpBible == 284) ||
				(nChpBible == 298) ||
				(nChpBible == 310) ||
				(nChpBible == 339) ||
				(nChpBible == 357) ||
				(nChpBible == 585) ||
				(nChpBible == 631) ||
				(nChpBible == 776) ||
				(nChpBible == 820)) {
				strSherbergJones += "_MP3WRAP.mp3";
			} else {
				strSherbergJones += ".mp3";
			}
			if (flagsBAS & BAS_SHERBERG_JONES) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Sherberg/Jones";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrSherbergJonesURL).arg(strSherbergJones)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}
		}

		if (bRV1865Valid) {
			// Reina-Valera 1865:
			QString strRVBkChp = QString("%1.%2_%3.mp3").arg(nChpTst, 3, 10, QChar('0')).arg(g_arrconstrRV1865Books[nBkTst-1]).arg(nChp);
			if (flagsBAS & BAS_REINA_VALERA_1865) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Reina Valera 1865";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrRV1865URL).arg(strRVBkChp)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}
		}
	}

	return QJsonDocument(arrBibleAudioList).toJson(QJsonDocument::Compact);
}

// ============================================================================
