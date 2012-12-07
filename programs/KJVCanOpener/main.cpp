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

	app.setApplicationVersion(VER_QT);
	app.setOrganizationDomain(VER_COMPANYDOMAIN_STR);
	app.setOrganizationName(VER_COMPANYNAME_STR);

	Q_INIT_RESOURCE(KJVCanOpener);

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

//	qRegisterMetaTypeStreamOperators<TPhraseTag>("TPhraseTag");


//CBuildDatabase adb(splash);
//adb.BuildDatabase(fiDatabase.absoluteFilePath());
//return 0;

	if (argc > 1) {
		if (stricmp(argv[1], "builddb") == 0) {
			CBuildDatabase bdb(splash);
			if (!bdb.BuildDatabase(fiDatabase.absoluteFilePath())) {
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
	CKJVCanOpener wMain(fiUserDatabase.absoluteFilePath());
	wMain.show();
	splash->finish(&wMain);
	delete splash;

	wMain.Initialize();

	return app.exec();
}

