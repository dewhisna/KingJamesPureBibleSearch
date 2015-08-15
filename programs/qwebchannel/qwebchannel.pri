!contains( included_modules, qwebchannel/qwebchannel.pri) {
	included_modules += qwebchannel/qwebchannel.pri

	# QWebChannel support only on Qt 5.5+:
	webchannel:if(greaterThan(QT_MAJOR_VERSION,5) | equals(QT_MAJOR_VERSION,5):greaterThan(QT_MINOR_VERSION,4)) {

		INCLUDEPATH += $$PWD
		DEPENDPATH += $$PWD

		QT *= webchannel websockets

		SOURCES += \
			$$PWD/headlessSearchResults.cpp \
			$$PWD/webChannelServer.cpp \
			$$PWD/webChannelObjects.cpp \
			$$PWD/websocketclientwrapper.cpp \
			$$PWD/websockettransport.cpp

		HEADERS += \
			$$PWD/headlessSearchResults.h \
			$$PWD/webChannelServer.h \
			$$PWD/webChannelObjects.h \
			$$PWD/websocketclientwrapper.h \
			$$PWD/websockettransport.h

		DEFINES += USING_WEBCHANNEL

		WEBFILES += \
			$$PWD/html/qwebchannel.js \
			$$PWD/html/index.html \
			$$PWD/html/error.html

		webchannelDeploy.files = $$WEBFILES
		webchannelDeploy.path = .

		INSTALLS += webchannelDeploy

		!equals($$PWD, $$OUT_PWD) {
			# Shadow build, copy all example assets.
			webchannel_copyfiles = $$WEBFILES
		}

		defineReplace(stripSrcDir) {
			return($$basename(1))
		}

		webchannel_build.input = webchannel_copyfiles
		webchannel_build.output = $$OUT_PWD/${QMAKE_FUNC_FILE_IN_stripSrcDir}
		webchannel_build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
		webchannel_build.name = COPY ${QMAKE_FILE_IN}
		webchannel_build.CONFIG = no_link target_predeps
		QMAKE_EXTRA_COMPILERS += webchannel_build

	}

}
