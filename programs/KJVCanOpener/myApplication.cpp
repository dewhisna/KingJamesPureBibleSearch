/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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
#include "BusyCursor.h"

#if !defined(QT_NO_WARNING_OUTPUT)
#include <QDebug>
#endif

#if defined(VNCSERVER) || defined(USING_WEBCHANNEL)
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#if defined(USING_WEBCHANNEL) && defined(USING_MMDB)
#include <mmdblookup.h>
#endif

#ifdef USING_SINGLEAPPLICATION
#include <singleapplication.h>
#endif

#ifdef USING_QT_SPEECH
#include <QUrl>
#endif

#ifdef SHOW_SPLASH_SCREEN
#include <QPixmap>
#include <QSplashScreen>
#include <QElapsedTimer>
#endif

#ifdef IS_CONSOLE_APP
#include <QDateTime>
#endif

#include <QProxyStyle>
#include <QFont>
#include <QFontDatabase>
#if QT_VERSION < 0x050000
#include <QDesktopWidget>
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
#include "Translator.h"

#ifdef USING_QT_WEBENGINE
#include "ScriptureWebEngine.h"
#endif

#ifdef BUILD_KJV_DATABASE
#include "BuildDB.h"
#endif
#include "ReadDB.h"

#ifdef IS_CONSOLE_APP
#include <iostream>
#endif

#include "PathConsts.h"

// ============================================================================

QPointer<CMyApplication> g_pMyApplication = nullptr;
QPointer<QMdiArea> g_pMdiArea = nullptr;

#ifdef USING_QT_SPEECH
QPointer<QtSpeech> CMyApplication::m_pSpeech = nullptr;
#endif

#ifdef USING_WEBCHANNEL
QPointer<CWebChannelServer> CMyApplication::m_pWebChannelServer = nullptr;
#endif

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
	const QString constrLanguageKey("Language");

	// Main Bible Database Settings:
	const QString constrMainAppBibleDatabaseGroup("MainApp/BibleDatabase");
	const QString constrDatabaseUUIDKey("UUID");

	// Main Dictionary Database Settings:
	const QString constrMainAppDictDatabaseGroup("MainApp/DictionaryDatabase");
	//const QString constrDatabaseUUIDKey("UUID");

	// Bible Database Settings:
	const QString constrBibleDatabaseSettingsGroup("BibleDatabaseSettings");
	//const QString constrDatabaseUUIDKey("UUID");
	const QString constrLoadOnStartKey("LoadOnStart");
	const QString constrHideHyphensKey("HideHyphens");
	const QString constrHyphenSensitiveKey("HyphenSensitive");

	// Dictionary Database Settings:
	const QString constrDictDatabaseSettingsGroup("DictionaryDatabaseSettings");
	//const QString constrDatabaseUUIDKey("UUID");									// Entries in the dictionary settings signify selecting it for the corresponding language
	//const QString constrLoadOnStartKey("LoadOnStart");

	// Text-To-Speech Settings:
	const QString constrTTSSettingsGroup("TextToSpeech");
	const QString constrTTSServerURLKey("TTSServerURL");
	const QString constrTTSSelectedVoiceIDKey("TTSSelectedVoiceID");

	//////////////////////////////////////////////////////////////////////

#ifdef SHOW_SPLASH_SCREEN
	const int g_connMinSplashTimeMS = 5000;			// Minimum number of milliseconds to display splash screen
	const int g_connInterAppSplashTimeMS = 2000;	// Splash Time for Inter-Application communications
#endif

	const QString g_constrInitialization = QObject::tr("King James Pure Bible Search Initialization", "Errors");

	//////////////////////////////////////////////////////////////////////

#ifdef LOAD_APPLICATION_FONTS

#if defined(EMSCRIPTEN)
	// --------------------------------------------------------------------------------------------------------- EMSCRIPTEN ---------------------
	// Note: Emscripten uses auto-loading of .qpf fonts from deployed qt-fonts folder,
	//	except for WebAssembly (WASM), which has them embedded as resources instead.
	#ifdef EMSCRIPTEN_NATIVE
		const char *g_constrDejaVuSans_BoldOblique = "./data/DejaVuSans-BoldOblique.ttf";
		const char *g_constrDejaVuSans_Bold = "./data/DejaVuSans-Bold.ttf";
		const char *g_constrDejaVuSansMono_BoldOblique = "./data/DejaVuSansMono-BoldOblique.ttf";
		const char *g_constrDejaVuSansMono_Bold = "./data/DejaVuSansMono-Bold.ttf";
		const char *g_constrDejaVuSansMono_Oblique = "./data/DejaVuSansMono-Oblique.ttf";
		const char *g_constrDejaVuSansMono = "./data/DejaVuSansMono.ttf";
		const char *g_constrDejaVuSans_Oblique = "./data/DejaVuSans-Oblique.ttf";
		const char *g_constrDejaVuSans = "./data/DejaVuSans.ttf";
		const char *g_constrDejaVuSerif_BoldItalic = "./data/DejaVuSerif-BoldItalic.ttf";
		const char *g_constrDejaVuSerif_Bold = "./data/DejaVuSerif-Bold.ttf";
		const char *g_constrDejaVuSerif_Italic = "./data/DejaVuSerif-Italic.ttf";
		const char *g_constrDejaVuSerif = "./data/DejaVuSerif.ttf";
	#elif defined(Q_OS_WASM)
		const char *g_constrDejaVuSans_BoldOblique = ":/fonts/DejaVuSans-BoldOblique.ttf";
		const char *g_constrDejaVuSans_Bold = ":/fonts/DejaVuSans-Bold.ttf";
		const char *g_constrDejaVuSansMono_BoldOblique = ":/fonts/DejaVuSansMono-BoldOblique.ttf";
		const char *g_constrDejaVuSansMono_Bold = ":/fonts/DejaVuSansMono-Bold.ttf";
		const char *g_constrDejaVuSansMono_Oblique = ":/fonts/DejaVuSansMono-Oblique.ttf";
		const char *g_constrDejaVuSansMono = ":/fonts/DejaVuSansMono.ttf";
		const char *g_constrDejaVuSans_Oblique = ":/fonts/DejaVuSans-Oblique.ttf";
		const char *g_constrDejaVuSans = ":/fonts/DejaVuSans.ttf";
		const char *g_constrDejaVuSerif_BoldItalic = ":/fonts/DejaVuSerif-BoldItalic.ttf";
		const char *g_constrDejaVuSerif_Bold = ":/fonts/DejaVuSerif-Bold.ttf";
		const char *g_constrDejaVuSerif_Italic = ":/fonts/DejaVuSerif-Italic.ttf";
		const char *g_constrDejaVuSerif = ":/fonts/DejaVuSerif.ttf";
	#else
		const char *g_constrDejaVuSans_BoldOblique = "data/DejaVuSans-BoldOblique.ttf";
		const char *g_constrDejaVuSans_Bold = "data/DejaVuSans-Bold.ttf";
		const char *g_constrDejaVuSansMono_BoldOblique = "data/DejaVuSansMono-BoldOblique.ttf";
		const char *g_constrDejaVuSansMono_Bold = "data/DejaVuSansMono-Bold.ttf";
		const char *g_constrDejaVuSansMono_Oblique = "data/DejaVuSansMono-Oblique.ttf";
		const char *g_constrDejaVuSansMono = "data/DejaVuSansMono.ttf";
		const char *g_constrDejaVuSans_Oblique = "data/DejaVuSans-Oblique.ttf";
		const char *g_constrDejaVuSans = "data/DejaVuSans.ttf";
		const char *g_constrDejaVuSerif_BoldItalic = "data/DejaVuSerif-BoldItalic.ttf";
		const char *g_constrDejaVuSerif_Bold = "data/DejaVuSerif-Bold.ttf";
		const char *g_constrDejaVuSerif_Italic = "data/DejaVuSerif-Italic.ttf";
		const char *g_constrDejaVuSerif = "data/DejaVuSerif.ttf";
	#endif
