#pragma once

#include "openrig/core/audio/AudioBlockView.h"
#include "openrig/core/audio/PrepareSpec.h"
#include "openrig/core/parameter/Parameter.h"

#include <cstdint>
#include <span>

namespace openrig
{
class AudioNode
{
public:
    virtual ~AudioNode() = default;

    virtual void prepare(const PrepareSpec& spec) = 0;
    virtual void reset() noexcept = 0;

    virtual void process(
        const AudioBlockView& input,
        const AudioBlockView& output,
        std::uint32_t numFrames) noexcept = 0;

    virtual void setBypassed(bool bypassed) noexcept = 0;

    [[nodiscard]] virtual std::span<const ParameterDescriptor> parameters() const noexcept = 0;
    virtual bool setParameter(ParameterIndex index, Sample value) noexcept = 0;

    [[nodiscard]] virtual std::uint32_t latencySamples() const noexcept { return 0; }
    [[nodiscard]] virtual std::uint32_t tailSamples() const noexcept { return 0; }
};
} // namespace openrig
