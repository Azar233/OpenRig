#include "test_support.h"

#include "openrig/core/graph/NodeRegistry.h"
#include "openrig/core/realtime/SpscQueue.h"
#include "openrig/dsp/drive/SoftClipNode.h"
#include "openrig/dsp/utility/GainNode.h"

#include <array>

int main()
{
    TestSuite tests;

    openrig::realtime::SpscQueue<int, 4> queue;
    int value = 0;
    tests.check(queue.tryPush(42), "SPSC accepts an element");
    tests.check(queue.tryPop(value) && value == 42, "SPSC preserves values");
    tests.check(!queue.tryPop(value), "SPSC empty state is observable");

    constexpr std::uint32_t frames = 10;
    std::array<openrig::Sample, frames> input{};
    std::array<openrig::Sample, frames> output{};
    input.fill(1.0F);
    std::array<openrig::Sample*, 1> inputChannels{input.data()};
    std::array<openrig::Sample*, 1> outputChannels{output.data()};
    const openrig::AudioBlockView inputView{inputChannels.data(), 1, frames};
    const openrig::AudioBlockView outputView{outputChannels.data(), 1, frames};

    openrig::dsp::GainNode gain;
    gain.prepare({1'000.0, frames, 1, 1});
    tests.check(gain.parameters().size() == 1 && gain.parameters().front().key == "gain", "Gain exposes a stable descriptor");
    tests.check(gain.capabilities().inputChannels == 1 && gain.capabilities().outputChannels == 1, "Gain declares mono capabilities");
    gain.process(inputView, outputView, frames);
    tests.check(near(output.front(), 1.0F) && near(output.back(), 1.0F), "Gain defaults to unity");
    tests.check(gain.setParameter(openrig::dsp::GainNode::kGain, 0.0F), "Gain accepts its parameter");
    gain.process(inputView, outputView, frames);
    tests.check(near(output.front(), 0.9F) && near(output.back(), 0.0F), "Gain smoothing lasts exactly 10 ms");
    gain.reset();
    gain.process(inputView, outputView, frames);
    tests.check(near(output.front(), 0.0F), "Gain reset restores its current target");

    openrig::dsp::SoftClipNode clip;
    clip.prepare({1'000.0, frames, 1, 1});
    clip.setBypassed(true);
    clip.process(inputView, outputView, frames);
    tests.check(near(output.front(), input.front()) && near(output.back(), input.back()), "SoftClip bypass is transparent");
    clip.setBypassed(false);
    input.fill(0.05F);
    tests.check(clip.setParameter(openrig::dsp::SoftClipNode::kDrive, 20.0F), "SoftClip accepts Drive");
    clip.process(inputView, outputView, frames);
    tests.check(output.front() < output.back() && near(output.back(), std::tanh(1.0F) * 0.75F), "Drive smoothing lasts exactly 10 ms");

    openrig::graph::NodeRegistry registry;
    tests.check(registry.registerNode("gain", &openrig::dsp::makeGainNode), "Registry accepts a valid node");
    tests.check(!registry.registerNode("gain", &openrig::dsp::makeGainNode), "Registry rejects duplicate types");
    tests.check(!registry.contains("missing") && registry.create("missing") == nullptr, "Registry reports unknown types");

    return tests.finish("node tests");
}
