/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Milian Wolff <milian.wolff@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <cstdlib>

#include <sanitizer/asan_interface.h>

#include <QDebug>

Q_DECL_EXPORT void qt_assert(const char *assertion, const char *file, int line) noexcept
{
    int stack_var = 0;
    qWarning("ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
    __asan_report_error(__builtin_frame_address(0), &stack_var, &stack_var, NULL, 0, 1);
    std::abort();
}

extern "C" {
#include "../../general/asan_assert_fail/asan_assert_fail.c"
}
