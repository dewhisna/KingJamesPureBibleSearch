!contains( included_modules, qtiocompressor/src/qtiocompressor.pri) {
		included_modules += qtiocompressor/src/qtiocompressor.pri

include(../common.pri)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# We need to find zlib.h. If it's not in the system includes, this will
# find the one includes with Qt, assuming a developer-style "in-place" install.
# Otherwise, edit this to point at the directory where zlib.h resides.
exists($$[QT_INSTALL_PREFIX]/include/QtZlib/zlib.h) {
	INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/QtZlib
} else {
	INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib
}

qtiocompressor-uselib:!qtiocompressor-buildlib {
    LIBS += -L$$QTIOCOMPRESSOR_LIBDIR -l$$QTIOCOMPRESSOR_LIBNAME
} else {
    SOURCES += $$PWD/qtiocompressor.cpp
    HEADERS += $$PWD/qtiocompressor.h
}

win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QT_QTIOCOMPRESSOR_EXPORT
    else:qtiocompressor-uselib:DEFINES += QT_QTIOCOMPRESSOR_IMPORT
}

# Qt-5.2.0-beta1 Pre-built Android and iOS installs apparently doesn't include qt-zlib and since
#	we are currently using the prebuilt Android and iOS versions, we'll just include the .pri
#	file here to link it statically in our project:
android|ios {
	include($$[QT_INSTALL_PREFIX]/src/3rdparty/zlib.pri)
}
macx {
	greaterThan(QT_MAJOR_VERSION,4):include($$[QT_INSTALL_PREFIX]/src/3rdparty/zlib.pri)
}
!android:!ios:!macx {
	!contains(QT_CONFIG, system-zlib) {
		!contains(QT_CONFIG, zlib) {
			include($$[QT_INSTALL_PREFIX]/src/3rdparty/zlib.pri)
		}
	} else {
		if(unix|win32-g++*):LIBS += -lz
		else:               LIBS += zdll.lib
	}
}

}
