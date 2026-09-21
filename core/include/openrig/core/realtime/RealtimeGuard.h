#pragma once

namespace openrig::realtime
{
// The allocation hook is intentionally deferred until T037. This scope already makes
// thread ownership explicit and provides a stable API for debug instrumentation.
class RealtimeGuard
{
public:
    [[nodiscard]] static bool isRealtimeThread() noexcept { return realtimeDepth_ > 0; }

private:
    friend class RealtimeScope;
    inline static thread_local unsigned int realtimeDepth_ = 0;
};

class RealtimeScope
{
public:
    RealtimeScope() noexcept { ++RealtimeGuard::realtimeDepth_; }
    ~RealtimeScope() { --RealtimeGuard::realtimeDepth_; }

    RealtimeScope(const RealtimeScope&) = delete;
    RealtimeScope& operator=(const RealtimeScope&) = delete;
};
} // namespace openrig::realtime
