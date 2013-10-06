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

#include "ScriptureEdit.h"
#include "KJVSearchResult.h"
#include "KJVSearchCriteria.h"
#include "DictionaryWidget.h"
#include "PersistentSettings.h"
#include "Highlighter.h"
#include "SearchCompleter.h"
#include "PhraseEdit.h"
#include "RenameHighlighterDlg.h"
#include "BusyCursor.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QGridLayout>
#include <QSplitter>
#include <QSizePolicy>
#include <QFontDatabase>
#include <QListWidget>
#include <QListWidgetItem>
#include <QwwColorButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QRegExp>

// ============================================================================

CHighlighterColorButtonSignalReflector::CHighlighterColorButtonSignalReflector(CKJVTextFormatConfig *pConfigurator, const QString &strUserDefinedHighlighterName)
	:	QObject(NULL),
		m_strUserDefinedHighlighterName(strUserDefinedHighlighterName)
{
	connect(this, SIGNAL(colorPicked(const QString &, const QColor &)), pConfigurator, SLOT(en_HighlighterColorPicked(const QString &, const QColor &)));
	connect(this, SIGNAL(colorClicked(const QString &)), pConfigurator, SLOT(en_HighlighterColorClicked(const QString &)));
	connect(this, SIGNAL(enableChanged(const QString &, bool)), pConfigurator, SLOT(en_HighlighterEnableChanged(const QString &, bool)));
}

CHighlighterColorButtonSignalReflector::~CHighlighterColorButtonSignalReflector()
{

}

void CHighlighterColorButtonSignalReflector::en_colorPicked(const QColor &color)
{
	emit colorPicked(m_strUserDefinedHighlighterName, color);
}

void CHighlighterColorButtonSignalReflector::en_clicked()
{
	emit colorClicked(m_strUserDefinedHighlighterName);
}

void CHighlighterColorButtonSignalReflector::en_enableClicked(bool bEnabled)
{
	emit enableChanged(m_strUserDefinedHighlighterName, bEnabled);
}

// ============================================================================

class CHighlighterColorButton : public CHighlighterColorButtonSignalReflector, public QListWidgetItem
{
public:
	CHighlighterColorButton(CKJVTextFormatConfig *pConfigurator, QListWidget *pList, const QString &strUserDefinedHighlighterName);
	~CHighlighterColorButton();

	virtual bool operator <(const QListWidgetItem & other) const
	{
		const CHighlighterColorButton &myOther = static_cast<const CHighlighterColorButton &>(other);
		HighlighterNameSortPredicate sortPred;

		return sortPred(highlighterName(), myOther.highlighterName());
	}

	bool enabled() const { return m_pEnableCheckbox->isChecked(); }
	void setEnabled(bool bEnabled)
	{
		m_pEnableCheckbox->setChecked(bEnabled);
		emit enableChanged(highlighterName(), bEnabled);
	}

protected slots:
	virtual void en_setTextBrightness(bool bInvert, int nBrightness);
	virtual void en_adjustDialogElementBrightnessChanged(bool bAdjust);

private:
	void setBrightness(bool bAdjust, bool bInvert, int nBrightness);

private:
	QWidget *m_pWidget;
	QHBoxLayout *m_pHorzLayout;
	QwwColorButton *m_pColorButton;
	QCheckBox *m_pEnableCheckbox;
};

