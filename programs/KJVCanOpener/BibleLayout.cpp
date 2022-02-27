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

#include "BibleLayout.h"

#include "dbstruct.h"

// ============================================================================

const TBibleVersificationList g_arrBibleVersifications =
{
	CBibleVersifications::tr("King James"),
	CBibleVersifications::tr("Hebrew Masoretic"),
};

// ============================================================================

const TBibleTestamentNameList g_arrBibleTestamentNames =
{
	CBibleTestaments::tr("Old Testament"),
	CBibleTestaments::tr("New Testament"),
	CBibleTestaments::tr("Apocrypha/Deuterocanon"),
};

QString translatedBibleTestamentName(unsigned int nTst)
{
	if ((nTst < 1) || (nTst > static_cast<unsigned int>(g_arrBibleTestamentNames.size()))) return QString();
	return g_arrBibleTestamentNames.at(nTst-1);
}

// ============================================================================

const TBibleBookCategoryGroupList g_arrBibleBookCategoryGroups =
{
	CBibleBookCategoryGroups::tr("King James"),
	CBibleBookCategoryGroups::tr("Hebrew Masoretic"),
};

// ============================================================================

const TBibleBookCategoryList g_arrBibleBookCategories =
{
	CBibleBookCategories::tr("Law"),
	CBibleBookCategories::tr("Historical"),
	CBibleBookCategories::tr("Wisdom/Poetic"),
	CBibleBookCategories::tr("Writings"),
	CBibleBookCategories::tr("Major Prophets"),
	CBibleBookCategories::tr("Minor Prophets"),
	CBibleBookCategories::tr("Prophets"),
	CBibleBookCategories::tr("Gospels"),
	CBibleBookCategories::tr("Pauline Epistles"),
	CBibleBookCategories::tr("General Epistles"),
	CBibleBookCategories::tr("Apocalyptic Epistle"),
};

// ============================================================================

