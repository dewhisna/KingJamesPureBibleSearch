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
#include "myApplication.h"
#include "VerseListModel.h"
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
#include "SaveLoadFileDialog.h"
#endif
#include "BibleWordDiffListModel.h"
#include "Translator.h"
#include "BibleDatabaseInfoDlg.h"
#include "DictDatabaseInfoDlg.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QGridLayout>
#include <QSplitter>
#include <QSizePolicy>
#include <QListWidget>
#include <QListWidgetItem>
#include <QwwColorButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QRegExp>
#include <QFileDialog>
#ifdef USING_QT_SPEECH
#include <QtSpeech>
#include <QUrl>
#endif

#ifdef MODELTEST
// For some reason, including the modeltest code causes our wwWidgets QwwColorButton class to
//		no longer be able to import the QColorDialog and you end up with a QColorDialog::getColor()
//		undefined during link.  But, by including an object from QColorDialog, we force it to
//		get included during linking...  Yes, quite a hack...
#include <QColorDialog>
#include <QPointer>
static QPointer<QColorDialog> g_pdlgColor;
#endif

// Originally, we were using QFontDatabase::standardSizes() to get the range of font
//		sizes for all platforms, but starting with Qt 5, the Mac OSX platform started
//		returning 9 to 288 instead of 6 to 72 like all other platforms.  Apparently,
//		these ridiculous sizes were by design from some sort of Mac history.  I
//		reported it to Qt (QTBUG #40057) and they rejected it as invalid.  But 9 to 288
//		is ridiculous and unusable, so I'll just hardcode the sizes, which is apparently
//		what Qt 4 used to do anyway:

#define FONT_MIN_SIZE_APP 6
#define FONT_MAX_SIZE_APP 24

#define FONT_MIN_SIZE 6
#define FONT_MAX_SIZE 144

// ============================================================================

static QwwColorButton *toQwwColorButton(QPushButton *pButton) { return reinterpret_cast<QwwColorButton *>(pButton); }

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
	assert(!g_pUserNotesDatabase.isNull());

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
	m_pEnableCheckbox->setText(CHighlighterColorButtonSignalReflector::tr("Enable", "MainMenu"));
	m_pEnableCheckbox->setToolTip(CHighlighterColorButtonSignalReflector::tr("Enable/Disable this highlighter", "MainMenu"));
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
	assert(!pBibleDatabase.isNull());
	assert(!g_pUserNotesDatabase.isNull());

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
	m_pSearchResultsTreeView->setToolTip(tr("Search Results Preview", "MainMenu"));
	m_pSearchResultsTreeView->setShowHighlightersInSearchResults(false);

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
	m_pScriptureBrowser->setToolTip(tr("Scripture Browser Preview", "MainMenu"));			// Note:  Also disables the "Press Ctrl-D" tooltip, since that mode isn't enable in the configurator

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

	if (!pDictionary.isNull()) {
		m_pDictionaryWidget = new CDictionaryWidget(pDictionary, pBibleDatabase->language(), this);
		m_pDictionaryWidget->setObjectName(QString::fromUtf8("widgetDictionary"));
		QSizePolicy aSizePolicyDictionary(QSizePolicy::Expanding, QSizePolicy::Expanding);
		aSizePolicyDictionary.setHorizontalStretch(20);
		aSizePolicyDictionary.setVerticalStretch(0);
		aSizePolicyDictionary.setHeightForWidth(m_pDictionaryWidget->sizePolicy().hasHeightForWidth());
		m_pDictionaryWidget->setSizePolicy(aSizePolicyDictionary);
		m_pDictionaryWidget->setToolTip(tr("Dictionary Window Preview", "MainMenu"));

		delete ui.widgetDictionary;
		ui.widgetDictionary = NULL;
		ui.splitterDictionary->insertWidget(ndx, m_pDictionaryWidget);
	} else {
		delete ui.widgetDictionary;
		ui.widgetDictionary = NULL;
	}

	// --------------------------------------------------------------

	ui.splitterDictionary->setStretchFactor(0, 10);
	if (ui.splitterDictionary->count() > 1) ui.splitterDictionary->setStretchFactor(1, 1);

	// --------------------------------------------------------------

	delete ui.buttonWordsOfJesusColor;
	delete ui.buttonSearchResultsColor;
	delete ui.buttonCursorFollowColor;

	ui.buttonWordsOfJesusColor = new QwwColorButton(this);
	ui.buttonWordsOfJesusColor->setObjectName(QString::fromUtf8("buttonWordsOfJesusColor"));
	toQwwColorButton(ui.buttonWordsOfJesusColor)->setShowName(false);			// Must do this before setting our real text
	ui.buttonWordsOfJesusColor->setText(tr("Words of Jesus", "MainMenu"));
	ui.buttonWordsOfJesusColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	ui.horzLayoutWordsOfJesusColor->insertWidget(0, ui.buttonWordsOfJesusColor, 10);

	ui.buttonSearchResultsColor = new QwwColorButton(this);
	ui.buttonSearchResultsColor->setObjectName(QString::fromUtf8("buttonSearchResultsColor"));
	toQwwColorButton(ui.buttonSearchResultsColor)->setShowName(false);			// Must do this before setting our real text
	ui.buttonSearchResultsColor->setText(tr("Search Results", "MainMenu"));
	ui.buttonSearchResultsColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	ui.vertLayoutColorOptions->addWidget(ui.buttonSearchResultsColor);

	ui.buttonCursorFollowColor = new QwwColorButton(this);
	ui.buttonCursorFollowColor->setObjectName(QString::fromUtf8("buttonCursorFollowColor"));
	toQwwColorButton(ui.buttonCursorFollowColor)->setShowName(false);			// Must do this before setting our real text
	ui.buttonCursorFollowColor->setText(tr("Cursor Tracker", "MainMenu"));
	ui.buttonCursorFollowColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	ui.vertLayoutColorOptions->addWidget(ui.buttonCursorFollowColor);

	connect(toQwwColorButton(ui.buttonWordsOfJesusColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_WordsOfJesusColorPicked(const QColor &)));
	connect(ui.checkBoxEnableWordsOfJesusColor, SIGNAL(clicked(bool)), this, SLOT(en_clickedEnableWordsOfJesusColor(bool)));
	connect(toQwwColorButton(ui.buttonSearchResultsColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_SearchResultsColorPicked(const QColor &)));
	connect(toQwwColorButton(ui.buttonCursorFollowColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_CursorTrackerColorPicked(const QColor &)));

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
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
#else
	ui.listWidgetHighlighterColors->hide();
	ui.comboBoxHighlighters->hide();
	ui.toolButtonAddHighlighter->hide();
	ui.toolButtonRemoveHighlighter->hide();
	ui.toolButtonRenameHighlighter->hide();
#endif

	// --------------------------------------------------------------

	// Reinsert them in the correct TabOrder:
	QWidget::setTabOrder(ui.toolButtonRenameHighlighter, ui.buttonWordsOfJesusColor);
	QWidget::setTabOrder(ui.buttonWordsOfJesusColor, ui.checkBoxEnableWordsOfJesusColor);
	QWidget::setTabOrder(ui.checkBoxEnableWordsOfJesusColor, ui.buttonSearchResultsColor);
	QWidget::setTabOrder(ui.buttonSearchResultsColor, ui.buttonCursorFollowColor);
	QWidget::setTabOrder(ui.buttonCursorFollowColor, m_pSearchResultsTreeView);
	QWidget::setTabOrder(m_pSearchResultsTreeView, m_pScriptureBrowser);
	if (m_pDictionaryWidget != NULL) QWidget::setTabOrder(m_pScriptureBrowser, m_pDictionaryWidget);

	// --------------------------------------------------------------

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	if (CPersistentSettings::instance()->settings() == NULL) {
		ui.fontComboBoxApplication->setEnabled(false);
		ui.fontComboBoxApplication->setToolTip(tr("Application Font can't be changed in Stealth Mode.  Launch app with -stylesheet to change it instead.", "MainMenu"));
		ui.dblSpinBoxApplicationFontSize->setEnabled(false);
		ui.dblSpinBoxApplicationFontSize->setToolTip(tr("Application Font can't be changed in Stealth Mode.  Launch app with -stylesheet to change it instead.", "MainMenu"));
	}
#endif

	ui.dblSpinBoxApplicationFontSize->setRange(FONT_MIN_SIZE_APP, FONT_MAX_SIZE_APP);
	ui.dblSpinBoxScriptureBrowserFontSize->setRange(FONT_MIN_SIZE, FONT_MAX_SIZE);
	ui.dblSpinBoxSearchResultsFontSize->setRange(FONT_MIN_SIZE, FONT_MAX_SIZE);
	ui.dblSpinBoxDictionaryFontSize->setRange(FONT_MIN_SIZE, FONT_MAX_SIZE);

	connect(ui.fontComboBoxApplication, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_ApplicationFontChanged(const QFont &)));
	connect(ui.fontComboBoxScriptureBrowser, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_ScriptureBrowserFontChanged(const QFont &)));
	connect(ui.fontComboBoxSearchResults, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_SearchResultsFontChanged(const QFont &)));
	connect(ui.fontComboBoxDictionary, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_DictionaryFontChanged(const QFont &)));
	connect(ui.dblSpinBoxApplicationFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_ApplicationFontSizeChanged(double)));
	connect(ui.dblSpinBoxScriptureBrowserFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_ScriptureBrowserFontSizeChanged(double)));
	connect(ui.dblSpinBoxSearchResultsFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_SearchResultsFontSizeChanged(double)));
	connect(ui.dblSpinBoxDictionaryFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_DictionaryFontSizeChanged(double)));

	// --------------------------------------------------------------

	connect(ui.checkBoxInvertTextBrightness, SIGNAL(clicked(bool)), this, SLOT(en_InvertTextBrightnessChanged(bool)));
	connect(ui.horzSliderTextBrigtness, SIGNAL(valueChanged(int)), this, SLOT(en_TextBrightnessChanged(int)));
	connect(ui.checkBoxAdjustDialogElementBrightness, SIGNAL(clicked(bool)), this, SLOT(en_AdjustDialogElementBrightness(bool)));

	// --------------------------------------------------------------

	connect(g_pUserNotesDatabase.data(), SIGNAL(changedUserNotesDatabase()), this, SLOT(en_userNotesChanged()));

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
	ui.buttonWordsOfJesusColor->setEnabled(CPersistentSettings::instance()->colorWordsOfJesus().isValid());
	ui.checkBoxEnableWordsOfJesusColor->setChecked(CPersistentSettings::instance()->colorWordsOfJesus().isValid());
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

	ui.fontComboBoxApplication->setCurrentFont(QApplication::font());
	ui.fontComboBoxScriptureBrowser->setCurrentFont(m_fntScriptureBrowser);
	ui.fontComboBoxSearchResults->setCurrentFont(m_fntSearchResults);
	ui.fontComboBoxDictionary->setCurrentFont(m_fntDictionary);

	ui.dblSpinBoxApplicationFontSize->setValue(QApplication::font().pointSizeF());
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
	// Updating of the SearchResults font needs to be done down in the individual change
	//		notifications.  Without that, the PhraseNavigator::setDocumentToVerse can't
	//		get the correct font to set our preview.  The downside is that having the
	//		change there, it sends an update notification to all of the app and changes
	//		it everywhere.  But without it, the preview doesn't work correctly.  In
	//		anycase, that's why that persistent setting update isn't here in this function...
	CPersistentSettings::instance()->setFontDictionary(m_fntDictionary);
	CPersistentSettings::instance()->setAdjustDialogElementBrightness(m_bAdjustDialogElementBrightness);
	CPersistentSettings::instance()->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);

	// Save application font only if not in stealth mode:
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	if (CPersistentSettings::instance()->settings() != NULL) {
#endif
		QFont fntApp;
		fntApp.setFamily(ui.fontComboBoxApplication->currentFont().family());
		fntApp.setPointSizeF(ui.dblSpinBoxApplicationFontSize->value());
		QApplication::setFont(fntApp);
		CMyApplication::saveApplicationFontSettings();
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	}
#endif

	m_bIsDirty = false;
}

void CKJVTextFormatConfig::en_ApplicationFontChanged(const QFont &font)
{
	if (m_bLoadingData) return;
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	if (CPersistentSettings::instance()->settings() == NULL) return;
#endif

	Q_UNUSED(font);
	m_bIsDirty = true;
	emit dataChanged(true);
}

