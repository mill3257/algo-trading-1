#include "core/ohlcv_aggregator.h"
#include "alpha/alpha_generator.h"
#include "alpha/regime_detector.h"
#include "risk/risk_manager.h"
#include "risk/fill_model.h"
#include "execution/paper_trader.h"
#include <cassert>
#include <cstdio>

// Smoke test: construct every module, push a synthetic bar through, no crash.
// This will compile and run even before any DSP logic is implemented —
// it just verifies the module wiring is correct.

int main() {
    // Construct all modules
    dsp_trader::risk::RiskManager   risk({}, 100'000.0);
    dsp_trader::risk::FillModel     fill({});
    dsp_trader::execution::PaperTrader trader({}, risk, fill);
    dsp_trader::alpha::AlphaGenerator  alpha({});
    dsp_trader::alpha::RegimeDetector  regime;

    // Synthetic bar
    dsp_trader::core::OHLCVBar bar{};
    bar.open  = 450.0;
    bar.high  = 451.0;
    bar.low   = 449.0;
    bar.close = 450.5;
    bar.volume = 1000.0;
    bar.tick_count = 100;

    // Drive one step through the alpha generator
    // (will return a zeroed AlphaSignal until DSP logic is implemented)
    auto sig = alpha.on_bar(bar, regime.current_regime());
    (void)sig;

    std::puts("[PASS] pipeline smoke test — all modules constructed and connected");
    return 0;
}
