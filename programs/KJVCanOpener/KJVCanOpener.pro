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

QT       += core gui xml
!emscripten {
	QT += sql
} else {
	DEFINES += NOT_USING_SQL
}
greaterThan(QT_MAJOR_VERSION,4):QT+=widgets

!emscripten {
	CONFIG += wwwidgets
}

win32 {
	CONFIG += rtti
	CONFIG -= debug_and_release							# Get rid of double debug/release subfolders and do correct shadow build
	equals(MSVC_VER, "12.0"):QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01		# Enable Support for WinXP if we are building with MSVC 2013, as MSVC 2010 already does
}

exceptions_off:DEFINES += NOT_USING_EXCEPTIONS

!android:!ios:!emscripten:!vnc {
	CONFIG += buildKJVDatabase
	DEFINES += BUILD_KJV_DATABASE
}

if(android | ios):DEFINES += IS_MOBILE_APP

!android:!ios:!emscripten:!vnc:CONFIG += QtSpeech			# Enable Text-To-Speech support

#QRegularExpression Qt5->Qt4 experimentation:
#CONFIG += pcre

unix:!mac:!vnc {
	CONFIG += static
	QMAKE_CXXFLAGS += -static
}

android {
# These (or parts of these) only needed for JNI calls and our intense experimentation with the Android filesystem:
#	QT += androidextras
#	include(../qtassetsmanager/qtassetsmanager.pri)
#	LIBS += -landroid
}

include(../qtiocompressor/src/qtiocompressor.pri)
include(../grantlee/textdocument/textdocument.pri)

!android:!ios:!emscripten:!vnc {
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
vnc:QT += network

QtSpeech {
	DEFINES += USING_QT_SPEECH
	include(../qt-speech/QtSpeech.pri)
}


# Add the Plastique Style:
# In Qt4, Plastique is built-in to Qt itself:
lessThan(QT_MAJOR_VERSION,5):DEFINES += PLASTIQUE_STATIC
# Disable on iOS until we get make further progress on the port:
!ios:greaterThan(QT_MAJOR_VERSION,4) {
	static {
		include(../qtstyleplugins/src/qtstyleplugins.pri)
		DEFINES += PLASTIQUE_STATIC
	} else {
		# Use Dynamic Plugin Style:
		QTPLUGIN += qplastiquestyle
		DEFINES -= PLASTIQUE_STATIC
	}
}

# The following is absolutely needed on Qt5.2.0 and Qt5.2.1 (on Mac), or else we'll crash in the
#	failure of the accessibility factory from creating an accessibility
#	object.  Should probably be there for all platforms to make sure the
#	accessibility support gets loaded:
# Was fixed on the Mac in Qt5.3.0 -- having this defined caused a (non-fatal) redundant definition:
!emscripten:!win32:if(!contains(QT_CONFIG, static) | lessThan(QT_MAJOR_VERSION,5) | equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,3)):QTPLUGIN += qtaccessiblewidgets

# Miscellaneous Special-Testing and Cache modes that can be enabled:
#DEFINES += VERSE_LIST_PLAIN_TEXT_CACHE
#DEFINES += VERSE_LIST_RICH_TEXT_CACHE
#DEFINES += BIBLE_DATABASE_RICH_TEXT_CACHE
#DEFINES += SEARCH_PHRASE_SPY SEARCH_RESULTS_SPY
#DEFINES += SEARCH_COMPLETER_DEBUG_OUTPUT
!emscripten:!vnc:declarative_debug:DEFINES += SIGNAL_SPY_DEBUG
#DEFINES += USE_MDI_MAIN_WINDOW
#DEFINES += DEBUG_CURSOR_SELECTION
#DEFINES += DEBUG_SEARCH_RESULTS_NODE_TOOLTIPS
#CONFIG += modeltest				# Enable to turn on QAbstractItemModel testing for CVerseListModel

# Special QAbstractItemModel test suite:
modeltest {
	include(../modeltest/modeltest.pri)
	DEFINES += MODELTEST
}

# Enable workarounds for some QTBUGs:
DEFINES += WORKAROUND_QTBUG_13768											# Hover attribute for QSplitter
equals(QT_MAJOR_VERSION,5):if(lessThan(QT_MINOR_VERSION,2) | equals(QT_MINOR_VERSION,2):equals(QT_PATCH_VERSION,0)):DEFINES += WORKAROUND_QTBUG_33906			# singleStep QTreeView Scroll Bug
ios:greaterThan(QT_MAJOR_VERSION,4):CONFIG += WORKAROUND_QTBUG_34490		# iOS Font Bug
ios:greaterThan(QT_MAJOR_VERSION,4):DEFINES += WORKAROUND_QTBUG_35787		# iOS SplashScreen Bug
greaterThan(QT_MAJOR_VERSION,4):DEFINES += WORKAROUND_QTBUG_BROWSER_BOUNCE	# Not a submitted Qt bug, that I know of, but a Qt 5 bug
macx:lessThan(QT_MAJOR_VERSION,5):DEFINES += WORKAROUND_QTBUG_32789			# Qt 4 Font Bug on MacX Mavericks
android:DEFINES += WORKAROUND_QTBUG_35313_35687								# Android QMessageBox dialogs have wrong theming (workaround uses Android Native Dialogs)
lessThan(QT_MAJOR_VERSION,5):DEFINES += WORKAROUND_QTBUG_ABSLIST_DROP_ACTIONS		# Workaround in Qt 4 for the Abstract List Drop Actions issue that causes it to not do drops into other apps without this returning Copy or Move