void CKJVTextFormatConfig::en_ScriptureBrowserFontChanged(const QFont &font)
{
	if (m_bLoadingData) return;

	m_fntScriptureBrowser.setFamily(font.family());
	m_pScriptureBrowser->setFont(m_fntScriptureBrowser);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_SearchResultsFontChanged(const QFont &font)
{
	if (m_bLoadingData) return;

	m_fntSearchResults.setFamily(font.family());
	CPersistentSettings::instance()->setFontSearchResults(m_fntSearchResults);		// Needed to be here instead of saveSettings() so preview works
	m_pSearchResultsTreeView->setFontSearchResults(m_fntSearchResults);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_DictionaryFontChanged(const QFont &font)
{
	if (m_bLoadingData) return;

	m_fntDictionary.setFamily(font.family());
	if (m_pDictionaryWidget != NULL) m_pDictionaryWidget->setFont(m_fntDictionary);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_ApplicationFontSizeChanged(double nFontSize)
{
	if (m_bLoadingData) return;
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	if (CPersistentSettings::instance()->settings() == NULL) return;
#endif

	Q_UNUSED(nFontSize);
	m_bIsDirty = true;
	emit dataChanged(true);
}

void CKJVTextFormatConfig::en_ScriptureBrowserFontSizeChanged(double nFontSize)
{
	if (m_bLoadingData) return;

	m_fntScriptureBrowser.setPointSizeF(nFontSize);
	m_pScriptureBrowser->setFont(m_fntScriptureBrowser);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_SearchResultsFontSizeChanged(double nFontSize)
{
	if (m_bLoadingData) return;

	m_fntSearchResults.setPointSizeF(nFontSize);
	CPersistentSettings::instance()->setFontSearchResults(m_fntSearchResults);		// Needed to be here instead of saveSettings() so preview works
	m_pSearchResultsTreeView->setFontSearchResults(m_fntSearchResults);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_DictionaryFontSizeChanged(double nFontSize)
{
	if (m_bLoadingData) return;

	m_fntDictionary.setPointSizeF(nFontSize);
	if (m_pDictionaryWidget != NULL) m_pDictionaryWidget->setFont(m_fntDictionary);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_InvertTextBrightnessChanged(bool bInvert)
{
	if (m_bLoadingData) return;

	m_bInvertTextBrightness = bInvert;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_TextBrightnessChanged(int nBrightness)
{
	if (m_bLoadingData) return;

	m_nTextBrightness = nBrightness;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_AdjustDialogElementBrightness(bool bAdjust)
{
	if (m_bLoadingData) return;

	m_bAdjustDialogElementBrightness = bAdjust;
	setPreview();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_WordsOfJesusColorPicked(const QColor &color)
{
	if (m_bLoadingData) return;

	ui.buttonWordsOfJesusColor->setEnabled(color.isValid());
	ui.checkBoxEnableWordsOfJesusColor->setChecked(color.isValid());
	CPersistentSettings::instance()->setColorWordsOfJesus(color);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_clickedEnableWordsOfJesusColor(bool bEnable)
{
	if (m_bLoadingData) return;

	if (bEnable) {
		CPersistentSettings::instance()->setColorWordsOfJesus(QColor("red"));
		toQwwColorButton(ui.buttonWordsOfJesusColor)->setCurrentColor(CPersistentSettings::instance()->colorWordsOfJesus());
		ui.buttonWordsOfJesusColor->setEnabled(true);
	} else {
		CPersistentSettings::instance()->setColorWordsOfJesus(QColor());
		toQwwColorButton(ui.buttonWordsOfJesusColor)->setCurrentColor(QColor());
		ui.buttonWordsOfJesusColor->setEnabled(false);
	}
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_SearchResultsColorPicked(const QColor &color)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setColorSearchResults(color);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_CursorTrackerColorPicked(const QColor &color)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setColorCursorFollow(color);
//	setPreview();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_HighlighterColorPicked(const QString &strUserDefinedHighlighterName, const QColor &color)
{
	if (m_bLoadingData) return;

	assert(!g_pUserNotesDatabase.isNull());
	assert(g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
	g_pUserNotesDatabase->setHighlighterColor(strUserDefinedHighlighterName, color);
	recalcColorListWidth();			// If color was previously invalid and is now valid, we'll have a preview to paint and so the width can change
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged(false);
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

	assert(!g_pUserNotesDatabase.isNull());
	assert(g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName));
	g_pUserNotesDatabase->setHighlighterEnabled(strUserDefinedHighlighterName, bEnabled);
	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged(false);
	en_HighlighterColorClicked(strUserDefinedHighlighterName);
}

void CKJVTextFormatConfig::en_comboBoxHighlightersTextChanged(const QString &strUserDefinedHighlighterName)
{
	assert(!g_pUserNotesDatabase.isNull());
	ui.toolButtonAddHighlighter->setEnabled(!strUserDefinedHighlighterName.trimmed().isEmpty() && !g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName.trimmed()) &&
													(strUserDefinedHighlighterName.size() <= MAX_HIGHLIGHTER_NAME_SIZE));
	ui.toolButtonRemoveHighlighter->setEnabled(!strUserDefinedHighlighterName.trimmed().isEmpty() && g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName.trimmed()));
	ui.toolButtonRenameHighlighter->setEnabled(!strUserDefinedHighlighterName.trimmed().isEmpty() && g_pUserNotesDatabase->existsHighlighter(strUserDefinedHighlighterName.trimmed()));
}

void CKJVTextFormatConfig::en_addHighlighterClicked()
{
	if (m_bLoadingData) return;

	assert(!g_pUserNotesDatabase.isNull());
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
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_removeHighlighterClicked()
{
	if (m_bLoadingData) return;

	assert(!g_pUserNotesDatabase.isNull());
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
																				"So instead, would you like to disable it so that text highlighted with this Highlighter isn't visible??", "Errors"),
																		  (QMessageBox::Ok  | QMessageBox::Cancel), QMessageBox::Ok);
				if (nResult == QMessageBox::Ok) pButtonItem->setEnabled(false);
			} else {
				QMessageBox::information(this, windowTitle(), tr("That highlighter currently has highlighted text associated with it and cannot be removed.  To remove it, "
																 "use the \"View Highlighters\" mode to display the highlighted passages, select the passages associated "
																 "with this highlighter, and drag them to a different highlighter.  And then you can return here and remove "
																 "this highlighter.  Or, open a new King James Notes file.  The Highlighter is already disabled so no text "
																 "highlighted with this Highlighter will be visible.", "Errors"), QMessageBox::Ok, QMessageBox::Ok);
			}
			return;		// Note: the setEnabled() call above will take care of updating our demo text and marking us dirty, etc, and nothing should have changed size...
		}
	}

	recalcColorListWidth();

	navigateToDemoText();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTextFormatConfig::en_renameHighlighterClicked()
{
	if (m_bLoadingData) return;

	assert(!g_pUserNotesDatabase.isNull());
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
														 "associated with the desired highlighters, and drag them to a different highlighter.", "Errors"));
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
		emit dataChanged(false);
	} else {
		QMessageBox::warning(this, windowTitle(), tr("That highlighter currently has highlighted text associated with it and cannot be renamed.  "
													 "To rename it, create a new highlighter with the desired name.  Then, use the \"View Highlighters\" "
													 "mode to display the highlighted passages, select the passages associated with this highlighter, "
													 "and drag them to the new highlighter.  And then you can return here and remove this highlighter.", "Errors"));
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
	m_pScriptureBrowser->navigator().setDocumentToChapter(ndxPreview, defaultDocumentToChapterFlags | CPhraseNavigator::TRO_ScriptureBrowser);
	m_pScriptureBrowser->navigator().selectWords(TPhraseTag(ndxPreview));
	m_pScriptureBrowser->navigator().doHighlighting(CSearchResultHighlighter(TPhraseTag(ndxPreview, 5)));
	m_pScriptureBrowser->navigator().doHighlighting(CSearchResultHighlighter(TPhraseTag(ndxPreview2, 32)));

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
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
															TPhraseTag(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->DenormalizeIndex(nNormalizedIndex), 5)));
			nNormalizedIndex += 7;
		}
	}
#endif

	// Search for "trumpet".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strTrumpet = tr("trumpet", "ConfigurationSearchPreviewKeyword");
	TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->language());
	if (!pTranslator.isNull()) {
		QString strTemp = pTranslator->translatorApp().translate("CKJVTextFormatConfig", "trumpet", "ConfigurationSearchPreviewKeyword");
		if (!strTemp.isEmpty()) strTrumpet = strTemp;
	} else if (m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->language().compare("grc") == 0) {
		// Special case for the Greek Textus Receptus texts, since we currently don't have our app translated to Greek
		strTrumpet = "σαλπι*";
	}
	m_previewSearchPhrase.ParsePhrase(strTrumpet);
	m_previewSearchPhrase.FindWords();
	CSearchResultsData aSearchResultsData;
	aSearchResultsData.m_SearchCriteria.setSearchWithin(m_pSearchResultsTreeView->vlmodel()->bibleDatabase());
	aSearchResultsData.m_lstParsedPhrases.append(&m_previewSearchPhrase);
	m_pSearchResultsTreeView->setParsedPhrases(aSearchResultsData);
	m_pSearchResultsTreeView->setDisplayMode(CVerseListModel::VDME_RICHTEXT);
	m_pSearchResultsTreeView->setTreeMode(CVerseListModel::VTME_TREE_CHAPTERS);
	m_pSearchResultsTreeView->expandAll();
}

void CKJVTextFormatConfig::setPreview()
{
	m_pSearchResultsTreeView->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
	m_pScriptureBrowser->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
	if (m_pDictionaryWidget != NULL) m_pDictionaryWidget->setTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
}

void CKJVTextFormatConfig::en_selectionChangedBrowser()
{
	TPhraseTag tagSelection = m_pScriptureBrowser->selection().primarySelection();

	if ((tagSelection.isSet()) && (tagSelection.count() < 2) && (m_pDictionaryWidget != NULL)) {
		m_pDictionaryWidget->setWord(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->wordAtIndex(m_pSearchResultsTreeView->vlmodel()->bibleDatabase()->NormalizeIndex(tagSelection.relIndex())), false);
	}
}

void CKJVTextFormatConfig::en_userNotesChanged()
{
	loadSettings();
}

// ============================================================================
// ============================================================================

CKJVBibleDatabaseConfig::CKJVBibleDatabaseConfig(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	m_pBibleDatabaseListModel = new CBibleDatabaseListModel(ui.treeBibleDatabases);
	ui.treeBibleDatabases->setModel(m_pBibleDatabaseListModel);
	ui.treeBibleDatabases->resizeColumnToContents(0);
	ui.treeBibleDatabases->resizeColumnToContents(1);

#if QT_VERSION >= 0x050000
	ui.treeBibleDatabases->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif

	ui.comboBoxMainBibleDatabaseSelect->setModel(m_pBibleDatabaseListModel);

	m_pBibleWordDiffListModel = new CBibleWordDiffListModel(CBibleDatabasePtr(), ui.treeDatabaseWordChanges);
	ui.treeDatabaseWordChanges->setModel(m_pBibleWordDiffListModel);

	ui.comboBoxHyphenHideMode->clear();
	ui.comboBoxHyphenHideMode->addItem(tr("None", "HyphenModes"), TBibleDatabaseSettings::HHO_None);
	ui.comboBoxHyphenHideMode->addItem(tr("Places/Names", "HyphenModes"), TBibleDatabaseSettings::HHO_ProperWords);
	ui.comboBoxHyphenHideMode->addItem(tr("Ordinary Words", "HyphenModes"), TBibleDatabaseSettings::HHO_OrdinaryWords);
	ui.comboBoxHyphenHideMode->addItem(tr("Both", "HyphenModes"), (TBibleDatabaseSettings::HHO_ProperWords | TBibleDatabaseSettings::HHO_OrdinaryWords));

	connect(ui.treeBibleDatabases->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(en_currentChanged(const QModelIndex &, const QModelIndex &)));
	connect(m_pBibleDatabaseListModel, SIGNAL(loadBibleDatabase(BIBLE_DESCRIPTOR_ENUM)), this, SLOT(en_loadBibleDatabase(BIBLE_DESCRIPTOR_ENUM)));
	connect(m_pBibleDatabaseListModel, SIGNAL(changedAutoLoadStatus(const QString &, bool)), this, SLOT(en_changedAutoLoadStatus(const QString &, bool)));

	connect(ui.comboBoxMainBibleDatabaseSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedMainDBCurrentChanged(int)));

	connect(ui.checkBoxHideHyphens, SIGNAL(clicked(bool)), this, SLOT(en_changedHideHyphens(bool)));
	connect(ui.comboBoxHyphenHideMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedHyphenHideMode(int)));
	connect(ui.checkBoxHyphenSensitive, SIGNAL(clicked(bool)), this, SLOT(en_changedHyphenSensitive(bool)));

	connect(ui.buttonDisplayBibleInfo, SIGNAL(clicked()), this, SLOT(en_displayBibleInformation()));

	setSettingControls();

	loadSettings();
}

CKJVBibleDatabaseConfig::~CKJVBibleDatabaseConfig()
{

}

