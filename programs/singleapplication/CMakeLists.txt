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
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================

set(singleapplication_SHARED_HEADERS
	singleapplication.h
)

set(singleapplication_PRIVATE_HEADERS
	singleapplication_p.h
)

set(singleapplication_SOURCES
	singleapplication.cpp
)

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
endif()

# -----------------------------------------------------------------------------


add_library(singleapplication STATIC ${singleapplication_SOURCES} ${singleapplication_SHARED_HEADERS} ${singleapplication_PRIVATE_HEADERS})
set_target_properties(singleapplication PROPERTIES PUBLIC_HEADER "${singleapplication_SHARED_HEADERS}")
target_compile_options(singleapplication PUBLIC ${MY_COMPILE_OPTIONS})
target_link_libraries(singleapplication
	Qt${QT_VERSION_MAJOR}::Core
)

if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(singleapplication PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()

if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(singleapplication PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

target_include_directories(singleapplication SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:singleapplication/include>  # <prefix>/singleapplication/include
)

# -----------------------------------------------------------------------------

install(TARGETS singleapplication
		EXPORT singleapplication
		ARCHIVE DESTINATION "singleapplication/lib"
		PUBLIC_HEADER DESTINATION "singleapplication/include"
)
install(EXPORT singleapplication DESTINATION singleapplication/cmake)


