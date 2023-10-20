# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
set(asan_assert_fail_SOURCES asan_assert_fail.c)

add_library(asan_assert_fail SHARED ${asan_assert_fail_SOURCES})
