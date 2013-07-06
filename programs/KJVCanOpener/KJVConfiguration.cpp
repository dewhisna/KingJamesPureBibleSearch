/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "KJVConfiguration.h"
#include "ui_KJVTextFormatConfig.h"

#include "PhraseEdit.h"
#include "ScriptureEdit.h"
#include "KJVSearchResult.h"
#include "KJVSearchCriteria.h"
#include "PersistentSettings.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QSizePolicy>
#include <QFontDatabase>
#include <QListWidget>
#include <QListWidgetItem>
#include <QwwColorButton>

// ============================================================================

CHighlighterColorButtonSignalReflector::CHighlighterColorButtonSignalReflector(CKJVTextFormatConfig *pConfigurator, int nHighlighterIndex)
	:	QObject(NULL),
		m_nUserDefinedHighlighterIndex(nHighlighterIndex)
{
	connect(this, SIGNAL(colorPicked(int, const QColor &)), pConfigurator, SLOT(en_HighlighterColorPicked(int, const QColor &)));
}

CHighlighterColorButtonSignalReflector::~CHighlighterColorButtonSignalReflector()
{

}

void CHighlighterColorButtonSignalReflector::en_colorPicked(const QColor &color)
{
	emit colorPicked(m_nUserDefinedHighlighterIndex, color);
}

// ============================================================================

class CHighlighterColorButton : public CHighlighterColorButtonSignalReflector, public QListWidgetItem
{
public:
	CHighlighterColorButton(CKJVTextFormatConfig *pConfigurator, QListWidget *pList, int nHighlighterIndex);
	~CHighlighterColorButton();

private:
	QwwColorButton *m_pColorButton;
};

CHighlighterColorButton::CHighlighterColorButton(CKJVTextFormatConfig *pConfigurator, QListWidget *pList, int nHighlighterIndex)
	:	CHighlighterColorButtonSignalReflector(pConfigurator, nHighlighterIndex),
		QListWidgetItem(pList, 0),
		m_pColorButton(NULL)
{
	assert(pList != NULL);

	m_pColorButton = new QwwColorButton(pList);
	m_pColorButton->setObjectName(QString("buttonHighlighter%1Color").arg(nHighlighterIndex, 2, QChar('0')));
	m_pColorButton->setShowName(false);			// Must do this before setting our real text
	m_pColorButton->setText(QObject::tr("Highlighter #%1").arg(nHighlighterIndex));
	m_pColorButton->setCurrentColor(CPersistentSettings::instance()->userDefinedColor(nHighlighterIndex));
	setSizeHint(m_pColorButton->sizeHint());

	pList->setItemWidget(this, m_pColorButton);
	pList->setMinimumWidth(m_pColorButton->minimumWidth());

	connect(m_pColorButton, SIGNAL(colorPicked(const QColor &)), this, SLOT(en_colorPicked(const QColor &)));
}

CHighlighterColorButton::~CHighlighterColorButton()
{

}

// ============================================================================

