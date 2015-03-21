TARGET  = qcleanlooksstyle
PLUGIN_TYPE = styles
PLUGIN_EXTENDS = widgets
PLUGIN_CLASS_NAME = QCleanlooksStylePlugin
load(qt_plugin)

QT = core gui widgets
win32:CONFIG-=debug_and_release

HEADERS += qcleanlooksstyle.h
SOURCES += qcleanlooksstyle.cpp
SOURCES += plugin.cpp

include(../shared/shared.pri)

OTHER_FILES += cleanlooks.json
