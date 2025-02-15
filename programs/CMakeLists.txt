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

# Note: 3.21 needed for PROJECT_IS_TOP_LEVEL
cmake_minimum_required(VERSION 3.21...3.28)

include(cmake/GenerateVersion.cmake)

set(LANGUAGES C CXX)
if(WIN32)
	list(APPEND LANGUAGES RC)
endif()

project(KJPBS VERSION 4.0.0 DESCRIPTION "KJPBS" LANGUAGES ${LANGUAGES})
project_genver(HEADER_FILE
	DEFINES
		LEGAL_COPYRIGHT="Copyright\(c\)2012-2025 Donna Whisnant, a.k.a. Dewtronics"
		ORGNAME="Dewtronics"
		ORGDOMAIN="dewtronics.com"
)

add_subdirectory(KJVCanOpener)
add_subdirectory(KJVDataParse)
add_subdirectory(KJVDataDump)
add_subdirectory(KJVDictWord)
add_subdirectory(KJVDiff)
add_subdirectory(KJVLookup)
add_subdirectory(KJVPhraseDump)
add_subdirectory(KJVPhraseSearch)
add_subdirectory(KJVSearch)
add_subdirectory(KJVSumThing)
# -------------------------------------
add_subdirectory(MyBibleParse)
# -------------------------------------
add_subdirectory(eSwordParse)
add_subdirectory(eSwordDecrypt2)
# -------------------------------------
add_subdirectory(ELSSearch)
# -------------------------------------
add_subdirectory(CrossRefParse)
