##*****************************************************************************
##
## Copyright (C) 2012-2025 Donna Whisnant, a.k.a. Dewtronics.
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

cmake_minimum_required(VERSION 3.21...3.28)

include(../cmake/GenerateVersion.cmake)

if(WIN32)
	set(LANGUAGES C CXX RC)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
	set(LANGUAGES C CXX OBJCXX)
else()
	set(LANGUAGES C CXX)
endif()

project(KJVCanOpener
	DESCRIPTION "King James Pure Bible Search"
	HOMEPAGE_URL "http://visitbethelchurch.com/"
	LANGUAGES ${LANGUAGES}
)
if(PROJECT_IS_TOP_LEVEL)
	message(FATAL_ERROR "Don't use this project directly.  Use the CMakeLists.txt in parent folder instead!")
endif()

set(KJVCanOpener_TARGET KingJamesPureBibleSearch)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================

set(QT_COMPONENTS
	Core Gui Widgets Xml
)

set(QT_OPTIONAL_COMPONENTS
	Sql
)

# -----------------------------------------------------------------------------

# Compile Options

include(CMakeDependentOption)

option(BUILD_TESTING "Enable Testing Build" OFF)

option(CONSOLE "Console-Only Application Mode" OFF)
cmake_dependent_option(WASM "WASM Build" OFF "NOT CONSOLE" OFF)
cmake_dependent_option(EMSCRIPTEN "Emscripen Build" OFF "NOT CONSOLE;NOT WASM" OFF)
cmake_dependent_option(VNC "VNC Build" OFF "NOT CONSOLE;NOT WASM;NOT EMSCRIPTEN" OFF)
cmake_dependent_option(WEBCHANNEL "Enable WebChannel Support" ON "UNIX;NOT WASM;NOT EMSCRIPTEN;NOT VNC;NOT APPLE;NOT WIN32" OFF)
cmake_dependent_option(LITEHTML "Use litehtml Renderer" ON "NOT CONSOLE" OFF)
cmake_dependent_option(ELSSEARCH "Use ELSSearch" ON "NOT CONSOLE" OFF)
# -------------------------------------
option(OPTION_EXCEPTIONS_OFF "Disable Exception Processing" OFF)
cmake_dependent_option(OPTION_BUILD_BIBLE_DATABASE "Enable Bible Database Building" ON "NOT CONSOLE;NOT WASM;NOT EMSCRIPTEN;NOT VNC;NOT ANDRIOD;NOT IOS" OFF)
cmake_dependent_option(OPTION_TEXT_TO_SPEECH "Enable Text-To-Speech" OFF "NOT WASM;NOT EMSCRIPTEN;NOT VNC;NOT ANDROID;NOT IOS" OFF)
set(OPTION_SINGLE_APPLICATION_TYPE "QtSingleApplication" CACHE STRING "Application Singleton Control Class")
set_property(CACHE OPTION_SINGLE_APPLICATION_TYPE PROPERTY STRINGS "QtSingleApplication" "SingleApplication")
option(OPTION_USE_MDI_MAIN_WINDOW "Use MDI Main Windows" OFF)
# -------------------------------------
option(DEPRECATION_ENABLE_WARNINGS "Enable Qt Deprecation Warnings" ON)
option(DEPRECATION_DISABLE_APIS "Disable Deprecated Qt APIs" OFF)
# -------------------------------------
option(DEBUG_SEARCH_PHRASE_RESULTS_SPY "Enable Search Phrase/Results Spy" OFF)
option(DEBUG_SHOW_SEARCH_COMPLETER_OUTPUT "Enable Search Completer Debug Output" OFF)
option(DEBUG_CURSOR_SELECTION "Enable Cursor Selection Debugging" OFF)
option(DEBUG_SEARCH_RESULTS_NODE_TOOLTIPS "Enable Search Results Node ToolTips Debugging" OFF)
option(DEBUG_MODEL_TEST "Enable QAbstractItemModel testing for CVerseListModel" OFF)
cmake_dependent_option(DEBUG_SIGNALSPY "Enable Signal Spy Debugging" OFF "NOT WASM;NOT EMSCRIPTEN;NOT VNC" OFF)		# make declarative_debug only?
# -------------------------------------
cmake_dependent_option(SHOW_SPLASH_SCREEN "Show Splash Screen" ON "NOT CONSOLE;NOT VNC" OFF)
option(SHOW_ONLY_LOADED_BIBLE_DATABASES "Show only Loaded Bible Databases in New Search Window Action" OFF)
option(SHOW_ONLY_LOADED_DICTIONARY_DATABASES "Show only Loaded Dictionary Databases in Select Dictionary Action" OFF)
# -------------------------------------
cmake_dependent_option(PRESERVE_MAINWINDOW_GEOMETRY "Preserve Main KJVCanOpener Window Geometry" ON "NOT ANDROID;NOT IOS" OFF)
option(PRESERVE_MAINWINDOW_STATE "Preserve Main KJVCanOpener Toolbars and DockWidget State" ON)
cmake_dependent_option(PRESERVE_MAINWINDOW_SPLITTER_STATE "Preserve KJVCanOpener Splitter State" ON "NOT ANDROID;NOT IOS" OFF)
cmake_dependent_option(PRESERVE_DIALOG_GEOMETRY "Preserve size/layout of UserDatabase Dialogs for Notes and CrossRefs" ON "NOT ANDROID;NOT IOS" OFF)
# -------------------------------------
option(USE_SEARCH_PHRASE_COMPLETER_POPUP_DELAY "Enable Search Phrase Auto-Completer Popup Delay" ON)
option(USE_SEARCH_RESULTS_UPDATE_DELAY "Enable Search Results Update Delay" OFF)
option(USE_MULTITHREADED_SEARCH_RESULTS "Enable Multithreaded Search Results Computation" ON)
option(USE_MULTITHREADED_INVERT_LOGIC "Use Single-Threaded as Default when Multithreading Enabled" ON)
option(USE_EXTENDED_REL_INDEXES "Use Extended RelIndexes" OFF)
option(USE_GEMATRIA "Use Gematria Calculations" ON)
cmake_dependent_option(USE_GEOMAP "Use Geo Map Support" OFF "NOT CONSOLE;NOT VNC;NOT WASM;NOT EMSCRIPTEN" OFF)
# -------------------------------------
option(ENABLE_TRANSLATIONS "Enable Translations" ON)