CHighlighterColorButton::CHighlighterColorButton(CKJVTextFormatConfig *pConfigurator, QListWidget *pList, const QString &strUserDefinedHighlighterName)
	:	CHighlighterColorButtonSignalReflector(pConfigurator, strUserDefinedHighlighterName),
		QListWidgetItem(pList, 0),
		m_pWidget(NULL),
		m_pHorzLayout(NULL),
		m_pColorButton(NULL),
		m_pEnableCheckbox(NULL)
{
	assert(pList != NULL);
	assert(g_pUserNotesDatabase != NULL);

	m_pWidget = new QWidget(pList);
	m_pWidget->setObjectName(QString("widget_%1").arg(strUserDefinedHighlighterName));

	m_pHorzLayout = new QHBoxLayout(m_pWidget);
	m_pHorzLayout->setObjectName(QString("hboxLayout_%1").arg(strUserDefinedHighlighterName));
	m_pHorzLayout->setMargin(0);
	m_pHorzLayout->setContentsMargins(0, 0, 0, 0);

	m_pColorButton = new QwwColorButton(m_pWidget);
	m_pColorButton->setObjectName(QString("buttonHighlighterColor_%1").arg(strUserDefinedHighlighterName));
	m_pColorButton->setShowName(false);			// Must do this before setting our real text
	m_pColorButton->setText(strUserDefinedHighlighterName);
	m_pColorButton->setCurrentColor(g_pUserNotesDatabase->highlighterColor(strUserDefinedHighlighterName));
	m_pColorButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	m_pColorButton->updateGeometry();
	m_pColorButton->setMinimumWidth(m_pColorButton->sizeHint().width());
	m_pHorzLayout->addWidget(m_pColorButton);

	m_pEnableCheckbox = new QCheckBox(m_pWidget);
	m_pEnableCheckbox->setObjectName(QString("checkbox_%1").arg(strUserDefinedHighlighterName));
	m_pEnableCheckbox->setCheckable(true);
	m_pEnableCheckbox->setChecked(g_pUserNotesDatabase->highlighterEnabled(strUserDefinedHighlighterName));
	m_pEnableCheckbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pEnableCheckbox->setText(tr("Enable"));
	m_pEnableCheckbox->setToolTip(tr("Enable/Disable this highlighter"));
	m_pEnableCheckbox->updateGeometry();
	m_pHorzLayout->addWidget(m_pEnableCheckbox);

	setBrightness(CPersistentSettings::instance()->adjustDialogElementBrightness(), CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(en_setTextBrightness(bool, int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(en_adjustDialogElementBrightnessChanged(bool)));

	m_pHorzLayout->addStretch(0);

	m_pWidget->setLayout(m_pHorzLayout);
	m_pWidget->updateGeometry();
	setSizeHint(m_pWidget->sizeHint());
	pList->setItemWidget(this, m_pWidget);

	connect(m_pColorButton, SIGNAL(colorPicked(const QColor &)), this, SLOT(en_colorPicked(const QColor &)));
	connect(m_pColorButton, SIGNAL(clicked()), this, SLOT(en_clicked()));
	connect(m_pEnableCheckbox, SIGNAL(clicked(bool)), this, SLOT(en_enableClicked(bool)));
}

CHighlighterColorButton::~CHighlighterColorButton()
{

}

void CHighlighterColorButton::en_setTextBrightness(bool bInvert, int nBrightness)
{
	setBrightness(CPersistentSettings::instance()->adjustDialogElementBrightness(), bInvert, nBrightness);
}

void CHighlighterColorButton::en_adjustDialogElementBrightnessChanged(bool bAdjust)
{
	setBrightness(bAdjust, CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
}

void CHighlighterColorButton::setBrightness(bool bAdjust, bool bInvert, int nBrightness)
{
	QColor clrBackground = CPersistentSettings::textBackgroundColor(bInvert, nBrightness);
	QColor clrForeground = CPersistentSettings::textForegroundColor(bInvert, nBrightness);

	if (bAdjust) {
		if (!bInvert) {
			m_pEnableCheckbox->setStyleSheet(QString("QCheckBox { background-color:%1; color:%2; }\n"
													 "QCheckBox::indicator {\n"
													 "    color: %2;\n"
													 "    background-color: %1;\n"
													 "    border: 1px solid %2;\n"
													 "    width: 9px;\n"
													 "    height: 9px;\n"
													 "}\n"
													 "QCheckBox::indicator:checked {\n"
													 "    image:url(:/res/checkbox2.png);\n"
													 "}\n"
													)
													.arg(clrBackground.name())
													.arg(clrForeground.name()));
		} else {
			m_pEnableCheckbox->setStyleSheet(QString("QCheckBox { background-color:%1; color:%2; }\n"
													 "QCheckBox::indicator {\n"
													 "    color: %2;\n"
													 "    background-color: %1;\n"
													 "    border: 1px solid %2;\n"
													 "    width: 9px;\n"
													 "    height: 9px;\n"
													 "}\n"
													 "QCheckBox::indicator:checked {\n"
													 "    image:url(:/res/checkbox.png);\n"
													 "}\n"
													)
													.arg(clrBackground.name())
													.arg(clrForeground.name()));
		}
	} else {
		m_pEnableCheckbox->setStyleSheet(QString("QCheckBox::indicator {\n"
												 "    border: 1px solid;\n"
												 "    width: 9px;\n"
												 "    height: 9px;\n"
												 "}\n"
												 "QCheckBox::indicator:checked {\n"
												 "    image:url(:/res/checkbox2.png);\n"
												 "}\n"));
	}
}

// ============================================================================

CKJVTextFormatConfig::CKJVTextFormatConfig(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent) :
	QWidget(parent),
	//	m_pBibleDatabase(pBibleDatabase),
	//	m_pDictionaryDatabase(pDictionary),
	m_previewSearchPhrase(pBibleDatabase),
	m_pSearchResultsTreeView(NULL),
	m_pScriptureBrowser(NULL),
	m_pDictionaryWidget(NULL),
	m_bIsDirty(false),
	m_bLoadingData(false)
{
	assert(pBibleDatabase != NULL);
	assert(pDictionary != NULL);
	assert(g_pUserNotesDatabase != NULL);

	ui.setupUi(this);

	// --------------------------------------------------------------

	//	Swapout the treeViewSearchResultsPreview from the layout with
	//		one that we can set the database on:

	int ndx = ui.splitter->indexOf(ui.treeViewSearchResultsPreview);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pSearchResultsTreeView = new CSearchResultsTreeView(pBibleDatabase, g_pUserNotesDatabase, this);
	m_pSearchResultsTreeView->setObjectName(QString::fromUtf8("treeViewSearchResultsPreview"));
	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy1.setHorizontalStretch(10);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(m_pSearchResultsTreeView->sizePolicy().hasHeightForWidth());
	m_pSearchResultsTreeView->setSizePolicy(sizePolicy1);
	m_pSearchResultsTreeView->setContextMenuPolicy(Qt::NoContextMenu);
	m_pSearchResultsTreeView->setToolTip(tr("Search Results Preview"));

	delete ui.treeViewSearchResultsPreview;
	ui.treeViewSearchResultsPreview = NULL;
	ui.splitter->insertWidget(ndx, m_pSearchResultsTreeView);

	// --------------------------------------------------------------

	//	Swapout the textScriptureBrowserPreview from the layout with
	//		one that we can set the database on:

	ndx = ui.splitterDictionary->indexOf(ui.textScriptureBrowserPreview);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pScriptureBrowser = new CScriptureBrowser(pBibleDatabase, this);
	m_pScriptureBrowser->setObjectName(QString::fromUtf8("textScriptureBrowserPreview"));
	QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy2.setHorizontalStretch(20);
	sizePolicy2.setVerticalStretch(10);
	sizePolicy2.setHeightForWidth(m_pScriptureBrowser->sizePolicy().hasHeightForWidth());
	m_pScriptureBrowser->setSizePolicy(sizePolicy2);
	m_pScriptureBrowser->setMouseTracking(true);
	m_pScriptureBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_pScriptureBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pScriptureBrowser->setTabChangesFocus(true);
	m_pScriptureBrowser->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
	m_pScriptureBrowser->setOpenLinks(false);
	m_pScriptureBrowser->setContextMenuPolicy(Qt::DefaultContextMenu);
	m_pScriptureBrowser->setToolTip(tr("Scripture Browser Preview"));			// Note:  Also disables the "Press Ctrl-D" tooltip, since that mode isn't enable in the configurator

	delete ui.textScriptureBrowserPreview;
	ui.textScriptureBrowserPreview = NULL;
	ui.splitterDictionary->insertWidget(ndx, m_pScriptureBrowser);

	connect(m_pScriptureBrowser, SIGNAL(cursorPositionChanged()), this, SLOT(en_selectionChangedBrowser()));

	// --------------------------------------------------------------

	//	Swapout the widgetDictionary from the layout with
	//		one that we can set the database on:

	ndx = ui.splitterDictionary->indexOf(ui.widgetDictionary);
	assert(ndx != -1);
	if (ndx == -1) return;

	m_pDictionaryWidget = new CDictionaryWidget(pDictionary, this);
	m_pDictionaryWidget->setObjectName(QString::fromUtf8("widgetDictionary"));
	QSizePolicy aSizePolicyDictionary(QSizePolicy::Expanding, QSizePolicy::Expanding);
	aSizePolicyDictionary.setHorizontalStretch(20);
	aSizePolicyDictionary.setVerticalStretch(0);
	aSizePolicyDictionary.setHeightForWidth(m_pDictionaryWidget->sizePolicy().hasHeightForWidth());
	m_pDictionaryWidget->setSizePolicy(aSizePolicyDictionary);
	m_pDictionaryWidget->setToolTip(tr("Dictionary Window Preview"));

	delete ui.widgetDictionary;
	ui.widgetDictionary = NULL;
	ui.splitterDictionary->insertWidget(ndx, m_pDictionaryWidget);

	// --------------------------------------------------------------

	ui.splitterDictionary->setStretchFactor(0, 10);
	ui.splitterDictionary->setStretchFactor(1, 1);

	// --------------------------------------------------------------

	delete ui.buttonWordsOfJesusColor;
	delete ui.buttonSearchResultsColor;
	delete ui.buttonCursorFollowColor;

	ui.buttonWordsOfJesusColor = new QwwColorButton(this);
	ui.buttonWordsOfJesusColor->setObjectName(QString::fromUtf8("buttonWordsOfJesusColor"));
	toQwwColorButton(ui.buttonWordsOfJesusColor)->setShowName(false);			// Must do this before setting our real text
	ui.buttonWordsOfJesusColor->setText(tr("Words of Jesus"));
	ui.buttonWordsOfJesusColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	ui.vertLayoutColorOptions->addWidget(ui.buttonWordsOfJesusColor);

	ui.buttonSearchResultsColor = new QwwColorButton(this);
	ui.buttonSearchResultsColor->setObjectName(QString::fromUtf8("buttonSearchResultsColor"));
	toQwwColorButton(ui.buttonSearchResultsColor)->setShowName(false);			// Must do this before setting our real text
	ui.buttonSearchResultsColor->setText(tr("Search Results"));
	ui.buttonSearchResultsColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	ui.vertLayoutColorOptions->addWidget(ui.buttonSearchResultsColor);

	ui.buttonCursorFollowColor = new QwwColorButton(this);
	ui.buttonCursorFollowColor->setObjectName(QString::fromUtf8("buttonCursorFollowColor"));
	toQwwColorButton(ui.buttonCursorFollowColor)->setShowName(false);			// Must do this before setting our real text
	ui.buttonCursorFollowColor->setText(tr("Cursor Tracker"));
	ui.buttonCursorFollowColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	ui.vertLayoutColorOptions->addWidget(ui.buttonCursorFollowColor);

	connect(toQwwColorButton(ui.buttonWordsOfJesusColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_WordsOfJesusColorPicked(const QColor &)));
	connect(toQwwColorButton(ui.buttonSearchResultsColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_SearchResultsColorPicked(const QColor &)));
	connect(toQwwColorButton(ui.buttonCursorFollowColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_CursorTrackerColorPicked(const QColor &)));

	ui.listWidgetHighlighterColors->setSelectionMode(QAbstractItemView::NoSelection);
	ui.listWidgetHighlighterColors->setSortingEnabled(true);

	ui.comboBoxHighlighters->lineEdit()->setMaxLength(MAX_HIGHLIGHTER_NAME_SIZE);
	ui.toolButtonAddHighlighter->setEnabled(false);
	ui.toolButtonRemoveHighlighter->setEnabled(false);
	ui.toolButtonRenameHighlighter->setEnabled(false);

	connect(ui.comboBoxHighlighters, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(en_comboBoxHighlightersTextChanged(const QString &)));
	connect(ui.comboBoxHighlighters, SIGNAL(editTextChanged(const QString &)), this, SLOT(en_comboBoxHighlightersTextChanged(const QString &)));
	connect(ui.comboBoxHighlighters, SIGNAL(enterPressed()), ui.toolButtonAddHighlighter, SLOT(click()));

	connect(ui.toolButtonAddHighlighter, SIGNAL(clicked()), this, SLOT(en_addHighlighterClicked()));
	connect(ui.toolButtonRemoveHighlighter, SIGNAL(clicked()), this, SLOT(en_removeHighlighterClicked()));
	connect(ui.toolButtonRenameHighlighter, SIGNAL(clicked()), this, SLOT(en_renameHighlighterClicked()));

	connect(ui.listWidgetHighlighterColors, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(en_currentColorListViewItemChanged(QListWidgetItem*, QListWidgetItem*)));

	// --------------------------------------------------------------

	// Reinsert them in the correct TabOrder:
	QWidget::setTabOrder(ui.toolButtonRenameHighlighter, ui.buttonWordsOfJesusColor);
	QWidget::setTabOrder(ui.buttonWordsOfJesusColor, ui.buttonSearchResultsColor);
	QWidget::setTabOrder(ui.buttonSearchResultsColor, ui.buttonCursorFollowColor);
	QWidget::setTabOrder(ui.buttonCursorFollowColor, m_pSearchResultsTreeView);
	QWidget::setTabOrder(m_pSearchResultsTreeView, m_pScriptureBrowser);
	QWidget::setTabOrder(m_pScriptureBrowser, m_pDictionaryWidget);

	// --------------------------------------------------------------

	QList<int> lstStandardFontSizes = QFontDatabase::standardSizes();
	int nFontMin = -1;
	int nFontMax = -1;
	for (int ndx=0; ndx<lstStandardFontSizes.size(); ++ndx) {
		if ((nFontMin == -1) || (lstStandardFontSizes.at(ndx) < nFontMin)) nFontMin = lstStandardFontSizes.at(ndx);
		if ((nFontMax == -1) || (lstStandardFontSizes.at(ndx) > nFontMax)) nFontMax = lstStandardFontSizes.at(ndx);
	}
	ui.dblSpinBoxScriptureBrowserFontSize->setRange(nFontMin, nFontMax);
	ui.dblSpinBoxSearchResultsFontSize->setRange(nFontMin, nFontMax);
	ui.dblSpinBoxDictionaryFontSize->setRange(nFontMin, nFontMax);

	connect(ui.fontComboBoxScriptureBrowser, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_ScriptureBrowserFontChanged(const QFont &)));
	connect(ui.fontComboBoxSearchResults, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_SearchResultsFontChanged(const QFont &)));
	connect(ui.fontComboBoxDictionary, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_DictionaryFontChanged(const QFont &)));
	connect(ui.dblSpinBoxScriptureBrowserFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_ScriptureBrowserFontSizeChanged(double)));
	connect(ui.dblSpinBoxSearchResultsFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_SearchResultsFontSizeChanged(double)));
	connect(ui.dblSpinBoxDictionaryFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_DictionaryFontSizeChanged(double)));

	// --------------------------------------------------------------

	connect(ui.checkBoxInvertTextBrightness, SIGNAL(clicked(bool)), this, SLOT(en_InvertTextBrightnessChanged(bool)));
	connect(ui.horzSliderTextBrigtness, SIGNAL(valueChanged(int)), this, SLOT(en_TextBrightnessChanged(int)));
	connect(ui.checkBoxAdjustDialogElementBrightness, SIGNAL(clicked(bool)), this, SLOT(en_AdjustDialogElementBrightness(bool)));

	// --------------------------------------------------------------

	loadSettings();
}

