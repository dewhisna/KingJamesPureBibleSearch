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

#include "myApplication.h"
#include "KJVCanOpener.h"
#include "ReportError.h"
#include "BusyCursor.h"

#ifdef VNCSERVER
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef USING_SINGLEAPPLICATION
#include <singleapplication.h>
#endif

#ifdef SHOW_SPLASH_SCREEN
#include <QPixmap>
#include <QSplashScreen>
#include <QElapsedTimer>
#endif

#include <QProxyStyle>
#include <QFont>
#include <QFontDatabase>
#include <QDesktopWidget>
#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif
#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSharedPointer>

#include <QList>

#include <QMdiSubWindow>

#include "version.h"
#include "PersistentSettings.h"
#include "UserNotesDatabase.h"
#include "DelayedExecutionTimer.h"
#include "Translator.h"

#ifdef BUILD_KJV_DATABASE
#include "BuildDB.h"
#endif
#include "ReadDB.h"

#include <assert.h>

// ============================================================================

QPointer<CMyApplication> g_pMyApplication = NULL;
QPointer<QMdiArea> g_pMdiArea = NULL;

const QString g_constrApplicationID = "KingJamesPureBibleSearch";

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------
	const QString constrMainAppControlGroup("MainApp/Controls");
	const QString constrFontNameKey("FontName");
	const QString constrFontSizeKey("FontSize");
	const QString constrLanguageKey("Language");

	// Main Bible Database Settings:
	const QString constrMainAppBibleDatabaseGroup("MainApp/BibleDatabase");
	const QString constrBibleDatabaseUUIDKey("UUID");

	// Bible Database Settings:
	const QString constrBibleDatabaseSettingsGroup("BibleDatabaseSettings");
	//const QString constrBibleDatabaseUUIDKey("UUID");
	const QString constrLoadOnStartKey("LoadOnStart");
	const QString constrHideHyphensKey("HideHyphens");
	const QString constrHyphenSensitiveKey("HyphenSensitive");

	//////////////////////////////////////////////////////////////////////

#ifdef SHOW_SPLASH_SCREEN
	const int g_connMinSplashTimeMS = 5000;			// Minimum number of milliseconds to display splash screen
	const int g_connInterAppSplashTimeMS = 2000;	// Splash Time for Inter-Application communications
#endif

	const QString g_constrInitialization = QObject::tr("King James Pure Bible Search Initialization", "Errors");

	//////////////////////////////////////////////////////////////////////

	const char *g_constrTranslationFilenamePrefix = "kjpbs";

#ifdef Q_OS_ANDROID
	// --------------------------------------------------------------------------------------------------------- Android ------------------------
// Android deploy mechanism will automatically include our plugins, so these shouldn't be needed:
//	const char *g_constrPluginsPath = "assets:/plugins/";
//	const char *g_constrPluginsPath = "/data/data/com.dewtronics.KingJamesPureBibleSearch/qt-reserved-files/plugins/";

	const char *g_constrBibleDatabasePath = "../qt-reserved-files/files/KJVCanOpener/db/";
	const char *g_constrDictionaryDatabasePath = "../qt-reserved-files/files/KJVCanOpener/db/";
	const char *g_constrTranslationsPath = "../qt-reserved-files/files/KJVCanOpener/translations/";
#elif defined(Q_OS_IOS)
	// --------------------------------------------------------------------------------------------------------- iOS ----------------------------
	const char *g_constrPluginsPath = "./Frameworks/";

	const char *g_constrBibleDatabasePath = "./assets/KJVCanOpener/db/";
	const char *g_constrDictionaryDatabasePath = "./assets/KJVCanOpener/db/";
	const char *g_constrTranslationsPath = "./assets/KJVCanOpener/translations/";
#elif defined(Q_OS_OSX) || defined(Q_OS_MACX)
	// --------------------------------------------------------------------------------------------------------- Mac ----------------------------
	const char *g_constrPluginsPath = "../PlugIns/";

	const char *g_constrBibleDatabasePath = "../Resources/db/";
	const char *g_constrDictionaryDatabasePath = "../Resources/db/";
	const char *g_constrTranslationsPath = "../Resources/translations/";
#elif defined(EMSCRIPTEN)
	// --------------------------------------------------------------------------------------------------------- EMSCRIPTEN ---------------------
	// No plugins on Empscripten

	#ifdef EMSCRIPTEN_NATIVE
		const char *g_constrBibleDatabasePath = "./data/";
		const char *g_constrTranslationsPath = "./data/";
	#else
		const char *g_constrBibleDatabasePath = "data/";
		const char *g_constrTranslationsPath = "data/";
	#endif
#elif defined(VNCSERVER)
	// --------------------------------------------------------------------------------------------------------- VNCSERVER ----------------------
	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";

	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";
	const char *g_constrDictionaryDatabasePath = "../../KJVCanOpener/db/";
	const char *g_constrTranslationsPath = "../../KJVCanOpener/translations/";
#else
	// --------------------------------------------------------------------------------------------------------- Linux and Win32 ----------------
	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";

	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";
	const char *g_constrDictionaryDatabasePath = "../../KJVCanOpener/db/";
	const char *g_constrTranslationsPath = "../../KJVCanOpener/translations/";
#endif

	//////////////////////////////////////////////////////////////////////

#ifdef LOAD_APPLICATION_FONTS

#ifdef Q_OS_ANDROID
	// --------------------------------------------------------------------------------------------------------- Android ------------------------
	const char *g_constrScriptBLFontFilename = "../qt-reserved-files/files/KJVCanOpener/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "../qt-reserved-files/files/KJVCanOpener/fonts/DejaVuSerif.ttf";
#elif defined(Q_OS_IOS)
	// --------------------------------------------------------------------------------------------------------- iOS ----------------------------
#ifndef WORKAROUND_QTBUG_34490
	const char *g_constrScriptBLFontFilename = "./assets/KJVCanOpener/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "./assets/KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "./assets/KJVCanOpener/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "./assets/KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "./assets/KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "./assets/KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "./assets/KJVCanOpener/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "./assets/KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "./assets/KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "./assets/KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "./assets/KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "./assets/KJVCanOpener/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "./assets/KJVCanOpener/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "./assets/KJVCanOpener/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "./assets/KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "./assets/KJVCanOpener/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "./assets/KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "./assets/KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "./assets/KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "./assets/KJVCanOpener/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "./assets/KJVCanOpener/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "./assets/KJVCanOpener/fonts/DejaVuSerif.ttf";
#endif
#elif defined(Q_OS_OSX) || defined(Q_OS_MACX)
	// --------------------------------------------------------------------------------------------------------- Mac ----------------------------
	const char *g_constrScriptBLFontFilename = "../Resources/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "../Resources/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "../Resources/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "../Resources/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "../Resources/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../Resources/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../Resources/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../Resources/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "../Resources/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "../Resources/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../Resources/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../Resources/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../Resources/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../Resources/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "../Resources/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "../Resources/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../Resources/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "../Resources/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "../Resources/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "../Resources/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "../Resources/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "../Resources/fonts/DejaVuSerif.ttf";
