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

	Copyright (C) 2014-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#include <QtCore>
#include <QtSpeech>
#include <QtSpeech_unx.h>
#include <festival.h>
#include <QTimer>

namespace QtSpeech_v2 { // API v2.0

// ============================================================================

//#define DEBUG_SERVER_IO
#define SERVER_IO_TIMEOUT 3000				// Timeout in msec for read, write, connect, etc
#define SERVER_IO_BLOCK_MAX 65536l			// Maximum number of read/write bytes per transfer

// ============================================================================

namespace {
	const QString constr_VoiceId = QString("(voice_%1)");

	const QtSpeech::TVoiceName convn_DefaultVoiceName = { "rab_diphone", "British English Male residual LPC, diphone ", "en" };

	// Internal names are those supplied without our custom build of Festival:
	const QtSpeech::TVoiceName convnarr_internalVoiceNames[] =
	{
		{ "cmu_us_awb_cg", "Scottish English Male (with US frontend) clustergen", "en" },
		{ "cmu_us_slt_arctic_hts", "American English Female, HTS", "en" },
		{ "cmu_us_rms_cg", "American English Male using clustergen", "en" },
		{ "kal_diphone", "American English Male residual LPC diphone", "en" },
		{ "rab_diphone", "British English Male residual LPC, diphone ", "en" },
		{ "", "", "" }
	};

	// Common names are ones likely to be encountered on a server used to
	//		resolve a human readable description and language identifiers
	//		when running in client/server mode:
	const QtSpeech::TVoiceName convnarr_commonVoiceNames[] =
	{
		{ "cmu_us_awb_cg", "Scottish English Male (with US frontend) clustergen", "en" },
		{ "cmu_us_slt_arctic_hts", "American English Female, HTS", "en" },
		{ "cmu_us_rms_cg", "American English Male using clustergen", "en" },
		{ "cmu_us_clb_artic_clunits", "English (female)", "en" },
		{ "ked_diphone", "English (male)", "en" },
		{ "cmu_us_jmk_arctic_clunits", "English (male)", "en" },
		{ "cmu_us_rms_arctic_clunits", "English (male)", "en" },
		{ "en1_mbrola", "English (male)", "en" },
		{ "kal_diphone", "American English Male residual LPC diphone", "en" },
		{ "don_diphone", "English (male)", "en" },
		{ "rab_diphone", "British English Male residual LPC, diphone ", "en" },
		{ "us2_mbrola", "English (male)", "en" },
		{ "us3_mbrola", "English (male)", "en" },
		{ "cmu_us_awb_arctic_clunits", "English (male)", "en" },
		{ "us1_mbrola", "English (male)", "en" },
		{ "cmu_us_bdl_arctic_clunits", "English (male)", "en" },
		{ "el_diphone", "Spanish (male)", "es" },
		{ "", "", "" }
	};
}

// ============================================================================

// internal data
class QtSpeech::Private
{
public:
	Private() {}

};

// ============================================================================

// global data
class QtSpeech_GlobalData : public QtSpeech_asyncServerIOMonitor
{
public:
	QtSpeech_GlobalData()
	{
		qRegisterMetaType<TAsyncTalkingObject>("TAsyncTalkingObject");
	}
	virtual ~QtSpeech_GlobalData()
	{
#ifdef USE_FESTIVAL_SERVER
		if (!m_pAsyncServerIO.isNull()) {
			disconnect(m_pAsyncServerIO.data(), 0, 0, 0);			// Disconnect everything to prevent event firing on dead object
			delete m_pAsyncServerIO.data();
		}
#endif
	}

	bool createWorkerThread()
	{
		bool bCreatedWorker = false;

		if (m_pSpeechThread.isNull()) {
			m_pSpeechThread = new QThread;
			m_pSpeechThread->start();
		}

		if (m_pSpeechTalker_th.isNull()) {
			bCreatedWorker = true;
			m_pSpeechTalker_th = new QtSpeech_th();
			m_pSpeechTalker_th->moveToThread(m_pSpeechThread);
			connect(m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SLOT(en_beginTalking()), Qt::QueuedConnection);
			connect(m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SLOT(en_doneTalking(bool)), Qt::QueuedConnection);
			connect(m_pSpeechTalker_th.data(), SIGNAL(finished()), m_pSpeechTalker_th.data(), SLOT(deleteLater()), Qt::QueuedConnection);
		}

		return bCreatedWorker;
	}

