/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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

#include "BibleDatabaseInfoDlg.h"

// ============================================================================

CBibleDatabaseInfoDialog::CBibleDatabaseInfoDialog(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	assert(!pBibleDatabase.isNull());

	ui.setupUi(this);

#ifdef USE_ASYNC_DIALOGS
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_ShowModal, true);
#endif

	ui.lblBibleDatabaseName->setText(pBibleDatabase->description());
	ui.textBrowser->setHtml(pBibleDatabase->info());

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

CBibleDatabaseInfoDialog::~CBibleDatabaseInfoDialog()
{

}

// ============================================================================
