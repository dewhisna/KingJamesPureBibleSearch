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

	Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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

// ============================================================================

#include <QtCore>
#include <QtSpeech>
#include <QtSpeech_mac.h>

// ============================================================================

namespace QtSpeech_v2 { // API v2.0

namespace {
	const QString constr_VoiceId = QString("%1");

}

// some defines for throwing exceptions
#define Where QString("%1:%2:").arg(__FILE__).arg(__LINE__)
#define SysCall(x,e) {\
	OSErr ok = x;\
	if (ok != noErr) {\
		QString msg = #e;\
		msg += ":"+QString(__FILE__);\
		msg += ":"+QString::number(__LINE__)+":"+#x;\
		throw e(msg);\
	}\
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
		try {

		} catch (...) { /* TODO : Add saving of error message */ }
	}
	virtual ~QtSpeech_GlobalData()
	{

	}

	bool createWorkerThread()
	{
		bool bCreatedWorker = false;

		QtSpeech::TVoiceName theVoice = m_vnRequestedVoiceName;
		m_vnRequestedVoiceName.clear();

		if (!theVoice.isEmpty()) {
			m_vnSelectedVoiceName = theVoice;
		}

		if ((!m_pSpeechTalker_th.isNull()) &&
			(!theVoice.isEmpty()) &&
			(theVoice != m_vnSelectedVoiceName)) {
			delete m_pSpeechTalker_th.data();
			m_pSpeechTalker_th.clear();
		}

		if (m_pSpeechTalker_th.isNull()) {
			bCreatedWorker = true;
			m_pSpeechTalker_th = new QtSpeech_th(m_vnSelectedVoiceName);
			connect(m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SLOT(en_beginTalking()), Qt::QueuedConnection);
			connect(m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SLOT(en_doneTalking(bool)), Qt::QueuedConnection);
			connect(m_pSpeechTalker_th.data(), SIGNAL(finished()), m_pSpeechTalker_th.data(), SLOT(deleteLater()), Qt::QueuedConnection);
		}

		return bCreatedWorker;
	}

	QPointer<QtSpeech_th> m_pSpeechTalker_th;			// Worker object for talking

	QtSpeech::TVoiceName m_vnSelectedVoiceName;
	QtSpeech::TVoiceName m_vnRequestedVoiceName;
	QtSpeech::TVoiceNamesList m_lstVoiceNames;

	static void speechFinished(SpeechChannel nChannel, SpeechDoneUPP_ARG2 refCon);

protected slots:
	virtual void en_lostServer() { }

} g_QtSpeechGlobal;

// ============================================================================

static QtSpeech::TVoiceName defaultVoiceName()
{
	QtSpeech::TVoiceName aVoiceName;

	try {
		VoiceDescription desc;
		SysCall( GetVoiceDescription(NULL, &desc, sizeof(VoiceDescription)), QtSpeech::InitError);
		aVoiceName.id = constr_VoiceId.arg(desc.voice.id);
		aVoiceName.name = QString::fromLatin1(reinterpret_cast<const char *>(&desc.name[1]), desc.name[0]);			// Convert from Pascal string
		// TODO : Set aVoiceName.lang
	} catch (...) { /* TODO : Add saving of error message */ }

	return aVoiceName;
}

// ============================================================================

bool QtSpeech_th::m_bInit = false;

QtSpeech_th::QtSpeech_th(const QtSpeech::TVoiceName &aVoiceName, QObject *pParent)
	:	QObject(pParent),
		err(""),
		has_error(false),
		m_bAmTalking(false)
{
	doInit();

	try {
		// SetVoice() handled here implicitly:
		QtSpeech::TVoiceName theVoice = aVoiceName;
		if (theVoice.isEmpty()) theVoice = defaultVoiceName();

		SInt16 nCount;
		VoiceSpec voice;
		VoiceSpec *pVoice = NULL;
		SysCall( CountVoices(&nCount), QtSpeech::InitError);
		for (int i=1; i<= nCount; ++i) {
			SysCall( GetIndVoice(i, &voice), QtSpeech::InitError);
			QString strID = constr_VoiceId.arg(voice.id);
			if (strID == theVoice.id) {
				pVoice = &voice;
				break;
			}
		}

		m_pDoneCall = NewSpeechDoneUPP(&QtSpeech_GlobalData::speechFinished);
		SysCall( NewSpeechChannel(pVoice, &m_nChannel), QtSpeech::InitError);
		SysCall( SetSpeechInfo(m_nChannel, soSpeechDoneCallBack, (void *)m_pDoneCall), QtSpeech::InitError);
	} catch (...) { /* TODO : Add saving of error message */ }
	connect(this, SIGNAL(sayNext(bool)), this, SLOT(en_sayNext(bool)), Qt::QueuedConnection);
}

QtSpeech_th::~QtSpeech_th()
{
	Q_ASSERT(m_nChannel);

	try {
		SysCall( StopSpeech(m_nChannel), QtSpeech::CloseError);
		SysCall( DisposeSpeechChannel(m_nChannel), QtSpeech::CloseError);
		DisposeSpeechDoneUPP(m_pDoneCall);
	} catch (...) { /* TODO : Add saving of error message */ }
}

void QtSpeech_th::doInit()
{
	if (!m_bInit) {
		// TODO : Additional init operations
		m_bInit = true;
	}
}

