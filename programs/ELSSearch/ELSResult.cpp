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

#include "LetterMatrix.h"

#ifndef IS_CONSOLE_APP
#include "../KJVCanOpener/MimeHelper.h"
#include "../KJVCanOpener/BusyCursor.h"
#include <QMimeData>
#include <QSet>
#endif

#include <QVariant>

// ============================================================================

CELSResultListModel::CELSResultListModel(const CLetterMatrix &letterMatrix, bool bUppercase, QObject *parent)
	:	QAbstractListModel(parent),
		m_letterMatrix(letterMatrix),
		m_bUppercase(bUppercase)
{
}

CELSResultListModel::~CELSResultListModel()
{
}

QVariant CELSResultListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
		switch (section) {
			case 0:
				return tr("Word", "CELSResult");
			case 1:
				return tr("Skip", "CELSResult");
			case 2:
				return tr("Dir", "CELSResult");
			case 3:
				return tr("Nom. Reference", "CELSResult");
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
				return QString(result.m_nDirection == Qt::LeftToRight ? tr("Fwd", "CELSResult") : tr("Rev", "CELSResult"));
			case 3:
				return m_letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxNominal, false);
		}
	} else if (role == UserRole_Reference) {		// Returns the reference
		const CELSResult & result = m_lstResults.at(index.row());
		return QVariant::fromValue(result.m_ndxNominal);
	} else if (role == UserRole_MIMEData) {			// Mime Data for Drag
		const CELSResult & result = m_lstResults.at(index.row());
		QString strMimeData;
		strMimeData += tr("Word", "CELSResult") + QString(": \"%1\"\n").arg(m_bUppercase ? result.m_strWord.toUpper() : result.m_strWord);
		strMimeData += tr("Start Location", "CELSResult") + QString(": %1\n").arg(m_letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxStart, false));
		strMimeData += tr("Nominal Location", "CELSResult") + QString(": %1\n").arg(m_letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxNominal, false));
		strMimeData += tr("End Location", "CELSResult") + QString(": %1\n").arg(m_letterMatrix.bibleDatabase()->PassageReferenceText(result.m_ndxEnd, false));
		strMimeData += tr("Skip", "CELSResult") + QString(": %1\n").arg(result.m_nSkip);
		strMimeData += tr("Direction", "CELSResult") + QString(": %1\n").arg((result.m_nDirection == Qt::LeftToRight) ? "Forward" : "Reverse");
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
	strText += tr("Bible:", "CELSResult") + " " + m_letterMatrix.bibleDatabase()->description() + "\n";
	if (m_letterMatrix.textModifierOptions().testFlag(LMTMO_WordsOfJesusOnly)) {
		strText += tr("Words of Jesus Only", "CELSResult") + "\n";
	} else {
		// There's no Words of Jesus in Colophons or Superscriptions or Book/Chapter Prologues
		if (m_letterMatrix.textModifierOptions().testFlag(LMTMO_IncludeBookPrologues)) strText += tr("Including Book Prologues", "CELSResult") + "\n";
		if (m_letterMatrix.textModifierOptions().testFlag(LMTMO_IncludeChapterPrologues)) strText += tr("Including Chapter Prologues", "CELSResult") + "\n";

		if (m_letterMatrix.textModifierOptions() & (LMTMO_RemoveColophons | LMTMO_RemoveSuperscriptions)) {
			strText += tr("Without", "CELSResult") + " ";
			if (m_letterMatrix.textModifierOptions().testFlag(LMTMO_RemoveColophons)) {
				strText += tr("Colophons", "CELSResult");
				if (m_letterMatrix.textModifierOptions().testFlag(LMTMO_RemoveSuperscriptions)) {
					strText += " " + tr("or Superscriptions", "CELSResult");
				}
			} else {
				strText += tr("Superscriptions", "CELSResult");
			}
			strText += "\n";
		}
	}
	QSet<int> setIndexRows;
	for (auto const & item : indexes) {
		if (setIndexRows.contains(item.row())) continue;
		setIndexRows.insert(item.row());
		strText += "----------------------------------------\n";
		strText += item.data(UserRole_MIMEData).toString();
	}
	strText += "----------------------------------------\n";

	QMimeData *mime = new QMimeData();
	mime->setData(g_constrPlainTextMimeType, strText.toUtf8());

	if (setIndexRows.size() == 1) {
		TPhraseTag tag(CRelIndexEx(indexes.at(0).data(UserRole_Reference).value<CRelIndexEx>()), 1);
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
			return QObject::tr("Word, Skip, Ref", "CELSResult");
		case ESO_WRS:
			return QObject::tr("Word, Ref, Skip", "CELSResult");
		case ESO_RWS:
			return QObject::tr("Ref, Word, Skip", "CELSResult");
		case ESO_RSW:
			return QObject::tr("Ref, Skip, Word", "CELSResult");
		case ESO_SRW:
			return QObject::tr("Skip, Ref, Word", "CELSResult");
		case ESO_SWR:
			return QObject::tr("Skip, Word, Ref", "CELSResult");
		default:
			break;
	}
	return QString();
}

