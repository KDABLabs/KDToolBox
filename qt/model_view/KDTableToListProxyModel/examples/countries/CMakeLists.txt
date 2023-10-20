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
    Quick
)

include_directories(../../src/)
set(countries_SOURCES ../../src/KDTableToListProxyModel.cpp ../../src/KDTableToListProxyModel.h main.cpp)

set(countries_QRCS countries.qrc)
qt_add_resources(countries_SOURCES ${countries_QRCS})

add_executable(countries ${countries_SOURCES})
target_link_libraries(countries PUBLIC Qt::Core Qt::Gui Qt::Quick)
