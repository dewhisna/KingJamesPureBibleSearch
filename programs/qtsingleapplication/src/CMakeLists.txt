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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================

option(QtSingleApplication_Application "Enable Single QApplication" ON)
option(QtSingleApplication_CoreApplication "Enable Single QCoreApplication" OFF)

set(SINGLE_APPLICATION_TARGETS)
if(QtSingleApplication_Application)
	list(APPEND SINGLE_APPLICATION_TARGETS qtsingleapplication)
endif()
if(QtSingleApplication_CoreApplication)
	list(APPEND SINGLE_APPLICATION_TARGETS qtsinglecoreapplication)
endif()

# =============================================================================

set(qtsingleapplication_SHARED_HEADERS
	QtLockedFile
	qtlockedfile.h
	qtlocalpeer.h
	qtsingleapplication.h
)

set(qtsingleapplication_PRIVATE_HEADERS
)

set(qtsingleapplication_SOURCES
	qtsingleapplication.cpp
	qtlocalpeer.cpp
	# Note: qtlocalpeer.cpp slurps in qtlockedfile.cpp, qtlockedfile_unix.cpp, and qtlockedfile_win.cpp
	#	so those sources aren't listed here
)

# -----------------------------------------------------------------------------

set(qtsinglecoreapplication_SHARED_HEADERS
	QtLockedFile
	qtlockedfile.h
	qtlocalpeer.h
	qtsinglecoreapplication.h
)

set(qtsinglecoreapplication_PRIVATE_HEADERS
)

set(qtsinglecoreapplication_SOURCES
	qtsinglecoreapplication.cpp
	qtlocalpeer.cpp
	# Note: qtlocalpeer.cpp slurps in qtlockedfile.cpp, qtlockedfile_unix.cpp, and qtlockedfile_win.cpp
	#	so those sources aren't listed here
)

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall")
endif()

# -----------------------------------------------------------------------------


if(QtSingleApplication_Application)
	add_library(qtsingleapplication STATIC ${qtsingleapplication_SOURCES} ${qtsingleapplication_SHARED_HEADERS} ${qtsingleapplication_PRIVATE_HEADERS})
	set_target_properties(qtsingleapplication PROPERTIES PUBLIC_HEADER "${qtsingleapplication_SHARED_HEADERS}")
	target_compile_options(qtsingleapplication PUBLIC ${MY_COMPILE_OPTIONS})
	target_link_libraries(qtsingleapplication
		Qt${QT_VERSION_MAJOR}::Core
		Qt${QT_VERSION_MAJOR}::Gui
		Qt${QT_VERSION_MAJOR}::Widgets
		Qt${QT_VERSION_MAJOR}::Network
	)

	if(DEPRECATION_ENABLE_WARNINGS)
		target_compile_definitions(qtsingleapplication PRIVATE
			QT_DEPRECATED_WARNINGS
		)
	endif()

	if(DEPRECATION_DISABLE_APIS)
		target_compile_definitions(qtsingleapplication PRIVATE
			QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
		)
	endif()

	target_include_directories(qtsingleapplication SYSTEM INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:qtsingleapplication/include>  # <prefix>/qtsingleapplication/include
	)
endif()

# -----------------------------------------------------------------------------

if(QtSingleApplication_CoreApplication)
	add_library(qtsinglecoreapplication STATIC ${qtsinglecoreapplication_SOURCES} ${qtsinglecoreapplication_SHARED_HEADERS} ${qtsinglecoreapplication_PRIVATE_HEADERS})
	set_target_properties(qtsinglecoreapplication PROPERTIES PUBLIC_HEADER "${qtsinglecoreapplication_SHARED_HEADERS}")
	target_compile_options(qtsinglecoreapplication PUBLIC ${MY_COMPILE_OPTIONS})
	target_link_libraries(qtsinglecoreapplication
		Qt${QT_VERSION_MAJOR}::Core
		Qt${QT_VERSION_MAJOR}::Gui
		Qt${QT_VERSION_MAJOR}::Network
	)

	if(DEPRECATION_ENABLE_WARNINGS)
		target_compile_definitions(qtsinglecoreapplication PRIVATE
			QT_DEPRECATED_WARNINGS
		)
	endif()

	if(DEPRECATION_DISABLE_APIS)
		target_compile_definitions(qtsinglecoreapplication PRIVATE
			QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
		)
	endif()

	target_include_directories(qtsinglecoreapplication SYSTEM INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:qtsingleapplication/include>  # <prefix>/qtsingleapplication/include
	)
endif()

# -----------------------------------------------------------------------------

install(TARGETS ${SINGLE_APPLICATION_TARGETS}
		EXPORT qtsingleapplication
		ARCHIVE DESTINATION "qtsingleapplication/lib"
		PUBLIC_HEADER DESTINATION "qtsingleapplication/include"
)
install(EXPORT qtsingleapplication DESTINATION qtsingleapplication/cmake)


