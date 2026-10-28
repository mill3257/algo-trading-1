#pragma once
#include <atomic>
#include <array>
#include <optional>
#include <cstddef>

namespace dsp_trader::core {

//Single-producer, single-consumer lock-free ring buffer.

//Design:
//Power-of-two capacity --> index masking instead of modulo
//head_ and tail_ on separate cache lines = no false sharing
//Acquire/release ordering at the producer/consumer boundary only
//T must be trivially copyable (Tick qualifies)

//TODO:
//push(const T&): write item, advance head, return false if full
//pop(): read item, advance tail, return nullopt if empty
//size() / empty(): snapshot queries
template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two");
    static_assert(std::is_trivially_copyable_v<T>);

    static constexpr std::size_t kMask = Capacity - 1;

public:
    RingBuffer() = default;

    bool push(const T& item) noexcept {
        const std::size_t h = head_.load(std::memory_order_relaxed);
        if (h - tail_.load(std::memory_order_acquire) == Capacity)
            return false;
        data_[h & kMask] = item;
        head_.store(h + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() noexcept {
        const std::size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire))
            return std::nullopt; //empty
        T item = data_[t & kMask];
        tail_.store(t + 1, std::memory_order_release);
        return item;
    }

    bool empty() const noexcept {
        return tail_.load(std::memory_order_acquire) ==
               head_.load(std::memory_order_acquire);
    }
    std::size_t size() const noexcept {
        const auto h = head_.load(std::memory_order_acquire);
        const auto t = tail_.load(std::memory_order_acquire);
        return h - t;
    }
    static constexpr std::size_t capacity() { return Capacity; }

private:
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
    alignas(64) std::array<T, Capacity> data_{};
};

} 
