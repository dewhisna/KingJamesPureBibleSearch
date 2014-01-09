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

#include <QtCore>
#include <QPixmap>
#include <QSplashScreen>
#include <QWidget>
#include <QMainWindow>
#include <QElapsedTimer>
#include <QDesktopWidget>
#include <QPainter>
#include <QLocale>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
//#include <QtPlugin>
#include <QFontDatabase>
#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif
#include <QDir>
#include <QObject>
#include <QProxyStyle>
#include <QSharedPointer>
#include <QPointer>
#include <QList>
#ifdef USING_SINGLEAPPLICATION
#include <singleapplication.h>
#endif

#include "myApplication.h"
#include "KJVCanOpener.h"

#include "version.h"
#include "dbstruct.h"
#include "BuildDB.h"
#include "ReadDB.h"

#include "PersistentSettings.h"
#include "UserNotesDatabase.h"

#include <assert.h>

#ifdef Q_OS_WIN
// Needed to call CreateMutex to lockout installer running while we are:
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//Q_IMPORT_PLUGIN(qsqlite)

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const int g_connMinSplashTimeMS = 5000;		// Minimum number of milliseconds to display splash screen
	const int g_connInterAppSplashTimeMS = 2000;	// Splash Time for Inter-Application communications

	const QString g_constrInitialization = QObject::tr("King James Pure Bible Search Initialization");

#ifdef Q_OS_ANDROID
// Android deploy mechanism will automatically include our plugins, so these shouldn't be needed:
//	const char *g_constrPluginsPath = "assets:/plugins/";
//	const char *g_constrPluginsPath = "/data/data/com.dewtronics.KingJamesPureBibleSearch/plugins/";

	const char *g_constrKJVDatabaseFilename = "KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "KJVCanOpener/db/kjvuser.s3db";
	const char *g_constrWeb1828DatabaseFilename = "KJVCanOpener/db/dct-web1828.s3db";
#elif defined(Q_OS_IOS)
	const char *g_constrPluginsPath = "./Frameworks/";
	const char *g_constrKJVDatabaseFilename = "./assets/KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "./assets/KJVCanOpener/db/kjvuser.s3db";
	const char *g_constrWeb1828DatabaseFilename = "./assets/KJVCanOpener/db/dct-web1828.s3db";
#elif defined(Q_OS_OSX) || defined(Q_OS_MACX)
	const char *g_constrPluginsPath = "../Frameworks/";
	const char *g_constrKJVDatabaseFilename = "../Resources/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "../Resources/db/kjvuser.s3db";
	const char *g_constrWeb1828DatabaseFilename = "../Resources/db/dct-web1828.s3db";
#else
	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";
	const char *g_constrKJVDatabaseFilename = "../../KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "../../KJVCanOpener/db/kjvuser.s3db";
	const char *g_constrWeb1828DatabaseFilename = "../../KJVCanOpener/db/dct-web1828.s3db";
#endif
	const char *g_constrUserDatabaseFilename = "kjvuser.s3db";

#ifdef Q_OS_ANDROID
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
#elif defined(Q_OS_OSX) || defined(Q_OS_MACX)
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
#else
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
}	// namespace

// ============================================================================

