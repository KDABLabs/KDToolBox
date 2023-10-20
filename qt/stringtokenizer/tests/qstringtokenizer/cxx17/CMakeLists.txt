# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
find_package(Qt${QT_VERSION_MAJOR} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../../../include)

set(tst_qstringtokenizer_cxx17_SOURCES ../tst_qstringtokenizer.cpp)

add_executable(tst_qstringtokenizer_cxx17 ${tst_qstringtokenizer_cxx17_SOURCES})
target_link_libraries(tst_qstringtokenizer_cxx17 PUBLIC Qt::Core Qt::Test)
