TARGET  = qcleanlooksstyle
PLUGIN_TYPE = styles
PLUGIN_EXTENDS = widgets
load(qt_plugin)

QT = core gui widgets

HEADERS += qcleanlooksstyle.h
SOURCES += qcleanlooksstyle.cpp
SOURCES += plugin.cpp

include(../shared/shared.pri)

OTHER_FILES += cleanlooks.json
