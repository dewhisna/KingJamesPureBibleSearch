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
//#include <QtPlugin>
#include <QFontDatabase>
#include <QDesktopServices>
#include <QDir>
#include <QObject>
#include <QProxyStyle>
#include <QSharedPointer>
#include <QPointer>
#include <QList>
#ifdef USING_SINGLEAPPLICATION
#include <singleapplication.h>
#endif
#include <QMdiArea>
#include <QMdiSubWindow>

#include "main.h"
#include "KJVCanOpener.h"

#include "version.h"
#include "dbstruct.h"
#include "BuildDB.h"
#include "ReadDB.h"

#include "PersistentSettings.h"
#include "UserNotesDatabase.h"

#include <assert.h>
#include <iostream>

#ifdef Q_OS_WIN
// Needed to call CreateMutex to lockout installer running while we are:
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//Q_IMPORT_PLUGIN(qsqlite)

QPointer<CMyApplication> g_pMyApplication = NULL;
QPointer<QMdiArea> g_pMdiArea = NULL;

const QString g_constrApplicationID = "KingJamesPureBibleSearch";

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const int g_connMinSplashTimeMS = 5000;		// Minimum number of milliseconds to display splash screen
	const int g_connInterAppSplasTimeMS = 2000;	// Splash Time for Inter-Application communications

	const QString g_constrInitialization = QObject::tr("King James Pure Bible Search Initialization");

#ifndef Q_OS_MAC
	const char *g_constrPluginsPath = "../../KJVCanOpener/plugins/";
	const char *g_constrDatabaseFilename = "../../KJVCanOpener/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "../../KJVCanOpener/db/kjvuser.s3db";
#else
	const char *g_constrPluginsPath = "../Frameworks/";
	const char *g_constrDatabaseFilename = "../Resources/db/kjvtext.s3db";
	const char *g_constrUserDatabaseTemplateFilename = "../Resources/db/kjvuser.s3db";
#endif
	const char *g_constrUserDatabaseFilename = "kjvuser.s3db";

#ifndef Q_OS_MAC
	const char *g_constrScriptBLFontFilename = "../../KJVCanOpener/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "../../KJVCanOpener/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../../KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../../KJVCanOpener/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../../KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "../../KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "../../KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../../KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../../KJVCanOpener/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../../KJVCanOpener/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../../KJVCanOpener/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "../../KJVCanOpener/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "../../KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "../../KJVCanOpener/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "../../KJVCanOpener/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "../../KJVCanOpener/fonts/DejaVuSerif.ttf";
#else
	const char *g_constrScriptBLFontFilename = "../Resources/fonts/SCRIPTBL.TTF";
	const char *g_constrDejaVuSans_BoldOblique = "../Resources/fonts/DejaVuSans-BoldOblique.ttf";
	const char *g_constrDejaVuSans_Bold = "../Resources/fonts/DejaVuSans-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_BoldOblique = "../Resources/fonts/DejaVuSansCondensed-BoldOblique.ttf";
	const char *g_constrDejaVuSansCondensed_Bold = "../Resources/fonts/DejaVuSansCondensed-Bold.ttf";
	const char *g_constrDejaVuSansCondensed_Oblique = "../Resources/fonts/DejaVuSansCondensed-Oblique.ttf";
	const char *g_constrDejaVuSansCondensed = "../Resources/fonts/DejaVuSansCondensed.ttf";
	const char *g_constrDejaVuSans_ExtraLight = "../Resources/fonts/DejaVuSans-ExtraLight.ttf";
	const char *g_constrDejaVuSansMono_BoldOblique = "../Resources/fonts/DejaVuSansMono-BoldOblique.ttf";
	const char *g_constrDejaVuSansMono_Bold = "../Resources/fonts/DejaVuSansMono-Bold.ttf";
	const char *g_constrDejaVuSansMono_Oblique = "../Resources/fonts/DejaVuSansMono-Oblique.ttf";
	const char *g_constrDejaVuSansMono = "../Resources/fonts/DejaVuSansMono.ttf";
	const char *g_constrDejaVuSans_Oblique = "../Resources/fonts/DejaVuSans-Oblique.ttf";
	const char *g_constrDejaVuSans = "../Resources/fonts/DejaVuSans.ttf";
	const char *g_constrDejaVuSerif_BoldItalic = "../Resources/fonts/DejaVuSerif-BoldItalic.ttf";
	const char *g_constrDejaVuSerif_Bold = "../Resources/fonts/DejaVuSerif-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_BoldItalic = "../Resources/fonts/DejaVuSerifCondensed-BoldItalic.ttf";
	const char *g_constrDejaVuSerifCondensed_Bold = "../Resources/fonts/DejaVuSerifCondensed-Bold.ttf";
	const char *g_constrDejaVuSerifCondensed_Italic = "../Resources/fonts/DejaVuSerifCondensed-Italic.ttf";
	const char *g_constrDejaVuSerifCondensed = "../Resources/fonts/DejaVuSerifCondensed.ttf";
	const char *g_constrDejaVuSerif_Italic = "../Resources/fonts/DejaVuSerif-Italic.ttf";
	const char *g_constrDejaVuSerif = "../Resources/fonts/DejaVuSerif.ttf";
