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
		// Default Text Brightness Options:
		m_bInvertTextBrightness(false),
		m_nTextBrightness(100),
		m_bAdjustDialogElementBrightness(false),
		// Default Special Text Colors:
		m_clrWordsOfJesus(QColor("red")),
		m_clrSearchResults(QColor("blue")),
		m_clrCursorFollow(QColor("blue"))
{
	// Set Default Highlighters:
	m_mapUserHighlighters[tr("Basic Highlighter #1")] = QColor(255, 255, 170);			// "yellow" highlighter
	m_mapUserHighlighters[tr("Basic Highlighter #2")] = QColor(170, 255, 255);			// "blue" highlighter
	m_mapUserHighlighters[tr("Basic Highlighter #3")] = QColor(170, 255, 170);			// "green" highligher
	m_mapUserHighlighters[tr("Basic Highlighter #4")] = QColor(255, 170, 255);			// "pink" highlighter
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

		if (pSource->m_bInvertTextBrightness != pTarget->m_bInvertTextBrightness) emit invertTextBrightnessChanged(pTarget->m_bInvertTextBrightness);
		if (pSource->m_nTextBrightness != pTarget->m_nTextBrightness) emit textBrightnessChanged(pTarget->m_nTextBrightness);
		if ((pSource->m_bInvertTextBrightness != pTarget->m_bInvertTextBrightness) ||
			(pSource->m_nTextBrightness != pTarget->m_nTextBrightness)) emit changedTextBrightness(pTarget->m_bInvertTextBrightness, pTarget->m_nTextBrightness);
		if (pSource->m_bAdjustDialogElementBrightness != pTarget->m_bAdjustDialogElementBrightness) emit adjustDialogElementBrightnessChanged(pTarget->m_bAdjustDialogElementBrightness);

		if (pSource->m_clrWordsOfJesus != pTarget->m_clrWordsOfJesus) emit changedColorWordsOfJesus(pTarget->m_clrWordsOfJesus);
		if (pSource->m_clrSearchResults != pTarget->m_clrSearchResults) emit changedColorSearchResults(pTarget->m_clrSearchResults);
		if (pSource->m_clrCursorFollow != pTarget->m_clrCursorFollow) emit changedColorCursorFollow(pTarget->m_clrCursorFollow);

		if (pSource->m_mapUserHighlighters != pTarget->m_mapUserHighlighters) emit changedUserDefinedColors();
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

void CPersistentSettings::setUserDefinedColor(const QString &strUserDefinedHighlighterName, const QColor &color)
{
	m_pPersistentSettingData->m_mapUserHighlighters[strUserDefinedHighlighterName] = color;
	emit changedUserDefinedColor(strUserDefinedHighlighterName, color);
	emit changedUserDefinedColors();
}

void CPersistentSettings::removeUserDefinedColor(const QString &strUserDefinedHighlighterName)
{
	if (existsUserDefinedColor(strUserDefinedHighlighterName)) {
		m_pPersistentSettingData->m_mapUserHighlighters.remove(strUserDefinedHighlighterName);
		emit removedUserDefinedColor(strUserDefinedHighlighterName);
		emit changedUserDefinedColors();
	}
}

void CPersistentSettings::removeAllUserDefinedColors()
{
	m_pPersistentSettingData->m_mapUserHighlighters.clear();
	emit changedUserDefinedColors();
}
