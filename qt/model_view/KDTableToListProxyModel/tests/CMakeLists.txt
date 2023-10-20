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

include_directories(../src/)
set(tst_kdtabletolistproxymodel_SOURCES ../src/KDTableToListProxyModel.cpp ../src/KDTableToListProxyModel.h
                                        tst_kdtabletolistproxymodel.cpp
)

add_executable(tst_kdtabletolistproxymodel ${tst_kdtabletolistproxymodel_SOURCES})
target_link_libraries(tst_kdtabletolistproxymodel PUBLIC Qt::Core Qt::Gui Qt::Test)
