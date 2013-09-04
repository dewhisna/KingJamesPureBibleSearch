/*******************************************************************************
SingleApplication provides a way to detect a running instance.
Copyright (C) 2008  Ritt K.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
********************************************************************************/

#include "singleapplication.h"
#include "singleapplication_p.h"

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QHash>
#include <QtCore/QReadWriteLock>
#include <QtCore/QRegExp>
#include <QtCore/QSharedMemory>
#include <QtCore/QString>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#endif

// timeouts are in ms
static const int i_timeout_connect	= 200;
static const int i_timeout_read		= 250;
static const int i_timeout_write	= 500;

static QString login()
{
	static QString login;
	if(login.isEmpty())
	{
#ifdef Q_WS_QWS
		login = QLatin1String("qws");
#endif
#ifdef Q_OS_WIN
		QT_WA({
			wchar_t buffer[256];
			DWORD bufferSize = sizeof(buffer) / sizeof(wchar_t) - 1;
			GetUserNameW(buffer, &bufferSize);
			login = QString::fromUtf16((ushort*)buffer);
		},
		{
			char buffer[256];
			DWORD bufferSize = sizeof(buffer) / sizeof(char) - 1;
			GetUserNameA(buffer, &bufferSize);
			login = QString::fromLocal8Bit(buffer);
		});
#else
		struct passwd* pwd = getpwuid(getuid());
		if(pwd)
			login = QLatin1String(pwd->pw_name);
#endif
		Q_ASSERT(!login.isEmpty());
	}

	return login;
}

static bool writeMessage(QLocalSocket* socket, const QString& message, int timeout = i_timeout_write)
{
	if(!socket || socket->state() != QLocalSocket::ConnectedState)
		return false;

	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out << (quint32)0;
	out << message;
	out.device()->seek(0);
	out << (quint32)(block.size() - sizeof(quint32));

	bool ok = (socket->write(block) != -1);
	if(ok)
		ok &= socket->waitForBytesWritten(timeout);

	return ok;
}

static QString readMessage(QLocalSocket* socket, bool* ok = 0)
{
	QString message;
	if(ok)
		*ok = false;

	if(!socket || socket->state() != QLocalSocket::ConnectedState)
		return message;

	while(socket->bytesAvailable() < (int)sizeof(quint32))
	{
		if(!socket->waitForReadyRead(i_timeout_read))
			return message;
	}

	quint32 blockSize;
	QDataStream in(socket);
	in >> blockSize;
	while(socket->bytesAvailable() < blockSize)
	{
		if(!socket->waitForReadyRead(i_timeout_read))
		{
			in >> message;
			return message;
		}
	}
	in >> message;

	if(ok)
		*ok = true;

	return message;
}

void LocalThread::run()
{
	QLocalSocket socket;
	m_socket = &socket;
	connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(m_socket, SIGNAL(disconnected()), this, SLOT(quit()));

	if(!m_socket->setSocketDescriptor(m_socketDescriptor) || m_socket->state() != QLocalSocket::ConnectedState)
		return;
	// send key to client for verification
	if(!writeMessage(m_socket, m_key))
		return;

	exec();
}

void LocalThread::readyRead()
{
	bool ok = false;
	QString message;
	while(m_socket->bytesAvailable() > 0)
	{
		// read message from client
		message = readMessage(m_socket, &ok);
		if(ok)
		{
			writeMessage(m_socket, m_key, i_timeout_write); // confirm receiving
			emit messageReceived(message);
		}
	}
}


class ServerCache : public QHash<QString, QLocalServer*>
{
public:
	mutable QReadWriteLock lock;
};
Q_GLOBAL_STATIC(ServerCache, serverCache)


class SingleApplicationPrivate
{
public:
	inline SingleApplicationPrivate(SingleApplication* parent) : q(parent),
		socket(0),
		isRunning(false)
	{}
	inline ~SingleApplicationPrivate()
	{}

	void init();
	bool connectToServer();
	bool sendMessage(const QString& message, int timeout);

	SingleApplication* q;

	QString key;
	QString uniqueKey;

	QLocalSocket* socket;

	bool isRunning;
};

