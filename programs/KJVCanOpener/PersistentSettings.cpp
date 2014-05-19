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

#include "PersistentSettings.h"

#if !defined(NO_PERSISTENT_SETTINGS) && !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)
#include "version.h"
#endif

#include <QCoreApplication>
#include <QApplication>

#include <assert.h>

// ============================================================================

namespace
{
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------

}

// ============================================================================

QString groupCombine(const QString &strSubgroup, const QString &strGroup)
{
	QString strCombinedGroup = strSubgroup;

	if ((!strSubgroup.isEmpty()) && (!strSubgroup.endsWith('/'))) strCombinedGroup += '/';
	strCombinedGroup += strGroup;

	return strCombinedGroup;
}

// ============================================================================

CPersistentSettings::TPersistentSettingData::TPersistentSettingData()
	:
		// Default Fonts:
#if defined(Q_OS_MAC)
		m_fntScriptureBrowser("DejaVu Serif", 14),
		m_fntSearchResults("DejaVu Serif", 14),
		m_fntDictionary("DejaVu Serif", 14),
#elif defined(EMSCRIPTEN)
		m_fntScriptureBrowser("DejaVu Serif", 14),
		m_fntSearchResults("DejaVu Serif", 14),
		m_fntDictionary("DejaVu Serif", 14),
#elif defined(Q_OS_WIN32)
		m_fntScriptureBrowser("Times New Roman", 12),
		m_fntSearchResults("Times New Roman", 12),
		m_fntDictionary("Times New Roman", 12),
#elif defined(VNCSERVER)
		m_fntScriptureBrowser("DejaVu Serif", 12),
		m_fntSearchResults("DejaVu Serif", 12),
		m_fntDictionary("DejaVu Serif", 12),
#else
		m_fntScriptureBrowser("DejaVu Serif", 12),
		m_fntSearchResults("DejaVu Serif", 12),
		m_fntDictionary("DejaVu Serif", 12),
#endif
		// Default Text Brightness Options:
		m_bInvertTextBrightness(false),
		m_nTextBrightness(100),
		m_bAdjustDialogElementBrightness(false),
		// Default Special Text Colors:
		m_clrWordsOfJesus(QColor("red")),
		m_clrSearchResults(QColor("blue")),
		m_clrCursorFollow(QColor("blue")),
		// Default Note Options:
		m_clrDefaultNoteBackground("#F0F0A0"),			// Default note background (stick-note yellow)
		// Default Search Phrase Options:
		m_nSearchPhraseCompleterFilterMode(CSearchCompleter::SCFME_NORMAL),
#ifndef EMSCRIPTEN
#ifndef IS_CONSOLE_APP
		m_nSearchActivationDelay(QApplication::doubleClickInterval()),
#else
		m_nSearchActivationDelay(0),
#endif
#else
		m_nSearchActivationDelay(QApplication::doubleClickInterval() * 2),
#endif
		m_nInitialNumberOfSearchPhrases(1),
		m_bAutoExpandSearchResultsTree(false),
		// Default Browser Options:
#ifndef EMSCRIPTEN
#ifndef IS_CONSOLE_APP
		m_nNavigationActivationDelay(QApplication::doubleClickInterval()),
#else
		m_nNavigationActivationDelay(0),
#endif
#else
		m_nNavigationActivationDelay(QApplication::doubleClickInterval() * 2),
#endif
		m_nPassageReferenceActivationDelay(2000),
		m_bShowExcludedSearchResultsInBrowser(true),
		m_nChapterScrollbarMode(CSME_NONE),
		m_nVerseRenderingMode(CPhraseNavigator::VRME_FF),
		m_bShowPilcrowMarkers(true),
		// Default Dictionary Options:
		m_nDictionaryCompleterFilterMode(CSearchCompleter::SCFME_NORMAL),
#ifndef EMSCRIPTEN
#ifndef IS_CONSOLE_APP
		m_nDictionaryActivationDelay(QApplication::doubleClickInterval()),
#else
		m_nDictionaryActivationDelay(0),
#endif
#else
		m_nDictionaryActivationDelay(QApplication::doubleClickInterval() * 2),