const TBibleBookList g_arrBibleBooks =
{
	// ---- Begin Old Testament:
	{ 0, { "Gen" }, "GEN", BBCE_LAW, CBibleBookNames::tr("Genesis"), CBibleBookAbbr::tr("Gen;Gn"), CBibleBookDescription::tr("The First Book of Moses") },
	{ 0, { "Exod" }, "EXOD", BBCE_LAW, CBibleBookNames::tr("Exodus"), CBibleBookAbbr::tr("Exod;Exo;Ex"), CBibleBookDescription::tr("The Second Book of Moses") },
	{ 0, { "Lev" }, "LEV", BBCE_LAW, CBibleBookNames::tr("Leviticus"), CBibleBookAbbr::tr("Lev;Lv"), CBibleBookDescription::tr("The Third Book of Moses") },
	{ 0, { "Num" }, "NUM", BBCE_LAW, CBibleBookNames::tr("Numbers"), CBibleBookAbbr::tr("Num;Nm"), CBibleBookDescription::tr("The Fourth Book of Moses") },
	{ 0, { "Deut" }, "DEUT", BBCE_LAW, CBibleBookNames::tr("Deuteronomy"), CBibleBookAbbr::tr("Deut;Deu;Dt"), CBibleBookDescription::tr("The Fifth Book of Moses") },
	{ 0, { "Josh" }, "JOSH", BBCE_HISTORICAL, CBibleBookNames::tr("Joshua"), CBibleBookAbbr::tr("Josh;Jos;Jo"), "" },
	{ 0, { "Judg" }, "JUDG", BBCE_HISTORICAL, CBibleBookNames::tr("Judges"), CBibleBookAbbr::tr("Judg;Jdg;Jgs"), "" },
	{ 0, { "Ruth" }, "RUTH", BBCE_HISTORICAL, CBibleBookNames::tr("Ruth"), CBibleBookAbbr::tr("Ruth;Rut;Ru"), "" },
	{ 0, { "1Sam" }, "SAM1", BBCE_HISTORICAL, CBibleBookNames::tr("1 Samuel"), CBibleBookAbbr::tr("1Sam;1Sm"), CBibleBookDescription::tr("The First Book of Samuel Otherwise Called, The First Book of the Kings") },
	{ 0, { "2Sam" }, "SAM2", BBCE_HISTORICAL, CBibleBookNames::tr("2 Samuel"), CBibleBookAbbr::tr("2Sam;2Sm"), CBibleBookDescription::tr("The Second Book of Samuel Otherwise Called, The Second Book of the Kings") },
	{ 0, { "1Kgs" }, "KGS1", BBCE_HISTORICAL, CBibleBookNames::tr("1 Kings"), CBibleBookAbbr::tr("1Kgs"), CBibleBookDescription::tr("The First Book of the Kings Commonly Called, The Third Book of the Kings") },
	{ 0, { "2Kgs" }, "KGS2", BBCE_HISTORICAL, CBibleBookNames::tr("2 Kings"), CBibleBookAbbr::tr("2Kgs"), CBibleBookDescription::tr("The Second Book of the Kings Commonly Called, The Fourth Book of the Kings") },
	{ 0, { "1Chr" }, "CHR1", BBCE_HISTORICAL, CBibleBookNames::tr("1 Chronicles"), CBibleBookAbbr::tr("1Chr;1Chron;1Ch"), CBibleBookDescription::tr("The First Book of the Chronicles") },
	{ 0, { "2Chr" }, "CHR2", BBCE_HISTORICAL, CBibleBookNames::tr("2 Chronicles"), CBibleBookAbbr::tr("2Chr;2Chron;2Ch"), CBibleBookDescription::tr("The Second Book of the Chronicles") },
	{ 0, { "Ezra" }, "EZRA", BBCE_HISTORICAL, CBibleBookNames::tr("Ezra"), CBibleBookAbbr::tr("Ezra;Ezr"), "" },
	{ 0, { "Neh" }, "NEH", BBCE_HISTORICAL, CBibleBookNames::tr("Nehemiah"), CBibleBookAbbr::tr("Neh"), "" },
	{ 0, { "Esth" }, "ESTH", BBCE_HISTORICAL, CBibleBookNames::tr("Esther"), CBibleBookAbbr::tr("Est;Esth"), "" },
	{ 0, { "Job" }, "JOB", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Job"), CBibleBookAbbr::tr("Job;Jb"), "" },
	{ 0, { "Ps" }, "PS", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Psalms"), CBibleBookAbbr::tr("Ps;Pss"), "" },
	{ 0, { "Prov" }, "PROV", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Proverbs"), CBibleBookAbbr::tr("Prov;Prv;Pv"), "" },
	{ 0, { "Eccl" }, "ECCL", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Ecclesiastes"), CBibleBookAbbr::tr("Eccl;Eccles"), CBibleBookDescription::tr("Ecclesiastes; Or, The Preacher") },
	{ 0, { "Song" }, "SONG", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Song Of Solomon"), CBibleBookAbbr::tr("Song;Sg"), "" },
	{ 0, { "Isa" }, "ISA", BBCE_MAJOR_PROPHETS, CBibleBookNames::tr("Isaiah"), CBibleBookAbbr::tr("Isa;Is"), CBibleBookDescription::tr("The Book of the Prophet Isaiah") },
	{ 0, { "Jer" }, "JER", BBCE_MAJOR_PROPHETS, CBibleBookNames::tr("Jeremiah"), CBibleBookAbbr::tr("Jer"), CBibleBookDescription::tr("The Book of the Prophet Jeremiah") },
	{ 0, { "Lam" }, "LAM", BBCE_MAJOR_PROPHETS, CBibleBookNames::tr("Lamentations"), CBibleBookAbbr::tr("Lam"), CBibleBookDescription::tr("The Lamentations of Jeremiah") },
	{ 0, { "Ezek" }, "EZEK", BBCE_MAJOR_PROPHETS, CBibleBookNames::tr("Ezekiel"), CBibleBookAbbr::tr("Ezek;Eze;Ez"), CBibleBookDescription::tr("The Book of the Prophet Ezekiel") },
	{ 0, { "Dan" }, "DAN", BBCE_MAJOR_PROPHETS, CBibleBookNames::tr("Daniel"), CBibleBookAbbr::tr("Dan;Dn"), CBibleBookDescription::tr("The Book of <i>the Prophet</i> Daniel") },
	{ 0, { "Hos" }, "HOS", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Hosea"), CBibleBookAbbr::tr("Hos"), "" },
	{ 0, { "Joel" }, "JOEL", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Joel"), CBibleBookAbbr::tr("Joel;Joe;Jl"), "" },
	{ 0, { "Amos" }, "AMOS", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Amos"), CBibleBookAbbr::tr("Amos;Amo;Am"), "" },
	{ 0, { "Obad" }, "OBAD", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Obadiah"), CBibleBookAbbr::tr("Obad;Oba;Ob"), "" },
	{ 0, { "Jonah" }, "JONAH", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Jonah"), CBibleBookAbbr::tr("Jonah;Jona;Jon"), "" },
	{ 0, { "Mic" }, "MIC", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Micah"), CBibleBookAbbr::tr("Mic;Mi"), "" },
	{ 0, { "Nah" }, "NAH", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Nahum"), CBibleBookAbbr::tr("Nah;Na"), "" },
	{ 0, { "Hab" }, "HAB", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Habakkuk"), CBibleBookAbbr::tr("Hab;Hb"), "" },
	{ 0, { "Zeph" }, "ZEPH", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Zephaniah"), CBibleBookAbbr::tr("Zeph;Zep"), "" },
	{ 0, { "Hag" }, "HAG", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Haggai"), CBibleBookAbbr::tr("Hag;Hg"), "" },
	{ 0, { "Zech" }, "ZECH", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Zechariah"), CBibleBookAbbr::tr("Zech;Zec"), "" },
	{ 0, { "Mal" }, "MAL", BBCE_MINOR_PROPHETS, CBibleBookNames::tr("Malachi"), CBibleBookAbbr::tr("Mal"), "" },
	// ---- Begin New Testament:
	{ 0, { "Matt" }, "MATT", BBCE_GOSPELS, CBibleBookNames::tr("Matthew"), CBibleBookAbbr::tr("Matt;Mt"), CBibleBookDescription::tr("The Gospel According to Saint Matthew") },
	{ 0, { "Mark" }, "MARK", BBCE_GOSPELS, CBibleBookNames::tr("Mark"), CBibleBookAbbr::tr("Mark;Mk"), CBibleBookDescription::tr("The Gospel According to Saint Mark") },
	{ 0, { "Luke" }, "LUKE", BBCE_GOSPELS, CBibleBookNames::tr("Luke"), CBibleBookAbbr::tr("Luke;Lk"), CBibleBookDescription::tr("The Gospel According to Saint Luke") },
	{ 0, { "John" }, "JOHN", BBCE_GOSPELS, CBibleBookNames::tr("John"), CBibleBookAbbr::tr("John;Jhn;Jn"), CBibleBookDescription::tr("The Gospel According to Saint John") },
	{ 0, { "Acts" }, "ACTS", BBCE_HISTORICAL, CBibleBookNames::tr("Acts"), CBibleBookAbbr::tr("Acts"), CBibleBookDescription::tr("The Acts of the Apostles") },
	{ 0, { "Rom" }, "ROM", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Romans"), CBibleBookAbbr::tr("Rom"), CBibleBookDescription::tr("The Epistle of Paul the Apostle to the Romans") },
	{ 0, { "1Cor" }, "COR1", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("1 Corinthians"), CBibleBookAbbr::tr("1Cor"), CBibleBookDescription::tr("The First Epistle of Paul the Apostle to the Corinthians") },
	{ 0, { "2Cor" }, "COR2", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("2 Corinthians"), CBibleBookAbbr::tr("2Cor"), CBibleBookDescription::tr("The Second Epistle of Paul the Apostle to the Corinthians") },
	{ 0, { "Gal" }, "GAL", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Galatians"), CBibleBookAbbr::tr("Gal"), CBibleBookDescription::tr("The Epistle of Paul the Apostle to the Galatians") },
	{ 0, { "Eph" }, "EPH", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Ephesians"), CBibleBookAbbr::tr("Eph"), CBibleBookDescription::tr("The Epistle of Paul the Apostle to the Ephesians") },
	{ 0, { "Phil" }, "PHIL", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Philippians"), CBibleBookAbbr::tr("Phil"), CBibleBookDescription::tr("The Epistle of Paul the Apostle to the Philippians") },
	{ 0, { "Col" }, "COL", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Colossians"), CBibleBookAbbr::tr("Col"), CBibleBookDescription::tr("The Epistle of Paul the Apostle to the Colossians") },
	{ 0, { "1Thess" }, "THESS1", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("1 Thessalonians"), CBibleBookAbbr::tr("1Thess;1Thes;1Th"), CBibleBookDescription::tr("The First Epistle of Paul the Apostle to the Thessalonians") },
	{ 0, { "2Thess" }, "THESS2", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("2 Thessalonians"), CBibleBookAbbr::tr("2Thess;2Thes;2Th"), CBibleBookDescription::tr("The Second Epistle of Paul the Apostle to the Thessalonains") },
	{ 0, { "1Tim" }, "TIM1", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("1 Timothy"), CBibleBookAbbr::tr("1Tim;1Tm"), CBibleBookDescription::tr("The First Epistle of Paul the Apostle to Timothy") },
	{ 0, { "2Tim" }, "TIM2", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("2 Timothy"), CBibleBookAbbr::tr("2Tim;2Tm"), CBibleBookDescription::tr("The Second Epistle of Paul the Apostle to Timothy") },
	{ 0, { "Titus" }, "TITUS", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Titus"), CBibleBookAbbr::tr("Titus;Ti"), CBibleBookDescription::tr("The Epistle of Paul to Titus") },
	{ 0, { "Phlm" }, "PHLM", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Philemon"), CBibleBookAbbr::tr("Phlm;Philem"), CBibleBookDescription::tr("The Epistle of Paul to Philemon") },
	{ 0, { "Heb" }, "HEB", BBCE_PAULINE_EPISTLES, CBibleBookNames::tr("Hebrews"), CBibleBookAbbr::tr("Heb"), CBibleBookDescription::tr("The Epistle of Paul the Apostle to the Hebrews") },
	{ 0, { "Jas" }, "JAS", BBCE_GENERAL_EPISTLES, CBibleBookNames::tr("James"), CBibleBookAbbr::tr("Jas"), CBibleBookDescription::tr("The General Epistle of James") },
	{ 0, { "1Pet" }, "PET1", BBCE_GENERAL_EPISTLES, CBibleBookNames::tr("1 Peter"), CBibleBookAbbr::tr("1Pet;1Pt"), CBibleBookDescription::tr("The First General Epistle of Peter") },
	{ 0, { "2Pet" }, "PET2", BBCE_GENERAL_EPISTLES, CBibleBookNames::tr("2 Peter"), CBibleBookAbbr::tr("2Pet;2Pt"), CBibleBookDescription::tr("The Second General Epistle of Peter") },
	{ 0, { "1John" }, "JOHN1", BBCE_GENERAL_EPISTLES, CBibleBookNames::tr("1 John"), CBibleBookAbbr::tr("1John;1Jn"), CBibleBookDescription::tr("The First General Epistle of John") },
	{ 0, { "2John" }, "JOHN2", BBCE_GENERAL_EPISTLES, CBibleBookNames::tr("2 John"), CBibleBookAbbr::tr("2John;2Jn"), CBibleBookDescription::tr("The Second General Epistle of John") },
	{ 0, { "3John" }, "JOHN3", BBCE_GENERAL_EPISTLES, CBibleBookNames::tr("3 John"), CBibleBookAbbr::tr("3John;3Jn"), CBibleBookDescription::tr("The Third General Epistle of John") },
	{ 0, { "Jude" }, "JUDE", BBCE_GENERAL_EPISTLES, CBibleBookNames::tr("Jude"), CBibleBookAbbr::tr("Jude"), CBibleBookDescription::tr("The General Epistle of Jude") },
	{ 0, { "Rev" }, "REV", BBCE_APOCALYPTIC_EPISTLE, CBibleBookNames::tr("Revelation"), CBibleBookAbbr::tr("Rev;Rv;Apoc"), CBibleBookDescription::tr("The Revelation of Jesus Christ") },
	// ---- Begin Apocrypha/Deuterocanon:
	{ 0, { "1Esd" }, "ESD1", BBCE_HISTORICAL, CBibleBookNames::tr("1 Esdras"), CBibleBookAbbr::tr("1Esd;1Es"), CBibleBookDescription::tr("The First Book of Esdras") },
	{ 0, { "2Esd" }, "ESD2", BBCE_HISTORICAL, CBibleBookNames::tr("2 Esdras"), CBibleBookAbbr::tr("2Esd;2Es"), CBibleBookDescription::tr("The Second Book of Esdras") },
	{ 0, { "Tob" }, "TOB", BBCE_HISTORICAL, CBibleBookNames::tr("Tobit"), CBibleBookAbbr::tr("Tob;Tb"), CBibleBookDescription::tr("The Book of Tobit") },
	{ 0, { "Jdt" }, "JDT", BBCE_HISTORICAL, CBibleBookNames::tr("Judith"), CBibleBookAbbr::tr("Jdt;Jth"), CBibleBookDescription::tr("The Book of Judith") },
	{ CRelIndex(0, 10, 4, 0).index(), { "EsthGr", "AddEsth" }, "ESTHGR", BBCE_HISTORICAL, CBibleBookNames::tr("Esther (Greek)"), CBibleBookAbbr::tr("EsthGr;AddEst;AddEsth"), CBibleBookDescription::tr("The Rest of the Chapters of the Book of Esther, which are found neither in the Hebrew, nor in the Chaldee") },
	{ 0, { "Wis" }, "WIS", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Wisdom"), CBibleBookAbbr::tr("Wis;Ws"), CBibleBookDescription::tr("The Book of Wisdom or The Wisdom of Solomon") },
	{ 0, { "Sir" }, "SIR", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Sirach"), CBibleBookAbbr::tr("Sir;Ecclus"), CBibleBookDescription::tr("The Wisdom of Jesus the Son of Sirach, or Ecclesiasticus") },
	{ 0, { "Bar" }, "BAR", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Baruch"), CBibleBookAbbr::tr("Bar;Ba"), CBibleBookDescription::tr("The Book of Baruch (with The Epistle of Jeremy)") },
	{ 0, { "PrAzar" }, "PRAZAR", BBCE_PROPHETS, CBibleBookNames::tr("Prayer of Azariah"), CBibleBookAbbr::tr("PrAzar;S3Y;SG3;AddDan"), CBibleBookDescription::tr("The Prayer of Azariah (Song of the Three Young Men)") },
	{ 0, { "Sus" }, "SUS", BBCE_PROPHETS, CBibleBookNames::tr("Susanna"), CBibleBookAbbr::tr("Sus"), CBibleBookDescription::tr("The History of Susanna [in Daniel]") },
	{ 0, { "Bel" }, "BEL", BBCE_PROPHETS, CBibleBookNames::tr("Bel and the Dragon"), CBibleBookAbbr::tr("Bel"), CBibleBookDescription::tr("The Book of Bel and the Dragon [in Daniel]") },
	{ 0, { "PrMan" }, "PRMAN", BBCE_PROPHETS, CBibleBookNames::tr("Prayer of Manasses"), CBibleBookAbbr::tr("PrMan"), CBibleBookDescription::tr("The Prayer of Manasseh, or, The Prayer of Manasses King of Judah, When He was Held Captive in Babylon") },
	{ 0, { "1Macc" }, "MACC1", BBCE_HISTORICAL, CBibleBookNames::tr("1 Maccabees"), CBibleBookAbbr::tr("1Macc;1Mc;1Ma"), CBibleBookDescription::tr("The First Book of the Maccabees") },
	{ 0, { "2Macc" }, "MACC2", BBCE_HISTORICAL, CBibleBookNames::tr("2 Maccabees"), CBibleBookAbbr::tr("2Macc;2Mc;2Ma"), CBibleBookDescription::tr("The Second Book of the Maccabees") },
	{ 0, { "3Macc" }, "MACC3", BBCE_HISTORICAL, CBibleBookNames::tr("3 Maccabees"), CBibleBookAbbr::tr("3Macc;3Mc;3Ma"), CBibleBookDescription::tr("The Third Book of the Maccabees") },
	{ 0, { "4Macc" }, "MACC4", BBCE_HISTORICAL, CBibleBookNames::tr("4 Maccabees"), CBibleBookAbbr::tr("4Macc;4Mc;4Ma"), CBibleBookDescription::tr("The Fourth Book of the Maccabees") },
	{ 0, { "Ps151", "AddPs" }, "PS151", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Psalm 151"), CBibleBookAbbr::tr("Ps151"), CBibleBookDescription::tr("Psalm 151") },
	{ 0, { "Odes" }, "ODES", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Odes"), CBibleBookAbbr::tr("Odes;Oda"), CBibleBookDescription::tr("Book of Odes") },
	{ 0, { "PsSal", "PssSol", "PsSol" }, "PSS", BBCE_WISDOM_POETIC, CBibleBookNames::tr("Psalms of Salomon"), CBibleBookAbbr::tr("PssSol;PsSal;PsSol"), CBibleBookDescription::tr("Psalms of Salomon") },
};

// ============================================================================

CKJVBibleChapterVerseCounts::CKJVBibleChapterVerseCounts()
{
	static const QString arrChapterVerseCounts[NUM_BK] =
	{
		// ---- Begin Old Testament:
		/* Gen  */ "31,25,24,26,32,22,24,22,29,32,32,20,18,24,21,16,27,33,38,18,34,24,20,67,34,35,46,22,35,43,55,32,20,31,29,43,36,30,23,23,57,38,34,34,28,34,31,22,33,26",
		/* Exod */ "22,25,22,31,23,30,25,32,35,29,10,51,22,31,27,36,16,27,25,26,36,31,33,18,40,37,21,43,46,38,18,35,23,35,35,38,29,31,43,38",
		/* Lev  */ "17,16,17,35,19,30,38,36,24,20,47,8,59,57,33,34,16,30,37,27,24,33,44,23,55,46,34",
		/* Num  */ "54,34,51,49,31,27,89,26,23,36,35,16,33,45,41,50,13,32,22,29,35,41,30,25,18,65,23,31,40,16,54,42,56,29,34,13",
		/* Deut */ "46,37,29,49,33,25,26,20,29,22,32,32,18,29,23,22,20,22,21,20,23,30,25,22,19,19,26,68,29,20,30,52,29,12",
		/* Josh */ "18,24,17,24,15,27,26,35,27,43,23,24,33,15,63,10,18,28,51,9,45,34,16,33",
		/* Judg */ "36,23,31,24,31,40,25,35,57,18,40,15,25,20,20,31,13,31,30,48,25",
		/* Ruth */ "22,23,18,22",
		/* 1Sam */ "28,36,21,22,12,21,17,22,27,27,15,25,23,52,35,23,58,30,24,42,15,23,29,22,44,25,12,25,11,31,13",
		/* 2Sam */ "27,32,39,12,25,23,29,18,13,19,27,31,39,33,37,23,29,33,43,26,22,51,39,25",
		/* 1Kgs */ "53,46,28,34,18,38,51,66,28,29,43,33,34,31,34,34,24,46,21,43,29,53",
		/* 2Kgs */ "18,25,27,44,27,33,20,29,37,36,21,21,25,29,38,20,41,37,37,21,26,20,37,20,30",
		/* 1Chr */ "54,55,24,43,26,81,40,40,44,14,47,40,14,17,29,43,27,17,19,8,30,19,32,31,31,32,34,21,30",
		/* 2Chr */ "17,18,17,22,14,42,22,18,31,19,23,16,22,15,19,14,19,34,11,37,20,12,21,27,28,23,9,27,36,27,21,33,25,33,27,23",
		/* Ezra */ "11,70,13,24,17,22,28,36,15,44",
		/* Neh  */ "11,20,32,23,19,19,73,18,38,39,36,47,31",
		/* Esth */ "22,23,15,17,14,14,10,17,32,3",
		/* Job  */ "22,13,26,21,27,30,21,22,35,22,20,25,28,22,35,22,16,21,29,29,34,30,17,25,6,14,23,28,25,31,40,22,33,37,16,33,24,41,30,24,34,17",
		/* Ps   */ "6,12,8,8,12,10,17,9,20,18,7,8,6,7,5,11,15,50,14,9,13,31,6,10,22,12,14,9,11,12,24,11,22,22,28,12,40,22,13,17,13,11,5,26,17,11,9,14,20,23,19,9,6,7,23,13,11,11,17,12,8,12,11,10,13,20,7,35,36,5,24,20,28,23,10,12,20,72,13,19,16,8,18,12,13,17,7,18,52,17,16,15,5,23,11,13,12,9,9,5,8,28,22,35,45,48,43,13,31,7,10,10,9,8,18,19,2,29,176,7,8,9,4,8,5,6,5,6,8,8,3,18,3,3,21,26,9,8,24,13,10,7,12,15,21,10,20,14,9,6",
		/* Prov */ "33,22,35,27,23,35,27,36,18,32,31,28,25,35,33,33,28,24,29,30,31,29,35,34,28,28,27,28,27,33,31",
		/* Eccl */ "18,26,22,16,20,12,29,17,18,20,10,14",
		/* Song */ "17,17,11,16,16,13,13,14",
		/* Isa  */ "31,22,26,6,30,13,25,22,21,34,16,6,22,32,9,14,14,7,25,6,17,25,18,23,12,21,13,29,24,33,9,20,24,17,10,22,38,22,8,31,29,25,28,28,25,13,15,22,26,11,23,15,12,17,13,12,21,14,21,22,11,12,19,12,25,24",
		/* Jer  */ "19,37,25,31,31,30,34,22,26,25,23,17,27,22,21,21,27,23,15,18,14,30,40,10,38,24,22,17,32,24,40,44,26,22,19,32,21,28,18,16,18,22,13,30,5,28,7,47,39,46,64,34",
		/* Lam  */ "22,22,66,22,22",
		/* Ezek */ "28,10,27,17,17,14,27,18,11,22,25,28,23,23,8,63,24,32,14,49,32,31,49,27,17,21,36,26,21,26,18,32,33,31,15,38,28,23,29,49,26,20,27,31,25,24,23,35",
		/* Dan  */ "21,49,30,37,31,28,28,27,27,21,45,13",
		/* Hos  */ "11,23,5,19,15,11,16,14,17,15,12,14,16,9",
		/* Joel */ "20,32,21",
		/* Amos */ "15,16,15,13,27,14,17,14,15",
		/* Obad */ "21",
		/* Jonah */ "17,10,10,11",
		/* Mic  */ "16,13,12,13,15,16,20",
		/* Nah  */ "15,13,19",
		/* Hab  */ "17,20,19",
		/* Zeph */ "18,15,20",
		/* Hag  */ "15,23",
		/* Zech */ "21,13,10,14,11,15,14,23,17,12,17,14,9,21",
		/* Mal  */ "14,17,18,6",
		// ---- Begin New Testament:
		/* Matt */ "25,23,17,25,48,34,29,34,38,42,30,50,58,36,39,28,27,35,30,34,46,46,39,51,46,75,66,20",
		/* Mark */ "45,28,35,41,43,56,37,38,50,52,33,44,37,72,47,20",
		/* Luke */ "80,52,38,44,39,49,50,56,62,42,54,59,35,35,32,31,37,43,48,47,38,71,56,53",
		/* John */ "51,25,36,54,47,71,53,59,41,42,57,50,38,31,27,33,26,40,42,31,25",
		/* Acts */ "26,47,26,37,42,15,60,40,43,48,30,25,52,28,41,40,34,28,41,38,40,30,35,27,27,32,44,31",
		/* Rom  */ "32,29,31,25,21,23,25,39,33,21,36,21,14,23,33,27",
		/* 1Cor */ "31,16,23,21,13,20,40,13,27,33,34,31,13,40,58,24",
		/* 2Cor */ "24,17,18,18,21,18,16,24,15,18,33,21,14",
		/* Gal  */ "24,21,29,31,26,18",
		/* Eph  */ "23,22,21,32,33,24",
		/* Phil */ "30,30,21,23",
		/* Col  */ "29,23,25,18",
		/* 1Thess */ "10,20,13,18,28",
		/* 2Thess */ "12,17,18",
		/* 1Tim */ "20,15,16,16,25,21",
		/* 2Tim */ "18,26,17,22",
		/* Titus */ "16,15,15",
		/* Phlm */ "25",
		/* Heb  */ "14,18,19,16,14,20,28,13,28,39,40,29,25",
		/* Jas  */ "27,26,18,17,20",
		/* 1Pet */ "25,25,22,19,14",
		/* 2Pet */ "21,22,18",
		/* 1John */ "10,29,24,21,21",
		/* 2John */ "13",
		/* 3John */ "14",
		/* Jude */ "25",
		/* Rev  */ "20,29,22,11,14,17,17,13,21,11,19,17,18,20,8,21,18,24,21,15,27,21",
		// ---- Begin Apocrypha/Deuterocanon:
		/* 1Esd */ "58,30,24,63,73,34,15,96,55",
		/* 2Esd */ "40,48,36,52,56,59,70,63,47,59,46,51,58,48,63,78",
		/* Tob  */ "22,14,17,21,22,17,18,21,6,12,19,22,18,15",
		/* Jdt  */ "16,28,10,15,24,21,32,36,14,23,23,20,20,19,13,25",
		/* EsthGr */ "0,0,0,0,0,0,0,0,0,13,12,6,18,19,16,24",			// 	(AddEsth) (starts at 10:4)
		/* Wis  */ "16,24,19,20,23,25,30,21,18,21,26,27,19,31,19,29,21,25,22",
		/* Sir  */ "30,18,31,31,15,37,36,19,18,31,34,18,26,27,20,30,32,33,30,32,28,27,28,34,26,29,30,26,28,25,31,24,31,26,20,26,31,34,35,30,24,25,33,22,26,20,25,25,16,29,30",
		/* Bar  */ "22,35,37,37,9,73",
		/* PrAzar */ "68",
		/* Sus  */ "64",
		/* Bel  */ "42",
		/* PrMan */ "14",
		/* 1Macc */ "64,70,60,61,68,63,50,32,73,89,74,53,53,49,41,24",
		/* 2Macc */ "36,32,40,50,27,31,42,36,29,38,38,45,26,46,39",
		// ---- Above is in the KJVA, Below is not
		/* 3Macc */ "29,33,30,21,51,41,23",		// 7 Chapters
		/* 4Macc */ "35,24,21,26,38,35,25,29,32,21,27,20,27,20,32,25,24,24",	// 18 Chapters
		/* Ps151 */ "16",	// Is divided into 7 verses for 151a (Hebrew), 2 for 151b (Hebrew and Syriac), and 7 (Greek)
		/* Odes  */ "5,0,11,15,15,18,26,22,12,6,24,13,4,10,10,20,17,16,11,10,9,12,22,14,12,13,3,20,11,7,13,3,13,6,7,8,4,22,13,6,16,20",		// 42 Odes
		/* PsSal */ "8,37,12,25,19,6,10,34,11,8,9,6,12,10,13,15,46,12",	// 18 Psalms
	};

	for (uint32_t ndx = 0; ndx < NUM_BK; ++ndx) {
		push_back(arrChapterVerseCounts[ndx].split(","));
	}
}

const CKJVBibleChapterVerseCounts *CKJVBibleChapterVerseCounts::instance()
{
	static const CKJVBibleChapterVerseCounts g_KJVBibleChapterVerseCounts;
	return &g_KJVBibleChapterVerseCounts;
}

// ============================================================================
