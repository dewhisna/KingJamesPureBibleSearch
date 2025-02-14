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
set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================

set(grantlee_SHARED_HEADERS
	abstractmarkupbuilder.h
	grantlee_gui_export.h
	grantlee_textdocument.h
	bbcodebuilder.h
	markupdirector.h
	mediawikimarkupbuilder.h
	plaintextmarkupbuilder.h
	texthtmlbuilder.h
)

set(grantlee_PRIVATE_HEADERS
	markupdirector_p.h
)

set(grantlee_SOURCES
	bbcodebuilder.cpp
	markupdirector.cpp
	mediawikimarkupbuilder.cpp
	plaintextmarkupbuilder.cpp
	texthtmlbuilder.cpp
)

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
endif()

# -----------------------------------------------------------------------------


add_library(grantlee STATIC ${grantlee_SOURCES} ${grantlee_SHARED_HEADERS} ${grantlee_PRIVATE_HEADERS})
set_target_properties(grantlee PROPERTIES PUBLIC_HEADER "${grantlee_SHARED_HEADERS}")
target_compile_options(grantlee PUBLIC ${MY_COMPILE_OPTIONS})
target_link_libraries(grantlee
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Gui
)

if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(grantlee PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()

if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(grantlee PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

target_include_directories(grantlee SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:grantlee/include>  # <prefix>/grantlee/include
)

install(TARGETS grantlee
		EXPORT grantlee
		ARCHIVE DESTINATION "grantlee/lib"
		PUBLIC_HEADER DESTINATION "grantlee/include"
)
install(EXPORT grantlee DESTINATION grantlee/cmake)


