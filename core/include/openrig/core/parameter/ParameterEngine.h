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
    // A hard limit prevents producer traffic from making a callback unbounded.
    template <typename Apply>
    [[nodiscard]] std::size_t drainUpTo(const std::size_t maxEvents, Apply&& apply) noexcept
    {
        ParameterEvent event;
        std::size_t consumed = 0;
        while (consumed < maxEvents && events_.tryPop(event))
        {
            apply(event);
            ++consumed;
        }
        return consumed;
    }

private:
    realtime::SpscQueue<ParameterEvent, kQueueCapacity> events_;
};
} // namespace openrig