class MyProxyStyle : public QProxyStyle
{
public:
	int styleHint(StyleHint hint, const QStyleOption *option = 0,
				const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const
	{
		if (hint == QStyle:: SH_ItemView_ActivateItemOnSingleClick) return 0;

		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

// ============================================================================

int main(int argc, char *argv[])
{
	CMyApplication app(argc, argv);
	g_pMyApplication = &app;
	app.setApplicationVersion(VER_QT);
	app.setApplicationName(VER_APPNAME_STR_QT);
	app.setOrganizationName(VER_ORGNAME_STR_QT);
	app.setOrganizationDomain(VER_ORGDOMAIN_STR_QT);
#ifdef USING_SINGLEAPPLICATION
	SingleApplication instance(g_constrApplicationID, &app);
#endif
#ifdef USING_QT_SINGLEAPPLICATION
	QtSingleApplication &instance = app;
#endif

	app.setStyle(new MyProxyStyle());			// Note: QApplication will take ownership of this (no need for delete)

	// Setup our SQL/Image Plugin paths:
#ifndef Q_OS_ANDROID
	QFileInfo fiPlugins(app.initialAppDirPath(), g_constrPluginsPath);
	app.addLibraryPath(fiPlugins.absolutePath());
#endif

	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
	QString strKJSFile;
	bool bBuildDB = false;
	bool bStealthMode = false;
	QString strStealthSettingsFilename;

	Q_INIT_RESOURCE(KJVCanOpener);

#if defined(USING_SINGLEAPPLICATION) || defined(USING_QT_SINGLEAPPLICATION)

	// Hook receiving messages from other KJPBS app launches:
	app.connect(&instance, SIGNAL(messageReceived(const QString &)), &app, SLOT(receivedKJPBSMessage(const QString &)));

#endif

#ifdef Q_OS_WIN32
	app.setWindowIcon(QIcon(":/res/bible.ico"));
#else
#ifndef Q_OS_MAC			// Normally, this would also include Mac, but Mac has its icon set in the .pro file.  Loading this one makes it fuzzy.
	app.setWindowIcon(QIcon(":/res/bible_48.png"));
#endif
#endif

	QPixmap pixSplash(":/res/KJPBS_SplashScreen800x500.png");
	QSplashScreen *splash = new QSplashScreen(pixSplash);
	splash->show();
#ifdef Q_OS_IOS
	// The following is a work-around for QTBUG-35787
	QEventLoop loop;
	QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
	loop.exec();
#endif
	splash->raise();
	QString strSpecialVersion(SPECIAL_BUILD ? QString(VER_SPECIALVERSION_STR) : QString());
	const QString strOffsetSpace = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	if (!strSpecialVersion.isEmpty()) strSpecialVersion = "<br>\n" + strOffsetSpace + strSpecialVersion;
	splash->showMessage(QString("<html><body><table height=375 width=500><tr><td>&nbsp;</td></tr></table><div align=\"center\"><font size=+1 color=#FFFFFF><b>") +
							strOffsetSpace + QObject::tr("Please Wait...") +
							strSpecialVersion +
							QString("</b></font></div></body></html>"), Qt::AlignBottom | Qt::AlignLeft);
	splash->repaint();
	app.processEvents();

#ifdef Q_OS_WIN
	HANDLE hMutex = CreateMutexW(NULL, false, L"KJVCanOpenerMutex");
	assert(hMutex != NULL);
	// Note: System will automatically close the mutex object when we
	//			exit and InnoSetup actually suggest we leave it open
#endif

	QElapsedTimer splashTimer;
	splashTimer.start();

	// Parse the Commmand-line:
	if (strKJSFile.isEmpty() && !app.fileToLoad().isEmpty()) strKJSFile = app.fileToLoad();

	bool bLookingForSettings = false;
	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg(argv[ndx]);
		if (!strArg.startsWith("-")) {
			if (bLookingForSettings) {
				strStealthSettingsFilename = strArg;
				bLookingForSettings = false;
			} else if (strKJSFile.isEmpty()) {
				strKJSFile = strArg;
			} else {
				QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Unexpected command-line filename \"%1\"").arg(strArg));
			}
		} else if (!bLookingForSettings) {
			if (strArg.compare("-builddb", Qt::CaseInsensitive) == 0) {
				bBuildDB = true;
			} else if (strArg.compare("-stealth", Qt::CaseInsensitive) == 0) {
				bStealthMode = true;
			} else if (strArg.compare("-settings", Qt::CaseInsensitive) == 0) {
				bStealthMode = true;
				bLookingForSettings = true;
			} else {
				QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Unrecognized command-line option \"%1\"").arg(strArg));
			}
		} else {
			if (bLookingForSettings) {
				QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Was expecting Settings Filename, but received: \"%1\" instead").arg(strArg));
				bLookingForSettings = false;
			}
		}
	}

#if defined(USING_SINGLEAPPLICATION) || defined(USING_QT_SINGLEAPPLICATION)

	// Check for existing KJPBS and have it handle this launch request:
	if (instance.isRunning()) {
		if (bBuildDB) {
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Can't Build Database while app is already running!"));
			delete splash;
			return -2;
		}

		do {
			app.processEvents();
		} while (!splashTimer.hasExpired(g_connInterAppSplashTimeMS));

		QString strMessage;
		if (!strKJSFile.isEmpty()) {
			QFileInfo fiKJSFile(strKJSFile);
			strMessage = app.createKJPBSMessage(CMyApplication::KAMCE_ACTIVATE_EXISTING_OPEN_KJS, QStringList(QString("KJS=%1").arg(fiKJSFile.absoluteFilePath())));
		} else{
			strMessage = app.createKJPBSMessage(CMyApplication::KAMCE_NEW_CANOPENER, QStringList());
		}

		if (instance.sendMessage(strMessage, 5000)) {
			delete splash;
			return 0;
		} else {
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("There appears to be another copy of King James Pure Bible Search running, but it is not responding. "
																			 "Please check the running copy to see if it's functioning and revive it and/or reboot."));
			delete splash;
			return -1;
		}
	}

#endif

	// Check/Set Stealth Mode:
	// Must do this after multiple window launch (above), but before we
	//		attempt to read any persistent settings:
	if (bStealthMode) {
		CPersistentSettings::instance()->setStealthMode(strStealthSettingsFilename);
	}

	// Setup our Fonts:
	//	Note: As of Qt 5.2, iOS doesn't currently load fonts correctly and causes:
	//			This plugin does not support application fonts
	//			This plugin does not support propagateSizeHints()
	//	See QTBUG-34490:	https://bugreports.qt-project.org/browse/QTBUG-34490
	//	Temporary workaround is to add these to the Info.plist so iOS will
	//		auto-load them for us:
