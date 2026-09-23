#include "test_support.h"

#include "openrig/core/graph/GraphCompiler.h"
#include "openrig/framework/iplug/IPlugAudioAdapter.h"
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

std::unique_ptr<openrig::AudioNode> makeMonoToStereoNode()
{
    return std::make_unique<MonoToStereoNode>();
}

std::unique_ptr<openrig::graph::CompiledGraph> compileGainGraph(const openrig::PrepareSpec& spec, const float gain)
{
    openrig::graph::NodeRegistry registry;
    static_cast<void>(registry.registerNode("gain", &openrig::dsp::makeGainNode));
    const openrig::graph::GraphDescription description{
        .nodes = {{1, "gain", false, {{"gain", gain}}}},
        .connections = {},
    };
    openrig::graph::GraphCompiler compiler;
    return compiler.compile(description, registry, spec).graph;
}

std::unique_ptr<openrig::graph::CompiledGraph> compileStereoGraph(const openrig::PrepareSpec& spec)
{
    openrig::graph::NodeRegistry registry;
    static_cast<void>(registry.registerNode("mono-to-stereo", &makeMonoToStereoNode));
    const openrig::graph::GraphDescription description{
        .nodes = {{1, "mono-to-stereo", false, {}}},
        .connections = {},
    };
    openrig::graph::GraphCompiler compiler;
    return compiler.compile(description, registry, spec).graph;
}
} // namespace

int main()
{
    using openrig::graph::GraphProcessStatus;
    using openrig::framework::iplug::IPlugAudioAdapter;
    TestSuite tests;
    constexpr std::uint32_t maximumFrames = 32;

    const openrig::PrepareSpec monoSpec{48'000.0, maximumFrames, 1, 1};
    openrig::AudioEngine monoEngine(monoSpec);
    IPlugAudioAdapter monoAdapter;
    monoAdapter.prepare(monoEngine, monoSpec);

    std::array<float, maximumFrames + 1> floatInput{};
    std::array<float, maximumFrames + 1> floatOutput{};
    floatInput.fill(0.5F);
    std::array<float*, 1> floatInputs{floatInput.data()};
    std::array<float*, 1> floatOutputs{floatOutput.data()};
    for (const auto frames : {1U, 7U, maximumFrames})
    {
        floatOutput.fill(-1.0F);
        tests.check(monoAdapter.process(floatInputs.data(), 1, floatOutputs.data(), 1, frames) == GraphProcessStatus::Ok, "Float adapter accepts variable block sizes");
        tests.check(near(floatOutput[frames - 1], 0.5F), "Float adapter preserves planar samples");
    }

    tests.check(monoEngine.tryPublishGraph(compileGainGraph(monoSpec, 0.25F)), "Gain graph publishes for conversion test");
    std::array<double, maximumFrames> doubleInput{};
    std::array<double, maximumFrames> doubleOutput{};
    doubleInput.fill(0.8);
    std::array<double*, 1> doubleInputs{doubleInput.data()};
    std::array<double*, 1> doubleOutputs{doubleOutput.data()};
    tests.check(monoAdapter.process(doubleInputs.data(), 1, doubleOutputs.data(), 1, maximumFrames) == GraphProcessStatus::Ok, "Double host samples process through float32 Core");
    tests.check(near(static_cast<float>(doubleOutput.front()), 0.2F), "Double-to-float32-to-double conversion preserves expected gain");

    doubleOutput.fill(2.0);
    tests.check(monoAdapter.process<double>(nullptr, 0, doubleOutputs.data(), 1, maximumFrames) == GraphProcessStatus::Ok, "Zero input channels are accepted safely");
    tests.check(near(static_cast<float>(doubleOutput.front()), 0.0F), "Missing input is mapped to silence");

    const openrig::PrepareSpec stereoSpec{48'000.0, maximumFrames, 1, 2};
    openrig::AudioEngine stereoEngine(stereoSpec);
    IPlugAudioAdapter stereoAdapter;
    stereoAdapter.prepare(stereoEngine, stereoSpec);
    tests.check(stereoEngine.tryPublishGraph(compileStereoGraph(stereoSpec)), "Mono-to-stereo graph publishes");
    std::array<double, maximumFrames> left{};
    std::array<double, maximumFrames> right{};
    std::array<double, maximumFrames> extra{};
    std::array<double*, 3> stereoOutputs{left.data(), right.data(), extra.data()};
    tests.check(stereoAdapter.process(doubleInputs.data(), 1, stereoOutputs.data(), 3, maximumFrames) == GraphProcessStatus::Ok, "Adapter maps mono Core input to stereo host output");
    tests.check(near(static_cast<float>(left.front()), 0.8F) && near(static_cast<float>(right.back()), 0.8F), "Both stereo channels receive Core output");
    tests.check(near(static_cast<float>(extra.front()), 0.0F), "Extra host output channels are cleared");

    openrig::AudioEngine duplicatedMonoEngine(monoSpec);
    IPlugAudioAdapter duplicatedMonoAdapter;
    duplicatedMonoAdapter.prepare(
        duplicatedMonoEngine,
        monoSpec,
        openrig::framework::iplug::OutputChannelPolicy::DuplicateMono);
    left.fill(0.0);
    right.fill(0.0);
    std::array<double*, 2> duplicatedOutputs{left.data(), right.data()};
    tests.check(
        duplicatedMonoAdapter.process(doubleInputs.data(), 1, duplicatedOutputs.data(), 2, maximumFrames) == GraphProcessStatus::Ok,
        "Adapter accepts explicit mono duplication policy");
    tests.check(
        near(static_cast<float>(left.front()), 0.8F) && near(static_cast<float>(right.back()), 0.8F),
        "Explicit policy duplicates mono Core output to stereo Host channels");

    floatOutput.fill(3.0F);
    const auto oversized = monoAdapter.process(floatInputs.data(), 1, floatOutputs.data(), 1, maximumFrames + 1);
    tests.check(oversized == GraphProcessStatus::BlockTooLarge, "Over-max host block reports a safe status");
    tests.check(std::all_of(floatOutput.begin(), floatOutput.end(), [](const auto sample) { return sample == 0.0F; }), "Over-max host block clears output");

    IPlugAudioAdapter unprepared;
    floatOutput.fill(4.0F);
    tests.check(unprepared.process(floatInputs.data(), 1, floatOutputs.data(), 1, maximumFrames) == GraphProcessStatus::InvalidBuffer, "Unprepared adapter rejects processing");
    tests.check(floatOutput.front() == 0.0F, "Unprepared adapter clears output");

    return tests.finish("iPlug adapter tests");
}