	QPointer<QThread> m_pSpeechThread;
	QPointer<QtSpeech_th> m_pSpeechTalker_th;			// Worker object for talking

	QtSpeech::TVoiceName m_vnSelectedVoiceName;
	QtSpeech::TVoiceName m_vnRequestedVoiceName;
	QtSpeech::TVoiceNamesList m_lstVoiceNames;

#ifdef USE_FESTIVAL_SERVER
	QPointer<QtSpeech_asyncServerIO> m_pAsyncServerIO;
#endif

	bool serverSupported();								// True if QtSpeech library compiled with server support
	bool serverConnected();								// True if currently connected to a speech server
	bool connectToServer(const QString &strHostname, int nPortNumber);
	void disconnectFromServer();

	void setVoice();

protected slots:
	virtual void en_lostServer();
} g_QtSpeechGlobal;

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
bool QtSpeech_th::m_bInit = false;

QtSpeech_th::QtSpeech_th(QObject *pParent)
	:	QObject(pParent),
		err(""),
		has_error(false),
		m_bAmTalking(false)
{
	connect(this, SIGNAL(sayNext(bool)), this, SLOT(en_sayNext(bool)), Qt::QueuedConnection);
}

void QtSpeech_th::doInit()
{
	if (!m_bInit) {
		festival_initialize(true, FESTIVAL_HEAP_SIZE);
		m_bInit = true;
	}
}

void QtSpeech_th::say(TAsyncTalkingObject aTalkingObject)
{
	doInit();
	m_lstTalkingObjects.append(aTalkingObject);
	if (!m_bAmTalking) en_sayNext(true);
}

void QtSpeech_th::eval(QString strExpr)
{
	try {
		doInit();
		has_error = false;
		EST_String est_voice(strExpr.toUtf8());
		SysCall(festival_eval_command(est_voice), QtSpeech::LogicError);
	}
	catch (const QtSpeech::LogicError &e) {
		has_error = true;
		err = e;
	}
	emit finished();
}

void QtSpeech_th::en_sayNext(bool bInitialSay)
{
	// Do this here rather than at the bottom of the function so
	//		that additional text can get queued from processing
	//		more say() events.  Otherwise, we'll think we are at
	//		the last message when more are still in the eventLoop:
	if (!bInitialSay) {
		emit doneTalking(m_lstTalkingObjects.isEmpty());
	} else {
		if (!m_lstTalkingObjects.isEmpty()) emit beginTalking();
	}

	if (m_lstTalkingObjects.isEmpty()) {
		m_bAmTalking = false;
		emit finished();
		return;
	}

	m_bAmTalking = true;

	try {
		has_error = false;
		EST_String est_text(m_lstTalkingObjects.at(0).m_strText.toUtf8());
		SysCall(festival_say_text(est_text), QtSpeech::LogicError);
	}
	catch (const QtSpeech::LogicError &e) {
		has_error = true;
		err = e;
	}

	if (m_lstTalkingObjects.at(0).hasNotificationSlot()) {
		QTimer::singleShot(0, m_lstTalkingObjects.at(0).m_pObject, m_lstTalkingObjects.at(0).m_pSlot);		// Note: singleShot IS a QueuedConnection!
	}
	m_lstTalkingObjects.pop_front();

	emit sayNext(false);
}

void QtSpeech_th::clearQueue()
{
	m_lstTalkingObjects.clear();
	if (!m_bAmTalking) emit finished();		// If we're already talking, the talking loop will emit the finished when it realizes we're done with things to say...  Otherwise we should emit it
}

// ============================================================================

// implementation
QtSpeech::QtSpeech(QObject *pParent)
	:	QObject(pParent),
		d(new Private)
{
	if (convn_DefaultVoiceName.isEmpty()) {
		qDebug("%s", QString("%1No default voice in system").arg(Where).toUtf8().data());
	}

	g_QtSpeechGlobal.m_vnRequestedVoiceName = convn_DefaultVoiceName;
}

QtSpeech::QtSpeech(const TVoiceName &aVoiceName, QObject *pParent)
	:	QObject(pParent),
		d(new Private)
{
	TVoiceName theVoiceName = aVoiceName;

	if (theVoiceName.isEmpty()) theVoiceName = convn_DefaultVoiceName;

	if (theVoiceName.isEmpty()) {
		qDebug("%s", QString("%1No default voice in system").arg(Where).toUtf8().data());
	}

	g_QtSpeechGlobal.m_vnRequestedVoiceName = theVoiceName;
}

QtSpeech::~QtSpeech()
{
	delete d;
}

// ----------------------------------------------------------------------------

const QtSpeech::TVoiceName &QtSpeech::voiceName() const
{
	if (!g_QtSpeechGlobal.m_vnSelectedVoiceName.isEmpty()) return g_QtSpeechGlobal.m_vnSelectedVoiceName;
	return g_QtSpeechGlobal.m_vnRequestedVoiceName;
}

void QtSpeech::setVoiceName(const TVoiceName &aVoiceName)
{
	TVoiceName theVoiceName = aVoiceName;

	if (theVoiceName.isEmpty()) theVoiceName = convn_DefaultVoiceName;
	if (!theVoiceName.isEmpty()) g_QtSpeechGlobal.m_vnRequestedVoiceName = theVoiceName;
}

QtSpeech::TVoiceNamesList QtSpeech::voices()
{
	if (!g_QtSpeechGlobal.m_lstVoiceNames.isEmpty()) return g_QtSpeechGlobal.m_lstVoiceNames;

	// Set default to our internal names in case we fail to setup client/server or aren't using the server:
	g_QtSpeechGlobal.m_lstVoiceNames.clear();
	for (int ndxInternalVoice = 0; (!convnarr_internalVoiceNames[ndxInternalVoice].isEmpty()); ++ndxInternalVoice) {
		g_QtSpeechGlobal.m_lstVoiceNames << convnarr_internalVoiceNames[ndxInternalVoice];
	}

#ifdef USE_FESTIVAL_SERVER
	if (!g_QtSpeechGlobal.m_pAsyncServerIO.isNull()) {
		QEventLoop el;
		connect(g_QtSpeechGlobal.m_pAsyncServerIO.data(), SIGNAL(readVoicesComplete()), &el, SLOT(quit()), Qt::QueuedConnection);
		g_QtSpeechGlobal.m_pAsyncServerIO->readVoices();
		el.exec(QEventLoop::ExcludeUserInputEvents);
	}
#endif

	return g_QtSpeechGlobal.m_lstVoiceNames;
}

// ----------------------------------------------------------------------------

bool QtSpeech::canSpeak() const
{
	return true;
}

bool QtSpeech::isTalking()
{
	return g_QtSpeechGlobal.isTalking();
}

bool QtSpeech::serverSupported()
{
	return g_QtSpeechGlobal.serverSupported();
}

bool QtSpeech::serverConnected()
{
	return g_QtSpeechGlobal.serverConnected();
}

bool QtSpeech::connectToServer(const QString &strHostname, int nPortNumber)
{
	return g_QtSpeechGlobal.connectToServer(strHostname, nPortNumber);
}

void QtSpeech::disconnectFromServer()
{
	g_QtSpeechGlobal.disconnectFromServer();
}

// ----------------------------------------------------------------------------

void QtSpeech::tell(const QString &strText, QObject *pObject, const char *pSlot) const
{
	g_QtSpeechGlobal.setVoice();

#ifdef USE_FESTIVAL_SERVER
	if (serverConnected()) {
		connect(g_QtSpeechGlobal.m_pAsyncServerIO.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), Qt::UniqueConnection);
		connect(g_QtSpeechGlobal.m_pAsyncServerIO.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), Qt::UniqueConnection);
		g_QtSpeechGlobal.m_pAsyncServerIO->say(TAsyncTalkingObject(strText, pObject, pSlot));
		return;
	}