#endif	// Q_OS_MAC


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
		NULL
	};


	// Key constants:
	// --------------
	const QString constrMainAppControlGroup("MainApp/Controls");
	const QString constrFontNameKey("FontName");
	const QString constrFontSizeKey("FontSize");


	// Bible Database Descriptor Constants:
	// ------------------------------------
	typedef struct {
		const QString m_strDBName;
		const QString m_strDBDesc;
		const QString m_strUUID;
	} TBibleDescriptor;

	const TBibleDescriptor constBibleDescriptors[] =
	{
		// Special Test Value:
		{ "Special Test", QObject::tr("Special Test Bible Database"), "00000000-0000-11E3-8FFD-0800200C9A66" },
		// KJV:
		{ "King James", QObject::tr("King James Version (1769)"), "85D8A6B0-E670-11E2-A28F-0800200C9A66" },
		// RVG2010:
		{ "Reina-Valera Gómez", QObject::tr("Reina-Valera Gómez Version (2010)"), "9233CB60-141A-11E3-8FFD-0800200C9A66" },
		// KJF2006:
		{ "King James Française 2006", QObject::tr("la Bible King James Française, édition 2006"), "31FC2ED0-141B-11E3-8FFD-0800200C9A66" }
	};

	enum BIBLE_DESCRIPTOR_ENUM {
		BDE_SPECIAL_TEST = 0,
		BDE_KJV = 1,
		BDE_RVG2010 = 2,
		BDE_KJF2006 = 3
	};

}	// namespace

// ============================================================================

CBibleDatabasePtr locateBibleDatabase(const QString &strUUID)
{
	QString strTargetUUID = strUUID;

	if (strTargetUUID.isEmpty()) {
		// Default database is KJV
		strTargetUUID = constBibleDescriptors[BDE_KJV].m_strUUID;
	}

	for (int ndx = 0; ndx < g_lstBibleDatabases.size(); ++ndx) {
		if (g_lstBibleDatabases.at(ndx)->compatibilityUUID().compare(strTargetUUID, Qt::CaseInsensitive) == 0)
			return g_lstBibleDatabases.at(ndx);
	}

	return CBibleDatabasePtr();
}

// ============================================================================