CKJVTextFormatConfig::~CKJVTextFormatConfig()
{

}

void CKJVTextFormatConfig::loadSettings()
{
	m_bLoadingData = true;

	// --------------------------------------------------------------

	toQwwColorButton(ui.buttonWordsOfJesusColor)->setCurrentColor(CPersistentSettings::instance()->colorWordsOfJesus());
	toQwwColorButton(ui.buttonSearchResultsColor)->setCurrentColor(CPersistentSettings::instance()->colorSearchResults());
	toQwwColorButton(ui.buttonCursorFollowColor)->setCurrentColor(CPersistentSettings::instance()->colorCursorFollow());

	// --------------------------------------------------------------

	ui.listWidgetHighlighterColors->clear();
	ui.comboBoxHighlighters->clear();

	const TUserDefinedColorMap mapHighlighters = g_pUserNotesDatabase->highlighterDefinitionsMap();
	for (TUserDefinedColorMap::const_iterator itrHighlighters = mapHighlighters.constBegin(); itrHighlighters != mapHighlighters.constEnd(); ++itrHighlighters) {
		new CHighlighterColorButton(this, ui.listWidgetHighlighterColors, itrHighlighters.key());
		ui.comboBoxHighlighters->addItem(itrHighlighters.key());
	}
	recalcColorListWidth();

	ui.comboBoxHighlighters->clearEditText();

	// --------------------------------------------------------------

	m_fntScriptureBrowser = CPersistentSettings::instance()->fontScriptureBrowser();
	m_fntSearchResults = CPersistentSettings::instance()->fontSearchResults();
	m_fntDictionary = CPersistentSettings::instance()->fontDictionary();

	ui.fontComboBoxScriptureBrowser->setCurrentFont(m_fntScriptureBrowser);
	ui.fontComboBoxSearchResults->setCurrentFont(m_fntSearchResults);
	ui.fontComboBoxDictionary->setCurrentFont(m_fntDictionary);

	ui.dblSpinBoxScriptureBrowserFontSize->setValue(m_fntScriptureBrowser.pointSizeF());
	ui.dblSpinBoxSearchResultsFontSize->setValue(m_fntSearchResults.pointSizeF());
	ui.dblSpinBoxDictionaryFontSize->setValue(m_fntDictionary.pointSizeF());

	// --------------------------------------------------------------

	m_bInvertTextBrightness = CPersistentSettings::instance()->invertTextBrightness();
	m_nTextBrightness = CPersistentSettings::instance()->textBrightness();
	m_bAdjustDialogElementBrightness = CPersistentSettings::instance()->adjustDialogElementBrightness();

	ui.checkBoxInvertTextBrightness->setChecked(m_bInvertTextBrightness);
	ui.horzSliderTextBrigtness->setValue(m_nTextBrightness);
	ui.checkBoxAdjustDialogElementBrightness->setChecked(m_bAdjustDialogElementBrightness);

	// --------------------------------------------------------------

	m_bLoadingData = false;
	m_bIsDirty = false;

	// --------------------------------------------------------------

	navigateToDemoText();
	setPreview();
}

void CKJVTextFormatConfig::saveSettings()
{
	CPersistentSettings::instance()->setFontScriptureBrowser(m_fntScriptureBrowser);
	CPersistentSettings::instance()->setFontSearchResults(m_fntSearchResults);
	CPersistentSettings::instance()->setFontDictionary(m_fntDictionary);
	CPersistentSettings::instance()->setInvertTextBrightness(m_bInvertTextBrightness);
	CPersistentSettings::instance()->setTextBrightness(m_nTextBrightness);
	CPersistentSettings::instance()->setAdjustDialogElementBrightness(m_bAdjustDialogElementBrightness);
	m_bIsDirty = false;
}

