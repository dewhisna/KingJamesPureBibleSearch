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
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QTimer>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif
//#include <QtPlugin>

#include "version.h"
#include "PersistentSettings.h"

#include <assert.h>

#include <iostream>

#ifdef  USING_WEBCHANNEL
#include <webChannelServer.h>
#endif

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

	const QString g_constrInitialization = QObject::tr("King James Pure Bible Search Initialization", "Errors");

	//////////////////////////////////////////////////////////////////////

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

#ifdef WORKAROUND_QTBUG_35313_35687
	// Workaround the dark background/contrast android dialogs on some devices by switching
	//		to native dialogs:
	qputenv("QT_USE_ANDROID_NATIVE_DIALOGS", "0");
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

//	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
	QString strKJSFile;
	bool bShowHelp = false;
	bool bBuildDB = false;
	bool bStealthMode = false;
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
#ifdef INVERT_MULTITHREADED_LOGIC
	bool bSingleThreadedSearchResults = true;
#else
	bool bSingleThreadedSearchResults = false;
#endif
#else
	bool bSingleThreadedSearchResults = true;
#endif
	QString strStealthSettingsFilename;
	QString strTTSServerURL;
	QString strWebChannelHostPort;

	Q_INIT_RESOURCE(KJVCanOpener);

#if defined(USING_SINGLEAPPLICATION) || defined(USING_QT_SINGLEAPPLICATION)

	// Hook receiving messages from other KJPBS app launches:
	pApp->connect(&instance, SIGNAL(messageReceived(const QString &)), pApp, SLOT(receivedKJPBSMessage(const QString &)));

#endif

#ifndef IS_CONSOLE_APP

#ifdef Q_OS_WIN32
	pApp->setWindowIcon(QIcon(":/res/bible.ico"));
#else
#ifndef Q_OS_MAC			// Normally, this would also include Mac, but Mac has its icon set in the .pro file.  Loading this one makes it fuzzy.
	pApp->setWindowIcon(QIcon(":/res/bible_48.png"));
#endif
#endif

#endif

#if defined(VNCSERVER) && defined(USING_WEBCHANNEL)
#error "Can't use WebChannel with VNC!"
#endif

	QWidget *pSplash = pApp->showSplash();

#ifdef Q_OS_WIN
	HANDLE hMutex = CreateMutexW(NULL, false, L"KJVCanOpenerMutex");
	assert(hMutex != NULL);
	// Note: System will automatically close the mutex object when we
	//			exit and InnoSetup actually suggest we leave it open
#endif

#if defined(VNCSERVER) || defined(USING_WEBCHANNEL)
	CMyDaemon *pSIGDaemon = new CMyDaemon(pApp);
	pSIGDaemon->setup_unix_signal_handlers();
