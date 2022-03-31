/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "SearchCriteria.h"
#include "PersistentSettings.h"
#include "ModelRowForwardIterator.h"
#include "ScriptureDocument.h"
#include "Translator.h"
#include "BibleLayout.h"

#include <QTextDocument>

#ifdef USING_WEBCHANNEL
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#endif

// ============================================================================

// Special Search Indexes:
const CRelIndex CSearchCriteria::SSI_COLOPHON = CRelIndex(0, 0, 0, 1);
const CRelIndex CSearchCriteria::SSI_SUPERSCRIPTION = CRelIndex(0, 0, 0, 2);

// ============================================================================

QString CSearchCriteria::searchWithinDescription(CBibleDatabasePtr pBibleDatabase) const
{
	CSearchWithinModel modelSearchWithin(pBibleDatabase, *this);
	return modelSearchWithin.searchWithinDescription();
}

QString CSearchCriteria::searchScopeDescription(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM ssmeScope)
{
	QString strScope;

	switch (ssmeScope) {
		case (SSME_UNSCOPED):
			strScope = QObject::tr("anywhere", "Scope");
			break;
		case (SSME_WHOLE_BIBLE):
			strScope = QObject::tr("together", "Scope");
			break;
		case (SSME_TESTAMENT):
			strScope = QObject::tr("in the same Testament", "Scope");
			break;
		case (SSME_CATEGORY):
			strScope = QObject::tr("in the same Category", "Scope");
			break;
		case (SSME_BOOK):
			strScope = QObject::tr("in the same Book", "Scope");
			break;
		case (SSME_CHAPTER):
			strScope = QObject::tr("in the same Chapter", "Scope");
			break;
		case (SSME_VERSE):
			strScope = QObject::tr("in the same Verse", "Scope");
			break;
		default:
			break;
	}

	return strScope;
}

QString CSearchCriteria::searchScopeDescription() const
{
	return searchScopeDescription(m_nSearchScopeMode);
}

// ============================================================================

CSearchWithinModel::CSearchWithinModel(CBibleDatabasePtr pBibleDatabase, const CSearchCriteria &aSearchCriteria, QObject *pParent)
	:	QAbstractItemModel(pParent),
		m_pBibleDatabase(pBibleDatabase),
		m_bBibleHasColophons(false),			// These will be setup in setupModel(), but need defaults...
		m_bBibleHasSuperscriptions(false)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	setupModel(aSearchCriteria.searchWithin());

	connect(TBibleDatabaseList::instance(), SIGNAL(endChangeBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &, const TBibleDatabaseSettings &, bool)), this, SLOT(en_endChangeBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &, const TBibleDatabaseSettings &, bool)));
}

CSearchWithinModel::~CSearchWithinModel()
{

}

// ============================================================================

void CSearchWithinModel::en_endChangeBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &oldSettings,
															const TBibleDatabaseSettings &newSettings, bool bForce)
{
	if ((strUUID.compare(m_pBibleDatabase->compatibilityUUID(), Qt::CaseInsensitive) == 0) &&
		(bForce ||
		 (newSettings.categoryGroup() != oldSettings.categoryGroup()) ||
		 (newSettings.versification() != oldSettings.versification()))) {
		setupModel(searchWithin());
	}
}