#endif

	if (g_QtSpeechGlobal.createWorkerThread()) {
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), Qt::QueuedConnection);
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), Qt::QueuedConnection);
	}

	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "say", Qt::QueuedConnection, Q_ARG(TAsyncTalkingObject, TAsyncTalkingObject(strText, pObject, pSlot)));
}

void QtSpeech::say(const QString &strText) const
{
	g_QtSpeechGlobal.setVoice();

#ifdef USE_FESTIVAL_SERVER
	if (serverConnected()) {
		QEventLoop el;
		connect(g_QtSpeechGlobal.m_pAsyncServerIO.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), Qt::UniqueConnection);
//		connect(g_QtSpeechGlobal.m_pAsyncServerIO.data(), SIGNAL(doneTalking(bool)), &el, SLOT(quit()), Qt::QueuedConnection);
		connect(g_QtSpeechGlobal.m_pAsyncServerIO.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), Qt::UniqueConnection);
		g_QtSpeechGlobal.m_pAsyncServerIO->say(TAsyncTalkingObject(strText, &el, SLOT(quit())));
		el.exec(QEventLoop::ExcludeUserInputEvents);
		return;
	}
#endif

	if (g_QtSpeechGlobal.createWorkerThread()) {
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), Qt::QueuedConnection);
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), Qt::QueuedConnection);
	}

	QEventLoop el;
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "say", Qt::QueuedConnection, Q_ARG(TAsyncTalkingObject, TAsyncTalkingObject(strText)));
	el.exec(QEventLoop::ExcludeUserInputEvents);
}

