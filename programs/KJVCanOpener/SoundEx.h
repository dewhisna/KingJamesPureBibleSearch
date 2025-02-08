/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SOUND_EX_H
#define SOUND_EX_H

#include <QString>
#include "dbDescriptors.h"

namespace SoundEx {

	enum SOUNDEX_OPTION_MODE_ENUM {
		SEOME_CLASSIC = 0,			// Classic SoundEx
		SEOME_ENHANCED = 1,			// Enhanced SoundEx, but Not Census Codes
		SEOME_CENSUS_NORMAL = 2,	// Normal Census Codes (Used in all censuses including 1920)
		SEOME_CENSUS_SPECIAL = 3	// Special Census Codes (Used intermittently in 1880, *, 1900, 1910) *1890 destroyed in a fire
	};


	enum SOUNDEX_LANGUAGES_ENUM {
		SELE_UNKNOWN = 0,
		SELE_ENGLISH = 1,
		SELE_FRENCH = 2,
		SELE_SPANISH = 3,
		SELE_GERMAN = 4
	};

	SOUNDEX_LANGUAGES_ENUM languageValue(const QString &strQtLanguage);	// Qt language Name
	SOUNDEX_LANGUAGES_ENUM languageValue(LANGUAGE_ID_ENUM nLangID);		// Bible/Dictionary Database Language ID
	// soundEx : Note: strWordIn should already be decomposed:
	QString soundEx(const QString &strWordIn, SOUNDEX_LANGUAGES_ENUM nLanguage = SELE_ENGLISH, int nLength = 4, SOUNDEX_OPTION_MODE_ENUM nOption = SEOME_ENHANCED);

};

#endif	// SOUND_EX_H


