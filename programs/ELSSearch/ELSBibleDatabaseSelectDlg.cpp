/****************************************************************************
**
** Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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
													   LetterMatrixTextModifierOptionFlags flagsLMTMO,
													   LMBookPrologueOptionFlags flagsLMBPO,
													   LMChapterPrologueOptionFlags flagsLMCPO,
													   LMVersePrologueOptionFlags flagsLMVPO,
													   QWidget *parent)
	:	QDialog(parent),
		ui(new Ui::CELSBibleDatabaseSelectDlg),
		m_strBibleUUID(strBibleUUID),
		m_flagsLMTMO(flagsLMTMO),
		m_flagsLMBPO(flagsLMBPO),
		m_flagsLMCPO(flagsLMCPO),
		m_flagsLMVPO(flagsLMVPO)
{
	ui->setupUi(this);

// TODO : Finish UI for BPO/CPO/VPO

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

	if (m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly))
		m_flagsLMTMO = LMTMO_WordsOfJesusOnly | LMTMO_RemoveColophons | LMTMO_RemoveSuperscriptions;	// No include book/chapter prologues

	ui->chkWordsOfJesusOnly->setChecked(m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkRemoveColophons->setChecked(m_flagsLMTMO.testFlag(LMTMO_RemoveColophons));
	ui->chkRemoveSuperscriptions->setChecked(m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions));
	ui->chkIncludeBookPrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues));
	ui->chkIncludeChapterPrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));

	ui->chkRemoveColophons->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkRemoveSuperscriptions->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeBookPrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeChapterPrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));

	// ---------------------------------

	connect(ui->cmbBible, SIGNAL(currentIndexChanged(int)), this, SLOT(en_selectionChanged(int)));
	connect(ui->chkWordsOfJesusOnly, &QCheckBox::toggled, this, [this](bool bWordsOfJesusOnly)->void {
		m_flagsLMTMO.setFlag(LMTMO_WordsOfJesusOnly, bWordsOfJesusOnly);
		if (bWordsOfJesusOnly) {
			if (!ui->chkRemoveColophons->isChecked()) ui->chkRemoveColophons->setChecked(true);
			if (!ui->chkRemoveSuperscriptions->isChecked()) ui->chkRemoveSuperscriptions->setChecked(true);
			if (ui->chkIncludeBookPrologues->isChecked()) ui->chkIncludeBookPrologues->setChecked(false);
			if (ui->chkIncludeChapterPrologues->isChecked()) ui->chkIncludeChapterPrologues->setChecked(false);

			ui->chkRemoveColophons->setEnabled(false);
			ui->chkRemoveSuperscriptions->setEnabled(false);
			ui->chkIncludeBookPrologues->setEnabled(false);
			ui->chkIncludeChapterPrologues->setEnabled(false);
		} else {
			ui->chkRemoveColophons->setEnabled(true);
			ui->chkRemoveSuperscriptions->setEnabled(true);
			ui->chkIncludeBookPrologues->setEnabled(true);
			ui->chkIncludeChapterPrologues->setEnabled(true);
		}
	});
	connect(ui->chkRemoveColophons, &QCheckBox::toggled, this, [this](bool bRemoveColophons)->void {
		m_flagsLMTMO.setFlag(LMTMO_RemoveColophons, bRemoveColophons);
	});
	connect(ui->chkRemoveSuperscriptions, &QCheckBox::toggled, this, [this](bool bRemoveSuperscriptions)->void {
		m_flagsLMTMO.setFlag(LMTMO_RemoveSuperscriptions, bRemoveSuperscriptions);
	});
	connect(ui->chkIncludeBookPrologues, &QCheckBox::toggled, this, [this](bool bIncludeBookPrologues)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludeBookPrologues, bIncludeBookPrologues);
	});
	connect(ui->chkIncludeChapterPrologues, &QCheckBox::toggled, this, [this](bool bIncludeChapterPrologues)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludeChapterPrologues, bIncludeChapterPrologues);
	});
}

CELSBibleDatabaseSelectDlg::~CELSBibleDatabaseSelectDlg()
{
	delete ui;
}

void CELSBibleDatabaseSelectDlg::en_selectionChanged(int nIndex)
{
	m_strBibleUUID = ui->cmbBible->itemData(nIndex).toString();
}

// ============================================================================

