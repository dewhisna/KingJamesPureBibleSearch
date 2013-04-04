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

CKJVSearchCriteriaWidget::CKJVSearchCriteriaWidget(QWidget *parent) :
	QWidget(parent),
	m_bDoingUpdate(false),
	ui(new Ui::CKJVSearchCriteriaWidget)
{
	ui->setupUi(this);

	ui->buttonAdd->setToolTip(tr("Add Phrase to Search Criteria"));
	ui->buttonAdd->setStatusTip(tr("Add another Phrase to the current Search Criteria"));

	ui->comboSearchScope->addItem(tr("Entire Bible"), CSearchCriteria::SSME_WHOLE_BIBLE);
	ui->comboSearchScope->addItem(tr("Same Testament"), CSearchCriteria::SSME_TESTAMENT);
	ui->comboSearchScope->addItem(tr("Same Book"), CSearchCriteria::SSME_BOOK);
	ui->comboSearchScope->addItem(tr("Same Chapter"), CSearchCriteria::SSME_CHAPTER);
	ui->comboSearchScope->addItem(tr("Same Verse"), CSearchCriteria::SSME_VERSE);
	ui->comboSearchScope->setToolTip(tr("Select Search Scope"));
	ui->comboSearchScope->setStatusTip(tr("Set Search Scope Mode for phrase searches"));

	// Set Initial Mode:
	ui->comboSearchScope->setCurrentIndex(ui->comboSearchScope->findData(m_SearchCriteria.searchScopeMode()));

	connect(ui->comboSearchScope, SIGNAL(currentIndexChanged(int)), this, SLOT(on_changeSearchScopeMode(int)));
	connect(ui->buttonAdd, SIGNAL(clicked()), this, SIGNAL(addSearchPhraseClicked()));
	connect(ui->buttonCopySummary, SIGNAL(clicked()), this, SIGNAL(copySearchPhraseSummary()));
}

CKJVSearchCriteriaWidget::~CKJVSearchCriteriaWidget()
{
	delete ui;
}

void CKJVSearchCriteriaWidget::on_changeSearchScopeMode(int ndx)
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

