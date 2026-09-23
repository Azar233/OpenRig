#include "test_support.h"

#include "openrig/core/graph/GraphCompiler.h"
#include "openrig/dsp/drive/SoftClipNode.h"
#include "openrig/dsp/utility/GainNode.h"

#include <algorithm>
#include <array>
#include <memory>

namespace
{
class MonoToStereoNode final : public openrig::AudioNode
{
public:
    void prepare(const openrig::PrepareSpec&) override {}
    void reset() noexcept override {}
    void setBypassed(bool) noexcept override {}
    [[nodiscard]] openrig::NodeCapabilities capabilities() const noexcept override { return {false, 1, 2}; }
    [[nodiscard]] std::span<const openrig::ParameterDescriptor> parameters() const noexcept override { return {}; }
    bool setParameter(openrig::ParameterIndex, openrig::Sample) noexcept override { return false; }
    void process(const openrig::AudioBlockView& input, const openrig::AudioBlockView& output, const std::uint32_t frames) noexcept override
    {
        for (std::uint32_t channel = 0; channel < output.numChannels; ++channel)
            std::copy_n(input.channel(0), frames, output.channel(channel));
    }
};

class MetricsNode final : public openrig::AudioNode
{
public:
    void prepare(const openrig::PrepareSpec&) override {}
    void reset() noexcept override {}
    void setBypassed(bool) noexcept override {}
    [[nodiscard]] std::span<const openrig::ParameterDescriptor> parameters() const noexcept override { return {}; }
    bool setParameter(openrig::ParameterIndex, openrig::Sample) noexcept override { return false; }
    [[nodiscard]] std::uint32_t latencySamples() const noexcept override { return 5; }
    [[nodiscard]] std::uint32_t tailSamples() const noexcept override { return 7; }
    void process(const openrig::AudioBlockView& input, const openrig::AudioBlockView& output, const std::uint32_t frames) noexcept override
    {
        std::copy_n(input.channel(0), frames, output.channel(0));
    }
};

std::unique_ptr<openrig::AudioNode> makeMonoToStereoNode() { return std::make_unique<MonoToStereoNode>(); }
std::unique_ptr<openrig::AudioNode> makeMetricsNode() { return std::make_unique<MetricsNode>(); }

openrig::graph::NodeRegistry makeRegistry()
{
    openrig::graph::NodeRegistry registry;
    static_cast<void>(registry.registerNode("gain", &openrig::dsp::makeGainNode));
    static_cast<void>(registry.registerNode("clip", &openrig::dsp::makeSoftClipNode));
    static_cast<void>(registry.registerNode("mono-to-stereo", &makeMonoToStereoNode));
    static_cast<void>(registry.registerNode("metrics", &makeMetricsNode));
    return registry;
}
} // namespace

