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
    Boston, MA 02110-1301 USA */

#ifndef QtSpeech_unx_H
#define QtSpeech_unx_H

#include <QObject>
#include <QPointer>
#include <QtSpeech>

#include <QString>
#include <QStringList>
#include <QList>

#ifdef USE_FESTIVAL_SERVER
#include <QTcpSocket>
#endif

namespace QtSpeech_v1 { // API v1.0

// ============================================================================

class QtSpeech_th : public QObject
{
	Q_OBJECT

public:
	QtSpeech_th(QObject * p =0L)
		:	QObject(p),
			err(""),
			has_error(false)
	{}
	virtual ~QtSpeech_th()
	{}

public slots:
	void doInit();
	void say(QString strText);
	void eval(QString strExpr);

signals:
    void logicError(QtSpeech::LogicError);
    void finished();

private:
    friend class QtSpeech;
    QtSpeech::LogicError err;
    bool has_error;
	static bool init;
};


class QtSpeech_asyncServerIOMonitor : public QObject
{
	Q_OBJECT
public:
	QtSpeech_asyncServerIOMonitor(QObject *pParent = 0L)
		:	QObject(pParent)
	{ }
	virtual ~QtSpeech_asyncServerIOMonitor() { }

protected slots:
	virtual void en_lostServer() = 0;							// Called when the connection with the server gets dropped (so that things like resetting the selected voice can happen)
};

// ============================================================================

#ifdef USE_FESTIVAL_SERVER

struct TAsyncTalkingObject
{
	TAsyncTalkingObject(const QString &strText, QObject *pObject = NULL, const char *pSlot = NULL)
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
typedef QList<TAsyncTalkingObject> TAsyncTalkingObjectsList;



class QtSpeech_asyncServerIO : public QObject
{
	Q_OBJECT

public:
	QtSpeech_asyncServerIO(const QString &strHostname, int nPortNumber, QObject *pParent = 0L);
	virtual ~QtSpeech_asyncServerIO();

	bool isConnected();

signals:
	void lostServer();
	void readVoicesComplete();
	void setVoiceComplete();

	void doneTalking();

public slots:
	void readVoices();
	void say(const TAsyncTalkingObject &aTalkingObject);
	void setVoice(const QtSpeech::VoiceName &aVoice);

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
	TAsyncTalkingObjectsList m_lstTalkingObjects;
};
#endif

// ============================================================================

}	// namespace QtSpeech_v1
#endif // QtSpeech_unx_H

