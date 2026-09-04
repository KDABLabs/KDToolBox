/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include <cassert>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

// This is inspired by
// C++ Weekly Episode 421 on Youtube
// by Jason Turner
// https://www.youtube.com/watch?v=0yJk5yfdih0

namespace KDToolBox
{
/**
 * std::optional replacement which enforces return value optimization and does away with exceptions.
 *
 * Normally with std::optional, there are several unnecessary copies made.
 * Really the only way to avoid that is to use the emplace() method.
 * But this means you can still make the mistake.
 *
 * This implementation solves it by omitting assignment/copy constructors/operators.
 * It thus forces the compiler to use the move constructor.
 * Note that this is an optimization in the compiler and might not be supported everywhere.
 * So if you are about to make the mistake that would lead to a copy -
 * your code simply should not compile.
 *
 * Additionally, instead of throwing exceptions, uses debug build asserts when
 * accessing the value in a bad way.
 * I.e.
 * KDToolBox::optional<uint64_t> opt;
 * auto v = opt.value(); <-- this will fail and cause the assert() but only in debug builds.
 *
 * So be careful, there is possible speed gains to be had but your code needs full test coverage.
 *
 * Bonus: Specialization for bool below which uses 1 byte only.
 *
 * @tparam T any type to store in the optional
 */
template<typename T>
class optional
{
public:
    static_assert(!std::is_reference_v<T>, "KDToolBox::optional cannot store reference types");
    static_assert(!std::is_array_v<T>, "KDToolBox::optional cannot store array types");
    ~optional() noexcept { reset(); }

    optional() noexcept
        : m_dummy()
    {
    }

    template<class... Types>
        requires(sizeof...(Types) > 0 &&
                 !(sizeof...(Types) == 1 && (std::is_same_v<std::remove_cvref_t<Types>, optional> || ...)))
    explicit optional(Types &&...v) noexcept(std::is_nothrow_constructible_v<T, Types...>)
        : m_value(std::forward<Types>(v)...)
        , m_has_value(true)
    {
    }

    [[nodiscard]] bool has_value() const noexcept { return m_has_value; }

    [[nodiscard]] explicit operator bool() const noexcept { return m_has_value; }
    [[nodiscard]] const T &operator*() const noexcept { return value(); }
    [[nodiscard]] const T *operator->() const noexcept { return &value(); }

    [[nodiscard]] const T &value() const noexcept
    {
        // std::optional will instead throw exception here
        // use an assert instead
        assert(m_has_value);
        return m_value;
    }

    void reset() noexcept
    {
        if (m_has_value)
        {
            m_value.~T();
        }

        m_has_value = false;
    }

    /**
     * Compatible with std::optional's emplace method.
     */
    template<class... Types>
    T &emplace(Types &&...v) noexcept(std::is_nothrow_constructible_v<T, Types...>)
    {
        // destroy the previous value
        reset();

        // construct a new value in-place
        new (&m_value) T(std::forward<Types>(v)...);
        m_has_value = true;
        return m_value;
    }

    optional(const optional &) = delete;
    optional &operator=(const optional &) = delete;

    // This is the method we don't want to have.
    // It allows the code in get_opt_bad_1 and get_opt_bad_2
    /*
    optional &operator=(optional &&other) noexcept
    {
        if (this != &other)
        {
            reset();
            if (other.m_has_value)
            {
                new (&m_value) T(std::move(other.m_value));
                m_has_value = true;
                other.reset();
            }
        }
        return *this;
    }
    */

    optional(optional &&other) noexcept
    {
        if (other.m_has_value)
        {
            new (&m_value) T(std::move(other.m_value));
            m_has_value = true;
            other.reset();
        }
        else
        {
            m_dummy = dummy_type{};
        }
    }

private:
    struct dummy_type
    {
        // This default constructor is user-provided to avoid zero-initialization when objects are value-initialized.
        constexpr dummy_type() noexcept = default;
    };

    union
    {
        dummy_type m_dummy;
        std::remove_cv_t<T> m_value;
    };
    bool m_has_value = false;
};

// specialization for bool which only uses one byte
template<>
class optional<bool>
{
public:
    optional() noexcept = default;

    explicit optional(bool v) noexcept
        : m_has_value(true)
        , m_value(v)
    {
    }

    [[nodiscard]] bool has_value() const noexcept { return m_has_value; }

