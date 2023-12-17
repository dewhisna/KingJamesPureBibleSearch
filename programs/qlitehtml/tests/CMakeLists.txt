# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)

add_executable(qlitehtmlbrowser
  ${GUI_TYPE}
  main.cpp)

target_link_libraries(qlitehtmlbrowser qlitehtml Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)
