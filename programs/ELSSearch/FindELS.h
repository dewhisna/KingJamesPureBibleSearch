/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef FIND_ELS_H
#define FIND_ELS_H

#include "../KJVCanOpener/dbstruct.h"

#include <QList>
#include <QString>
#include <QStringList>

// Forward Declarations
class CLetterMatrix;

// ============================================================================

class CELSResult {
public:
	QString m_strWord;
	int m_nSkip = 0;
	CRelIndexEx m_ndxStart;
	Qt::LayoutDirection m_nDirection = Qt::LeftToRight;
};

typedef QList<CELSResult> CELSResultList;

// ============================================================================

extern CELSResultList findELS(int nSkip, const CLetterMatrix &letterMatrix,
					   const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev,
					   unsigned int nBookStart, unsigned int nBookEnd);



// ============================================================================

#endif	// FIND_ELS_H
