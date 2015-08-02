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

int main(int argc, char ** argv)
{
    QCoreApplication a(argc, argv);
    foreach(QtSpeech::VoiceName v, QtSpeech::voices())
        qDebug() << "id:" << v.id << "name:" << v.name;
    
    QtSpeech speech;

    QString text = "Hello World!";

    qDebug() << "About to say synchrounously" << text << "using voice:" << speech.name().name;
    speech.say(text);
    //return 0;

    qDebug() << "About to say asynchrounously" << text << "using voice:" << speech.name().name;
    speech.tell(text, &a, SLOT(quit()));
    return a.exec();
}
