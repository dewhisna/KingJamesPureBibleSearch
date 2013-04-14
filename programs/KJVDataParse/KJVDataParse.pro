#-------------------------------------------------
#
# Project created by QtCreator 2013-04-08T19:54:43
#
#-------------------------------------------------

QT       += core gui xml sql

TARGET = KJVDataParse
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../KJVCanOpener/dbstruct.cpp \
    ../KJVCanOpener/ReadDB.cpp \
    ../KJVCanOpener/BuildDB.cpp \
    ../KJVCanOpener/CSV.cpp \
    ../KJVCanOpener/PhraseEdit.cpp \
    ../KJVCanOpener/PhraseListModel.cpp \
    ../KJVCanOpener/Highlighter.cpp \
    ../KJVCanOpener/ToolTipEdit.cpp

HEADERS += \
    ../KJVCanOpener/dbstruct.h \
    ../KJVCanOpener/ReadDB.h \
    ../KJVCanOpener/PhraseListModel.h \
    ../KJVCanOpener/PhraseEdit.h \
    ../KJVCanOpener/BuildDB.h \
    ../KJVCanOpener/CSV.h \
    ../KJVCanOpener/Highlighter.h \
    ../KJVCanOpener/ToolTipEdit.h


QMAKE_CXXFLAGS += -DNO_PERSISTENT_SETTINGS

