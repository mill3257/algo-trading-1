#pragma once
#include <cstdint>

namespace dsp_trader::core {

using Timestamp = int64_t; // nanoseconds since Unix epoch

/// Raw market tick — the smallest observable event.
/// Kept strictly POD so it can live in the lock-free ring buffer
/// without any copy/move overhead beyond memcpy.
struct Tick {
    Timestamp ts_ns;     // exchange timestamp
    double    price;     // last trade price
    double    bid;       // best bid
    double    ask;       // best ask
    double    size;      // trade size (shares)
    char      symbol[8]; // null-terminated, e.g. "SPY\0"

    double mid()    const { return (bid + ask) * 0.5; }
    double spread() const { return ask - bid; }
};

static_assert(sizeof(Tick) <= 64, "Tick must fit in one cache line");

} // namespace dsp_trader::core
