#include "test_support.h"

#include "openrig/core/parameter/ParameterEngine.h"

#include <array>

int main()
{
    TestSuite tests;
    openrig::ParameterEngine engine;
    for (openrig::NodeId id = 1; id <= 5; ++id)
        tests.check(engine.enqueue({{id, 0}, static_cast<openrig::Sample>(id)}), "Parameter event enqueue succeeds");

    std::array<openrig::NodeId, 5> received{};
    std::size_t index = 0;
    const auto first = engine.drainUpTo(2, [&](const openrig::ParameterEvent& event) noexcept {
        received[index++] = event.address.node;
    });
    tests.check(first == 2 && index == 2, "Per-block parameter drain is bounded");

    const auto second = engine.drainUpTo(8, [&](const openrig::ParameterEvent& event) noexcept {
        received[index++] = event.address.node;
    });
    tests.check(second == 3 && index == 5, "Remaining parameter events stay queued");
    for (std::size_t item = 0; item < received.size(); ++item)
        tests.check(received[item] == item + 1, "Parameter transport preserves FIFO order");

    tests.check(engine.drainUpTo(0, [](const auto&) noexcept {}) == 0, "Zero event budget consumes nothing");

    openrig::ParameterEngine saturated;
    bool acceptedAllUsableSlots = true;
    for (std::size_t item = 0; item + 1 < openrig::ParameterEngine::kQueueCapacity; ++item)
        acceptedAllUsableSlots = acceptedAllUsableSlots && saturated.enqueue({{1, 0}, 1.0F});
    tests.check(acceptedAllUsableSlots, "Parameter queue accepts every usable slot");
    tests.check(!saturated.enqueue({{1, 0}, 1.0F}), "Parameter queue overflow is observable");
    return tests.finish("parameter transport tests");
}
