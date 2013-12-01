!contains( included_modules, qtstyleplugins/src/qtstyleplugins.pri) {
		included_modules += qtstyleplugins/src/qtstyleplugins.pri

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

#INCLUDEPATH += $$PWD/plugins/styles/plastique
#DEPENDPATH += $$PWD/plugins/styles/plastique

QT += core-private gui-private

HEADERS += $$PWD/plugins/styles/plastique/qplastiquestyle.h
SOURCES += $$PWD/plugins/styles/plastique/qplastiquestyle.cpp
SOURCES += $$PWD/plugins/styles/plastique/plugin.cpp

#include(plugins/styles/shared/shared.pri)

OTHER_FILES += $$PWD/plugins/styles/plastique/plastique.json

}
