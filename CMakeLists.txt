# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.5)

project(KDToolBox LANGUAGES CXX)

option(KDTOOLBOX_CXX20 "Enabling C++20 tests" OFF)

include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE TRUE)
set(CMAKE_LINK_DEPENDS_NO_SHARED TRUE)

set(QT_REQUIRED_VERSION "5.15.0")
if(NOT DEFINED QT_VERSION_MAJOR)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
endif()
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Gui)

set(CMAKE_AUTOMOC ON)

if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

add_definitions(
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x060200
)

add_subdirectory(cpp)
add_subdirectory(qt)
add_subdirectory(general)
