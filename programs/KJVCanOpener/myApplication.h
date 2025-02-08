/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef MY_APPLICATION_H
#define MY_APPLICATION_H

#include "dbstruct.h"
#include "dbDescriptors.h"
#include "DelayedExecutionTimer.h"

#include <QCoreApplication>
#include <QApplication>
#include <QMdiArea>
#include <QEvent>
#include <QFileOpenEvent>
#include <QString>
#include <QStringList>
#include <QList>
#include <QAction>
#include <QPointer>
#include <QWidget>
#ifdef SHOW_SPLASH_SCREEN
#include <QElapsedTimer>
#endif

#if defined(VNCSERVER) || defined(USING_WEBCHANNEL)
#include <QSocketNotifier>
#endif

#ifdef USING_QT_SINGLEAPPLICATION
#include <QtSingleApplication>
#endif

#ifdef USING_QT_SPEECH
#include <QtSpeech>
#endif

#ifdef USING_WEBCHANNEL
#include <webChannelServer.h>
#endif

#ifdef SIGNAL_SPY_DEBUG
#include "signalspy/Q4puGenericSignalSpy.h"
#endif

extern const QString g_constrApplicationID;

// ============================================================================

// Forward Declarations:
class CKJVCanOpener;
class QSplashScreen;
class CMyApplication;
#ifdef USING_ELSSEARCH
class CELSSearchMainWindow;
#endif

// ============================================================================

#if defined(VNCSERVER) || defined(USING_WEBCHANNEL)

class CMyDaemon : public QObject
{
	Q_OBJECT

public:
	CMyDaemon(CMyApplication *pMyApplication);
	~CMyDaemon();

	// Unix signal handlers.
	static void hupSignalHandler(int unused);
	static void termSignalHandler(int unused);
	static void usr1SignalHandler(int unused);

	static int setup_unix_signal_handlers();

public slots:
	// Qt signal handlers.
	void handleSigHup();
	void handleSigTerm();
	void handleSigUsr1();

private:
	static int m_sighupFd[2];
	static int m_sigtermFd[2];
	static int m_sigusr1Fd[2];

	QSocketNotifier *m_psnHup;
	QSocketNotifier *m_psnTerm;
	QSocketNotifier *m_psnUsr1;

	QPointer<CMyApplication> m_pMyApplication;
};

#endif

// ============================================================================

