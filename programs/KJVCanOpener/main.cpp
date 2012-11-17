//#include <QtGui/QApplication>
#include <QtCore>
#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <QWidget>
#include <QMainWindow>
#include <QTime>
//#include <QTimer>
//#include <QThread>
#include <QDesktopWidget>
#include <QPainter>
#include <QLocale>
#include <QMessageBox>
#include <QFileInfo>

#include "KJVCanOpener.h"

#include "BuildDB.h"
#include "ReadDB.h"

#include <assert.h>

namespace {
	const int g_connMinSplashTimeMS = 5000;		// Minimum number of milliseconds to display splash screen

	const char *g_constrInitialization = "KJVCanOpener Initialization";

	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";
	const char *g_constrDatabaseFilename = "../../KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrUserDatabaseFilename = "../../KJVCanOpener/db/kjvuser.s3db";

}	// namespace


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

	Q_INIT_RESOURCE(KJVCanOpener);
//	CKJVCanOpener wMain;

/*
//	QImage splashScrImage("res/can-of-KJV.png");
//	QImage splashScrImage(":/res/can-of-KJV.png");
	QImage splashScrImage("qrc:///res/can-of-KJV.png");
	QSize screenSize = QApplication::desktop()->geometry().size();
	QImage splashScr(screenSize, QImage::Format_ARGB32_Premultiplied);
	QPainter painter(&splashScr);
	painter.fillRect(splashScr.rect(), Qt::black);
	QImage scaled = splashScrImage.scaled(screenSize, Qt::KeepAspectRatio);
	QRect scaledRect = scaled.rect();
	scaledRect.moveCenter(splashScr.rect().center());
	painter.drawImage(scaledRect, scaled);
	QPixmap Logo;
	Logo.convertFromImage(splashScr);
	QSplashScreen splashScrWindow(wMain, Logo, Qt::WindowStaysOnTopHint);
	splashScrWindow.move(0,0);
*/

	QSplashScreen *splash = new QSplashScreen;
	splash->setPixmap(QPixmap(":/res/can-of-KJV.png"));
	splash->show();

	QTime splashTimer;
	splashTimer.start();

	// Setup our SQL Plugin paths:
	QFileInfo fiPlugins(app.applicationDirPath(), g_constrPluginsPath);
	app.addLibraryPath(fiPlugins.absolutePath());

	// Database Paths:
	QFileInfo fiDatabase(app.applicationDirPath(), g_constrDatabaseFilename);
	QFileInfo fiUserDatabase(app.applicationDirPath(), g_constrUserDatabaseFilename);


//CBuildDatabase adb(splash);
//adb.BuildDatabase(g_constrDatabaseFilename);
//return 0;

	if (argc > 1) {
		if (stricmp(argv[1], "builddb") == 0) {
			CBuildDatabase bdb(splash);
			if (!bdb.BuildDatabase(g_constrDatabaseFilename)) {
				QMessageBox::warning(splash, g_constrInitialization, "Failed to Build KJV Database!\nAborting...");
				return -1;
			}
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

	// Must have database read above before we create main or else the
	//		data won't be available for the browser objects and such:
	CKJVCanOpener wMain;
	wMain.show();
	splash->finish(&wMain);
	delete splash;

	wMain.Initialize();

//	splashScrWindow.close();

	return app.exec();
}