    [[nodiscard]] bool value() const noexcept { return m_value; }

    [[nodiscard]] explicit operator bool() const noexcept { return m_has_value; }

    optional(const optional &) = delete;
    optional &operator=(const optional &) = delete;

    void reset() noexcept
    {
        m_has_value = false;
        m_value = false;
    }

    optional(optional &&other) noexcept
    {
        m_value = other.m_value;
        m_has_value = true;
        other.reset();
    }

    /**
     * Differs from the base emplace in that it
     * does not return a reference.
     * One can not return references to bit fields.
     */
    void emplace(bool v) noexcept
    {
        m_value = v;
        m_has_value = true;
    }

private:
    bool m_has_value : 7 = false;
    bool m_value : 1 = {};
};
}

// test / example code below ---------------------

#include <iostream>
#include <optional>
#include <source_location>

struct pod_struct
{
    uint8_t a;
    uint8_t b;
};

void print(const std::source_location &location = std::source_location::current()) noexcept
{
    std::puts(location.function_name());
}

// Jason Turner's Lifetime class to print behavior with some additions
struct Lifetime
{
    explicit Lifetime(uint8_t a) noexcept
    {
        m_value = a;
        print();
    }
    Lifetime() noexcept { print(); }
    Lifetime(Lifetime &&) noexcept { print(); }
    Lifetime(const Lifetime &) noexcept { print(); }
    ~Lifetime() noexcept { print(); }
    Lifetime &operator=(const Lifetime &) noexcept
    {
        print();
        return *this;
    }
    Lifetime &operator=(Lifetime &&) noexcept
    {
        print();
        return *this;
    }

    uint8_t m_value = 12;
};

std::optional<Lifetime> get_std_optional_bad_1()
{
    std::optional<Lifetime> opt;
    opt = Lifetime{42};
    return opt;
}

std::optional<Lifetime> get_std_optional_bad_2()
{
    return Lifetime{42};
}

std::optional<Lifetime> get_std_optional_good()
{
    std::optional<Lifetime> opt;
    opt.emplace(42);
    return opt;
}

KDToolBox::optional<Lifetime> get_opt_good_1()
{
    KDToolBox::optional<Lifetime> opt;
    opt.emplace(42);
    return opt;
}

KDToolBox::optional<Lifetime> get_opt_good_2()
{
    KDToolBox::optional<Lifetime> opt(42);
    return opt;
}

KDToolBox::optional<Lifetime> get_opt_good_3()
{
    return KDToolBox::optional<Lifetime>(42);
}

/*
This along with the optional &operator=(optional &&other)
results in the following (extra move and extra destructor):

__cdecl Lifetime::Lifetime(unsigned char) noexcept
__cdecl Lifetime::Lifetime(struct Lifetime &&) noexcept
__cdecl Lifetime::~Lifetime(void) noexcept
__cdecl Lifetime::~Lifetime(void) noexcept
*/
// KDToolBox::optional<Lifetime> get_opt_bad_1()
// {
// KDToolBox::optional<Lifetime> opt;
// opt = {42};
// return opt;
// }

/*
This along with the optional &operator=(optional &&other)
results in the following (some extra move and extra destructor calls):

__cdecl Lifetime::Lifetime(unsigned char) noexcept
__cdecl Lifetime::Lifetime(struct Lifetime &&) noexcept
__cdecl Lifetime::Lifetime(struct Lifetime &&) noexcept
__cdecl Lifetime::~Lifetime(void) noexcept
__cdecl Lifetime::~Lifetime(void) noexcept
__cdecl Lifetime::~Lifetime(void) noexcept
*/
// KDToolBox::optional<Lifetime> get_opt_bad_2()
// {
// KDToolBox::optional<Lifetime> opt;
// opt = Lifetime(42);
// return opt;
// }

void extra_move_and_destructor_examples()
{
    // {
    //     auto ret = get_opt_bad_1();
    // }

    // {
    //     auto ret = get_opt_bad_2();
    // }
}

static KDToolBox::optional<pod_struct> get_some_struct()
{
    KDToolBox::optional<pod_struct> result;
    result.emplace(pod_struct{42, 43});
    return result;
}

struct eight_optional_bools
{
    KDToolBox::optional<bool> m_bools[8];
};

static_assert(sizeof(eight_optional_bools) == sizeof(bool) * 8);

