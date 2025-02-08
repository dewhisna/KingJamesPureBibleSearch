/****************************************************************************
**
** Copyright (C) 2023-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef READ_DATABASE_EX_H
#define READ_DATABASE_EX_H

#include "ReadDB.h"

#include <QObject>

// ============================================================================

class CReadDatabaseEx : public CReadDatabase
{
public:
	enum DB_OVERRIDE_ENUM {
		DBOE_None = 0,
		DBOE_ChapterFirstWordCapital = 1,		// UPPERCASE first word of each chapter
		DBOE_ChapterFirstWordCapitalExt = 2,		// UPPERCASE first word of each chapter and second word too if first word is only a single character
		// ----
		DBOE_COUNT
	};

	static QString dboeDescription(DB_OVERRIDE_ENUM nOverride);

	// ------------------------------------------------------------------------

	CReadDatabaseEx(DB_OVERRIDE_ENUM nDBOE)
		:	m_nDBOE(nDBOE)
	{ }
	virtual bool ReadBibleDatabase(const TBibleDescriptor &bblDesc, bool bSetAsMain = false) override;

protected:
	DB_OVERRIDE_ENUM m_nDBOE = DBOE_None;
};

// ============================================================================

#endif	// READ_DATABASE_EX_H