class MyProxyStyle : public QProxyStyle
{
public:
	int styleHint(StyleHint hint, const QStyleOption *option = 0,
				const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const
	{
		if (hint == QStyle:: SH_ItemView_ActivateItemOnSingleClick) return 0;

		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

// ============================================================================

bool CMyApplication::notify(QObject *pReceiver, QEvent *pEvent)
{
	try {
		return QApplication::notify(pReceiver, pEvent);
	} catch (const std::exception &ex) {
		std::cerr << "std::exception was caught: " << ex.what() << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception was caught" << std::endl;
		assert(false);
	}

	return false;
}

bool CMyApplication::event(QEvent *event) {
	if (event->type() == QEvent::FileOpen) {
		m_strFileToLoad = static_cast<QFileOpenEvent *>(event)->file();
		emit loadFile(m_strFileToLoad);
		// Emulate receiving activate existing w/open KJS message:
		QString strMessage = createKJPBSMessage(KAMCE_ACTIVATE_EXISTING_OPEN_KJS, QStringList(QString("KJS=%1").arg(m_strFileToLoad)));
		receivedKJPBSMessage(strMessage);
		return true;
	}
	return QApplication::event(event);
}

#ifdef SIGNAL_SPY_DEBUG
Q4puGenericSignalSpy *CMyApplication::createSpy(QObject *pOwner, QObject *pSpyOn)
{
	assert(g_pMyApplication != NULL);
	Q4puGenericSignalSpy *pSpy = new Q4puGenericSignalSpy((pOwner != NULL) ? pOwner : g_pMyApplication);

	QObject::connect(pSpy, SIGNAL(caughtSignal(const QString&)), g_pMyApplication, SLOT(signalSpyCaughtSignal(const QString &)));
	QObject::connect(pSpy, SIGNAL(caughtSlot(const QString&)), g_pMyApplication, SLOT(signalSpyCaughtSlot(const QString &)));

	// If we are given an object to spy on, attach to it.  If not, but were given
	//		an owner, attach to it.  If not, don't attach to anything...
	if ((pSpyOn != NULL) || (pOwner != NULL)) {
		pSpy->spyOn((pSpyOn != NULL) ? pSpyOn : pOwner);
	}

	return pSpy;
}

void CMyApplication::signalSpyCaughtSignal(const QString &strMessage) const
{
	std::cerr << strMessage.toUtf8().data() << std::endl;
}

void CMyApplication::signalSpyCaughtSlot(const QString &strMessage) const
{
	std::cerr << strMessage.toUtf8().data() << std::endl;
}
#endif

// ============================================================================

CKJVCanOpener *CMyApplication::createKJVCanOpener(CBibleDatabasePtr pBibleDatabase)
{
	CKJVCanOpener *pCanOpener = new CKJVCanOpener(pBibleDatabase);
	m_lstKJVCanOpeners.append(pCanOpener);
	connect(pCanOpener, SIGNAL(destroyed(QObject*)), this, SLOT(removeKJVCanOpener(QObject*)));
	connect(pCanOpener, SIGNAL(windowActivated(CKJVCanOpener*)), this, SLOT(activatedKJVCanOpener(CKJVCanOpener*)));
	connect(pCanOpener, SIGNAL(canCloseChanged(CKJVCanOpener*, bool)), this, SLOT(en_canCloseChanged(CKJVCanOpener*, bool)));

	if (g_pMdiArea != NULL) {
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

	pCanOpener->initialize();
	pCanOpener->show();
	updateSearchWindowList();
	return pCanOpener;
}

void CMyApplication::removeKJVCanOpener(QObject *pKJVCanOpener)
{
	CKJVCanOpener *pCanOpener = static_cast<CKJVCanOpener*>(pKJVCanOpener);
	int ndxCanOpener = m_lstKJVCanOpeners.indexOf(pCanOpener);
	assert(ndxCanOpener != -1);
	if (ndxCanOpener == m_nLastActivateCanOpener) m_nLastActivateCanOpener = -1;
	if (ndxCanOpener != -1) m_lstKJVCanOpeners.removeAt(ndxCanOpener);
	if (g_pMdiArea != NULL) {
		if (m_lstKJVCanOpeners.size() == 0) {
			g_pMdiArea->deleteLater();
		} else {
			QList<QMdiSubWindow *> lstSubWindows = g_pMdiArea->subWindowList();
			for (int ndxSubWindows = 0; ndxSubWindows < lstSubWindows.size(); ++ndxSubWindows) {
				if (lstSubWindows.at(ndxSubWindows)->widget() == NULL) {
					lstSubWindows.at(ndxSubWindows)->close();
					break;
				}
			}
		}
	}
	updateSearchWindowList();
}

void CMyApplication::activatedKJVCanOpener(CKJVCanOpener *pCanOpener)
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		if (m_lstKJVCanOpeners.at(ndx) == pCanOpener) {
			m_nLastActivateCanOpener = ndx;
			return;
		}
	}

	assert(false);
	m_nLastActivateCanOpener = -1;
}

CKJVCanOpener *CMyApplication::activeCanOpener() const
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		if (m_lstKJVCanOpeners.at(ndx)->isActiveWindow()) return m_lstKJVCanOpeners.at(ndx);
	}
	return NULL;
}

