# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
project(test_variadic_template_arguments LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(test_variadic_template_arguments main.cpp)
