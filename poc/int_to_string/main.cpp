/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

// left-to-right integer-to-string conversion
//  - Extracts 2 digits (0–99) per iteration via a precomputed lookup table
//  - Determines digit count from a logarithm-derived table (log10 2^p)
//  - Uses at most: 1 divide + 1 multiply + 1 subtract per 2 digits

using result_buffer_type = std::array<char, 32>;

namespace detail
{

// precomputed powers of ten (indices 0..20 cover 10^0 .. 10^20) ──────────
static constexpr std::uint64_t pow10[21] = {1ULL,
                                            10ULL,
                                            100ULL,
                                            1000ULL,
                                            10000ULL,
                                            100000ULL,
                                            1000000ULL,
                                            10000000ULL,
                                            100000000ULL,
                                            1000000000ULL,
                                            10000000000ULL,
                                            100000000000ULL,
                                            1000000000000ULL,
                                            10000000000000ULL,
                                            100000000000000ULL,
                                            1000000000000000ULL,
                                            10000000000000000ULL,
                                            100000000000000000ULL,
                                            1000000000000000000ULL,
                                            10000000000000000000ULL,
                                            10000000000000000000ULL};

// logarithm-derived digit-count table
// for MSB position p (0-indexed), the number of decimal digits in 2^p
// is  floor(p · log₁₀ 2) + 1.
// this table lets us obtain L from the MSB in O(1) with at most one
// boundary comparison against the matching power of ten.
static constexpr std::uint8_t digit_count_lut[64] = {
    // p :  0  1  2  3  | 4  5  6  | 7  8  9  | 10 11 12 13
    1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4,
    // p : 14 15 16     | 17 18 19 | 20 21 22  | 23 24 25 26
    5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8,
    // p : 27 28 29     | 30 31 32 | 33 34 35 36 | 37 38
    9, 9, 9, 10, 10, 10, 11, 11, 11, 11, 12, 12,
    // p : 39 40 41 42  | 43 44 45 46 | 47 48 49
    13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15,
    // p : 50 51 52 53  | 54 55 56  | 57 58 59
    16, 16, 16, 16, 17, 17, 17, 18, 18, 18,
    // p : 60 61 62 63
    19, 19, 19, 19};

// two-digit → ASCII lookup (saves divide/modulo per digit)
// two_digit_lut[v*2]     → tens  digit character
// two_digit_lut[v*2 + 1] → ones  digit character
static constexpr char two_digit_lut[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', '1', '0', '1',
    '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2', '2',
    '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3',
    '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5',
    '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5',
    '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8',
    '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8',
    '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0', '9', '1',
    '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'};

// MSB position (0-indexed) via binary bit-scan – no floating point
constexpr std::uint8_t msb_pos(std::uint64_t n) noexcept
{
    std::uint8_t p = 0;
    if (n >= (1ULL << 32))
    {
        p += 32;
        n >>= 32;
    }
    if (n >= (1ULL << 16))
    {
        p += 16;
        n >>= 16;
    }
    if (n >= (1ULL << 8))
    {
        p += 8;
        n >>= 8;
    }
    if (n >= (1ULL << 4))
    {
        p += 4;
        n >>= 4;
    }
    if (n >= (1ULL << 2))
    {
        p += 2;
        n >>= 2;
    }
    if (n >= (1ULL << 1))
    {
        p += 1;
    }
    return p;
}

// exact decimal digit count from log-table + one boundary compare
constexpr std::uint8_t digit_count(std::uint64_t n) noexcept
{
    // fast path: 1-digit
    if (n < 10)
        return 1;

    std::uint8_t p = msb_pos(n);
    std::uint8_t L = digit_count_lut[p]; // floor(p·log₁₀2)+1

    // boundary correction (≤1 cmp)
    if (n >= pow10[L])
        ++L;

    return L;
}
}

template<typename T>
constexpr std::uint8_t int_to_string(T integer_value, result_buffer_type &target_buffer) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be an integer");
    static_assert(!std::is_same_v<T, bool>, "bool is not a supported integer type");
    static_assert(sizeof(T) <= 8, "T must be at most 64 bits wide");

    std::uint8_t target_index = 0;
    std::uint64_t number_to_encode;

    // sign handling (UB-safe negation for MIN)
    if constexpr (std::is_signed_v<T>)
    {
        if (integer_value < static_cast<T>(0))
        {
            target_buffer[0] = '-';
            target_index = 1;
            number_to_encode = static_cast<std::uint64_t>(-integer_value);
        }
        else
        {
            number_to_encode = static_cast<std::uint64_t>(integer_value);
        }
    }
    else
    {
        number_to_encode = static_cast<std::uint64_t>(integer_value);
    }

    // when zero, can return immediately
    if (number_to_encode == 0)
    {
        target_buffer[0] = '0';
        return 1;
    }

    // digit count via logarithm lookup
    std::uint8_t digits_left_to_encode = detail::digit_count(number_to_encode);

    // left-to-right extraction, 2 digits (0-99) per loop
    // Each iteration costs: 1 divide + 1 multiply + 1 subtract
    // (vs. 2 divides + 2 mods in the classic right-to-left scheme)
    while (digits_left_to_encode >= 2)
    {
        std::uint64_t divisor = detail::pow10[digits_left_to_encode - 2]; // 10^(L-2)
        std::uint64_t chunk = number_to_encode / divisor;                 // 0 … 99
        std::uint16_t lut_index = static_cast<std::uint16_t>(chunk) * 2;

        // tens
        target_buffer[target_index] = detail::two_digit_lut[lut_index];
        ++target_index;

        // ones
        target_buffer[target_index] = detail::two_digit_lut[lut_index + 1];
        ++target_index;

        // remove leading part
        number_to_encode -= chunk * divisor;
        digits_left_to_encode -= 2;
    }

    // remaining digit (this is also what is run when the value is <10)
    if (digits_left_to_encode == 1)
    {
        target_buffer[target_index] = static_cast<char>('0' + number_to_encode);
        ++target_index;
    }

    // number of characters written; no null terminator appended
    return target_index;
}