#elif defined(EMSCRIPTEN)
	// --------------------------------------------------------------------------------------------------------- EMSCRIPTEN ---------------------
	// Note: Emscripten uses auto-loading of .qpf fonts from deployed qt-fonts folder
	#ifdef EMSCRIPTEN_NATIVE
	const char *g_constrDejaVuSans_BoldOblique = "./data/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "./data/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "./data/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "./data/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "./data/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "./data/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "./data/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "./data/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldOblique = "./data/DejaVuSerif-BoldOblique.ttf";
	const char *g_constrDejaVuSerif_Bold = "./data/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerif_Oblique = "./data/DejaVuSerif-Oblique.ttf";
	const char *g_constrDejaVuSerif = "./data/DejaVuSerif.ttf";
	#else
		const char *g_constrDejaVuSans_BoldOblique = "data/DejaVuSans-BoldOblique.ttf";
		const char *g_constrDejaVuSans_Bold = "data/DejaVuSans-Bold.ttf";
		const char *g_constrDejaVuSansMono_BoldOblique = "data/DejaVuSansMono-BoldOblique.ttf";
		const char *g_constrDejaVuSansMono_Bold = "data/DejaVuSansMono-Bold.ttf";
		const char *g_constrDejaVuSansMono_Oblique = "data/DejaVuSansMono-Oblique.ttf";
		const char *g_constrDejaVuSansMono = "data/DejaVuSansMono.ttf";
		const char *g_constrDejaVuSans_Oblique = "data/DejaVuSans-Oblique.ttf";
		const char *g_constrDejaVuSans = "data/DejaVuSans.ttf";
		const char *g_constrDejaVuSerif_BoldOblique = "data/DejaVuSerif-BoldOblique.ttf";
		const char *g_constrDejaVuSerif_Bold = "data/DejaVuSerif-Bold.ttf";
		const char *g_constrDejaVuSerif_Oblique = "data/DejaVuSerif-Oblique.ttf";
		const char *g_constrDejaVuSerif = "data/DejaVuSerif.ttf";
	#endif
#elif defined(VNCSERVER)
	// --------------------------------------------------------------------------------------------------------- VNCSERVER ----------------------
	const char *g_constrScriptBLFontFilename = "../../KJVCanOpener/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "../../KJVCanOpener/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../../KJVCanOpener/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../../KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "../../KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../../KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../../KJVCanOpener/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../../KJVCanOpener/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../../KJVCanOpener/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "../../KJVCanOpener/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "../../KJVCanOpener/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "../../KJVCanOpener/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "../../KJVCanOpener/fonts/DejaVuSerif.ttf";
#else
	// --------------------------------------------------------------------------------------------------------- Linux and Win32 ----------------
	const char *g_constrScriptBLFontFilename = "../../KJVCanOpener/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "../../KJVCanOpener/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../../KJVCanOpener/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../../KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "../../KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../../KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../../KJVCanOpener/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../../KJVCanOpener/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../../KJVCanOpener/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "../../KJVCanOpener/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "../../KJVCanOpener/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "../../KJVCanOpener/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "../../KJVCanOpener/fonts/DejaVuSerif.ttf";
#endif

#ifndef WORKAROUND_QTBUG_34490

#ifndef EMSCRIPTEN
	const char *g_constrarrFontFilenames[] = {
		g_constrScriptBLFontFilename,
		g_constrDejaVuSans_BoldOblique,
		g_constrDejaVuSans_Bold,
		g_constrDejaVuSansCondensed_BoldOblique,
		g_constrDejaVuSansCondensed_Bold,
		g_constrDejaVuSansCondensed_Oblique,
		g_constrDejaVuSansCondensed,
		g_constrDejaVuSans_ExtraLight,
		g_constrDejaVuSansMono_BoldOblique,
		g_constrDejaVuSansMono_Bold,
		g_constrDejaVuSansMono_Oblique,
		g_constrDejaVuSansMono,
		g_constrDejaVuSans_Oblique,
		g_constrDejaVuSans,
		g_constrDejaVuSerif_BoldItalic,
		g_constrDejaVuSerif_Bold,
		g_constrDejaVuSerifCondensed_BoldItalic,
		g_constrDejaVuSerifCondensed_Bold,
		g_constrDejaVuSerifCondensed_Italic,
		g_constrDejaVuSerifCondensed,
		g_constrDejaVuSerif_Italic,
		g_constrDejaVuSerif,
		NULL
	};
#else
	const char *g_constrarrFontFilenames[] = {
		g_constrDejaVuSans_BoldOblique,
		g_constrDejaVuSans_Bold,
		g_constrDejaVuSansMono_BoldOblique,
		g_constrDejaVuSansMono_Bold,
		g_constrDejaVuSansMono_Oblique,
		g_constrDejaVuSansMono,
		g_constrDejaVuSans_Oblique,
		g_constrDejaVuSans,
		g_constrDejaVuSerif_BoldOblique,
		g_constrDejaVuSerif_Bold,
		g_constrDejaVuSerif_Oblique,
		g_constrDejaVuSerif,
		NULL
	};
#endif		// EMSCRIPTEN

#endif		// WORKAROUND_QTBUG_34490

#endif		//	LOAD_APPLICATION_FONTS

}	// namespace

// ============================================================================

#ifdef VNCSERVER

int CMyDaemon::m_sighupFd[2] = { 0, 0 };
int CMyDaemon::m_sigtermFd[2] = { 0, 0 };
int CMyDaemon::m_sigusr1Fd[2] = { 0, 0 };