void QtSpeech_th::say(TAsyncTalkingObject aTalkingObject)
{
	doInit();
	m_lstTalkingObjects.append(aTalkingObject);
	if (!m_bAmTalking) en_sayNext(true);
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

		CFStringRef cfText = CFStringCreateWithCharacters(0,
								reinterpret_cast<const UniChar *>(m_lstTalkingObjects.at(0).m_strText.unicode()),
								m_lstTalkingObjects.at(0).m_strText.length());

		OSErr ok = SpeakCFString(m_nChannel, cfText, NULL);
		CFRelease(cfText);
		if (ok != noErr) throw QtSpeech::LogicError(Where+"SpeakCFString()");
	}
	catch (const QtSpeech::LogicError &e) {
		has_error = true;
		err = e;
		m_lstTalkingObjects.clear();
		m_bAmTalking = false;
		emit finished();
	}
}

void QtSpeech_th::speechFinished(SpeechChannel nChannel, SpeechDoneUPP_ARG2 refCon)
{
	Q_ASSERT(nChannel == m_nChannel);
	Q_UNUSED(refCon);

	if (!m_lstTalkingObjects.isEmpty()) {		// Note: can be empty if clearQueue() has been called!
		if (m_lstTalkingObjects.at(0).hasNotificationSlot()) {
			QTimer::singleShot(0, m_lstTalkingObjects.at(0).m_pObject, m_lstTalkingObjects.at(0).m_pSlot);		// Note: singleShot IS a QueuedConnection!
		}
		m_lstTalkingObjects.pop_front();
	}

	emit sayNext(false);
}

void QtSpeech_th::clearQueue()
{
	m_lstTalkingObjects.clear();
	if (!m_bAmTalking) {
		emit finished();		// If we're already talking, the talking loop will emit the finished when it realizes we're done with things to say...  Otherwise we should emit it
	} else {
		try {
			SysCall( StopSpeech(m_nChannel), QtSpeech::CloseError);
		} catch (...) { /* TODO : Add saving of error message */ }
	}
}

// ============================================================================

// implementation
QtSpeech::QtSpeech(QObject *pParent)
	:	QObject(pParent),
		d(new Private)
{
	TVoiceName aVoiceName = defaultVoiceName();

	if (aVoiceName.isEmpty()) {
		qDebug("%s", QString("%1No default voice in system").arg(Where).toUtf8().data());
	}

	g_QtSpeechGlobal.m_vnRequestedVoiceName = aVoiceName;
}

QtSpeech::QtSpeech(const TVoiceName &aVoiceName, QObject *pParent)
	:	QObject(pParent),
		d(new Private)
{
	TVoiceName theVoiceName = aVoiceName;

	if (theVoiceName.isEmpty()) theVoiceName = defaultVoiceName();

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

	if (theVoiceName.isEmpty()) theVoiceName = defaultVoiceName();
	if (!theVoiceName.isEmpty()) g_QtSpeechGlobal.m_vnRequestedVoiceName = theVoiceName;
}

QtSpeech::TVoiceNamesList QtSpeech::voices()
{
	if (!g_QtSpeechGlobal.m_lstVoiceNames.isEmpty()) return g_QtSpeechGlobal.m_lstVoiceNames;

	try {
		SInt16 nCount;
//		VoiceDescription desc;
		SysCall( CountVoices(&nCount), LogicError);
//		SysCall( GetVoiceDescription(NULL, &desc, sizeof(VoiceDescription)), LogicError);
		for (int i=1; i<= nCount; ++i) {
			VoiceSpec voice;
			VoiceDescription desc;
			SysCall( GetIndVoice(i, &voice), LogicError);
			SysCall( GetVoiceDescription(&voice, &desc, sizeof(VoiceDescription)), LogicError);
			TVoiceName aVoiceName;
			aVoiceName.id = constr_VoiceId.arg(voice.id);
			aVoiceName.name = QString::fromLatin1(reinterpret_cast<const char *>(&desc.name[1]), desc.name[0]);		// Convert from Pascal string
			// TODO : Set aVoiceName.lang
			g_QtSpeechGlobal.m_lstVoiceNames << aVoiceName;
		}
	} catch (...) { /* TODO : Add saving of error message */ }

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
	return false;
}

bool QtSpeech::serverConnected()
{
	return false;
}

bool QtSpeech::connectToServer(const QString &strHostname, int nPortNumber)
{
	Q_UNUSED(strHostname);
	Q_UNUSED(nPortNumber);
	return false;
}

void QtSpeech::disconnectFromServer()
{

}

// ----------------------------------------------------------------------------

void QtSpeech::tell(const QString &strText, QObject *pObject, const char *pSlot) const
{
	g_QtSpeechGlobal.createWorkerThread();
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "say", Qt::QueuedConnection, Q_ARG(TAsyncTalkingObject, TAsyncTalkingObject(strText, pObject, pSlot)));
}

void QtSpeech::say(const QString &strText) const
{
	g_QtSpeechGlobal.createWorkerThread();
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));

	QEventLoop el;
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "say", Qt::QueuedConnection, Q_ARG(TAsyncTalkingObject, TAsyncTalkingObject(strText)));
	el.exec(QEventLoop::ExcludeUserInputEvents);
}

void QtSpeech::clearQueue()
{
	emit clearingQueue();

	g_QtSpeechGlobal.createWorkerThread();
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "clearQueue", Qt::QueuedConnection);
}

// ============================================================================

void QtSpeech_GlobalData::speechFinished(SpeechChannel nChannel, SpeechDoneUPP_ARG2 refCon)
{
	Q_ASSERT(!g_QtSpeechGlobal.m_pSpeechTalker_th.isNull());
	if (!g_QtSpeechGlobal.m_pSpeechTalker_th.isNull()) {
		g_QtSpeechGlobal.m_pSpeechTalker_th->speechFinished(nChannel, refCon);
	}
}

} // namespace QtSpeech_v2

// ============================================================================
