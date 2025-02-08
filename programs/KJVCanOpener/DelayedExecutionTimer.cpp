/*
Copyright (c) 2011, Andre Somers
All rights reserved.

File licence not repeated here for space reasons. See file "DelayedExecutionTimer.h" for details of licence.

-------------------------------------------------------------------------------

Modified for the behaviour desired in King James Pure Bible Search.
Modifications Copyright (C) 2013-2025, Donna Whisnant, a.k.a. Dewtronics

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
	m_lastInt(0),
	m_pLastObject(nullptr)
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
	m_lastInt(0),
	m_pLastObject(nullptr)
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
	emit triggered(static_cast<uint32_t>(m_lastInt));
	emit triggered(m_lastRelIndex);
	emit triggered(m_lastPhraseTag);
	emit triggered(m_lastPassageTag);
	emit triggered(m_pLastObject);
}

void DelayedExecutionTimer::untrigger()
{
	m_minimumTimer->stop();
	m_maximumTimer->stop();
}

bool DelayedExecutionTimer::isTriggered() const
{
	return (m_minimumTimer->isActive() || m_maximumTimer->isActive());
}

void DelayedExecutionTimer::trigger()
{
	if ((m_maximumDelay > 0) && (!m_maximumTimer->isActive())) {
		m_maximumTimer->start(m_maximumDelay);
	}
	m_minimumTimer->stop();
	if (m_minimumDelay > 0) m_minimumTimer->start(m_minimumDelay);

	if (((m_maximumDelay == -1) || (m_maximumDelay == 0)) &&
		((m_minimumDelay == -1) || (m_minimumDelay == 0))) timeout();		// If the delay is disabled, trigger immediately so we act as a pass-through
}

void DelayedExecutionTimer::trigger(const QString &string)
{
	m_lastString = string;
	trigger();
}

void DelayedExecutionTimer::trigger(int i)
{
	m_lastInt = i;
	trigger();
}

void DelayedExecutionTimer::trigger(uint32_t ui)
{
	m_lastInt = static_cast<int>(ui);
}

void DelayedExecutionTimer::trigger(const CRelIndex &ndx)
{
	m_lastRelIndex = ndx;
	trigger();
}

void DelayedExecutionTimer::trigger(const TPhraseTag &tag)
{
	m_lastPhraseTag = tag;
	trigger();
}

void DelayedExecutionTimer::trigger(const TPassageTag &tag)
{
	m_lastPassageTag = tag;
	trigger();
}

void DelayedExecutionTimer::trigger(QObject *pObject)
{
	m_pLastObject = pObject;
	trigger();
}
