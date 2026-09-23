#include "openrig/framework/iplug/IPlugAudioAdapter.h"

namespace openrig::framework::iplug
{
void IPlugAudioAdapter::prepare(
    AudioEngine& engine,
    const PrepareSpec& spec,
    const OutputChannelPolicy outputPolicy)
{
    spec_ = spec;
    outputPolicy_ = outputPolicy;
    inputStorage_.resize(static_cast<std::size_t>(spec.numInputChannels) * spec.maxBlockSize);
    outputStorage_.resize(static_cast<std::size_t>(spec.numOutputChannels) * spec.maxBlockSize);
    inputChannels_.resize(spec.numInputChannels);
    outputChannels_.resize(spec.numOutputChannels);

    for (std::uint32_t channel = 0; channel < spec.numInputChannels; ++channel)
        inputChannels_[channel] = inputStorage_.data() + static_cast<std::size_t>(channel) * spec.maxBlockSize;
    for (std::uint32_t channel = 0; channel < spec.numOutputChannels; ++channel)
        outputChannels_[channel] = outputStorage_.data() + static_cast<std::size_t>(channel) * spec.maxBlockSize;

    engine_ = &engine;
}
} // namespace openrig::framework::iplug
