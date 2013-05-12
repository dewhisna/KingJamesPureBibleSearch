/*
Copyright (c) 2011, Andre Somers
All rights reserved.

File licence not repeated here for space reasons. See file "DelayedExecutionTimer.h" for details of licence.

-------------------------------------------------------------------------------

Modified for the behaviour desired in King James Pure Bible Search.
Modifications Copyright 2013, Donna Whisnant, a.k.a. Dewtronics

-------------------------------------------------------------------------------
*/

#include "DelayedExecutionTimer.h"
#include <QTimer>
#include <QStringBuilder>

DelayedExecutionTimer::DelayedExecutionTimer(int maximumDelay, int minimumDelay, QObject* parent):
	QObject(parent),
	m_minimumDelay(minimumDelay),
	m_maximumDelay(maximumDelay),
	m_minimumTimer(new QTimer(this)),
	m_maximumTimer(new QTimer(this)),
	m_lastInt(0)
{
	connect(m_minimumTimer, SIGNAL(timeout()), SLOT(timeout()));
	connect(m_maximumTimer, SIGNAL(timeout()), SLOT(timeout()));
}

DelayedExecutionTimer::DelayedExecutionTimer(QObject* parent):
	QObject(parent),
	m_minimumDelay(250),
	m_maximumDelay(-1),
	m_minimumTimer(new QTimer(this)),
	m_maximumTimer(new QTimer(this)),
	m_lastInt(0)
{
	connect(m_minimumTimer, SIGNAL(timeout()), SLOT(timeout()));
	connect(m_maximumTimer, SIGNAL(timeout()), SLOT(timeout()));
}

void DelayedExecutionTimer::timeout()
{
	m_minimumTimer->stop();
	m_maximumTimer->stop();
	emit triggered();
	emit triggered(m_prefix % m_lastString % m_postfix);
	emit triggered(m_lastInt);
}

void DelayedExecutionTimer::trigger()
{
	if ((m_maximumDelay > 0) && (!m_maximumTimer->isActive())) {
		m_maximumTimer->start(m_maximumDelay);
	}
	m_minimumTimer->stop();
	if (m_minimumDelay > 0) m_minimumTimer->start(m_minimumDelay);
}

void DelayedExecutionTimer::trigger(QString string)
{
	m_lastString = string;
	trigger();
}

void DelayedExecutionTimer::trigger(int i)
{
	m_lastInt = i;
	trigger();
}
