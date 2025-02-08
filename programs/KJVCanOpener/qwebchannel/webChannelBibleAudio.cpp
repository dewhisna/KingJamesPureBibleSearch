/****************************************************************************
**
** Copyright (C) 2015-2025 Donna Whisnant, a.k.a. Dewtronics.
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
#include "BibleLayout.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <QUrl>

// ============================================================================

namespace {

	// Book names as they appear on the FCBH recording files:
	const QString g_arrconstrFCBHBooks[NUM_BK_OT_NT] = {
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

	const QString g_constrFCBHNonDramaURL = "https://audios.dewtronics.com/KingJamesBible/ON_NonDrama/%1";
	const QString g_constrFCBHDramaURL = "https://audios.dewtronics.com/KingJamesBible/ON_Drama/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Scourby NonDrama recording files:
	const QString g_arrconstrScourbyNDBooks[NUM_BK_OT_NT] = {
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

	const QString g_constrScourbyNDURL = "https://audios.dewtronics.com/KingJamesBible/Scourby/NonDrama/%1";

	// Book names as they appear on the Scourby NonDrama recording #2 files:
	const QString g_arrconstrScourbyND2Books[NUM_BK_OT_NT] = {
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

	const QString g_constrScourbyND2URL = "https://audios.dewtronics.com/KingJamesBible/Scourby/NonDrama2/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Scourby Drama recording files:
	const QString g_arrconstrScourbyWDBooks[NUM_BK_OT_NT] = {
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
	const QString g_arrconstrScourbyWDBooksPre[NUM_BK_OT_NT] = {
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

	const QString g_constrScourbyWDURL = "https://audios.dewtronics.com/KingJamesBible/Scourby/Drama/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the DanWagner recording files:
	const QString g_arrconstrDWBooks[NUM_BK_OT_NT] = {
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

	const QString g_constrDWURL = "https://audios.dewtronics.com/KingJamesBible/DanWagner/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the StephenJohnston recording files:
	const QString g_arrconstrSJBooks[NUM_BK_OT_NT] = {
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

	const QString g_constrSJURL = "https://audios.dewtronics.com/KingJamesBible/StephenJohnston/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the ChristopherGlyn recording files:
	const QString g_arrconstrCGBooks[NUM_BK_OT_NT] = {
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

	const QString g_constrCGURL = "https://audios.dewtronics.com/KingJamesBible/ChristopherGlyn/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Willard Waggoner recording files:
	const QString g_arrconstrWWBooks[NUM_BK_OT_NT] = {
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

	const QString g_constrWWURL = "https://audios.dewtronics.com/KingJamesBible/WillardWaggoner/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Sherberg/Jones recording files:
	const QString g_arrconstrSherbergJonesBooks[NUM_BK_OT_NT] = {
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

	const QString g_constrSherbergJonesURL = "https://audios.dewtronics.com/KingJamesBible/SherbergJones/%1";

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

	const QString g_constrRV1865URL = "https://audios.dewtronics.com/ReinaValera1865/%1";

	// ------------------------------------------------------------------------

	// 1876 Russian Synodal:
	//	Filename format: xx/yy.mp3
	//		xx = Book number, zero-filled to 2-digits, but strangely were
	//				suppied in the wrong order in the NT, so we have the
	//				mapping here below
	//		yy = Chapter number, zero-filled to 2-digits.  In Psalms, the
	//				chapter number will go to 3-digits, but it's not
	//				zero-filled above the lower 2-digits.

	const uint32_t g_arrconRussianSynodalBooks[NUM_BK_OT_NT] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,			// Sounds_OT_1.zip
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,		// Sounds_OT_2.zip
		40, // Matthew (40)																// Sounds_NT.zip
		41, // Mark (41)
		42, // Luke (42)
		43, // John (43)
		44, // Acts (44)
		52, // Romans (45)
		53, // 1Cor (46)
		54, // 2Cor (47)
		55, // Galations (48)
		56, // Ephesians (49)
		57, // Philippians (50)
		58, // Colossians (51)
		59, // 1Thess (52)
		60, // 2Thess (53)
		61, // 1Tim (54)
		62, // 2Tim (55)
		63, // Titus (56)
		64, // Philemon (57)
		65, // Hebrews (58)
		45, // James (59)
		46, // 1Peter (60)
		47, // 2Peter (61)
		48, // 1John (62)
		49, // 2John (63)
		50, // 3John (64)
		51, // Jude (65)
		66, // Revelation (66)
	};

	const QString g_constrRussianSynodalURL = "https://audios.dewtronics.com/RussianSynodal1876/%1";

	// ------------------------------------------------------------------------

	// Talking Bibles : 1876 Russian Synodal:
	//	Filename format: x_TN/yy_BOOKNAME/yy_BOOKNAME_zzz.mp3
	//		x = Testament number (1 = OT, 2 = NT)
	//		TN = 2-character Testament Name in uppercase (OT or NT)
	//		yy = Book number, 1-based, zero-filled to 2-digits.  Book number
	//			is RELATIVE to the testament.  That is, it starts over
	//			at 01 for Matthew.
	//		BOOKNAME = lowercase bookname from the list here
	//		zzz = Chapter number, 1-based, zero-filled to 3-digits in the
	//				OT, but 2-digits in the NT, with the exception of
	//				the books with only 1 chapter (Obadiah, Philemon, 2John,
	//				3John, and Jude) which have no "_zzz" part and are the
	//				same as the book entry.

	// Book names as they appear on the Talking Bibles 1876 Russian Synodal recording files:
	const QString g_arrconstrTBRussianSynodalBooks[NUM_BK_OT_NT] = {
		"01_genesis",
		"02_exodus",
		"03_leviticus",
		"04_numbers",
		"05_deuteronomy",
		"06_joshua",
		"07_judges",
		"08_ruth",
		"09_1-samuel",
		"10_2-samuel",
		"11_1-kings",
		"12_2-kings",
		"13_1-chronicles",
		"14_2-chronicles",
		"15_ezra",
		"16_nehemiah",
		"17_esther",
		"18_job",
		"19_psalms",
		"20_proverbs",
		"21_ecclesiastes",
		"22_songofsolomon",
		"23_isaiah",
		"24_jeremiah",
		"25_lamentations",
		"26_ezekiel",
		"27_daniel",
		"28_hosea",
		"29_joel",
		"30_amos",
		"31_obadiah",
		"32_jonah",
		"33_micah",
		"34_nahum",
		"35_habakkuk",
		"36_zephaniah",
		"37_haggai",
		"38_zechariah",
		"39_malachi",
		"01_matthew",
		"02_mark",
		"03_luke",
		"04_john",
		"05_acts",
		"06_romans",
		"07_1-corinthians",
		"08_2-corinthians",
		"09_galatians",
		"10_ephesians",
		"11_philippians",
		"12_colossians",
		"13_1-thessalonians",
		"14_2-thessalonians",
		"15_1-timothy",
		"16_2-timothy",
		"17_titus",
		"18_philemon",
		"19_hebrews",
		"20_james",
		"21_1-peter",
		"22_2-peter",
		"23_1-john",
		"24_2-john",
		"25_3-john",
		"26_jude",
		"27_revelation",
	};

	const QString g_constrTBRussianSynodalURL = "https://audios.dewtronics.com/TalkingBibles/RussianSynodal1876/%1";

	// ------------------------------------------------------------------------

	// Book names as they appear on the Masoretic, Letteris Bible,
	//	Abraham Shmuelof Narration recording files:
	struct TMasoreticLetteris {
		const QString m_strIndex;
		const QString m_strName;
	} g_arrconstrMasoreticLetterisHebrewBooks[NUM_BK_OT] = {
		{ "01", "Genesis" },
		{ "02", "Exodus" },
		{ "03", "Leviticus" },
		{ "04", "Numbers" },
		{ "05", "Deuteronomy" },
		{ "06", "Joshua" },
		{ "07", "Judges" },
		{ "29", "Ruth" },
		{ "08a", "1_Samuel" },
		{ "08b", "2_Samuel" },
		{ "09a", "1_Kings" },
		{ "09b", "2_Kings" },
		{ "25a", "1_Chronicles" },
		{ "25b", "2_Chronicles" },
		{ "35a", "Ezra" },
		{ "35b", "Nehemiah" },
		{ "33", "Esther" },
		{ "27", "Job" },
		{ "26", "Psalms" },
		{ "28", "Proverbs" },
		{ "31", "Ecclesiastes" },
		{ "30", "Song_of_Songs" },
		{ "10", "Isaiah" },
		{ "11", "Jeremiah" },
		{ "32", "Lamentations" },
		{ "12", "Ezekiel" },
		{ "34", "Daniel" },
		{ "13", "Hosea" },
		{ "14", "Joel" },
		{ "15", "Amos" },
		{ "16", "Obadiah" },
		{ "17", "Jonah" },
		{ "18", "Micah" },
		{ "19", "Nahum" },
		{ "20", "Habakkuk" },
		{ "21", "Zephaniah" },
		{ "22", "Haggai" },
		{ "23", "Zechariah" },
		{ "24", "Malachi" },
	};

	//
	// TODO : We have to figure out a KJV->Masoretic remapping because the
	//	audio here uses Masoretic versification and our database has been
	//	rearchitected for KJV versification.  For example, the Masoretic
	//	has 4 chapters in Joel and the KJV has 3.  And the Masoretic has
	//	3 chapters in Malachi and the KJV has 4.  Not to mention the verse
	//	shift through other chapters from place to place.
	//	As this is currently written, it will not work for Joel and Malachi
	//	and will be confusing at the other locations.  Don't publish this
	//	on the live WebChannel until the versification mapping issue is
	//	resolved.
	//

	const QString g_constrMasoreticLetterisHebrewURL = "https://audios.dewtronics.com/HebrewMasoretic/Letteris/%1";

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
	Q_ASSERT(!pBibleDatabase.isNull());

	CRelIndex ndxDecolophonated(ndxRel);
	if (ndxDecolophonated.isColophon()) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxDecolophonated);
		if (pBook) {
			ndxDecolophonated.setChapter(pBook->m_nNumChp);
		}
	}

	QJsonArray arrBibleAudioList;

	if (ndxDecolophonated.isSet()) {
		uint32_t nTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_TESTAMENT, ndxDecolophonated).ofBible().first;
		uint32_t nBkTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_BOOK, ndxDecolophonated).ofTestament().first;
		uint32_t nBk = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_BOOK, ndxDecolophonated).ofBible().first;
		uint32_t nChp = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofBook().first;
		uint32_t nChpTst = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofTestament().first;
		uint32_t nChpBible = CRefCountCalc(pBibleDatabase.data(), CRefCountCalc::RTE_CHAPTER, ndxDecolophonated).ofBible().first;

		bool bKJVValid = (((nTst > 0) && (nTst <= 2)) && (nBkTst != 0) && ((nBk > 0) && (nBk <= NUM_BK_OT_NT)));
		if ((pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVPCE).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJVA).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_KJV1611A).m_strUUID, Qt::CaseInsensitive) != 0))
			bKJVValid = false;

		bool bRV1865Valid = ((nTst == 2) && ((nBkTst > 0) && (nBkTst <= NUM_BK_NT)));
		if (pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_RV1865mv20180504).m_strUUID, Qt::CaseInsensitive) != 0)
			bRV1865Valid = false;

		bool bRussianSynodalValid = (((nTst > 0) && (nTst <= 2)) && (nBkTst != 0) && ((nBk > 0) && (nBk <= NUM_BK_OT_NT)));
		if ((pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_RUSSYNODAL_20101106).m_strUUID, Qt::CaseInsensitive) != 0) &&
			(pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_RUSSYNODAL_20201221).m_strUUID, Qt::CaseInsensitive) != 0))
			bRussianSynodalValid = false;

		bool bHebrewMasoreticValid = ((nTst == 1) && ((nBkTst > 0) && (nBkTst <= NUM_BK_OT)));
		if (pBibleDatabase->compatibilityUUID().compare(bibleDescriptor(BDE_OSHB).m_strUUID, Qt::CaseInsensitive) != 0)
			bHebrewMasoreticValid = false;

		if (bKJVValid) {
			// Faith Comes By Hearing:
			QString strFCBHBookName = QString("%1").arg(g_arrconstrFCBHBooks[nBk-1], -12, QChar('_'));
			QString strFCBHBkTst = QString("%1%2").arg((nTst == 1) ? "A" : "B").arg(nBkTst, 2, 10, QChar('0'));
			QString strFCBHChp = ((nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_PSALMS).book()) ?
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
//			if ((nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_OBADIAH).book()) ||
//				(nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_PHILEMON).book()) ||
//				(nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_2JOHN).book()) ||
//				(nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_3JOHN).book()) ||
//				(nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_JUDE).book())) {			// No digits
//			} else if ((nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_ZEPHANIAH).book()) ||
//				(nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_2PETER).book())) {		// 1 Digit
//				strScourbyNDBkChp += QString(" %1").arg(nChp, 1, 10, QChar('0'));
//			} else if (nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_PSALMS).book()) {	// 3 Digits
//				strScourbyNDBkChp += QString(" %1").arg(nChp, 3, 10, QChar('0'));
//			} else {																			// 2 Digits
//				strScourbyNDBkChp += QString(" %1").arg(nChp, 2, 10, QChar('0'));
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
			if (nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_NUMBERS).book()) {
				strDWBkChp = QString("%1%2").arg(g_arrconstrDWBooks[nBk-1]).arg(nChp, 2, 10, QChar('0'));
			} else if ((nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_PHILEMON).book()) ||
					   (nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_2JOHN).book()) ||
					   (nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_3JOHN).book()) ||
					   (nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_JUDE).book())) {
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
													.arg(nChp, (nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_PSALMS).book()) ? 3 : 2, 10, QChar('0'));
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
													.arg(nChp, (nBk == pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_PSALMS).book()) ? 3 : 2, 10, QChar('0'));
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

		if (bRussianSynodalValid) {
			// Talking Bibles : 1876 Russian Synodal:
			if (flagsBAS & BAS_TB_1876_RUSSIAN_SYNODAL) {
				QString strTBRSTstBkChp = QString("%1/%2/%2").arg((nTst == 1) ? "1_OT" : "2_NT")
										.arg(g_arrconstrTBRussianSynodalBooks[nBk-1]);
				//	Filename format: x_TN/yy_BOOKNAME/yy_BOOKNAME_zzz.mp3
				if ((nTst == 1) &&
					(nBk != pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_OBADIAH).book())) {
					strTBRSTstBkChp.append(QString("_%1").arg(nChp, 3, 10, QChar('0')));
				} else if ((nTst == 2) &&
					((nBk != pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_PHILEMON).book()) &&
					 (nBk != pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_2JOHN).book()) &&
					 (nBk != pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_3JOHN).book()) &&
					 (nBk != pBibleDatabase->bookIndexFromOSISAbbr(OSISNAME_JUDE).book()))) {
					strTBRSTstBkChp.append(QString("_%1").arg(nChp, 2, 10, QChar('0')));
				}
				strTBRSTstBkChp.append(".mp3");

				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Talking Bible (Non-Drama)";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrTBRussianSynodalURL).arg(strTBRSTstBkChp)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}

			// 1876 Russian Synodal:
			if (flagsBAS & BAS_1876_RUSSIAN_SYNODAL) {
				QString strRSBkChp = QString("%1/%2.mp3").arg(g_arrconRussianSynodalBooks[nBk-1], 2, 10, QChar('0'))
														.arg(nChp, 2, 10, QChar('0'));

				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Russian Bible Society (Drama)";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrRussianSynodalURL).arg(strRSBkChp)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}
		}

		if (bHebrewMasoreticValid) {
			// Hebrew Masoretic, Letteris Bible, Abraham Shmuelof Narration:
			QString strHEBkChp = QString("%1_%2/t%1%3%4.mp3")
										.arg(g_arrconstrMasoreticLetterisHebrewBooks[nBk-1].m_strIndex)
										.arg(g_arrconstrMasoreticLetterisHebrewBooks[nBk-1].m_strName)
										.arg(nChp/10, 1, 16)
										.arg(nChp%10, 1, 10);
			if (flagsBAS & BAS_HEBREW_MASORETIC_LETTERIS) {
				QJsonObject objBibleAudio;
				objBibleAudio["name"] = "Hebrew Masoretic, Letteris, Abraham Shmuelof Narration";
				objBibleAudio["url"] = QString(QUrl(QString(g_constrMasoreticLetterisHebrewURL).arg(strHEBkChp)).toEncoded());
				arrBibleAudioList.append(objBibleAudio);
			}
		}
	}

	return QJsonDocument(arrBibleAudioList).toJson(QJsonDocument::Compact);
}

// ============================================================================