// --- below is tests / benchmarks


void print_number(result_buffer_type &source, size_t num_characters)
{
    for (size_t i = 0; i < num_characters; i++)
    {
        printf("%c", source[i]);
    }
}

bool compare_number(result_buffer_type &source, size_t num_characters, std::string expected)
{
    if (expected.size() != num_characters)
        return false;

    for (size_t i = 0; i < num_characters; i++)
    {
        if (source[i] != expected[i])
            return false;
    }

    return true;
}

void test_conversion(auto integer_value, std::string expected)
{
    result_buffer_type target = {};
    auto characters_written = int_to_string(integer_value, target);
    print_number(target, characters_written);
    printf("\n");
    assert(compare_number(target, characters_written, expected));
}

#include <chrono>
#include <random>

void benchmark()
{
    // A small, probably not accurate benchmark

    // The int_to_string conversion method seems to use 87.48% of the time of itoa
    // but does not support different bases.

    std::cout << "\n\n*************\n\nStarting benchmark" << '\n';

    auto seed =
        static_cast<std::mt19937::result_type>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    // calculate a reduction value based on randomness with seed from current time
    uint64_t reduction_value;
    {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<uint64_t> dist(1, 2);
        reduction_value = dist(rng);
        std::cout << "\n\nInteger reduction value per loop cycle: " << reduction_value << '\n';
    }

    size_t loop_count = 0;
    {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<uint64_t> dist(10000000, 20000000);
        loop_count = dist(rng);
        std::cout << "Loop count: " << loop_count << '\n';
    }

    std::cout << "_ui64toa:      ";
    {
        result_buffer_type buffer = {};
        uint64_t integer = std::numeric_limits<std::uint64_t>::max();

        auto start = std::chrono::system_clock::now();
        for (size_t i = 0; i < loop_count; ++i)
        {
            _ui64toa(integer, buffer.data(), 10);
            integer -= reduction_value;
        }
        auto end = std::chrono::system_clock::now();
        auto elapsed = end - start;
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms\n";
    }

    std::cout << "std::to_chars: ";
    {
        result_buffer_type buffer = {};

        uint64_t integer = std::numeric_limits<std::uint64_t>::max();
        uint64_t accumulated_characters = 0;
        auto start = std::chrono::system_clock::now();
        for (size_t i = 0; i < loop_count; ++i)
        {
            auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), integer);
            accumulated_characters += static_cast<std::size_t>(ptr - buffer.data());
            integer -= reduction_value;
        }
        auto end = std::chrono::system_clock::now();

        auto elapsed = end - start;
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms       ";

        // if this is not printed, the whole thing will be optimized out
        std::cout << " - accumulated_characters:" << accumulated_characters << '\n';
    }

    std::cout << "int_to_string: ";
    {
        result_buffer_type buffer = {};
        uint64_t integer = std::numeric_limits<std::uint64_t>::max();
        uint64_t accumulated_characters = 0;

        auto start = std::chrono::system_clock::now();
        for (size_t i = 0; i < loop_count; ++i)
        {
            accumulated_characters += int_to_string(integer, buffer);
            integer -= reduction_value;
        }
        auto end = std::chrono::system_clock::now();

        auto elapsed = end - start;

        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms      ";

        // if this is not printed, the whole thing will be optimized out
        std::cout << " - accumulated_characters:" << accumulated_characters << '\n';
    }
}

int main()
{
    test_conversion(1024ULL, "1024");
    test_conversion(-1024, "-1024");
    test_conversion(-1, "-1");
    test_conversion(0, "0");
    test_conversion(9, "9");
    test_conversion(10, "10");
    test_conversion(99, "99");
    test_conversion(100, "100");
    test_conversion(101, "101");
    test_conversion(999, "999");
    test_conversion(1'000, "1000");
    test_conversion(-1, "-1");
    test_conversion(-100, "-100");
    test_conversion(std::numeric_limits<std::int8_t>::min(), "-128");
    test_conversion(std::numeric_limits<std::int8_t>::max(), "127");

    test_conversion(std::numeric_limits<std::uint8_t>::min(), "0");
    test_conversion(std::numeric_limits<std::uint8_t>::max(), "255");

    test_conversion(std::numeric_limits<std::int64_t>::min(), "-9223372036854775808");
    test_conversion(std::numeric_limits<std::int64_t>::max(), "9223372036854775807");

    test_conversion(std::numeric_limits<std::uint64_t>::min(), "0");
    test_conversion(std::numeric_limits<std::uint64_t>::max(), "18446744073709551615");

    benchmark();
}
