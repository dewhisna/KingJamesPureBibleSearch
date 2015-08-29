/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

#include "KJVSearchCriteria.h"
#include "PersistentSettings.h"
#include "ModelRowForwardIterator.h"
#include "ScriptureDocument.h"
#include "Translator.h"
#include "dbDescriptors.h"
#include "VerseListModel.h"

#include "BusyCursor.h"

#include <QColor>
#include <QTextDocument>

#ifdef USING_WEBCHANNEL
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#endif

#include <assert.h>

// ============================================================================

// Special Search Indexes:
const CRelIndex CSearchCriteria::SSI_COLOPHON = CRelIndex(0, 0, 0, 1);
const CRelIndex CSearchCriteria::SSI_SUPERSCRIPTION = CRelIndex(0, 0, 0, 2);

// ============================================================================

bool CSearchCriteria::bibleHasColophons(CBibleDatabasePtr pBibleDatabase) const
{
	assert(!pBibleDatabase.isNull());

	for (unsigned int nBk = 1; nBk <= pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry *pBookEntry = pBibleDatabase->bookEntry(nBk);
		if (pBookEntry == NULL) continue;
		if (pBookEntry->m_bHaveColophon) return true;
	}

	return false;
}

bool CSearchCriteria::bibleHasSuperscriptions(CBibleDatabasePtr pBibleDatabase) const
{
	assert(!pBibleDatabase.isNull());

	for (unsigned int nBk = 1; nBk <= pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry *pBookEntry = pBibleDatabase->bookEntry(nBk);
		if (pBookEntry == NULL) continue;
		for (unsigned int nChp = 1; nChp <= pBookEntry->m_nNumChp; ++nChp) {
			const CChapterEntry *pChapterEntry = pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0));
			if ((pChapterEntry != NULL) && (pChapterEntry->m_bHaveSuperscription)) return true;
		}
	}

	return false;
}

QString CSearchCriteria::searchWithinDescription(CBibleDatabasePtr pBibleDatabase) const
{
	CSearchWithinModel modelSearchWithin(pBibleDatabase, *this);
	return modelSearchWithin.searchWithinDescription();
}

