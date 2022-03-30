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

	Copyright (C) 2014-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef QtSpeech_unx_H
#define QtSpeech_unx_H

#include <QObject>
#include <QPointer>
#include <QtSpeech>

#include <QString>
#include <QStringList>

#ifdef USE_FESTIVAL_SERVER
#include <QTcpSocket>
#endif

#include "QtSpeech_ATO.h"

// ============================================================================

namespace QtSpeech_v2 { // API v2.0

class QtSpeech_th : public QObject
{
	Q_OBJECT

public:
	QtSpeech_th(QObject *pParent = nullptr);
	virtual ~QtSpeech_th()
	{}

public slots:
	void doInit();
	void say(TAsyncTalkingObject aTalkingObject);
	void eval(QString strExpr);
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
	TAsyncTalkingObjectsList m_lstTalkingObjects;		// Used with the multi-threaded local Festival (as opposed to client/server)
};

// ============================================================================

#ifdef USE_FESTIVAL_SERVER

class QtSpeech_asyncServerIO : public QObject
{
	Q_OBJECT

public:
	QtSpeech_asyncServerIO(const QString &strHostname, int nPortNumber, QObject *pParent = nullptr);
	virtual ~QtSpeech_asyncServerIO();

	bool isConnected();

signals:
	void lostServer();
	void readVoicesComplete();
	void setVoiceComplete();

	void beginTalking();								// Triggered only when queue goes from empty to non-empty
	void doneTalking(bool bQueueEmpty);					// Triggered once for every say() operation.  Last item in buffer will send "true"

public slots:
	void readVoices();
	void say(const TAsyncTalkingObject &aTalkingObject);
	void setVoice(const QtSpeech::TVoiceName &aVoice);
	void clearQueue();

protected slots:
	bool connectToServer();
	void disconnectFromServer();
	QStringList sendCommand(const QString &strCommand, bool bWaitForReply = true);
	void en_readyRead();
	void en_sayNext();

private:
	QTcpSocket m_sockFestival;
	QString m_strHostname;
	int m_nPortNumber;
	bool m_bAmTalking;
	bool m_bSendCommandInProgress;
	TAsyncTalkingObjectsList m_lstTalkingObjects;
};
#endif

}	// namespace QtSpeech_v2

// ============================================================================

#endif // QtSpeech_unx_H

