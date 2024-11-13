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
#include <QFont>

#include "ELSResult.h"

// Forward Declarations
class CLetterMatrix;

// ============================================================================

class CLetterMatrixTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit CLetterMatrixTableModel(const CLetterMatrix &letterMatrix,
									 int nWidth,
									 QObject *parent = nullptr);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	const CLetterMatrix &matrix() const { return m_letterMatrix; }

	int width() const { return m_nWidth; }

	QModelIndex modelIndexFromMatrixIndex(uint32_t nMatrixIndex);

public slots:
	void setWidth(int nWidth);

	void setSearchResults(const CELSResultList &results);
	void clearSearchResults();

private:
	const CLetterMatrix &m_letterMatrix;
	int m_nWidth = 10;					// Width of Matrix for displaying
	// ----
	QList<int> m_lstCharacterFound;		// Number of times a character is found in search result data (used for highlighting)
	// ----
	QFont m_fontMatrix;
};

// ============================================================================

#endif // LETTER_MATRIX_TABLE_MODEL_H
