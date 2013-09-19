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

#include "version.h"

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
		m_fntScriptureBrowser("Times New Roman", 12),
		m_fntSearchResults("Times New Roman", 12),
		m_fntDictionary("Times New Roman", 12),
		// Default Text Brightness Options:
		m_bInvertTextBrightness(false),
		m_nTextBrightness(100),
		m_bAdjustDialogElementBrightness(false),
		// Default Special Text Colors:
		m_clrWordsOfJesus(QColor("red")),
		m_clrSearchResults(QColor("blue")),
		m_clrCursorFollow(QColor("blue")),
		// Default Search Phrase Options:
		m_nSearchPhraseCompleterFilterMode(CSearchCompleter::SCFME_NORMAL),
		m_nSearchActivationDelay(QApplication::doubleClickInterval()),
		// Default Browser Options:
		m_nNavigationActivationDelay(QApplication::doubleClickInterval()),
		m_nPassageReferenceActivationDelay(2000),
		// Default Copy Options:
		m_nReferenceDelimiterMode(CPhraseNavigator::RDME_PARENTHESES),
		m_bReferencesUseAbbreviatedBookNames(false),
		m_bReferencesInBold(true),
		m_nVerseNumberDelimiterMode(CPhraseNavigator::RDME_CURLY_BRACES),
		m_bVerseNumbersUseAbbreviatedBookNames(true),
		m_bVerseNumbersInBold(true),
		m_bAddQuotesAroundVerse(true),
		m_nTransChangeAddWordMode(CPhraseNavigator::TCAWME_ITALICS)
{

}

// ============================================================================

CPersistentSettings::CPersistentSettings(QObject *parent)
	:	QObject(parent),
		m_pPersistentSettingData(&m_PersistentSettingData1)
{
	// Must set these in main() before caling settings!:
	assert(QCoreApplication::applicationName().compare(VER_APPNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationName().compare(VER_ORGNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationDomain().compare(VER_ORGDOMAIN_STR_QT) == 0);

	m_pSettings = new QSettings(this);
}

CPersistentSettings::~CPersistentSettings()
{
}

CPersistentSettings *CPersistentSettings::instance()
{
	static CPersistentSettings thePersistentSettings;
	return &thePersistentSettings;
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

		if (pSource->m_bInvertTextBrightness != pTarget->m_bInvertTextBrightness) emit invertTextBrightnessChanged(pTarget->m_bInvertTextBrightness);
		if (pSource->m_nTextBrightness != pTarget->m_nTextBrightness) emit textBrightnessChanged(pTarget->m_nTextBrightness);
		if ((pSource->m_bInvertTextBrightness != pTarget->m_bInvertTextBrightness) ||
			(pSource->m_nTextBrightness != pTarget->m_nTextBrightness)) emit changedTextBrightness(pTarget->m_bInvertTextBrightness, pTarget->m_nTextBrightness);
		if (pSource->m_bAdjustDialogElementBrightness != pTarget->m_bAdjustDialogElementBrightness) emit adjustDialogElementBrightnessChanged(pTarget->m_bAdjustDialogElementBrightness);

		if (pSource->m_clrWordsOfJesus != pTarget->m_clrWordsOfJesus) emit changedColorWordsOfJesus(pTarget->m_clrWordsOfJesus);
		if (pSource->m_clrSearchResults != pTarget->m_clrSearchResults) emit changedColorSearchResults(pTarget->m_clrSearchResults);
		if (pSource->m_clrCursorFollow != pTarget->m_clrCursorFollow) emit changedColorCursorFollow(pTarget->m_clrCursorFollow);

		if (pSource->m_nSearchPhraseCompleterFilterMode != pTarget->m_nSearchPhraseCompleterFilterMode) emit changedSearchPhraseCompleterFilterMode(pTarget->m_nSearchPhraseCompleterFilterMode);
		if (pSource->m_nSearchActivationDelay != pTarget->m_nSearchActivationDelay) emit changedSearchPhraseActivationDelay(pTarget->m_nSearchActivationDelay);

		if (pSource->m_nNavigationActivationDelay != pTarget->m_nNavigationActivationDelay) emit changedNavigationActivationDelay(pTarget->m_nNavigationActivationDelay);
		if (pSource->m_nPassageReferenceActivationDelay != pTarget->m_nPassageReferenceActivationDelay) emit changedPassageReferenceActivationDelay(pTarget->m_nPassageReferenceActivationDelay);

		if ((pSource->m_nReferenceDelimiterMode != pTarget->m_nReferenceDelimiterMode) ||
			(pSource->m_bReferencesUseAbbreviatedBookNames != pTarget->m_bReferencesUseAbbreviatedBookNames) ||
			(pSource->m_bReferencesInBold != pTarget->m_bReferencesInBold) ||
			(pSource->m_nVerseNumberDelimiterMode != pTarget->m_nVerseNumberDelimiterMode) ||
			(pSource->m_bVerseNumbersUseAbbreviatedBookNames != pTarget->m_bVerseNumbersUseAbbreviatedBookNames) ||
			(pSource->m_bVerseNumbersInBold != pTarget->m_bVerseNumbersInBold) ||
			(pSource->m_bAddQuotesAroundVerse != pTarget->m_bAddQuotesAroundVerse) ||
			(pSource->m_nTransChangeAddWordMode != pTarget->m_nTransChangeAddWordMode)) emit changedCopyOptions();
	}
}

// ----------------------------------------------------------------------------

void CPersistentSettings::setFontScriptureBrowser(const QFont &aFont)
{
	m_pPersistentSettingData->m_fntScriptureBrowser = aFont;
	emit fontChangedScriptureBrowser(aFont);
}

void CPersistentSettings::setFontSearchResults(const QFont &aFont)
{
	m_pPersistentSettingData->m_fntSearchResults = aFont;
	emit fontChangedSearchResults(aFont);
}

void CPersistentSettings::setFontDictionary(const QFont &aFont)
{
	m_pPersistentSettingData->m_fntDictionary = aFont;
	emit fontChangedDictionary(aFont);
}

void CPersistentSettings::setInvertTextBrightness(bool bInvert)
{
	m_pPersistentSettingData->m_bInvertTextBrightness = bInvert;
	emit invertTextBrightnessChanged(m_pPersistentSettingData->m_bInvertTextBrightness);
	emit changedTextBrightness(m_pPersistentSettingData->m_bInvertTextBrightness, m_pPersistentSettingData->m_nTextBrightness);
}

void CPersistentSettings::setTextBrightness(int nBrightness)
{
	assert((nBrightness >= 0) && (nBrightness <= 100));
	if (nBrightness < 0) nBrightness = 0;
	if (nBrightness > 100) nBrightness = 100;
	m_pPersistentSettingData->m_nTextBrightness = nBrightness;
	emit textBrightnessChanged(m_pPersistentSettingData->m_nTextBrightness);
	emit changedTextBrightness(m_pPersistentSettingData->m_bInvertTextBrightness, m_pPersistentSettingData->m_nTextBrightness);
}

void CPersistentSettings::setAdjustDialogElementBrightness(bool bAdjust)
{
	m_pPersistentSettingData->m_bAdjustDialogElementBrightness = bAdjust;
	emit adjustDialogElementBrightnessChanged(m_pPersistentSettingData->m_bAdjustDialogElementBrightness);
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
	m_pPersistentSettingData->m_clrWordsOfJesus = color;
	emit changedColorWordsOfJesus(m_pPersistentSettingData->m_clrWordsOfJesus);
}

void CPersistentSettings::setColorSearchResults(const QColor &color)
{
	m_pPersistentSettingData->m_clrSearchResults = color;
	emit changedColorSearchResults(m_pPersistentSettingData->m_clrSearchResults);
}

void CPersistentSettings::setColorCursorFollow(const QColor &color)
{
	m_pPersistentSettingData->m_clrCursorFollow = color;
	emit changedColorCursorFollow(m_pPersistentSettingData->m_clrCursorFollow);
}

void CPersistentSettings::setSearchPhraseCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode)
{
	m_pPersistentSettingData->m_nSearchPhraseCompleterFilterMode = nMode;
	emit changedSearchPhraseCompleterFilterMode(m_pPersistentSettingData->m_nSearchPhraseCompleterFilterMode);
}

