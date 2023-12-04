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

add_definitions(-Wall)
include_directories(../src/)

set(test_tmp_1_SOURCES ../src/notifyguard.cpp ../src/notifyguard.h tst_notifyguard.cpp)

add_executable(test_tmp_1 ${test_tmp_1_SOURCES})
target_link_libraries(test_tmp_1 PUBLIC Qt::Core Qt::Gui Qt::Test)
