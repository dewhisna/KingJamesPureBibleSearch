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

#include "ELSResult.h"

#include <QObject>
#include <QFuture>
#include <QList>
#include <QStringList>
#include <QtConcurrent>

#include <numeric>			// for std::iota

// Forward Declarations
class CLetterMatrix;

// ============================================================================

class CFindELS
{
public:
	CFindELS(const CLetterMatrix &letterMatrix, const QStringList &lstSearchWords);

	bool setBookEnds(unsigned int nBookStart = 0, unsigned int nBookEnd = 0);
	unsigned int bookStart() const { return m_nBookStart; }
	unsigned int bookEnd() const { return m_nBookEnd; }

	template<typename ReduceFunctor>
	QFuture<CELSResultList> future(int nMinSkip, int nMaxSkip, ReduceFunctor &&fnReduce)		// Multithread runner to compute ELS
	{
		// Build list of skips to search:
#if QT_VERSION < 0x060000
		// NOTE: Unlike Qt6, Qt 5 has no constructor to prepopulate the list:
		m_lstSkips.clear();
		m_lstSkips.reserve(nMaxSkip - nMinSkip + 1);
		for (int nSkip = nMinSkip; nSkip <= nMaxSkip; ++nSkip) m_lstSkips.append(0);
#else
		m_lstSkips = QList<int>(nMaxSkip - nMinSkip + 1);
#endif
		std::iota(m_lstSkips.begin(), m_lstSkips.end(), nMinSkip);

		// NOTE: Qt 5 can't use lambdas for the functors in the mappedReduced() call.
		//	So this dance works around that by having normal functions for it:
		return QtConcurrent::mappedReduced(m_lstSkips,
										   std::bind(&CFindELS::run, this, std::placeholders::_1),
										   fnReduce);
	}

	CELSResultList run(int nSkip) const;

	static void reduce(CELSResultList &lstResults, const CELSResultList &result)		// Helper function if caller doesn't need to override
	{
		lstResults.append(result);
	}

private:
	QList<int> m_lstSkips;
	const CLetterMatrix &m_letterMatrix;
	QStringList m_lstSearchWords;
	QStringList m_lstSearchWordsRev;
	unsigned int m_nBookStart = 0;
	unsigned int m_nBookEnd = 0;
};

// ============================================================================

#endif	// FIND_ELS_H