ELSRESULT_SORT_ORDER_ENUM elsresultSortOrderFromLetters(const QString &strLetters)
{
	if (strLetters.compare("wsr", Qt::CaseInsensitive) == 0) {
		return ESO_WSR;
	} else if (strLetters.compare("wrs", Qt::CaseInsensitive) == 0) {
		return ESO_WRS;
	} else if (strLetters.compare("rws", Qt::CaseInsensitive) == 0) {
		return ESO_RWS;
	} else if (strLetters.compare("rsw", Qt::CaseInsensitive) == 0) {
		return ESO_RSW;
	} else if (strLetters.compare("srw", Qt::CaseInsensitive) == 0) {
		return ESO_SRW;
	} else if (strLetters.compare("swr", Qt::CaseInsensitive) == 0) {
		return ESO_SWR;
	}
	return ESO_COUNT;
}

QString elsresultSortOrderToLetters(ELSRESULT_SORT_ORDER_ENUM nSortOrder)
{
	switch (nSortOrder) {
		case ESO_WSR:
			return "wsr";
		case ESO_WRS:
			return "wrs";
		case ESO_RWS:
			return "rws";
		case ESO_RSW:
			return "rsw";
		case ESO_SRW:
			return "srw";
		case ESO_SWR:
			return "swr";
		default:
			break;
	}
	return QString();
}

// ----------------------------------------------------------------------------

QString elsSearchTypeDescription(ELS_SEARCH_TYPE_ENUM nSearchType)
{
	switch (nSearchType) {
		case ESTE_ELS:
			return QObject::tr("Equidistant Letter Sequence (ELS)", "CELSResult");
		case ESTE_FLS:
			return QObject::tr("Fibonacci Letter Sequence (FLS)", "CELSResult");
		case ESTE_FLS_C9_ALL:
			return QObject::tr("FLS Vortex-Based ALL", "CELSResult");
		case ESTE_FLS_C9_124875:
			return QObject::tr("FLS Vortex-Based 1-2-4-8-7-5", "CELSResult");
		case ESTE_FLS_C9_147:
			return QObject::tr("FLS Vortex-Based 1-4-7", "CELSResult");
		case ESTE_FLS_C9_852:
			return QObject::tr("FLS Vortex-Based 8-5-2", "CELSResult");
		case ESTE_FLS_C9_18:
			return QObject::tr("FLS Vortex-Based 1-8", "CELSResult");
		case ESTE_FLS_C9_45:
			return QObject::tr("FLS Vortex-Based 4-5", "CELSResult");
		case ESTE_FLS_C9_72:
			return QObject::tr("FLS Vortex-Based 7-2", "CELSResult");
		case ESTE_FLS_C9_36:
			return QObject::tr("FLS Vortex-Based 3-6", "CELSResult");
		case ESTE_FLS_C9_1:
			return QObject::tr("FLS Vortex-Based 1 Only", "CELSResult");
		case ESTE_FLS_C9_2:
			return QObject::tr("FLS Vortex-Based 2 Only", "CELSResult");
		case ESTE_FLS_C9_3:
			return QObject::tr("FLS Vortex-Based 3 Only", "CELSResult");
		case ESTE_FLS_C9_4:
			return QObject::tr("FLS Vortex-Based 4 Only", "CELSResult");
		case ESTE_FLS_C9_5:
			return QObject::tr("FLS Vortex-Based 5 Only", "CELSResult");
		case ESTE_FLS_C9_6:
			return QObject::tr("FLS Vortex-Based 6 Only", "CELSResult");
		case ESTE_FLS_C9_7:
			return QObject::tr("FLS Vortex-Based 7 Only", "CELSResult");
		case ESTE_FLS_C9_8:
			return QObject::tr("FLS Vortex-Based 8 Only", "CELSResult");
		default:
			break;
	}
	return QString();
}

