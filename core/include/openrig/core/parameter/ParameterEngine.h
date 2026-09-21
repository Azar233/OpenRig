#pragma once

#include "openrig/core/parameter/Parameter.h"
#include "openrig/core/realtime/SpscQueue.h"

#include <cstddef>

namespace openrig
{
class ParameterEngine
{
public:
    static constexpr std::size_t kQueueCapacity = 1024;

    // Control/UI producer. A false return means backpressure and must be handled by the caller.
    [[nodiscard]] bool enqueue(const ParameterEvent& event) noexcept { return events_.tryPush(event); }

    // Audio-thread consumer. The callback must be noexcept and allocation-free.
    template <typename Apply>
    void drain(Apply&& apply) noexcept
    {
        ParameterEvent event;
        while (events_.tryPop(event))
            apply(event);
    }

private:
    realtime::SpscQueue<ParameterEvent, kQueueCapacity> events_;
};
} // namespace openrig
