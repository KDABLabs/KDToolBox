# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(tst_pointer_cast_SOURCES tst_pointer_cast.cpp)

add_executable(tst_pointer_cast ${tst_pointer_cast_SOURCES})
target_link_libraries(tst_pointer_cast PUBLIC Qt::Core Qt::Test)
