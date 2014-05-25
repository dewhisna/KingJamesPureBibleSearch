include(../wwwidgets_utils.pri)

linux-g++|linux-g++-64:CONFIG(release, debug|release):QMAKE_STRIP = strip

HEADERS += wwglobal.h wwglobal_p.h colormodel.h
SOURCES += wwglobal_p.cpp colormodel.cpp

QT += core gui
greaterThan(QT_MAJOR_VERSION,4):QT+=widgets

RESOURCES += ../images/wwwidgets.qrc
INCLUDEPATH += .

TEMPLATE = lib
CONFIG += warn_on
TARGET = $$qtLibraryTarget(wwwidgets4)

message("Target:" $$TARGET)

linux-g++|linux-g++-64 {
	CONFIG += static debug_and_release separate_debug_info
#	CONFIG += debug_and_release separate_debug_info
} else {
	# For Cocoa-static:
	mac {
		CONFIG += static
	} else {
		CONFIG += debug_and_release
	}
}

# The following will build the correct .a libs, but for
#	whatever reason, make install breaks things!!  So
#	they have to be manually copied! ugh!
ios {
	QMAKE_IOS_DEVICE_ARCHS = armv7
	QMAKE_IOS_SIMULATOR_ARCHS = i386
}

dlltarget.path = $$[QT_INSTALL_BINS]
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target dlltarget


win32 {
	CONFIG += build_all dll
    QMAKE_LFLAGS += -shared
}

mac {
	CONFIG += lib_bundle
	macx:CONFIG += build_all
}

DEFINES += WW_BUILD_WWWIDGETS
DISTFILES += ../TODO

CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

TRANSLATIONS = ../translations/wwwidgets_pl.ts
DISTFILES += ../translations/*.qm


message($$CONFIG)