#endif

	// Parse the Commmand-line:
	if (strKJSFile.isEmpty() && !pApp->fileToLoad().isEmpty()) strKJSFile = pApp->fileToLoad();

	bool bLookingForSettings = false;
	bool bLookingForBibleDB = false;
	bool bLookingForDictDB = false;
	bool bLookingForTTSServerURL = false;
	bool bLookingForWebChannelHostPort = false;
	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg(argv[ndx]);
		if (!strArg.startsWith("-")) {
			if (bLookingForSettings) {
				strStealthSettingsFilename = strArg;
				bLookingForSettings = false;
			} else if (bLookingForBibleDB) {
				bLookingForBibleDB = false;
				if (strArg.toUInt() < bibleDescriptorCount()) {
					pApp->setSelectedMainBibleDB(static_cast<BIBLE_DESCRIPTOR_ENUM>(strArg.toUInt()));
				} else {
					displayWarning(pSplash, g_constrInitialization, QObject::tr("Unrecognized Bible Database Index \"%1\"", "Errors").arg(strArg));
				}
			} else if (bLookingForDictDB) {
				bLookingForDictDB = false;
				if (strArg.toUInt() < dictionaryDescriptorCount()) {
					pApp->setSelectedMainDictDB(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(strArg.toUInt()));
				} else {
					displayWarning(pSplash, g_constrInitialization, QObject::tr("Unrecognized Dictionary Database Index \"%1\"", "Errors").arg(strArg));
				}
			} else if (bLookingForTTSServerURL) {
				strTTSServerURL = strArg;
				bLookingForTTSServerURL = false;
			} else if (bLookingForWebChannelHostPort) {
				strWebChannelHostPort = strArg;
				bLookingForWebChannelHostPort = false;
			} else if (strKJSFile.isEmpty()) {
				strKJSFile = strArg;
			} else {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Unexpected command-line filename \"%1\"", "Errors").arg(strArg));
			}
		} else if ((!bLookingForSettings) &&
					(!bLookingForBibleDB) &&
					(!bLookingForDictDB) &&
					(!bLookingForTTSServerURL) &&
					(!bLookingForWebChannelHostPort)) {
			if ((strArg.compare("-h", Qt::CaseInsensitive) == 0) ||
				(strArg.compare("--help", Qt::CaseInsensitive) == 0)) {
				bShowHelp = true;
			} else if (strArg.compare("-builddb", Qt::CaseInsensitive) == 0) {
				bBuildDB = true;
			} else if (strArg.compare("-bbl", Qt::CaseInsensitive) == 0) {
				bLookingForBibleDB = true;
			} else if (strArg.compare("-dct", Qt::CaseInsensitive) == 0) {
				bLookingForDictDB = true;
			} else if (strArg.compare("-stealth", Qt::CaseInsensitive) == 0) {
				bStealthMode = true;
			} else if (strArg.compare("-settings", Qt::CaseInsensitive) == 0) {
				bStealthMode = true;
				bLookingForSettings = true;
			} else if ((strArg.compare("-singlethreaded", Qt::CaseInsensitive) == 0) ||
						(strArg.compare("-st", Qt::CaseInsensitive) == 0)) {
				bSingleThreadedSearchResults = true;
			} else if ((strArg.compare("-multithreaded", Qt::CaseInsensitive) == 0) ||
						(strArg.compare("-mt", Qt::CaseInsensitive) == 0)) {
				bSingleThreadedSearchResults = false;
			} else if (strArg.compare("-TTSServer", Qt::CaseInsensitive) == 0) {
				bLookingForTTSServerURL = true;
			} else if (strArg.compare("-webchannel", Qt::CaseInsensitive) == 0) {
				bLookingForWebChannelHostPort = true;
			} else {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Unrecognized command-line option \"%1\"", "Errors").arg(strArg));
			}
		} else {
			if (bLookingForSettings) {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Settings Filename, but received: \"%1\" instead", "Errors").arg(strArg));
				bLookingForSettings = false;
			}
			if (bLookingForBibleDB) {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Bible Descriptor Index, but received: \"%1\" instead", "Errors").arg(strArg));
				bLookingForBibleDB = false;
			}
			if (bLookingForDictDB) {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Dictionary Descriptor Index, but received: \"%1\" instead", "Errors").arg(strArg));
				bLookingForDictDB = false;
			}
			if (bLookingForTTSServerURL) {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Text-To-Speech Server URL, but received: \"%1\" instead", "Errors").arg(strArg));
				bLookingForTTSServerURL = false;
			}
			if (bLookingForWebChannelHostPort) {
				displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting WebChannel Host/Port, but but received: \"%1\" instead", "Errors").arg(strArg));
				bLookingForWebChannelHostPort = false;
			}
		}
	}

	bool bBadArgs = false;
	if (bLookingForSettings) {
		displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Settings Filename, but none was specified.", "Errors"));
		bBadArgs = true;
	}
	if (bLookingForBibleDB) {
		displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Bible Descriptor Index, but none was specified.", "Errors"));
		bBadArgs = true;
	}
	if (bLookingForDictDB) {
		displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Dictionary Descriptor Index, but none was specified.", "Errors"));
		bBadArgs = true;
	}
	if (bLookingForTTSServerURL) {
		displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting Text-To-Speech Server URL, but none was specified.", "Errors"));
		bBadArgs = true;
	}
	if (bLookingForWebChannelHostPort) {
		displayWarning(pSplash, g_constrInitialization, QObject::tr("Was expecting WebChannel Host/Port, but none was specified.", "Errors"));
		bBadArgs = true;
	}
	if (bBadArgs) {
		delete pApp;
		return -3;
	}

	if (bShowHelp) {
		std::cout << "King James Pure Bible Search\n";
		std::cout << "Usage information:\n\n";
		std::cout << QString("%1 [options] [<KJSFile>]\n\n").arg(QFileInfo(QApplication::applicationFilePath()).fileName()).toUtf8().data();
		std::cout << "Where:\n";
		std::cout << "    [<KJSFile>] = Optional King James Search file to load\n\n";
		std::cout << "Options\n";
		std::cout << "-h, --help   = Show this usage information\n\n";
		std::cout << "-builddb     = Build Bible Database (Requires /data from KJVDataParse)\n\n";
		std::cout << "-bbl <index> = Bible Database Index to use\n";
		std::cout << "               (for building or initial search window)\n";
		for (unsigned int dbNdx = 0; dbNdx < bibleDescriptorCount(); ++dbNdx) {
			std::cout << QString("    %1 : %2\n").arg(dbNdx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx)).m_strDBDesc).toUtf8().data();
		}
		std::cout << "\n";
		std::cout << "-dct <indx>  = Dictionary Database Index to use\n";
		std::cout << "               (for initial search window)\n";
		for (unsigned int dbNdx = 0; dbNdx < dictionaryDescriptorCount(); ++dbNdx) {
			std::cout << QString("    %1 : %2\n").arg(dbNdx).arg(dictionaryDescriptor(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(dbNdx)).m_strDBDesc).toUtf8().data();
		}
		std::cout << "\n";
		std::cout << "-stealth     = Don't write settings to system registry, ~/.config,\n";
		std::cout << "               or ~/Library.  Configuration changes will not be saved\n";
		std::cout << "               in stealth mode unless you use -settings also, see below.\n\n";
		std::cout << "-settings <file> = Write settings to the specified filepath.  This option\n";
		std::cout << "               implies -stealth option above.\n\n";
		std::cout << "-st, -singlethreaded = Use single-threaded search result generation";
