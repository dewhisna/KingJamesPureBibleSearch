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
#include <QMap>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QAbstractListModel>
#include <QMetaType>
#include <QModelIndexList>

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
extern ELSRESULT_SORT_ORDER_ENUM elsresultSortOrderFromLetters(const QString &strLetters);		// Returns ESO_COUNT if invalid
extern QString elsresultSortOrderToLetters(ELSRESULT_SORT_ORDER_ENUM nSortOrder);

// ============================================================================

// List of Apophenia search types:
enum ELS_SEARCH_TYPE_ENUM {
	ESTE_ELS = 0,				// Normal ELS Search
	ESTE_FLS = 1,				// Fibonacci FLS Search
	ESTE_FLS_C9_ALL = 2,		// FLS Search with Casting out 9's, ALL
	ESTE_FLS_C9_124875 = 3,		// FLS Search with Casting out 9's, 1-2-4-8-7-5
	ESTE_FLS_C9_147 = 4,		// FLS Search with Casting out 9's, 1-4-7
	ESTE_FLS_C9_852 = 5,		// FLS Search with Casting out 9's, 8-5-2
	ESTE_FLS_C9_18 = 6,			// FLS Search with Casting out 9's, 1-8
	ESTE_FLS_C9_45 = 7,			// FLS Search with Casting out 9's, 4-5
	ESTE_FLS_C9_72 = 8,			// FLS Search with Casting out 9's, 7-2
	ESTE_FLS_C9_36 = 9,			// FLS Search with Casting out 9's, 3-6
	ESTE_FLS_C9_1 = 10,			// FLS Search with Casting out 9's, 1-Only
	ESTE_FLS_C9_2 = 11,			// FLS Search with Casting out 9's, 2-Only
	ESTE_FLS_C9_3 = 12,			// FLS Search with Casting out 9's, 3-Only
	ESTE_FLS_C9_4 = 13,			// FLS Search with Casting out 9's, 4-Only
	ESTE_FLS_C9_5 = 14,			// FLS Search with Casting out 9's, 5-Only
	ESTE_FLS_C9_6 = 15,			// FLS Search with Casting out 9's, 6-Only
	ESTE_FLS_C9_7 = 16,			// FLS Search with Casting out 9's, 7-Only
	ESTE_FLS_C9_8 = 17,			// FLS Search with Casting out 9's, 8-Only
	// ----
	ESTE_COUNT
};
constexpr ELS_SEARCH_TYPE_ENUM ESTE_FIRST = ESTE_ELS;
Q_DECLARE_METATYPE(ELS_SEARCH_TYPE_ENUM)

extern QString elsSearchTypeDescription(ELS_SEARCH_TYPE_ENUM nSearchType);
extern ELS_SEARCH_TYPE_ENUM elsSearchTypeFromID(const QString &strID);		// Returns ESTE_COUNT if invalid
extern QString elsSearchTypeToID(ELS_SEARCH_TYPE_ENUM nSearchType);

// ============================================================================

class CELSResult
{
public:
	QString m_strWord;
	int m_nSkip = 0;
	ELS_SEARCH_TYPE_ENUM m_nSearchType = ESTE_ELS;			// Skip algorithm -- needed here to know how to traverse the data for a specific result
	CRelIndexEx m_ndxStart;									// Index of first letter (in matrix order, i.e. will be last letter if found in RightToLeft direction)
	CRelIndexEx m_ndxEnd;									// Index of last letter (in matrix order, i.e. will be last letter if found in LeftToRight direction)
	CRelIndexEx m_ndxNominal;								// Nominal index of word -- dependent on search type weighting
	Qt::LayoutDirection m_nDirection = Qt::LeftToRight;

	// operator<() needed for QMap:
	inline bool operator<(const CELSResult &result) const	// This '<' operator is for QMap containing functionality only, and not for actually sorting CELSResult values!
	{
		if (m_strWord < result.m_strWord) {
			return true;
		} else if (m_strWord == result.m_strWord) {
			if (m_nSkip < result.m_nSkip) {
				return true;
			} else if (m_nSkip == result.m_nSkip) {
				if (m_nSearchType < result.m_nSearchType) {
					return true;
				} else if (m_nSearchType == result.m_nSearchType) {
					if (m_ndxStart < result.m_ndxStart) {
						return true;
					} else if (m_ndxStart == result.m_ndxStart) {
						if (m_ndxNominal < result.m_ndxNominal) {
							return true;
						} else if (m_ndxNominal == result.m_ndxNominal) {
							if (m_ndxEnd < result.m_ndxEnd) {
								return true;
							} else if (m_ndxEnd == result.m_ndxEnd) {
								if (m_nDirection < result.m_nDirection) {
									return true;
								}
							}
						}
					}
				}
			}
		}

		return false;
	}

//	// operator==() and !=() needed for QList::contains()
//	inline bool operator==(const CELSResult &result) const
//	{
//		return ((m_strWord == result.m_strWord) &&
//				(m_nSkip == result.m_nSkip) &&
//				(m_nSearchType == result.m_nSearchType) &&
//				(m_ndxStart == result.m_ndxStart) &&
//				(m_ndxNominal == result.m_ndxNominal) &&
//				(m_ndxEnd == result.m_ndxEnd) &&
//				(m_nDirection == result.m_nDirection));
//	}
//	inline bool operator!=(const CELSResult &result) const
//	{
//		return !operator==(result);
//	}
};

typedef QList<CELSResult> CELSResultList;
typedef QMap<CELSResult, bool> CELSResultSet;		// "Set" implemented as a QMap -- value not used, but QSet requires a qHash function
Q_DECLARE_METATYPE(CELSResultList)
Q_DECLARE_METATYPE(CELSResultSet)

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

	static constexpr int UserRole_Reference = Qt::UserRole;
	static constexpr int UserRole_MIMEData = Qt::UserRole+1;
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

	QModelIndexList getResultIndexes(const CELSResultSet &setResults);

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
	CELSResultSet m_mapResults;			// Map of results to have a quick way to check for duplicates -- QSet would be better, but it requires a qHash function
	// ----
	ELSRESULT_SORT_ORDER_ENUM m_nSortOrder = ESO_RWS;
};

// ============================================================================

#endif	// ELS_RESULT_H
