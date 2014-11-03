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

#include <QtCore>
#include <QtSpeech>
#include <QtSpeech_unx.h>
#include <festival.h>

namespace QtSpeech_v1 { // API v1.0

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

// qobject for speech thread
bool QtSpeech_th::init = false;
void QtSpeech_th::say(QString text) {
    try {
        if (!init) {
            int heap_size = FESTIVAL_HEAP_SIZE;
            festival_initialize(true,heap_size);
            init = true;
        }
        has_error = false;
        EST_String est_text(text.toUtf8());
        SysCall(festival_say_text(est_text), QtSpeech::LogicError);
    }
    catch(QtSpeech::LogicError e) {
        has_error = true;
        err = e;
    }
    emit finished();
}

// internal data
class QtSpeech::Private {
public:
    Private()
        :onFinishSlot(0L) {}

    VoiceName name;
    static const QString VoiceId;

    const char * onFinishSlot;
    QPointer<QObject> onFinishObj;
    static QPointer<QThread> speechThread;
};
QPointer<QThread> QtSpeech::Private::speechThread = 0L;
const QString QtSpeech::Private::VoiceId = QString("festival:%1");

// implementation
QtSpeech::QtSpeech(QObject * parent)
    :QObject(parent), d(new Private)
{
    VoiceName n = {Private::VoiceId.arg("english"), "English"};
    if (n.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    d->name = n;
}

QtSpeech::QtSpeech(VoiceName n, QObject * parent)
    :QObject(parent), d(new Private)
{
    if (n.id.isEmpty()) {
        VoiceName def = {Private::VoiceId.arg("english"), "English"};
        n = def;
    }

    if (n.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    d->name = n;
}

QtSpeech::~QtSpeech()
{
    //if ()
    delete d;
}

const QtSpeech::VoiceName & QtSpeech::name() const {
    return d->name;
}

QtSpeech::VoiceNames QtSpeech::voices()
{
    VoiceNames vs;
    VoiceName n = {Private::VoiceId.arg("english"), "English"};
    vs << n;
    return vs;
}

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

    QtSpeech_th * th = new QtSpeech_th;
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
    QtSpeech_th th;
    th.moveToThread(d->speechThread);
    connect(&th, SIGNAL(finished()), &el, SLOT(quit()), Qt::QueuedConnection);
    QMetaObject::invokeMethod(&th, "say", Qt::QueuedConnection, Q_ARG(QString,text));
    el.exec();

    if (th.has_error)
        throw th.err;
}

void QtSpeech::timerEvent(QTimerEvent * te)
{
    QObject::timerEvent(te);
}

} // namespace QtSpeech_v1
