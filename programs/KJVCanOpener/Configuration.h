/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "dbstruct.h"
#include "PhraseEdit.h"
#include "UserNotesDatabase.h"
#include "BibleDBListModel.h"
#include "DictDBListModel.h"

#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QwwConfigWidget>
#include <QFont>
#include <QPushButton>

#include <functional>

// ============================================================================

// Forward Declarations:
class CSearchResultsTreeView;
class CScriptureBrowser;
class CScriptureEdit;
class CDictionaryWidget;
class QwwColorButton;
class CKJVTextFormatConfig;
class QListWidgetItem;
class CBibleWordDiffListModel;

// ============================================================================


class CHighlighterColorButtonSignalReflector : public QObject
{
	Q_OBJECT

public:
	CHighlighterColorButtonSignalReflector(CKJVTextFormatConfig *pConfigurator, const QString &strUserDefinedHighlighterName);
	~CHighlighterColorButtonSignalReflector();

	QString highlighterName() const { return m_strUserDefinedHighlighterName; }

signals:
	void colorPicked(const QString &strUserDefinedHighlighterName, const QColor &color);
	void colorClicked(const QString &strUserDefinedHighlighterName);
	void enableChanged(const QString &strUserDefinedHighlighterName, bool bEnabled);

public slots:
	void en_colorPicked(const QColor &color);
	void en_clicked();
	void en_enableClicked(bool bEnabled);

protected slots:
	virtual void en_setTextBrightness(bool bInvert, int nBrightness) = 0;
	virtual void en_adjustDialogElementBrightnessChanged(bool bAdjust) = 0;

private:
	QString m_strUserDefinedHighlighterName;		// Name of User Defined Highlighter to use from persistent settings
};

// ============================================================================

#include "ui_KJVTextFormatConfig.h"

class CKJVTextFormatConfig : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVTextFormatConfig(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent = nullptr);						// Databases for the preview
	~CKJVTextFormatConfig();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

public slots:
	void en_ApplicationFontChanged(const QFont &font);
	void en_ScriptureBrowserFontChanged(const QFont &font);
	void en_SearchResultsFontChanged(const QFont &font);
	void en_DictionaryFontChanged(const QFont &font);
	void en_ApplicationFontSizeChanged(double nFontSize);
	void en_ScriptureBrowserFontSizeChanged(double nFontSize);
	void en_SearchResultsFontSizeChanged(double nFontSize);
	void en_DictionaryFontSizeChanged(double nFontSize);

	void en_InvertTextBrightnessChanged(bool bInvert);
	void en_TextBrightnessChanged(int nBrightness);
	void en_AdjustDialogElementBrightness(bool bAdjust);

	void en_WordsOfJesusColorPicked(const QColor &color);
	void en_clickedEnableWordsOfJesusColor(bool bEnable);
	void en_SearchResultsColorPicked(const QColor &color);
	void en_CursorTrackerColorPicked(const QColor &color);
	void en_HighlighterColorPicked(const QString &strUserDefinedHighlighterName, const QColor &color);
	void en_HighlighterColorClicked(const QString &strUserDefinedHighlighterName);
	void en_HighlighterEnableChanged(const QString &strUserDefinedHighlighterName, bool bEnabled);

	void en_comboBoxHighlightersTextChanged(const QString &strUserDefinedHighlighterName);
	void en_comboBoxHighlightersTextChanged(int ndxUserDefinedHighlighterName);
	void en_addHighlighterClicked();
	void en_removeHighlighterClicked();
	void en_renameHighlighterClicked();
	void en_currentColorListViewItemChanged(QListWidgetItem *pCurrent, QListWidgetItem *pPrevious);

	void navigateToDemoText();
	void setPreview();

	void en_selectionChangedBrowser();

	void en_userNotesChanged();

private:
	void recalcColorListWidth();

// Private Data:
private:
//	CBibleDatabasePtr m_pBibleDatabase;
//	CDictionaryDatabasePtr m_pDictionaryDatabase;
	CParsedPhrase m_previewSearchPhrase;		// Phrase for searching for preview
	QFont m_fntScriptureBrowser;
	QFont m_fntSearchResults;
	QFont m_fntDictionary;
	bool m_bInvertTextBrightness;
	int m_nTextBrightness;
	bool m_bAdjustDialogElementBrightness;

