# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
add_subdirectory(cxx11)
add_subdirectory(cxx14)
add_subdirectory(cxx17)
if(KDTOOLBOX_CXX20)
    add_subdirectory(cxx20)
endif()
