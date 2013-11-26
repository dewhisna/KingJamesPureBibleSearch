##*****************************************************************************
##
## Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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
# Project created by QtCreator 2012-09-22T22:59:45
#
#-------------------------------------------------

QT       += core gui sql xml
greaterThan(QT_MAJOR_VERSION,4):QT+=widgets

CONFIG += rtti

CONFIG += wwwidgets

#QRegularExpression Qt5->Qt4 experimentation:
#CONFIG += pcre

android {
	QT += androidextras
	include(../qtassetsmanager/qtassetsmanager.pri)
	LIBS += -landroid
}

include(../qtiocompressor/src/qtiocompressor.pri)
include(../grantlee/textdocument/textdocument.pri)

!android {
	# Select Desired Package:
	#	SingleApplication
	#	QtSingleApplication
	CONFIG += QtSingleApplication

	SingleApplication {
		include(../singleapplication/singleapplication.pri)
		DEFINES += USING_SINGLEAPPLICATION
		QT += network
	}
	QtSingleApplication {
		include(../qtsingleapplication/src/qtsingleapplication.pri)
		DEFINES += USING_QT_SINGLEAPPLICATION
		QT += network
	}
}

#greaterThan(QT_MAJOR_VERSION,4):include(../qtstyleplugins/src/qtstyleplugins.pri)
greaterThan(QT_MAJOR_VERSION,4):QTPLUGIN += qplastiquestyle

# Miscellaneous Special-Testing and Cache modes that can be enabled:
#DEFINES += VERSE_LIST_PLAIN_TEXT_CACHE
#DEFINES += VERSE_LIST_RICH_TEXT_CACHE
#DEFINES += BIBLE_DATABASE_RICH_TEXT_CACHE
#DEFINES += SEARCH_PHRASE_SPY SEARCH_RESULTS_SPY
#DEFINES += SEARCH_COMPLETER_DEBUG_OUTPUT
declarative_debug:DEFINES += SIGNAL_SPY_DEBUG
#DEFINES += USE_MDI_MAIN_WINDOW

unix:!macx {
	CONFIG += static
	QMAKE_CXXFLAGS += -static
}

macx:CONFIG += x86 x86_64
lessThan(QT_MAJOR_VERSION,5):macx:static:LIBS += -lQtCore -lQtGui -lQtSql -dead_strip
greaterThan(QT_MAJOR_VERSION,4):macx:static:release:LIBS += -lQt5Core -lQt5Gui -lQt5Widgets -lQt5Sql -lQt5Xml -dead_strip
greaterThan(QT_MAJOR_VERSION,4):macx:static:debug:LIBS += -lQt5Core_debug -lQt5Gui_debug -lQt5Widgets_debug -lQt5Sql_debug -lQt5Xml_debug -dead_strip

# No longer need to have this here since we also needed to do the same thing with
#   the Qt build itself.  It was just easier to patch it into the mkspec file:
#   mkspecs/common/gcc-base-macx.conf so that we pick it up for everything...
# The following fixes a bad codegen, pointer diff to global weak in symbol vtable
#   error on Mac with the QtSharedPointer that we are using for the Bible Database.
#   It's only an issue when linking with the Static Qt we use for release:
#   (It's an XCode 4 to XCode 5 thing)
#macx:release:QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden

# On the latest XCode release with gcc mapped to Apple LLVM version 5.0 (clang-500.2.79),
#   we started getting false overloaded-virtual and unused-private-field warnings, the
#   only way I've found to disable them is to remove the warn_on feature and manually
#   output the "-Wall -W" settings it would have and add our own overrides to disable
#   these two.  Just adding the disables didn't work with warn_on still on:
macx:CONFIG -= warn_on
macx:QMAKE_CXXFLAGS += -Wall -W -Wno-overloaded-virtual -Wno-unused-private-field

#QTPLUGIN += qsqlite

TARGET = KingJamesPureBibleSearch
TEMPLATE = app

