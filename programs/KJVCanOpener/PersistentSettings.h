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

#ifndef PERSISTENTSETTINGS_H
#define PERSISTENTSETTINGS_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QFont>
#include <QColor>
#include <QList>
#include <QMap>

#include "dbstruct.h"
#include "SearchCompleter.h"
#include "PhraseEdit.h"

// ============================================================================

extern QString groupCombine(const QString &strSubgroup, const QString &strGroup);

enum CHAPTER_SCROLLBAR_MODE_ENUM {
	CSME_NONE = 0,
	CSME_LEFT = 1,
	CSME_RIGHT = 2
};

// ============================================================================

class CPersistentSettings : public QObject
{
	Q_OBJECT
private:				// Enforce Singleton:
	CPersistentSettings(QObject *parent = 0);

public:
	~CPersistentSettings();
	void setStealthMode(const QString &strFilename);
	static CPersistentSettings *instance();
	QSettings *settings();

	const CPhraseList userPhrases(const QString &strUUID) const;
	void setUserPhrases(const QString &strUUID, const CPhraseList &lstUserPhrases);
	void addUserPhrase(const QString &strUUID, const CPhraseEntry &aPhraseEntry);
	void removeUserPhrase(const QString &strUUID, const CPhraseEntry &aPhraseEntry);

	QFont fontScriptureBrowser() const { return m_pPersistentSettingData->m_fntScriptureBrowser; }
	QFont fontSearchResults() const { return m_pPersistentSettingData->m_fntSearchResults; }
	QFont fontDictionary() const { return m_pPersistentSettingData->m_fntDictionary; }

	bool invertTextBrightness() const { return m_pPersistentSettingData->m_bInvertTextBrightness; }
	int textBrightness() const { return m_pPersistentSettingData->m_nTextBrightness; }
	bool adjustDialogElementBrightness() const { return m_pPersistentSettingData->m_bAdjustDialogElementBrightness; }

	static QColor textForegroundColor(bool bInvert, int nBrightness);
	static QColor textBackgroundColor(bool bInvert, int nBrightness);

	QColor textForegroundColor() const;
	QColor textBackgroundColor() const;

	QColor colorWordsOfJesus() const { return m_pPersistentSettingData->m_clrWordsOfJesus; }
	QColor colorSearchResults() const { return m_pPersistentSettingData->m_clrSearchResults; }
	QColor colorCursorFollow() const { return m_pPersistentSettingData->m_clrCursorFollow; }

	QColor colorDefaultNoteBackground() const { return m_pPersistentSettingData->m_clrDefaultNoteBackground; }

	CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM searchPhraseCompleterFilterMode() const { return m_pPersistentSettingData->m_nSearchPhraseCompleterFilterMode; }
	int searchActivationDelay() const { return m_pPersistentSettingData->m_nSearchActivationDelay; }
	int initialNumberOfSearchPhrases() const { return m_pPersistentSettingData->m_nInitialNumberOfSearchPhrases; }
	bool autoExpandSearchResultsTree() const { return m_pPersistentSettingData->m_bAutoExpandSearchResultsTree; }

	int navigationActivationDelay() const { return m_pPersistentSettingData->m_nNavigationActivationDelay; }
	int passageReferenceActivationDelay() const { return m_pPersistentSettingData->m_nPassageReferenceActivationDelay; }
	bool showExcludedSearchResultsInBrowser() const { return m_pPersistentSettingData->m_bShowExcludedSearchResultsInBrowser; }
	CHAPTER_SCROLLBAR_MODE_ENUM chapterScrollbarMode() const { return m_pPersistentSettingData->m_nChapterScrollbarMode; }
	CPhraseNavigator::VERSE_RENDERING_MODE_ENUM verseRenderingMode() const { return m_pPersistentSettingData->m_nVerseRenderingMode; }
	bool showPilcrowMarkers() const { return m_pPersistentSettingData->m_bShowPilcrowMarkers; }

	CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM dictionaryCompleterFilterMode() const { return m_pPersistentSettingData->m_nDictionaryCompleterFilterMode; }
	int dictionaryActivationDelay() const { return m_pPersistentSettingData->m_nDictionaryActivationDelay; }

	CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM referenceDelimiterMode() const { return m_pPersistentSettingData->m_nReferenceDelimiterMode; }
	bool referencesUseAbbreviatedBookNames() const { return m_pPersistentSettingData->m_bReferencesUseAbbreviatedBookNames; }
	bool referencesInBold() const { return m_pPersistentSettingData->m_bReferencesInBold; }
	bool referencesAtEnd() const { return m_pPersistentSettingData->m_bReferencesAtEnd; }
	CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM verseNumberDelimiterMode() const { return m_pPersistentSettingData->m_nVerseNumberDelimiterMode; }
	bool verseNumbersUseAbbreviatedBookNames() const { return m_pPersistentSettingData->m_bVerseNumbersUseAbbreviatedBookNames; }
	bool verseNumbersInBold() const { return m_pPersistentSettingData->m_bVerseNumbersInBold; }
	bool addQuotesAroundVerse() const { return m_pPersistentSettingData->m_bAddQuotesAroundVerse; }
	CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM transChangeAddWordMode() const { return m_pPersistentSettingData->m_nTransChangeAddWordMode; }
	CPhraseNavigator::VERSE_RENDERING_MODE_ENUM verseRenderingModeCopying() const { return m_pPersistentSettingData->m_nVerseRenderingModeCopying; }
	bool copyPilcrowMarkers() const { return m_pPersistentSettingData->m_bCopyPilcrowMarkers; }
	CPhraseNavigator::COPY_FONT_SELECTION_ENUM copyFontSelection() const { return m_pPersistentSettingData->m_nCopyFontSelection; }
	QFont fontCopyFont() const { return m_pPersistentSettingData->m_fntCopyFont; }

	bool showOCntInSearchResultsRefs() const { return m_pPersistentSettingData->m_bShowOCntInSearchResultsRefs; }
	bool copyOCntInSearchResultsRefs() const { return m_pPersistentSettingData->m_bCopyOCntInSearchResultsRefs; }
	bool showWrdNdxInSearchResultsRefs() const { return m_pPersistentSettingData->m_bShowWrdNdxInSearchResultsRefs; }
	bool copyWrdNdxInSearchResultsRefs() const { return m_pPersistentSettingData->m_bCopyWrdNdxInSearchResultsRefs; }

	QStringList bibleDatabaseSettingsUUIDList() const;
	TBibleDatabaseSettings bibleDatabaseSettings(const QString &strUUID) const;
	void setBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings);
	void setMainBibleDatabaseUUID(const QString &strUUID);
	QString mainBibleDatabaseUUID() const { return m_pPersistentSettingData->m_strMainBibleDatabaseUUID; }

	void togglePersistentSettingData(bool bCopy);

signals:
	void changedUserPhrases(const QString &strUUID);

	void fontChangedScriptureBrowser(const QFont &aFont);
	void fontChangedSearchResults(const QFont &aFont);
	void fontChangedDictionary(const QFont &aFont);

	void adjustDialogElementBrightnessChanged(bool bAdjust);
	void changedTextBrightness(bool bInvert, int nBrightness);

	void changedColorWordsOfJesus(const QColor &color);
	void changedColorSearchResults(const QColor &color);
	void changedColorCursorFollow(const QColor &color);

	void changedColorDefaultNoteBackground(const QColor &color);

	void changedSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM);
	void changedSearchPhraseActivationDelay(int nDelay);
	void changedInitialNumberOfSearchPhrases(int nInitialNumberOfSearchPhrases);
	void changedAutoExpandSearchResultsTree(bool bAutoExpandSearchResultsTree);

	void changedNavigationActivationDelay(int nDelay);
	void changedPassageReferenceActivationDelay(int nDelay);
	void changedShowExcludedSearchResultsInBrowser(bool bShowExcludedSearchResults);
	void changedChapterScrollbarMode(CHAPTER_SCROLLBAR_MODE_ENUM nMode);
	void changedVerseRenderingMode(CPhraseNavigator::VERSE_RENDERING_MODE_ENUM nMode);
	void changedShowPilcrowMarkers(bool bShowPilcrowMarkers);

	void changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM);
	void changedDictionaryActivationDelay(int nDelay);

	void changedCopyOptions();

	void changedShowOCntInSearchResultsRefs(bool bShow);
	void changedShowWrdNdxInSearchResultsRefs(bool bShow);

	void changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings);
	void changedMainBibleDatabaseSelection(const QString &strUUID);

