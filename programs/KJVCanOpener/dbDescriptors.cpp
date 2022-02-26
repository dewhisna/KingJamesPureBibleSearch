/****************************************************************************
**
** Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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
#include "BibleLayout.h"

#include <QObject>

// ============================================================================

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Bible Database Descriptor Constants:
	// ------------------------------------
	const char * const constUUID_SPECIAL_TEST =			"00000000-0000-11E3-8FFD-0800200C9A66";
	const char * const constUUID_KJV =					"85D8A6B0-E670-11E2-A28F-0800200C9A66";
	const char * const constUUID_RVG2010_20140126 =		"9233CB60-141A-11E3-8FFD-0800200C9A66";
	const char * const constUUID_KJF2006 =				"31FC2ED0-141B-11E3-8FFD-0800200C9A66";
	const char * const constUUID_KJVPCE =				"C9BA8970-A114-11E3-A5E2-0800200C9A66";
	const char * const constUUID_KJVA =					"B93D0E40-BA16-11E3-A5E2-0800200C9A66";
	const char * const constUUID_UKJV =					"BA852FE0-C762-11E3-9C1A-0800200C9A66";
	const char * const constUUID_GERLUT1545 =			"D7376840-C75F-11E3-9C1A-0800200C9A66";
	const char * const constUUID_RV1865lcbp20100713 =	"9378F5B0-CE8F-11E3-9C1A-0800200C9A66";
	const char * const constUUID_RV1602Prrb20110825 =	"94B7B600-CF39-11E3-9C1A-0800200C9A66";
	const char * const constUUID_GERSCH2000 =			"D326F220-CFFC-11E3-9C1A-0800200C9A66";
	const char * const constUUID_KJV1611A =				"8BFE29A0-D014-11E3-9C1A-0800200C9A66";
	const char * const constUUID_KJV1611 =				"8D999FF0-D650-11E3-9C1A-0800200C9A66";
	const char * const constUUID_RV1865sbv20140622 =	"919DAB50-1151-11E4-9191-0800200C9A66";
	const char * const constUUID_RVG2010_20140705 =		"92EF65E0-56F0-11E4-8ED6-0800200C9A66";
	const char * const constUUID_KJF2015 =				"36708290-9BB0-11E4-BD06-0800200C9A66";
	const char * const constUUID_RVG2010_20150120 =		"925744F0-6B1B-11E5-A837-0800200C9A66";
	const char * const constUUID_TR_20140413_X1 =		"4D025330-5BAC-11E7-9598-0800200C9A66";
	const char * const constUUID_TR_20140413_X2 =		"4C0C5630-5C77-11E7-9598-0800200C9A66";
	const char * const constUUID_SPMT_20120627 =		"5939EB40-5BAC-11E7-9598-0800200C9A66";
	const char * const constUUID_LXX_20080722 =			"4F4BD170-6264-11E7-9598-0800200C9A66";
	const char * const constUUID_RV1865mv20180504 =		"28A10630-5728-11E8-B566-0800200C9A66";
	const char * const constUUID_RUSSYNODAL_20101106 =	"B5F0F4C0-28E5-11E9-B56E-0800200C9A66";
	// ----
	const char * const constUUID_ASV_20061025 =			"6DA72D40-A830-11E9-B475-0800200C9A66";
	const char * const constUUID_ISV_20100807 =			"7C305670-A830-11E9-B475-0800200C9A66";
	// ----
	const char * const constUUID_RUSSYNODAL_20201221 =	"B6157FE0-8128-11EC-BF44-0800200C9A66";
	// ----
	const char * const constUUID_MYBIBLE_ESV_2001 =		"7B3F4F70-821A-11EC-BF44-0800200C9A66";
	const char * const constUUID_MYBIBLE_NASB_1971 =	"7B3F4F71-821A-11EC-BF44-0800200C9A66";
	const char * const constUUID_MYBIBLE_NASB_2020 =	"7B3F4F72-821A-11EC-BF44-0800200C9A66";
	const char * const constUUID_MYBIBLE_NIV_1978 =		"7B3F4F73-821A-11EC-BF44-0800200C9A66";
	const char * const constUUID_MYBIBLE_NIV_1984 =		"7B3F4F74-821A-11EC-BF44-0800200C9A66";
	const char * const constUUID_MYBIBLE_NIV_2011 =		"7B3F4F75-821A-11EC-BF44-0800200C9A66";
	const char * const constUUID_MYBIBLE_NKJV_1982 =	"7B3F4F76-821A-11EC-BF44-0800200C9A66";
	const char * const constUUID_MYBIBLE_TNIV_2005 =	"7B3F4F77-821A-11EC-BF44-0800200C9A66";
	// ----
	const char * const constUUID_OSHB =					"565612E0-8A24-11EC-B1E5-0800200C9A66";
	// ----

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

	// Use the RUSSYNODAL_20101106 UUID for the highlighter tag for all RUSSYNODAL text.
	//		They aren't overtly different, just marked up better in their OSIS content.
	//		Assuming we ever complete and adopt RUSSYNODAL_20201221 and newer:
	const char *constUUID_RUSSYNODAL = constUUID_RUSSYNODAL_20101106;

	const TBibleDescriptor constBibleDescriptors[] =
	{
		// Special Test Value:
		{ BibleTypeOptionsFlags(BTO_SpecialTest), Qt::LeftToRight, "SPECIAL", "en", QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Bible Database"), constUUID_SPECIAL_TEST, "bbl-specTest.s3db", "bbl-specTest.ccdb", constUUID_SPECIAL_TEST },
		// KJV standard database (w/o Strongs Lemma/Morph):
		{ BibleTypeOptionsFlags(BTO_AutoLoad), Qt::LeftToRight, "KJV", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769)"), constUUID_KJV, "bbl-kjv1769.s3db", "bbl-kjv1769.ccdb", constUUID_KJV },
		// RVG2010-20140126:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RVG2010-20140126", "es", QString::fromUtf8("Reina-Valera Gómez 2010"), QString::fromUtf8("Reina-Valera Gómez 2010 Version (20140126)"), constUUID_RVG2010_20140126, "bbl-rvg2010-20140126.s3db", "bbl-rvg2010-20140126.ccdb", constUUID_RVG2010 },
		// KJF2006:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "KJF2006", "fr", QString::fromUtf8("King James Française 2006"), QString::fromUtf8("la Bible King James Française, édition 2006"), constUUID_KJF2006, "bbl-kjf2006.s3db", "bbl-kjf2006.ccdb", constUUID_KJF2006 },
		// KJVPureCambridge:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "KJVPCE", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Pure Cambridge Edition"), constUUID_KJVPCE, "bbl-kjvpce.s3db", "bbl-kjvpce.ccdb", constUUID_KJV },
		// KJVA (KJV 1769 with Apocrypha):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "KJVA", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769) w/Apocrypha"), constUUID_KJVA, "bbl-kjva.s3db", "bbl-kjva.ccdb", constUUID_KJV },
		// UKJV (Updated King James Version):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "UKJV", "en", QString::fromUtf8("King James"), QString::fromUtf8("Updated King James Version"), constUUID_UKJV, "bbl-ukjv.s3db", "bbl-ukjv.ccdb", constUUID_UKJV },
		// GERLUT1545 (German Luther 1545):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "GERLUT1545", "de", QString::fromUtf8("1545 Luther Bibelübersetzung"), QString::fromUtf8("German Unrevidierte Luther Übersetzung von 1545"), constUUID_GERLUT1545, "bbl-gerlut1545.s3db", "bbl-gerlut1545.ccdb", constUUID_GERLUT1545 },
		// RV1865-lcbp20100713 (Reina-Valera 1865, Local Church Bible Publishers 2010-07-13 Release)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RV1865-lcbp20100713", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Local Church Bible Publishers (20100713)"), constUUID_RV1865lcbp20100713, "bbl-rv1865lcbp20100713.s3db", "bbl-rv1865lcbp20100713.ccdb", constUUID_RV1865lcbp20100713 },
		// RV1602P (Reina-Valera 1602 Purificada)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RV1602P-rrb20110825", "es", QString::fromUtf8("Reina-Valera 1602 Purificada"), QString::fromUtf8("Reina-Valera 1602 Purificada (20110825)"), constUUID_RV1602Prrb20110825, "bbl-rv1602p.s3db", "bbl-rv1602p.ccdb", constUUID_RV1602Prrb20110825 },
		// GERSCH2000 (German Schlachter 2000)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "GERSCH2000", "de", QString::fromUtf8("Schlachter 2000"), QString::fromUtf8("German Schlachter 2000"), constUUID_GERSCH2000, "bbl-gersch2000.s3db", "bbl-gersch2000.ccdb", constUUID_GERSCH2000 },
		// KJV1611A (1611 with Apocrypha)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "KJV1611A", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James 1611 w/Apocrypha"), constUUID_KJV1611A, "bbl-kjv1611a.s3db", "bbl-kjv1611a.ccdb", constUUID_KJV1611 },
		// KJV1611 (1611 w/o Apocrypha)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "KJV1611", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James 1611"), constUUID_KJV1611, "bbl-kjv1611.s3db", "bbl-kjv1611.ccdb", constUUID_KJV1611 },
		// RV1865-sbv20140622 (Reina-Valera 1865, from Jeff McArdle, Sociedad Bíblica Valera 2014-06-22 Release)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RV1865-sbv20140622", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Sociedad Bíblica Valera (20140622)"), constUUID_RV1865sbv20140622, "bbl-rv1865sbv20140622.s3db", "bbl-rv1865sbv20140622.ccdb", constUUID_RV1865sbv20140622 },
		// RVG2010-20140705:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RVG2010-20140705", "es", QString::fromUtf8("Reina-Valera Gómez 2010"), QString::fromUtf8("Reina-Valera Gómez 2010 Version (20140705)"), constUUID_RVG2010_20140705, "bbl-rvg2010-20140705.s3db", "bbl-rvg2010-20140705.ccdb", constUUID_RVG2010 },
		// KJF2015 (Nadine's Work-In-Progress:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "KJF2015", "fr", QString::fromUtf8("King James Française 2015 Work-In-Progress"), QString::fromUtf8("la Bible King James Française, édition 2015 Work-In-Progress"), constUUID_KJF2015, "bbl-kjf2015-workinprogress.s3db", "bbl-kjf2015-workinprogress.ccdb", constUUID_KJF2015 },
		// RVG2010-20150120:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RVG2010-20150120", "es", QString::fromUtf8("Reina-Valera Gómez 2010"), QString::fromUtf8("Reina-Valera Gómez 2010 Version (20150120)"), constUUID_RVG2010_20150120, "bbl-rvg2010-20150120.s3db", "bbl-rvg2010-20150120.ccdb", constUUID_RVG2010 },
		// TR-20140413-X1 : OSIS seg variant x-1 of TR-20140413 OSIS (Stephens 1550)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "TR-20140413-X1", "grc", QString::fromUtf8("Textus Receptus Greek New Testament (Stephens 1550)"), QString::fromUtf8("Textus Receptus Greek New Testament (Stephens 1550) (20140413)"), constUUID_TR_20140413_X1, "bbl-tr-20140413-x1.s3db", "bbl-tr-20140413-x1.ccdb", constUUID_TR_20140413_X1 },
		// TR-20140413-X2 : OSIS seg variant x-2 of TR-20140413 OSIS (Scrivener 1894)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "TR-20140413-X2", "grc", QString::fromUtf8("Textus Receptus Greek New Testament (Scrivener 1894)"), QString::fromUtf8("Textus Receptus Greek New Testament (Scrivener 1894) (20140413)"), constUUID_TR_20140413_X2, "bbl-tr-20140413-x2.s3db", "bbl-tr-20140413-x2.ccdb", constUUID_TR_20140413_X2 },
		// SPMT-20120627:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::RightToLeft, "SPMT-20120627", "he", QString::fromUtf8("Samaritan Pentateuch Masoretic Text"), QString::fromUtf8("Samaritan Pentateuch Masoretic Text (20120627)"), constUUID_SPMT_20120627, "bbl-spmt-20120627.s3db", "bbl-spmt-20120627.ccdb", constUUID_SPMT_20120627 },
		// LXX-20080722:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "LXX-20080722", "grc", QString::fromUtf8("LXX Greek Septuagint Old Testament Version 270 BC"), QString::fromUtf8("LXX Greek Septuagint Old Testament Version 270 BC (20080722)"), constUUID_LXX_20080722, "bbl-lxx-20080722.s3db", "bbl-lxx-20080722.ccdb", constUUID_LXX_20080722 },
		// RV1865-mv20180504: (Reina-Valera 1865, from Vince LaRue, Ministerios Valera 1865, 2018-05-04 Release)
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RV1865-mv20180504", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Ministerios Valera 1865 (20180504)"), constUUID_RV1865mv20180504, "bbl-rv1865mv20180504.s3db", "bbl-rv1865mv20180504.ccdb", constUUID_RV1865mv },
		// RUSSYNODAL_20101106 (1876 Russian Synodal Bible):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RUSSYNODAL_20101106", "ru", QString::fromUtf8("1876 Russian Synodal Bible"), QString::fromUtf8("1876 Russian Synodal Bible (20101106a)"),  constUUID_RUSSYNODAL_20101106, "bbl-russynodal20101106.s3db", "bbl-russynodal20101106.ccdb", constUUID_RUSSYNODAL },
		// ----
		// American Standard Version (1901):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "ASV_1901_20061025", "en", QString::fromUtf8("American Standard Version (1901)"), QString::fromUtf8("American Standard Version (1901) (20061025)"), constUUID_ASV_20061025, "bbl-asv1901-20061025.s3db", "bbl-asv1901-20061025.ccdb", constUUID_ASV_20061025 },
		// International Standard Version (2001):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "ISV_2001_20100807", "en", QString::fromUtf8("International Standard Version (2001)"), QString::fromUtf8("International Standard Version (2001) (20100807)"), constUUID_ISV_20100807, "bbl-isv2001-20100807.s3db", "bbl-isv2001-20100807.ccdb", constUUID_ISV_20100807 },
		// ----
		// KJV Full database (with Strongs Lemma/Morph):
		{ BibleTypeOptionsFlags(BTO_AutoLoad | BTO_Preferred | BTO_HasStrongs), Qt::LeftToRight, "KJV", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769) w/Strongs"), constUUID_KJV, "bbl-kjv1769-full.s3db", "bbl-kjv1769-full.ccdb", constUUID_KJV },
		// RUSSYNODAL_20201221 (1876 Russian Synodal Bible):
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "RUSSYNODAL_20201221", "ru", QString::fromUtf8("1876 Russian Synodal Bible"), QString::fromUtf8("1876 Russian Synodal Bible (20201221)"),  constUUID_RUSSYNODAL_20201221, "bbl-russynodal20201221.s3db", "bbl-russynodal20201221.ccdb", constUUID_RUSSYNODAL },
		// ----
		// English Standard Version 2001:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_ESV_2001", "en", QString::fromUtf8("MyBible English Standard Version (2001)"), QString::fromUtf8("English Standard Version (2001)"), constUUID_MYBIBLE_ESV_2001, "bbl-mybible-esv-2001.s3db", "bbl-mybible-esv-2001.ccdb", constUUID_MYBIBLE_ESV_2001 },
		// New American Standard Bible 1971:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_NASB_1971", "en", QString::fromUtf8("MyBible New American Standard Bible (1971)"), QString::fromUtf8("New American Standard Bible (1971)"), constUUID_MYBIBLE_NASB_1971, "bbl-mybible-nasb-1971.s3db", "bbl-mybible-nasb-1971.ccdb", constUUID_MYBIBLE_NASB_1971 },
		// New American Standard Bible 2020:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_NASB_2020", "en", QString::fromUtf8("MyBible New American Standard Bible (2020)"), QString::fromUtf8("New American Standard Bible (2020)"), constUUID_MYBIBLE_NASB_2020, "bbl-mybible-nasb-2020.s3db", "bbl-mybible-nasb-2020.ccdb", constUUID_MYBIBLE_NASB_2020 },
		// New International Version, 1978, Red letter edition:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_NIV_1978", "en", QString::fromUtf8("MyBible New International Version (1978)"), QString::fromUtf8("New International Version, 1978, Red letter edition"), constUUID_MYBIBLE_NIV_1978, "bbl-mybible-niv-1978.s3db", "bbl-mybible-niv-1978.ccdb", constUUID_MYBIBLE_NIV_1978 },
		// New International Version, 1984:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_NIV_1984", "en", QString::fromUtf8("MyBible New International Version (1984)"), QString::fromUtf8("New International Version (1984)"), constUUID_MYBIBLE_NIV_1984, "bbl-mybible-niv-1984.s3db", "bbl-mybible-niv-1984.ccdb", constUUID_MYBIBLE_NIV_1984 },
		// New International Version, 2011:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_NIV_2011", "en", QString::fromUtf8("MyBible New International Version (2011)"), QString::fromUtf8("New International Version (2011)"), constUUID_MYBIBLE_NIV_2011, "bbl-mybible-niv-2011.s3db", "bbl-mybible-niv-2011.ccdb", constUUID_MYBIBLE_NIV_2011 },
		// New King James Version, 1982:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_NKJV_1982", "en", QString::fromUtf8("MyBible New King James Version (1982)"), QString::fromUtf8("New King James Version (1982)"), constUUID_MYBIBLE_NKJV_1982, "bbl-mybible-nkjv-1982.s3db", "bbl-mybible-nkjv-1982.ccdb", constUUID_MYBIBLE_NKJV_1982 },
		// Today's New International Version, 2005:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::LeftToRight, "MYBIBLE_TNIV_2005", "en", QString::fromUtf8("MyBible Today's New International Version (2005)"), QString::fromUtf8("Today's New International Version (2005)"), constUUID_MYBIBLE_TNIV_2005, "bbl-mybible-tniv-2005.s3db", "bbl-mybible-tniv-2005.ccdb", constUUID_MYBIBLE_TNIV_2005 },
		// ----
		// Open Scriptures Hebrew Bible:
		{ BibleTypeOptionsFlags(defaultBibleTypeFlags), Qt::RightToLeft, "OSHB", "hbo" /* "he" */ /* "iw" */, QString::fromUtf8("Open Scriptures Hebrew"), QString::fromUtf8("Open Scriptures Hebrew Bible"), constUUID_OSHB, "bbl-oshb.s3db", "bbl-oshb.ccdb", constUUID_OSHB },
		// ----
	};

	//////////////////////////////////////////////////////////////////////

	// Dictionary Database Descriptor Constants:
	// -----------------------------------------
	const char * const constUUID_DCT_SPECIAL_TEST =	"00000000-0000-11E3-8224-0800200C9A66";
	const char * const constUUID_DCT_WEB1828 =		"6A94E150-1E6C-11E3-8224-0800200C9A66";
	const char * const constUUID_DCT_WEB1913 =		"70C95C30-3893-11E4-916C-0800200C9A66";
	const char * const constUUID_DCT_WEB1806 =		"8d71d7c0-3d4a-11e4-916c-0800200c9a66";
	const char * const constUUID_DCT_USSHER =		"92f1c570-3d4a-11e4-916c-0800200c9a66";
	const char * const constUUID_DCT_NAVE =			"160E7960-5FEE-11E4-9803-0800200C9A66";
	const char * const constUUID_DCT_THOMPSON =		"356898E0-5FEE-11E4-9803-0800200C9A66";
	const char * const constUUID_DCT_TOPICAL =		"48A64970-5FEE-11E4-9803-0800200C9A66";
	const char * const constUUID_DCT_TORREY =		"53A8BCE0-5FEE-11E4-9803-0800200C9A66";
	const char * const constUUID_DCT_STRONGS =		constUUID_KJV;		// Uses same UUID since the Strongs Dictionary is embedded in the extended KJV database

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
		{ DictionaryTypeOptionsFlags(defaultTopicalDctTypeFlags), "en", QString::fromUtf8("Torrey's New Topical Textbook"), QString::fromUtf8("Torrey's New Topical Textbook"), constUUID_DCT_TORREY, "dct-t-torrey.s3db", "dct-t-torrey.ccdb" },
		// KJV Strongs (note: no files as it's embedded in the KJV Full Bible Database):
		{ DictionaryTypeOptionsFlags(defaultStrongsDctTypeFlags | DTO_Preferred), "en", QString::fromUtf8("Strong's"), QString::fromUtf8("Strong's Dictionary of Bible Words"), constUUID_DCT_STRONGS, "", "" },
	};

}	// namespace