void QtSpeech::clearQueue()
{
	emit clearingQueue();

#ifdef USE_FESTIVAL_SERVER
	if (serverConnected()) {
		g_QtSpeechGlobal.m_pAsyncServerIO->clearQueue();
		return;
	}
#endif

	if (g_QtSpeechGlobal.createWorkerThread()) {
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), Qt::QueuedConnection);
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), Qt::QueuedConnection);
	}

	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "clearQueue", Qt::QueuedConnection);
}

// ============================================================================

bool QtSpeech_GlobalData::serverSupported()
{
#ifdef USE_FESTIVAL_SERVER
	return true;
#else
	return false;
#endif
}

bool QtSpeech_GlobalData::serverConnected()
{
#ifdef USE_FESTIVAL_SERVER
	if (!m_pAsyncServerIO.isNull()) {
		return m_pAsyncServerIO->isConnected();
	}
#endif

	return false;
}

bool QtSpeech_GlobalData::connectToServer(const QString &strHostname, int nPortNumber)
{
#ifdef USE_FESTIVAL_SERVER
	disconnectFromServer();		// Disconnect from any existing server
	m_pAsyncServerIO = new QtSpeech_asyncServerIO(strHostname, nPortNumber);
	connect(m_pAsyncServerIO.data(), SIGNAL(lostServer()), this, SLOT(en_lostServer()));
	connect(m_pAsyncServerIO.data(), SIGNAL(beginTalking()), this, SLOT(en_beginTalking()));
	connect(m_pAsyncServerIO.data(), SIGNAL(doneTalking(bool)), this, SLOT(en_doneTalking(bool)));
	return serverConnected();
#else
	Q_UNUSED(strHostname);
	Q_UNUSED(nPortNumber);
	return false;
#endif
}

void QtSpeech_GlobalData::disconnectFromServer()
{
#ifdef USE_FESTIVAL_SERVER
	if (!m_pAsyncServerIO.isNull()) {
		disconnect(m_pAsyncServerIO.data(), 0, 0, 0);			// Disconnect everything to prevent event firing on dead object
		delete m_pAsyncServerIO.data();
#ifdef DEBUG_SERVER_IO
		qDebug("Switching to internal Festival...");
#endif
	}

	// If we switch from server to internal OR internal to server, we need to reselect the last requested
	//		voice on the new festival:
	if (m_vnRequestedVoiceName.isEmpty()) {
		m_vnRequestedVoiceName = m_vnSelectedVoiceName;
	}
	m_vnSelectedVoiceName.clear();								// Must needs reselect voice
#endif
}