void CPersistentSettings::setSearchActivationDelay(int nDelay)
{
	m_pPersistentSettingData->m_nSearchActivationDelay = nDelay;
	emit changedSearchPhraseActivationDelay(m_pPersistentSettingData->m_nSearchActivationDelay);
}

void CPersistentSettings::setNavigationActivationDelay(int nDelay)
{
	m_pPersistentSettingData->m_nNavigationActivationDelay = nDelay;
	emit changedNavigationActivationDelay(m_pPersistentSettingData->m_nNavigationActivationDelay);
}

void CPersistentSettings::setPassageReferenceActivationDelay(int nDelay)
{
	m_pPersistentSettingData->m_nPassageReferenceActivationDelay = nDelay;
	emit changedPassageReferenceActivationDelay(m_pPersistentSettingData->m_nPassageReferenceActivationDelay);
}

void CPersistentSettings::setReferenceDelimiterMode(CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM nMode)
{
	m_pPersistentSettingData->m_nReferenceDelimiterMode = nMode;
	emit changedCopyOptions();
}

void CPersistentSettings::setReferencesUseAbbreviatedBookNames(bool bUseAbbrBookNames)
{
	m_pPersistentSettingData->m_bReferencesUseAbbreviatedBookNames = bUseAbbrBookNames;
	emit changedCopyOptions();
}

void CPersistentSettings::setReferencesInBold(bool bInBold)
{
	m_pPersistentSettingData->m_bReferencesInBold = bInBold;
	emit changedCopyOptions();
}

void CPersistentSettings::setVerseNumberDelimiterMode(CPhraseNavigator::REFERENCE_DELIMITER_MODE_ENUM nMode)
{
	m_pPersistentSettingData->m_nVerseNumberDelimiterMode = nMode;
	emit changedCopyOptions();
}

void CPersistentSettings::setVerseNumbersUseAbbreviatedBookNames(bool bUseAbbrBookNames)
{
	m_pPersistentSettingData->m_bVerseNumbersUseAbbreviatedBookNames = bUseAbbrBookNames;
	emit changedCopyOptions();
}

void CPersistentSettings::setVerseNumbersInBold(bool bInBold)
{
	m_pPersistentSettingData->m_bVerseNumbersInBold = bInBold;
	emit changedCopyOptions();
}

void CPersistentSettings::setAddQuotesAroundVerse(bool bAddQuotes)
{
	m_pPersistentSettingData->m_bAddQuotesAroundVerse = bAddQuotes;
	emit changedCopyOptions();
}

void CPersistentSettings::setTransChangeAddWordMode(CPhraseNavigator::TRANS_CHANGE_ADD_WORD_MODE_ENUM nMode)
{
	m_pPersistentSettingData->m_nTransChangeAddWordMode = nMode;
	emit changedCopyOptions();
}

