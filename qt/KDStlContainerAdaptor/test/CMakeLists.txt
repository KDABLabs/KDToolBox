# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(tst_KDStlContainerAdaptor_SOURCES tst_KDStlContainerAdaptor.cpp)

add_executable(tst_KDStlContainerAdaptor ${tst_KDStlContainerAdaptor_SOURCES})
target_link_libraries(tst_KDStlContainerAdaptor PUBLIC Qt::Core Qt::Test)
