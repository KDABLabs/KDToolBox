# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(CMAKE_CXX_STANDARD 14)

find_package(
    Qt${QT_VERSION_MAJOR}
    ${QT_REQUIRED_VERSION}
    CONFIG
    REQUIRED
    Core
    Gui
    Widgets
)

include_directories(../../src/)

set(signalThrottlersDemo_SOURCES ../../src/KDSignalThrottler.cpp ../../src/KDSignalThrottler.h main.cpp)

add_executable(signalThrottlersDemo ${signalThrottlersDemo_SOURCES})
target_link_libraries(signalThrottlersDemo PUBLIC Qt::Core Qt::Gui Qt::Widgets)
