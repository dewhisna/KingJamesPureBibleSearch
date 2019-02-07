##*****************************************************************************
##
## Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
## Contact: http://www.dewtronics.com/
##
## This file is part of the KJVCanOpener Application as originally written
## and developed for Bethel Church, Festus, MO.
##
## GNU General Public License Usage
## This file may be used under the terms of the GNU General Public License
## version 3.0 as published by the Free Software Foundation and appearing
## in the file gpl-3.0.txt included in the packaging of this file. Please
## review the following information to ensure the GNU General Public License
## version 3.0 requirements will be met:
## http://www.gnu.org/copyleft/gpl.html.
##
## Other Usage
## Alternatively, this file may be used in accordance with the terms and
## conditions contained in a signed written agreement between you and
## Dewtronics.
##
##*****************************************************************************

#-------------------------------------------------
#
# Project created by QtCreator 2013-04-08T19:54:43
#
#-------------------------------------------------

QT       += core gui xml

CONFIG	+= c++11		# This works on Qt5+ only
lessThan(QT_MAJOR_VERSION,5):QMAKE_CXXFLAGS += -std=c++11		# manual workaround for Qt 4

# Workaround QTBUG-30594 (was fixed in 4.8.5):
equals(QT_MAJOR_VERSION,4):equals(QT_MINOR_VERSION,8):lessThan(QT_PATCH_VERSION,5):QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

TARGET = KJVDataParse
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += NOT_USING_SQL
DEFINES += NO_PERSISTENT_SETTINGS
DEFINES += OSIS_PARSER_BUILD
console:DEFINES += IS_CONSOLE_APP

include(../qtiocompressor/src/qtiocompressor.pri)
include(../grantlee/textdocument/textdocument.pri)

CODECFORSRC = UTF-8
CODECFORTR  = UTF-8

###############################################################################

TRANSLATIONS += \
		translations/kjvdataparse.en.ts \
		translations/kjvdataparse.fr.ts \
		translations/kjvdataparse.es.ts \
		translations/kjvdataparse.de.ts \
		translations/kjvdataparse.ru.ts

SOURCES += main.cpp \
	../KJVCanOpener/BuildDB.cpp \
	../KJVCanOpener/CSV.cpp \
	../KJVCanOpener/dbDescriptors.cpp \
	../KJVCanOpener/dbstruct.cpp \
	../KJVCanOpener/Highlighter.cpp \
	../KJVCanOpener/ParseSymbols.cpp \
	../KJVCanOpener/PersistentSettings.cpp \
	../KJVCanOpener/PhraseEdit.cpp \
	../KJVCanOpener/PhraseListModel.cpp \
	../KJVCanOpener/ReadDB.cpp \
	../KJVCanOpener/ReportError.cpp \
	../KJVCanOpener/ScriptureDocument.cpp \
	../KJVCanOpener/SearchCompleter.cpp \
	../KJVCanOpener/Translator.cpp \
	../KJVCanOpener/UserNotesDatabase.cpp \
	../KJVCanOpener/VerseRichifier.cpp

HEADERS += \
	xc_KJVDataParse.h \
	../KJVCanOpener/BuildDB.h \
	../KJVCanOpener/CSV.h \
	../KJVCanOpener/dbDescriptors.h \
	../KJVCanOpener/dbstruct.h \
	../KJVCanOpener/Highlighter.h \
	../KJVCanOpener/ParseSymbols.h \
	../KJVCanOpener/PersistentSettings.h \
	../KJVCanOpener/PhraseEdit.h \
	../KJVCanOpener/PhraseListModel.h \
	../KJVCanOpener/ReadDB.h \
	../KJVCanOpener/ReportError.h \
	../KJVCanOpener/ScriptureDocument.h \
	../KJVCanOpener/SearchCompleter.h \
	../KJVCanOpener/Translator.h \
	../KJVCanOpener/UserNotesDatabase.h \
	../KJVCanOpener/VerseRichifier.h

###############################################################################

# Build Translations:
!isEmpty(TRANSLATIONS) {
	DEFINES+=HAVE_TRANSLATIONS
	for(f, TRANSLATIONS):translationDeploy.files += $$quote($${PWD}/$$replace(f, .ts, .qm))
	for(f, TRANSLATIONS):translation_source.files += $$quote($${PWD}/$$f)
	exists($$[QT_INSTALL_BINS]/lrelease) {
		translation_build.output = $$translationDeploy.files
		translation_build.target = $$translationDeploy.files
		translation_build.input = $$translation_source.files
		translation_build.depends = $$translation_source.files
		translation_build.commands = $$quote($$[QT_INSTALL_BINS]/lrelease $$_PRO_FILE_$$escape_expand(\\n\\t))
		translation_build.CONFIG = no_link
		QMAKE_EXTRA_TARGETS += translation_build $$translation_source.files
		QMAKE_EXTRA_COMPILERS += translation_build
		POST_TARGETDEPS +=  $$translation_source.files
		QMAKE_POST_LINK += $$quote($$[QT_INSTALL_BINS]/lrelease $$_PRO_FILE_$$escape_expand(\\n\\t))
	} else {
		message("Can't build translations!  Using previously built translations if possible")
	}
	unix:!mac {
		translationDeploy.path = .
#		QMAKE_POST_LINK += $$quote(cp $$translationDeploy.files $$translationDeploy.path$$escape_expand(\\n\\t))
	}
	#INSTALLS += translationDeploy
	message("Deploying translations:" $$TRANSLATIONS$$escape_expand(\\n))
}

###############################################################################

message($$CONFIG$$escape_expand(\\n))

