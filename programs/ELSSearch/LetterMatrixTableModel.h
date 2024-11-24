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
									 bool bUppercase,
									 QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

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
	bool uppercase() const { return m_bUppercase; }

	QModelIndex modelIndexFromMatrixIndex(uint32_t nMatrixIndex);
	uint32_t matrixIndexFromModelIndex(const QModelIndex &index) const;
	uint32_t matrixIndexFromRowCol(int nRow, int nCol) const;

	const QFontMetrics &fontMetrics() const { return m_fontMatrixMetrics; }

public slots:
	void setWidth(int nWidth);
	void setUppercase(bool bUppercase);

	void setSearchResults(const CELSResultList &results);
	void clearSearchResults();

private:
	const CLetterMatrix &m_letterMatrix;
	int m_nWidth = 10;					// Width of Matrix for displaying
	bool m_bUppercase = false;
	// ----
	QList<int> m_lstCharacterFound;		// Number of times a character is found in search result data (used for highlighting)
	// ----
	QFont m_fontMatrix;
	QFontMetrics m_fontMatrixMetrics;
};

// ============================================================================

#endif // LETTER_MATRIX_TABLE_MODEL_H
