/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2026 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include "int_to_string.h"

#include <chrono>
#include <random>
#include <iostream>

int main()
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
