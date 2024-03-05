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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(test_qt_std_format_SOURCES tst_qt_std_format.cpp)

add_executable(test_qt_std_format ${test_qt_std_format_SOURCES})
target_link_libraries(test_qt_std_format PUBLIC Qt::Core Qt::Gui Qt::Test)
