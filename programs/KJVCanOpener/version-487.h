/****************************************************************************
**
** Copyright (C) 2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef VERSION_487_H
#define VERSION_487_H

// Normally we need to include QtGlobal here to get Q_OS_WASM defined based on
//	system.  However, if we do, the compiler detection fails for RC compiler on
//	Windows.  So instead, we'll use the __EMSCRIPTEN__ define (see below) for
//	that, since that's what QtCore/qsystemdetection.h uses.
//#include <QtGlobal>

#define KJVCanOpener_VERSION		"4.0.0"

#if defined(VNCSERVER)
#define KJVCanOpener_VERSION_SPECIALBUILD	"(VNC Web-Version)"
#else
#define KJVCanOpener_VERSION_SPECIALBUILD	"Alpha Test Version\0"
#endif

#define KJVCanOpener_HOMEPAGE_URL "http://visitbethelchurch.com/"

#define KJVCanOpener_APPNAME "KingJamesPureBibleSearch"
#define KJPBS_ORGNAME "Dewtronics"
#define KJPBS_ORGDOMAIN "dewtronics.com"

#endif // VERSION_487_H
