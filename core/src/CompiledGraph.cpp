#include "openrig/core/graph/CompiledGraph.h"

#include <algorithm>

namespace openrig::graph
{
namespace
{
bool hasValidChannels(const AudioBlockView& block, const std::uint32_t requiredChannels) noexcept
{
    if (block.channels == nullptr || block.numChannels != requiredChannels)
        return false;
    for (std::uint32_t channel = 0; channel < block.numChannels; ++channel)
        if (block.channels[channel] == nullptr)
            return false;
    return true;
}

void clearAudio(const AudioBlockView& output, const std::uint32_t frames) noexcept
{
    if (output.channels == nullptr)
        return;
    for (std::uint32_t channel = 0; channel < output.numChannels; ++channel)
        if (output.channels[channel] != nullptr)
            std::fill_n(output.channels[channel], frames, 0.0F);
}

void copyAudio(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t frames) noexcept
{
    for (std::uint32_t channel = 0; channel < input.numChannels; ++channel)
        std::copy_n(input.channels[channel], frames, output.channels[channel]);
}
} // namespace

GraphProcessStatus CompiledGraph::process(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t numFrames) noexcept
{
    const auto clearFrames = std::min(numFrames, output.numFrames);
    if (!hasValidChannels(output, spec_.numOutputChannels))
    {
        clearAudio(output, clearFrames);
        return GraphProcessStatus::InvalidBuffer;
    }
    if (numFrames > spec_.maxBlockSize)
    {
        clearAudio(output, clearFrames);
        return GraphProcessStatus::BlockTooLarge;
    }
    if (!hasValidChannels(input, spec_.numInputChannels) || input.numFrames < numFrames || output.numFrames < numFrames)
    {
        clearAudio(output, clearFrames);
        return GraphProcessStatus::InvalidBuffer;
    }

    if (nodes_.empty())
    {
        copyAudio(input, output, numFrames);
        return GraphProcessStatus::Ok;
    }

    AudioBlockView currentInput{input.channels, input.numChannels, numFrames};
    for (std::size_t index = 0; index < nodes_.size(); ++index)
    {
        const bool isLast = index + 1 == nodes_.size();
        AudioBlockView currentOutput{output.channels, output.numChannels, numFrames};

        if (!isLast)
        {
            auto& pointers = index % 2 == 0 ? channelsA_ : channelsB_;
            currentOutput = {pointers.data(), nodeOutputChannels_[index], numFrames};
        }

        nodes_[index]->process(currentInput, currentOutput, numFrames);
        currentInput = currentOutput;
    }
    return GraphProcessStatus::Ok;
}

bool CompiledGraph::applyParameter(const ParameterEvent& event) noexcept
{
    const auto route = std::lower_bound(routes_.begin(), routes_.end(), event.address.node, [](const auto& item, const NodeId id) {
        return item.id < id;
    });
    return route != routes_.end() && route->id == event.address.node &&
           route->node->setParameter(event.address.parameter, event.value);
}
} // namespace openrig::graph
