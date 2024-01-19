/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_PROPAGATE_CONST
#define KDTOOLBOX_PROPAGATE_CONST

#include <functional>
#include <type_traits>
#include <utility>

// Our SFINAE is too noisy and clang-format gets confused.
// clang-format off

namespace KDToolBox
{

template <typename T>
class propagate_const;

namespace detail {
// Type traits to detect propagate_const specializations
template <typename T>
struct is_propagate_const : std::false_type
{
};

template <typename T>
struct is_propagate_const<propagate_const<T>> : std::true_type
{
};

// Using SFINAE in a base class to constrain the conversion operators.
// Otherwise, if we make them function templates and apply SFINAE directly on
// them, we would not be able to do certain conversions (cf.
// [over.ics.user/3]).
template <typename T>
using propagate_const_element_type = std::remove_reference_t<decltype(*std::declval<T &>())>;

// Const conversion.
// NON-STANDARD: checks that `const T` is convertible, not `T`.
// See https://wg21.link/lwg3812
template <typename T,
          bool = std::disjunction_v<
              std::is_pointer<T>,
              std::is_convertible<const T, const propagate_const_element_type<T> *>
              >
          >
struct propagate_const_const_conversion_operator_base
{
};

template <typename T>
struct propagate_const_const_conversion_operator_base<T, true>
{
    constexpr operator const propagate_const_element_type<T> *() const;
};

// Non-const conversion
template <typename T,
          bool = std::disjunction_v<
              std::is_pointer<T>,
              std::is_convertible<T, propagate_const_element_type<T> *>
              >
          >
struct propagate_const_non_const_conversion_operator_base
{
};

template <typename T>
struct propagate_const_non_const_conversion_operator_base<T, true>
{
    constexpr operator propagate_const_element_type<T> *();
};

template <typename T>
struct propagate_const_base
        : propagate_const_const_conversion_operator_base<T>,
          propagate_const_non_const_conversion_operator_base<T>
{};

} // namespace detail

/*
    TODO: This code could benefit from a C++20 overhaul:
    * concepts
    * three-way comparisons
    * explicit(bool)

    However we can't depend on C++20 yet...
*/

template <typename T>
class propagate_const : public detail::propagate_const_base<T>
{
public:
    using element_type = detail::propagate_const_element_type<T>;

    // Special member functions
    propagate_const() = default;
    propagate_const(propagate_const &&) = default;
    propagate_const &operator=(propagate_const &&) = default;
    propagate_const(const propagate_const &) = delete;
    propagate_const &operator=(const propagate_const &) = delete;
    ~propagate_const() = default;

    // Constructor from values

    template <
        typename U,
        std::enable_if_t< // This constructor is enabled if:
            std::conjunction_v<
                std::is_constructible<T, U>, // 1) we can build a T from a U,
                std::negation<detail::is_propagate_const<std::decay_t<U>>>, // 2) we are not making a
                                                                          // converting constructor,
                std::is_convertible<U, T> // 3) and the conversion from U to T is implicit;
                >,
            bool> = true>
    /* implicit */ // then, this constructor is implicit too.
    propagate_const(U &&other)
        : m_t(std::forward<U>(other))
    {
    }

    template <
        typename U,
        std::enable_if_t< // This constructor is enabled if:
            std::conjunction_v<
                std::is_constructible<T, U>, // 1) we can build a T from a U,
                std::negation<detail::is_propagate_const<std::decay_t<U>>>, // 2) we are not making a
                                                                          // converting constructor,
                std::negation<std::is_convertible<U, T>> // 3) and the conversion from U to T is
                                                         // explicit;
                >,
            bool> = true>
    explicit // then, this constructor is explicit.
        propagate_const(U &&other)
        : m_t(std::forward<U>(other))
    {
    }

    // Constructors from other propagate_const (converting constructors)
    template <typename U,
              std::enable_if_t< // This constructor is enabled if:
                  std::conjunction_v<std::is_constructible<T, U>, // 1) we can do the conversion,
                                     std::is_convertible<U, T> // 2) and the conversion is implicit;
                                     >,
                  bool> = true>
    /* implicit */ // then, this constructor is implicit.
    constexpr propagate_const(propagate_const<U> &&other)
        : m_t(std::move(get_underlying(other)))
    {
    }