#endif
		// Default Copy Options:
		m_nReferenceDelimiterMode(CPhraseNavigator::RDME_PARENTHESES),
		m_bReferencesUseAbbreviatedBookNames(false),
		m_bReferencesInBold(true),
		m_bReferencesAtEnd(false),
		m_nVerseNumberDelimiterMode(CPhraseNavigator::RDME_CURLY_BRACES),
		m_bVerseNumbersUseAbbreviatedBookNames(true),
		m_bVerseNumbersInBold(true),
		m_bAddQuotesAroundVerse(true),
		m_nTransChangeAddWordMode(CPhraseNavigator::TCAWME_ITALICS),
		m_nVerseRenderingModeCopying(CPhraseNavigator::VRME_FF),
		m_bCopyPilcrowMarkers(true),
		m_bCopyColophons(false),
		m_bCopySuperscriptions(false),
		m_nCopyFontSelection(CPhraseNavigator::CFSE_NONE),
		m_fntCopyFont(m_fntScriptureBrowser),
		// ----
		m_bSearchResultsAddBlankLineBetweenVerses(false),
		// ----
		m_bShowOCntInSearchResultsRefs(true),
		m_bCopyOCntInSearchResultsRefs(true),
		m_bShowWrdNdxInSearchResultsRefs(true),
		m_bCopyWrdNdxInSearchResultsRefs(true),
		// ----
		m_strMainBibleDatabaseUUID(bibleDescriptor(BDE_KJV).m_strUUID),			// Default to reading KJV
		// ----
		m_strApplicationLanguage(QString())				// Default to System Locale language
{

}

// ============================================================================