CMyDaemon::CMyDaemon(CMyApplication *pMyApplication)
	:	QObject(pMyApplication),
		m_psnHup(NULL),
		m_psnTerm(NULL),
		m_psnUsr1(NULL),
		m_pMyApplication(pMyApplication)
{
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sighupFd))
		qFatal("Couldn't create SIGHUP socketpair");

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigtermFd))
		qFatal("Couldn't create SIGTERM socketpair");

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigusr1Fd))
		qFatal("Couldn't create SIGUSR1 socketpair");

	m_psnHup = new QSocketNotifier(m_sighupFd[1], QSocketNotifier::Read, this);
	connect(m_psnHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
	m_psnTerm = new QSocketNotifier(m_sigtermFd[1], QSocketNotifier::Read, this);
	connect(m_psnTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
	m_psnUsr1 = new QSocketNotifier(m_sigusr1Fd[1], QSocketNotifier::Read, this);
	connect(m_psnUsr1, SIGNAL(activated(int)), this, SLOT(handleSigUsr1()));
}

CMyDaemon::~CMyDaemon()
{

}

int CMyDaemon::setup_unix_signal_handlers()
{
	struct sigaction hup, term, usr1;

	hup.sa_handler = CMyDaemon::hupSignalHandler;
	sigemptyset(&hup.sa_mask);
	hup.sa_flags = 0;
	hup.sa_flags |= SA_RESTART;

	if (sigaction(SIGHUP, &hup, 0) > 0)
		return 1;

	term.sa_handler = CMyDaemon::termSignalHandler;
	sigemptyset(&term.sa_mask);
	term.sa_flags |= SA_RESTART;

	if (sigaction(SIGTERM, &term, 0) > 0)
		return 2;

	usr1.sa_handler = CMyDaemon::usr1SignalHandler;
	sigemptyset(&usr1.sa_mask);
	usr1.sa_flags |= SA_RESTART;

	if (sigaction(SIGUSR1, &usr1, 0) >0)
		return 3;

	return 0;
}

void CMyDaemon::hupSignalHandler(int)
{
	char a = 1;
	ssize_t szWrite = ::write(m_sighupFd[0], &a, sizeof(a));
	assert(szWrite == sizeof(a));
}

void CMyDaemon::termSignalHandler(int)
{
	char a = 1;
	ssize_t szWrite = ::write(m_sigtermFd[0], &a, sizeof(a));
	assert(szWrite == sizeof(a));
}

void CMyDaemon::usr1SignalHandler(int)
{
	char a = 1;
	ssize_t szWrite = ::write(m_sigusr1Fd[0], &a, sizeof(a));
	assert(szWrite == sizeof(a));
}

void CMyDaemon::handleSigHup()
{
	m_psnHup->setEnabled(false);
	char tmp;
	ssize_t szRead = ::read(m_sighupFd[1], &tmp, sizeof(tmp));
	assert(szRead == sizeof(tmp));

	// do Qt stuff
	if (m_pMyApplication.data() != NULL) {
		m_pMyApplication->closeAllWindows();
	}

	m_psnHup->setEnabled(true);
}

void CMyDaemon::handleSigTerm()
{
	m_psnTerm->setEnabled(false);
	char tmp;
	ssize_t szRead = ::read(m_sigtermFd[1], &tmp, sizeof(tmp));
	assert(szRead == sizeof(tmp));

	// do Qt stuff
	if (m_pMyApplication.data() != NULL) {
		m_pMyApplication->closeAllWindows();
	}

	m_psnTerm->setEnabled(true);
}

void CMyDaemon::handleSigUsr1()
{
	m_psnUsr1->setEnabled(false);
	char tmp;
	ssize_t szRead = ::read(m_sigusr1Fd[1], &tmp, sizeof(tmp));
	assert(szRead == sizeof(tmp));

	// do Qt stuff
	QWidget *pParent = NULL;
	if (m_pMyApplication.data() != NULL) {
		pParent = m_pMyApplication->activeCanOpener();
	}
	QMessageBox::warning(pParent, tr("King James Pure Bible Search", "Errors"), tr("Warning: Your VNC King James Pure Bible Search Session expires in 5 minutes.", "Errors"));

	m_psnUsr1->setEnabled(true);
}

#endif	// VNCSERVER

// ============================================================================

class MyProxyStyle : public QProxyStyle
{
public:
	int styleHint(StyleHint hint, const QStyleOption *option = 0,
				const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const
	{
		if (hint == QStyle::SH_ItemView_ActivateItemOnSingleClick) return 0;

		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

// ============================================================================

CMyApplication::CMyApplication(int & argc, char ** argv)
#ifdef USING_QT_SINGLEAPPLICATION
	:	QtSingleApplication(g_constrApplicationID, argc, argv),
#else
	:	QApplication(argc, argv),
#endif
		m_nLastActivateCanOpener(-1),
		m_bUsingCustomStyleSheet(false),
		m_bAreRestarting(false),
		m_pSplash(NULL),
		m_nSelectedMainBibleDB(BDE_UNKNOWN)
{
#ifdef Q_OS_ANDROID
	m_strInitialAppDirPath = QDir::homePath();
#else
	m_strInitialAppDirPath = applicationDirPath();
#endif
	m_strStartupStyleSheet = styleSheet();

	// Setup our SQL/Image and Platform Plugin paths.  Ideally, this would be
	//	done in main() before instantiating the object in order to make the
	//	Platform plugins to work correctly on Qt 5, however, the
	//	QCoreApplication::applicationDirPath() can't be called until after the
	//	QApplication object has been instantiated.  So, we'll just have to put
	//	the Platform plugins in the app folder:
#if !defined(Q_OS_ANDROID) && !defined(EMSCRIPTEN)
	QFileInfo fiPlugins(m_strInitialAppDirPath, g_constrPluginsPath);
	QCoreApplication::addLibraryPath(fiPlugins.absolutePath());
#endif

	if (m_strStartupStyleSheet.startsWith(QLatin1String("file:///"))) {
		// If the startupStyleSheet was a file, read it:
		m_strStartupStyleSheet.remove(0, 8);
		QFile fileSS(m_strStartupStyleSheet);
		if (fileSS.open(QFile::ReadOnly)) {
			QTextStream stream(&fileSS);
			m_strStartupStyleSheet = stream.readAll();
		} else {
			qWarning() << "Failed to load stylesheet file " << m_strStartupStyleSheet;
			m_strStartupStyleSheet.clear();
			setStyleSheet(QString());
		}
	}

	setStyle(new MyProxyStyle());			// Note: QApplication will take ownership of this (no need for delete)

	g_strTranslationsPath = QFileInfo(initialAppDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);
}

CMyApplication::~CMyApplication()
{
	if (m_pSplash != NULL) {
		delete m_pSplash;
		m_pSplash = NULL;
	}

	// We must clean up our databases and things before exiting or else
	//		the destructor tear-down order might cause us to crash, particularly
	//		with SQL Database things:
	TBibleDatabaseList::instance()->clear();
	TDictionaryDatabaseList::instance()->clear();
	g_pUserNotesDatabase.clear();

#ifdef LOAD_APPLICATION_FONTS
#ifndef WORKAROUND_QTBUG_34490
	QFontDatabase::removeAllApplicationFonts();
#endif	// WORKAROUND_QTBUG_34490
#endif	// LOAD_APPLICATION_FONTS
}

QWidget *CMyApplication::showSplash()
{
#ifdef SHOW_SPLASH_SCREEN
	QPixmap pixSplash(":/res/KJPBS_SplashScreen800x500.png");
	m_pSplash = new QSplashScreen(pixSplash);
	if (m_pSplash) {
		m_pSplash->show();
#ifdef WORKAROUND_QTBUG_35787
		// The following is a work-around for QTBUG-35787 where the
		//		splashscreen won't display on iOS unless an event
		//		loop completes:
		QEventLoop loop;
		QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
		loop.exec();
#endif
		m_pSplash->raise();
		setSplashMessage();
		processEvents();
	}

	m_splashTimer.start();
#endif

	return static_cast<QWidget *>(m_pSplash);
}

void CMyApplication::completeInterAppSplash()
{
	if (m_pSplash != NULL) {
#ifdef SHOW_SPLASH_SCREEN
		assert(m_splashTimer.isValid());
		do {
			processEvents();
		} while (!m_splashTimer.hasExpired(g_connInterAppSplashTimeMS));
#endif
	}
}

void CMyApplication::setSplashMessage(const QString &strMessage)
{
	if (m_pSplash != NULL) {
#ifdef SHOW_SPLASH_SCREEN
		m_pSplash->clearMessage();
		const QString strOffsetSpace = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		QString strSpecialVersion(SPECIAL_BUILD ? QString(VER_SPECIALVERSION_STR) : QString());
		if (!strSpecialVersion.isEmpty()) strSpecialVersion = "<br>\n" + strOffsetSpace + strSpecialVersion;
		QString strStatus;
		if (!strMessage.isEmpty()) strStatus += "<br>\n" + strOffsetSpace + strMessage;
		m_pSplash->showMessage(QString("<html><body><table height=375 width=500><tr><td>&nbsp;</td></tr></table><div align=\"center\"><font size=+1 color=#FFFFFF><b>") +
										strOffsetSpace + tr("Please Wait...", "Errors") +
										strSpecialVersion +
										strStatus +
										QString("</b></font></div></body></html>"), Qt::AlignBottom | Qt::AlignLeft);
		m_pSplash->repaint();
		processEvents();
#else
		Q_UNUSED(strMessage);
#endif
	}
}

// ============================================================================

void CMyApplication::saveApplicationFontSettings()
{
	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrMainAppControlGroup);
		settings.setValue(constrFontNameKey, font().family());
		settings.setValue(constrFontSizeKey, font().pointSize());
		settings.endGroup();
	}
}

void CMyApplication::restoreApplicationFontSettings()
{
	// Setup our default font for our controls:
#ifdef Q_OS_WIN32
	QFont fntAppControls = QFont("MS Shell Dlg 2", 8);
#elif defined(Q_OS_MAC)
//	QFont fntAppControls = QFont("Arial", 12);
	QFont fntAppControls = QFont("Lucida Grande", 12);
#elif defined(EMSCRIPTEN)
	QFont fntAppControls = QFont("DejaVu Sans", 12);
#elif defined(VNCSERVER)
	QFont fntAppControls = QFont("DejaVu Sans", 12);
#else
	QFont fntAppControls = QFont("DejaVu Sans", 8);
#endif

	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());

		settings.beginGroup(constrMainAppControlGroup);
		QString strFontName = settings.value(constrFontNameKey, fntAppControls.family()).toString();
		int nFontSize = settings.value(constrFontSizeKey, fntAppControls.pointSize()).toInt();
		settings.endGroup();

		if ((!strFontName.isEmpty()) && (nFontSize>0)) {
			fntAppControls.setFamily(strFontName);
			fntAppControls.setPointSize(nFontSize);
		}
	}

	setFont(fntAppControls);
}

