!contains( included_modules, qtassetsmanager/qtassetsmanager.pri) {
		included_modules += qtassetsmanager/qtassetsmanager.pri

	INCLUDEPATH += $$PWD
	DEPENDPATH += $$PWD

	HEADERS +=	$$PWD/qabstractfileengine_p.h \
				$$PWD/qandroidassetsfileenginehandler.h

	SOURCES +=	$$PWD/qandroidassetsfileenginehandler.cpp

}
