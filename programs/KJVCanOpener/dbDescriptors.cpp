/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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

#include "dbDescriptors.h"

#include <QObject>

#include <assert.h>

// ============================================================================

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

// ============================================================================

QString g_strBibleDatabasePath;
QString g_strDictionaryDatabasePath;

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Bible Database Descriptor Constants:
	// ------------------------------------
	const char *constUUID_SPECIAL_TEST =		"00000000-0000-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJV =					"85D8A6B0-E670-11E2-A28F-0800200C9A66";
	const char *constUUID_RVG2010_20140126 =	"9233CB60-141A-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJF2006 =				"31FC2ED0-141B-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJVPCE =				"C9BA8970-A114-11E3-A5E2-0800200C9A66";
	const char *constUUID_KJVA =				"B93D0E40-BA16-11E3-A5E2-0800200C9A66";
	const char *constUUID_UKJV =				"BA852FE0-C762-11E3-9C1A-0800200C9A66";
	const char *constUUID_GERLUT1545 =			"D7376840-C75F-11E3-9C1A-0800200C9A66";
	const char *constUUID_RV1865lcbp20100713 =	"9378F5B0-CE8F-11E3-9C1A-0800200C9A66";
	const char *constUUID_RV1602Prrb20110825 =	"94B7B600-CF39-11E3-9C1A-0800200C9A66";
	const char *constUUID_GERSCH2000 =			"D326F220-CFFC-11E3-9C1A-0800200C9A66";
	const char *constUUID_KJV1611A =			"8BFE29A0-D014-11E3-9C1A-0800200C9A66";
	const char *constUUID_KJV1611 =				"8D999FF0-D650-11E3-9C1A-0800200C9A66";
	const char *constUUID_RV1865sbv20140622 =	"919DAB50-1151-11E4-9191-0800200C9A66";
	const char *constUUID_RVG2010_20140705 =	"92EF65E0-56F0-11E4-8ED6-0800200C9A66";
	const char *constUUID_KJF2015 =				"36708290-9BB0-11E4-BD06-0800200C9A66";
	const char *constUUID_RVG2010_20150120 =	"925744F0-6B1B-11E5-A837-0800200C9A66";
	const char *constUUID_TR_20140413_X1 =		"4D025330-5BAC-11E7-9598-0800200C9A66";
	const char *constUUID_TR_20140413_X2 =		"4C0C5630-5C77-11E7-9598-0800200C9A66";
	const char *constUUID_SPMT_20120627 =		"5939EB40-5BAC-11E7-9598-0800200C9A66";
	const char *constUUID_LXX_20080722 =		"4F4BD170-6264-11E7-9598-0800200C9A66";
	const char *constUUID_RV1865mv20180504 =	"28A10630-5728-11E8-B566-0800200C9A66";

	// Use the RVG2010-201401026 UUID for the highlighter tag for RVG2010 in general.  Even
	//		though they technically aren't compatible, since we've already released the
	//		preview edition with the 20140126 database, people may already have done some
	//		highlighting in their Spanish Bible.  SO, we'll adopt it as the highlighter
	//		UUID for the 20140705 and subsequent releases.  This is what happens when
	//		"The Word" keeps changing (ugh!):
	const char *constUUID_RVG2010 = constUUID_RVG2010_20140126;

	// Use the RV1865mv20180504 UUID for the highlighter tag for all RV1865mv, 20180504 and
	//		newer in general.  Even though they technically aren't compatible.  This will
	//		exclude the original sbv20140622 edition, which was only ever published on
	//		KJPBS WebChannel and shouldn't have any highlighting associated with it.  The
	//		new versions have insisted on returning to nonstandard versification and this
	//		UUID will become the "standard" for that highlighter UUID tag:
	const char *constUUID_RV1865mv = constUUID_RV1865mv20180504;

	const TBibleDescriptor constBibleDescriptors[] =
	{
		// Special Test Value:
		{ BibleTypeOptionsFlags(BTO_SpecialTest), "SPECIAL", "en", QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Bible Database"), constUUID_SPECIAL_TEST, "bbl-specTest.s3db", "bbl-specTest.ccdb", constUUID_SPECIAL_TEST },
		// KJV:
		{ BibleTypeOptionsFlags(BTO_AutoLoad), "KJV", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769)"), constUUID_KJV, "bbl-kjv1769.s3db", "bbl-kjv1769.ccdb", constUUID_KJV },
		// RVG2010-20140126:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "RVG2010-20140126", "es", QString::fromUtf8("Reina-Valera Gómez 2010"), QString::fromUtf8("Reina-Valera Gómez 2010 Version (20140126)"), constUUID_RVG2010_20140126, "bbl-rvg2010-20140126.s3db", "bbl-rvg2010-20140126.ccdb", constUUID_RVG2010 },
		// KJF2006:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "KJF2006", "fr", QString::fromUtf8("King James Française 2006"), QString::fromUtf8("la Bible King James Française, édition 2006"), constUUID_KJF2006, "bbl-kjf2006.s3db", "bbl-kjf2006.ccdb", constUUID_KJF2006 },
		// KJVPureCambridge:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "KJVPCE", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Pure Cambridge Edition"), constUUID_KJVPCE, "bbl-kjvpce.s3db", "bbl-kjvpce.ccdb", constUUID_KJV },
		// KJVA (KJV 1769 with Apocrypha):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "KJVA", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769) w/Apocrypha"), constUUID_KJVA, "bbl-kjva.s3db", "bbl-kjva.ccdb", constUUID_KJV },
		// UKJV (Updated King James Version):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "UKJV", "en", QString::fromUtf8("King James"), QString::fromUtf8("Updated King James Version"), constUUID_UKJV, "bbl-ukjv.s3db", "bbl-ukjv.ccdb", constUUID_UKJV },
		// GERLUT1545 (German Luther 1545):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "GERLUT1545", "de", QString::fromUtf8("1545 Luther Bibelübersetzung"), QString::fromUtf8("German Unrevidierte Luther Übersetzung von 1545"), constUUID_GERLUT1545, "bbl-gerlut1545.s3db", "bbl-gerlut1545.ccdb", constUUID_GERLUT1545 },
		// RV1865-lcbp20100713 (Reina-Valera 1865, Local Church Bible Publishers 2010-07-13 Release)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "RV1865-lcbp20100713", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Local Church Bible Publishers (20100713)"), constUUID_RV1865lcbp20100713, "bbl-rv1865lcbp20100713.s3db", "bbl-rv1865lcbp20100713.ccdb", constUUID_RV1865lcbp20100713 },
		// RV1602P (Reina-Valera 1602 Purificada)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "RV1602P-rrb20110825", "es", QString::fromUtf8("Reina-Valera 1602 Purificada"), QString::fromUtf8("Reina-Valera 1602 Purificada (20110825)"), constUUID_RV1602Prrb20110825, "bbl-rv1602p.s3db", "bbl-rv1602p.ccdb", constUUID_RV1602Prrb20110825 },
		// GERSCH2000 (German Schlachter 2000)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "GERSCH2000", "de", QString::fromUtf8("Schlachter 2000"), QString::fromUtf8("German Schlachter 2000"), constUUID_GERSCH2000, "bbl-gersch2000.s3db", "bbl-gersch2000.ccdb", constUUID_GERSCH2000 },
		// KJV1611A (1611 with Apocrypha)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "KJV1611A", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James 1611 w/Apocrypha"), constUUID_KJV1611A, "bbl-kjv1611a.s3db", "bbl-kjv1611a.ccdb", constUUID_KJV1611 },
		// KJV1611 (1611 w/o Apocrypha)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "KJV1611", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James 1611"), constUUID_KJV1611, "bbl-kjv1611.s3db", "bbl-kjv1611.ccdb", constUUID_KJV1611 },
		// RV1865-sbv20140622 (Reina-Valera 1865, from Jeff McArdle, Sociedad Bíblica Valera 2014-06-22 Release)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "RV1865-sbv20140622", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Sociedad Bíblica Valera (20140622)"), constUUID_RV1865sbv20140622, "bbl-rv1865sbv20140622.s3db", "bbl-rv1865sbv20140622.ccdb", constUUID_RV1865sbv20140622 },
		// RVG2010-20140705:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "RVG2010-20140705", "es", QString::fromUtf8("Reina-Valera Gómez 2010"), QString::fromUtf8("Reina-Valera Gómez 2010 Version (20140705)"), constUUID_RVG2010_20140705, "bbl-rvg2010-20140705.s3db", "bbl-rvg2010-20140705.ccdb", constUUID_RVG2010 },
		// KJF2015 (Nadine's Work-In-Progress:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "KJF2015", "fr", QString::fromUtf8("King James Française 2015 Work-In-Progress"), QString::fromUtf8("la Bible King James Française, édition 2015 Work-In-Progress"), constUUID_KJF2015, "bbl-kjf2015-workinprogress.s3db", "bbl-kjf2015-workinprogress.ccdb", constUUID_KJF2015 },
		// RVG2010-20150120:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "RVG2010-20150120", "es", QString::fromUtf8("Reina-Valera Gómez 2010"), QString::fromUtf8("Reina-Valera Gómez 2010 Version (20150120)"), constUUID_RVG2010_20150120, "bbl-rvg2010-20150120.s3db", "bbl-rvg2010-20150120.ccdb", constUUID_RVG2010 },
		// TR-20140413-X1 : OSIS seg variant x-1 of TR-20140413 OSIS (Stephens 1550)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "TR-20140413-X1", "grc", QString::fromUtf8("Textus Receptus Greek New Testament (Stephens 1550)"), QString::fromUtf8("Textus Receptus Greek New Testament (Stephens 1550) (20140413)"), constUUID_TR_20140413_X1, "bbl-tr-20140413-x1.s3db", "bbl-tr-20140413-x1.ccdb", constUUID_TR_20140413_X1 },
		// TR-20140413-X2 : OSIS seg variant x-2 of TR-20140413 OSIS (Scrivener 1894)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "TR-20140413-X2", "grc", QString::fromUtf8("Textus Receptus Greek New Testament (Scrivener 1894)"), QString::fromUtf8("Textus Receptus Greek New Testament (Scrivener 1894) (20140413)"), constUUID_TR_20140413_X2, "bbl-tr-20140413-x2.s3db", "bbl-tr-20140413-x2.ccdb", constUUID_TR_20140413_X2 },
		// SPMT-20120627:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "SPMT-20120627", "he", QString::fromUtf8("Samaritan Pentateuch Masoretic Text"), QString::fromUtf8("Samaritan Pentateuch Masoretic Text (20120627)"), constUUID_SPMT_20120627, "bbl-spmt-20120627.s3db", "bbl-spmt-20120627.ccdb", constUUID_SPMT_20120627 },
		// LXX-20080722:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "LXX-20080722", "grc", QString::fromUtf8("LXX Greek Septuagint Old Testament Version 270 BC"), QString::fromUtf8("LXX Greek Septuagint Old Testament Version 270 BC (20080722)"), constUUID_LXX_20080722, "bbl-lxx-20080722.s3db", "bbl-lxx-20080722.ccdb", constUUID_LXX_20080722 },
		// RV1865-mv20180504: (Reina-Valera 1865, from Vince LaRue, Ministerios Valera 1865, 2018-05-04 Release)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), "RV1865-mv20180504", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Ministerios Valera 1865 (20180504)"), constUUID_RV1865mv20180504, "bbl-rv1865mv20180504.s3db", "bbl-rv1865mv20180504.ccdb", constUUID_RV1865mv },
	};

	//////////////////////////////////////////////////////////////////////

	// Dictionary Database Descriptor Constants:
	// -----------------------------------------
	const char *constUUID_DCT_SPECIAL_TEST =	"00000000-0000-11E3-8224-0800200C9A66";
	const char *constUUID_DCT_WEB1828 =			"6A94E150-1E6C-11E3-8224-0800200C9A66";
	const char *constUUID_DCT_WEB1913 =			"70C95C30-3893-11E4-916C-0800200C9A66";
	const char *constUUID_DCT_WEB1806 =			"8d71d7c0-3d4a-11e4-916c-0800200c9a66";
	const char *constUUID_DCT_USSHER =			"92f1c570-3d4a-11e4-916c-0800200c9a66";
	const char *constUUID_DCT_NAVE =			"160E7960-5FEE-11E4-9803-0800200C9A66";
	const char *constUUID_DCT_THOMPSON =		"356898E0-5FEE-11E4-9803-0800200C9A66";
	const char *constUUID_DCT_TOPICAL =			"48A64970-5FEE-11E4-9803-0800200C9A66";
	const char *constUUID_DCT_TORREY =			"53A8BCE0-5FEE-11E4-9803-0800200C9A66";

	const TDictionaryDescriptor constDictionaryDescriptors[] =
	{
		// Special Test Value:
		{ DictionaryTypeOptionsFlags(DTO_SpecialTest), "en", QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Dictionary Database"), constUUID_DCT_SPECIAL_TEST, "dct-specTest.s3db", "dct-specTest.ccdb" },
		// Webster 1828:
		{ DictionaryTypeOptionsFlags(defaultDictionaryTypeFlags), "en", QString::fromUtf8("Webster 1828"), QString::fromUtf8("Webster's Unabridged Dictionary, 1828"), constUUID_DCT_WEB1828, "dct-web1828.s3db", "dct-web1828.ccdb" },
		// Webster 1913:
		{ DictionaryTypeOptionsFlags(defaultDictionaryTypeFlags), "en", QString::fromUtf8("Webster 1913"), QString::fromUtf8("Webster's Unabridged Dictionary, 1913"), constUUID_DCT_WEB1913, "dct-web1913.s3db", "dct-web1913.ccdb" },
		// Webster 1806:
		{ DictionaryTypeOptionsFlags(defaultDictionaryTypeFlags), "en", QString::fromUtf8("Webster 1806"), QString::fromUtf8("Webster's Compendious Dictionary, 1806"), constUUID_DCT_WEB1806, "dct-web1806.s3db", "dct-web1806.ccdb" },
		// Ussher's Annals of the World, 1654:
		{ DictionaryTypeOptionsFlags(DTO_TimeLineDictionary), "en", QString::fromUtf8("Ussher"), QString::fromUtf8("Ussher's Annals of the World, 1654"), constUUID_DCT_USSHER, "dct-ussher.s3db", "dct-ussher.ccdb" },
		// Nave's Topical Bible Dictionary:
		{ DictionaryTypeOptionsFlags(defaultTopicalDctTypeFlags), "en", QString::fromUtf8("Nave's Topical Bible"), QString::fromUtf8("Nave's Topical Bible Dictionary"), constUUID_DCT_NAVE, "dct-t-nave.s3db", "dct-t-nave.ccdb" },
		// Thompson Chain References:
		{ DictionaryTypeOptionsFlags(defaultTopicalDctTypeFlags), "en", QString::fromUtf8("Thompson Chain References"), QString::fromUtf8("Thompson Chain References"), constUUID_DCT_THOMPSON, "dct-t-thompson.s3db", "dct-t-thompson.ccdb" },
		// Topical Study Dictionary:
		{ DictionaryTypeOptionsFlags(defaultTopicalDctTypeFlags), "en", QString::fromUtf8("Topical Study"), QString::fromUtf8("Topical Study Dictionary"), constUUID_DCT_TOPICAL, "dct-t-topical.s3db", "dct-t-topical.ccdb" },
		// Torrey's New Topical Textbook:
		{ DictionaryTypeOptionsFlags(defaultTopicalDctTypeFlags), "en", QString::fromUtf8("Torrey's New Topical Textbook"), QString::fromUtf8("Torrey's New Topical Textbook"), constUUID_DCT_TORREY, "dct-t-torrey.s3db", "dct-t-torrey.ccdb" }
	};

}	// namespace

// ============================================================================

unsigned int bibleDescriptorCount()
{
	return _countof(constBibleDescriptors);
}

const TBibleDescriptor &bibleDescriptor(BIBLE_DESCRIPTOR_ENUM nIndex)
{
	assert(nIndex != BDE_UNKNOWN);
	assert(static_cast<unsigned int>(nIndex) < _countof(constBibleDescriptors));
	return constBibleDescriptors[nIndex];
}

BIBLE_DESCRIPTOR_ENUM bibleDescriptorFromUUID(const QString &strUUID)
{
	for (unsigned int ndx = 0; ndx < _countof(constBibleDescriptors); ++ndx) {
		if (constBibleDescriptors[ndx].m_strUUID.compare(strUUID, Qt::CaseInsensitive) == 0) return static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx);
	}
	return BDE_UNKNOWN;
}