ELS_SEARCH_TYPE_ENUM elsSearchTypeFromID(const QString &strID)
{
	if (strID.compare("ELS", Qt::CaseInsensitive) == 0) {
		return ESTE_ELS;
	} else if (strID.compare("FLS", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS;
	} else if (strID.compare("FLS_C9_ALL", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_ALL;
	} else if (strID.compare("FLS_C9_124875", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_124875;
	} else if (strID.compare("FLS_C9_147", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_147;
	} else if (strID.compare("FLS_C9_852", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_852;
	} else if (strID.compare("FLS_C9_18", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_18;
	} else if (strID.compare("FLS_C9_45", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_45;
	} else if (strID.compare("FLS_C9_72", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_72;
	} else if (strID.compare("FLS_C9_36", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_36;
	} else if (strID.compare("FLS_C9_1", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_1;
	} else if (strID.compare("FLS_C9_2", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_2;
	} else if (strID.compare("FLS_C9_3", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_3;
	} else if (strID.compare("FLS_C9_4", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_4;
	} else if (strID.compare("FLS_C9_5", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_5;
	} else if (strID.compare("FLS_C9_6", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_6;
	} else if (strID.compare("FLS_C9_7", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_7;
	} else if (strID.compare("FLS_C9_8", Qt::CaseInsensitive) == 0) {
		return ESTE_FLS_C9_8;
	}
	return ESTE_COUNT;
}

QString elsSearchTypeToID(ELS_SEARCH_TYPE_ENUM nSearchType)
{
	switch (nSearchType) {
		case ESTE_ELS:
			return "ELS";
		case ESTE_FLS:
			return "FLS";
		case ESTE_FLS_C9_ALL:
			return "FLS_C9_ALL";
		case ESTE_FLS_C9_124875:
			return "FLS_C9_124875";
		case ESTE_FLS_C9_147:
			return "FLS_C9_147";
		case ESTE_FLS_C9_852:
			return "FLS_C9_852";
		case ESTE_FLS_C9_18:
			return "FLS_C9_18";
		case ESTE_FLS_C9_45:
			return "FLS_C9_45";
		case ESTE_FLS_C9_72:
			return "FLS_C9_72";
		case ESTE_FLS_C9_36:
			return "FLS_C9_36";
		case ESTE_FLS_C9_1:
			return "FLS_C9_1";
		case ESTE_FLS_C9_2:
			return "FLS_C9_2";
		case ESTE_FLS_C9_3:
			return "FLS_C9_3";
		case ESTE_FLS_C9_4:
			return "FLS_C9_4";
		case ESTE_FLS_C9_5:
			return "FLS_C9_5";
		case ESTE_FLS_C9_6:
			return "FLS_C9_6";
		case ESTE_FLS_C9_7:
			return "FLS_C9_7";
		case ESTE_FLS_C9_8:
			return "FLS_C9_8";
		default:
			break;
	}
	return QString();
}

// ----------------------------------------------------------------------------

QModelIndexList CELSResultListModel::getResultIndexes(const CELSResultSet &setResults)
{
	QModelIndexList lstIndexes;

	for (int ndx = 0; ndx < m_lstResults.size(); ++ndx) {
		if (setResults.contains(m_lstResults.at(ndx)))
			lstIndexes.append(createIndex(ndx, 0));
	}

	return lstIndexes;
}

// ----------------------------------------------------------------------------

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
	for (auto const & result : lstResults) {
		if (!m_mapResults.contains(result)) {
			m_mapResults[result] = true;
			m_lstResults.append(result);
		}
	}
	sortResults();
	endResetModel();
}

void CELSResultListModel::clearSearchResults()
{
	beginResetModel();
	m_lstResults.clear();
	m_mapResults.clear();
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
					  return std::pair<bool,bool>(r1.m_ndxNominal.indexEx() < r2.m_ndxNominal.indexEx(),
												   r1.m_ndxNominal.indexEx() == r2.m_ndxNominal.indexEx());
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
