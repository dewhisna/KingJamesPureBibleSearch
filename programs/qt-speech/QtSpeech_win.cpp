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
#include <QtSpeech_win.h>
#include <vector>

#include <sapi.h>
#include <sphelper_fixed.h>
#include <comdef.h>

#include <windows.h>

// ============================================================================

namespace QtSpeech_v2 { // API v2.0

// some defines for throwing exceptions
#define Where QString("%1:%2:").arg(__FILE__).arg(__LINE__)
#define SysCall(x,e) {\
	HRESULT hr = x;\
	if (FAILED(hr)) {\
		QString msg = #e;\
		msg += ":"+QString(__FILE__);\
		msg += ":"+QString::number(__LINE__)+":"+#x+":";\
		msg += QString::fromWCharArray(_com_error(hr).ErrorMessage());\
		throw e(msg);\
	}\
}

// ============================================================================

class CarrWCHAR : public std::vector<WCHAR>
{
public:
	CarrWCHAR(const QString &strSource)
	{
		const unsigned short *pWCHAR = strSource.utf16();
		assign(pWCHAR, &pWCHAR[strSource.size()+2]);			// +2 : +1 to add NULL to the array which utf16() promises to provide and +1 for the end() iterator
	}
};

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
			CoInitialize(NULL);
			SysCall( m_pSpVoice.CoCreateInstance( __uuidof(SpVoice) ), QtSpeech::InitError);
		} catch (...) { /* TODO : Add saving of error message */ }
	}
	virtual ~QtSpeech_GlobalData()
	{
		m_pSpVoice.Release();
		CoUninitialize();
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

	CComPtr<ISpVoice> m_pSpVoice;						// SDK Voice Object

	void setVoice();

protected slots:
	virtual void en_lostServer() { }

} g_QtSpeechGlobal;

// ============================================================================

static QtSpeech::TVoiceName defaultVoiceName()
{
	QtSpeech::TVoiceName aVoiceName;

	try {
		WCHAR *pwcID = NULL;
		WCHAR *pwcName = NULL;
		CComPtr<ISpObjectToken> pSysVoice;
		if (g_QtSpeechGlobal.m_pSpVoice == NULL) return aVoiceName;
		SysCall( g_QtSpeechGlobal.m_pSpVoice->GetVoice(&pSysVoice), QtSpeech::LogicError );
		SysCall( SpGetDescription(pSysVoice, &pwcName ), QtSpeech::LogicError );
		SysCall( pSysVoice->GetId(&pwcID), QtSpeech::LogicError );
		aVoiceName.id = QString::fromWCharArray(pwcID);
		aVoiceName.name = QString::fromWCharArray(pwcName);
		// TODO : Set aVoiceName.lang
		pSysVoice.Release();
	} catch (...) { /* TODO : Add saving of error message */ }

	return aVoiceName;
}

// ============================================================================

bool QtSpeech_th::m_bInit = false;

QtSpeech_th::QtSpeech_th(QObject *pParent)
	:	QObject(pParent),
		err(""),
		has_error(false),
		m_bAmTalking(false)
{
	try {
		// Create secondary object on separate thread to keep from having to marshall
		//		the COM object across thread boundary:
		CoInitialize(NULL);			// CoInitialize for Secondary Thread Only
		SysCall( m_pSpVoice.CoCreateInstance( __uuidof(SpVoice) ), QtSpeech::InitError);
	} catch (...) { /* TODO : Add saving of error message */ }
	connect(this, SIGNAL(sayNext(bool)), this, SLOT(en_sayNext(bool)), Qt::QueuedConnection);
}