// ============================================================================

unsigned int dictionaryDescriptorCount()
{
	return _countof(constDictionaryDescriptors);
}

const TDictionaryDescriptor &dictionaryDescriptor(DICTIONARY_DESCRIPTOR_ENUM nIndex)
{
	assert(nIndex != DDE_UNKNOWN);
	assert(static_cast<unsigned int>(nIndex) < _countof(constDictionaryDescriptors));
	return constDictionaryDescriptors[nIndex];
}

DICTIONARY_DESCRIPTOR_ENUM dictionaryDescriptorFromUUID(const QString &strUUID)
{
	for (unsigned int ndx = 0; ndx < _countof(constDictionaryDescriptors); ++ndx) {
		if (constDictionaryDescriptors[ndx].m_strUUID.compare(strUUID, Qt::CaseInsensitive) == 0) return static_cast<DICTIONARY_DESCRIPTOR_ENUM>(ndx);
	}
	return DDE_UNKNOWN;
}

// ============================================================================


QString xc_dbDescriptors::translatedBibleTestamentName(const QString &strUUID, unsigned int nTst)

{
	Q_UNUSED(strUUID);		// Currently UUID isn't used, but if we have different Bibles with different Testament mapping, it will be used for that mapping

#define NUM_TST 3u				// Total Number of Testaments (or pseudo-testaments, in the case of Apocrypha)
	const QString arrstrTstNames[NUM_TST] =
		{	tr("Old Testament", "testament_names"),
			tr("New Testament", "testament_names"),
			tr("Apocrypha/Deuterocanon", "testament_names")
		};

	if ((nTst < 1) || (nTst > NUM_TST)) return QString();
	return arrstrTstNames[nTst-1];
}

// ============================================================================