if(NOT EMSCRIPTEN AND NOT WASM)
	set(USING_wwWidgets ON)
else()
	set(USING_wwWidgets OFF)
endif()

# -----------------------------------------------------------------------------

if(WASM)
	set(PROJECT_VERSION_SPECIALBUILD "(WebAssembly Version)")
elseif(EMSCRIPTEN)
	set(PROJECT_VERSION_SPECIALBUILD "(Emscripten Web-Version)")
elseif(VNC)
	set(PROJECT_VERSION_SPECIALBUILD "(VNC Web-Version)")
elseif(NOT "${CMAKE_PROJECT_VERSION_PRERELEASE}" STREQUAL "")
	set(PROJECT_VERSION_SPECIALBUILD "Alpha Test Version")
endif()

project_genver(HEADER_FILE
	DEFINES
		APPNAME=${KJVCanOpener_TARGET}
)

# -----------------------------------------------------------------------------

if(WEBCHANNEL)
	include(qwebchannel/qwebchannel.cmake)

	list(APPEND QT_COMPONENTS ${qwebchannel_QT_COMPONENTS})
endif()


if(WEBCHANNEL OR VNC OR (NOT CONSOLE AND NOT WASM AND NOT EMSCRIPTEN AND NOT VNC AND NOT ANDRIOD AND NOT IOS))	# If using OPTION_SINGLE_APPLICATION_TYPE, we need Network
	list(APPEND QT_COMPONENTS Network)
endif()


if(ELSSEARCH)
	include(../ELSSearch/ELSSearch.cmake)

	list(APPEND QT_COMPONENTS ${elssearch_QT_COMPONENTS})
endif()


if(ENABLE_TRANSLATIONS)
	list(APPEND QT_COMPONENTS_NOLINK LinguistTools)
endif()


if(NOT CONSOLE)
	set(GUI_TYPE)
	if(WIN32)
		set(GUI_TYPE WIN32)
	endif()
	if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
		set(GUI_TYPE MACOSX_BUNDLE)
		set(MACOSX_BUNDLE_ICON_FILE bible.icns)

		set(MACOSX_BUNDLE_BUNDLE_NAME ${KJVCanOpener_TARGET})
		set(MACOSX_BUNDLE_BUNDLE_SPOKEN_NAME ${KJVCanOpener_DESCRIPTION})
		set(MACOSX_BUNDLE_EXECUTABLE_NAME ${KJVCanOpener_TARGET})

		set(MACOSX_BUNDLE_COPYRIGHT ${KJVCanOpener_LEGAL_COPYRIGHT})

		set(MACOSX_BUNDLE_BUNDLE_VERSION ${KJVCanOpener_VERSION})			# KJVCanOpener_VERSION requires previous call to project_genver() or would need to be CMAKE_PROJECT_VERSION
		set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${KJVCanOpener_VERSION})		# KJVCanOpener_VERSION requires previous call to project_genver() or would need to be CMAKE_PROJECT_VERSION

		set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/res/bible.icns")
		set_source_files_properties(${app_icon_macos} PROPERTIES
			   MACOSX_PACKAGE_LOCATION "Resources")
	endif()
endif()


if(USE_GEOMAP)
	list(APPEND QT_COMPONENTS Qml Quick)
endif()


# -----------------------------------------------------------------------------

