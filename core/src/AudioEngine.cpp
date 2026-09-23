#include "openrig/core/audio/AudioEngine.h"

#include "openrig/core/realtime/RealtimeGuard.h"

#include <algorithm>

namespace openrig
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
} // namespace

AudioEngine::AudioEngine(const PrepareSpec spec, const std::size_t parameterBudget) noexcept
    : spec_(spec), parameterBudget_(parameterBudget)
{
}

AudioEngine::~AudioEngine()
{
    graph::CompiledGraph* queued = nullptr;
    while (publishQueue_.tryPop(queued))
        delete queued;
    while (retireQueue_.tryPop(queued))
        delete queued;
    delete deferredRetire_;
    delete stagedGraph_;
    delete activeGraph_;
}

bool AudioEngine::tryPublishGraph(std::unique_ptr<graph::CompiledGraph> graph) noexcept
{
    if (!graph || !publishQueue_.tryPush(graph.get()))
    {
        rejectedPublishes_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    static_cast<void>(graph.release());
    return true;
}

bool AudioEngine::enqueueParameter(const ParameterEvent& event) noexcept
{
    if (parameters_.enqueue(event))
        return true;
    rejectedParameters_.fetch_add(1, std::memory_order_relaxed);
    return false;
}

graph::GraphProcessStatus AudioEngine::process(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t numFrames) noexcept
{
    const realtime::RealtimeScope realtimeScope;
    serviceGraphMailbox();
    static_cast<void>(parameters_.drainUpTo(parameterBudget_, [&](const ParameterEvent& event) noexcept {
        if (activeGraph_ != nullptr)
            static_cast<void>(activeGraph_->applyParameter(event));
    }));

    return activeGraph_ != nullptr ? activeGraph_->process(input, output, numFrames)
                                   : processWithoutGraph(input, output, numFrames);
}

std::size_t AudioEngine::drainRetiredGraphs(const std::size_t maximum) noexcept
{
    graph::CompiledGraph* retired = nullptr;
    std::size_t drained = 0;
    while (drained < maximum && retireQueue_.tryPop(retired))
    {
        delete retired;
        ++drained;
    }
    return drained;
}

void AudioEngine::serviceGraphMailbox() noexcept
{
    if (deferredRetire_ != nullptr)
    {
        if (!retireQueue_.tryPush(deferredRetire_))
        {
            retireBackpressure_.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        deferredRetire_ = nullptr;
    }

    if (stagedGraph_ == nullptr)
        static_cast<void>(publishQueue_.tryPop(stagedGraph_));
    if (stagedGraph_ == nullptr)
        return;

    auto* oldGraph = activeGraph_;
    activeGraph_ = stagedGraph_;
    stagedGraph_ = nullptr;
    if (oldGraph != nullptr && !retireQueue_.tryPush(oldGraph))
    {
        deferredRetire_ = oldGraph;
        retireBackpressure_.fetch_add(1, std::memory_order_relaxed);
    }
}

graph::GraphProcessStatus AudioEngine::processWithoutGraph(
    const AudioBlockView& input,
    const AudioBlockView& output,
    const std::uint32_t numFrames) const noexcept
{
    const auto clearFrames = std::min(numFrames, output.numFrames);
    if (!hasValidChannels(output, spec_.numOutputChannels))
    {
        clearAudio(output, clearFrames);
        return graph::GraphProcessStatus::InvalidBuffer;
    }
    if (numFrames > spec_.maxBlockSize)
    {
        clearAudio(output, clearFrames);
        return graph::GraphProcessStatus::BlockTooLarge;
    }
    if (!hasValidChannels(input, spec_.numInputChannels) || input.numFrames < numFrames || output.numFrames < numFrames)
    {
        clearAudio(output, clearFrames);
        return graph::GraphProcessStatus::InvalidBuffer;
    }

    if (input.numChannels != output.numChannels)
    {
        clearAudio(output, numFrames);
        return graph::GraphProcessStatus::Ok;
    }
    for (std::uint32_t channel = 0; channel < input.numChannels; ++channel)
        std::copy_n(input.channels[channel], numFrames, output.channels[channel]);
    return graph::GraphProcessStatus::Ok;
}
} // namespace openrig
