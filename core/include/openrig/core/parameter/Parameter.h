#pragma once

#include "openrig/core/Types.h"

#include <string>

namespace openrig
{
enum class ParameterCurve
{
    Linear,
    Logarithmic,
    Decibel,
};

enum class ParameterKind
{
    Continuous,
    Integer,
    Boolean,
    Choice,
};

struct ParameterDescriptor
{
    std::string key;
    std::string name;
    std::string unit;
    ParameterKind kind = ParameterKind::Continuous;
    Sample minimum = 0.0F;
    Sample maximum = 1.0F;
    Sample defaultValue = 0.0F;
    ParameterCurve curve = ParameterCurve::Linear;
    Sample smoothingTimeMs = 0.0F;
};

struct ParameterAddress
{
    NodeId node = kInvalidNodeId;
    ParameterIndex parameter = 0;
};

struct ParameterEvent
{
    ParameterAddress address;
    Sample value = 0.0F;
};
} // namespace openrig