// UI Private:
private:
	CSearchResultsTreeView *m_pSearchResultsTreeView;
	CScriptureBrowser *m_pScriptureBrowser;
	CDictionaryWidget *m_pDictionaryWidget;
	bool m_bIsDirty;
	bool m_bLoadingData;

	Ui::CKJVTextFormatConfig ui;
};

// ============================================================================

#include "ui_ConfigBibleDatabase.h"

class CConfigBibleDatabase : public QWidget
{
	Q_OBJECT

public:
	explicit CConfigBibleDatabase(QWidget *parent = nullptr);
	~CConfigBibleDatabase();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_changedHideHyphens(bool bHideHyphens);
	void en_changedHyphenHideMode(int index);
	void en_changedHyphenSensitive(bool bHyphenSensitive);

	void en_currentChanged(const QModelIndex &indexCurrent, const QModelIndex &indexPrevious);
	void en_loadBibleDatabase(const QString &strUUID);
	void en_changedAutoLoadStatus(const QString &strUUID, bool bAutoLoad);

	void en_changedMainDBCurrentChanged(int index);

	void en_displayBibleInformation();

private:
	void setSettingControls(const QString &strUUID = QString());

// Data Private:
private:
	CBibleDatabaseListModel *m_pBibleDatabaseListModel;
	CBibleWordDiffListModel *m_pBibleWordDiffListModel;

// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;
	QString m_strSelectedDatabaseUUID;

	Ui::CConfigBibleDatabase ui;
};

// ============================================================================

#if defined(USING_DICTIONARIES)

#include "ui_ConfigDictDatabase.h"

class CConfigDictDatabase : public QWidget
{
	Q_OBJECT

public:
	explicit CConfigDictDatabase(QWidget *parent = nullptr);
	~CConfigDictDatabase();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_currentChanged(const QModelIndex &indexCurrent, const QModelIndex &indexPrevious);
	void en_loadDictDatabase(const QString &strUUID);
	void en_changedAutoLoadStatus(const QString &strUUID, bool bAutoLoad);

	void en_changedMainDBCurrentChanged(int index);

	void en_displayDictInformation();

private:
	void setSettingControls(const QString &strUUID = QString());

// Data Private:
private:
	CDictDatabaseListModel *m_pDictDatabaseListModel;

// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;
	QString m_strSelectedDatabaseUUID;

	Ui::CConfigDictDatabase ui;
};

#endif	// USING_DICTIONARIES

// ============================================================================

#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)

#include "ui_KJVTTSOptionsConfig.h"

class CKJVTTSOptionsConfig : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVTTSOptionsConfig(QWidget *parent = nullptr);
	~CKJVTTSOptionsConfig();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_changedTTSServerURL(const QString &strTTSServerURL);
	void en_changedTTSVoiceSelection(int nIndex);

// Data Private:
private:


// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;

	Ui::CKJVTTSOptionsConfig ui;
};

#endif	// QtSpeech && !EMSCRIPTEN && !VNCSERVER

// ============================================================================

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)

#include "ui_KJVUserNotesDatabaseConfig.h"

class CKJVUserNotesDatabaseConfig : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVUserNotesDatabaseConfig(CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent = nullptr);
	~CKJVUserNotesDatabaseConfig();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_clickedSetPrimaryUserNotesFilename();
	void en_clickedStartNewUserNotesFile();
	void en_changedPrimaryUserNotesFilename(const QString &strFilename);
	void en_changedKeepBackup();
	void en_changedBackupExtension();
	void en_changedAutoSaveTime(int nAutoSaveTime);
	void en_DefaultNoteBackgroundColorPicked(const QColor &color);

private:
	bool loadUserNotesDatabase();				// Isolated to function for busyCursor

// Data Private:
private:
	CUserNotesDatabasePtr m_pUserNotesDatabase;

// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;

	Ui::CKJVUserNotesDatabaseConfig ui;
};

#endif

// ============================================================================

#include "ui_ConfigSearchOptions.h"

class CConfigSearchOptions : public QWidget
{
	Q_OBJECT

public:
	explicit CConfigSearchOptions(QWidget *parent = nullptr);
	~CConfigSearchOptions();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_changedSearchPhraseCompleterFilterMode(int nIndex);
	void en_changedSearchPhraseActivationDelay(int nValue);
	void en_changedAutoCompleterActivationDelay(int nValue);
	void en_changedInitialNumberOfSearchPhrases(int nValue);
	void en_changedHideMatchingPhrasesLists(bool bHideMatchingPhrasesLists);
	void en_changedAutoExpandSearchResultsTree(bool bAutoExpandSearchResultsTree);
	void en_changedHideNotFoundInStatistics(bool bHideNotFoundInStatistics);

// Data Private:
private:


// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;

