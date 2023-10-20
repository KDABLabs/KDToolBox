# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(../../src/)

set(tst_messagehandler_SOURCES ../../src/messagehandler.cpp ../../src/messagehandler.h tst_messagehandler.cpp)

add_executable(tst_messagehandler ${tst_messagehandler_SOURCES})
target_link_libraries(tst_messagehandler PUBLIC Qt::Core Qt::Test)
