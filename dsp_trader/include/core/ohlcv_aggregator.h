#pragma once
#include "core/tick.h"
#include <functional>

namespace dsp_trader::core {

struct OHLCVBar {
    Timestamp ts_open_ns;
    Timestamp ts_close_ns;
    double open, high, low, close;
    double volume;
    double vwap;       // volume-weighted average price
    int    tick_count;
};

/// Aggregates a raw tick stream into fixed-duration OHLCV bars.
/// Calls on_bar_complete whenever a bar window closes.
/// bar_duration_ns is configurable (e.g. 1s = 1e9, 1min = 60e9).
class OHLCVAggregator {
public:
    using BarCallback = std::function<void(const OHLCVBar&)>;

    explicit OHLCVAggregator(int64_t bar_duration_ns, BarCallback on_bar_complete);

    void on_tick(const Tick& tick);
    void flush(); // force-close the current partial bar (end of session)

    const OHLCVBar& current_bar() const { return current_; }

private:
    int64_t     bar_duration_ns_;
    BarCallback on_bar_complete_;
    OHLCVBar    current_{};
    bool        bar_open_{false};

    void reset_bar(Timestamp ts);
    void emit_bar();
};

} // namespace dsp_trader::core
