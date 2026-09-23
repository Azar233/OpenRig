#include "OpenRigStandalone.h"
#include "IPlug_include_in_plug_src.h"

#include <algorithm>
#include <cstdint>

OpenRigStandalone::OpenRigStandalone(const iplug::InstanceInfo& info)
    : iplug::Plugin(info, iplug::MakeConfig(0, 1))
{
}

#if IPLUG_DSP
void OpenRigStandalone::OnReset()
{
    const auto sampleRate = GetSampleRate() > 0.0 ? GetSampleRate() : 48'000.0;
    const auto maximumFrames = static_cast<std::uint32_t>(std::max(GetBlockSize(), 1));
    const openrig::PrepareSpec spec{sampleRate, maximumFrames, 1, 2};

    adapter_.reset();
    engine_ = std::make_unique<openrig::AudioEngine>(spec);
    adapter_.prepare(*engine_, spec);
}

void OpenRigStandalone::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, const int nFrames)
{
    if (nFrames <= 0)
        return;

    const auto inputChannels = static_cast<std::uint32_t>(std::max(NInChansConnected(), 0));
    const auto outputChannels = static_cast<std::uint32_t>(std::max(NOutChansConnected(), 0));
    static_cast<void>(adapter_.process(
        inputs,
        inputChannels,
        outputs,
        outputChannels,
        static_cast<std::uint32_t>(nFrames)));
}
#endif
