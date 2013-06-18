/*******************************************************
**
** Declaration of Q4puGenericSignalSpy
**
** Copyright (C) 2005 Prashanth N Udupa
**
** Q4puGenericSignalSpy can be used to spy on QObjects
** for signals that they emit, and the slots that are
** invoked in them.
**
********************************************************/

/***************************************************************************
                           Q4puGenericSignalSpy.h
                             -------------------
    begin                : Mon 8 August 2005
    copyright            : (C) 2005 by Prashanth Udupa
    email                : prashanth.udupa@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef Q4PU_GENERIC_SIGNAL_SPY
#define Q4PU_GENERIC_SIGNAL_SPY

#include <QObject>

class Q4puGenericSignalSpyPrivate;

class Q4puGenericSignalSpy : public QObject
{
	Q_OBJECT
	
public:
	Q4puGenericSignalSpy(QObject* parent);
	~Q4puGenericSignalSpy();
	
	void spyOn(QObject* object) { _Object = object; }
	QObject* spyingOn() const   { return _Object; }
	
	void emitCaughtSignal(const QString &sig);
	void emitCaughtSlot(const QString &slot);
	
signals:
	void caughtSignal(const QString &name);
	void caughtSlot(const QString &name);
	
private:
	QObject* _Object;
};

inline void Q4puGenericSignalSpy::emitCaughtSignal(const QString &sig)
{
	emit caughtSignal(sig);
}

inline void Q4puGenericSignalSpy::emitCaughtSlot(const QString &slot)
{
	emit caughtSlot(slot);
}

#endif
