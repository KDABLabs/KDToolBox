# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
set(asan_assert_fail_qt_SOURCES asan_assert_fail_qt.cpp)

add_library(asan_assert_fail_qt SHARED ${asan_assert_fail_qt_SOURCES})
target_link_libraries(asan_assert_fail_qt PUBLIC Qt::Core)