CPersistentSettings::CPersistentSettings(QObject *parent)
	:	QObject(parent),
		m_pPersistentSettingData(&m_PersistentSettingData1),
		m_pSettings(NULL),
		m_bStealthMode(false)
{
#ifndef NO_PERSISTENT_SETTINGS
	// Must set these in main() before caling settings!:
	assert(QCoreApplication::applicationName().compare(VER_APPNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationName().compare(VER_ORGNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationDomain().compare(VER_ORGDOMAIN_STR_QT) == 0);
#else
	// If running without Persistent Settings, use Stealth Mode:
	setStealthMode(QString());
#endif
}

CPersistentSettings::~CPersistentSettings()
{
}

void CPersistentSettings::setStealthMode(const QString &strFilename)
{
	assert(m_pSettings == NULL);			// Can only set once
	m_bStealthMode = true;
	if (!strFilename.isEmpty()) {
		m_pSettings = new QSettings(strFilename, QSettings::IniFormat, this);
	}
}

CPersistentSettings *CPersistentSettings::instance()
{
	static CPersistentSettings thePersistentSettings;
	return &thePersistentSettings;
}

QSettings *CPersistentSettings::settings()
{
	if ((m_pSettings == NULL) && (!m_bStealthMode)) {
		// Create NON-Stealth Settings:
		m_pSettings = new QSettings(this);
	}
	return m_pSettings;
}

const CPhraseList CPersistentSettings::userPhrases(const QString &strUUID) const
{
	return m_mapUserPhrases.value(strUUID);
}

void CPersistentSettings::setUserPhrases(const QString &strUUID, const CPhraseList &lstUserPhrases)
{
	m_mapUserPhrases[strUUID] = lstUserPhrases;
	emit changedUserPhrases(strUUID);
}

void CPersistentSettings::addUserPhrase(const QString &strUUID, const CPhraseEntry &aPhraseEntry)
{
	if (userPhrases(strUUID).contains(aPhraseEntry)) return;
	m_mapUserPhrases[strUUID].append(aPhraseEntry);
	emit changedUserPhrases(strUUID);
}

void CPersistentSettings::removeUserPhrase(const QString &strUUID, const CPhraseEntry &aPhraseEntry)
{
	if (!userPhrases(strUUID).contains(aPhraseEntry)) return;
	m_mapUserPhrases[strUUID].removeAll(aPhraseEntry);
	emit changedUserPhrases(strUUID);
}

void CPersistentSettings::togglePersistentSettingData(bool bCopy)
{
	TPersistentSettingData *pSource = ((m_pPersistentSettingData == &m_PersistentSettingData1) ? &m_PersistentSettingData1 : &m_PersistentSettingData2);
	TPersistentSettingData *pTarget = ((m_pPersistentSettingData == &m_PersistentSettingData1) ? &m_PersistentSettingData2 : &m_PersistentSettingData1);

	if (bCopy) *pTarget = *pSource;

	m_pPersistentSettingData = pTarget;

	// Signal changes if we aren't copying and something changed:
	if (!bCopy) {
		if (pSource->m_fntScriptureBrowser != pTarget->m_fntScriptureBrowser) emit fontChangedScriptureBrowser(pTarget->m_fntScriptureBrowser);
		if (pSource->m_fntSearchResults != pTarget->m_fntSearchResults) emit fontChangedSearchResults(pTarget->m_fntSearchResults);
		if (pSource->m_fntDictionary != pTarget->m_fntDictionary) emit fontChangedDictionary(pTarget->m_fntDictionary);

		if ((pSource->m_bInvertTextBrightness != pTarget->m_bInvertTextBrightness) ||
			(pSource->m_nTextBrightness != pTarget->m_nTextBrightness)) emit changedTextBrightness(pTarget->m_bInvertTextBrightness, pTarget->m_nTextBrightness);
		if (pSource->m_bAdjustDialogElementBrightness != pTarget->m_bAdjustDialogElementBrightness) emit adjustDialogElementBrightnessChanged(pTarget->m_bAdjustDialogElementBrightness);

		if (pSource->m_clrWordsOfJesus != pTarget->m_clrWordsOfJesus) emit changedColorWordsOfJesus(pTarget->m_clrWordsOfJesus);
		if (pSource->m_clrSearchResults != pTarget->m_clrSearchResults) emit changedColorSearchResults(pTarget->m_clrSearchResults);
		if (pSource->m_clrCursorFollow != pTarget->m_clrCursorFollow) emit changedColorCursorFollow(pTarget->m_clrCursorFollow);

		if (pSource->m_clrDefaultNoteBackground != pTarget->m_clrDefaultNoteBackground) emit changedColorDefaultNoteBackground(pTarget->m_clrDefaultNoteBackground);

		if (pSource->m_nSearchPhraseCompleterFilterMode != pTarget->m_nSearchPhraseCompleterFilterMode) emit changedSearchPhraseCompleterFilterMode(pTarget->m_nSearchPhraseCompleterFilterMode);
		if (pSource->m_nSearchActivationDelay != pTarget->m_nSearchActivationDelay) emit changedSearchPhraseActivationDelay(pTarget->m_nSearchActivationDelay);
		if (pSource->m_nInitialNumberOfSearchPhrases != pTarget->m_nInitialNumberOfSearchPhrases) emit changedInitialNumberOfSearchPhrases(pTarget->m_nInitialNumberOfSearchPhrases);
		if (pSource->m_bAutoExpandSearchResultsTree != pTarget->m_bAutoExpandSearchResultsTree) emit changedAutoExpandSearchResultsTree(pTarget->m_bAutoExpandSearchResultsTree);

		if (pSource->m_nNavigationActivationDelay != pTarget->m_nNavigationActivationDelay) emit changedNavigationActivationDelay(pTarget->m_nNavigationActivationDelay);
		if (pSource->m_nPassageReferenceActivationDelay != pTarget->m_nPassageReferenceActivationDelay) emit changedPassageReferenceActivationDelay(pTarget->m_nPassageReferenceActivationDelay);
		if (pSource->m_bShowExcludedSearchResultsInBrowser != pTarget->m_bShowExcludedSearchResultsInBrowser) emit changedShowExcludedSearchResultsInBrowser(pTarget->m_bShowExcludedSearchResultsInBrowser);
		if (pSource->m_nChapterScrollbarMode != pTarget->m_nChapterScrollbarMode) emit changedChapterScrollbarMode(pTarget->m_nChapterScrollbarMode);
		if (pSource->m_nVerseRenderingMode != pTarget->m_nVerseRenderingMode) emit changedVerseRenderingMode(pTarget->m_nVerseRenderingMode);
		if (pSource->m_bShowPilcrowMarkers != pTarget->m_bShowPilcrowMarkers) emit changedShowPilcrowMarkers(pTarget->m_bShowPilcrowMarkers);

		if (pSource->m_nDictionaryCompleterFilterMode != pTarget->m_nDictionaryCompleterFilterMode) emit changedDictionaryCompleterFilterMode(pTarget->m_nDictionaryCompleterFilterMode);
		if (pSource->m_nDictionaryActivationDelay != pTarget->m_nDictionaryActivationDelay) emit changedDictionaryActivationDelay(pTarget->m_nDictionaryActivationDelay);

		if ((pSource->m_nReferenceDelimiterMode != pTarget->m_nReferenceDelimiterMode) ||
			(pSource->m_bReferencesUseAbbreviatedBookNames != pTarget->m_bReferencesUseAbbreviatedBookNames) ||
			(pSource->m_bReferencesInBold != pTarget->m_bReferencesInBold) ||
			(pSource->m_bReferencesAtEnd != pTarget->m_bReferencesAtEnd) ||
			(pSource->m_nVerseNumberDelimiterMode != pTarget->m_nVerseNumberDelimiterMode) ||
			(pSource->m_bVerseNumbersUseAbbreviatedBookNames != pTarget->m_bVerseNumbersUseAbbreviatedBookNames) ||
			(pSource->m_bVerseNumbersInBold != pTarget->m_bVerseNumbersInBold) ||
			(pSource->m_bAddQuotesAroundVerse != pTarget->m_bAddQuotesAroundVerse) ||
			(pSource->m_nTransChangeAddWordMode != pTarget->m_nTransChangeAddWordMode) ||
			(pSource->m_nVerseRenderingModeCopying != pTarget->m_nVerseRenderingModeCopying) ||
			(pSource->m_bCopyPilcrowMarkers != pTarget->m_bCopyPilcrowMarkers) ||
			(pSource->m_bCopyColophons != pTarget->m_bCopyColophons) ||
			(pSource->m_bCopySuperscriptions != pTarget->m_bCopySuperscriptions) ||
			(pSource->m_nCopyFontSelection != pTarget->m_nCopyFontSelection) ||
			(pSource->m_fntCopyFont != pTarget->m_fntCopyFont) ||
			(pSource->m_bSearchResultsAddBlankLineBetweenVerses != pTarget->m_bSearchResultsAddBlankLineBetweenVerses) ||
			(pSource->m_bCopyOCntInSearchResultsRefs != pTarget->m_bCopyOCntInSearchResultsRefs) ||
			(pSource->m_bCopyWrdNdxInSearchResultsRefs != pTarget->m_bCopyWrdNdxInSearchResultsRefs)) emit changedCopyOptions();

		if (pSource->m_bShowOCntInSearchResultsRefs != pTarget->m_bShowOCntInSearchResultsRefs) emit changedShowOCntInSearchResultsRefs(pTarget->m_bShowOCntInSearchResultsRefs);
		if (pSource->m_bShowWrdNdxInSearchResultsRefs != pTarget->m_bShowWrdNdxInSearchResultsRefs) emit changedShowWrdNdxInSearchResultsRefs(pTarget->m_bShowWrdNdxInSearchResultsRefs);

		if (pSource->m_mapBibleDatabaseSettings != pTarget->m_mapBibleDatabaseSettings) {
			for (TBibleDatabaseSettingsMap::ConstIterator itrUUIDs = pTarget->m_mapBibleDatabaseSettings.constBegin();
															itrUUIDs != pTarget->m_mapBibleDatabaseSettings.constEnd();
															++itrUUIDs) {
				emit changedBibleDatabaseSettings(itrUUIDs.key(), itrUUIDs.value());
			}
		}
		if (pSource->m_strMainBibleDatabaseUUID != pTarget->m_strMainBibleDatabaseUUID) emit changedMainBibleDatabaseSelection(pTarget->m_strMainBibleDatabaseUUID);
		if (pSource->m_strApplicationLanguage != pTarget->m_strApplicationLanguage) emit changedApplicationLanguage(pTarget->m_strApplicationLanguage);
	}
}

// ----------------------------------------------------------------------------

void CPersistentSettings::setFontScriptureBrowser(const QFont &aFont)
{
	if (m_pPersistentSettingData->m_fntScriptureBrowser != aFont) {
		m_pPersistentSettingData->m_fntScriptureBrowser = aFont;
		emit fontChangedScriptureBrowser(aFont);
	}
}

void CPersistentSettings::setFontSearchResults(const QFont &aFont)
{
	if (m_pPersistentSettingData->m_fntSearchResults != aFont) {
		m_pPersistentSettingData->m_fntSearchResults = aFont;
		emit fontChangedSearchResults(aFont);
	}
}

void CPersistentSettings::setFontDictionary(const QFont &aFont)
{
	if (m_pPersistentSettingData->m_fntDictionary != aFont) {
		m_pPersistentSettingData->m_fntDictionary = aFont;
		emit fontChangedDictionary(aFont);
	}
}

void CPersistentSettings::setTextBrightness(bool bInvert, int nBrightness)
{
	assert((nBrightness >= 0) && (nBrightness <= 100));
	if (nBrightness < 0) nBrightness = 0;
	if (nBrightness > 100) nBrightness = 100;
	if ((m_pPersistentSettingData->m_nTextBrightness != nBrightness) ||
		(m_pPersistentSettingData->m_bInvertTextBrightness != bInvert)) {
		m_pPersistentSettingData->m_bInvertTextBrightness = bInvert;
		m_pPersistentSettingData->m_nTextBrightness = nBrightness;
		emit changedTextBrightness(m_pPersistentSettingData->m_bInvertTextBrightness, m_pPersistentSettingData->m_nTextBrightness);
	}
}

void CPersistentSettings::setAdjustDialogElementBrightness(bool bAdjust)
{
	if (m_pPersistentSettingData->m_bAdjustDialogElementBrightness != bAdjust) {
		m_pPersistentSettingData->m_bAdjustDialogElementBrightness = bAdjust;
		emit adjustDialogElementBrightnessChanged(m_pPersistentSettingData->m_bAdjustDialogElementBrightness);
	}
}

QColor CPersistentSettings::textForegroundColor() const
{
	return textForegroundColor(m_pPersistentSettingData->m_bInvertTextBrightness, m_pPersistentSettingData->m_nTextBrightness);
}

QColor CPersistentSettings::textForegroundColor(bool bInvert, int nBrightness)
{
	assert((nBrightness >= 0) && (nBrightness <= 100));
	QColor clrForeground = (bInvert ? QColor(255, 255, 255) : QColor(0, 0, 0));
	return (bInvert ? clrForeground.darker(300 - (nBrightness * 2)) : clrForeground.lighter(300 - (nBrightness * 2)));
}

QColor CPersistentSettings::textBackgroundColor() const
{
	return textBackgroundColor(m_pPersistentSettingData->m_bInvertTextBrightness, m_pPersistentSettingData->m_nTextBrightness);
}

QColor CPersistentSettings::textBackgroundColor(bool bInvert, int nBrightness)
{
	assert((nBrightness >= 0) && (nBrightness <= 100));
	QColor clrBackground = (bInvert ? QColor(0, 0, 0) : QColor(255, 255, 255));
	return (bInvert ? clrBackground.lighter(300 - (nBrightness * 2)) : clrBackground.darker(300 - (nBrightness * 2)));
}

void CPersistentSettings::setColorWordsOfJesus(const QColor &color)
{
	if (m_pPersistentSettingData->m_clrWordsOfJesus != color) {
		m_pPersistentSettingData->m_clrWordsOfJesus = color;
		emit changedColorWordsOfJesus(m_pPersistentSettingData->m_clrWordsOfJesus);
	}
}

void CPersistentSettings::setColorSearchResults(const QColor &color)
{
	if (m_pPersistentSettingData->m_clrSearchResults != color) {
		m_pPersistentSettingData->m_clrSearchResults = color;
		emit changedColorSearchResults(m_pPersistentSettingData->m_clrSearchResults);
	}
}

void CPersistentSettings::setColorCursorFollow(const QColor &color)
{
	if (m_pPersistentSettingData->m_clrCursorFollow != color) {
		m_pPersistentSettingData->m_clrCursorFollow = color;
		emit changedColorCursorFollow(m_pPersistentSettingData->m_clrCursorFollow);
	}
}

void CPersistentSettings::setColorDefaultNoteBackground(const QColor &color)
{
	if (m_pPersistentSettingData->m_clrDefaultNoteBackground != color) {
		m_pPersistentSettingData->m_clrDefaultNoteBackground = color;
		emit changedColorDefaultNoteBackground(m_pPersistentSettingData->m_clrDefaultNoteBackground);
	}
}

void CPersistentSettings::setSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nSearchPhraseCompleterFilterMode != nMode) {
		m_pPersistentSettingData->m_nSearchPhraseCompleterFilterMode = nMode;
		emit changedSearchPhraseCompleterFilterMode(m_pPersistentSettingData->m_nSearchPhraseCompleterFilterMode);
	}
}