# Enable Splash Screen:
!vnc:DEFINES += SHOW_SPLASH_SCREEN

# Enabled desired random passage mode:
# (Pick only one of RANDOM_PASSAGE entries)
#DEFINES += RANDOM_PASSAGE_VERSE_WEIGHT			# Weigh passages evenly by verses (books with more verses picked more often)
DEFINES += RANDOM_PASSAGE_EVEN_WEIGHT			# Weigh passages evenly by book/chapter/verse (pick book, then chapter, then verse)

# Enable to only show loaded Bible Databases in New Search Window action:
#DEFINES += ENABLE_ONLY_LOADED_BIBLE_DATABASES

# Enable to only show loaded Dictionary Databases in the Select Dictionary action:
#DEFINES += ENABLE_ONLY_LOADED_DICTIONARY_DATABASES

# Enable Loading of our Application Fonts (Note: Emscripten uses auto-loading of .qpf fonts from deployed qt-fonts folder):
!emscripten:DEFINES += LOAD_APPLICATION_FONTS

# Enable Asynchronous Dialogs
#if(emscripten | macx):DEFINES += USE_ASYNC_DIALOGS
emscripten:DEFINES += USE_ASYNC_DIALOGS

# Enable Gesture/TouchDevice processing:
if(ios | android):greaterThan(QT_MAJOR_VERSION,4):DEFINES += TOUCH_GESTURE_PROCESSING
#greaterThan(QT_MAJOR_VERSION,4):DEFINES += TOUCH_GESTURE_PROCESSING

# Saving/Restoring of KJVCanOpener Window State/Geometry and Splitter State:
!android:!ios:DEFINES += PRESERVE_MAINWINDOW_GEOMETRY						# Physical size and layout of KJVCanOpener
DEFINES += PRESERVE_MAINWINDOW_STATE										# Toolbars and DockWidgets of KJVCanOpener
!android:!ios:DEFINES += PRESERVE_MAINWINDOW_SPLITTER_STATE					# Splitters on KJVCanOpener
!android:!ios:DEFINES += PRESERVE_DIALOG_GEOMETRY							# Physical size and layout of UserDatabase editor dialogs for Notes and CrossRefs

# Enable Search Phrase Auto-Completer Popup Delay:
DEFINES += USE_SEARCH_PHRASE_COMPLETER_POPUP_DELAY							# Enable to delay search phrase completer popup based on the search phrase activation time

# Enable Search Results Update Delay:
#DEFINES += USE_SEARCH_RESULTS_UPDATE_DELAY									# Enable to delay extra staged search results update delay after the delayed phrase change notification is received

# Enable Multithreaded Search Results:
DEFINES += USE_MULTITHREADED_SEARCH_RESULTS									# Enable to spawn separate thread to handle calculation of Scoped Search Results for the VerseListModel
DEFINES += INVERT_MULTITHREADED_LOGIC										# Invert the defaults of single vs. multi threaded when USE_MULTITHREADED_SEARCH_RESULTS is enabled (with this Single is default without command-line option instead of Multi)

# Enable Sandboxing on Mac:
macx {
# TODO : Enable this once we figure out a mechanism of calling the
#		sandbox interaction functions to keep the NS Keys for our
#		notes files:
#	DEFINES += IN_MAC_SANDBOX
#	CONFIG += sandboxed
}

lessThan(QT_MAJOR_VERSION,5):macx:CONFIG += x86 x86_64
greaterThan(QT_MAJOR_VERSION,4):macx:static:CONFIG += x86
greaterThan(QT_MAJOR_VERSION,4):macx:!static:CONFIG += x86_64
lessThan(QT_MAJOR_VERSION,5):macx:static:LIBS += -lQtCore -lQtGui -lQtSql -dead_strip
greaterThan(QT_MAJOR_VERSION,4):macx:static:!declarative_debug:LIBS += -lQt5Core -lQt5Gui -lQt5Widgets -lQt5Sql -lQt5Xml -dead_strip
greaterThan(QT_MAJOR_VERSION,4):macx:static:declarative_debug:LIBS += -lQt5Core_debug -lQt5Gui_debug -lQt5Widgets_debug -lQt5Sql_debug -lQt5Xml_debug -dead_strip

ios {
	QMAKE_IOS_DEVICE_ARCHS = armv7
	QMAKE_IOS_SIMULATOR_ARCHS = i386
}

# No longer need to have this here since we also needed to do the same thing with
#   the Qt build itself.  It was just easier to patch it into the mkspec file:
#   mkspecs/common/gcc-base-macx.conf so that we pick it up for everything...
# The following fixes a bad codegen, pointer diff to global weak in symbol vtable
#   error on Mac with the QtSharedPointer that we are using for the Bible Database.
#   It's only an issue when linking with the Static Qt we use for release:
#   (It's an XCode 4 to XCode 5 thing)
#macx:release:QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden

# On the latest XCode release with gcc mapped to Apple LLVM version 5.0 (clang-500.2.79),
#   we started getting false unused-private-field warnings from Qt source on Qt4.8.x, the
#   only way I've found to disable them is to remove the warn_on feature and manually
#   output the "-Wall -W" settings it would have and add our own overrides to disable
#   these two.  Just adding the disables didn't work with warn_on still on:
lessThan(QT_MAJOR_VERSION,5) {
	mac:CONFIG -= warn_on
	mac:QMAKE_CXXFLAGS += -Wall -W -Wno-unused-private-field
}

