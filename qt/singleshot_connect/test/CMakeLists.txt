# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(tst_singleshot_connect_SOURCES ../singleshot_connect.h tst_singleshot_connect.cpp)

add_executable(tst_singleshot_connect ${tst_singleshot_connect_SOURCES})
target_link_libraries(tst_singleshot_connect PUBLIC Qt::Core Qt::Test)