void CMyApplication::setupTextBrightnessStyleHooks()
{
	// Setup Default TextBrightness:
	en_setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(en_setTextBrightness(bool, int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(en_setAdjustDialogElementBrightness(bool)));
}

// ============================================================================

void CMyApplication::saveApplicationLanguage()
{
	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrMainAppControlGroup);
		settings.setValue(constrLanguageKey, CPersistentSettings::instance()->applicationLanguage());
		settings.endGroup();
	}
}

void CMyApplication::restoreApplicationLanguage()
{
	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrMainAppControlGroup);
		CPersistentSettings::instance()->setApplicationLanguage(settings.value(constrLanguageKey, CPersistentSettings::instance()->applicationLanguage()).toString());
		settings.endGroup();
	}
	CTranslatorList::instance()->setApplicationLanguage(CPersistentSettings::instance()->applicationLanguage());
}

// ============================================================================

bool CMyApplication::notify(QObject *pReceiver, QEvent *pEvent)
{
#ifdef EMSCRIPTEN
	// For some reason, Emscripten causes us to receive Action Changed notifications on
	//		a NULL Receiver object.  Haven't found exactly where these are originating,
	//		but they seem fairly beneign.  So just defaulting them here so we don't get
	//		a recurrent debug log notification message:
	if (pReceiver == NULL) return true;
#endif

#if !defined(NOT_USING_EXCEPTIONS) && defined(QT_DEBUG)
	try {
#endif
#ifdef USING_QT_SINGLEAPPLICATION
		return QtSingleApplication::notify(pReceiver, pEvent);
#else
		return QApplication::notify(pReceiver, pEvent);
#endif
#if !defined(NOT_USING_EXCEPTIONS) && defined(QT_DEBUG)
	} catch (const std::exception &ex) {
		qDebug("std::exception was caught: %s", ex.what());
	} catch (...) {
		qDebug("Unknown exception was caught");
		assert(false);
	}

	return false;
#endif
}

bool CMyApplication::event(QEvent *event) {
	if (event->type() == QEvent::FileOpen) {
		setFileToLoad(static_cast<QFileOpenEvent *>(event)->file());
		emit loadFile(fileToLoad());
		// Emulate receiving activate existing w/open KJS message:
		QString strMessage = createKJPBSMessage(KAMCE_ACTIVATE_EXISTING_OPEN_KJS, QStringList(QString("KJS=%1").arg(fileToLoad())));
		receivedKJPBSMessage(strMessage);
		return true;
	}
#ifdef USING_QT_SINGLEAPPLICATION
	return QtSingleApplication::event(event);
#else
	return QApplication::event(event);
#endif
}

#ifdef SIGNAL_SPY_DEBUG
Q4puGenericSignalSpy *CMyApplication::createSpy(QObject *pOwner, QObject *pSpyOn)
{
	assert(g_pMyApplication.data() != NULL);
	Q4puGenericSignalSpy *pSpy = new Q4puGenericSignalSpy((pOwner != NULL) ? pOwner : g_pMyApplication);

	QObject::connect(pSpy, SIGNAL(caughtSignal(const QString&)), g_pMyApplication, SLOT(signalSpyCaughtSignal(const QString &)));
	QObject::connect(pSpy, SIGNAL(caughtSlot(const QString&)), g_pMyApplication, SLOT(signalSpyCaughtSlot(const QString &)));

	// If we are given an object to spy on, attach to it.  If not, but were given
	//		an owner, attach to it.  If not, don't attach to anything...
	if ((pSpyOn != NULL) || (pOwner != NULL)) {
		pSpy->spyOn((pSpyOn != NULL) ? pSpyOn : pOwner);
	}

	return pSpy;
}

void CMyApplication::signalSpyCaughtSignal(const QString &strMessage) const
{
	qDebug("%s", strMessage.toUtf8().data());
}

void CMyApplication::signalSpyCaughtSlot(const QString &strMessage) const
{
	qDebug("%s", strMessage.toUtf8().data());
}
#endif

// ============================================================================

CKJVCanOpener *CMyApplication::createKJVCanOpener(CBibleDatabasePtr pBibleDatabase)
{
	m_bAreRestarting = false;			// Once we create a new CanOpener we are no longer restarting...
	CKJVCanOpener *pCanOpener = new CKJVCanOpener(pBibleDatabase);
	m_lstKJVCanOpeners.append(pCanOpener);
	connect(pCanOpener, SIGNAL(isClosing(CKJVCanOpener*)), this, SLOT(removeKJVCanOpener(CKJVCanOpener*)));
	connect(pCanOpener, SIGNAL(windowActivated(CKJVCanOpener*)), this, SLOT(activatedKJVCanOpener(CKJVCanOpener*)));
	connect(pCanOpener, SIGNAL(canCloseChanged(CKJVCanOpener*, bool)), this, SLOT(en_canCloseChanged(CKJVCanOpener*, bool)));

	if (g_pMdiArea.data() != NULL) {
		QMdiSubWindow *pSubWindow = new QMdiSubWindow;
		pSubWindow->setWidget(pCanOpener);
		pSubWindow->setAttribute(Qt::WA_DeleteOnClose);
		QMenu *pSysMenu = pSubWindow->systemMenu();
		if (pSysMenu) {
			QList<QAction *> lstActions = pSysMenu->actions();
			for (int ndxAction = 0; ndxAction < lstActions.size(); ++ndxAction) {
				if (lstActions.at(ndxAction)->shortcut() == QKeySequence(QKeySequence::Close)) {
					pSysMenu->removeAction(lstActions.at(ndxAction));
					delete lstActions.at(ndxAction);
					break;
				}
			}
		}
		g_pMdiArea->addSubWindow(pSubWindow);
		connect(pSubWindow, SIGNAL(aboutToActivate()), pCanOpener, SLOT(setFocus()));
	}

	// Note: no call to initialize() or show() here for the CanOpner.  We'll do it inside
	//	KJVCanOpener in the delayed restorePersistentSettings() function
	updateSearchWindowList();
	return pCanOpener;
}