# On Qt 5.3.0, they apparently fixed this where we don't need qsqlite, and adding this
#	on that version causes "Redundant entries in QTPLUGIN: qsqlite"
ios:if(!contains(QT_CONFIG, static)):if(lessThan(QT_MAJOR_VERSION,5) | equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,3)):QTPLUGIN += qsqlite

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

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD


TRANSLATIONS += \
	translations/kjpbs.en.ts \
	translations/kjpbs.fr.ts \
	translations/kjpbs.es.ts \
	translations/kjpbs.de.ts

# Qt Translation Files:
# Define these in a new name so that lupdate, etc, won't pick them up, since they
#	aren't part of the code.  This is needed because Qt4 -> Qt5 broke the namespace
#	for QDialogButtonBox, changing it to QPlatformTheme and yet they didn't properly
#	include ES and FR translations for them, only DE.  So, we'll just manually build
#	all of the translations and deploy with ours:

greaterThan(QT_MAJOR_VERSION,4) {
	TRANSLATIONS_QT += \
		translations/qtbase_fr.ts \
		translations/qtbase_es.ts \
		translations/qtbase_de.ts
		# English is native and doesn't have a separate translation file
} else {
	TRANSLATIONS_QT += \
		translations/qt_fr.ts \
		translations/qt_es.ts \
		translations/qt_de.ts
		# English is native and doesn't have a separate translation file
}

# wwWidgets Translation Files:
# Define these in a new name so that lupdate, etc, won't pick them up, since they
#	aren't part of this project (they are built via the wwWidgets4 project):
	TRANSLATIONS_WWWIDGETS4_QM += \
		$$[QT_INSTALL_TRANSLATIONS]/wwwidgets_en.qm \
		$$[QT_INSTALL_TRANSLATIONS]/wwwidgets_fr.qm \
		$$[QT_INSTALL_TRANSLATIONS]/wwwidgets_es.qm \
		$$[QT_INSTALL_TRANSLATIONS]/wwwidgets_de.qm


SOURCES += \
	main.cpp \
	myApplication.cpp \
	BibleDBListModel.cpp \
	BibleWordDiffListModel.cpp \
	BusyCursor.cpp \
	CSV.cpp \
	dbDescriptors.cpp \
	dbstruct.cpp \
	DelayedExecutionTimer.cpp \
	DictDBListModel.cpp \
	Highlighter.cpp \
	KJVAboutDlg.cpp \
	KJVBrowser.cpp \
	KJVCanOpener.cpp \
	KJVPassageNavigator.cpp \
	KJVPassageNavigatorDlg.cpp \
	KJVSearchCriteria.cpp \
	KJVSearchPhraseEdit.cpp \
	KJVSearchResult.cpp \
	KJVSearchSpec.cpp \
	MimeHelper.cpp \
	ModelRowForwardIterator.cpp \
	NoteKeywordWidget.cpp \
	ParseSymbols.cpp \
	PassageReferenceWidget.cpp \
	PersistentSettings.cpp \
	PhraseEdit.cpp \
	PhraseListModel.cpp \
	QtFindReplaceDialog/dialogs/finddialog.cpp \
	QtFindReplaceDialog/dialogs/findform.cpp \
	QtFindReplaceDialog/dialogs/findreplaceform.cpp \
	QtFindReplaceDialog/dialogs/findreplacedialog.cpp \
	ReadDB.cpp \
	ReflowDelegate.cpp \
	ReportError.cpp \
	ScriptureDocument.cpp \
	ScriptureEdit.cpp \
	SearchCompleter.cpp \
	SearchPhraseListModel.cpp \
	SubControls.cpp \
	ToolTipEdit.cpp \
	Translator.cpp \
	UserNotesDatabase.cpp \
	VerseListModel.cpp \
	VerseListDelegate.cpp \
	VerseRichifier.cpp

buildKJVDatabase:SOURCES += \
	BuildDB.cpp

!emscripten:SOURCES += \
	BibleDatabaseInfoDlg.cpp \
	DictDatabaseInfoDlg.cpp \
	DictionaryWidget.cpp \
	KJVConfiguration.cpp \
	RenameHighlighterDlg.cpp

!emscripten:!vnc:SOURCES += \
	EditWWWLinkDlg.cpp \
	KJVCrossRefEditDlg.cpp \
	KJVNoteEditDlg.cpp \
	SaveLoadFileDialog.cpp \
	signalspy/Q4puGenericSignalSpy.cpp


