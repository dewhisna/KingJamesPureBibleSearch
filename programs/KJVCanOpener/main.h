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

#include <QApplication>
#include <QEvent>
#include <QFileOpenEvent>

#ifdef SIGNAL_SPY_DEBUG
#include "signalspy/Q4puGenericSignalSpy.h"
#endif

class CMyApplication : public QApplication
{
	Q_OBJECT
public:
	CMyApplication(int & argc, char ** argv)
		:	QApplication(argc, argv)
	{

	}

	virtual ~CMyApplication()
	{

	}

	virtual bool notify(QObject *pReceiver, QEvent *pEvent);

	const QString &fileToLoad() const { return m_strFileToLoad; }

signals:
	void loadFile(const QString &strFilename);

#ifdef SIGNAL_SPY_DEBUG
public slots:
	void signalSpyCaughtSignal(const QString &strMessage) const;
	void signalSpyCaughtSlot(const QString &strMessage) const;

public:
	static Q4puGenericSignalSpy *createSpy(QObject *pOwner, QObject *pSpyOn = NULL);
#endif

protected:
	bool event(QEvent *event);
	QString m_strFileToLoad;
};

#endif // MAIN_H
