TARGET  = qplastiquestyle
PLUGIN_TYPE = styles
PLUGIN_EXTENDS = widgets
load(qt_plugin)

QT = core core-private gui gui-private widgets
win32:CONFIG-=debug_and_release

HEADERS += qplastiquestyle.h
SOURCES += qplastiquestyle.cpp
SOURCES += plugin.cpp

include(../shared/shared.pri)

OTHER_FILES += plastique.json