list(REMOVE_DUPLICATES QT_COMPONENTS)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${QT_COMPONENTS} ${QT_COMPONENTS_NOLINK} OPTIONAL_COMPONENTS ${QT_OPTIONAL_COMPONENTS})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS} ${QT_COMPONENTS_NOLINK} OPTIONAL_COMPONENTS ${QT_OPTIONAL_COMPONENTS})
set(QT_LINK_LIBS)
foreach(_component ${QT_COMPONENTS})
	list(APPEND QT_LINK_LIBS Qt${QT_VERSION_MAJOR}::${_component})
endforeach()
foreach(_component ${QT_OPTIONAL_COMPONENTS})
	if(Qt${QT_VERSION_MAJOR}${_component}_FOUND)
		list(APPEND QT_LINK_LIBS Qt${QT_VERSION_MAJOR}::${_component})
	endif()
endforeach()

# -----------------------------------------------------------------------------

if(LITEHTML)
	if(NOT TARGET qlitehtml)
		add_subdirectory(../qlitehtml qlitehtml EXCLUDE_FROM_ALL)
		add_subdirectory(../qlitehtml/tests qlitehtml_tests EXCLUDE_FROM_ALL)
	endif()
endif()

# -----------------------------------------------------------------------------


# =============================================================================

# Source Files:
# -------------

set(KJVCanOpener_SOURCES
	"main.cpp"
	"myApplication.cpp"
	"AboutDlg.cpp"
	"BibleDBListModel.cpp"
	"BibleLayout.cpp"
	"BibleWordDiffListModel.cpp"
	"BrowserWidget.cpp"
	"CSV.cpp"
	"dbDescriptors.cpp"
	"dbstruct.cpp"
	"DelayedExecutionTimer.cpp"
	"DictDBListModel.cpp"
	"Highlighter.cpp"
	"HighlighterButtons.cpp"
	"KJVCanOpener.cpp"
	"MimeHelper.cpp"
	"ModelRowForwardIterator.cpp"
	"NoteKeywordWidget.cpp"
	"NotificationToolTip.cpp"
	"ParseSymbols.cpp"
	"PassageNavigator.cpp"
	"PassageNavigatorDlg.cpp"
	"PassageReferenceResolver.cpp"
	"PassageReferenceWidget.cpp"
	"PersistentSettings.cpp"
	"PhraseCursor.cpp"
	"PhraseListModel.cpp"
	"PhraseParser.cpp"
	"ReadDB.cpp"
	"ReflowDelegate.cpp"
	"ReportError.cpp"
	"ScriptureDocument.cpp"
	"ScriptureEdit.cpp"
	"SearchCompleter.cpp"
	"SearchCompleterListModel.cpp"
	"SearchCriteria.cpp"
	"SearchCriteriaWidget.cpp"
	"SearchPhraseEdit.cpp"
	"SearchPhraseListModel.cpp"
	"SearchResults.cpp"
	"SearchSpecWidget.cpp"
	"SoundEx.cpp"
	"SubControls.cpp"
	"TextNavigator.cpp"
	"TextNavigatorEdit.cpp"
	"TextRenderer.cpp"
	"ToolTipEdit.cpp"
	"Translator.cpp"
	"UserNotesDatabase.cpp"
	"VerseListModel.cpp"
	"VerseListDelegate.cpp"
	"VerseRichifier.cpp"
	"XML.cpp"
)

if(OPTION_BUILD_BIBLE_DATABASE)
	list(APPEND KJVCanOpener_SOURCES
		"BuildDB.cpp"
	)
endif()

# These files are FOR the GUI and WASM, but not
#	Emscripten-only (note that Emscripten is also
#	set for WASM, but not vice versa):
if((NOT EMSCRIPTEN OR WASM) AND (NOT CONSOLE))
	list(APPEND KJVCanOpener_SOURCES
		"BibleDatabaseInfoDlg.cpp"
		"Configuration.cpp"
		"DictDatabaseInfoDlg.cpp"
		"DictionaryWidget.cpp"
		"RenameHighlighterDlg.cpp"
	)
endif()

if(NOT EMSCRIPTEN AND NOT VNC AND NOT CONSOLE)
	list(APPEND KJVCanOpener_SOURCES
		"CrossRefEditDlg.cpp"
		"EditWWWLinkDlg.cpp"
		"NoteEditDlg.cpp"
		"SaveLoadFileDialog.cpp"
	)
endif()

if(DEBUG_SIGNALSPY)
	list(APPEND KJVCanOpener_SOURCES
		"signalspy/Q4puGenericSignalSpy.cpp"
	)
endif()

if(USE_GEMATRIA)
	list(APPEND KJVCanOpener_SOURCES
		"Gematria.cpp"
	)
endif()

if(USE_GEOMAP)
	list(APPEND KJVCanOpener_SOURCES
		"GeoMap.cpp"
	)
endif()

# -------------------------------------

# Headers Files:
# --------------

