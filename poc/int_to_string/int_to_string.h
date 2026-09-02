/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2026 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

// left-to-right integer-to-string conversion
//  - Extracts 2 digits (0-99) per iteration via a precomputed lookup table
//  - Determines digit count from a logarithm-derived table (log10 2^p)
//  - Uses at most: 1 divide + 1 multiply + 1 subtract per 2 digits

using result_buffer_type = std::array<char, 32>;

namespace detail
{

// precomputed powers of ten (indices 0..20 cover 10^0 .. 10^20)
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
// is  floor(p * log10 2) + 1.
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

// two-digit -> ASCII lookup (saves divide/modulo per digit)
// two_digit_lut[v*2]     -> tens  digit character
// two_digit_lut[v*2 + 1] -> ones  digit character
static constexpr std::array<char,200> two_digit_lut = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', '1', '0', '1',
    '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2', '2',
    '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3',
    '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5',
    '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5',
    '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8',
    '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8',
    '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0', '9', '1',
    '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'};


// MSB position (0-indexed) via binary bit-scan - no floating point
constexpr std::uint8_t msb_pos(std::uint64_t n) noexcept
{
    std::uint8_t p = 0;
    if (n >= (1ULL << 32))
    {
        p += 32U;
        n >>= 32U;
    }
    if (n >= (1ULL << 16))
    {
        p += 16U;
        n >>= 16U;
    }
    if (n >= (1ULL << 8))
    {
        p += 8U;
        n >>= 8U;
    }
    if (n >= (1ULL << 4))
    {
        p += 4U;
        n >>= 4U;
    }
    if (n >= (1ULL << 2))
    {
        p += 2U;
        n >>= 2U;
    }
    if (n >= (1ULL << 1))
    {
        p += 1U;
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
    std::uint8_t L = digit_count_lut[p]; // floor(p * log10_2)+1

    // boundary correction (<=1 cmp)
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

            // when integer_value is the minimum value, special handling is needed.
            // For example: -128 for int8_t:
            //   1. +1: -127
            //   2. cast to negative value and uint64_t: 127
            //   3. +1: 128
            {
                target_index = 1;
                T temp_value = integer_value + 1;
                number_to_encode = static_cast<std::uint64_t>(-temp_value);
                number_to_encode++;
            }
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

    static_assert(std::clamp(static_cast<uint16_t>(201), static_cast<uint16_t>(0U), static_cast<uint16_t>(199U)) == 199);

    // left-to-right extraction, 2 digits (0-99) per loop
    // Each iteration costs: 1 divide + 1 multiply + 1 subtract
    // (vs. 2 divides + 2 mods in the classic right-to-left scheme)
    while (digits_left_to_encode >= 2)
    {
        std::uint64_t divisor = detail::pow10[digits_left_to_encode - 2]; // 10^(L-2)
        auto chunk = static_cast<uint16_t>((number_to_encode / divisor) & 0xFFFFULL);

        // chunk must be possible to multiply by 2 without overflow
        std::uint8_t lut_index = (chunk & static_cast<uint8_t>(0b01111111)) * 2;

        lut_index = std::clamp(lut_index, static_cast<uint8_t>(0U), static_cast<uint8_t>(detail::two_digit_lut.size() - 1));

        // tens
        target_buffer[target_index] = detail::two_digit_lut[lut_index];
        ++target_index;

        // ones
        target_buffer[target_index] = detail::two_digit_lut[lut_index + 1];
        ++target_index;

        // remove leading part
        uint64_t number_to_reduce = static_cast<uint64_t>(chunk) * divisor; // when this is on the next line, tool misinterprets the whole thing
        number_to_encode -= number_to_reduce;

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
