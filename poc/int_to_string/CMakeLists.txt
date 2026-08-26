# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
project(test_int_to_string LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(test_int_to_string main.cpp)
target_compile_options(test_int_to_string PRIVATE -O3)
