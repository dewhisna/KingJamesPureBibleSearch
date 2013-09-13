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

#ifndef MAIN_H
#define MAIN_H

#include "dbstruct.h"

#include <QApplication>
#include <QEvent>
#include <QFileOpenEvent>
#include <QString>
#include <QStringList>
#include <QList>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QPointer>

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

	CMyApplication(int & argc, char ** argv)
#ifdef USING_QT_SINGLEAPPLICATION
		:	QtSingleApplication(g_constrApplicationID, argc, argv),
#else
		:	QApplication(argc, argv),
#endif
			m_nLastActivateCanOpener(-1)
	{

	}

	virtual ~CMyApplication()
	{

	}

	virtual bool notify(QObject *pReceiver, QEvent *pEvent);

	const QString &fileToLoad() const { return m_strFileToLoad; }

	CKJVCanOpener *createKJVCanOpener(CBibleDatabasePtr pBibleDatabase);
	bool isFirstCanOpener() const { return (m_lstKJVCanOpeners.size() == 0); }
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
	void receivedKJPBSMessage(const QString &strMessage);
	void activateCanOpener(CKJVCanOpener *pCanOpener) const;
	void activateCanOpener(int ndx) const;
	void activateAllCanOpeners() const;
	void closeAllCanOpeners() const;
	void updateSearchWindowList();

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
	void removeKJVCanOpener(QObject *pKJVCanOpener);
	void activatedKJVCanOpener(CKJVCanOpener *pCanOpener);
	void en_triggeredKJVCanOpener(QAction *pAction);

	void en_canCloseChanged(CKJVCanOpener *pCanOpener, bool bCanClose);

protected:
	bool event(QEvent *event);

protected:
	QString m_strFileToLoad;

	QList<CKJVCanOpener *> m_lstKJVCanOpeners;
	int m_nLastActivateCanOpener;						// Index of last KJVCanOpener that was activated by the user
};

#endif // MAIN_H
