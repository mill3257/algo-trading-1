#include "core/ohlcv_aggregator.h"
#include <algorithm>

namespace dsp_trader::core {

OHLCVAggregator::OHLCVAggregator(int64_t bar_duration_ns, BarCallback cb)
    : bar_duration_ns_(bar_duration_ns), on_bar_complete_(std::move(cb)) {}

void OHLCVAggregator::on_tick(const Tick& tick) {
    if (!bar_open_) reset_bar(tick.ts_ns);

    // Roll bar if this tick lands past the current window
    if (tick.ts_ns >= current_.ts_open_ns + bar_duration_ns_) {
        emit_bar();
        reset_bar(tick.ts_ns);
    }

    if (current_.tick_count == 0) current_.open = tick.price;
    current_.high = std::max(current_.high, tick.price);
    current_.low  = std::min(current_.low,  tick.price);
    current_.close = tick.price;

    // Incremental VWAP: vwap_new = (vwap_old * vol_old + price * size) / vol_new
    double prev_vol   = current_.volume;
    current_.volume  += tick.size;
    if (current_.volume > 0.0)
        current_.vwap = (current_.vwap * prev_vol + tick.price * tick.size)
                      / current_.volume;

    current_.ts_close_ns = tick.ts_ns;
    ++current_.tick_count;
}

void OHLCVAggregator::flush() {
    if (bar_open_ && current_.tick_count > 0) emit_bar();
}

void OHLCVAggregator::reset_bar(Timestamp ts) {
    current_          = OHLCVBar{};
    current_.ts_open_ns = ts;
    current_.high     = -1e18;
    current_.low      =  1e18;
    bar_open_         = true;
}

void OHLCVAggregator::emit_bar() {
    if (on_bar_complete_) on_bar_complete_(current_);
    bar_open_ = false;
}

} // namespace dsp_trader::core