KDToolBox::optional<bool> get_good_bool_1()
{
    KDToolBox::optional<bool> result;
    result.emplace(true);
    return result;
}

KDToolBox::optional<bool> get_good_bool_2()
{
    return KDToolBox::optional<bool>(true);
}

KDToolBox::optional<uint8_t> get_good_uint8_t()
{
    return KDToolBox::optional<uint8_t>(45);
}

int main()
{
    extra_move_and_destructor_examples();

    // Bad output looks like:
    // __cdecl Lifetime::Lifetime(unsigned char) noexcept
    // __cdecl Lifetime::Lifetime(struct Lifetime &&) noexcept
    // __cdecl Lifetime::~Lifetime(void) noexcept
    // __cdecl Lifetime::~Lifetime(void) noexcept

    // baseline std::optional - bad examples:
    printf("BEGIN: get_std_optional_bad_1():\n");
    {
        auto ret = get_std_optional_bad_1();
        assert(ret.has_value());

        // this is 12 and not 42 because original value not copied in Lifetime class
        assert(ret.value().m_value == 12);
    }
    printf("END: get_std_optional_bad_1()\n\n");

    printf("BEGIN: get_std_optional_bad_2():\n");
    {
        auto ret = get_std_optional_bad_2();
        assert(ret.has_value());

        // this is 12 and not 42 because original value not copied in Lifetime class
        assert(ret.value().m_value == 12);
    }
    printf("END: get_stdopt_bad_2()\n\n");

    // Good output looks like:
    //   Lifetime::Lifetime(int) noexcept
    //   Lifetime::~Lifetime(void) noexcept

    // baseline std::optional - good (uses emplace internally)
    printf("BEGIN: get_std_optional_good():\n");
    {
        auto ret = get_std_optional_good();
        assert(ret.has_value());
        assert(ret.value().m_value == 42);
    }
    printf("END: get_std_optional_good()\n\n");

    // RVO-enforced optional
    // validate return value optimization
    printf("BEGIN: get_opt_good_1():\n");
    {
        auto ret = get_opt_good_1();
        assert(ret.has_value());
        assert(ret.value().m_value == 42);
    }
    printf("END: get_opt_good_1()\n\n");

    printf("BEGIN: get_opt_good_2():\n");
    {
        auto ret = get_opt_good_2();
        assert(ret.has_value());
        assert(ret.value().m_value == 42);
    }
    printf("END: get_opt_good_2()\n\n");

    printf("BEGIN: get_opt_good_3():\n");
    {
        auto ret = get_opt_good_3();
        assert(ret.has_value());
        assert(ret.value().m_value == 42);
    }
    printf("END: get_opt_good_3()\n\n");

    {
        auto ret = get_good_bool_1();
        assert(ret.has_value());
        assert(ret.value() == true);
    }

    {
        auto ret = get_good_bool_2();
        assert(ret.has_value());
        assert(ret.value() == true);
    }

    {
        auto ret = get_good_uint8_t();
        assert(ret.has_value());
        assert(ret.value() == 45);
    }

    // Bad value access
    // This code will not throw an exception but will
    // call assert internally in a Debug build.
    {
        // KDToolBox::optional<Lifetime> test;
        // auto bad_value = test.value();
        // if this is a release build, the default constructor
        // of the Lifetime struct will be called
        // assert(bad_value.m_value == 12);
    }

    // This code will throw an exception
    // {
    //     std::optional<Lifetime> test;
    //     auto bad_value = test.value();
    //     assert(bad_value.m_value == 1);
    // }

    // regular optional - 2 bytes
    {
        KDToolBox::optional<uint8_t> opt;
        assert(sizeof(opt) == 2);
        opt.emplace(42);
        assert(opt.has_value());
        assert(opt.value() == 42);
    }

    // bool specialization, this is smaller
    {
        KDToolBox::optional<bool> opt;
        assert(sizeof(opt) == 1);
        assert(!opt.has_value());
        opt.emplace(true);
        assert(opt.has_value());
        assert(opt.value() == true);
        bool bool_value = false;
        opt.emplace(bool_value);
    }

    // test with plain-old-data struct
    {
        auto ret = get_some_struct();
        assert(ret.has_value());
        assert(ret.value().a == 42);
        assert(ret.value().b == 43);
    }
}