set(KJVCanOpener_HEADERS
	"PathConsts.h"
	"myApplication.h"
	"AboutDlg.h"
	"BibleDBListModel.h"
	"BibleLayout.h"
	"BibleWordDiffListModel.h"
	"BrowserWidget.h"
	"BusyCursor.h"
	"CSV.h"
	"dbDescriptors.h"
	"dbstruct.h"
	"DelayedExecutionTimer.h"
	"DictDBListModel.h"
	"Highlighter.h"
	"HighlighterButtons.h"
	"KJVCanOpener.h"
	"MimeHelper.h"
	"ModelRowForwardIterator.h"
	"NoteKeywordWidget.h"
	"NotificationToolTip.h"
	"ParseSymbols.h"
	"PassageNavigator.h"
	"PassageNavigatorDlg.h"
	"PassageReferenceResolver.h"
	"PassageReferenceWidget.h"
	"PersistentSettings.h"
	"PhraseCursor.h"
	"PhraseListModel.h"
	"PhraseParser.h"
	"Qt_QStyleOption_stub.h"
	"ReadDB.h"
	"ReflowDelegate.h"
	"ReportError.h"
	"ScriptureDocument.h"
	"ScriptureEdit.h"
	"ScriptureTextFormatProperties.h"
	"SearchCompleter.h"
	"SearchCompleterListModel.h"
	"SearchCriteria.h"
	"SearchCriteriaWidget.h"
	"SearchPhraseEdit.h"
	"SearchPhraseListModel.h"
	"SearchResults.h"
	"SearchSpecWidget.h"
	"SoundEx.h"
	"SubControls.h"
	"TextNavigator.h"
	"TextNavigatorEdit.h"
	"TextRenderer.h"
	"ThreadedSearchResults.h"
	"ToolTipEdit.h"
	"Translator.h"
	"UserNotesDatabase.h"
	"VerseListModel.h"
	"VerseListDelegate.h"
	"VerseRichifier.h"
	"XML.h"
)

if(OPTION_BUILD_BIBLE_DATABASE)
	list(APPEND KJVCanOpener_HEADERS
		"BuildDB.h"
	)
endif()

# These files are FOR the GUI and WASM, but not
#	Emscripten-only (note that Emscripten is also
#	set for WASM, but not vice versa):
if((NOT EMSCRIPTEN OR WASM) AND (NOT CONSOLE))
	list(APPEND KJVCanOpener_HEADERS
		"BibleDatabaseInfoDlg.h"
		"Configuration.h"
		"DictDatabaseInfoDlg.h"
		"DictionaryWidget.h"
		"RenameHighlighterDlg.h"
	)
endif()

# These are for the desktop GUI only:
if(NOT EMSCRIPTEN AND NOT VNC AND NOT CONSOLE)
	list(APPEND KJVCanOpener_HEADERS
		"CrossRefEditDlg.h"
		"EditWWWLinkDlg.h"
		"NoteEditDlg.h"
		"SaveLoadFileDialog.h"
	)
endif()

if(DEBUG_SIGNALSPY)
	list(APPEND KJVCanOpener_HEADERS
		"signalspy/Q4puGenericSignalSpy.h"
	)
endif()

if(USE_GEMATRIA)
	list(APPEND KJVCanOpener_HEADERS
		"Gematria.h"
	)
endif()

if(USE_GEOMAP)
	list(APPEND KJVCanOpener_HEADERS
		"GeoMap.h"
	)
endif()

# -------------------------------------

# Forms Files:
# ------------

set(KJVCanOpener_FORMS
	"AboutDlg.ui"
	"BrowserWidget.ui"
	"KJVCanOpener.ui"
	"NoteKeywordWidget.ui"
	"PassageNavigator.ui"
	"PassageNavigatorDlg.ui"
	"PassageReferenceWidget.ui"
	"SearchCriteriaWidget.ui"
	"SearchPhraseEdit.ui"
	"SearchSpecWidget.ui"
)

# These files are FOR the GUI and WASM, but not
#	Emscripten-only (note that Emscripten is also
#	set for WASM, but not vice versa):
if((NOT EMSCRIPTEN OR WASM) AND (NOT CONSOLE))
	list(APPEND KJVCanOpener_FORMS
		"BibleDatabaseInfoDlg.ui"
		"ConfigBibleDatabase.ui"
		"ConfigBrowserOptions.ui"
		"ConfigCopyOptions.ui"
		"ConfigDictDatabase.ui"
		"ConfigDictionaryOptions.ui"
		"ConfigGeneralSettings.ui"
		"ConfigLocale.ui"
		"ConfigSearchOptions.ui"
		"ConfigTextFormat.ui"
		"DictDatabaseInfoDlg.ui"
		"DictionaryWidget.ui"
		"RenameHighlighterDlg.ui"
	)
endif()

if(NOT EMSCRIPTEN AND NOT VNC AND NOT CONSOLE)
	list(APPEND KJVCanOpener_FORMS
		"ConfigUserNotesDatabase.ui"
		"CrossRefEditDlg.ui"
		"EditWWWLinkDlg.ui"
		"NoteEditDlg.ui"
	)
endif()

