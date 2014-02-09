/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#ifdef USING_QT_SINGLEAPPLICATION
#include <QtSingleApplication>
#endif

#ifdef SIGNAL_SPY_DEBUG
#include "signalspy/Q4puGenericSignalSpy.h"
#endif

extern const QString g_constrApplicationID;

// ============================================================================

// Forward Declarations:
class CKJVCanOpener;
class QSplashScreen;

// ============================================================================

#ifdef USING_QT_SINGLEAPPLICATION
class CMyApplication : public QtSingleApplication
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

	QWidget *showSplash();
	void completeInterAppSplash();

	bool areRestarting() const { return m_bAreRestarting; }

	static void saveApplicationFontSettings();
	static void restoreApplicationFontSettings();
	void setupTextBrightnessStyleHooks();

	QString initialAppDirPath() const { return m_strInitialAppDirPath; }
	QString startupStyleSheet() const { return m_strStartupStyleSheet; }

	virtual bool notify(QObject *pReceiver, QEvent *pEvent);

	const QString fileToLoad() const { return m_strFileToLoad; }
	void setFileToLoad(const QString &strFilename) { m_strFileToLoad = strFilename; }

	CKJVCanOpener *createKJVCanOpener(CBibleDatabasePtr pBibleDatabase);
	bool isFirstCanOpener(bool bInCanOpenerConstructor = false) const
	{
		if (bInCanOpenerConstructor) {
			return (m_lstKJVCanOpeners.size() == 0);
		} else {
			return (m_lstKJVCanOpeners.size() <= 1);
		}
	}
	bool isLastCanOpener() const { return (m_lstKJVCanOpeners.size() <= 1); }

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

public slots:
	int execute(bool bBuildDB = false);
	void receivedKJPBSMessage(const QString &strMessage);
	void activateCanOpener(CKJVCanOpener *pCanOpener) const;
	void activateCanOpener(int ndx) const;
	void activateAllCanOpeners() const;
	void closeAllCanOpeners() const;
	void updateSearchWindowList();
	void restartApp();

signals:
	void loadFile(const QString &strFilename);
	void canQuitChanged(bool bCanQuit);

#ifdef SIGNAL_SPY_DEBUG
public slots:
	void signalSpyCaughtSignal(const QString &strMessage) const;
	void signalSpyCaughtSlot(const QString &strMessage) const;

public:
	static Q4puGenericSignalSpy *createSpy(QObject *pOwner, QObject *pSpyOn = NULL);
#endif

private slots:
	void removeKJVCanOpener(CKJVCanOpener *pKJVCanOpener);
	void activatedKJVCanOpener(CKJVCanOpener *pCanOpener);
	void en_triggeredKJVCanOpener(QAction *pAction);

	void en_canCloseChanged(CKJVCanOpener *pCanOpener, bool bCanClose);

	void en_setTextBrightness(bool bInvert, int nBrightness);
	void en_setAdjustDialogElementBrightness(bool bAdjust);

protected:
	bool event(QEvent *event);

	void setSplashMessage(const QString &strMessage = QString());

protected:
	QString m_strFileToLoad;

	QList<CKJVCanOpener *> m_lstKJVCanOpeners;
	int m_nLastActivateCanOpener;						// Index of last KJVCanOpener that was activated by the user
	QString m_strInitialAppDirPath;						// Initial applicationDirPath() -- needed since according to Qt documentation, QCoreApplcation::applicationDirPath assumes we haven't changed our current directory
	QString m_strStartupStyleSheet;						// Initial stylesheet given to us at startup, which will be the user's StyleSheet if they used the "-stylesheet" option
	bool m_bUsingCustomStyleSheet;						// Set to true if we've overridden the StartupStyleSheet
	bool m_bAreRestarting;								// Set to true if we are exiting to restart the app
#ifdef SHOW_SPLASH_SCREEN
	QElapsedTimer m_splashTimer;
#endif
	QSplashScreen *m_pSplash;							// Splash, used to parent error dialogs -- will be NULL if not doing a splash screen
};
extern QPointer<CMyApplication> g_pMyApplication;
extern QPointer<QMdiArea> g_pMdiArea;

#endif // MY_APPLICATION_H
