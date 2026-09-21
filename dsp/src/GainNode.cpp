#include "openrig/dsp/utility/GainNode.h"

#include <algorithm>

namespace openrig::dsp
{
void GainNode::prepare(const PrepareSpec& spec)
{
    gain_.prepare(spec.sampleRate, descriptors_[kGain].smoothingTimeMs);
    reset();
}

void GainNode::reset() noexcept
{
    gain_.reset(targetGain_);
}

bool GainNode::setParameter(const ParameterIndex index, const Sample value) noexcept
{
    if (index != kGain)
        return false;

    targetGain_ = std::clamp(value, descriptors_[kGain].minimum, descriptors_[kGain].maximum);
    gain_.setTarget(targetGain_);
    return true;
}

void GainNode::process(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t numFrames) noexcept
{
    const auto channels = std::min(input.numChannels, output.numChannels);
    for (std::uint32_t frame = 0; frame < numFrames; ++frame)
    {
        const auto gain = bypassed_ ? 1.0F : gain_.next();
        for (std::uint32_t channel = 0; channel < channels; ++channel)
            output.channel(channel)[frame] = input.channel(channel)[frame] * gain;
    }

    for (std::uint32_t channel = channels; channel < output.numChannels; ++channel)
        std::fill_n(output.channel(channel), numFrames, 0.0F);
}

std::unique_ptr<AudioNode> makeGainNode()
{
    return std::make_unique<GainNode>();
}
} // namespace openrig::dsp