if(NOT EMSCRIPTEN AND NOT VNC AND OPTION_TEXT_TO_SPEECH)
	list(APPEND KJVCanOpener_FORMS
		"ConfigTTSOptions.ui"
	)
endif()

# -------------------------------------

# Resources Files:
# ----------------

set(KJVCanOpener_RESOURCES
	"KJVCanOpener.qrc"
	"darkstyle.qrc"
)

if(WIN32)
	list(APPEND KJVCanOpener_RESOURCES
		"KJVCanOpener.rc"			# descibes program icon and version details for Windows-only
	)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
	list(APPEND KJVCanOpener_RESOURCES ${app_icon_macos})
endif()

# =============================================================================

# Translations Files:
# -------------------

# Main KJVCanOpener Translations:
set(TRANSLATIONS
	"translations/kjpbs.en.ts"
	"translations/kjpbs.fr.ts"
	"translations/kjpbs.es.ts"
	"translations/kjpbs.de.ts"
	"translations/kjpbs.ru.ts"
)

# Qt Translation Files:
# Define these in a new name so that lupdate, etc, won't pick them up, since they
#	aren't part of the code.  This is needed because Qt4 -> Qt5 broke the namespace
#	for QDialogButtonBox, changing it to QPlatformTheme and yet they didn't properly
#	include ES and FR translations for them, only DE.  So, we'll just manually build
#	all of the translations and deploy with ours:
if(QT_VERSION_MAJOR GREATER 4)
	set(TRANSLATIONS_QT
		"translations/qtbase_fr.ts"
		"translations/qtbase_es.ts"
		"translations/qtbase_de.ts"
		"translations/qtbase_ru.ts"
		# English is native and doesn't have a separate translation file
	)
else()
	set(TRANSLATIONS_QT
		"translations/qt_fr.ts"
		"translations/qt_es.ts"
		"translations/qt_de.ts"
		"translations/qt_ru.ts"
		# English is native and doesn't have a separate translation file
	)
endif()

# =============================================================================

if(WEBCHANNEL)
	list(APPEND KJVCanOpener_SOURCES ${qwebchannel_SOURCES})
	list(APPEND KJVCanOpener_HEADERS ${qwebchannel_SHARED_HEADERS} ${qwebchannel_PRIVATE_HEADERS})
endif()

# =============================================================================

if(ELSSEARCH)
	list(APPEND KJVCanOpener_SOURCES ${elssearch_SOURCES})
	list(APPEND KJVCanOpener_HEADERS ${elssearch_SHARED_HEADERS} ${elssearch_PRIVATE_HEADERS})
	list(APPEND KJVCanOpener_FORMS ${elssearch_FORMS})
	list(APPEND KJVCanOpener_RESOURCES ${elssearch_RESOURCES})
endif()

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
endif()

# -----------------------------------------------------------------------------

add_executable(${KJVCanOpener_TARGET} ${GUI_TYPE}
	${KJVCanOpener_SOURCES}
	${KJVCanOpener_HEADERS}
	${KJVCanOpener_FORMS}
	${KJVCanOpener_RESOURCES}
)
target_compile_options(${KJVCanOpener_TARGET} PUBLIC ${MY_COMPILE_OPTIONS})

if(NOT CONSOLE)
	if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
		set_target_properties(${KJVCanOpener_TARGET} PROPERTIES
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/KJVCanOpener.Info.plist.in"
		)
	endif()
endif()

target_link_libraries(${KJVCanOpener_TARGET} PRIVATE
	${QT_LINK_LIBS}
	QtFindReplaceDialog
	grantlee
	QtIOCompressor
)

if(WIN32)
	target_versioninfo(${KJVCanOpener_TARGET}
		Comments="Don't make me have to open a can on you!\\n"
		LegalCopyright=${KJVCanOpener_LEGAL_COPYRIGHT}
		CompanyName=${KJVCanOpener_ORGNAME}
	)

	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		UNICODE					# Windows API
		_UNICODE				# MSVCRT
		NOMINMAX
		WIN32_LEAN_AND_MEAN
		WINVER=0x0601			# 0x0601 is Win7 and higher
		_WIN32_WINNT=0x0601		# 0x0601 is Win7 and higher
	)
endif()

if(ENABLE_TRANSLATIONS)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		HAVE_TRANSLATIONS
	)

	set_source_files_properties(${TRANSLATIONS} ${TRANSLATIONS_QT}
    	PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

	if(QT_VERSION_MAJOR EQUAL 5)
		qt5_add_translation(TRANSLATION_QM_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT})
		target_sources(${KJVCanOpener_TARGET} PRIVATE ${TRANSLATION_QM_FILES})
	else()
		qt_add_lrelease(${KJVCanOpener_TARGET} TS_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT}
				QM_FILES_OUTPUT_VARIABLE TRANSLATION_QM_FILES)

		qt_add_lupdate(${KJVCanOpener_TARGET} TS_FILES ${TRANSLATIONS}
						SOURCES ${KJVCanOpener_SOURCES}
						INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
	endif()

	if(USING_wwWidgets)
		add_custom_command(TARGET ${KJVCanOpener_TARGET}
			POST_BUILD
			COMMENT "Deploying wwWidgets translations..."
			COMMAND ${CMAKE_COMMAND} -E copy_directory "wwWidgets/translations/" "translations/"
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		)
	endif()
endif()

# -----------------------------------------------------------------------------

if(LITEHTML AND TARGET qlitehtml)
	target_link_libraries(${KJVCanOpener_TARGET} PRIVATE qlitehtml)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USING_LITEHTML
	)

	# Enable workarounds for some LiteHtml Bugs (or incompleteness):
	# --------------------------------------------------------------
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_LITEHTML_81			# Missing Support for CSS "direction" property
	)