    template <typename U,
              std::enable_if_t< // This constructor is enabled if:
                  std::conjunction_v<std::is_constructible<T, U>, // 1) we can do the conversion,
                                     std::negation<std::is_convertible<U, T>> // 2) and the
                                                                              // conversion is
                                                                              // explicit;
                                     >,
                  bool> = true>
    explicit // then, this constructor is explicit.
        constexpr propagate_const(propagate_const<U> &&other)
        : m_t(std::move(get_underlying(other)))
    {
    }

    // Converting assignment
    template <typename U,
              std::enable_if_t< // This assignment operator is enabled if
                  std::is_convertible_v<U, T>, // the conversion from U to T is implicit
                  bool> = true>
    constexpr propagate_const &operator=(propagate_const<U> &&other)
    {
        m_t = std::move(get_underlying(other));
        return *this;
    }

    template <typename U,
              std::enable_if_t< // This assignment operator is enabled if:
                  std::conjunction_v<
                      std::is_convertible<U, T>, // 1) the conversion from U to T is implicit,
                      std::negation<detail::is_propagate_const<std::decay_t<U>>> // 2) and U is not a
                                                                               // propagate_const
                      >,
                  bool> = true>
    constexpr propagate_const &operator=(U &&other)
    {
        m_t = std::forward<U>(other);
        return *this;
    }

    // Swap
    constexpr void swap(propagate_const &other) noexcept(std::is_nothrow_swappable_v<T>)
    {
        using std::swap;
        swap(m_t, other.m_t);
    }

    // Const observers
    constexpr explicit operator bool() const { return static_cast<bool>(m_t); }

    constexpr const element_type *get() const { return get_impl(m_t); }

    constexpr const element_type &operator*() const { return *get(); }

    constexpr const element_type *operator->() const { return get(); }

    // Non-const observers
    constexpr element_type *get() { return get_impl(m_t); }

    constexpr element_type &operator*() { return *get(); }

    constexpr element_type *operator->() { return get(); }

    // Non-member utilities: extract the contained object
    template <typename U>
    friend constexpr auto &get_underlying(propagate_const<U> &p);

    template <typename U>
    friend constexpr const auto &get_underlying(const propagate_const<U> &p);

private:
    // Implementation of get() that works with raw pointers and smart
    // pointers. Similar to std::to_address, but to_address is C++20,
    // and propagate_const spec does not match it.
    template <typename U>
    static constexpr element_type *get_impl(U *u)
    {
        return u;
    }

    template <typename U>
    static constexpr element_type *get_impl(U &u)
    {
        return u.get();
    }

    template <typename U>
    static constexpr const element_type *get_impl(const U *u)
    {
        return u;
    }

    template <typename U>
    static constexpr const element_type *get_impl(const U &u)
    {
        return u.get();
    }

