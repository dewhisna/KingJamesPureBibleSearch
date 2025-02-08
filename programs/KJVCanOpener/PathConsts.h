/****************************************************************************
**
** Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PATH_CONSTS_H
#define PATH_CONSTS_H

#include <QtGlobal>

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const char * const g_constrTranslationFilenamePrefix = "kjpbs";

#ifdef Q_OS_ANDROID
	// --------------------------------------------------------------------------------------------------------- Android ------------------------
// Android deploy mechanism will automatically include our plugins, so these shouldn't be needed:
//	const char * const g_constrPluginsPath = "assets:/plugins/";
//	const char * const g_constrPluginsPath = "/data/data/com.dewtronics.KingJamesPureBibleSearch/qt-reserved-files/plugins/";

	const char * const g_constrBibleDatabasePath = "db/";
	const char * const g_constrDictionaryDatabasePath = "db/";
	const char * const g_constrTranslationsPath = "translations/";

	const char * const g_constrKJPBSBasePath = "../qt-reserved-files/files/KJVCanOpener/";
#elif defined(Q_OS_IOS)
	// --------------------------------------------------------------------------------------------------------- iOS ----------------------------
	const char * const g_constrPluginsPath = "../../Frameworks/";

	const char * const g_constrBibleDatabasePath = "db/";
	const char * const g_constrDictionaryDatabasePath = "db/";
	const char * const g_constrTranslationsPath = "translations/";

	const char * const g_constrKJPBSBasePath = "./assets/KJVCanOpener/";
#elif defined(Q_OS_OSX) || defined(Q_OS_MACX)
	// --------------------------------------------------------------------------------------------------------- Mac ----------------------------
	const char * const g_constrPluginsPath = "../PlugIns/";

	const char * const g_constrBibleDatabasePath = "db/";
	const char * const g_constrDictionaryDatabasePath = "db/";
	const char * const g_constrTranslationsPath = "translations/";

	const char * const g_constrKJPBSBasePath = "../Resources/";
#elif defined(EMSCRIPTEN)
	// --------------------------------------------------------------------------------------------------------- EMSCRIPTEN ---------------------
	// No plugins on Empscripten

	#ifdef EMSCRIPTEN_NATIVE
		const char * const g_constrBibleDatabasePath = "./data/";
		const char * const g_constrDictionaryDatabasePath = "./data/";
		const char * const g_constrTranslationsPath = "./data/";
	#else
		const char * const g_constrBibleDatabasePath = "data/";
		const char * const g_constrDictionaryDatabasePath = "data/";
		const char * const g_constrTranslationsPath = "data/";
	#endif

	const char * const g_constrKJPBSBasePath = "";
#else
	// --------------------------------------------------------------------------------------------------------- Linux, VNCSERVER, and Win32 ----------------
	const char * const g_constrPluginsPath = "plugins/";

	const char * const g_constrBibleDatabasePath = "db/";
	const char * const g_constrDictionaryDatabasePath = "db/";
	const char * const g_constrTranslationsPath = "translations/";

	const char * const g_constrKJPBSBasePath = "";
#endif

#ifdef USING_MMDB
	const char * const g_constrMMDBPath = "geoip/GeoLite2-City.mmdb";
#endif

	//////////////////////////////////////////////////////////////////////

}	// namespace

// ============================================================================

#endif	// PATH_CONSTS_H
