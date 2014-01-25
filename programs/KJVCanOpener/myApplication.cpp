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

#include "myApplication.h"
#include "KJVCanOpener.h"

#ifdef USING_SINGLEAPPLICATION
#include <singleapplication.h>
#endif

#include <QFont>

#include <QMdiSubWindow>

#include "PersistentSettings.h"

#include <assert.h>

// ============================================================================

QPointer<CMyApplication> g_pMyApplication = NULL;
QPointer<QMdiArea> g_pMdiArea = NULL;

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


	// Bible Database Descriptor Constants:
	// ------------------------------------
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


	// Dictionary Database Descriptor Constants:
	// -----------------------------------------
	const TDictionaryDescriptor constDictionaryDescriptors[] =
	{
		// Special Test Value:
		{ "Special Test", QObject::tr("Special Test Dictionary Database"), "00000000-0000-11E3-8224-0800200C9A66" },
		// Webster 1828:
		{ "Webster 1828", QObject::tr("Webster's Unabridged Dictionary, 1828"), "6A94E150-1E6C-11E3-8224-0800200C9A66" }
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

CDictionaryDatabasePtr locateDictionaryDatabase(const QString &strUUID)
{
	QString strTargetUUID = strUUID;

	if (strTargetUUID.isEmpty()) {
		// Default database is Web1828
		strTargetUUID = constDictionaryDescriptors[DDE_WEB1828].m_strUUID;
	}
	for (int ndx = 0; ndx < g_lstDictionaryDatabases.size(); ++ndx) {
		if (g_lstDictionaryDatabases.at(ndx)->compatibilityUUID().compare(strTargetUUID, Qt::CaseInsensitive) == 0)
			return g_lstDictionaryDatabases.at(ndx);
	}

	return CDictionaryDatabasePtr();
}

const TBibleDescriptor &bibleDescriptor(BIBLE_DESCRIPTOR_ENUM nIndex)
{
	assert(static_cast<unsigned int>(nIndex) < _countof(constBibleDescriptors));
	return constBibleDescriptors[nIndex];
}

const TDictionaryDescriptor &dictionaryDescriptor(DICTIONARY_DESCRIPTOR_ENUM nIndex)
{
	assert(static_cast<unsigned int>(nIndex) < _countof(constDictionaryDescriptors));
	return constDictionaryDescriptors[nIndex];
}

// ============================================================================

CMyApplication::CMyApplication(int & argc, char ** argv)
#ifdef USING_QT_SINGLEAPPLICATION
	:	QtSingleApplication(g_constrApplicationID, argc, argv),
#else
	:	QApplication(argc, argv),
#endif
		m_nLastActivateCanOpener(-1),
		m_bUsingCustomStyleSheet(false),
		m_bAreRestarting(false)
{
#ifdef Q_OS_ANDROID
	m_strInitialAppDirPath = QDir::homePath();
#else
	m_strInitialAppDirPath = applicationDirPath();
#endif
	m_strStartupStyleSheet = styleSheet();

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
}

CMyApplication::~CMyApplication()
{
	// We must clean up our databases and things before exiting or else
	//		the destructor tear-down order might cause us to crash, particularly
	//		with SQL Database things:
	g_lstBibleDatabases.clear();
	g_pMainBibleDatabase.clear();
	g_lstDictionaryDatabases.clear();
	g_pMainDictionaryDatabase.clear();
	g_pUserNotesDatabase.clear();
}

// ============================================================================

void CMyApplication::saveApplicationFontSettings()
{
	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		settings.beginGroup(constrMainAppControlGroup);
		settings.setValue(constrFontNameKey, font().family());
		settings.setValue(constrFontSizeKey, font().pointSize());
		settings.endGroup();
	}
}

void CMyApplication::restoreApplicationFontSettings()
{
	// Setup our default font for our controls:
#ifdef Q_OS_WIN32
	QFont fntAppControls = QFont("MS Shell Dlg 2", 8);
#elif defined(Q_OS_MAC)
	QFont fntAppControls = QFont("Arial", 12);
#elif EMSCRIPTEN
	QFont fntAppControls = QFont("DejaVu Sans", 12);
#else
	QFont fntAppControls = QFont("DejaVu Sans", 8);
#endif

	if (CPersistentSettings::instance()->settings() != NULL) {
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
}


void CMyApplication::setupTextBrightnessStyleHooks()
{
	// Setup Default TextBrightness:
	en_setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(en_setTextBrightness(bool, int)));
	connect(CPersistentSettings::instance(), SIGNAL(adjustDialogElementBrightnessChanged(bool)), this, SLOT(en_setAdjustDialogElementBrightness(bool)));
}

