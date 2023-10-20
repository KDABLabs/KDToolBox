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
    Test
    Gui
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(tst_qt_hasher_SOURCES ../qt_hasher.h tst_qt_hasher.cpp tst_qt_hasher.h)

add_executable(tst_qt_hasher ${tst_qt_hasher_SOURCES})
target_link_libraries(tst_qt_hasher PUBLIC Qt::Core Qt::Test Qt::Gui)