void CKJVBibleDatabaseConfig::loadSettings()
{
	m_bLoadingData = true;

	ui.treeBibleDatabases->setCurrentIndex(ui.treeBibleDatabases->model()->index(0, 0));
	for (int ndx = 0; ndx < ui.comboBoxMainBibleDatabaseSelect->count(); ++ndx) {
		BIBLE_DESCRIPTOR_ENUM nBibleDB = ui.comboBoxMainBibleDatabaseSelect->itemData(ndx, CBibleDatabaseListModel::BDDRE_BIBLE_DESCRIPTOR_ROLE).value<BIBLE_DESCRIPTOR_ENUM>();
		const TBibleDescriptor &bblDesc = bibleDescriptor(nBibleDB);
		if (CPersistentSettings::instance()->mainBibleDatabaseUUID().compare(bblDesc.m_strUUID, Qt::CaseInsensitive) == 0) {
			ui.comboBoxMainBibleDatabaseSelect->setCurrentIndex(ndx);
			break;
		}
	}

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CKJVBibleDatabaseConfig::saveSettings()
{
	// We've already saved settings in the change notification slots.  Just reset our
	//		our isDirty flag in case we aren't exiting yet and only doing an apply:
	m_bIsDirty = false;

	m_bLoadingData = true;

	// Unload unused Bible Databases:
	for (int ndx = TBibleDatabaseList::instance()->size()-1; ndx >= 0; --ndx) {
		QString strUUID = TBibleDatabaseList::instance()->at(ndx)->compatibilityUUID();
		if ((m_pBibleDatabaseListModel->data(bibleDescriptorFromUUID(strUUID), Qt::CheckStateRole) == Qt::Unchecked) &&
			(TBibleDatabaseList::instance()->mainBibleDatabase() != TBibleDatabaseList::instance()->atUUID(strUUID)) &&		// MainDB check is a safeguard against race condition of changing MainDB selection and the Model Check State
			(!g_pMyApplication.isNull()) && (g_pMyApplication->bibleDatabaseCanOpenerRefCount(strUUID) == 0)) {
			TBibleDatabaseList::instance()->removeBibleDatabase(strUUID);
			continue;
		}
	}
	m_pBibleDatabaseListModel->updateBibleDatabaseList();

	m_bLoadingData = false;

	loadSettings();		// Reload page with new settings
}

void CKJVBibleDatabaseConfig::en_changedHideHyphens(bool bHideHyphens)
{
	if (m_bLoadingData) return;
	if (m_strSelectedDatabaseUUID.isEmpty()) return;

	TBibleDatabaseSettings bdbSettings = CPersistentSettings::instance()->bibleDatabaseSettings(m_strSelectedDatabaseUUID);
	if ((bHideHyphens) && (bdbSettings.hideHyphens() == TBibleDatabaseSettings::HHO_None)) {
		bdbSettings.setHideHyphens(TBibleDatabaseSettings::HHO_ProperWords);
	} else if ((!bHideHyphens) && (bdbSettings.hideHyphens() != TBibleDatabaseSettings::HHO_None)) {
		bdbSettings.setHideHyphens(TBibleDatabaseSettings::HHO_None);
	}
	bool bCanBeSensitive = (!((bdbSettings.hideHyphens() & TBibleDatabaseSettings::HHO_ProperWords) ||
							  (bdbSettings.hideHyphens() & TBibleDatabaseSettings::HHO_OrdinaryWords)));
	if (!bCanBeSensitive) bdbSettings.setHyphenSensitive(false);
	CPersistentSettings::instance()->setBibleDatabaseSettings(m_strSelectedDatabaseUUID, bdbSettings);
	setSettingControls(m_strSelectedDatabaseUUID);

	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVBibleDatabaseConfig::en_changedHyphenHideMode(int index)
{
	if (m_bLoadingData) return;
	if (m_strSelectedDatabaseUUID.isEmpty()) return;

	TBibleDatabaseSettings bdbSettings = CPersistentSettings::instance()->bibleDatabaseSettings(m_strSelectedDatabaseUUID);
	bdbSettings.setHideHyphens(ui.comboBoxHyphenHideMode->itemData(index).toUInt());
	bool bCanBeSensitive = (!((bdbSettings.hideHyphens() & TBibleDatabaseSettings::HHO_ProperWords) ||
							  (bdbSettings.hideHyphens() & TBibleDatabaseSettings::HHO_OrdinaryWords)));
	if (!bCanBeSensitive) bdbSettings.setHyphenSensitive(false);
	CPersistentSettings::instance()->setBibleDatabaseSettings(m_strSelectedDatabaseUUID, bdbSettings);
	setSettingControls(m_strSelectedDatabaseUUID);

	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVBibleDatabaseConfig::en_changedHyphenSensitive(bool bHyphenSensitive)
{
	if (m_bLoadingData) return;
	if (m_strSelectedDatabaseUUID.isEmpty()) return;

	TBibleDatabaseSettings bdbSettings = CPersistentSettings::instance()->bibleDatabaseSettings(m_strSelectedDatabaseUUID);
	bdbSettings.setHyphenSensitive(bHyphenSensitive);
	CPersistentSettings::instance()->setBibleDatabaseSettings(m_strSelectedDatabaseUUID, bdbSettings);
	setSettingControls(m_strSelectedDatabaseUUID);

	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVBibleDatabaseConfig::en_currentChanged(const QModelIndex &indexCurrent, const QModelIndex &indexPrevious)
{
	Q_UNUSED(indexPrevious);
	setSettingControls(m_pBibleDatabaseListModel->data(indexCurrent, CBibleDatabaseListModel::BDDRE_UUID_ROLE).toString());
}

void CKJVBibleDatabaseConfig::setSettingControls(const QString &strUUID)
{
	bool bLoadingData = m_bLoadingData;
	m_bLoadingData = true;

	if (!strUUID.isEmpty()) {
		m_strSelectedDatabaseUUID = strUUID;
	}

	if (m_strSelectedDatabaseUUID.isEmpty()) {
		ui.checkBoxHideHyphens->setEnabled(false);
		ui.checkBoxHideHyphens->setChecked(false);
		ui.comboBoxHyphenHideMode->setEnabled(false);
		ui.comboBoxHyphenHideMode->setCurrentIndex(ui.comboBoxHyphenHideMode->findData(TBibleDatabaseSettings::HHO_None));
		ui.checkBoxHyphenSensitive->setEnabled(false);
		ui.checkBoxHyphenSensitive->setChecked(false);
		ui.buttonDisplayBibleInfo->setEnabled(false);
		m_pBibleWordDiffListModel->setBibleDatabase(CBibleDatabasePtr());
	} else {
		const TBibleDatabaseSettings bdbSettings = CPersistentSettings::instance()->bibleDatabaseSettings(m_strSelectedDatabaseUUID);
		ui.checkBoxHideHyphens->setChecked(bdbSettings.hideHyphens() != TBibleDatabaseSettings::HHO_None);
		ui.comboBoxHyphenHideMode->setCurrentIndex(ui.comboBoxHyphenHideMode->findData(bdbSettings.hideHyphens()));
		ui.checkBoxHyphenSensitive->setChecked(bdbSettings.hyphenSensitive());
		bool bCanBeSensitive = (!((bdbSettings.hideHyphens() & TBibleDatabaseSettings::HHO_ProperWords) ||
								  (bdbSettings.hideHyphens() & TBibleDatabaseSettings::HHO_OrdinaryWords)));
		if (!bCanBeSensitive) {
			assert(bdbSettings.hyphenSensitive() == false);
		}
		ui.checkBoxHideHyphens->setEnabled(true);
		ui.comboBoxHyphenHideMode->setEnabled(true);
		ui.checkBoxHyphenSensitive->setEnabled(bCanBeSensitive);
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(m_strSelectedDatabaseUUID);
		ui.buttonDisplayBibleInfo->setEnabled((!pBibleDatabase.isNull()) && (!pBibleDatabase->info().isEmpty()));
		m_pBibleWordDiffListModel->setBibleDatabase(pBibleDatabase);
	}
	ui.treeDatabaseWordChanges->resizeColumnToContents(0);
	ui.treeDatabaseWordChanges->resizeColumnToContents(1);

	m_bLoadingData = bLoadingData;
}

void CKJVBibleDatabaseConfig::en_loadBibleDatabase(BIBLE_DESCRIPTOR_ENUM nBibleDB)
{
	assert(nBibleDB != BDE_UNKNOWN);
	TBibleDatabaseList::loadBibleDatabase(nBibleDB, false, this);
}

void CKJVBibleDatabaseConfig::en_changedAutoLoadStatus(const QString &strUUID, bool bAutoLoad)
{
	Q_UNUSED(strUUID);
	Q_UNUSED(bAutoLoad);

	assert(!m_bLoadingData);
	if (strUUID.compare(m_strSelectedDatabaseUUID, Qt::CaseInsensitive) == 0) setSettingControls(m_strSelectedDatabaseUUID);		// Changing load status may cause our word-diff preview to change
	ui.treeBibleDatabases->resizeColumnToContents(0);
	ui.treeBibleDatabases->resizeColumnToContents(1);
	setSettingControls();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVBibleDatabaseConfig::en_changedMainDBCurrentChanged(int index)
{
	if (m_bLoadingData) return;

	if (index == -1) return;

	BIBLE_DESCRIPTOR_ENUM nBibleDB = ui.comboBoxMainBibleDatabaseSelect->itemData(index, CBibleDatabaseListModel::BDDRE_BIBLE_DESCRIPTOR_ROLE).value<BIBLE_DESCRIPTOR_ENUM>();
	m_pBibleDatabaseListModel->setData(nBibleDB, true, Qt::CheckStateRole);
	CPersistentSettings::instance()->setMainBibleDatabaseUUID(ui.comboBoxMainBibleDatabaseSelect->itemData(index, CBibleDatabaseListModel::BDDRE_UUID_ROLE).toString());
	setSettingControls();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVBibleDatabaseConfig::en_displayBibleInformation()
{
	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(m_strSelectedDatabaseUUID);
	assert(!pBibleDatabase.isNull());

#ifndef USE_ASYNC_DIALOGS
	CBibleDatabaseInfoDialogPtr pDlg(pBibleDatabase, this);
	pDlg->exec();
#else
	CBibleDatabaseInfoDialog *pDlg = new CBibleDatabaseInfoDialog(pBibleDatabase, this);
	pDlg->show();
#endif
}

// ============================================================================
// ============================================================================

CKJVDictDatabaseConfig::CKJVDictDatabaseConfig(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	m_pDictDatabaseListModel = new CDictDatabaseListModel(ui.treeDictDatabases);
	ui.treeDictDatabases->setModel(m_pDictDatabaseListModel);
	ui.treeDictDatabases->resizeColumnToContents(0);
	ui.treeDictDatabases->resizeColumnToContents(1);

#if QT_VERSION >= 0x050000
	ui.treeDictDatabases->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif

	ui.comboBoxMainDictDatabaseSelect->setModel(m_pDictDatabaseListModel);

	connect(ui.treeDictDatabases->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(en_currentChanged(const QModelIndex &, const QModelIndex &)));
	connect(m_pDictDatabaseListModel, SIGNAL(loadDictDatabase(DICTIONARY_DESCRIPTOR_ENUM)), this, SLOT(en_loadDictDatabase(DICTIONARY_DESCRIPTOR_ENUM)));
	connect(m_pDictDatabaseListModel, SIGNAL(changedAutoLoadStatus(const QString &, bool)), this, SLOT(en_changedAutoLoadStatus(const QString &, bool)));

	connect(ui.comboBoxMainDictDatabaseSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedMainDBCurrentChanged(int)));

	connect(ui.buttonDisplayDictInfo, SIGNAL(clicked()), this, SLOT(en_displayDictInformation()));

	setSettingControls();

	loadSettings();
}

CKJVDictDatabaseConfig::~CKJVDictDatabaseConfig()
{

}

void CKJVDictDatabaseConfig::loadSettings()
{
	m_bLoadingData = true;

	ui.treeDictDatabases->setCurrentIndex(ui.treeDictDatabases->model()->index(0, 0));
	for (int ndx = 0; ndx < ui.comboBoxMainDictDatabaseSelect->count(); ++ndx) {
		DICTIONARY_DESCRIPTOR_ENUM nDictDB = ui.comboBoxMainDictDatabaseSelect->itemData(ndx, CDictDatabaseListModel::DDDRE_DICTIONARY_DESCRIPTOR_ROLE).value<DICTIONARY_DESCRIPTOR_ENUM>();
		const TDictionaryDescriptor &dctDesc = dictionaryDescriptor(nDictDB);
		if (CPersistentSettings::instance()->mainDictDatabaseUUID().compare(dctDesc.m_strUUID, Qt::CaseInsensitive) == 0) {
			ui.comboBoxMainDictDatabaseSelect->setCurrentIndex(ndx);
			break;
		}
	}

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CKJVDictDatabaseConfig::saveSettings()
{
	// We've already saved settings in the change notification slots.  Just reset our
	//		our isDirty flag in case we aren't exiting yet and only doing an apply:
	m_bIsDirty = false;

	m_bLoadingData = true;

	// Unload unused Dictionary Databases:
	for (int ndx = TDictionaryDatabaseList::instance()->size()-1; ndx >= 0; --ndx) {
		QString strUUID = TDictionaryDatabaseList::instance()->at(ndx)->compatibilityUUID();
		if ((m_pDictDatabaseListModel->data(dictionaryDescriptorFromUUID(strUUID), Qt::CheckStateRole) == Qt::Unchecked) &&
			(TDictionaryDatabaseList::instance()->mainDictionaryDatabase() != TDictionaryDatabaseList::instance()->atUUID(strUUID)) &&		// MainDB check is a safeguard against race condition of changing MainDB selection and the Model Check State
			(!g_pMyApplication.isNull()) && (g_pMyApplication->dictDatabaseCanOpenerRefCount(strUUID) == 0)) {
			TDictionaryDatabaseList::instance()->removeDictionaryDatabase(strUUID);
			continue;
		}
	}
	m_pDictDatabaseListModel->updateDictDatabaseList();

	m_bLoadingData = false;

	loadSettings();		// Reload page with new settings
}

void CKJVDictDatabaseConfig::en_currentChanged(const QModelIndex &indexCurrent, const QModelIndex &indexPrevious)
{
	Q_UNUSED(indexPrevious);
	setSettingControls(m_pDictDatabaseListModel->data(indexCurrent, CDictDatabaseListModel::DDDRE_UUID_ROLE).toString());
}

void CKJVDictDatabaseConfig::setSettingControls(const QString &strUUID)
{
	bool bLoadingData = m_bLoadingData;
	m_bLoadingData = true;

	if (!strUUID.isEmpty()) {
		m_strSelectedDatabaseUUID = strUUID;
	}

	if (m_strSelectedDatabaseUUID.isEmpty()) {
		ui.buttonDisplayDictInfo->setEnabled(false);
	} else {
		CDictionaryDatabasePtr pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(m_strSelectedDatabaseUUID);
		ui.buttonDisplayDictInfo->setEnabled((!pDictDatabase.isNull()) && (!pDictDatabase->info().isEmpty()));
	}

	m_bLoadingData = bLoadingData;
}

void CKJVDictDatabaseConfig::en_loadDictDatabase(DICTIONARY_DESCRIPTOR_ENUM nDictDB)
{
	assert(nDictDB != DDE_UNKNOWN);
	TDictionaryDatabaseList::loadDictionaryDatabase(nDictDB, false, this);
}

void CKJVDictDatabaseConfig::en_changedAutoLoadStatus(const QString &strUUID, bool bAutoLoad)
{
	Q_UNUSED(strUUID);
	Q_UNUSED(bAutoLoad);

	assert(!m_bLoadingData);
	if (strUUID.compare(m_strSelectedDatabaseUUID, Qt::CaseInsensitive) == 0) setSettingControls(m_strSelectedDatabaseUUID);		// Changing load status may cause our preview to change
	ui.treeDictDatabases->resizeColumnToContents(0);
	ui.treeDictDatabases->resizeColumnToContents(1);
	setSettingControls();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVDictDatabaseConfig::en_changedMainDBCurrentChanged(int index)
{
	if (m_bLoadingData) return;

	if (index == -1) return;

	DICTIONARY_DESCRIPTOR_ENUM nDictDB = ui.comboBoxMainDictDatabaseSelect->itemData(index, CDictDatabaseListModel::DDDRE_DICTIONARY_DESCRIPTOR_ROLE).value<DICTIONARY_DESCRIPTOR_ENUM>();
	QString strUUID = ui.comboBoxMainDictDatabaseSelect->itemData(index, CDictDatabaseListModel::DDDRE_UUID_ROLE).toString();
	// Must set main dict first so list model will update correctly:
	if (TDictionaryDatabaseList::instance()->atUUID(strUUID).isNull()) TDictionaryDatabaseList::loadDictionaryDatabase(nDictDB, false, this);
	TDictionaryDatabaseList::instance()->setMainDictionaryDatabase(strUUID);
	CPersistentSettings::instance()->setMainDictDatabaseUUID(ui.comboBoxMainDictDatabaseSelect->itemData(index, CDictDatabaseListModel::DDDRE_UUID_ROLE).toString());
	m_pDictDatabaseListModel->setData(nDictDB, m_pDictDatabaseListModel->data(nDictDB, Qt::CheckStateRole), Qt::CheckStateRole);		// Update entry to same check to force status text update
	setSettingControls();
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVDictDatabaseConfig::en_displayDictInformation()
{
	CDictionaryDatabasePtr pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(m_strSelectedDatabaseUUID);
	assert(!pDictDatabase.isNull());

#ifndef USE_ASYNC_DIALOGS
	CDictDatabaseInfoDialogPtr pDlg(pDictDatabase, this);
	pDlg->exec();
#else
	CDictDatabaseInfoDialog *pDlg = new CDictDatabaseInfoDialog(pDictDatabase, this);
	pDlg->show();
#endif
}

// ============================================================================
// ============================================================================

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)

CKJVUserNotesDatabaseConfig::CKJVUserNotesDatabaseConfig(CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent)
	:	QWidget(parent),
		m_pUserNotesDatabase(pUserNotesDatabase),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	assert(!pUserNotesDatabase.isNull());

	ui.setupUi(this);

	int ndx = ui.horizontalLayoutNoteBackgroundColor->indexOf(ui.buttonDefaultNoteBackgroundColor);
	assert(ndx != -1);

	delete ui.buttonDefaultNoteBackgroundColor;

	ui.buttonDefaultNoteBackgroundColor = new QwwColorButton(this);
	ui.buttonDefaultNoteBackgroundColor->setObjectName(QString::fromUtf8("buttonDefaultNoteBackgroundColor"));
	toQwwColorButton(ui.buttonDefaultNoteBackgroundColor)->setShowName(false);		// Must do this before setting our real text
	ui.buttonDefaultNoteBackgroundColor->setText(tr("Default Note Background &Color", "MainMenu"));
	ui.buttonDefaultNoteBackgroundColor->setToolTip(tr("Set the Default Background Color for New Notes", "MainMenu"));
	ui.buttonDefaultNoteBackgroundColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	ui.horizontalLayoutNoteBackgroundColor->insertWidget(ndx, ui.buttonDefaultNoteBackgroundColor);

	connect(ui.btnSetPrimaryUserNotesFilename, SIGNAL(clicked()), this, SLOT(en_clickedSetPrimaryUserNotesFilename()));
	connect(ui.btnStartNewUserNotesFile, SIGNAL(clicked()), this, SLOT(en_clickedStartNewUserNotesFile()));
	connect(ui.editPrimaryUserNotesFilename, SIGNAL(textChanged(const QString &)), this, SLOT(en_changedPrimaryUserNotesFilename(const QString &)));
	connect(ui.checkBoxKeepBackup, SIGNAL(clicked()), this, SLOT(en_changedKeepBackup()));
	connect(ui.editBackupExtension, SIGNAL(textChanged(const QString &)), this, SLOT(en_changedBackupExtension()));

	connect(ui.spinBoxAutoSaveTime, SIGNAL(valueChanged(int)), this, SLOT(en_changedAutoSaveTime(int)));

	connect(toQwwColorButton(ui.buttonDefaultNoteBackgroundColor), SIGNAL(colorPicked(const QColor &)), this, SLOT(en_DefaultNoteBackgroundColorPicked(const QColor &)));

	loadSettings();
	// If initial name was empty, we won't have gotten a change notification, so
	//		handle it manually:
	if (ui.editPrimaryUserNotesFilename->text().isEmpty()) en_changedPrimaryUserNotesFilename(QString());
}

CKJVUserNotesDatabaseConfig::~CKJVUserNotesDatabaseConfig()
{

}

void CKJVUserNotesDatabaseConfig::loadSettings()
{
	m_bLoadingData = true;

	ui.editPrimaryUserNotesFilename->setText(m_pUserNotesDatabase->filePathName());
	ui.editBackupExtension->setText(CPersistentSettings::instance()->notesBackupFilenamePostfix().remove(QRegExp("^\\.*")));
	ui.checkBoxKeepBackup->setChecked(CPersistentSettings::instance()->keepNotesBackup());

	ui.spinBoxAutoSaveTime->setValue(CPersistentSettings::instance()->notesFileAutoSaveTime());

	toQwwColorButton(ui.buttonDefaultNoteBackgroundColor)->setCurrentColor(CPersistentSettings::instance()->colorDefaultNoteBackground());

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CKJVUserNotesDatabaseConfig::saveSettings()
{
	QString strExtension = ui.editBackupExtension->text().trimmed().remove(QRegExp("^\\.*")).trimmed();
	strExtension = "." + strExtension;
	CPersistentSettings::instance()->setKeepNotesBackup(ui.checkBoxKeepBackup->isChecked() && !strExtension.isEmpty());
	CPersistentSettings::instance()->setNotesBackupFilenamePostfix(strExtension);
	CPersistentSettings::instance()->setNotesFileAutoSaveFile(ui.spinBoxAutoSaveTime->value());
	CPersistentSettings::instance()->setColorDefaultNoteBackground(toQwwColorButton(ui.buttonDefaultNoteBackgroundColor)->currentColor());
	m_bIsDirty = false;
}

void CKJVUserNotesDatabaseConfig::en_clickedSetPrimaryUserNotesFilename()
{
	if (m_bLoadingData) return;

	int nResult;
	bool bPromptFilename = false;
	bool bDone = false;

	if (m_pUserNotesDatabase->isDirty()) {
		// If we don't have a file name, yet made some change to the KJN, prompt them for a path:
		if (m_pUserNotesDatabase->filePathName().isEmpty()) {
			if (m_pUserNotesDatabase->errorFilePathName().isEmpty()) {
				// If we don't have a filename at all, prompt for new setup:
				nResult = QMessageBox::warning(this, windowTitle(), tr("You have edited Notes, Highlighters, and/or References, but don't yet have a King James Notes File setup.\n\n"
																		 "Do you wish to setup a Notes File and save your changes??\nWarning: If you select 'No', then your changes will be lost.", "Errors"),
														(QMessageBox::Yes  | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
			} else {
				// If we originally had a filename, but failed in opening it, just prompt the user about saving it since it's
				//		possible they don't want to attempt to overwrite the one that failed since we couldn't load it:
				nResult = QMessageBox::warning(this, windowTitle(), tr("The previous attempt to load your King James Notes File failed.\n"
																	   "Do you wish to save the changes you've made?\n"
																	   "Warning, if you save this file overtop of your original file, you will "
																	   "lose all ability to recover the remaining data in your original file.  It's "
																	   "recommended that you save it to a new file.\n\n"
																	   "Click 'Yes' to enter a filename and save your new changes, or\n"
																	   "Click 'No' to lose your changes and continue on to Select a Notes File to Load (recommended if you haven't really done any editing), or\n"
																	   "Click 'Cancel' to return to King James Pure Bible Search...", "Errors"),
														(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
			}
			// If the user cancelled, return:
			if ((nResult != QMessageBox::Yes) && (nResult != QMessageBox::No)) {
				return;
			}
			// If they want to save, but don't have path yet, so we need to prompt them for a path:
			if (nResult == QMessageBox::Yes) {
				bPromptFilename = true;
			}
		}
		// If the user has a file path already (or is wanting to create a new one), try to save it:
		if ((!m_pUserNotesDatabase->filePathName().isEmpty()) || (bPromptFilename)) {
			bDone = false;
			do {
				if (bPromptFilename) {
					QString strFilePathName = CSaveLoadFileDialog::getSaveFileName(this, tr("Save King James Notes File", "FileFilters"), m_pUserNotesDatabase->errorFilePathName(), tr("King James Notes Files (*.kjn)", "FileFilters"), "kjn", NULL, 0);
					if (!strFilePathName.isEmpty()) {
						m_pUserNotesDatabase->setFilePathName(strFilePathName);
						ui.editPrimaryUserNotesFilename->setText(m_pUserNotesDatabase->filePathName());
					} else {
						// If the user aborted treating after the path after all:
						return;
					}
				}

				if (!m_pUserNotesDatabase->save()) {
					nResult = QMessageBox::warning(this, tr("King James Notes File Error", "Errors"),  m_pUserNotesDatabase->lastLoadSaveError() + QString("\n\n") +
														tr("Unable to save the King James Notes File!\n\n"
														   "Click 'Yes' to try again, or\n"
														   "Click 'No' to lose your changes and continue on to Select a Notes File to Load, or\n"
														   "Click 'Cancel' to return to King James Pure Bible Search...", "Errors"),
												   (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
					// If the user cancelled, return back to the app:
					if ((nResult != QMessageBox::Yes) && (nResult != QMessageBox::No)) {
						return;
					}
					// If they want to lose their changes, break out of loop:
					if (nResult == QMessageBox::No) {
						bDone = true;
					}
					// Set our error file path in case we are prompting the user in the loop:
					m_pUserNotesDatabase->setErrorFilePathName(m_pUserNotesDatabase->filePathName());
				} else {
					// If the save was successful, break out of loop:
					bDone = true;
					if (bPromptFilename) return;			// If we prompted the user for the filename, we've already set it above and can exit, we're done...
				}
			} while (!bDone);
		}
		// Either the user aborted creating the User Notes File or the User Notes File Saved OK....
	}	//	(or we didn't have an updated file to save)...

	bool bLoadFailed = false;
	bDone = false;
	while (!bDone) {
		QString strNewFilePathName = m_pUserNotesDatabase->errorFilePathName();
		if (strNewFilePathName.isEmpty()) strNewFilePathName = m_pUserNotesDatabase->filePathName();
		strNewFilePathName = CSaveLoadFileDialog::getOpenFileName(this, tr("Load King James Notes File", "FileFilters"), strNewFilePathName, tr("King James Notes File (*.kjn)", "FileFilters"), NULL, 0);
		if (strNewFilePathName.isEmpty()) {		// Empty if user cancels
			if (bLoadFailed) {
				// If our previous load failed, we now have an uninitalized notes file.  So,
				//		let's just initialize like a new Notes File.  Note the edit box will
				//		already show the empty filename from the path below:
				m_pUserNotesDatabase->initUserNotesDatabaseData();				// Note, this will leave dirty as false until user actually edits the notes file (as it should be)
				m_pUserNotesDatabase->toggleUserNotesDatabaseData(true);		// Do a pseudo-copy.  This will trigger the equivalent of an apply since they have to change (it isn't a cancelable option)
			}
			return;
		}

		m_pUserNotesDatabase->setFilePathName(strNewFilePathName);
		ui.editPrimaryUserNotesFilename->setText(m_pUserNotesDatabase->filePathName());

		if (!loadUserNotesDatabase()) {
			QMessageBox::warning(this, tr("King James Notes File Error", "Errors"),  m_pUserNotesDatabase->lastLoadSaveError());
			// Leave the isDirty flag set, but clear the filename to force the user to re-navigate to
			//		it, or else we may accidentally overwrite the file if it happens to be "fixed" by
			//		the time we exit.  But save a reference to it so we can get the user navigated back there:
			m_pUserNotesDatabase->setErrorFilePathName(m_pUserNotesDatabase->filePathName());
			m_pUserNotesDatabase->setFilePathName(QString());
			ui.editPrimaryUserNotesFilename->setText(m_pUserNotesDatabase->filePathName());
			bLoadFailed = true;
			continue;
		} else {
			if (m_pUserNotesDatabase->version() < KJN_FILE_VERSION) {
				QMessageBox::warning(this, tr("Loading King James Notes File", "Errors"), tr("Warning: The King James Notes File being loaded was last saved on "
											"an older version of King James Pure Bible Search.  It will automatically be updated to this version of "
											"King James Pure Bible Search.  However, if you wish to keep a copy of your Notes File in the old format, you must "
											"manually save a copy of your file now BEFORE you continue!\n\nFilename: \"%1\"", "Errors").arg(m_pUserNotesDatabase->filePathName()));
			} else if (m_pUserNotesDatabase->version() > KJN_FILE_VERSION) {
				QMessageBox::warning(this, tr("Loading King James Notes File", "Errors"), tr("Warning: The King James Notes File being loaded was created on "
											"a newer version of King James Pure Bible Search.  It may contain data or settings for things not "
											"supported on this version of King James Pure Bible Search.  If so, those new things will be LOST the "
											"next time your Notes Files is saved.  If you wish to keep a copy of your original Notes File and not "
											"risk losing any data from it, you must manually save a copy of your file now BEFORE you continue!"
											"\n\nFilename: \"%1\"", "Errors").arg(m_pUserNotesDatabase->filePathName()));
			}
			bDone = true;
		}
	}

	m_pUserNotesDatabase->toggleUserNotesDatabaseData(true);		// Do a pseudo-copy.  This will trigger the equivalent of an apply since they have to change (it isn't a cancelable option)
}

bool CKJVUserNotesDatabaseConfig::loadUserNotesDatabase()
{
	CBusyCursor iAmBusy(NULL);
	return m_pUserNotesDatabase->load();
}

void CKJVUserNotesDatabaseConfig::en_clickedStartNewUserNotesFile()
{
	if ((m_pUserNotesDatabase->filePathName().isEmpty()) || (ui.editPrimaryUserNotesFilename->text().isEmpty())) return;

	bool bDone = false;
	while (!bDone) {
		if ((m_pUserNotesDatabase->isDirty()) && (!m_pUserNotesDatabase->save())) {
			int nResult = QMessageBox::warning(this, tr("King James Notes File Error", "Errors"),  m_pUserNotesDatabase->lastLoadSaveError() + QString("\n\n") +
													tr("Unable to save the current King James Notes File!\n\n"
													   "Click 'Yes' to try again, or\n"
													   "Click 'No' to lose your changes and continue on to Select a Notes File to Load, or\n"
													   "Click 'Cancel' to return to King James Pure Bible Search...", "Errors"),
											   (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Yes);
			// If the user cancelled, return back to the app:
			if ((nResult != QMessageBox::Yes) && (nResult != QMessageBox::No)) {
				return;
			}
			// If they want to lose their changes, break out of loop:
			if (nResult == QMessageBox::No) {
				bDone = true;
			}
		} else {
			bDone = true;
		}
	}

	CBusyCursor iAmBusy(NULL);

	m_pUserNotesDatabase->setErrorFilePathName(QString());
	m_pUserNotesDatabase->setFilePathName(QString());
	ui.editPrimaryUserNotesFilename->setText(m_pUserNotesDatabase->filePathName());
	m_pUserNotesDatabase->initUserNotesDatabaseData();				// Note, this will leave dirty as false until user actually edits the notes file (as it should be)
	m_pUserNotesDatabase->toggleUserNotesDatabaseData(true);		// Do a pseudo-copy.  This will trigger the equivalent of an apply since they have to change (it isn't a cancelable option)
}

void CKJVUserNotesDatabaseConfig::en_changedPrimaryUserNotesFilename(const QString &strFilename)
{
	bool bHadFocus = ui.btnStartNewUserNotesFile->hasFocus();
	ui.btnStartNewUserNotesFile->setEnabled(!strFilename.isEmpty());
	if ((bHadFocus) && (strFilename.isEmpty())) ui.btnSetPrimaryUserNotesFilename->setFocus();		// If the Start New Button had focus (i.e. user clicked it), pass focus off to the Set/Load button
}

void CKJVUserNotesDatabaseConfig::en_changedKeepBackup()
{
	if (m_bLoadingData) return;
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVUserNotesDatabaseConfig::en_changedBackupExtension()
{
	if (m_bLoadingData) return;
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVUserNotesDatabaseConfig::en_changedAutoSaveTime(int nAutoSaveTime)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nAutoSaveTime);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVUserNotesDatabaseConfig::en_DefaultNoteBackgroundColorPicked(const QColor &color)
{
	if (m_bLoadingData) return;

	Q_UNUSED(color);
	m_bIsDirty = true;
	emit dataChanged(false);
}

#endif	// !EMSCRIPTEN && !VNCSERVER

// ============================================================================
// ============================================================================

CConfigSearchOptions::CConfigSearchOptions(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	ui.comboSearchPhraseCompleterMode->addItem(tr("Normal Filter", "SoundExModes"), CSearchCompleter::SCFME_NORMAL);
	ui.comboSearchPhraseCompleterMode->addItem(tr("SoundEx Filter", "SoundExModes"), CSearchCompleter::SCFME_SOUNDEX);
	ui.comboSearchPhraseCompleterMode->addItem(tr("Unfiltered", "SoundExModes"), CSearchCompleter::SCFME_UNFILTERED);

#ifndef USE_SEARCH_PHRASE_COMPLETER_POPUP_DELAY
	ui.lblAutoCompleterActivationDelay->setEnabled(false);
	ui.spinAutoCompleterActivationDelay->setEnabled(false);
#endif

	connect(ui.comboSearchPhraseCompleterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSearchPhraseCompleterFilterMode(int)));
	connect(ui.spinSearchPhraseActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedSearchPhraseActivationDelay(int)));
	connect(ui.spinAutoCompleterActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedAutoCompleterActivationDelay(int)));
	connect(ui.spinInitialNumberOfSearchPhrases, SIGNAL(valueChanged(int)), this, SLOT(en_changedInitialNumberOfSearchPhrases(int)));
	connect(ui.checkBoxHideMatchingPhrasesLists, SIGNAL(clicked(bool)), this, SLOT(en_changedHideMatchingPhrasesLists(bool)));
	connect(ui.checkBoxAutoExpandSearchResultsTree, SIGNAL(clicked(bool)), this, SLOT(en_changedAutoExpandSearchResultsTree(bool)));
	connect(ui.checkBoxHideNotFoundInStatistics, SIGNAL(clicked(bool)), this, SLOT(en_changedHideNotFoundInStatistics(bool)));

	loadSettings();
}

CConfigSearchOptions::~CConfigSearchOptions()
{

}

void CConfigSearchOptions::loadSettings()
{
	bool bKeepDirty = false;

	m_bLoadingData = true;

	int nIndex = ui.comboSearchPhraseCompleterMode->findData(CPersistentSettings::instance()->searchPhraseCompleterFilterMode());
	if (nIndex != -1) {
		ui.comboSearchPhraseCompleterMode->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboSearchPhraseCompleterMode->setCurrentIndex(0);
	}
	ui.spinSearchPhraseActivationDelay->setValue(CPersistentSettings::instance()->searchActivationDelay());
	ui.spinAutoCompleterActivationDelay->setValue(CPersistentSettings::instance()->autoCompleterActivationDelay());
	ui.spinInitialNumberOfSearchPhrases->setValue(CPersistentSettings::instance()->initialNumberOfSearchPhrases());
	ui.checkBoxHideMatchingPhrasesLists->setChecked(CPersistentSettings::instance()->hideMatchingPhrasesLists());
	ui.checkBoxAutoExpandSearchResultsTree->setChecked(CPersistentSettings::instance()->autoExpandSearchResultsTree());
	ui.checkBoxHideNotFoundInStatistics->setChecked(CPersistentSettings::instance()->hideNotFoundInStatistcs());

	m_bLoadingData = false;
	m_bIsDirty = bKeepDirty;
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
	CPersistentSettings::instance()->setAutoCompleterActivationDelay(ui.spinAutoCompleterActivationDelay->value());
	CPersistentSettings::instance()->setInitialNumberOfSearchPhrases(ui.spinInitialNumberOfSearchPhrases->value());
	CPersistentSettings::instance()->setHideMatchingPhrasesLists(ui.checkBoxHideMatchingPhrasesLists->isChecked());
	CPersistentSettings::instance()->setAutoExpandSearchResultsTree(ui.checkBoxAutoExpandSearchResultsTree->isChecked());
	CPersistentSettings::instance()->setHideNotFoundInStatistics(ui.checkBoxHideNotFoundInStatistics->isChecked());
}

void CConfigSearchOptions::en_changedSearchPhraseCompleterFilterMode(int nIndex)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nIndex);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigSearchOptions::en_changedSearchPhraseActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigSearchOptions::en_changedAutoCompleterActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigSearchOptions::en_changedInitialNumberOfSearchPhrases(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigSearchOptions::en_changedHideMatchingPhrasesLists(bool bHideMatchingPhrasesLists)
{
	if (m_bLoadingData) return;

	Q_UNUSED(bHideMatchingPhrasesLists);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigSearchOptions::en_changedAutoExpandSearchResultsTree(bool bAutoExpandSearchResultsTree)
{
	if (m_bLoadingData) return;

	Q_UNUSED(bAutoExpandSearchResultsTree);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigSearchOptions::en_changedHideNotFoundInStatistics(bool bHideNotFoundInStatistics)
{
	if (m_bLoadingData) return;

	Q_UNUSED(bHideNotFoundInStatistics);
	m_bIsDirty = true;
	emit dataChanged(false);
}

// ============================================================================

CConfigBrowserOptions::CConfigBrowserOptions(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	ui.comboBoxChapterScrollbarMode->addItem(tr("None", "ScrollbarModes"), CSME_NONE);
	ui.comboBoxChapterScrollbarMode->addItem(tr("Left-Side", "ScrollbarModes"), CSME_LEFT);
	ui.comboBoxChapterScrollbarMode->addItem(tr("Right-Side", "ScrollbarModes"), CSME_RIGHT);

	ui.comboBoxVerseRenderingMode->addItem(tr("Free-Flow/Paragraph", "VerseRenderingModes"), CPhraseNavigator::VRME_FF);
	ui.comboBoxVerseRenderingMode->addItem(tr("Verse-Per-Line", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL);
	ui.comboBoxVerseRenderingMode->addItem(tr("Verse-Per-Line Double-Spaced", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL_DS);
	ui.comboBoxVerseRenderingMode->addItem(tr("Verse-Per-Line with Indent", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL_INDENT);
	ui.comboBoxVerseRenderingMode->addItem(tr("Verse-Per-Line with Hanging Indent", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL_HANGING);
	ui.comboBoxVerseRenderingMode->addItem(tr("Verse-Per-Line Double-Spaced with Indent", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL_DS_INDENT);
	ui.comboBoxVerseRenderingMode->addItem(tr("Verse-Per-Line Double-Spaced with Hanging Indent", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL_DS_HANGING);

	connect(ui.spinBrowserNavigationActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedNavigationActivationDelay(int)));
	connect(ui.spinBrowserPassageReferenceActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedPassageReferenceActivationDelay(int)));
	connect(ui.checkBoxShowExcludedSearchResults, SIGNAL(clicked(bool)), this, SLOT(en_changedShowExcludedSearchResults(bool)));
	connect(ui.comboBoxChapterScrollbarMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedChapterScrollbarMode(int)));
	connect(ui.comboBoxVerseRenderingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedVerseRenderingMode(int)));
	connect(ui.spinBoxLineHeight, SIGNAL(valueChanged(double)), this, SLOT(en_changedLineHeight(double)));
	connect(ui.checkBoxShowPilcrowMarkers, SIGNAL(clicked(bool)), this, SLOT(en_changedShowPilcrowMarkers(bool)));

	loadSettings();
}

CConfigBrowserOptions::~CConfigBrowserOptions()
{

}

void CConfigBrowserOptions::loadSettings()
{
	bool bKeepDirty = false;

	m_bLoadingData = true;

	ui.spinBrowserNavigationActivationDelay->setValue(CPersistentSettings::instance()->navigationActivationDelay());
	ui.spinBrowserPassageReferenceActivationDelay->setValue(CPersistentSettings::instance()->passageReferenceActivationDelay());
	ui.checkBoxShowExcludedSearchResults->setChecked(CPersistentSettings::instance()->showExcludedSearchResultsInBrowser());

	int nIndex = ui.comboBoxChapterScrollbarMode->findData(CPersistentSettings::instance()->chapterScrollbarMode());
	if (nIndex != -1) {
		ui.comboBoxChapterScrollbarMode->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboBoxChapterScrollbarMode->setCurrentIndex(0);
	}

	nIndex = ui.comboBoxVerseRenderingMode->findData(CPersistentSettings::instance()->verseRenderingMode());
	if (nIndex != -1) {
		ui.comboBoxVerseRenderingMode->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboBoxVerseRenderingMode->setCurrentIndex(0);
	}

	ui.spinBoxLineHeight->setValue(CPersistentSettings::instance()->scriptureBrowserLineHeight());

	ui.checkBoxShowPilcrowMarkers->setChecked(CPersistentSettings::instance()->showPilcrowMarkers());

	m_bLoadingData = false;
	m_bIsDirty = bKeepDirty;
}

void CConfigBrowserOptions::saveSettings()
{
	CPersistentSettings::instance()->setNavigationActivationDelay(ui.spinBrowserNavigationActivationDelay->value());
	CPersistentSettings::instance()->setPassageReferenceActivationDelay(ui.spinBrowserPassageReferenceActivationDelay->value());
	CPersistentSettings::instance()->setShowExcludedSearchResultsInBrowser(ui.checkBoxShowExcludedSearchResults->isChecked());
	int nIndex = ui.comboBoxChapterScrollbarMode->currentIndex();
	if (nIndex != -1) {
		CPersistentSettings::instance()->setChapterScrollbarMode(static_cast<CHAPTER_SCROLLBAR_MODE_ENUM>(ui.comboBoxChapterScrollbarMode->itemData(nIndex).toUInt()));
		m_bIsDirty = false;
	} else {
		assert(false);
	}
	nIndex = ui.comboBoxVerseRenderingMode->currentIndex();
	if (nIndex != -1) {
		CPersistentSettings::instance()->setVerseRenderingMode(static_cast<CPhraseNavigator::VERSE_RENDERING_MODE_ENUM>(ui.comboBoxVerseRenderingMode->itemData(nIndex).toUInt()));
	} else {
		assert(false);
	}
	CPersistentSettings::instance()->setScriptureBrowserLineHeight(ui.spinBoxLineHeight->value());
	CPersistentSettings::instance()->setShowPilcrowMarkers(ui.checkBoxShowPilcrowMarkers->isChecked());
	m_bIsDirty = false;
}

void CConfigBrowserOptions::en_changedNavigationActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigBrowserOptions::en_changedPassageReferenceActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigBrowserOptions::en_changedShowExcludedSearchResults(bool bShowExcludedSearchResults)
{
	if (m_bLoadingData) return;

	Q_UNUSED(bShowExcludedSearchResults);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigBrowserOptions::en_changedChapterScrollbarMode(int nIndex)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nIndex);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigBrowserOptions::en_changedVerseRenderingMode(int nIndex)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nIndex);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigBrowserOptions::en_changedLineHeight(double nLineHeight)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nLineHeight);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigBrowserOptions::en_changedShowPilcrowMarkers(bool bShowPilcrowMarkers)
{
	if (m_bLoadingData) return;

	Q_UNUSED(bShowPilcrowMarkers);
	m_bIsDirty = true;
	emit dataChanged(false);
}

// ============================================================================

CConfigDictionaryOptions::CConfigDictionaryOptions(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	ui.comboDictionaryCompleterMode->addItem(tr("Normal Filter", "SoundExModes"), CSearchCompleter::SCFME_NORMAL);
	ui.comboDictionaryCompleterMode->addItem(tr("SoundEx Filter", "SoundExModes"), CSearchCompleter::SCFME_SOUNDEX);
	ui.comboDictionaryCompleterMode->addItem(tr("Unfiltered", "SoundExModes"), CSearchCompleter::SCFME_UNFILTERED);

	connect(ui.comboDictionaryCompleterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedDictionaryCompleterFilterMode(int)));
	connect(ui.spinDictionaryActivationDelay, SIGNAL(valueChanged(int)), this, SLOT(en_changedDictionaryActivationDelay(int)));

	loadSettings();
}

CConfigDictionaryOptions::~CConfigDictionaryOptions()
{

}

void CConfigDictionaryOptions::loadSettings()
{
	bool bKeepDirty = false;

	m_bLoadingData = true;

	int nIndex = ui.comboDictionaryCompleterMode->findData(CPersistentSettings::instance()->dictionaryCompleterFilterMode());
	if (nIndex != -1) {
		ui.comboDictionaryCompleterMode->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboDictionaryCompleterMode->setCurrentIndex(0);
	}

	ui.spinDictionaryActivationDelay->setValue(CPersistentSettings::instance()->dictionaryActivationDelay());

	m_bLoadingData = false;
	m_bIsDirty = bKeepDirty;
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
	emit dataChanged(false);
}

void CConfigDictionaryOptions::en_changedDictionaryActivationDelay(int nValue)
{
	if (m_bLoadingData) return;

	Q_UNUSED(nValue);
	m_bIsDirty = true;
	emit dataChanged(false);
}

// ============================================================================

CConfigCopyOptions::CConfigCopyOptions(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QWidget(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_bIsDirty(false),
		m_bLoadingData(false),
		m_pEditCopyOptionPreview(NULL)
{
	assert(!pBibleDatabase.isNull());

	ui.setupUi(this);

	initialize();

	// ----------

	ui.comboReferenceDelimiterMode->addItem(tr("No Delimiters", "Delimiters"), CPhraseNavigator::RDME_NO_DELIMITER);
	ui.comboReferenceDelimiterMode->addItem(tr("Square Brackets", "Delimiters"), CPhraseNavigator::RDME_SQUARE_BRACKETS);
	ui.comboReferenceDelimiterMode->addItem(tr("Curly Braces", "Delimiters"), CPhraseNavigator::RDME_CURLY_BRACES);
	ui.comboReferenceDelimiterMode->addItem(tr("Parentheses", "Delimiters"), CPhraseNavigator::RDME_PARENTHESES);

	connect(ui.comboReferenceDelimiterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedReferenceDelimiterMode(int)));

	// ----------

	connect(ui.checkBoxReferencesUseAbbreviatedBookNames, SIGNAL(clicked(bool)), this, SLOT(en_changedReferencesUseAbbreviatedBookNames(bool)));

	// ----------

	connect(ui.checkBoxReferencesInBold, SIGNAL(clicked(bool)), this, SLOT(en_changedReferencesInBold(bool)));

	// ----------

	connect(ui.checkBoxReferencesAtEnd, SIGNAL(clicked(bool)), this, SLOT(en_changedReferencesAtEnd(bool)));

	// ----------

	ui.comboVerseNumberDelimiterMode->addItem(tr("No Numbers", "Delimiters"), CPhraseNavigator::RDME_NO_NUMBER);
	ui.comboVerseNumberDelimiterMode->addItem(tr("No Delimiters", "Delimiters"), CPhraseNavigator::RDME_NO_DELIMITER);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Square Brackets", "Delimiters"), CPhraseNavigator::RDME_SQUARE_BRACKETS);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Curly Braces", "Delimiters"), CPhraseNavigator::RDME_CURLY_BRACES);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Parentheses", "Delimiters"), CPhraseNavigator::RDME_PARENTHESES);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Superscript", "Delimiters"), CPhraseNavigator::RDME_SUPERSCRIPT);
	ui.comboVerseNumberDelimiterMode->addItem(tr("Complete Reference", "Delimiters"), CPhraseNavigator::RDME_COMPLETE_REFERENCE);

	connect(ui.comboVerseNumberDelimiterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedVerseNumberDelimiterMode(int)));

	// ----------

	connect(ui.checkBoxVerseNumbersUseAbbreviatedBookNames, SIGNAL(clicked(bool)), this, SLOT(en_changedVerseNumbersUseAbbreviatedBookNames(bool)));

	// ----------

	connect(ui.checkBoxVerseNumbersInBold, SIGNAL(clicked(bool)), this, SLOT(en_changedVerseNumbersInBold(bool)));

	// ----------

	connect(ui.checkBoxAddQuotesAroundVerse, SIGNAL(clicked(bool)), this, SLOT(en_changedAddQuotesAroundVerse(bool)));

	// ----------

	ui.comboTransChangeAddedMode->addItem(tr("No Marking", "Delimiters"), CPhraseNavigator::TCAWME_NO_MARKING);
	ui.comboTransChangeAddedMode->addItem(tr("Italics", "Delimiters"), CPhraseNavigator::TCAWME_ITALICS);
	ui.comboTransChangeAddedMode->addItem(tr("Brackets", "Delimiters"), CPhraseNavigator::TCAWME_BRACKETS);

	connect(ui.comboTransChangeAddedMode, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedTransChangeAddWordMode(int)));

	// ----------

	ui.comboBoxVerseRenderingModeCopying->addItem(tr("Free-Flow/Paragraph", "VerseRenderingModes"), CPhraseNavigator::VRME_FF);
	ui.comboBoxVerseRenderingModeCopying->addItem(tr("Verse-Per-Line", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL);
	ui.comboBoxVerseRenderingModeCopying->addItem(tr("Verse-Per-Line Double-Spaced", "VerseRenderingModes"), CPhraseNavigator::VRME_VPL_DS);

	connect(ui.comboBoxVerseRenderingModeCopying, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedVerseRenderingModeCopying(int)));

	// ----------

	connect(ui.checkBoxCopyPilcrowMarkers, SIGNAL(clicked(bool)), this, SLOT(en_changedCopyPilcrowMarkers(bool)));

	// ----------

	connect(ui.checkBoxCopyColophons, SIGNAL(clicked(bool)), this, SLOT(en_changedCopyColophons(bool)));
	connect(ui.checkBoxCopySuperscriptions, SIGNAL(clicked(bool)), this, SLOT(en_changedCopySuperscriptions(bool)));

	// ----------

	ui.comboBoxCopyFontSelection->addItem(tr("No Font Hint", "CopyFontModes"), CPhraseNavigator::CFSE_NONE);
	ui.comboBoxCopyFontSelection->addItem(tr("Copy Font", "CopyFontModes"), CPhraseNavigator::CFSE_COPY_FONT);
	ui.comboBoxCopyFontSelection->addItem(tr("Scripture Browser Font", "CopyFontModes"), CPhraseNavigator::CFSE_SCRIPTURE_BROWSER);
	ui.comboBoxCopyFontSelection->addItem(tr("Search Results Font", "CopyFontModes"), CPhraseNavigator::CFSE_SEARCH_RESULTS);

	connect(ui.comboBoxCopyFontSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedCopyFontSelection(int)));

	ui.dblSpinBoxCopyFontSize->setRange(FONT_MIN_SIZE, FONT_MAX_SIZE);

	connect(ui.fontComboBoxCopyFont, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(en_changedFontCopyFont(const QFont &)));
	connect(ui.dblSpinBoxCopyFontSize, SIGNAL(valueChanged(double)), this, SLOT(en_changedFontCopyFontSize(double)));

	// ----------

	ui.comboBoxCopyMimeType->addItem(tr("Both", "CopyMimeTypes"), CMTE_ALL);
	ui.comboBoxCopyMimeType->addItem(tr("HTML-Only", "CopyMimeTypes"), CMTE_HTML);
	ui.comboBoxCopyMimeType->addItem(tr("Text-Only", "CopyMimeTypes"), CMTE_TEXT);

	connect(ui.comboBoxCopyMimeType, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedCopyMimeType(int)));

	// ----------

	connect(ui.checkBoxSearchResultsAddBlankLineBetweenVerses, SIGNAL(clicked(bool)), this, SLOT(en_changedSearchResultsAddBlankLineBetweenVerses(bool)));

	ui.comboBoxSearchResultsVerseCopyOrder->addItem(tr("Selected Order", "VerseCopyOrder"), VCOE_SELECTED);
	ui.comboBoxSearchResultsVerseCopyOrder->addItem(tr("Bible Order Ascending"), VCOE_BIBLE_ASCENDING);
	ui.comboBoxSearchResultsVerseCopyOrder->addItem(tr("Bible Order Descending"), VCOE_BIBLE_DESCENDING);

	connect(ui.comboBoxSearchResultsVerseCopyOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedSearchResultsVerseCopyOrder(int)));

	connect(ui.checkBoxShowOCntInSearchResultsRefs, SIGNAL(clicked(bool)), this, SLOT(en_changedShowOCntInSearchResultsRefs(bool)));
	connect(ui.checkBoxCopyOCntInSearchResultsRefs, SIGNAL(clicked(bool)), this, SLOT(en_changedCopyOCntInSearchResultsRefs(bool)));
	connect(ui.checkBoxShowWrdNdxInSearchResultsRefs, SIGNAL(clicked(bool)), this, SLOT(en_changedShowWrdNdxInSearchResultsRefs(bool)));
	connect(ui.checkBoxCopyWrdNdxInSearchResultsRefs, SIGNAL(clicked(bool)), this, SLOT(en_changedCopyWrdNdxInSearchResultsRefs(bool)));

	// ----------

	loadSettings();
}

CConfigCopyOptions::~CConfigCopyOptions()
{

}

void CConfigCopyOptions::initialize()
{
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

	int nIndex = ui.verticalLayoutCopyOptionPreview->indexOf(ui.editCopyOptionPreview);
	assert(nIndex != -1);
	delete ui.editCopyOptionPreview;
	ui.editCopyOptionPreview = NULL;
	ui.verticalLayoutCopyOptionPreview->insertWidget(nIndex, m_pEditCopyOptionPreview);

	// ----------

	setVerseCopyPreview();
	setSearchResultsRefsPreview();
	QTextCursor aCursor(m_pEditCopyOptionPreview->textCursor());
	aCursor.movePosition(QTextCursor::Start);
	m_pEditCopyOptionPreview->setTextCursor(aCursor);
}

void CConfigCopyOptions::loadSettings()
{
	bool bKeepDirty = false;

	m_bLoadingData = true;

	int nIndex;

	// ----------

	nIndex = ui.comboReferenceDelimiterMode->findData(CPersistentSettings::instance()->referenceDelimiterMode());
	if (nIndex != -1) {
		ui.comboReferenceDelimiterMode->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboReferenceDelimiterMode->setCurrentIndex(0);
		CPersistentSettings::instance()->setReferenceDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(ui.comboReferenceDelimiterMode->itemData(0).toUInt()));
		setVerseCopyPreview();
	}

	// ----------

	ui.checkBoxReferencesUseAbbreviatedBookNames->setChecked(CPersistentSettings::instance()->referencesUseAbbreviatedBookNames());

	// ----------

	ui.checkBoxReferencesInBold->setChecked(CPersistentSettings::instance()->referencesInBold());

	// ----------

	ui.checkBoxReferencesAtEnd->setChecked(CPersistentSettings::instance()->referencesAtEnd());

	// ----------

	nIndex = ui.comboVerseNumberDelimiterMode->findData(CPersistentSettings::instance()->verseNumberDelimiterMode());
	if (nIndex != -1) {
		ui.comboVerseNumberDelimiterMode->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboVerseNumberDelimiterMode->setCurrentIndex(0);
		CPersistentSettings::instance()->setVerseNumberDelimiterMode(static_cast<CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM>(ui.comboVerseNumberDelimiterMode->itemData(0).toUInt()));
		setVerseCopyPreview();
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
		bKeepDirty = true;
		ui.comboTransChangeAddedMode->setCurrentIndex(0);
		CPersistentSettings::instance()->setTransChangeAddWordMode(static_cast<CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM>(ui.comboTransChangeAddedMode->itemData(0).toUInt()));
		setVerseCopyPreview();
	}

	// ----------

	nIndex = ui.comboBoxVerseRenderingModeCopying->findData(CPersistentSettings::instance()->verseRenderingModeCopying());
	if (nIndex != -1) {
		ui.comboBoxVerseRenderingModeCopying->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboBoxVerseRenderingModeCopying->setCurrentIndex(0);
		CPersistentSettings::instance()->setVerseRenderingModeCopying(static_cast<CPhraseNavigator::VERSE_RENDERING_MODE_ENUM>(ui.comboBoxVerseRenderingModeCopying->itemData(0).toUInt()));
		setVerseCopyPreview();
	}

	// ----------

	ui.checkBoxCopyPilcrowMarkers->setChecked(CPersistentSettings::instance()->copyPilcrowMarkers());

	// ----------

	ui.checkBoxCopyColophons->setChecked(CPersistentSettings::instance()->copyColophons());
	ui.checkBoxCopySuperscriptions->setChecked(CPersistentSettings::instance()->copySuperscriptions());

	// ----------

	nIndex = ui.comboBoxCopyFontSelection->findData(CPersistentSettings::instance()->copyFontSelection());
	if (nIndex != -1) {
		ui.comboBoxCopyFontSelection->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboBoxCopyFontSelection->setCurrentIndex(0);
		CPhraseNavigator::COPY_FONT_SELECTION_ENUM nCFSE = static_cast<CPhraseNavigator::COPY_FONT_SELECTION_ENUM>(ui.comboBoxCopyFontSelection->itemData(0).toUInt());
		CPersistentSettings::instance()->setCopyFontSelection(nCFSE);
		ui.fontComboBoxCopyFont->setEnabled(nCFSE == CPhraseNavigator::CFSE_COPY_FONT);
		ui.dblSpinBoxCopyFontSize->setEnabled(nCFSE == CPhraseNavigator::CFSE_COPY_FONT);
		setVerseCopyPreview();
	}

	m_fntCopyFont = CPersistentSettings::instance()->fontCopyFont();
	ui.fontComboBoxCopyFont->setCurrentFont(m_fntCopyFont);
	ui.dblSpinBoxCopyFontSize->setValue(m_fntCopyFont.pointSizeF());

	ui.fontComboBoxCopyFont->setEnabled(CPersistentSettings::instance()->copyFontSelection() == CPhraseNavigator::CFSE_COPY_FONT);
	ui.dblSpinBoxCopyFontSize->setEnabled(CPersistentSettings::instance()->copyFontSelection() == CPhraseNavigator::CFSE_COPY_FONT);

	// ----------

	nIndex = ui.comboBoxCopyMimeType->findData(CPersistentSettings::instance()->copyMimeType());
	if (nIndex != -1) {
		ui.comboBoxCopyMimeType->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboBoxCopyMimeType->setCurrentIndex(0);
		CPersistentSettings::instance()->setCopyMimeType(static_cast<COPY_MIME_TYPE_ENUM>(ui.comboBoxCopyMimeType->itemData(0).toUInt()));
	}

	// ----------

	ui.checkBoxSearchResultsAddBlankLineBetweenVerses->setChecked(CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses());

	nIndex = ui.comboBoxSearchResultsVerseCopyOrder->findData(CPersistentSettings::instance()->searchResultsVerseCopyOrder());
	if (nIndex != -1) {
		ui.comboBoxSearchResultsVerseCopyOrder->setCurrentIndex(nIndex);
	} else {
		bKeepDirty = true;
		ui.comboBoxSearchResultsVerseCopyOrder->setCurrentIndex(0);
		CPersistentSettings::instance()->setSearchResultsVerseCopyOrder(static_cast<VERSE_COPY_ORDER_ENUM>(ui.comboBoxSearchResultsVerseCopyOrder->itemData(0).toUInt()));
	}

	ui.checkBoxShowOCntInSearchResultsRefs->setChecked(CPersistentSettings::instance()->showOCntInSearchResultsRefs());
	ui.checkBoxCopyOCntInSearchResultsRefs->setChecked(CPersistentSettings::instance()->copyOCntInSearchResultsRefs());
	ui.checkBoxShowWrdNdxInSearchResultsRefs->setChecked(CPersistentSettings::instance()->showWrdNdxInSearchResultsRefs());
	ui.checkBoxCopyWrdNdxInSearchResultsRefs->setChecked(CPersistentSettings::instance()->copyWrdNdxInSearchResultsRefs());

	// ----------

	m_bLoadingData = false;
	m_bIsDirty = bKeepDirty;
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
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedReferencesUseAbbreviatedBookNames(bool bUseAbbrBookName)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setReferencesUseAbbreviatedBookNames(bUseAbbrBookName);
	m_bIsDirty = true;
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedReferencesInBold(bool bInBold)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setReferencesInBold(bInBold);
	m_bIsDirty = true;
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedReferencesAtEnd(bool bAtEnd)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setReferencesAtEnd(bAtEnd);
	m_bIsDirty = true;
	emit dataChanged(false);
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
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedVerseNumbersUseAbbreviatedBookNames(bool bUseAbbrBookName)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setVerseNumbersUseAbbreviatedBookNames(bUseAbbrBookName);
	m_bIsDirty = true;
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedVerseNumbersInBold(bool bInBold)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setVerseNumbersInBold(bInBold);
	m_bIsDirty = true;
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedAddQuotesAroundVerse(bool bAddQuotes)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setAddQuotesAroundVerse(bAddQuotes);
	m_bIsDirty = true;
	emit dataChanged(false);
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
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedVerseRenderingModeCopying(int nIndex)
{
	if (m_bLoadingData) return;

	if (nIndex != -1) {
		CPersistentSettings::instance()->setVerseRenderingModeCopying(static_cast<CPhraseNavigator::VERSE_RENDERING_MODE_ENUM>(ui.comboBoxVerseRenderingModeCopying->itemData(nIndex).toUInt()));
	} else {
		assert(false);
	}
	m_bIsDirty = true;
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedCopyPilcrowMarkers(bool bCopyPilcrowMarkers)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setCopyPilcrowMarkers(bCopyPilcrowMarkers);
	m_bIsDirty = true;
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedCopyColophons(bool bCopyColophons)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setCopyColophons(bCopyColophons);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigCopyOptions::en_changedCopySuperscriptions(bool bCopySuperscriptions)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setCopySuperscriptions(bCopySuperscriptions);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigCopyOptions::en_changedCopyFontSelection(int nIndex)
{
	if (m_bLoadingData) return;

	if (nIndex != -1) {
		CPhraseNavigator::COPY_FONT_SELECTION_ENUM nCFSE = static_cast<CPhraseNavigator::COPY_FONT_SELECTION_ENUM>(ui.comboBoxCopyFontSelection->itemData(nIndex).toUInt());
		CPersistentSettings::instance()->setCopyFontSelection(nCFSE);
		ui.fontComboBoxCopyFont->setEnabled(nCFSE == CPhraseNavigator::CFSE_COPY_FONT);
		ui.dblSpinBoxCopyFontSize->setEnabled(nCFSE == CPhraseNavigator::CFSE_COPY_FONT);
	} else {
		assert(false);
	}
	m_bIsDirty = true;
	emit dataChanged(false);
	setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedFontCopyFont(const QFont &aFont)
{
	if (m_bLoadingData) return;

	m_fntCopyFont.setFamily(aFont.family());
	CPersistentSettings::instance()->setFontCopyFont(m_fntCopyFont);
	m_bIsDirty = true;
	emit dataChanged(false);
	if (CPersistentSettings::instance()->copyFontSelection() == CPhraseNavigator::CFSE_COPY_FONT) setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedFontCopyFontSize(double nFontSize)
{
	if (m_bLoadingData) return;

	m_fntCopyFont.setPointSizeF(nFontSize);
	CPersistentSettings::instance()->setFontCopyFont(m_fntCopyFont);
	m_bIsDirty = true;
	emit dataChanged(false);
	if (CPersistentSettings::instance()->copyFontSelection() == CPhraseNavigator::CFSE_COPY_FONT) setVerseCopyPreview();
}

void CConfigCopyOptions::en_changedCopyMimeType(int nIndex)
{
	if (m_bLoadingData) return;

	if (nIndex != -1) {
		CPersistentSettings::instance()->setCopyMimeType(static_cast<COPY_MIME_TYPE_ENUM>(ui.comboBoxCopyMimeType->itemData(nIndex).toUInt()));
	} else {
		assert(false);
	}
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigCopyOptions::en_changedSearchResultsAddBlankLineBetweenVerses(bool bAddBlankLine)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setSearchResultsAddBlankLineBetweenVerses(bAddBlankLine);
	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigCopyOptions::en_changedSearchResultsVerseCopyOrder(int nIndex)
{
	if (m_bLoadingData) return;

	if (nIndex != -1) {
		VERSE_COPY_ORDER_ENUM nVCOE = static_cast<VERSE_COPY_ORDER_ENUM>(ui.comboBoxSearchResultsVerseCopyOrder->itemData(nIndex).toUInt());
		CPersistentSettings::instance()->setSearchResultsVerseCopyOrder(nVCOE);
	} else {
		assert(false);
	}

	m_bIsDirty = true;
	emit dataChanged(false);
}

void CConfigCopyOptions::en_changedShowOCntInSearchResultsRefs(bool bShow)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setShowOCntInSearchResultsRefs(bShow);
	m_bIsDirty = true;
	emit dataChanged(false);
	setSearchResultsRefsPreview();
}

void CConfigCopyOptions::en_changedCopyOCntInSearchResultsRefs(bool bCopy)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setCopyOCntInSearchResultsRefs(bCopy);
	m_bIsDirty = true;
	emit dataChanged(false);
	setSearchResultsRefsPreview();
}

void CConfigCopyOptions::en_changedShowWrdNdxInSearchResultsRefs(bool bShow)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setShowWrdNdxInSearchResultsRefs(bShow);
	m_bIsDirty = true;
	emit dataChanged(false);
	setSearchResultsRefsPreview();
}

void CConfigCopyOptions::en_changedCopyWrdNdxInSearchResultsRefs(bool bCopy)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setCopyWrdNdxInSearchResultsRefs(bCopy);
	m_bIsDirty = true;
	emit dataChanged(false);
	setSearchResultsRefsPreview();
}

void CConfigCopyOptions::setVerseCopyPreview()
{
	assert(!m_pBibleDatabase.isNull());

	QString strHtml;
	QTextDocument doc;
	CPhraseNavigator navigator(m_pBibleDatabase, doc);
	if (!navigator.setDocumentToFormattedVerses(TPassageTagList(TPassageTag(CRelIndex(1, 1, 1, 0), 3))).isEmpty()) {
		strHtml += doc.toHtml();
		strHtml += "<hr>\n";
	}
	if (!navigator.setDocumentToFormattedVerses(TPassageTagList(TPassageTag(CRelIndex(40, 24, 50, 0), 4))).isEmpty()) {
		strHtml += doc.toHtml();
		strHtml += "<hr>\n";
	}
	if (!navigator.setDocumentToFormattedVerses(TPassageTagList(TPassageTag(CRelIndex(41, 13, 24, 0), 2))).isEmpty()) {
		strHtml += doc.toHtml();
		strHtml += "<hr>\n";
	}
	if (!navigator.setDocumentToFormattedVerses(TPassageTagList(TPassageTag(CRelIndex(41, 13, 31, 0), 3))).isEmpty()) {
		strHtml += doc.toHtml();
		strHtml += "<hr>\n";
	}
	if (!navigator.setDocumentToFormattedVerses(TPassageTagList(TPassageTag(CRelIndex(65, 1, 25, 0), 3))).isEmpty()) {
		strHtml += doc.toHtml();
	}
	m_pEditCopyOptionPreview->document()->setHtml(strHtml);
}

void CConfigCopyOptions::setSearchResultsRefsPreview()
{
	assert(!m_pBibleDatabase.isNull());

	TPhraseTagList lstTags;
	lstTags.append(TPhraseTag(CRelIndex(40, 24, 50, 1)));
	lstTags.append(TPhraseTag(CRelIndex(40, 24, 50, 3)));
	lstTags.append(TPhraseTag(CRelIndex(40, 24, 50, 5)));
	CVerseListItem vliTemp(TVerseIndex(CRelIndex(40, 24, 50, 0), VLMRTE_SEARCH_RESULTS, VLMNTE_VERSE_TERMINATOR_NODE), m_pBibleDatabase, lstTags);
	ui.lineEditShowSearchResultsRefsPreview->setText(vliTemp.getHeading(false));
	ui.lineEditCopySearchResultsRefsPreview->setText(vliTemp.getHeading(true));
}

// ============================================================================

CKJVGeneralSettingsConfig::CKJVGeneralSettingsConfig(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QWidget(parent)
{
	assert(!pBibleDatabase.isNull());

	ui.setupUi(this);

	connect(ui.widgetSearchOptions, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
	connect(ui.widgetBrowserOptions, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
	connect(ui.widgetDictionaryOptions, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
}

CKJVGeneralSettingsConfig::~CKJVGeneralSettingsConfig()
{

}

void CKJVGeneralSettingsConfig::loadSettings()
{
	ui.widgetSearchOptions->loadSettings();
	ui.widgetBrowserOptions->loadSettings();
	ui.widgetDictionaryOptions->loadSettings();
}

void CKJVGeneralSettingsConfig::saveSettings()
{
	ui.widgetSearchOptions->saveSettings();
	ui.widgetBrowserOptions->saveSettings();
	ui.widgetDictionaryOptions->saveSettings();
}

bool CKJVGeneralSettingsConfig::isDirty() const
{
	return (ui.widgetSearchOptions->isDirty() || ui.widgetBrowserOptions->isDirty() || ui.widgetDictionaryOptions->isDirty());
}

// ============================================================================
// ============================================================================

CKJVLocaleConfig::CKJVLocaleConfig(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	ui.comboBoxLanguageList->clear();
	QList<CTranslatorList::TLanguageName> lstLanguages = CTranslatorList::instance()->languageList();
	ui.comboBoxLanguageList->addItem(tr("< System Locale >", "languageNames"), QString());
	for (int ndx = 0; ndx < lstLanguages.size(); ++ndx) {
		assert(!lstLanguages.at(ndx).first.isEmpty());
		if (lstLanguages.at(ndx).first.isEmpty()) continue;
		ui.comboBoxLanguageList->addItem(lstLanguages.at(ndx).second, lstLanguages.at(ndx).first);
	}

	connect(ui.comboBoxLanguageList, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changeApplicationLanguage(int)));

	loadSettings();
}

CKJVLocaleConfig::~CKJVLocaleConfig()
{

}

void CKJVLocaleConfig::loadSettings()
{
	m_bLoadingData = true;

	int nIndex = ui.comboBoxLanguageList->findData(CPersistentSettings::instance()->applicationLanguage());
	if (nIndex == -1) {
		nIndex = ui.comboBoxLanguageList->findData(QString());
		assert(nIndex != -1);
	}
	ui.comboBoxLanguageList->setCurrentIndex(nIndex);

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CKJVLocaleConfig::saveSettings()
{
	CMyApplication::saveApplicationLanguage();
	CTranslatorList::instance()->setApplicationLanguage(CPersistentSettings::instance()->applicationLanguage());

	m_bIsDirty = false;
}

void CKJVLocaleConfig::en_changeApplicationLanguage(int nIndex)
{
	if (m_bLoadingData) return;

	assert(nIndex != -1);
	if (nIndex == -1) return;

	QString strLangName = ui.comboBoxLanguageList->itemData(nIndex).toString();
	CPersistentSettings::instance()->setApplicationLanguage(strLangName);

	m_bIsDirty = true;
	emit dataChanged(true);
}

// ============================================================================
// ============================================================================

#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)

CKJVTTSOptionsConfig::CKJVTTSOptionsConfig(QWidget *parent)
	:	QWidget(parent),
		m_bIsDirty(false),
		m_bLoadingData(false)
{
	ui.setupUi(this);

	if (!QtSpeech::serverSupported()) {
		ui.editTTSServerURL->setEnabled(false);
		ui.editTTSServerURL->setPlaceholderText(tr("Not used on this platform", "SpeechSettings"));
	}

	connect(ui.editTTSServerURL, SIGNAL(textChanged(const QString &)), this, SLOT(en_changedTTSServerURL(const QString &)));

	ui.comboBoxTTSVoiceSelection->clear();
	QtSpeech::TVoiceNamesList lstVoiceNames = QtSpeech::voices();
	for (int ndx = 0; ndx < lstVoiceNames.size(); ++ndx) {
		assert(!lstVoiceNames.at(ndx).isEmpty());
		if (lstVoiceNames.at(ndx).isEmpty()) continue;
		ui.comboBoxTTSVoiceSelection->addItem(lstVoiceNames.at(ndx).name, lstVoiceNames.at(ndx).id);
	}

	connect(ui.comboBoxTTSVoiceSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(en_changedTTSVoiceSelection(int)));

	loadSettings();
}

CKJVTTSOptionsConfig::~CKJVTTSOptionsConfig()
{

}

void CKJVTTSOptionsConfig::loadSettings()
{
	m_bLoadingData = true;

	ui.editTTSServerURL->setText(CPersistentSettings::instance()->ttsServerURL());

	int nIndex = ui.comboBoxTTSVoiceSelection->findData(CPersistentSettings::instance()->ttsSelectedVoiceID());
	if (nIndex != -1) {
		ui.comboBoxTTSVoiceSelection->setCurrentIndex(nIndex);
	}

	m_bLoadingData = false;
	m_bIsDirty = false;
}

void CKJVTTSOptionsConfig::saveSettings()
{
	CMyApplication::saveTTSSettings();
	if (QtSpeech::serverSupported()) {
		QString strTTSServer = CPersistentSettings::instance()->ttsServerURL();
		if (!strTTSServer.isEmpty()) {
			QUrl urlTTSServer(strTTSServer);
			QString strTTSHost = urlTTSServer.host();
			if (urlTTSServer.scheme().compare(QTSPEECH_SERVER_SCHEME_NAME, Qt::CaseInsensitive) != 0) {
				QMessageBox::warning(this, windowTitle(), tr("Unknown Text-To-Speech Server Scheme name.\nExpected \"%1\".", "Errors").arg(QTSPEECH_SERVER_SCHEME_NAME));
			} else if ((!strTTSHost.isEmpty()) &&
						(!QtSpeech::connectToServer(strTTSHost, urlTTSServer.port(QTSPEECH_DEFAULT_SERVER_PORT)))) {
				QMessageBox::warning(this, windowTitle(), tr("Failed to connect to Text-To-Speech Server!\n\n\"%1\"", "Errors").arg(strTTSServer));
			} else {
				if (strTTSHost.isEmpty()) {
					QtSpeech::disconnectFromServer();			// Empty Host means we disconnect from current server
				}
			}
		} else {
			QtSpeech::disconnectFromServer();				// Empty URL means we disconnect from current server
		}
	}

	if ((!g_pMyApplication.isNull()) && (g_pMyApplication->speechSynth() != NULL)) {
		QtSpeech::TVoiceName aVoiceName;
		aVoiceName.id = CPersistentSettings::instance()->ttsSelectedVoiceID();
		g_pMyApplication->speechSynth()->setVoiceName(aVoiceName);
	}

	m_bIsDirty = false;
}

void CKJVTTSOptionsConfig::en_changedTTSServerURL(const QString &strTTSServerURL)
{
	if (m_bLoadingData) return;

	CPersistentSettings::instance()->setTTSServerURL(strTTSServerURL);

	m_bIsDirty = true;
	emit dataChanged(false);
}

void CKJVTTSOptionsConfig::en_changedTTSVoiceSelection(int nIndex)
{
	if (m_bLoadingData) return;

	assert(nIndex != -1);
	if (nIndex == -1) return;

	QString strVoiceID = ui.comboBoxTTSVoiceSelection->itemData(nIndex).toString();
	CPersistentSettings::instance()->setTTSSelectedVoiceID(strVoiceID);

	m_bIsDirty = true;
	emit dataChanged(false);
}

#endif	// QtSpeech && !EMSCRIPTEN && !VNCSERVER

// ============================================================================
// ============================================================================

CKJVConfiguration::CKJVConfiguration(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent, CONFIGURATION_PAGE_SELECTION_ENUM nInitialPage)
	:	QwwConfigWidget(parent),
		m_pGeneralSettingsConfig(NULL),
		m_pCopyOptionsConfig(NULL),
		m_pTextFormatConfig(NULL),
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		m_pUserNotesDatabaseConfig(NULL),
#endif
		m_pBibleDatabaseConfig(NULL),
		m_pDictDatabaseConfig(NULL),
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		m_pTTSOptionsConfig(NULL),
#endif
		m_pLocaleConfig(NULL)
{
	assert(!pBibleDatabase.isNull());
	assert(!g_pUserNotesDatabase.isNull());

	m_pGeneralSettingsConfig = new CKJVGeneralSettingsConfig(pBibleDatabase, this);
	m_pCopyOptionsConfig = new CConfigCopyOptions(pBibleDatabase, this);
	m_pTextFormatConfig = new CKJVTextFormatConfig(pBibleDatabase, pDictionary, this);
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	m_pUserNotesDatabaseConfig = new CKJVUserNotesDatabaseConfig(g_pUserNotesDatabase, this);
#endif
	m_pBibleDatabaseConfig = new CKJVBibleDatabaseConfig(this);
	m_pDictDatabaseConfig = new CKJVDictDatabaseConfig(this);
	m_pLocaleConfig = new CKJVLocaleConfig(this);
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	m_pTTSOptionsConfig = new CKJVTTSOptionsConfig(this);
#endif

	addGroup(m_pGeneralSettingsConfig, QIcon(":/res/ControlPanel-256.png"), tr("General Settings", "MainMenu"));
	addGroup(m_pCopyOptionsConfig, QIcon(":/res/copy_128.png"), tr("Copy Options", "MainMenu"));
	addGroup(m_pTextFormatConfig, QIcon(":/res/Font_Graphics_Color_Icon_128.png"), tr("Text Color and Fonts", "MainMenu"));
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	addGroup(m_pUserNotesDatabaseConfig, QIcon(":/res/Data_management_Icon_128.png"), tr("Notes File Settings", "MainMenu"));
#endif
	addGroup(m_pBibleDatabaseConfig, QIcon(":/res/Database4-128.png"), tr("Bible Database", "MainMenu"));
	addGroup(m_pDictDatabaseConfig, QIcon(":/res/Apps-accessories-dictionary-icon-128.png"), tr("Dictionary Database", "MainMenu"));
	addGroup(m_pLocaleConfig, QIcon(":/res/language_256.png"), tr("Locale Settings", "MainMenu"));
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	addGroup(m_pTTSOptionsConfig, QIcon(":/res/Actions-text-speak-icon-128.png"), tr("Text-To-Speech", "MainMenu"));
#endif

	QWidget *pSelect = m_pGeneralSettingsConfig;		// Default page

	switch (nInitialPage) {
		case CPSE_GENERAL_SETTINGS:
			pSelect = m_pGeneralSettingsConfig;
			break;
		case CPSE_COPY_OPTIONS:
			pSelect = m_pCopyOptionsConfig;
			break;
		case CPSE_TEXT_FORMAT:
			pSelect = m_pTextFormatConfig;
			break;
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		case CPSE_USER_NOTES_DATABASE:
			pSelect = m_pUserNotesDatabaseConfig;
			break;
#endif
		case CPSE_BIBLE_DATABASE:
			pSelect = m_pBibleDatabaseConfig;
			break;
		case CPSE_DICT_DATABASE:
			pSelect = m_pDictDatabaseConfig;
			break;
		case CPSE_LOCALE:
			pSelect = m_pLocaleConfig;
			break;
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		case CPSE_TTS_OPTIONS:
			pSelect = m_pTTSOptionsConfig;
			break;
#endif
		case CPSE_DEFAULT:
			break;
		default:
			assert(false);
			break;
	}

	setCurrentGroup(pSelect);

	connect(m_pGeneralSettingsConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
	connect(m_pCopyOptionsConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
	connect(m_pTextFormatConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	connect(m_pUserNotesDatabaseConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
#endif
	connect(m_pBibleDatabaseConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
	connect(m_pDictDatabaseConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
	connect(m_pLocaleConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	connect(m_pTTSOptionsConfig, SIGNAL(dataChanged(bool)), this, SIGNAL(dataChanged(bool)));
#endif
}

CKJVConfiguration::~CKJVConfiguration()
{

}

void CKJVConfiguration::loadSettings()
{
	m_pGeneralSettingsConfig->loadSettings();
	m_pCopyOptionsConfig->loadSettings();
	m_pTextFormatConfig->loadSettings();
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	m_pUserNotesDatabaseConfig->loadSettings();
#endif
	m_pBibleDatabaseConfig->loadSettings();
	m_pDictDatabaseConfig->loadSettings();
	m_pLocaleConfig->loadSettings();
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	m_pTTSOptionsConfig->loadSettings();
#endif
}

void CKJVConfiguration::saveSettings()
{
	m_pGeneralSettingsConfig->saveSettings();
	m_pCopyOptionsConfig->saveSettings();
	m_pTextFormatConfig->saveSettings();
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	m_pUserNotesDatabaseConfig->saveSettings();
#endif
	m_pBibleDatabaseConfig->saveSettings();
	m_pDictDatabaseConfig->saveSettings();
	m_pLocaleConfig->saveSettings();
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	m_pTTSOptionsConfig->saveSettings();
#endif
}

bool CKJVConfiguration::isDirty(CONFIGURATION_PAGE_SELECTION_ENUM nPage) const
{
	switch (nPage) {
		case CPSE_GENERAL_SETTINGS:
			return m_pGeneralSettingsConfig->isDirty();
		case CPSE_COPY_OPTIONS:
			return m_pCopyOptionsConfig->isDirty();
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		case CPSE_USER_NOTES_DATABASE:
			return m_pUserNotesDatabaseConfig->isDirty();
#endif
		case CPSE_TEXT_FORMAT:
			return m_pTextFormatConfig->isDirty();
		case CPSE_BIBLE_DATABASE:
			return m_pBibleDatabaseConfig->isDirty();
		case CPSE_DICT_DATABASE:
			return m_pDictDatabaseConfig->isDirty();
		case CPSE_LOCALE:
			return m_pLocaleConfig->isDirty();
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		case CPSE_TTS_OPTIONS:
			return m_pTTSOptionsConfig->isDirty();
#endif
		case CPSE_DEFAULT:
		default:
			return (m_pGeneralSettingsConfig->isDirty() ||
					m_pCopyOptionsConfig->isDirty() ||
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
					m_pUserNotesDatabaseConfig->isDirty() ||
#endif
					m_pTextFormatConfig->isDirty() ||
					m_pBibleDatabaseConfig->isDirty() ||
					m_pDictDatabaseConfig->isDirty() ||
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
					m_pTTSOptionsConfig->isDirty() ||
#endif
					m_pLocaleConfig->isDirty());
	}
}

// ============================================================================

CKJVConfigurationDialog::CKJVConfigurationDialog(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent, CONFIGURATION_PAGE_SELECTION_ENUM nInitialPage)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		m_nLastIndex(-1),
		m_bHandlingPageSwap(false),
		m_pConfiguration(NULL),
		m_pButtonBox(NULL),
		m_bNeedRestart(false),
		m_bRestartApp(false)
{
	assert(!pBibleDatabase.isNull());
	assert(!g_pUserNotesDatabase.isNull());

#ifdef MODELTEST
	if (g_pdlgColor == NULL) g_pdlgColor = new QColorDialog(this);
#endif

	// --------------------------------------------------------------

	// Make a working copy of our settings:
	CPersistentSettings::instance()->togglePersistentSettingData(true);
	g_pUserNotesDatabase->toggleUserNotesDatabaseData(true);

	// --------------------------------------------------------------

	QVBoxLayout *pLayout = new QVBoxLayout(this);
	pLayout->setObjectName(QString::fromUtf8("verticalLayout"));

	m_pConfiguration = new CKJVConfiguration(pBibleDatabase, pDictionary, this, nInitialPage);
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
	adjustSize();

	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(m_pButtonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

	connect(m_pConfiguration, SIGNAL(dataChanged(bool)), this, SLOT(en_dataChanged(bool)));
}

CKJVConfigurationDialog::~CKJVConfigurationDialog()
{

}

void CKJVConfigurationDialog::en_dataChanged(bool bNeedRestart)
{
	updateGeometry();
	m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(m_pConfiguration->isDirty());
	m_bNeedRestart = (bNeedRestart && m_pConfiguration->isDirty());
}

void CKJVConfigurationDialog::accept()
{
	if (m_bNeedRestart) m_bRestartApp = promptRestart();

	CBusyCursor iAmBusy(NULL);

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
																	   "Click 'Cancel' to stay here in the configuration window.", "Errors"),
																  (QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult == QMessageBox::Cancel) return;
	}
	restore(false);
	QDialog::reject();
}

void CKJVConfigurationDialog::apply()
{
	assert(!g_pUserNotesDatabase.isNull());

	if (m_bNeedRestart) m_bRestartApp = promptRestart();

	CBusyCursor iAmBusy(NULL);

	// Make sure our persistent settings have been updated, and we'll
	//		copy the settings over to the original, making them permanent
	//		as the user is "applying" them:
	m_pConfiguration->saveSettings();
	if (m_bRestartApp) {
		QDialog::accept();
		// If we are restarting, then leave the settings permanent, by
		//		not copying them back in the persistent settings object.
		//		This is the same functionality of "accept()".  We have
		//		already prompted the user above...
	} else {
		CPersistentSettings::instance()->togglePersistentSettingData(true);
		g_pUserNotesDatabase->toggleUserNotesDatabaseData(true);
		en_dataChanged(false);
	}
}

void CKJVConfigurationDialog::restore(bool bRecopy)
{
	assert(!g_pUserNotesDatabase.isNull());

	// Restore original settings by switching back to the original
	//		settings without copying:
	CPersistentSettings::instance()->togglePersistentSettingData(false);
	g_pUserNotesDatabase->toggleUserNotesDatabaseData(false);

	if (bRecopy) {
		// Make a working copy of our settings:
		CPersistentSettings::instance()->togglePersistentSettingData(true);
		g_pUserNotesDatabase->toggleUserNotesDatabaseData(true);
		m_pConfiguration->loadSettings();
		en_dataChanged(false);
	}
}

// ----------------------------------------------------------------------------

void CKJVConfigurationDialog::en_configurationIndexChanged(int index)
{
	if (m_bHandlingPageSwap) return;

	assert(m_nLastIndex != -1);				// We should have set our initial page index and can never navigate away from some page!
	if (m_nLastIndex == index) return;
	if (m_pConfiguration->isDirty(static_cast<CONFIGURATION_PAGE_SELECTION_ENUM>(m_nLastIndex))) {
		m_bHandlingPageSwap = true;

		int nResult = QMessageBox::information(this, windowTitle(), tr("You have changed some settings on the previous page.  Do you wish to apply those settings??\n\n"
																	   "Click 'Yes' to apply the setting changes and continue.\n"
																	   "Click 'No' to discard those setting changes and continue.\n"
																	   "Click 'Cancel' to stay on this settings page.", "Errors"),
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

		m_bHandlingPageSwap = false;
	}

#ifdef USING_QT_SPEECH
	if (static_cast<CONFIGURATION_PAGE_SELECTION_ENUM>(index) == CPSE_TTS_OPTIONS) {
		if ((!g_pMyApplication.isNull()) && (g_pMyApplication->speechSynth() != NULL) && (!g_pMyApplication->speechSynth()->canSpeak())) {
			QMessageBox::warning(this, windowTitle(), tr("Failed to load system Text-To-Speech Module.\n"
														"To use Text-To-Speech, you may need to install the Text-To-Speech support package."));
		}
	}
#endif

	m_nLastIndex = index;

	m_pButtonBox->button(QDialogButtonBox::Apply)->setEnabled(m_pConfiguration->isDirty(static_cast<CONFIGURATION_PAGE_SELECTION_ENUM>(index)));
}

void CKJVConfigurationDialog::en_setToLastIndex()
{
	assert(m_bHandlingPageSwap);
	assert(m_nLastIndex != -1);
	m_pConfiguration->setCurrentIndex(m_nLastIndex);
	m_bHandlingPageSwap = false;
}

bool CKJVConfigurationDialog::promptRestart()
{
	int nResult = QMessageBox::information(this, windowTitle(), tr("The changes you have made require that the program be restarted before they take affect.  "
																   "Doing so will close all Search Windows just like exiting the program.  "
																   "If you choose not to exit, they will be applied the next time you run the program.\n\n"
																   "Do you wish to restart the app??", "Errors"),
																(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
	return (nResult == QMessageBox::Yes);
}

// ============================================================================
