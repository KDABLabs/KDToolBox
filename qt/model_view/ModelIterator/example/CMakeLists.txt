# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Gui)

if(KDTOOLBOX_CXX20)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(example_tmp_3_SOURCES ../src/ModelIterator.cpp ../src/ModelIterator.h main.cpp)

add_executable(example_tmp_3 ${example_tmp_3_SOURCES})
target_link_libraries(example_tmp_3 PUBLIC Qt::Core Qt::Gui)
