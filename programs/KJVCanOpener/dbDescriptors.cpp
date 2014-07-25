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
	const char *constUUID_RVG2010 =				"9233CB60-141A-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJF2006 =				"31FC2ED0-141B-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJVPCE =				"C9BA8970-A114-11E3-A5E2-0800200C9A66";
	const char *constUUID_KJVA =				"B93D0E40-BA16-11E3-A5E2-0800200C9A66";
	const char *constUUID_UKJV =				"BA852FE0-C762-11E3-9C1A-0800200C9A66";
	const char *constUUID_GERLUT1545 =			"D7376840-C75F-11E3-9C1A-0800200C9A66";
	const char *constUUID_RV1865lcbp20100713 =	"9378F5B0-CE8F-11E3-9C1A-0800200C9A66";
	const char *constUUID_RV1865sbv20140622 =	"919DAB50-1151-11E4-9191-0800200C9A66";
	const char *constUUID_RV1602Prrb20110825 =	"94B7B600-CF39-11E3-9C1A-0800200C9A66";
	const char *constUUID_GERSCH2000 =			"D326F220-CFFC-11E3-9C1A-0800200C9A66";
	const char *constUUID_KJV1611A =			"8BFE29A0-D014-11E3-9C1A-0800200C9A66";
	const char *constUUID_KJV1611 =				"8D999FF0-D650-11E3-9C1A-0800200C9A66";

	const TBibleDescriptor constBibleDescriptors[] =
	{
		// Special Test Value:
		{ false, "SPECIAL", "en", QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Bible Database"), constUUID_SPECIAL_TEST, "bbl-specTest.s3db", "bbl-specTest.ccdb", constUUID_SPECIAL_TEST },
		// KJV:
		{ true, "KJV", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769)"), constUUID_KJV, "bbl-kjv1769.s3db", "bbl-kjv1769.ccdb", constUUID_KJV },
		// RVG2010:
		{ false, "RVG2010", "es", QString::fromUtf8("Reina-Valera Gómez 2010"), QString::fromUtf8("Reina-Valera Gómez 2010 Version (20140126)"), constUUID_RVG2010, "bbl-rvg2010.s3db", "bbl-rvg2010.ccdb", constUUID_RVG2010 },
		// KJF2006:
		{ false, "KJF2006", "fr", QString::fromUtf8("King James Française 2006"), QString::fromUtf8("la Bible King James Française, édition 2006"), constUUID_KJF2006, "bbl-kjf2006.s3db", "bbl-kjf2006.ccdb", constUUID_KJF2006 },
		// KJVPureCambridge:
		{ false, "KJVPCE", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Pure Cambridge Edition"), constUUID_KJVPCE, "bbl-kjvpce.s3db", "bbl-kjvpce.ccdb", constUUID_KJV },
		// KJVA (KJV 1769 with Apocrypha):
		{ false, "KJVA", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769) w/Apocrypha"), constUUID_KJVA, "bbl-kjva.s3db", "bbl-kjva.ccdb", constUUID_KJV },
		// UKJV (Updated King James Version):
		{ false, "UKJV", "en", QString::fromUtf8("King James"), QString::fromUtf8("Updated King James Version"), constUUID_UKJV, "bbl-ukjv.s3db", "bbl-ukjv.ccdb", constUUID_UKJV },
		// GERLUT1545 (German Luther 1545):
		{ false, "GERLUT1545", "de", QString::fromUtf8("1545 Luther Bibelübersetzung"), QString::fromUtf8("German Unrevidierte Luther Übersetzung von 1545"), constUUID_GERLUT1545, "bbl-gerlut1545.s3db", "bbl-gerlut1545.ccdb", constUUID_GERLUT1545 },
		// RV1865-lcbp20100713 (Reina-Valera 1865, Local Church Bible Publishers 2010-07-13 Release)
		{ false, "RV1865-lcbp20100713", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Local Church Bible Publishers (20100713)"), constUUID_RV1865lcbp20100713, "bbl-rv1865lcbp20100713.s3db", "bbl-rv1865lcbp20100713.ccdb", constUUID_RV1865lcbp20100713 },
		// RV1602P (Reina-Valera 1602 Purificada)
		{ false, "RV1602P-rrb20110825", "es", QString::fromUtf8("Reina-Valera 1602 Purificada"), QString::fromUtf8("Reina-Valera 1602 Purificada (20110825)"), constUUID_RV1602Prrb20110825, "bbl-rv1602p.s3db", "bbl-rv1602p.ccdb", constUUID_RV1602Prrb20110825 },
		// GERSCH2000 (German Schlachter 2000)
		{ false, "GERSCH2000", "de", QString::fromUtf8("Schlachter 2000"), QString::fromUtf8("German Schlachter 2000"), constUUID_GERSCH2000, "bbl-gersch2000.s3db", "bbl-gersch2000.ccdb", constUUID_GERSCH2000 },
		// KJV1611A (1611 with Apocrypha)
		{ false, "KJV1611A", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James 1611 w/Apocrypha"), constUUID_KJV1611A, "bbl-kjv1611a.s3db", "bbl-kjv1611a.ccdb", constUUID_KJV1611 },
		// KJV1611 (1611 w/o Apocrypha)
		{ false, "KJV1611", "en", QString::fromUtf8("King James"), QString::fromUtf8("King James 1611"), constUUID_KJV1611, "bbl-kjv1611.s3db", "bbl-kjv1611.ccdb", constUUID_KJV1611 },
		// RV1865-sbv20140622 (Reina-Valera 1865, from Jeff McArdle, Sociedad Bíblica Valera 2014-06-22 Release)
		{ false, "RV1865-sbv20140622", "es", QString::fromUtf8("Reina-Valera 1865"), QString::fromUtf8("Reina-Valera 1865, Sociedad Bíblica Valera (20140622)"), constUUID_RV1865sbv20140622, "bbl-rv1865sbv20140622.s3db", "bbl-rv1865sbv20140622.ccdb", constUUID_RV1865sbv20140622 }
	};

	//////////////////////////////////////////////////////////////////////

	// Dictionary Database Descriptor Constants:
	// -----------------------------------------
	const TDictionaryDescriptor constDictionaryDescriptors[] =
	{
		// Special Test Value:
		{ false, "en", QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Dictionary Database"), "00000000-0000-11E3-8224-0800200C9A66", "dct-specTest.s3db", "dct-specTest.ccdb" },
		// Webster 1828:
		{ true, "en", QString::fromUtf8("Webster 1828"), QString::fromUtf8("Webster's Unabridged Dictionary, 1828"), "6A94E150-1E6C-11E3-8224-0800200C9A66", "dct-web1828.s3db", "dct-web1828.ccdb" }
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