void CSearchWithinModel::setupModel(const TRelativeIndexSet &aSetSearchWithin)
{
	beginResetModel();

	m_rootSearchWithinModelIndex.clear();

	m_bBibleHasColophons = m_pBibleDatabase->hasColophons();
	m_bBibleHasSuperscriptions = m_pBibleDatabase->hasSuperscriptions();

	// Build our model data:
	// The root node is the "Whole Bible"
	if (m_bBibleHasColophons) {
		m_rootSearchWithinModelIndex.insertIndex(CSearchCriteria::SSME_COLOPHON, 0)->setCheck(aSetSearchWithin.find(CSearchCriteria::SSI_COLOPHON) != aSetSearchWithin.end());
	}
	if (m_bBibleHasSuperscriptions) {
		m_rootSearchWithinModelIndex.insertIndex(CSearchCriteria::SSME_SUPERSCRIPTION, 0)->setCheck(aSetSearchWithin.find(CSearchCriteria::SSI_SUPERSCRIPTION) != aSetSearchWithin.end());
	}
	for (unsigned int nTst = 1; nTst <= m_pBibleDatabase->bibleEntry().m_nNumTst; ++nTst) {
		const CTestamentEntry *pTestamentEntry = m_pBibleDatabase->testamentEntry(nTst);
		Q_ASSERT(pTestamentEntry != nullptr);
		if (pTestamentEntry == nullptr) continue;
		if (pTestamentEntry->m_nNumWrd == 0) continue;		// Skip testament if it has no words (like a New Testament only database)
		CSearchWithinModelIndex *pIndexTestament = m_rootSearchWithinModelIndex.insertIndex(CSearchCriteria::SSME_TESTAMENT, nTst);
		CSearchWithinModelIndexMap mapCategoryIndexes;
		for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
			const CBookEntry *pBookEntry = m_pBibleDatabase->bookEntry(nBk);
			Q_ASSERT(pBookEntry != nullptr);
			if (pBookEntry == nullptr) continue;
			if (pBookEntry->m_nTstNdx != nTst) continue;
			if (pBookEntry->m_nNumWrd == 0) continue;		// Skip books if it has no words (like a Pentateuch only database)
			BIBLE_BOOK_CATEGORIES_ENUM nCat = m_pBibleDatabase->bookCategory(CRelIndex(nBk, 0, 0, 0));
			CSearchWithinModelIndexMap::const_iterator itrCategoryIndexes = mapCategoryIndexes.constFind(nCat);
			CSearchWithinModelIndex *pIndexCategory = nullptr;
			if (itrCategoryIndexes != mapCategoryIndexes.constEnd()) {
				pIndexCategory = itrCategoryIndexes.value();
			} else {
				pIndexCategory = pIndexTestament->insertIndex(CSearchCriteria::SSME_CATEGORY, nCat);
				mapCategoryIndexes.insert(nCat, pIndexCategory);
			}
			Q_ASSERT(pIndexCategory != nullptr);
			CSearchWithinModelIndex *pIndexBook = pIndexCategory->insertIndex(CSearchCriteria::SSME_BOOK, nBk);
			pIndexBook->setCheck(aSetSearchWithin.find(CRelIndex(nBk, 0, 0, 0)) != aSetSearchWithin.end());
		}
	}

	endResetModel();
}

// ============================================================================

QString CSearchWithinModel::searchWithinDescription() const
{
	QStringList lstDescription;
	const CSearchWithinModelIndex *pColophon = nullptr;
	const CSearchWithinModelIndex *pSuperscription = nullptr;

	for (CModelRowForwardIterator fwdItr(this); fwdItr; /* Increment inside loop */) {
		const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(*fwdItr);
		Q_ASSERT(pSearchWithinModelIndex != nullptr);
		// Fully checked items completely define it, so use it -- except for Category,
		//	since they are subjective, which will translate to the child names:
		if ((pSearchWithinModelIndex->checkState() == Qt::Checked) && (pSearchWithinModelIndex->ssme() != CSearchCriteria::SSME_CATEGORY)) {
			if (pSearchWithinModelIndex->ssme() == CSearchCriteria::SSME_COLOPHON) {
				pColophon = pSearchWithinModelIndex;
			} else if (pSearchWithinModelIndex->ssme() == CSearchCriteria::SSME_SUPERSCRIPTION) {
				pSuperscription = pSearchWithinModelIndex;
			} else {
				lstDescription.append(fwdItr->data(Qt::EditRole).toString());
			}
			fwdItr.nextSibling();
		} else {
			++fwdItr;
		}
	}
	if ((m_bBibleHasColophons) && (pColophon != nullptr)) lstDescription.append(data(pColophon, Qt::EditRole).toString());
	if ((m_bBibleHasSuperscriptions) && (pSuperscription != nullptr)) lstDescription.append(data(pSuperscription, Qt::EditRole).toString());

	return lstDescription.join(QString(", "));
}

