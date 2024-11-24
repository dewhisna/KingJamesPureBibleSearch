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
	CFindELS(const CLetterMatrix &letterMatrix, const QStringList &lstSearchWords, ELS_SEARCH_TYPE_ENUM nSearchType);

	bool setBookEnds(unsigned int nBookStart = 0, unsigned int nBookEnd = 0);
	unsigned int bookStart() const { return m_nBookStart; }
	unsigned int bookEnd() const { return m_nBookEnd; }

	template<typename ReduceFunctor>
	QFuture<CELSResultList> future(int nMinSkip, int nMaxSkip, ReduceFunctor &&fnReduce)		// Multithread runner to compute ELS
	{
		// Build list of skips to search:
		m_lstSkips.clear();
		switch (m_nSearchType) {
			case ESTE_ELS:
			case ESTE_FLS:
#if QT_VERSION < 0x060000
				// NOTE: Unlike Qt6, Qt 5 has no constructor to prepopulate the list:
				m_lstSkips.reserve(nMaxSkip - nMinSkip + 1);
				for (int nSkip = nMinSkip; nSkip <= nMaxSkip; ++nSkip) m_lstSkips.append(0);
#else
				m_lstSkips = QList<int>(nMaxSkip - nMinSkip + 1);
#endif
				std::iota(m_lstSkips.begin(), m_lstSkips.end(), nMinSkip);
				break;

			case ESTE_FLS_C9_ALL:
				for (int i = 1; i <= 8; ++i) {
					m_lstSkips.append(i);
				}
				break;

			case ESTE_FLS_C9_124875:
				m_lstSkips.append(1);
				m_lstSkips.append(2);
				m_lstSkips.append(4);
				m_lstSkips.append(8);
				m_lstSkips.append(7);
				m_lstSkips.append(5);
				break;

			case ESTE_FLS_C9_147:
				m_lstSkips.append(1);
				m_lstSkips.append(4);
				m_lstSkips.append(7);
				break;

			case ESTE_FLS_C9_852:
				m_lstSkips.append(8);
				m_lstSkips.append(5);
				m_lstSkips.append(2);
				break;

			case ESTE_FLS_C9_18:
				m_lstSkips.append(1);
				m_lstSkips.append(8);
				break;

			case ESTE_FLS_C9_45:
				m_lstSkips.append(4);
				m_lstSkips.append(5);
				break;

			case ESTE_FLS_C9_72:
				m_lstSkips.append(7);
				m_lstSkips.append(2);
				break;

			case ESTE_FLS_C9_36:
				m_lstSkips.append(3);
				m_lstSkips.append(6);
				break;

			case ESTE_FLS_C9_1:
				m_lstSkips.append(1);
				break;

			case ESTE_FLS_C9_2:
				m_lstSkips.append(2);
				break;

			case ESTE_FLS_C9_3:
				m_lstSkips.append(3);
				break;

			case ESTE_FLS_C9_4:
				m_lstSkips.append(4);
				break;

			case ESTE_FLS_C9_5:
				m_lstSkips.append(5);
				break;

			case ESTE_FLS_C9_6:
				m_lstSkips.append(6);
				break;

			case ESTE_FLS_C9_7:
				m_lstSkips.append(7);
				break;

			case ESTE_FLS_C9_8:
				m_lstSkips.append(8);
				break;

			default:
				Q_ASSERT(false);
		}

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

	static int maxDistance(int nSkip, int nLen, ELS_SEARCH_TYPE_ENUM nSearchType);			// Compute max distance for a given word length for a given search type
	static int nextOffset(int nSkip, int nLetterPos, ELS_SEARCH_TYPE_ENUM nSearchType);		// Compute next skip for a given word letter index for a given search type

private:
	static void initFibonacciCast9Table();

	// Concurrent Threading function to locate the ELS entries for a single skip distance:
	static CELSResultList findELS(int nSkip, const CLetterMatrix &letterMatrix,
								  const QStringList &lstSearchWords, const QStringList &lstSearchWordsRev,
								  unsigned int nBookStart, unsigned int nBookEnd, ELS_SEARCH_TYPE_ENUM nSearchType);

private:
	QList<int> m_lstSkips;
	const CLetterMatrix &m_letterMatrix;
	QStringList m_lstSearchWords;
	QStringList m_lstSearchWordsRev;
	unsigned int m_nBookStart = 0;
	unsigned int m_nBookEnd = 0;
	ELS_SEARCH_TYPE_ENUM m_nSearchType = ESTE_ELS;
	// ----
	static int g_conFibonacciCast9[8][24];
};

// ============================================================================

#endif	// FIND_ELS_H
