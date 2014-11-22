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

#if defined(Q_OS_LINUX)
#define QTSPEECH_DEFAULT_SERVER_PORT 1314				// Festival's typical server port number
#define QTSPEECH_SERVER_SCHEME_NAME "festival"
#elif defined(Q_OS_WIN)
#define QTSPEECH_DEFAULT_SERVER_PORT -1
#define QTSPEECH_SERVER_SCHEME_NAME ""
#elif defined(Q_OS_MACX)
#define QTSPEECH_DEFAULT_SERVER_PORT -1
#define QTSPEECH_SERVER_SCHEME_NAME ""
#else
// TODO : Add other OS's here
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
	struct TVoiceName {
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
		inline bool operator==(const TVoiceName &other) const
		{
			return (id == other.id);
		}
		inline bool operator!=(const TVoiceName &other) const
		{
			return (id != other.id);
		}

	};
	typedef QList<TVoiceName> TVoiceNamesList;

	// api
	QtSpeech(QObject * parent);
	QtSpeech(const TVoiceName &aVoiceName = TVoiceName(), QObject * parent = 0L);
	virtual ~QtSpeech();

	const TVoiceName &voiceName() const; //!< Name of current voice
	void setVoiceName(const TVoiceName &aVoiceName);
	static TVoiceNamesList voices();     //!< List of available voices in system

	virtual bool canSpeak() const;					// Returns true if speech is supported on this system (i.e. no errors during initialization)

	static bool isTalking();						// Returns true if currently talking (i.e. between beginning and finished signals)

	static bool serverSupported();					// True if QtSpeech library compiled with server support
	static bool serverConnected();					// True if currently connected to a speech server
	static bool connectToServer(const QString &strHostname = "localhost", int nPortNumber = 1314);
	static void disconnectFromServer();

public slots:
	virtual void say(const QString &strText) const;														//!< Say something, synchronous
	virtual void tell(const QString &strText, QObject *pObject = 0L, const char *pSlot = 0L) const;		//!< Tell something, asynchronous
	virtual void clearQueue();						// Clear all pending "tell" operations

signals:
	void beginning();								// Triggered on first say or tell operation when queue is empty (additional say/tell operations do NOT trigger it until finished(true) has been triggered)
	void finished(bool bQueueEmpty);				// Triggered at the end of ALL say/tell operations.  Last item in queue will trigger with finished(true).
	void clearingQueue();							// Triggered when clearQueue is called so app can set waitCursor, etc.

protected:
	virtual void timerEvent(QTimerEvent *);

private:
	class Private;
	Private * d;
};

} // namespace QtSpeech_v1

#endif // QtSpeech_H

