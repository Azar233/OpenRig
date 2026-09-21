#pragma once

#include <cstdint>
#include <limits>

namespace openrig
{
using Sample = float;
using NodeId = std::uint64_t;
using ParameterIndex = std::uint32_t;

inline constexpr NodeId kInvalidNodeId = 0;
inline constexpr std::uint32_t kInfiniteTail = std::numeric_limits<std::uint32_t>::max();
} // namespace openrig
