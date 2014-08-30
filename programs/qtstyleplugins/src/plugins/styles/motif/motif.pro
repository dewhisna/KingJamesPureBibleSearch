TARGET  = qmotifstyle
PLUGIN_TYPE = styles
PLUGIN_EXTENDS = widgets
load(qt_plugin)

QT = core gui widgets
win32:CONFIG-=debug_and_release

HEADERS += qcdestyle.h
HEADERS += qmotifstyle.h
SOURCES += qcdestyle.cpp
SOURCES += qmotifstyle.cpp
SOURCES += plugin.cpp

OTHER_FILES += motif.json
