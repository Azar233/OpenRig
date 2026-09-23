#include "test_support.h"

#include "openrig/dsp/DefaultGraph.h"

#include <array>
#include <cmath>

int main()
{
    TestSuite tests;
    constexpr std::uint32_t frames = 32;
    const openrig::PrepareSpec spec{48'000.0, frames, 1, 1};

    auto result = openrig::dsp::compileDefaultGraph(spec);
    tests.check(result && result.graph->nodeCount() == 3, "Default Gain-SoftClip-Gain graph compiles with three nodes");

    std::array<openrig::Sample, frames> input{};
    std::array<openrig::Sample, frames> output{};
    input.fill(0.5F);
    std::array<openrig::Sample*, 1> inputChannels{input.data()};
    std::array<openrig::Sample*, 1> outputChannels{output.data()};

    const auto status = result.graph->process(
        {inputChannels.data(), 1, frames},
        {outputChannels.data(), 1, frames},
        frames);
    const auto expected = std::tanh(1.0F) * 0.75F;
    tests.check(status == openrig::graph::GraphProcessStatus::Ok, "Default graph processes a mono block");
    tests.check(near(output.front(), expected) && near(output.back(), expected), "Default graph applies SoftClip instead of dry passthrough");
    tests.check(
        result.graph->applyParameter({{openrig::dsp::kDefaultDriveNodeId, 0}, 4.0F}),
        "Default Drive parameter is addressable by stable NodeId");

    const auto stereoResult = openrig::dsp::compileDefaultGraph({48'000.0, frames, 1, 2});
    tests.check(!stereoResult, "Default Core graph rejects implicit mono-to-stereo conversion");

    return tests.finish("default graph tests");
}
