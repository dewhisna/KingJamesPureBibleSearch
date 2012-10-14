#include <QtGui/QApplication>
#include "KJVCanOpener.h"

#include "BuildDB.h"
#include "ReadDB.h"

#include <assert.h>
#include <QLocale>
#include <QMessageBox>

namespace {
	const char *g_constrInitialization = "KJVCanOpener Initialization";

	const char *g_constrDatabaseFilename = "../KJVCanOpener/db/kjvtext.s3db";

}	// namespace


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CKJVCanOpener wMain;
	wMain.show();

	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

//CBuildDatabase adb(&wMain);
//adb.BuildDatabase(g_constrDatabaseFilename);
//return 0;

	if (argc > 1) {
		if (stricmp(argv[1], "builddb") == 0) {
			CBuildDatabase bdb(&wMain);
			if (!bdb.BuildDatabase(g_constrDatabaseFilename)) {
				QMessageBox::warning(&wMain, g_constrInitialization, "Failed to Build KJV Database!\nAborting...");
				return -1;
			}
		}
	}

	CReadDatabase rdb(&wMain);
	if (!rdb.ReadDatabase(g_constrDatabaseFilename)) {
		QMessageBox::warning(&wMain, g_constrInitialization, "Failed to Read and Validate KJV Database!\nCheck Installation!");
		return -2;
	}

	wMain.Initialize();

	return a.exec();
}
