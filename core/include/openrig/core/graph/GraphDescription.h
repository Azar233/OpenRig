#pragma once

#include "openrig/core/Types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace openrig::graph
{
struct NodeDescription
{
    NodeId id = kInvalidNodeId;
    std::string type;
    bool bypassed = false;
    std::unordered_map<std::string, Sample> parameters;
};

struct ConnectionDescription
{
    NodeId source = kInvalidNodeId;
    NodeId destination = kInvalidNodeId;
};

struct GraphDescription
{
    std::vector<NodeDescription> nodes;
    std::vector<ConnectionDescription> connections;
};
} // namespace openrig::graph
