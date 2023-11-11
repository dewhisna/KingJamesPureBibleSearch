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

#include "SearchCriteriaWidget.h"

#include "PersistentSettings.h"
#include "BusyCursor.h"

#include <QColor>
#include <QAbstractItemModel>

// ============================================================================

CSearchCriteriaWidget::CSearchCriteriaWidget(QWidget *parent) :
	QWidget(parent),
	m_pSearchWithinModel(nullptr),
	m_bDoingUpdate(false)
{
	ui.setupUi(this);

	ui.comboSearchScope->addItem(tr("Anywhere in Selected Search Text (Unscoped)", "ScopeMenu"), CSearchCriteria::SSME_UNSCOPED);
	ui.comboSearchScope->addItem(tr("Together in Selected Search Text", "ScopeMenu"), CSearchCriteria::SSME_WHOLE_BIBLE);
	ui.comboSearchScope->addItem(tr("Same Testament", "ScopeMenu"), CSearchCriteria::SSME_TESTAMENT);
	ui.comboSearchScope->addItem(tr("Same Book", "ScopeMenu"), CSearchCriteria::SSME_BOOK);
	ui.comboSearchScope->addItem(tr("Same Chapter", "ScopeMenu"), CSearchCriteria::SSME_CHAPTER);
	ui.comboSearchScope->addItem(tr("Same Verse", "ScopeMenu"), CSearchCriteria::SSME_VERSE);
	ui.comboSearchScope->setToolTip(tr("Select Search Scope", "MainMenu"));
	ui.comboSearchScope->setStatusTip(tr("Set Search Scope Mode for phrase searches", "MainMenu"));

	// Set Initial Mode:
	ui.comboSearchScope->setCurrentIndex(ui.comboSearchScope->findData(m_SearchCriteria.searchScopeMode()));

	connect(ui.comboSearchScope, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSearchScopeMode(int)));

	connect(ui.treeViewSearchWithin, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(en_SearchWithinItemActivated(QModelIndex)));

	// Setup Default TextBrightness:
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool,int)), this, SLOT(setTextBrightness(bool,int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(setAdjustDialogElementBrightness(bool)));
}

CSearchCriteriaWidget::~CSearchCriteriaWidget()
{

}

void CSearchCriteriaWidget::initialize(CBibleDatabasePtr pBibleDatabase)
{
	Q_ASSERT(!pBibleDatabase.isNull());
	m_pBibleDatabase = pBibleDatabase;

	begin_update();

	Q_ASSERT(m_pSearchWithinModel == nullptr);		// Must be setting for the first time
	QAbstractItemModel *pOldModel = ui.treeViewSearchWithin->model();
	m_pSearchWithinModel = new CSearchWithinModel(m_pBibleDatabase, m_SearchCriteria, this);
	ui.treeViewSearchWithin->setModel(m_pSearchWithinModel);
	if (pOldModel) delete pOldModel;

	expandTreeView();

	connect(m_pSearchWithinModel, SIGNAL(changedSearchWithin()), this, SLOT(en_changedSearchWithin()));
	connect(m_pSearchWithinModel, SIGNAL(modelReset()), this, SLOT(en_modelReset()));

	end_update();
}

void CSearchCriteriaWidget::expandTreeView()
{
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
}

void CSearchCriteriaWidget::en_changedSearchScopeMode(int ndx)
{
	if (m_bDoingUpdate) return;

	begin_update();

	if (ndx == -1) return;
	m_SearchCriteria.setSearchScopeMode(static_cast<CSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(ui.comboSearchScope->itemData(ndx).toInt()));
	emit changedSearchCriteria();

	end_update();
}

void CSearchCriteriaWidget::en_changedSearchWithin()
{
	Q_ASSERT(m_pSearchWithinModel != nullptr);

	if (m_bDoingUpdate) return;

	begin_update();

	m_SearchCriteria.setSearchWithin(m_pSearchWithinModel->searchWithin());
	emit changedSearchCriteria();

	end_update();
}

void CSearchCriteriaWidget::en_SearchWithinItemActivated(const QModelIndex &index)
{
	if (index.isValid()) {
		CRelIndex ndxReference = m_pSearchWithinModel->data(index, CSearchWithinModel::SWMDRE_REL_INDEX_ROLE).value<CRelIndex>();
		if ((ndxReference.isSet()) &&
			(ndxReference != CSearchCriteria::SSI_COLOPHON) &&
			(ndxReference != CSearchCriteria::SSI_SUPERSCRIPTION)) {
			emit gotoIndex(CRelIndex::navigationIndexFromLogicalIndex(ndxReference));
		}
	}
}

void CSearchCriteriaWidget::en_modelReset()
{
	expandTreeView();
	emit changedSearchCriteria();
}

void CSearchCriteriaWidget::setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode)
{
	ui.comboSearchScope->setCurrentIndex(ui.comboSearchScope->findData(mode));
}

void CSearchCriteriaWidget::setSearchWithin(const TRelativeIndexSet &aSetSearchWithin)
{
	begin_update();

	m_SearchCriteria.setSearchWithin(aSetSearchWithin);
	m_pSearchWithinModel->setSearchWithin(m_SearchCriteria.searchWithin());
	emit changedSearchCriteria();			// This is needed because the begin/end update will prevent us from firing it in the model en_changedSearchWithin callback

	end_update();
}

void CSearchCriteriaWidget::setSearchWithin(const QString &strSearchWithin)
{
	begin_update();

	m_SearchCriteria.setSearchWithin(m_pBibleDatabase, strSearchWithin);
	m_pSearchWithinModel->setSearchWithin(m_SearchCriteria.searchWithin());
	emit changedSearchCriteria();			// This is needed because the begin/end update will prevent us from firing it in the model en_changedSearchWithin callback

	end_update();
}

void CSearchCriteriaWidget::setTextBrightness(bool bInvert, int nBrightness)
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

void CSearchCriteriaWidget::setAdjustDialogElementBrightness(bool bAdjust)
{
	Q_UNUSED(bAdjust);
//	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
}

// ============================================================================
