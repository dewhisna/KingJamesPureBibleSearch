include(../wwwidgets_utils.pri)

linux-g++|linux-g++-32|linux-g++-64:CONFIG(release, debug|release):QMAKE_STRIP = strip

HEADERS += wwglobal.h wwglobal_p.h colormodel.h
SOURCES += wwglobal_p.cpp colormodel.cpp

QT += core gui
greaterThan(QT_MAJOR_VERSION,4):QT+=widgets

linux-g++|linux-g++-32|linux-g++-64 {
	CONFIG += static separate_debug_info
#	CONFIG += debug_and_release separate_debug_info
} else {
	# For Cocoa-static:
	mac:lessThan(QT_MAJOR_VERSION, 5) {
		CONFIG += static
	} else {
		CONFIG -= debug_and_release
	}
}

RESOURCES += ../images/wwwidgets.qrc
INCLUDEPATH += .

TEMPLATE = lib
CONFIG += warn_on
linux-g++|linux-g++-32|linux-g++-64 {
	CONFIG(release, debug|release) {
		TARGET = $$qtLibraryTarget(wwwidgets4)
	} else {
		TARGET = $$qtLibraryTarget(wwwidgets4d)
	}
} else {
	TARGET = $$qtLibraryTarget(wwwidgets4)
}

message("Target:" $$TARGET)

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
#	CONFIG += build_all dll
	CONFIG += dll
	QMAKE_LFLAGS += -shared
}

mac {
	CONFIG += lib_bundle
	macx:CONFIG += build_all
	macx {
		greaterThan(QT_MAJOR_VERSION, 4) {
			CONFIG += create_prl shared
			QMAKE_INFO_PLIST = ../wwwidgets4.Info.plist
			QMAKE_POST_LINK += $$quote(install_name_tool -id $$[QT_INSTALL_LIBS]/wwwidgets4.framework/Versions/1/$$TARGET wwwidgets4.framework/$$TARGET$$escape_expand(\\n\\t))
		} else {
			CONFIG += link_prl
		}
	}
}

DEFINES += WW_BUILD_WWWIDGETS
DISTFILES += ../TODO

CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

TRANSLATIONS = ../translations/wwwidgets_pl.ts \
				../translations/wwwidgets_en.ts \
				../translations/wwwidgets_fr.ts \
				../translations/wwwidgets_es.ts \
				../translations/wwwidgets_de.ts

DISTFILES += ../translations/*.qm


message($$CONFIG)
