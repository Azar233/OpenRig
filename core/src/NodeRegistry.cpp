#include "openrig/core/graph/NodeRegistry.h"

namespace openrig::graph
{
bool NodeRegistry::registerNode(std::string type, const Factory factory)
{
    if (type.empty() || factory == nullptr)
        return false;

    return factories_.emplace(std::move(type), factory).second;
}

bool NodeRegistry::contains(const std::string_view type) const
{
    return factories_.find(std::string(type)) != factories_.end();
}

std::unique_ptr<AudioNode> NodeRegistry::create(const std::string_view type) const
{
    const auto iterator = factories_.find(std::string(type));
    return iterator == factories_.end() ? nullptr : iterator->second();
}
} // namespace openrig::graph