#else
	const char *g_constrScriptBLFontFilename = "fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "fonts/DejaVuSerif.ttf";
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
		nullptr
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
		g_constrDejaVuSerif_BoldItalic,
		g_constrDejaVuSerif_Bold,
		g_constrDejaVuSerif_Italic,
		g_constrDejaVuSerif,
		nullptr
	};
#endif		// EMSCRIPTEN

#endif		// WORKAROUND_QTBUG_34490

#endif		//	LOAD_APPLICATION_FONTS

}	// namespace

// ============================================================================

#if defined(VNCSERVER) || defined(USING_WEBCHANNEL)

int CMyDaemon::m_sighupFd[2] = { 0, 0 };
int CMyDaemon::m_sigtermFd[2] = { 0, 0 };
int CMyDaemon::m_sigusr1Fd[2] = { 0, 0 };

CMyDaemon::CMyDaemon(CMyApplication *pMyApplication)
	:	QObject(pMyApplication),
		m_psnHup(nullptr),
		m_psnTerm(nullptr),
		m_psnUsr1(nullptr),
		m_pMyApplication(pMyApplication)
{
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sighupFd))
		qFatal("Couldn't create SIGHUP socketpair");

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigtermFd))
		qFatal("Couldn't create SIGTERM socketpair");

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigusr1Fd))
		qFatal("Couldn't create SIGUSR1 socketpair");

	m_psnHup = new QSocketNotifier(m_sighupFd[1], QSocketNotifier::Read, this);
	connect(m_psnHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
	m_psnTerm = new QSocketNotifier(m_sigtermFd[1], QSocketNotifier::Read, this);
	connect(m_psnTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
	m_psnUsr1 = new QSocketNotifier(m_sigusr1Fd[1], QSocketNotifier::Read, this);
	connect(m_psnUsr1, SIGNAL(activated(int)), this, SLOT(handleSigUsr1()));
}

CMyDaemon::~CMyDaemon()
{

}

int CMyDaemon::setup_unix_signal_handlers()
{
	struct sigaction hup, term, usr1;

	hup.sa_handler = CMyDaemon::hupSignalHandler;
	sigemptyset(&hup.sa_mask);
	hup.sa_flags = 0;
	hup.sa_flags |= SA_RESTART;

	if (sigaction(SIGHUP, &hup, nullptr) > 0)
		return 1;

	term.sa_handler = CMyDaemon::termSignalHandler;
	sigemptyset(&term.sa_mask);
	term.sa_flags |= SA_RESTART;

	if (sigaction(SIGTERM, &term, nullptr) > 0)
		return 2;

	usr1.sa_handler = CMyDaemon::usr1SignalHandler;
	sigemptyset(&usr1.sa_mask);
	usr1.sa_flags |= SA_RESTART;

	if (sigaction(SIGUSR1, &usr1, nullptr) >0)
		return 3;

	return 0;
}

void CMyDaemon::hupSignalHandler(int)
{
	char a = 1;
	ssize_t szWrite = ::write(m_sighupFd[0], &a, sizeof(a));
	Q_ASSERT(szWrite == sizeof(a));
}

void CMyDaemon::termSignalHandler(int)
{
	char a = 1;
	ssize_t szWrite = ::write(m_sigtermFd[0], &a, sizeof(a));
	Q_ASSERT(szWrite == sizeof(a));
}

void CMyDaemon::usr1SignalHandler(int)
{
	char a = 1;
	ssize_t szWrite = ::write(m_sigusr1Fd[0], &a, sizeof(a));
	Q_ASSERT(szWrite == sizeof(a));
}

void CMyDaemon::handleSigHup()
{
	m_psnHup->setEnabled(false);
	char tmp;
	ssize_t szRead = ::read(m_sighupFd[1], &tmp, sizeof(tmp));
	Q_ASSERT(szRead == sizeof(tmp));

	// do Qt stuff
	if (!m_pMyApplication.isNull()) {
#if !defined(IS_CONSOLE_APP) || !defined(USING_WEBCHANNEL)
		m_pMyApplication->closeAllWindows();
#else
		m_pMyApplication->exit(0);
#endif
	}

	m_psnHup->setEnabled(true);
}

void CMyDaemon::handleSigTerm()
{
	m_psnTerm->setEnabled(false);
	char tmp;
	ssize_t szRead = ::read(m_sigtermFd[1], &tmp, sizeof(tmp));
	Q_ASSERT(szRead == sizeof(tmp));

	// do Qt stuff
	if (!m_pMyApplication.isNull()) {
#if !defined(IS_CONSOLE_APP) || !defined(USING_WEBCHANNEL)
		m_pMyApplication->closeAllWindows();
#else
		m_pMyApplication->exit(0);
#endif
	}

	m_psnTerm->setEnabled(true);
}

void CMyDaemon::handleSigUsr1()
{
	m_psnUsr1->setEnabled(false);
	char tmp;
	ssize_t szRead = ::read(m_sigusr1Fd[1], &tmp, sizeof(tmp));
	Q_ASSERT(szRead == sizeof(tmp));

#if defined(VNCSERVER)
	// do Qt stuff
	QWidget *pParent = nullptr;
	if (!m_pMyApplication.isNull()) {
		pParent = m_pMyApplication->activeCanOpener();
	}
	displayWarning(pParent, tr("King James Pure Bible Search", "Errors"), tr("Warning: Your VNC King James Pure Bible Search Session expires in 5 minutes.", "Errors"));
#endif

	m_psnUsr1->setEnabled(true);
}

#endif	// VNCSERVER

// ============================================================================

class MyProxyStyle : public QProxyStyle
{
public:
	int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
				const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const
	{
		if (hint == QStyle::SH_ItemView_ActivateItemOnSingleClick) return 0;

		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

// ============================================================================

CMyApplication::CMyApplication(int & argc, char ** argv)
#if defined(USING_QT_SINGLEAPPLICATION)
	:	QtSingleApplication(g_constrApplicationID, argc, argv),
#elif defined(IS_CONSOLE_APP)
	:	QCoreApplication(argc, argv),
#else
	:	QApplication(argc, argv),
#endif
#ifdef USE_MULTITHREADED_SEARCH_RESULTS
#ifdef INVERT_MULTITHREADED_LOGIC
		m_bSingleThreadedSearchResults(true),
#else
		m_bSingleThreadedSearchResults(false),
#endif
#else
		m_bSingleThreadedSearchResults(true),
#endif
		m_nLastActivateCanOpener(-1),
		m_bUsingCustomStyleSheet(false),
		m_bAreRestarting(false),
		m_pSplash(nullptr)
{
#ifndef IS_CONSOLE_APP
	m_strStartupStyleSheet = styleSheet();
#endif

	// Setup our SQL/Image and Platform Plugin paths.  Ideally, this would be
	//	done in main() before instantiating the object in order to make the
	//	Platform plugins to work correctly on Qt 5, however, the
	//	QCoreApplication::applicationDirPath() can't be called until after the
	//	QApplication object has been instantiated.  So, we'll just have to put
	//	the Platform plugins in the app folder:
#if !defined(Q_OS_ANDROID) && !defined(EMSCRIPTEN)
	QFileInfo fiPlugins(initialAppDirPath(), g_constrPluginsPath);
	QCoreApplication::addLibraryPath(fiPlugins.absolutePath());
#endif

#ifndef IS_CONSOLE_APP
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
#endif

	g_strTranslationsPath = QFileInfo(initialAppDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);
}

CMyApplication::~CMyApplication()
{
	if (m_pSplash != nullptr) {
		delete m_pSplash;
		m_pSplash = nullptr;
	}

	// We must clean up our databases and things before exiting or else
	//		the destructor tear-down order might cause us to crash, particularly
	//		with SQL Database things:
	TBibleDatabaseList::instance()->clear();
	TDictionaryDatabaseList::instance()->clear();
	g_pUserNotesDatabase.clear();

#ifdef USING_QT_SPEECH
	if (QtSpeech::serverSupported()) QtSpeech::disconnectFromServer();
#endif

#ifdef USING_WEBCHANNEL
	if (!m_pWebChannelServer.isNull()) delete m_pWebChannelServer.data();
#endif

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
	if (m_pSplash != nullptr) {
#ifdef SHOW_SPLASH_SCREEN
		Q_ASSERT(m_splashTimer.isValid());
		do {
			processEvents();
		} while (!m_splashTimer.hasExpired(g_connInterAppSplashTimeMS));
#endif
	}
}

void CMyApplication::setSplashMessage(const QString &strMessage)
{
#ifdef SHOW_SPLASH_SCREEN
	if (m_pSplash != nullptr) {
		m_pSplash->clearMessage();
		const QString strOffsetSpace = "";
		QString strSpecialVersion(SPECIAL_BUILD ? QString(VER_SPECIALVERSION_STR) : QString());
		if (!strSpecialVersion.isEmpty()) strSpecialVersion = "<br>\n" + strOffsetSpace + strSpecialVersion;
		QString strStatus;
		if (!strMessage.isEmpty()) strStatus += "<br>\n" + strOffsetSpace + strMessage;
		m_pSplash->showMessage(QString("<html><body><table height=375 width=500><tr><td>&nbsp;</td></tr></table><div align=\"center\"><font size=+1 color=#FFFFFF><b>") +
										strOffsetSpace + tr("Please Wait...", "Errors") +
										strSpecialVersion +
										strStatus +
										QString("</b></font></div></body></html>"), Qt::AlignBottom | Qt::AlignLeft);
		m_pSplash->repaint();
		processEvents();
	}
#elif defined(IS_CONSOLE_APP)
	std::cout << QString("%1 UTC : ").arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8().data();
	std::cout << strMessage.toUtf8().data();
	std::cout << "\n";
	std::cout.flush();
#else
	Q_UNUSED(strMessage);
#endif
}

// ============================================================================

void CMyApplication::saveApplicationFontSettings()
{
#ifndef IS_CONSOLE_APP
	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrMainAppControlGroup);
		settings.setValue(constrFontNameKey, font().family());
		settings.setValue(constrFontSizeKey, font().pointSize());
		settings.endGroup();
	}
#endif	// IS_CONSOLE_APP
}

void CMyApplication::restoreApplicationFontSettings()
{
#ifndef IS_CONSOLE_APP

	// Setup our default font for our controls:
#ifdef Q_OS_WIN32
	QFont fntAppControls = QFont("DejaVu Sans", 8);
#elif defined(Q_OS_MAC)
//	QFont fntAppControls = QFont("Arial", 12);
	QFont fntAppControls = QFont("Lucida Grande", 12);
#elif defined(EMSCRIPTEN)
	QFont fntAppControls = QFont("DejaVu Sans", 12);
#elif defined(VNCSERVER)
	QFont fntAppControls = QFont("DejaVu Sans", 12);
#else
	QFont fntAppControls = QFont("DejaVu Sans", 8);
#endif

	if (CPersistentSettings::instance()->settings() != nullptr) {
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

#endif	// IS_CONSOLE_APP
}

void CMyApplication::setupTextBrightnessStyleHooks()
{
	// Setup Default TextBrightness:
	en_setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(en_setTextBrightness(bool, int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(en_setAdjustDialogElementBrightness(bool)));
}

// ============================================================================

void CMyApplication::saveApplicationLanguage()
{
	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrMainAppControlGroup);
		settings.setValue(constrLanguageKey, CPersistentSettings::instance()->applicationLanguage());
		settings.endGroup();
	}
}

void CMyApplication::restoreApplicationLanguage()
{
	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrMainAppControlGroup);
		CPersistentSettings::instance()->setApplicationLanguage(settings.value(constrLanguageKey, CPersistentSettings::instance()->applicationLanguage()).toString());
		settings.endGroup();
	}
	CTranslatorList::instance()->setApplicationLanguage(CPersistentSettings::instance()->applicationLanguage());
}

