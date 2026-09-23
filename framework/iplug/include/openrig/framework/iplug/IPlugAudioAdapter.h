#pragma once

#include "openrig/core/audio/AudioEngine.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace openrig::framework::iplug
{
enum class OutputChannelPolicy
{
    ClearExtra,
    DuplicateMono,
};

class IPlugAudioAdapter
{
public:
    void prepare(
        AudioEngine& engine,
        const PrepareSpec& spec,
        OutputChannelPolicy outputPolicy = OutputChannelPolicy::ClearExtra);
    void reset() noexcept { engine_ = nullptr; }

    [[nodiscard]] const PrepareSpec& spec() const noexcept { return spec_; }
    [[nodiscard]] bool isPrepared() const noexcept { return engine_ != nullptr; }

    template <typename HostSample>
    [[nodiscard]] graph::GraphProcessStatus process(
        HostSample* const* inputs,
        std::uint32_t inputChannels,
        HostSample* const* outputs,
        std::uint32_t outputChannels,
        std::uint32_t numFrames) noexcept
    {
        static_assert(std::is_floating_point_v<HostSample>, "Host sample type must be floating point");

        const auto clearHostOutputs = [&]() noexcept {
            if (outputs == nullptr)
                return;
            for (std::uint32_t channel = 0; channel < outputChannels; ++channel)
                if (outputs[channel] != nullptr)
                    std::fill_n(outputs[channel], numFrames, static_cast<HostSample>(0));
        };

        if (engine_ == nullptr || outputs == nullptr)
        {
            clearHostOutputs();
            return graph::GraphProcessStatus::InvalidBuffer;
        }
        if (numFrames > spec_.maxBlockSize)
        {
            clearHostOutputs();
            return graph::GraphProcessStatus::BlockTooLarge;
        }

        for (std::uint32_t channel = 0; channel < spec_.numInputChannels; ++channel)
        {
            auto* destination = inputChannels_[channel];
            const auto* source = inputs != nullptr && channel < inputChannels ? inputs[channel] : nullptr;
            if (source == nullptr)
                std::fill_n(destination, numFrames, 0.0F);
            else
                for (std::uint32_t frame = 0; frame < numFrames; ++frame)
                    destination[frame] = static_cast<Sample>(source[frame]);
        }

        const AudioBlockView inputView{inputChannels_.data(), spec_.numInputChannels, numFrames};
        const AudioBlockView outputView{outputChannels_.data(), spec_.numOutputChannels, numFrames};
        const auto status = engine_->process(inputView, outputView, numFrames);

        for (std::uint32_t channel = 0; channel < outputChannels; ++channel)
        {
            auto* destination = outputs[channel];
            if (destination == nullptr)
                continue;
            if (channel >= spec_.numOutputChannels)
            {
                if (outputPolicy_ == OutputChannelPolicy::DuplicateMono && spec_.numOutputChannels == 1)
                {
                    const auto* source = outputChannels_.front();
                    for (std::uint32_t frame = 0; frame < numFrames; ++frame)
                        destination[frame] = static_cast<HostSample>(source[frame]);
                }
                else
                {
                    std::fill_n(destination, numFrames, static_cast<HostSample>(0));
                }
                continue;
            }
            const auto* source = outputChannels_[channel];
            for (std::uint32_t frame = 0; frame < numFrames; ++frame)
                destination[frame] = static_cast<HostSample>(source[frame]);
        }
        return status;
    }

private:
    AudioEngine* engine_ = nullptr;
    PrepareSpec spec_{};
    OutputChannelPolicy outputPolicy_ = OutputChannelPolicy::ClearExtra;
    std::vector<Sample> inputStorage_;
    std::vector<Sample> outputStorage_;
    std::vector<Sample*> inputChannels_;
    std::vector<Sample*> outputChannels_;
};
} // namespace openrig::framework::iplug
