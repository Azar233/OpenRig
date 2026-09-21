#include "openrig/core/graph/GraphCompiler.h"
#include "openrig/core/graph/NodeRegistry.h"
#include "openrig/core/realtime/SpscQueue.h"
#include "openrig/dsp/drive/SoftClipNode.h"
#include "openrig/dsp/utility/GainNode.h"

#include <array>
#include <cmath>
#include <iostream>
#include <string_view>

namespace
{
int failures = 0;

void check(const bool condition, const std::string_view message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void testSpscQueue()
{
    openrig::realtime::SpscQueue<int, 4> queue;
    int value = 0;
    check(queue.tryPush(42), "SPSC accepts an element");
    check(queue.tryPop(value), "SPSC returns an element");
    check(value == 42, "SPSC preserves values");
    check(!queue.tryPop(value), "SPSC empty state is observable");
}

void testGainNode()
{
    constexpr std::uint32_t frames = 4;
    std::array<openrig::Sample, frames> input{0.25F, -0.25F, 0.5F, -0.5F};
    std::array<openrig::Sample, frames> output{};
    std::array<openrig::Sample*, 1> inputChannels{input.data()};
    std::array<openrig::Sample*, 1> outputChannels{output.data()};

    openrig::dsp::GainNode gain;
    gain.prepare({48'000.0, frames, 1, 1});
    gain.process({inputChannels.data(), 1, frames}, {outputChannels.data(), 1, frames}, frames);

    for (std::uint32_t index = 0; index < frames; ++index)
        check(output[index] == input[index], "Gain defaults to unity");
}

void testLinearGraph()
{
    openrig::graph::NodeRegistry registry;
    check(registry.registerNode("openrig.utility.input_gain", &openrig::dsp::makeGainNode), "register gain node");
    check(registry.registerNode("openrig.drive.soft_clip", &openrig::dsp::makeSoftClipNode), "register clip node");

    openrig::graph::GraphDescription description{
        .nodes = {
            {1, "openrig.utility.input_gain", false, {}},
            {2, "openrig.drive.soft_clip", false, {}},
        },
        .connections = {{1, 2}},
    };

    openrig::graph::GraphCompiler compiler;
    auto result = compiler.compile(description, registry, {48'000.0, 128, 1, 1});
    check(static_cast<bool>(result), "linear graph compiles");
    check(result.graph && result.graph->nodeCount() == 2, "compiled graph owns both nodes");

    description.connections.push_back({2, 1});
    result = compiler.compile(description, registry, {48'000.0, 128, 1, 1});
    check(!result, "cycle or non-linear connection is rejected");
}
} // namespace

int main()
{
    testSpscQueue();
    testGainNode();
    testLinearGraph();

    if (failures == 0)
        std::cout << "All OpenRig skeleton tests passed.\n";
    return failures == 0 ? 0 : 1;
}
