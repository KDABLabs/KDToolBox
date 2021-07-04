/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020-2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Milian Wolff <milian.wolff@kdab.com>
**
** This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, ** and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice (including the next paragraph)
** shall be included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF ** CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
****************************************************************************/

#include <sanitizer/asan_interface.h>

#include <stdio.h>
#include <stdlib.h>

extern const char *__progname;

void __assert_fail(const char *assertion, const char *file,
                   unsigned int line, const char *function)
{
    int stack_var = 0;
    fprintf(stderr, "%s: %s:%u: %s: Assertion `%s' failed.\n",
            __progname, file, line, function, assertion);
    __asan_report_error(__builtin_frame_address(0), &stack_var, &stack_var, NULL, 0, 1);
    abort();
}