void QtSpeech_GlobalData::setVoice()
{
	QtSpeech::TVoiceName theVoice = m_vnRequestedVoiceName;
	m_vnRequestedVoiceName.clear();

	if (theVoice.isEmpty()) return;
	if (m_vnSelectedVoiceName == theVoice) return;

	m_vnSelectedVoiceName = theVoice;

#ifdef DEBUG_SERVER_IO
	qDebug("Setting voice to: %s", theVoice.id.toUtf8().data());
#endif

#ifdef USE_FESTIVAL_SERVER
	if (serverConnected()) {
		QEventLoop el;
		connect(m_pAsyncServerIO.data(), SIGNAL(setVoiceComplete()), &el, SLOT(quit()), Qt::QueuedConnection);
		m_pAsyncServerIO->setVoice(theVoice);
		el.exec(QEventLoop::ExcludeUserInputEvents);
		return;
	}
#endif

	if (m_pSpeechThread.isNull()) {
		m_pSpeechThread = new QThread;
		m_pSpeechThread->start();
	}

	QEventLoop el;
	QtSpeech_th th;
	th.moveToThread(m_pSpeechThread);
	connect(&th, SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(&th, "eval", Qt::QueuedConnection, Q_ARG(QString, constr_VoiceId.arg(theVoice.id)));
	el.exec(QEventLoop::ExcludeUserInputEvents);
}

void QtSpeech_GlobalData::en_lostServer()
{
#ifdef USE_FESTIVAL_SERVER
	if (!m_pAsyncServerIO.isNull()) {
#ifdef DEBUG_SERVER_IO
		qDebug("Lost connection to Festival Server...");
#endif
		disconnectFromServer();							// Delete our connectivity object and switch to internal Festival
	}
#endif
}

// ============================================================================

#ifdef USE_FESTIVAL_SERVER

QtSpeech_asyncServerIO::QtSpeech_asyncServerIO(const QString &strHostname, int nPortNumber, QObject *pParent)
	:	QObject(pParent),
		m_strHostname(strHostname),
		m_nPortNumber(nPortNumber),
		m_bAmTalking(false),
		m_bSendCommandInProgress(false)
{
	connect(&m_sockFestival, SIGNAL(readyRead()), this, SLOT(en_readyRead()));
	connect(&m_sockFestival, SIGNAL(disconnected()), this, SIGNAL(lostServer()));
	connectToServer();
}

QtSpeech_asyncServerIO::~QtSpeech_asyncServerIO()
{
	disconnectFromServer();
}

bool QtSpeech_asyncServerIO::isConnected()
{
	return (m_sockFestival.state() == QAbstractSocket::ConnectedState);
}

bool QtSpeech_asyncServerIO::connectToServer()
{
	if (isConnected()) return true;

	m_sockFestival.connectToHost(m_strHostname, m_nPortNumber);

#ifdef DEBUG_SERVER_IO
	qDebug("Connecting to Festival Server...");
#endif

	if (!m_sockFestival.waitForConnected(SERVER_IO_TIMEOUT)) {
#ifdef DEBUG_SERVER_IO
		qDebug("Failed to connect");
#endif
		return false;
	}

#ifdef DEBUG_SERVER_IO
	qDebug("Connected");
#endif

	return true;
}

void QtSpeech_asyncServerIO::disconnectFromServer()
{
#ifdef DEBUG_SERVER_IO
	qDebug("Disconnecting from Festival Server...");
#endif

	m_sockFestival.close();
}

QStringList QtSpeech_asyncServerIO::sendCommand(const QString &strCommand, bool bWaitForReply)
{
	if (!isConnected()) return QStringList();

	m_bSendCommandInProgress = true;

	m_sockFestival.readAll();			// Flush incoming
	m_sockFestival.write(QString("%1\n").arg(strCommand).toUtf8());

#ifdef DEBUG_SERVER_IO
	qDebug("Wrote \"%s\" Command", strCommand.toUtf8().data());
#endif

	QStringList lstResultLines;

	if (m_sockFestival.waitForBytesWritten(SERVER_IO_TIMEOUT) &&
		(bWaitForReply && m_sockFestival.waitForReadyRead(SERVER_IO_TIMEOUT))) {

#ifdef DEBUG_SERVER_IO
		qDebug("Read Ready (sendCommand)... Data:");
#endif

		QString strResult;
		do {
			strResult += QString::fromUtf8(m_sockFestival.read(SERVER_IO_BLOCK_MAX));
		} while ((!strResult.contains(QLatin1String("ft_StUfF_key"))) &&
				 ((m_sockFestival.bytesAvailable() != 0) ||
				  (bWaitForReply && m_sockFestival.waitForReadyRead(SERVER_IO_TIMEOUT))));

#ifdef DEBUG_SERVER_IO
		qDebug("%s", strResult.toUtf8().data());
#endif

		lstResultLines = strResult.split(QChar('\n'));
	}

	m_bSendCommandInProgress = false;

	return lstResultLines;
}

void QtSpeech_asyncServerIO::en_readyRead()
{
	if (m_bAmTalking) {
#ifdef DEBUG_SERVER_IO
		qDebug("Read Ready (asyncReadyTalking)... Data:");
#endif

		QString strResult;
		do {
			strResult += QString::fromUtf8(m_sockFestival.read(SERVER_IO_BLOCK_MAX));
		} while ((!strResult.contains(QLatin1String("ft_StUfF_key"))) &&
				 ((m_sockFestival.bytesAvailable() != 0) || (m_sockFestival.waitForReadyRead(SERVER_IO_TIMEOUT))));

#ifdef DEBUG_SERVER_IO
		qDebug("%s", strResult.toUtf8().data());
#endif

		m_bAmTalking = false;

		emit doneTalking(m_lstTalkingObjects.size() <= 1);
		if (!m_lstTalkingObjects.isEmpty()) {
			if (m_lstTalkingObjects.at(0).hasNotificationSlot()) {
				QTimer::singleShot(0, m_lstTalkingObjects.at(0).m_pObject, m_lstTalkingObjects.at(0).m_pSlot);
			}
			m_lstTalkingObjects.pop_front();
			en_sayNext();
		}
	} else {
		if (!m_bSendCommandInProgress) {
			// Otherwise purge buffer because talking may have been stoped via a queue clear:
			QString strData = QString::fromUtf8(m_sockFestival.readAll());
#ifdef DEBUG_SERVER_IO
			qDebug("Read Ready (asyncReadyIdle)... Data:\n%s", strData.toUtf8().data());
#else
			Q_UNUSED(strData);
#endif
		}
	}
}

// ----------------------------------------------------------------------------

void QtSpeech_asyncServerIO::readVoices()
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
			g_QtSpeechGlobal.m_lstVoiceNames.clear();
			for (int ndx = 0; ndx < lstVoices.size(); ++ndx) {
				QtSpeech::TVoiceName aVoice = { lstVoices.at(ndx), "Unknown", "" };
				int nFoundNdx = -1;
				for (int ndxCommonVoice = 0; ((nFoundNdx == -1) && (!convnarr_commonVoiceNames[ndxCommonVoice].isEmpty())); ++ndxCommonVoice) {
					if (aVoice.id.compare(convnarr_commonVoiceNames[ndxCommonVoice].id) == 0) nFoundNdx = ndxCommonVoice;
				}
				if (nFoundNdx != -1) {
					g_QtSpeechGlobal.m_lstVoiceNames << convnarr_commonVoiceNames[nFoundNdx];
				} else {
					g_QtSpeechGlobal.m_lstVoiceNames << aVoice;
				}
			}
		}
	}

	emit readVoicesComplete();
}

