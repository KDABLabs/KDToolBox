/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include <iostream>

namespace
{
// store a string in a more formal manner
template <size_t N>
struct fixed_string
{
    char data[N]{};

    consteval fixed_string() = default;

    consteval fixed_string(const char (&str)[N])
    {
        for (auto i = 0; i < N; ++i)
        {
            data[i] = str[i];
        }
    }
};

// rule for how to deduce strings into fixed_string
// "abc" will be deduced into fixed_string<4> (including null terminator)
template <size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

// the count needs to be performed separately
// thus this is a separate function
template <fixed_string S>
consteval size_t count_replaced_length()
{
    constexpr size_t source_length = sizeof(S.data);
    size_t target_string_character_count = 0;
    for (size_t source_index = 0; source_index + 1 < source_length; ++source_index)
    {
        const auto& source_character = S.data[source_index];

        if (source_character == '\0')
        {
            // reached end of string
            break;
        }

        if (source_character == '%' && S.data[source_index + 1] == 's')
        {
            // skip over '%'
            ++source_index;
        }

        ++target_string_character_count;
    }

    // add null terminator
    ++target_string_character_count;

    return target_string_character_count;
}

// perform the actual replacement in the string
template <fixed_string S>
consteval auto compute_replaced()
{
    constexpr size_t source_length = sizeof(S.data);
    constexpr size_t target_length = count_replaced_length<S>();
    fixed_string<target_length> result{};
    size_t target_index = 0;
    for (size_t source_index = 0; source_index < source_length; source_index++)
    {
        if (S.data[source_index] == '\0')
            break;

        // look for %s in the source string
        if (source_index + 1 < source_length && S.data[source_index] == '%' && S.data[source_index + 1] == 's')
        {
            // write the replacement character
            result.data[target_index] = 0x01;

            // skip over the '%' character
            source_index += 1;
        }
        else
        {
            result.data[target_index] = S.data[source_index];
        }

        ++target_index;
    }

    // add null terminator
    result.data[target_index] = '\0';
    return result;
}

// user-facing function to replace %s with 0x01
template <fixed_string S>
consteval const char* replace_percent_s()
{
    static constexpr auto value = compute_replaced<S>();
    return value.data;
}

// User-defined literal option: "..."_replace
template <fixed_string S>
consteval const char* operator""_replace()
{
    static constexpr auto value = compute_replaced<S>();
    return value.data;
}

// these strings won't get stored in the binary
static_assert(replace_percent_s<"%s">()[0] == 0x01);
static_assert(replace_percent_s<"Hello %s world">()[6] == 0x01);
static_assert(replace_percent_s<"100%">()[3] == '%');
static_assert(replace_percent_s<"">()[0] == '\0');
}

namespace some_namespace
{
    // User-defined literal option: "..."_rep
    template <fixed_string S>
    consteval const char* operator""_rep()
    {
        static constexpr auto value = compute_replaced<S>();
        return value.data;
    }

    static void print()
    {
        std::cout << "3: Hey, welcome to %s "_rep << "!\n";
    }
}

int main()
{
    // example 1 - call the replace_percent_s method directly
    std::cout << replace_percent_s<"1: Hello and welcome to %s ">() << "!\n";

    // example 2 - use the suffix operator _replace (nicer)
    std::cout << "2: Hello and welcome to %s "_replace << "!\n";

    // example 3 - use inside namespace
    some_namespace::print();

    // example 4 - use the namespace specific suffix operator
    {
        using namespace some_namespace;
        std::cout << "4: Hello and welcome to %s "_rep << "!\n";
    }

    return 0;
}