bool CMyApplication::isFirstCanOpener(bool bInCanOpenerConstructor, const QString &strBblUUID) const
{
	return (bInCanOpenerConstructor ? (bibleDatabaseCanOpenerRefCount(strBblUUID) == 0) : (bibleDatabaseCanOpenerRefCount(strBblUUID) <= 1));
}

bool CMyApplication::isLastCanOpener(const QString &strBblUUID) const
{
	return (bibleDatabaseCanOpenerRefCount(strBblUUID) <= 1);
}

int CMyApplication::bibleDatabaseCanOpenerRefCount(const QString &strBblUUID) const
{
	if (strBblUUID.isEmpty()) {
		return m_lstKJVCanOpeners.size();
	} else {
		int nCount = 0;
		for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
			if (m_lstKJVCanOpeners.at(ndx) == NULL) continue;
			CBibleDatabasePtr pBibleDatabase = m_lstKJVCanOpeners.at(ndx)->bibleDatabase();
			if (pBibleDatabase.data() == NULL) continue;
			if (pBibleDatabase->compatibilityUUID().compare(strBblUUID, Qt::CaseInsensitive) == 0) ++nCount;
		}
		return nCount;
	}
}

void CMyApplication::removeKJVCanOpener(CKJVCanOpener *pKJVCanOpener)
{
	int ndxCanOpener = m_lstKJVCanOpeners.indexOf(pKJVCanOpener);
	assert(ndxCanOpener != -1);
	if (ndxCanOpener == m_nLastActivateCanOpener) m_nLastActivateCanOpener = -1;
	if (ndxCanOpener != -1) m_lstKJVCanOpeners.removeAt(ndxCanOpener);
	if (g_pMdiArea.data() != NULL) {
		if (m_lstKJVCanOpeners.size() == 0) {
			if (!areRestarting()) g_pMdiArea->deleteLater();
		} else {
			QList<QMdiSubWindow *> lstSubWindows = g_pMdiArea->subWindowList();
			for (int ndxSubWindows = 0; ndxSubWindows < lstSubWindows.size(); ++ndxSubWindows) {
				if (lstSubWindows.at(ndxSubWindows)->widget() == NULL) {
					lstSubWindows.at(ndxSubWindows)->close();
					break;
				}
			}
		}
	}
	updateSearchWindowList();
}

void CMyApplication::activatedKJVCanOpener(CKJVCanOpener *pCanOpener)
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		if (m_lstKJVCanOpeners.at(ndx) == pCanOpener) {
			m_nLastActivateCanOpener = ndx;
			return;
		}
	}

	// The following is needed on Mac to make sure the menu of the
	//      new KJVCanOpen gets set:
	if (activeWindow() != static_cast<QWidget *>(pCanOpener))
			setActiveWindow(pCanOpener);

	assert(false);
	m_nLastActivateCanOpener = -1;
}

CKJVCanOpener *CMyApplication::activeCanOpener() const
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		if (m_lstKJVCanOpeners.at(ndx)->isActiveWindow()) return m_lstKJVCanOpeners.at(ndx);
	}
	return NULL;
}

template<class T>
CKJVCanOpener *CMyApplication::findCanOpenerFromChild(const T *pChild) const
{
	assert(pChild != NULL);
	for (int ndxCanOpener = 0; ndxCanOpener < m_lstKJVCanOpeners.size(); ++ndxCanOpener) {
		QList<T *>lstFoundChildren = m_lstKJVCanOpeners.at(ndxCanOpener)->findChildren<T *>(pChild->objectName());
		for (int ndxChild = 0; ndxChild < lstFoundChildren.size(); ++ndxChild) {
			if (lstFoundChildren.at(ndxChild) == pChild) return m_lstKJVCanOpeners.at(ndxCanOpener);
		}
	}
	return NULL;
}

class CSearchResultsTreeView;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<CSearchResultsTreeView>(const CSearchResultsTreeView *) const;

class i_CScriptureBrowser;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<i_CScriptureBrowser>(const i_CScriptureBrowser *) const;

class i_CScriptureEdit;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<i_CScriptureEdit>(const i_CScriptureEdit *) const;


void CMyApplication::activateCanOpener(CKJVCanOpener *pCanOpener) const
{
	assert(pCanOpener != NULL);
	if (g_pMdiArea.data() != NULL) {
		QList<QMdiSubWindow *> lstSubWindows = g_pMdiArea->subWindowList();
		for (int ndx = 0; ndx < lstSubWindows.size(); ++ndx) {
			if (lstSubWindows.at(ndx)->widget() == static_cast<QWidget *>(pCanOpener)) {
				lstSubWindows.at(ndx)->setWindowState(lstSubWindows.at(ndx)->windowState() & ~Qt::WindowMinimized);
				g_pMdiArea->setActiveSubWindow(lstSubWindows.at(ndx));
			}
		}
	}
	pCanOpener->setWindowState(pCanOpener->windowState() & ~Qt::WindowMinimized);
	pCanOpener->raise();
	pCanOpener->activateWindow();
}

void CMyApplication::activateCanOpener(int ndx) const
{
	assert((ndx >= 0) && (ndx < m_lstKJVCanOpeners.size()));
	if ((ndx < 0) || (ndx >= m_lstKJVCanOpeners.size())) return;

	activateCanOpener(m_lstKJVCanOpeners.at(ndx));
}

void CMyApplication::activateAllCanOpeners() const
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		activateCanOpener(ndx);
	}
}

void CMyApplication::closeAllCanOpeners() const
{
	assert(canQuit());
	if (!canQuit()) return;

	// Close in reverse order:
	for (int ndx = (m_lstKJVCanOpeners.size()-1); ndx >= 0; --ndx) {
		QTimer::singleShot(0, m_lstKJVCanOpeners.at(ndx), SLOT(close()));
	}
	// Note: List update will happen automatically as the windows close...
}

void CMyApplication::updateSearchWindowList()
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		m_lstKJVCanOpeners.at(ndx)->en_updateSearchWindowList();
	}
}

void CMyApplication::restartApp()
{
	m_bAreRestarting = true;
	closeAllCanOpeners();
}

void CMyApplication::en_triggeredKJVCanOpener(QAction *pAction)
{
	assert(pAction != NULL);
	int nIndex = pAction->data().toInt();
	activateCanOpener(nIndex);
}

bool CMyApplication::canQuit() const
{
	bool bCanQuit = true;
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		if (!m_lstKJVCanOpeners.at(ndx)->canClose()) {
			bCanQuit = false;
			break;
		}
	}
	return bCanQuit;
}

void CMyApplication::en_canCloseChanged(CKJVCanOpener *pCanOpener, bool bCanClose)
{
	Q_UNUSED(pCanOpener);
	Q_UNUSED(bCanClose);
	emit canQuitChanged(canQuit());
}

