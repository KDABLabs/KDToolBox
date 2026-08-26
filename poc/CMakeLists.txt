# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

add_subdirectory(compile_time_string_replacement)

if(WIN32)
    add_subdirectory(int_to_string)
endif()

add_subdirectory(variadic_template_arguments)