TRelativeIndexSet CSearchWithinModel::searchWithin() const
{
	TRelativeIndexSet setIndexes;

	bool bSelectAll = true;
	for (CModelRowForwardIterator fwdItr(this); fwdItr; ++fwdItr) {
		const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(*fwdItr);
		Q_ASSERT(pSearchWithinModelIndex != nullptr);
		CRelIndex ndxItem = fwdItr->data(SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
		if ((ndxItem.isSet()) && (pSearchWithinModelIndex->checkState() == Qt::Checked)) {
			setIndexes.insert(ndxItem);
		} else {
			bSelectAll = false;
		}
	}

	// Treat no superscriptions and no colophons as if the are enabled (only non-existant):
	if (!m_bBibleHasColophons) setIndexes.insert(CSearchCriteria::SSI_COLOPHON);
	if (!m_bBibleHasSuperscriptions) setIndexes.insert(CSearchCriteria::SSI_SUPERSCRIPTION);

	if (bSelectAll) setIndexes.clear();			// An empty list is a special case for select all

	return setIndexes;
}

void CSearchWithinModel::setSearchWithin(const TRelativeIndexSet &aSetSearchWithin)
{
	for (CModelRowForwardIterator fwdItr(this); fwdItr; ++fwdItr) {
		CRelIndex ndxItem = fwdItr->data(SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
		if (ndxItem.isSet()) {
			setData(*fwdItr, ((aSetSearchWithin.find(ndxItem) != aSetSearchWithin.end()) ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}
	}
}

int CSearchWithinModel::rowCount(const QModelIndex &zParent) const
{
	if (!zParent.isValid()) return 1;			// Whole Bible has 1 Entry
	return toSearchWithinModelIndex(zParent)->childIndexCount();
}

int CSearchWithinModel::columnCount(const QModelIndex &zParent) const
{
	if (!zParent.isValid()) return 1;			// Whole Bible has 1 Entry
	return (toSearchWithinModelIndex(zParent)->childIndexCount() ? 1 : 0);
}

QModelIndex	CSearchWithinModel::index(int row, int column, const QModelIndex &zParent) const
{
	if (!hasIndex(row, column, zParent)) return QModelIndex();

	if (!zParent.isValid()) {
		Q_ASSERT(row == 0);
		return createIndex(0, 0, fromSearchWithinModelIndex(&m_rootSearchWithinModelIndex));
	}
	const CSearchWithinModelIndex *pSearchWithinModelParentIndex = toSearchWithinModelIndex(zParent);
	Q_ASSERT(pSearchWithinModelParentIndex != nullptr);
	if (pSearchWithinModelParentIndex == nullptr) return QModelIndex();

	Q_ASSERT(row < pSearchWithinModelParentIndex->childIndexCount());
	return createIndex(row, column, fromSearchWithinModelIndex(pSearchWithinModelParentIndex->childIndex(row)));
}

QModelIndex CSearchWithinModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) return QModelIndex();

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	Q_ASSERT(pSearchWithinModelIndex != nullptr);
	if (pSearchWithinModelIndex == nullptr) return QModelIndex();
	const CSearchWithinModelIndex *pParentSearchWithinModelIndex = pSearchWithinModelIndex->parentIndex();
	if (pParentSearchWithinModelIndex == nullptr) return QModelIndex();
	int nRow = 0;
	const CSearchWithinModelIndex *pParentParentSearchWithinModelIndex = pParentSearchWithinModelIndex->parentIndex();
	if (pParentParentSearchWithinModelIndex != nullptr) nRow = pParentParentSearchWithinModelIndex->indexOfChild(pParentSearchWithinModelIndex);

	return createIndex(nRow, 0, fromSearchWithinModelIndex(pParentSearchWithinModelIndex));
}

QVariant CSearchWithinModel::data(const QModelIndex &index, int role) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	if (!index.isValid()) return QVariant();

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	Q_ASSERT(pSearchWithinModelIndex != nullptr);

	return data(pSearchWithinModelIndex, role);
}

QVariant CSearchWithinModel::data(const CSearchWithinModelIndex *pSearchWithinModelIndex, int role) const
{
	if (pSearchWithinModelIndex == nullptr) return QVariant();

	uint32_t nItem = pSearchWithinModelIndex->itemIndex();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole) || (role == CSearchWithinModel::SWMDRE_WEBCHANNEL_ROLE)) {
		switch (pSearchWithinModelIndex->ssme()) {
			case CSearchCriteria::SSME_WHOLE_BIBLE:
			{
				// Search for "Entire Bible".  First try and see if we can translate it in the language of the selected Bible,
				//		but if not, try in the current language setting
				QString strEntireBible = tr("Entire Bible", "Scope");
				if ((role == Qt::DisplayRole) ||		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					(role == SWMDRE_WEBCHANNEL_ROLE)) {
					TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(toQtLanguageName(m_pBibleDatabase->langID()));
					if (!pTranslator.isNull()) {
						QString strTemp = pTranslator->translatorApp().translate("CSearchWithinModel", "Entire Bible", "Scope");
						if (!strTemp.isEmpty()) strEntireBible = strTemp;
					}
				}
				return strEntireBible;
			}
			case CSearchCriteria::SSME_COLOPHON:
			{
				// Search for "Colophons".  First try and see if we can translate it in the language of the selected Bible,
				//		but if not, try in the current language setting
				QString strColophons = tr("Colophons", "Scope");
				if ((role == Qt::DisplayRole) ||		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					(role == SWMDRE_WEBCHANNEL_ROLE)) {
					TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(toQtLanguageName(m_pBibleDatabase->langID()));
					if (!pTranslator.isNull()) {
						QString strTemp = pTranslator->translatorApp().translate("CSearchWithinModel", "Colophons", "Scope");
						if (!strTemp.isEmpty()) strColophons = strTemp;
					}
				}
				return strColophons;
			}
			case CSearchCriteria::SSME_SUPERSCRIPTION:
			{
				// Search for "Superscriptions".  First try and see if we can translate it in the language of the selected Bible,
				//		but if not, try in the current language setting
				QString strSuperscriptions = tr("Superscriptions", "Scope");
				if ((role == Qt::DisplayRole) ||		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					(role == SWMDRE_WEBCHANNEL_ROLE)) {
					TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(toQtLanguageName(m_pBibleDatabase->langID()));
					if (!pTranslator.isNull()) {
						QString strTemp = pTranslator->translatorApp().translate("CSearchWithinModel", "Superscriptions", "Scope");
						if (!strTemp.isEmpty()) strSuperscriptions = strTemp;
					}
				}
				return strSuperscriptions;
			}
			case CSearchCriteria::SSME_TESTAMENT:
			{
				if ((role == Qt::DisplayRole) ||		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					(role == SWMDRE_WEBCHANNEL_ROLE)) {
					Q_ASSERT(m_pBibleDatabase->testamentEntry(nItem) != nullptr);
					return m_pBibleDatabase->testamentEntry(nItem)->m_strTstName;
				} else {
					return CBibleTestaments::name(nItem);
				}
			}
			case CSearchCriteria::SSME_CATEGORY:
				Q_ASSERT(nItem < BBCE_COUNT);
				return CBibleBookCategories::name(static_cast<BIBLE_BOOK_CATEGORIES_ENUM>(nItem));
			case CSearchCriteria::SSME_BOOK:
			{
				const CBookEntry *pBookEntry = m_pBibleDatabase->bookEntry(nItem);
				Q_ASSERT(pBookEntry != nullptr);
				QString strBook = pBookEntry->m_strBkName;
				// role == CSearchWithinModel::SWMDRE_WEBCHANNEL_ROLE ignores description like Qt::EditRole
				if ((role == Qt::DisplayRole) && (!pBookEntry->m_strDesc.isEmpty())) {
					QTextDocument docBook;
					strBook = QString("<pre>") + strBook;
					strBook += QString(" (%1)").arg(pBookEntry->m_strDesc);
					strBook += QString("</pre>");
					docBook.setHtml(strBook);
					CScripturePlainTextBuilder plainTextBook;
					CScriptureTextDocumentDirector scriptureDirector(&plainTextBook);
					scriptureDirector.processDocument(&docBook);
					strBook = plainTextBook.getResult().trimmed();
				}
				return strBook;
			}
			default:
				Q_ASSERT(false);
				break;
		}
	} else if (role == Qt::ToolTipRole) {
		switch (pSearchWithinModelIndex->ssme()) {
			case CSearchCriteria::SSME_COLOPHON:
				return tr("A Colophon is an inscription at the end of a book\n"
						  "or manuscript usually with facts about its production.\n"
						  "In the Bible, they are usually found at the end of\n"
						  "the Epistles of the New Testament.", "Scope");
			case CSearchCriteria::SSME_SUPERSCRIPTION:
				return tr("A Superscription is text written above a chapter\n"
						  "describing the content of the chapter.  In the Bible,\n"
						  "they are usually found in the Book of Psalms.", "Scope");
			default:
				return QString();
		}
	} else if (role == Qt::CheckStateRole) {
		return pSearchWithinModelIndex->checkState();
	} else if (role == SWMDRE_REL_INDEX_ROLE) {
		switch (pSearchWithinModelIndex->ssme()) {
			case CSearchCriteria::SSME_COLOPHON:
				return QVariant::fromValue(CSearchCriteria::SSI_COLOPHON);
			case CSearchCriteria::SSME_SUPERSCRIPTION:
				return QVariant::fromValue(CSearchCriteria::SSI_SUPERSCRIPTION);
			case CSearchCriteria::SSME_BOOK:
			{
				const CBookEntry *pBookEntry = m_pBibleDatabase->bookEntry(nItem);
				Q_ASSERT(pBookEntry != nullptr);
				return QVariant::fromValue(CRelIndex(nItem, 0, 0, 0));
			}
			default:
				return QVariant::fromValue(CRelIndex());
		}
	}

	return QVariant();
}

