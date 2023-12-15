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

set(zlib_SOURCES
	"adler32.c"
	"compress.c"
	"crc32.c"
	"deflate.c"
	"gzclose.c"
	"gzlib.c"
	"gzread.c"
	"gzwrite.c"
	"infback.c"
	"inffast.c"
	"inflate.c"
	"inftrees.c"
	"trees.c"
	"uncompr.c"
	"zutil.c"
)

set(zlib_SHARED_HEADERS
	"zlib.h"
	"zconf.h"
)

set(zlib_PRIVATE_HEADERS
	"crc32.h"
	"deflate.h"
	"gzguts.h"
	"inffast.h"
	"inffixed.h"
	"inflate.h"
	"inftrees.h"
	"trees.h"
	"zutil.h"
)

set_property(SOURCE ${zlib_SOURCES} ${zlib_SHARED_HEADERS} ${zlib_PRIVATE_HEADERS} PROPERTY SKIP_AUTOMOC ON)

# =============================================================================

# This is too noisy, besides, this is a 3rdparty dependency:
#if(WIN32)
#	set(MY_COMPILE_OPTIONS "/W3")
#else()
#	set(MY_COMPILE_OPTIONS "-Wall" "-Wextra")
#endif()

if(WIN32)
	set(MY_COMPILE_OPTIONS "/D_CRT_NONSTDC_NO_WARNINGS")
else()
	set(MY_COMPILE_OPTIONS "-Wno-deprecated")
endif()

# -----------------------------------------------------------------------------

add_library(MyZLIB STATIC ${zlib_SOURCES} ${zlib_SHARED_HEADERS} ${zlib_PRIVATE_HEADERS})
set_target_properties(MyZLIB PROPERTIES PUBLIC_HEADER "${zlib_SHARED_HEADERS}")
target_compile_options(MyZLIB PUBLIC ${MY_COMPILE_OPTIONS})

target_include_directories(MyZLIB SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:zlib/include>  # <prefix>/zlib/include
)

set_target_properties(MyZLIB PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(MyZLIB PROPERTIES CXX_VISIBILITY_PRESET hidden)

# -----------------------------------------------------------------------------

install(TARGETS MyZLIB
		EXPORT MyZLIB
		ARCHIVE DESTINATION "zlib/lib"
		PUBLIC_HEADER DESTINATION "zlib/include"
)
install(EXPORT MyZLIB DESTINATION zlib/cmake)


