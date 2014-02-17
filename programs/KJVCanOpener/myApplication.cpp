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
#include "dbDescriptors.h"
#include "KJVCanOpener.h"
#include "ReportError.h"

#ifdef USING_SINGLEAPPLICATION
#include <singleapplication.h>
#endif

#ifdef SHOW_SPLASH_SCREEN
#include <QPixmap>
#include <QSplashScreen>
#include <QElapsedTimer>
#endif

#include <QProxyStyle>
//#include <QtPlugin>
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

#ifdef BUILD_KJV_DATABASE
#include "BuildDB.h"
#endif
#include "ReadDB.h"

#include <assert.h>

//Q_IMPORT_PLUGIN(qsqlite)

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

	//////////////////////////////////////////////////////////////////////

#ifdef SHOW_SPLASH_SCREEN
	const int g_connMinSplashTimeMS = 5000;			// Minimum number of milliseconds to display splash screen
	const int g_connInterAppSplashTimeMS = 2000;	// Splash Time for Inter-Application communications
#endif

	const QString g_constrInitialization = QObject::tr("King James Pure Bible Search Initialization");

	//////////////////////////////////////////////////////////////////////

#ifdef Q_OS_ANDROID
	// --------------------------------------------------------------------------------------------------------- Android ------------------------
// Android deploy mechanism will automatically include our plugins, so these shouldn't be needed:
//	const char *g_constrPluginsPath = "assets:/plugins/";
//	const char *g_constrPluginsPath = "/data/data/com.dewtronics.KingJamesPureBibleSearch/plugins/";

	const char *g_constrBibleDatabasePath = "KJVCanOpener/db/";
	const char *g_constrDictionaryDatabasePath = "KJVCanOpener/db/";
	const char *g_constrUserDatabaseTemplateFilename = "KJVCanOpener/db/kjvuser.s3db";
#elif defined(Q_OS_IOS)
	// --------------------------------------------------------------------------------------------------------- iOS ----------------------------
	const char *g_constrPluginsPath = "./Frameworks/";
	const char *g_constrBibleDatabasePath = "./assets/KJVCanOpener/db/";
	const char *g_constrDictionaryDatabasePath = "./assets/KJVCanOpener/db/";
	const char *g_constrUserDatabaseTemplateFilename = "./assets/KJVCanOpener/db/kjvuser.s3db";
#elif defined(Q_OS_OSX) || defined(Q_OS_MACX)
	// --------------------------------------------------------------------------------------------------------- Mac ----------------------------
	const char *g_constrPluginsPath = "../Frameworks/";
	const char *g_constrBibleDatabasePath = "../Resources/db/";
	const char *g_constrDictionaryDatabasePath = "../Resources/db/";
	const char *g_constrUserDatabaseTemplateFilename = "../Resources/db/kjvuser.s3db";
#elif defined(EMSCRIPTEN)
	// --------------------------------------------------------------------------------------------------------- EMSCRIPTEN ---------------------
	#ifdef EMSCRIPTEN_NATIVE
		const char *g_constrBibleDatabasePath = "./data/";
	#else
		const char *g_constrBibleDatabasePath = "data/";
	#endif
#else
	// --------------------------------------------------------------------------------------------------------- Linux --------------------------
	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";
	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";
	const char *g_constrDictionaryDatabasePath = "../../KJVCanOpener/db/";
	const char *g_constrUserDatabaseTemplateFilename = "../../KJVCanOpener/db/kjvuser.s3db";
#endif
	const char *g_constrUserDatabaseFilename = "kjvuser.s3db";

	//////////////////////////////////////////////////////////////////////

#ifdef LOAD_APPLICATION_FONTS

#ifdef Q_OS_ANDROID
	// --------------------------------------------------------------------------------------------------------- Android ------------------------
	const char *g_constrScriptBLFontFilename = "KJVCanOpener/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "KJVCanOpener/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "KJVCanOpener/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "KJVCanOpener/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "KJVCanOpener/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "KJVCanOpener/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "KJVCanOpener/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "KJVCanOpener/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "KJVCanOpener/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "KJVCanOpener/fonts/DejaVuSerif.ttf";
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
#else
	// --------------------------------------------------------------------------------------------------------- Linux --------------------------
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

CBibleDatabasePtr locateBibleDatabase(const QString &strUUID)
{
	QString strTargetUUID = strUUID;

	if (strTargetUUID.isEmpty()) {
		// Default database is KJV
		strTargetUUID = bibleDescriptor(BDE_KJV).m_strUUID;
	}

	for (int ndx = 0; ndx < g_lstBibleDatabases.size(); ++ndx) {
		if (g_lstBibleDatabases.at(ndx)->compatibilityUUID().compare(strTargetUUID, Qt::CaseInsensitive) == 0)
			return g_lstBibleDatabases.at(ndx);
	}

	return CBibleDatabasePtr();
}

