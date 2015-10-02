/****************************************************************************
**
** Copyright (C) 2015 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef WEBCHANNEL_BIBLE_AUDIO_H
#define WEBCHANNEL_BIBLE_AUDIO_H

#include "dbstruct.h"

#include <QString>
#include <QFlags>

// ============================================================================

class CWebChannelBibleAudio
{
public:
	// Bit-Flag list of sources to include
	enum BibleAudioSources {
		BAS_NONE = 0,
		BAS_FCBH_NONDRAMA = 1,				// Faith Comes By Hearing Non-Drama Version
		BAS_FCBH_DRAMA = 2,					// Faith Comes By Hearing Drama Version
		BAS_ALL = 0xFFFFFFFF
	};
	Q_DECLARE_FLAGS(BibleAudioSourcesFlags, BibleAudioSources)

protected:
	CWebChannelBibleAudio();

public:
	virtual ~CWebChannelBibleAudio();
	static CWebChannelBibleAudio *instance();

	virtual QString urlsForChapterAudio(const CBibleDatabasePtr pBibleDatabase, const CRelIndex &ndxRel, BibleAudioSourcesFlags flagsBAS = BibleAudioSourcesFlags(BAS_ALL));
};

// ============================================================================

#endif	// WEBCHANNEL_BIBLE_AUDIO_H