HEADERS += \
	myApplication.h \
	BibleDBListModel.h \
	BibleWordDiffListModel.h \
	BusyCursor.h \
	CSV.h \
	dbDescriptors.h \
	dbstruct.h \
	DelayedExecutionTimer.h \
	DictDBListModel.h \
	Highlighter.h \
	KJVAboutDlg.h \
	KJVBrowser.h \
	KJVCanOpener.h \
	KJVPassageNavigator.h \
	KJVPassageNavigatorDlg.h \
	KJVSearchCriteria.h \
	KJVSearchPhraseEdit.h \
	KJVSearchResult.h \
	KJVSearchSpec.h \
	MimeHelper.h \
	ModelRowForwardIterator.h \
	NoteKeywordWidget.h \
	ParseSymbols.h \
	PassageReferenceWidget.h \
	PersistentSettings.h \
	PhraseEdit.h \
	PhraseListModel.h \
	QtFindReplaceDialog/dialogs/finddialog.h \
	QtFindReplaceDialog/dialogs/findform.h \
	QtFindReplaceDialog/dialogs/findreplaceform.h \
	QtFindReplaceDialog/dialogs/findreplacedialog.h \
	QtFindReplaceDialog/dialogs/findreplace_global.h \
	ReadDB.h \
	ReflowDelegate.h \
	ReportError.h \
	ScriptureDocument.h \
	ScriptureEdit.h \
	ScriptureTextFormatProperties.h \
	SearchCompleter.h \
	SearchPhraseListModel.h \
	SubControls.h \
	ThreadedSearchResults.h \
	ToolTipEdit.h \
	Translator.h \
	UserNotesDatabase.h \
	VerseListModel.h \
	VerseListDelegate.h \
	VerseRichifier.h \
	version.h

buildKJVDatabase:HEADERS += \
	BuildDB.h

!emscripten:HEADERS += \
	BibleDatabaseInfoDlg.h \
	DictDatabaseInfoDlg.h \
	DictionaryWidget.h \
	KJVConfiguration.h \
	RenameHighlighterDlg.h

!emscripten:!vnc:HEADERS += \
	EditWWWLinkDlg.h \
	KJVCrossRefEditDlg.h \
	KJVNoteEditDlg.h \
	SaveLoadFileDialog.h \
	signalspy/Q4puGenericSignalSpy.h


FORMS += \
	KJVAboutDlg.ui \
	KJVBrowser.ui \
	KJVCanOpener.ui \
	KJVPassageNavigator.ui \
	KJVPassageNavigatorDlg.ui \
	KJVSearchCriteria.ui \
	KJVSearchPhraseEdit.ui \
	KJVSearchSpec.ui \
	NoteKeywordWidget.ui \
	PassageReferenceWidget.ui \
	QtFindReplaceDialog/dialogs/findreplaceform.ui \
	QtFindReplaceDialog/dialogs/findreplacedialog.ui

!emscripten:FORMS += \
	BibleDatabaseInfoDlg.ui \
	ConfigBrowserOptions.ui \
	ConfigCopyOptions.ui \
	ConfigDictionaryOptions.ui \
	ConfigSearchOptions.ui \
	DictDatabaseInfoDlg.ui \
	DictionaryWidget.ui \
	KJVBibleDatabaseConfig.ui \
	KJVDictDatabaseConfig.ui \
	KJVGeneralSettingsConfig.ui \
	KJVLocaleConfig.ui \
	KJVTextFormatConfig.ui \
	RenameHighlighterDlg.ui

!emscripten:!vnc:FORMS += \
	EditWWWLinkDlg.ui \
	KJVCrossRefEditDlg.ui \
	KJVNoteEditDlg.ui \
	KJVUserNotesDatabaseConfig.ui


RESOURCES += \
	KJVCanOpener.qrc

# ICON for Mac OSX/iOS:
mac:ICON = res/bible.icns

# Info.plist for Mac OSX:
# This is broken in qmake (on Qt4).  Copy KJVCanOpener.Info.plist.app to ~/Qt/.../mkspecs/default/Info.plist.app
macx:greaterThan(QT_MAJOR_VERSION,4):QMAKE_INFO_PLIST = KJVCanOpener.Info.plist.app
# Temporary workaround for QTBUG-34490:	https://bugreports.qt-project.org/browse/QTBUG-34490
#	We'll add the fonts to the Info.plist so iOS will auto-load them for us:
ios:greaterThan(QT_MAJOR_VERSION,4) {
	WORKAROUND_QTBUG_34490 {
		QMAKE_INFO_PLIST = KJVCanOpener.iOS.fonts.Info.plist.app
		DEFINES += WORKAROUND_QTBUG_34490
	} else {
		QMAKE_INFO_PLIST = KJVCanOpener.iOS.Info.plist.app
	}
}

###############################################################################


# Build Translations:
!isEmpty(TRANSLATIONS) {
	DEFINES+=HAVE_TRANSLATIONS
	for(f, TRANSLATIONS):translationDeploy.files += $$quote($${PWD}/$$replace(f, .ts, .qm))
	for(f, TRANSLATIONS):translation_source.files += $$quote($${PWD}/$$f)
	!isEmpty(TRANSLATIONS_QT) {
		for(f, TRANSLATIONS_QT):translationDeploy.files += $$quote($${PWD}/$$replace(f, .ts, .qm))
		for(f, TRANSLATIONS_QT):translation_source.files += $$quote($${PWD}/$$f)
	}
	!isEmpty(TRANSLATIONS_WWWIDGETS4_QM) {
		translationDeploy.files += $$TRANSLATIONS_WWWIDGETS4_QM
	}
	exists($$[QT_INSTALL_BINS]/lrelease) {
		translation_build.output = $$translationDeploy.files
		translation_build.target = $$translationDeploy.files
		translation_build.input = $$translation_source.files
		translation_build.depends = $$translation_source.files
#		translation_build.commands = $$quote($$[QT_INSTALL_BINS]/lrelease $$_PRO_FILE_$$escape_expand(\\n\\t))
		trnaslation_build.commands = $$quote($$[QT_INSTALL_BINS]/lrelease $$translation_source.files $$escape_expand(\\n\\t))
		translation_build.CONFIG = no_link
		QMAKE_EXTRA_TARGETS += translation_build $$translation_source.files
		QMAKE_EXTRA_COMPILERS += translation_build
		POST_TARGETDEPS +=  $$translation_source.files
#		QMAKE_POST_LINK += $$quote($$[QT_INSTALL_BINS]/lrelease $$_PRO_FILE_$$escape_expand(\\n\\t))
		QMAKE_POST_LINK += $$quote($$[QT_INSTALL_BINS]/lrelease $$translation_source.files $$escape_expand(\\n\\t))
		for(f, TRANSLATIONS_WWWIDGETS4_QM):QMAKE_POST_LINK += $$quote(cp $$f ../../KJVCanOpener/translations/$$basename(f) $$escape_expand(\\n\\t))
	} else {
		message("Can't build translations!  Using previously built translations if possible")
	}
	unix:!mac {
		translationDeploy.path = .
#		QMAKE_POST_LINK += $$quote(cp $$translationDeploy.files $$translationDeploy.path$$escape_expand(\\n\\t))
	}
	#INSTALLS += translationDeploy
	message("Deploying translations:" $$TRANSLATIONS$$TRANSLATIONS_QT$$TRANSLATIONS_WWWIDGETS4_QM$$escape_expand(\\n))
}

