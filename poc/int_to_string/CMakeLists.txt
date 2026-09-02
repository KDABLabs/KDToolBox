# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2026 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
project(test_int_to_string LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(test_int_to_string main.cpp)
add_executable(benchmark_int_to_string benchmark.cpp)
