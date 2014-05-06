!contains( included_modules, modeltest/modeltest.pri) {
		included_modules += modeltest/modeltest.pri

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

load(qttest_p4)
SOURCES         += $$PWD/modeltest.cpp $$PWD/dynamictreemodel.cpp
HEADERS         += $$PWD/modeltest.h $$PWD/dynamictreemodel.h

}
