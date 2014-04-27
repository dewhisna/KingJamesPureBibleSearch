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
	win32:CONFIG += rtti
	CONFIG += wwwidgets
}

!android:!ios:!emscripten:!vnc {
	CONFIG += buildKJVDatabase
	DEFINES += BUILD_KJV_DATABASE
}

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

# The following is absolutely needed on Qt5.2.0, or else we'll crash in the
#	failure of the accessibility factory from creating an accessibility
#	object.  Should probably be there for all platforms to make sure the
#	accessibility support gets loaded:
!emscripten:!win32:QTPLUGIN += qtaccessiblewidgets

# Miscellaneous Special-Testing and Cache modes that can be enabled:
#DEFINES += VERSE_LIST_PLAIN_TEXT_CACHE
#DEFINES += VERSE_LIST_RICH_TEXT_CACHE
#DEFINES += BIBLE_DATABASE_RICH_TEXT_CACHE
#DEFINES += SEARCH_PHRASE_SPY SEARCH_RESULTS_SPY
#DEFINES += SEARCH_COMPLETER_DEBUG_OUTPUT
!emscripten:!vnc:declarative_debug:DEFINES += SIGNAL_SPY_DEBUG
#DEFINES += USE_MDI_MAIN_WINDOW
#DEFINES += DEBUG_CURSOR_SELECTION

# Enable workarounds for some QTBUGs:
DEFINES += WORKAROUND_QTBUG_13768											# Hover attribute for QSplitter
equals(QT_MAJOR_VERSION,5):if(lessThan(QT_MINOR_VERSION,2) | equals(QT_MINOR_VERSION,2):equals(QT_PATCH_VERSION,0)):DEFINES += WORKAROUND_QTBUG_33906			# singleStep QTreeView Scroll Bug
ios:greaterThan(QT_MAJOR_VERSION,4):CONFIG += WORKAROUND_QTBUG_34490		# iOS Font Bug
ios:greaterThan(QT_MAJOR_VERSION,4):DEFINES += WORKAROUND_QTBUG_35787		# iOS SplashScreen Bug
greaterThan(QT_MAJOR_VERSION,4):DEFINES += WORKAROUND_QTBUG_BROWSER_BOUNCE	# Not a submitted Qt bug, that I know of, but a Qt 5 bug
macx:lessThan(QT_MAJOR_VERSION,5):DEFINES += WORKAROUND_QTBUG_32789			# Qt 4 Font Bug on MacX Mavericks

# Enable Splash Screen:
!vnc:DEFINES += SHOW_SPLASH_SCREEN

# Enabled desired random passage mode:
# (Pick only one of RANDOM_PASSAGE entries)
#DEFINES += RANDOM_PASSAGE_VERSE_WEIGHT			# Weigh passages evenly by verses (books with more verses picked more often)
DEFINES += RANDOM_PASSAGE_EVEN_WEIGHT			# Weigh passages evenly by book/chapter/verse (pick book, then chapter, then verse)

# Enable to only show loaded Bible Databases in New Search Window action:
#DEFINES += ENABLE_ONLY_LOADED_BIBLE_DATABASES

# Enable Loading of our Application Fonts (Note: Emscripten uses auto-loading of .qpf fonts from deployed qt-fonts folder):
!emscripten:DEFINES += LOAD_APPLICATION_FONTS

# Enable Asynchronous Dialogs
#if(emscripten | macx):DEFINES += USE_ASYNC_DIALOGS
emscripten:DEFINES += USE_ASYNC_DIALOGS

# Enable Gesture/TouchDevice processing:
if(ios | android):greaterThan(QT_MAJOR_VERSION,4):DEFINES += TOUCH_GESTURE_PROCESSING

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

ios:QTPLUGIN += qsqlite

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
	DictionaryWidget.cpp \
	KJVConfiguration.cpp \
	RenameHighlighterDlg.cpp

!emscripten:!vnc:SOURCES += \
	KJVCrossRefEditDlg.cpp \
	KJVNoteEditDlg.cpp \
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
	DictionaryWidget.h \
	KJVConfiguration.h \
	RenameHighlighterDlg.h

!emscripten:!vnc:HEADERS += \
	KJVCrossRefEditDlg.h \
	KJVNoteEditDlg.h \
	SaveFileDialog.h \
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
	ConfigBrowserOptions.ui \
	ConfigCopyOptions.ui \
	ConfigDictionaryOptions.ui \
	ConfigSearchOptions.ui \
	DictionaryWidget.ui \
	KJVBibleDatabaseConfig.ui \
	KJVGeneralSettingsConfig.ui \
	KJVLocaleConfig.ui \
	KJVTextFormatConfig.ui \
	RenameHighlighterDlg.ui

!emscripten:!vnc:FORMS += \
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
		dbDeploy.files = ../KJVCanOpener/db/kjvtext.ccdb ../KJVCanOpener/db/kjvuser.s3db ../KJVCanOpener/db/dct-web1828.s3db
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

		INSTALLS += dbDeploy fontDeploy docDeploy examplesDeploy licDeploy
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
		dbDeploy.files =  ../../KJVCanOpener/db/kjvtext.ccdb ../../KJVCanOpener/db/kjvuser.s3db ../../KJVCanOpener/db/dct-web1828.s3db
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
		dbDeploy.files =  ../../KJVCanOpener/db/kjvtext.ccdb ../../KJVCanOpener/db/kjvuser.s3db ../../KJVCanOpener/db/dct-web1828.s3db
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
		examplesDeploy.path = /Contents/SharedSupport/examples
		licDeploy.files = ../../KJVCanOpener/gpl-3.0.txt
		licDeploy.path = /Contents/SharedSupport/license

		QMAKE_BUNDLE_DATA += nibDeploy dbDeploy fontDeploy docDeploy examplesDeploy licDeploy
		!isEmpty(TRANSLATIONS) {
			translationDeploy.path = /Contents/Resources/translations
			QMAKE_BUNDLE_DATA += translationDeploy
		}
	}
}

message($$CONFIG$$escape_expand(\\n))
