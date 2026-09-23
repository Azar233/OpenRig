#pragma once

#include "openrig/core/audio/AudioBlockView.h"
#include "openrig/core/audio/AudioNode.h"
#include "openrig/core/audio/PrepareSpec.h"
#include "openrig/core/parameter/Parameter.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace openrig::graph
{
class GraphCompiler;

enum class GraphProcessStatus
{
    Ok,
    InvalidBuffer,
    BlockTooLarge,
};

class CompiledGraph
{
public:
    CompiledGraph() = default;
    CompiledGraph(const CompiledGraph&) = delete;
    CompiledGraph& operator=(const CompiledGraph&) = delete;
    CompiledGraph(CompiledGraph&&) noexcept = default;
    CompiledGraph& operator=(CompiledGraph&&) noexcept = default;

    [[nodiscard]] GraphProcessStatus process(
        const AudioBlockView& input,
        const AudioBlockView& output,
        std::uint32_t numFrames) noexcept;

    [[nodiscard]] bool applyParameter(const ParameterEvent& event) noexcept;

    [[nodiscard]] std::uint32_t latencySamples() const noexcept { return latencySamples_; }
    [[nodiscard]] std::uint32_t tailSamples() const noexcept { return tailSamples_; }
    [[nodiscard]] std::size_t nodeCount() const noexcept { return nodes_.size(); }

private:
    friend class GraphCompiler;

    PrepareSpec spec_{};
    std::vector<std::unique_ptr<AudioNode>> nodes_;
    struct NodeRoute
    {
        NodeId id = kInvalidNodeId;
        AudioNode* node = nullptr;
    };
    std::vector<NodeRoute> routes_;
    std::vector<std::uint32_t> nodeOutputChannels_;
    std::vector<Sample> scratchA_;
    std::vector<Sample> scratchB_;
    std::vector<Sample*> channelsA_;
    std::vector<Sample*> channelsB_;
    std::uint32_t latencySamples_ = 0;
    std::uint32_t tailSamples_ = 0;
};
} // namespace openrig::graph