#QRegularExpression Qt5->Qt4 experimentation:
#pcre {
#	include($$PWD/Qt5/3rdparty/pcre.pri)
#	SOURCES += $$PWD/Qt5/QRegularExpression/qregularexpression.cpp
#	HEADERS += $$PWD/Qt5/QRegularExpression/qregularexpression.h
#	INCLUDEPATH += $$PWD/Qt5/QRegularExpression
#} else {
#	LIBS_PRIVATE += -lpcre16
#}

CODECFORSRC = UTF-8
CODECFORTR  = UTF-8

RC_FILE += 	KJVCanOpener.rc  # descibes program icon and version

SOURCES += main.cpp \
	KJVCanOpener.cpp \
	CSV.cpp \
	dbstruct.cpp \
	BuildDB.cpp \
	ReadDB.cpp \
	KJVBrowser.cpp \
	KJVSearchPhraseEdit.cpp \
	VerseListModel.cpp \
	KJVSearchCriteria.cpp \
	PhraseListModel.cpp \
	KJVPassageNavigator.cpp \
	KJVPassageNavigatorDlg.cpp \
	PhraseEdit.cpp \
	ScriptureEdit.cpp \
	Highlighter.cpp \
	VerseListDelegate.cpp \
	KJVAboutDlg.cpp \
	SearchPhraseListModel.cpp \
	MimeHelper.cpp \
	ToolTipEdit.cpp \
	PersistentSettings.cpp \
	QtFindReplaceDialog/dialogs/findreplaceform.cpp \
	QtFindReplaceDialog/dialogs/findreplacedialog.cpp \
	QtFindReplaceDialog/dialogs/findform.cpp \
	QtFindReplaceDialog/dialogs/finddialog.cpp \
	KJVSearchResult.cpp \
	KJVSearchSpec.cpp \
	ParseSymbols.cpp \
	VerseRichifier.cpp \
	DelayedExecutionTimer.cpp \
	BusyCursor.cpp \
	ModelRowForwardIterator.cpp \
	ReflowDelegate.cpp \
	signalspy/Q4puGenericSignalSpy.cpp \
	SearchCompleter.cpp \
	KJVConfiguration.cpp \
	UserNotesDatabase.cpp \
	SubControls.cpp \
	KJVNoteEditDlg.cpp \
	ScriptureDocument.cpp \
	NoteKeywordWidget.cpp \
	KJVCrossRefEditDlg.cpp \
	RenameHighlighterDlg.cpp \
	PassageReferenceWidget.cpp \
	DictionaryWidget.cpp

HEADERS  += main.h \
	KJVCanOpener.h \
	CSV.h \
	dbstruct.h \
	ReadDB.h \
	BuildDB.h \
	KJVBrowser.h \
	KJVSearchPhraseEdit.h \
	VerseListModel.h \
	KJVSearchCriteria.h \
	PhraseListModel.h \
	KJVPassageNavigator.h \
	KJVPassageNavigatorDlg.h \
	PhraseEdit.h \
	ScriptureEdit.h \
	Highlighter.h \
	VerseListDelegate.h \
	KJVAboutDlg.h \
	version.h \
	SearchPhraseListModel.h \
	MimeHelper.h \
	ToolTipEdit.h \
	PersistentSettings.h \
	QtFindReplaceDialog/dialogs/findreplaceform.h \
	QtFindReplaceDialog/dialogs/findreplacedialog.h \
	QtFindReplaceDialog/dialogs/findform.h \
	QtFindReplaceDialog/dialogs/finddialog.h \
	QtFindReplaceDialog/dialogs/findreplace_global.h \
	KJVSearchResult.h \
	KJVSearchSpec.h \
	ParseSymbols.h \
	VerseRichifier.h \
	DelayedExecutionTimer.h \
	BusyCursor.h \
	ModelRowForwardIterator.h \
	ReflowDelegate.h \
	signalspy/Q4puGenericSignalSpy.h \
	SearchCompleter.h \
	KJVConfiguration.h \
	UserNotesDatabase.h \
	SubControls.h \
	KJVNoteEditDlg.h \
	ScriptureDocument.h \
	ScriptureTextFormatProperties.h \
	NoteKeywordWidget.h \
	KJVCrossRefEditDlg.h \
	RenameHighlighterDlg.h \
	PassageReferenceWidget.h \
	DictionaryWidget.h

