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
    Qml
    Quick
)

set(example_tmp_4_SOURCES ../QmlStackTraceHelper.cpp main.cpp)

set(example_tmp_4_QRCS qrc.qrc)
qt_add_resources(example_tmp_4_SOURCES ${example_tmp_4_QRCS})

add_executable(example_tmp_4 ${example_tmp_4_SOURCES})
target_link_libraries(example_tmp_4 PUBLIC Qt::Core Qt::Gui Qt::QmlPrivate Qt::Quick)
