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

#include "LetterMatrixTableModel.h"

#include "LetterMatrix.h"
#include "ELSResult.h"

#include <QColor>
#include <QSize>

// ============================================================================

CLetterMatrixTableModel::CLetterMatrixTableModel(const CLetterMatrix &letterMatrix,
												 int nWidth, bool bUppercase,
												 QObject *parent)
	:	QAbstractTableModel(parent),
		m_letterMatrix(letterMatrix),
		m_nWidth(nWidth),
		m_bUppercase(bUppercase),
		m_fontMatrix("Courier", 14),
		m_fontMatrixMetrics(m_fontMatrix)
{
#if QT_VERSION < 0x060000
	// NOTE: Unlike Qt6, Qt 5 has no constructor to prepopulate the list:
	m_lstCharacterFound.clear();
	m_lstCharacterFound.reserve(m_letterMatrix.size());
	for (auto const &c : m_letterMatrix) {
		Q_UNUSED(c);
		m_lstCharacterFound.append(0);
	}
#else
	m_lstCharacterFound = QList<int>(m_letterMatrix.size());
#endif
}

int CLetterMatrixTableModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_letterMatrix.size() / m_nWidth;
}

int CLetterMatrixTableModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_nWidth;
}

QVariant CLetterMatrixTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	uint32_t nMatrixIndex = (index.row() * m_nWidth) + index.column() + 1;
	Q_ASSERT(nMatrixIndex < static_cast<uint32_t>(m_letterMatrix.size()));

	switch (role) {
		case Qt::DisplayRole:
			return m_bUppercase ? m_letterMatrix.at(nMatrixIndex).toUpper() : m_letterMatrix.at(nMatrixIndex);

		case Qt::UserRole:					// Returns the Matrix Index for the data cell
			return nMatrixIndex;

		case Qt::FontRole:
			return m_fontMatrix;

		case Qt::BackgroundRole:
			if (m_lstCharacterFound.at(nMatrixIndex))
				return ((m_lstCharacterFound.at(nMatrixIndex) > 1) ? QColor("lightgreen") :  QColor("yellow"));
			break;

		case Qt::SizeHintRole:
			return QSize(m_fontMatrixMetrics.maxWidth()+2, m_fontMatrixMetrics.height()+2);

		case Qt::TextAlignmentRole:
			return Qt::AlignCenter;

		case Qt::ToolTipRole:
			if (nMatrixIndex) return m_letterMatrix.bibleDatabase()->PassageReferenceText(
										m_letterMatrix.relIndexFromMatrixIndex(nMatrixIndex), false);
			break;
	}

	return QVariant();
}

// ----------------------------------------------------------------------------

void CLetterMatrixTableModel::setWidth(int nWidth)
{
	if (m_nWidth != nWidth) {
		emit layoutAboutToBeChanged();
		m_nWidth = nWidth;
		emit layoutChanged();
	}
}

void CLetterMatrixTableModel::setUppercase(bool bUppercase)
{
	if (m_bUppercase != bUppercase) {
		m_bUppercase = bUppercase;
		emit dataChanged(createIndex(0, 0), createIndex(rowCount()-1, columnCount()-1), { Qt::DisplayRole });
	}
}

// ----------------------------------------------------------------------------

void CLetterMatrixTableModel::setSearchResults(const CELSResultList &results)
{
	for (auto const & result : results) {
		CRelIndexEx ndxResult = result.m_ndxStart;
		uint32_t matrixIndexResult = m_letterMatrix.matrixIndexFromRelIndex(ndxResult);
		if (matrixIndexResult == 0) continue;
		for (auto const & c : result.m_strWord) {
			Q_UNUSED(c);
			m_lstCharacterFound[matrixIndexResult]++;
			matrixIndexResult += result.m_nSkip+1;				// TODO : Figure out how to do non-uniform skips for Fibonacci, etc
			if (matrixIndexResult >= static_cast<uint32_t>(m_lstCharacterFound.size())) {
				Q_ASSERT(false);
				break;
			}
		}
	}

	emit dataChanged(createIndex(0, 0), createIndex(rowCount()-1, columnCount()-1), { Qt::BackgroundRole });
}

void CLetterMatrixTableModel::clearSearchResults()
{
	for (auto & count : m_lstCharacterFound) count = 0;

	emit dataChanged(createIndex(0, 0), createIndex(rowCount()-1, columnCount()-1), { Qt::BackgroundRole });
}

QModelIndex CLetterMatrixTableModel::modelIndexFromMatrixIndex(uint32_t nMatrixIndex)
{
	if (nMatrixIndex == 0) return QModelIndex();
	return createIndex((nMatrixIndex-1)/m_nWidth, (nMatrixIndex-1)%m_nWidth);
}

// ============================================================================
