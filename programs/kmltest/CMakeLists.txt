##*****************************************************************************
##
## Copyright (C) 2022 Donna Whisnant, a.k.a. Dewtronics.
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

project(kmltest VERSION 1.0.0 DESCRIPTION "KML Parser Test" LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(QT_COMPONENTS Core Gui Widgets Xml Positioning)
set(QT_COMPONENTS Core Gui Xml Positioning)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${QT_COMPONENTS})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})

set(QT_LINK_LIBS)
foreach(_component ${QT_COMPONENTS})
	list(APPEND QT_LINK_LIBS Qt${QT_VERSION_MAJOR}::${_component})
endforeach()

add_executable(kmltest
	kmltest.cpp
	../KJVCanOpener/KML.cpp
	../KJVCanOpener/KML.h
	../KJVCanOpener/XML.cpp
	../KJVCanOpener/XML.h
)
target_link_libraries(kmltest PRIVATE
	${QT_LINK_LIBS}
)



