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

#include "myApplication.h"
#include "KJVCanOpener.h"
#include "ReportError.h"

#ifdef USING_SINGLEAPPLICATION
#include <singleapplication.h>
#endif

#include <QtCore>
#include <QWidget>
#include <QLocale>
#include <QFileInfo>
#include <QObject>
#include <QTimer>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include "version.h"
#include "PersistentSettings.h"

#include <assert.h>

#ifdef Q_OS_WIN
// Needed to call CreateMutex to lockout installer running while we are:
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const QString g_constrInitialization = QObject::tr("King James Pure Bible Search Initialization");


}	// namespace

// ============================================================================

#ifdef EMSCRIPTEN_NATIVE
extern int emscriptenQtSDLMain(int argc, char *argv[]);
#include <QtGui/emscripten-qt-sdl.h>
void triggerAssert()
{
	Q_ASSERT(false);
}

int main(int argc, char *argv[])
{
	EmscriptenQtSDL::setAttemptedLocalEventLoopCallback(triggerAssert);
	return EmscriptenQtSDL::run(1280, 720, argc, argv);
}

int emscriptenQtSDLMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#ifdef WORKAROUND_QTBUG_32789
	// fix Mac OS X 10.9 (mavericks) font issue
	// https://bugreports.qt-project.org/browse/QTBUG-32789
	// http://successfulsoftware.net/2013/10/23/fixing-qt-4-for-mac-os-x-10-9-mavericks/
	if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8) {
		QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
	}
#endif

	CMyApplication *pApp = new CMyApplication(argc, argv);
	g_pMyApplication = pApp;
	pApp->setApplicationVersion(VER_QT);
	pApp->setApplicationName(VER_APPNAME_STR_QT);
	pApp->setOrganizationName(VER_ORGNAME_STR_QT);
	pApp->setOrganizationDomain(VER_ORGDOMAIN_STR_QT);
#ifdef USING_SINGLEAPPLICATION
	SingleApplication instance(g_constrApplicationID, pApp);
#endif
#ifdef USING_QT_SINGLEAPPLICATION
	QtSingleApplication &instance = *pApp;
#endif

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
	QString strKJSFile;
	bool bBuildDB = false;
	bool bStealthMode = false;
	QString strStealthSettingsFilename;

	Q_INIT_RESOURCE(KJVCanOpener);

#if defined(USING_SINGLEAPPLICATION) || defined(USING_QT_SINGLEAPPLICATION)

	// Hook receiving messages from other KJPBS app launches:
	pApp->connect(&instance, SIGNAL(messageReceived(const QString &)), pApp, SLOT(receivedKJPBSMessage(const QString &)));

#endif

#ifdef Q_OS_WIN32
	pApp->setWindowIcon(QIcon(":/res/bible.ico"));
#else
#ifndef Q_OS_MAC			// Normally, this would also include Mac, but Mac has its icon set in the .pro file.  Loading this one makes it fuzzy.
	pApp->setWindowIcon(QIcon(":/res/bible_48.png"));
#endif
#endif

	QWidget *pSplash = pApp->showSplash();

#ifdef Q_OS_WIN
	HANDLE hMutex = CreateMutexW(NULL, false, L"KJVCanOpenerMutex");
	assert(hMutex != NULL);
	// Note: System will automatically close the mutex object when we
	//			exit and InnoSetup actually suggest we leave it open
#endif

	// Parse the Commmand-line:
	if (strKJSFile.isEmpty() && !pApp->fileToLoad().isEmpty()) strKJSFile = pApp->fileToLoad();

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
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Unexpected command-line filename \"%1\"").arg(strArg));
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
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Unrecognized command-line option \"%1\"").arg(strArg));
			}
		} else {
			if (bLookingForSettings) {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Settings Filename, but received: \"%1\" instead").arg(strArg));
				bLookingForSettings = false;
			}
		}
	}

	pApp->setFileToLoad(strKJSFile);

#ifdef EMSCRIPTEN
	bStealthMode = true;
#endif

#if defined(USING_SINGLEAPPLICATION) || defined(USING_QT_SINGLEAPPLICATION)

	// Check for existing KJPBS and have it handle this launch request:
	if (instance.isRunning()) {
		if (bBuildDB) {
			displayWarning(pSplash, g_constrInitialization, QObject::tr("Can't Build Database while app is already running!"));
			delete pApp;
			return -2;
		}

		pApp->completeInterAppSplash();

		QString strMessage;
		if (!strKJSFile.isEmpty()) {
			QFileInfo fiKJSFile(strKJSFile);
			strMessage = pApp->createKJPBSMessage(CMyApplication::KAMCE_ACTIVATE_EXISTING_OPEN_KJS, QStringList(QString("KJS=%1").arg(fiKJSFile.absoluteFilePath())));
		} else{
			strMessage = pApp->createKJPBSMessage(CMyApplication::KAMCE_NEW_CANOPENER, QStringList());
		}

		if (instance.sendMessage(strMessage, 5000)) {
			delete pApp;
			return 0;
		} else {
			displayWarning(pSplash, g_constrInitialization, QObject::tr("There appears to be another copy of King James Pure Bible Search running, but it is not responding. "
																		 "Please check the running copy to see if it's functioning and revive it and/or reboot."));
			delete pApp;
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

	int nRetVal = 0;

#ifndef EMSCRIPTEN
	bool bDone = false;

	if (pSplash == NULL) {
		// If we don't have a splash screen already, we will terminate unless we
		//		go ahead and launch our app:
		nRetVal = pApp->execute(bBuildDB);
	} else {
		if (!bBuildDB) {
			QTimer::singleShot(100, pApp, SLOT(execute()));
		} else {
			nRetVal = pApp->execute(bBuildDB);
		}
	}

	if (nRetVal == 0) {
		while (!bDone) {
			nRetVal = pApp->exec();
			if ((nRetVal != 0) || (!pApp->areRestarting())) {
				bDone = true;
			} else {
				pApp->createKJVCanOpener(g_pMainBibleDatabase);
			}
		}
	}

	delete pApp;
#else
	if (pSplash != NULL) {
		nRetVal = pApp->exec();
		QTimer::singleShot(2000, pApp, SLOT(execute()));
	} else {
		nRetVal = pApp->execute(bBuildDB);
		if (nRetVal == 0) pApp->exec();
	}

#ifdef EMSCRIPTEN_NATIVE
	while (true) {
		pApp->processEvents();
	}
#endif

#endif

	return nRetVal;
}

