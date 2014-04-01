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
	const TBibleDescriptor constBibleDescriptors[] =
	{
		// Special Test Value:
		{ false, QString::fromUtf8("Special Test"), QString::fromUtf8("Special Test Bible Database"), "00000000-0000-11E3-8FFD-0800200C9A66", "", "bbl-specTest.s3db", "bbl-specTest.ccdb" },
		// KJV:
		{ true, QString::fromUtf8("King James"), QString::fromUtf8("King James Bible (1769)"), "85D8A6B0-E670-11E2-A28F-0800200C9A66", "", "kjvtext.s3db", "kjvtext.ccdb" },
		// RVG2010:
		{ false, QString::fromUtf8("Reina-Valera Gómez"), QString::fromUtf8("Reina-Valera Gómez Version (2010)"), "9233CB60-141A-11E3-8FFD-0800200C9A66", "", "bbl-rvg2010.s3db", "bbl-rvg2010.ccdb" },
		// KJF2006:
		{ false, QString::fromUtf8("King James Française 2006"), QString::fromUtf8("la Bible King James Française, édition 2006"), "31FC2ED0-141B-11E3-8FFD-0800200C9A66", "", "bbl-kjf2006.s3db", "bbl-kjf2006.ccdb" },
		// KJVPureCambridge:
		{ false, QString::fromUtf8("King James"), QString::fromUtf8("King James Pure Cambridge Edition"), "C9BA8970-A114-11E3-A5E2-0800200C9A66", "", "bbl-kjvpce.s3db", "bbl-kjvpce.ccdb" }
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

