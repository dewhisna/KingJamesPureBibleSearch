##*****************************************************************************
##
## Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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

cmake_minimum_required(VERSION 3.20...3.28)

set(LANGUAGES CXX)

project(KJVDataParse VERSION 4.0.0 DESCRIPTION "KJPBS DataParse Tool" LANGUAGES ${LANGUAGES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================

set(QT_COMPONENTS
	Core Gui Xml
)

# -----------------------------------------------------------------------------

# Compile Options

include(CMakeDependentOption)


set(ENABLE_TRANSLATIONS ON)

# -----------------------------------------------------------------------------

if(ENABLE_TRANSLATIONS)
	set(QT_COMPONENTS_NOLINK
		LinguistTools
	)
endif()

set(GUI_TYPE)

set(CONSOLE ON)

# -----------------------------------------------------------------------------

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


# =============================================================================

# Source Files:
# -------------

set(KJVDataParse_SOURCES
	"main.cpp"
	"../KJVCanOpener/BibleLayout.cpp"
	"../KJVCanOpener/CSV.cpp"
	"../KJVCanOpener/dbDescriptors.cpp"
	"../KJVCanOpener/dbstruct.cpp"
	"../KJVCanOpener/Highlighter.cpp"
	"../KJVCanOpener/ParseSymbols.cpp"
	"../KJVCanOpener/PersistentSettings.cpp"
	"../KJVCanOpener/PhraseCursor.cpp"
	"../KJVCanOpener/PhraseListModel.cpp"
	"../KJVCanOpener/PhraseNavigator.cpp"
	"../KJVCanOpener/ReadDB.cpp"
	"../KJVCanOpener/ReportError.cpp"
	"../KJVCanOpener/ScriptureDocument.cpp"
	"../KJVCanOpener/SoundEx.cpp"
	"../KJVCanOpener/Translator.cpp"
	"../KJVCanOpener/UserNotesDatabase.cpp"
	"../KJVCanOpener/VerseRichifier.cpp"
	"../KJVCanOpener/XML.cpp"
)

# -------------------------------------

# Headers Files:
# --------------

set(KJVDataParse_HEADERS
	"xc_KJVDataParse.h"
	"../KJVCanOpener/BibleLayout.h"
	"../KJVCanOpener/PathConsts.h"
	"../KJVCanOpener/BusyCursor.h"
	"../KJVCanOpener/CSV.h"
	"../KJVCanOpener/dbDescriptors.h"
	"../KJVCanOpener/dbstruct.h"
	"../KJVCanOpener/Highlighter.h"
	"../KJVCanOpener/ParseSymbols.h"
	"../KJVCanOpener/PersistentSettings.h"
	"../KJVCanOpener/PhraseCursor.h"
	"../KJVCanOpener/PhraseListModel.h"
	"../KJVCanOpener/PhraseNavigator.h"
	"../KJVCanOpener/ReadDB.h"
	"../KJVCanOpener/ReportError.h"
	"../KJVCanOpener/ScriptureDocument.h"
	"../KJVCanOpener/SoundEx.h"
	"../KJVCanOpener/Translator.h"
	"../KJVCanOpener/UserNotesDatabase.h"
	"../KJVCanOpener/VerseRichifier.h"
	"../KJVCanOpener/XML.h"
)

# -------------------------------------

# Forms Files:
# ------------

set(KJVDataParse_FORMS
)

# -------------------------------------

# Resources Files:
# ----------------

set(KJVDataParse_RESOURCES
)


# =============================================================================

# Translations Files:
# -------------------

# KJVDataParse Translations:
set(TRANSLATIONS
	"translations/kjvdataparse.en.ts"
	"translations/kjvdataparse.fr.ts"
	"translations/kjvdataparse.es.ts"
	"translations/kjvdataparse.de.ts"
	"translations/kjvdataparse.ru.ts"
)

set(TRANSLATIONS_QT)

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
endif()

# -----------------------------------------------------------------------------

set(KJVDataParse_TARGET KJVDataParse)

add_executable(${KJVDataParse_TARGET} ${GUI_TYPE}
	${KJVDataParse_SOURCES}
	${KJVDataParse_HEADERS}
	${KJVDataParse_FORMS}
	${KJVDataParse_RESOURCES}
)
target_compile_options(${KJVDataParse_TARGET} PUBLIC ${MY_COMPILE_OPTIONS})

target_link_libraries(${KJVDataParse_TARGET} PRIVATE
	${QT_LINK_LIBS}
	grantlee
	QtIOCompressor
)

if(ENABLE_TRANSLATIONS)
	target_compile_definitions(${KJVDataParse_TARGET} PRIVATE
		HAVE_TRANSLATIONS
	)

	set_source_files_properties(${TRANSLATIONS} ${TRANSLATIONS_QT}
    	PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

	if(QT_VERSION_MAJOR EQUAL 5)
		qt5_add_translation(TRANSLATION_QM_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT})
		target_sources(${KJVDataParse_TARGET} PRIVATE ${TRANSLATION_QM_FILES})
	else()
		qt_add_lrelease(${KJVDataParse_TARGET} TS_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT}
				QM_FILES_OUTPUT_VARIABLE TRANSLATION_QM_FILES)
	endif()
endif()

# -----------------------------------------------------------------------------


## See: https://stackoverflow.com/questions/18666799/how-to-prevent-qmake-from-adding-the-console-subsystem-on-the-linker-command-lin
#testlib:QT.testlib.CONFIG -= console


if(CONSOLE)
	target_compile_definitions(${KJVDataParse_TARGET} PRIVATE
		IS_CONSOLE_APP
	)
endif()


# -----------------------------------

target_compile_definitions(${KJVDataParse_TARGET} PRIVATE
	NOT_USING_SQL
	NO_PERSISTENT_SETTINGS
	OSIS_PARSER_BUILD
)

# -----------------------------------------------------------------------------

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


# -----------------------------------------------------------------------------

if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(${KJVDataParse_TARGET} PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()


if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(${KJVDataParse_TARGET} PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

# -----------------------------------------------------------------------------


