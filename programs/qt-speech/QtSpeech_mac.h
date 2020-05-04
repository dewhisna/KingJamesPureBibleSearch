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

	Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef QtSpeech_mac_H
#define QtSpeech_mac_H

#include <QObject>
#include <QPointer>
#include <QtSpeech>

#include <QString>
#include <QStringList>

#include <ApplicationServices/ApplicationServices.h>

#include "QtSpeech_ATO.h"

// ============================================================================

namespace QtSpeech_v2 { // API v2.0

#ifdef Q_OS_MAC64
#define SpeechDoneUPP_ARG2 void *
#else
#define SpeechDoneUPP_ARG2 long
#endif


class QtSpeech_th : public QObject
{
	Q_OBJECT
public:
	QtSpeech_th(const QtSpeech::TVoiceName &aVoiceName, QObject *pParent = nullptr);
	virtual ~QtSpeech_th();

	void speechFinished(SpeechChannel nChannel, SpeechDoneUPP_ARG2 refCon);		// Called by QtSpeech_GlobalData when async speech event has finished

public slots:
	void doInit();
	void say(TAsyncTalkingObject aTalkingObject);
	void clearQueue();

protected slots:
	void en_sayNext(bool bInitialSay);

signals:
	void logicError(QtSpeech::LogicError);
	void finished();
	void sayNext(bool bInitialSay);
	void beginTalking();								// Triggered only when queue goes from empty to non-empty
	void doneTalking(bool bQueueEmpty);					// Triggered once for every say() operation.  Last item in buffer will send "true"

private:
	friend class QtSpeech;
	QtSpeech::LogicError err;
	bool has_error;
	static bool m_bInit;
	bool m_bAmTalking;
	TAsyncTalkingObjectsList m_lstTalkingObjects;

	SpeechChannel m_nChannel;
	SpeechDoneUPP m_pDoneCall;
};

}	// namespace QtSpeech_v2

// ============================================================================
#endif	// QtSpeech_mac_H
