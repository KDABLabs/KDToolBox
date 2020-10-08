/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Marc Mutz <marc.mutz@kdab.com>
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

#pragma once

#include <unordered_set>

#include <k20/detail/erase_if.h>

// like std::erase/_if
namespace k20 {
#if defined(__cpp_lib_erase_if) && _cpp_lib_erase_if >= 202002L // the version that returns size_type
    using std::erase_if;

    // std::experimental::erase_if isn't good enough: returns void
#else
    template <typename UnaryPredicate, typename...Args>
    typename std::unordered_set<Args...>::size_type
    erase_if(std::unordered_set<Args...>& c, UnaryPredicate pred)
    {
        return detail::node_erase_if(c, pred);
    }

    template <typename UnaryPredicate, typename...Args>
    typename std::unordered_multiset<Args...>::size_type
    erase_if(std::unordered_multiset<Args...>& c, UnaryPredicate pred)
    {
        return detail::node_erase_if(c, pred);
    }
#endif
} // namespace k20
