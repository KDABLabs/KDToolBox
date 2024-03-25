# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(
    Qt${QT_VERSION_MAJOR}
    ${QT_REQUIRED_VERSION}
    CONFIG
    REQUIRED
    Core
    Gui
    Test
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../fmt/include/)

set(test_qt_fmt_SOURCES ../fmt/src/format.cc tst_qt_fmt.cpp)

add_executable(test_qt_fmt ${test_qt_fmt_SOURCES})
target_link_libraries(test_qt_fmt PUBLIC Qt::Core Qt::Gui Qt::Test)
