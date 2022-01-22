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

cmake_minimum_required(VERSION 3.20)

set(LANGUAGES CXX)

project(KJVDictWord VERSION 4.0.0 DESCRIPTION "KJPBS DictWord Tool" LANGUAGES ${LANGUAGES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================

set(QT_COMPONENTS
	Core Gui Xml Sql
)

# -----------------------------------------------------------------------------

# Compile Options

include(CMakeDependentOption)


set(ENABLE_TRANSLATIONS OFF)

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
if(QT_VERSION_MAJOR EQUAL 6)
	# Qt6 drops SAX XML Parser which we need.  For now, get it from the Qt5 compatibility shim:
	list(APPEND QT_COMPONENTS Core5Compat)
endif()
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

set(KJVDictWord_SOURCES
	"main.cpp"
	"../KJVCanOpener/CSV.cpp"
	"../KJVCanOpener/dbDescriptors.cpp"
	"../KJVCanOpener/dbstruct.cpp"
	"../KJVCanOpener/Highlighter.cpp"
	"../KJVCanOpener/KJVSearchCriteria.cpp"
	"../KJVCanOpener/ModelRowForwardIterator.cpp"
	"../KJVCanOpener/ParseSymbols.cpp"
	"../KJVCanOpener/PersistentSettings.cpp"
	"../KJVCanOpener/PhraseEdit.cpp"
	"../KJVCanOpener/PhraseListModel.cpp"
	"../KJVCanOpener/ReadDB.cpp"
	"../KJVCanOpener/ReportError.cpp"
	"../KJVCanOpener/ScriptureDocument.cpp"
	"../KJVCanOpener/SearchCompleter.cpp"
	"../KJVCanOpener/Translator.cpp"
	"../KJVCanOpener/UserNotesDatabase.cpp"
	"../KJVCanOpener/VerseRichifier.cpp"
)

# -------------------------------------

# Headers Files:
# --------------

set(KJVDictWord_HEADERS
	"../KJVCanOpener/PathConsts.h"
	"../KJVCanOpener/CSV.h"
	"../KJVCanOpener/dbDescriptors.h"
	"../KJVCanOpener/dbstruct.h"
	"../KJVCanOpener/Highlighter.h"
	"../KJVCanOpener/KJVSearchCriteria.h"
	"../KJVCanOpener/ModelRowForwardIterator.h"
	"../KJVCanOpener/ParseSymbols.h"
	"../KJVCanOpener/PersistentSettings.h"
	"../KJVCanOpener/PhraseEdit.h"
	"../KJVCanOpener/PhraseListModel.h"
	"../KJVCanOpener/ReadDB.h"
	"../KJVCanOpener/ReportError.h"
	"../KJVCanOpener/ScriptureDocument.h"
	"../KJVCanOpener/SearchCompleter.h"
	"../KJVCanOpener/Translator.h"
	"../KJVCanOpener/UserNotesDatabase.h"
	"../KJVCanOpener/VerseRichifier.h"
)

# -------------------------------------

# Forms Files:
# ------------

set(KJVDictWord_FORMS
)

# -------------------------------------

# Resources Files:
# ----------------

set(KJVDictWord_RESOURCES
)


# =============================================================================

# Translations Files:
# -------------------

# KJVDictWord Translations:
set(TRANSLATIONS)

set(TRANSLATIONS_QT)

# =============================================================================

set(KJVDictWord_TARGET KJVDictWord)

add_executable(${KJVDictWord_TARGET} ${GUI_TYPE}
	${KJVDictWord_SOURCES}
	${KJVDictWord_HEADERS}
	${KJVDictWord_FORMS}
	${KJVDictWord_RESOURCES}
)

target_link_libraries(${KJVDictWord_TARGET} PRIVATE
	${QT_LINK_LIBS}
	grantlee
	QtIOCompressor
)

if(ENABLE_TRANSLATIONS)
	target_compile_definitions(${KJVDictWord_TARGET} PRIVATE
		HAVE_TRANSLATIONS
	)

	set_source_files_properties(${TRANSLATIONS} ${TRANSLATIONS_QT}
    	PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

	if(QT_VERSION_MAJOR EQUAL 5)
		qt_add_translation(TRANSLATION_QM_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT})
	else()
		qt_add_lrelease(${KJVDictWord_TARGET} TS_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT}
				QM_FILES_OUTPUT_VARIABLE TRANSLATION_QM_FILES)
	endif()
endif()

# -----------------------------------------------------------------------------


## See: https://stackoverflow.com/questions/18666799/how-to-prevent-qmake-from-adding-the-console-subsystem-on-the-linker-command-lin
#testlib:QT.testlib.CONFIG -= console


if(CONSOLE)
	target_compile_definitions(${KJVDictWord_TARGET} PRIVATE
		IS_CONSOLE_APP
	)
endif()


# -----------------------------------

target_compile_definitions(${KJVDictWord_TARGET} PRIVATE
	NO_PERSISTENT_SETTINGS
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
	target_compile_definitions(${KJVDictWord_TARGET} PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()


if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(${KJVDictWord_TARGET} PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

# -----------------------------------------------------------------------------