bool CSearchWithinModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) return false;

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	Q_ASSERT(pSearchWithinModelIndex != nullptr);

	if (role == Qt::CheckStateRole) {
		pSearchWithinModelIndex->setCheck(value.toBool());
		fireChildrenChange(index);
		QModelIndex indexParent = parent(index);
		while (indexParent.isValid()) {
			emit dataChanged(indexParent, indexParent);
			indexParent = parent(indexParent);
		}
		emit changedSearchWithin();
		return true;
	}

	return false;
}

void CSearchWithinModel::fireChildrenChange(const QModelIndex &index)
{
	if (!index.isValid()) return;
	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	Q_ASSERT(pSearchWithinModelIndex != nullptr);
	emit dataChanged(index, index);
	for (int ndx = 0; ndx < pSearchWithinModelIndex->childIndexCount(); ++ndx) {
		fireChildrenChange(createIndex(ndx, 0, fromSearchWithinModelIndex(pSearchWithinModelIndex->childIndex(ndx))));
	}
}

Qt::ItemFlags CSearchWithinModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	Q_ASSERT(pSearchWithinModelIndex != nullptr);
	if (pSearchWithinModelIndex == nullptr) return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	if (index.data(SWMDRE_REL_INDEX_ROLE).value<CRelIndex>() == CSearchCriteria::SSI_COLOPHON) {
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled |
				(m_bBibleHasColophons ? Qt::ItemIsEnabled : Qt::ItemFlags()) |
				(m_bBibleHasColophons ? Qt::ItemIsUserCheckable : Qt::ItemFlags());
	}

	if (index.data(SWMDRE_REL_INDEX_ROLE).value<CRelIndex>() == CSearchCriteria::SSI_SUPERSCRIPTION) {
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled |
				(m_bBibleHasSuperscriptions ? Qt::ItemIsEnabled : Qt::ItemFlags()) |
				(m_bBibleHasSuperscriptions ? Qt::ItemIsUserCheckable : Qt::ItemFlags());
	}

	if (pSearchWithinModelIndex->ssme() != CSearchCriteria::SSME_BOOK) {
#if QT_VERSION >= 0x050600
		return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate;
#else
		return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsTristate;
#endif
	}
	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;
}

