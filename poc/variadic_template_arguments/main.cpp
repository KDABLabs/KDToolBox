/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include <iostream>
#include <cstdint>

namespace
{
struct some_struct {
    int a = 0;
    int b = 0;
};

template <class... Types>
void print_types(const char *normal_argument, const Types &...args) {
    std::cout << normal_argument << '\n';

    auto check_and_print = [](const auto &arg) {
        // strip away the reference and/or const/volatile (cv) qualifiers
        using T = std::remove_cvref_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, some_struct>) {
            std::cout << "Some struct value of a: " << arg.a << " ... value of b: " << arg.b << '\n';
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "Double value: " << arg << '\n';
        } else if constexpr (std::is_same_v<T, float>) {
            std::cout << "Float value: " << arg << '\n';
        } else if constexpr (std::is_integral_v<T>) {
            std::cout << "Integer value - ";
            if constexpr (std::is_same_v<T, uint64_t>)
                std::cout << "uint64_t value: " << arg << '\n';
            else
            if constexpr (std::is_same_v<T, int64_t>)
                std::cout << "int64_t value: " << arg << '\n';
            else
                std::cout << "Other integer value: " << arg << '\n';
        } else {
            std::cout << "Unsupported type..."<< '\n';
        }
    };

    // Unary fold expression over comma operator
    (check_and_print(args), ...);
}
}

int main()
{
    print_types(
        "abc",
        12, // other integer value
        18ULL, // uint64_t
        -9LL, // int64_t
        1.1, // double
        1.3f, // float
        some_struct{.a=42}, // some_struct
        nullptr // unsupported type
    );
    return 0;
}