###############################################################################

android {
	ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
	ANDROID_PACKAGE = org.dewtronics.KingJamesPureBibleSearch
	ANDROID_MINIMUM_VERSION = 11
	ANDROID_TARGET_VERSION = 18
	ANDROID_APP_NAME = King James Pure Bible Search

	OTHER_FILES += \
		android/AndroidManifest.xml \
		android/res/values/assets.xml

	android_install {
		dbDeploy.files += ../KJVCanOpener/db/bbl-kjv1769.ccdb
		dbDeploy.files += ../KJVCanOpener/db/dct-web1828.s3db
#		dbDeploy.files += ../KJVCanOpener/db/dct-web1913.s3db
		dbDeploy.path = /assets/KJVCanOpener/db
		fontDeploy.files = ../KJVCanOpener/fonts/DejaVu*.ttf ../KJVCanOpener/fonts/SCRIPTBL.TTF
		fontDeploy.path = /assets/KJVCanOpener/fonts
		docDeploy.files += ../KJVCanOpener/doc/KingJamesPureBibleSearch.pdf
#		docDeploy.files += ../KJVCanOpener/kjvdatagen/kjv_summary.xls
#		docDeploy.files += ../KJVCanOpener/articles/kjv_stats.xls
		docDeploy.path = /assets/KJVCanOpener/doc
		examplesDeploy.files = ../KJVCanOpener/examples/example*.kjs
		examplesDeploy.path = /assets/KJVCanOpener/examples
		licDeploy.files = ../KJVCanOpener/gpl-3.0.txt
		licDeploy.path = /assets/KJVCanOpener/license

#		INSTALLS += dbDeploy fontDeploy docDeploy examplesDeploy licDeploy
		INSTALLS += dbDeploy fontDeploy licDeploy
		!isEmpty(TRANSLATIONS) {
			translationDeploy.path = /assets/KJVCanOpener/translations
			INSTALLS += translationDeploy
		}
	}
}

ios {
	app_bundle {
		# Note: For some reason, wildcards don't work with the builtin-copy operation on Mac/iOS
		#		so we have to explicitly name each file:
		iconDeploy.files = ../../KJVCanOpener/res/bible_64.png
		iconDeploy.path = .
		dbDeploy.files +=  ../../KJVCanOpener/db/bbl-kjv1769.ccdb
		dbDeploy.files += ../../KJVCanOpener/db/dct-web1828.s3db
#		dbDeploy.files += ../../KJVCanOpener/db/dct-web1913.s3db
		dbDeploy.path = /assets/KJVCanOpener/db
		fontDeploy.files += ../../KJVCanOpener/fonts/SCRIPTBL.TTF
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-Oblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-Italic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif.ttf
		fontDeploy.path = /assets/KJVCanOpener/fonts
		docDeploy.files += ../../KJVCanOpener/doc/KingJamesPureBibleSearch.pdf
#		docDeploy.files += ../../KJVCanOpener/kjvdatagen/kjv_summary.xls
#		docDeploy.files += ../../KJVCanOpener/articles/kjv_stats.xls
		docDeploy.path = /assets/KJVCanOpener/doc
		examplesDeploy.files += ../../KJVCanOpener/examples/example01.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example02.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example03.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example04.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example05.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example06.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example07.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example08.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example09.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example10.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example11.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example12.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example13.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example14.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example15.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example16.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example17.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example18.kjs
		examplesDeploy.path = /assets/KJVCanOpener/examples
		licDeploy.files = ../../KJVCanOpener/gpl-3.0.txt
		licDeploy.path = /assets/KJVCanOpener/license

		QMAKE_BUNDLE_DATA += iconDeploy dbDeploy fontDeploy docDeploy examplesDeploy licDeploy
		!isEmpty(TRANSLATIONS) {
			translationDeploy.path = /assets/KJVCanOpener/translations
			QMAKE_BUNDLE_DATA += translationDeploy
		}
	}
}

