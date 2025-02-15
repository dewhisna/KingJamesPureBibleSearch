##*****************************************************************************
##
## Copyright (C) 2025 Donna Whisnant, a.k.a. Dewtronics.
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
include_guard(GLOBAL)

function(string_quote var str)
	# CMake does not support escaping string values for use in JSON. However, it
	# does properly escape keys in JSON object literals. We use that fact to escape
	# string values by constructing a JSON object with a single named property,
	# whose key is the string that we want to escape. Then we strip away all of the
	# stringified JSON object except said key.
	string(JSON tmp_json SET "{}" "${str}" "null")
	set(ws "[ \t\r\n]*")
	if(tmp_json MATCHES "^\\{${ws}(\".*\")${ws}:${ws}null${ws}\\}\$")
		set(${var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
	else()
		message(SEND_ERROR "string JSON produced unexpected output ${tmp_json}")
	endif()
endfunction()

# Output files:
#
# [HEADER_FILE [version.h]]
# Write a C preprocessor header file containing the PROJECT_VERSION* variables as `#define NAME value
# All values are "string", except for PROJECT_VERSION_{MAJOR,MINOR,PATCH,TWEAK} which are integers
# This is intended to be compatible with (at least) C/C++/RC/MIDL/ISPP, or other languages with a C-like #include facility
#
# [JSON_FILE [VERSION.json]])
# Same as above, but as a JSON object instead of #defines

function(project_genver)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "HEADER_FILE;JSON_FILE" "")

	if(NOT (PROJECT_NAME AND PROJECT_SOURCE_DIR AND PROJECT_BINARY_DIR))
		message(FATAL_ERROR "call project() before project_genver()")
	endif()

	set(return_vars)
	function(set_project suffix value)
		set(vars "PROJECT_${suffix}" "${PROJECT_NAME}_${suffix}")
		if(PROJECT_IS_TOP_LEVEL) # CMAKE_PROJECT_* was introduced in 3.12, but before 3.21 introduced PROJECT_IS_TOP_LEVEL project() didn't record any way to know
			list(APPEND vars "CMAKE_PROJECT_${suffix}")
		endif()
		foreach(var ${vars})
			set(${var} "${value}" PARENT_SCOPE)
		endforeach()
		set(return_vars ${return_vars} ${vars} PARENT_SCOPE)
	endfunction()

	# Current Project inherits version of Top-Project if not set otherwise:
	if("${PROJECT_VERSION}" STREQUAL "")
		set(PROJECT_VERSION ${CMAKE_PROJECT_VERSION})
		if(NOT "${CMAKE_PROJECT_VERSION_MAJOR}" STREQUAL "")
			set(PROJECT_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
		endif()
		if(NOT "${CMAKE_PROJECT_VERSION_MINOR}" STREQUAL "")
			set(PROJECT_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
		endif()
		if(NOT "${CMAKE_PROJECT_VERSION_PATCH}" STREQUAL "")
			set(PROJECT_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})
		endif()
		if(NOT "${CMAKE_PROJECT_VERSION_TWEAK}" STREQUAL "")
			set(PROJECT_VERSION_TWEAK ${CMAKE_PROJECT_VERSION_TWEAK})
		endif()
	endif()

	if(("${CMAKE_PROJECT_VERSION}" STREQUAL "") OR ("${PROJECT_VERSION}" STREQUAL ""))
		message(FATAL_ERROR "No VERSION defined on project(s), cannot create version.h")
	endif()

	if(NOT "${PROJECT_VERSION_PRERELEASE}" STREQUAL "")
		set(PROJECT_VERSION_SEMVER "${PROJECT_VERSION}-${PROJECT_VERSION_PRERELEASE}")
	else()
		set(PROJECT_VERSION_SEMVER "${PROJECT_VERSION}")
	endif()

	set(out_suffix NAME DESCRIPTION HOMEPAGE_URL VERSION_SEMVER VERSION VERSION_MAJOR VERSION_MINOR VERSION_PATCH VERSION_TWEAK VERSION_PRERELEASE VERSION_PRIVATEBUILD VERSION_SPECIALBUILD)
	set(quote_suffix NAME DESCRIPTION HOMEPAGE_URL VERSION_SEMVER VERSION VERSION_PRERELEASE VERSION_PRIVATEBUILD VERSION_SPECIALBUILD)

	if(HEADER_FILE IN_LIST arg_KEYWORDS_MISSING_VALUES)
		set(arg_HEADER_FILE ${PROJECT_BINARY_DIR}/version.h)
	endif()
	if(arg_HEADER_FILE)
		string(MAKE_C_IDENTIFIER "${PROJECT_NAME}" PROJECT_IDENTIFIER) # CMake allows non-identifier characters like '.' in PROJECT_NAME
		set(cpp_CONTENT [=[
#ifndef PROJECT_NAME
	#define PROJECT_NAME @PROJECT_IDENTIFIER@ /* current project() */
#endif
]=])

		foreach(suffix ${out_suffix})
			if(NOT "${PROJECT_${suffix}}" STREQUAL "")
				if(suffix IN_LIST quote_suffix)
					string_quote(PROJECT_${suffix}_C "${PROJECT_${suffix}}")
					list(APPEND cpp_CONTENT "#define ${PROJECT_IDENTIFIER}_${suffix} @PROJECT_${suffix}_C@")
				else()
					list(APPEND cpp_CONTENT "#define ${PROJECT_IDENTIFIER}_${suffix} @PROJECT_${suffix}@")
				endif()
			else()
				list(APPEND cpp_CONTENT "/* #undef ${PROJECT_IDENTIFIER}_${suffix} */")
			endif()
		endforeach()

		if(PROJECT_VERSION_HEADER_FILE)
			cmake_path(RELATIVE_PATH PROJECT_VERSION_HEADER_FILE BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE RELATIVE_VERSION_HEADER_FILE)
			list(APPEND cpp_CONTENT "\n#include \"${RELATIVE_VERSION_HEADER_FILE}\" /* parent project_genver(HEADER_FILE) */")
		elseif(PROJECT_IS_TOP_LEVEL)
			list(APPEND cpp_CONTENT "\n#define CMAKE_PROJECT_NAME @PROJECT_IDENTIFIER@ /* top-level project() */")
		endif()

		list(JOIN cpp_CONTENT "\n" cpp_CONTENT)
		set(INCLUDE_GUARD "Version_${PROJECT_IDENTIFIER}_HEADER_FILE")
		file(CONFIGURE OUTPUT ${arg_HEADER_FILE} CONTENT "#ifndef ${INCLUDE_GUARD}\n#define ${INCLUDE_GUARD}\n\n${cpp_CONTENT}\n\n#endif /* ${INCLUDE_GUARD} */\n" @ONLY)
		set_project(VERSION_HEADER_FILE "${arg_HEADER_FILE}")
	endif()

	if(JSON_FILE IN_LIST arg_KEYWORDS_MISSING_VALUES)
		set(arg_JSON_FILE ${PROJECT_BINARY_DIR}/version.json)
	endif()
	if(arg_JSON_FILE)
		set(json_CONTENT)
		foreach(prefix CMAKE_PROJECT_NAME PROJECT_NAME)
			string_quote(${prefix}_JSON "${${prefix}}")
			list(APPEND json_CONTENT "\"${prefix}\": @${prefix}_JSON@")
			set(prefix "${${prefix}}")
			set(prefix_CONTENT)
			foreach(suffix ${out_suffix})
				if(NOT "${${prefix}_${suffix}}" STREQUAL "")
					if(suffix IN_LIST quote_suffix)
						string_quote(${prefix}_${suffix}_JSON "${${prefix}_${suffix}}")
						list(APPEND prefix_CONTENT "\"${suffix}\": @${prefix}_${suffix}_JSON@")
					else()
						list(APPEND prefix_CONTENT "\"${suffix}\": @${prefix}_${suffix}@")
					endif()
				endif()
			endforeach()
			list(JOIN prefix_content ",\n" prefix_content)
			list(APPEND json_CONTENT "\"${prefix}\": {\n${prefix_CONTENT}\n}")
		endforeach()
		list(JOIN json_CONTENT ",\n" json_CONTENT)
		file(CONFIGURE OUTPUT ${arg_JSON_FILE} CONTENT "{\n${json_CONTENT}\n}\n" @ONLY)
	endif()

	foreach(var ${return_vars})
		set(${var} "${${var}}" PARENT_SCOPE)
	endforeach()
endfunction()

# Add a VS_VERSION_INFO resource to the target based on project(VERSION ... DESCRIPTION ...)
# and project_genver()
#
# [SpecialBuild|PrivateBuild "..."]
# Sets the string and corresponding VS_FF_SPECIALBUILD|VS_FF_PRIVATEBUILD in FileFlags
#
# [Name "..."]...
# Any other VALUE "Name" "..." pairs you would like to add to the StringFileInfo table
# explorer doesn't really show custom additions in its GUI, though they do get written

function(target_versioninfo target)
	cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "")
	if(WIN32 OR WIN32_WINE)
		set(VS_VERSION_INFO_StringFileInfo)
		set(versioninfo_CONTENT)

		# Get a list of the optional keys present in StringFileInfo
		set(StringFileInfo_Keys "${ARGN}")
		list(TRANSFORM StringFileInfo_Keys REPLACE "^([^=]*)=.*" "\\1")

		if(NOT "FileDescription" IN_LIST StringFileInfo_Keys)
			set(ProjectDescription "${PROJECT_NAME}") # fall back on the name if no description was available
			if(target STREQUAL PROJECT_NAME)
				if(${PROJECT_NAME}_DESCRIPTION) # project(DESCRIPTION "...") if available
					set(ProjectDescription "${${PROJECT_NAME}_DESCRIPTION}")
				endif()
				set(FileDescription "${ProjectDescription}")
			else()
				foreach(prefix PROJECT CMAKE_PROJECT)
					if(${prefix}_DESCRIPTION)
						set(ProjectDescription "${${prefix}_DESCRIPTION}")
						break()
					endif()
				endforeach()
#				set(FileDescription "${target} - ${ProjectDescription}")
				set(FileDescription "${ProjectDescription}")
			endif()
			list(APPEND ARGN "FileDescription=\"${FileDescription}\"")
			list(APPEND StringFileInfo_Keys "FileDescription")
		endif()

		if(PROJECT_VERSION_PRIVATEBUILD AND NOT "PrivateBuild" IN_LIST StringFileInfo_Keys)
			list(APPEND ARGN "PrivateBuild=\"${PROJECT_VERSION_PRIVATEBUILD}\"")
			list(APPEND StringFileInfo_Keys "PrivateBuild")
		endif()

		if(PROJECT_VERSION_SPECIALBUILD AND NOT "SpecialBuild" IN_LIST StringFileInfo_Keys)
			list(APPEND ARGN "SpecialBuild=\"${PROJECT_VERSION_SPECIALBUILD}\"")
			list(APPEND StringFileInfo_Keys "SpecialBuild")
		endif()

		# assume anything else is custom Name1=Value1 Name2=Value2 ...
		foreach(arg IN LISTS ARGN)
			if(arg MATCHES "^([^=]*)=(.*)")
				set(name "${CMAKE_MATCH_1}")
				set(value "${CMAKE_MATCH_2}")
			else()
				message(SEND_ERROR "target_versioninfo: StringFileInfo must be name=value: ${arg}")
			endif()
			list(APPEND VS_VERSION_INFO_StringFileInfo "VALUE \"${name}\", ${value}")
		endforeach()
		string(JOIN "\n\t\t\t" VS_VERSION_INFO_StringFileInfo ${VS_VERSION_INFO_StringFileInfo})

		string(MAKE_C_IDENTIFIER "${PROJECT_NAME}" VS_VERSION_INFO_FILE)
		string(MAKE_C_IDENTIFIER "${CMAKE_PROJECT_NAME}" VS_VERSION_INFO_PRODUCT)

		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/VS_VERSION_INFO.rc.in)
		file(READ ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/VS_VERSION_INFO.rc.in VS_VERSION_INFO_CONTENT)
		# wine's wrc does not handle absolute #include paths, so use a relative one
		if(PROJECT_VERSION_HEADER_FILE)
			cmake_path(RELATIVE_PATH PROJECT_VERSION_HEADER_FILE BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE VERSION_HEADER_FILE)
		else()
			message(SEND_ERROR "target_versioninfo() requires using project_genver(HEADER_FILE)")
		endif()
		string(CONFIGURE "${VS_VERSION_INFO_CONTENT}" VS_VERSION_INFO_CONTENT)
		file(GENERATE OUTPUT ${target}.VS_VERSION_INFO.rc CONTENT "${VS_VERSION_INFO_CONTENT}" TARGET ${target})
		target_sources(${target} PRIVATE ${target}.VS_VERSION_INFO.rc)
	endif()
endfunction()