// ============================================================================

bool CMyApplication::notify(QObject *pReceiver, QEvent *pEvent)
{
	try {
#ifdef USING_QT_SINGLEAPPLICATION
		return QtSingleApplication::notify(pReceiver, pEvent);
#else
		return QApplication::notify(pReceiver, pEvent);
#endif
	} catch (const std::exception &ex) {
		qDebug("std::exception was caught: %s", ex.what());
	} catch (...) {
		qDebug("Unknown exception was caught");
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
#ifdef USING_QT_SINGLEAPPLICATION
	return QtSingleApplication::event(event);
#else
	return QApplication::event(event);
#endif
}

#ifdef SIGNAL_SPY_DEBUG
Q4puGenericSignalSpy *CMyApplication::createSpy(QObject *pOwner, QObject *pSpyOn)
{
	assert(g_pMyApplication.data() != NULL);
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
	qDebug("%s", strMessage.toUtf8().data());
}

void CMyApplication::signalSpyCaughtSlot(const QString &strMessage) const
{
	qDebug("%s", strMessage.toUtf8().data());
}
#endif

// ============================================================================

CKJVCanOpener *CMyApplication::createKJVCanOpener(CBibleDatabasePtr pBibleDatabase)
{
	m_bAreRestarting = false;			// Once we create a new CanOpener we are no longer restarting...
	CKJVCanOpener *pCanOpener = new CKJVCanOpener(pBibleDatabase);
	m_lstKJVCanOpeners.append(pCanOpener);
	connect(pCanOpener, SIGNAL(isClosing(CKJVCanOpener*)), this, SLOT(removeKJVCanOpener(CKJVCanOpener*)));
	connect(pCanOpener, SIGNAL(windowActivated(CKJVCanOpener*)), this, SLOT(activatedKJVCanOpener(CKJVCanOpener*)));
	connect(pCanOpener, SIGNAL(canCloseChanged(CKJVCanOpener*, bool)), this, SLOT(en_canCloseChanged(CKJVCanOpener*, bool)));

	if (g_pMdiArea.data() != NULL) {
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
	updateSearchWindowList();
	return pCanOpener;
}

void CMyApplication::removeKJVCanOpener(CKJVCanOpener *pKJVCanOpener)
{
	int ndxCanOpener = m_lstKJVCanOpeners.indexOf(pKJVCanOpener);
	assert(ndxCanOpener != -1);
	if (ndxCanOpener == m_nLastActivateCanOpener) m_nLastActivateCanOpener = -1;
	if (ndxCanOpener != -1) m_lstKJVCanOpeners.removeAt(ndxCanOpener);
	if (g_pMdiArea.data() != NULL) {
		if (m_lstKJVCanOpeners.size() == 0) {
			if (!areRestarting()) g_pMdiArea->deleteLater();
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

	// The following is needed on Mac to make sure the menu of the
	//      new KJVCanOpen gets set:
	if (activeWindow() != static_cast<QWidget *>(pCanOpener))
			setActiveWindow(pCanOpener);

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
	if (g_pMdiArea.data() != NULL) {
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
	for (int ndx = 0; ndx < m_lstKJVCanOpeners.size(); ++ndx) {
		m_lstKJVCanOpeners.at(ndx)->en_updateSearchWindowList();
	}
}

void CMyApplication::restartApp()
{
	m_bAreRestarting = true;
	closeAllCanOpeners();
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

void CMyApplication::en_setTextBrightness(bool bInvert, int nBrightness)
{
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
}

void CMyApplication::en_setAdjustDialogElementBrightness(bool bAdjust)
{
	Q_UNUSED(bAdjust);
	en_setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());
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
				if (pBibleDatabase.data() == NULL) pBibleDatabase = g_pMainBibleDatabase;
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