CKJVTextFormatConfig::CKJVTextFormatConfig(CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QWidget(parent),
	//	m_pBibleDatabase(pBibleDatabase),
	m_pSearchResultsTreeView(NULL),
	m_pScriptureBrowser(NULL),
	m_bIsDirty(false),
	ui(new Ui::CKJVTextFormatConfig)
{
	assert(pBibleDatabase != NULL);

	ui->setupUi(this);

	// --------------------------------------------------------------

	//	Swapout the treeViewSearchResultsPreview from the layout with
	//		one that we can set the database on:

	int ndx = ui->splitter->indexOf(ui->treeViewSearchResultsPreview);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pSearchResultsTreeView = new CSearchResultsTreeView(pBibleDatabase, this);
	m_pSearchResultsTreeView->setObjectName(QString::fromUtf8("treeViewSearchResultsPreview"));
	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy1.setHorizontalStretch(5);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(m_pSearchResultsTreeView->sizePolicy().hasHeightForWidth());
	m_pSearchResultsTreeView->setSizePolicy(sizePolicy1);

	delete ui->treeViewSearchResultsPreview;
	ui->treeViewSearchResultsPreview = NULL;
	ui->splitter->insertWidget(ndx, m_pSearchResultsTreeView);

	// --------------------------------------------------------------

	//	Swapout the textScriptureBrowserPreview from the layout with
	//		one that we can set the database on:

	ndx = ui->splitter->indexOf(ui->textScriptureBrowserPreview);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pScriptureBrowser = new CScriptureBrowser(pBibleDatabase, this);
	m_pScriptureBrowser->setObjectName(QString::fromUtf8("textScriptureBrowserPreview"));
	QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy2.setHorizontalStretch(10);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(m_pScriptureBrowser->sizePolicy().hasHeightForWidth());
	m_pScriptureBrowser->setSizePolicy(sizePolicy2);
	m_pScriptureBrowser->setMouseTracking(true);
	m_pScriptureBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_pScriptureBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pScriptureBrowser->setTabChangesFocus(true);
	m_pScriptureBrowser->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
	m_pScriptureBrowser->setOpenLinks(false);
	m_pScriptureBrowser->setContextMenuPolicy(Qt::NoContextMenu);
	m_pScriptureBrowser->setToolTip(QString());			// Disable the "Press Ctrl-D" tooltip, since that mode isn't enable in the configurator

	delete ui->textScriptureBrowserPreview;
	ui->textScriptureBrowserPreview = NULL;
	ui->splitter->insertWidget(ndx, m_pScriptureBrowser);

	// --------------------------------------------------------------

	delete ui->buttonWordsOfJesusColor;
	delete ui->buttonSearchResultsColor;
	delete ui->buttonCursorFollowColor;

	ui->buttonWordsOfJesusColor = new QwwColorButton(this);
	ui->buttonWordsOfJesusColor->setObjectName(QString::fromUtf8("buttonWordsOfJesusColor"));
	toQwwColorButton(ui->buttonWordsOfJesusColor)->setShowName(false);			// Must do this before setting our real text
	ui->buttonWordsOfJesusColor->setText(tr("Words of Jesus"));
	ui->vertLayoutColorOptions->addWidget(ui->buttonWordsOfJesusColor);

	ui->buttonSearchResultsColor = new QwwColorButton(this);
	ui->buttonSearchResultsColor->setObjectName(QString::fromUtf8("buttonSearchResultsColor"));
	toQwwColorButton(ui->buttonSearchResultsColor)->setShowName(false);			// Must do this before setting our real text
	ui->buttonSearchResultsColor->setText(tr("Search Results"));
	ui->vertLayoutColorOptions->addWidget(ui->buttonSearchResultsColor);

	ui->buttonCursorFollowColor = new QwwColorButton(this);
	ui->buttonCursorFollowColor->setObjectName(QString::fromUtf8("buttonCursorFollowColor"));
	toQwwColorButton(ui->buttonCursorFollowColor)->setShowName(false);			// Must do this before setting our real text
	ui->buttonCursorFollowColor->setText(tr("Cursor Tracker"));
	ui->vertLayoutColorOptions->addWidget(ui->buttonCursorFollowColor);

	toQwwColorButton(ui->buttonWordsOfJesusColor)->setCurrentColor(CPersistentSettings::instance()->colorWordsOfJesus());
	toQwwColorButton(ui->buttonSearchResultsColor)->setCurrentColor(CPersistentSettings::instance()->colorSearchResults());
	toQwwColorButton(ui->buttonCursorFollowColor)->setCurrentColor(CPersistentSettings::instance()->colorCursorFollow());

	connect(toQwwColorButton(ui->buttonWordsOfJesusColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_WordsOfJesusColorPicked(const QColor &)));
	connect(toQwwColorButton(ui->buttonSearchResultsColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_SearchResultsColorPicked(const QColor &)));
	connect(toQwwColorButton(ui->buttonCursorFollowColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_CursorTrackerColorPicked(const QColor &)));

	const TUserDefinedColorMap &userDefinedColorMap = CPersistentSettings::instance()->userDefinedColorMap();
	for (TUserDefinedColorMap::const_iterator itrHighlighters = userDefinedColorMap.constBegin(); itrHighlighters != userDefinedColorMap.constEnd(); ++itrHighlighters) {
		new CHighlighterColorButton(this, ui->listWidgetHighlighterColors, itrHighlighters.key());
	}
	updateGeometry();

	// --------------------------------------------------------------

	// Reinsert them in the correct TabOrder:
	QWidget::setTabOrder(ui->toolButtonRemoveHighlighter, ui->buttonWordsOfJesusColor);
	QWidget::setTabOrder(ui->buttonWordsOfJesusColor, ui->buttonSearchResultsColor);
	QWidget::setTabOrder(ui->buttonSearchResultsColor, ui->buttonCursorFollowColor);
	QWidget::setTabOrder(ui->buttonCursorFollowColor, m_pSearchResultsTreeView);
	QWidget::setTabOrder(m_pSearchResultsTreeView, m_pScriptureBrowser);

	// --------------------------------------------------------------

	m_fntScriptureBrowser = CPersistentSettings::instance()->fontScriptureBrowser();
	m_fntSearchResults = CPersistentSettings::instance()->fontSearchResults();

	ui->fontComboBoxScriptureBrowser->setCurrentFont(m_fntScriptureBrowser);
	ui->fontComboBoxSearchResults->setCurrentFont(m_fntSearchResults);

	QList<int> lstStandardFontSizes = QFontDatabase::standardSizes();
	int nFontMin = -1;
	int nFontMax = -1;
	for (int ndx=0; ndx<lstStandardFontSizes.size(); ++ndx) {
		if ((nFontMin == -1) || (lstStandardFontSizes.at(ndx) < nFontMin)) nFontMin = lstStandardFontSizes.at(ndx);
		if ((nFontMax == -1) || (lstStandardFontSizes.at(ndx) > nFontMax)) nFontMax = lstStandardFontSizes.at(ndx);
	}
	ui->dblSpinBoxScriptureBrowserFontSize->setRange(nFontMin, nFontMax);
	ui->dblSpinBoxScriptureBrowserFontSize->setValue(m_fntScriptureBrowser.pointSizeF());
	ui->dblSpinBoxSearchResultsFontSize->setRange(nFontMin, nFontMax);
	ui->dblSpinBoxSearchResultsFontSize->setValue(m_fntSearchResults.pointSizeF());

	connect(ui->fontComboBoxScriptureBrowser, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_ScriptureBrowserFontChanged(const QFont &)));
	connect(ui->fontComboBoxSearchResults, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_SearchResultsFontChanged(const QFont &)));
	connect(ui->dblSpinBoxScriptureBrowserFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_ScriptureBrowserFontSizeChanged(double)));
	connect(ui->dblSpinBoxSearchResultsFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_SearchResultsFontSizeChanged(double)));

	// --------------------------------------------------------------

	m_bInvertTextBrightness = CPersistentSettings::instance()->invertTextBrightness();
	m_nTextBrightness = CPersistentSettings::instance()->textBrightness();
	m_bAdjustDialogElementBrightness = CPersistentSettings::instance()->adjustDialogElementBrightness();

	ui->checkBoxInvertTextBrightness->setChecked(m_bInvertTextBrightness);
	ui->horzSliderTextBrigtness->setValue(m_nTextBrightness);
	ui->checkBoxAdjustDialogElementBrightness->setChecked(m_bAdjustDialogElementBrightness);

	connect(ui->checkBoxInvertTextBrightness, SIGNAL(clicked(bool)), this, SLOT(en_InvertTextBrightnessChanged(bool)));
	connect(ui->horzSliderTextBrigtness, SIGNAL(valueChanged(int)), this, SLOT(en_TextBrightnessChanged(int)));
	connect(ui->checkBoxAdjustDialogElementBrightness, SIGNAL(clicked(bool)), this, SLOT(en_AdjustDialogElementBrightness(bool)));

	// --------------------------------------------------------------

	navigateToDemoText();
	setPreview();
}