QString CSearchCriteria::searchScopeDescription() const
{
	QString strScope;

	switch (m_nSearchScopeMode) {
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

// ============================================================================

CSearchWithinModel::CSearchWithinModel(CBibleDatabasePtr pBibleDatabase, const CSearchCriteria &aSearchCriteria, QObject *pParent)
	:	QAbstractItemModel(pParent),
		m_pBibleDatabase(pBibleDatabase)
{
	assert(!m_pBibleDatabase.isNull());

	const TRelativeIndexSet &aSetSearchWithin = aSearchCriteria.searchWithin();
	m_bBibleHasColophons = aSearchCriteria.bibleHasColophons(pBibleDatabase);
	m_bBibleHasSuperscriptions = aSearchCriteria.bibleHasSuperscriptions(pBibleDatabase);

	// Build our model data (which should be static with the given Bible Database):
	// The root node is the "Whole Bible"
	if (m_bBibleHasColophons) {
		m_rootSearchWithinModelIndex.insertIndex(CSearchCriteria::SSME_COLOPHON, 0)->setCheck(aSetSearchWithin.find(CSearchCriteria::SSI_COLOPHON) != aSetSearchWithin.end());
	}
	if (m_bBibleHasSuperscriptions) {
		m_rootSearchWithinModelIndex.insertIndex(CSearchCriteria::SSME_SUPERSCRIPTION, 0)->setCheck(aSetSearchWithin.find(CSearchCriteria::SSI_SUPERSCRIPTION) != aSetSearchWithin.end());
	}
	for (unsigned int nTst = 1; nTst <= m_pBibleDatabase->bibleEntry().m_nNumTst; ++nTst) {
		const CTestamentEntry *pTestamentEntry = m_pBibleDatabase->testamentEntry(nTst);
		assert(pTestamentEntry != NULL);
		if (pTestamentEntry == NULL) continue;
		CSearchWithinModelIndex *pIndexTestament = m_rootSearchWithinModelIndex.insertIndex(CSearchCriteria::SSME_TESTAMENT, nTst);
		CSearchWithinModelIndexMap mapCategoryIndexes;
		for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
			const CBookEntry *pBookEntry = m_pBibleDatabase->bookEntry(nBk);
			assert(pBookEntry != NULL);
			if (pBookEntry == NULL) continue;
			if (pBookEntry->m_nTstNdx != nTst) continue;
			CSearchWithinModelIndexMap::const_iterator itrCategoryIndexes = mapCategoryIndexes.find(pBookEntry->m_nCatNdx);
			CSearchWithinModelIndex *pIndexCategory = NULL;
			if (itrCategoryIndexes != mapCategoryIndexes.constEnd()) {
				pIndexCategory = itrCategoryIndexes.value();
			} else {
				pIndexCategory = pIndexTestament->insertIndex(CSearchCriteria::SSME_CATEGORY, pBookEntry->m_nCatNdx);
				mapCategoryIndexes.insert(pBookEntry->m_nCatNdx, pIndexCategory);
			}
			assert(pIndexCategory != NULL);
			CSearchWithinModelIndex *pIndexBook = pIndexCategory->insertIndex(CSearchCriteria::SSME_BOOK, nBk);
			pIndexBook->setCheck(aSetSearchWithin.find(CRelIndex(nBk, 0, 0, 0)) != aSetSearchWithin.end());
		}
	}
}

CSearchWithinModel::~CSearchWithinModel()
{

}

QString CSearchWithinModel::searchWithinDescription() const
{
	QStringList lstDescription;
	const CSearchWithinModelIndex *pColophon = NULL;
	const CSearchWithinModelIndex *pSuperscription = NULL;

	for (CModelRowForwardIterator fwdItr(this); fwdItr; /* Increment inside loop */) {
		const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(*fwdItr);
		assert(pSearchWithinModelIndex != NULL);
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
	if ((m_bBibleHasColophons) && (pColophon != NULL)) lstDescription.append(data(pColophon, Qt::EditRole).toString());
	if ((m_bBibleHasSuperscriptions) && (pSuperscription != NULL)) lstDescription.append(data(pSuperscription, Qt::EditRole).toString());

	return lstDescription.join(QString(", "));
}

TRelativeIndexSet CSearchWithinModel::searchWithin() const
{
	TRelativeIndexSet setIndexes;

	bool bSelectAll = true;
	for (CModelRowForwardIterator fwdItr(this); fwdItr; ++fwdItr) {
		const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(*fwdItr);
		assert(pSearchWithinModelIndex != NULL);
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
		assert(row == 0);
		return createIndex(0, 0, fromSearchWithinModelIndex(&m_rootSearchWithinModelIndex));
	}
	const CSearchWithinModelIndex *pSearchWithinModelParentIndex = toSearchWithinModelIndex(zParent);
	assert(pSearchWithinModelParentIndex != NULL);
	if (pSearchWithinModelParentIndex == NULL) return QModelIndex();

	assert(row < pSearchWithinModelParentIndex->childIndexCount());
	return createIndex(row, column, fromSearchWithinModelIndex(pSearchWithinModelParentIndex->childIndex(row)));
}

QModelIndex CSearchWithinModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) return QModelIndex();

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	assert(pSearchWithinModelIndex != NULL);
	if (pSearchWithinModelIndex == NULL) return QModelIndex();
	const CSearchWithinModelIndex *pParentSearchWithinModelIndex = pSearchWithinModelIndex->parentIndex();
	if (pParentSearchWithinModelIndex == NULL) return QModelIndex();
	int nRow = 0;
	const CSearchWithinModelIndex *pParentParentSearchWithinModelIndex = pParentSearchWithinModelIndex->parentIndex();
	if (pParentParentSearchWithinModelIndex != NULL) nRow = pParentParentSearchWithinModelIndex->indexOfChild(pParentSearchWithinModelIndex);

	return createIndex(nRow, 0, fromSearchWithinModelIndex(pParentSearchWithinModelIndex));
}

QVariant CSearchWithinModel::data(const QModelIndex &index, int role) const
{
	assert(!m_pBibleDatabase.isNull());

	if (!index.isValid()) return QVariant();

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	assert(pSearchWithinModelIndex != NULL);

	return data(pSearchWithinModelIndex, role);
}

