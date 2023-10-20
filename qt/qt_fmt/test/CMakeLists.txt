# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

set(test_tmp_2_SOURCES ../fmt/src/format.cc tst_qt_fmt.cpp)

add_executable(test_tmp_2 ${test_tmp_2_SOURCES})
target_link_libraries(test_tmp_2 PUBLIC Qt::Core Qt::Gui Qt::Test)
