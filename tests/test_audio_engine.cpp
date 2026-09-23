#include "test_support.h"

#include "openrig/core/audio/AudioEngine.h"
#include "openrig/core/graph/GraphCompiler.h"
#include "openrig/core/realtime/RealtimeGuard.h"

#include <array>
#include <atomic>
#include <cstdlib>
#include <memory>
#include <new>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

namespace
{
std::atomic<std::uint64_t> realtimeAllocations{0};

void recordRealtimeAllocation() noexcept
{
    if (openrig::realtime::RealtimeGuard::isRealtimeThread())
        realtimeAllocations.fetch_add(1, std::memory_order_relaxed);
}
} // namespace

void* operator new(const std::size_t size)
{
    recordRealtimeAllocation();
    if (auto* memory = std::malloc(size == 0 ? 1 : size))
        return memory;
    throw std::bad_alloc{};
}

void* operator new[](const std::size_t size)
{
    return ::operator new(size);
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }

#if defined(_MSC_VER)
void* operator new(const std::size_t size, const std::align_val_t alignment)
{
    recordRealtimeAllocation();
    if (auto* memory = _aligned_malloc(size == 0 ? 1 : size, static_cast<std::size_t>(alignment)))
        return memory;
    throw std::bad_alloc{};
}

void* operator new[](const std::size_t size, const std::align_val_t alignment)
{
    return ::operator new(size, alignment);
}

void operator delete(void* memory, std::align_val_t) noexcept { _aligned_free(memory); }
void operator delete[](void* memory, std::align_val_t) noexcept { _aligned_free(memory); }
void operator delete(void* memory, std::size_t, std::align_val_t) noexcept { _aligned_free(memory); }
void operator delete[](void* memory, std::size_t, std::align_val_t) noexcept { _aligned_free(memory); }
#endif

namespace
{
class ProbeNode final : public openrig::AudioNode
{
public:
    ~ProbeNode() override
    {
        destructions.fetch_add(1, std::memory_order_relaxed);
        if (openrig::realtime::RealtimeGuard::isRealtimeThread())
            realtimeDestructions.fetch_add(1, std::memory_order_relaxed);
    }

    void prepare(const openrig::PrepareSpec&) override {}
    void reset() noexcept override {}
    void setBypassed(const bool bypassed) noexcept override { bypassed_ = bypassed; }
    [[nodiscard]] std::span<const openrig::ParameterDescriptor> parameters() const noexcept override { return descriptors_; }
    bool setParameter(const openrig::ParameterIndex index, const openrig::Sample value) noexcept override
    {
        if (index != 0)
            return false;
        value_ = value;
        return true;
    }
    void process(const openrig::AudioBlockView& input, const openrig::AudioBlockView& output, const std::uint32_t frames) noexcept override
    {
        for (std::uint32_t frame = 0; frame < frames; ++frame)
            output.channel(0)[frame] = input.channel(0)[frame] * (bypassed_ ? 1.0F : value_);
    }

