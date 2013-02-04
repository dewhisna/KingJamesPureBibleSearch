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

#include "KJVCanOpener.h"

#include "version.h"
#include "dbstruct.h"
#include "BuildDB.h"
#include "ReadDB.h"

#include <assert.h>

#ifdef Q_WS_WIN
// Needed to call CreateMutex to lockout installer running while we are:
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

QWidget *g_pMainWindow = NULL;

namespace {
	const int g_connMinSplashTimeMS = 5000;		// Minimum number of milliseconds to display splash screen

	const char *g_constrInitialization = "King James Pure Bible Search Initialization";

	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";
	const char *g_constrDatabaseFilename = "../../KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrUserDatabaseFilename = "../../KJVCanOpener/db/kjvuser.s3db";

}	// namespace


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

	// Setup our SQL Plugin paths:
	QFileInfo fiPlugins(app.applicationDirPath(), g_constrPluginsPath);
	app.addLibraryPath(fiPlugins.absolutePath());

	// Database Paths:
	QFileInfo fiDatabase(app.applicationDirPath(), g_constrDatabaseFilename);
	QFileInfo fiUserDatabase(app.applicationDirPath(), g_constrUserDatabaseFilename);

//	qRegisterMetaTypeStreamOperators<TPhraseTag>("TPhraseTag");


//CBuildDatabase adb(splash);
//adb.BuildDatabase(fiDatabase.absoluteFilePath());
//return 0;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg(argv[ndx]);
		if (strArg.compare("/builddb", Qt::CaseInsensitive) == 0) {
			bBuildDB = true;
		} else if (strArg.compare("/nolimits", Qt::CaseInsensitive) == 0) {
			g_bEnableNoLimits = true;
		} else if ((!strArg.startsWith("/")) && (strKJSFile.isEmpty())) {
			strKJSFile = strArg;
		} else {
			QMessageBox::warning(splash, g_constrInitialization, QString("Unrecognized command-line option \"%1\"").arg(strArg));
		}
	}

	if (bBuildDB) {
		CBuildDatabase bdb(splash);
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
	if (fiUserDatabase.exists()) {
		if (!rdb.ReadUserDatabase(fiUserDatabase.absoluteFilePath())) {
			QMessageBox::warning(splash, g_constrInitialization, "Failed to Read KJV User Database!\nCheck Installation and Verify Database File!");
			return -3;
		}
	}

	// Show splash for minimum time:
	int nElapsed;
	do {
		nElapsed = splashTimer.elapsed();
	} while ((nElapsed>=0) && (nElapsed<g_connMinSplashTimeMS));		// Test the 0 case in case of DST shift so user doesn't have to sit here for an extra hour

	app.setFont(QFont("Sans", 8));

	// Must have database read above before we create main or else the
	//		data won't be available for the browser objects and such:
	CKJVCanOpener wMain(fiUserDatabase.absoluteFilePath());
	g_pMainWindow = &wMain;
	wMain.setWindowIcon(QIcon(":/res/bible.ico"));
	wMain.show();
	splash->finish(&wMain);
	delete splash;

	wMain.Initialize();

	if (!strKJSFile.isEmpty()) wMain.openKJVSearchFile(strKJSFile);

	return app.exec();
}

