# QtSpeech -- a small cross-platform library to use TTS
# Copyright (C) 2010-2011 LynxLine.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this library; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

TEMPLATE = lib

win32 {
	DEFINES += WIN32_LEAN_AND_MEAN
	DEFINES += _CRT_SECURE_NO_WARNINGS
	DEFINES += UNICODE _UNICODE

	CONFIG -= debug_and_release                                                     # Get rid of double debug/release subfolders and do correct shadow build
	equals(MSVC_VER, "12.0"):QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01         # Enable Support for WinXP if we are building with MSVC 2013, as MSVC 2010 already does
}

useserver {
	QT += network
	DEFINES += USE_FESTIVAL_SERVER
}

CONFIG += staticlib
CONFIG += qtspeech_libbuild
DEFINES += QTSPEECH_STATIC
CONFIG(debug, debug|release) {
    TARGET = QtSpeechd
}
CONFIG(release, debug|release) {
    TARGET = QtSpeech
}

include(QtSpeech.pri)