// ============================================================================

void CMyApplication::saveTTSSettings()
{
	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrTTSSettingsGroup);
		if (!CPersistentSettings::instance()->ttsServerURL().isEmpty()) {
			settings.setValue(constrTTSServerURLKey, CPersistentSettings::instance()->ttsServerURL());
		} else {
			settings.remove(constrTTSServerURLKey);
		}
		if (!CPersistentSettings::instance()->ttsSelectedVoiceID().isEmpty()) {
			settings.setValue(constrTTSSelectedVoiceIDKey, CPersistentSettings::instance()->ttsSelectedVoiceID());
		} else {
			settings.remove(constrTTSSelectedVoiceIDKey);
		}
		settings.endGroup();
	}
}

void CMyApplication::restoreTTSSettings()
{
	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrTTSSettingsGroup);
		CPersistentSettings::instance()->setTTSServerURL(settings.value(constrTTSServerURLKey, CPersistentSettings::instance()->ttsServerURL()).toString());
		CPersistentSettings::instance()->setTTSSelectedVoiceID(settings.value(constrTTSSelectedVoiceIDKey, CPersistentSettings::instance()->ttsSelectedVoiceID()).toString());
		settings.endGroup();
	}
}

// ============================================================================

bool CMyApplication::notify(QObject *pReceiver, QEvent *pEvent)
{
#ifdef EMSCRIPTEN
	// For some reason, Emscripten causes us to receive Action Changed notifications on
	//		a NULL Receiver object.  Haven't found exactly where these are originating,
	//		but they seem fairly beneign.  So just defaulting them here so we don't get
	//		a recurrent debug log notification message:
	if (pReceiver == nullptr) return true;
#endif

#if !defined(NOT_USING_EXCEPTIONS) && defined(QT_DEBUG)
	try {
#endif
#if defined(USING_QT_SINGLEAPPLICATION)
		return QtSingleApplication::notify(pReceiver, pEvent);
#elif defined(IS_CONSOLE_APP)
		return QCoreApplication::notify(pReceiver, pEvent);
#else
		return QApplication::notify(pReceiver, pEvent);
#endif
#if !defined(NOT_USING_EXCEPTIONS) && defined(QT_DEBUG)
	} catch (const std::exception &ex) {
		qDebug("std::exception was caught: %s", ex.what());
	} catch (...) {
		qDebug("Unknown exception was caught");
		Q_ASSERT(false);
	}

	return false;
#endif
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
#if defined(USING_QT_SINGLEAPPLICATION)
	return QtSingleApplication::event(event);
#elif defined(IS_CONSOLE_APP)
	return QCoreApplication::event(event);
#else
	return QApplication::event(event);
#endif
}

