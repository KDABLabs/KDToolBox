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

if (Qt6Core_VERSION VERSION_GREATER_EQUAL "6.10.0")
    find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED CorePrivate)
endif()

add_definitions(-DQT_DEPRECATED_WARNINGS)
set(tst_sortproxymodeltest_SOURCES ../sortproxymodel.cpp ../sortproxymodel.h tst_sortproxymodeltest.cpp vectormodel.h)

add_executable(tst_sortproxymodeltest ${tst_sortproxymodeltest_SOURCES})
target_link_libraries(tst_sortproxymodeltest PUBLIC Qt::Core Qt::Gui Qt::Test Qt::CorePrivate)
