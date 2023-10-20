# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../../../../include)

set(tst_erase_if_14_SOURCES ../tst_erase_if.cpp)

add_executable(tst_erase_if_14 ${tst_erase_if_14_SOURCES})
target_link_libraries(tst_erase_if_14 PUBLIC Qt::Core Qt::Test)