#ifdef SIGNAL_SPY_DEBUG
Q4puGenericSignalSpy *CMyApplication::createSpy(QObject *pOwner, QObject *pSpyOn)
{
	Q_ASSERT(!g_pMyApplication.isNull());
	Q4puGenericSignalSpy *pSpy = new Q4puGenericSignalSpy((pOwner != nullptr) ? pOwner : g_pMyApplication);

	QObject::connect(pSpy, SIGNAL(caughtSignal(const QString&)), g_pMyApplication, SLOT(signalSpyCaughtSignal(const QString &)));
	QObject::connect(pSpy, SIGNAL(caughtSlot(const QString&)), g_pMyApplication, SLOT(signalSpyCaughtSlot(const QString &)));

	// If we are given an object to spy on, attach to it.  If not, but were given
	//		an owner, attach to it.  If not, don't attach to anything...
	if ((pSpyOn != nullptr) || (pOwner != nullptr)) {
		pSpy->spyOn((pSpyOn != nullptr) ? pSpyOn : pOwner);
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

#ifdef USING_QT_SPEECH

void CMyApplication::en_clearingSpeechQueue()
{
	// Note: This function may get called multiple times during a queue clear because the
	//		QtSpeech::clearQueue can get called multiple times because of multiple widgets
	//		trying to handle the speechStop button...
	Q_ASSERT(!m_pSpeech.isNull());
	if (connect(m_pSpeech.data(), SIGNAL(finished(bool)), this, SLOT(en_speechFinished(bool)), Qt::UniqueConnection)) {
#ifndef IS_CONSOLE_APP
		setOverrideCursor(Qt::WaitCursor);
#endif
	}
}

void CMyApplication::en_speechFinished(bool bQueueEmpty)
{
	if (bQueueEmpty) {
		Q_ASSERT(!m_pSpeech.isNull());
		disconnect(m_pSpeech.data(), SIGNAL(finished(bool)), this, SLOT(en_speechFinished(bool)));
#ifndef IS_CONSOLE_APP
		restoreOverrideCursor();
#endif
	}
}

#endif	// USING_QT_SPEECH

// ============================================================================

CKJVCanOpener *CMyApplication::createKJVCanOpener(CBibleDatabasePtr pBibleDatabase)
{
	m_bAreRestarting = false;			// Once we create a new CanOpener we are no longer restarting...
	CKJVCanOpener *pCanOpener = new CKJVCanOpener(pBibleDatabase);
	m_lstKJVCanOpeners.append(pCanOpener);
	connect(pCanOpener, SIGNAL(isClosing(CKJVCanOpener*)), this, SLOT(removeKJVCanOpener(CKJVCanOpener*)));
	connect(pCanOpener, SIGNAL(windowActivated(CKJVCanOpener*)), this, SLOT(activatedKJVCanOpener(CKJVCanOpener*)));
	connect(pCanOpener, SIGNAL(canCloseChanged(CKJVCanOpener*, bool)), this, SLOT(en_canCloseChanged(CKJVCanOpener*, bool)));
	//	Do this via a QueuedConnection so that KJVCanOpeners coming/going during opening other search windows
	//	won't crash if the menu that was triggering it gets yanked out from under it:
	connect(pCanOpener, SIGNAL(triggerUpdateSearchWindowList()), this, SIGNAL(updateSearchWindowList()), Qt::QueuedConnection);

	if (!g_pMdiArea.isNull()) {
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
	emit updateSearchWindowList();
	return pCanOpener;
}

bool CMyApplication::isFirstCanOpener(bool bInCanOpenerConstructor, const QString &strBblUUID) const
{
	return (bInCanOpenerConstructor ? (bibleDatabaseCanOpenerRefCount(strBblUUID) == 0) : (bibleDatabaseCanOpenerRefCount(strBblUUID) <= 1));
}

bool CMyApplication::isLastCanOpener(const QString &strBblUUID) const
{
	return (bibleDatabaseCanOpenerRefCount(strBblUUID) <= 1);
}

int CMyApplication::bibleDatabaseCanOpenerRefCount(const QString &strBblUUID) const
{
	if (strBblUUID.isEmpty()) {
		return m_lstKJVCanOpeners.size();
	} else {
		int nCount = 0;
		for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
			if (m_lstKJVCanOpeners.at(ndx) == nullptr) continue;
			CBibleDatabasePtr pBibleDatabase = m_lstKJVCanOpeners.at(ndx)->bibleDatabase();
			if (pBibleDatabase.isNull()) continue;
			if (pBibleDatabase->compatibilityUUID().compare(strBblUUID, Qt::CaseInsensitive) == 0) ++nCount;
		}
		return nCount;
	}
}

int CMyApplication::dictDatabaseCanOpenerRefCount(const QString &strDctUUID) const
{
	if (strDctUUID.isEmpty()) {
		return m_lstKJVCanOpeners.size();
	} else {
		int nCount = 0;
		for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
			if (m_lstKJVCanOpeners.at(ndx) == nullptr) continue;
			CDictionaryDatabasePtr pDictDatabase = m_lstKJVCanOpeners.at(ndx)->dictionaryDatabase();
			if (pDictDatabase.isNull()) continue;
			if (pDictDatabase->compatibilityUUID().compare(strDctUUID, Qt::CaseInsensitive) == 0) ++nCount;
		}
		return nCount;
	}
}

void CMyApplication::removeKJVCanOpener(CKJVCanOpener *pKJVCanOpener)
{
	int ndxCanOpener = m_lstKJVCanOpeners.indexOf(pKJVCanOpener);
	Q_ASSERT(ndxCanOpener != -1);
	if (ndxCanOpener == m_nLastActivateCanOpener) m_nLastActivateCanOpener = -1;
	if (ndxCanOpener != -1) m_lstKJVCanOpeners.removeAt(ndxCanOpener);
	if (!g_pMdiArea.isNull()) {
		if (m_lstKJVCanOpeners.size() == 0) {
			if (!areRestarting()) g_pMdiArea->deleteLater();
		} else {
			QList<QMdiSubWindow *> lstSubWindows = g_pMdiArea->subWindowList();
			for (int ndxSubWindows = 0; ndxSubWindows < lstSubWindows.size(); ++ndxSubWindows) {
				if (lstSubWindows.at(ndxSubWindows)->widget() == nullptr) {
					lstSubWindows.at(ndxSubWindows)->close();
					break;
				}
			}
		}
	}
	emit updateSearchWindowList();
}

void CMyApplication::activatedKJVCanOpener(CKJVCanOpener *pCanOpener)
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		if (m_lstKJVCanOpeners.at(ndx) == pCanOpener) {
			m_nLastActivateCanOpener = ndx;
			return;
		}
	}