int main()
{
    using openrig::graph::GraphDescription;
    using openrig::graph::GraphProcessStatus;
    TestSuite tests;
    const openrig::graph::GraphCompiler compiler;

    constexpr std::uint32_t maximumFrames = 64;
    std::array<openrig::Sample, maximumFrames + 1> input{};
    std::array<openrig::Sample, maximumFrames + 1> output{};
    input.fill(1.0F);
    std::array<openrig::Sample*, 1> inputChannels{input.data()};
    std::array<openrig::Sample*, 1> outputChannels{output.data()};

    auto empty = compiler.compile({}, makeRegistry(), {48'000.0, maximumFrames, 1, 1});
    tests.check(empty && empty.graph->process({inputChannels.data(), 1, maximumFrames}, {outputChannels.data(), 1, maximumFrames}, maximumFrames) == GraphProcessStatus::Ok, "Empty graph processes");
    tests.check(near(output.front(), 1.0F) && near(output[maximumFrames - 1], 1.0F), "Empty graph is transparent");

    GraphDescription gainDescription{.nodes = {{1, "gain", false, {{"gain", 0.25F}}}}, .connections = {}};
    auto gain = compiler.compile(gainDescription, makeRegistry(), {48'000.0, maximumFrames, 1, 1});
    output.fill(0.0F);
    tests.check(gain && gain.graph->process({inputChannels.data(), 1, maximumFrames}, {outputChannels.data(), 1, maximumFrames}, maximumFrames) == GraphProcessStatus::Ok, "Persisted parameter graph processes");
    tests.check(near(output.front(), 0.25F) && near(output[maximumFrames - 1], 0.25F), "Persisted parameter is applied before first block");

    gainDescription.nodes.front().bypassed = true;
    auto bypassed = compiler.compile(gainDescription, makeRegistry(), {48'000.0, maximumFrames, 1, 1});
    static_cast<void>(bypassed.graph->process({inputChannels.data(), 1, maximumFrames}, {outputChannels.data(), 1, maximumFrames}, maximumFrames));
    tests.check(near(output.front(), 1.0F), "Compiled bypass state is applied");

    GraphDescription chain{
        .nodes = {{1, "gain", false, {}}, {2, "clip", false, {}}, {3, "gain", false, {}}},
        .connections = {{1, 2}, {2, 3}},
    };
    auto graph = compiler.compile(chain, makeRegistry(), {48'000.0, maximumFrames, 1, 1});
    for (const auto frames : {1U, 7U, 31U, maximumFrames})
    {
        output.fill(-9.0F);
        tests.check(graph.graph->process({inputChannels.data(), 1, maximumFrames}, {outputChannels.data(), 1, maximumFrames}, frames) == GraphProcessStatus::Ok, "Gain-SoftClip-Gain accepts variable block sizes");
        tests.check(std::all_of(output.begin(), output.begin() + frames, [](const auto sample) { return std::isfinite(sample); }), "Graph output remains finite");
    }

    tests.check(graph.graph->applyParameter({{2, openrig::dsp::SoftClipNode::kLevel}, 0.0F}), "Parameter routes to the requested node");
    tests.check(!graph.graph->applyParameter({{99, 0}, 1.0F}), "Unknown NodeId is rejected without recompiling");
    tests.check(!graph.graph->applyParameter({{2, 99}, 1.0F}), "Unknown ParameterIndex is rejected");

    output.fill(4.0F);
    const auto overMaximum = graph.graph->process({inputChannels.data(), 1, maximumFrames + 1}, {outputChannels.data(), 1, maximumFrames + 1}, maximumFrames + 1);
    tests.check(overMaximum == GraphProcessStatus::BlockTooLarge, "Over-max block reports a safe status");
    tests.check(std::all_of(output.begin(), output.end(), [](const auto sample) { return sample == 0.0F; }), "Over-max block clears output");

    GraphDescription stereoDescription{
        .nodes = {{1, "gain", false, {}}, {2, "mono-to-stereo", false, {}}},
        .connections = {{1, 2}},
    };
    auto stereo = compiler.compile(stereoDescription, makeRegistry(), {48'000.0, maximumFrames, 1, 2});
    std::array<openrig::Sample, maximumFrames> left{};
    std::array<openrig::Sample, maximumFrames> right{};
    std::array<openrig::Sample*, 2> stereoChannels{left.data(), right.data()};
    tests.check(stereo && stereo.graph->process({inputChannels.data(), 1, maximumFrames}, {stereoChannels.data(), 2, maximumFrames}, maximumFrames) == GraphProcessStatus::Ok, "Explicit mono-to-stereo node compiles and runs");
    tests.check(near(left.front(), 1.0F) && near(right.back(), 1.0F), "Mono-to-stereo node controls channel expansion");

    GraphDescription metricsDescription{
        .nodes = {{1, "metrics", false, {}}, {2, "metrics", false, {}}},
        .connections = {{1, 2}},
    };
    auto metrics = compiler.compile(metricsDescription, makeRegistry(), {48'000.0, maximumFrames, 1, 1});
    tests.check(metrics && metrics.graph->latencySamples() == 10 && metrics.graph->tailSamples() == 14, "Serial latency and tail are aggregated");

    return tests.finish("graph execution tests");
}
