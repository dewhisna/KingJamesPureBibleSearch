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

#include "ELSResult.h"

#ifndef IS_CONSOLE_APP
#include "../KJVCanOpener/MimeHelper.h"
#include "../KJVCanOpener/BusyCursor.h"
#include <QMimeData>
#endif

#include <QVariant>

// ============================================================================

CELSResultListModel::CELSResultListModel(CBibleDatabasePtr pBibleDatabase, bool bUppercase, QObject *parent)
	:	QAbstractListModel(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_bUppercase(bUppercase)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());
}

CELSResultListModel::~CELSResultListModel()
{
}

QVariant CELSResultListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
		switch (section) {
			case 0:
				return QString("Word");
			case 1:
				return QString("Skip");
			case 2:
				return QString("Dir");
			case 3:
				return QString("Reference");
		}
	}

	return QVariant();
}

bool CELSResultListModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	Q_UNUSED(section);
	Q_UNUSED(orientation);
	Q_UNUSED(value);
	Q_UNUSED(role);
	return false;
}

int CELSResultListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstResults.size();
}

int CELSResultListModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;
	return 4;
}

QVariant CELSResultListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	if (role == Qt::DisplayRole) {
		const CELSResult & result = m_lstResults.at(index.row());

		switch (index.column()) {
			case 0:
				return m_bUppercase ? result.m_strWord.toUpper() : result.m_strWord;
			case 1:
				return result.m_nSkip;
			case 2:
				return QString(result.m_nDirection == Qt::LeftToRight ? "Fwd" : "Rev");
			case 3:
				return m_pBibleDatabase->PassageReferenceText(result.m_ndxStart, false);
		}
	} else if (role == Qt::UserRole) {		// Returns the reference
		const CELSResult & result = m_lstResults.at(index.row());
		return QVariant::fromValue(result.m_ndxStart);
	} else if (role == Qt::UserRole+1) {	// Mime Data for Drag
		const CELSResult & result = m_lstResults.at(index.row());
		QString strMimeData;
		strMimeData += QString("Word: \"%1\"\n").arg(m_bUppercase ? result.m_strWord.toUpper() : result.m_strWord);
		strMimeData += QString("Start Location: %1\n").arg(m_pBibleDatabase->PassageReferenceText(result.m_ndxStart, false));
		strMimeData += QString("Skip: %1\n").arg(result.m_nSkip);
		strMimeData == QString("Direction: %1\n").arg((result.m_nDirection == Qt::LeftToRight) ? "Forward" : "Reverse");
		return strMimeData;
	}

	return QVariant();
}

#ifndef IS_CONSOLE_APP
Qt::DropActions CELSResultListModel::supportedDragActions() const
{
	return Qt::CopyAction;
}

QMimeData *CELSResultListModel::mimeData(const QModelIndexList &indexes) const
{
	if (indexes.isEmpty()) return nullptr;

	CBusyCursor iAmBusy(nullptr);

	QString strText;
	for (auto const & item : indexes) {
		strText += "----------------------------------------\n";
		strText += item.data(Qt::UserRole+1).toString();
	}
	strText += "----------------------------------------\n";

	QMimeData *mime = new QMimeData();
	mime->setData(g_constrPlainTextMimeType, strText.toUtf8());

	if (indexes.size() == 1) {
		TPhraseTag tag(CRelIndexEx(indexes.at(0).data(Qt::UserRole).value<CRelIndexEx>()));
		CMimeHelper::addPhraseTagToMimeData(mime, tag);
	}

	return mime;
}

QStringList CELSResultListModel::mimeTypes() const
{
	QStringList lstTypes;
	lstTypes << g_constrPlainTextMimeType;
	lstTypes << g_constrPhraseTagMimeType;
	return lstTypes;
}
#endif

bool CELSResultListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);
	return false;
}

Qt::ItemFlags CELSResultListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
}

// ----------------------------------------------------------------------------