CDictionaryDatabasePtr locateDictionaryDatabase(const QString &strUUID)
{
	QString strTargetUUID = strUUID;

	if (strTargetUUID.isEmpty()) {
		// Default database is Web1828
		strTargetUUID = dictionaryDescriptor(DDE_WEB1828).m_strUUID;
	}
	for (int ndx = 0; ndx < g_lstDictionaryDatabases.size(); ++ndx) {
		if (g_lstDictionaryDatabases.at(ndx)->compatibilityUUID().compare(strTargetUUID, Qt::CaseInsensitive) == 0)
			return g_lstDictionaryDatabases.at(ndx);
	}

	return CDictionaryDatabasePtr();
}

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
		m_pSplash(NULL)
{
#ifdef Q_OS_ANDROID
	m_strInitialAppDirPath = QDir::homePath();
#else
	m_strInitialAppDirPath = applicationDirPath();
#endif
	m_strStartupStyleSheet = styleSheet();

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

	// Setup our SQL/Image Plugin paths:
#if !defined(Q_OS_ANDROID) && !defined(EMSCRIPTEN)
	QFileInfo fiPlugins(initialAppDirPath(), g_constrPluginsPath);
	addLibraryPath(fiPlugins.absolutePath());
#endif
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
	g_lstBibleDatabases.clear();
	g_pMainBibleDatabase.clear();
	g_lstDictionaryDatabases.clear();
	g_pMainDictionaryDatabase.clear();
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
										strOffsetSpace + QObject::tr("Please Wait...") +
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
#elif EMSCRIPTEN
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

bool CMyApplication::notify(QObject *pReceiver, QEvent *pEvent)
{
#ifdef EMSCRIPTEN
	// For some reason, Emscripten causes us to receive Action Changed notifications on
	//		a NULL Receiver object.  Haven't found exactly where these are originating,
	//		but they seem fairly beneign.  So just defaulting them here so we don't get
	//		a recurrent debug log notification message:
	if (pReceiver == NULL) return true;
#endif

	try {
#ifdef USING_QT_SINGLEAPPLICATION
		return QtSingleApplication::notify(pReceiver, pEvent);
#else
		return QApplication::notify(pReceiver, pEvent);
#endif
	} catch (const std::exception &ex) {
		qDebug("std::exception was caught: %s", ex.what());
	} catch (...) {
		qDebug("Unknown exception was caught");
		assert(false);
	}

	return false;
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
			bool bForceOpen = ((nCommand == KAMCE_NEW_CANOPENER_OPEN_KJS) || (nCommand == KAMCE_NEW_CANOPENER));
			CKJVCanOpener *pCanOpener = NULL;
			if ((bForceOpen) || (m_lstKJVCanOpeners.size() != 1)) {
				// If we have more than one, just open a new window and launch the file:
				CBibleDatabasePtr pBibleDatabase = locateBibleDatabase(strBibleUUID);
				if (pBibleDatabase.data() == NULL) pBibleDatabase = g_pMainBibleDatabase;
				pCanOpener = createKJVCanOpener(pBibleDatabase);
				assert(pCanOpener != NULL);
			} else {
				pCanOpener = m_lstKJVCanOpeners.at(0);
			}
			activateCanOpener(pCanOpener);
			if (!strKJSFileName.isEmpty()) pCanOpener->openKJVSearchFile(strKJSFileName);
			break;
		}
		default:
			break;
	}
}

// ============================================================================

