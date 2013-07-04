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

CPersistentSettings::CPersistentSettings(QObject *parent)
	:	QObject(parent),
		m_fntScriptureBrowser("Times New Roman", 12),					// Default fonts
		m_fntSearchResults("Times New Roman", 12),
		m_bInvertTextBrightness(false),
		m_nTextBrightness(100),
		m_bAdjustDialogElementBrightness(false)
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

void CPersistentSettings::setFontScriptureBrowser(const QFont &aFont)
{
	m_fntScriptureBrowser = aFont;
	emit fontChangedScriptureBrowser(aFont);
}

void CPersistentSettings::setFontSearchResults(const QFont &aFont)
{
	m_fntSearchResults = aFont;
	emit fontChangedSearchResults(aFont);
}

void CPersistentSettings::setInvertTextBrightness(bool bInvert)
{
	m_bInvertTextBrightness = bInvert;
	emit invertTextBrightnessChanged(m_bInvertTextBrightness);
	emit changedTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
}

void CPersistentSettings::setTextBrightness(int nBrightness)
{
	assert((nBrightness >= 0) && (nBrightness <= 100));
	if (nBrightness < 0) nBrightness = 0;
	if (nBrightness > 100) nBrightness = 100;
	m_nTextBrightness = nBrightness;
	emit textBrightnessChanged(m_nTextBrightness);
	emit changedTextBrightness(m_bInvertTextBrightness, m_nTextBrightness);
}

void CPersistentSettings::setAdjustDialogElementBrightness(bool bAdjust)
{
	m_bAdjustDialogElementBrightness = bAdjust;
	emit adjustDialogElementBrightnessChanged(m_bAdjustDialogElementBrightness);
}

QColor CPersistentSettings::textForegroundColor() const
{
	return textForegroundColor(m_bInvertTextBrightness, m_nTextBrightness);
}

QColor CPersistentSettings::textForegroundColor(bool bInvert, int nBrightness)
{
	assert((nBrightness >= 0) && (nBrightness <= 100));
	QColor clrForeground = (bInvert ? QColor(255, 255, 255) : QColor(0, 0, 0));
	return (bInvert ? clrForeground.darker(300 - (nBrightness * 2)) : clrForeground.lighter(300 - (nBrightness * 2)));
}

QColor CPersistentSettings::textBackgroundColor() const
{
	return textBackgroundColor(m_bInvertTextBrightness, m_nTextBrightness);
}

QColor CPersistentSettings::textBackgroundColor(bool bInvert, int nBrightness)
{
	assert((nBrightness >= 0) && (nBrightness <= 100));
	QColor clrBackground = (bInvert ? QColor(0, 0, 0) : QColor(255, 255, 255));
	return (bInvert ? clrBackground.lighter(300 - (nBrightness * 2)) : clrBackground.darker(300 - (nBrightness * 2)));
}