	Ui::CConfigSearchOptions ui;
};

// ============================================================================

#include "ui_ConfigBrowserOptions.h"

class CConfigBrowserOptions : public QWidget
{
	Q_OBJECT

public:
	explicit CConfigBrowserOptions(QWidget *parent = nullptr);
	~CConfigBrowserOptions();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_changedNavigationActivationDelay(int nValue);
	void en_changedPassageReferenceActivationDelay(int nValue);
	void en_changedShowExcludedSearchResults(bool bShowExcludedSearchResults);
	void en_changedChapterScrollbarMode(int nIndex);
	void en_changedDefaultDisplayMode(int nIndex);
	void en_changedRandomPassageWeightMode(int nIndex);
	void en_changedVerseRenderingMode(int nIndex);
	void en_changedLineHeight(double nLineHeight);
	void en_changedShowPilcrowMarkers(bool bShowPilcrowMarkers);
	void en_changedFootnoteRenderingMode(int nIndex);

// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;

	Ui::CConfigBrowserOptions ui;
};

// ============================================================================

#include "ui_ConfigDictionaryOptions.h"

class CConfigDictionaryOptions : public QWidget
{
	Q_OBJECT

public:
	explicit CConfigDictionaryOptions(QWidget *parent = nullptr);
	~CConfigDictionaryOptions();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_changedDictionaryCompleterFilterMode(int nIndex);
	void en_changedDictionaryActivationDelay(int nValue);

// Data Private:
private:


// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;

	Ui::CConfigDictionaryOptions ui;
};

// ============================================================================

#include "ui_ConfigCopyOptions.h"

class CConfigCopyOptions : public QWidget
{
	Q_OBJECT

public:
	explicit CConfigCopyOptions(CBibleDatabasePtr pBibleDatabase, QWidget *parent = nullptr);		// Database for the preview
	~CConfigCopyOptions();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_changedReferenceDelimiterMode(int nIndex);
	void en_changedReferencesUseAbbreviatedBookNames(bool bUseAbbrBookName);
	void en_changedReferencesInBold(bool bInBold);
	void en_changedReferencesAtEnd(bool bAtEnd);
	void en_changedVerseNumberDelimiterMode(int nIndex);
	void en_changedVerseNumbersUseAbbreviatedBookNames(bool bUseAbbrBookName);
	void en_changedVerseNumbersInBold(bool bInBold);
	void en_changedAddQuotesAroundVerse(bool bAddQuotes);
	void en_changedTransChangeAddWordMode(int nIndex);
	void en_changedVerseRenderingModeCopying(int nIndex);
	void en_changedCopyPilcrowMarkers(bool bCopyPilcrowMarkers);
	void en_changedCopyColophons(bool bCopyColophons);
	void en_changedCopySuperscriptions(bool bCopySuperscriptions);
	void en_changedCopyFontSelection(int nIndex);
	void en_changedFontCopyFont(const QFont &aFont);
	void en_changedFontCopyFontSize(double nFontSize);
	void en_changedCopyMimeType(int nIndex);
	void en_changedSearchResultsAddBlankLineBetweenVerses(bool bAddBlankLine);
	void en_changedSearchResultsVerseCopyOrder(int nIndex);
	void en_changedShowOCntInSearchResultsRefs(bool bShow);
	void en_changedCopyOCntInSearchResultsRefs(bool bCopy);
	void en_changedShowWrdNdxInSearchResultsRefs(bool bShow);
	void en_changedCopyWrdNdxInSearchResultsRefs(bool bCopy);

private:
	void initialize();
	void setVerseCopyPreview();
	void setSearchResultsRefsPreview();

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	QFont m_fntCopyFont;

// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;
	CScriptureEdit *m_pEditCopyOptionPreview;

	Ui::CConfigCopyOptions ui;
};

// ============================================================================

#include "ui_ConfigGeneralSettings.h"

class CConfigGeneralSettings : public QWidget
{
	Q_OBJECT

public:
	explicit CConfigGeneralSettings(CBibleDatabasePtr pBibleDatabase, QWidget *parent = nullptr);
	~CConfigGeneralSettings();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const;

signals:
	void dataChanged(bool bNeedRestart);

// Data Private:
private:

// UI Private:
private:
	Ui::CConfigGeneralSettings ui;
};

