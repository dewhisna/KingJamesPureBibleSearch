/*  QtSpeech -- a small cross-platform library to use TTS
	Copyright (C) 2010-2011 LynxLine.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General
	Public License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301 USA

===============================================================================

	This version modified for KJPBS usage as follows:

	Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
	Contact: http://www.dewtronics.com/

	This file is part of the KJVCanOpener Application as originally written
	and developed for Bethel Church, Festus, MO.

	GNU General Public License Usage
	This file may be used under the terms of the GNU General Public License
	version 3.0 as published by the Free Software Foundation and appearing
	in the file gpl-3.0.txt included in the packaging of this file. Please
	review the following information to ensure the GNU General Public License
	version 3.0 requirements will be met:
	http://www.gnu.org/copyleft/gpl.html.

	Other Usage
	Alternatively, this file may be used in accordance with the terms and
	conditions contained in a signed written agreement between you and
	Dewtronics.

*/

#ifndef QtSpeech_ATO_H
#define QtSpeech_ATO_H

#include <QObject>
#include <QString>
#include <QPointer>
#include <QList>
#include <QMetaType>

// ============================================================================

struct TAsyncTalkingObject
{
	TAsyncTalkingObject(const QString &strText = QString(), QObject *pObject = NULL, const char *pSlot = NULL)
		:	m_strText(strText),
			m_pObject(pObject),
			m_pSlot(pSlot)
	{ }

	bool hasNotificationSlot() const
	{
		return ((m_pSlot != NULL) && (!m_pObject.isNull()));
	}

	QString m_strText;
	QPointer<QObject> m_pObject;
	const char *m_pSlot;
};
Q_DECLARE_METATYPE(TAsyncTalkingObject)
typedef QList<TAsyncTalkingObject> TAsyncTalkingObjectsList;

// ============================================================================

class QtSpeech_asyncServerIOMonitor : public QObject
{
	Q_OBJECT
public:
	QtSpeech_asyncServerIOMonitor(QObject *pParent = 0L)
		:	QObject(pParent),
			m_bIsTalking(false)
	{
		qRegisterMetaType<TAsyncTalkingObject>("TAsyncTalkingObject");
	}
	virtual ~QtSpeech_asyncServerIOMonitor() { }

	virtual bool isTalking() const { return m_bIsTalking; }

protected slots:
	virtual void en_lostServer() = 0;							// Called when the connection with the server gets dropped (so that things like resetting the selected voice can happen)

	virtual void en_beginTalking()
	{
		m_bIsTalking = true;
	}

	virtual void en_doneTalking(bool bQueueEmpty)
	{
		if (bQueueEmpty) m_bIsTalking = false;
	}

private:
	bool m_bIsTalking;
};

// ============================================================================

#endif	// QtSpeech_ATO_H
