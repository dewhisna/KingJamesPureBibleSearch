##*****************************************************************************
##
## Copyright (C) 2023 Donna Whisnant, a.k.a. Dewtronics.
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

##
## qlitehtml:
## ----------
## Copyright (C) 2022 The Qt Company Ltd.
## SPDX-License-Identifier: BSD-3-Clause
##

cmake_minimum_required(VERSION 3.7...3.28)
project(qlitehtml VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QT 5.15 NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
set(Qt Qt${QT_VERSION_MAJOR})

set(QLITEHTML_EXPORT qlitehtml)

set(QLITEHTML_VERSION ${PROJECT_VERSION})
set(QLITEHTML_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(QLITEHTML_VERSION_COMPAT ${QLITEHTML_VERSION} CACHE STRING "qlitehtml compat version number.")

option(BUILD_TESTING "Enable Testing Build" OFF)

if(NOT TARGET litehtml)
	set(ORIG_BUILD_TESTING ${BUILD_TESTING})
	set(ORIG_FPIC ${CMAKE_POSITION_INDEPENDENT_CODE})
	set(ORIG_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
	if (WIN32)
		set(LITEHTML_UTF8 ON CACHE BOOL "")
	endif()
	set(BUILD_TESTING OFF)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(BUILD_SHARED_LIBS OFF)

	add_subdirectory(litehtml litehtml EXCLUDE_FROM_ALL)

	# suppress compiler warnings from litehtml
	set_target_properties(
		litehtml
		PROPERTIES
			QT_COMPILE_OPTIONS_DISABLE_WARNINGS ON
	)
	set_target_properties(
		gumbo
		PROPERTIES
			QT_COMPILE_OPTIONS_DISABLE_WARNINGS ON
	)

	set(BUILD_TESTING ${ORIG_BUILD_TESTING})
	set(CMAKE_POSITION_INDEPENDENT_CODE "${ORIG_FPIC}")
	set(BUILD_SHARED_LIBS ${ORIG_BUILD_SHARED_LIBS})
	# force optimized litehtml even in debug
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		# except for windows
		if (NOT WIN32)
			target_compile_options(gumbo PRIVATE -O2)
			target_compile_options(litehtml PRIVATE -O2)
		endif()
	endif()
endif()

set(QLITEHTML_SHARED_HEADERS
	container_qpainter.h
	container_qpainter_p.h
	qlitehtml_global.h
	qlitehtmlwidget.h
)

set(QLITEHTML_SOURCE_FILES
	container_qpainter.cpp
	qlitehtmlwidget.cpp
)

if(NOT QLITEHTML_LIBRARY_TYPE)
	set(QLITEHTML_LIBRARY_TYPE STATIC)
endif()

add_library(qlitehtml ${QLITEHTML_LIBRARY_TYPE} ${QLITEHTML_SHARED_HEADERS} ${QLITEHTML_SOURCE_FILES})

target_include_directories(qlitehtml PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(qlitehtml PUBLIC Qt${QT_VERSION_MAJOR}::Widgets PRIVATE litehtml)
target_compile_definitions(qlitehtml PRIVATE
	QT_NO_JAVA_STYLE_ITERATORS
	QT_NO_CAST_TO_ASCII QT_RESTRICTED_CAST_FROM_ASCII
	QT_USE_QSTRINGBUILDER
)

if("${QLITEHTML_LIBRARY_TYPE}" STREQUAL "SHARED")
	target_compile_definitions(qlitehtml PRIVATE QLITEHTML_LIBRARY)
else()
	target_compile_definitions(qlitehtml PUBLIC QLITEHTML_STATIC_LIBRARY)
endif()
if (WIN32)
	target_compile_definitions(qlitehtml PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
	if (NOT BUILD_WITH_PCH)
		# Windows 8 0x0602
		target_compile_definitions(qlitehtml PRIVATE
			WINVER=0x0602
			_WIN32_WINNT=0x0602
			WIN32_LEAN_AND_MEAN
		)
	endif()
endif()


set(_EXPORT)
if(DEFINED QLITEHTML_EXPORT)
	set(_EXPORT EXPORT ${QLITEHTML_EXPORT})
	add_library(${QLITEHTML_EXPORT}::qlitehtml ALIAS qlitehtml)
endif()


target_include_directories(qlitehtml PUBLIC
	$<INSTALL_INTERFACE:qlitehtml/include>  # <prefix>/qlitehtml/include
)

# -----------------------------------------------------------------------------

install(TARGETS qlitehtml
		EXPORT qlitehtml
		ARCHIVE DESTINATION "qlitehtml/lib"
		PUBLIC_HEADER DESTINATION "qlitehtml/include"
)
install(EXPORT qlitehtml DESTINATION qlitehtml/cmake)


