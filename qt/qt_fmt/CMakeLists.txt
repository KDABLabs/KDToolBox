# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
include(CheckSubmoduleExists)

check_submodule_exists(fmt fmt)

add_subdirectory(testfmt)

if(KDTOOLBOX_CXX20)
    add_subdirectory(teststd)
endif()
