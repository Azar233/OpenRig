#include "openrig/core/graph/CompiledGraph.h"

#include <algorithm>
#include <cassert>

namespace openrig::graph
{
namespace
{
void copyAudio(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t frames) noexcept
{
    const auto channels = std::min(input.numChannels, output.numChannels);
    for (std::uint32_t channel = 0; channel < channels; ++channel)
        std::copy_n(input.channel(channel), frames, output.channel(channel));

    for (std::uint32_t channel = channels; channel < output.numChannels; ++channel)
        std::fill_n(output.channel(channel), frames, 0.0F);
}
} // namespace

void CompiledGraph::process(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t numFrames) noexcept
{
    assert(numFrames <= spec_.maxBlockSize);
    if (numFrames > spec_.maxBlockSize)
        return;

    if (nodes_.empty())
    {
        copyAudio(input, output, numFrames);
        return;
    }

    AudioBlockView currentInput = input;
    for (std::size_t index = 0; index < nodes_.size(); ++index)
    {
        const bool isLast = index + 1 == nodes_.size();
        AudioBlockView currentOutput = output;

        if (!isLast)
        {
            auto& pointers = index % 2 == 0 ? channelsA_ : channelsB_;
            currentOutput = {pointers.data(), static_cast<std::uint32_t>(pointers.size()), numFrames};
        }

        nodes_[index]->process(currentInput, currentOutput, numFrames);
        currentInput = currentOutput;
    }
}
} // namespace openrig::graph
