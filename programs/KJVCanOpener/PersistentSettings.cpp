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
	:	QObject(parent)
{
	// Must set these in main() before caling settings!:
	assert(QCoreApplication::applicationName().compare(VER_APPNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationName().compare(VER_ORGNAME_STR_QT) == 0);
	assert(QCoreApplication::organizationDomain().compare(VER_ORGDOMAIN_STR_QT) == 0);

	m_pSettings = new QSettings();
}

CPersistentSettings::~CPersistentSettings()
{
	delete m_pSettings;
}

CPersistentSettings *CPersistentSettings::instance()
{
	static CPersistentSettings thePersistentSettings;
	return &thePersistentSettings;
}

