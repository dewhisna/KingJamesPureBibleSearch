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
#include "ui_KJVSearchCriteria.h"
#include "PersistentSettings.h"

#include "BusyCursor.h"

#include <QColor>

#include <assert.h>

// ============================================================================

CSearchWithinModel::CSearchWithinModel(CBibleDatabasePtr pBibleDatabase, QObject *pParent)
	:	QAbstractItemModel(pParent),
		m_pBibleDatabase(pBibleDatabase)
{
	assert(m_pBibleDatabase != NULL);

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
			pIndexCategory->insertIndex(CSearchCriteria::SSME_BOOK, nBk);
		}
	}
}

CSearchWithinModel::~CSearchWithinModel()
{

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
	assert(m_pBibleDatabase);

	if (!index.isValid()) return QVariant();

	const CSearchWithinModelIndex *pSearchWithinModelIndex = toSearchWithinModelIndex(index);
	assert(pSearchWithinModelIndex != NULL);
	if (pSearchWithinModelIndex == NULL) return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		uint32_t nItem = pSearchWithinModelIndex->itemIndex();
		switch (pSearchWithinModelIndex->ssme()) {
			case CSearchCriteria::SSME_WHOLE_BIBLE:
				return tr("Entire Bible");
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
				if (!pBookEntry->m_strDesc.isEmpty()) strBook += QString(" (%1)").arg(pBookEntry->m_strDesc);
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
	m_bDoingUpdate(false),
	ui(new Ui::CKJVSearchCriteriaWidget)
{
	ui->setupUi(this);

	ui->buttonAdd->setToolTip(tr("Add Phrase to Search Criteria"));
	ui->buttonAdd->setStatusTip(tr("Add another Phrase to the current Search Criteria"));

	ui->comboSearchScope->addItem(tr("Anywhere in Selected Search Text"), CSearchCriteria::SSME_WHOLE_BIBLE);
	ui->comboSearchScope->addItem(tr("Same Testament"), CSearchCriteria::SSME_TESTAMENT);
	ui->comboSearchScope->addItem(tr("Same Category"), CSearchCriteria::SSME_CATEGORY);
	ui->comboSearchScope->addItem(tr("Same Book"), CSearchCriteria::SSME_BOOK);
	ui->comboSearchScope->addItem(tr("Same Chapter"), CSearchCriteria::SSME_CHAPTER);
	ui->comboSearchScope->addItem(tr("Same Verse"), CSearchCriteria::SSME_VERSE);
	ui->comboSearchScope->setToolTip(tr("Select Search Scope"));
	ui->comboSearchScope->setStatusTip(tr("Set Search Scope Mode for phrase searches"));

	// Set Initial Mode:
	ui->comboSearchScope->setCurrentIndex(ui->comboSearchScope->findData(m_SearchCriteria.searchScopeMode()));

	connect(ui->comboSearchScope, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changeSearchScopeMode(int)));
	connect(ui->buttonAdd, SIGNAL(clicked()), this, SIGNAL(addSearchPhraseClicked()));
	connect(ui->buttonCopySummary, SIGNAL(clicked()), this, SIGNAL(copySearchPhraseSummary()));

	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(setAdjustDialogElementBrightness(bool)));
}

CKJVSearchCriteriaWidget::~CKJVSearchCriteriaWidget()
{
	delete ui;
}

void CKJVSearchCriteriaWidget::initialize(CBibleDatabasePtr pBibleDatabase)
{
	assert(pBibleDatabase != NULL);
	m_pBibleDatabase = pBibleDatabase;

	QAbstractItemModel *pOldModel = ui->treeViewSearchWithin->model();
	CSearchWithinModel *pModel = new CSearchWithinModel(m_pBibleDatabase, this);
	ui->treeViewSearchWithin->setModel(pModel);
	if (pOldModel) delete pOldModel;
	ui->treeViewSearchWithin->expandAll();
	ui->treeViewSearchWithin->resizeColumnToContents(0);
}

void CKJVSearchCriteriaWidget::en_changeSearchScopeMode(int ndx)
{
	if (m_bDoingUpdate) return;

	begin_update();

	if (ndx == -1) return;
	m_SearchCriteria.setSearchScopeMode(static_cast<CSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(ui->comboSearchScope->itemData(ndx).toInt()));
	emit changedSearchScopeMode(m_SearchCriteria.searchScopeMode());

	end_update();
}

void CKJVSearchCriteriaWidget::enableCopySearchPhraseSummary(bool bEnable)
{
	ui->buttonCopySummary->setEnabled(bEnable);
}

void CKJVSearchCriteriaWidget::setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode)
{
	ui->comboSearchScope->setCurrentIndex(ui->comboSearchScope->findData(mode));
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
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
}

// ============================================================================

