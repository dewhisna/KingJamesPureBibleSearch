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

cmake_minimum_required(VERSION 3.20...3.28)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================

# Qt has made a royal mess out of the zlib stuff.  Back in the Qt 4 days with
#	qmake, you could just include zlib in the .pro file list of components and get the
#	Qt bundled zlib as a qt component include.  Qt 5 seems to have started out
#	like that with a the Qt5Zlib component (unless of course you are using the
#	system zlib and didn't compile it in the Qt bundle).  But Qt 6 seems to have
#	dropped it completely for the desktop targets.  However, they have introduced
#	a new one, at least for the WASM targets.  And instead of Qt6Zlib, it's called
#	Qt6BundledZLIB.  There is also a FindWrapZLIB helper, but of course it only
#	works if the Qt build actually has it available.  In the early Qt 6 (like 6.2.2)
#	versions, it worked OK.  But strangely, it's moved its Qt6Zlib to an internal-only
#	Qt6ZlibPrivate package, so apps can no longer use it. However, they apparently
#	make the Qt6BundledZLIB available for the WASM target, perhaps to minimize code
#	size there of having multiple identical zlibs linked?
#
# Anyway...  to deal with this mess...  This code will look for either a Qt{ver}Zlib
#	package or a Qt{ver}BundledZLIB package and use those if it exists, which will cover
#	things on Qt 5 with a bundled Zlib or Qt 6 on the WASM target.  If those don't exist,
#	we will simply build it from our own copy of the zlib source, which we snarfed out of
#	the Qt source tree, building it as our own internal static library.
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
	else()
		add_subdirectory(../../../support/zlib zlib)
		message(STATUS "${CMAKE_PROJECT_NAME} : Building zlib from source")
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
	MyZLIB
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


