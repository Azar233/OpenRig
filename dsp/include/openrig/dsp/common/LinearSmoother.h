#pragma once

#include "openrig/core/Types.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace openrig::dsp
{
class LinearSmoother
{
public:
    void prepare(const double sampleRate, const Sample timeMs) noexcept
    {
        rampLength_ = static_cast<std::uint32_t>(
            std::max(1.0, std::round(sampleRate * static_cast<double>(timeMs) * 0.001)));
    }

    void reset(const Sample value) noexcept
    {
        current_ = value;
        target_ = value;
        step_ = 0.0F;
        remaining_ = 0;
    }

    void setTarget(const Sample target) noexcept
    {
        target_ = target;
        remaining_ = rampLength_;
        step_ = (target_ - current_) / static_cast<Sample>(remaining_);
    }

    [[nodiscard]] Sample next() noexcept
    {
        if (remaining_ > 0)
        {
            current_ += step_;
            if (--remaining_ == 0)
                current_ = target_;
        }
        return current_;
    }

private:
    Sample current_ = 0.0F;
    Sample target_ = 0.0F;
    Sample step_ = 0.0F;
    std::uint32_t rampLength_ = 1;
    std::uint32_t remaining_ = 0;
};
} // namespace openrig::dsp
