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
	const char *constUUID_SPECIAL_TEST =	"00000000-0000-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJV =				"85D8A6B0-E670-11E2-A28F-0800200C9A66";
	const char *constUUID_RVG2010 =			"9233CB60-141A-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJF2006 =			"31FC2ED0-141B-11E3-8FFD-0800200C9A66";
	const char *constUUID_KJVPCE =			"C9BA8970-A114-11E3-A5E2-0800200C9A66";
	const char *constUUID_KJVA =			"B93D0E40-BA16-11E3-A5E2-0800200C9A66";

	const char *constKJVCompatUUID[] = {
		constUUID_KJV,
		constUUID_KJVPCE,
		constUUID_KJVA,
		NULL
	};

	class TStringListGen : public QStringList
	{
	public:
		TStringListGen(const char **pList = NULL)
		{
			if (pList) {
				int i = 0;
				while (pList[i]) {
					append(pList[i]);
					++i;
				}
			}
		}
	};

	const TBibleDescriptor constBibleDescriptors[] =
	{
		// Special Test Value:
		{ false, "en", QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Bible Database"), constUUID_SPECIAL_TEST, "", "bbl-specTest.s3db", "bbl-specTest.ccdb", QStringList() },
		// KJV:
		{ true, "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769)"), constUUID_KJV, "", "kjvtext.s3db", "kjvtext.ccdb", TStringListGen(constKJVCompatUUID) },
		// RVG2010:
		{ false, "es", QString::fromUtf8("Reina-Valera Gómez"), QString::fromUtf8("Reina-Valera Gómez Version (2010)"), constUUID_RVG2010, "", "bbl-rvg2010.s3db", "bbl-rvg2010.ccdb", QStringList() },
		// KJF2006:
		{ false, "fr", QString::fromUtf8("King James Française 2006"), QString::fromUtf8("la Bible King James Française, édition 2006"), constUUID_KJF2006, "", "bbl-kjf2006.s3db", "bbl-kjf2006.ccdb", QStringList() },
		// KJVPureCambridge:
		{ false, "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Pure Cambridge Edition"), constUUID_KJVPCE, "", "bbl-kjvpce.s3db", "bbl-kjvpce.ccdb", TStringListGen(constKJVCompatUUID) },
		// KJVA (KJV 1769 with Apocrypha):
		{ false, "en", QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769) w/Apocrypha"), constUUID_KJVA, "", "bbl-kjva.s3db", "bbl-kjva.ccdb", TStringListGen(constKJVCompatUUID) }
	};

	//////////////////////////////////////////////////////////////////////

	// Dictionary Database Descriptor Constants:
	// -----------------------------------------
	const TDictionaryDescriptor constDictionaryDescriptors[] =
	{
		// Special Test Value:
		{ false, "en", QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Dictionary Database"), "00000000-0000-11E3-8224-0800200C9A66", "", "dct-specTest.s3db", "dct-specTest.ccdb" },
		// Webster 1828:
		{ true, "en", QString::fromUtf8("Webster 1828"), QString::fromUtf8("Webster's Unabridged Dictionary, 1828"), "6A94E150-1E6C-11E3-8224-0800200C9A66", "", "dct-web1828.s3db", "dct-web1828.ccdb" }
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

