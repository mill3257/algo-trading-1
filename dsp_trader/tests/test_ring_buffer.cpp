#include "core/ring_buffer.h"
#include "core/tick.h"
#include <cassert>
#include <cstdio>
#include <thread>

using namespace dsp_trader::core;

// TODO: fill in once push/pop are implemented

void test_empty_on_init() {
    RingBuffer<int, 8> rb;
    assert(rb.empty());
    std::puts("[PASS] empty on init");
}

void test_push_pop_roundtrip() {
    // TODO: push a value, pop it, assert equality
    std::puts("[SKIP] push/pop roundtrip — implement push/pop first");
}

void test_full_rejection() {
    // TODO: fill buffer to capacity, assert next push returns false
    std::puts("[SKIP] full rejection — implement push first");
}

void test_fifo_ordering() {
    // TODO: push N items, pop N items, verify order preserved
    std::puts("[SKIP] FIFO ordering — implement push/pop first");
}

void test_tick_roundtrip() {
    // TODO: push a Tick, verify fields survive the round-trip
    std::puts("[SKIP] Tick round-trip — implement push/pop first");
}

void test_spsc_concurrent() {
    // TODO: producer thread pushes 0..N-1, consumer accumulates sum,
    //       verify sum == N*(N-1)/2
    std::puts("[SKIP] SPSC concurrent — implement push/pop first");
}

int main() {
    test_empty_on_init();
    test_push_pop_roundtrip();
    test_full_rejection();
    test_fifo_ordering();
    test_tick_roundtrip();
    test_spsc_concurrent();
    return 0;
}
