/*
This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Jonatan Wallmander <jonatan.wallmander@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include <chrono>
#include <iostream>
#include <windows.h>

namespace
{
[[maybe_unused]] LARGE_INTEGER calculate_timer_due_in_milliseconds(const LONGLONG milliseconds)
{
    LARGE_INTEGER result;
    result.QuadPart = -(1'0000LL * milliseconds);
    return result;
}

[[maybe_unused]] LARGE_INTEGER calculate_timer_due_in_microseconds(const LONGLONG milliseconds)
{
    LARGE_INTEGER result;
    result.QuadPart = -(1'0LL * milliseconds);
    return result;
}

void print_elapsed_time(auto old_timestamp, auto new_timestamp)
{
    auto elapsed = new_timestamp - old_timestamp;
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " microseconds\n";
}
}

int main()
{
    HANDLE hTimer = CreateWaitableTimer(nullptr, FALSE, nullptr);

    LARGE_INTEGER liDueTime = calculate_timer_due_in_milliseconds(100);

    std::cout << "Waiting for 100 milliseconds, then repeatedly waiting for 100 milliseconds...\n";

    if (!SetWaitableTimer(hTimer, &liDueTime, 100, nullptr, nullptr, 0))
    {
        std::cout << "SetWaitableTimer failed (" << GetLastError() << ")\n";
        return 2;
    }

    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

    if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
        std::cout << "WaitForSingleObject failed (" << GetLastError() << ")\n";

    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    print_elapsed_time(t1, t2);

    std::chrono::steady_clock::time_point t_previous_period = t2;

    // wait for 1000 events
    for (int i = 0; i < 1000; i++)
    {
        // this now waits and consumes no CPU while waiting
        if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
            std::cout << "WaitForSingleObject failed (" << GetLastError() << ")\n";
        auto t_period = std::chrono::steady_clock::now();

        // The output will look something like this:
        // 99755 microseconds
        // 100031 microseconds
        // 100197 microseconds
        // 100240 microseconds
        // 99487 microseconds
        // ...
        // but over many iterations, the average will be around 100 ms

        print_elapsed_time(t_previous_period, t_period);
        t_previous_period = t_period;
    }

    CloseHandle(hTimer);
    return 0;
}