public slots:
	void setFontScriptureBrowser(const QFont &aFont);
	void setFontSearchResults(const QFont &aFont);
	void setFontDictionary(const QFont &aFont);

	void setTextBrightness(bool bInvert, int nBrightness);
	void setAdjustDialogElementBrightness(bool bAdjust);

	void setColorWordsOfJesus(const QColor &color);
	void setColorSearchResults(const QColor &color);
	void setColorCursorFollow(const QColor &color);

	void setColorDefaultNoteBackground(const QColor &color);

	void setSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode);
	void setSearchActivationDelay(int nDelay);
	void setInitialNumberOfSearchPhrases(int nInitialNumberOfSearchPhrases);
	void setAutoExpandSearchResultsTree(bool bAutoExpandSearchResultsTree);

	void setNavigationActivationDelay(int nDelay);
	void setPassageReferenceActivationDelay(int nDelay);
	void setShowExcludedSearchResultsInBrowser(bool bShowExcludedSearchResults);
	void setChapterScrollbarMode(CHAPTER_SCROLLBAR_MODE_ENUM nMode);
	void setVerseRenderingMode(CPhraseNavigator::VERSE_RENDERING_MODE_ENUM nMode);
	void setShowPilcrowMarkers(bool bShowPilcrowMarkers);

	void setDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode);
	void setDictionaryActivationDelay(int nDelay);

	void setReferenceDelimiterMode(CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM nMode);
	void setReferencesUseAbbreviatedBookNames(bool bUseAbbrBookNames);
	void setReferencesInBold(bool bInBold);
	void setReferencesAtEnd(bool bAtEnd);
	void setVerseNumberDelimiterMode(CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM nMode);
	void setVerseNumbersUseAbbreviatedBookNames(bool bUseAbbrBookNames);
	void setVerseNumbersInBold(bool bInBold);
	void setAddQuotesAroundVerse(bool bAddQuotes);
	void setTransChangeAddWordMode(CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM nMode);
	void setVerseRenderingModeCopying(CPhraseNavigator::VERSE_RENDERING_MODE_ENUM nMode);
	void setCopyPilcrowMarkers(bool bCopyPilcrowMarkers);
	void setCopyFontSelection(CPhraseNavigator::COPY_FONT_SELECTION_ENUM nCopyFontSelection);
	void setFontCopyFont(const QFont &aFont);

	void setShowOCntInSearchResultsRefs(bool bShow);
	void setCopyOCntInSearchResultsRefs(bool bCopy);
	void setShowWrdNdxInSearchResultsRefs(bool bShow);
	void setCopyWrdNdxInSearchResultsRefs(bool bCopy);

