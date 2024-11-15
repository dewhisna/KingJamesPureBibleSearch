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

#ifndef ELS_RESULT_H
#define ELS_RESULT_H

#include "../KJVCanOpener/dbstruct.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QAbstractListModel>
#include <QMetaType>

// Forward Declarations
class QMimeData;
class CLetterMatrix;

// ============================================================================

enum ELSRESULT_SORT_ORDER_ENUM {
	ESO_WSR,
	ESO_WRS,
	ESO_RWS,
	ESO_RSW,
	ESO_SRW,
	ESO_SWR,
	// ----
	ESO_COUNT
};
constexpr ELSRESULT_SORT_ORDER_ENUM ESO_FIRST = ESO_WSR;
Q_DECLARE_METATYPE(ELSRESULT_SORT_ORDER_ENUM)

extern QString elsresultSortOrderDescription(ELSRESULT_SORT_ORDER_ENUM nSortOrder);

// ============================================================================

class CELSResult
{
public:
	QString m_strWord;
	int m_nSkip = 0;
	CRelIndexEx m_ndxStart;
	Qt::LayoutDirection m_nDirection = Qt::LeftToRight;
};

typedef QList<CELSResult> CELSResultList;

extern void sortELSResultList(ELSRESULT_SORT_ORDER_ENUM nSortOrder, CELSResultList &lstResults);

// ============================================================================

class CELSResultListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	explicit CELSResultListModel(const CLetterMatrix &letterMatrix, bool bUppercase, QObject *parent = nullptr);
	virtual ~CELSResultListModel();

	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

#ifndef IS_CONSOLE_APP
	Qt::DropActions supportedDragActions() const override;
	QMimeData *mimeData(const QModelIndexList &indexes) const override;
	QStringList mimeTypes() const override;
#endif

	// Editable:
	bool setData(const QModelIndex &index, const QVariant &value,
				 int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	// --------------------------------

	ELSRESULT_SORT_ORDER_ENUM sortOrder() const { return m_nSortOrder; }

	bool uppercase() const { return m_bUppercase; }

public slots:
	void setSortOrder(ELSRESULT_SORT_ORDER_ENUM nSortOrder);
	void setSearchResults(const CELSResultList &lstResults);
	void clearSearchResults();

	void setUppercase(bool bUppercase);

protected:
	void sortResults();

private:
	const CLetterMatrix &m_letterMatrix;
	bool m_bUppercase;
	// ----
	CELSResultList m_lstResults;
	// ----
	ELSRESULT_SORT_ORDER_ENUM m_nSortOrder = ESO_WSR;
};

// ============================================================================

#endif	// ELS_RESULT_H
