#pragma once

#include "openrig/core/Types.h"
#include "openrig/core/audio/PrepareSpec.h"
#include "openrig/core/graph/GraphCompiler.h"

#include <string_view>

namespace openrig::dsp
{
inline constexpr std::string_view kGainNodeType = "openrig.utility.gain";
inline constexpr std::string_view kSoftClipNodeType = "openrig.drive.soft_clip";

inline constexpr NodeId kDefaultInputGainNodeId = 1;
inline constexpr NodeId kDefaultDriveNodeId = 2;
inline constexpr NodeId kDefaultOutputGainNodeId = 3;

[[nodiscard]] graph::GraphCompileResult compileDefaultGraph(const PrepareSpec& spec);
} // namespace openrig::dsp