else()
	if(LITEHTML)
		message(WARNING "LITEHTML selected but library not built!")
	endif()
endif()


if(NOT Qt${QT_VERSION_MAJOR}Sql_FOUND)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		NOT_USING_SQL
	)
	message(WARNING "Qt SQL Module NOT found!  Disabling Dictionaries.")
else()
	# Dictionaries require SQL, so enable them only if we have SQL:
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USING_DICTIONARIES
	)
endif()


## See: https://stackoverflow.com/questions/18666799/how-to-prevent-qmake-from-adding-the-console-subsystem-on-the-linker-command-lin
#testlib:QT.testlib.CONFIG -= console


if(VNC)
	# Unlike our special qws/vnc-linux-x86_64-g++ target for 4.8.7
	#	VNC builds, the Qt5/Qt6 VNC platform doesn't define VNCSERVER
	#	so we must do it ourselves:
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		VNCSERVER
	)
endif()


if(CONSOLE)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		IS_CONSOLE_APP
	)
endif()


if(WEBCHANNEL)
	setup_qwebchannel_subtargets(${KJVCanOpener_TARGET})
endif()


if(ELSSEARCH)
	setup_elssearch_subtargets(${KJVCanOpener_TARGET})
endif()

if(USING_wwWidgets)
	if(NOT TARGET wwWidgets)
		add_subdirectory(../../support/wwWidgets wwWidgets)
	endif()
	target_link_libraries(${KJVCanOpener_TARGET} PRIVATE
		wwWidgets
	)
endif()


# -----------------------------------


#win32 {
#	CONFIG += rtti
#	CONFIG -= debug_and_release							# Get rid of double debug/release subfolders and do correct shadow build
#	equals(MSVC_VER, "12.0"):QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01		# Enable Support for WinXP if we are building with MSVC 2013, as MSVC 2010 already does
#	DEFINES += _USING_V110_SDK71_												# Needed to run on WinXP and use ATL (as needed for QtSpeech)
#}
#
#ios {
#	CONFIG -= iphonesimulator_and_iphoneos				# Get rid of double iphoneos/iphonesimulator subfolders and do correct shadow build
#}


# -----------------------------------


if(OPTION_EXCEPTIONS_OFF)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		NOT_USING_EXCEPTIONS
	)
endif()


if(OPTION_BUILD_BIBLE_DATABASE)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		BUILD_BIBLE_DATABASE
	)
endif()


if(ANDROID OR IOS)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		IS_MOBILE_APP
	)
endif()


if(OPTION_TEXT_TO_SPEECH)
	# **NOTE** We are at a crossroads here on whether we try to drag
	#	along the old QtSpeech or if we switch to the newer QtSpeech
	#	in Qt 5.15.x, especially when they haven't yet committed to
	#	bring it over to 6.x.
	# Add QtSpeech, but exclude from 'all' build so it doesn't get
	#       "installed" with this application binary "make install":
	# TODO : Add cmake file for qtspeech and include it here:
#	if(NOT TARGET qt-speech)
#		add_subdirectory(../qt-speech QtSpeech EXCLUDE_FROM_ALL)
#	endif()

	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USING_QT_SPEECH
		QTSPEECH_STATIC		# Use QtSpeech as static lib regardless of static/dynamicness of the executable : TODO - Investigate and fix this in cmake logic
	)
endif()


if(OPTION_USE_MDI_MAIN_WINDOW)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_MDI_MAIN_WINDOW
	)
endif()


if(ANDROID)
# These (or parts of these) only needed for JNI calls and our intense experimentation with the Android filesystem:
#	QT += androidextras
#	include(../qtassetsmanager/qtassetsmanager.pri)
#	LIBS += -landroid
endif()


# Add QtIOCompressor, but exclude from 'all' build so it doesn't get
#       "installed" with this application binary "make install":
if(NOT TARGET QtIOCompressor)
	add_subdirectory(../qtiocompressor/src QtIOCompressor EXCLUDE_FROM_ALL)
endif()


# Add grantlee, but exclude from 'all' build so it doesn't get
#       "installed" with this application binary "make install":
if(NOT TARGET grantlee)
	add_subdirectory(../grantlee/textdocument grantlee EXCLUDE_FROM_ALL)