private:
	// m_PersistentSettingData1 and m_PersistentSettingData2 are
	//		two complete copies of our persistent setting data.  Only
	//		one will be active and used at any given time.  Classes
	//		like KJVConfiguration can request that the settings be
	//		copied to the other copy and that other copy made to be
	//		the main copy for preview purposes so that controls will
	//		appear with the new set of settings.  When it's done with
	//		it, it can either revert back to the original copy without
	//		copying it back or leave the new settings to be the new
	//		settings.
	class TPersistentSettingData {
	public:
		TPersistentSettingData();

		QFont m_fntScriptureBrowser;
		QFont m_fntSearchResults;
		QFont m_fntDictionary;
		bool m_bInvertTextBrightness;
		int m_nTextBrightness;
		bool m_bAdjustDialogElementBrightness;
		QColor m_clrWordsOfJesus;						// Color for the Words of Jesus (usually "red")
		QColor m_clrSearchResults;						// Color for the Search Results text we find (usually "blue")
		QColor m_clrCursorFollow;						// Color for the CursorFollow underline highlighter (usually "blue")
		// ----
		QColor m_clrDefaultNoteBackground;				// Default Note Background Color (usually sticky-note yellow)
		// ----
		CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM m_nSearchPhraseCompleterFilterMode;
		int m_nSearchActivationDelay;					// Search Delay to set on all Search Phrases
		int m_nInitialNumberOfSearchPhrases;			// Number of Search Phrases to create to opening a new Search Window
		bool m_bAutoExpandSearchResultsTree;			// True to auto-expand Search Results Tree on Search
		// ----
		int m_nNavigationActivationDelay;				// Navigation Delay to set on Scripture Browser controls
		int m_nPassageReferenceActivationDelay;			// Manually Typed Passage Reference Activation Delay to set on Scripture Browser controls
		bool m_bShowExcludedSearchResultsInBrowser;		// True if Excluded Search Results will be Highlighted in the Scripture Browser
		CHAPTER_SCROLLBAR_MODE_ENUM m_nChapterScrollbarMode;	// Location of Chapter Scrollbar relative to the Scripture Browser
		CPhraseNavigator::VERSE_RENDERING_MODE_ENUM m_nVerseRenderingMode;	// How to display verses within the Scripture Browser
		bool m_bShowPilcrowMarkers;						// If enabled, the pilcrow symbols (¶) will be rendered
		// ----
		CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM m_nDictionaryCompleterFilterMode;
		int m_nDictionaryActivationDelay;				// Delay for Dictionary word change until activation
		// ----
		CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM m_nReferenceDelimiterMode;
		bool m_bReferencesUseAbbreviatedBookNames;
		bool m_bReferencesInBold;
		bool m_bReferencesAtEnd;
		CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM m_nVerseNumberDelimiterMode;
		bool m_bVerseNumbersUseAbbreviatedBookNames;
		bool m_bVerseNumbersInBold;
		bool m_bAddQuotesAroundVerse;
		CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM m_nTransChangeAddWordMode;
		CPhraseNavigator::VERSE_RENDERING_MODE_ENUM m_nVerseRenderingModeCopying;	// How to copy verses from Scripture Browser (VPL, FF, etc)
		bool m_bCopyPilcrowMarkers;						// If enabled, the pilcrow symbols (¶) will be copied
		CPhraseNavigator::COPY_FONT_SELECTION_ENUM m_nCopyFontSelection;	// Font to use for the copy font hint in the generated HTML
		QFont m_fntCopyFont;							// Font to use for the Copy Font for CFSE_COPY_FONT mode
		// ----
		bool m_bShowOCntInSearchResultsRefs;			// True if showing Occurrence Counts in Search Results References
		bool m_bCopyOCntInSearchResultsRefs;			// True if copying Occurrence Counts in Search Results References <--- Considered a Copy Option and will use the changedCopyOptions() signal
		bool m_bShowWrdNdxInSearchResultsRefs;			// True if showing Word Indexes in Search Results References
		bool m_bCopyWrdNdxInSearchResultsRefs;			// True if copying Word Indexes in Search Results References <--- Considered a Copy Option and will use the changedCopyOptions() signal
		// ----
		TBibleDatabaseSettingsMap m_mapBibleDatabaseSettings;		// Map of Bible UUIDs to settings for saving/preserving (written in KJVCanOpener shutdown, read in myApplication execute)
		QString m_strMainBibleDatabaseUUID;				// UUID of Main Bible Database to load (written in KJVCanOpener shutdown, read in myApplication execute)
	} m_PersistentSettingData1, m_PersistentSettingData2, *m_pPersistentSettingData;

	QSettings *m_pSettings;
	bool m_bStealthMode;								// True if we're either writing to a special alternate file or not writing settings
	QMap<QString, CPhraseList> m_mapUserPhrases;		// User-defined phrases read per Bible Database
};

// ============================================================================

#endif // PERSISTENTSETTINGS_H