int CMyApplication::execute(bool bBuildDB)
{
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
			displayWarning(m_pSplash, g_constrInitialization, QObject::tr("Failed to load font file:\n\"%1\"").arg(strFontFileName));
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

#ifndef EMSCRIPTEN
	QString strBibleDatabasePath = QFileInfo(initialAppDirPath(), g_constrBibleDatabasePath).absoluteFilePath();
	QString strDictionaryDatabasePath = QFileInfo(initialAppDirPath(), g_constrDictionaryDatabasePath).absoluteFilePath();
#else
	QString strBibleDatabasePath = g_constrBibleDatabasePath;
	QString strDictionaryDatabasePath;
#endif

	// Read (and/or Build) our Databases:
	{
#ifdef BUILD_KJV_DATABASE
		CBuildDatabase bdb(m_pSplash);
		if (bBuildDB) {
			// Database Paths for building (Default to KJV names when building, user can move them to proper names when done):
#ifdef NOT_USING_SQL
			// If we can't support SQL, we can't:
			QString strKJVSQLDatabasePath;
#else
			QString strKJVSQLDatabasePath = QFileInfo(strBibleDatabasePath, bibleDescriptor(BDE_KJV).m_strS3DBFilename).absoluteFilePath();
#endif
			QString strKJVCCDatabasePath = QFileInfo(strBibleDatabasePath, bibleDescriptor(BDE_KJV).m_strCCDBFilename).absoluteFilePath();

			if (!bdb.BuildDatabase(strKJVSQLDatabasePath, strKJVCCDatabasePath)) {
				displayWarning(m_pSplash, g_constrInitialization, QObject::tr("Failed to Build Bible Database!\nAborting..."));
				return -2;
			}
		}
#else
		if (bBuildDB) {
			displayWarning(m_pSplash, g_constrInitialization, QObject::tr("Database building isn't supported on this platform/build..."));
			return -2;
		}
#endif

		// Read Main Database(s)
		for (unsigned int dbNdx = 0; dbNdx < bibleDescriptorCount(); ++dbNdx) {
			const TBibleDescriptor &bblDesc = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx));
			CReadDatabase rdbMain(strBibleDatabasePath, strDictionaryDatabasePath, m_pSplash);
			if (!rdbMain.haveBibleDatabaseFiles(bblDesc)) continue;
			setSplashMessage(QString("Reading: %1 Bible").arg(bblDesc.m_strDBName));
			if (!rdbMain.ReadBibleDatabase(bblDesc, (g_pMainBibleDatabase.data() == NULL))) {
				displayWarning(m_pSplash, g_constrInitialization, QObject::tr("Failed to Read and Validate Bible Database!\n%1\nCheck Installation!").arg(bblDesc.m_strDBDesc));
				return -3;
			}
		}
		if (g_pMainBibleDatabase.data() == NULL) {
			displayWarning(m_pSplash, g_constrInitialization, QObject::tr("Failed to find and load a Bible Database!  Check Installation!"));
			return -3;
		}

#ifndef EMSCRIPTEN
		// Read User Database:
		QFileInfo fiUserDatabaseTemplate(initialAppDirPath(), g_constrUserDatabaseTemplateFilename);
	#if QT_VERSION < 0x050000
		QString strDataFolder = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	#else
		QString strDataFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	#endif
		QFileInfo fiUserDatabase(strDataFolder, g_constrUserDatabaseFilename);
		if (CPersistentSettings::instance()->settings() == NULL) {			// Will be NULL if we are in stealth mode
			QDir dirDataFolder;
			dirDataFolder.mkpath(strDataFolder);
		}
		CReadDatabase rdbUser(strBibleDatabasePath, strDictionaryDatabasePath, m_pSplash);
		if (!fiUserDatabase.exists()) {
			// If the user's database doesn't exist, see if the template one
			//		does.  If so, read and use it:
			if ((fiUserDatabaseTemplate.exists()) && (fiUserDatabaseTemplate.isFile())) {
				rdbUser.ReadUserDatabase(CReadDatabase::DTE_SQL, fiUserDatabaseTemplate.absoluteFilePath(), true);
			}
		} else {
			// If the user's database does exist, read it. But if it isn't a proper file
			//		or if the read fails, try reading the template if it exists:
			if ((!fiUserDatabase.isFile()) || (!rdbUser.ReadUserDatabase(CReadDatabase::DTE_SQL, fiUserDatabase.absoluteFilePath(), true))) {
				if ((fiUserDatabaseTemplate.exists()) && (fiUserDatabaseTemplate.isFile())) {
					rdbUser.ReadUserDatabase(CReadDatabase::DTE_SQL, fiUserDatabaseTemplate.absoluteFilePath(), true);
				}
			}
		}
		// At this point, userPhrases() will either be the:
		//		- User Database if it existed
		//		- Else, the Template Database if it existed
		//		- Else, empty

		// Read Dictionary Database:
		for (unsigned int dbNdx = 0; dbNdx < dictionaryDescriptorCount(); ++dbNdx) {
			const TDictionaryDescriptor &dctDesc = dictionaryDescriptor(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(dbNdx));
			CReadDatabase rdbDict(strBibleDatabasePath, strDictionaryDatabasePath, m_pSplash);
			if (!rdbDict.haveDictionaryDatabaseFiles(dctDesc)) continue;
			setSplashMessage(QString("Reading: %1 Dictionary").arg(dctDesc.m_strDBName));
			if (!rdbDict.ReadDictionaryDatabase(dctDesc, true, (g_pMainDictionaryDatabase.data() == NULL))) {
				displayWarning(m_pSplash, g_constrInitialization, QObject::tr("Failed to Read and Validate Dictionary Database!\n%1\nCheck Installation!").arg(dctDesc.m_strDBDesc));
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
	CKJVCanOpener *pMain = createKJVCanOpener(g_pMainBibleDatabase);
#ifdef SHOW_SPLASH_SCREEN
	if (m_pSplash != NULL) {
		m_pSplash->finish((g_pMdiArea.data() != NULL) ? static_cast<QWidget *>(g_pMdiArea.data()) : static_cast<QWidget *>(pMain));
		delete m_pSplash;
		m_pSplash = NULL;
	}
#endif

	return 0;
}

// ============================================================================
