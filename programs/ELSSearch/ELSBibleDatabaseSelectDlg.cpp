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

#include "ELSBibleDatabaseSelectDlg.h"
#include "ui_ELSBibleDatabaseSelectDlg.h"

#include "../KJVCanOpener/dbstruct.h"

// ============================================================================

CELSBibleDatabaseSelectDlg::CELSBibleDatabaseSelectDlg(const QString &strBibleUUID,
														bool bRemoveColophons,
														bool bRemoveSuperscriptions,
														QWidget *parent)
	:	QDialog(parent),
		ui(new Ui::CELSBibleDatabaseSelectDlg),
		m_strBibleUUID(strBibleUUID),
		m_bRemoveColophons(bRemoveColophons),
		m_bRemoveSuperscriptions(bRemoveSuperscriptions)
{
	ui->setupUi(this);

	// --------------------------------

	int nSelected = -1;
	const QList<TBibleDescriptor> &lstAvailableBBLDescs = TBibleDatabaseList::availableBibleDatabases();
	for (int ndx = 0; ndx < lstAvailableBBLDescs.size(); ++ndx) {
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(lstAvailableBBLDescs.at(ndx).m_strUUID);

		if (!pBibleDatabase.isNull()) {
			ui->cmbBible->addItem(pBibleDatabase->description(), pBibleDatabase->compatibilityUUID());
			if (pBibleDatabase->compatibilityUUID().compare(m_strBibleUUID, Qt::CaseInsensitive) == 0) nSelected = ndx;
		} else {
			Q_ASSERT(lstAvailableBBLDescs.at(ndx).isValid());
			ui->cmbBible->addItem(lstAvailableBBLDescs.at(ndx).m_strDBDesc, lstAvailableBBLDescs.at(ndx).m_strUUID);
			if (lstAvailableBBLDescs.at(ndx).m_strUUID.compare(m_strBibleUUID, Qt::CaseInsensitive) == 0) nSelected = ndx;
		}
	}
	if (nSelected >= 0) ui->cmbBible->setCurrentIndex(nSelected);

	ui->chkRemoveColophons->setChecked(m_bRemoveColophons);
	ui->chkRemoveSuperscriptions->setChecked(m_bRemoveSuperscriptions);

	// ---------------------------------

	connect(ui->cmbBible, SIGNAL(currentIndexChanged(int)), this, SLOT(en_selectionChanged(int)));
	connect(ui->chkRemoveColophons, SIGNAL(toggled(bool)), this, SLOT(setRemoveColophons(bool)));
	connect(ui->chkRemoveSuperscriptions, SIGNAL(toggled(bool)), this, SLOT(setRemoveSuperscriptions(bool)));
}

CELSBibleDatabaseSelectDlg::~CELSBibleDatabaseSelectDlg()
{
	delete ui;
}

void CELSBibleDatabaseSelectDlg::en_selectionChanged(int nIndex)
{
	setBibleUUID(ui->cmbBible->itemData(nIndex).toString());
}

// ============================================================================

