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

#include <QGridLayout>

#include <QTimer>
#include <assert.h>

CKJVPassageNavigatorDlg::CKJVPassageNavigatorDlg(CBibleDatabasePtr pBibleDatabase,
												 QWidget *parent,
												 CKJVPassageNavigator::NavigatorRefTypeOptionFlags flagsRefTypes,
												 CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nRefType)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		m_pBibleDatabase(pBibleDatabase),
		m_pApplyButton(NULL),
		m_pModeButton(NULL),
		m_pResetButton(NULL),
		m_pOKButton(NULL),
		m_pCancelButton(NULL),
		m_pNavigator(NULL)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui.setupUi(this);

#ifdef USE_ASYNC_DIALOGS
	setAttribute(Qt::WA_DeleteOnClose);
#endif

	// --------------------------------------------------------------

	//	Swapout the widgetKJVPassageNavigator from the layout with
	//		one that we can set the database on:

	int ndx = ui.gridLayout->indexOf(ui.widgetKJVPassageNavigator);
	assert(ndx != -1);
	if (ndx == -1) return;
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;
	ui.gridLayout->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pNavigator = new CKJVPassageNavigator(pBibleDatabase, this, flagsRefTypes, nRefType);
	m_pNavigator->setObjectName(QString::fromUtf8("widgetKJVPassageNavigator"));
	delete ui.widgetKJVPassageNavigator;
	ui.widgetKJVPassageNavigator = NULL;
	ui.gridLayout->addWidget(m_pNavigator, nRow, nCol, nRowSpan, nColSpan);

	// --------------------------------------------------------------

	assert(m_pNavigator != NULL);

	m_pApplyButton = ui.buttonBox->addButton(tr("&Apply Resolved to From Location", "CKJVPassageNavigatorDlg"), QDialogButtonBox::ApplyRole);
	connect(m_pApplyButton, SIGNAL(clicked()), this, SLOT(en_ApplyResolvedClicked()));

	m_pModeButton = ui.buttonBox->addButton(tr("&Switch Mode", "CKJVPassageNavigatorDlg"), QDialogButtonBox::ActionRole);
	connect(m_pModeButton, SIGNAL(clicked()), this, SLOT(en_ModeClicked()));

	m_pResetButton = ui.buttonBox->addButton(tr("&Reset", "CKJVPassageNavigatorDlg"), QDialogButtonBox::ResetRole);
	connect(m_pResetButton, SIGNAL(clicked()), m_pNavigator, SLOT(reset()));

	m_pOKButton = ui.buttonBox->addButton(tr("&Goto", "CKJVPassageNavigatorDlg"), QDialogButtonBox::AcceptRole);

	m_pCancelButton = ui.buttonBox->addButton(tr("&Cancel", "CKJVPassageNavigatorDlg"), QDialogButtonBox::RejectRole);

	// Setup initial mode to match widget:
	en_modeChanged(m_pNavigator->isRelative());

	connect(m_pNavigator, SIGNAL(modeChanged(bool)), this, SLOT(en_modeChanged(bool)));
	connect(m_pNavigator, SIGNAL(gotoIndex(const TPhraseTag &)), this, SLOT(en_gotoIndex(const TPhraseTag &)));

	// --------------------------------------------------------------

#ifndef Q_OS_MAC
	setWindowModality(Qt::WindowModal);		// Only block our parentCanOpener, not the whole app
#endif
}

CKJVPassageNavigatorDlg::~CKJVPassageNavigatorDlg()
{

}

void CKJVPassageNavigatorDlg::setGotoButtonText(const QString &strText)
{
	m_pOKButton->setText(strText);
}

TPhraseTag CKJVPassageNavigatorDlg::passage() const
{
	return m_pNavigator->passage();
}

void CKJVPassageNavigatorDlg::setPassage(const TPhraseTag &tag)
{
	m_pNavigator->setPassage(tag);
}

CKJVPassageNavigator &CKJVPassageNavigatorDlg::navigator()
{
	return *(m_pNavigator);
}

CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM CKJVPassageNavigatorDlg::refType() const
{
	return m_pNavigator->refType();
}

void CKJVPassageNavigatorDlg::setRefType(CKJVPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nRefType)
{
	m_pNavigator->setRefType(nRefType);
}

void CKJVPassageNavigatorDlg::en_modeChanged(bool bRelative)
{
	if (bRelative) {
		setWindowTitle(tr("Passage Navigator - Relative Mode", "CKJVPassageNavigatorDlg"));
		m_pApplyButton->show();
		m_pModeButton->setText(tr("&Switch to Absolute Mode", "CKJVPassageNavigatorDlg"));
	} else {
		setWindowTitle(tr("Passage Navigator - Absolute Mode", "CKJVPassageNavigatorDlg"));
		m_pApplyButton->hide();
		m_pModeButton->setText(tr("&Switch to Relative Mode", "CKJVPassageNavigatorDlg"));
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
	QTimer::singleShot(0, this, SLOT(en_resizeMe()));
}

void CKJVPassageNavigatorDlg::en_ApplyResolvedClicked()
{
	// Reversing and swapping passage and startRef are symmetric:
	m_pNavigator->startRelativeMode(m_pNavigator->passage(), m_pNavigator->isReversed());
}

void CKJVPassageNavigatorDlg::en_ModeClicked()
{
	if (m_pNavigator->isAbsolute()) {
		m_pNavigator->startRelativeMode(m_pNavigator->passage());
		m_pNavigator->reset();
	} else {
		m_pNavigator->startAbsoluteMode(m_pNavigator->passage());
	}
}

void CKJVPassageNavigatorDlg::en_gotoIndex(const TPhraseTag &tag)
{
	// Easiest way to simulate this is to apply the click-navigated passage as an
	//		absolute reference and accept it.  Otherwise, if we just setPassage,
	//		then if we're in relative mode, we'll move by the relative offset
	//		instead of navigating to it:
	m_pNavigator->startAbsoluteMode(tag);
	accept();
}

void CKJVPassageNavigatorDlg::accept()
{
	emit gotoIndex(passage());
	QDialog::accept();
}

void CKJVPassageNavigatorDlg::en_resizeMe()
{
	adjustSize();
}