#ifndef Q_OS_IOS
	for (int ndxFont = 0; g_constrarrFontFilenames[ndxFont] != NULL; ++ndxFont) {
		QFileInfo fiFont(app.initialAppDirPath(), g_constrarrFontFilenames[ndxFont]);
		int nFontStatus = QFontDatabase::addApplicationFont(fiFont.absoluteFilePath());
		if (nFontStatus == -1) {
#ifdef QT_DEBUG
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to load font file:\n\"%1\"").arg(fiFont.absoluteFilePath()));
#endif
		}
	}
#endif

	// Sometimes the splash screen fails to paint, so we'll pump events again
	//	between the fonts and database:
	splash->repaint();
	app.processEvents();

	// Database Paths:
	QFileInfo fiKJVDatabase(app.initialAppDirPath(), g_constrKJVDatabaseFilename);
	QFileInfo fiUserDatabaseTemplate(app.initialAppDirPath(), g_constrUserDatabaseTemplateFilename);
	QFileInfo fiWeb1828DictDatabase(app.initialAppDirPath(), g_constrWeb1828DatabaseFilename);
#if QT_VERSION < 0x050000
	QString strDataFolder = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
	QString strDataFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
	QFileInfo fiUserDatabase(strDataFolder, g_constrUserDatabaseFilename);
	if (!bStealthMode) {
		QDir dirDataFolder;
		dirDataFolder.mkpath(strDataFolder);
	}

//	qRegisterMetaTypeStreamOperators<TPhraseTag>("TPhraseTag");


//CBuildDatabase adb(splash);
//adb.BuildDatabase(fiKJVDatabase.absoluteFilePath());
//return 0;

	// Read (and/or Build) our Databases:
	{
		CBuildDatabase bdb(splash);
		if (bBuildDB) {
			if (!bdb.BuildDatabase(fiKJVDatabase.absoluteFilePath())) {
				QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to Build Bible Database!\nAborting..."));
				delete splash;
				return -2;
			}
		}

		// Read Main Database
		CReadDatabase rdbMain(splash);
		if (!rdbMain.ReadBibleDatabase(fiKJVDatabase.absoluteFilePath(), true)) {
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to Read and Validate Bible Database!\n%1\nCheck Installation!").arg(fiKJVDatabase.absoluteFilePath()));
			delete splash;
			return -3;
		}

		// Read User Database:
		CReadDatabase rdbUser(splash);
		if (!fiUserDatabase.exists()) {
			// If the user's database doesn't exist, see if the template one
			//		does.  If so, read and use it:
			if ((fiUserDatabaseTemplate.exists()) && (fiUserDatabaseTemplate.isFile())) {
				rdbUser.ReadUserDatabase(fiUserDatabaseTemplate.absoluteFilePath(), true);
			}
		} else {
			// If the user's database does exist, read it. But if it isn't a proper file
			//		or if the read fails, try reading the template if it exists:
			if ((!fiUserDatabase.isFile()) || (!rdbUser.ReadUserDatabase(fiUserDatabase.absoluteFilePath(), true))) {
				if ((fiUserDatabaseTemplate.exists()) && (fiUserDatabaseTemplate.isFile())) {
					rdbUser.ReadUserDatabase(fiUserDatabaseTemplate.absoluteFilePath(), true);
				}
			}
		}
		// At this point, userPhrases() will either be the:
		//		- User Database if it existed
		//		- Else, the Template Database if it existed
		//		- Else, empty

		// Read Dictionary Database:
		CReadDatabase rdbDict(splash);
		if (fiWeb1828DictDatabase.exists()) {
			const TDictionaryDescriptor &descWeb1828 = dictionaryDescriptor(DDE_WEB1828);
			if (!rdbDict.ReadDictionaryDatabase(fiWeb1828DictDatabase.absoluteFilePath(), descWeb1828.m_strDBName, descWeb1828.m_strDBDesc, descWeb1828.m_strUUID, true, true)) {
				QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to Read and Validate Webster 1828 Dictionary Database!\nCheck Installation!"));
				delete splash;
				return -5;
			}
		}
	}

	// Show splash for minimum time:
	do {
		app.processEvents();
	} while (!splashTimer.hasExpired(g_connMinSplashTimeMS));


	// Setup our default font for our controls:
	app.restoreApplicationFontSettings();


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
	app.setDesktopSettingsAware(false);
#endif

	// Update settings for next time.  Use application font instead of
	//		our variables in case Qt substituted for another available font:
	app.saveApplicationFontSettings();

	// Connect TextBrightness change notifications:
	app.setupTextBrightnessStyleHooks();

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
	CKJVCanOpener *pMain = app.createKJVCanOpener(g_pMainBibleDatabase);
	splash->finish((g_pMdiArea.data() != NULL) ? static_cast<QWidget *>(g_pMdiArea.data()) : static_cast<QWidget *>(pMain));
	delete splash;

	if (!strKJSFile.isEmpty()) pMain->openKJVSearchFile(strKJSFile);

	int nRetVal = 0;
	bool bDone = false;

	while (!bDone) {
		nRetVal = app.exec();
		if ((nRetVal != 0) || (!app.areRestarting())) {
			bDone = true;
		} else {
			app.createKJVCanOpener(g_pMainBibleDatabase);
		}
	}

	QFontDatabase::removeAllApplicationFonts();

	return nRetVal;
}

