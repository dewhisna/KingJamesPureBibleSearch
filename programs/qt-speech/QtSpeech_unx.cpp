/*  Q..tSpeech -- a small cross-platform library to use TTS
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

#include <QtCore>
#include <QtSpeech>
#include <QtSpeech_unx.h>
#include <festival.h>

#include <assert.h>

namespace QtSpeech_v1 { // API v1.0

// ============================================================================

#define DEBUG_SERVER_IO
#define SERVER_IO_TIMEOUT 3000				// Timeout in msec for read, write, connect, etc
#define SERVER_IO_BLOCK_MAX 65536l			// Maximum number of read/write bytes per transfer
#define SERVER_AUTO_DISCONNECT_TIME 60000	// TCP Auto Disconnect time from server in msec

// ============================================================================

// internal data
class QtSpeech::Private {
public:
	Private()
		:onFinishSlot(0L) {}

	VoiceName name;
#ifdef USE_FESTIVAL_SERVER
	QtSpeech_asyncServerIO *asyncServerIO()
	{
		if ((m_pAsyncServerIO.isNull()) && (serverRunning())) {
			assert(!serverThread.isNull());
			m_pAsyncServerIO = new QtSpeech_asyncServerIO(serverPort);
			connect(serverThread.data(), SIGNAL(serverStopped()), m_pAsyncServerIO.data(), SLOT(deleteLater()));
		}
		return (m_pAsyncServerIO.data());
	}
#endif

	static const QString VoiceId;
	static bool serverRunning() { return (pidServer != 0); }

	const char * onFinishSlot;
	QPointer<QObject> onFinishObj;

	static QPointer<QThread> speechThread;
	static const VoiceName DefaultVoiceName;
	static VoiceNames lstVoiceNames;
	static const VoiceName commonVoiceNames[];
	static pid_t pidServer;
	static QPointer<QtSpeech_th> serverThread;			// Valid on when Festival Server is running -- used for the shutdown process
	static int serverPort;								// Server Port number, valid when server is running
#ifdef USE_FESTIVAL_SERVER
private:
	QPointer<QtSpeech_asyncServerIO> m_pAsyncServerIO;
#endif
};
QPointer<QThread> QtSpeech::Private::speechThread = 0L;
const QString QtSpeech::Private::VoiceId = QString("(voice_%1)");
const QtSpeech::VoiceName QtSpeech::Private::DefaultVoiceName = {QtSpeech::Private::VoiceId.arg("cmu_us_slt_arctic_hts"), "English (Male)", "en"};
QtSpeech::VoiceNames QtSpeech::Private::lstVoiceNames;
pid_t QtSpeech::Private::pidServer = 0;
QPointer<QtSpeech_th> QtSpeech::Private::serverThread = 0L;
int QtSpeech::Private::serverPort = FESTIVAL_DEFAULT_PORT;

const QtSpeech::VoiceName QtSpeech::Private::commonVoiceNames[] =
{
	{ QtSpeech::Private::VoiceId.arg("cmu_us_awb_cg"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("cmu_us_slt_arctic_hts"), "English (female)", "en" },
	{ QtSpeech::Private::VoiceId.arg("cmu_us_rms_cg"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("cmu_us_clb_artic_clunits"), "English (female)", "en" },
	{ QtSpeech::Private::VoiceId.arg("ked_diphone"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("cmu_us_jmk_arctic_clunits"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("cmu_us_rms_arctic_clunits"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("en1_mbrola"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("kal_diphone"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("don_diphone"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("rab_diphone"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("us2_mbrola"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("us3_mbrola"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("cmu_us_awb_arctic_clunits"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("us1_mbrola"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("cmu_us_bdl_arctic_clunits"), "English (male)", "en" },
	{ QtSpeech::Private::VoiceId.arg("el_diphone"), "Spanish (male)", "es" },
	{ "", "", "" }
};

// ============================================================================

// some defines for throwing exceptions
#define Where QString("%1:%2:").arg(__FILE__).arg(__LINE__)
#define SysCall(x,e) {\
    int ok = x;\
    if (!ok) {\
        QString msg = #e;\
        msg += ":"+QString(__FILE__);\
        msg += ":"+QString::number(__LINE__)+":"+#x;\
        throw e(msg);\
    }\
}

// ============================================================================

// qobject for speech thread
bool QtSpeech_th::haveSelectedVoice = false;
bool QtSpeech_th::init = false;
void QtSpeech_th::doInit()
{
	if (!init) {
		festival_initialize(true, FESTIVAL_HEAP_SIZE);
		init = true;
	}
}

void QtSpeech_th::say(QString text) {
    try {
		doInit();
        has_error = false;
		if (!haveSelectedVoice) {
			EST_String est_voice(selectedVoiceName.id.toUtf8());
			SysCall(festival_eval_command(est_voice), QtSpeech::LogicError);
			haveSelectedVoice = true;
		}
        EST_String est_text(text.toUtf8());
        SysCall(festival_say_text(est_text), QtSpeech::LogicError);
    }
	catch (const QtSpeech::LogicError &e) {
        has_error = true;
        err = e;
    }
    emit finished();
}

#ifdef USE_FESTIVAL_SERVER

void QtSpeech_th::startServer(int nPort)
{
	if (QtSpeech::Private::pidServer != 0) return;

//	doInit();

	pid_t pidFork = fork();
	if (pidFork) {
		// For Parent, save the PID so we can later kill it:
		QtSpeech::Private::pidServer = pidFork;
		emit serverStarted();
		return;
	}

	// This doesn't exit!
	festival_initialize(true, FESTIVAL_HEAP_SIZE);
	festival_start_server(nPort);
}

void QtSpeech_th::stopServer()
{
	if (QtSpeech::Private::pidServer == 0) return;

	emit serverStopped();

	kill(QtSpeech::Private::pidServer, SIGTERM);
	QtSpeech::Private::pidServer = 0;
	emit finished();
}

#endif

// ============================================================================

// implementation
QtSpeech::QtSpeech(QObject * parent)
    :QObject(parent), d(new Private)
{
	if (Private::DefaultVoiceName.id.isEmpty())
        throw InitError(Where+"No default voice in system");

	d->name = Private::DefaultVoiceName;
}

QtSpeech::QtSpeech(VoiceName aVoiceName, QObject * parent)
    :QObject(parent), d(new Private)
{
	if (aVoiceName.isEmpty()) {
		aVoiceName = Private::DefaultVoiceName;
    }

	if (aVoiceName.isEmpty())
        throw InitError(Where+"No default voice in system");

	d->name = aVoiceName;
}

QtSpeech::~QtSpeech()
{
	delete d;
}

// ----------------------------------------------------------------------------

const QtSpeech::VoiceName &QtSpeech::name() const
{
    return d->name;
}

QtSpeech::VoiceNames QtSpeech::voices()
{
	if (!Private::lstVoiceNames.isEmpty()) return Private::lstVoiceNames;

	// Set default in case we fail to setup client/server or aren't using the server:
	Private::lstVoiceNames.clear();
	for (int ndxCommonVoice = 0; (!Private::commonVoiceNames[ndxCommonVoice].isEmpty()); ++ndxCommonVoice) {
		Private::lstVoiceNames << Private::commonVoiceNames[ndxCommonVoice];
	}

#ifdef USE_FESTIVAL_SERVER
	if (Private::serverRunning()) {
		QtSpeech_asyncServerIO asyncServerIO(Private::serverPort);
		QEventLoop el;
		connect(&asyncServerIO, SIGNAL(operationComplete()), &el, SLOT(quit()), Qt::QueuedConnection);
		asyncServerIO.asyncReadVoices();
		el.exec(QEventLoop::ExcludeUserInputEvents);
	}
#endif

	return Private::lstVoiceNames;
}

// ----------------------------------------------------------------------------

bool QtSpeech::serverAvailable()
{
#ifdef USE_FESTIVAL_SERVER
	return true;
#else
	return false;
#endif
}

bool QtSpeech::serverRunning()
{
	return Private::serverRunning();
}

bool QtSpeech::startServer(int nPort)
{
#ifdef USE_FESTIVAL_SERVER
	if (serverRunning()) return true;
	assert(Private::serverThread.isNull());

	Private::serverPort = nPort;

	if (Private::speechThread.isNull()) {
		Private::speechThread = new QThread;
		Private::speechThread->start();
	}

	Private::serverThread = new QtSpeech_th(Private::DefaultVoiceName);
	Private::serverThread->moveToThread(Private::speechThread);
	connect(Private::serverThread.data(), SIGNAL(finished()), Private::serverThread.data(), SLOT(deleteLater()), Qt::QueuedConnection);
	QEventLoop el;
	connect(Private::serverThread.data(), SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	connect(Private::serverThread.data(), SIGNAL(serverStarted()), &el, SLOT(quit()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(Private::serverThread.data(), "startServer", Qt::QueuedConnection, Q_ARG(int, nPort));
	el.exec(QEventLoop::ExcludeUserInputEvents);

	return true;
#else
	Q_UNUSED(nPort);
	return false;
#endif
}

void QtSpeech::stopServer()
{
#ifdef USE_FESTIVAL_SERVER
	if (!serverRunning()) return;

	assert(!Private::serverThread.isNull());
	assert(!Private::speechThread.isNull());

	QEventLoop el;
	connect(Private::serverThread.data(), SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	connect(Private::serverThread.data(), SIGNAL(serverStopped()), &el, SLOT(quit()), Qt::QueuedConnection);

	QMetaObject::invokeMethod(Private::serverThread.data(), "stopServer", Qt::QueuedConnection);
	el.exec(QEventLoop::ExcludeUserInputEvents);
#endif
}

// ----------------------------------------------------------------------------

void QtSpeech::tell(QString text) const
{
#ifdef USE_FESTIVAL_SERVER
	if (d->serverRunning()) {
		QtSpeech_asyncServerIO *pAsyncServerIO = d->asyncServerIO();
		assert(pAsyncServerIO != NULL);
		if (pAsyncServerIO != NULL) {
			pAsyncServerIO->say(text);
		}
	}
#else
	if (!d->speechThread) {
		d->speechThread = new QThread;
		d->speechThread->start();
	}

	QtSpeech_th * th = new QtSpeech_th(name());
	th->moveToThread(d->speechThread);
	connect(th, SIGNAL(finished()), this, SIGNAL(finished()), Qt::QueuedConnection);
	connect(th, SIGNAL(finished()), th, SLOT(deleteLater()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(th, "say", Qt::QueuedConnection, Q_ARG(QString,text));
#endif
}

void QtSpeech::say(QString text) const
{
#ifdef USE_FESTIVAL_SERVER
	if (d->serverRunning()) {
		QtSpeech_asyncServerIO *pAsyncServerIO = d->asyncServerIO();
		assert(pAsyncServerIO != NULL);
		if (pAsyncServerIO != NULL) {
			QEventLoop el;
			connect(pAsyncServerIO, SIGNAL(operationComplete()), &el, SLOT(quit()), Qt::QueuedConnection);
			pAsyncServerIO->say(text);
			el.exec(QEventLoop::ExcludeUserInputEvents);
		}
	}
#else
	if (!d->speechThread) {
		d->speechThread = new QThread;
		d->speechThread->start();
	}

	QEventLoop el;
	QtSpeech_th th(name());
	th.moveToThread(d->speechThread);
	connect(&th, SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(&th, "say", Qt::QueuedConnection, Q_ARG(QString,text));
	el.exec(QEventLoop::ExcludeUserInputEvents);

	if (th.has_error)
		throw th.err;
#endif
}

void QtSpeech::timerEvent(QTimerEvent * te)
{
    QObject::timerEvent(te);
}

// ============================================================================

#ifdef USE_FESTIVAL_SERVER

QtSpeech_asyncServerIO::QtSpeech_asyncServerIO(int nPortNumber, QObject *pParent)
	:	QObject(pParent),
		m_nPortNumber(nPortNumber)
{
	connect(&m_tmrAutoDisconnect, SIGNAL(timeout()), this, SLOT(disconnectFromServer()));
}

QtSpeech_asyncServerIO::~QtSpeech_asyncServerIO()
{
	disconnectFromServer();
}

bool QtSpeech_asyncServerIO::connectToServer()
{
	if (m_sockFestival.state() == QAbstractSocket::ConnectedState) {
		m_tmrAutoDisconnect.start(SERVER_AUTO_DISCONNECT_TIME);
		return true;
	}

	m_tmrAutoDisconnect.stop();

	m_sockFestival.connectToHost("localhost", m_nPortNumber);

#ifdef DEBUG_SERVER_IO
	qDebug("Connecting...");
#endif

	if (!m_sockFestival.waitForConnected(SERVER_IO_TIMEOUT)) {
#ifdef DEBUG_SERVER_IO
		qDebug("Failed");
#endif
		return false;
	}

#ifdef DEBUG_SERVER_IO
	qDebug("Connected");
#endif

	m_tmrAutoDisconnect.start(SERVER_AUTO_DISCONNECT_TIME);

	return true;
}

void QtSpeech_asyncServerIO::disconnectFromServer()
{
#ifdef DEBUG_SERVER_IO
	qDebug("Closing...");
#endif

	m_tmrAutoDisconnect.stop();
	m_sockFestival.close();
}

QStringList QtSpeech_asyncServerIO::sendCommand(const QString &strCommand)
{
	if (m_sockFestival.state() != QAbstractSocket::ConnectedState) return QStringList();

	m_sockFestival.write(QString("%1\n").arg(strCommand).toUtf8());

#ifdef DEBUG_SERVER_IO
	qDebug("Wrote \"%s\" Command", strCommand.toUtf8().data());
#endif

	QStringList lstResultLines;

	if (m_sockFestival.waitForBytesWritten(SERVER_IO_TIMEOUT) && m_sockFestival.waitForReadyRead(SERVER_IO_TIMEOUT)) {

#ifdef DEBUG_SERVER_IO
		qDebug("Read Ready... Data:");
#endif

		QString strResult;
		do {
			strResult += QString::fromUtf8(m_sockFestival.read(SERVER_IO_BLOCK_MAX));
		} while ((!strResult.contains(QLatin1String("ft_StUfF_key"))) && (m_sockFestival.waitForReadyRead(SERVER_IO_TIMEOUT)));

#ifdef DEBUG_SERVER_IO
		qDebug("%s", strResult.toUtf8().data());
#endif

		lstResultLines = strResult.split(QChar('\n'));
	}

	m_tmrAutoDisconnect.start(SERVER_AUTO_DISCONNECT_TIME);
	return lstResultLines;
}

// ----------------------------------------------------------------------------

void QtSpeech_asyncServerIO::asyncReadVoices()
{
	if (connectToServer()) {
		QStringList lstResultLines = sendCommand("(voice.list)");

		// Example response:
		// LP
		// (cmu_us_awb_cg cmu_us_slt_arctic_hts cmu_us_rms_cg cmu_us_clb_arctic_clunits ked_diphone cmu_us_slt_arctic_clunits cmu_us_jmk_arctic_clunits cmu_us_rms_arctic_clunits en1_mbrola kal_diphone don_diphone rab_diphone us2_mbrola us3_mbrola cmu_us_awb_arctic_clunits us1_mbrola cmu_us_bdl_arctic_clunits el_diphone)
		// ft_StUfF_keyOK

		if ((lstResultLines.count() >= 2) &&
			(lstResultLines.at(0).trimmed().compare("LP") == 0)) {
			QString strVoices = lstResultLines.at(1);
			strVoices.remove(QChar('('));
			strVoices.remove(QChar(')'));
			QStringList lstVoices = strVoices.split(QChar(' '));
			QtSpeech::Private::lstVoiceNames.clear();
			for (int ndx = 0; ndx < lstVoices.size(); ++ndx) {
				QtSpeech::VoiceName aVoice = { QtSpeech::Private::VoiceId.arg(lstVoices.at(ndx)), "Unknown", "" };
				int nFoundNdx = -1;
				for (int ndxCommonVoice = 0; ((nFoundNdx == -1) && (!QtSpeech::Private::commonVoiceNames[ndxCommonVoice].isEmpty())); ++ndxCommonVoice) {
					if (aVoice.id.compare(QtSpeech::Private::commonVoiceNames[ndxCommonVoice].id) == 0) nFoundNdx = ndxCommonVoice;
				}
				if (nFoundNdx != -1) {
					QtSpeech::Private::lstVoiceNames << QtSpeech::Private::commonVoiceNames[nFoundNdx];
				} else {
					QtSpeech::Private::lstVoiceNames << aVoice;
				}
			}
		}
		emit operationSucceeded();
	} else {
		emit operationFailed();
	}

	emit operationComplete();
}

void QtSpeech_asyncServerIO::say(const QString &strText)
{
	if (connectToServer()) {
		QString strEscText = strText;
		strEscText.remove(QChar('\"'));
		sendCommand(QString("(SayText \"%1\")").arg(strEscText));
		emit operationSucceeded();
	} else {
		emit operationFailed();
	}

	emit operationComplete();
}

#endif	// USE_FESTIVAL_SERVER

// ============================================================================

} // namespace QtSpeech_v1
