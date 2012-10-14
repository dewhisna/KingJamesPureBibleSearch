//#include <QtGui/QApplication>
#include <QtCore>
#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <QWidget>
#include <QMainWindow>
//#include <QTimer>
//#include <QThread>
#include <QDesktopWidget>
#include <QPainter>
#include <QLocale>
#include <QMessageBox>


#include "KJVCanOpener.h"

#include "BuildDB.h"
#include "ReadDB.h"

#include <assert.h>




namespace {
	const char *g_constrInitialization = "KJVCanOpener Initialization";

	const char *g_constrDatabaseFilename = "../KJVCanOpener/db/kjvtext.s3db";

}	// namespace


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

//	Q_INIT_RESOURCE(graphlib);
	CKJVCanOpener *wMain = new CKJVCanOpener();

/*
	QImage splashScrImage("res/can-of-KJV.png");
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

	wMain->show();


/*
	int main(int argc, char* argv[])
	{
	 QApplication app(argc, argv);
	 QMainWindow* viewer = new QMainWindow();
	 QImage splashScrImage ("splash.png");
	 QSize screenSize = QApplication::desktop()->geometry().size();
	 QImage splashScr (screenSize, QImage::Format_ARGB32_Premultiplied);
	 QPainter painter (&splashScr);
	 painter.fillRect(splashScr.rect(), Qt::black);
	 QImage scaled = splashScrImage.scaled(screenSize, Qt::KeepAspectRatio);
	 QRect scaledRect = scaled.rect();
	 scaledRect.moveCenter(splashScr.rect().center());
	 painter.drawImage(scaledRect, scaled);
	 QPixmap Logo;
	 Logo.convertFromImage(splashScr);
	 QSplashScreen splashScrWindow (viewer, Logo, Qt::WindowStaysOnTopHint);
	 splashScrWindow.move(0,0);
	 splashScrWindow.show();
	 QTimer::singleShot(3000, &splashScrWindow ,SLOT(close()));
	 viewer->show();
	 return app.exec();
	}
*/



//CBuildDatabase adb(&wMain);
//adb.BuildDatabase(g_constrDatabaseFilename);
//return 0;

	if (argc > 1) {
		if (stricmp(argv[1], "builddb") == 0) {
			CBuildDatabase bdb(wMain);
			if (!bdb.BuildDatabase(g_constrDatabaseFilename)) {
				QMessageBox::warning(wMain, g_constrInitialization, "Failed to Build KJV Database!\nAborting...");
				return -1;
			}
		}
	}

	CReadDatabase rdb(wMain);
	if (!rdb.ReadDatabase(g_constrDatabaseFilename)) {
		QMessageBox::warning(wMain, g_constrInitialization, "Failed to Read and Validate KJV Database!\nCheck Installation!");
		return -2;
	}

	wMain->Initialize();

//	splashScrWindow.close();

	return app.exec();
}

