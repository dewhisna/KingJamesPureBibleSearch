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
													   LMFullVerseTextOptionFlags flagsLMFVTO,
													   QWidget *parent)
	:	QDialog(parent),
		ui(new Ui::CELSBibleDatabaseSelectDlg),
		m_strBibleUUID(strBibleUUID),
		m_flagsLMTMO(flagsLMTMO),
		m_flagsLMBPO(flagsLMBPO),
		m_flagsLMCPO(flagsLMCPO),
		m_flagsLMVPO(flagsLMVPO),
		m_flagsLMFVTO(flagsLMFVTO)
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

	ui->chkRevelationOfJesus->setChecked(m_flagsLMBPO.testFlag(LMBPO_RevelationOfJesus));

	ui->cmbCPONumbers->addItem(tr("None"), LMCPO_NumbersNone);
	ui->cmbCPONumbers->addItem(tr("Roman"), LMCPO_NumbersRoman);
	ui->cmbCPONumbers->addItem(tr("Arabic"), LMCPO_NumbersArabic);
	ui->cmbCPONumbers->setCurrentIndex(ui->cmbCPONumbers->findData(QVariant::fromValue(static_cast<unsigned int>(m_flagsLMCPO & LMCPO_NumberOptionsMask))));

	ui->chkCPOPsalmBooks->setChecked(m_flagsLMCPO.testFlag(LMCPO_PsalmBookTags));
	ui->cmbCPOPsalmBookNumbers->addItem(tr("None"), LMCPO_PsalmBookNumbersNone);
	ui->cmbCPOPsalmBookNumbers->addItem(tr("Roman"), LMCPO_PsalmBookNumbersRoman);
	ui->cmbCPOPsalmBookNumbers->addItem(tr("Arabic"), LMCPO_PsalmBookNumbersArabic);
	ui->cmbCPOPsalmBookNumbers->setCurrentIndex(ui->cmbCPOPsalmBookNumbers->findData(QVariant::fromValue(static_cast<unsigned int>(m_flagsLMCPO & LMCPO_PsalmBookNumberOptionsMask))));
	ui->chkDisableChap1LabelAllBooks->setChecked(m_flagsLMCPO.testFlag(LMCPO_DisableChap1LabelAllBooks));
	ui->chkDisableSingleChapLabel->setChecked(m_flagsLMCPO.testFlag(LMCPO_DisableSingleChapLabel));

	ui->cmbVPONumbers->addItem(tr("None"), LMVPO_NumbersNone);
	ui->cmbVPONumbers->addItem(tr("Roman"), LMVPO_NumbersRoman);
	ui->cmbVPONumbers->addItem(tr("Arabic"), LMVPO_NumbersArabic);
	ui->cmbVPONumbers->setCurrentIndex(ui->cmbVPONumbers->findData(QVariant::fromValue(static_cast<unsigned int>(m_flagsLMVPO & LMVPO_NumberOptionsMask))));
	ui->chkDisableVerse1Number->setChecked(m_flagsLMVPO.testFlag(LMVPO_DisableVerse1Number));
	ui->chkEnableVerse1SingleChapter->setChecked(m_flagsLMVPO.testFlag(LMVPO_EnableVerse1SingleChap));
	ui->chkIncludePs119Hebrew->setChecked(m_flagsLMVPO.testFlag(LMVPO_PS119_HebrewLetter));
	ui->chkIncludePs119Transliteration->setChecked(m_flagsLMVPO.testFlag(LMVPO_PS119_Transliteration));
	ui->chkIncludePs119Punctuation->setChecked(m_flagsLMVPO.testFlag(LMVPO_PS119_Punctuation));

	ui->chkNoTransChangeAddedTags->setChecked(m_flagsLMFVTO.testFlag(LMFVTO_NoBracketsForTransChange));
	ui->chkIncludePilcrowMarkers->setChecked(m_flagsLMFVTO.testFlag(LMFVTO_IncludePilcrowMarkers));

	ui->chkWordsOfJesusOnly->setChecked(m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkRemoveColophons->setChecked(m_flagsLMTMO.testFlag(LMTMO_RemoveColophons));
	ui->chkRemoveSuperscriptions->setChecked(m_flagsLMTMO.testFlag(LMTMO_RemoveSuperscriptions));
	ui->chkIncludeBookPrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues));
	ui->chkIncludeChapterPrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->chkIncludeVersePrologues->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->chkIncludePunctuation->setChecked(m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation));
	ui->chkDecomposeLetters->setChecked(m_flagsLMTMO.testFlag(LMTMO_DecomposeLetters));

	ui->chkRemoveColophons->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkRemoveSuperscriptions->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeBookPrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeChapterPrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludeVersePrologues->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkIncludePunctuation->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly));
	ui->chkDecomposeLetters->setEnabled(true);		// Valid for all modes!

	ui->chkRevelationOfJesus->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues));

	ui->lblCPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->cmbCPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->chkCPOPsalmBooks->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->cmbCPOPsalmBookNumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) &&
										   m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues) &&
										   m_flagsLMCPO.testFlag(LMCPO_PsalmBookTags));
	ui->chkDisableChap1LabelAllBooks->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
	ui->chkDisableSingleChapLabel->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));

	ui->lblVPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->cmbVPONumbers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->chkDisableVerse1Number->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->chkEnableVerse1SingleChapter->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->chkIncludePs119Hebrew->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->chkIncludePs119Transliteration->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	ui->chkIncludePs119Punctuation->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues) &&
											   m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation));

	ui->chkNoTransChangeAddedTags->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && (textModifierOptions() & LMTMO_FTextModeMask));
	ui->chkIncludePilcrowMarkers->setEnabled(!m_flagsLMTMO.testFlag(LMTMO_WordsOfJesusOnly) && (textModifierOptions() & LMTMO_FTextModeMask));

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

			ui->chkRemoveColophons->setEnabled(false);
			ui->chkRemoveSuperscriptions->setEnabled(false);
			ui->chkIncludeBookPrologues->setEnabled(false);
			ui->chkIncludeChapterPrologues->setEnabled(false);
			ui->chkIncludeVersePrologues->setEnabled(false);
			ui->chkIncludePunctuation->setEnabled(false);

			ui->chkRevelationOfJesus->setEnabled(false);

			ui->lblCPONumbers->setEnabled(false);
			ui->cmbCPONumbers->setEnabled(false);
			ui->chkCPOPsalmBooks->setEnabled(false);
			ui->cmbCPOPsalmBookNumbers->setEnabled(false);
			ui->chkDisableChap1LabelAllBooks->setEnabled(false);
			ui->chkDisableSingleChapLabel->setEnabled(false);

			ui->lblVPONumbers->setEnabled(false);
			ui->cmbVPONumbers->setEnabled(false);
			ui->chkDisableVerse1Number->setEnabled(false);
			ui->chkEnableVerse1SingleChapter->setEnabled(false);
			ui->chkIncludePs119Hebrew->setEnabled(false);
			ui->chkIncludePs119Transliteration->setEnabled(false);
			ui->chkIncludePs119Punctuation->setEnabled(false);

			ui->chkNoTransChangeAddedTags->setEnabled(false);
			ui->chkIncludePilcrowMarkers->setEnabled(false);
		} else {
			ui->chkRemoveColophons->setEnabled(true);
			ui->chkRemoveSuperscriptions->setEnabled(true);
			ui->chkIncludeBookPrologues->setEnabled(true);
			ui->chkIncludeChapterPrologues->setEnabled(true);
			ui->chkIncludeVersePrologues->setEnabled(true);
			ui->chkIncludePunctuation->setEnabled(true);

			ui->chkRevelationOfJesus->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeBookPrologues));

			ui->lblCPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
			ui->cmbCPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
			ui->chkCPOPsalmBooks->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
			ui->cmbCPOPsalmBookNumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues) &&
												   m_flagsLMCPO.testFlag(LMCPO_PsalmBookTags));
			ui->chkDisableChap1LabelAllBooks->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));
			ui->chkDisableSingleChapLabel->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeChapterPrologues));

			ui->lblVPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
			ui->cmbVPONumbers->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
			ui->chkDisableVerse1Number->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
			ui->chkEnableVerse1SingleChapter->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
			ui->chkIncludePs119Hebrew->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
			ui->chkIncludePs119Transliteration->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
			ui->chkIncludePs119Punctuation->setEnabled(m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues) &&
													   m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation));

			ui->chkNoTransChangeAddedTags->setEnabled((textModifierOptions() & LMTMO_FTextModeMask) != 0);
			ui->chkIncludePilcrowMarkers->setEnabled((textModifierOptions() & LMTMO_FTextModeMask) != 0);
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

		ui->chkRevelationOfJesus->setEnabled(bIncludeBookPrologues);
	});
	connect(ui->chkIncludeChapterPrologues, &QCheckBox::toggled, this, [this](bool bIncludeChapterPrologues)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludeChapterPrologues, bIncludeChapterPrologues);

		ui->lblCPONumbers->setEnabled(bIncludeChapterPrologues);
		ui->cmbCPONumbers->setEnabled(bIncludeChapterPrologues);
		ui->chkCPOPsalmBooks->setEnabled(bIncludeChapterPrologues);
		ui->cmbCPOPsalmBookNumbers->setEnabled(bIncludeChapterPrologues && m_flagsLMCPO.testFlag(LMCPO_PsalmBookTags));
		ui->chkDisableChap1LabelAllBooks->setEnabled(bIncludeChapterPrologues);
		ui->chkDisableSingleChapLabel->setEnabled(bIncludeChapterPrologues);
	});
	connect(ui->chkIncludeVersePrologues, &QCheckBox::toggled, this, [this](bool bIncludeVersePrologues)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludeVersePrologues, bIncludeVersePrologues);

		ui->lblVPONumbers->setEnabled(bIncludeVersePrologues);
		ui->cmbVPONumbers->setEnabled(bIncludeVersePrologues);
		ui->chkDisableVerse1Number->setEnabled(bIncludeVersePrologues);
		ui->chkEnableVerse1SingleChapter->setEnabled(bIncludeVersePrologues);
		ui->chkIncludePs119Hebrew->setEnabled(bIncludeVersePrologues);
		ui->chkIncludePs119Transliteration->setEnabled(bIncludeVersePrologues);
		ui->chkIncludePs119Punctuation->setEnabled(bIncludeVersePrologues && m_flagsLMTMO.testFlag(LMTMO_IncludePunctuation));
	});
	connect(ui->chkIncludePunctuation, &QCheckBox::toggled, this, [this](bool bIncludePunctuation)->void {
		m_flagsLMTMO.setFlag(LMTMO_IncludePunctuation, bIncludePunctuation);

		ui->chkNoTransChangeAddedTags->setEnabled((textModifierOptions() & LMTMO_FTextModeMask) != 0);
		ui->chkIncludePilcrowMarkers->setEnabled((textModifierOptions() & LMTMO_FTextModeMask) != 0);

		ui->chkIncludePs119Punctuation->setEnabled(bIncludePunctuation && m_flagsLMTMO.testFlag(LMTMO_IncludeVersePrologues));
	});
	connect(ui->chkDecomposeLetters, &QCheckBox::toggled, this, [this](bool bDecomposeLetters)->void {
		m_flagsLMTMO.setFlag(LMTMO_DecomposeLetters, bDecomposeLetters);
	});

	connect(ui->chkRevelationOfJesus, &QCheckBox::toggled, this, [this](bool bRevelationOfJesus)->void {
		m_flagsLMBPO.setFlag(LMBPO_RevelationOfJesus, bRevelationOfJesus);
	});

	connect(ui->cmbCPONumbers, SIGNAL(currentIndexChanged(int)), this, SLOT(en_CPONumberSelectionChanged(int)));
	connect(ui->chkCPOPsalmBooks, &QCheckBox::toggled, this, [this](bool bCPOPsalmBooks)->void {
		m_flagsLMCPO.setFlag(LMCPO_PsalmBookTags, bCPOPsalmBooks);

		ui->cmbCPOPsalmBookNumbers->setEnabled(bCPOPsalmBooks);
	});
	connect(ui->cmbCPOPsalmBookNumbers, SIGNAL(currentIndexChanged(int)), this, SLOT(en_CPOPsalmBookNumberSelectionChanged(int)));
	connect(ui->chkDisableChap1LabelAllBooks, &QCheckBox::toggled, this, [this](bool bDisableChap1LabelAllBooks)->void {
		m_flagsLMCPO.setFlag(LMCPO_DisableChap1LabelAllBooks, bDisableChap1LabelAllBooks);
	});
	connect(ui->chkDisableSingleChapLabel, &QCheckBox::toggled, this, [this](bool bDisableSingleChapLabel)->void {
		m_flagsLMCPO.setFlag(LMCPO_DisableSingleChapLabel, bDisableSingleChapLabel);
	});

	connect(ui->cmbVPONumbers, SIGNAL(currentIndexChanged(int)), this, SLOT(en_VPONumberSelectionChanged(int)));
	connect(ui->chkDisableVerse1Number, &QCheckBox::toggled, this, [this](bool bDisableVerse1Number)->void {
		m_flagsLMVPO.setFlag(LMVPO_DisableVerse1Number, bDisableVerse1Number);
	});
	connect(ui->chkEnableVerse1SingleChapter, &QCheckBox::toggled, this, [this](bool bEnableVerse1SingleChapter)->void {
		m_flagsLMVPO.setFlag(LMVPO_EnableVerse1SingleChap, bEnableVerse1SingleChapter);
	});
	connect(ui->chkIncludePs119Hebrew, &QCheckBox::toggled, this, [this](bool bIncludePs119Hebrew)->void {
		m_flagsLMVPO.setFlag(LMVPO_PS119_HebrewLetter, bIncludePs119Hebrew);
	});
	connect(ui->chkIncludePs119Transliteration, &QCheckBox::toggled, this, [this](bool bIncludePs119Transliteration)->void {
		m_flagsLMVPO.setFlag(LMVPO_PS119_Transliteration, bIncludePs119Transliteration);
	});
	connect(ui->chkIncludePs119Punctuation, &QCheckBox::toggled, this, [this](bool bIncludePs119Punctuation)->void {
		m_flagsLMVPO.setFlag(LMVPO_PS119_Punctuation, bIncludePs119Punctuation);
	});

	connect(ui->chkNoTransChangeAddedTags, &QCheckBox::toggled, this, [this](bool bNoTransChangeAddedTags)->void {
		m_flagsLMFVTO.setFlag(LMFVTO_NoBracketsForTransChange, bNoTransChangeAddedTags);
	});
	connect(ui->chkIncludePilcrowMarkers, &QCheckBox::toggled, this, [this](bool bIncludePilcrowMarkers)->void {
		m_flagsLMFVTO.setFlag(LMFVTO_IncludePilcrowMarkers, bIncludePilcrowMarkers);
	});
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