#if (defined(USE_MULTITHREADED_SEARCH_RESULTS) && defined(INVERT_MULTITHREADED_LOGIC)) || !defined(USE_MULTITHREADED_SEARCH_RESULTS)
		std::cout << " (default)\n\n";
#else
		std::cout << "\n\n";
#endif
		std::cout << "-mt, -multithreaded = Use multi-threaded search result generation";
#if (defined(USE_MULTITHREADED_SEARCH_RESULTS) && !defined(INVERT_MULTITHREADED_LOGIC))
		std::cout << " (default)\n\n";
#else
		std::cout << "\n\n";
#endif
		std::cout << "-TTSServer <URL> = Use Speech Server at specified URL\n";
		std::cout << "               (only applies to builds using text-to-speech server)\n\n";
		std::cout << "-webchannel <port[,interface]> = Starts WebChannel Server Daemon\n";
		std::cout << "               Where port is the desired port to listen on\n";
		std::cout << "               Where interface is one of:\n";
		std::cout << "                   127.0.0.1 = localhost IPv4\n";
		std::cout << "                   ::1 = localhost IPv6\n";
		std::cout << "                   255.255.255.255 = broadcast IPv4 (not advised)\n";
		std::cout << "                   0.0.0.0 = Any IPv4 interface\n";
		std::cout << "                   :: = Any IPv6 interface\n";
		std::cout << "                   Dual stack, Any IPv4 or IPv6 if not specified\n";
		std::cout << "    examples:\n";
		std::cout << "        -webchannel 12345,127.0.0.1  (listen localhost, port 12345)\n";
		std::cout << "        -webchannel 12345            (listen Any IPv4/IPv6, port 12345)\n";
		std::cout << "        -webchannel 12345,0.0.0.0    (listen Any IPv4, port 12345)\n\n";
		std::cout << "    (Send SIGHUP or SIGTERM to shutdown server)\n";
		std::cout << "\n";
		delete pApp;
		return 0;
	}

	pApp->setFileToLoad(strKJSFile);
	pApp->setTTSServerURL(strTTSServerURL);
	pApp->setWebChannelHostPort(strWebChannelHostPort);

#if defined(EMSCRIPTEN) || defined(VNCSERVER)
	bStealthMode = true;
#endif

#if defined(USING_WEBCHANNEL)
	if (!strWebChannelHostPort.isEmpty()) {
		bStealthMode = true;
	}
#endif

#if defined(USING_SINGLEAPPLICATION) || defined(USING_QT_SINGLEAPPLICATION)

	// Check for existing KJPBS and have it handle this launch request:
	if (instance.isRunning()) {
		if (bBuildDB) {
			displayWarning(pSplash, g_constrInitialization, QObject::tr("Can't Build Database while app is already running!", "Errors"));
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
																		 "Please check the running copy to see if it's functioning and revive it and/or reboot.", "Errors"));
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

	pApp->setSingleThreadedSearchResults(bSingleThreadedSearchResults);

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

#ifdef IS_CONSOLE_APP
#ifdef USING_WEBCHANNEL
	if ((pApp->webChannelServer() == NULL) ||
		(!pApp->webChannelServer()->isListening())) {
		std::cerr << "error: Failed to start WebChannel server listening.  Exiting...\n";
		bDone = true;
		delete pApp->webChannelServer();		// Get rid of server object so we don't do shutdown log message, since we never started
	}
#else
	bDone = true;
#endif
#endif

	if (nRetVal == 0) {
		while (!bDone) {
			nRetVal = pApp->exec();
#if !defined(IS_CONSOLE_APP) || !defined(USING_WEBCHANNEL)
			if ((nRetVal != 0) || (!pApp->areRestarting())) {
				bDone = true;
			} else {
				pApp->createKJVCanOpener(TBibleDatabaseList::instance()->mainBibleDatabase());
			}
#else
			// On console only webchannel apps (i.e. daemon server), don't perform "restart"
			bDone = true;
#endif
		}
	}

#ifdef USING_WEBCHANNEL
	// Close web server before we shutdown and tearout Bible Databases, etc,
	//		if we don't do this, we will crash with a segfault cleaning up the
	//		CWebChannelSearchResults after the fact:
	CWebChannelServer *pWebChannelServer = pApp->webChannelServer();
	if (pWebChannelServer) {
		pWebChannelServer->close();
	}
#endif

	delete pApp;
#else
#ifdef USING_QT_SPEECH
#error "Can't use Qt Speech with Emscripten!"
#endif
#ifdef USING_WEBCHANNEL
#error "Can't use WebChannel with Emscripten!"
#endif
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

