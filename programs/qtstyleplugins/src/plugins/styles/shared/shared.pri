INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qhexstring_p.h \
    $$PWD/qstylecache_p.h \
    $$PWD/qstylehelper_p.h

!staticQt:SOURCES += \
    $$PWD/qstylehelper.cpp
