!contains( included_modules, qtstyleplugins/src/qtstyleplugins.pri) {
		included_modules += qtstyleplugins/src/qtstyleplugins.pri

	INCLUDEPATH += $$PWD
	DEPENDPATH += $$PWD

	static {
		#macx and ios:
		mac:!declarative_debug:LIBS += -L$$[QT_INSTALL_PLUGINS]/styles -lqplastiquestyle
		mac:declarative_debug:LIBS += -L$$[QT_INSTALL_PLUGINS]/styles -lqplastiquestyle_debug

		#win32 will be done dynamically always

		#linux and android:
		linux|android:LIBS += -L$$[QT_INSTALL_PLUGINS]/styles -lqplastiquestyle
	}
}
