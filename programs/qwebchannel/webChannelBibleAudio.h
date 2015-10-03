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

/****************************************************************************

  -------------------------------------
  Audio Bible Texts Supported by KJPBS:
  -------------------------------------

  Faith Comes By Hearing : https://www.faithcomesbyhearing.com/
	2421 Aztec Rd NE Albuquerque, NM 87107
	800.545.6552 | 505.881.3321
	info@faithcomesbyhearing.com
	http://www.bible.is/terms
	Free

  Alexander Scourby : http://scourby.com/
	info@litchfieldltd.com
	US Phone Contact 813-997-2470
	UK Phone Contact +44-28 9568-0022
	http://scourby.com/terms_page.php?q=2
	NOT Free

  Dan Wagner : http://www.mp3bible.ca/
	This audio Bible is produced by Audio Scriptures International.
	Below you will find the transcript of their copyright audio tag:
	"Thank you for listening to this recording of the King James Bible
	produced by Audio Scriptures International. Audio Scriptures
	International believes it is more important to spread the Word of
	God than to make a large profit. Please feel free to make copies of
	these recordings and distribute them freely so long as you adhere
	to the following guidelines:

	1. You may not charge any more than the cost to duplicate and send them.
	2. You must make a complete copy of the recording, including this message.

	For commercial and fund raising distribution, please contact Audio
	Scriptures International for additional information."
	Listen to copyright notice : http://www.mp3bible.ca/copyright/Audio_Scriptures_International_Copyright_Info.mp3

  Stephen Johnston : http://www.audiotreasure.com/indexKJV.htm
	The King James Version Bible
	(c)2002 Firefighters for Christ / Narrated by Stephen Johnston/ Used by permission

  Christopher Glyn : http://www.audiobiblekjv.com/
	RadioActive Productions
	Telephone: U.S.: 281-941-5678
	Address: P.O. Box 126, La Porte, Texas 77572
	Free

  Jon Sherberg and James Earl Jones : http://www.amazon.com/gp/product/1600775845
	Publisher: Topics Entertainment (October 20, 2009)
	Language: English
	ISBN-10: 1600775845
	ISBN-13: 978-1600775840
	NOT Free

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
		BAS_FCBH_NONDRAMA = 0x1,			// Faith Comes By Hearing Non-Drama Version
		BAS_FCBH_DRAMA = 0x2,				// Faith Comes By Hearing Drama Version
		BAS_SCOURBY_NONDRAMA = 0x4,			// Alexandander Scourby Non-Drama Version
		BAS_SCOURBY_DRAMA = 0x8,			// Alexandander Scourby Drama Version
		BAS_DAN_WAGNER = 0x10,				// Dan Wagner (Retired California Minister)
		BAS_STEPHEN_JOHNSTON = 0x20,		// Stephen Johnston Version
		BAS_CHRISTOPHER_GLYN = 0x40,		// Christopher Glyn Version
		BAS_SHERBERG_JONES = 0x80,			// Jon Sherberg and James Earl Jones Version
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
