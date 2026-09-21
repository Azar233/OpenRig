#pragma once

#include <cstdint>

namespace openrig
{
struct NodeCapabilities
{
    bool supportsInPlace = false;
    std::uint32_t inputChannels = 1;
    std::uint32_t outputChannels = 1;
};
} // namespace openrig