#ifndef IS_CONSOLE_APP

	// The following is needed on Mac to make sure the menu of the
	//      new KJVCanOpen gets set:
	if (activeWindow() != static_cast<QWidget *>(pCanOpener))
			setActiveWindow(pCanOpener);

#endif

	Q_ASSERT(false);
	m_nLastActivateCanOpener = -1;
}

CKJVCanOpener *CMyApplication::activeCanOpener() const
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		if (m_lstKJVCanOpeners.at(ndx)->isActiveWindow()) return m_lstKJVCanOpeners.at(ndx);
	}
	return nullptr;
}

template<class T>
CKJVCanOpener *CMyApplication::findCanOpenerFromChild(const T *pChild) const
{
	Q_ASSERT(pChild != nullptr);
	for (int ndxCanOpener = 0; ndxCanOpener < m_lstKJVCanOpeners.size(); ++ndxCanOpener) {
		QList<T *>lstFoundChildren = m_lstKJVCanOpeners.at(ndxCanOpener)->findChildren<T *>(pChild->objectName());
		for (int ndxChild = 0; ndxChild < lstFoundChildren.size(); ++ndxChild) {
			if (lstFoundChildren.at(ndxChild) == pChild) return m_lstKJVCanOpeners.at(ndxCanOpener);
		}
	}
	return nullptr;
}

class CSearchResultsTreeView;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<CSearchResultsTreeView>(const CSearchResultsTreeView *) const;

class i_CScriptureBrowser;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<i_CScriptureBrowser>(const i_CScriptureBrowser *) const;

class i_CScriptureEdit;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<i_CScriptureEdit>(const i_CScriptureEdit *) const;

#ifdef USING_QT_WEBENGINE
class CScriptureWebEngineView;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<CScriptureWebEngineView>(const CScriptureWebEngineView *) const;
#endif

void CMyApplication::activateCanOpener(CKJVCanOpener *pCanOpener) const
{
	Q_ASSERT(pCanOpener != nullptr);
	if (!g_pMdiArea.isNull()) {
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
	Q_ASSERT((ndx >= 0) && (ndx < m_lstKJVCanOpeners.size()));
	if ((ndx < 0) || (ndx >= m_lstKJVCanOpeners.size())) return;

	activateCanOpener(m_lstKJVCanOpeners.at(ndx));
}

void CMyApplication::activateAllCanOpeners() const
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		activateCanOpener(ndx);
	}
}

void CMyApplication::closeAllCanOpeners(CKJVCanOpener *pActiveCanOpener)
{
	Q_ASSERT(canQuit());
	if (!canQuit()) return;

	int nLastCanOpener = 0;

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();
	if (pActiveCanOpener) pBibleDatabase = pActiveCanOpener->bibleDatabase();

	if (m_bAreRestarting) {
		// Hold the last instance until we create the new one, because if we delete it, we exit.
		//	This is particularly important on WebAssembly since we can't let
		//	the main exec() exit due to operating in asynchronous mode.  On
		//	desktop/vnc builds, we can exit and the loop in main() will check to
		//	see if we are restarting and recreate the main CanOpener.  However,
		//	that loop can't easily start back on the same database as the
		//	currently active CanOpener.  So we'll let both paths follow the
		//	same course here and try to recreate the main CanOpener here with
		//	the database of the current CanOpener and let the main() just be
		//	a fallback on desktop/vnc builds.  The fallback is needed for
		//	older Qt's that don't support functor calls in singleShot
		//	(see CKJVCanOpener::en_Configure):
		nLastCanOpener = 1;
	}

	// Close in reverse order:
	for (int ndx = (m_lstKJVCanOpeners.size()-1); ndx >= nLastCanOpener; --ndx) {
		QTimer::singleShot(0, m_lstKJVCanOpeners.at(ndx), SLOT(close()));
	}
	// Note: List update will happen automatically as the windows close...

	if (m_bAreRestarting) {
		createKJVCanOpener(pBibleDatabase);
		QTimer::singleShot(0, m_lstKJVCanOpeners.at(0), SLOT(close()));
	}

}

void CMyApplication::restartApp(CKJVCanOpener *pCallingCanOpener)
{
	m_bAreRestarting = true;
	closeAllCanOpeners(pCallingCanOpener);
}

void CMyApplication::en_triggeredKJVCanOpener(QAction *pAction)
{
	Q_ASSERT(pAction != nullptr);
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

#ifndef IS_CONSOLE_APP

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
							  "QTreeView { background-color:%1; color:%2; }\n"						// Bible Database List
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

#else
	Q_UNUSED(bInvert);
	Q_UNUSED(nBrightness);
#endif	// IS_CONSOLE_APP

}

void CMyApplication::en_setAdjustDialogElementBrightness(bool bAdjust)
{
	Q_UNUSED(bAdjust);
	en_setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
}

// ============================================================================

void CMyApplication::en_notesFileAutoSaveTriggered()
{
	if (g_pUserNotesDatabase.isNull()) return;			// Shouldn't happen, but just in case
	if ((g_pUserNotesDatabase->isDirty()) && (!g_pUserNotesDatabase->filePathName().isEmpty())) {
		CBusyCursor iAmBusy(nullptr);
		if (!g_pUserNotesDatabase->save()) m_dlyNotesFilesAutoSave.trigger();		// If save failed, retrigger to try again
	}
}

void CMyApplication::en_changedUserNotesDatabase()
{
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	if ((CPersistentSettings::instance()->notesFileAutoSaveTime() > 0) && (!m_dlyNotesFilesAutoSave.isTriggered())) {
		if (g_pUserNotesDatabase->isDirty()) m_dlyNotesFilesAutoSave.trigger();
	} else if (!g_pUserNotesDatabase->isDirty()) {
		// If the file has been saved already, untrigger:
		m_dlyNotesFilesAutoSave.untrigger();
	}
}