template<class T>
CKJVCanOpener *CMyApplication::findCanOpenerFromChild(const T *pChild) const
{
	assert(pChild != NULL);
	for (int ndxCanOpener = 0; ndxCanOpener < m_lstKJVCanOpeners.size(); ++ndxCanOpener) {
		QList<T *>lstFoundChildren = m_lstKJVCanOpeners.at(ndxCanOpener)->findChildren<T *>(pChild->objectName());
		for (int ndxChild = 0; ndxChild < lstFoundChildren.size(); ++ndxChild) {
			if (lstFoundChildren.at(ndxChild) == pChild) return m_lstKJVCanOpeners.at(ndxCanOpener);
		}
	}
	return NULL;
}

class CSearchResultsTreeView;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<CSearchResultsTreeView>(const CSearchResultsTreeView *) const;

class i_CScriptureBrowser;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<i_CScriptureBrowser>(const i_CScriptureBrowser *) const;

class i_CScriptureEdit;
template CKJVCanOpener *CMyApplication::findCanOpenerFromChild<i_CScriptureEdit>(const i_CScriptureEdit *) const;


void CMyApplication::activateCanOpener(CKJVCanOpener *pCanOpener) const
{
	assert(pCanOpener != NULL);
	if (g_pMdiArea != NULL) {
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
	assert((ndx >= 0) && (ndx < m_lstKJVCanOpeners.size()));
	if ((ndx < 0) || (ndx >= m_lstKJVCanOpeners.size())) return;

	activateCanOpener(m_lstKJVCanOpeners.at(ndx));
}

void CMyApplication::activateAllCanOpeners() const
{
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		activateCanOpener(ndx);
	}
}

void CMyApplication::closeAllCanOpeners() const
{
	assert(canQuit());
	if (!canQuit()) return;

	// Close in reverse order:
	for (int ndx = (m_lstKJVCanOpeners.size()-1); ndx >= 0; --ndx) {
		QTimer::singleShot(0, m_lstKJVCanOpeners.at(ndx), SLOT(close()));
	}
	// Note: List update will happen automatically as the windows close...
}

void CMyApplication::updateSearchWindowList()
{
	assert(m_pActionSearchWindowList != NULL);
	if (m_pActionSearchWindowList == NULL) return;
	assert(m_pActionSearchWindowList->menu() != NULL);
	if (m_pActionSearchWindowList->menu() == NULL) return;

	if (m_pActionGroupSearchWindowLists != NULL) delete m_pActionGroupSearchWindowLists;
	m_pActionGroupSearchWindowLists = new QActionGroup(this);

	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		QAction *pAction = new QAction(m_lstKJVCanOpeners.at(ndx)->searchWindowDescription(), m_pActionGroupSearchWindowLists);
		pAction->setData(ndx);
		m_pActionSearchWindowList->menu()->addAction(pAction);
	}
	connect(m_pActionGroupSearchWindowLists.data(), SIGNAL(triggered(QAction*)), this, SLOT(en_triggeredKJVCanOpener(QAction*)));
}