bool CSearchWithinModel::insertRows(int row, int count, const QModelIndex &zParent)
{
	Q_UNUSED(row);
	Q_UNUSED(count);
	Q_UNUSED(zParent);
	return false;
}

bool CSearchWithinModel::removeRows(int row, int count, const QModelIndex &zParent)
{
	Q_UNUSED(row);
	Q_UNUSED(count);
	Q_UNUSED(zParent);

	return false;
}

void CSearchWithinModel::sort(int column, Qt::SortOrder order)
{
	Q_UNUSED(column);
	Q_UNUSED(order);
	Q_ASSERT(false);
}

// ============================================================================

#ifdef USING_WEBCHANNEL

static void nodeToJson(const CSearchWithinModel &model, QJsonArray &arrayRoot, QModelIndex &mdlIndex, int nLevel)
{
	if (model.hasChildren(mdlIndex)) {
		int nRowCount = model.rowCount(mdlIndex);
		int nRow = 0;
		while (nRowCount--) {
			QModelIndex mdlIndexChild = model.index(nRow, 0, mdlIndex);
			QJsonObject objNode;
			if (model.hasChildren(mdlIndexChild)) {
				QJsonArray arrayNode;
				nodeToJson(model, arrayNode, mdlIndexChild, nLevel+1);
				objNode["title"] = model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_WEBCHANNEL_ROLE).toString();
				if (nLevel < 2) objNode["expanded"] = true;
				objNode["folder"] = true;
				objNode["children"] = arrayNode;
				CRelIndex relNdx = model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
				if (relNdx.isSet()) objNode["key"] = relNdx.asAnchor();
				const CSearchWithinModelIndex *pSearchWithinModelIndex = model.toSearchWithinModelIndex(mdlIndexChild);
				if ((pSearchWithinModelIndex) && (pSearchWithinModelIndex->checkState() != Qt::Unchecked)) objNode["selected"] = true;
				QString strToolTip = model.data(mdlIndexChild, Qt::ToolTipRole).toString();
				if (!strToolTip.isEmpty()) objNode["tooltip"] = strToolTip;
			} else {
				objNode["title"] = model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_WEBCHANNEL_ROLE).toString();
				CRelIndex relNdx = model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
				if (relNdx.isSet()) objNode["key"] = relNdx.asAnchor();
				const CSearchWithinModelIndex *pSearchWithinModelIndex = model.toSearchWithinModelIndex(mdlIndexChild);
				if ((pSearchWithinModelIndex) && (pSearchWithinModelIndex->checkState() != Qt::Unchecked)) objNode["selected"] = true;
				QString strToolTip = model.data(mdlIndexChild, Qt::ToolTipRole).toString();
				if (!strToolTip.isEmpty()) objNode["tooltip"] = strToolTip;
			}
			arrayRoot.append(objNode);
			++nRow;
		}
	}
}

