# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
add_subdirectory(eventfilter)
add_subdirectory(KDSignalThrottler)
add_subdirectory(KDSqlDatabaseTransaction)
add_subdirectory(KDStlContainerAdaptor)
add_subdirectory(messagehandler)
add_subdirectory(model_view)
add_subdirectory(notify_guard)
add_subdirectory(pointer_cast)
add_subdirectory(qml)
add_subdirectory(qt_hasher)
add_subdirectory(singleshot_connect)
add_subdirectory(tabWindow)
add_subdirectory(ui_watchdog)
add_subdirectory(qt6_natvis)
add_subdirectory(qt_fmt)

if(LINUX)
    add_subdirectory(asan_assert_fail_qt)
endif()

if(QT_VERSION_MAJOR EQUAL 5)
    # Qt6 has it's own QStringTokenizer
    add_subdirectory(stringtokenizer)
endif()
