#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "openrig/framework/iplug/IPlugAudioAdapter.h"

#include <memory>

class OpenRigStandalone final : public iplug::Plugin
{
public:
    explicit OpenRigStandalone(const iplug::InstanceInfo& info);

#if IPLUG_DSP
    void OnReset() override;
    void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) override;
#endif

private:
    std::unique_ptr<openrig::AudioEngine> engine_;
    openrig::framework::iplug::IPlugAudioAdapter adapter_;
};