    T m_t;
};

// Swap
template <typename T,
          std::enable_if_t<std::is_swappable_v<T>, bool> = true>
constexpr void swap(propagate_const<T> &lhs, propagate_const<T> &rhs)
    noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

// Implementation of get_underlying
template <typename T>
constexpr auto &get_underlying(propagate_const<T> &p)
{
    return p.m_t;
}

template <typename T>
constexpr const auto &get_underlying(const propagate_const<T> &p)
{
    return p.m_t;
}

// Implementation of the conversion operators
template <typename T>
constexpr detail::propagate_const_const_conversion_operator_base<T, true>
    ::operator const detail::propagate_const_element_type<T> *() const
{
    return static_cast<const propagate_const<T> *>(this)->get();
}

template <typename T>
constexpr detail::propagate_const_non_const_conversion_operator_base<T, true>
    ::operator detail::propagate_const_element_type<T> *()
{
    return static_cast<propagate_const<T> *>(this)->get();
}

// Comparisons. As per spec, they're free function templates.

// Comparisons against nullptr.
template <typename T>
constexpr bool operator==(const propagate_const<T> &p, std::nullptr_t)
{
    return get_underlying(p) == nullptr;
}

template <typename T>
constexpr bool operator!=(const propagate_const<T> &p, std::nullptr_t)
{
    return get_underlying(p) != nullptr;
}

template <typename T>
constexpr bool operator==(std::nullptr_t, const propagate_const<T> &p)
{
    return nullptr == get_underlying(p);
}

template <typename T>
constexpr bool operator!=(std::nullptr_t, const propagate_const<T> &p)
{
    return nullptr == get_underlying(p);
}

// Comparisons between propagate_const
#define DEFINE_PROPAGATE_CONST_COMPARISON_OP(op)                                              \
    template <typename T, typename U>                                                         \
    constexpr bool operator op (const propagate_const<T> &lhs, const propagate_const<U> &rhs) \
    {                                                                                         \
        return get_underlying(lhs) op get_underlying(rhs);                                    \
    }                                                                                         \

DEFINE_PROPAGATE_CONST_COMPARISON_OP(==)
DEFINE_PROPAGATE_CONST_COMPARISON_OP(!=)
DEFINE_PROPAGATE_CONST_COMPARISON_OP(<)
DEFINE_PROPAGATE_CONST_COMPARISON_OP(<=)
DEFINE_PROPAGATE_CONST_COMPARISON_OP(>)
DEFINE_PROPAGATE_CONST_COMPARISON_OP(>=)

#undef DEFINE_PROPAGATE_CONST_COMPARISON_OP

// Comparisons against other (smart) pointers
#define DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP(op)                       \
    template <typename T, typename U>                                        \
    constexpr bool operator op (const propagate_const<T> &lhs, const U &rhs) \
    {                                                                        \
        return get_underlying(lhs) op rhs;                                   \
    }                                                                        \
    template <typename T, typename U>                                        \
    constexpr bool operator op (const T &lhs, const propagate_const<U> &rhs) \
    {                                                                        \
        return lhs op get_underlying(rhs);                                   \
    }                                                                        \

DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP(==)
DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP(!=)
DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP(<)
DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP(<=)
DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP(>)
DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP(>=)

#undef DEFINE_PROPAGATE_CONST_MIXED_COMPARISON_OP

} // namespace KDToolBox

// std::hash specialization
namespace std
{
template <typename T>
struct hash<KDToolBox::propagate_const<T>>
{
    constexpr size_t operator()(const KDToolBox::propagate_const<T> &t) const
        noexcept(noexcept(hash<T>{}(get_underlying(t))))
    {
        return hash<T>{}(get_underlying(t));
    }
};

#define DEFINE_COMP_OBJECT_SPECIALIZATION_FOR_PROPAGATE_CONST(COMP)     \
    template <typename T>                                               \
    struct COMP<KDToolBox::propagate_const<T>>                          \
    {                                                                   \
    constexpr bool operator()(const KDToolBox::propagate_const<T> &lhs, \
                              const KDToolBox::propagate_const<T> &rhs) \
    {                                                                   \
        return COMP<T>{}(get_underlying(lhs), get_underlying(rhs));     \
    }                                                                   \
    };                                                                  \

DEFINE_COMP_OBJECT_SPECIALIZATION_FOR_PROPAGATE_CONST(equal_to)
DEFINE_COMP_OBJECT_SPECIALIZATION_FOR_PROPAGATE_CONST(not_equal_to)
DEFINE_COMP_OBJECT_SPECIALIZATION_FOR_PROPAGATE_CONST(less)
DEFINE_COMP_OBJECT_SPECIALIZATION_FOR_PROPAGATE_CONST(greater)
DEFINE_COMP_OBJECT_SPECIALIZATION_FOR_PROPAGATE_CONST(less_equal)
DEFINE_COMP_OBJECT_SPECIALIZATION_FOR_PROPAGATE_CONST(greater_equal)

#undef DEFINE_COMP_OBJECT_SPECIALIZATION

} // namespace std

#endif // KDTOOLBOX_PROPAGATE_CONST