void SingleApplicationPrivate::init()
{
	key.replace(QRegExp(QLatin1String("[^A-Za-z0-9]")), QString());
	if(key.isEmpty())
		key = QLatin1String("SADEFAULTKEY");
	uniqueKey = QString("_sa_").append(key).append(QLatin1Char('_')).append(login().toUtf8().toHex());

	ServerCache* cache = serverCache();
	QWriteLocker locker(&cache->lock);

	QLocalServer* server = cache->value(uniqueKey, 0);
	if(!server)
	{
		QSharedMemory* shMem = new QSharedMemory(uniqueKey, qApp);
		if(shMem->create(1))
		{
			server = new ThreadedLocalServer(key, qApp);
			if(!server->listen(uniqueKey))
			{
				qWarning("QLocalServer::listen: %s (Used key is '%s')",
						 server->errorString().toUtf8().data(), uniqueKey.toLatin1().data());
				delete server;
				server = 0;
			}
			cache->insert(uniqueKey, server);
		}
		else
		{
			isRunning = (shMem->error() == QSharedMemory::AlreadyExists);
			if(!isRunning)
			{
				qWarning("QSharedMemory::create: %s (Used key is '%s')",
						 shMem->errorString().toUtf8().data(), uniqueKey.toLatin1().data());
			}
			delete shMem;
		}
	}
	if(server)
	{
		QObject::connect(server, SIGNAL(messageReceived(const QString&)),
						 q, SIGNAL(messageReceived(const QString&)),
						 Qt::QueuedConnection);
	}
}

bool SingleApplicationPrivate::connectToServer()
{
	if(socket && socket->state() == QLocalSocket::ConnectedState)
		return true;

	if(!socket)
		socket = new QLocalSocket(q);
	socket->connectToServer(uniqueKey);
	if(socket->waitForConnected(i_timeout_connect))
	{
		bool ok;
		const QString message = readMessage(socket, &ok);
		// now compare received bytes with key
		if(ok && message.compare(key) == 0)
			return true;

		socket->disconnectFromServer();
		socket->waitForDisconnected(i_timeout_connect);
	}
	socket->abort();

	return false;
}

bool SingleApplicationPrivate::sendMessage(const QString& message, int timeout)
{
	if(!isRunning || !connectToServer())
		return false;

	if(!writeMessage(socket, message, timeout))
		return false;

	bool ok;
	QString response = readMessage(socket, &ok);

	return (ok && response.compare(key) == 0);
}


/*!
  \class SingleApplication
  \brief The SingleApplication class provides an crossplatform interface to detect a running
  instance, and to send command strings to that instance.

  The SingleApplication component is basically imitating QtSingleApplication commercial class.
  Unlike QtSingleApplication the SingleApplication implementation uses Shared Memory
  to detect a running instance and so-called "Local Sockets" to communicate with it.
*/

/*!
  Creates a SingleApplication object with the parent \a parent and the default key.

  \sa key(), QCoreApplication::applicationName()
*/

SingleApplication::SingleApplication(QObject* parent) : QObject(parent),
	d(new SingleApplicationPrivate(this))
{
	d->key = QCoreApplication::applicationName();
	d->init();
}

/*!
  Creates a SingleApplication object with the parent \a parent and the key \a key.

  \sa key()
*/

SingleApplication::SingleApplication(const QString& key, QObject* parent) : QObject(parent),
	d(new SingleApplicationPrivate(this))
{
	d->key = key;
	d->init();
}

/*!
  The destructor destroys the SingleApplication object, but the
  underlying shared memory and local server are not removed from the system
  unless application exit.

  Note: If another instance of application with the same key is started again it WILL find this instance.
*/

SingleApplication::~SingleApplication()
{
	delete d;
}

/*!
  Returns the key of this single application.
*/

QString SingleApplication::key() const
{
	return d->key;
}

/*!
  Returns true if another instance of this application has started;
  otherwise returns false.

  This function does not find instances of this application that are
  being run by a different user.
*/

bool SingleApplication::isRunning() const
{
	return d->isRunning;
}

/*!
  This is an overloaded static member function, provided for convenience.

  Returns true if another instance of this application has started;
  otherwise returns false.
*/

bool SingleApplication::isRunning(const QString& key)
{
	return SingleApplication(key).isRunning();
}

/*!
  Tries to send the text \a message to the currently running instance.
  The SingleApplication object in the running instance
  will emit the messageReceived() signal when it receives the message.

  This function returns true if the message has been sent to, and
  processed by, the current instance. If there is no instance
  currently running, or if the running instance fails to process the
  message within \a timeout milliseconds this function return false.

  \sa messageReceived()
*/

bool SingleApplication::sendMessage(const QString& message, int timeout)
{
	return d->sendMessage(message, timeout);
}

/*!
  This is an overloaded static member function, provided for convenience.

  Tries to send the text \a message to the currently running instance.
*/

bool SingleApplication::sendMessage(const QString& key, const QString& message, int timeout)
{
	return SingleApplication(key).sendMessage(message, timeout);
}

/*!
  \fn void SingleApplication::messageReceived(const QString& message)

  This signal is emitted when the current instance receives a \a
  message from another instance of this application.

  \sa sendMessage()
*/

#include "moc_singleapplication.cpp"
