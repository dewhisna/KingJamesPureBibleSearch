include(../wwwidgets_utils.pri)

HEADERS += wwwidgets.h wwinterfaces.h colorlisteditor.h
SOURCES += wwwidgets.cpp wwinterfaces.cpp colorlisteditor.cpp

#for(cls, WWWIDGETS){
#    low = $$lower($$cls)
#    HEADERS += $${low}iface.h
#    SOURCES += $${low}iface.cpp
#}

FORMS += about.ui colorlisteditor.ui

TEMPLATE = lib
TARGET = wwwidgetsplugin
CONFIG += plugin designer warn_on release
CONFIG -= debug
# deps


unix:QMAKE_CXXFLAGS_RELEASE -= -g
unix:LIBS += -L../widgets

win32 {
    CONFIG(release): LIBS += -L../widgets/release
#    CONFIG(debug): LIBS += -L../widgets/debug 
}

qtAddLibrary(wwwidgets4)



# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
dlltarget.path = $$[QT_INSTALL_PLUGINS]/desiger
INSTALLS += dlltarget


INCLUDEPATH += ../widgets/ .

linux-g++:QMAKE_STRIP = strip

