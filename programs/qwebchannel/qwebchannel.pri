!contains( included_modules, qwebchannel/qwebchannel.pri) {
	included_modules += qwebchannel/qwebchannel.pri

	# QWebChannel support only on Qt 5.5+:
	webchannel:if(equals(QT_MAJOR_VERSION,5):greaterThan(QT_MINOR_VERSION,4) | greaterThan(QT_MAJOR_VERSION,5)) {

		INCLUDEPATH += $$PWD
		DEPENDPATH += $$PWD

		QT *= webchannel websockets

		SOURCES += \
			$$PWD/webChannelSearchResults.cpp \
			$$PWD/webChannelServer.cpp \
			$$PWD/webChannelObjects.cpp \
			$$PWD/webChannelGeoLocate.cpp \
			$$PWD/webChannelBibleAudio.cpp \
			$$PWD/websocketclientwrapper.cpp \
			$$PWD/websockettransport.cpp

		!nommdb:SOURCES += \
			$$PWD/mmdblookup.cpp

		HEADERS += \
			$$PWD/webChannelSearchResults.h \
			$$PWD/webChannelServer.h \
			$$PWD/webChannelObjects.h \
			$$PWD/webChannelGeoLocate.h \
			$$PWD/webChannelBibleAudio.h \
			$$PWD/websocketclientwrapper.h \
			$$PWD/websockettransport.h

		!nommdb:HEADERS += \
			$$PWD/mmdblookup.h

		DEFINES += USING_WEBCHANNEL
		!nommdb:DEFINES += USING_MMDB

		!nommdb:LIBS += -lmaxminddb

		WEBFILES += \
			$$PWD/html/*

		webchannelDeploy.files = $$WEBFILES
		webchannelDeploy.path = ./html

		INSTALLS += webchannelDeploy

		!equals($$PWD, $$OUT_PWD) {
			# Shadow build, copy all example assets.
			webchannel_copyfiles = $$WEBFILES
		}

		defineReplace(stripSrcDir) {
			return($$basename(1))
		}

		webchannel_build.input = webchannel_copyfiles
		webchannel_build.output = $$OUT_PWD/html/${QMAKE_FUNC_FILE_IN_stripSrcDir}
		webchannel_build.commands = $$QMAKE_MKDIR html; $$QMAKE_COPY_DIR ${QMAKE_FILE_IN} html/
		webchannel_build.name = COPY ${QMAKE_FILE_IN}
		webchannel_build.CONFIG = no_link target_predeps
		QMAKE_EXTRA_COMPILERS += webchannel_build

		# Add target for 'clean' so we can also clean the recursed 'html' folders:
		!equals($$PWD, $$OUT_PWD) {
			html_clean.commands = -$(DEL_FILE) -r $${OUT_PWD}/html
			clean.depends = html_clean
			QMAKE_EXTRA_TARGETS += clean html_clean
		}

		# Generate Random Admin Access Key:
		KEYGEN_SCRIPT_FILES = $$PWD/webChannelKeyGen.sh
		webChannelKeyGen.input = KEYGEN_SCRIPT_FILES
		webChannelKeyGen.output = $$OUT_PWD/webChannelKeys.cpp
		webChannelKeyGen.commands = ${QMAKE_FILE_IN}
		webChannelKeyGen.name = Generating WebChannel Keys with ${QMAKE_FILE_IN}
		webChannelKeyGen.CONFIG = no_link target_predeps
		webChannelKeyGen.depends = html/admin
		webChannelKeyGen.variable_out = SOURCES
		QMAKE_EXTRA_COMPILERS += webChannelKeyGen

	} else {
		webchannel:error("WebChannel support build requires Qt 5.5+")
	}

}
