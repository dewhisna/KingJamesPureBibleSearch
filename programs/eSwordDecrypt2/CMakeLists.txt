##*****************************************************************************
##
## Copyright (C) 2024-2025 Donna Whisnant, a.k.a. Dewtronics.
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

project(eSwordDecrypt VERSION 4.0.0 DESCRIPTION "eSword SQLite Decrypting Tool" LANGUAGES ${LANGUAGES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================

set(QT_COMPONENTS
	Core Gui Widgets Sql
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

# The MyZLIB library in QtIOCompressor is an ALIAS when it's bundled from
#   Qt and isn't globally visible to parent.  So, if it would be the Qt
#   Bundled version, we need to realias it here.  For the other cases
#   we will already have built MyZLIB from source.  See comments in
#	QtIOCompressor.
find_package(Qt${QT_VERSION_MAJOR}Zlib QUIET)
if(Qt${QT_VERSION_MAJOR}Zlib_FOUND)
	# If we have a Qt Zlib, wrap it ourselves:
	add_library(MyZLIB ALIAS Qt${QT_VERSION_MAJOR}::Zlib)
	message(STATUS "${CMAKE_PROJECT_NAME} Found: Qt${QT_VERSION_MAJOR}Zlib")
else()
	find_package(Qt${QT_VERSION_MAJOR}BundledZLIB QUIET)
	if(Qt${QT_VERSION_MAJOR}BundledZLIB_FOUND)
		# If we have a Qt BundledZLIB, wrap it ourselves:
		add_library(MyZLIB ALIAS Qt${QT_VERSION_MAJOR}::BundledZLIB)
		message(STATUS "${CMAKE_PROJECT_NAME} Found: Qt${QT_VERSION_MAJOR}BundledZLIB")
	endif()
endif()

# =============================================================================

# Source Files:
# -------------

set(eSwordDecrypt_SOURCES
	"eSwordDecrypt.cpp"
	"twofish_opt2.cpp"
	# ----
	"../KJVCanOpener/dbDescriptors.cpp"
	"../KJVCanOpener/BibleLayout.cpp"
)

# -------------------------------------

# Headers Files:
# --------------

set(eSwordDecrypt_HEADERS
	"twofish_opt2.h"
	"tables.h"
	# ----
	"../KJVCanOpener/dbDescriptors.h"
	"../KJVCanOpener/BibleLayout.h"
)

# -------------------------------------

# Forms Files:
# ------------

set(eSwordDecrypt_FORMS
)

# -------------------------------------

# Resources Files:
# ----------------

set(eSwordDecrypt_RESOURCES
)


# =============================================================================

# Translations Files:
# -------------------

# eSwordDecrypt Translations:
set(TRANSLATIONS)

set(TRANSLATIONS_QT)

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
endif()

# -----------------------------------------------------------------------------

set(eSwordDecrypt_TARGET eSwordDecrypt)

add_executable(${eSwordDecrypt_TARGET} ${GUI_TYPE}
	${eSwordDecrypt_SOURCES}
	${eSwordDecrypt_HEADERS}
	${eSwordDecrypt_FORMS}
	${eSwordDecrypt_RESOURCES}
)
target_compile_options(${eSwordDecrypt_TARGET} PUBLIC ${MY_COMPILE_OPTIONS})

target_link_libraries(${eSwordDecrypt_TARGET} PRIVATE
	${QT_LINK_LIBS}
	MyZLIB
)

#target_include_directories(${eSwordDecrypt_TARGET} PRIVATE
#)

if(ENABLE_TRANSLATIONS)
	target_compile_definitions(${eSwordDecrypt_TARGET} PRIVATE
		HAVE_TRANSLATIONS
	)

	set_source_files_properties(${TRANSLATIONS} ${TRANSLATIONS_QT}
    	PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

	if(QT_VERSION_MAJOR EQUAL 5)
		qt5_add_translation(TRANSLATION_QM_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT})
		target_sources(${eSwordDecrypt_TARGET} PRIVATE ${TRANSLATION_QM_FILES})
	else()
		qt_add_lrelease(${eSwordDecrypt_TARGET} TS_FILES ${TRANSLATIONS} ${TRANSLATIONS_QT}
				QM_FILES_OUTPUT_VARIABLE TRANSLATION_QM_FILES)
	endif()
endif()

# -----------------------------------------------------------------------------


## See: https://stackoverflow.com/questions/18666799/how-to-prevent-qmake-from-adding-the-console-subsystem-on-the-linker-command-lin
#testlib:QT.testlib.CONFIG -= console


if(CONSOLE)
	target_compile_definitions(${eSwordDecrypt_TARGET} PRIVATE
		IS_CONSOLE_APP
	)
endif()


# -----------------------------------

target_compile_definitions(${eSwordDecrypt_TARGET} PRIVATE
	NO_PERSISTENT_SETTINGS
	_CRT_SECURE_NO_WARNINGS
)

# -----------------------------------------------------------------------------

# Add QtIOCompressor, but exclude from 'all' build so it doesn't get
#       "installed" with this application binary "make install":
if(NOT TARGET QtIOCompressor)
	add_subdirectory(../qtiocompressor/src QtIOCompressor EXCLUDE_FROM_ALL)
endif()

# -----------------------------------------------------------------------------

if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(${eSwordDecrypt_TARGET} PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()


if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(${eSwordDecrypt_TARGET} PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

# -----------------------------------------------------------------------------



