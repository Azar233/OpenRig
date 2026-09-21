#pragma once

#include "openrig/core/audio/AudioNode.h"
#include "openrig/dsp/common/LinearSmoother.h"

#include <array>
#include <memory>

namespace openrig::dsp
{
class GainNode final : public AudioNode
{
public:
    enum : ParameterIndex
    {
        kGain = 0,
    };

    void prepare(const PrepareSpec& spec) override;
    void reset() noexcept override;
    void process(const AudioBlockView& input, const AudioBlockView& output, std::uint32_t numFrames) noexcept override;
    void setBypassed(bool bypassed) noexcept override { bypassed_ = bypassed; }
    [[nodiscard]] std::span<const ParameterDescriptor> parameters() const noexcept override { return descriptors_; }
    bool setParameter(ParameterIndex index, Sample value) noexcept override;

private:
    inline static const std::array<ParameterDescriptor, 1> descriptors_{{
        {"gain", "Gain", "linear", ParameterKind::Continuous, 0.0F, 2.0F, 1.0F, ParameterCurve::Linear, 10.0F},
    }};

    LinearSmoother gain_;
    Sample targetGain_ = 1.0F;
    bool bypassed_ = false;
};

[[nodiscard]] std::unique_ptr<AudioNode> makeGainNode();
} // namespace openrig::dsp