void CKJVTextFormatConfig::en_ScriptureBrowserFontChanged(const QFont &font)
{
	if (m_bLoadingData) return;

	m_fntScriptureBrowser.setFamily(font.family());
	m_pScriptureBrowser->setFont(m_fntScriptureBrowser);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_SearchResultsFontChanged(const QFont &font)
{
	if (m_bLoadingData) return;

	m_fntSearchResults.setFamily(font.family());
	m_pSearchResultsTreeView->setFontSearchResults(m_fntSearchResults);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_DictionaryFontChanged(const QFont &font)
{
	if (m_bLoadingData) return;

	m_fntDictionary.setFamily(font.family());
	m_pDictionaryWidget->setFont(m_fntDictionary);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_ScriptureBrowserFontSizeChanged(double nFontSize)
{
	if (m_bLoadingData) return;

	m_fntScriptureBrowser.setPointSizeF(nFontSize);
	m_pScriptureBrowser->setFont(m_fntScriptureBrowser);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_SearchResultsFontSizeChanged(double nFontSize)
{
	if (m_bLoadingData) return;

	m_fntSearchResults.setPointSizeF(nFontSize);
	m_pSearchResultsTreeView->setFontSearchResults(m_fntSearchResults);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_DictionaryFontSizeChanged(double nFontSize)
{
	if (m_bLoadingData) return;

	m_fntDictionary.setPointSizeF(nFontSize);
	m_pDictionaryWidget->setFont(m_fntDictionary);
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_InvertTextBrightnessChanged(bool bInvert)
{
	if (m_bLoadingData) return;

	m_bInvertTextBrightness = bInvert;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_TextBrightnessChanged(int nBrightness)
{
	if (m_bLoadingData) return;

	m_nTextBrightness = nBrightness;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_AdjustDialogElementBrightness(bool bAdjust)
{
	if (m_bLoadingData) return;

	m_bAdjustDialogElementBrightness = bAdjust;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_WordsOfJesusColorPicked(const QColor &color)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setColorWordsOfJesus(color);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_SearchResultsColorPicked(const QColor &color)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setColorSearchResults(color);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_CursorTrackerColorPicked(const QColor &color)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setColorCursorFollow(color);
//	setPreview();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_HighlighterColorPicked(const QString &strUserDefinedHighlighterName, const QColor &color)
{
	if (m_bLoadingData) return;

	assert(g_pUserNotesDatabase != NULL);
	assert(g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
	g_pUserNotesDatabase->setHighlighterColor(strUserDefinedHighlighterName, color);
	recalcColorListWidth();			// If color was previously invalid and is now valid, we'll have a preview to paint and so the width can change
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
	en_HighlighterColorClicked(strUserDefinedHighlighterName);
}

void CKJVTextFormatConfig::en_HighlighterColorClicked(const QString &strUserDefinedHighlighterName)
{
	if (m_bLoadingData) return;

	ui.comboBoxHighlighters->setEditText(strUserDefinedHighlighterName);
}

void CKJVTextFormatConfig::en_HighlighterEnableChanged(const QString &strUserDefinedHighlighterName, bool bEnabled)
{
	if (m_bLoadingData) return;

	assert(g_pUserNotesDatabase != NULL);
	assert(g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
	g_pUserNotesDatabase->setHighlighterEnabled(strUserDefinedHighlighterName, bEnabled);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
	en_HighlighterColorClicked(strUserDefinedHighlighterName);
}

void CKJVTextFormatConfig::en_comboBoxHighlightersTextChanged(const QString &strUserDefinedHighlighterName)
{
	assert(g_pUserNotesDatabase != NULL);
	ui.toolButtonAddHighlighter->setEnabled(!strUserDefinedHighlighterName.trimmed().isEmpty() && !g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName.trimmed()) &&
													(strUserDefinedHighlighterName.size() <= MAX_HIGHLIGHTER_NAME_SIZE));
	ui.toolButtonRemoveHighlighter->setEnabled(!strUserDefinedHighlighterName.trimmed().isEmpty() && g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName.trimmed()));
	ui.toolButtonRenameHighlighter->setEnabled(!strUserDefinedHighlighterName.trimmed().isEmpty() && g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName.trimmed()));
}

void CKJVTextFormatConfig::en_addHighlighterClicked()
{
	if (m_bLoadingData) return;

	assert(g_pUserNotesDatabase != NULL);
	QString strUserDefinedHighlighterName = ui.comboBoxHighlighters->currentText().trimmed();
	assert(!strUserDefinedHighlighterName.isEmpty() && !g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
	if ((strUserDefinedHighlighterName.isEmpty()) || (g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName))) return;
	if (strUserDefinedHighlighterName.size() > MAX_HIGHLIGHTER_NAME_SIZE) return;

	g_pUserNotesDatabase->setHighlighterColor(strUserDefinedHighlighterName, QColor());
	new CHighlighterColorButton(this, ui.listWidgetHighlighterColors, strUserDefinedHighlighterName);
	ui.comboBoxHighlighters->addItem(strUserDefinedHighlighterName);
	// Note: ComboBox text might change above, so use currentText() here, not strUserDefinedHighlighterName:
	en_comboBoxHighlightersTextChanged(ui.comboBoxHighlighters->currentText());		// Update add/remove controls

	recalcColorListWidth();

	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_removeHighlighterClicked()
{
	if (m_bLoadingData) return;

	assert(g_pUserNotesDatabase != NULL);
	QString strUserDefinedHighlighterName = ui.comboBoxHighlighters->currentText().trimmed();
	assert(!strUserDefinedHighlighterName.isEmpty() && g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
	if ((strUserDefinedHighlighterName.isEmpty()) || (!g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName))) return;

	bool bCantRemove = g_pUserNotesDatabase->existsHighlighterTagsFor(strUserDefinedHighlighterName);

	if (!bCantRemove) {
		g_pUserNotesDatabase->removeHighlighter(strUserDefinedHighlighterName);
		assert(!g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
		int nComboIndex = ui.comboBoxHighlighters->findText(strUserDefinedHighlighterName);
		assert(nComboIndex != -1);
		if (nComboIndex != -1) {
			ui.comboBoxHighlighters->removeItem(nComboIndex);
		}
		// Note: ComboBox text might change above, so use currentText() here, not strUserDefinedHighlighterName:
		en_comboBoxHighlightersTextChanged(ui.comboBoxHighlighters->currentText());		// Update add/remove controls
	}

	int nListWidgetIndex = -1;
	for (int ndx = 0; ndx < ui.listWidgetHighlighterColors->count(); ++ndx) {
		if (static_cast<CHighlighterColorButton *>(ui.listWidgetHighlighterColors->item(ndx))->highlighterName().compare(strUserDefinedHighlighterName) == 0) {
			nListWidgetIndex = ndx;
			break;
		}
	}
	assert(nListWidgetIndex != -1);
	if (nListWidgetIndex != -1) {
		if (!bCantRemove) {
			QListWidgetItem *pItem = ui.listWidgetHighlighterColors->takeItem(nListWidgetIndex);
			delete pItem;
		} else {
			CHighlighterColorButton *pButtonItem = static_cast<CHighlighterColorButton *>(ui.listWidgetHighlighterColors->item(nListWidgetIndex));
			if (pButtonItem->enabled()) {
				int nResult = QMessageBox::information(this, windowTitle(), tr("That highlighter currently has highlighted text associated with it and cannot be removed.  To remove it, "
																			   "use the \"View Highlighters\" mode to display the highlighted passages, select the passages associated "
																			   "with this highlighter, and drag them to a different highlighter.  And then you can return here and remove "
																			   "this highlighter.  Or, open a new King James Notes file.\n\n"
																				"So instead, would you like to disable it so that text highlighted with this Highlighter isn't visible??"),
																		  (QMessageBox::Ok  | QMessageBox::Cancel), QMessageBox::Ok);
				if (nResult == QMessageBox::Ok) pButtonItem->setEnabled(false);
			} else {
				QMessageBox::information(this, windowTitle(), tr("That highlighter currently has highlighted text associated with it and cannot be removed.  To remove it, "
																 "use the \"View Highlighters\" mode to display the highlighted passages, select the passages associated "
																 "with this highlighter, and drag them to a different highlighter.  And then you can return here and remove "
																 "this highlighter.  Or, open a new King James Notes file.  The Highlighter is already disabled so no text "
																 "highlighted with this Highlighter will be visible."), QMessageBox::Ok, QMessageBox::Ok);
			}
			return;		// Note: the setEnabled() call above will take care of updating our demo text and marking us dirty, etc, and nothing should have changed size...
		}
	}

	recalcColorListWidth();

	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVTextFormatConfig::en_renameHighlighterClicked()
{
	if (m_bLoadingData) return;

	assert(g_pUserNotesDatabase != NULL);
	QString strUserDefinedHighlighterName = ui.comboBoxHighlighters->currentText().trimmed();
	assert(!strUserDefinedHighlighterName.isEmpty() && g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
	if ((strUserDefinedHighlighterName.isEmpty()) || (!g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName))) return;

	bool bCantRename = g_pUserNotesDatabase->existsHighlighterTagsFor(strUserDefinedHighlighterName);

	if (!bCantRename) {
		CRenameHighlighterDlg dlgRename(strUserDefinedHighlighterName);

		if (dlgRename.exec() != QDialog::Accepted) return;
		if (g_pUserNotesDatabase->existsHighlighter(dlgRename.newName())) {
			QMessageBox::warning(this, windowTitle(), tr("That highlighter name already exists and can't be used as a new name for this highlighter. "
														 "To try again, click the rename button again. Or, to combine highlighter tags, use the "
														 "\"View Highlighters\" mode to display the highlighted passages, select the passages "
														 "associated with the desired highlighters, and drag them to a different highlighter."));
			return;
		}

		int nListWidgetIndex = -1;
		for (int ndx = 0; ndx < ui.listWidgetHighlighterColors->count(); ++ndx) {
			if (static_cast<CHighlighterColorButton *>(ui.listWidgetHighlighterColors->item(ndx))->highlighterName().compare(strUserDefinedHighlighterName) == 0) {
				nListWidgetIndex = ndx;
				break;
			}
		}
		assert(nListWidgetIndex != -1);
		if (nListWidgetIndex != -1) {
			QListWidgetItem *pItem = ui.listWidgetHighlighterColors->takeItem(nListWidgetIndex);
			delete pItem;
		}
		if (!g_pUserNotesDatabase->renameHighlighter(strUserDefinedHighlighterName, dlgRename.newName())) {
			assert(false);
			return;
		}

		new CHighlighterColorButton(this, ui.listWidgetHighlighterColors, dlgRename.newName());

		int nComboIndex = ui.comboBoxHighlighters->findText(strUserDefinedHighlighterName);
		assert(nComboIndex != -1);
		if (nComboIndex != -1) {
			ui.comboBoxHighlighters->setItemText(nComboIndex, dlgRename.newName());
		}
		ui.comboBoxHighlighters->setCurrentIndex(nComboIndex);
		// Note: ComboBox text might change above, so use currentText() here, not strUserDefinedHighlighterName:
		en_comboBoxHighlightersTextChanged(ui.comboBoxHighlighters->currentText());		// Update add/remove controls

		recalcColorListWidth();

		navigateToDemoText();
		m_bIsDirty = true;
		emit dataChanged();
	} else {
		QMessageBox::warning(this, windowTitle(), tr("That highlighter currently has highlighted text associated with it and cannot be renamed.  "
													 "To rename it, create a new highlighter with the desired name.  Then, use the \"View Highlighters\" "
													 "mode to display the highlighted passages, select the passages associated with this highlighter, "
													 "and drag them to the new highlighter.  And then you can return here and remove this highlighter."));
	}
}

void CKJVTextFormatConfig::en_currentColorListViewItemChanged(QListWidgetItem *pCurrent, QListWidgetItem *pPrevious)
{
	if (m_bLoadingData) return;

	Q_UNUSED(pPrevious);
	if (pCurrent != NULL) {
		CHighlighterColorButton *pButtonItem = static_cast<CHighlighterColorButton *>(pCurrent);
		en_HighlighterColorClicked(pButtonItem->highlighterName());
	}
}

void CKJVTextFormatConfig::recalcColorListWidth()
{
	ui.listWidgetHighlighterColors->setMinimumWidth(0);
	ui.listWidgetHighlighterColors->updateGeometry();
	int nWidth = 0;
	for (int ndx = 0; ndx < ui.listWidgetHighlighterColors->count(); ++ndx) {
		QWidget *pButton = ui.listWidgetHighlighterColors->itemWidget(ui.listWidgetHighlighterColors->item(ndx));
		pButton->updateGeometry();
		pButton->setMinimumWidth(pButton->sizeHint().width());
		ui.listWidgetHighlighterColors->item(ndx)->setSizeHint(pButton->sizeHint());
		nWidth = qMax(nWidth, pButton->sizeHint().width());
	}
	ui.listWidgetHighlighterColors->setMinimumWidth(nWidth + ui.listWidgetHighlighterColors->verticalScrollBar()->sizeHint().width() + ui.listWidgetHighlighterColors->spacing()*2 );
	updateGeometry();
	adjustSize();
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
		// Originally I had a const_iterator over the g_pUserNotesDatabase->highlighterDefinitionsMap, but something
		//		in the process of doHighlighting kept invalidating our iterator and causing an infinite loop.
		//		Solution was to iterate over the buttons in our QListWidget of Highlighter Set buttons.. <sigh>
		for (int ndx = 0; ndx < ui.listWidgetHighlighterColors->count(); ++ndx) {
			CHighlighterColorButton *pColorButton = static_cast<CHighlighterColorButton *>(ui.listWidgetHighlighterColors->item(ndx));
			assert(pColorButton != NULL);
			if (pColorButton == NULL) continue;
			if (!g_pUserNotesDatabase->highlighterDefinition(pColorButton->highlighterName()).isValid()) continue;
			if (!g_pUserNotesDatabase->highlighterEnabled(pColorButton->highlighterName())) continue;
			m_pScriptureBrowser->navigator().doHighlighting(CUserDefinedHighlighter(pColorButton->highlighterName(),
															TPhraseTag(CRelIndex(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->DenormalizeIndex(nNormalizedIndex)), 5)));
			nNormalizedIndex += 7;
		}
	}

	m_previewSearchPhrase.ParsePhrase("trumpet");
	m_previewSearchPhrase.FindWords();
	TParsedPhrasesList lstPhrases;
	lstPhrases.append(&m_previewSearchPhrase);
	CSearchCriteria aSearchCriteria;
	aSearchCriteria.setSearchWithin(m_pSearchResultsTreeView->vlmodel()->bibleDatabase());
	m_pSearchResultsTreeView->setParsedPhrases(aSearchCriteria, lstPhrases);
	m_pSearchResultsTreeView->setDisplayMode(CVerseListModel::VDME_RICHTEXT);
	m_pSearchResultsTreeView->setTreeMode(CVerseListModel::VTME_TREE_CHAPTERS);
	m_pSearchResultsTreeView->expandAll();
}

void CKJVTextFormatConfig::setPreview()
{
	m_pSearchResultsTreeView->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
	m_pScriptureBrowser->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
	m_pDictionaryWidget->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
}

void CKJVTextFormatConfig::en_selectionChangedBrowser()
{
	TPhraseTag tagSelection = m_pScriptureBrowser->selection();

	if ((tagSelection.isSet()) && (tagSelection.count() < 2)) {
		m_pDictionaryWidget->setWord(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->wordAtIndex(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->NormalizeIndex(tagSelection.relIndex())));
	}
}

// ============================================================================
// ============================================================================

CKJVBibleDatabaseConfig::CKJVBibleDatabaseConfig(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QWidget(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	assert(pBibleDatabase != NULL);

	ui.setupUi(this);

	loadSettings();
}

CKJVBibleDatabaseConfig::~CKJVBibleDatabaseConfig()
{

}

void CKJVBibleDatabaseConfig::loadSettings()
{
	m_bLoadingData = true;

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CKJVBibleDatabaseConfig::saveSettings()
{

}

// ============================================================================
// ============================================================================

CKJVUserNotesDatabaseConfig::CKJVUserNotesDatabaseConfig(CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent)
	:	QWidget(parent),
		m_pUserNotesDatabase(pUserNotesDatabase),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	assert(pUserNotesDatabase != NULL);

	ui.setupUi(this);

	connect(ui.btnSetPrimaryUserNotesFilename, SIGNAL(clicked()), this, SLOT(en_clickedSetPrimaryUserNotesFilename()));
	connect(ui.checkBoxKeepBackup, SIGNAL(clicked()), this, SLOT(en_changedKeepBackup()));
	connect(ui.editBackupExtension, SIGNAL(textChanged(const QString &)), this, SLOT(en_changedBackupExtension()));

	loadSettings();
}

CKJVUserNotesDatabaseConfig::~CKJVUserNotesDatabaseConfig()
{

}

void CKJVUserNotesDatabaseConfig::loadSettings()
{
	m_bLoadingData = true;

	ui.editPrimaryUserNotesFilename->setText(m_pUserNotesDatabase->filePathName());
	ui.editBackupExtension->setText(m_pUserNotesDatabase->backupFilenamePostfix().remove(QRegExp("^\\.*")));
	ui.checkBoxKeepBackup->setChecked(m_pUserNotesDatabase->keepBackup());

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CKJVUserNotesDatabaseConfig::saveSettings()
{
	QString strExtension = ui.editBackupExtension->text().trimmed().remove(QRegExp("^\\.*")).trimmed();
	strExtension = "." + strExtension;
	m_pUserNotesDatabase->setKeepBackup(ui.checkBoxKeepBackup->isChecked() && !strExtension.isEmpty());
	m_pUserNotesDatabase->setBackupFilenamePostfix(strExtension);
}

void CKJVUserNotesDatabaseConfig::en_clickedSetPrimaryUserNotesFilename()
{
	if (m_bLoadingData) return;

}

void CKJVUserNotesDatabaseConfig::en_changedKeepBackup()
{
	if (m_bLoadingData) return;
	m_bIsDirty = true;
	emit dataChanged();
}

void CKJVUserNotesDatabaseConfig::en_changedBackupExtension()
{
	if (m_bLoadingData) return;
	m_bIsDirty = true;
	emit dataChanged();
}

// ============================================================================
// ============================================================================

CConfigSearchOptions::CConfigSearchOptions(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	ui.comboSearchPhraseCompleterMode->addItem(tr("Normal Filter"), CSearchCompleter::SCFME_NORMAL);
	ui.comboSearchPhraseCompleterMode->addItem(tr("SoundEx Filter"), CSearchCompleter::SCFME_SOUNDEX);
	ui.comboSearchPhraseCompleterMode->addItem(tr("Unfiltered"), CSearchCompleter::SCFME_UNFILTERED);

	connect(ui.comboSearchPhraseCompleterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSearchPhraseCompleterFilterMode(int)));
	connect(ui.spinSearchPhraseActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedSearchPhraseActivationDelay(int)));

	loadSettings();
}

CConfigSearchOptions::~CConfigSearchOptions()
{

}

void CConfigSearchOptions::loadSettings()
{
	m_bLoadingData = true;

	int nIndex = ui.comboSearchPhraseCompleterMode->findData(CPersistentSettings::instance()->searchPhraseCompleterFilterMode());
	if (nIndex != -1) {
		ui.comboSearchPhraseCompleterMode->setCurrentIndex(nIndex);
	} else {
		assert(false);
	}

	ui.spinSearchPhraseActivationDelay->setValue(CPersistentSettings::instance()->searchActivationDelay());

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CConfigSearchOptions::saveSettings()
{
	int nIndex = ui.comboSearchPhraseCompleterMode->currentIndex();
	if (nIndex != -1) {
		CPersistentSettings::instance()->setSearchPhraseCompleterFilterMode(static_cast<CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM>(ui.comboSearchPhraseCompleterMode->itemData(nIndex).toUInt()));
		m_bIsDirty = false;
	} else {
		assert(false);
	}
	CPersistentSettings::instance()->setSearchActivationDelay(ui.spinSearchPhraseActivationDelay->value());
}

void CConfigSearchOptions::en_changedSearchPhraseCompleterFilterMode(int nIndex)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nIndex);
	m_bIsDirty = true;
	emit dataChanged();
}

void CConfigSearchOptions::en_changedSearchPhraseActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged();
}

// ============================================================================

CConfigBrowserOptions::CConfigBrowserOptions(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	connect(ui.spinBrowserNavigationActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedNavigationActivationDelay(int)));
	connect(ui.spinBrowserPassageReferenceActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedPassageReferenceActivationDelay(int)));

	loadSettings();
}

CConfigBrowserOptions::~CConfigBrowserOptions()
{

}

void CConfigBrowserOptions::loadSettings()
{
	m_bLoadingData = true;

	ui.spinBrowserNavigationActivationDelay->setValue(CPersistentSettings::instance()->navigationActivationDelay());
	ui.spinBrowserPassageReferenceActivationDelay->setValue(CPersistentSettings::instance()->passageReferenceActivationDelay());

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CConfigBrowserOptions::saveSettings()
{
	CPersistentSettings::instance()->setNavigationActivationDelay(ui.spinBrowserNavigationActivationDelay->value());
	CPersistentSettings::instance()->setPassageReferenceActivationDelay(ui.spinBrowserPassageReferenceActivationDelay->value());
	m_bIsDirty = false;
}

void CConfigBrowserOptions::en_changedNavigationActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged();
}

void CConfigBrowserOptions::en_changedPassageReferenceActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged();
}

// ============================================================================

CConfigDictionaryOptions::CConfigDictionaryOptions(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	ui.comboDictionaryCompleterMode->addItem(tr("Normal Filter"), CSearchCompleter::SCFME_NORMAL);
	ui.comboDictionaryCompleterMode->addItem(tr("SoundEx Filter"), CSearchCompleter::SCFME_SOUNDEX);
	ui.comboDictionaryCompleterMode->addItem(tr("Unfiltered"), CSearchCompleter::SCFME_UNFILTERED);

	connect(ui.comboDictionaryCompleterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedDictionaryCompleterFilterMode(int)));
	connect(ui.spinDictionaryActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedDictionaryActivationDelay(int)));

	loadSettings();
}

CConfigDictionaryOptions::~CConfigDictionaryOptions()
{

}

void CConfigDictionaryOptions::loadSettings()
{
	m_bLoadingData = true;

	int nIndex = ui.comboDictionaryCompleterMode->findData(CPersistentSettings::instance()->dictionaryCompleterFilterMode());
	if (nIndex != -1) {
		ui.comboDictionaryCompleterMode->setCurrentIndex(nIndex);
	} else {
		assert(false);
	}

	ui.spinDictionaryActivationDelay->setValue(CPersistentSettings::instance()->dictionaryActivationDelay());

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CConfigDictionaryOptions::saveSettings()
{
	int nIndex = ui.comboDictionaryCompleterMode->currentIndex();
	if (nIndex != -1) {
		CPersistentSettings::instance()->setDictionaryCompleterFilterMode(static_cast<CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM>(ui.comboDictionaryCompleterMode->itemData(nIndex).toUInt()));
		m_bIsDirty = false;
	} else {
		assert(false);
	}
	CPersistentSettings::instance()->setDictionaryActivationDelay(ui.spinDictionaryActivationDelay->value());
}

void CConfigDictionaryOptions::en_changedDictionaryCompleterFilterMode(int nIndex)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nIndex);
	m_bIsDirty = true;
	emit dataChanged();
}

void CConfigDictionaryOptions::en_changedDictionaryActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged();
}

// ============================================================================

CConfigCopyOptions::CConfigCopyOptions(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false),
		m_pEditCopyOptionPreview(NULL)
{
	ui.setupUi(this);

	// ----------

	ui.comboReferenceDelimiterMode->addItem(tr("No Delimiters"), CPhraseNavigator::RDME_NO_DELIMITER);
	ui.comboReferenceDelimiterMode->addItem(tr("Square Brackets"), CPhraseNavigator::RDME_SQUARE_BRACKETS);
	ui.comboReferenceDelimiterMode->addItem(tr("Curly Braces"), CPhraseNavigator::RDME_CURLY_BRACES);
	ui.comboReferenceDelimiterMode->addItem(tr("Parentheses"), CPhraseNavigator::RDME_PARENTHESES);

	connect(ui.comboReferenceDelimiterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedReferenceDelimiterMode(int)));

	// ----------

	connect(ui.checkBoxReferencesUseAbbreviatedBookNames, SIGNAL(clicked(bool)), this, SLOT(en_changedReferencesUseAbbreviatedBookNames(bool)));

	// ----------

	connect(ui.checkBoxReferencesInBold, SIGNAL(clicked(bool)), this, SLOT(en_changedReferencesInBold(bool)));

	// ----------

	ui.comboVerseNumberDelimiterMode->addItem(tr("No Numbers"), CPhraseNavigator::RDME_NO_NUMBER);
	ui.comboVerseNumberDelimiterMode->addItem(tr("No Delimiters"), CPhraseNavigator::RDME_NO_DELIMITER);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Square Brackets"), CPhraseNavigator::RDME_SQUARE_BRACKETS);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Curly Braces"), CPhraseNavigator::RDME_CURLY_BRACES);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Parentheses"), CPhraseNavigator::RDME_PARENTHESES);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Superscript"), CPhraseNavigator::RDME_SUPERSCRIPT);

	connect(ui.comboVerseNumberDelimiterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedVerseNumberDelimiterMode(int)));

	// ----------

	connect(ui.checkBoxVerseNumbersUseAbbreviatedBookNames, SIGNAL(clicked(bool)), this, SLOT(en_changedVerseNumbersUseAbbreviatedBookNames(bool)));

	// ----------

	connect(ui.checkBoxVerseNumbersInBold, SIGNAL(clicked(bool)), this, SLOT(en_changedVerseNumbersInBold(bool)));

	// ----------

	connect(ui.checkBoxAddQuotesAroundVerse, SIGNAL(clicked(bool)), this, SLOT(en_changedAddQuotesAroundVerse(bool)));

	// ----------

	ui.comboTransChangeAddedMode->addItem(tr("No Marking"), CPhraseNavigator::TCAWME_NO_MARKING);
	ui.comboTransChangeAddedMode->addItem(tr("Italics"), CPhraseNavigator::TCAWME_ITALICS);
	ui.comboTransChangeAddedMode->addItem(tr("Brackets"), CPhraseNavigator::TCAWME_BRACKETS);

	connect(ui.comboTransChangeAddedMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedTransChangeAddWordMode(int)));

	// ----------

	loadSettings();
}

