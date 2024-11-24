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
#include "FindELS.h"

#ifndef IS_CONSOLE_APP
#include "../KJVCanOpener/MimeHelper.h"
#include "../KJVCanOpener/BusyCursor.h"
#include <QMimeData>
#include <QMap>
#include <utility>			// for std::pair
#endif

#include <QVariant>
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

	return ((m_letterMatrix.size()-1) / m_nWidth) + (((m_letterMatrix.size()-1) % m_nWidth) ? 1 : 0);
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

	uint32_t nMatrixIndex = matrixIndexFromModelIndex(index);
	if (nMatrixIndex >= static_cast<uint32_t>(m_letterMatrix.size())) nMatrixIndex = 0;

	switch (role) {
		case Qt::DisplayRole:
			if (nMatrixIndex) {
				return m_bUppercase ? m_letterMatrix.at(nMatrixIndex).toUpper() : m_letterMatrix.at(nMatrixIndex);
			}
			break;

		case Qt::UserRole:					// Returns the reference
			return QVariant::fromValue(m_letterMatrix.relIndexFromMatrixIndex(nMatrixIndex));

		case Qt::UserRole+1:				// Returns the Matrix Index
			return nMatrixIndex;

		case Qt::UserRole+2:				// Returns the data for MIME export
		{
			QString strValue;
			if (nMatrixIndex) {
				switch (m_lstCharacterFound.at(nMatrixIndex)) {
					case 0:
						strValue += " ";
						break;
					case 1:
						strValue += "[";
						break;
					default:
						strValue += "{";
						break;
				}
				strValue += m_bUppercase ? m_letterMatrix.at(nMatrixIndex).toUpper() : m_letterMatrix.at(nMatrixIndex);
				switch (m_lstCharacterFound.at(nMatrixIndex)) {
					case 0:
						strValue += " ";
						break;
					case 1:
						strValue += "]";
						break;
					default:
						strValue += "}";
						break;
				}
			} else {
				strValue = "   ";
			}
			return strValue;
		}

		case Qt::FontRole:
			return m_fontMatrix;

		case Qt::BackgroundRole:
			if ((nMatrixIndex != 0) && m_lstCharacterFound.at(nMatrixIndex)) {
				return ((m_lstCharacterFound.at(nMatrixIndex) > 1) ? QColor("lightgreen") :  QColor("yellow"));
			}
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

#ifndef IS_CONSOLE_APP
Qt::DropActions CLetterMatrixTableModel::supportedDragActions() const
{
	return Qt::CopyAction;
}

QMimeData *CLetterMatrixTableModel::mimeData(const QModelIndexList &indexes) const
{
	if (indexes.isEmpty()) return nullptr;

	CBusyCursor iAmBusy(nullptr);

	QString strText;
	typedef QMap<int, QString> TColDataMap;			// Map of column index to value to export (for sorting and generation)
	typedef QMap<int, TColDataMap> TRowDataMap;		// Map of the above by rows (for sorting and generation)
	TRowDataMap mapRows;

	int nMinRow = -1;
	int nMaxRow = -1;
	int nMinCol = -1;
	int nMaxCol = -1;

	for (auto const & item : indexes) {
		mapRows[item.row()][item.column()] = item.data(Qt::UserRole+2).toString();
		if ((nMinRow == -1) || (item.row() < nMinRow)) nMinRow = item.row();
		if ((nMaxRow == -1) || (item.row() > nMaxRow)) nMaxRow = item.row();
		if ((nMinCol == -1) || (item.column() < nMinCol)) nMinCol = item.column();
		if ((nMaxCol == -1) || (item.column() > nMaxCol)) nMaxCol = item.column();
	}

	int nCurRow = nMinRow;
	for (TRowDataMap::const_key_value_iterator itrRowData = mapRows.constKeyValueBegin();
		 itrRowData != mapRows.constKeyValueEnd(); ++itrRowData) {
		const std::pair<int, TColDataMap> pairRow = *itrRowData;
		while (pairRow.first > nCurRow) {
			strText += "\n";
			++nCurRow;
		}
		int nCurCol = nMinCol;
		for (TColDataMap::const_key_value_iterator itrColData = pairRow.second.constKeyValueBegin();
			 itrColData != pairRow.second.constKeyValueEnd(); ++itrColData) {
			const std::pair<int, QString> pairCol = *itrColData;
			while (pairCol.first > nCurCol) {
				strText += "   ";
				++nCurCol;
			}
			strText += pairCol.second;
			++nCurCol;
		}
		strText += "\n";
		++nCurRow;
	}

	QMimeData *mime = new QMimeData();
	mime->setData(g_constrPlainTextMimeType, strText.toUtf8());

	if (indexes.size() == 1) {
		TPhraseTag tag(CRelIndexEx(indexes.at(0).data(Qt::UserRole).value<CRelIndexEx>()), 1);
		CMimeHelper::addPhraseTagToMimeData(mime, tag);
	}

	return mime;
}

QStringList CLetterMatrixTableModel::mimeTypes() const
{
	QStringList lstTypes;
	lstTypes << g_constrPlainTextMimeType;
	lstTypes << g_constrPhraseTagMimeType;
	return lstTypes;
}
#endif

Qt::ItemFlags CLetterMatrixTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled;
}

// ----------------------------------------------------------------------------

void CLetterMatrixTableModel::setWidth(int nWidth)
{
	if (nWidth < 1) nWidth = 1;
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
		for (int i = 0; i < result.m_strWord.size(); ++i) {
			if (matrixIndexResult >= static_cast<uint32_t>(m_lstCharacterFound.size())) {
				Q_ASSERT(false);
				break;
			}
			m_lstCharacterFound[matrixIndexResult]++;
			matrixIndexResult += CFindELS::nextOffset(result.m_nSkip, i, result.m_nSearchType);
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

uint32_t CLetterMatrixTableModel::matrixIndexFromModelIndex(const QModelIndex &index) const
{
	if (!index.isValid()) return 0;
	return (index.row() * m_nWidth) + index.column() + 1;
}

uint32_t CLetterMatrixTableModel::matrixIndexFromRowCol(int nRow, int nCol) const
{
	return (nRow * m_nWidth) + nCol + 1;
}

// ============================================================================