void CMyApplication::en_changedNotesFileAutoSaveTime(int nAutoSaveTime)
{
	m_dlyNotesFilesAutoSave.setMinimumDelay(nAutoSaveTime*60000);		// Convert minutes->milliseconds
	if (nAutoSaveTime > 0) {
		if (m_dlyNotesFilesAutoSave.isTriggered()) m_dlyNotesFilesAutoSave.trigger();		// Retrigger to extend time if our setting changed
	} else {
		m_dlyNotesFilesAutoSave.untrigger();		// If disabling, stop any existing triggers
	}
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

	QStringList lstMsg = strMessage.split(";", My_QString_KeepEmptyParts);
	Q_ASSERT(lstMsg.size() >= 1);
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
				Q_ASSERT(false);
				return;
			}
			activateCanOpener(nIndex);
			break;
		}
		case KAMCE_ACTIVATE_EXISTING_OPEN_KJS:
		case KAMCE_NEW_CANOPENER:
		case KAMCE_NEW_CANOPENER_OPEN_KJS:
		{
			bool bNeedDB = false;
			if ((strBibleUUID.isEmpty()) &&  (!strKJSFileName.isEmpty())) {
				// If no UUID was specified and we have a KJS file to open, try to determine the correct
				//		Bible Database from the KJS file itself:
				strBibleUUID = CKJVCanOpener::determineBibleUUIDForKJVSearchFile(strKJSFileName);
				if (!strBibleUUID.isEmpty()) {
					if (TBibleDatabaseList::instance()->atUUID(strBibleUUID).isNull()) bNeedDB = true;
				}
			}
			bool bForceOpen = ((nCommand == KAMCE_NEW_CANOPENER_OPEN_KJS) || (nCommand == KAMCE_NEW_CANOPENER));
			if ((m_lstKJVCanOpeners.size() == 1) && (m_lstKJVCanOpeners.at(0)->bibleDatabase()->compatibilityUUID().compare(strBibleUUID, Qt::CaseInsensitive) != 0)) bForceOpen = true;
			CKJVCanOpener *pCanOpener = nullptr;
			if ((bForceOpen) || (bNeedDB) || (m_lstKJVCanOpeners.size() != 1)) {
				// If we have more than one, just open a new window and launch the file:
				CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strBibleUUID);
				if ((pBibleDatabase.isNull()) && (!strBibleUUID.isEmpty())) {
					if (TBibleDatabaseList::loadBibleDatabase(strBibleUUID, true, nullptr)) {
						pBibleDatabase = TBibleDatabaseList::instance()->atUUID(strBibleUUID);
					}
				}
				if (pBibleDatabase.isNull()) pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();
				setFileToLoad(strKJSFileName);
				pCanOpener = createKJVCanOpener(pBibleDatabase);
				Q_ASSERT(pCanOpener != nullptr);
			} else {
				pCanOpener = m_lstKJVCanOpeners.at(0);
				if (!strKJSFileName.isEmpty()) pCanOpener->openKJVSearchFile(strKJSFileName);
			}
			activateCanOpener(pCanOpener);
			break;
		}
		default:
			break;
	}
}

// ============================================================================

int CMyApplication::execute(bool bBuildDB)
{
	// Restore Locale Language Setting (and save for next time):
	restoreApplicationLanguage();
	saveApplicationLanguage();

	// Restore Text-To-Speech Settings (and save for next time):
	restoreTTSSettings();
	saveTTSSettings();

	// Setup our Fonts:
#ifdef LOAD_APPLICATION_FONTS
	//	Note: As of Qt 5.2, iOS doesn't currently load fonts correctly and causes:
	//			This plugin does not support application fonts
	//			This plugin does not support propagateSizeHints()
	//	See QTBUG-34490:	https://bugreports.qt-project.org/browse/QTBUG-34490
	//	Temporary workaround is to add these to the Info.plist so iOS will
	//		auto-load them for us:
#ifndef WORKAROUND_QTBUG_34490
	for (int ndxFont = 0; g_constrarrFontFilenames[ndxFont] != nullptr; ++ndxFont) {
#ifndef EMSCRIPTEN
		QString strFontFileName = QFileInfo(initialAppDirPath(), g_constrarrFontFilenames[ndxFont]).absoluteFilePath();
#else
		QString strFontFileName = g_constrarrFontFilenames[ndxFont];
#endif
		int nFontStatus = QFontDatabase::addApplicationFont(strFontFileName);
		if (nFontStatus == -1) {
#ifdef QT_DEBUG
			displayWarning(m_pSplash, g_constrInitialization, tr("Failed to load font file:\n\"%1\"", "Errors").arg(strFontFileName));
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

	qRegisterMetaType<TBibleDatabaseSettings>("TBibleDatabaseSettings");			// Needed to do queued connection of the CPersistentSettings::changedBibleDatabaseSettings()
	qRegisterMetaType<CBibleDatabasePtr>("CBibleDatabasePtr");						// Needed to do queued connection on WebChannel threads
	qRegisterMetaType<CDictionaryDatabasePtr>("CDictionaryDatabasePtr");			// Needed to do queued connection on WebChannel threads
	qRegisterMetaType<CUserNotesDatabasePtr>("CUserNotesDatabasePtr");				// Needed to do queued connection on WebChannel threads
	qRegisterMetaType<uint32_t>("uint32_t");										// Needed to do queued connection on WebChannel threads

	// Setup Text-To-Speech:
#ifdef USING_QT_SPEECH
	if (QtSpeech::serverSupported()) {
		QString strTTSServer = m_strTTSServerURL;
		if (strTTSServer.isEmpty()) strTTSServer = CPersistentSettings::instance()->ttsServerURL();
		if (!strTTSServer.isEmpty()) {
			QUrl urlTTSServer(strTTSServer);
			QString strTTSHost = urlTTSServer.host();
			if (urlTTSServer.scheme().compare(QTSPEECH_SERVER_SCHEME_NAME, Qt::CaseInsensitive) != 0) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Unknown Text-To-Speech Server Scheme name.\nExpected \"%1\".", "Errors").arg(QTSPEECH_SERVER_SCHEME_NAME));
			} else if ((!strTTSHost.isEmpty()) &&
						(!QtSpeech::connectToServer(strTTSHost, urlTTSServer.port(QTSPEECH_DEFAULT_SERVER_PORT)))) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Failed to connect to Text-To-Speech Server!\n\n\"%1\"", "Errors").arg(strTTSServer));
			}
		}
	} else {
		// If user specified a TTS Server on the command-line and this build doesn't support server mode, warn him:
		if (!m_strTTSServerURL.isEmpty()) {
			displayWarning(m_pSplash, g_constrInitialization, tr("Text-To-Speech Server was specified, but this build of King James Pure Bible Search doesn't support external servers", "Errors"));
		}
	}

	if (m_pSpeech.isNull()) {
		QtSpeech::TVoiceName vnSelectedVoice;
		vnSelectedVoice.id = CPersistentSettings::instance()->ttsSelectedVoiceID();
		m_pSpeech = new QtSpeech(vnSelectedVoice, this);
		connect(m_pSpeech.data(), SIGNAL(clearingQueue()), this, SLOT(en_clearingSpeechQueue()));
		vnSelectedVoice = m_pSpeech->voiceName();		// Get resolved name in case it wasn't set or was invalid
		CPersistentSettings::instance()->setTTSSelectedVoiceID(vnSelectedVoice.id);
		saveTTSSettings();								// And save new default name in case it changed
	}
#else
	// If user specified a TTS Server on the command-line and this build doesn't support TTS, warn him:
	if (!m_strTTSServerURL.isEmpty()) {
		displayWarning(m_pSplash, g_constrInitialization, tr("Text-To-Speech Server was specified, but this build of King James Pure Bible Search doesn't support Text-To-Speech", "Errors"));
	}