void CPersistentSettings::setSearchActivationDelay(int nDelay)
{
	if (m_pPersistentSettingData->m_nSearchActivationDelay != nDelay) {
		m_pPersistentSettingData->m_nSearchActivationDelay = nDelay;
		emit changedSearchPhraseActivationDelay(m_pPersistentSettingData->m_nSearchActivationDelay);
	}
}

void CPersistentSettings::setInitialNumberOfSearchPhrases(int nInitialNumberOfSearchPhrases)
{
	if (m_pPersistentSettingData->m_nInitialNumberOfSearchPhrases != nInitialNumberOfSearchPhrases) {
		m_pPersistentSettingData->m_nInitialNumberOfSearchPhrases = nInitialNumberOfSearchPhrases;
		emit changedInitialNumberOfSearchPhrases(m_pPersistentSettingData->m_nInitialNumberOfSearchPhrases);
	}
}

void CPersistentSettings::setAutoExpandSearchResultsTree(bool bAutoExpandSearchResultsTree)
{
	if (m_pPersistentSettingData->m_bAutoExpandSearchResultsTree != bAutoExpandSearchResultsTree) {
		m_pPersistentSettingData->m_bAutoExpandSearchResultsTree = bAutoExpandSearchResultsTree;
		emit changedAutoExpandSearchResultsTree(m_pPersistentSettingData->m_bAutoExpandSearchResultsTree);
	}
}

