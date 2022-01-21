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

set(QT_COMPONENTS
	Core Gui Network WebChannel WebSockets Xml
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${QT_COMPONENTS})
if(QT_VERSION_MAJOR EQUAL 6)
	# Qt6 drops SAX XML Parser which we need.  For now, get it from the Qt5 compatibility shim:
	list(APPEND QT_COMPONENTS Core5Compat)
endif()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================

option(OPTION_USE_MMDB "Use MaxMind GeoLocate Database" ON)

if(OPTION_USE_MMDB)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LIBMAXMINDDB REQUIRED IMPORTED_TARGET libmaxminddb)
endif()

# =============================================================================

set(qwebchannel_SHARED_HEADERS
	webChannelServer.h
	websocketclientwrapper.h
)
if(OPTION_USE_MMDB)
	list(APPEND qwebchannel_SHARED_HEADERS
		mmdblookup.h
	)
endif()

set(qwebchannel_PRIVATE_HEADERS
	webChannelSearchResults.h
	webChannelObjects.h
	webChannelGeoLocate.h
	webChannelBibleAudio.h
	websockettransport.h
)

set(qwebchannel_SOURCES
	webChannelSearchResults.cpp
	webChannelServer.cpp
	webChannelObjects.cpp
	webChannelGeoLocate.cpp
	webChannelBibleAudio.cpp
	websocketclientwrapper.cpp
	websockettransport.cpp
)
if(OPTION_USE_MMDB)
	list(APPEND qwebchannel_SOURCES
		mmdblookup.cpp
	)
endif()

# =============================================================================

if(WIN32)
	set(MY_COMPILE_OPTIONS "/W3")
else()
	set(MY_COMPILE_OPTIONS "-Wall")
endif()

# -----------------------------------------------------------------------------

add_custom_command(
	OUTPUT html/admin/webChannelKeys.js "${CMAKE_CURRENT_BINARY_DIR}/webChannelKeys.cpp"
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/webChannelKeyGen.sh"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_library(qwebchannel STATIC ${qwebchannel_SOURCES} ${qwebchannel_SHARED_HEADERS} ${qwebchannel_PRIVATE_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/webChannelKeys.cpp")
set_target_properties(qwebchannel PROPERTIES PUBLIC_HEADER "${qwebchannel_SHARED_HEADERS}")
target_compile_options(qwebchannel PUBLIC ${MY_COMPILE_OPTIONS})
target_include_directories(qwebchannel PRIVATE ../KJVCanOpener)
set(QT_LINK_LIBS)
foreach(_component ${QT_COMPONENTS})
	list(APPEND QT_LINK_LIBS Qt${QT_VERSION_MAJOR}::${_component})
endforeach()
target_link_libraries(qwebchannel
	${QT_LINK_LIBS}
)

if(DEPRECATION_ENABLE_WARNINGS)
	target_compile_definitions(qwebchannel PRIVATE
		QT_DEPRECATED_WARNINGS
	)
endif()

if(DEPRECATION_DISABLE_APIS)
	target_compile_definitions(qwebchannel PRIVATE
		QT_DISABLE_DEPRECATED_BEFORE=0x060000			# disables all the APIs deprecated before Qt 6.0.0
	)
endif()

target_compile_definitions(qwebchannel PRIVATE
	USING_WEBCHANNEL
	NOT_USING_SQL
)
if(OPTION_USE_MMDB)
	target_compile_definitions(qwebchannel PRIVATE
		USING_MMDB
	)
endif()
if(OPTION_USE_MMDB)
	target_link_libraries(qwebchannel
		PkgConfig::LIBMAXMINDDB
	)
endif()

target_include_directories(qwebchannel SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:qwebchannel/include>  # <prefix>/qwebchannel/include
)

add_custom_command(TARGET qwebchannel
	POST_BUILD
	COMMENT "Deploying WebChannel HTML files..."
	COMMAND ${CMAKE_COMMAND} -E copy_directory "html/" "${CMAKE_CURRENT_BINARY_DIR}/html/"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# -----------------------------------------------------------------------------

install(TARGETS qwebchannel
		EXPORT qwebchannel
		ARCHIVE DESTINATION "qwebchannel/lib"
		PUBLIC_HEADER DESTINATION "qwebchannel/include"
)
install(EXPORT qwebchannel DESTINATION qwebchannel/cmake)


