/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marc Mutz <marc.mutz@kdab.com>

  SPDX-License-Identifier: MIT
*/

#pragma once

#include <deque>

#include <k20/detail/erase_if.h>

// like std::erase/_if
namespace k20
{
#if defined(__cpp_lib_erase_if) && _cpp_lib_erase_if >= 202002L // the version that returns size_type
using std::erase;
using std::erase_if;
#else
template<typename Value, typename... Args>
typename std::deque<Args...>::size_type erase(std::deque<Args...> &c, const Value &value)
{
    return detail::seq_erase(c, value);
}

template<typename UnaryPredicate, typename... Args>
typename std::deque<Args...>::size_type erase_if(std::deque<Args...> &c, UnaryPredicate pred)
{
    return detail::seq_erase_if(c, pred);
}
#endif
} // namespace k20