void CMyApplication::en_setTextBrightness(bool bInvert, int nBrightness)
{
	// Note: This code needs to cooperate with the setStyleSheet in the constructor
	//			of KJVCanOpener that works around QTBUG-13768...

	if (CPersistentSettings::instance()->adjustDialogElementBrightness()) {
		// Note: This will automatically cause a repaint:
		setStyleSheet(QString("%3\n"
							  "CPhraseLineEdit { background-color:%1; color:%2; }\n"
							  "QLineEdit { background-color:%1; color:%2; }\n"
							  "QComboBox { background-color:%1; color:%2; }\n"
							  "QComboBox QAbstractItemView { background-color:%1; color:%2; }\n"
							  "QFontComboBox { background-color:%1; color:%2; }\n"
							  "QListView { background-color:%1; color:%2; }\n"						// Completers and QwwConfigWidget
							  "QTreeView { background-color:%1; color:%2; }\n"						// Bible Database List
							  "QSpinBox { background-color:%1; color:%2; }\n"
							  "QDoubleSpinBox { background-color:%1; color:%2; }\n"
							).arg(CPersistentSettings::textBackgroundColor(bInvert, nBrightness).name())
							 .arg(CPersistentSettings::textForegroundColor(bInvert, nBrightness).name())
							 .arg(startupStyleSheet()));
		m_bUsingCustomStyleSheet = true;
	} else {
		if (m_bUsingCustomStyleSheet) {
			setStyleSheet(startupStyleSheet());
			m_bUsingCustomStyleSheet = false;
		}
	}
}

void CMyApplication::en_setAdjustDialogElementBrightness(bool bAdjust)
{
	Q_UNUSED(bAdjust);
	en_setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
}

// ============================================================================

void CMyApplication::en_notesFileAutoSaveTriggered()
{
	if (g_pUserNotesDatabase.data() == NULL) return;			// Shouldn't happen, but just in case
	if ((g_pUserNotesDatabase->isDirty()) && (!g_pUserNotesDatabase->filePathName().isEmpty())) {
		CBusyCursor iAmBusy(NULL);
		if (!g_pUserNotesDatabase->save()) m_dlyNotesFilesAutoSave.trigger();		// If save failed, retrigger to try again
	}
}

void CMyApplication::en_changedUserNotesDatabase()
{
	assert(g_pUserNotesDatabase.data() != NULL);
	if ((CPersistentSettings::instance()->notesFileAutoSaveTime() > 0) && (!m_dlyNotesFilesAutoSave.isTriggered())) {
		if (g_pUserNotesDatabase->isDirty()) m_dlyNotesFilesAutoSave.trigger();
	} else if (!g_pUserNotesDatabase->isDirty()) {
		// If the file has been saved already, untrigger:
		m_dlyNotesFilesAutoSave.untrigger();
	}
}

void CMyApplication::en_changedNoteesFileAutoSaveTime(int nAutoSaveTime)
{
	m_dlyNotesFilesAutoSave.setMinimumDelay(nAutoSaveTime*60000);		// Convert minutes->milliseconds
	if (nAutoSaveTime > 0) {
		if (m_dlyNotesFilesAutoSave.isTriggered()) m_dlyNotesFilesAutoSave.trigger();		// Retrigger to extend time if our setting changed
	} else {
		m_dlyNotesFilesAutoSave.untrigger();		// If disabling, stop any existing triggers
	}
}

// ============================================================================

QString CMyApplication::createKJPBSMessage(KJPBS_APP_MESSAGE_COMMAND_ENUM nCommand, const QStringList &lstArgs) const
{
	QString strMessage;

	switch (nCommand) {
		case KAMCE_ACTIVATE_EXISTING:
			strMessage += "ACTIVATE";
			break;
		case KAMCE_ACTIVATE_EXISTING_OPEN_KJS:
			strMessage += "ACTIVATE_OPENKJS";
			break;
		case KAMCE_NEW_CANOPENER:
			strMessage += "NEW_CANOPENER";
			break;
		case KAMCE_NEW_CANOPENER_OPEN_KJS:
			strMessage += "NEW_CANOPENER_OPENKJS";
			break;
		default:
			return QString();
	}

	strMessage += ";";
	strMessage += lstArgs.join(";");
	return strMessage;
}

void CMyApplication::receivedKJPBSMessage(const QString &strMessage)
{
	if (strMessage.isEmpty()) {
		activateAllCanOpeners();
		return;
	}

	QStringList lstMsg = strMessage.split(";", QString::KeepEmptyParts);
	assert(lstMsg.size() >= 1);
	if (lstMsg.size() < 1) return;

	QString strKJSFileName;
	QString strBibleUUID;

	KJPBS_APP_MESSAGE_COMMAND_ENUM nCommand = KAMCE_UNKNOWN;
	QString strCommand = lstMsg.at(0);
	if (strCommand.compare("ACTIVATE", Qt::CaseInsensitive) == 0) {
		nCommand = KAMCE_ACTIVATE_EXISTING;
	} else if (strCommand.compare("ACTIVATE_OPENKJS", Qt::CaseInsensitive) == 0) {
		nCommand = KAMCE_ACTIVATE_EXISTING_OPEN_KJS;
	} else if (strCommand.compare("NEW_CANOPENER", Qt::CaseInsensitive) == 0) {
		nCommand = KAMCE_NEW_CANOPENER;
	} else if (strCommand.compare("NEW_CANOPENER_OPENKJS", Qt::CaseInsensitive) == 0) {
		nCommand = KAMCE_NEW_CANOPENER_OPEN_KJS;
	} else {
		qDebug("*** KJPBS : Unrecognized inter-application command : %s", strCommand.toUtf8().data());
	}

	for (int ndxArgs = 1; ndxArgs < lstMsg.size(); ++ndxArgs) {
		if (lstMsg.at(ndxArgs).isEmpty()) continue;
		QStringList lstArg = lstMsg.at(ndxArgs).split(QChar('='));
		if (lstArg.size() != 2) {
			qDebug("*** KJPBS : Malformed inter-application argument : %s", lstMsg.at(ndxArgs).toUtf8().data());
		} else {
			if (lstArg.at(0).compare("KJS", Qt::CaseInsensitive) == 0) {
				strKJSFileName = lstArg.at(1);
			} else if (lstArg.at(1).compare("BibleUUID", Qt::CaseInsensitive) == 0) {
				strBibleUUID = lstArg.at(1);
			} else {
				qDebug("*** KJPBS : Unrecognized inter-application argument : %s", lstMsg.at(ndxArgs).toUtf8().data());
			}
		}
	}

	switch (nCommand) {
		case KAMCE_ACTIVATE_EXISTING:
		{
			int nIndex = m_nLastActivateCanOpener;
			if (nIndex == -1) {
				if (m_lstKJVCanOpeners.size() > 1) nIndex = 0;
			} else {
				assert(false);
				return;
			}
			activateCanOpener(nIndex);
			break;
		}
		case KAMCE_ACTIVATE_EXISTING_OPEN_KJS:
		case KAMCE_NEW_CANOPENER:
		case KAMCE_NEW_CANOPENER_OPEN_KJS:
		{
			bool bNeedDB = false;
			if ((strBibleUUID.isEmpty()) &&  (!strKJSFileName.isEmpty())) {
				// If no UUID was specified and we have a KJS file to open, try to determine the correct
				//		Bible Database from the KJS file itself:
				strBibleUUID = CKJVCanOpener::determineBibleUUIDForKJVSearchFile(strKJSFileName);
				if (!strBibleUUID.isEmpty()) {
					if (TBibleDatabaseList::instance()->atUUID(strBibleUUID).data() == NULL) bNeedDB = true;
				}
			}
			bool bForceOpen = ((nCommand == KAMCE_NEW_CANOPENER_OPEN_KJS) || (nCommand == KAMCE_NEW_CANOPENER));
			if ((m_lstKJVCanOpeners.size() == 1) && (m_lstKJVCanOpeners.at(0)->bibleDatabase()->compatibilityUUID().compare(strBibleUUID, Qt::CaseInsensitive) != 0)) bForceOpen = true;
			CKJVCanOpener *pCanOpener = NULL;
			if ((bForceOpen) || (bNeedDB) || (m_lstKJVCanOpeners.size() != 1)) {
				// If we have more than one, just open a new window and launch the file:
				CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strBibleUUID);
				if ((pBibleDatabase.data() == NULL) && (!strBibleUUID.isEmpty())) {
					if (TBibleDatabaseList::loadBibleDatabase(strBibleUUID, true, NULL)) {
						pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strBibleUUID);
					}
				}
				if (pBibleDatabase.data() == NULL) pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();
				setFileToLoad(strKJSFileName);
				pCanOpener = createKJVCanOpener(pBibleDatabase);
				assert(pCanOpener != NULL);
			} else {
				pCanOpener = m_lstKJVCanOpeners.at(0);
				if (!strKJSFileName.isEmpty()) pCanOpener->openKJVSearchFile(strKJSFileName);
			}
			activateCanOpener(pCanOpener);
			break;
		}
		default:
			break;
	}
}

