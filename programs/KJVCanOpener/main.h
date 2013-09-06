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
#include <QList>

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
	CMyApplication(int & argc, char ** argv)
#ifdef USING_QT_SINGLEAPPLICATION
		:	QtSingleApplication(g_constrApplicationID, argc, argv)
#else
		:	QApplication(argc, argv)
#endif
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

signals:
	void loadFile(const QString &strFilename);

#ifdef SIGNAL_SPY_DEBUG
public slots:
	void signalSpyCaughtSignal(const QString &strMessage) const;
	void signalSpyCaughtSlot(const QString &strMessage) const;

public:
	static Q4puGenericSignalSpy *createSpy(QObject *pOwner, QObject *pSpyOn = NULL);
#endif

private slots:
	void removeKJVCanOpener(QObject *pKJVCanOpener);

protected:
	bool event(QEvent *event);
	QString m_strFileToLoad;

	QList<CKJVCanOpener *> m_lstKJVCanOpeners;
};

#endif // MAIN_H
