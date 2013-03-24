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

#include "KJVPassageNavigatorDlg.h"
#include "ui_KJVPassageNavigatorDlg.h"

#include <QTimer>
#include <assert.h>

CKJVPassageNavigatorDlg::CKJVPassageNavigatorDlg(CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
	m_pApplyButton(NULL),
	m_pModeButton(NULL),
	m_pResetButton(NULL),
	m_pOKButton(NULL),
	m_pCancelButton(NULL),
	ui(new Ui::CKJVPassageNavigatorDlg)
{
	ui->setupUi(this);

	assert(pBibleDatabase.data() != NULL);
	ui->widgetKJVPassageNavigator->initialize(pBibleDatabase);

	m_pApplyButton = ui->buttonBox->addButton("&Apply Resolved to From Location", QDialogButtonBox::ApplyRole);
	connect(m_pApplyButton, SIGNAL(clicked()), this, SLOT(on_ApplyResolvedClicked()));

	m_pModeButton = ui->buttonBox->addButton("&Switch Mode", QDialogButtonBox::ActionRole);
	connect(m_pModeButton, SIGNAL(clicked()), this, SLOT(on_ModeClicked()));

	m_pResetButton = ui->buttonBox->addButton("&Reset", QDialogButtonBox::ResetRole);
	connect(m_pResetButton, SIGNAL(clicked()), ui->widgetKJVPassageNavigator, SLOT(reset()));

	m_pOKButton = ui->buttonBox->addButton("&Goto", QDialogButtonBox::AcceptRole);

	m_pCancelButton = ui->buttonBox->addButton("&Cancel", QDialogButtonBox::RejectRole);

	// Setup initial mode to match widget:
	on_modeChanged(ui->widgetKJVPassageNavigator->isRelative());

	connect(ui->widgetKJVPassageNavigator, SIGNAL(modeChanged(bool)), this, SLOT(on_modeChanged(bool)));
	connect(ui->widgetKJVPassageNavigator, SIGNAL(gotoIndex(const TPhraseTag &)), this, SLOT(on_gotoIndex(const TPhraseTag &)));
}

CKJVPassageNavigatorDlg::~CKJVPassageNavigatorDlg()
{
	delete ui;
}

TPhraseTag CKJVPassageNavigatorDlg::passage() const
{
	return ui->widgetKJVPassageNavigator->passage();
}

void CKJVPassageNavigatorDlg::setPassage(const TPhraseTag &tag)
{
	ui->widgetKJVPassageNavigator->setPassage(tag);
}

CKJVPassageNavigator &CKJVPassageNavigatorDlg::navigator()
{
	return *(ui->widgetKJVPassageNavigator);
}

void CKJVPassageNavigatorDlg::on_modeChanged(bool bRelative)
{
	if (bRelative) {
		setWindowTitle("Passage Navigator - Relative Mode");
		m_pApplyButton->show();
		m_pModeButton->setText("&Switch to Absolute Mode");
	} else {
		setWindowTitle("Passage Navigator - Absolute Mode");
		m_pApplyButton->hide();
		m_pModeButton->setText("&Switch to Relative Mode");
	}
	// It's sometimes confusing knowing which mode you are
	//	in, so we'll resize back to our minimum size each
	//	time so they'll look distinctively different to the
	//	user:
	//
	// Note:  The minimumSizeHint isn't computed until the
	//	event loop runs, so just calling adjustSize here has
	//	no effect.  So, we'll setup a dummy timer and
	//	trigger it later in the event stack:
	QTimer::singleShot(0, this, SLOT(on_resizeMe()));
}

void CKJVPassageNavigatorDlg::on_ApplyResolvedClicked()
{
	// Reversing and swapping passage and startRef are symmetric:
	ui->widgetKJVPassageNavigator->startRelativeMode(ui->widgetKJVPassageNavigator->passage(), ui->widgetKJVPassageNavigator->isReversed());
}

void CKJVPassageNavigatorDlg::on_ModeClicked()
{
	if (ui->widgetKJVPassageNavigator->isAbsolute()) {
		ui->widgetKJVPassageNavigator->startRelativeMode(ui->widgetKJVPassageNavigator->passage());
		ui->widgetKJVPassageNavigator->reset();
	} else {
		ui->widgetKJVPassageNavigator->startAbsoluteMode(ui->widgetKJVPassageNavigator->passage());
	}
}

void CKJVPassageNavigatorDlg::on_gotoIndex(const TPhraseTag &tag)
{
	// Easiest way to simulate this is to apply the click-navigated passage as an
	//		absolute reference and accept it.  Otherwise, if we just setPassage,
	//		then if we're in relative mode, we'll move by the relative offset
	//		instead of navigating to it:
	ui->widgetKJVPassageNavigator->startAbsoluteMode(tag);
	accept();
}

void CKJVPassageNavigatorDlg::on_resizeMe()
{
	adjustSize();
}

