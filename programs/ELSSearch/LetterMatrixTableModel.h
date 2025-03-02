/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef LETTER_MATRIX_TABLE_MODEL_H
#define LETTER_MATRIX_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QString>
#include <QStringList>
#include <QFont>
#include <QFontMetrics>

#include "ELSResult.h"

// Forward Declarations
class QMimeData;
class CLetterMatrix;

// ============================================================================

class CLetterMatrixTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit CLetterMatrixTableModel(const CLetterMatrix &letterMatrix,
									 int nWidth,
									 int nOffset,
									 LETTER_CASE_ENUM nLetterCase,
									 QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	static constexpr int UserRole_Reference = Qt::UserRole;
	static constexpr int UserRole_MatrixIndex = Qt::UserRole+1;
	static constexpr int UserRole_MIMEPlainText = Qt::UserRole+2;
	static constexpr int UserRole_MIMEHTMLText = Qt::UserRole+3;
	static constexpr int UserRole_ResultsSet = Qt::UserRole+4;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

#ifndef IS_CONSOLE_APP
	Qt::DropActions supportedDragActions() const override;
	QMimeData *mimeData(const QModelIndexList &indexes) const override;
	QStringList mimeTypes() const override;
#endif

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	// --------------------------------

	const CLetterMatrix &matrix() const { return m_letterMatrix; }

	int width() const { return m_nWidth; }
	int offset() const { return m_nOffset; }
	LETTER_CASE_ENUM letterCase() const { return m_nLetterCase; }

	QModelIndex modelIndexFromMatrixIndex(uint32_t nMatrixIndex);
	uint32_t matrixIndexFromModelIndex(const QModelIndex &index) const;
	uint32_t matrixIndexFromRowCol(int nRow, int nCol) const;
	const CELSResultSet &resultsSet(uint32_t nMatrixIndex) const;
	const CELSResultSet &resultsSet(const QModelIndex &index) const;

	const QFontMetrics &fontMetrics() const { return m_fontMatrixMetrics; }

public slots:
	void setWidth(int nWidth);
	void setOffset(int nOffset);
	void setLetterCase(LETTER_CASE_ENUM nLetterCase);

	void setSearchResults(const CELSResultList &results);
	void clearSearchResults();

signals:
	void widthChanged(int nWidth);
	void offsetChanged(int nOffset);

private:
	const CLetterMatrix &m_letterMatrix;
	int m_nWidth = 10;					// Width of Matrix for displaying
	int m_nOffset = 0;					// Offset of the Matrix starting position (value is 0 to Width-1)
	LETTER_CASE_ENUM m_nLetterCase = LCE_LOWER;		// Case to use for letter rendering
	// ----
	QList<CELSResultSet> m_lstCharacterResultMap;		// Map of words intersecting this location, whose size is the number of different words a character is found in search result data (used for highlighting and results selection)
	// ----
	QFont m_fontMatrix;
	QFontMetrics m_fontMatrixMetrics;
};

// ============================================================================

#endif // LETTER_MATRIX_TABLE_MODEL_H
