#include "openrig/dsp/drive/SoftClipNode.h"

#include <algorithm>
#include <cmath>

namespace openrig::dsp
{
void SoftClipNode::prepare(const PrepareSpec& spec)
{
    drive_.prepare(spec.sampleRate, descriptors_[kDrive].smoothingTimeMs);
    level_.prepare(spec.sampleRate, descriptors_[kLevel].smoothingTimeMs);
    reset();
}

void SoftClipNode::reset() noexcept
{
    drive_.reset(targetDrive_);
    level_.reset(targetLevel_);
}

bool SoftClipNode::setParameter(const ParameterIndex index, const Sample value) noexcept
{
    if (index == kDrive)
    {
        targetDrive_ = std::clamp(value, descriptors_[kDrive].minimum, descriptors_[kDrive].maximum);
        drive_.setTarget(targetDrive_);
        return true;
    }
    if (index == kLevel)
    {
        targetLevel_ = std::clamp(value, descriptors_[kLevel].minimum, descriptors_[kLevel].maximum);
        level_.setTarget(targetLevel_);
        return true;
    }
    return false;
}

void SoftClipNode::process(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t numFrames) noexcept
{
    const auto channels = std::min(input.numChannels, output.numChannels);
    for (std::uint32_t frame = 0; frame < numFrames; ++frame)
    {
        const auto drive = drive_.next();
        const auto level = level_.next();
        for (std::uint32_t channel = 0; channel < channels; ++channel)
        {
            const auto sample = input.channel(channel)[frame];
            output.channel(channel)[frame] = bypassed_ ? sample : std::tanh(sample * drive) * level;
        }
    }

    for (std::uint32_t channel = channels; channel < output.numChannels; ++channel)
        std::fill_n(output.channel(channel), numFrames, 0.0F);
}

std::unique_ptr<AudioNode> makeSoftClipNode()
{
    return std::make_unique<SoftClipNode>();
}
} // namespace openrig::dsp