QString elsresultSortOrderDescription(ELSRESULT_SORT_ORDER_ENUM nSortOrder)
{
	switch (nSortOrder) {
		case ESO_WSR:
			return "Word, Skip, Ref";
		case ESO_WRS:
			return "Word, Ref, Skip";
		case ESO_RWS:
			return "Ref, Word, Skip";
		case ESO_RSW:
			return "Ref, Skip, Word";
		case ESO_SRW:
			return "Skip, Ref, Word";
		case ESO_SWR:
			return "Skip, Word, Ref";
		default:
			break;
	}
	return QString();
}

void CELSResultListModel::setSortOrder(ELSRESULT_SORT_ORDER_ENUM nSortOrder)
{
	m_nSortOrder = nSortOrder;
	beginResetModel();
	sortResults();
	endResetModel();
}

void CELSResultListModel::setSearchResults(const CELSResultList &lstResults)
{
	beginResetModel();
	m_lstResults.append(lstResults);
	sortResults();
	endResetModel();
}

void CELSResultListModel::clearSearchResults()
{
	beginResetModel();
	m_lstResults.clear();
	endResetModel();
}

void CELSResultListModel::setUppercase(bool bUppercase)
{
	if (m_bUppercase != bUppercase) {
		m_bUppercase = bUppercase;
		emit dataChanged(createIndex(0, 0), createIndex(m_lstResults.size()-1, columnCount()-1), { Qt::DisplayRole });
	}
}

void CELSResultListModel::sortResults()
{
	sortELSResultList(m_nSortOrder, m_lstResults);
}

void sortELSResultList(ELSRESULT_SORT_ORDER_ENUM nSortOrder, CELSResultList &lstResults)
{
	std::sort(lstResults.begin(), lstResults.end(),
			  [nSortOrder](const CELSResult &r1, const CELSResult &r2)->bool {
				  auto fnWord = [](const CELSResult &r1, const CELSResult &r2)->std::pair<bool,bool> {
					  int nComp = r1.m_strWord.compare(r2.m_strWord);
					  return std::pair<bool,bool>(nComp < 0, nComp == 0);
				  };
				  auto fnSkip = [](const CELSResult &r1, const CELSResult &r2)->std::pair<bool,bool> {
					  return std::pair<bool,bool>(r1.m_nSkip < r2.m_nSkip, r1.m_nSkip == r2.m_nSkip);
				  };
				  auto fnRef = [](const CELSResult &r1, const CELSResult &r2)->std::pair<bool,bool> {
					  return std::pair<bool,bool>(r1.m_ndxStart.indexEx() < r2.m_ndxStart.indexEx(),
												   r1.m_ndxStart.indexEx() == r2.m_ndxStart.indexEx());
				  };
				  struct TFuncs {
					  std::pair<bool,bool> (*m_first)(const CELSResult &, const CELSResult &);
					  std::pair<bool,bool> (*m_second)(const CELSResult &, const CELSResult &);
					  std::pair<bool,bool> (*m_third)(const CELSResult &, const CELSResult &);
				  } sortFuncs[] = {			// Order must match ELSRESULT_SORT_ORDER_ENUM
					  { fnWord, fnSkip, fnRef },	// ESO_WSR
					  { fnWord, fnRef, fnSkip },	// ESO_WRS
					  { fnRef, fnWord, fnSkip },	// ESO_RWS
					  { fnRef, fnSkip, fnWord },	// ESO_RSW
					  { fnSkip, fnRef, fnWord },	// ESO_SRW
					  { fnSkip, fnWord, fnRef },	// ESO_SWR
				  };
				  std::pair<bool,bool> cmpFirst = sortFuncs[nSortOrder].m_first(r1, r2);
				  if (cmpFirst.first) return true;
				  if (cmpFirst.second) {
					  std::pair<bool,bool> cmpSecond = sortFuncs[nSortOrder].m_second(r1, r2);
					  if (cmpSecond.first) return true;
					  if (cmpSecond.second) {
						  std::pair<bool,bool> cmpThird = sortFuncs[nSortOrder].m_third(r1, r2);
						  if (cmpThird.first) return true;
					  }
				  }
				  return false;
			  });
}

// ============================================================================
