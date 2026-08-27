# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
project(test_rvo_optional LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(test_rvo_optional main.cpp)
target_compile_options(test_rvo_optional PRIVATE -O3)
