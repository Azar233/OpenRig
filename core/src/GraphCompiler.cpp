#include "openrig/core/graph/GraphCompiler.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace openrig::graph
{
GraphCompileResult GraphCompiler::compile(
    const GraphDescription& description,
    const NodeRegistry& registry,
    const PrepareSpec& spec) const
{
    auto compiled = std::make_unique<CompiledGraph>();
    compiled->spec_ = spec;

    if (description.nodes.empty())
        return {std::move(compiled), {}};

    std::unordered_map<NodeId, const NodeDescription*> nodesById;
    std::unordered_map<NodeId, NodeId> nextById;
    std::unordered_map<NodeId, std::uint32_t> incomingCount;

    for (const auto& node : description.nodes)
    {
        if (node.id == kInvalidNodeId)
            return {nullptr, "node id 0 is reserved"};
        if (!nodesById.emplace(node.id, &node).second)
            return {nullptr, "duplicate node id"};
        if (!registry.contains(node.type))
            return {nullptr, "unregistered node type: " + node.type};
        incomingCount[node.id] = 0;
    }

    if (description.connections.size() + 1 != description.nodes.size())
        return {nullptr, "v0.1 graphs must be one connected linear chain"};

    for (const auto& connection : description.connections)
    {
        if (!nodesById.contains(connection.source) || !nodesById.contains(connection.destination))
            return {nullptr, "connection references a missing node"};
        if (connection.source == connection.destination)
            return {nullptr, "self connection is not allowed"};
        if (!nextById.emplace(connection.source, connection.destination).second)
            return {nullptr, "branching is not supported in v0.1"};
        if (++incomingCount[connection.destination] > 1)
            return {nullptr, "merging is not supported in v0.1"};
    }

    std::vector<NodeId> starts;
    for (const auto& [id, count] : incomingCount)
        if (count == 0)
            starts.push_back(id);

    if (starts.size() != 1)
        return {nullptr, "graph must have exactly one input node"};

    std::vector<NodeId> orderedIds;
    std::unordered_set<NodeId> visited;
    auto current = starts.front();
    while (true)
    {
        if (!visited.insert(current).second)
            return {nullptr, "cycle detected"};
        orderedIds.push_back(current);

        const auto next = nextById.find(current);
        if (next == nextById.end())
            break;
        current = next->second;
    }

    if (orderedIds.size() != description.nodes.size())
        return {nullptr, "graph is disconnected or cyclic"};

    compiled->nodes_.reserve(orderedIds.size());
    for (const auto id : orderedIds)
    {
        const auto* descriptionNode = nodesById.at(id);
        auto node = registry.create(descriptionNode->type);
        if (!node)
            return {nullptr, "node factory failed: " + descriptionNode->type};
        node->prepare(spec);
        node->setBypassed(descriptionNode->bypassed);
        compiled->latencySamples_ += node->latencySamples();
        compiled->nodes_.push_back(std::move(node));
    }

    const auto channelCount = std::max(spec.numInputChannels, spec.numOutputChannels);
    const auto sampleCount = static_cast<std::size_t>(channelCount) * spec.maxBlockSize;
    compiled->scratchA_.resize(sampleCount);
    compiled->scratchB_.resize(sampleCount);
    compiled->channelsA_.resize(channelCount);
    compiled->channelsB_.resize(channelCount);

    for (std::uint32_t channel = 0; channel < channelCount; ++channel)
    {
        const auto offset = static_cast<std::size_t>(channel) * spec.maxBlockSize;
        compiled->channelsA_[channel] = compiled->scratchA_.data() + offset;
        compiled->channelsB_[channel] = compiled->scratchB_.data() + offset;
    }

    return {std::move(compiled), {}};
}
} // namespace openrig::graph