void CPersistentSettings::setNavigationActivationDelay(int nDelay)
{
	if (m_pPersistentSettingData->m_nNavigationActivationDelay != nDelay) {
		m_pPersistentSettingData->m_nNavigationActivationDelay = nDelay;
		emit changedNavigationActivationDelay(m_pPersistentSettingData->m_nNavigationActivationDelay);
	}
}

void CPersistentSettings::setPassageReferenceActivationDelay(int nDelay)
{
	if (m_pPersistentSettingData->m_nPassageReferenceActivationDelay != nDelay) {
		m_pPersistentSettingData->m_nPassageReferenceActivationDelay = nDelay;
		emit changedPassageReferenceActivationDelay(m_pPersistentSettingData->m_nPassageReferenceActivationDelay);
	}
}

void CPersistentSettings::setShowExcludedSearchResultsInBrowser(bool bShowExcludedSearchResults)
{
	if (m_pPersistentSettingData->m_bShowExcludedSearchResultsInBrowser != bShowExcludedSearchResults) {
		m_pPersistentSettingData->m_bShowExcludedSearchResultsInBrowser = bShowExcludedSearchResults;
		emit changedShowExcludedSearchResultsInBrowser(m_pPersistentSettingData->m_bShowExcludedSearchResultsInBrowser);
	}
}