#endif

	// Setup Bible Databases:
	QString strMainBibleDatabaseUUID;
	QString strMainDictDatabaseUUID;

	if (CPersistentSettings::instance()->settings() != nullptr) {
		QSettings &settings(*CPersistentSettings::instance()->settings());

		// Main Bible Database:
		settings.beginGroup(constrMainAppBibleDatabaseGroup);
		strMainBibleDatabaseUUID = settings.value(constrDatabaseUUIDKey, CPersistentSettings::instance()->mainBibleDatabaseUUID()).toString();
		if (!strMainBibleDatabaseUUID.isEmpty()) {
			CPersistentSettings::instance()->setMainBibleDatabaseUUID(strMainBibleDatabaseUUID);
		}
		settings.endGroup();

		// Bible Database Settings:
		int nBDBSettings = settings.beginReadArray(constrBibleDatabaseSettingsGroup);
		if (nBDBSettings != 0) {
			for (int ndx = 0; ndx < nBDBSettings; ++ndx) {
				settings.setArrayIndex(ndx);
				QString strUUID = settings.value(constrDatabaseUUIDKey, QString()).toString();
				if (!strUUID.isEmpty()) {
					TBibleDatabaseSettings bdbSettings = CPersistentSettings::instance()->bibleDatabaseSettings(strUUID);
					bdbSettings.setLoadOnStart(settings.value(constrLoadOnStartKey, bdbSettings.loadOnStart()).toBool());
					bdbSettings.setHideHyphens(settings.value(constrHideHyphensKey, bdbSettings.hideHyphens()).toUInt());
					bdbSettings.setHyphenSensitive(settings.value(constrHyphenSensitiveKey, bdbSettings.hyphenSensitive()).toBool());
					CPersistentSettings::instance()->setBibleDatabaseSettings(strUUID, bdbSettings);
				}
			}
		}
		settings.endArray();

		// Main Dictionary Database:
		settings.beginGroup(constrMainAppDictDatabaseGroup);
		strMainDictDatabaseUUID = settings.value(constrDatabaseUUIDKey, CPersistentSettings::instance()->mainDictDatabaseUUID()).toString();
		if (!strMainDictDatabaseUUID.isEmpty()) {
			CPersistentSettings::instance()->setMainDictDatabaseUUID(strMainDictDatabaseUUID);
		}
		settings.endGroup();

		// Dictionary Database Settings:
		int nDDBSettings = settings.beginReadArray(constrDictDatabaseSettingsGroup);
		if (nDDBSettings != 0) {
			for (int ndx = 0; ndx < nDDBSettings; ++ndx) {
				settings.setArrayIndex(ndx);
				QString strUUID = settings.value(constrDatabaseUUIDKey, QString()).toString();
				if (!strUUID.isEmpty()) {
					TDictionaryDatabaseSettings ddbSettings = CPersistentSettings::instance()->dictionaryDatabaseSettings(strUUID);
					ddbSettings.setLoadOnStart(settings.value(constrLoadOnStartKey, ddbSettings.loadOnStart()).toBool());
					CPersistentSettings::instance()->setDictionaryDatabaseSettings(strUUID, ddbSettings);
				}
			}
		}
		settings.endArray();
	}

	if (m_strSelectedMainBibleDB.isEmpty()) {
		// If command-line override wasn't specified, first see if we will be loading a KJS file.
		//		If so, try and determine it's Bible Database:
		if (!fileToLoad().isEmpty()) {
			m_strSelectedMainBibleDB = CKJVCanOpener::determineBibleUUIDForKJVSearchFile(fileToLoad());
		}
		// Else, see if a persistent settings was previously set:
		if (m_strSelectedMainBibleDB.isEmpty()) {
			m_strSelectedMainBibleDB = strMainBibleDatabaseUUID;
		}
		if (m_strSelectedMainBibleDB.isEmpty()) m_strSelectedMainBibleDB = bibleDescriptor(BDE_KJV).m_strUUID;			// Default to KJV unless we're told otherwise
	}

	if (m_strSelectedMainDictDB.isEmpty()) {
		// If command-line override for dictionary wasn't specified, see if a persistent setting was previously set:
		m_strSelectedMainDictDB = strMainDictDatabaseUUID;
		if (m_strSelectedMainDictDB.isEmpty()) m_strSelectedMainDictDB = dictionaryDescriptor(DDE_WEB1828).m_strUUID;	// Default to WEB1828 unless we're told otherwise
	}

	// Read (and/or Build) our Databases:
	{
#ifdef BUILD_KJV_DATABASE
		CBuildDatabase bdb(m_pSplash);
		if (bBuildDB) {
			// Database Paths for building:
#ifdef NOT_USING_SQL
			// If we can't support SQL, we can't:
			QString strKJVSQLDatabasePath;
#else
			QString strKJVSQLDatabasePath = QFileInfo(TBibleDatabaseList::bibleDatabasePath(),
					m_bblDescSelectedForBuild.isValid() ?
						m_bblDescSelectedForBuild.m_strS3DBFilename :
						TBibleDatabaseList::availableBibleDatabaseDescriptor(m_strSelectedMainBibleDB).m_strS3DBFilename
					).absoluteFilePath();
#endif
			QString strKJVCCDatabasePath = QFileInfo(TBibleDatabaseList::bibleDatabasePath(),
					m_bblDescSelectedForBuild.isValid() ?
						m_bblDescSelectedForBuild.m_strCCDBFilename :
						TBibleDatabaseList::availableBibleDatabaseDescriptor(m_strSelectedMainBibleDB).m_strCCDBFilename
					).absoluteFilePath();

			if (!bdb.BuildDatabase(strKJVSQLDatabasePath, strKJVCCDatabasePath)) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Failed to Build Bible Database!\nAborting...", "Errors"));
				return -2;
			}
		}
#else
		if (bBuildDB) {
			displayWarning(m_pSplash, g_constrInitialization, tr("Database building isn't supported on this platform/build...", "Errors"));
			return -2;
		}
#endif

		// Read Main Database(s)
		const QList<TBibleDescriptor> &lstAvailableBBLDescs = TBibleDatabaseList::availableBibleDatabases();
		for (int ndx = 0; ndx < lstAvailableBBLDescs.size(); ++ndx) {
			const TBibleDescriptor &bblDesc = lstAvailableBBLDescs.at(ndx);
			if ((!(bblDesc.m_btoFlags & BTO_AutoLoad)) &&
				(m_strSelectedMainBibleDB.compare(bblDesc.m_strUUID, Qt::CaseInsensitive) != 0) &&
				(!CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID).loadOnStart())) continue;
			CReadDatabase rdbMain(m_pSplash);
			Q_ASSERT(rdbMain.haveBibleDatabaseFiles(bblDesc));
			setSplashMessage(tr("Reading:", "Errors") + QString(" %1 ").arg(bblDesc.m_strDBName) + tr("Bible", "Errors"));
			if (!rdbMain.ReadBibleDatabase(bblDesc, (m_strSelectedMainBibleDB.compare(bblDesc.m_strUUID, Qt::CaseInsensitive) == 0))) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Failed to Read and Validate Bible Database!\n%1\nCheck Installation!", "Errors").arg(bblDesc.m_strDBDesc));
				return -3;
			}
		}
		// If the specified database wasn't found, see if we loaded any database and if so, make the
		//		first one loaded (from our priority list) the main database:
		if ((TBibleDatabaseList::instance()->mainBibleDatabase().isNull()) &&
			(TBibleDatabaseList::instance()->size() > 0)) {
			TBibleDatabaseList::instance()->setMainBibleDatabase(TBibleDatabaseList::instance()->at(0)->compatibilityUUID());
		}
		if (TBibleDatabaseList::instance()->mainBibleDatabase().isNull()) {
			displayWarning(m_pSplash, g_constrInitialization, tr("Failed to find and load a Bible Database!  Check Installation!", "Errors"));
			return -3;
		}

