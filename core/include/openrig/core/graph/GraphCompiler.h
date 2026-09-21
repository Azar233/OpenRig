#pragma once

#include "openrig/core/audio/PrepareSpec.h"
#include "openrig/core/graph/CompiledGraph.h"
#include "openrig/core/graph/GraphDescription.h"
#include "openrig/core/graph/NodeRegistry.h"

#include <memory>
#include <string>

namespace openrig::graph
{
struct GraphCompileResult
{
    std::unique_ptr<CompiledGraph> graph;
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