void CPersistentSettings::setChapterScrollbarMode(CHAPTER_SCROLLBAR_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nChapterScrollbarMode != nMode) {
		m_pPersistentSettingData->m_nChapterScrollbarMode = nMode;
		emit changedChapterScrollbarMode(m_pPersistentSettingData->m_nChapterScrollbarMode);
	}
}

void CPersistentSettings::setVerseRenderingMode(CPhraseNavigator::VERSE_RENDERING_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nVerseRenderingMode != nMode) {
		m_pPersistentSettingData->m_nVerseRenderingMode = nMode;
		emit changedVerseRenderingMode(m_pPersistentSettingData->m_nVerseRenderingMode);
	}
}

void CPersistentSettings::setShowPilcrowMarkers(bool bShowPilcrowMarkers)
{
	if (m_pPersistentSettingData->m_bShowPilcrowMarkers != bShowPilcrowMarkers) {
		m_pPersistentSettingData->m_bShowPilcrowMarkers = bShowPilcrowMarkers;
		emit changedShowPilcrowMarkers(m_pPersistentSettingData->m_bShowPilcrowMarkers);
	}
}

void CPersistentSettings::setDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nDictionaryCompleterFilterMode != nMode) {
		m_pPersistentSettingData->m_nDictionaryCompleterFilterMode = nMode;
		emit changedDictionaryCompleterFilterMode(m_pPersistentSettingData->m_nDictionaryCompleterFilterMode);
	}
}

