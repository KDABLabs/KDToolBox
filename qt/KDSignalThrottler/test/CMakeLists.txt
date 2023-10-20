# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../src/)

set(test_SOURCES ../src/KDSignalThrottler.cpp ../src/KDSignalThrottler.h tst_KDSignalThrottler.cpp)

add_executable(test ${test_SOURCES})
target_link_libraries(test PUBLIC Qt::Core Qt::Test)