#if defined(USING_DICTIONARIES)
		// Read Dictionary Database:
		const QList<TDictionaryDescriptor> &lstAvailableDictDescs = TDictionaryDatabaseList::availableDictionaryDatabases();
		for (int ndx = 0; ndx < lstAvailableDictDescs.size(); ++ndx) {
			const TDictionaryDescriptor &dctDesc = lstAvailableDictDescs.at(ndx);
			if ((!(dctDesc.m_dtoFlags & DTO_AutoLoad)) &&
				(m_strSelectedMainDictDB.compare(lstAvailableDictDescs.at(ndx).m_strUUID, Qt::CaseInsensitive) != 0) &&
				(!CPersistentSettings::instance()->dictionaryDatabaseSettings(dctDesc.m_strUUID).loadOnStart())) continue;
			bool bHaveLanguageMatch = false;
			for (int nBBLNdx = 0; nBBLNdx < lstAvailableBBLDescs.size(); ++nBBLNdx) {
				if (lstAvailableBBLDescs.at(nBBLNdx).m_strLanguage.compare(dctDesc.m_strLanguage, Qt::CaseInsensitive) == 0) {
					bHaveLanguageMatch = true;
					break;
				}
			}
			if (!bHaveLanguageMatch) continue;			// No need loading the dictionary for a language we don't have a Bible database for
			CReadDatabase rdbDict(m_pSplash);
			Q_ASSERT(rdbDict.haveDictionaryDatabaseFiles(dctDesc));
			setSplashMessage(tr("Reading:", "Errors") + QString(" %1 ").arg(dctDesc.m_strDBName) + tr("Dictionary", "Errors"));
			if (!rdbDict.ReadDictionaryDatabase(dctDesc, true, (m_strSelectedMainDictDB.compare(lstAvailableDictDescs.at(ndx).m_strUUID, Qt::CaseInsensitive) == 0))) {
				displayWarning(m_pSplash, g_constrInitialization, tr("Failed to Read and Validate Dictionary Database!\n%1\nCheck Installation!", "Errors").arg(dctDesc.m_strDBDesc));
				return -5;
			}
		}
		// If the specified database wasn't found, see if we loaded any database and if so, make the
		//		first one loaded matching our main Bible Database language the main dictionary:
		for (int nDctNdx = 0; ((nDctNdx < TDictionaryDatabaseList::instance()->size()) &&
							   (TDictionaryDatabaseList::instance()->mainDictionaryDatabase().isNull())); ++nDctNdx) {
			if ((!TDictionaryDatabaseList::instance()->at(nDctNdx).isNull()) &&			// Note: Main Bible Database is guaranteed to be set above in Bible DB Loading
				(TDictionaryDatabaseList::instance()->at(nDctNdx)->language().compare(TBibleDatabaseList::instance()->mainBibleDatabase()->language(), Qt::CaseInsensitive) == 0)) {
				TDictionaryDatabaseList::instance()->setMainDictionaryDatabase(TDictionaryDatabaseList::instance()->at(nDctNdx)->compatibilityUUID());
			}
		}
#endif	// USING_DICTIONARIES

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

	m_dlyNotesFilesAutoSave.setMinimumDelay(CPersistentSettings::instance()->notesFileAutoSaveTime()*60000);		// Convert minutes->milliseconds
	connect(&m_dlyNotesFilesAutoSave, SIGNAL(triggered()), this, SLOT(en_notesFileAutoSaveTriggered()));
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedUserNotesDatabase()), this, SLOT(en_changedUserNotesDatabase()));
	connect(CPersistentSettings::instance(), SIGNAL(changedNotesFileAutoSaveTime(int)), this, SLOT(en_changedNotesFileAutoSaveTime(int)));

#ifdef USING_WEBCHANNEL

#ifdef USING_MMDB
	// Setup MMDB Path detail:
	QFileInfo fiMMDBPath(initialAppDirPath(), g_constrMMDBPath);
	CMMDBLookup::setMMDBPath(fiMMDBPath.absoluteFilePath());
#endif

	// Launch WebChannel:
	if (!m_strWebChannelHostPort.isEmpty()) {
		QStringList lstHostPort = m_strWebChannelHostPort.split(QChar(','), My_QString_KeepEmptyParts);
		Q_ASSERT(lstHostPort.size() >= 1);
		quint16 nPort = lstHostPort.at(0).toUInt();
		if (nPort) {
			if (lstHostPort.size() == 1) {
				m_pWebChannelServer = new CWebChannelServer(QHostAddress::Any, nPort, this);
			} else {
				m_pWebChannelServer = new CWebChannelServer(QHostAddress(lstHostPort.at(1)), nPort, this);
			}
		} else {
			displayWarning(m_pSplash, g_constrInitialization, tr("Invalid WebChannel Host Port was specified.", "Errors"));
		}
	}
#else
	// If user specified a WebChannel Host Port on the command-line and this build doesn't support WebChannel, warn him:
	if (!m_strWebChannelHostPort.isEmpty()) {
		displayWarning(m_pSplash, g_constrInitialization, tr("WebChannel Host Port was specified, but this build of King James Pure Bible Search doesn't support WebChannel", "Errors"));
	}
#endif


#if !defined(IS_CONSOLE_APP) || !defined(USING_WEBCHANNEL)
	// Create the main KJVCanOpener window only if we aren't doing a console webchannel app (i.e. daemon only app)

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
#ifdef SHOW_SPLASH_SCREEN
	CKJVCanOpener *pMain = createKJVCanOpener(TBibleDatabaseList::instance()->mainBibleDatabase());
	if (m_pSplash != nullptr) {
		m_pSplash->finish((!g_pMdiArea.isNull()) ? static_cast<QWidget *>(g_pMdiArea.data()) : static_cast<QWidget *>(pMain));
		delete m_pSplash;
		m_pSplash = nullptr;
	}
#else
	createKJVCanOpener(TBibleDatabaseList::instance()->mainBibleDatabase());
#endif

#endif

	return 0;
}

// executeEvent is a wrapper for execute() to handle the QTimer::singleShot() calls to
//		post an quit/exit to the application if execute() returns an error (non-zero) value
void CMyApplication::executeEvent(bool bBuildDB)
{
	int nRetVal = execute(bBuildDB);
	if (nRetVal != 0) {
		exit(nRetVal);			// If KJPBS fails to start, exit with an error code
	}
}

// ============================================================================