CKJVTextFormatConfig::~CKJVTextFormatConfig()
{
	delete ui;
}

void CKJVTextFormatConfig::saveSettings()
{
	CPersistentSettings::instance()->setFontScriptureBrowser(m_fntScriptureBrowser);
	CPersistentSettings::instance()->setFontSearchResults(m_fntSearchResults);
	CPersistentSettings::instance()->setInvertTextBrightness(m_bInvertTextBrightness);
	CPersistentSettings::instance()->setTextBrightness(m_nTextBrightness);
	CPersistentSettings::instance()->setAdjustDialogElementBrightness(m_bAdjustDialogElementBrightness);
	m_bIsDirty = false;
}

void CKJVTextFormatConfig::en_ScriptureBrowserFontChanged(const QFont &font)
{
	m_fntScriptureBrowser.setFamily(font.family());
	m_pScriptureBrowser->setFont(m_fntScriptureBrowser);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_SearchResultsFontChanged(const QFont &font)
{
	m_fntSearchResults.setFamily(font.family());
	m_pSearchResultsTreeView->setFontSearchResults(m_fntSearchResults);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_ScriptureBrowserFontSizeChanged(double nFontSize)
{
	m_fntScriptureBrowser.setPointSizeF(nFontSize);
	m_pScriptureBrowser->setFont(m_fntScriptureBrowser);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_SearchResultsFontSizeChanged(double nFontSize)
{
	m_fntSearchResults.setPointSizeF(nFontSize);
	m_pSearchResultsTreeView->setFontSearchResults(m_fntSearchResults);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_InvertTextBrightnessChanged(bool bInvert)
{
	m_bInvertTextBrightness = bInvert;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_TextBrightnessChanged(int nBrightness)
{
	m_nTextBrightness = nBrightness;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_AdjustDialogElementBrightness(bool bAdjust)
{
	m_bAdjustDialogElementBrightness = bAdjust;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_WordsOfJesusColorPicked(const QColor &color)
{
	CPersistentSettings::instance()->setColorWordsOfJesus(color);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_SearchResultsColorPicked(const QColor &color)
{
	CPersistentSettings::instance()->setColorSearchResults(color);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_CursorTrackerColorPicked(const QColor &color)
{
	CPersistentSettings::instance()->setColorCursorFollow(color);
//	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_HighlighterColorPicked(int nHighlighterIndex, const QColor &color)
{
	CPersistentSettings::instance()->setUserDefinedColor(nHighlighterIndex, color);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::navigateToDemoText()
{
	CRelIndex ndxPreview(41, 9, 1, 1);						// Goto Mark 9:1 for Preview (as it has some red-letter text)
	CRelIndex ndxPreview2(41, 9, 3, 1);						// Goto Mark 9:3 for additional Search Results highlight so we can get all combinations of highlighters...
	m_pScriptureBrowser->navigator().setDocumentToChapter(ndxPreview);
	m_pScriptureBrowser->navigator().selectWords(TPhraseTag(ndxPreview));
	m_pScriptureBrowser->navigator().doHighlighting(CSearchResultHighlighter(TPhraseTag(ndxPreview, 5)));
	m_pScriptureBrowser->navigator().doHighlighting(CSearchResultHighlighter(TPhraseTag(ndxPreview2, 32)));

	uint32_t nNormalizedIndex = m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->NormalizeIndex(ndxPreview) + 10;

	for (int i = 0; i < 3; ++i) {
		// Originally I had a const_iterator over the CPersistentSettings::userDefinedColorMap, but something
		//		in the process of doHighlighting kept invalidating our iterator and causing an infinite loop.
		//		Solution was to iterate over the buttons in our QListWidget of Highlighter Set buttons.. <sigh>
		for (int ndx = 0; ndx < ui->listWidgetHighlighterColors->count(); ++ndx) {
			m_pScriptureBrowser->navigator().doHighlighting(CUserDefinedHighlighter(static_cast<CHighlighterColorButton *>(ui->listWidgetHighlighterColors->item(ndx))->highlighterIndex(),
															TPhraseTag(CRelIndex(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->DenormalizeIndex(nNormalizedIndex)), 5)));
			nNormalizedIndex += 7;
		}
	}

	CParsedPhrase phrase(m_pSearchResultsTreeView->vlmodel()->bibleDatabase());
	phrase.ParsePhrase("trumpet");
	phrase.FindWords(phrase.GetCursorWordPos());
	TParsedPhrasesList lstPhrases;
	lstPhrases.append(&phrase);
	CSearchCriteria aSearchCriteria;
	m_pSearchResultsTreeView->setParsedPhrases(aSearchCriteria, lstPhrases);
	m_pSearchResultsTreeView->setDisplayMode(CVerseListModel::VDME_RICHTEXT);
	m_pSearchResultsTreeView->setTreeMode(CVerseListModel::VTME_TREE_CHAPTERS);
	m_pSearchResultsTreeView->expandAll();
}

void CKJVTextFormatConfig::setPreview()
{
	m_pSearchResultsTreeView->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);

	m_pScriptureBrowser->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
}

// ============================================================================

CKJVConfiguration::CKJVConfiguration(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QwwConfigWidget(parent)
{
	assert(pBibleDatabase != NULL);

	m_pTextFormatConfig = new CKJVTextFormatConfig(pBibleDatabase, this);

	addGroup(m_pTextFormatConfig, QIcon(":res/Font_Graphics_Color_Icon_128.png"), "Text Color and Fonts");
	setCurrentGroup(m_pTextFormatConfig);

	connect(m_pTextFormatConfig, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
}

CKJVConfiguration::~CKJVConfiguration()
{

}

void CKJVConfiguration::saveSettings()
{
	m_pTextFormatConfig->saveSettings();
}

// ============================================================================

CKJVConfigurationDialog::CKJVConfigurationDialog(CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QDialog(parent),
	m_pConfiguration(NULL),
	m_pButtonBox(NULL)
{
	assert(pBibleDatabase != NULL);

	// --------------------------------------------------------------

	// Make a working copy of our settings:
	CPersistentSettings::instance()->togglePersistentSettingData(true);

	// --------------------------------------------------------------

	QVBoxLayout *pLayout = new QVBoxLayout(this);
	pLayout->setObjectName(QString::fromUtf8("verticalLayout"));

	m_pConfiguration = new CKJVConfiguration(pBibleDatabase, this);
	m_pConfiguration->setObjectName(QString::fromUtf8("configurationWidget"));
	pLayout->addWidget(m_pConfiguration);

	m_pButtonBox = new QDialogButtonBox(this);
	m_pButtonBox->setObjectName(QString::fromUtf8("buttonBox"));
	m_pButtonBox->setOrientation(Qt::Horizontal);
	m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
	m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	pLayout->addWidget(m_pButtonBox);

	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(m_pButtonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

	connect(m_pConfiguration, SIGNAL(dataChanged()), this, SLOT(en_dataChanged()));
}

CKJVConfigurationDialog::~CKJVConfigurationDialog()
{

}

void CKJVConfigurationDialog::en_dataChanged()
{
	m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(m_pConfiguration->isDirty());
}

void CKJVConfigurationDialog::accept()
{
	m_pConfiguration->saveSettings();
	QDialog::accept();
	// Note: Leave the settings permanent, by not copying
	//		them back in the persistent settings object
}

void CKJVConfigurationDialog::reject()
{
	// Restore original settings by switching back to the original
	//		settings without copying:
	CPersistentSettings::instance()->togglePersistentSettingData(false);
	QDialog::reject();
}

void CKJVConfigurationDialog::apply()
{
	// Make sure our persistent settings have been updated, and we'll
	//		copy the settings over to the original, making them permanent
	//		as the user is "applying" them:
	m_pConfiguration->saveSettings();
	CPersistentSettings::instance()->togglePersistentSettingData(true);
	en_dataChanged();
}

// ============================================================================
