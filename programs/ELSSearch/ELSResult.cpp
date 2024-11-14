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

#include "../KJVCanOpener/MimeHelper.h"
#include "../KJVCanOpener/BusyCursor.h"

#include <QVariant>
#include <QMimeData>

// ============================================================================

CELSResultListModel::CELSResultListModel(CBibleDatabasePtr pBibleDatabase, QObject *parent)
	:	QAbstractListModel(parent),
		m_pBibleDatabase(pBibleDatabase)
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
				return result.m_strWord;
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
		strMimeData += QString("Word: \"%1\"\n").arg(result.m_strWord);
		strMimeData += QString("Start Location: %1\n").arg(m_pBibleDatabase->PassageReferenceText(result.m_ndxStart, false));
		strMimeData += QString("Skip: %1\n").arg(result.m_nSkip);
		strMimeData == QString("Direction: %1\n").arg((result.m_nDirection == Qt::LeftToRight) ? "Forward" : "Reverse");
		return strMimeData;
	}

	return QVariant();
}

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

void CELSResultListModel::setSearchResults(const CELSResultList &lstResults)
{
	beginResetModel();
	m_lstResults.append(lstResults);
	endResetModel();
}

void CELSResultListModel::clearSearchResults()
{
	beginResetModel();
	m_lstResults.clear();
	endResetModel();
}

// ============================================================================
