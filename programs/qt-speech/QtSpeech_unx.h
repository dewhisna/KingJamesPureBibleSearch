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

//#define USE_FESTIVAL_SERVER			// TODO : REMOVE this line
#ifdef USE_FESTIVAL_SERVER
#include <QTcpSocket>
#endif

namespace QtSpeech_v1 { // API v1.0

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


#ifdef USE_FESTIVAL_SERVER
class QtSpeech_asyncServerIO : public QObject
{
	Q_OBJECT

public:
	QtSpeech_asyncServerIO(const QString &strHostname, int nPortNumber, QObject *pParent = 0L);
	virtual ~QtSpeech_asyncServerIO();

	bool isConnected();

signals:
	void operationFailed();
	void operationSucceeded();
	void operationComplete();

public slots:
	void readVoices();
	void say(const QString &strText);
	void setVoice(const QtSpeech::VoiceName &aVoice);

protected slots:
	bool connectToServer();
	void disconnectFromServer();
	QStringList sendCommand(const QString &strCommand, bool bWaitForReply = true);

private:
	QTcpSocket m_sockFestival;
	QString m_strHostname;
	int m_nPortNumber;
};
#endif

}	// namespace QtSpeech_v1
#endif // QtSpeech_unx_H

