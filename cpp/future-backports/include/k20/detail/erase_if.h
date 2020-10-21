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

#include <algorithm>

namespace k20 {
namespace detail {

    template <typename SequenceContainer, typename Value>
    typename SequenceContainer::size_type
    seq_erase(SequenceContainer& c, const Value& value)
    {
        using std::remove;
        using std::distance;
        auto it = remove(c.begin(), c.end(), value);
        auto r = distance(it, c.end());
        c.erase(it, c.end());
        return r;
    }

    template <typename SequenceContainer, typename Predicate>
    typename SequenceContainer::size_type
    seq_erase_if(SequenceContainer& c, Predicate pred)
    {
        using std::remove;
        using std::distance;
        auto it = remove_if(c.begin(), c.end(), pred);
        auto r = distance(it, c.end());
        c.erase(it, c.end());
        return r;
    }

    template <typename NodeContainer, typename Predicate>
    typename NodeContainer::size_type
    node_erase_if(NodeContainer& c, Predicate pred)
    {
        auto original_size = c.size();
        for (auto i = c.begin(), last = c.end(); i != last; ) {
          if (pred(*i)) {
            i = c.erase(i);
          } else {
            ++i;
          }
        }
        return original_size - c.size();
    }

    template <typename ListContainer, typename Predicate>
    typename ListContainer::size_type
    list_erase_if(ListContainer& c, Predicate pred)
    {
#ifdef __cpp_lib_list_remove_return_type // P0646 implemented
        return c.remove_if(pred);
#else
        typename ListContainer::size_type count = 0;
        c.remove_if([pred, &count](const typename ListContainer::value_type& e) {
            if (pred(e)) {
                // this counting works, because the standard guarantees exactly
                // one application of the predicate per element:
                // for list: https://eel.is/c++draft/list#ops-18
                // for forward_list: https://eel.is/c++draft/forwardlist#ops-16
                ++count;
                return true;
            } else {
                return false;
            }
        });
        return count;
#endif
    }

    template <typename ListContainer, typename Value>
    typename ListContainer::size_type
    list_erase(ListContainer& c, const Value& value)
    {
        return list_erase_if(c, [&](const typename ListContainer::value_type& e) {
            return e == value;
        });
    }

} // namespace detail
} // namespace k20
