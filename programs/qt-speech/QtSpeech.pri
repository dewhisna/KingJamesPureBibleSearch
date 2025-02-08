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
#
# =============================================================================
#
# This version modified for KJPBS usage as follows:
#
# Copyright (C) 2014-2025 Donna Whisnant, a.k.a. Dewtronics.
# Contact: http://www.dewtronics.com/
#
# This file is part of the KJVCanOpener Application as originally written
# and developed for Bethel Church, Festus, MO.
#
# GNU General Public License Usage
# This file may be used under the terms of the GNU General Public License
# version 3.0 as published by the Free Software Foundation and appearing
# in the file gpl-3.0.txt included in the packaging of this file. Please
# review the following information to ensure the GNU General Public License
# version 3.0 requirements will be met:
# http://www.gnu.org/copyleft/gpl.html.
#
# Other Usage
# Alternatively, this file may be used in accordance with the terms and
# conditions contained in a signed written agreement between you and
# Dewtronics.
#


!contains( included_modules, qt-speech/QtSpeech.pri) {
		included_modules += qt-speech/QtSpeech.pri

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/QtSpeech \
	$$PWD/QtSpeech.h \

!qtspeech_libbuild {
	CONFIG(debug, debug|release) {
		!macx {
			LIBS += -lQtSpeechd
		} else {
			LIBS += -L$$[QT_INSTALL_LIBS] -lQtSpeech_debug
		}
	}
	CONFIG(release, debug|release) {
		!macx {
			LIBS += -lQtSpeech
		} else {
			LIBS += -L$$[QT_INSTALL_LIBS] -lQtSpeech
		}
	}
}

macx {
	qtspeech_libbuild{
		HEADERS += $$PWD/QtSpeech_mac.h \
					$$PWD/QtSpeech_ATO.h
		SOURCES += $$PWD/QtSpeech_mac.cpp
	}

	LIBS *= -framework AppKit

	# Mac: use system Frameworks
	#LIBS += -framework CoreAudio -framework AudioUnit -framework AudioToolbox -framework Carbon

}

win32 {
	qtspeech_libbuild {
		HEADERS += $$PWD/QtSpeech_win.h \
					$$PWD/QtSpeech_ATO.h
		SOURCES += $$PWD/QtSpeech_win.cpp

#		INCLUDEPATH += "C:/Program Files/PSDK/Include"
#		INCLUDEPATH += "C:/Program Files/PSDK/Include/atl"
		INCLUDEPATH += "C:/Program Files (x86)/Microsoft Speech SDK 5.1/Include"
		INCLUDEPATH += "$$PWD/sapi51_fixes"

		LIBS += -L"C:/Program Files (x86)/Microsoft Speech SDK 5.1/Lib/i386"
	}
}

unix:!mac {
	qtspeech_libbuild {
		HEADERS += $$PWD/QtSpeech_unx.h \
					$$PWD/QtSpeech_ATO.h
		SOURCES += $$PWD/QtSpeech_unx.cpp
	}

	INCLUDEPATH += $$PWD/festival/speech_tools/include
	INCLUDEPATH += $$PWD/festival/festival/src/include

	LIBS += -L$$PWD/festival/festival/src/lib -lFestival
	LIBS += -L$$PWD/festival/speech_tools/lib -lestools -lestbase -leststring
	LIBS += -lncurses

	# Linux: use asound
	LIBS += -lasound

}

}
