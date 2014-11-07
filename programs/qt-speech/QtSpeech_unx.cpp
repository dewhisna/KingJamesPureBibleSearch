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

namespace QtSpeech_v1 { // API v1.0

// ============================================================================

#define DEBUG_SERVER_IO
#define SERVER_IO_TIMEOUT 3000				// Timeout in msec for read, write, connect, etc
#define SERVER_IO_BLOCK_MAX 65536l			// Maximum number of read/write bytes per transfer
#define SERVER_AUTO_DISCONNECT_TIME 10000	// TCP Auto Disconnect time from server in msec

// ============================================================================

// internal data
class QtSpeech::Private {
public:
	Private()
		:onFinishSlot(0L) {}

	VoiceName name;
	static const QString VoiceId;
	static bool serverRunning() { return (pidServer != 0); }

	const char * onFinishSlot;
	QPointer<QObject> onFinishObj;

	static QPointer<QThread> speechThread;
	static const VoiceName DefaultVoiceName;
	static VoiceNames lstVoiceNames;
	static const VoiceName commonVoiceNames[];
	static pid_t pidServer;
};
QPointer<QThread> QtSpeech::Private::speechThread = 0L;
const QString QtSpeech::Private::VoiceId = QString("(voice_%1)");
const QtSpeech::VoiceName QtSpeech::Private::DefaultVoiceName = {QtSpeech::Private::VoiceId.arg("cmu_us_slt_arctic_hts"), "English (Male)", "en"};
QtSpeech::VoiceNames QtSpeech::Private::lstVoiceNames;
pid_t QtSpeech::Private::pidServer = 0;

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

void QtSpeech_th::startServer()
{
	if (QtSpeech::Private::pidServer != 0) return;

	doInit();

	emit serverStarted();

	pid_t pidFork = fork();
	if (pidFork) {
		// For Parent, save the PID so we can later kill it:
		QtSpeech::Private::pidServer = pidFork;
		return;
	}

	// This doesn't exit!
	festival_start_server(FESTIVAL_DEFAULT_PORT);
}

void QtSpeech_th::stopServer()
{
	if (QtSpeech::Private::pidServer == 0) return;

	kill(QtSpeech::Private::pidServer, SIGTERM);
	QtSpeech::Private::pidServer = 0;
	emit serverStopped();
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

	Private::lstVoiceNames.clear();

#ifdef USE_FESTIVAL_SERVER

	// Set default in case we fail to setup client/server:
	Private::lstVoiceNames << Private::DefaultVoiceName;

	if (Private::speechThread.isNull()) {
		Private::speechThread = new QThread;
		Private::speechThread->start();
	}

	QtSpeech_th * th = new QtSpeech_th(Private::DefaultVoiceName);
	th->moveToThread(Private::speechThread);
	connect(th, SIGNAL(finished()), th, SLOT(deleteLater()), Qt::QueuedConnection);
	QtSpeech_asyncServerIO asyncReadVoices(FESTIVAL_DEFAULT_PORT);
	QEventLoop el;
	connect(th, SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	connect(&asyncReadVoices, SIGNAL(readFailed()), &el, SLOT(quit()), Qt::QueuedConnection);
	connect(&asyncReadVoices, SIGNAL(readComplete()), &el, SLOT(quit()), Qt::QueuedConnection);
	connect(th, SIGNAL(serverStarted()), &asyncReadVoices, SLOT(asyncReadVoices()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(th, "startServer", Qt::QueuedConnection);
	el.exec(QEventLoop::ExcludeUserInputEvents);

	QMetaObject::invokeMethod(th, "stopServer", Qt::QueuedConnection);
	el.exec(QEventLoop::ExcludeUserInputEvents);

#else
	for (int ndxCommonVoice = 0; (!Private::commonVoiceNames[ndxCommonVoice].isEmpty()); ++ndxCommonVoice) {
		Private::lstVoiceNames << Private::commonVoiceNames[ndxCommonVoice];
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
	Q_UNUSED(nPort);
	return false;
}

bool QtSpeech::stopServer()
{
	return false;
}

// ----------------------------------------------------------------------------

void QtSpeech::tell(QString text) const {
    tell(text, 0L,0L);
}

void QtSpeech::tell(QString text, QObject * obj, const char * slot) const
{
    if (!d->speechThread) {
        d->speechThread = new QThread;
        d->speechThread->start();
    }

    d->onFinishObj = obj;
    d->onFinishSlot = slot;
    if (obj && slot)
        connect(const_cast<QtSpeech *>(this), SIGNAL(finished()), obj, slot);

	QtSpeech_th * th = new QtSpeech_th(name());
    th->moveToThread(d->speechThread);
    connect(th, SIGNAL(finished()), this, SIGNAL(finished()), Qt::QueuedConnection);
    connect(th, SIGNAL(finished()), th, SLOT(deleteLater()), Qt::QueuedConnection);
    QMetaObject::invokeMethod(th, "say", Qt::QueuedConnection, Q_ARG(QString,text));
}

void QtSpeech::say(QString text) const
{
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

bool QtSpeech_asyncServerIO::connectToServer(int nPortNumber)
{
	if (m_sockFestival.state() == QAbstractSocket::ConnectedState) {
		m_tmrAutoDisconnect.start(SERVER_AUTO_DISCONNECT_TIME);
		return true;
	}

	m_tmrAutoDisconnect.stop();

	m_sockFestival.connectToHost("localhost", ((nPortNumber != -1) ? nPortNumber : m_nPortNumber));

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

		emit readComplete();
	} else {
		emit readFailed();
	}
}

#endif	// USE_FESTIVAL_SERVER

// ============================================================================

} // namespace QtSpeech_v1
