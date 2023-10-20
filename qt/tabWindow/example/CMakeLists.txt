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
    Widgets
)

set(tabwindow_SOURCES ../src/tabwindow.cpp ../src/tabwindow.h main.cpp)

add_executable(tabwindow ${tabwindow_SOURCES})
target_link_libraries(tabwindow PUBLIC Qt::Core Qt::Gui Qt::Widgets)
