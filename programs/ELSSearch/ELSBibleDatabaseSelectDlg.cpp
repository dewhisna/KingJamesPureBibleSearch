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

	ui->cmbCPONumbers->addItem(tr("None"), LMCPO_NumbersNone);
	ui->cmbCPONumbers->addItem(tr("Roman"), LMCPO_NumbersRoman);
	ui->cmbCPONumbers->addItem(tr("Arabic"), LMCPO_NumbersArabic);
	ui->cmbCPONumbers->setCurrentIndex(ui->cmbCPONumbers->findData(QVariant::fromValue(static_cast<unsigned int>(m_flagsLMCPO & LMCPO_NumberOptionsMask))));

	ui->chkCPOPsalmBooks->setChecked(m_flagsLMCPO.testFlag(LMCPO_PsalmBookTags));
	ui->cmbCPOPsalmBookNumbers->addItem(tr("None"), LMCPO_PsalmBookNumbersNone);
	ui->cmbCPOPsalmBookNumbers->addItem(tr("Roman"), LMCPO_PsalmBookNumbersRoman);
	ui->cmbCPOPsalmBookNumbers->addItem(tr("Arabic"), LMCPO_PsalmBookNumbersArabic);
	ui->cmbCPOPsalmBookNumbers->setCurrentIndex(ui->cmbCPOPsalmBookNumbers->findData(QVariant::fromValue(static_cast<unsigned int>(m_flagsLMCPO & LMCPO_PsalmBookNumberOptionsMask))));

	ui->cmbVPONumbers->addItem(tr("None"), LMVPO_NumbersNone);
	ui->cmbVPONumbers->addItem(tr("Roman"), LMVPO_NumbersRoman);
	ui->cmbVPONumbers->addItem(tr("Arabic"), LMVPO_NumbersArabic);
	ui->cmbVPONumbers->setCurrentIndex(ui->cmbVPONumbers->findData(QVariant::fromValue(static_cast<unsigned int>(m_flagsLMVPO & LMVPO_NumberOptionsMask))));

	ui->chkWordsOfJesusOnly->setChecked(m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkRemoveColophons->setChecked(m_flagsLMTMO.testFlag(LMTMO_RemoveColophons));
	ui->chkRemoveSuperscriptions->setChecked(m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions));
	ui->chkIncludeBookPrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues));
	ui->chkIncludeChapterPrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->chkIncludeVersePrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->chkIncludePunctuation->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation));
	ui->chkIncludeSpaces->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeSpaces));

	ui->chkRemoveColophons->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkRemoveSuperscriptions->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeBookPrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeChapterPrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeVersePrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludePunctuation->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeSpaces->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));

	ui->lblCPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->cmbCPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->chkCPOPsalmBooks->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->cmbCPOPsalmBookNumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));

	ui->lblVPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->cmbVPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));

	// ---------------------------------

	connect(ui->cmbBible, SIGNAL(currentIndexChanged(int)), this, SLOT(en_BibleSelectionChanged(int)));
	connect(ui->chkWordsOfJesusOnly, &QCheckBox::toggled, this, [this](bool bWordsOfJesusOnly)->void {
		m_flagsLMTMO.setFlag(LMTMO_WordsOfJesusOnly, bWordsOfJesusOnly);
		if (bWordsOfJesusOnly) {
			if (!ui->chkRemoveColophons->isChecked()) ui->chkRemoveColophons->setChecked(true);
			if (!ui->chkRemoveSuperscriptions->isChecked()) ui->chkRemoveSuperscriptions->setChecked(true);
			if (ui->chkIncludeBookPrologues->isChecked()) ui->chkIncludeBookPrologues->setChecked(false);
			if (ui->chkIncludeChapterPrologues->isChecked()) ui->chkIncludeChapterPrologues->setChecked(false);
			if (ui->chkIncludeVersePrologues->isChecked()) ui->chkIncludeVersePrologues->setChecked(false);
			if (ui->chkIncludePunctuation->isChecked()) ui->chkIncludePunctuation->setChecked(false);
			if (ui->chkIncludeSpaces->isChecked()) ui->chkIncludeSpaces->setChecked(false);

			ui->chkRemoveColophons->setEnabled(false);
			ui->chkRemoveSuperscriptions->setEnabled(false);
			ui->chkIncludeBookPrologues->setEnabled(false);
			ui->chkIncludeChapterPrologues->setEnabled(false);
			ui->chkIncludeVersePrologues->setEnabled(false);
			ui->chkIncludePunctuation->setEnabled(false);
			ui->chkIncludeSpaces->setEnabled(false);

			ui->lblCPONumbers->setEnabled(false);
			ui->cmbCPONumbers->setEnabled(false);
			ui->chkCPOPsalmBooks->setEnabled(false);
			ui->cmbCPOPsalmBookNumbers->setEnabled(false);

			ui->lblVPONumbers->setEnabled(false);
			ui->cmbVPONumbers->setEnabled(false);
		} else {
			ui->chkRemoveColophons->setEnabled(true);
			ui->chkRemoveSuperscriptions->setEnabled(true);
			ui->chkIncludeBookPrologues->setEnabled(true);
			ui->chkIncludeChapterPrologues->setEnabled(true);
			ui->chkIncludeVersePrologues->setEnabled(true);
			ui->chkIncludePunctuation->setEnabled(true);
			ui->chkIncludeSpaces->setEnabled(true);

			ui->lblCPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
			ui->cmbCPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
			ui->chkCPOPsalmBooks->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
			ui->cmbCPOPsalmBookNumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));

			ui->lblVPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
			ui->cmbVPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
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

		ui->lblCPONumbers->setEnabled(bIncludeChapterPrologues);
		ui->cmbCPONumbers->setEnabled(bIncludeChapterPrologues);
		ui->chkCPOPsalmBooks->setEnabled(bIncludeChapterPrologues);
		ui->cmbCPOPsalmBookNumbers->setEnabled(bIncludeChapterPrologues);
	});
	connect(ui->chkIncludeVersePrologues, &QCheckBox::toggled, this, [this](bool bIncludeVersePrologues)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludeVersePrologues, bIncludeVersePrologues);

		ui->lblVPONumbers->setEnabled(bIncludeVersePrologues);
		ui->cmbVPONumbers->setEnabled(bIncludeVersePrologues);
	});
	connect(ui->chkIncludePunctuation, &QCheckBox::toggled, this, [this](bool bIncludePunctuation)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludePunctuation, bIncludePunctuation);
	});
	connect(ui->chkIncludeSpaces, &QCheckBox::toggled, this, [this](bool bIncludeSpaces)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludeSpaces, bIncludeSpaces);
	});

	connect(ui->cmbCPONumbers, SIGNAL(currentIndexChanged(int)), this, SLOT(en_CPONumberSelectionChanged(int)));
	connect(ui->chkCPOPsalmBooks, &QCheckBox::toggled, this, [this](bool bCPOPsalmBooks)->void {
		m_flagsLMCPO.setFlag(LMCPO_PsalmBookTags, bCPOPsalmBooks);
	});
	connect(ui->cmbCPOPsalmBookNumbers, SIGNAL(currentIndexChanged(int)), this, SLOT(en_CPOPsalmBookNumberSelectionChanged(int)));

	connect(ui->cmbVPONumbers, SIGNAL(currentIndexChanged(int)), this, SLOT(en_VPONumberSelectionChanged(int)));
}