#if defined(USING_QT_SINGLEAPPLICATION)
class CMyApplication : public QtSingleApplication
#elif defined(IS_CONSOLE_APP)
class CMyApplication : public QCoreApplication
#else
class CMyApplication : public QApplication
#endif
{
	Q_OBJECT
public:
	enum KJPBS_APP_MESSAGE_COMMAND_ENUM {
		KAMCE_UNKNOWN = -1,
		KAMCE_ACTIVATE_EXISTING = 0,			// Activate application (another copy is launching and wants us to restore the window and bring it to the front without starting new pseudo-instance)
		KAMCE_ACTIVATE_EXISTING_OPEN_KJS = 1,	// Activate application and Open a .KJS Search File
		KAMCE_NEW_CANOPENER = 2,				// Launch a new CanOpener Search Window (another copy is launching and wants us to start a new search window)
		KAMCE_NEW_CANOPENER_OPEN_KJS = 3		// Launch a new CanOpener Search Window and Open a .KJS Search File
	};

	CMyApplication(int & argc, char ** argv);
	virtual ~CMyApplication();

	bool isSingleThreadedSearchResults() const { return m_bSingleThreadedSearchResults; }
	void setSingleThreadedSearchResults(bool bSingleThreadedSearchResults) { m_bSingleThreadedSearchResults = bSingleThreadedSearchResults; }		// Warning: Only call this in main() before calling execute(), otherwise isn't thread-safe

	QString selectedMainBibleDB() const { return m_strSelectedMainBibleDB; }
	void setSelectedMainBibleDB(const QString &strUUID) { m_strSelectedMainBibleDB = strUUID; }
	void setSelectedBibleBuildDBDescriptor(const TBibleDescriptor &desc) { m_bblDescSelectedForBuild = desc; }

	QString selectedMainDictDB() const { return m_strSelectedMainDictDB; }
	void setSelectedMainDictDB(const QString &strUUID) { m_strSelectedMainDictDB = strUUID; }

	QWidget *showSplash();
	void completeInterAppSplash();

	bool areRestarting() const { return m_bAreRestarting; }

	static void saveApplicationFontSettings();
	static void restoreApplicationFontSettings();
protected:
	void setupTextBrightnessStyleHooks();
public:

#ifndef IS_CONSOLE_APP
	bool isDarkMode() const;						// System light/dark if we are following system or user selection if not
	bool colorThemeCanFollowSystem() const;			// True if we are able to follow system color theme (i.e. newer Qt, etc)
#endif

	static void saveApplicationLanguage();
	static void restoreApplicationLanguage();

	static void saveTTSSettings();
	static void restoreTTSSettings();

#ifdef USING_QT_SPEECH
	static QtSpeech *speechSynth() { return m_pSpeech.data(); }
#endif

#ifdef USING_WEBCHANNEL
	static CWebChannelServer *webChannelServer() { return m_pWebChannelServer.data(); }
#endif

	QString startupStyleSheet() const { return m_strStartupStyleSheet; }

	QString ttsServerURL() const { return m_strTTSServerURL; }
	void setTTSServerURL(const QString &strTTSServerURL) { m_strTTSServerURL = strTTSServerURL; }		// Set before calling execute() so we'll connect to server at startup

	QString webChannelHostPort() const { return m_strWebChannelHostPort; }
	void setWebChannelHostPort(const QString &strWebChannelHostPort) { m_strWebChannelHostPort = strWebChannelHostPort; }

	virtual bool notify(QObject *pReceiver, QEvent *pEvent) override;

	const QString fileToLoad() const { return m_strFileToLoad; }
	void setFileToLoad(const QString &strFilename) { m_strFileToLoad = strFilename; }

	CKJVCanOpener *createKJVCanOpener(CBibleDatabasePtr pBibleDatabase);
	bool isFirstCanOpener(bool bInCanOpenerConstructor = false, const QString &strBblUUID = QString()) const;		// If strBblUUID.isEmpty(), returns overall "first" status, otherwise, it's the first of the specified Bible Database
	bool isLastCanOpener(const QString &strBblUUID = QString()) const;					// If strBblUUID.isEmpty(), returns overall "last" status, otherwise, it's the last of the specified Bible Database
	int bibleDatabaseCanOpenerRefCount(const QString &strBblUUID = QString()) const;	// Returns the number of KJVCanOpeners opened with the specified database.  If strBblUUID is empty, returns total number of open CanOpeners
	int dictDatabaseCanOpenerRefCount(const QString &strDctUUID = QString()) const;		// Returns the number of KJVCanOpeners opened with the specified database.  If strDctUUID is empty, returns total number of open CanOpeners

	CKJVCanOpener *activeCanOpener() const;
	template<class T>
	CKJVCanOpener *findCanOpenerFromChild(const T *pChild) const;
	const QList<CKJVCanOpener *> &canOpeners() const { return m_lstKJVCanOpeners; }

	bool canQuit() const;

	// Message Format:
	//		<command>;<args>
	//
	//		<command> is one of:
	//
	//
	//		<args> are Name=Value format separated by semi-colon:
	//
	// Valid arg names are:
	//		BibleUUID - UUID for the Bible Database to load/use
	//			example:	BibleUUID=85D8A6B0-E670-11E2-A28F-0800200C9A66
	//		KJS - KJS FilePathName to load
	//			example:	KJS=/home/username/Documents/MySearch.kjs
	//

	QString createKJPBSMessage(KJPBS_APP_MESSAGE_COMMAND_ENUM nCommand, const QStringList &lstArgs) const;

	int execute(bool bBuildDB = false, int nVersion = KJPBS_CCDB_VERSION);
public slots:
	void executeEvent(bool bBuildDB = false, int nVersion = KJPBS_CCDB_VERSION);
	void receivedKJPBSMessage(const QString &strMessage);
	void activateCanOpener(CKJVCanOpener *pCanOpener) const;
	void activateCanOpener(int ndx) const;
	void activateAllCanOpeners() const;
	void closeAllCanOpeners(QWidget *pCallingMainWindow = nullptr);
	void restartApp(QWidget *pCallingMainWindow = nullptr);

#ifdef USING_ELSSEARCH
	void registerELSSearchWindow(CELSSearchMainWindow *pELSSearch);
#endif

#ifndef IS_CONSOLE_APP
	void configureSettings(int nInitialPage = -1);

	int confirmFollowLink();					// Returns either QMessageBox::Yes or QMessageBox::No

	void showHelpManual();
	void showHelpAbout();
	void gotoPureBibleSearchDotCom();
#endif

signals:
	void loadFile(const QString &strFilename);
	void canQuitChanged(bool bCanQuit);
	void updateSearchWindowList();
	void changeActiveCanOpener(CKJVCanOpener *pNewActiveCanOpener, CKJVCanOpener *pOldActiveCanOpener);

#ifdef SIGNAL_SPY_DEBUG
public slots:
	void signalSpyCaughtSignal(const QString &strMessage) const;
	void signalSpyCaughtSlot(const QString &strMessage) const;

public:
	static Q4puGenericSignalSpy *createSpy(QObject *pOwner, QObject *pSpyOn = nullptr);
#endif

private slots:
#ifdef USING_QT_SPEECH
	void en_clearingSpeechQueue();						// Used to set waitCursor while stopping speech operations
	void en_speechFinished(bool bQueueEmpty);
#endif

	void removeKJVCanOpener(CKJVCanOpener *pKJVCanOpener);
	void activateKJVCanOpener(CKJVCanOpener *pCanOpener);
	void en_triggeredKJVCanOpener(QAction *pAction);

	void en_canCloseChanged(CKJVCanOpener *pCanOpener, bool bCanClose);

	void en_changedUseSystemColorTheme(bool bUseSystemColorTheme);
	void en_setTextBrightness(bool bInvert, int nBrightness);
	void en_setAdjustDialogElementBrightness(bool bAdjust);
	void en_setDisableToolTips(bool bDisableToolTips);

	void en_notesFileAutoSaveTriggered();
	void en_changedUserNotesDatabase();
	void en_changedNotesFileAutoSaveTime(int nAutoSaveTime);

protected:
	virtual bool event(QEvent *event) override;

	void setSplashMessage(const QString &strMessage = QString());

protected:
	bool m_bSingleThreadedSearchResults;				// True if we are using single-threaded Search Results processing
	QString m_strFileToLoad;

#ifdef USING_ELSSEARCH
	QList< QPointer<CELSSearchMainWindow> > m_lstELSSearchWindows;		// List of ELSSearch Windows to close at shutdown
#endif

	QList<CKJVCanOpener *> m_lstKJVCanOpeners;
	int m_nLastActivatedCanOpener;						// Index of last KJVCanOpener that was activated by the user
	QString m_strDarkStyleSheet;						// KJPBS Dark StyleSheet
	QString m_strStartupStyleSheet;						// Initial stylesheet given to us at startup, which will be the user's StyleSheet if they used the "-stylesheet" option
	QPointer<class MyProxyStyle> m_pMyProxyStyle;
	bool m_bUsingCustomStyleSheet;						// Set to true if we've overridden the StartupStyleSheet
	bool m_bAreRestarting;								// Set to true if we are exiting to restart the app
	QString m_strTTSServerURL;							// Text-To-Speech server URL for use with Festival, etc. from the command-line "-TTSServer" option
	QString m_strWebChannelHostPort;					// WebChannel host interface and port to use from the command-line "-webchannel" option
#ifdef SHOW_SPLASH_SCREEN
	QElapsedTimer m_splashTimer;
	QSplashScreen *m_pSplash;							// Splash, used to parent error dialogs -- will be NULL if not doing a splash screen
#else
	QWidget *m_pSplash;									// Splash, used to parent error dialogs -- will be NULL if not doing a splash screen
#endif
#ifdef USING_QT_SPEECH
	static QPointer<QtSpeech> m_pSpeech;				// "Global" singleton for Speech
#endif
#ifdef USING_WEBCHANNEL
	static QPointer<CWebChannelServer> m_pWebChannelServer;	// "Global" singletons for WebChannel
#endif
	QString m_strSelectedMainBibleDB;					// Selected (or Default) Main Bible Database UUID
	TBibleDescriptor m_bblDescSelectedForBuild;			// Selected Bible Database Descriptor when Building Database and using -bbl index (so that filename is correct)
	QString m_strSelectedMainDictDB;					// Selected (or Default) Main Dictionary Database UUID
	DelayedExecutionTimer m_dlyNotesFilesAutoSave;		// Delay timer for notes file auto-save trigger
};
extern QPointer<CMyApplication> g_pMyApplication;
extern QPointer<QMdiArea> g_pMdiArea;

#endif // MY_APPLICATION_H