QtSpeech_th::~QtSpeech_th()
{
	m_pSpVoice.Release();
	CoUninitialize();				// CoUninitialize on Secondary Thread
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

void QtSpeech_th::setVoice(QString strVoiceID)
{
	doInit();

	if (m_pSpVoice != NULL) {
		try {
			ULONG nCount = 0;
			CComPtr<IEnumSpObjectTokens> pVoicesEnum;

			SysCall( SpEnumTokens(SPCAT_VOICES, NULL, NULL, &pVoicesEnum), QtSpeech::LogicError );
			SysCall( pVoicesEnum->GetCount(&nCount), QtSpeech::LogicError );
			for (ULONG ndx = 0; ndx < nCount; ++ndx) {
				QString strTempVoiceID;
				WCHAR *pwcID = NULL;
				CComPtr<ISpObjectToken> pSysVoice;
				SysCall( pVoicesEnum->Next(1, &pSysVoice, NULL), QtSpeech::LogicError );
				SysCall( pSysVoice->GetId(&pwcID), QtSpeech::LogicError );
				strTempVoiceID = QString::fromWCharArray(pwcID);
				if (strTempVoiceID == strVoiceID) m_pSpVoice->SetVoice(pSysVoice);
				pSysVoice.Release();
			}
		} catch (...) { /* TODO : Add saving of error message */ }
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

		if (m_pSpVoice != NULL) {
			CarrWCHAR wcText(m_lstTalkingObjects.at(0).m_strText);
			SysCall( m_pSpVoice->Speak(wcText.data(), /* SPF_ASYNC | */ SPF_IS_NOT_XML, NULL), QtSpeech::LogicError );
		}
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
	if (g_QtSpeechGlobal.m_pSpVoice == NULL) return g_QtSpeechGlobal.m_lstVoiceNames;

	try {
		ULONG nCount = 0;
		CComPtr<IEnumSpObjectTokens> pVoicesEnum;

		SysCall( SpEnumTokens(SPCAT_VOICES, NULL, NULL, &pVoicesEnum), LogicError );
		SysCall( pVoicesEnum->GetCount(&nCount), LogicError );
		for (ULONG ndx = 0; ndx < nCount; ++ndx) {
			TVoiceName aVoiceName;
			WCHAR *pwcID = NULL;
			WCHAR *pwcName = NULL;
			CComPtr<ISpObjectToken> pSysVoice;
			SysCall( pVoicesEnum->Next(1, &pSysVoice, NULL), LogicError );
			SysCall( SpGetDescription(pSysVoice, &pwcName), LogicError );
			SysCall( pSysVoice->GetId(&pwcID), LogicError );
			aVoiceName.id = QString::fromWCharArray(pwcID);
			aVoiceName.name = QString::fromWCharArray(pwcName);
			// TODO : Set aVoiceName.lang
			pSysVoice.Release();
			g_QtSpeechGlobal.m_lstVoiceNames << aVoiceName;
		}
	} catch (...) { /* TODO : Add saving of error message */ }

	return g_QtSpeechGlobal.m_lstVoiceNames;
}

// ----------------------------------------------------------------------------

bool QtSpeech::canSpeak() const
{
	return (g_QtSpeechGlobal.m_pSpVoice != NULL);
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
	g_QtSpeechGlobal.setVoice();

	if (g_QtSpeechGlobal.createWorkerThread()) {
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), Qt::QueuedConnection);
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), Qt::QueuedConnection);
	}

	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "say", Qt::QueuedConnection, Q_ARG(TAsyncTalkingObject, TAsyncTalkingObject(strText, pObject, pSlot)));
}

void QtSpeech::say(const QString &strText) const
{
	g_QtSpeechGlobal.setVoice();

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

	if (g_QtSpeechGlobal.createWorkerThread()) {
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(beginTalking()), this, SIGNAL(beginning()), Qt::QueuedConnection);
		connect(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), SIGNAL(doneTalking(bool)), this, SIGNAL(finished(bool)), Qt::QueuedConnection);
	}

	QMetaObject::invokeMethod(g_QtSpeechGlobal.m_pSpeechTalker_th.data(), "clearQueue", Qt::QueuedConnection);
}

// ============================================================================


void QtSpeech_GlobalData::setVoice()
{
	QtSpeech::TVoiceName theVoice = m_vnRequestedVoiceName;
	m_vnRequestedVoiceName.clear();

	if (theVoice.isEmpty()) return;
	if (m_vnSelectedVoiceName == theVoice) return;

	m_vnSelectedVoiceName = theVoice;

	if (m_pSpeechThread.isNull()) {
		m_pSpeechThread = new QThread;
		m_pSpeechThread->start();
	}

	QEventLoop el;
	QtSpeech_th th;
	th.moveToThread(m_pSpeechThread);
	connect(&th, SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
	QMetaObject::invokeMethod(&th, "setVoice", Qt::QueuedConnection, Q_ARG(QString, theVoice.id));
	el.exec(QEventLoop::ExcludeUserInputEvents);
}

} // namespace QtSpeech_v2

// ============================================================================
