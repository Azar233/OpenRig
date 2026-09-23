#pragma once

#include "openrig/core/audio/AudioNode.h"
#include "openrig/dsp/common/LinearSmoother.h"

#include <array>
#include <memory>

namespace openrig::dsp
{
class SoftClipNode final : public AudioNode
{
public:
    enum : ParameterIndex
    {
        kDrive = 0,
        kLevel = 1,
    };

    void prepare(const PrepareSpec& spec) override;
    void reset() noexcept override;
    void process(const AudioBlockView& input, const AudioBlockView& output, std::uint32_t numFrames) noexcept override;
    void setBypassed(bool bypassed) noexcept override { bypassed_ = bypassed; }
    [[nodiscard]] NodeCapabilities capabilities() const noexcept override { return {false, 1, 1}; }
    [[nodiscard]] std::span<const ParameterDescriptor> parameters() const noexcept override { return descriptors_; }
    bool setParameter(ParameterIndex index, Sample value) noexcept override;

private:
    inline static const std::array<ParameterDescriptor, 2> descriptors_{{
        {"drive", "Drive", "", ParameterKind::Continuous, 1.0F, 20.0F, 2.0F, ParameterCurve::Logarithmic, 10.0F},
        {"level", "Level", "linear", ParameterKind::Continuous, 0.0F, 1.0F, 0.75F, ParameterCurve::Linear, 10.0F},
    }};

    LinearSmoother drive_;
    LinearSmoother level_;
    Sample targetDrive_ = 2.0F;
    Sample targetLevel_ = 0.75F;
    bool bypassed_ = false;
};

[[nodiscard]] std::unique_ptr<AudioNode> makeSoftClipNode();
} // namespace openrig::dsp
