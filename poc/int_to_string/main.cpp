/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2026 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include "int_to_string.h"

#include <cassert>
#include <iostream>

constexpr bool compare_number(result_buffer_type &source, size_t num_characters, const std::string& expected)
{
    for (size_t i = 0; i < num_characters; i++)
    {
        if (source[i] != expected[i])
            return false;
    }

    return true;
}

void test_conversion(auto integer_value, const std::string& expected)
{
    result_buffer_type target = {};
    auto characters_written = int_to_string(integer_value, target);
    assert(compare_number(target, characters_written, expected));
}

consteval bool test_conversion_ce(auto integer_value, const std::string& expected)
{
    result_buffer_type target = {};
    auto characters_written = int_to_string(integer_value, target);
    return compare_number(target, characters_written, expected);
}

static_assert(test_conversion_ce(1024ULL, "1024"));
static_assert(test_conversion_ce(-1024, "-1024"));
static_assert(test_conversion_ce(-1, "-1"));
static_assert(test_conversion_ce(0, "0"));
static_assert(test_conversion_ce(9, "9"));
static_assert(test_conversion_ce(10, "10"));
static_assert(test_conversion_ce(99, "99"));
static_assert(test_conversion_ce(100, "100"));
static_assert(test_conversion_ce(101, "101"));
static_assert(test_conversion_ce(999, "999"));
static_assert(test_conversion_ce(1'000, "1000"));
static_assert(test_conversion_ce(-1, "-1"));
static_assert(test_conversion_ce(-100, "-100"));
static_assert(test_conversion_ce(static_cast<int8_t>(-1), "-1"));

// 8-bit
static_assert(test_conversion_ce(std::numeric_limits<std::int8_t>::min(), "-128"));
static_assert(test_conversion_ce(std::numeric_limits<std::int8_t>::max(), "127"));
static_assert(test_conversion_ce(std::numeric_limits<std::uint8_t>::min(), "0"));
static_assert(test_conversion_ce(std::numeric_limits<std::uint8_t>::max(), "255"));

// 16-bit
static_assert(test_conversion_ce(std::numeric_limits<std::uint16_t>::min(), "0"));
static_assert(test_conversion_ce(std::numeric_limits<std::uint16_t>::max(), "65535"));
static_assert(test_conversion_ce(std::numeric_limits<std::int16_t>::min(), "-32768"));
static_assert(test_conversion_ce(std::numeric_limits<std::int16_t>::max(), "32767"));

// 32-bit
static_assert(test_conversion_ce(std::numeric_limits<std::uint32_t>::min(), "0"));
static_assert(test_conversion_ce(std::numeric_limits<std::uint32_t>::max(), "4294967295"));
static_assert(test_conversion_ce(std::numeric_limits<std::int32_t>::min(), "-2147483648"));
static_assert(test_conversion_ce(std::numeric_limits<std::int32_t>::max(), "2147483647"));

// 64-bit
static_assert(test_conversion_ce(std::numeric_limits<std::int64_t>::min(), "-9223372036854775808"));
static_assert(test_conversion_ce(std::numeric_limits<std::int64_t>::max(), "9223372036854775807"));
static_assert(test_conversion_ce(std::numeric_limits<std::uint64_t>::min(), "0"));
static_assert(test_conversion_ce(std::numeric_limits<std::uint64_t>::max(), "18446744073709551615"));

int main()
{
    test_conversion(static_cast<uint64_t>(1024), "1024");
    test_conversion(static_cast<int16_t>(-1024), "-1024");
    test_conversion(static_cast<int16_t>(-1), "-1");
    test_conversion(static_cast<uint8_t>(0), "0");
    test_conversion(static_cast<uint8_t>(9), "9");
    test_conversion(static_cast<uint8_t>(10), "10");
    test_conversion(static_cast<uint8_t>(99), "99");
    test_conversion(static_cast<uint8_t>(100), "100");
    test_conversion(static_cast<uint8_t>(101), "101");
    test_conversion(static_cast<uint16_t>(999), "999");
    test_conversion(static_cast<uint16_t>(1000), "1000");
    test_conversion(static_cast<int16_t>(-1), "-1");
    test_conversion(static_cast<int16_t>(-100), "-100");
    test_conversion(static_cast<int8_t>(-1), "-1");

    // 8-bit
    test_conversion(std::numeric_limits<std::int8_t>::min(), "-128");
    test_conversion(std::numeric_limits<std::int8_t>::max(), "127");
    test_conversion(std::numeric_limits<std::uint8_t>::min(), "0");
    test_conversion(std::numeric_limits<std::uint8_t>::max(), "255");

    // 16-bit
    test_conversion(std::numeric_limits<std::uint16_t>::min(), "0");
    test_conversion(std::numeric_limits<std::uint16_t>::max(), "65535");
    test_conversion(std::numeric_limits<std::int16_t>::min(), "-32768");
    test_conversion(std::numeric_limits<std::int16_t>::max(), "32767");

    // 32-bit
    test_conversion(std::numeric_limits<std::uint32_t>::min(), "0");
    test_conversion(std::numeric_limits<std::uint32_t>::max(), "4294967295");
    test_conversion(std::numeric_limits<std::int32_t>::min(), "-2147483648");
    test_conversion(std::numeric_limits<std::int32_t>::max(), "2147483647");

    // 64-bit
    test_conversion(std::numeric_limits<std::int64_t>::min(), "-9223372036854775808");
    test_conversion(std::numeric_limits<std::int64_t>::max(), "9223372036854775807");
    test_conversion(std::numeric_limits<std::uint64_t>::min(), "0");
    test_conversion(std::numeric_limits<std::uint64_t>::max(), "18446744073709551615");

    // benchmark();
    return 0;
}
