# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../../../include)

set(tst_duplicatetracker_20_SOURCES ../tst_duplicatetracker.cpp)

add_executable(tst_duplicatetracker_20 ${tst_duplicatetracker_20_SOURCES})
target_link_libraries(tst_duplicatetracker_20 PUBLIC Qt::Core Qt::Test)