// ============================================================================

int CMyApplication::execute(bool bBuildDB)
{
	// Restore Locale Language Setting (and save for next time):
	restoreApplicationLanguage();
	saveApplicationLanguage();

	// Setup our Fonts:
#ifdef LOAD_APPLICATION_FONTS
	//	Note: As of Qt 5.2, iOS doesn't currently load fonts correctly and causes:
	//			This plugin does not support application fonts
	//			This plugin does not support propagateSizeHints()
	//	See QTBUG-34490:	https://bugreports.qt-project.org/browse/QTBUG-34490
	//	Temporary workaround is to add these to the Info.plist so iOS will
	//		auto-load them for us:
#ifndef WORKAROUND_QTBUG_34490
	for (int ndxFont = 0; g_constrarrFontFilenames[ndxFont] != NULL; ++ndxFont) {
#ifndef EMSCRIPTEN
		QString strFontFileName = QFileInfo(initialAppDirPath(), g_constrarrFontFilenames[ndxFont]).absoluteFilePath();
#else
		QString strFontFileName = g_constrarrFontFilenames[ndxFont];
#endif
		int nFontStatus = QFontDatabase::addApplicationFont(strFontFileName);
		if (nFontStatus == -1) {
#ifdef QT_DEBUG
			displayWarning(m_pSplash, g_constrInitialization, tr("Failed to load font file:\n\"%1\"", "Errors").arg(strFontFileName));
#endif	// QT_DEBUG
		}
	}
#endif	// WORKAROUND_QTBUG_34490
#endif	// LOAD_APPLICATION_FONTS

#ifdef SHOW_SPLASH_SCREEN
	// Sometimes the splash screen fails to paint, so we'll pump events again
	//	between the fonts and database:
	if (m_pSplash) {
		m_pSplash->repaint();
		processEvents();
	}
#endif

	//	qRegisterMetaTypeStreamOperators<TPhraseTag>("TPhraseTag");

	QString strMainDatabaseUUID;

	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());

		// Main Bible Database:
		settings.beginGroup(constrMainAppBibleDatabaseGroup);
		strMainDatabaseUUID = settings.value(constrBibleDatabaseUUIDKey, CPersistentSettings::instance()->mainBibleDatabaseUUID()).toString();
		if (!strMainDatabaseUUID.isEmpty()) {
			CPersistentSettings::instance()->setMainBibleDatabaseUUID(strMainDatabaseUUID);
		}
		settings.endGroup();

		// Bible Database Settings:
		int nBDBSettings = settings.beginReadArray(constrBibleDatabaseSettingsGroup);
		if (nBDBSettings != 0) {
			for (int ndx = 0; ndx < nBDBSettings; ++ndx) {
				TBibleDatabaseSettings bdbSettings;
				settings.setArrayIndex(ndx);
				QString strUUID = settings.value(constrBibleDatabaseUUIDKey, QString()).toString();
				if (!strUUID.isEmpty()) {
					bdbSettings.setLoadOnStart(settings.value(constrLoadOnStartKey, bdbSettings.loadOnStart()).toBool());
					bdbSettings.setHideHyphens(settings.value(constrHideHyphensKey, bdbSettings.hideHyphens()).toUInt());
					bdbSettings.setHyphenSensitive(settings.value(constrHyphenSensitiveKey, bdbSettings.hyphenSensitive()).toBool());
					CPersistentSettings::instance()->setBibleDatabaseSettings(strUUID, bdbSettings);
				}
			}
		}
		settings.endArray();
	}

	if (m_nSelectedMainBibleDB == BDE_UNKNOWN) {
		// If command-line override wasn't specified, first see if we will be loading a KJS file.
		//		If so, try and determine it's Bible Database:
		if (!fileToLoad().isEmpty()) {
			m_nSelectedMainBibleDB = bibleDescriptorFromUUID(CKJVCanOpener::determineBibleUUIDForKJVSearchFile(fileToLoad()));
		}
		// Else, see if a persistent settings was previously set:
		if (m_nSelectedMainBibleDB == BDE_UNKNOWN) {
			for (unsigned int dbNdx = 0; dbNdx < bibleDescriptorCount(); ++dbNdx) {
				if ((!strMainDatabaseUUID.isEmpty()) && (strMainDatabaseUUID.compare(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx)).m_strUUID, Qt::CaseInsensitive) == 0)) {
					m_nSelectedMainBibleDB = static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx);
					break;
				}
			}
		}
		if (m_nSelectedMainBibleDB == BDE_UNKNOWN) m_nSelectedMainBibleDB = BDE_KJV;				// Default to KJV unless we're told otherwise
	}

#ifndef EMSCRIPTEN
	g_strBibleDatabasePath = QFileInfo(initialAppDirPath(), g_constrBibleDatabasePath).absoluteFilePath();
	g_strDictionaryDatabasePath = QFileInfo(initialAppDirPath(), g_constrDictionaryDatabasePath).absoluteFilePath();
#else
	g_strBibleDatabasePath = g_constrBibleDatabasePath;
#endif

	// Read (and/or Build) our Databases:
	{
#ifdef BUILD_KJV_DATABASE
		CBuildDatabase bdb(m_pSplash);
		if (bBuildDB) {
			// Database Paths for building:
#ifdef NOT_USING_SQL
			// If we can't support SQL, we can't:
			QString strKJVSQLDatabasePath;
#else
			QString strKJVSQLDatabasePath = QFileInfo(g_strBibleDatabasePath, bibleDescriptor(m_nSelectedMainBibleDB).m_strS3DBFilename).absoluteFilePath();
#endif
			QString strKJVCCDatabasePath = QFileInfo(g_strBibleDatabasePath, bibleDescriptor(m_nSelectedMainBibleDB).m_strCCDBFilename).absoluteFilePath();

			if (!bdb.BuildDatabase(strKJVSQLDatabasePath, strKJVCCDatabasePath)) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Failed to Build Bible Database!\nAborting...", "Errors"));
				return -2;
			}
		}
