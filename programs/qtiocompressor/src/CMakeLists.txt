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
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================

# Qt6 adds a WrapZLIB that isn't in Qt5.  Try it first and use it
#	if it's available.  If not, try to implement it ourselves.  This
#	code based on ~/Qt/6.2.2/lib/cmake/Qt6/FindWrapZLIB.cmake
find_package(WrapZLIB)

if(NOT WrapZLIB_FOUND)
	find_package(ZLIB ${WrapZLIB_FIND_VERSION})

	if(ZLIB_FOUND)
	    set(WrapZLIB_FOUND ON)

	    add_library(WrapZLIB::WrapZLIB INTERFACE IMPORTED)
	    if(APPLE)
    	    # On Darwin platforms FindZLIB sets IMPORTED_LOCATION to the absolute path of the library
    	    # within the framework. This ends up as an absolute path link flag, which we don't want,
    	    # because that makes our .prl files un-relocatable and also breaks iOS simulator_and_device
    	    # SDK switching in Xcode.
    	    # Just pass a linker flag instead.
    	    target_link_libraries(WrapZLIB::WrapZLIB INTERFACE "-lz")
	    else()
    	    target_link_libraries(WrapZLIB::WrapZLIB INTERFACE ZLIB::ZLIB)
		endif()
	endif()
endif()

# =============================================================================

set(QtIOCompressor_SHARED_HEADERS
	QtIOCompressor
	qtiocompressor.h
)

set(QtIOCompressor_PRIVATE_HEADERS
)

set(QtIOCompressor_SOURCES
	qtiocompressor.cpp
)

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
endif()

# -----------------------------------------------------------------------------


add_library(QtIOCompressor STATIC ${QtIOCompressor_SOURCES} ${QtIOCompressor_SHARED_HEADERS} ${QtIOCompressor_PRIVATE_HEADERS})
set_target_properties(QtIOCompressor PROPERTIES PUBLIC_HEADER "${QtIOCompressor_SHARED_HEADERS}")
target_compile_options(QtIOCompressor PUBLIC ${MY_COMPILE_OPTIONS})
target_link_libraries(QtIOCompressor
	Qt${QT_VERSION_MAJOR}::Core
	WrapZLIB::WrapZLIB
)

if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(QtIOCompressor PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()

if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(QtIOCompressor PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

target_include_directories(QtIOCompressor SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:QtIOCompressor/include>  # <prefix>/QtIOCompressor/include
)

# -----------------------------------------------------------------------------

install(TARGETS QtIOCompressor
		EXPORT QtIOCompressor
		ARCHIVE DESTINATION "QtIOCompressor/lib"
		PUBLIC_HEADER DESTINATION "QtIOCompressor/include"
)
install(EXPORT QtIOCompressor DESTINATION QtIOCompressor/cmake)


