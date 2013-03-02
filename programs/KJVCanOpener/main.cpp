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
#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <QWidget>
#include <QMainWindow>
#include <QTime>
#include <QDesktopWidget>
#include <QPainter>
#include <QLocale>
#include <QMessageBox>
#include <QFileInfo>
//#include <QtPlugin>
#include <QFontDatabase>
#include <QDesktopServices>
#include <QDir>

#include "KJVCanOpener.h"

#include "version.h"
#include "dbstruct.h"
#include "BuildDB.h"
#include "ReadDB.h"

#include "PersistentSettings.h"

#include <assert.h>

#ifdef Q_WS_WIN
// Needed to call CreateMutex to lockout installer running while we are:
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//Q_IMPORT_PLUGIN(qsqlite)

QWidget *g_pMainWindow = NULL;

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const int g_connMinSplashTimeMS = 5000;		// Minimum number of milliseconds to display splash screen

	const char *g_constrInitialization = "King James Pure Bible Search Initialization";

#ifndef Q_WS_MAC
	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";
	const char *g_constrDatabaseFilename = "../../KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "../../KJVCanOpener/db/kjvuser.s3db";
#else
	const char *g_constrPluginsPath = "../Frameworks/";
	const char *g_constrDatabaseFilename = "../Resources/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "../Resources/db/kjvuser.s3db";
#endif
	const char *g_constrUserDatabaseFilename = "kjvuser.s3db";

#ifndef Q_WS_MAC

	const char *g_constrScriptBLFontFilename = "../../KJVCanOpener/fonts/SCRIPTBL.TTF";
#ifndef Q_WS_WIN
//	const char *g_constrDejaVuSans_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf";
//	const char *g_constrDejaVuSans_Bold = "../../KJVCanOpener/fonts/DejaVuSans-Bold.ttf";
//	const char *g_constrDejaVuSansCondensed_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf";
//	const char *g_constrDejaVuSansCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../../KJVCanOpener/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../../KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf";
//	const char *g_constrDejaVuSansMono_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf";
//	const char *g_constrDejaVuSansMono_Bold = "../../KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../../KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../../KJVCanOpener/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../../KJVCanOpener/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../../KJVCanOpener/fonts/DejaVuSans.ttf";
//	const char *g_constrDejaVuSerif_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf";
//	const char *g_constrDejaVuSerif_Bold = "../../KJVCanOpener/fonts/DejaVuSerif-Bold.ttf";
//	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
//	const char *g_constrDejaVuSerifCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf";
//	const char *g_constrDejaVuSerifCondensed_Italic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf";
//	const char *g_constrDejaVuSerifCondensed = "../../KJVCanOpener/fonts/DejaVuSerifCondensed.ttf";
//	const char *g_constrDejaVuSerif_Italic = "../../KJVCanOpener/fonts/DejaVuSerif-Italic.ttf";
//	const char *g_constrDejaVuSerif = "../../KJVCanOpener/fonts/DejaVuSerif.ttf";
#endif	// Q_WS_WIN

#else
	const char *g_constrScriptBLFontFilename = "../Resources/fonts/SCRIPTBL.TTF";

