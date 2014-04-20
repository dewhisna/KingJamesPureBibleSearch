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

#include "BusyCursor.h"

#include <QColor>
#include <QTextDocument>

#include <assert.h>

// ============================================================================

QString CSearchCriteria::searchWithinDescription(CBibleDatabasePtr pBibleDatabase) const
{
	CSearchWithinModel modelSearchWithin(pBibleDatabase, m_setSearchWithin);
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

CSearchWithinModel::CSearchWithinModel(CBibleDatabasePtr pBibleDatabase, const TRelativeIndexSet &aSetSearchWithin, QObject *pParent)
	:	QAbstractItemModel(pParent),
		m_pBibleDatabase(pBibleDatabase)
{
	assert(m_pBibleDatabase.data() != NULL);

	// Build our model data (which should be static with the given Bible Database):
	// The root node is the "Whole Bible"
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

	for (CModelRowForwardIterator fwdItr(this); fwdItr; /* Increment inside loop */) {
		const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(*fwdItr);
		assert(pSearchWithinModelIndex != NULL);
		// Fully checked items completely define it, so use it -- except for Category,
		//	since they are subjective, which will translate to the child names:
		if ((pSearchWithinModelIndex->checkState() == Qt::Checked) && (pSearchWithinModelIndex->ssme() != CSearchCriteria::SSME_CATEGORY)) {
			lstDescription.append(fwdItr->data(Qt::EditRole).toString());
			fwdItr.nextSibling();
		} else {
			++fwdItr;
		}
	}

	return lstDescription.join(QString(", "));
}

TRelativeIndexSet CSearchWithinModel::searchWithin() const
{
	TRelativeIndexSet setIndexes;

	bool bSelectAll = true;
	for (CModelRowForwardIterator fwdItr(this); fwdItr; ++fwdItr) {
		const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(*fwdItr);
		assert(pSearchWithinModelIndex != NULL);
		if ((pSearchWithinModelIndex->childIndexCount() == 0) && (pSearchWithinModelIndex->checkState() == Qt::Checked)) {
			setIndexes.insert(CRelIndex(pSearchWithinModelIndex->itemIndex(), 0, 0, 0));
		} else {
			bSelectAll = false;
		}
	}
	if (bSelectAll) setIndexes.clear();			// An empty list is a special case for select all

	return setIndexes;
}

void CSearchWithinModel::setSearchWithin(const TRelativeIndexSet &aSetSearchWithin)
{
	for (CModelRowForwardIterator fwdItr(this); fwdItr; ++fwdItr) {
		const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(*fwdItr);
		assert(pSearchWithinModelIndex != NULL);
		if (pSearchWithinModelIndex->childIndexCount() == 0) {
			setData(*fwdItr, ((aSetSearchWithin.find(CRelIndex(pSearchWithinModelIndex->itemIndex(), 0, 0, 0)) != aSetSearchWithin.end()) ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
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
	assert(m_pBibleDatabase.data() != NULL);

	if (!index.isValid()) return QVariant();

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	assert(pSearchWithinModelIndex != NULL);
	if (pSearchWithinModelIndex == NULL) return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		uint32_t nItem = pSearchWithinModelIndex->itemIndex();
		switch (pSearchWithinModelIndex->ssme()) {
			case CSearchCriteria::SSME_WHOLE_BIBLE:
			{
				// Search for "Entire Bible".  First try and see if we can translate it in the language of the selected Bible,
				//		but if not, try in the current language setting
				QString strEntireBible = tr("Entire Bible", "Scope");
				TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pBibleDatabase->language());
				if (pTranslator.data() != NULL) {
					QString strTemp = pTranslator->translator().translate("CSearchWithinModel", "Entire Bible", "Scope");
					if (!strTemp.isEmpty()) strEntireBible = strTemp;
				}
				return strEntireBible;
			}
			case CSearchCriteria::SSME_TESTAMENT:
				assert(m_pBibleDatabase->testamentEntry(nItem) != NULL);
				return m_pBibleDatabase->testamentEntry(nItem)->m_strTstName;
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

	if (role == Qt::CheckStateRole) {
		return pSearchWithinModelIndex->checkState();
	}

	if (role == SWMDRE_REL_INDEX_ROLE) {
		uint32_t nItem = pSearchWithinModelIndex->itemIndex();
		switch (pSearchWithinModelIndex->ssme()) {
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
		return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	assert(pSearchWithinModelIndex != NULL);
	if (pSearchWithinModelIndex == NULL) return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

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

CKJVSearchCriteriaWidget::CKJVSearchCriteriaWidget(QWidget *parent) :
	QWidget(parent),
	m_pSearchWithinModel(NULL),
	m_bDoingUpdate(false)
{
	ui.setupUi(this);

	ui.buttonAdd->setToolTip(tr("Add Phrase to Search Criteria", "MainMenu"));
	ui.buttonAdd->setStatusTip(tr("Add another Phrase to the current Search Criteria", "MainMenu"));

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
	connect(ui.buttonAdd, SIGNAL(clicked()), this, SIGNAL(addSearchPhraseClicked()));
	connect(ui.buttonCopySummary, SIGNAL(clicked()), this, SIGNAL(copySearchPhraseSummary()));

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
	assert(pBibleDatabase.data() != NULL);
	m_pBibleDatabase = pBibleDatabase;

	begin_update();

	assert(m_pSearchWithinModel == NULL);		// Must be setting for the first time
	QAbstractItemModel *pOldModel = ui.treeViewSearchWithin->model();
	m_pSearchWithinModel = new CSearchWithinModel(m_pBibleDatabase, m_SearchCriteria.searchWithin(), this);
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
		if (ndxReference.isSet()) {
			emit gotoIndex(ndxReference);
		}
	}
}

void CKJVSearchCriteriaWidget::enableCopySearchPhraseSummary(bool bEnable)
{
	ui.buttonCopySummary->setEnabled(bEnable);
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

