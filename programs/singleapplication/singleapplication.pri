!contains( included_modules, singleapplication/singleapplication.pri) {
		included_modules += singleapplication/singleapplication.pri

DEPENDPATH	+= $$PWD
INCLUDEPATH	+= $$PWD

HEADERS		+= singleapplication.h \
                singleapplication_p.h

SOURCES		+= singleapplication.cpp

}