//	const char *g_constrDejaVuSans_BoldOblique = "../Resources/fonts/DejaVuSans-BoldOblique.ttf";
//	const char *g_constrDejaVuSans_Bold = "../Resources/fonts/DejaVuSans-Bold.ttf";
//	const char *g_constrDejaVuSansCondensed_BoldOblique = "../Resources/fonts/DejaVuSansCondensed-BoldOblique.ttf";
//	const char *g_constrDejaVuSansCondensed_Bold = "../Resources/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../Resources/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../Resources/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../Resources/fonts/DejaVuSans-ExtraLight.ttf";
//	const char *g_constrDejaVuSansMono_BoldOblique = "../Resources/fonts/DejaVuSansMono-BoldOblique.ttf";
//	const char *g_constrDejaVuSansMono_Bold = "../Resources/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../Resources/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../Resources/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../Resources/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../Resources/fonts/DejaVuSans.ttf";
//	const char *g_constrDejaVuSerif_BoldItalic = "../Resources/fonts/DejaVuSerif-BoldItalic.ttf";
//	const char *g_constrDejaVuSerif_Bold = "../Resources/fonts/DejaVuSerif-Bold.ttf";
//	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../Resources/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
//	const char *g_constrDejaVuSerifCondensed_Bold = "../Resources/fonts/DejaVuSerifCondensed-Bold.ttf";
//	const char *g_constrDejaVuSerifCondensed_Italic = "../Resources/fonts/DejaVuSerifCondensed-Italic.ttf";
//	const char *g_constrDejaVuSerifCondensed = "../Resources/fonts/DejaVuSerifCondensed.ttf";
//	const char *g_constrDejaVuSerif_Italic = "../Resources/fonts/DejaVuSerif-Italic.ttf";
//	const char *g_constrDejaVuSerif = "../Resources/fonts/DejaVuSerif.ttf";
#endif	// Q_WS_MAC


	const char *g_constrarrFontFilenames[] = {
		g_constrScriptBLFontFilename,
#ifndef Q_WS_WIN
//		g_constrDejaVuSans_BoldOblique,
//		g_constrDejaVuSans_Bold,
//		g_constrDejaVuSansCondensed_BoldOblique,
//		g_constrDejaVuSansCondensed_Bold,
		g_constrDejaVuSansCondensed_Oblique,
		g_constrDejaVuSansCondensed,
		g_constrDejaVuSans_ExtraLight,
//		g_constrDejaVuSansMono_BoldOblique,
//		g_constrDejaVuSansMono_Bold,
		g_constrDejaVuSansMono_Oblique,
		g_constrDejaVuSansMono,
		g_constrDejaVuSans_Oblique,
		g_constrDejaVuSans,
//		g_constrDejaVuSerif_BoldItalic,
//		g_constrDejaVuSerif_Bold,
//		g_constrDejaVuSerifCondensed_BoldItalic,
//		g_constrDejaVuSerifCondensed_Bold,
//		g_constrDejaVuSerifCondensed_Italic,
//		g_constrDejaVuSerifCondensed,
//		g_constrDejaVuSerif_Italic,
//		g_constrDejaVuSerif,
#endif
		NULL
	};


	// Key constants:
	// --------------
	const QString constrMainAppControlGroup("MainApp/Controls");
	const QString constrFontNameKey("FontName");
	const QString constrFontSizeKey("FontSize");

}	// namespace

// ============================================================================


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
	QString strKJSFile;
	bool bBuildDB = false;

	app.setApplicationVersion(VER_QT);
	app.setApplicationName(VER_APPNAME_STR_QT);
	app.setOrganizationName(VER_ORGNAME_STR_QT);
	app.setOrganizationDomain(VER_ORGDOMAIN_STR_QT);

	Q_INIT_RESOURCE(KJVCanOpener);

	QSplashScreen *splash = new QSplashScreen;
	splash->setPixmap(QPixmap(":/res/can-of-KJV.png"));
	splash->show();
	splash->showMessage("<html><body><br /><br /><br /><div align=\"center\"><font size=+1 color=#FFFFFF><b>Please Wait...</b></font></div></body></html>", Qt::AlignCenter);
	qApp->processEvents();

#ifdef Q_WS_WIN
	HANDLE hMutex = CreateMutexW(NULL, false, L"KJVCanOpenerMutex");
	assert(hMutex != NULL);
	// Note: System will automatically close the mutex object when we
	//			exit and InnoSetup actually suggest we leave it open
#endif

	QTime splashTimer;
	splashTimer.start();

	// Setup our Fonts:
	for (int ndxFont = 0; g_constrarrFontFilenames[ndxFont] != NULL; ++ndxFont) {
		QFileInfo fiFont(app.applicationDirPath(), g_constrarrFontFilenames[ndxFont]);
		int nFontStatus = QFontDatabase::addApplicationFont(fiFont.absoluteFilePath());
		if (nFontStatus == -1) {
#ifdef QT_DEBUG
			QMessageBox::warning(splash, g_constrInitialization, QString("Failed to load font file:\n\"%1\"").arg(fiFont.absoluteFilePath()));
#endif
		}
	}

	// Sometimes the splash screen fails to paint, so we'll pump events again
	//	between the fonts and database:
	qApp->processEvents();

	// Setup our SQL Plugin paths:
	QFileInfo fiPlugins(app.applicationDirPath(), g_constrPluginsPath);
	app.addLibraryPath(fiPlugins.absolutePath());

	// Database Paths:
	QFileInfo fiDatabase(app.applicationDirPath(), g_constrDatabaseFilename);
	QFileInfo fiUserDatabaseTemplate(app.applicationDirPath(), g_constrUserDatabaseTemplateFilename);
	QString strDataFolder = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QFileInfo fiUserDatabase(strDataFolder, g_constrUserDatabaseFilename);
	QDir dirDataFolder;
	dirDataFolder.mkpath(strDataFolder);

//	qRegisterMetaTypeStreamOperators<TPhraseTag>("TPhraseTag");


