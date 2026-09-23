#pragma once

#include <cmath>
#include <iostream>
#include <string_view>

struct TestSuite
{
    int failures = 0;

    void check(const bool condition, const std::string_view message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            ++failures;
        }
    }

    [[nodiscard]] int finish(const std::string_view name) const
    {
        if (failures == 0)
            std::cout << name << " passed.\n";
        return failures == 0 ? 0 : 1;
    }
};

inline bool near(const float left, const float right, const float tolerance = 1.0e-5F)
{
    return std::abs(left - right) <= tolerance;
}