QVariant CSearchWithinModel::data(const CSearchWithinModelIndex *pSearchWithinModelIndex, int role) const
{
	if (pSearchWithinModelIndex == NULL) return QVariant();

	uint32_t nItem = pSearchWithinModelIndex->itemIndex();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		switch (pSearchWithinModelIndex->ssme()) {
			case CSearchCriteria::SSME_WHOLE_BIBLE:
			{
				// Search for "Entire Bible".  First try and see if we can translate it in the language of the selected Bible,
				//		but if not, try in the current language setting
				QString strEntireBible = tr("Entire Bible", "Scope");
				if (role == Qt::DisplayRole) {		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
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
				if (role == Qt::DisplayRole) {		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
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
				if (role == Qt::DisplayRole) {		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
					if (!pTranslator.isNull()) {
						QString strTemp = pTranslator->translatorApp().translate("CSearchWithinModel", "Superscriptions", "Scope");
						if (!strTemp.isEmpty()) strSuperscriptions = strTemp;
					}
				}
				return strSuperscriptions;
			}
			case CSearchCriteria::SSME_TESTAMENT:
			{
				if (role == Qt::DisplayRole) {		// The Edit-Role will return the current language version and the Display-Role will be for the Bible Database
					assert(m_pBibleDatabase->testamentEntry(nItem) != NULL);
					return m_pBibleDatabase->testamentEntry(nItem)->m_strTstName;
				} else {
					return xc_dbDescriptors::translatedBibleTestamentName(m_pBibleDatabase->compatibilityUUID(), nItem);
				}
			}
			case CSearchCriteria::SSME_CATEGORY:
				assert(m_pBibleDatabase->bookCategoryEntry(nItem) != NULL);
				return m_pBibleDatabase->bookCategoryEntry(nItem)->m_strCategoryName;
			case CSearchCriteria::SSME_BOOK:
			{
				const CBookEntry *pBookEntry = m_pBibleDatabase->bookEntry(nItem);
				assert(pBookEntry != NULL);
				QString strBook = pBookEntry->m_strBkName;
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
				assert(false);
				break;
		}
	}

	if (role == Qt::ToolTipRole) {
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
	}

	if (role == Qt::CheckStateRole) {
		return pSearchWithinModelIndex->checkState();
	}

	if (role == SWMDRE_REL_INDEX_ROLE) {
		switch (pSearchWithinModelIndex->ssme()) {
			case CSearchCriteria::SSME_COLOPHON:
				return QVariant::fromValue(CSearchCriteria::SSI_COLOPHON);
			case CSearchCriteria::SSME_SUPERSCRIPTION:
				return QVariant::fromValue(CSearchCriteria::SSI_SUPERSCRIPTION);
			case CSearchCriteria::SSME_BOOK:
			{
				const CBookEntry *pBookEntry = m_pBibleDatabase->bookEntry(nItem);
				assert(pBookEntry != NULL);
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
	assert(pSearchWithinModelIndex != NULL);

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
	assert(pSearchWithinModelIndex != NULL);
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
	assert(pSearchWithinModelIndex != NULL);
	if (pSearchWithinModelIndex == NULL) return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	if (index.data(SWMDRE_REL_INDEX_ROLE).value<CRelIndex>() == CSearchCriteria::SSI_COLOPHON) {
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled |
				(m_bBibleHasColophons ? Qt::ItemIsEnabled : static_cast<Qt::ItemFlags>(0)) |
				(m_bBibleHasColophons ? Qt::ItemIsUserCheckable : static_cast<Qt::ItemFlags>(0));
	}

	if (index.data(SWMDRE_REL_INDEX_ROLE).value<CRelIndex>() == CSearchCriteria::SSI_SUPERSCRIPTION) {
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled |
				(m_bBibleHasSuperscriptions ? Qt::ItemIsEnabled : static_cast<Qt::ItemFlags>(0)) |
				(m_bBibleHasSuperscriptions ? Qt::ItemIsUserCheckable : static_cast<Qt::ItemFlags>(0));
	}

	if (pSearchWithinModelIndex->ssme() != CSearchCriteria::SSME_BOOK) {
		return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsTristate;
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
	assert(false);
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
				objNode["title"] = model.data(mdlIndexChild, Qt::DisplayRole).toString();
				if (nLevel < 2) objNode["expanded"] = true;
				objNode["folder"] = true;
				objNode["children"] = arrayNode;
				CRelIndex relNdx = model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
				if (relNdx.isSet()) objNode["key"] = relNdx.asAnchor();
				const CSearchWithinModelIndex *pSearchWithinModelIndex = model.toSearchWithinModelIndex(mdlIndexChild);
				if ((pSearchWithinModelIndex) && (pSearchWithinModelIndex->checkState() != Qt::Unchecked)) objNode["selected"] = true;
				arrayRoot.append(objNode);
			} else {
				objNode["title"] = model.data(mdlIndexChild, Qt::DisplayRole).toString();
				CRelIndex relNdx = model.data(mdlIndexChild, CSearchWithinModel::SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
				if (relNdx.isSet()) objNode["key"] = relNdx.asAnchor();
				const CSearchWithinModelIndex *pSearchWithinModelIndex = model.toSearchWithinModelIndex(mdlIndexChild);
				if ((pSearchWithinModelIndex) && (pSearchWithinModelIndex->checkState() != Qt::Unchecked)) objNode["selected"] = true;
				arrayRoot.append(objNode);
			}
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
										.arg(model.data(mdlIndexChild, Qt::DisplayRole).toString());
				nodeToHtml(model, strResult, mdlIndexChild, nLevel+1);
			} else {
				strResult += QString("  ").repeated(nLevel*2+2);
				strResult += QString("<li class=\"leaf\"><input type=\"checkbox\" /><label>%1</label></li>\n")
										.arg(model.data(mdlIndexChild, Qt::DisplayRole).toString());
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

CKJVSearchCriteriaWidget::CKJVSearchCriteriaWidget(QWidget *parent) :
	QWidget(parent),
	m_pSearchWithinModel(NULL),
	m_bDoingUpdate(false)
{
	ui.setupUi(this);

	ui.comboSearchScope->addItem(tr("Anywhere in Selected Search Text (Unscoped)", "ScopeMenu"), CSearchCriteria::SSME_UNSCOPED);
	ui.comboSearchScope->addItem(tr("Together in Selected Search Text", "ScopeMenu"), CSearchCriteria::SSME_WHOLE_BIBLE);
	ui.comboSearchScope->addItem(tr("Same Testament", "ScopeMenu"), CSearchCriteria::SSME_TESTAMENT);
	ui.comboSearchScope->addItem(tr("Same Category", "ScopeMenu"), CSearchCriteria::SSME_CATEGORY);
	ui.comboSearchScope->addItem(tr("Same Book", "ScopeMenu"), CSearchCriteria::SSME_BOOK);
	ui.comboSearchScope->addItem(tr("Same Chapter", "ScopeMenu"), CSearchCriteria::SSME_CHAPTER);
	ui.comboSearchScope->addItem(tr("Same Verse", "ScopeMenu"), CSearchCriteria::SSME_VERSE);
	ui.comboSearchScope->setToolTip(tr("Select Search Scope", "MainMenu"));
	ui.comboSearchScope->setStatusTip(tr("Set Search Scope Mode for phrase searches", "MainMenu"));

	// Set Initial Mode:
	ui.comboSearchScope->setCurrentIndex(ui.comboSearchScope->findData(m_SearchCriteria.searchScopeMode()));

	connect(ui.comboSearchScope, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSearchScopeMode(int)));

	connect(ui.treeViewSearchWithin, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(en_SearchWithinItemActivated(const QModelIndex &)));

	// Setup Default TextBrightness:
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(setAdjustDialogElementBrightness(bool)));
}

CKJVSearchCriteriaWidget::~CKJVSearchCriteriaWidget()
{

}

void CKJVSearchCriteriaWidget::initialize(CBibleDatabasePtr pBibleDatabase)
{
	assert(!pBibleDatabase.isNull());
	m_pBibleDatabase = pBibleDatabase;

	begin_update();

	assert(m_pSearchWithinModel == NULL);		// Must be setting for the first time
	QAbstractItemModel *pOldModel = ui.treeViewSearchWithin->model();
	m_pSearchWithinModel = new CSearchWithinModel(m_pBibleDatabase, m_SearchCriteria, this);
	ui.treeViewSearchWithin->setModel(m_pSearchWithinModel);
	if (pOldModel) delete pOldModel;
	ui.treeViewSearchWithin->expandAll();
	ui.treeViewSearchWithin->resizeColumnToContents(0);
	ui.treeViewSearchWithin->collapseAll();

	for (int nRow = 0; nRow < m_pSearchWithinModel->rowCount(); ++nRow) {
		QModelIndex index = m_pSearchWithinModel->index(nRow);
		ui.treeViewSearchWithin->expand(index);
		for (int nRow2 = 0; nRow2 < m_pSearchWithinModel->rowCount(index); ++nRow2) {
			ui.treeViewSearchWithin->expand(m_pSearchWithinModel->index(nRow2, 0, index));
		}
	}

	connect(m_pSearchWithinModel, SIGNAL(changedSearchWithin()), this, SLOT(en_changedSearchWithin()));

	end_update();
}

void CKJVSearchCriteriaWidget::en_changedSearchScopeMode(int ndx)
{
	if (m_bDoingUpdate) return;

	begin_update();

	if (ndx == -1) return;
	m_SearchCriteria.setSearchScopeMode(static_cast<CSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(ui.comboSearchScope->itemData(ndx).toInt()));
	emit changedSearchCriteria();

	end_update();
}

void CKJVSearchCriteriaWidget::en_changedSearchWithin()
{
	assert(m_pSearchWithinModel != NULL);

	if (m_bDoingUpdate) return;

	begin_update();

	m_SearchCriteria.setSearchWithin(m_pSearchWithinModel->searchWithin());
	emit changedSearchCriteria();

	end_update();
}

void CKJVSearchCriteriaWidget::en_SearchWithinItemActivated(const QModelIndex &index)
{
	if (index.isValid()) {
		CRelIndex ndxReference = m_pSearchWithinModel->data(index, CSearchWithinModel::SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
		if ((ndxReference.isSet()) &&
			(ndxReference != CSearchCriteria::SSI_COLOPHON) &&
			(ndxReference != CSearchCriteria::SSI_SUPERSCRIPTION)) {
			emit gotoIndex(CVerseListModel::navigationIndexFromLogicalIndex(ndxReference));
		}
	}
}

void CKJVSearchCriteriaWidget::setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode)
{
	ui.comboSearchScope->setCurrentIndex(ui.comboSearchScope->findData(mode));
}

void CKJVSearchCriteriaWidget::setSearchWithin(const TRelativeIndexSet &aSetSearchWithin)
{
	begin_update();

	m_SearchCriteria.setSearchWithin(aSetSearchWithin);
	m_pSearchWithinModel->setSearchWithin(m_SearchCriteria.searchWithin());
	emit changedSearchCriteria();			// This is needed because the begin/end update will prevent us from firing it in the model en_changedSearchWithin callback

	end_update();
}

void CKJVSearchCriteriaWidget::setSearchWithin(const QString &strSearchWithin)
{
	begin_update();

	m_SearchCriteria.setSearchWithin(m_pBibleDatabase, strSearchWithin);
	m_pSearchWithinModel->setSearchWithin(m_SearchCriteria.searchWithin());
	emit changedSearchCriteria();			// This is needed because the begin/end update will prevent us from firing it in the model en_changedSearchWithin callback

	end_update();
}

void CKJVSearchCriteriaWidget::setTextBrightness(bool bInvert, int nBrightness)
{
	QColor clrBackground = CPersistentSettings::textBackgroundColor(bInvert, nBrightness);
	QColor clrForeground = CPersistentSettings::textForegroundColor(bInvert, nBrightness);

	// Note: This will automatically cause a repaint:
//	if (CPersistentSettings::instance()->adjustDialogElementBrightness()) {
		if (!bInvert) {
			setStyleSheet(QString("QTreeView { background-color:%1; color:%2; }\n"
									"QTreeView::indicator {\n"
									"    color: %2;\n"
									"    background-color: %1;\n"
									"    border: 1px solid %2;\n"
									"    width: 9px;\n"
									"    height: 9px;\n"
									"}\n"
									"QTreeView::indicator:checked {\n"
									"    image:url(:/res/checkbox2.png);\n"
									"}\n"
									"QTreeView::indicator:indeterminate {\n"
									"    image:url(:/res/checkbox.png);\n"
									"}\n"
								)
								.arg(clrBackground.name())
								.arg(clrForeground.name()));
		} else {
			setStyleSheet(QString("QTreeView { background-color:%1; color:%2; }\n"
									"QTreeView::indicator {\n"
									"    color: %2;\n"
									"    background-color: %1;\n"
									"    border: 1px solid %2;\n"
									"    width: 9px;\n"
									"    height: 9px;\n"
									"}\n"
									"QTreeView::indicator:checked {\n"
									"    image:url(:/res/checkbox.png);\n"
									"}\n"
									"QTreeView::indicator:indeterminate {\n"
									"    image:url(:/res/checkbox2.png);\n"
									"}\n"
								)
								.arg(clrBackground.name())
								.arg(clrForeground.name()));
		}
//	} else {
//		setStyleSheet(QString("QTreeView::indicator {\n"
//								"    border: 1px solid;\n"
//								"    width: 9px;\n"
//								"    height: 9px;\n"
//								"}\n"
//								"QTreeView::indicator:checked {\n"
//								"    image:url(:/res/checkbox2.png);\n"
//								"}\n"
//								"QTreeView::indicator:indeterminate {\n"
//								"    image:url(:/res/checkbox.png);\n"
//								"}\n"
//							  ));
//	}
}

void CKJVSearchCriteriaWidget::setAdjustDialogElementBrightness(bool bAdjust)
{
	Q_UNUSED(bAdjust);
//	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
}

// ============================================================================

