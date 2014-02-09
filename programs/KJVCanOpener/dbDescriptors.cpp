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

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Bible Database Descriptor Constants:
	// ------------------------------------
	const TBibleDescriptor constBibleDescriptors[] =
	{
		// Special Test Value:
		{ QString::fromUtf8("Special Test"), QObject::tr("Special Test Bible Database"), "00000000-0000-11E3-8FFD-0800200C9A66", "", "bbl-specTest.s3db", "bbl-specTest.ccdb" },
		// KJV:
		{ QString::fromUtf8("King James"), QObject::tr("King James Version (1769)"), "85D8A6B0-E670-11E2-A28F-0800200C9A66", "", "kjvtext.s3db", "kjvtext.ccdb" },
		// RVG2010:
		{ QString::fromUtf8("Reina-Valera Gómez"), QObject::tr("Reina-Valera Gómez Version (2010)"), "9233CB60-141A-11E3-8FFD-0800200C9A66", "", "bbl-rvg2010.s3db", "bbl-rvg2010.ccdb" },
		// KJF2006:
		{ QString::fromUtf8("King James Française 2006"), QObject::tr("la Bible King James Française, édition 2006"), "31FC2ED0-141B-11E3-8FFD-0800200C9A66", "", "bbl-kjf2006.s3db", "bbl-kjf2006.ccdb" }
	};

	//////////////////////////////////////////////////////////////////////

	// Dictionary Database Descriptor Constants:
	// -----------------------------------------
	const TDictionaryDescriptor constDictionaryDescriptors[] =
	{
		// Special Test Value:
		{ QString::fromUtf8("Special Test"), QObject::tr("Special Test Dictionary Database"), "00000000-0000-11E3-8224-0800200C9A66", "", "dct-specTest.s3db", "dct-specTest.ccdb" },
		// Webster 1828:
		{ QString::fromUtf8("Webster 1828"), QObject::tr("Webster's Unabridged Dictionary, 1828"), "6A94E150-1E6C-11E3-8224-0800200C9A66", "", "dct-web1828.s3db", "dct-web1828.ccdb" }
	};

}	// namespace

// ============================================================================

unsigned int bibleDescriptorCount()
{
	return _countof(constBibleDescriptors);
}

const TBibleDescriptor &bibleDescriptor(BIBLE_DESCRIPTOR_ENUM nIndex)
{
	assert(static_cast<unsigned int>(nIndex) < _countof(constBibleDescriptors));
	return constBibleDescriptors[nIndex];
}

unsigned int dictionaryDescriptorCount()
{
	return _countof(constDictionaryDescriptors);
}

const TDictionaryDescriptor &dictionaryDescriptor(DICTIONARY_DESCRIPTOR_ENUM nIndex)
{
	assert(static_cast<unsigned int>(nIndex) < _countof(constDictionaryDescriptors));
	return constDictionaryDescriptors[nIndex];
}

// ============================================================================

