#include "openrig/core/graph/GraphCompiler.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace openrig::graph
{
namespace
{
GraphCompileResult fail(const GraphCompileErrorCode code, std::string message)
{
    return {nullptr, code, std::move(message)};
}

std::uint32_t saturatingAdd(const std::uint32_t left, const std::uint32_t right) noexcept
{
    constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
    return left > maximum - right ? maximum : left + right;
}
} // namespace

GraphCompileResult GraphCompiler::compile(
    const GraphDescription& description,
    const NodeRegistry& registry,
    const PrepareSpec& spec) const
{
    if (!std::isfinite(spec.sampleRate) || spec.sampleRate <= 0.0 || spec.maxBlockSize == 0 || spec.numInputChannels == 0 ||
        spec.numOutputChannels == 0)
        return fail(GraphCompileErrorCode::InvalidPrepareSpec, "invalid prepare specification");

    auto compiled = std::make_unique<CompiledGraph>();
    compiled->spec_ = spec;

    if (description.nodes.empty())
    {
        if (!description.connections.empty())
            return fail(GraphCompileErrorCode::MissingConnectionEndpoint, "empty graph contains a connection");
        if (spec.numInputChannels != spec.numOutputChannels)
            return fail(GraphCompileErrorCode::UnsupportedChannelLayout, "empty graph requires matching input and output channels");
        return {std::move(compiled), GraphCompileErrorCode::None, {}};
    }

    std::unordered_map<NodeId, const NodeDescription*> nodesById;
    std::unordered_map<NodeId, NodeId> nextById;
    std::unordered_map<NodeId, std::uint32_t> incomingCount;

    for (const auto& node : description.nodes)
    {
        if (node.id == kInvalidNodeId)
            return fail(GraphCompileErrorCode::ReservedNodeId, "node id 0 is reserved");
        if (!nodesById.emplace(node.id, &node).second)
            return fail(GraphCompileErrorCode::DuplicateNodeId, "duplicate node id");
        if (!registry.contains(node.type))
            return fail(GraphCompileErrorCode::UnregisteredNodeType, "unregistered node type: " + node.type);
        incomingCount[node.id] = 0;
    }

    std::set<std::pair<NodeId, NodeId>> connections;
    for (const auto& connection : description.connections)
    {
        if (!nodesById.contains(connection.source) || !nodesById.contains(connection.destination))
            return fail(GraphCompileErrorCode::MissingConnectionEndpoint, "connection references a missing node");
        if (connection.source == connection.destination)
            return fail(GraphCompileErrorCode::SelfConnection, "self connection is not allowed");
        if (!connections.emplace(connection.source, connection.destination).second)
            return fail(GraphCompileErrorCode::DuplicateConnection, "duplicate connection");
        if (!nextById.emplace(connection.source, connection.destination).second)
            return fail(GraphCompileErrorCode::BranchingUnsupported, "branching is not supported in v0.1");
        if (++incomingCount[connection.destination] > 1)
            return fail(GraphCompileErrorCode::MergingUnsupported, "merging is not supported in v0.1");
    }

    auto remainingIncoming = incomingCount;
    std::queue<NodeId> ready;
    for (const auto& [id, count] : remainingIncoming)
        if (count == 0)
            ready.push(id);

    std::size_t sortedCount = 0;
    while (!ready.empty())
    {
        const auto id = ready.front();
        ready.pop();
        ++sortedCount;
        const auto next = nextById.find(id);
        if (next != nextById.end() && --remainingIncoming[next->second] == 0)
            ready.push(next->second);
    }
    if (sortedCount != description.nodes.size())
        return fail(GraphCompileErrorCode::Cycle, "cycle detected");

    std::vector<NodeId> starts;
    for (const auto& [id, count] : incomingCount)
        if (count == 0)
            starts.push_back(id);
    if (starts.size() != 1 || description.connections.size() + 1 != description.nodes.size())
        return fail(GraphCompileErrorCode::DisconnectedGraph, "graph must be one connected linear chain");

    std::vector<NodeId> orderedIds;
    auto current = starts.front();
    while (true)
    {
        orderedIds.push_back(current);
        const auto next = nextById.find(current);
        if (next == nextById.end())
            break;
        current = next->second;
    }
    if (orderedIds.size() != description.nodes.size())
        return fail(GraphCompileErrorCode::DisconnectedGraph, "graph is disconnected");

    compiled->nodes_.reserve(orderedIds.size());
    compiled->routes_.reserve(orderedIds.size());
    compiled->nodeOutputChannels_.reserve(orderedIds.size());
    auto currentChannels = spec.numInputChannels;

    for (const auto id : orderedIds)
    {
        const auto* descriptionNode = nodesById.at(id);
        auto node = registry.create(descriptionNode->type);
        if (!node)
            return fail(GraphCompileErrorCode::NodeFactoryFailed, "node factory failed: " + descriptionNode->type);

        const auto capabilities = node->capabilities();
        if (capabilities.inputChannels == 0 || capabilities.outputChannels == 0 ||
            capabilities.inputChannels != currentChannels)
            return fail(GraphCompileErrorCode::UnsupportedChannelLayout, "unsupported input channel layout at node: " + descriptionNode->type);

        node->prepare(spec);
        for (const auto& [key, value] : descriptionNode->parameters)
        {
            const auto descriptors = node->parameters();
            const auto descriptor = std::find_if(descriptors.begin(), descriptors.end(), [&](const auto& item) {
                return item.key == key;
            });
            if (descriptor == descriptors.end())
                return fail(GraphCompileErrorCode::UnknownParameter, "unknown parameter '" + key + "' on node: " + descriptionNode->type);
            const auto index = static_cast<ParameterIndex>(std::distance(descriptors.begin(), descriptor));
            if (!node->setParameter(index, value))
                return fail(GraphCompileErrorCode::UnknownParameter, "node rejected parameter '" + key + "': " + descriptionNode->type);
        }
        node->reset();
        node->setBypassed(descriptionNode->bypassed);

        currentChannels = capabilities.outputChannels;
        compiled->latencySamples_ = saturatingAdd(compiled->latencySamples_, node->latencySamples());
        compiled->tailSamples_ = saturatingAdd(compiled->tailSamples_, node->tailSamples());
        compiled->nodeOutputChannels_.push_back(currentChannels);
        compiled->routes_.push_back({id, node.get()});
        compiled->nodes_.push_back(std::move(node));
    }

    if (currentChannels != spec.numOutputChannels)
        return fail(GraphCompileErrorCode::UnsupportedChannelLayout, "compiled graph output channel layout is unsupported");

    std::sort(compiled->routes_.begin(), compiled->routes_.end(), [](const auto& left, const auto& right) {
        return left.id < right.id;
    });

    const auto channelCount = std::max(spec.numInputChannels, spec.numOutputChannels);
    const auto maximumNodeChannels = *std::max_element(compiled->nodeOutputChannels_.begin(), compiled->nodeOutputChannels_.end());
    const auto scratchChannels = std::max(channelCount, maximumNodeChannels);
    const auto sampleCount = static_cast<std::size_t>(scratchChannels) * spec.maxBlockSize;
    compiled->scratchA_.resize(sampleCount);
    compiled->scratchB_.resize(sampleCount);
    compiled->channelsA_.resize(scratchChannels);
    compiled->channelsB_.resize(scratchChannels);

    for (std::uint32_t channel = 0; channel < scratchChannels; ++channel)
    {
        const auto offset = static_cast<std::size_t>(channel) * spec.maxBlockSize;
        compiled->channelsA_[channel] = compiled->scratchA_.data() + offset;
        compiled->channelsB_[channel] = compiled->scratchB_.data() + offset;
    }

    return {std::move(compiled), GraphCompileErrorCode::None, {}};
}
} // namespace openrig::graph
