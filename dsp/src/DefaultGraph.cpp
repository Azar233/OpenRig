#include "openrig/dsp/DefaultGraph.h"

#include "openrig/core/graph/GraphDescription.h"
#include "openrig/core/graph/NodeRegistry.h"
#include "openrig/dsp/drive/SoftClipNode.h"
#include "openrig/dsp/utility/GainNode.h"

#include <string>

namespace openrig::dsp
{
graph::GraphCompileResult compileDefaultGraph(const PrepareSpec& spec)
{
    graph::NodeRegistry registry;
    static_cast<void>(registry.registerNode(std::string{kGainNodeType}, &makeGainNode));
    static_cast<void>(registry.registerNode(std::string{kSoftClipNodeType}, &makeSoftClipNode));

    const graph::GraphDescription description{
        .nodes = {
            {kDefaultInputGainNodeId, std::string{kGainNodeType}, false, {{"gain", 1.0F}}},
            {kDefaultDriveNodeId, std::string{kSoftClipNodeType}, false, {{"drive", 2.0F}, {"level", 0.75F}}},
            {kDefaultOutputGainNodeId, std::string{kGainNodeType}, false, {{"gain", 1.0F}}},
        },
        .connections = {
            {kDefaultInputGainNodeId, kDefaultDriveNodeId},
            {kDefaultDriveNodeId, kDefaultOutputGainNodeId},
        },
    };

    return graph::GraphCompiler{}.compile(description, registry, spec);
}
} // namespace openrig::dsp
