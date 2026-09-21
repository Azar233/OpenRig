#include "openrig/core/audio/AudioBlockView.h"
#include "openrig/core/audio/PrepareSpec.h"
#include "openrig/dsp/utility/GainNode.h"

#include <array>
#include <iostream>

int main()
{
    constexpr std::uint32_t frames = 8;
    std::array<openrig::Sample, frames> input{0.0F, 0.1F, 0.2F, 0.3F, 0.4F, 0.5F, 0.6F, 0.7F};
    std::array<openrig::Sample, frames> output{};
    std::array<openrig::Sample*, 1> inputChannels{input.data()};
    std::array<openrig::Sample*, 1> outputChannels{output.data()};

    openrig::dsp::GainNode gain;
    gain.prepare({48'000.0, frames, 1, 1});
    gain.process(
        {inputChannels.data(), 1, frames},
        {outputChannels.data(), 1, frames},
        frames);

    std::cout << "OpenRig DSP sandbox: " << output.back() << '\n';
    return 0;
}