QString CSearchWithinModel::toWebChannelJson() const
{
	QJsonArray arrayRoot;
	QModelIndex mdlIndex;

	if (hasChildren(mdlIndex)) {
		nodeToJson(*this, arrayRoot, mdlIndex, 0);
	}

	return QJsonDocument(arrayRoot).toJson(QJsonDocument::Compact);
}


static void nodeToHtml(const CSearchWithinModel &model, QString &strResult, QModelIndex &mdlIndex, int nLevel)
{
	if (model.hasChildren(mdlIndex)) {
		strResult += QString("  ").repeated(nLevel*2+1);
		strResult += "<ul>\n";
		int nRowCount = model.rowCount(mdlIndex);
		int nRow = 0;
		while (nRowCount--) {
			QModelIndex mdlIndexChild = model.index(nRow, 0, mdlIndex);
			if (model.hasChildren(mdlIndexChild)) {
				strResult += QString("  ").repeated(nLevel*2+2);
				strResult += QString("<li class=\"%1\"><input type=\"checkbox\" /><label>%2</label></li>\n")
										.arg((nLevel < 2) ? "expanded" : "collapsed")
										.arg(model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_WEBCHANNEL_ROLE).toString());
				nodeToHtml(model, strResult, mdlIndexChild, nLevel+1);
			} else {
				strResult += QString("  ").repeated(nLevel*2+2);
				strResult += QString("<li class=\"leaf\"><input type=\"checkbox\" /><label>%1</label></li>\n")
										.arg(model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_WEBCHANNEL_ROLE).toString());
			}
			++nRow;
		}
		strResult += QString("  ").repeated(nLevel*2+1);
		strResult += "</ul>\n";
	}
}

QString CSearchWithinModel::toWebChannelHtml() const
{
	QString strResult;
	QModelIndex mdlIndex;

	if (hasChildren(mdlIndex)) {
		strResult += "<div id=\"searchWithinTree\">\n";
		nodeToHtml(*this, strResult, mdlIndex, 0);
		strResult += "</div>\n";
	}

	return strResult;
}

#endif

// ============================================================================