void CMyApplication::en_triggeredKJVCanOpener(QAction *pAction)
{
	assert(pAction != NULL);
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

	QStringList lstMsg = strMessage.split(";", QString::KeepEmptyParts);
	assert(lstMsg.size() >= 1);
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
		std::cerr << "*** KJPBS : Unrecognized inter-application command : " << strCommand.toUtf8().data() << std::endl;
	}

	for (int ndxArgs = 1; ndxArgs < lstMsg.size(); ++ndxArgs) {
		if (lstMsg.at(ndxArgs).isEmpty()) continue;
		QStringList lstArg = lstMsg.at(ndxArgs).split(QChar('='));
		if (lstArg.size() != 2) {
			std::cerr << "*** KJPBS : Malformed inter-application argument : " << lstMsg.at(ndxArgs).toUtf8().data() << std::endl;
		} else {
			if (lstArg.at(0).compare("KJS", Qt::CaseInsensitive) == 0) {
				strKJSFileName = lstArg.at(1);
			} else if (lstArg.at(1).compare("BibleUUID", Qt::CaseInsensitive) == 0) {
				strBibleUUID = lstArg.at(1);
			} else {
				std::cerr << "*** KJPBS : Unrecognized inter-application argument : " << lstMsg.at(ndxArgs).toUtf8().data() << std::endl;
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
				assert(false);
				return;
			}
			activateCanOpener(nIndex);
			break;
		}
		case KAMCE_ACTIVATE_EXISTING_OPEN_KJS:
		case KAMCE_NEW_CANOPENER:
		case KAMCE_NEW_CANOPENER_OPEN_KJS:
		{
			bool bForceOpen = ((nCommand == KAMCE_NEW_CANOPENER_OPEN_KJS) || (nCommand == KAMCE_NEW_CANOPENER));
			CKJVCanOpener *pCanOpener = NULL;
			if ((bForceOpen) || (m_lstKJVCanOpeners.size() != 1)) {
				// If we have more than one, just open a new window and launch the file:
				CBibleDatabasePtr pBibleDatabase = locateBibleDatabase(strBibleUUID);
				if (pBibleDatabase == NULL) pBibleDatabase = g_pMainBibleDatabase;
				pCanOpener = createKJVCanOpener(pBibleDatabase);
				assert(pCanOpener != NULL);
			} else {
				pCanOpener = m_lstKJVCanOpeners.at(0);
			}
			activateCanOpener(pCanOpener);
			if (!strKJSFileName.isEmpty()) pCanOpener->openKJVSearchFile(strKJSFileName);
			break;
		}
		default:
			break;
	}
}

// ============================================================================