endif()


# Add QtFindReplaceDialog, but exclude from 'all' build so it doesn't get
#       "installed" with this application binary "make install":
if(NOT TARGET QtFindReplaceDialog)
	add_subdirectory(../QtFindReplaceDialog/dialogs QtFindReplaceDialog EXCLUDE_FROM_ALL)
endif()


if(NOT CONSOLE AND NOT WASM AND NOT EMSCRIPTEN AND NOT VNC AND NOT ANDRIOD AND NOT IOS)
	if (OPTION_SINGLE_APPLICATION_TYPE STREQUAL SingleApplication)
		target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
			USING_SINGLEAPPLICATION
		)
		# Add SingleApplication, but exclude from 'all' build so it doesn't get
		#       "installed" with this application binary "make install":
		if(NOT TARGET singleapplication)
			add_subdirectory(../singleapplication SingleApplication EXCLUDE_FROM_ALL)
		endif()
		target_link_libraries(${KJVCanOpener_TARGET} PRIVATE
			singleapplication
		)
	endif()

	if (OPTION_SINGLE_APPLICATION_TYPE STREQUAL QtSingleApplication)
		target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
			USING_QT_SINGLEAPPLICATION
		)
		# Add QtSingleApplication, but exclude from 'all' build so it doesn't get
		#       "installed" with this application binary "make install":
		if(NOT TARGET qtsingleapplication)
			add_subdirectory(../qtsingleapplication/src QtSingleApplication EXCLUDE_FROM_ALL)
		endif()
		target_link_libraries(${KJVCanOpener_TARGET} PRIVATE
			qtsingleapplication
		)
	endif()
endif()


# -----------------------------------


if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()


if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()


# -----------------------------------

# Miscellaneous Special-Testing and Debug modes that can be enabled:
# ------------------------------------------------------------------

if(DEBUG_SEARCH_PHRASE_RESULTS_SPY)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		SEARCH_PHRASE_SPY
		SEARCH_RESULTS_SPY
	)
endif()


if(DEBUG_SHOW_SEARCH_COMPLETER_OUTPUT)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		SEARCH_COMPLETER_DEBUG_OUTPUT
	)
endif()


if(DEBUG_CURSOR_SELECTION)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		DEBUG_CURSOR_SELECTION
	)
endif()


if(DEBUG_SEARCH_RESULTS_NODE_TOOLTIPS)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		DEBUG_SEARCH_RESULTS_NODE_TOOLTIPS
	)
endif()


if(DEBUG_MODEL_TEST)
	# Special QAbstractItemModel test suite:
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		MODELTEST
	)
	# Add modeltest, but exclude from 'all' build so it doesn't get
	#       "installed" with this application binary "make install":
	# TODO : Add cmake file for modeltest and include it here:
#	if(NOT TARGET modeltest)
#	add_subdirectory(../modeltest modeltest EXCLUDE_FROM_ALL)
#	endif()
endif()

#!emscripten:!vnc:declarative_debug:CONFIG *= signalspy
if(DEBUG_SIGNALSPY)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		SIGNAL_SPY_DEBUG
	)
endif()

# -----------------------------------


if(SHOW_SPLASH_SCREEN)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		SHOW_SPLASH_SCREEN
	)
endif()


if(SHOW_ONLY_LOADED_BIBLE_DATABASES)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		ENABLE_ONLY_LOADED_BIBLE_DATABASES
	)
endif()


if(SHOW_ONLY_LOADED_DICTIONARY_DATABASES)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		ENABLE_ONLY_LOADED_DICTIONARY_DATABASES
	)
endif()


# -------------------------------------

# Saving/Restoring of KJVCanOpener Window State/Geometry and Splitter State:


if(PRESERVE_MAINWINDOW_GEOMETRY)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		PRESERVE_MAINWINDOW_GEOMETRY				# Physical size and layout of KJVCanOpener
	)
endif()


if(PRESERVE_MAINWINDOW_STATE)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		PRESERVE_MAINWINDOW_STATE					# Toolbars and DockWidgets of KJVCanOpener
	)
endif()


if(PRESERVE_MAINWINDOW_SPLITTER_STATE)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		PRESERVE_MAINWINDOW_SPLITTER_STATE			# Splitters on KJVCanOpener
	)
endif()


if(PRESERVE_DIALOG_GEOMETRY)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		PRESERVE_DIALOG_GEOMETRY					# Physical size and layout of UserDatabase editor dialogs for Notes and CrossRefs
	)
endif()


# -------------------------------------


# Enable Search Phrase Auto-Completer Popup Delay:
if(USE_SEARCH_PHRASE_COMPLETER_POPUP_DELAY)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_SEARCH_PHRASE_COMPLETER_POPUP_DELAY		# Enable to delay search phrase completer popup based on the search phrase activation time
	)
endif()


# Enable Search Results Update Delay:
if(USE_SEARCH_RESULTS_UPDATE_DELAY)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_SEARCH_RESULTS_UPDATE_DELAY				# Enable to delay extra staged search results update delay after the delayed phrase change notification is received
	)