#else
		if (bBuildDB) {
			displayWarning(m_pSplash, g_constrInitialization, tr("Database building isn't supported on this platform/build...", "Errors"));
			return -2;
		}
#endif

		// Read Main Database(s)
		QList<BIBLE_DESCRIPTOR_ENUM> lstAvailableBDEs = TBibleDatabaseList::instance()->availableBibleDatabases();
		for (int ndx = 0; ndx < lstAvailableBDEs.size(); ++ndx) {
			const TBibleDescriptor &bblDesc = bibleDescriptor(lstAvailableBDEs.at(ndx));
			if ((!bblDesc.m_bAutoLoad) &&
				(m_nSelectedMainBibleDB != lstAvailableBDEs.at(ndx)) &&
				(!CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID).loadOnStart())) continue;
			CReadDatabase rdbMain(g_strBibleDatabasePath, g_strDictionaryDatabasePath, m_pSplash);
			assert(rdbMain.haveBibleDatabaseFiles(bblDesc));
			setSplashMessage(tr("Reading:", "Errors") + QString(" %1 ").arg(bblDesc.m_strDBName) + tr("Bible", "Errors"));
			if (!rdbMain.ReadBibleDatabase(bblDesc, (m_nSelectedMainBibleDB == lstAvailableBDEs.at(ndx)))) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Failed to Read and Validate Bible Database!\n%1\nCheck Installation!", "Errors").arg(bblDesc.m_strDBDesc));
				return -3;
			}
		}
		// If the specified database wasn't found, see if we loaded any database and if so, make the
		//		first one loaded (from our priority list) the main database:
		if ((TBibleDatabaseList::instance()->mainBibleDatabase().data() == NULL) &&
			(TBibleDatabaseList::instance()->size() > 0)) {
			TBibleDatabaseList::instance()->setMainBibleDatabase(TBibleDatabaseList::instance()->at(0)->compatibilityUUID());
		}
		if (TBibleDatabaseList::instance()->mainBibleDatabase().data() == NULL) {
			displayWarning(m_pSplash, g_constrInitialization, tr("Failed to find and load a Bible Database!  Check Installation!", "Errors"));
			return -3;
		}

#ifndef EMSCRIPTEN
		// Read Dictionary Database:
		for (unsigned int dbNdx = 0; dbNdx < dictionaryDescriptorCount(); ++dbNdx) {
			const TDictionaryDescriptor &dctDesc = dictionaryDescriptor(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(dbNdx));
			if (!dctDesc.m_bAutoLoad) continue;
			bool bHaveLanguageMatch = false;
			for (int nBBLNdx = 0; nBBLNdx < TBibleDatabaseList::instance()->size(); ++nBBLNdx) {
				if ((TBibleDatabaseList::instance()->at(nBBLNdx).data() != NULL) &&
					(TBibleDatabaseList::instance()->at(nBBLNdx)->language().compare(dctDesc.m_strLanguage, Qt::CaseInsensitive) == 0)) {
					bHaveLanguageMatch = true;
					break;
				}
			}
			if (!bHaveLanguageMatch) continue;			// No need loading the dictionary for a language we don't have a Bible database for
			CReadDatabase rdbDict(g_strBibleDatabasePath, g_strDictionaryDatabasePath, m_pSplash);
			if (!rdbDict.haveDictionaryDatabaseFiles(dctDesc)) continue;
			setSplashMessage(tr("Reading:", "Errors") + QString(" %1 ").arg(dctDesc.m_strDBName) + tr("Dictionary", "Errors"));
			if (!rdbDict.ReadDictionaryDatabase(dctDesc, true, (TDictionaryDatabaseList::instance()->mainDictionaryDatabase().data() == NULL))) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Failed to Read and Validate Dictionary Database!\n%1\nCheck Installation!", "Errors").arg(dctDesc.m_strDBDesc));
				return -5;
			}
		}
#endif	// !EMSCRIPTEN

	}

#ifdef SHOW_SPLASH_SCREEN
	// Show splash for minimum time:
	do {
		processEvents();
	} while (!m_splashTimer.hasExpired(g_connMinSplashTimeMS));
#endif

	// Setup our default font for our controls:
	restoreApplicationFontSettings();


	// Set setDesktopSettingsAware here instead of before app being
	//	created.  Yes, I know that Qt documentation says that this
	//	must be set before creating your QApplication object.
	//	However, that will cause all desktop properties to not
	//	propogate at all.  We actually want them to propogate through,
	//	but not to reprogate when the screen is toggled.  So,
	//	calling it here after it's been created propogates them
	//	the first time, just not if the user (or system) changes
	//	the properties.  This works around the Qt Mac bug as
	//	reported at the bottom of this blog:
	//	http://blog.qt.digia.com/blog/2008/11/16/font-and-palette-propagation-in-qt/
#ifdef Q_OS_MAC
	setDesktopSettingsAware(false);
#endif

	// Update settings for next time.  Use application font instead of
	//		our variables in case Qt substituted for another available font:
	saveApplicationFontSettings();

	// Connect TextBrightness change notifications:
	setupTextBrightnessStyleHooks();

	// Create default empty KJN file before we create CKJVCanOpener:
	g_pUserNotesDatabase = QSharedPointer<CUserNotesDatabase>(new CUserNotesDatabase());

	m_dlyNotesFilesAutoSave.setMinimumDelay(CPersistentSettings::instance()->notesFileAutoSaveTime()*60000);		// Convert minutes->milliseconds
	connect(&m_dlyNotesFilesAutoSave, SIGNAL(triggered()), this, SLOT(en_notesFileAutoSaveTriggered()));
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedUserNotesDatabase()), this, SLOT(en_changedUserNotesDatabase()));
	connect(CPersistentSettings::instance(), SIGNAL(changedNotesFileAutoSaveTime(int)), this, SLOT(en_changedNoteesFileAutoSaveTime(int)));

#ifdef USE_MDI_MAIN_WINDOW
	g_pMdiArea = new QMdiArea();
	g_pMdiArea->show();
#ifdef Q_OS_WIN32
	g_pMdiArea->setWindowIcon(QIcon(":/res/bible.ico"));
#else
	g_pMdiArea->setWindowIcon(QIcon(":/res/bible_48.png"));
#endif
#endif

	// Must have database read above before we create main or else the
	//		data won't be available for the browser objects and such:
#ifdef SHOW_SPLASH_SCREEN
	CKJVCanOpener *pMain = createKJVCanOpener(TBibleDatabaseList::instance()->mainBibleDatabase());
	if (m_pSplash != NULL) {
		m_pSplash->finish((g_pMdiArea.data() != NULL) ? static_cast<QWidget *>(g_pMdiArea.data()) : static_cast<QWidget *>(pMain));
		delete m_pSplash;
		m_pSplash = NULL;
	}
#else
	createKJVCanOpener(TBibleDatabaseList::instance()->mainBibleDatabase());
#endif

	return 0;
}

// ============================================================================
