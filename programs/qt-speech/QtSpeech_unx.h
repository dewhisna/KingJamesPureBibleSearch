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
#include <QtSpeech>

#include <QString>
#include <QStringList>

#define USE_FESTIVAL_SERVER			// TODO : REMOVE this line
#ifdef USE_FESTIVAL_SERVER
#include <QTcpSocket>
#include <QTimer>
#endif

namespace QtSpeech_v1 { // API v1.0

class QtSpeech_th : public QObject
{
	Q_OBJECT

public:
	QtSpeech_th(const QtSpeech::VoiceName &aVoiceName, QObject * p =0L)
		:	QObject(p),
			err(""),
			has_error(false),
			selectedVoiceName(aVoiceName)
	{}
	virtual ~QtSpeech_th()
	{}

public slots:
	void doInit();
	void say(QString text);
#ifdef USE_FESTIVAL_SERVER
	void startServer(int nPort);
	void stopServer();
#endif

signals:
    void logicError(QtSpeech::LogicError);
    void finished();
#ifdef USE_FESTIVAL_SERVER
	void serverStarted();
	void serverStopped();
#endif

private:
    friend class QtSpeech;
    QtSpeech::LogicError err;
    bool has_error;
	QtSpeech::VoiceName selectedVoiceName;
	static bool init;
	static bool haveSelectedVoice;
};


#ifdef USE_FESTIVAL_SERVER
class QtSpeech_asyncServerIO : public QObject
{
	Q_OBJECT

public:
	QtSpeech_asyncServerIO(int nPortNumber, QObject *pParent = 0L);
	virtual ~QtSpeech_asyncServerIO();

signals:
	void operationFailed();
	void operationSucceeded();
	void operationComplete();

public slots:
	void asyncReadVoices();
	void say(const QString &strText);

protected slots:
	bool connectToServer();
	void disconnectFromServer();
	QStringList sendCommand(const QString &strCommand);

private:
	QTcpSocket m_sockFestival;
	QTimer m_tmrAutoDisconnect;
	int m_nPortNumber;
};
#endif

}	// namespace QtSpeech_v1
#endif // QtSpeech_unx_H

