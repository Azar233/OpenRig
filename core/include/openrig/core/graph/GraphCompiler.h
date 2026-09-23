#pragma once

#include "openrig/core/audio/PrepareSpec.h"
#include "openrig/core/graph/CompiledGraph.h"
#include "openrig/core/graph/GraphDescription.h"
#include "openrig/core/graph/NodeRegistry.h"

#include <memory>
#include <string>

namespace openrig::graph
{
enum class GraphCompileErrorCode
{
    None,
    InvalidPrepareSpec,
    ReservedNodeId,
    DuplicateNodeId,
    UnregisteredNodeType,
    MissingConnectionEndpoint,
    DuplicateConnection,
    SelfConnection,
    BranchingUnsupported,
    MergingUnsupported,
    Cycle,
    DisconnectedGraph,
    UnsupportedChannelLayout,
    UnknownParameter,
    NodeFactoryFailed,
};

struct GraphCompileResult
{
    std::unique_ptr<CompiledGraph> graph;
    GraphCompileErrorCode code = GraphCompileErrorCode::None;
    std::string error;

    [[nodiscard]] explicit operator bool() const noexcept { return graph != nullptr; }
};

class GraphCompiler
{
public:
    [[nodiscard]] GraphCompileResult compile(
        const GraphDescription& description,
        const NodeRegistry& registry,
        const PrepareSpec& spec) const;
};
} // namespace openrig::graph