int main(int argc, char *argv[])
{
	CMyApplication app(argc, argv);
	g_pMyApplication = &app;
	app.setApplicationVersion(VER_QT);
	app.setApplicationName(VER_APPNAME_STR_QT);
	app.setOrganizationName(VER_ORGNAME_STR_QT);
	app.setOrganizationDomain(VER_ORGDOMAIN_STR_QT);
#ifdef USING_SINGLEAPPLICATION
	SingleApplication instance(g_constrApplicationID, &app);
#endif
#ifdef USING_QT_SINGLEAPPLICATION
	QtSingleApplication &instance = app;
#endif

	app.setStyle(new MyProxyStyle());			// Note: QApplication will take ownership of this (no need for delete)

	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
	QString strKJSFile;
	bool bBuildDB = false;

	Q_INIT_RESOURCE(KJVCanOpener);

	// Hook receiving messages from other KJPBS app launches:
	app.connect(&instance, SIGNAL(messageReceived(const QString &)), &app, SLOT(receivedKJPBSMessage(const QString &)));

	QPixmap pixSplash(":/res/KJPBS_SplashScreen800x500.png");
	QSplashScreen *splash = new QSplashScreen(pixSplash);
	splash->show();
	splash->raise();
	splash->showMessage(QString("<html><body><table height=425 width=500><tr><td>&nbsp;</td></tr></table><div align=\"center\"><font size=+1 color=#FFFFFF><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") +
							QObject::tr("Please Wait...") +
							QString("</b></font></div></body></html>"), Qt::AlignBottom | Qt::AlignLeft);
	splash->repaint();
	qApp->processEvents();

#ifdef Q_OS_WIN
	HANDLE hMutex = CreateMutexW(NULL, false, L"KJVCanOpenerMutex");
	assert(hMutex != NULL);
	// Note: System will automatically close the mutex object when we
	//			exit and InnoSetup actually suggest we leave it open
#endif

	QTime splashTimer;
	splashTimer.start();

	// Parse the Commmand-line:
	if (strKJSFile.isEmpty() && !app.fileToLoad().isEmpty()) strKJSFile = app.fileToLoad();

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg(argv[ndx]);
		if (strArg.compare("-builddb", Qt::CaseInsensitive) == 0) {
			bBuildDB = true;
		} else if ((!strArg.startsWith("-")) && (strKJSFile.isEmpty())) {
			strKJSFile = strArg;
		} else {
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Unrecognized command-line option \"%1\"").arg(strArg));
		}
	}

	// Check for existing KJPBS and have it handle this launch request:
	if (instance.isRunning()) {
		int nElapsed;
		do {
			nElapsed = splashTimer.elapsed();
		} while ((nElapsed>=0) && (nElapsed<g_connInterAppSplasTimeMS));		// Test the 0 case in case of DST shift so user doesn't have to sit here for an extra hour

		QString strMessage;
		if (!strKJSFile.isEmpty()) {
			QFileInfo fiKJSFile(strKJSFile);
			strMessage = app.createKJPBSMessage(CMyApplication::KAMCE_ACTIVATE_EXISTING_OPEN_KJS, QStringList(QString("KJS=%1").arg(fiKJSFile.absoluteFilePath())));
		} else{
			strMessage = app.createKJPBSMessage(CMyApplication::KAMCE_NEW_CANOPENER, QStringList());
		}

		if (instance.sendMessage(strMessage, 5000)) {
			delete splash;
			return 0;
		} else {
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("There appears to be another copy of King James Pure Bible Search running, but it is not responding. "
																			 "Please check the running copy to see if it's functioning and revive it and/or reboot."));
			delete splash;
			return -1;
		}
	}

	// Setup our Fonts:
	for (int ndxFont = 0; g_constrarrFontFilenames[ndxFont] != NULL; ++ndxFont) {
		QFileInfo fiFont(app.applicationDirPath(), g_constrarrFontFilenames[ndxFont]);
		int nFontStatus = QFontDatabase::addApplicationFont(fiFont.absoluteFilePath());
		if (nFontStatus == -1) {
#ifdef QT_DEBUG
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to load font file:\n\"%1\"").arg(fiFont.absoluteFilePath()));
#endif
		}
	}

	// Sometimes the splash screen fails to paint, so we'll pump events again
	//	between the fonts and database:
	splash->repaint();
	qApp->processEvents();

	// Setup our SQL Plugin paths:
	QFileInfo fiPlugins(app.applicationDirPath(), g_constrPluginsPath);
	app.addLibraryPath(fiPlugins.absolutePath());

	// Database Paths:
	QFileInfo fiDatabase(app.applicationDirPath(), g_constrDatabaseFilename);
	QFileInfo fiUserDatabaseTemplate(app.applicationDirPath(), g_constrUserDatabaseTemplateFilename);
	QString strDataFolder = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QFileInfo fiUserDatabase(strDataFolder, g_constrUserDatabaseFilename);
	QDir dirDataFolder;
	dirDataFolder.mkpath(strDataFolder);

//	qRegisterMetaTypeStreamOperators<TPhraseTag>("TPhraseTag");


