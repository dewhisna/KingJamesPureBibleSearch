cmake_minimum_required(VERSION 3.20)

project(HarExtract VERSION 1.0.0 DESCRIPTION "Firefox TalkingBible Har File Extractor" LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(SOURCE_FILES
	HarExtract.cpp
)

add_executable(HarExtract ${SOURCE_FILES})

target_link_libraries(HarExtract PRIVATE
	Qt${QT_VERSION_MAJOR}::Core
)


