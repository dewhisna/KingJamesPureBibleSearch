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
#include <QTextDocument>	// Needed for outputting HTML MIME data
#include <QMap>
#include <utility>			// for std::pair
#include <QColor>

#ifdef USING_ELSSEARCH
#include "../KJVCanOpener/myApplication.h"
#include <QPalette>
#elif QT_VERSION >= 0x060500
#include <QStyleHints>
#include <QApplication>
#endif

#endif	// !IS_CONSOLE_APP

#include <QVariant>
#include <QSize>

// ============================================================================

CLetterMatrixTableModel::CLetterMatrixTableModel(const CLetterMatrix &letterMatrix,
												 int nWidth, int nOffset, bool bUppercase,
												 QObject *parent)
	:	QAbstractTableModel(parent),
		m_letterMatrix(letterMatrix),
		m_nWidth(nWidth),
		m_nOffset(nOffset),
		m_bUppercase(bUppercase),
		m_fontMatrix("Courier", 14),
		m_fontMatrixMetrics(m_fontMatrix)
{
#if QT_VERSION < 0x060000
	// NOTE: Unlike Qt6, Qt 5 has no constructor to prepopulate the list:
	m_lstCharacterFoundWords.clear();
	m_lstCharacterFoundWords.reserve(m_letterMatrix.size());
	for (auto const &c : m_letterMatrix) {
		Q_UNUSED(c);
		m_lstCharacterFoundWords.append(QSet<QString>());
	}
#else
	m_lstCharacterFoundWords = QList< QSet<QString> >(m_letterMatrix.size());
#endif
}

int CLetterMatrixTableModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return ((m_letterMatrix.size()+m_nOffset-1) / m_nWidth) + (((m_letterMatrix.size()+m_nOffset-1) % m_nWidth) ? 1 : 0);
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

		case Qt::UserRole+2:				// Returns the plain text data for MIME export
		{
			QString strValue;
			if (nMatrixIndex) {
				switch (m_lstCharacterFoundWords.at(nMatrixIndex).size()) {
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
				switch (m_lstCharacterFoundWords.at(nMatrixIndex).size()) {
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

		case Qt::UserRole+3:				// Returns them HTML data for MIME export
		{
			QString strValue;
			if (nMatrixIndex) {
				if (!m_lstCharacterFoundWords.at(nMatrixIndex).isEmpty()) {
					strValue += QString("<td style=\"background-color:%1\">").arg((m_lstCharacterFoundWords.at(nMatrixIndex).size() > 1) ? "lightgreen" : "yellow");
				} else {
					strValue += "<td>";
				}
				strValue += m_bUppercase ? m_letterMatrix.at(nMatrixIndex).toUpper() : m_letterMatrix.at(nMatrixIndex);
				strValue += "</td>";
			} else {
				strValue = "<td>&nbsp;</td>";
			}
			return strValue;
		}

		case Qt::FontRole:
			return m_fontMatrix;

		case Qt::ForegroundRole:
#ifndef IS_CONSOLE_APP
#ifdef USING_ELSSEARCH
			if (g_pMyApplication->isDarkMode()) {
				if ((nMatrixIndex != 0) && !m_lstCharacterFoundWords.at(nMatrixIndex).isEmpty()) {
					return g_pMyApplication->palette("QTableView").base();
				}
			}
#elif QT_VERSION >= 0x060500
			if (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
				if ((nMatrixIndex != 0) && !m_lstCharacterFoundWords.at(nMatrixIndex).isEmpty()) {
					return QApplication::palette("QTableView").base();
				}
			}
#endif
#endif
			break;

		case Qt::BackgroundRole:
#ifndef IS_CONSOLE_APP
			if ((nMatrixIndex != 0) && !m_lstCharacterFoundWords.at(nMatrixIndex).isEmpty()) {
				return ((m_lstCharacterFoundWords.at(nMatrixIndex).size() > 1) ? QColor("lightgreen") :  QColor("yellow"));
			}
#endif
			break;

		case Qt::SizeHintRole:
			return QSize(m_fontMatrixMetrics.maxWidth()+2, m_fontMatrixMetrics.height()+2);

		case Qt::TextAlignmentRole:
			return Qt::AlignCenter;

		case Qt::ToolTipRole:
		case Qt::StatusTipRole:
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

	QString strHTMLText;							// HTML text generation
	QString strPlainText;							// Plain text generation
	struct TCellData {
		QString m_strPlainText;
		QString m_strHTMLText;
	};
	typedef QMap<int, TCellData> TColDataMap;		// Map of column index to value to export (for sorting and generation)
	typedef QMap<int, TColDataMap> TRowDataMap;		// Map of the above by rows (for sorting and generation)
	TRowDataMap mapRows;

	int nMinRow = -1;
	int nMaxRow = -1;
	int nMinCol = -1;
	int nMaxCol = -1;

	for (auto const & item : indexes) {
		mapRows[item.row()][item.column()] = { item.data(Qt::UserRole+2).toString(), item.data(Qt::UserRole+3).toString() };
		if ((nMinRow == -1) || (item.row() < nMinRow)) nMinRow = item.row();
		if ((nMaxRow == -1) || (item.row() > nMaxRow)) nMaxRow = item.row();
		if ((nMinCol == -1) || (item.column() < nMinCol)) nMinCol = item.column();
		if ((nMaxCol == -1) || (item.column() > nMaxCol)) nMaxCol = item.column();
	}

	strHTMLText += QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
						   "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
						   "<style type=\"text/css\">\n"
						   "table { font-family:'Courier'; font-size:14pt; }\n"
						   "</style></head><body>\n");

	strHTMLText += "<table>\n";
	int nCurRow = nMinRow;
	for (TRowDataMap::const_key_value_iterator itrRowData = mapRows.constKeyValueBegin();
		 itrRowData != mapRows.constKeyValueEnd(); ++itrRowData) {
		strHTMLText += "<tr>";
		const std::pair<int, TColDataMap> pairRow = *itrRowData;
		while (pairRow.first > nCurRow) {
			strPlainText += "\n";
			strHTMLText += "</tr>\n<tr>";
			++nCurRow;
		}
		int nCurCol = nMinCol;
		for (TColDataMap::const_key_value_iterator itrColData = pairRow.second.constKeyValueBegin();
			 itrColData != pairRow.second.constKeyValueEnd(); ++itrColData) {
			const std::pair<int, TCellData> pairCol = *itrColData;
			while (pairCol.first > nCurCol) {
				strPlainText += "   ";
				strHTMLText += "<td>&nbsp;</td>";
				++nCurCol;
			}
			strPlainText += pairCol.second.m_strPlainText;
			strHTMLText += pairCol.second.m_strHTMLText;
			++nCurCol;
		}
		strPlainText += "\n";
		strHTMLText += "</tr>\n";
		++nCurRow;
	}
	strHTMLText += "</table>\n";
	strHTMLText += "</body></html>";

	QTextDocument docHTMLText;						// HTML text generation to final HTML via QTextDocument
	docHTMLText.setHtml(strHTMLText);

	QMimeData *mime = new QMimeData();
	mime->setData(g_constrPlainTextMimeType, strPlainText.toUtf8());
	mime->setData(g_constrHTMLTextMimeType, docHTMLText.toHtml().toUtf8());

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
	lstTypes << g_constrHTMLTextMimeType;
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
		emit widthChanged(nWidth);
	}
}

void CLetterMatrixTableModel::setOffset(int nOffset)
{
	if (nOffset < 0) nOffset = 0;
	if (nOffset >= m_nWidth) nOffset = m_nWidth-1;
	if (m_nOffset != nOffset) {
		emit layoutAboutToBeChanged();
		m_nOffset = nOffset;
		emit layoutChanged();
		emit offsetChanged(nOffset);
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
			if (matrixIndexResult >= static_cast<uint32_t>(m_lstCharacterFoundWords.size())) {
				Q_ASSERT(false);
				break;
			}
			m_lstCharacterFoundWords[matrixIndexResult].insert(result.m_strWord);
			matrixIndexResult += CFindELS::nextOffset(result.m_nSkip, i, result.m_nSearchType);
		}
	}

	emit dataChanged(createIndex(0, 0), createIndex(rowCount()-1, columnCount()-1), { Qt::BackgroundRole });
}

void CLetterMatrixTableModel::clearSearchResults()
{
	for (auto & wordSet : m_lstCharacterFoundWords) wordSet.clear();

	emit dataChanged(createIndex(0, 0), createIndex(rowCount()-1, columnCount()-1), { Qt::BackgroundRole });
}

QModelIndex CLetterMatrixTableModel::modelIndexFromMatrixIndex(uint32_t nMatrixIndex)
{
	if (nMatrixIndex == 0) return QModelIndex();
	return createIndex((nMatrixIndex+m_nOffset-1)/m_nWidth, (nMatrixIndex+m_nOffset-1)%m_nWidth);
}

uint32_t CLetterMatrixTableModel::matrixIndexFromModelIndex(const QModelIndex &index) const
{
	if (!index.isValid()) return 0;
	return (index.row() * m_nWidth) + index.column() + 1 - m_nOffset;
}

uint32_t CLetterMatrixTableModel::matrixIndexFromRowCol(int nRow, int nCol) const
{
	return (nRow * m_nWidth) + nCol + 1 - m_nOffset;
}

// ============================================================================