// ============================================================================

LANGUAGE_ID_ENUM toLanguageID(const QString &strLang)
{
	if ((strLang.compare("en", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("eng", Qt::CaseInsensitive) == 0)) return LIDE_ENGLISH;
	if ((strLang.compare("fr", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("fre", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("fra", Qt::CaseInsensitive) == 0)) return LIDE_FRENCH;
	if ((strLang.compare("es", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("spa", Qt::CaseInsensitive) == 0)) return LIDE_SPANISH;
	if ((strLang.compare("de", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("deu", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("ger", Qt::CaseInsensitive) == 0)) return LIDE_GERMAN;
	if ((strLang.compare("ru", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("rus", Qt::CaseInsensitive) == 0)) return LIDE_RUSSIAN;
	if ((strLang.compare("he", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("heb", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("hbo", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("iw", Qt::CaseInsensitive) == 0)) return LIDE_HEBREW;
	if ((strLang.compare("el", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("ell", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("grc", Qt::CaseInsensitive) == 0) ||
		(strLang.compare("gre", Qt::CaseInsensitive) == 0)) return LIDE_GREEK;

	return LIDE_UNKNOWN;
}

QString toQtLanguageName(LANGUAGE_ID_ENUM nID)
{
	switch (nID) {
		case LIDE_ENGLISH:
			return "en";
		case LIDE_FRENCH:
			return "fr";
		case LIDE_SPANISH:
			return "es";
		case LIDE_GERMAN:
			return "de";
		case LIDE_RUSSIAN:
			return "ru";
		case LIDE_HEBREW:
			return "he";		// Modern Hebrew
		case LIDE_GREEK:
			return "el";		// Modern Greek
		case LIDE_UNKNOWN:
		default:
			return "  ";		// Any/C-Locale
	}
}

// ============================================================================

unsigned int bibleDescriptorCount()
{
	return _countof(constBibleDescriptors);
}

const TBibleDescriptor &bibleDescriptor(BIBLE_DESCRIPTOR_ENUM nIndex)
{
	Q_ASSERT(nIndex != BDE_UNKNOWN);
	Q_ASSERT(static_cast<unsigned int>(nIndex) < _countof(constBibleDescriptors));
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
	Q_ASSERT(nIndex != DDE_UNKNOWN);
	Q_ASSERT(static_cast<unsigned int>(nIndex) < _countof(constDictionaryDescriptors));
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
