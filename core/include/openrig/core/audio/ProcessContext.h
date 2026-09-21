#pragma once

#include "openrig/core/audio/AudioBlockView.h"

namespace openrig
{
struct ProcessContext
{
    AudioBlockView input;
    AudioBlockView output;
    std::uint32_t numFrames = 0;
};
} // namespace openrig