CConfigCopyOptions::~CConfigCopyOptions()
{

}

void CConfigCopyOptions::initialize(CBibleDatabasePtr pBibleDatabase)
{
	assert(pBibleDatabase != NULL);
	m_pBibleDatabase = pBibleDatabase;

	// ----------

	//	Swapout the editCopyOptionPreview from the layout with
	//		one that we can set the database on:

	m_pEditCopyOptionPreview = new CScriptureEdit(m_pBibleDatabase, this);
	m_pEditCopyOptionPreview->setObjectName(QString::fromUtf8("editCopyOptionPreview"));
	m_pEditCopyOptionPreview->setMinimumSize(QSize(200, 150));
	m_pEditCopyOptionPreview->setMouseTracking(true);
	m_pEditCopyOptionPreview->setAcceptDrops(false);
	m_pEditCopyOptionPreview->setTabChangesFocus(true);
	m_pEditCopyOptionPreview->setUndoRedoEnabled(false);
	m_pEditCopyOptionPreview->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
	m_pEditCopyOptionPreview->setReadOnly(true);
	m_pEditCopyOptionPreview->setContextMenuPolicy(Qt::DefaultContextMenu);

	delete ui.editCopyOptionPreview;
	ui.editCopyOptionPreview = NULL;
	ui.verticalLayoutMain->addWidget(m_pEditCopyOptionPreview);

	// ----------

	setVerseCopyPreview();
	QTextCursor aCursor(m_pEditCopyOptionPreview->textCursor());
	aCursor.movePosition(QTextCursor::Start);
	m_pEditCopyOptionPreview->setTextCursor(aCursor);
}

