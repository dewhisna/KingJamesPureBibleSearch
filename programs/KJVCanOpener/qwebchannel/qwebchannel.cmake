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

include(ExternalProject)

ExternalProject_Add(fancytree
	GIT_REPOSITORY		https://github.com/dewhisna/fancytree.git
	GIT_TAG				aeac90258778418385ec3265cc1827fb93fa6e16
	PREFIX				"jquery-ui/fancytree"
	BUILD_COMMAND		""
	BUILD_IN_SOURCE		true
	UPDATE_COMMAND		""
	INSTALL_COMMAND		${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/fancytree/src/fancytree/dist/" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/fancytree/dist/"
	CONFIGURE_COMMAND	""
)

ExternalProject_Add(geolocator
	GIT_REPOSITORY		https://github.com/dewhisna/geolocator.git
	GIT_TAG				42f51bb6aad3c3ebace000bc7909d17312a9e7d4
	PREFIX				"jquery-ui/geolocator"
	BUILD_COMMAND		""
	BUILD_IN_SOURCE		true
	UPDATE_COMMAND		""
	INSTALL_COMMAND		${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/geolocator/src/geolocator/src/" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/geolocator/src/"
	CONFIGURE_COMMAND	""
)

ExternalProject_Add(jquery-loading-overlay
	GIT_REPOSITORY		https://github.com/dewhisna/jquery-loading-overlay.git
	GIT_TAG				8e8d26fb6964b2f790ad291c2087a3c6760b675a
	PREFIX				"jquery-ui/jquery-loading-overlay"
	BUILD_COMMAND		""
	BUILD_IN_SOURCE		true
	UPDATE_COMMAND		""
	INSTALL_COMMAND		${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/jquery-loading-overlay/src/jquery-loading-overlay/dist/" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/jquery-loading-overlay/dist/"
			COMMAND		${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/jquery-loading-overlay/src/jquery-loading-overlay/demo/fonts/" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/jquery-loading-overlay/demo/fonts/"
	CONFIGURE_COMMAND	""
)

ExternalProject_Add(malihu-custom-scrollbar-plugin
	GIT_REPOSITORY		https://github.com/dewhisna/malihu-custom-scrollbar-plugin.git
	GIT_TAG				728a92de311a274e8c81ce9c2ff4c3d71d247d8a
	PREFIX				"jquery-ui/malihu-custom-scrollbar-plugin"
	BUILD_COMMAND		""
	BUILD_IN_SOURCE		true
	UPDATE_COMMAND		""
	INSTALL_COMMAND		${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/malihu-custom-scrollbar-plugin/src/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.js" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.js"
			COMMAND		${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/malihu-custom-scrollbar-plugin/src/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.concat.min.js" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.concat.min.js"
			COMMAND		${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/malihu-custom-scrollbar-plugin/src/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.css" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.css"
			COMMAND		${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/malihu-custom-scrollbar-plugin/src/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.min.css" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/malihu-custom-scrollbar-plugin/jquery.mCustomScrollbar.min.css"
	CONFIGURE_COMMAND	""
)

ExternalProject_Add(turn.js
	GIT_REPOSITORY		https://github.com/dewhisna/turn.js.git
	GIT_TAG				08c1f6599a1412b145c7bf645a1f28c14db12742
	PREFIX				"jquery-ui/turn.js"
	BUILD_COMMAND		""
	BUILD_IN_SOURCE		true
	UPDATE_COMMAND		""
	INSTALL_COMMAND		${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/turn.js/src/turn.js/turn.js" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/turn.js/turn.js"
			COMMAND		${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/jquery-ui/turn.js/src/turn.js/turn.min.js" "${CMAKE_CURRENT_BINARY_DIR}/html/jquery/jquery-ui/turn.js/turn.min.js"
	CONFIGURE_COMMAND	""
)

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

	add_dependencies(${Target}
		fancytree
		geolocator
		jquery-loading-overlay
		malihu-custom-scrollbar-plugin
		turn.js
	)
endfunction()

# =============================================================================