//CBuildDatabase adb(splash);
//adb.BuildDatabase(fiDatabase.absoluteFilePath());
//return 0;

	// Read User Database if it exists:
	QString strUserDatabaseFilename;

	{
		CBuildDatabase bdb(splash);
		if (bBuildDB) {
			if (!bdb.BuildDatabase(fiDatabase.absoluteFilePath())) {
				QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to Build KJV Database!\nAborting..."));
				return -2;
			}
		}

		// Read Main Database
		CReadDatabase rdb(splash);
		if (!rdb.ReadDatabase(fiDatabase.absoluteFilePath(), QObject::tr("King James"), QObject::tr("King James Version (1769)"), "85D8A6B0-E670-11E2-A28F-0800200C9A66", true)) {
			QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to Read and Validate KJV Database!\nCheck Installation!"));
			return -3;
		}

		if (!fiUserDatabase.exists()) {
			// If the user's database doesn't exist, see if the template one
			//		does.  And if so, see if we can copy from it to the user's:
			if (fiUserDatabaseTemplate.exists()) {
				if (rdb.ReadUserDatabase(fiUserDatabaseTemplate.absoluteFilePath(), true)) {
					if (bdb.BuildUserDatabase(fiUserDatabase.absoluteFilePath(), true)) {
						// Use the copy if that was successful.  Strings will have already
						//	been set, so no need to read again:
						strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
					}
					// If we were successful with reading the template database, we'll
					//	already have the strings loaded, so the user can use them.  But,
					//	we'll leave the pathname empty to disable user changes since
					//	the template will be a read-only copy
				} else {
					// Otherwise, if reading the template failed and there was no user
					//	database, see if we can just create a user database:
					if (bdb.BuildUserDatabase(fiUserDatabase.absoluteFilePath(), true)) {
						strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
					}
				}
			} else {
				// If there was no template and no user database, see if we can just create
				//	the user database.  If so, use it (this is like the failure case above,
				//	but where the template doesn't exist at all):
				if (bdb.BuildUserDatabase(fiUserDatabase.absoluteFilePath(), true)) {
					strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
				}
			}
		} else {
			if (!rdb.ReadUserDatabase(fiUserDatabase.absoluteFilePath())) {
				QMessageBox::warning(splash, g_constrInitialization, QObject::tr("Failed to Read KJV User Database!\nCheck Installation and Verify Database File!"));
				return -4;
			} else {
				strUserDatabaseFilename = fiUserDatabase.absoluteFilePath();
			}
		}
	}

	// Show splash for minimum time:
	int nElapsed;
	do {
		nElapsed = splashTimer.elapsed();
	} while ((nElapsed>=0) && (nElapsed<g_connMinSplashTimeMS));		// Test the 0 case in case of DST shift so user doesn't have to sit here for an extra hour


	// Setup our default font for our controls:

#ifdef Q_WS_WIN
	QFont fntAppControls = QFont("MS Shell Dlg 2", 8);
#elif defined(Q_WS_MAC)
	QFont fntAppControls = QFont("DejaVu Sans", 10);
#else
	QFont fntAppControls = QFont("DejaVu Sans", 8);
#endif

	QSettings &settings(CPersistentSettings::instance()->settings());

	settings.beginGroup(constrMainAppControlGroup);
	QString strFontName = settings.value(constrFontNameKey, fntAppControls.family()).toString();
	int nFontSize = settings.value(constrFontSizeKey, fntAppControls.pointSize()).toInt();
	settings.endGroup();

	if ((!strFontName.isEmpty()) && (nFontSize>0)) {
		fntAppControls.setFamily(strFontName);
		fntAppControls.setPointSize(nFontSize);
	}

	app.setFont(fntAppControls);

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
#ifdef Q_WS_MAC
	app.setDesktopSettingsAware(false);
#endif

	// Update settings for next time.  Use application font instead of
	//		our variables in case Qt substituted for another available font:
	settings.beginGroup(constrMainAppControlGroup);
	settings.setValue(constrFontNameKey, app.font().family());
	settings.setValue(constrFontSizeKey, app.font().pointSize());
	settings.endGroup();

	// Create default empty KJN file before we create CKJVCanOpener:
	g_pUserNotesDatabase = QSharedPointer<CUserNotesDatabase>(new CUserNotesDatabase());
	g_strUserDatabase = strUserDatabaseFilename;

#ifdef USE_MDI_MAIN_WINDOW
	g_pMdiArea = new QMdiArea();
	g_pMdiArea->show();
#ifdef Q_WS_WIN
	g_pMdiArea->setWindowIcon(QIcon(":/res/bible.ico"));
#else
	g_pMdiArea->setWindowIcon(QIcon(":/res/bible_48.png"));
#endif
#endif

	// Must have database read above before we create main or else the
	//		data won't be available for the browser objects and such:
	CKJVCanOpener *pMain = app.createKJVCanOpener(g_pMainBibleDatabase);
	splash->finish((g_pMdiArea.data() != NULL) ? static_cast<QWidget *>(g_pMdiArea.data()) : static_cast<QWidget *>(pMain));
	delete splash;

	if (!strKJSFile.isEmpty()) pMain->openKJVSearchFile(strKJSFile);

	int nRetVal = app.exec();

	QFontDatabase::removeAllApplicationFonts();

	return nRetVal;
}

