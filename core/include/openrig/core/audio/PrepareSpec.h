#pragma once

#include <cstdint>

namespace openrig
{
struct PrepareSpec
{
    double sampleRate = 48'000.0;
    std::uint32_t maxBlockSize = 512;
    std::uint32_t numInputChannels = 1;
    std::uint32_t numOutputChannels = 1;
};
} // namespace openrig
