# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
project(windows_waitable_timer CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(windows_waitable_timer windows_waitable_timer.cpp)
