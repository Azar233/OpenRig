#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace openrig
{
enum class ResourceKind
{
    ImpulseResponse,
    NamModel,
    PresetAsset,
};

struct ResourceRequest
{
    std::uint64_t requestId = 0;
    ResourceKind kind = ResourceKind::PresetAsset;
    std::filesystem::path path;
};

// Worker-thread boundary. Concrete resources are introduced with their integrations.
class ResourceManager
{
public:
    virtual ~ResourceManager() = default;
    virtual bool submit(ResourceRequest request) = 0;
    virtual void pollCompleted() = 0;
};
} // namespace openrig