void QtSpeech_asyncServerIO::say(const TAsyncTalkingObject &aTalkingObject)
{
	if (m_lstTalkingObjects.isEmpty() && !m_bAmTalking) emit beginTalking();
	m_lstTalkingObjects.append(aTalkingObject);
	if (!m_bAmTalking) en_sayNext();
}

void QtSpeech_asyncServerIO::en_sayNext()
{
	if (m_lstTalkingObjects.isEmpty()) return;

	if (connectToServer()) {
		m_sockFestival.readAll();			// Flush incoming
		QString strEscText = m_lstTalkingObjects.at(0).m_strText;
		strEscText.remove(QChar('\"'));
		m_bAmTalking = true;				// Set flag to announce doneTalking
		sendCommand(QString("(SayText \"%1\")").arg(strEscText), false);
	}
}

void QtSpeech_asyncServerIO::clearQueue()
{
	if (!m_bAmTalking) {
		// If we aren't talking, clear everything:
		m_lstTalkingObjects.clear();
	} else {
		// Otherwise, clear all but the current object:
		if (!m_lstTalkingObjects.isEmpty()) {
			TAsyncTalkingObject aTemp = m_lstTalkingObjects.at(0);
			m_lstTalkingObjects.clear();
			m_lstTalkingObjects.append(aTemp);
		}
	}
}

void QtSpeech_asyncServerIO::setVoice(const QtSpeech::TVoiceName &aVoice)
{
	if (connectToServer()) {
		if (!aVoice.isEmpty()) sendCommand(constr_VoiceId.arg(aVoice.id));
	}

	emit setVoiceComplete();
}

#endif	// USE_FESTIVAL_SERVER

// ============================================================================

} // namespace QtSpeech_v2
