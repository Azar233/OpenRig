#pragma once

#include "openrig/core/Types.h"

#include <cassert>
#include <cstdint>

namespace openrig
{
// Non-owning planar audio view. Allocation and lifetime belong to the caller.
struct AudioBlockView
{
    Sample* const* channels = nullptr;
    std::uint32_t numChannels = 0;
    std::uint32_t numFrames = 0;

    [[nodiscard]] Sample* channel(const std::uint32_t index) const noexcept
    {
        assert(index < numChannels);
        return channels[index];
    }
};
} // namespace openrig
