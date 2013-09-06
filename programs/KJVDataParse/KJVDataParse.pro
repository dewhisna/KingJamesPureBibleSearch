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

include(../qtiocompressor/src/qtiocompressor.pri)
include(../grantlee/textdocument/textdocument.pri)

CODECFORSRC = UTF-8
CODECFORTR  = UTF-8

TRANSLATIONS += \
		kjvdataparse_en.ts \
		kjvdataparse_fr.ts

SOURCES += main.cpp \
	../KJVCanOpener/dbstruct.cpp \
	../KJVCanOpener/ReadDB.cpp \
	../KJVCanOpener/BuildDB.cpp \
	../KJVCanOpener/CSV.cpp \
	../KJVCanOpener/PhraseEdit.cpp \
	../KJVCanOpener/PhraseListModel.cpp \
	../KJVCanOpener/Highlighter.cpp \
	../KJVCanOpener/ParseSymbols.cpp \
	../KJVCanOpener/VerseRichifier.cpp \
	../KJVCanOpener/SearchCompleter.cpp \
	../KJVCanOpener/ScriptureDocument.cpp \
	../KJVCanOpener/UserNotesDatabase.cpp \
	../KJVCanOpener/PersistentSettings.cpp

HEADERS += \
	../KJVCanOpener/dbstruct.h \
	../KJVCanOpener/ReadDB.h \
	../KJVCanOpener/PhraseListModel.h \
	../KJVCanOpener/PhraseEdit.h \
	../KJVCanOpener/BuildDB.h \
	../KJVCanOpener/CSV.h \
	../KJVCanOpener/Highlighter.h \
	../KJVCanOpener/ParseSymbols.h \
	../KJVCanOpener/VerseRichifier.h \
	../KJVCanOpener/SearchCompleter.h \
	../KJVCanOpener/ScriptureDocument.h \
	../KJVCanOpener/UserNotesDatabase.h \
	../KJVCanOpener/PersistentSettings.h


#QMAKE_CXXFLAGS += -DNO_PERSISTENT_SETTINGS
#QMAKE_CXXFLAGS += -DOSIS_PARSER_BUILD

DEFINES += NO_PERSISTENT_SETTINGS
DEFINES += OSIS_PARSER_BUILD


