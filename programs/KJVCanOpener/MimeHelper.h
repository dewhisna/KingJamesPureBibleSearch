/****************************************************************************
**
** Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef MIME_HELPER_H
#define MIME_HELPER_H

#include "dbstruct.h"

#include <QMimeData>

class CMimeHelper
{
public:
	CMimeHelper();

	static void addPhraseTagToClipboard(const TPhraseTag &tag);
	static void addPhraseTagToMimeData(QMimeData *pMime, const TPhraseTag &tag);
	static TPhraseTag getPhraseTagFromMimeData(const QMimeData *pMime);
};

#endif // MIME_HELPER_H