//CBuildDatabase adb(splash);
//adb.BuildDatabase(fiDatabase.absoluteFilePath());
//return 0;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg(argv[ndx]);
		if (strArg.compare("-builddb", Qt::CaseInsensitive) == 0) {
			bBuildDB = true;
		} else if (strArg.compare("-nolimits", Qt::CaseInsensitive) == 0) {
			g_bEnableNoLimits = true;
		} else if ((!strArg.startsWith("-")) && (strKJSFile.isEmpty())) {
			strKJSFile = strArg;
		} else {
			QMessageBox::warning(splash, g_constrInitialization, QString("Unrecognized command-line option \"%1\"").arg(strArg));
		}
	}

	CBuildDatabase bdb(splash);
	if (bBuildDB) {
		if (!bdb.BuildDatabase(fiDatabase.absoluteFilePath())) {
			QMessageBox::warning(splash, g_constrInitialization, "Failed to Build KJV Database!\nAborting...");
			return -1;
		}
	}

	// Read Main Database
	CReadDatabase rdb(splash);
	if (!rdb.ReadDatabase(fiDatabase.absoluteFilePath())) {
		QMessageBox::warning(splash, g_constrInitialization, "Failed to Read and Validate KJV Database!\nCheck Installation!");
		return -2;
	}

	// Read User Database if it exists:
	QString strUserDatabaseFilename;

	if (!fiUserDatabase.exists()) {
		// If the user's database doesn't exist, see if the template one
		//		does.  And if so, see if we can copy from it to the user's:
		if (fiUserDatabaseTemplate.exists()) {
			if (rdb.ReadUserDatabase(fiUserDatabaseTemplate.absoluteFilePath(), true)) {
				if (bdb.BuildUserDatabase(fiUserDatabase.absoluteFilePath(), true)) {
					// Use the copy if that was successful.  Strings will have already
					//	been set, so no need to read again:
					strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
				}
				// If we were successful with reading the template database, we'll
				//	already have the strings loaded, so the user can use them.  But,
				//	we'll leave the pathname empty to disable user changes since
				//	the template will be a read-only copy
			} else {
				// Otherwise, if reading the template failed and there was no user
				//	database, see if we can just create a user database:
				if (bdb.BuildUserDatabase(fiUserDatabase.absoluteFilePath(), true)) {
					strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
				}
			}
		} else {
			// If there was no template and no user database, see if we can just create
			//	the user database.  If so, use it (this is like the failure case above,
			//	but where the template doesn't exist at all):
			if (bdb.BuildUserDatabase(fiUserDatabase.absoluteFilePath(), true)) {
				strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
			}
		}
	} else {
		if (!rdb.ReadUserDatabase(fiUserDatabase.absoluteFilePath())) {
			QMessageBox::warning(splash, g_constrInitialization, "Failed to Read KJV User Database!\nCheck Installation and Verify Database File!");
			return -3;
		} else {
			strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
		}
	}

	// Show splash for minimum time:
	int nElapsed;
	do {
		nElapsed = splashTimer.elapsed();
	} while ((nElapsed>=0) && (nElapsed<g_connMinSplashTimeMS));		// Test the 0 case in case of DST shift so user doesn't have to sit here for an extra hour


	// Setup our default font for our controls:

#ifdef Q_WS_WIN
	QFont fntAppControls = QFont("MS Shell Dlg 2", 8);
#else
	QFont fntAppControls = QFont("DejaVu Sans", 8);
#endif

	QSettings &settings(CPersistentSettings::instance()->settings());

	settings.beginGroup(constrMainAppControlGroup);
	QString strFontName = settings.value(constrFontNameKey, fntAppControls.family()).toString();
	int nFontSize = settings.value(constrFontSizeKey, fntAppControls.pointSize()).toInt();
	settings.endGroup();

	if ((!strFontName.isEmpty()) && (nFontSize>0)) {
		fntAppControls.setFamily(strFontName);
		fntAppControls.setPointSize(nFontSize);
	}

	app.setFont(fntAppControls);

	// Update settings for next time.  Use application font instead of
	//		our variables in case Qt substituted for another available font:
	settings.beginGroup(constrMainAppControlGroup);
	settings.setValue(constrFontNameKey, app.font().family());
	settings.setValue(constrFontSizeKey, app.font().pointSize());
	settings.endGroup();


	// Must have database read above before we create main or else the
	//		data won't be available for the browser objects and such:
	CKJVCanOpener wMain(strUserDatabaseFilename);
	g_pMainWindow = &wMain;
	wMain.setWindowIcon(QIcon(":/res/bible.ico"));
	wMain.show();
	splash->finish(&wMain);
	delete splash;

	wMain.Initialize();

	if (!strKJSFile.isEmpty()) wMain.openKJVSearchFile(strKJSFile);

	int nRetVal = app.exec();

	QFontDatabase::removeAllApplicationFonts();

	return nRetVal;
}

