#-------------------------------------------------
#
# Project created by QtCreator 2012-09-22T22:59:45
#
#-------------------------------------------------

QT       += core gui sql

TARGET = KJVCanOpener
TEMPLATE = app


SOURCES += main.cpp\
	KJVCanOpener.cpp \
    CSV.cpp \
    dbstruct.cpp \
    BuildDB.cpp \
    ReadDB.cpp \
    KJVBrowser.cpp \
    KJVSearchPhraseEdit.cpp \
    VerseListModel.cpp

HEADERS  += KJVCanOpener.h \
    CSV.h \
    dbstruct.h \
    ReadDB.h \
    BuildDB.h \
    KJVBrowser.h \
    KJVSearchPhraseEdit.h \
    VerseListModel.h

FORMS    += KJVCanOpener.ui \
    KJVBrowser.ui \
    KJVSearchPhraseEdit.ui

RESOURCES += \
    KJVCanOpener.qrc