// ============================================================================

#include "ui_KJVLocaleConfig.h"

class CKJVLocaleConfig : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVLocaleConfig(QWidget *parent = nullptr);
	~CKJVLocaleConfig();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged(bool bNeedRestart);

private slots:
	void en_changeApplicationLanguage(int nIndex);

// Data Private:
private:

// UI Private:
private:
	bool m_bIsDirty;
	bool m_bLoadingData;

	Ui::CKJVLocaleConfig ui;
};

// ============================================================================

// NOTE: These enumerations must match the configuration pages defined, meaning
//		that on Emscripten and VNC, we must redefine the list accordingly or
//		else weird things will happen because the actual indexes won't match
//		the pages.  Therefore, we will not use explicit numbers here (except for
//		the first two) so that instead they will automatically assign themselves
//		sequentially:

enum CONFIGURATION_PAGE_SELECTION_ENUM {
	CPSE_DEFAULT = -1,
	CPSE_GENERAL_SETTINGS = 0,
	CPSE_COPY_OPTIONS,
	CPSE_TEXT_FORMAT,
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	CPSE_USER_NOTES_DATABASE,
#endif
	CPSE_BIBLE_DATABASE,
#if defined(USING_DICTIONARIES)
	CPSE_DICT_DATABASE,
#endif
	CPSE_LOCALE,
#ifdef USING_QT_SPEECH
	CPSE_TTS_OPTIONS,
#endif
};

class CConfiguration : public QwwConfigWidget
{
	Q_OBJECT

public:
	CConfiguration(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent = nullptr, CONFIGURATION_PAGE_SELECTION_ENUM nInitialPage = CPSE_DEFAULT);
	virtual ~CConfiguration();

	void loadSettings();					// Reloads the settings (used for restore operation when abandoning changes)
	void saveSettings();					// Writes changes back to system
	bool isDirty(CONFIGURATION_PAGE_SELECTION_ENUM nPage = CPSE_DEFAULT) const;

signals:
	void dataChanged(bool bNeedRestart);

private:
	CConfigGeneralSettings *m_pGeneralSettingsConfig;
	CConfigCopyOptions *m_pCopyOptionsConfig;
	CKJVTextFormatConfig *m_pTextFormatConfig;
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	CKJVUserNotesDatabaseConfig *m_pUserNotesDatabaseConfig;
#endif
	CConfigBibleDatabase *m_pBibleDatabaseConfig;
#if defined(USING_DICTIONARIES)
	CConfigDictDatabase *m_pDictDatabaseConfig;
#endif
#if defined(USING_QT_SPEECH) && !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	CKJVTTSOptionsConfig *m_pTTSOptionsConfig;
#endif
	CKJVLocaleConfig *m_pLocaleConfig;
};

// ============================================================================

class CConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	CConfigurationDialog(CBibleDatabasePtr pBibleDatabase, CDictionaryDatabasePtr pDictionary, QWidget *parent = nullptr, CONFIGURATION_PAGE_SELECTION_ENUM nInitialPage = CPSE_DEFAULT);
	virtual ~CConfigurationDialog();

	bool restartApp() const { return m_bRestartApp; }

public slots:
	virtual void en_dataChanged(bool bNeedRestart);
	virtual void accept() override;
	virtual void reject() override;
	virtual void apply();
	virtual void restore(bool bRecopy);		// Restores setting changes (converse of apply).  If bRecopy=true, a fresh copy of the settings are made to continue edit.  If false, they are switched back to the original and left

private slots:
	void en_configurationIndexChanged(int index);
	void en_setToLastIndex();

private:
	void promptRestart(std::function<void (bool bRestart)> fnCompletion);

private:
	int m_nLastIndex;						// Last Configuration Index active
	bool m_bHandlingPageSwap;				// Set to true while we are handling a page swap, used as a safe-guard in case we need to switch pages back
	CConfiguration *m_pConfiguration;
	QDialogButtonBox *m_pButtonBox;
	bool m_bNeedRestart;					// True if we need a restart to apply these settings, but haven't prompted user yet -- user will be prompted on accept/apply...
	bool m_bRestartApp;						// True if we need to restart app and user has accepted for us to
};

// ============================================================================

#endif	// CONFIGURATION_H

