#pragma once

#include "openrig/core/audio/AudioNode.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace openrig::graph
{
class NodeRegistry
{
public:
    using Factory = std::unique_ptr<AudioNode> (*)();

    [[nodiscard]] bool registerNode(std::string type, Factory factory);
    [[nodiscard]] bool contains(std::string_view type) const;
    [[nodiscard]] std::unique_ptr<AudioNode> create(std::string_view type) const;

private:
    std::unordered_map<std::string, Factory> factories_;
};
} // namespace openrig::graph