FORMS    += KJVCanOpener.ui \
	KJVBrowser.ui \
	KJVSearchPhraseEdit.ui \
	KJVSearchCriteria.ui \
	KJVPassageNavigator.ui \
	KJVPassageNavigatorDlg.ui \
	KJVAboutDlg.ui \
	QtFindReplaceDialog/dialogs/findreplaceform.ui \
	QtFindReplaceDialog/dialogs/findreplacedialog.ui \
	KJVSearchSpec.ui \
	KJVTextFormatConfig.ui \
	KJVNoteEditDlg.ui \
	NoteKeywordWidget.ui \
	KJVCrossRefEditDlg.ui \
	KJVBibleDatabaseConfig.ui \
	KJVUserNotesDatabaseConfig.ui \
	KJVGeneralSettingsConfig.ui \
	ConfigSearchOptions.ui \
	ConfigBrowserOptions.ui \
	ConfigDictionaryOptions.ui \
	ConfigCopyOptions.ui \
	RenameHighlighterDlg.ui \
	PassageReferenceWidget.ui \
	DictionaryWidget.ui

RESOURCES += \
	KJVCanOpener.qrc

# ICON for Mac OSX:
ICON = res/bible.icns

# Info.plist for Mac OSX:
# This is broken in qmake.  Copy KJVCanOpener.Info.plist.app to ~/Qt/.../mkspecs/default/Info.plist.app
#QMAKE_INFO_PLIST = KJVCanOpener.Info.plist.app

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
ANDROID_PACKAGE = org.dewtronics.KingJamesPureBibleSearch
ANDROID_MINIMUM_VERSION = 8
ANDROID_TARGET_VERSION = 18
ANDROID_APP_NAME = King James Pure Bible Search

android:OTHER_FILES += \
	android/AndroidManifest.xml


android_install {
#       plugins.files += $$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlite.so
#       plugins.path = /data/plugins

#       dbDeploy.files = ../KJVCanOpener/db/*.s3db
#       dbDeploy.path = /data/db
#       fontDeploy.files = ../KJVCanOpener/fonts/DejaVu*.ttf ../KJVCanOpener/fonts/SCRIPTBL.TTF
#       fontDeploy.path = /data/fonts
#       docDeploy.files = ../KJVCanOpener/doc/KingJamesPureBibleSearch.pdf
#       docDeploy.path = /data/doc

		dbDeploy.files = ../KJVCanOpener/db/kjvtext.s3db ../KJVCanOpener/db/kjvuser.s3db ../KJVCanOpener/db/dct-web1828.s3db
		dbDeploy.path = /assets/KJVCanOpener/db
		fontDeploy.files = ../KJVCanOpener/fonts/DejaVu*.ttf ../KJVCanOpener/fonts/SCRIPTBL.TTF
		fontDeploy.path = /assets/KJVCanOpener/fonts
		docDeploy.files = ../KJVCanOpener/doc/KingJamesPureBibleSearch.pdf
		docDeploy.path = /assets/KJVCanOpener/doc

#		pushDeploy.files = ../KJVCanOpener/android_push.sh
#		pushDeploy.path = /

#		myData.files = ../KJVCanOpener/android/data/*
#       myData.path = /data

#		DEPLOYMENT += plugins dbDeploy fontDeploy docDeploy
#		DEPLOYMENT += plugins myData
#		DEPLOYMENT_PLUGIN += qsqlite
#		INSTALLS += dbDeploy fontDeploy docDeploy pushDeploy
		INSTALLS += dbDeploy fontDeploy docDeploy

}

message($$CONFIG)
