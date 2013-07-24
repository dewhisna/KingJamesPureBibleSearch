include(../wwwidgets_utils.pri)

linux-g++|linux-g++-64:CONFIG(release, debug|release):QMAKE_STRIP = strip

HEADERS += wwglobal.h wwglobal_p.h colormodel.h
SOURCES += wwglobal_p.cpp colormodel.cpp

QT += core gui widgets

RESOURCES += ../images/wwwidgets.qrc
INCLUDEPATH += .

TEMPLATE = lib
CONFIG += warn_on
TARGET = $$qtLibraryTarget(wwwidgets4)

linux-g++|linux-g++-64 {
	CONFIG += static debug_and_release separate_debug_info
#	CONFIG += debug_and_release separate_debug_info
} else {
	# For Cocoa-static:
	macx {
		CONFIG += static
	} else {
		CONFIG += debug_and_release
	}
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
	CONFIG += build_all
}

DEFINES += WW_BUILD_WWWIDGETS
DISTFILES += ../TODO

CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

TRANSLATIONS = ../translations/wwwidgets_pl.ts
DISTFILES += ../translations/*.qm


message($$CONFIG)
