#-------------------------------------------------
#
#	BibleDNA Study
#
#	Project created by QtCreator 2019-01-15T09:02:06
#
#	Requires Qt 5+
#-------------------------------------------------

QT		*= core gui widgets xml

console {
	CONFIG   -= app_bundle
} else {
#	CONFIG	+= wwwidgets
}

TARGET = BibleDNA
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES *= QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES *= QT_DISABLE_DEPRECATED_BEFORE=0x060000		# disables all the APIs deprecated before Qt 6.0.0

DEFINES *= KJV_SEARCH_BUILD				# Piggyback on the Search Build
DEFINES *= USE_EXTENDED_INDEXES
DEFINES *= NOT_USING_SQL
DEFINES *= NO_PERSISTENT_SETTINGS
console:DEFINES += IS_CONSOLE_APP

CONFIG += c++11

CODECFORSRC = UTF-8
CODECFORTR  = UTF-8

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES *= QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES *= QT_DISABLE_DEPRECATED_BEFORE=0x060000               # disables all the APIs deprecated before Qt 6.0.0

###############################################################################

SOURCES += \
		main.cpp \
		MainWindow.cpp \
		BibleText.cpp \
		HebrewLetters.cpp

HEADERS += \
		MainWindow.h \
		BibleText.h \
		HebrewLetters.h

!console:FORMS += \
		MainWindow.ui

###############################################################################

PATH_TO_KJPBS = $$PWD/../KJVCanOpener

include($$PATH_TO_KJPBS/../qtiocompressor/src/qtiocompressor.pri)
include($$PATH_TO_KJPBS/../grantlee/textdocument/textdocument.pri)
#include($$PATH_TO_KJPBS/../QtFindReplaceDialog/qtfindreplacedialog.pri)

INCLUDEPATH += $$PATH_TO_KJPBS

SOURCES += \
	$$PATH_TO_KJPBS/CSV.cpp \
	$$PATH_TO_KJPBS/dbDescriptors.cpp \
	$$PATH_TO_KJPBS/dbstruct.cpp \
	$$PATH_TO_KJPBS/Highlighter.cpp \
	$$PATH_TO_KJPBS/KJVSearchCriteria.cpp \
	$$PATH_TO_KJPBS/ModelRowForwardIterator.cpp \
	$$PATH_TO_KJPBS/ParseSymbols.cpp \
	$$PATH_TO_KJPBS/PersistentSettings.cpp \
	$$PATH_TO_KJPBS/PhraseEdit.cpp \
	$$PATH_TO_KJPBS/PhraseListModel.cpp \
	$$PATH_TO_KJPBS/ReadDB.cpp \
	$$PATH_TO_KJPBS/ReportError.cpp \
	$$PATH_TO_KJPBS/ScriptureDocument.cpp \
	$$PATH_TO_KJPBS/SearchCompleter.cpp \
	$$PATH_TO_KJPBS/Translator.cpp \
	$$PATH_TO_KJPBS/UserNotesDatabase.cpp \
	$$PATH_TO_KJPBS/VerseRichifier.cpp

buildKJVDatabase:SOURCES += \
	$$PATH_TO_KJPBS/BuildDB.cpp

HEADERS += \
	$$PATH_TO_KJPBS/CSV.h \
	$$PATH_TO_KJPBS/dbDescriptors.h \
	$$PATH_TO_KJPBS/dbstruct.h \
	$$PATH_TO_KJPBS/Highlighter.h \
	$$PATH_TO_KJPBS/KJVSearchCriteria.h \
	$$PATH_TO_KJPBS/ModelRowForwardIterator.h \
	$$PATH_TO_KJPBS/ParseSymbols.h \
	$$PATH_TO_KJPBS/PersistentSettings.h \
	$$PATH_TO_KJPBS/PhraseEdit.h \
	$$PATH_TO_KJPBS/PhraseListModel.h \
	$$PATH_TO_KJPBS/ReadDB.h \
	$$PATH_TO_KJPBS/ReportError.h \
	$$PATH_TO_KJPBS/ScriptureDocument.h \
	$$PATH_TO_KJPBS/SearchCompleter.h \
	$$PATH_TO_KJPBS/Translator.h \
	$$PATH_TO_KJPBS/UserNotesDatabase.h \
	$$PATH_TO_KJPBS/VerseRichifier.h

buildKJVDatabase:HEADERS += \
	$$PATH_TO_KJPBS/BuildDB.h

FORMS += \
	$$PATH_TO_KJPBS/KJVSearchCriteria.ui


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

# -----------------------------------------------------------------------------

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

###############################################################################

message($$CONFIG$$escape_expand(\\n))
