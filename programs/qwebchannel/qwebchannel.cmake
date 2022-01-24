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

set(qwebchannel_QT_COMPONENTS
	Core Gui Network WebChannel WebSockets Xml
)

set(qwebchannel_path ${CMAKE_CURRENT_LIST_DIR})

# =============================================================================

option(OPTION_USE_MMDB "Use MaxMind GeoLocate Database" ON)

if(OPTION_USE_MMDB)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LIBMAXMINDDB REQUIRED IMPORTED_TARGET libmaxminddb)
endif()

# =============================================================================

set(qwebchannel_SHARED_HEADERS
	"${qwebchannel_path}/webChannelServer.h"
	"${qwebchannel_path}/websocketclientwrapper.h"
)
if(OPTION_USE_MMDB)
	list(APPEND qwebchannel_SHARED_HEADERS
		"${qwebchannel_path}/mmdblookup.h"
	)
endif()

set(qwebchannel_PRIVATE_HEADERS
	"${qwebchannel_path}/webChannelSearchResults.h"
	"${qwebchannel_path}/webChannelObjects.h"
	"${qwebchannel_path}/webChannelGeoLocate.h"
	"${qwebchannel_path}/webChannelBibleAudio.h"
	"${qwebchannel_path}/websockettransport.h"
)

set(qwebchannel_SOURCES
	"${qwebchannel_path}/webChannelSearchResults.cpp"
	"${qwebchannel_path}/webChannelServer.cpp"
	"${qwebchannel_path}/webChannelObjects.cpp"
	"${qwebchannel_path}/webChannelGeoLocate.cpp"
	"${qwebchannel_path}/webChannelBibleAudio.cpp"
	"${qwebchannel_path}/websocketclientwrapper.cpp"
	"${qwebchannel_path}/websockettransport.cpp"
	# ----
	"${CMAKE_CURRENT_BINARY_DIR}/webChannelKeys.cpp"		# Generated file from webChannelKeyGen.sh custom_command
)
if(OPTION_USE_MMDB)
	list(APPEND qwebchannel_SOURCES
		"${qwebchannel_path}/mmdblookup.cpp"
	)
endif()

# =============================================================================

add_custom_command(
	OUTPUT html/admin/webChannelKeys.js "${CMAKE_CURRENT_BINARY_DIR}/webChannelKeys.cpp"
	COMMAND "${qwebchannel_path}/webChannelKeyGen.sh"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
	VERBATIM
)

# =============================================================================

set(qwebchannel_link_libraries)
set(qwebchannel_compile_definitions
	USING_WEBCHANNEL
)
if(OPTION_USE_MMDB)
	list(APPEND qwebchannel_compile_definitions
		USING_MMDB
	)
endif()
if(OPTION_USE_MMDB)
	list(APPEND qwebchannel_link_libraries
		PkgConfig::LIBMAXMINDDB
	)
endif()

# =============================================================================

function(setup_qwebchannel_subtargets Target)
	# Note: these here HAVE to use ${qwebchannel_path} instead of
	#	${CMAKE_CURRENT_LIST_DIR} because that value changes for
	#	the include and where this function gets called from...
	add_custom_command(TARGET ${Target}
		POST_BUILD
		COMMENT "Deploying WebChannel HTML files..."
		COMMAND ${CMAKE_COMMAND} -E copy_directory "html/" "${CMAKE_CURRENT_BINARY_DIR}/html/"
		WORKING_DIRECTORY "${qwebchannel_path}"
		VERBATIM
	)

	target_include_directories(${Target} PRIVATE ${qwebchannel_path})

	target_compile_definitions(${Target} PRIVATE
		${qwebchannel_compile_definitions}
	)

	target_link_libraries(${Target} PRIVATE
		${qwebchannel_link_libraries}
	)
endfunction()

# =============================================================================