macx {
	app_bundle {
		# Note: For some reason, wildcards don't work with the builtin-copy operation on Mac/iOS
		#		so we have to explicitly name each file:
		nibDeploy.files += $$[QT_INSTALL_PREFIX]/src/gui/mac/qt_menu.nib/classes.nib
		nibDeploy.files += $$[QT_INSTALL_PREFIX]/src/gui/mac/qt_menu.nib/info.nib
		nibDeploy.files += $$[QT_INSTALL_PREFIX]/src/gui/mac/qt_menu.nib/keyedobjects.nib
		nibDeploy.path = /Contents/Resources/qt_menu.nib
		dbDeploy.files +=  ../../KJVCanOpener/db/bbl-kjv1769.ccdb
		dbDeploy.files +=  ../../KJVCanOpener/db/bbl-rvg2010-20140705.ccdb
		dbDeploy.files +=  ../../KJVCanOpener/db/dct-web1828.s3db
		dbDeploy.files +=  ../../KJVCanOpener/db/dct-web1913.s3db
		dbDeploy.path = /Contents/Resources/db
		fontDeploy.files += ../../KJVCanOpener/fonts/SCRIPTBL.TTF
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-Oblique.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-Italic.ttf
		fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif.ttf
		fontDeploy.path = /Contents/Resources/fonts
		docDeploy.files += ../../KJVCanOpener/doc/KingJamesPureBibleSearch.pdf
		docDeploy.files += ../../KJVCanOpener/kjvdatagen/kjv_summary.xls
		docDeploy.files += ../../KJVCanOpener/articles/kjv_stats.xls
		docDeploy.path = /Contents/SharedSupport/doc
		examplesDeploy.files += ../../KJVCanOpener/examples/example01.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example02.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example03.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example04.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example05.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example06.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example07.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example08.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example09.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example10.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example11.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example12.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example13.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example14.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example15.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example16.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example17.kjs
		examplesDeploy.files += ../../KJVCanOpener/examples/example18.kjs
		examplesDeploy.path = /Contents/SharedSupport/examples
		licDeploy.files = ../../KJVCanOpener/gpl-3.0.txt
		licDeploy.path = /Contents/SharedSupport/license

		QMAKE_BUNDLE_DATA += nibDeploy dbDeploy fontDeploy docDeploy examplesDeploy licDeploy
		!isEmpty(TRANSLATIONS) {
			translationDeploy.path = /Contents/Resources/translations
			QMAKE_BUNDLE_DATA += translationDeploy
		}

		greaterThan(QT_MAJOR_VERSION,4):shared:qt_framework {
			exists($$[QT_INSTALL_BINS]/macdeployqt) {
				QMAKE_POST_LINK += $$quote(@echo Deploying...$$escape_expand(\\n\\t))
				contains(QTPLUGIN, qplastiquestyle) {
					QMAKE_POST_LINK += $$quote(mkdir -p KingJamesPureBibleSearch.app/Contents/PlugIns/styles$$escape_expand(\\n\\t))
					QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_PLUGINS]/styles/libqplastiquestyle.dylib $${TARGET}.app/Contents/PlugIns/styles/$$escape_expand(\\n\\t))
					QMAKE_POST_LINK += $$quote($$[QT_INSTALL_BINS]/macdeployqt $${TARGET}.app -executable=$${TARGET}.app/Contents/PlugIns/styles/libqplastiquestyle.dylib$$escape_expand(\\n\\t))
				} else {
					QMAKE_POST_LINK += $$quote($$[QT_INSTALL_BINS]/macdeployqt $${TARGET}.app$$escape_expand(\\n\\t))
				}
			} else {
				error("Can't deploy Qt Framework bundle!")
			}

			# Copy in the Info.plist files to the Resources folders
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/QtCore.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtCore.framework/Resources/$$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/QtGui.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtGui.framework/Resources/$$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/QtNetwork.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtNetwork.framework/Resources/$$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/QtPrintSupport.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtPrintSupport.framework/Resources/$$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/QtSql.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtSql.framework/Resources/$$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/QtWidgets.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtWidgets.framework/Resources/$$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/QtXml.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtXml.framework/Resources/$$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(cp $$[QT_INSTALL_LIBS]/wwwidgets4.framework/Contents/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/wwwidgets4.framework/Resources/$$escape_expand(\\n\\t))

			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/QtCore/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtCore.framework/Resources/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/QtGui/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtGui.framework/Resources/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/QtNetwork/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtNetwork.framework/Resources/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/QtPrintSupport/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtPrintSupport.framework/Resources/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/QtSql/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtSql.framework/Resources/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/QtWidgets/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtWidgets.framework/Resources/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/QtXml/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/QtXml.framework/Resources/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(cp ../../KJVCanOpener/macxsign_qt5_plists/wwwidgets4/Info.plist KingJamesPureBibleSearch.app/Contents/Frameworks/wwwidgets4.framework/Resources/$$escape_expand(\\n\\t))

			# Now, move the Resources folders to the correct location
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/QtCore.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtCore.framework/Versions/$${QT_MAJOR_VERSION}/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/QtGui.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtGui.framework/Versions/$${QT_MAJOR_VERSION}/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/QtNetwork.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtNetwork.framework/Versions/$${QT_MAJOR_VERSION}/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/QtPrintSupport.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtPrintSupport.framework/Versions/$${QT_MAJOR_VERSION}/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/QtSql.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtSql.framework/Versions/$${QT_MAJOR_VERSION}/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/QtWidgets.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtWidgets.framework/Versions/$${QT_MAJOR_VERSION}/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/QtXml.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtXml.framework/Versions/$${QT_MAJOR_VERSION}/$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(mv KingJamesPureBibleSearch.app/Contents/Frameworks/wwwidgets4.framework/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/wwwidgets4.framework/Versions/1/$$escape_expand(\\n\\t))

			# Now, create symbolic links for the "Current" version:
			QMAKE_POST_LINK += $$quote(ln -s $${QT_MAJOR_VERSION} KingJamesPureBibleSearch.app/Contents/Frameworks/QtCore.framework/Versions/Current$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s $${QT_MAJOR_VERSION} KingJamesPureBibleSearch.app/Contents/Frameworks/QtGui.framework/Versions/Current$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s $${QT_MAJOR_VERSION} KingJamesPureBibleSearch.app/Contents/Frameworks/QtNetwork.framework/Versions/Current$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s $${QT_MAJOR_VERSION} KingJamesPureBibleSearch.app/Contents/Frameworks/QtPrintSupport.framework/Versions/Current$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s $${QT_MAJOR_VERSION} KingJamesPureBibleSearch.app/Contents/Frameworks/QtSql.framework/Versions/Current$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s $${QT_MAJOR_VERSION} KingJamesPureBibleSearch.app/Contents/Frameworks/QtWidgets.framework/Versions/Current$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s $${QT_MAJOR_VERSION} KingJamesPureBibleSearch.app/Contents/Frameworks/QtXml.framework/Versions/Current$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s 1 KingJamesPureBibleSearch.app/Contents/Frameworks/wwwidgets4.framework/Versions/Current$$escape_expand(\\n\\t))

			# Create the top level symbolic links for "Resources":
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtCore.framework/Resources$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtGui.framework/Resources$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtNetwork.framework/Resources$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtPrintSupport.framework/Resources$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtSql.framework/Resources$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtWidgets.framework/Resources$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/QtXml.framework/Resources$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/Resources KingJamesPureBibleSearch.app/Contents/Frameworks/wwwidgets4.framework/Resources$$escape_expand(\\n\\t))

			# Create the top level symbolic links for Framework Executables:
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/QtCore KingJamesPureBibleSearch.app/Contents/Frameworks/QtCore.framework/QtCore$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/QtGui KingJamesPureBibleSearch.app/Contents/Frameworks/QtGui.framework/QtGui$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/QtNetwork KingJamesPureBibleSearch.app/Contents/Frameworks/QtNetwork.framework/QtNetwork$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/QtPrintSupport KingJamesPureBibleSearch.app/Contents/Frameworks/QtPrintSupport.framework/QtPrintSupport$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/QtSql KingJamesPureBibleSearch.app/Contents/Frameworks/QtSql.framework/QtSql$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/QtWidgets KingJamesPureBibleSearch.app/Contents/Frameworks/QtWidgets.framework/QtWidgets$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/QtXml KingJamesPureBibleSearch.app/Contents/Frameworks/QtXml.framework/QtXml$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(ln -s Versions/Current/wwwidgets4 KingJamesPureBibleSearch.app/Contents/Frameworks/wwwidgets4.framework/wwwidgets4$$escape_expand(\\n\\t))

			# Keep unsigned version so we have one we can distribute to people that doesn't leave signature footprints:
			QMAKE_POST_LINK += $$quote(cp -R KingJamesPureBibleSearch.app KingJamesPureBibleSearch_unsigned.app$$escape_expand(\\n\\t))

			sandboxed {
				QMAKE_POST_LINK += $$quote(../../KJVCanOpener/macxsign_qt5_bundle_sandboxed$$escape_expand(\\n\\t))
			} else {
				QMAKE_POST_LINK += $$quote(../../KJVCanOpener/macxsign_qt5_bundle_unsandboxed$$escape_expand(\\n\\t))
			}
		}
	}
}