void CPersistentSettings::setDictionaryActivationDelay(int nDelay)
{
	if (m_pPersistentSettingData->m_nDictionaryActivationDelay != nDelay) {
		m_pPersistentSettingData->m_nDictionaryActivationDelay = nDelay;
		emit changedDictionaryActivationDelay(m_pPersistentSettingData->m_nDictionaryActivationDelay);
	}
}

void CPersistentSettings::setReferenceDelimiterMode(CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nReferenceDelimiterMode != nMode) {
		m_pPersistentSettingData->m_nReferenceDelimiterMode = nMode;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setReferencesUseAbbreviatedBookNames(bool bUseAbbrBookNames)
{
	if (m_pPersistentSettingData->m_bReferencesUseAbbreviatedBookNames != bUseAbbrBookNames) {
		m_pPersistentSettingData->m_bReferencesUseAbbreviatedBookNames = bUseAbbrBookNames;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setReferencesInBold(bool bInBold)
{
	if (m_pPersistentSettingData->m_bReferencesInBold != bInBold) {
		m_pPersistentSettingData->m_bReferencesInBold = bInBold;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setReferencesAtEnd(bool bAtEnd)
{
	if (m_pPersistentSettingData->m_bReferencesAtEnd != bAtEnd) {
		m_pPersistentSettingData->m_bReferencesAtEnd = bAtEnd;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setVerseNumberDelimiterMode(CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nVerseNumberDelimiterMode != nMode) {
		m_pPersistentSettingData->m_nVerseNumberDelimiterMode = nMode;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setVerseNumbersUseAbbreviatedBookNames(bool bUseAbbrBookNames)
{
	if (m_pPersistentSettingData->m_bVerseNumbersUseAbbreviatedBookNames != bUseAbbrBookNames) {
		m_pPersistentSettingData->m_bVerseNumbersUseAbbreviatedBookNames = bUseAbbrBookNames;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setVerseNumbersInBold(bool bInBold)
{
	if (m_pPersistentSettingData->m_bVerseNumbersInBold != bInBold) {
		m_pPersistentSettingData->m_bVerseNumbersInBold = bInBold;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setAddQuotesAroundVerse(bool bAddQuotes)
{
	if (m_pPersistentSettingData->m_bAddQuotesAroundVerse != bAddQuotes) {
		m_pPersistentSettingData->m_bAddQuotesAroundVerse = bAddQuotes;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setTransChangeAddWordMode(CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nTransChangeAddWordMode != nMode) {
		m_pPersistentSettingData->m_nTransChangeAddWordMode = nMode;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setVerseRenderingModeCopying(CPhraseNavigator::VERSE_RENDERING_MODE_ENUM nMode)
{
	if (m_pPersistentSettingData->m_nVerseRenderingModeCopying != nMode) {
		m_pPersistentSettingData->m_nVerseRenderingModeCopying = nMode;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setCopyPilcrowMarkers(bool bCopyPilcrowMarkers)
{
	if (m_pPersistentSettingData->m_bCopyPilcrowMarkers != bCopyPilcrowMarkers) {
		m_pPersistentSettingData->m_bCopyPilcrowMarkers = bCopyPilcrowMarkers;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setCopyColophons(bool bCopyColophons)
{
	if (m_pPersistentSettingData->m_bCopyColophons != bCopyColophons) {
		m_pPersistentSettingData->m_bCopyColophons = bCopyColophons;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setCopySuperscriptions(bool bCopySuperscriptions)
{
	if (m_pPersistentSettingData->m_bCopySuperscriptions != bCopySuperscriptions) {
		m_pPersistentSettingData->m_bCopySuperscriptions = bCopySuperscriptions;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setCopyFontSelection(CPhraseNavigator::COPY_FONT_SELECTION_ENUM nCopyFontSelection)
{
	if (m_pPersistentSettingData->m_nCopyFontSelection != nCopyFontSelection) {
		m_pPersistentSettingData->m_nCopyFontSelection = nCopyFontSelection;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setFontCopyFont(const QFont &aFont)
{
	if (m_pPersistentSettingData->m_fntCopyFont != aFont) {
		m_pPersistentSettingData->m_fntCopyFont = aFont;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setSearchResultsAddBlankLineBetweenVerses(bool bAddBlankLine)
{
	if (m_pPersistentSettingData->m_bSearchResultsAddBlankLineBetweenVerses != bAddBlankLine) {
		m_pPersistentSettingData->m_bSearchResultsAddBlankLineBetweenVerses = bAddBlankLine;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setShowOCntInSearchResultsRefs(bool bShow)
{
	if (m_pPersistentSettingData->m_bShowOCntInSearchResultsRefs != bShow) {
		m_pPersistentSettingData->m_bShowOCntInSearchResultsRefs = bShow;
		emit changedShowOCntInSearchResultsRefs(m_pPersistentSettingData->m_bShowOCntInSearchResultsRefs);
	}
}

void CPersistentSettings::setCopyOCntInSearchResultsRefs(bool bCopy)
{
	if (m_pPersistentSettingData->m_bCopyOCntInSearchResultsRefs != bCopy) {
		m_pPersistentSettingData->m_bCopyOCntInSearchResultsRefs = bCopy;
		emit changedCopyOptions();
	}
}

void CPersistentSettings::setShowWrdNdxInSearchResultsRefs(bool bShow)
{
	if (m_pPersistentSettingData->m_bShowWrdNdxInSearchResultsRefs != bShow) {
		m_pPersistentSettingData->m_bShowWrdNdxInSearchResultsRefs = bShow;
		emit changedShowWrdNdxInSearchResultsRefs(m_pPersistentSettingData->m_bShowWrdNdxInSearchResultsRefs);
	}
}

void CPersistentSettings::setCopyWrdNdxInSearchResultsRefs(bool bCopy)
{
	if (m_pPersistentSettingData->m_bCopyWrdNdxInSearchResultsRefs != bCopy) {
		m_pPersistentSettingData->m_bCopyWrdNdxInSearchResultsRefs = bCopy;
		emit changedCopyOptions();
	}
}

// ----------------------------------------------------------------------------

QStringList CPersistentSettings::bibleDatabaseSettingsUUIDList() const
{
	QStringList lstUUIDs;

	lstUUIDs.reserve(m_pPersistentSettingData->m_mapBibleDatabaseSettings.size());
	for (TBibleDatabaseSettingsMap::ConstIterator itrUUIDs = m_pPersistentSettingData->m_mapBibleDatabaseSettings.constBegin();
													itrUUIDs != m_pPersistentSettingData->m_mapBibleDatabaseSettings.constEnd();
													++itrUUIDs) {
		lstUUIDs.append(itrUUIDs.key());
	}

	return lstUUIDs;
}

TBibleDatabaseSettings CPersistentSettings::bibleDatabaseSettings(const QString &strUUID) const
{
	return (m_pPersistentSettingData->m_mapBibleDatabaseSettings.value(strUUID, TBibleDatabaseSettings()));
}

void CPersistentSettings::setBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings)
{
	bool bFound = m_pPersistentSettingData->m_mapBibleDatabaseSettings.contains(strUUID);
	TBibleDatabaseSettings oldSettings = m_pPersistentSettingData->m_mapBibleDatabaseSettings.value(strUUID, TBibleDatabaseSettings());
	m_pPersistentSettingData->m_mapBibleDatabaseSettings[strUUID] = aSettings;
	if ((!bFound) || (oldSettings != aSettings)) emit changedBibleDatabaseSettings(strUUID, aSettings);
}

void CPersistentSettings::setMainBibleDatabaseUUID(const QString &strUUID)
{
	if (m_pPersistentSettingData->m_strMainBibleDatabaseUUID != strUUID) {
		m_pPersistentSettingData->m_strMainBibleDatabaseUUID = strUUID;
		emit changedMainBibleDatabaseSelection(strUUID);
	}
}

void CPersistentSettings::setApplicationLanguage(const QString &strLangName)
{
	if (m_pPersistentSettingData->m_strApplicationLanguage != strLangName) {
		m_pPersistentSettingData->m_strApplicationLanguage = strLangName;
		emit changedApplicationLanguage(strLangName);
	}
}

// ============================================================================
