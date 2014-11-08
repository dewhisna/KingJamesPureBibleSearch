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

#ifndef QtSpeech_H
#define QtSpeech_H

#include <QObject>

#if defined(QTSPEECH_STATIC)
#   define QTSPEECH_API
#elif defined(QTSPEECH_LIBRARY)
#   define QTSPEECH_API Q_DECL_EXPORT
#else
#   define QTSPEECH_API Q_DECL_IMPORT
#endif

namespace QtSpeech_v1 { // API v1.0

class QTSPEECH_API QtSpeech : public QObject
{
	Q_OBJECT

public:
    // exceptions
    struct Error { QString msg; Error(QString s):msg(s) {} };
    struct InitError : Error { InitError(QString s):Error(s) {} };
    struct LogicError : Error { LogicError(QString s):Error(s) {} };
    struct CloseError : Error { CloseError(QString s):Error(s) {} };

    // types
	struct VoiceName {
		QString id;
		QString name;
		QString lang;

		inline void clear()
		{
			id.clear();
			name.clear();
			lang.clear();
		}
		inline bool isEmpty() const { return id.isEmpty(); }
		inline bool operator==(const VoiceName &other) const
		{
			return (id == other.id);
		}

	};
    typedef QList<VoiceName> VoiceNames;

    // api
    QtSpeech(QObject * parent);
	QtSpeech(VoiceName aVoiceName = VoiceName(), QObject * parent = 0L);
    virtual ~QtSpeech();

	const VoiceName & name() const; //!< Name of current voice
    static VoiceNames voices();     //!< List of available voices in system

	static bool serverSupported();								// True if QtSpeech library compiled with server support
	static bool serverConnected();								// True if currently connected to a speech server
	static bool connectToServer(const QString &strHostname = "localhost", int nPortNumber = 1314);
	static void disconnectFromServer();

public slots:
    void say(QString) const;                                    //!< Say something, synchronous
    void tell(QString) const;                                   //!< Tell something, asynchronous

signals:
    void finished();

protected:
	void setVoice(const VoiceName &aVoice = VoiceName()) const;

	virtual void timerEvent(QTimerEvent *);

private:
	class Private;
    Private * d;
};

} // namespace QtSpeech_v1
#endif // QtSpeech_H