CELSBibleDatabaseSelectDlg::~CELSBibleDatabaseSelectDlg()
{
	delete ui;
}

void CELSBibleDatabaseSelectDlg::en_BibleSelectionChanged(int nIndex)
{
	m_strBibleUUID = ui->cmbBible->itemData(nIndex).toString();
}

void CELSBibleDatabaseSelectDlg::en_CPONumberSelectionChanged(int nIndex)
{
	m_flagsLMCPO &= ~LMCPO_NumberOptionsMask;
	m_flagsLMCPO |= static_cast<LMChapterPrologueOptions>(ui->cmbCPONumbers->itemData(nIndex).toUInt());
}

void CELSBibleDatabaseSelectDlg::en_CPOPsalmBookNumberSelectionChanged(int nIndex)
{
	m_flagsLMCPO &= ~LMCPO_PsalmBookNumberOptionsMask;
	m_flagsLMCPO |= static_cast<LMChapterPrologueOptions>(ui->cmbCPOPsalmBookNumbers->itemData(nIndex).toUInt());
}

void CELSBibleDatabaseSelectDlg::en_VPONumberSelectionChanged(int nIndex)
{
	m_flagsLMVPO &= ~LMVPO_NumberOptionsMask;
	m_flagsLMVPO |= static_cast<LMVersePrologueOptions>(ui->cmbVPONumbers->itemData(nIndex).toUInt());
}

// ============================================================================

