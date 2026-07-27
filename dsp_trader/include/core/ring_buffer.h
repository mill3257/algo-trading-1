#pragma once
#include <atomic>
#include <array>
#include <optional>
#include <cstddef>

namespace dsp_trader::core {

/// Single-producer, single-consumer lock-free ring buffer.
///
/// Design:
///   - Power-of-two capacity → index masking instead of modulo
///   - head_ and tail_ on separate cache lines → no false sharing
///   - Acquire/release ordering at the producer/consumer boundary only
///   - T must be trivially copyable (Tick qualifies)
///
/// TODO (you implement):
///   - push(const T&)  : write item, advance head, return false if full
///   - pop()           : read item, advance tail, return nullopt if empty
///   - size() / empty(): snapshot queries (relaxed loads OK here)
template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two");
    static_assert(std::is_trivially_copyable_v<T>);

    static constexpr std::size_t kMask = Capacity - 1;

public:
    RingBuffer() = default;

    // TODO: implement — producer thread only
    bool push(const T& item) noexcept;

    // TODO: implement — consumer thread only
    std::optional<T> pop() noexcept;

    bool empty() const noexcept {
        return tail_.load(std::memory_order_acquire) ==
               head_.load(std::memory_order_acquire);
    }
    std::size_t size() const noexcept {
        const auto h = head_.load(std::memory_order_acquire);
        const auto t = tail_.load(std::memory_order_acquire);
        return (h - t + Capacity) & kMask;
    }
    static constexpr std::size_t capacity() { return Capacity; }

private:
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
    alignas(64) std::array<T, Capacity>  data_{};
};

} // namespace dsp_trader::core
