INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qhexstring_p.h \
    $$PWD/qstylecache_p.h \
    $$PWD/qstylehelper_p.h

!contains(QT_CONFIG, static):SOURCES += \
    $$PWD/qstylehelper.cpp