endif()


# Enable Multithreaded Search Results:
if(USE_MULTITHREADED_SEARCH_RESULTS)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_MULTITHREADED_SEARCH_RESULTS			# Enable to spawn separate thread to handle calculation of Scoped Search Results for the VerseListModel
	)
endif()


if(USE_MULTITHREADED_INVERT_LOGIC)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		INVERT_MULTITHREADED_LOGIC					# Invert the defaults of single vs. multi threaded when USE_MULTITHREADED_SEARCH_RESULTS is enabled (with this Single is default without command-line option instead of Multi)
	)
endif()


# Enable Extended RelIndexes:
if(USE_EXTENDED_REL_INDEXES)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_EXTENDED_INDEXES
	)
endif()


# Enable Gematria Calculations:
if(USE_GEMATRIA)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_GEMATRIA
	)
endif()


# Enable GeoMap Visualizations:
if(USE_GEOMAP)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_GEOMAP
	)
endif()


# -----------------------------------------------------------------------------

# Enable workarounds for some QTBUGs:
# -----------------------------------

target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
	WORKAROUND_QTBUG_13768				# Hover attribute for QSplitter
)


target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
	WORKAROUND_QTBUG_100879				# Composed characters inside anchors are broken : https://bugreports.qt.io/browse/QTBUG-100879
)


#if((QT_VERSION >= 5.0.0) AND (QT_VERSION <= 5.2.0))
#	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
#		WORKAROUND_QTBUG_33906			# singleStep QTreeView Scroll Bug
#	)
#endif()


if(IOS AND (QT_VERSION_MAJOR GREATER_EQUAL 5))
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_QTBUG_34490			# iOS Font Bug
	)
	set(WORKAROUND_QTBUG_34490 ON)
else()
	set(WORKAROUND_QTBUG_34490 OFF)
endif()


if(IOS AND (QT_VERSION_MAJOR GREATER_EQUAL 5))
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_QTBUG_35787			# iOS SplashScreen Bug
	)
endif()


if(QT_VERSION_MAJOR GREATER_EQUAL 5)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_QTBUG_BROWSER_BOUNCE		# Not a submitted Qt bug, that I know of, but a Qt 5 bug
	)
endif()


if((CMAKE_SYSTEM_NAME STREQUAL Darwin) AND (QT_VERSION_MAJOR LESS 5))
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_QTBUG_32789			# Qt 4 Font Bug on MacX Mavericks
	)
endif()


if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_QTBUG_88928			# Fix Broken Standard Icons on QMessageBox on MacOS
	)
endif()


if(ANDROID)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_QTBUG_35313_35687	# Android QMessageBox dialogs have wrong theming (workaround uses Android Native Dialogs)
	)
endif()


if((CMAKE_SYSTEM_NAME STREQUAL Darwin) AND (QT_VERSION_MAJOR GREATER_EQUAL 5) AND NOT CONSOLE)
	# Resolves this warning on Sonoma:
	#	WARNING: Secure coding is not enabled for restorable state! Enable secure coding by implementing NSApplicationDelegate.applicationSupportsSecureRestorableState: and returning YES.
	#	See: https://stackoverflow.com/questions/77283578/sonoma-and-nsapplicationdelegate-applicationsupportssecurerestorablestate
	target_sources(${KJVCanOpener_TARGET} PRIVATE
		"CocoaDelegate.mm"
	)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		WORKAROUND_QTBUG_SecureRestorableState
	)
endif()


# -----------------------------------------------------------------------------

# Non-configurable typedefs, dependent on build target:
# -----------------------------------------------------

# Enable Loading of our Application Fonts (Note: old Emscripten-Qt
#	uses auto-loading of .qpf fonts from deployed qt-fonts folder)
#	But, WebAssembly Emscripten Qt does it directly from resources:
#
# These files are FOR the GUI and WASM, but not
#	Emscripten-only (note that Emscripten is also
#	set for WASM, but not vice versa):
if((NOT EMSCRIPTEN OR WASM) AND NOT CONSOLE)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		LOAD_APPLICATION_FONTS
	)
endif()


# Enable Asynchronous Dialogs
if(EMSCRIPTEN)		# if(emscripten | macx)
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		USE_ASYNC_DIALOGS
	)
endif()


# Enable Gesture/TouchDevice processing:
if((IOS OR ANDRIOD) AND (QT_VERSION_MAJOR GREATER 4))
	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
		TOUCH_GESTURE_PROCESSING
	)
endif()


## Enable Sandboxing on Mac:
#if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
#	# TODO : Enable this once we figure out a mechanism of calling the
#	#		sandbox interaction functions to keep the NS Keys for our
#	#		notes files:
#	target_compile_definitions(${KJVCanOpener_TARGET} PRIVATE
#		IN_MAC_SANDBOX
#	)
#	set(SANDBOXED ON)
#endif()