win32:!declarative_debug:equals(MAKEFILE_GENERATOR, "MSBUILD") {
#
# For Windows:
# Be sure to define environment variables for:
#			VCINSTALLDIR	(ex:  C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\)
#			QTDIR			(ex:  C:\Qt\5.3.1-msvc2013-opengl\5.3\msvc2013_opengl)
#
# Add: "%QTDIR%/bin" to %PATH%
#
	WINBUILD = $${PWD}/winbuild

	dbDeploy.files +=  ../../KJVCanOpener/db/bbl-kjv1769.ccdb
	dbDeploy.files +=  ../../KJVCanOpener/db/bbl-rvg2010-20140705.ccdb
	dbDeploy.files +=  ../../KJVCanOpener/db/dct-web1828.s3db
	dbDeploy.files +=  ../../KJVCanOpener/db/dct-web1913.s3db
	dbDeploy.path = $${WINBUILD}/KJVCanOpener/db
	fontDeploy.files += ../../KJVCanOpener/fonts/SCRIPTBL.TTF
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-BoldOblique.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-Bold.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-BoldOblique.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-Bold.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed-Oblique.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansCondensed.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-ExtraLight.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-BoldOblique.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-Bold.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono-Oblique.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSansMono.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans-Oblique.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSans.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-BoldItalic.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-Bold.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-BoldItalic.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-Bold.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed-Italic.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerifCondensed.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif-Italic.ttf
	fontDeploy.files += ../../KJVCanOpener/fonts/DejaVuSerif.ttf
	fontDeploy.path = $${WINBUILD}/KJVCanOpener/fonts
	docDeploy.files += ../../KJVCanOpener/doc/KingJamesPureBibleSearch.pdf
	docDeploy.files += ../../KJVCanOpener/kjvdatagen/kjv_summary.xls
	docDeploy.files += ../../KJVCanOpener/articles/kjv_stats.xls
	docDeploy.path = $${WINBUILD}/KJVCanOpener/doc
	examplesDeploy.files += ../../KJVCanOpener/examples/example01.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example02.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example03.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example04.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example05.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example06.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example07.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example08.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example09.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example10.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example11.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example12.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example13.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example14.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example15.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example16.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example17.kjs
	examplesDeploy.files += ../../KJVCanOpener/examples/example18.kjs
	examplesDeploy.path = $${WINBUILD}/KJVCanOpener/examples
	licDeploy.files = ../../KJVCanOpener/gpl-3.0.txt
	licDeploy.path = $${WINBUILD}/KJVCanOpener/license

	INSTALLS += dbDeploy fontDeploy docDeploy examplesDeploy licDeploy
	!isEmpty(TRANSLATIONS) {
		translationDeploy.path = $${WINBUILD}/KJVCanOpener/translations
		INSTALLS += translationDeploy
	}

	greaterThan(QT_MAJOR_VERSION,4):shared {
		equals(MSVC_VER, "10.0") {
			vcRuntimeDeploy.files += $$(VCINSTALLDIR)/redist/x86/Microsoft.VC100.CRT/msvcp100.dll
			vcRuntimeDeploy.files += $$(VCINSTALLDIR)/redist/x86/Microsoft.VC100.CRT/msvcr100.dll
		}
		equals(MSVC_VER, "12.0") {
			vcRuntimeDeploy.files += $$(VCINSTALLDIR)/redist/x86/Microsoft.VC120.CRT/msvcp120.dll
			vcRuntimeDeploy.files += $$(VCINSTALLDIR)/redist/x86/Microsoft.VC120.CRT/msvcr120.dll
			vcRuntimeDeploy.files += $$(VCINSTALLDIR)/redist/x86/Microsoft.VC120.CRT/vccorlib120.dll
		}
		isEmpty(vcRuntimeDeploy.files) {
			error("Can't deploy MSVC Runtime!")
		}
		vcRuntimeDeploy.path = $${WINBUILD}/KJVCanOpener/app
		INSTALLS += vcRuntimeDeploy

		exists($$[QT_INSTALL_BINS]/windeployqt.exe) {
			QMAKE_POST_LINK += $$quote(@echo Deploying...$$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(nmake install $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(-mkdir $$shell_path($${WINBUILD})\\KJVCanOpener\\app $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(copy /y $${TARGET}.exe $$shell_path($${WINBUILD})\\KJVCanOpener\\app $$escape_expand(\\n\\t))
			contains(QTPLUGIN, qplastiquestyle) {
				QMAKE_POST_LINK += $$quote(mkdir $$shell_path($${WINBUILD})\\KJVCanOpener\\plugins\\styles $$escape_expand(\\n\\t))
				QMAKE_POST_LINK += $$quote(copy $$shell_path($$[QT_INSTALL_PLUGINS])\\styles\\qplastiquestyle.dll $$shell_path($${WINBUILD})\\KJVCanOpener\\plugins\\styles\\ $$escape_expand(\\n\\t))
			}
			QMAKE_POST_LINK += $$quote(windeployqt --release --no-compiler-runtime $${TARGET}.exe $$escape_expand(\\n\\t))		# Their compiler runtime deploy copies the exe not the dlls!
			QMAKE_POST_LINK += $$quote(move accessible $$shell_path($${WINBUILD})\\KJVCanOpener\\plugins\\ $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(move bearer $$shell_path($${WINBUILD})\\KJVCanOpener\\plugins\\ $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(move iconengines $$shell_path($${WINBUILD})\\KJVCanOpener\\plugins\\ $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(move imageformats $$shell_path($${WINBUILD})\\KJVCanOpener\\plugins\\ $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(move platforms $$shell_path($${WINBUILD})\\KJVCanOpener\\app\\ $$escape_expand(\\n\\t))			# platforms must be in app folder!
			QMAKE_POST_LINK += $$quote(move sqldrivers $$shell_path($${WINBUILD})\\KJVCanOpener\\plugins\\ $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(copy $$shell_path($$[QT_INSTALL_LIBS])\\wwwidgets4.dll $$shell_path($${WINBUILD})\\KJVCanOpener\\app\\ $$escape_expand(\\n\\t))
			QMAKE_POST_LINK += $$quote(move *.dll $$shell_path($${WINBUILD})\\KJVCanOpener\\app\\ $$escape_expand(\\n\\t))
#			QMAKE_POST_LINK += $$quote(move *.qm $$shell_path($${WINBUILD})\\KJVCanOpener\translations\\ $$escape_expand(\\n\\t))
		} else {
			error("Can't deploy KJPBS/Qt Bundle!")
		}
#		QMAKE_POST_LINK += $$quote(../../KJVCanOpener/macxsign_qt5_bundle$$escape_expand(\\n\\t))
	}

	message("Deployment Path: " $$shell_path($${WINBUILD})$$escape_expand(\\n))
}

message("Config: " $$CONFIG$$escape_expand(\\n))
message("QtConfig: " $$QT_CONFIG$$escape_expand(\\n))
