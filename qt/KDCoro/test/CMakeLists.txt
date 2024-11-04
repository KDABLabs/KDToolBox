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
    Sql
    Test
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(tst_KDCoro_SOURCES tst_KDCoro.cpp ../KDCoro.h)

add_executable(tst_KDCoro ${tst_KDCoro_SOURCES})
target_link_libraries(tst_KDCoro PUBLIC Qt::Core Qt::Test)
