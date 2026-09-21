#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <type_traits>

namespace openrig::realtime
{
template <typename T, std::size_t Capacity>
class SpscQueue
{
    static_assert(Capacity > 1, "SPSC capacity must be greater than one");
    static_assert(std::is_nothrow_copy_assignable_v<T>, "Realtime queue elements must copy without throwing");

public:
    [[nodiscard]] bool tryPush(const T& value) noexcept
    {
        const auto write = writeIndex_.load(std::memory_order_relaxed);
        const auto next = increment(write);
        if (next == readIndex_.load(std::memory_order_acquire))
            return false;

        storage_[write] = value;
        writeIndex_.store(next, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool tryPop(T& value) noexcept
    {
        const auto read = readIndex_.load(std::memory_order_relaxed);
        if (read == writeIndex_.load(std::memory_order_acquire))
            return false;

        value = storage_[read];
        readIndex_.store(increment(read), std::memory_order_release);
        return true;
    }

private:
    [[nodiscard]] static constexpr std::size_t increment(const std::size_t index) noexcept
    {
        return (index + 1) % Capacity;
    }

    std::array<T, Capacity> storage_{};
    std::atomic<std::size_t> readIndex_{0};
    std::atomic<std::size_t> writeIndex_{0};
};
} // namespace openrig::realtime
