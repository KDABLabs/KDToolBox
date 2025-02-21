# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.14)

project(debug LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Gui)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

qt_add_executable(debug main.cpp)
qt_add_resources(debug "debug_assets" PREFIX "/" FILES kdab-logo.png)
target_link_libraries(debug PRIVATE Qt6::Gui)

install(TARGETS debug LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
