#pragma once

#include "openrig/core/audio/PrepareSpec.h"
#include "openrig/core/graph/CompiledGraph.h"
#include "openrig/core/parameter/ParameterEngine.h"
#include "openrig/core/realtime/SpscQueue.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>

namespace openrig
{
class AudioEngine
{
public:
    static constexpr std::size_t kGraphQueueCapacity = 8;
    static constexpr std::size_t kDefaultParameterBudget = 64;

    explicit AudioEngine(PrepareSpec spec, std::size_t parameterBudget = kDefaultParameterBudget) noexcept;
    // The caller must stop the audio callback before destruction.
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    // Control-thread producer. A rejected graph is destroyed on the caller's thread.
    [[nodiscard]] bool tryPublishGraph(std::unique_ptr<graph::CompiledGraph> graph) noexcept;
    [[nodiscard]] bool enqueueParameter(const ParameterEvent& event) noexcept;

    // Audio-thread entry point. Graph adoption and parameter application occur at the block boundary.
    [[nodiscard]] graph::GraphProcessStatus process(
        const AudioBlockView& input,
        const AudioBlockView& output,
        std::uint32_t numFrames) noexcept;

    // Control-thread consumer. Retired graphs are destroyed only from this function or the stopped engine destructor.
    [[nodiscard]] std::size_t drainRetiredGraphs(
        std::size_t maximum = std::numeric_limits<std::size_t>::max()) noexcept;

    [[nodiscard]] std::uint64_t rejectedPublishCount() const noexcept { return rejectedPublishes_.load(std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t retireBackpressureCount() const noexcept { return retireBackpressure_.load(std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t rejectedParameterCount() const noexcept { return rejectedParameters_.load(std::memory_order_relaxed); }

private:
    void serviceGraphMailbox() noexcept;
    [[nodiscard]] graph::GraphProcessStatus processWithoutGraph(
        const AudioBlockView& input,
        const AudioBlockView& output,
        std::uint32_t numFrames) const noexcept;

    PrepareSpec spec_{};
    std::size_t parameterBudget_ = kDefaultParameterBudget;
    ParameterEngine parameters_;
    realtime::SpscQueue<graph::CompiledGraph*, kGraphQueueCapacity> publishQueue_;
    realtime::SpscQueue<graph::CompiledGraph*, kGraphQueueCapacity> retireQueue_;
    graph::CompiledGraph* activeGraph_ = nullptr;
    graph::CompiledGraph* stagedGraph_ = nullptr;
    graph::CompiledGraph* deferredRetire_ = nullptr;
    std::atomic<std::uint64_t> rejectedPublishes_{0};
    std::atomic<std::uint64_t> retireBackpressure_{0};
    std::atomic<std::uint64_t> rejectedParameters_{0};
};
} // namespace openrig