    inline static std::atomic<std::uint64_t> destructions{0};
    inline static std::atomic<std::uint64_t> realtimeDestructions{0};

private:
    inline static const std::array<openrig::ParameterDescriptor, 1> descriptors_{{
        {"value", "Value", "", openrig::ParameterKind::Continuous, 0.0F, 1.0F, 1.0F, openrig::ParameterCurve::Linear, 0.0F},
    }};
    openrig::Sample value_ = 1.0F;
    bool bypassed_ = false;
};

std::unique_ptr<openrig::AudioNode> makeProbeNode()
{
    return std::make_unique<ProbeNode>();
}

std::unique_ptr<openrig::graph::CompiledGraph> compileProbeGraph(const openrig::Sample value)
{
    openrig::graph::NodeRegistry registry;
    static_cast<void>(registry.registerNode("probe", &makeProbeNode));
    const openrig::graph::GraphDescription description{
        .nodes = {{1, "probe", false, {{"value", value}}}},
        .connections = {},
    };
    openrig::graph::GraphCompiler compiler;
    auto result = compiler.compile(description, registry, {48'000.0, 16, 1, 1});
    return std::move(result.graph);
}
} // namespace

int main()
{
    using openrig::graph::GraphProcessStatus;
    TestSuite tests;
    constexpr std::uint32_t frames = 16;
    const openrig::PrepareSpec spec{48'000.0, frames, 1, 1};
    std::array<openrig::Sample, frames> input{};
    std::array<openrig::Sample, frames> output{};
    input.fill(1.0F);
    std::array<openrig::Sample*, 1> inputChannels{input.data()};
    std::array<openrig::Sample*, 1> outputChannels{output.data()};
    const openrig::AudioBlockView inputView{inputChannels.data(), 1, frames};
    const openrig::AudioBlockView outputView{outputChannels.data(), 1, frames};

    realtimeAllocations.store(0, std::memory_order_relaxed);
    int* allocationProbe = nullptr;
    {
        const openrig::realtime::RealtimeScope scope;
        allocationProbe = new int{42};
    }
    delete allocationProbe;
    tests.check(realtimeAllocations.load(std::memory_order_relaxed) == 1, "Realtime allocation hook detects new inside RealtimeScope");

    ProbeNode::destructions.store(0, std::memory_order_relaxed);
    ProbeNode::realtimeDestructions.store(0, std::memory_order_relaxed);
    realtimeAllocations.store(0, std::memory_order_relaxed);
    {
        openrig::AudioEngine engine(spec, 1);
        tests.check(engine.process(inputView, outputView, frames) == GraphProcessStatus::Ok && near(output.front(), 1.0F), "Engine safely passes through without an active graph");
        tests.check(!openrig::realtime::RealtimeGuard::isRealtimeThread(), "RealtimeScope ends with the callback");

        tests.check(engine.tryPublishGraph(compileProbeGraph(0.25F)), "First graph publication succeeds");
        tests.check(engine.process(inputView, outputView, frames) == GraphProcessStatus::Ok && near(output.front(), 0.25F), "Published graph activates at the next block boundary");
        tests.check(engine.tryPublishGraph(compileProbeGraph(0.75F)), "Replacement graph publication succeeds");
        static_cast<void>(engine.process(inputView, outputView, frames));
        tests.check(near(output.front(), 0.75F), "Replacement graph runs after the boundary swap");
        tests.check(ProbeNode::destructions.load(std::memory_order_relaxed) == 0, "Old graph is not destroyed by the audio callback");
        tests.check(engine.drainRetiredGraphs() == 1 && ProbeNode::destructions.load(std::memory_order_relaxed) == 1, "Control thread drains and destroys the retired graph");

        tests.check(engine.enqueueParameter({{1, 0}, 0.20F}) && engine.enqueueParameter({{1, 0}, 0.80F}), "Parameter events enter the engine queue");
        static_cast<void>(engine.process(inputView, outputView, frames));
        tests.check(near(output.front(), 0.20F), "Per-block parameter budget limits consumption");
        static_cast<void>(engine.process(inputView, outputView, frames));
        tests.check(near(output.front(), 0.80F), "Deferred parameter event is applied on the next block");
    }
    tests.check(ProbeNode::realtimeDestructions.load(std::memory_order_relaxed) == 0, "Engine shutdown does not destroy a graph in RealtimeScope");

    {
        openrig::AudioEngine fullMailbox(spec);
        bool accepted = true;
        for (std::size_t index = 0; index + 1 < openrig::AudioEngine::kGraphQueueCapacity; ++index)
            accepted = accepted && fullMailbox.tryPublishGraph(compileProbeGraph(1.0F));
        tests.check(accepted, "Publish mailbox accepts every usable slot");
        tests.check(!fullMailbox.tryPublishGraph(compileProbeGraph(1.0F)) && fullMailbox.rejectedPublishCount() == 1, "Full publish mailbox rejects without blocking");
    }

    ProbeNode::realtimeDestructions.store(0, std::memory_order_relaxed);
    {
        openrig::AudioEngine retirePressure(spec);
        for (std::size_t index = 0; index < openrig::AudioEngine::kGraphQueueCapacity + 2; ++index)
        {
            tests.check(retirePressure.tryPublishGraph(compileProbeGraph(1.0F)), "Retire pressure publication succeeds");
            static_cast<void>(retirePressure.process(inputView, outputView, frames));
        }
        tests.check(retirePressure.retireBackpressureCount() > 0, "Full retire queue exposes backpressure");
        tests.check(ProbeNode::realtimeDestructions.load(std::memory_order_relaxed) == 0, "Full retire queue never falls back to audio-thread destruction");
        tests.check(retirePressure.drainRetiredGraphs() == openrig::AudioEngine::kGraphQueueCapacity - 1, "Control thread drains the full retire queue");
        static_cast<void>(retirePressure.process(inputView, outputView, frames));
        tests.check(retirePressure.drainRetiredGraphs() >= 1, "Deferred retirement resumes after control-thread drain");
    }

    ProbeNode::realtimeDestructions.store(0, std::memory_order_relaxed);
    {
        openrig::AudioEngine stressed(spec);
        tests.check(stressed.tryPublishGraph(compileProbeGraph(1.0F)), "Stress graph publication succeeds");
        for (std::uint32_t block = 0; block < 100'000; ++block)
        {
            if (block != 0 && block % 1'000 == 0)
                tests.check(stressed.tryPublishGraph(compileProbeGraph(block % 2 == 0 ? 0.5F : 1.0F)), "Stress replacement publication succeeds");
            tests.check(stressed.enqueueParameter({{1, 0}, block % 2 == 0 ? 0.25F : 0.75F}), "Stress parameter enqueue succeeds");
            tests.check(stressed.process(inputView, outputView, frames) == GraphProcessStatus::Ok, "Stress block processes successfully");
            static_cast<void>(stressed.drainRetiredGraphs());
        }
    }
    tests.check(ProbeNode::realtimeDestructions.load(std::memory_order_relaxed) == 0, "100,000-block stress never destroys a graph on the audio path");
    tests.check(realtimeAllocations.load(std::memory_order_relaxed) == 0, "100,000-block stress performs zero realtime heap allocations");

    return tests.finish("audio engine tests");
}