void CConfigCopyOptions::loadSettings()
{
	m_bLoadingData = true;

	int nIndex;

	// ----------

	nIndex = ui.comboReferenceDelimiterMode->findData(CPersistentSettings::instance()->referenceDelimiterMode());
	if (nIndex != -1) {
		ui.comboReferenceDelimiterMode->setCurrentIndex(nIndex);
	} else {
		assert(false);
	}

	// ----------

	ui.checkBoxReferencesUseAbbreviatedBookNames->setChecked(CPersistentSettings::instance()->referencesUseAbbreviatedBookNames());

	// ----------

	ui.checkBoxReferencesInBold->setChecked(CPersistentSettings::instance()->referencesInBold());

	// ----------

	nIndex = ui.comboVerseNumberDelimiterMode->findData(CPersistentSettings::instance()->verseNumberDelimiterMode());
	if (nIndex != -1) {
		ui.comboVerseNumberDelimiterMode->setCurrentIndex(nIndex);
	} else {
		assert(false);
	}

	// ----------

	ui.checkBoxVerseNumbersUseAbbreviatedBookNames->setChecked(CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames());

	// ----------

	ui.checkBoxVerseNumbersInBold->setChecked(CPersistentSettings::instance()->verseNumbersInBold());

	// ----------

	ui.checkBoxAddQuotesAroundVerse->setChecked(CPersistentSettings::instance()->addQuotesAroundVerse());

	// ----------

	nIndex = ui.comboTransChangeAddedMode->findData(CPersistentSettings::instance()->transChangeAddWordMode());
	if (nIndex != -1) {
		ui.comboTransChangeAddedMode->setCurrentIndex(nIndex);
	} else {
		assert(false);
	}

	// ----------

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CConfigCopyOptions::saveSettings()
{
	// We've already saved settings in the change notification slots.  Just reset our
	//		our isDirty flag in case we aren't exiting yet and only doing an apply:
	m_bIsDirty = false;
}

void CConfigCopyOptions::en_changedReferenceDelimiterMode(int nIndex)
{
	if (m_bLoadingData) return;

	if (nIndex != -1) {
		CPersistentSettings::instance()->setReferenceDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(ui.comboReferenceDelimiterMode->itemData(nIndex).toUInt()));
	} else {
		assert(false);
	}
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedReferencesUseAbbreviatedBookNames(bool bUseAbbrBookName)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setReferencesUseAbbreviatedBookNames(bUseAbbrBookName);
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedReferencesInBold(bool bInBold)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setReferencesInBold(bInBold);
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedVerseNumberDelimiterMode(int nIndex)
{
	if (m_bLoadingData) return;

	if (nIndex != -1) {
		CPersistentSettings::instance()->setVerseNumberDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(ui.comboVerseNumberDelimiterMode->itemData(nIndex).toUInt()));
	} else {
		assert(false);
	}
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedVerseNumbersUseAbbreviatedBookNames(bool bUseAbbrBookName)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setVerseNumbersUseAbbreviatedBookNames(bUseAbbrBookName);
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedVerseNumbersInBold(bool bInBold)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setVerseNumbersInBold(bInBold);
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedAddQuotesAroundVerse(bool bAddQuotes)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setAddQuotesAroundVerse(bAddQuotes);
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedTransChangeAddWordMode(int nIndex)
{
	if (m_bLoadingData) return;

	if (nIndex != -1) {
		CPersistentSettings::instance()->setTransChangeAddWordMode(static_cast<CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM>(ui.comboTransChangeAddedMode->itemData(nIndex).toUInt()));
	} else {
		assert(false);
	}
	m_bIsDirty = true;
	emit dataChanged();
	setVerseCopyPreview();
}

void CConfigCopyOptions::setVerseCopyPreview()
{
	assert(m_pBibleDatabase != NULL);

	QString strHtml;
	QTextDocument doc;
	CPhraseNavigator navigator(m_pBibleDatabase, doc);
	navigator.setDocumentToFormattedVerses(TPassageTag(CRelIndex(1, 1, 1, 0), 3));
	strHtml += doc.toHtml();
	strHtml += "<hr>\n";
	navigator.setDocumentToFormattedVerses(TPassageTag(CRelIndex(40, 24, 50, 0), 4));
	strHtml += doc.toHtml();
	strHtml += "<hr>\n";
	navigator.setDocumentToFormattedVerses(TPassageTag(CRelIndex(65, 1, 25, 0), 3));
	strHtml += doc.toHtml();
	m_pEditCopyOptionPreview->document()->setHtml(strHtml);
}

// ============================================================================

CKJVGeneralSettingsConfig::CKJVGeneralSettingsConfig(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QWidget(parent)
{
	assert(pBibleDatabase != NULL);

	ui.setupUi(this);

	ui.widgetCopyOptions->initialize(pBibleDatabase);

	connect(ui.widgetSearchOptions, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	connect(ui.widgetBrowserOptions, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	connect(ui.widgetDictionaryOptions, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	connect(ui.widgetCopyOptions, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
}

CKJVGeneralSettingsConfig::~CKJVGeneralSettingsConfig()
{

}

void CKJVGeneralSettingsConfig::loadSettings()
{
	ui.widgetSearchOptions->loadSettings();
	ui.widgetBrowserOptions->loadSettings();
	ui.widgetDictionaryOptions->loadSettings();
	ui.widgetCopyOptions->loadSettings();
}

void CKJVGeneralSettingsConfig::saveSettings()
{
	ui.widgetSearchOptions->saveSettings();
	ui.widgetBrowserOptions->saveSettings();
	ui.widgetDictionaryOptions->saveSettings();
	ui.widgetCopyOptions->saveSettings();
}

bool CKJVGeneralSettingsConfig::isDirty() const
{
	return (ui.widgetSearchOptions->isDirty() || ui.widgetBrowserOptions->isDirty() || ui.widgetDictionaryOptions->isDirty() || ui.widgetCopyOptions->isDirty());
}

// ============================================================================
// ============================================================================

CKJVConfiguration::CKJVConfiguration(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent)
	:	QwwConfigWidget(parent),
		m_pGeneralSettingsConfig(NULL),
		m_pTextFormatConfig(NULL),
		m_pUserNotesDatabaseConfig(NULL),
		m_pBibleDatabaseConfig(NULL)
{
	assert(pBibleDatabase != NULL);
	assert(pDictionary != NULL);
	assert(g_pUserNotesDatabase != NULL);

	m_pGeneralSettingsConfig = new CKJVGeneralSettingsConfig(pBibleDatabase, this);
	m_pTextFormatConfig = new CKJVTextFormatConfig(pBibleDatabase, pDictionary, this);
	m_pUserNotesDatabaseConfig = new CKJVUserNotesDatabaseConfig(g_pUserNotesDatabase, this);
	m_pBibleDatabaseConfig = new CKJVBibleDatabaseConfig(pBibleDatabase, this);

	addGroup(m_pGeneralSettingsConfig, QIcon(":/res/ControlPanel-256.png"), tr("General Settings"));
	addGroup(m_pTextFormatConfig, QIcon(":/res/Font_Graphics_Color_Icon_128.png"), tr("Text Color and Fonts"));
	addGroup(m_pUserNotesDatabaseConfig, QIcon(":/res/Data_management_Icon_128.png"), tr("Notes File Settings"));
	addGroup(m_pBibleDatabaseConfig, QIcon(":/res/Database4-128.png"), tr("Bible Database"));
	setCurrentGroup(m_pGeneralSettingsConfig);

	connect(m_pGeneralSettingsConfig, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	connect(m_pTextFormatConfig, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	connect(m_pUserNotesDatabaseConfig, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	connect(m_pBibleDatabaseConfig, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
}

CKJVConfiguration::~CKJVConfiguration()
{

}

void CKJVConfiguration::loadSettings()
{
	m_pGeneralSettingsConfig->loadSettings();
	m_pTextFormatConfig->loadSettings();
	m_pUserNotesDatabaseConfig->loadSettings();
	m_pBibleDatabaseConfig->loadSettings();
}

void CKJVConfiguration::saveSettings()
{
	m_pGeneralSettingsConfig->saveSettings();
	m_pTextFormatConfig->saveSettings();
	m_pUserNotesDatabaseConfig->saveSettings();
	m_pBibleDatabaseConfig->saveSettings();
}

bool CKJVConfiguration::isDirty() const
{
	return (m_pGeneralSettingsConfig->isDirty() ||
			m_pTextFormatConfig->isDirty() ||
			m_pUserNotesDatabaseConfig->isDirty() ||
			m_pBibleDatabaseConfig->isDirty());
}

// ============================================================================

CKJVConfigurationDialog::CKJVConfigurationDialog(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		m_nLastIndex(-1),
		m_bHandlingPageSwap(false),
		m_pConfiguration(NULL),
		m_pButtonBox(NULL)
{
	assert(pBibleDatabase != NULL);
	assert(pDictionary != NULL);
	assert(g_pUserNotesDatabase != NULL);

	// --------------------------------------------------------------

	// Make a working copy of our settings:
	CPersistentSettings::instance()->togglePersistentSettingData(true);
	g_pUserNotesDatabase->toggleUserNotesDatabaseData(true);

	// --------------------------------------------------------------

	QVBoxLayout *pLayout = new QVBoxLayout(this);
	pLayout->setObjectName(QString::fromUtf8("verticalLayout"));

	m_pConfiguration = new CKJVConfiguration(pBibleDatabase, pDictionary, this);
	m_pConfiguration->setObjectName(QString::fromUtf8("configurationWidget"));
	pLayout->addWidget(m_pConfiguration);

	m_nLastIndex = m_pConfiguration->currentIndex();
	connect(m_pConfiguration, SIGNAL(currentIndexChanged(int)), this, SLOT(en_configurationIndexChanged(int)));

	m_pButtonBox = new QDialogButtonBox(this);
	m_pButtonBox->setObjectName(QString::fromUtf8("buttonBox"));
	m_pButtonBox->setOrientation(Qt::Horizontal);
	m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
	m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	pLayout->addWidget(m_pButtonBox);

	m_pConfiguration->setMinimumWidth(m_pConfiguration->sizeHint().width());
	updateGeometry();

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
	updateGeometry();
	m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(m_pConfiguration->isDirty());
}

void CKJVConfigurationDialog::accept()
{
	CBusyCursor iAmBusy(this);

	m_pConfiguration->saveSettings();
	QDialog::accept();
	// Note: Leave the settings permanent, by not copying
	//		them back in the persistent settings object
}

void CKJVConfigurationDialog::reject()
{
	if (m_pConfiguration->isDirty()) {
		int nResult = QMessageBox::information(this, windowTitle(), tr("You still have unapplied changes.  Do you wish to discard these changes??\n\n"
																	   "Click 'OK' to discard the changes and close this configuration window.\n"
																	   "Click 'Cancel' to stay here in the configuration window."),
																  (QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult == QMessageBox::Cancel) return;
	}
	restore(false);
	QDialog::reject();
}

void CKJVConfigurationDialog::apply()
{
	assert(g_pUserNotesDatabase != NULL);

	CBusyCursor iAmBusy(this);

	// Make sure our persistent settings have been updated, and we'll
	//		copy the settings over to the original, making them permanent
	//		as the user is "applying" them:
	m_pConfiguration->saveSettings();
	CPersistentSettings::instance()->togglePersistentSettingData(true);
	g_pUserNotesDatabase->toggleUserNotesDatabaseData(true);
	en_dataChanged();
}

void CKJVConfigurationDialog::restore(bool bRecopy)
{
	assert(g_pUserNotesDatabase != NULL);

	// Restore original settings by switching back to the original
	//		settings without copying:
	CPersistentSettings::instance()->togglePersistentSettingData(false);
	g_pUserNotesDatabase->toggleUserNotesDatabaseData(false);

	if (bRecopy) {
		// Make a working copy of our settings:
		CPersistentSettings::instance()->togglePersistentSettingData(true);
		g_pUserNotesDatabase->toggleUserNotesDatabaseData(true);
		m_pConfiguration->loadSettings();
		en_dataChanged();
	}
}

// ----------------------------------------------------------------------------

void CKJVConfigurationDialog::en_configurationIndexChanged(int index)
{
	if (m_bHandlingPageSwap) return;

	assert(m_nLastIndex != -1);				// We should have set our initial page index and can never navigate away from some page!
	if (m_nLastIndex == index) return;
	if (!m_pConfiguration->isDirty()) {
		m_nLastIndex = index;
		return;
	}

	m_bHandlingPageSwap = true;

	int nResult = QMessageBox::information(this, windowTitle(), tr("You have changed some settings on the previous page.  Do you wish to apply those settings??\n\n"
																   "Click 'Yes' to apply the setting changes and continue.\n"
																   "Click 'No' to discard those setting changes and continue.\n"
																   "Click 'Cancel' to stay on this settings page."),
															  (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
	if (nResult == QMessageBox::Yes) {
		apply();
	} else if (nResult == QMessageBox::No) {
		restore(true);
	} else {
		// It doesn't work right to call setCurrentIndex() here to set us back
		//		to the last index, because the listWidgetView still hasn't updated
		//		at the time of this call.  However, we can work around it by setting
		//		an event to happen when the event loop runs again, which will trigger
		//		after this current setting process has completed:
		QTimer::singleShot(0, this, SLOT(en_setToLastIndex()));
		return;
	}
	m_nLastIndex = index;

	m_bHandlingPageSwap = false;
}

void CKJVConfigurationDialog::en_setToLastIndex()
{
	assert(m_bHandlingPageSwap);
	assert(m_nLastIndex != -1);
	m_pConfiguration->setCurrentIndex(m_nLastIndex);
	m_bHandlingPageSwap = false;
}

// ============================================================================
