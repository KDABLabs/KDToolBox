/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Milian Wolff <milian.wolff@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <sanitizer/asan_interface.h>

#include <stdio.h>
#include <stdlib.h>

extern const char *__progname;

void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function)
{
    int stack_var = 0;
    fprintf(stderr, "%s: %s:%u: %s: Assertion `%s' failed.\n", __progname, file, line, function, assertion);
    __asan_report_error(__builtin_frame_address(0), &stack_var, &stack_var, NULL, 0, 1);
    abort();
}
